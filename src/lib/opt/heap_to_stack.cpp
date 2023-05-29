#include "heap_to_stack.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"

using namespace llvm;
using namespace std;

namespace sc::opt::heap_to_stack {
PreservedAnalyses HeapToStack::run(Function &F, FunctionAnalysisManager &FAM) {
  LLVMContext &context = F.getContext();
  IRBuilder<> builder(context);
  BasicBlock *entry_block = &F.getEntryBlock();
  builder.SetInsertPoint(entry_block, entry_block->getFirstInsertionPt());

  set<Instruction *> toErase;
  for (auto &BB : F) {
    for (auto &Inst : BB) {
      if (auto *Call = dyn_cast<CallInst>(&Inst)) {
        bool proceed = false;

        Function *Callee = Call->getCalledFunction();
        if (Callee && Callee->getName() == "malloc") {
          Value *AllocSize = Call->getArgOperand(0);
          Type *AllocType = Call->getType()->getPointerElementType();

          if (dyn_cast<ConstantInt>(AllocSize)) {
            for (auto User : Call->users()) {
              // Check if the user is a CallInst and corresponds to 'free'
              if (auto *CI = dyn_cast<CallInst>(User)) {
                Function *Callee2 = CI->getCalledFunction();
                if (Callee2 && Callee2->getName() == "free") {
                  if (CI->getParent() == &BB) {
                    proceed = true;
                    break;
                  }
                }
              }
            }
            if (proceed) {
              outs() << "Found malloc: " << Inst << '\n';
              outs().flush();
              Instruction *Alloca =
                  builder.CreateAlloca(AllocType, AllocSize, "new_alloca");
              outs() << "Replacing with: ";
              Alloca->print(outs());
              outs() << '\n';
              outs().flush();
              for (auto User : Call->users()) {
                // Check if the user is a CallInst and corresponds to 'free'
                if (auto *CI = dyn_cast<CallInst>(User)) {
                  Function *Callee2 = CI->getCalledFunction();
                  if (Callee2 && Callee2->getName() == "free") {
                    // Remove the 'free' call instruction
                    outs() << "Removing free: ";
                    CI->print(outs());
                    outs() << '\n';
                    outs().flush();
                    toErase.insert(CI);
                  }
                }
              }
              Call->replaceAllUsesWith(Alloca);
              toErase.insert(Call);
            }
          }
        }
      }
    }
  }
  for (auto I : toErase) {
    I->eraseFromParent();
  }
  return PreservedAnalyses::none();
}

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
}
} // namespace sc::opt::heap_to_stack