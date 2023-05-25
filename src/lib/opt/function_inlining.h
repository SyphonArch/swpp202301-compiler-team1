#ifndef SC_OPT_FUNCTION_INLINING
#define SC_OPT_FUNCTION_INLINING

#include "llvm/IR/Instructions.h"
#include "llvm/IR/PassManager.h"

using namespace std;
using namespace llvm;

namespace sc::opt::function_inlining {
class FunctionInlining : public PassInfoMixin<FunctionInlining> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &MAM);
};
} // namespace sc::opt::function_inlining
#endif // SC_OPT_FUNCTION_INLINING