#include "add_to_sum.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/Analysis/BranchProbabilityInfo.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

using namespace llvm;
using namespace std;

namespace sc::opt::add_to_sum {
PreservedAnalyses AddToSum::run(Function &F, FunctionAnalysisManager &FAM) {
  outs() << "-- [Begin add-to-sum Pass] --\n";

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

  // Sort the values in increasing order of depth
  vector<pair<Instruction *, int>> AddDepthVec(AddDepthMap.begin(),
                                               AddDepthMap.end());
  std::sort(AddDepthVec.begin(), AddDepthVec.end(),
            [](auto &lhs, auto &rhs) { return lhs.second < rhs.second; });

  // Create a map of Instructions to vector of potential sum operands
  DenseMap<Instruction *, SmallVector<Value *, 8>> AddToSumOps;
  set<Instruction *> toDeleteSet;
  vector<Instruction *> toDeleteVec;


  for (auto &entry : AddDepthVec) {
    Instruction *inst = entry.first;
    int depth = entry.second;
    if (depth == 1) { // Both operands are non-add!
      AddToSumOps[inst].push_back(inst->getOperand(0));
      AddToSumOps[inst].push_back(inst->getOperand(1));
    } else { // At least one operand is an add!
      auto *op1_inst = dyn_cast<Instruction>(inst->getOperand(0));
      auto *op2_inst = dyn_cast<Instruction>(inst->getOperand(1));
      bool inapt = false;
      for (int i = 0; i < 2; ++i) {
        Instruction *op_instr = i == 0 ? op1_inst : op2_inst;
        if (op_instr && op_instr->getOpcode() == Instruction::Add) {
          if (!op_instr->hasOneUse()) {
            inapt = true;
          }
        }
      }
      if (!inapt) { // Both operands are either non-add or one-use adds
        for (int i = 0; i < 2; ++i) {
          Instruction *op_inst = i == 0 ? op1_inst : op2_inst;
          if (op_inst == nullptr) { // non-add
            AddToSumOps[inst].push_back(inst->getOperand(i));
          }
        }
        for (int i = 0; i < 2; ++i) {
          Instruction *op_inst = i == 0 ? op1_inst : op2_inst;
          if (op_inst) {                        // adds
            if (AddToSumOps[op_inst].empty()) { // non-marked add
              AddToSumOps[inst].push_back(op_inst);
            } else { // marked add
              int max_ops_to_add = min(7, (int)(8 - AddToSumOps[inst].size()));
              if (AddToSumOps[op_inst].size() <= max_ops_to_add) {
                // parent add will be merged
                toDeleteSet.insert(op_inst);
                toDeleteVec.push_back(op_inst);
                for (auto &val : AddToSumOps[op_inst]) {
                  AddToSumOps[inst].push_back(val);
                }
              } else {
                AddToSumOps[inst].push_back(op_inst);
              }
            }
          }
        }
      }
    }
  }

  for (auto entry=AddDepthVec.rbegin(); entry!=AddDepthVec.rend(); ++entry) {
    Instruction *inst = (*entry).first;
    int depth = (*entry).second;
    outs() << "Depth: " << depth << " | " << inst->getName() << " | ";
    if (AddToSumOps.count(inst)) {
      for (auto &val : AddToSumOps[inst]) {
        outs() << val->getName() << ", ";
      }
      outs() << '\n';
    }

    if (!toDeleteSet.count(inst) && AddToSumOps[inst].size() >= 3) {
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
  for (auto inst=toDeleteVec.rbegin(); inst != toDeleteVec.rend(); ++inst) {
    (*inst)->eraseFromParent();
  }

  return PreservedAnalyses::none(); // or all();
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