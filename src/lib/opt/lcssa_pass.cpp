#include "lcssa_pass.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Transforms/Utils/LCSSA.h"

using namespace llvm;
using namespace std;

namespace sc::opt::lcssa_pass {
PreservedAnalyses LCSSApass::run(Function &F, FunctionAnalysisManager &FAM) {
  
  PreservedAnalyses PA = LCSSAPass().run(F, FAM);
  return PA;
}

extern "C" ::llvm::PassPluginLibraryInfo llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "LCSSApass", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "lcssa-pass") {
                    FPM.addPass(LCSSApass());
                    return true;
                  }
                  return false;
                });
          }};
};

} // namespace sc::opt::lcssa_pass
