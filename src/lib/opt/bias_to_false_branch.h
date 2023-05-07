#ifndef SC_OPT_BIAS_TO_FALSE_BRANCH_H
#define SC_OPT_BIAS_TO_FALSE_BRANCH_H

#include "llvm/Analysis/BranchProbabilityInfo.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/PassManager.h"

using namespace std;
using namespace llvm;

namespace sc::opt::bias_to_false_branch {
class BiasToFalseBranch : public PassInfoMixin<BiasToFalseBranch> {
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM);

private:
  double getFalseBranchProbability(BranchInst *BI, BranchProbabilityInfo &BPI);
};
} // namespace sc::opt::bias_to_false_branch
#endif // SC_OPT_BIAS_TO_FALSE_BRANCH_H