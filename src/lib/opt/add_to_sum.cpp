#include "add_to_sum.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"

using namespace llvm;
using namespace std;

namespace sc::opt::add_to_sum {
PreservedAnalyses AddToSum::run(Function &F, FunctionAnalysisManager &FAM) {
  outs() << "-- [Begin add-to-sum Pass] --\n";

  /* ===== PHASE 1 ============================
   * Calculate the Depth of Add instructions */

  DenseMap<Instruction *, int> AddDepthMap;
  // Define a recursive function to compute the depth of an Instruction
  function<int(Value *)> computeDepth = [&](Value *val) {
    // If val is not an Instruction, return 0
    auto *inst = dyn_cast<Instruction>(val);
    if (inst == nullptr) {
      return 0;
    }
    // If inst is not an `add`/`sub` instruction, return 0
    if (inst->getOpcode() != Instruction::Add &&
        inst->getOpcode() != Instruction::Sub) {
      return 0;
    }
    // If the depth of this Instruction has already been computed, return it
    auto iter = AddDepthMap.find(inst);
    if (iter != AddDepthMap.end()) {
      return iter->second;
    }
    // Compute the depth of the operands and add 1
    int depth1 = computeDepth(inst->getOperand(0));
    int depth2 = computeDepth(inst->getOperand(1));
    int depth = max(depth1, depth2) + 1;

    // Store the computed depth in the map and return it
    AddDepthMap[inst] = depth;
    return depth;
  };

  // Compute the depth of all add instructions in the function
  for (BasicBlock &BB : F) {
    for (Instruction &I : BB) {
      computeDepth(&I);
    }
  }

  /* ===== PHASE 2 ============================
   * Get Ready to Traverse Add instructions */

  // Sort the values in increasing order of depth
  vector<pair<Instruction *, int>> AddDepthVec(AddDepthMap.begin(),
                                               AddDepthMap.end());
  std::sort(AddDepthVec.begin(), AddDepthVec.end(),
            [](auto &lhs, auto &rhs) { return lhs.second < rhs.second; });

  // Create a map of Instructions to vector of potential sum operands
  DenseMap<Instruction *, SmallVector<Value *, 8>> AddToSumOps;
  DenseMap<Instruction *, SmallVector<bool, 8>> AddToSumOpsSign;
  DenseMap<Instruction *, SmallVector<int>> AddToSumOpsCount;
  set<Instruction *> toDeleteSet;
  vector<Instruction *> toDeleteVec;

  // Add an operand to an instruction's potential sum operand list
  function<void(Instruction *, Value *, bool, int weight)> addOp =
      [&](Instruction *inst, Value *val, bool sign, int weight) {
        if (auto *cnst = dyn_cast<ConstantInt>(val)) {
          // Constants are always given a positive `sign` by flipping their
          // values, if necessary. (Of course the constant values themselves can
          // be negative.) Constants are also merged together for space.
          if (cnst->isZero()) {
            return;
          }
          if (sign) {
            auto *negConst = dyn_cast<ConstantInt>(ConstantExpr::getNeg(cnst));
            assert(negConst);
            cnst = negConst;
            val = negConst;
            sign = false;
          }
          // Search for existing constants, and merge
          for (int idx = 0; idx < AddToSumOps[inst].size(); ++idx) {
            if (auto *existing_cnst =
                    dyn_cast<ConstantInt>(AddToSumOps[inst][idx])) {
              AddToSumOps[inst][idx] =
                  ConstantExpr::getAdd(existing_cnst, cnst);
              AddToSumOpsCount[inst][idx] += weight;
              return;
            }
          }
        }
        AddToSumOps[inst].push_back(val);
        AddToSumOpsSign[inst].push_back(sign);
        AddToSumOpsCount[inst].push_back(weight);
        assert(AddToSumOps[inst].size() <= 8);
      };

  /* ===== PHASE 3 ============================
   * Traverse Add instructions, Creating Sum Operand Information */

  // Loop over all Add instructions, in increasing depth order
  for (auto &entry : AddDepthVec) {
    Instruction *inst = entry.first;
    bool op2_sign = inst->getOpcode() == Instruction::Sub;
    int depth = entry.second;
    if (depth == 1) { // Both operands are non-add!
      for (int i = 0; i < 2; ++i) {
        addOp(inst, inst->getOperand(i), i && op2_sign, 1);
      }
    } else { // At least one operand is an add!
      assert(depth > 1);
      auto *op1_inst = dyn_cast<Instruction>(inst->getOperand(0));
      auto *op2_inst = dyn_cast<Instruction>(inst->getOperand(1));

      // Add non-add operands first
      for (int i = 0; i < 2; ++i) {
        Instruction *op_inst = i == 0 ? op1_inst : op2_inst;
        if (op_inst == nullptr ||
            op_inst->getOpcode() != Instruction::Add &&
                op_inst->getOpcode() != Instruction::Sub) { // non-add/sub
          addOp(inst, inst->getOperand(i), i && op2_sign, 1);
        }
      }
      // Add `add` operands
      for (int i = 0; i < 2; ++i) {
        Instruction *op_inst = i == 0 ? op1_inst : op2_inst;
        if (op_inst && (op_inst->getOpcode() == Instruction::Add ||
                        op_inst->getOpcode() == Instruction::Sub)) { // add/subs
          assert(!AddToSumOps[op_inst].empty());
          if (!op_inst->hasOneUse()) { // non-one-use `add`/`sub` operand
            addOp(inst, op_inst, i && op2_sign, 1);
            continue;
          }
          // Calculate how many more `sum` operands can be used
          int max_ops_to_add = min(7, (int)(8 - AddToSumOps[inst].size()));
          // too many operands for `sum`
          if (AddToSumOps[op_inst].size() > max_ops_to_add) {
            addOp(inst, op_inst, i && op2_sign, 1);
            continue;
          }
          // Operands can be merged now!
          // mark operands for deletion
          toDeleteSet.insert(op_inst);
          toDeleteVec.push_back(op_inst);
          // add operands' operands to current instruction's operands
          for (int idx = 0; idx < AddToSumOps[op_inst].size(); ++idx) {
            addOp(inst, AddToSumOps[op_inst][idx],
                  (i && op2_sign) ^ AddToSumOpsSign[op_inst][idx],
                  AddToSumOpsCount[op_inst][idx]);
          }
        }
      }
    }
  }

  /* ===== PHASE 4 ============================
   * Change Add Instructions into Sum Instructions */

  // It is critical that traversal is in decreasing order of depth,
  // because only so does `replaceAllUsesWith` correctly replace all occurrences
  // of the Instruction. This is because replacing `add` instructions into
  // `sum` instructions introduces new usages of lower-depth `add` Instructions.
  set<Instruction *> CheckForDeletion;
  SmallVector<CallInst *> SumInstructions;
  set<Instruction *> DeletedInstructions;
  for (auto entry = AddDepthVec.rbegin(); entry != AddDepthVec.rend();
       ++entry) {
    Instruction *inst = (*entry).first;
    int depth = (*entry).second;
    int weight_total = 0;
    for (auto &weight : AddToSumOpsCount[inst]) {
      weight_total += weight;
    }
    if (!toDeleteSet.count(inst) && weight_total >= 3) {
      // For 3 operands, don't replace with sum if there are any `sub`s involved
      if (weight_total == 3) {
        bool has_sign = false;
        for (auto &sign : AddToSumOpsSign[inst]) {
          if (sign) {
            has_sign = true;
            break;
          }
        }
        if (has_sign) {
          continue;
        }
      }
      // Replace signed operands
      for (int idx = 0; idx < AddToSumOps[inst].size(); ++idx) {
        if (AddToSumOpsSign[inst][idx]) {
          // All constants should have positive sign
          assert(!dyn_cast<ConstantInt>(AddToSumOps[inst][idx]));
          // Instructions are multiplied by -1
          auto negInst = BinaryOperator::CreateMul(
              AddToSumOps[inst][idx], ConstantInt::get(inst->getType(), -1),
              "neg." + AddToSumOps[inst][idx]->getName());
          if (auto *op = dyn_cast<Instruction>(AddToSumOps[inst][idx])) {
            // operand was an instruction
            negInst->insertAfter(op);
          } else {
            // operand was an argument or a global variable
            F.getEntryBlock().getInstList().push_front(negInst);
          }
          AddToSumOps[inst][idx] = negInst;
        }
      }

      // Now check for possible operand expansions
      if (AddToSumOps.count(inst)) { // If there are operands
        for (int idx = 0; idx < AddToSumOps[inst].size(); ++idx) {
          ulong operand_space_left = 8 - AddToSumOps[inst].size();
          auto *op = dyn_cast<Instruction>(AddToSumOps[inst][idx]);
          if (op == nullptr)
            continue;
          // Check for possible `mul` expansion
          if (op->getOpcode() == Instruction::Mul) {
            for (int op_i = 0; op_i < 2; ++op_i) {
              if (auto *const_op =
                      dyn_cast<ConstantInt>(op->getOperand(op_i))) {
                // If `mul` operand has a small enough constant as its operand
                if (!const_op->isNegative() &&
                    const_op->getZExtValue() <= operand_space_left + 1) {
                  CheckForDeletion.insert(op);
                  AddToSumOps[inst].erase(AddToSumOps[inst].begin() + idx);
                  for (int i = 0; i < const_op->getZExtValue(); ++i) {
                    AddToSumOps[inst].push_back(op->getOperand((op_i + 1) % 2));
                  }
                  --idx;
                  break;
                }
              }
            }
          } else if (op->getOpcode() == Instruction::Add) {
            // Expand non-one-use `add`
            // May expand multiple times
            if (operand_space_left >= AddToSumOps[op].size()) {
              CheckForDeletion.insert(op);
              AddToSumOps[inst].erase(AddToSumOps[inst].begin() + idx);
              for (auto &opop : AddToSumOps[op]) {
                AddToSumOps[inst].push_back(opop);
              }
              --idx;
            }
          }
        }
      }

      outs() << "Depth: " << depth << " | " << inst->getName() << " | ";
      if (AddToSumOps.count(inst)) {
        for (auto &val : AddToSumOps[inst]) {
          outs() << val->getName() << ", ";
        }
      }
      outs() << '\n';
      outs().flush();
      IRBuilder<> Builder(inst);
      LLVMContext &Ctx = inst->getContext();
      FunctionType *FuncType;
      Module *M = inst->getModule();
      FunctionCallee FC;
      Type *intType = inst->getType();
      Constant *zero = ConstantInt::get(intType, 0);
      FuncType = FunctionType::get(intType,
                                   {
                                       intType,
                                       intType,
                                       intType,
                                       intType,
                                       intType,
                                       intType,
                                       intType,
                                       intType,
                                   },
                                   false);
      std::string typeName;
      llvm::raw_string_ostream typeStream(typeName);
      intType->print(typeStream);
      FC = M->getOrInsertFunction("int_sum_" + typeName, FuncType);
      int zero_count = 8 - (int)AddToSumOps[inst].size();
      for (int i = 0; i < zero_count; ++i) {
        AddToSumOps[inst].push_back(zero);
      }
      ArrayRef<Value *> args = makeArrayRef(AddToSumOps[inst]);

      Value *sum_call = Builder.CreateCall(FC, args);
      StringRef prev_inst_name = inst->getName();
      inst->replaceAllUsesWith(sum_call);
      DeletedInstructions.insert(inst);
      inst->eraseFromParent();
      sum_call->setName(prev_inst_name);

      auto *sum_call_inst = dyn_cast<CallInst>(sum_call);
      assert(sum_call_inst);
      SumInstructions.push_back(sum_call_inst);
    }
  }

  /* ===== PHASE 5 ============================
   * Remove Redundant Instructions */

  for (auto inst = toDeleteVec.rbegin(); inst != toDeleteVec.rend(); ++inst) {
    // This check is necessary because some `sub` instructions might not
    // actually have been replaced by `sum`
    if ((*inst)->use_empty()) {
      DeletedInstructions.insert(*inst);
      (*inst)->eraseFromParent();
    }
  }

  for (auto &inst : CheckForDeletion) {
    if (!DeletedInstructions.count(inst) && inst->use_empty()) {
      inst->eraseFromParent();
    }
  }

  /* ===== PHASE 6 ============================
   * Shift operands down if possible */
  for (auto &sum_inst : SumInstructions) {
    for (int idx = 0; idx < sum_inst->getNumOperands(); ++idx) {
      Value *arg = sum_inst->getOperand(idx);
      if (auto arg_inst = dyn_cast<Instruction>(arg)) {
        if (arg_inst->hasOneUse()) {
          assert(*arg_inst->user_begin() == sum_inst);
          arg_inst->moveBefore(sum_inst);
        }
      }
    }
  }

  return PreservedAnalyses::none();
}

extern "C" ::llvm::PassPluginLibraryInfo llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "AddToSum", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "add-to-sum") {
                    FPM.addPass(AddToSum());
                    return true;
                  }
                  return false;
                });
          }};
}
} // namespace sc::opt::add_to_sum
