#ifndef GVN_PASS_H
#define GVN_PASS_H

#include "llvm/IR/PassManager.h"

using namespace std;
using namespace llvm;

namespace sc::opt::gvn_pass {
    class GVNpass
            : public PassInfoMixin<GVNpass> {
    public:
        PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM);
    };
} // namespace
#endif