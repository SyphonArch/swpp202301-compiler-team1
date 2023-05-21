#include "loop_unrolling.h"
#include "llvm/Analysis/LoopInfo.h"
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
  ScalarEvolution &SE = FAM.getResult<ScalarEvolutionAnalysis>(F);
  DominatorTree &DT = FAM.getResult<DominatorTreeAnalysis>(F);

  for (Loop *L : LI) {
    if (L->isInnermost() && L->isLCSSAForm(DT)) {
      // Set loop unrolling options
      // Count, Force, Runtime, AllowExpensiveTripCount, UnrollRemainder,
      // ForgetAllSCEV All options are set to true to force maximal unrolling
      UnrollLoopOptions ULO{8, true, true, true, true, true};

      // Perform loop unrolling using UnrollLoop function
      LoopUnrollResult Result =
          UnrollLoop(L, ULO, &LI, &SE, &DT, nullptr, nullptr, nullptr, false);
    }
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
