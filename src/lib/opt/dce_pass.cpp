#include "dce_pass.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Transforms/Scalar/DCE.h"

using namespace llvm;
using namespace std;

namespace sc::opt::dce_pass {
PreservedAnalyses DCEpass::run(Function &F, FunctionAnalysisManager &FAM) {
  
  PreservedAnalyses PA = DCEPass().run(F, FAM);
  return PA;
}

extern "C" ::llvm::PassPluginLibraryInfo llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "DCEpass", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "dce-pass") {
                    FPM.addPass(DCEpass());
                    return true;
                  }
                  return false;
                });
          }};
};

} // namespace sc::opt::dce_pass
