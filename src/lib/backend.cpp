#include "backend.h"

#include "backend/assembly.h"
#include "backend/emitter.h"
#include "backend/check_oracle.h"
#include "backend/const_expr_eliminate.h"
#include "backend/gep_eliminate.h"
#include "backend/gv_eliminate.h"
#include "backend/alloca_eliminate.h"
#include "backend/gep_const_combine.h"
#include "backend/phi_preprocess.h"
#include "backend/register_allocate.h"
#include "print_ir.h"

using namespace std::string_literals;

namespace sc::backend {
BackendInternalError::BackendInternalError(const std::exception &__e) noexcept {
  message = "exception thrown from backend\n"s + __e.what();
}

Result<std::string, BackendInternalError>
emitAssembly(std::unique_ptr<llvm::Module> &&__M,
             llvm::ModuleAnalysisManager &__MAM) noexcept {
  using RetType = Result<std::string, BackendInternalError>;

  try {
    llvm::ModulePassManager MPM;
    MPM.addPass(check_oracle::CheckOraclePass());
    MPM.addPass(ce_elim::ConstExprEliminatePass());
    MPM.addPass(gep_elim::GEPEliminatePass());
    MPM.addPass(gv_elim::GVEliminatePass());
    MPM.addPass(alloca_elim::AllocaEliminatePass());
    MPM.addPass(gc_comb::GEPConstCombinePass());
    MPM.run(*__M, __MAM);
  } catch (const std::exception &e) {
    return RetType::Err(BackendInternalError(e));
  }
  sc::print_ir::printIRIfVerbose(*__M, "After backend passes"s);

  symbol::SymbolMap SM;
  try {
    llvm::ModulePassManager MPM;
    MPM.addPass(phi_prep::PHIPreprocessPass());
    MPM.addPass(reg_alloc::RegisterAllocatePass(SM));
    MPM.run(*__M, __MAM);
  } catch (const std::exception &e) {
    return RetType::Err(BackendInternalError(e));
  }
  sc::print_ir::printIRIfVerbose(*__M, "After register allocation"s);

  std::string assembly;
  try {
    auto AE = emitter::AssemblyEmitter(SM);
    AE.visit(*__M);
    assembly = AE.getAssembly();
  } catch (const std::exception &e) {
    return RetType::Err(BackendInternalError(e));
  }

  return RetType::Ok(assembly);
}
} // namespace sc::backend
