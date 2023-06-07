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

#define MAX_LOOP_SIZE 100
#define DO_LCSSA_CONVERSION true

using namespace llvm;

namespace sc::opt::loop_unrolling {

// Convert loops into LCSSA form
void lcssa(Function &F, FunctionAnalysisManager &FAM) {
  LoopInfo &LI = FAM.getResult<LoopAnalysis>(F);
  if (LI.empty())
    return;

  DominatorTree &DT = FAM.getResult<DominatorTreeAnalysis>(F);
  ScalarEvolution &SE = FAM.getResult<ScalarEvolutionAnalysis>(F);

  bool modified = false;

  // LI.getLoopsInPreorder() is used instead if LI, which the loop alters
  for (Loop *L : LI.getLoopsInPreorder()) {
    modified |= formLCSSARecursively(*L, DT, &LI, &SE);
  }

  if (modified) {
    FAM.invalidate(F, getLoopPassPreservedAnalyses());
  }
}

// Rotates loops
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
  // LI.getLoopsInPreorder() is used instead if LI, which the loop alters
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

// Canonicalizes loops
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

  // LI.getLoopsInPreorder() is used instead if LI, which the loop alters
  for (Loop *L : LI.getLoopsInPreorder()) {
    modified |= simplifyLoop(L, &DT, &LI, &SE, &AC, &MSSAU, true);
  }

  if (modified) {
    FAM.invalidate(F, getLoopPassPreservedAnalyses());
  }
}

// Replaces all freeze instructions by their operands
int remove_freeze(Function &F) {
  int removal_count = 0;
  // Iterate over all basic blocks in the function
  for (auto &BB : F) {
    // Make a list of freeze instructions because we can't modify the list of
    // instructions while iterating over it
    vector<Instruction *> freezeInstructions;
    // Iterate over all instructions in the basic block
    for (auto &I : BB) {
      // If this instruction is a freeze instruction, add it to the list
      if (I.getOpcode() == Instruction::Freeze) {
        freezeInstructions.push_back(&I);
      }
    }
    // Now, replace all freeze instructions with their operands
    for (auto *FI : freezeInstructions) {
      FI->replaceAllUsesWith(FI->getOperand(0));
      FI->eraseFromParent();
      ++removal_count;
    }
  }
  return removal_count;
}

PreservedAnalyses LoopUnrolling::run(Function &F,
                                     FunctionAnalysisManager &FAM) {

  // Do not modify oracle code!
  if (F.getName() == "oracle") {
    return PreservedAnalyses::all();
  }

  // Loops must be in rotated canonical form for unrolling to happen
  if (DO_LCSSA_CONVERSION)
    lcssa(F, FAM);
  rotate_loop(F, FAM);
  simplify_loop(F, FAM);

  LoopInfo &LI = FAM.getResult<LoopAnalysis>(F);
  ScalarEvolution &SE = FAM.getResult<ScalarEvolutionAnalysis>(F);
  DominatorTree &DT = FAM.getResult<DominatorTreeAnalysis>(F);
  DomTreeUpdater DTU(DT, DomTreeUpdater::UpdateStrategy::Eager);
  AssumptionCache &AC = FAM.getResult<AssumptionAnalysis>(F);
  TargetTransformInfo &TTI = FAM.getResult<TargetIRAnalysis>(F);

  // LI.getLoopsInPreorder() is used instead if LI, which the loop alters
  for (Loop *L : LI.getLoopsInPreorder()) {
    if (L->isInnermost() && L->isLCSSAForm(DT)) {
      // Skip loop if loop contains too many instructions
      size_t loop_size = 0;
      for (auto loop_block: L->blocks()) {
        loop_size += loop_block->size();
      }
      if (loop_size > MAX_LOOP_SIZE) {
        continue;
      }
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
        vector<BasicBlock *> loop_blocks = L->getBlocksVector();
        is_merged = false;
        for (BasicBlock *BB : loop_blocks) {
          is_merged |= MergeBlockIntoPredecessor(BB, &DTU, &LI);
        }
      } while (is_merged);
    }
  }

  // Runtime unrolling may introduce freeze instructions.
  // Our backend doesn't like freeze instructions.
  // We have no choice but to... pull them out.
  int removed_freezes = remove_freeze(F);

  if (removed_freezes) {
    outs() << "loop-unrolling: removed " << removed_freezes << " freezes\n";
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
