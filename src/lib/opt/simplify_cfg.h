#ifndef SIMPLIFY_CFG_H
#define SIMPLIFY_CFG_H

#include "llvm/IR/PassManager.h"

using namespace std;
using namespace llvm;

namespace sc::opt::simplify_cfg {
class SimplifyCFG : public PassInfoMixin<SimplifyCFG> {
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM);
};
} // namespace sc::opt::simplify_cfg
#endif