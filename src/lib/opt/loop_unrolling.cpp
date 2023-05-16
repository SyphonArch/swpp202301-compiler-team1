#include "loop_unrolling.h"
#include "llvm/Analysis/AssumptionCache.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/OptimizationRemarkEmitter.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Transforms/Utils/UnrollLoop.h"

using namespace llvm;

namespace sc::opt::loop_unrolling {

PreservedAnalyses LoopUnrolling::run(Function &F,
                                     FunctionAnalysisManager &FAM) {
  LoopInfo &LI = FAM.getResult<LoopAnalysis>(F);

  for (Loop *L : LI) {
    // Determine the loop unrolling options based on your requirements
    UnrollLoopOptions ULO{2, true, true, true, true, true};

    // Set the remaining parameters as needed
    Loop *RemainderLoop = nullptr;
    bool PreserveLCSSA = false;

    // Perform loop unrolling using UnrollLoop function
    LoopUnrollResult Result = UnrollLoop(L, ULO, &LI, nullptr, nullptr, nullptr, nullptr, nullptr, PreserveLCSSA, &RemainderLoop);
  }

  return PreservedAnalyses::none();
}

extern "C" ::llvm::PassPluginLibraryInfo llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "LoopUnrolling", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "loop-unrolling") {
                    FPM.addPass(LoopUnrolling());
                    return true;
                  }
                  return false;
                });
          }};
}

} // namespace sc::opt::loop_unrolling
