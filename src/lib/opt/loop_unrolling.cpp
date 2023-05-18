#include "loop_unrolling.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Transforms/Utils/UnrollLoop.h"
#include "llvm/Transforms/Utils/LCSSA.h"

using namespace llvm;

namespace sc::opt::loop_unrolling {

PreservedAnalyses LoopUnrolling::run(Function &F,
                                     FunctionAnalysisManager &FAM) {

  LCSSAPass lcssa_pass;
  lcssa_pass.run(F, FAM);

  LoopInfo &LI = FAM.getResult<LoopAnalysis>(F);
  ScalarEvolution &SE = FAM.getResult<ScalarEvolutionAnalysis>(F);
  DominatorTree &DT = FAM.getResult<DominatorTreeAnalysis>(F);

  for (Loop *L : LI) {
    if (L->isInnermost() && L->isLCSSAForm(DT)) {
      // Determine the loop unrolling options based on your requirements
      UnrollLoopOptions ULO{8, true, true, true, true, true};

      // Set the remaining parameters as needed
      Loop *RemainderLoop = nullptr;
      bool PreserveLCSSA = false;

      // Perform loop unrolling using UnrollLoop function
      LoopUnrollResult Result = UnrollLoop(L, ULO, &LI, &SE, &DT, nullptr, nullptr, nullptr, PreserveLCSSA);
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
