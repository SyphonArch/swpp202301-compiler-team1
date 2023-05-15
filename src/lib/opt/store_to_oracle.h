// store_to_oracle.h

#ifndef SC_OPT_STORE_TO_ORACLE_H
#define SC_OPT_STORE_TO_ORACLE_H

#include "llvm/IR/PassManager.h"

using namespace std;
using namespace llvm;

namespace sc::opt::store_to_oracle {
    class StoreToOracle
            : public PassInfoMixin<StoreToOracle> {
    public:
        PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM);
    };
} // namespace sc::opt::store_to_oracle
#endif // SC_OPT_STORE_TO_ORACLE_H