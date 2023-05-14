#include "register_pressure_analysis.h"

#include "register_pressure.cpp"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/Debug.h"

using namespace llvm;

#define DEBUG_TYPE "register-pressure-analysis"

namespace sc::opt::register_pressure_analysis {
uint64_t RegisterPressureAnalysis::run(Function &F,
                                       FunctionAnalysisManager &FAM) {
  return calculateRegisterPressure(F);
};

PreservedAnalyses
RegisterPressurePrinterPass::run(Function &F, FunctionAnalysisManager &FAM) {
  uint64_t maxRegisterPressure = calculateRegisterPressure(F);
  outs() << "Approximate Register Pressure of @" << F.getName() << ": "
         << maxRegisterPressure << '\n';

  return PreservedAnalyses::all();
};

extern "C" ::llvm::PassPluginLibraryInfo llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "RegisterPressurePrinterPass",
          LLVM_VERSION_STRING, [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "register-pressure-printer-pass") {
                    FPM.addPass(RegisterPressurePrinterPass());
                    return true;
                  }
                  return false;
                });
          }};
};
} // namespace sc::opt::register_pressure_analysis