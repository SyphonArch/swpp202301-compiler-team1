#include "use_async_load.h"

#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/Type.h"
#include "llvm/Pass.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/Casting.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include <string>

using namespace llvm;
using namespace std;

// not the exact cost; just the lower bound
// especially, a load instruction is consdiered cost 1 since it could be replaced by an aload call 
// For later:   (1) Check whether all instructions not mentioned are translated into assemblies of nonzero cost.
//              (2) For further optimization that does not rely on heuristics, exact cost of all instructions are required. 
//              (3) Cost for resolving aload is not considered.
int getMinCost(Instruction *I) {
  unsigned int Op = I->getOpcode();
  if (isa<SwitchInst>(I) || I->isShift() || I->isBitwiseLogicOp()) {
    return 4;
  } else if (Op == llvm::Instruction::Add || Op == llvm::Instruction::Sub ||
             I->mayReadOrWriteMemory() && !(Op == llvm::Instruction::Load)) {
    return 5;
  } else if (auto *call = dyn_cast<CallInst>(I)) {
    llvm::Function *fun = call->getCalledFunction();
    assert(fun);
    StringRef name = fun->getName();
    int argNum = fun->getNumOperands();
    if (name.startswith("int_sum_i") || name.equals("oracle")) {
      return 5;
    } else if (name.startswith("assert_eq_i")) {
      return 0;
    } else if (name.startswith("aload_i") || name.startswith("incr_i") || name.startswith("decr_i")) {
      return 1;
    } else {
      return 2 + argNum;
    }
  } else {
    return 1;
  }
}

void replaceWithAload(Instruction &I) {
  LoadInst *loadInst = dyn_cast<LoadInst>(&I);
  Value *loadPtr = loadInst->getPointerOperand();
  Type *Ty = loadInst->getType();
  IntegerType *ITy = dyn_cast<IntegerType>(Ty);
  PointerType *PtrTy = PointerType::get(Ty, 0);
  unsigned bitWidth = ITy->getIntegerBitWidth();
  std::string BitWidthString = std::to_string(bitWidth);

  IRBuilder<> Builder(&I);
  LLVMContext &Ctx = I.getContext();
  Module *M = I.getModule();
  Value *Ptr = I.getOperand(0);

  FunctionType *FuncType = FunctionType::get(ITy, {PtrTy}, false);
  FunctionCallee FC =
      M->getOrInsertFunction("aload_i" + BitWidthString, FuncType);
  ArrayRef<Value *> args = llvm::makeArrayRef(Ptr);
  Value *Call = Builder.CreateCall(FC, args);
  I.replaceAllUsesWith(Call);
  I.eraseFromParent();
}

bool isAloadCall(Instruction &I) {
  CallInst *callInst = dyn_cast<CallInst>(&I);
  return callInst && callInst->getCalledFunction() &&
         callInst->getCalledFunction()->getName().startswith("aload_i");
}

namespace sc::opt::use_async_load {
PreservedAnalyses UseAsyncLoad::run(Function &F, FunctionAnalysisManager &FAM) {
  DominatorTree &DT = FAM.getResult<DominatorTreeAnalysis>(F);
  for (BasicBlock &BB : F) {
    if(BB.empty())
      continue;
    // Part 1 : Move load instructions up
    for (Instruction &I : BB) {
      if (!dyn_cast<LoadInst>(&I))
        continue;

      LoadInst *loadInst = dyn_cast<LoadInst>(&I);
      Value *loadPtr = loadInst->getPointerOperand();
      Instruction *priorLoadInst = loadInst->getPrevNode();

      // Find loadInst, starting from BB.begin(). Move up if priorLoadInst (1) is not a load / store instruction. (2) does not define loadPtr.
      // For Later: Optimize relative order of load instructions? Currently, initial ordering of load instructions is left unchanged.
      while (priorLoadInst) {
        if (dyn_cast<StoreInst>(priorLoadInst) ||
            dyn_cast<LoadInst>(priorLoadInst))
          break;
        if (loadPtr == priorLoadInst)
          break;
        loadInst->moveBefore(priorLoadInst);
        priorLoadInst = loadInst->getPrevNode();
      }

      // Part 2: Move instructions that does not use loadInst up.
      // Find indepInst that does not use loadInst, starting from loadInst->getNextNode()
      if (loadInst->isTerminator() || loadInst->getNextNode()->isTerminator())
        continue;
      for (auto j = loadInst->getNextNode()->getIterator(), e = BB.end();
           j != e;) {
        // j++ is requried to prevent infinite loop.
        // Without it, independent load instructions forming a 'cluster' will keep exchanging each others.
        Instruction &J = *(j++);
        if (J.isTerminator())
          break;
        if (dyn_cast<LoadInst>(&J))
          continue;
        bool usesLoadInst = false;
        for (const Use &Op : J.operands()) {
          if (Op.get() == loadInst)
            usesLoadInst = true;
        }
        if (usesLoadInst)
          continue;

        Instruction *indepInst = &J;
        Instruction *priorIndepInst = indepInst->getPrevNode();

        // Move up if (1) priorIndepInst is not used in indepInst (2) priorIndepInst is not a load instruction
        while (priorIndepInst) {
          if (dyn_cast<LoadInst>(priorIndepInst))
            break;
          bool usesPriorIndepInst = false;
          for (const Use &Op : indepInst->operands()) {
            if (Op.get() == priorIndepInst)
              usesPriorIndepInst = true;
          }
          if (usesPriorIndepInst)
            break;
          indepInst->moveBefore(priorIndepInst);
          priorIndepInst = indepInst->getPrevNode();
        }
      }
    }

    // Part 3 : Rearrange load instructions for best results. (NOT IMPLEMENTED)

    // Part 4: Replace load with aload (Assume that unused loads are removed in GVN pass.) 
    // (1) For each load, replace load with an aload call if the lower bound of cost before use is greater than or equal to 5.
    int sumMinCost;
    bool isLoadInstUsed;
    for (auto i = BB.begin(), e = BB.end(); i != e;) {
      // i++ required since replaceWithAload(I); destroys I and thus the loop without it.
      Instruction &I = *(i++);
      if (!dyn_cast<LoadInst>(&I))
        continue;
      LoadInst *loadInst = dyn_cast<LoadInst>(&I);
      sumMinCost = 0;
      isLoadInstUsed = false;
      for (auto j = I.getNextNode()->getIterator(), e = BB.end(); j != e; j++) {
        Instruction &J = *j;
        for (const Use &Op : J.operands()) {
          if (Op.get() == loadInst)
            isLoadInstUsed = true;
        }
        if (isLoadInstUsed)
          break;
        sumMinCost += getMinCost(&J);
      }
      if (sumMinCost >= 5)
        replaceWithAload(I);
    }

    // (2) After 1st pass, replace load with an aload call if an unchanged load instruction exists below. 
    // (Only check for consecutive load/aload instructions.)
    bool loadExistsBelow;
    for (BasicBlock::reverse_iterator i = BB.rbegin(), e = BB.rend(); i != e;) {
      // i++ required since replaceWithAload(I); destroys I and thus the loop without it.
      Instruction &I = *(i++);
      if (!dyn_cast<LoadInst>(&I) && !isAloadCall(I))
        loadExistsBelow = false;
      if (!dyn_cast<LoadInst>(&I))
        continue;

      if (loadExistsBelow)
        replaceWithAload(I);
      else
        loadExistsBelow = true;
    }
  }
  return PreservedAnalyses::none();
};

extern "C" ::llvm::PassPluginLibraryInfo llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "UseAsyncLoad", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "use-async-load") {
                    FPM.addPass(UseAsyncLoad());
                    return true;
                  }
                  return false;
                });
          }};
};
} // namespace sc::opt::use_async_load