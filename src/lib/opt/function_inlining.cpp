#include "function_inlining.h"

#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/Debug.h"

using namespace llvm;

#define DEBUG_TYPE "function-inlining"

namespace sc::opt::function_inlining {

static void cloneFunctionInto(Function *Src, Function *Dest,
                              ValueToValueMapTy &VMap,
                              SmallVectorImpl<ReturnInst *> &Returns) {

  // Create a mapping of the source function's basic blocks to the destination
  // function's basic blocks
  for (auto &BB : *Src) {
    BasicBlock *ClonedBB =
        BasicBlock::Create(Dest->getContext(), BB.getName(), Dest);
    VMap[&BB] = ClonedBB;
  }

  // Clone the instructions in the source function's basic blocks
  for (auto &BB : *Src) {
    BasicBlock *ClonedBB = cast<BasicBlock>(VMap[&BB]);

    for (auto &I : BB) {
      Instruction *ClonedInst = I.clone();
      ClonedBB->getInstList().push_back(ClonedInst);

      // Update the value map with the cloned instruction
      VMap[&I] = ClonedInst;
    }
  }

  // Remap operands for the cloned instructions
  for (auto &BB : *Src) {
    BasicBlock *ClonedBB = cast<BasicBlock>(VMap[&BB]);

    for (auto &I : *ClonedBB) {
      RemapInstruction(&I, VMap);
    }
  }

  // If the last instruction is a return, add it to the Returns vector
  for (auto &BB : *Src) {
    BasicBlock *ClonedBB = cast<BasicBlock>(VMap[&BB]);

    if (ReturnInst *RI = dyn_cast<ReturnInst>(ClonedBB->getTerminator())) {
      Returns.push_back(RI);
    }
  }
}

static void splice(BasicBlock &B, BasicBlock::iterator InsertPt, BasicBlock &BB,
                   BasicBlock::iterator FromBeginIt,
                   BasicBlock::iterator FromEndIt) {
  B.getInstList().splice(InsertPt, BB.getInstList(), FromBeginIt, FromEndIt);
}

/// Return the result of AI->isStaticAlloca() if AI were moved to the entry
/// block.
static bool allocaWouldBeStaticInEntry(const AllocaInst *AI) {
  return isa<Constant>(AI->getArraySize()) && !AI->isUsedWithInAlloca();
}

static void moveStaticAllocasUpIfExists(Function *Caller,
                                        Function::iterator &FirstNewBlock) {
  // If there are any alloca instructions in the block that used to be the entry
  // block for the callee, move them to the entry block of the caller.

  // Calculate which instruction they should be inserted before. We insert
  // the instructions at the end of the current alloca list.
  BasicBlock::iterator InsertPoint = Caller->begin()->begin();
  for (BasicBlock::iterator I = FirstNewBlock->begin(),
                            E = FirstNewBlock->end();
       I != E;) {
    AllocaInst *AI = dyn_cast<AllocaInst>(I++);
    if (!AI)
      continue;

    if (!allocaWouldBeStaticInEntry(AI))
      continue;

    // Scan for the block of allocas that we can move over, and move them
    // all at once.
    while (isa<AllocaInst>(I) && !cast<AllocaInst>(I)->use_empty() &&
           allocaWouldBeStaticInEntry(cast<AllocaInst>(I))) {
      ++I;
    }

    // Transfer all of the allocas over in a block.
    splice(Caller->getEntryBlock(), InsertPoint, *FirstNewBlock,
           AI->getIterator(), I);
  }
}

// Merge the inlined function's basic blocks into the caller function
static void mergeToCallSite(CallInst *CI, Function::iterator &FirstNewBlock,
                            SmallVectorImpl<ReturnInst *> &Returns) {
  LLVM_DEBUG(dbgs() << "Returns.size(): " << Returns.size() << "\n");
  assert(Returns.size() == 1 && "Currently only a single return function is "
                                "supported!");

  Function *Caller = CI->getFunction();
  Function *Callee = CI->getCalledFunction();

  BasicBlock *OrigBB = CI->getParent();
  if (std::distance(FirstNewBlock, Caller->end()) == 1) {
    // Move the inlined function's basic blocks to the call site
    OrigBB->getInstList().splice(CI->getIterator(),
                                 FirstNewBlock->getInstList());

    // Remove the now-empty basic block
    FirstNewBlock->eraseFromParent();

    // Replace the call instruction's uses with the inlined function's return
    // value
    if (!CI->use_empty()) {
      ReturnInst *R = Returns[0];
      CI->replaceAllUsesWith(R->getReturnValue());
    }

    // Remove the call instruction and the return instruction of the inlined
    // function
    CI->eraseFromParent();
    Returns[0]->eraseFromParent();

    return;
  }

  // Now, deal with the case of multiple basic blocks in the Callee.
  // First, split the basic block into [basic block before call] and [after
  // call]. Second, set the branch in the [basic block before call] to the
  // cloned function Third, merge the return block with the [basic block after
  // call]. Fourth, set the branch `to` the return block to the [basic block
  // after call]. Fifth, replace the call instruction and remove the return
  // instruction.

  // Split the basic block to before call and after call.
  BasicBlock *AfterCallBB =
      OrigBB->splitBasicBlock(CI->getIterator(), Callee->getName() + ".exit");

  // Set the branch instruction to the cloned function
  Instruction *Br = OrigBB->getTerminator();
  Br->setOperand(0, &*FirstNewBlock);

  LLVM_DEBUG(dbgs() << "Caller after setting initial branch:\n"
                    << *CI->getFunction() << "\n");

  // Merge the return block with the AfterCallBB.
  BasicBlock *ReturnBB = Returns[0]->getParent();
  AfterCallBB->getInstList().splice(AfterCallBB->begin(),
                                    ReturnBB->getInstList());

  // Set branch instruction to return basic block to AfterCallBB
  ReturnBB->replaceAllUsesWith(AfterCallBB);

  // Replace the uses of call instruction to the return instructions
  if (!CI->use_empty()) {
    ReturnInst *R = Returns[0];
    CI->replaceAllUsesWith(R->getReturnValue());
  }

  // Remove the call instruction and the return instruction of the inlined
  Returns[0]->eraseFromParent();
  ReturnBB->eraseFromParent();
  CI->eraseFromParent();
}

// Inline the callee function into the caller function
static void inlineFunction(CallInst *CI, Function &Callee) {
  Function *Caller = CI->getFunction();
  Function::iterator LastBlock = --Caller->end();
  ValueToValueMapTy VMap;

  // Map callee function arguments to the corresponding call instruction
  // operands
  for (unsigned i = 0, e = CI->arg_size(); i != e; ++i) {
    VMap[Callee.arg_begin() + i] = CI->getArgOperand(i);
  }

  SmallVector<ReturnInst *, 8> Returns;
  cloneFunctionInto(&Callee, Caller, VMap, Returns);

  // Get an iterator to the first new basic block (inlined function)
  Function::iterator FirstNewBlock = LastBlock;
  ++FirstNewBlock;

  // Move static allocas up. Assume all allocas are static, because
  // this machine does not handle other than static allocas in the backend.
  moveStaticAllocasUpIfExists(Caller, FirstNewBlock);

  // Merge the inlined function's basic blocks into the caller function
  mergeToCallSite(CI, FirstNewBlock, Returns);
}

void getReturns(Function *F, SmallVectorImpl<ReturnInst *> &Returns) {
  assert(Returns.empty() &&
         "Returns vector should be empty when calling getReturns!");

  for (auto &BB : *F) {
    if (ReturnInst *RI = dyn_cast<ReturnInst>(BB.getTerminator())) {
      Returns.push_back(RI);
    }
  }
}

bool shouldInline(CallInst *CI) {
  // Check conditions of the Caller and Callee to execute inlining.
  Function *Callee = CI->getCalledFunction();

  if (!Callee || Callee->isDeclaration()) {
    return false;
  }

  // Check if the function is recursive (i.e., the callee is the same as the
  // caller)
  if (Callee == CI->getFunction()) {
    return false;
  }

  // Check if the function has multiple return instruction.
  SmallVector<ReturnInst *, 4> Returns;
  getReturns(Callee, Returns);
  if (Returns.size() >= 2) {
    return false;
  }

  if (Callee->getName() == "oracle") {
    return false;
  }

  // Hard limit: code length 500.
  unsigned int totalInstructions = Callee->size() + CI->getFunction()->size();
  if (totalInstructions > 500) {
    return false;
  }

  return true;
}

PreservedAnalyses FunctionInlining::run(Module &M, ModuleAnalysisManager &MAM) {
  for (Function &F : M) {
    std::vector<CallInst *> callInstsToInline;
    // Gather all call instructions in the container
    for (auto &I : instructions(F)) {
      if (auto *CI = dyn_cast<CallInst>(&I)) {
        if (shouldInline(CI)) {
          callInstsToInline.push_back(CI);
        }
      }
    }

    // Process each call instruction
    for (auto it = callInstsToInline.begin(); it != callInstsToInline.end();) {
      CallInst *CI = *it;
      ++it; // Increment the iterator before inlining, since inlining
            // invalidates the iterator

      Function *Callee = CI->getCalledFunction();
      inlineFunction(CI, *Callee);
    }

    LLVM_DEBUG(dbgs() << "After Function Inlining Pass\n" << F << "\n");
  }
  return PreservedAnalyses::none();
};

extern "C" ::llvm::PassPluginLibraryInfo llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "FunctionInlining", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, ModulePassManager &MPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "function-inlining") {
                    MPM.addPass(FunctionInlining());
                    return true;
                  }
                  return false;
                });
          }};
};
} // namespace sc::opt::function_inlining