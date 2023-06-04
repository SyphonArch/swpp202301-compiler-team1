#ifndef SC_OPT_ORACLE_PASS
#define SC_OPT_ORACLE_PASS

#include "llvm/IR/Instructions.h"
#include "llvm/IR/PassManager.h"

using namespace std;
using namespace llvm;

namespace sc::opt::oracle_pass {
class OraclePass : public PassInfoMixin<OraclePass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &MAM);
};
} // namespace sc::opt::oracle_pass
#endif // SC_OPT_ORACLE_PASS