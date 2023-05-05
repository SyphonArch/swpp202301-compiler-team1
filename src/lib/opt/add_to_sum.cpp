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
    // If inst is not an Add instruction, return 0
    if (inst->getOpcode() != Instruction::Add) {
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
      if (I.getOpcode() == Instruction::Add) {
        computeDepth(&I);
      }
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
  set<Instruction *> toDeleteSet;
  vector<Instruction *> toDeleteVec;

  /* ===== PHASE 3 ============================
   * Traverse Add instructions, Creating Sum Operand Information */

  // Loop over all Add instructions, in increasing depth order
  for (auto &entry : AddDepthVec) {
    Instruction *inst = entry.first;
    int depth = entry.second;
    if (depth == 1) { // Both operands are non-add!
      AddToSumOps[inst].push_back(inst->getOperand(0));
      AddToSumOps[inst].push_back(inst->getOperand(1));
    } else { // At least one operand is an add!
      assert(depth > 1);
      auto *op1_inst = dyn_cast<Instruction>(inst->getOperand(0));
      auto *op2_inst = dyn_cast<Instruction>(inst->getOperand(1));

      // Add non-add operands first
      for (int i = 0; i < 2; ++i) {
        Instruction *op_inst = i == 0 ? op1_inst : op2_inst;
        if (op_inst == nullptr ||
            op_inst->getOpcode() != Instruction::Add) { // non-add
          AddToSumOps[inst].push_back(inst->getOperand(i));
        }
      }
      // Add `add` operands
      for (int i = 0; i < 2; ++i) {
        Instruction *op_inst = i == 0 ? op1_inst : op2_inst;
        if (op_inst && op_inst->getOpcode() == Instruction::Add) { // adds
          assert(!AddToSumOps[op_inst].empty());
          if (!op_inst->hasOneUse()) { // non-one-use `add` operand
            AddToSumOps[inst].push_back(op_inst);
            continue;
          }
          // Calculate how many more `sum` operands can be used
          int max_ops_to_add = min(7, (int)(8 - AddToSumOps[inst].size()));
          // too many operands for `sum`
          if (AddToSumOps[op_inst].size() > max_ops_to_add) {
            AddToSumOps[inst].push_back(op_inst);
            continue;
          }
          // Operands can be merged now!
          // mark operands for deletion
          toDeleteSet.insert(op_inst);
          toDeleteVec.push_back(op_inst);
          // add operands' operands to current instruction's operands
          for (auto &val : AddToSumOps[op_inst]) {
            AddToSumOps[inst].push_back(val);
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
  for (auto entry = AddDepthVec.rbegin(); entry != AddDepthVec.rend();
       ++entry) {
    Instruction *inst = (*entry).first;
    int depth = (*entry).second;

    if (!toDeleteSet.count(inst) && AddToSumOps[inst].size() >= 3) {
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
                auto const_op_val = const_op->getSExtValue();
                if (const_op_val <= operand_space_left + 1 &&
                    const_op_val >= 0) {
                  CheckForDeletion.insert(op);
                  AddToSumOps[inst].erase(AddToSumOps[inst].begin() + idx);
                  for (int i = 0; i < const_op_val; ++i) {
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

      Value *Call1 = Builder.CreateCall(FC, args);
      StringRef prev_inst_name = inst->getName();
      inst->replaceAllUsesWith(Call1);
      inst->eraseFromParent();
      Call1->setName(prev_inst_name);
    }
  }

  /* ===== PHASE 5 ============================
   * Remove Redundant Instructions */

  for (auto inst = toDeleteVec.rbegin(); inst != toDeleteVec.rend(); ++inst) {
    (*inst)->eraseFromParent();
  }

  for (auto &inst : CheckForDeletion) {
    if (inst->use_empty()) {
      (inst->eraseFromParent());
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
