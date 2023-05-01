#include "bias_to_false_branch.h"

#include "llvm/Analysis/BranchProbabilityInfo.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/Debug.h"

using namespace llvm;
using namespace std;

#define DEBUG_TYPE "bias-to-false-branch"

namespace sc::opt::bias_to_false_branch {
PreservedAnalyses BiasToFalseBranch::run(Function &F,
                                         FunctionAnalysisManager &FAM) {

  BranchProbabilityInfo &BPI = FAM.getResult<BranchProbabilityAnalysis>(F);

  // loop basic blocks and check for all conditional edges
  for (BasicBlock &BB : F) {
    BranchInst *BI = dyn_cast<BranchInst>(BB.getTerminator());
    if (!BI || !BI->isConditional()) {
      continue;
    }

    Value *Cond = BI->getCondition();
    ICmpInst *CI = dyn_cast<ICmpInst>(Cond);
    if (!CI) {
      continue;
    }

    // check if Cond is not used anywhere and only exists for branch
    // conditioning
    bool isCondOnlyForBranching = true;
    for (auto *U : Cond->users()) {
      if (U != BI) {
        isCondOnlyForBranching = false;
        break;
      }
    }
    if (!isCondOnlyForBranching) {
      continue;
    }

    // if the false branch is less probable than the true branch, swap the
    // condition
    BasicBlock *TrueBB = BI->getSuccessor(0);
    BasicBlock *FalseBB = BI->getSuccessor(1);
    BranchProbability false_prob = BPI.getEdgeProbability(&BB, FalseBB);

    double false_prob_float =
        ((double)false_prob.getNumerator()) / false_prob.getDenominator();

    LLVM_DEBUG(dbgs() << "Edge " << BB.getName() << " -> " << FalseBB->getName()
                      << " has probability " << false_prob_float << "\n");

    if (false_prob_float < 0.5) {
      // swap to make false branch more probable
      CI->setPredicate(CI->getInversePredicate());
      BI->swapSuccessors();
    }
    // else, false branch is more probable; do nothing
  }

  return PreservedAnalyses::none();
};

extern "C" ::llvm::PassPluginLibraryInfo llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "BiasToFalseBranch", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "bias-to-false-branch") {
                    FPM.addPass(BiasToFalseBranch());
                    return true;
                  }
                  return false;
                });
          }};
};
} // namespace sc::opt::bias_to_false_branch