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
// especially, a load instruction is consdiered cost 1 since it could be
// replaced by an aload call For later:   (1) Check whether all instructions not
// mentioned are translated into assemblies of nonzero cost.
//              (2) For further optimization that does not rely on heuristics,
//              exact cost of all instructions are required. (3) Cost for
//              resolving aload is not considered.

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
    if (name.startswith("int_sum_i")) {
      return 10;
    } else if (name.startswith("assert_eq_i")) {
      return 0;
    } else if (name.startswith("aload_i") || name.startswith("incr_i") ||
               name.startswith("decr_i")) {
      return 1;
    } // added oracle, maybe not that useful but who knows?
    else if (name.startswith("oracle")) {
      return 40;
    } else {
      return 2 + argNum;
    }
  }

  else if (dyn_cast<GetElementPtrInst>(I)) {
    // getelementptr operation spends 6 cost
    return 6;
  } else if (dyn_cast<StoreInst>(I)) {
    return 34;
  } else if (dyn_cast<LoadInst>(I)) {
    // if the value is not aload, just consider it as a the least cost possible
    // in our implementation, it finally checks whether if the load instruction
    // should be aload or not so no need to worry about cost incresing after
    // changing to aload the only benefit for 'not considering load cost as 1'
    // is it might be useful for preventing register spilling, but I think
    // reducing load cost is more imporatant
    return 1;
  } else {
    return 1;
  }
}

void replaceWithAload(Instruction &I) {
  LoadInst *loadInst = dyn_cast<LoadInst>(&I);
  Value *loadPtr = loadInst->getPointerOperand();
  Type *Ty = loadInst->getType();
  if (loadPtr == nullptr || Ty == nullptr)
    return;
  IntegerType *ITy = dyn_cast<IntegerType>(Ty);
  PointerType *PtrTy = PointerType::get(Ty, 0);
  if (ITy == nullptr || PtrTy == nullptr)
    return;
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

// newly added!
// cannot move over two way branch
// if the parent of the current block is the two way branch, it means there can
// the next block might not be the curr block so if we move the instruction from
// curr to parent, that instruction might be executed more than needed

bool isTwoWayBranchInstruction(llvm::Instruction *inst) {
  if (llvm::BranchInst *branchInst = llvm::dyn_cast<llvm::BranchInst>(inst)) {
    return branchInst->isConditional() && branchInst->getNumSuccessors() == 2;
  }
  return false;
}

// check all operand in getelementptr instruction and see if and of its operand
// is the prior instruction
bool isInstUsedInGEP(const GetElementPtrInst *geteleInst,
                     const Instruction *priorGEPInst) {
  for (const Use &use : geteleInst->operands()) {
    if (use.get() == priorGEPInst) {
      return true;
    }
  }
  return false;
}

// check all operand in getelementptr instruction and see if and of its operand
// is the prior instruction
bool hasOperandsext(const SExtInst *sextInst,
                    const Instruction *priorsextInst) {
  for (const Use &use : sextInst->operands()) {
    if (use.get() == priorsextInst) {
      return true;
    }
  }
  return false;
}

namespace sc::opt::use_async_load {
PreservedAnalyses UseAsyncLoad::run(Function &F, FunctionAnalysisManager &FAM) {

  // first, if the name of the function is 'oracle', do not use aload
  StringRef functionName = F.getName();
  if (functionName.equals("oracle")) {
    return PreservedAnalyses::all();
  }

  DominatorTree &DT = FAM.getResult<DominatorTreeAnalysis>(F);

//for(int temp = 0; temp < 2; temp ++) {
  
  for (BasicBlock &BB : F) {
    if (BB.empty())
      continue;
    // Part 1 : Move load instructions up
    for (BasicBlock::iterator instrIter = BB.begin(), endIter = BB.end(); instrIter != endIter; ) {
      Instruction &I = *instrIter;
      ++instrIter;

      // newly added!!

      /*
            // in this part, we check if the sext instruction
            // after looking at benchmark programs, sometimes getelementptr
         instructions were followed by sext instruction
            // for safety, at first we dont move it over phi or def of it

            if (dyn_cast<SExtInst>(&I)) {
              SExtInst *getsextinst = dyn_cast<SExtInst>(&I);
              Instruction *priorsextInst = getsextinst->getPrevNode();

              //make a new variable
              //this is used to somewhat prevent register spilling
              int cost_may_reduce_sext = 0;
              while (priorsextInst) {
                if (dyn_cast<PHINode>(priorsextInst) ||
                dyn_cast<GetElementPtrInst>(priorsextInst) ||
                dyn_cast<SExtInst>(priorsextInst) ||
                dyn_cast<ZExtInst>(priorsextInst))
                  break;
                if (hasOperandsext(getsextinst, priorsextInst))
                  break;
                cost_may_reduce_sext += getMinCost(priorsextInst);
                getsextinst->moveBefore(priorsextInst);
                priorsextInst = getsextinst->getPrevNode();
                if(cost_may_reduce_sext >= 24) break;
              }
            } else
      */

      // in this part, we check if the instruction is getelement ptr, and move
      // it upwards for safety, at first we dont move it over phi or def of it
      if (auto *getElemPtrInst = dyn_cast<GetElementPtrInst>(&I)) {
        Instruction *priorGEPInst = getElemPtrInst->getPrevNode();

        // make a new variable
        // this will count tue number of costs that will be reduced by changing
        // positions in here, it is the getelementptr instruction that are
        // moving, not load, but almost the same! if the reduced cost is bigger
        // than 24, which is the max coost that can be reduced, do not move
        // further! this is used to somewhat prevent register spilling
        int costMayReduceGEP = 0;
        while (priorGEPInst) {
          if (dyn_cast<PHINode>(priorGEPInst) ||
              dyn_cast<GetElementPtrInst>(priorGEPInst) ||
              dyn_cast<LoadInst>(priorGEPInst))
            break;
          if (isInstUsedInGEP(getElemPtrInst, priorGEPInst))
            break;
          costMayReduceGEP += getMinCost(priorGEPInst);
          getElemPtrInst->moveBefore(priorGEPInst);
          priorGEPInst = getElemPtrInst->getPrevNode();
          if (costMayReduceGEP >= 24)
            break;
        }
      }

      if (!dyn_cast<LoadInst>(&I))
        continue;

      LoadInst *loadInst = dyn_cast<LoadInst>(&I);
      Value *loadPtr = loadInst->getPointerOperand();
      Instruction *priorLoadInst = loadInst->getPrevNode();

      // Find loadInst, starting from BB.begin(). Move up if priorLoadInst (1)
      // is not a load / store / PHINode instruction. (2) does not define
      // loadPtr. For Later: Optimize relative order of load instructions?
      // Currently, initial ordering of load instructions is left unchanged.

      // new conditions added
      // 1. is twowaybranchinstruction
      // do not go over 2 way branch
      // it might increase cost

      // 2. getelementptr
      // if we let the load instruction go over getelementptr, inf loop occurs

      // 3. if cost_may_reduce >= 24 break
      // this will help us prevent register spilling

      int costMayReduceLoad = 0;

      while (priorLoadInst) {
        if (dyn_cast<StoreInst>(priorLoadInst) ||
            dyn_cast<LoadInst>(priorLoadInst) ||
            dyn_cast<PHINode>(priorLoadInst) ||
            priorLoadInst->mayReadOrWriteMemory())
      //       || dyn_cast<GetElementPtrInst>(priorLoadInst))
          break;
        if (loadPtr == priorLoadInst)
          break;
        costMayReduceLoad += getMinCost(priorLoadInst);
        loadInst->moveBefore(priorLoadInst);
        priorLoadInst = loadInst->getPrevNode();
        if (costMayReduceLoad >= 24)
          break;
      }

      // Part 2: Move instructions that does not use loadInst up.
      // Find indepInst that does not use loadInst, starting from
      // loadInst->getNextNode()

      // do not change order of instructions before use load inst
      bool isBelowUseLoadInst = false;
      Instruction *usingLoadInst;

      if (loadInst->isTerminator() || loadInst->getNextNode()->isTerminator())
        continue;
      for (auto j = loadInst->getNextNode()->getIterator(), e = BB.end();
           j != e;) {
        // j++ is requried to prevent infinite loop.
        // Without it, independent load instructions forming a 'cluster' will
        // keep exchanging each others.
        Instruction &J = *(j++);
        if (J.isTerminator())
          break;
        if (dyn_cast<LoadInst>(&J))
          continue;

        bool usesLoadInst = false;
        for (const Use &Op : J.operands()) {
          if (Op.get() == loadInst) {
            usingLoadInst = &J;
            usesLoadInst = true;
          }
        }

        if (usesLoadInst) {
          isBelowUseLoadInst = true;
          continue;
        }

        if (!isBelowUseLoadInst) continue;

        Instruction *indepInst = &J;
        Instruction *priorIndepInst = indepInst->getPrevNode();

        // Move up if (1) priorIndepInst is not used in indepInst (2)
        // priorIndepInst is not a load / PHINode instruction
        // again, move as small as possible to stop register spilling

        while (priorIndepInst) {
          if (dyn_cast<LoadInst>(priorIndepInst) ||
              dyn_cast<PHINode>(priorIndepInst) ||
              priorIndepInst->mayReadOrWriteMemory())
            break;
          bool usesPriorIndepInst = false;
          for (const Use &Op : indepInst->operands()) {
            if (Op.get() == priorIndepInst)
              usesPriorIndepInst = true;
          }
          if (usesPriorIndepInst)
            break;
          indepInst->moveBefore(priorIndepInst);

          //if the first us of the load instruction has gone down, do not change more
//          if(priorIndepInst == usingLoadInst)
//            break;

          priorIndepInst = indepInst->getPrevNode();
        }
      }
    }

    // Part 3 : Rearrange load instructions for best results. (NOT IMPLEMENTED)

    // Part 4: Replace load with aload (Assume that unused loads are removed in
    // GVN pass.) (1) For each load, replace load with an aload call if the
    // lower bound of cost before use is greater than or equal to 5.
    int sumMinCost;
    bool isLoadInstUsed;
    for (auto i = BB.begin(), e = BB.end(); i != e;) {
      // i++ required since replaceWithAload(I); destroys I and thus the loop
      // without it.
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
        if (isLoadInstUsed || sumMinCost > 5)
          break;
        sumMinCost += getMinCost(&J);
      }
      if (sumMinCost >= 5)
        replaceWithAload(I);
    }

    // (2) After 1st pass, replace load with an aload call if an unchanged load
    // instruction exists below. (Only check for consecutive load/aload
    // instructions.)
    bool loadExistsBelow;
    for (BasicBlock::reverse_iterator i = BB.rbegin(), e = BB.rend(); i != e;) {
      // i++ required since replaceWithAload(I); destroys I and thus the loop
      // without it.
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

//}

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
