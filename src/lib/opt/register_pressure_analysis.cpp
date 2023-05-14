#include "register_pressure_analysis.h"

#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/Debug.h"

#include "llvm/CodeGen/LiveIntervals.h"
#include "llvm/CodeGen/RegisterPressure.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"

#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"

#include <unordered_map>
#include <unordered_set>

#include "llvm/Analysis/AssumptionCache.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"

#include <algorithm>
#include <map>
#include <queue>

using namespace llvm;

#define DEBUG_TYPE "register-pressure-analysis"

namespace sc::opt::register_pressure_analysis {

uint64_t calculateRegisterPressure(Function &F) {
  // Traverse BasicBlock in post order.
  std::vector<BasicBlock *> postOrderBBs;
  for (BasicBlock *BB : post_order(&F.getEntryBlock())) {
    postOrderBBs.push_back(BB);
    LLVM_DEBUG(dbgs() << "Visiting BasicBlock: " << BB->getName() << "\n");
  }

  // Maps BasicBlocks to their corresponding live value counts.
  std::unordered_map<BasicBlock *, size_t> liveValueCounts;
  std::unordered_map<BasicBlock *, size_t> liveValueMaxCounts;

  // Initialize live value counts.
  for (auto *BB : postOrderBBs) {
    liveValueMaxCounts[BB] = 0;
  }

  // Iterate over the basic blocks in post-order.
  std::unordered_set<Value *> liveValues;
  for (auto *BB : postOrderBBs) {
    // Iterate over the instructions in reverse order.
    for (auto I = BB->rbegin(), E = BB->rend(); I != E; ++I) {
      // Remove the instruction from liveValues if it's a definition.
      liveValues.erase(&*I);

      // Add operands to liveValues set.
      for (unsigned i = 0; i < I->getNumOperands(); ++i) {
        Value *op = I->getOperand(i);
        if (isa<Instruction>(op) && !isa<Argument>(op)) {
          liveValues.insert(op);
        }
      
      }
      liveValueMaxCounts[BB] =
        std::max(liveValueMaxCounts[BB], liveValues.size());
      LLVM_DEBUG(dbgs() << "Instruction " << *I << ": liveValue size " << liveValues.size() << '\n');
    }
  }

  uint64_t maxRegisterPressure = 0;
  for (const auto &[_, liveValueMaxCount] : liveValueMaxCounts) {
    maxRegisterPressure =
        std::max(maxRegisterPressure, static_cast<uint64_t>(liveValueMaxCount));
  }

  return maxRegisterPressure;
}

uint64_t RegisterPressureAnalysis::run(Function &F,
                                                FunctionAnalysisManager &FAM) {
  return calculateRegisterPressure(F);
};

PreservedAnalyses RegisterPressurePrinterPass::run(Function &F, FunctionAnalysisManager &FAM) {
  uint64_t maxRegisterPressure = calculateRegisterPressure(F);
  outs() << "Approximate Register Pressure of @" << F.getName()
                    << ": " << maxRegisterPressure << '\n';

  return PreservedAnalyses::all();
};

extern "C" ::llvm::PassPluginLibraryInfo llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "RegisterPressurePrinterPass",
          LLVM_VERSION_STRING, [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "register-pressure-printer-pass") {
                    FPM.addPass(RegisterPressurePrinterPass());
                    return true;
                  }
                  return false;
                });
          }};
};
} // namespace sc::opt::register_pressure_analysis