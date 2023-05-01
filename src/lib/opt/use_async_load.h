// use_async_load.h

#ifndef SC_OPT_USE_ASYNC_LOAD_H
#define SC_OPT_USE_ASYNC_LOAD_H

#include "llvm/IR/PassManager.h"

using namespace std;
using namespace llvm;

namespace sc::opt::use_async_load {
    class UseAsyncLoad
            : public PassInfoMixin<UseAsyncLoad> {
    public:
        PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM);
    };
} // namespace
#endif // SC_OPT_USE_ASYNC_LOAD_H