#ifndef HEAP_TO_STACK_H
#define HEAP_TO_STACK_H

#include "llvm/IR/PassManager.h"

using namespace std;
using namespace llvm;

namespace sc::opt::heap_to_stack {
class HeapToStack : public PassInfoMixin<HeapToStack> {
public:
  static PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM);
};
} // namespace sc::opt::heap_to_stack
#endif