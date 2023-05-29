#include "heap_to_stack.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"

using namespace llvm;
using namespace std;

namespace sc::opt::heap_to_stack {
PreservedAnalyses HeapToStack::run(Function &F, FunctionAnalysisManager &FAM) {
  return PreservedAnalyses::all();
};

extern "C" ::llvm::PassPluginLibraryInfo llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "HeapToStack", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "heap-to-stack") {
                    FPM.addPass(HeapToStack());
                    return true;
                  }
                  return false;
                });
          }};
};
} // namespace sc::opt::gvn_pass