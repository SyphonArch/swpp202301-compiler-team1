#include "opt.h"

#include "../static_error.h"
#include "llvm/Analysis/CGSCCPassManager.h"

#include "print_ir.h"

#include "./opt/add_to_sum.h"
#include "./opt/arithmetic_pass.h"
#include "./opt/bias_to_false_branch.h"
#include "./opt/function_inlining.h"
#include "./opt/gep_elim.h"
#include "./opt/gvn_pass.h"
#include "./opt/lcssa_pass.h"
#include "./opt/loop_unrolling.h"
#include "./opt/heap_to_stack.h"
#include "./opt/oracle_pass.h"
#include "./opt/simplify_cfg.h"
#include "./opt/use_async_load.h"
using namespace std::string_literals;

namespace sc::opt {
OptInternalError::OptInternalError(const std::exception &__e) noexcept {
  message = "exception thrown from opt\n"s + __e.what();
}

Result<std::unique_ptr<llvm::Module>, OptInternalError>
optimizeIR(std::unique_ptr<llvm::Module> &&__M,
           llvm::ModuleAnalysisManager &__MAM) noexcept {
  using RetType = Result<std::unique_ptr<llvm::Module>, OptInternalError>;

  try {
    llvm::FunctionPassManager FPM;
    llvm::CGSCCPassManager CGPM;
    llvm::ModulePassManager MPM;

    // Add loop-level opt passes below

    // Add function-level opt passes below

    FPM.addPass(gvn_pass::GVNpass());
    FPM.addPass(simplify_cfg::SimplifyCFG());
    FPM.addPass(lcssa_pass::LCSSApass());
    FPM.addPass(loop_unrolling::LoopUnrolling());
    FPM.addPass(simplify_cfg::SimplifyCFG());
    FPM.addPass(bias_to_false_branch::BiasToFalseBranch());
    FPM.addPass(add_to_sum::AddToSum());
    FPM.addPass(gep_elim::GEPEliminatePass());
    FPM.addPass(arithmetic_pass::ArithmeticPass());
    FPM.addPass(use_async_load::UseAsyncLoad());

    CGPM.addPass(llvm::createCGSCCToFunctionPassAdaptor(std::move(FPM)));
    // Add CGSCC-level opt passes below

    MPM.addPass(llvm::createModuleToPostOrderCGSCCPassAdaptor(std::move(CGPM)));
    // Add module-level opt passes below
    MPM.addPass(heap_to_stack::HeapToStack());
    MPM.addPass(oracle_pass::OraclePass());
    MPM.addPass(function_inlining::FunctionInlining());
    MPM.addPass(createModuleToFunctionPassAdaptor(simplify_cfg::SimplifyCFG()));

    MPM.run(*__M, __MAM);
    sc::print_ir::printIRIfVerbose(*__M, "After optimization");
  } catch (const std::exception &e) {
    return RetType::Err(OptInternalError(e));
  }

  return RetType::Ok(std::move(__M));
}
} // namespace sc::opt
