#ifndef DCE_PASS_H
#define DCE_PASS_H

#include "llvm/IR/PassManager.h"

using namespace std;
using namespace llvm;

namespace sc::opt::dce_pass {
class DCEpass : public PassInfoMixin<DCEpass> {
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM);
};
} // namespace sc::opt::dce_pass
#endif