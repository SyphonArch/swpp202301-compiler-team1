#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Dominators.h"
#include "gvn_pass.h"

using namespace llvm;
using namespace std;

namespace sc::opt::gvn_pass {
PreservedAnalyses
GVNpass::run(Function &F, FunctionAnalysisManager &FAM) {

    //add existing GVN pass
    GVNPass().run(F, FAM);  

    return PreservedAnalyses::none(); // or all();
};

extern "C" ::llvm::PassPluginLibraryInfo llvmGetPassPluginInfo() {
    return {LLVM_PLUGIN_API_VERSION, "GVNpass", LLVM_VERSION_STRING,
            [](PassBuilder &PB) {
                PB.registerPipelineParsingCallback(
                        [](StringRef Name, FunctionPassManager &FPM,
                           ArrayRef<PassBuilder::PipelineElement>) {
                            if (Name == "gvn-pass") {
                                FPM.addPass(GVNpass());
                                return true;
                            }
                            return false;
                        });
            }};
};
}