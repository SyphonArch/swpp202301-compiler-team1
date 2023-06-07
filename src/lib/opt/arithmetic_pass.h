#ifndef ARITHMETIC_PASS_H
#define ARITHMETIC_PASS_H

#include "llvm/IR/PassManager.h"

using namespace std;
using namespace llvm;

namespace sc::opt::arithmetic_pass {
    class ArithmeticPass
            : public PassInfoMixin<ArithmeticPass> {
    public:
        PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM);
    };
} // namespace
#endif 