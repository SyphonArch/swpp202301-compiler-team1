#ifndef LCSSA_PASS_H
#define LCSSA_PASS_H

#include "llvm/IR/PassManager.h"

using namespace std;
using namespace llvm;

namespace sc::opt::lcssa_pass {
class LCSSApass : public PassInfoMixin<LCSSApass> {
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM);
};
} // namespace sc::opt::lcssa_pass
#endif