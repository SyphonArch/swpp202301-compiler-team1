#include "bias_to_false_branch.h"

#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/Debug.h"

using namespace llvm;

#define DEBUG_TYPE "bias-to-false-branch"

namespace sc::opt::bias_to_false_branch {

// Helper function to get the probability of a false branch for a given
// BranchInst
double
BiasToFalseBranch::getFalseBranchProbability(BranchInst *BI,
                                             BranchProbabilityInfo &BPI) {
  BasicBlock *parentBB = BI->getParent();
  BasicBlock *falseBB = BI->getSuccessor(1);
  BranchProbability prob = BPI.getEdgeProbability(parentBB, falseBB);
  return static_cast<double>(prob.getNumerator()) / prob.getDenominator();
}

PreservedAnalyses BiasToFalseBranch::run(Function &F,
                                         FunctionAnalysisManager &FAM) {

  BranchProbabilityInfo &BPI = FAM.getResult<BranchProbabilityAnalysis>(F);

  // Gather all unique condition instructions for branch instructions
  std::set<Value *> conditions;

  for (Instruction &I : instructions(F)) {
    if (BranchInst *BI = dyn_cast<BranchInst>(&I)) {
      if (BI->isConditional()) {
        conditions.insert(BI->getCondition());
      }
    }
  }

  // Iterate through all ICmp instructions
  for (Value *Cond : conditions) {
    std::vector<BranchInst *> branchInsts;

    // Gather all conditional branch instructions using the ICmp instruction
    for (User *U : Cond->users()) {
      if (BranchInst *BI = dyn_cast<BranchInst>(U)) {
        if (BI->isConditional() && BI->getCondition() == Cond) {
          LLVM_DEBUG(dbgs() << "User: " << *U << "\n");
          branchInsts.push_back(BI);
        }
      }
    }

    for (BranchInst *BI : branchInsts) {
      double prob = getFalseBranchProbability(BI, BPI);
      LLVM_DEBUG(dbgs() << "Branch: " << *BI << ", probability: " << prob
                        << "\n");
    }
    // Check if the inverted condition needs to be created
    bool allBranches = branchInsts.size() ==
                       std::distance(Cond->user_begin(), Cond->user_end());

    bool allProbLessThan07 = std::all_of(
        branchInsts.begin(), branchInsts.end(), [&](BranchInst *BI) {
          double prob = getFalseBranchProbability(BI, BPI);
          return prob < 0.7;
        });

    bool allprobLessOrEqualTo05 = std::all_of(
        branchInsts.begin(), branchInsts.end(), [&](BranchInst *BI) {
          double prob = getFalseBranchProbability(BI, BPI);
          return prob <= 0.5;
        });

    bool anyProbLessThan03 = std::any_of(
        branchInsts.begin(), branchInsts.end(), [&](BranchInst *BI) {
          double prob = getFalseBranchProbability(BI, BPI);
          return prob < 0.3;
        });

    ICmpInst *CI = dyn_cast<ICmpInst>(Cond);

    // When we can throw away the original condition, just invert the condition
    if (CI && allBranches && allProbLessThan07 &&
        (allprobLessOrEqualTo05 || anyProbLessThan03)) {
      CI->setPredicate(CI->getInversePredicate());
      for (BranchInst *BI : branchInsts) {
        BI->swapSuccessors();
      }
    }

    else {
      // Keep the original condition and optionally create the inverted
      // condition
      for (BranchInst *BI : branchInsts) {
        double prob_float = getFalseBranchProbability(BI, BPI);

        if (prob_float < 0.3) {
          IRBuilder<> builder(BI);
          ConstantInt *TrueValue = ConstantInt::get(builder.getInt1Ty(), 0);
          ConstantInt *FalseValue = ConstantInt::get(builder.getInt1Ty(), 1);
          Value *invertedCond = builder.CreateSelect(
              Cond, TrueValue, FalseValue, "not_" + Cond->getName());

          BI->setCondition(invertedCond);
          BI->swapSuccessors();
        }
      }
    }
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