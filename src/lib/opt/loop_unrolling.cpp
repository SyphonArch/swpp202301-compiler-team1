#include "loop_unrolling.h"
#include "llvm/Analysis/AssumptionCache.h"
#include "llvm/Analysis/DomTreeUpdater.h"
#include "llvm/Analysis/InstructionSimplify.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/MemorySSA.h"
#include "llvm/Analysis/MemorySSAUpdater.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/LoopRotationUtils.h"
#include "llvm/Transforms/Utils/LoopSimplify.h"
#include "llvm/Transforms/Utils/UnrollLoop.h"

using namespace llvm;

namespace sc::opt::loop_unrolling {

void rotate_loop(Function &F, FunctionAnalysisManager &FAM) {
  LoopInfo &LI = FAM.getResult<LoopAnalysis>(F);
  if (LI.empty())
    return;

  LoopAnalysisManager &LAM =
      FAM.getResult<LoopAnalysisManagerFunctionProxy>(F).getManager();
  DominatorTree &DT = FAM.getResult<DominatorTreeAnalysis>(F);
  AssumptionCache &AC = FAM.getResult<AssumptionAnalysis>(F);
  MemorySSA &MSSA = FAM.getResult<MemorySSAAnalysis>(F).getMSSA();
  MemorySSAUpdater MSSAU = MemorySSAUpdater(&MSSA);
  ScalarEvolution &SE = FAM.getResult<ScalarEvolutionAnalysis>(F);
  TargetTransformInfo &TTI = FAM.getResult<TargetIRAnalysis>(F);
  SimplifyQuery SQ = getBestSimplifyQuery(FAM, F);

  bool modified = false;
  for (Loop *L : LI.getLoopsInPreorder()) {
    if (LoopRotation(L, &LI, &TTI, &AC, &DT, &SE, &MSSAU, SQ, true, -1, true)) {
      LAM.invalidate(*L, PreservedAnalyses::none());
      modified = true;
    }
  }

  if (modified) {
    FAM.invalidate(F, PreservedAnalyses::none());
  }
}

void simplify_loop(Function &F, FunctionAnalysisManager &FAM) {
  LoopInfo &LI = FAM.getResult<LoopAnalysis>(F);
  if (LI.empty())
    return;

  DominatorTree &DT = FAM.getResult<DominatorTreeAnalysis>(F);
  AssumptionCache &AC = FAM.getResult<AssumptionAnalysis>(F);
  MemorySSA &MSSA = FAM.getResult<MemorySSAAnalysis>(F).getMSSA();
  MemorySSAUpdater MSSAU = MemorySSAUpdater(&MSSA);
  ScalarEvolution &SE = FAM.getResult<ScalarEvolutionAnalysis>(F);

  bool modified = false;

  for (Loop *L : LI.getLoopsInPreorder()) {
    modified |= simplifyLoop(L, &DT, &LI, &SE, &AC, &MSSAU, true);
  }

  if (modified) {
    FAM.invalidate(F, getLoopPassPreservedAnalyses());
  }
}

PreservedAnalyses LoopUnrolling::run(Function &F,
                                     FunctionAnalysisManager &FAM) {

  rotate_loop(F, FAM);
  simplify_loop(F, FAM);

  LoopInfo &LI = FAM.getResult<LoopAnalysis>(F);
  ScalarEvolution &SE = FAM.getResult<ScalarEvolutionAnalysis>(F);
  DominatorTree &DT = FAM.getResult<DominatorTreeAnalysis>(F);
  DomTreeUpdater DTU(DT, DomTreeUpdater::UpdateStrategy::Eager);
  AssumptionCache &AC = FAM.getResult<AssumptionAnalysis>(F);
  TargetTransformInfo &TTI = FAM.getResult<TargetIRAnalysis>(F);

  for (Loop *L : LI.getLoopsInPreorder()) {
    if (L->isInnermost() && L->isLCSSAForm(DT)) {
      // Set loop unrolling options
      // Count, Force, Runtime, AllowExpensiveTripCount, UnrollRemainder,
      // ForgetAllSCEV
      // Remainder is not unrolled as gains are minimal
      UnrollLoopOptions ULO{8, true, true, true, false, true};

      // Perform loop unrolling using UnrollLoop function
      LoopUnrollResult Result =
          UnrollLoop(L, ULO, &LI, &SE, &DT, &AC, &TTI, nullptr, true);

      // Merge blocks in the loop, if possible.
      // Continues to merge while merges happen.
      bool is_merged;
      do {
        vector<BasicBlock*> loop_blocks = L->getBlocksVector();
        is_merged = false;
        for (BasicBlock *BB : loop_blocks) {
          is_merged |= MergeBlockIntoPredecessor(BB, &DTU, &LI);
        }
      } while (is_merged);
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
