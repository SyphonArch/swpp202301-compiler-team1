#include "loop_unrolling.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"

using namespace llvm;

namespace sc::opt::loop_unrolling {
PreservedAnalyses LoopUnrolling::run(Function &F, FunctionAnalysisManager &FAM) {
  LoopInfo &LI = FAM.getResult<LoopAnalysis>(F);
  ScalarEvolution &SE = FAM.getResult<ScalarEvolutionAnalysis>(F);

  bool Changed = false;

  for (Loop *L : LI) {
    // Check if the loop has a constant trip count
    const SCEV *TripCountSCEV = SE.getBackedgeTakenInfo(L)->getExact(SE);
    const SCEVConstant *TripCountConst = dyn_cast<SCEVConstant>(TripCountSCEV);
    if (!TripCountConst)
      continue;

    uint64_t LoopTripCount = TripCountConst->getValue()->getZExtValue();

    // Check if loop is small enough to be unrolled
    const unsigned MaxUnrollFactor = 8; // Maximum unroll factor
    if (LoopTripCount > MaxUnrollFactor)
      continue;

    // Perform loop unrolling
    BasicBlock *Header = L->getHeader();
    BasicBlock *Latch = L->getLoopLatch();

    if (!Header || !Latch)
      continue;

    LLVMContext &Ctx = Header->getContext();
    IRBuilder<> Builder(Ctx);

    // Duplicate loop body instructions
    for (unsigned i = 1; i < LoopTripCount; ++i) {
      BasicBlock *NewBlock = BasicBlock::Create(Ctx, Header->getName() + ".unrolled" + Twine(i), Header->getParent());

      // Clone instructions from the original loop into the new block
      for (Instruction &I : Header->getInstList()) {
        Instruction *Clone = I.clone();
        NewBlock->getInstList().push_back(Clone);

        // Update operand uses to point to the cloned instructions
        for (Use &Op : Clone->operands()) {
          Value *Operand = Op.get();
          if (Instruction *OpInst = dyn_cast<Instruction>(Operand)) {
            if (L->contains(OpInst)) {
              Op.set(Header->getInstList().back().getOperand(Op.getOperandNo()));
            }
          }
        }
      }

      // Update branch instructions
      for (Instruction &I : *NewBlock) {
        if (BranchInst *Branch = dyn_cast<BranchInst>(&I)) {
          for (unsigned SuccIdx = 0; SuccIdx < Branch->getNumSuccessors(); ++SuccIdx) {
            if (Branch->getSuccessor(SuccIdx) == Latch) {
              Branch->setSuccessor(SuccIdx, NewBlock);
            }
          }
        }
      }

      // Insert the new block into the function
      Function::iterator InsertPos = Header->getIterator();
      F.getBasicBlockList().insert(InsertPos, NewBlock);
    }
    // Update branch in the header to point to the first unrolled block
    if (BranchInst *Branch = dyn_cast<BranchInst>(Header->getTerminator())) {
      Branch->setSuccessor(0, &*(++Header->getIterator()));
    }

    // Remove the original loop from the loop info
    L->getParentLoop()->removeChildLoop(L);

    Changed = true;
  }

  if (Changed)
    return PreservedAnalyses::none();
  else
    return PreservedAnalyses::all();
}

extern "C" ::llvm::PassPluginLibraryInfo llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "LoopUnrolling", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "loop-unrolling") {
                    FPM.addPass(LoopUnrolling());
                    return true;
                  }
                  return false;
                });
          }};
}
} // namespace sc::opt::loop_unrolling
