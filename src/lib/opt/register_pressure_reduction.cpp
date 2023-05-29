#include "register_pressure_reduction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"

using namespace llvm;
using namespace std;

namespace sc::opt::register_pressure_reduction {
PreservedAnalyses RegisterPressureReduction::run(Function &F,
                                                 FunctionAnalysisManager &FAM) {
  return PreservedAnalyses::none();
}

extern "C" ::llvm::PassPluginLibraryInfo llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "RegisterPressureReduction",
          LLVM_VERSION_STRING, [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "register-pressure-reduction") {
                    FPM.addPass(RegisterPressureReduction());
                    return true;
                  }
                  return false;
                });
          }};
}
} // namespace sc::opt::register_pressure_reduction