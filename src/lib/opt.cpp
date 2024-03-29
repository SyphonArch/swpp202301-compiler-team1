#include "opt.h"

#include "../static_error.h"
#include "llvm/Analysis/CGSCCPassManager.h"
#include "llvm/Transforms/Scalar/LoopPassManager.h"

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
#include "./opt/loop_extractor.h"
#include "llvm/Transforms/Utils/BreakCriticalEdges.h"
#include "llvm/Transforms/Utils/LoopSimplify.h"
#include "llvm/Transforms/Scalar/TailRecursionElimination.h"

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
    llvm::LoopPassManager LPM;
    llvm::FunctionPassManager FPM;
    llvm::CGSCCPassManager CGPM;
    llvm::ModulePassManager MPM;

    // Add loop-level opt passes below

    FPM.addPass(llvm::createFunctionToLoopPassAdaptor(std::move(LPM)));
    // Add function-level opt passes below
    FPM.addPass(gvn_pass::GVNpass());

    CGPM.addPass(llvm::createCGSCCToFunctionPassAdaptor(std::move(FPM)));
    // Add CGSCC-level opt passes below

    MPM.addPass(llvm::createModuleToPostOrderCGSCCPassAdaptor(std::move(CGPM)));
    // Add module-level opt passes below
    MPM.addPass(llvm::createModuleToFunctionPassAdaptor(TailCallElimPass()));

    MPM.addPass(function_inlining::FunctionInlining());
    MPM.addPass(llvm::createModuleToFunctionPassAdaptor(gvn_pass::GVNpass()));
    MPM.addPass(
        llvm::createModuleToFunctionPassAdaptor(simplify_cfg::SimplifyCFG()));

    MPM.addPass(llvm::createModuleToFunctionPassAdaptor(gvn_pass::GVNpass()));
    MPM.addPass(
        llvm::createModuleToFunctionPassAdaptor(simplify_cfg::SimplifyCFG()));

    MPM.addPass(llvm::createModuleToFunctionPassAdaptor(
        bias_to_false_branch::BiasToFalseBranch()));
    MPM.addPass(llvm::createModuleToFunctionPassAdaptor(gvn_pass::GVNpass()));

    MPM.addPass(
        llvm::createModuleToFunctionPassAdaptor(gep_elim::GEPEliminatePass()));

    MPM.addPass(llvm::createModuleToFunctionPassAdaptor(
        arithmetic_pass::ArithmeticPass()));

    MPM.addPass(heap_to_stack::HeapToStack());

    // function inlining is also disabled because this hurts performance with
    // oracle MPM.addPass(function_inlining::FunctionInlining());
    // MPM.addPass(llvm::createModuleToFunctionPassAdaptor(simplify_cfg::SimplifyCFG()));
    // MPM.addPass(llvm::createModuleToFunctionPassAdaptor(gvn_pass::GVNpass()));

    MPM.addPass(
        llvm::createModuleToFunctionPassAdaptor(BreakCriticalEdgesPass()));
    MPM.addPass(llvm::createModuleToFunctionPassAdaptor(LoopSimplifyPass()));
    MPM.addPass(loop_extractor::LoopExtractor2Pass());

    MPM.addPass(oracle_pass::OraclePass());

    MPM.addPass(llvm::createModuleToFunctionPassAdaptor(
        use_async_load::UseAsyncLoad()));

    MPM.addPass(llvm::createModuleToFunctionPassAdaptor(
        loop_unrolling::LoopUnrolling()));
    MPM.addPass(
        llvm::createModuleToFunctionPassAdaptor(add_to_sum::AddToSum()));

    MPM.addPass(llvm::createModuleToFunctionPassAdaptor(
        bias_to_false_branch::BiasToFalseBranch()));

    MPM.run(*__M, __MAM);
    sc::print_ir::printIRIfVerbose(*__M, "After optimization");
  } catch (const std::exception &e) {
    return RetType::Err(OptInternalError(e));
  }

  return RetType::Ok(std::move(__M));
}
} // namespace sc::opt
