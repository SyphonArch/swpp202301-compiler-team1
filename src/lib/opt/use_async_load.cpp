// use_async_load.cpp

#include "use_async_load.h"

#include "llvm/IR/Type.h"
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

#include "InstructionCost.h"

using namespace llvm;
using namespace std;

namespace sc::opt::use_async_load {
PreservedAnalyses
UseAsyncLoad::run(Function &F, FunctionAnalysisManager &FAM) {
    DominatorTree &DT = FAM.getResult<DominatorTreeAnalysis>(F);
   for (BasicBlock &BB : F) {
      for (Instruction &I : BB) {
        if (LoadInst *loadInst= dyn_cast<LoadInst>(&I)) {
          // Get the pointerOperand of the load instruction.
          Value *loadPtr = loadInst->getPointerOperand();
          Instruction *priorInst = loadInst->getPrevNode();

          // Traverse the instructions upwards from the load instruction
          // and check for dependence with each instruction.
          while ((priorInst != &BB.front()) && priorInst) {
            // Check whether priorInst is a store or a load instruction
            // Question: How to optimize relative order of the load instructions?
            if(StoreInst *priorstoreInst = dyn_cast<StoreInst>(priorInst)) break;
            if(LoadInst *priorloadInst = dyn_cast<LoadInst>(priorInst)) break;
            // Check whether priorInst declares loadPtr
            if(loadPtr == priorInst) break;
            // If no dependence found, exchange the instructions.
            loadInst->moveBefore(priorInst);
            priorInst = loadInst->getPrevNode();
          }
          // Traverse the instructions downwards from the beginnig of the basic block
          // and check for instructions that uses the loadInst
          for(Instruction &J : BB){
            bool usesLoadInst = false;
            for(const Use &Op : J.operands()){
                if(Op.get() == loadInst){
                    usesLoadInst = true;
                }
            }
            // If instruction J uses loadInst, move down if possible.
            // Further Optimization: Some unrelevant instructions could go up.
            // Maybe implement by else to the if in line 62
            if(usesLoadInst){
                Instruction *nextInst = J.getNextNode();
                Instruction *thisInst = &J;
                while((nextInst != &BB.back()) && nextInst){
                    bool usesThisInst = false;
                    for(const Use &Op : nextInst->operands()){
                        if(Op.get() == thisInst){
                            usesThisInst = true;
                            break;
                        }
                    }
                    if(usesThisInst){
                        break;
                    }
                    thisInst->moveAfter(nextInst);
                    nextInst = thisInst->getNextNode();
                }
            }
          }
        }
      }
      // replace Load with Aload
      // if n loads -> replace top n-1 loads
      // if single load -> replace if cost > 5 before use
      // Further Optimization: ?
      //Assume that unused loads are removed in GVN pass.
      bool isLoadAlone = true;
      bool replaceWithAload;
      for(auto i = BB.begin(), e = BB.end(); i != e;){
        auto temp = i++;
        Instruction &I = *temp;
        replaceWithAload = false;
        if(LoadInst *loadInst = dyn_cast<LoadInst>(&I)){
          if(isLoadAlone){
            if(LoadInst *nextInst = dyn_cast<LoadInst>(loadInst->getNextNode())){
              replaceWithAload = true;
            }
            else{
              // first load instruction (cost 20) is added
              int cost = -20;
              bool isUsed = false;
              for (BasicBlock::iterator It = I.getIterator(), E = BB.end(); It != E; ++It) {
                for(const Use &Op : It->operands()){
                  if(Op.get() == loadInst){
                    isUsed = true;
                    break;
                  }
                }
                if(isUsed) break;
                cost += getStrangeCost(dyn_cast<Instruction>(It));
              }
              if(cost >= 5){
                replaceWithAload = true;
              }
            }
          }
          else if(LoadInst *nextInst = dyn_cast<LoadInst>(loadInst->getNextNode())){
            replaceWithAload = true;
          }
        }
        else{
            isLoadAlone = true;
        }
        if(replaceWithAload){
          LoadInst *loadInst = dyn_cast<LoadInst>(&I);
          Value *loadPtr = loadInst->getPointerOperand();
          Type *Ty = loadInst->getType();
          IntegerType *ITy = dyn_cast<IntegerType>(Ty);
          unsigned BitWidth = ITy->getIntegerBitWidth();
          IRBuilder<> Builder(&I);
          LLVMContext &Ctx = I.getContext();
          Module *M = I.getModule();
          Value *Ptr = I.getOperand(0);

          if(BitWidth == 8){
            Type *Int8Ty = Type::getInt8Ty(Ctx);
            Type *Int8PtrTy = Type::getInt8PtrTy(Ctx);
            FunctionType *FuncType = FunctionType::get(Int8Ty,{Int8PtrTy}, false);
            FunctionCallee FC = M->getOrInsertFunction("aload_i8", FuncType);
            Value *Call = Builder.CreateCall(FC, Ptr);
            I.replaceAllUsesWith(Call);
            I.eraseFromParent();
          }
          else if(BitWidth == 16){
            Type *Int16Ty = Type::getInt16Ty(Ctx);
            Type *Int16PtrTy = Type::getInt16PtrTy(Ctx);
            FunctionType *FuncType = FunctionType::get(Int16Ty,{Int16PtrTy}, false);
            FunctionCallee FC = M->getOrInsertFunction("aload_i16", FuncType);
            Value *Call = Builder.CreateCall(FC, Ptr);
            I.replaceAllUsesWith(Call);
            I.eraseFromParent();
          }
          else if(BitWidth == 32){
            Type *Int32Ty = Type::getInt32Ty(Ctx);
            Type *Int32PtrTy = Type::getInt32PtrTy(Ctx);
            FunctionType *FuncType = FunctionType::get(Int32Ty,{Int32PtrTy}, false);
            FunctionCallee FC = M->getOrInsertFunction("aload_i32", FuncType);
            Value *Call = Builder.CreateCall(FC, loadPtr);
            I.replaceAllUsesWith(Call);
            I.eraseFromParent();
          }
          else if(BitWidth == 64){
            Type *Int64Ty = Type::getInt64Ty(Ctx);
            Type *Int64PtrTy = Type::getInt64PtrTy(Ctx);
            FunctionType *FuncType = FunctionType::get(Int64Ty,{Int64PtrTy}, false);
            FunctionCallee FC = M->getOrInsertFunction("aload_i64", FuncType);
            Value *Call = Builder.CreateCall(FC, Ptr);
            I.replaceAllUsesWith(Call);
            I.eraseFromParent();
          }
        }
      }
    } 

    return PreservedAnalyses::none(); // or all();
};

extern "C" ::llvm::PassPluginLibraryInfo llvmGetPassPluginInfo() {
    return {LLVM_PLUGIN_API_VERSION, "UseAsyncLoad", LLVM_VERSION_STRING,
            [](PassBuilder &PB) {
                PB.registerPipelineParsingCallback(
                        [](StringRef Name, FunctionPassManager &FPM,
                           ArrayRef <PassBuilder::PipelineElement>) {
                            if (Name == "use-async-load") {
                                FPM.addPass(UseAsyncLoad());
                                return true;
                            }
                            return false;
                        });
            }};
};
}