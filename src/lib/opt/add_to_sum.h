#ifndef SC_OPT_ADD_TO_SUM_H
#define SC_OPT_ADD_TO_SUM_H

#include "llvm/IR/PassManager.h"

using namespace std;
using namespace llvm;

namespace sc::opt::add_to_sum {
class AddToSum : public PassInfoMixin<AddToSum> {
public:
  static PreservedAnalyses run(Function &val, FunctionAnalysisManager &FAM);
};
} // namespace sc::opt::add_to_sum

#endif // SC_OPT_ADD_TO_SUM_H
