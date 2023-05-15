#ifndef SC_BACKEND_GEP_ELIMINATE_H
#define SC_BACKEND_GEP_ELIMINATE_H

#include "llvm/IR/PassManager.h"

using namespace std;
using namespace llvm;

namespace sc::opt::gep_elim {
class GEPEliminatePass : public llvm::PassInfoMixin<GEPEliminatePass> {
public:
  PreservedAnalyses run(llvm::Module &, llvm::ModuleAnalysisManager &);
};
} // namespace sc::opt::gep_elim
#endif // SC_BACKEND_GEP_ELIMINATE_H
