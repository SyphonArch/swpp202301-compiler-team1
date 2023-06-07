#include "simplify_cfg.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Transforms/Scalar/SimplifyCFG.h"
#include "llvm/Transforms/Utils/SimplifyCFGOptions.h"

using namespace llvm;
using namespace std;

namespace sc::opt::simplify_cfg {
PreservedAnalyses SimplifyCFG::run(Function &F, FunctionAnalysisManager &FAM) {
  auto Opts = SimplifyCFGOptions();
  Opts.needCanonicalLoops(false);
  Opts.hoistCommonInsts(true);
  Opts.sinkCommonInsts(true);

  PreservedAnalyses PA = SimplifyCFGPass(Opts).run(F, FAM);

  return PA;
};

extern "C" ::llvm::PassPluginLibraryInfo llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "SimplifyCFG", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "simplify-cfg") {
                    FPM.addPass(SimplifyCFG());
                    return true;
                  }
                  return false;
                });
          }};
};
} // namespace sc::opt::simplify_cfg