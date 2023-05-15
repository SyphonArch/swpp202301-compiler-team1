// store_to_oracle.cpp

#include "store_to_oracle.h"

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
#include <cstddef>

using namespace llvm;
using namespace std;


void replaceWithOracle (Function &F, Instruction &I, int storeInstCount, std::vector<StoreInst*> storeInstArray, std::vector<Value*> storeOperandsArray){
    // Define a struct to hold the operands for the Oracle intrinsic
    StructType* OperandStructType = StructType::get(
        F.getContext(),
        { Type::getInt64Ty(F.getContext()), // value operand
      Type::getInt64Ty(F.getContext()), // pointer operand
      Type::getInt8Ty(F.getContext())   // value type
      }
    );
  // Remove storeInst
  for (int i = 0; i < storeInstCount; i++) {
    storeInstArray[i]->eraseFromParent();
  }

  // Define the Oracle intrinsic
  FunctionType* OracleFuncType = FunctionType::get(Type::getVoidTy(F.getContext()), {OperandStructType->getPointerTo(), Type::getInt32Ty(F.getContext())}, false);
  Function* OracleFunc = Function::Create(OracleFuncType, Function::ExternalLinkage, "Oracle", F.getParent());
  
  // Implement the Oracle intrinsic
  BasicBlock* EntryBB = BasicBlock::Create(F.getContext(), "entry", OracleFunc);
  IRBuilder<> Builder(EntryBB);
  
  // Get the function arguments
  Argument* StoreOperandsArray = &*(OracleFunc->arg_begin());
  StoreOperandsArray->setName("storeOperandsArray");
  Argument* StoreInstCount = &*std::next(OracleFunc->arg_begin());
  StoreInstCount->setName("storeInstCount");
  
  // Loop through the store operand array and execute the store instructions
  for (int i = 0; i < cast<ConstantInt>(StoreInstCount)->getZExtValue(); i++) {
    Value* StoreOperandPtr = Builder.CreateInBoundsGEP(cast<PointerType>(StoreOperandsArray->getType())->getArrayElementType(), StoreOperandsArray, {ConstantInt::get(Type::getInt32Ty(F.getContext()), 0), ConstantInt::get(Type::getInt32Ty(F.getContext()), i)});
    Value* StoreOperand = Builder.CreateLoad(cast<PointerType>(StoreOperandsArray->getType())->getArrayElementType(), StoreOperandPtr);
    Value* ValueOperand = Builder.CreateExtractValue(StoreOperand, {0});
    Value* PointerOperand = Builder.CreateExtractValue(StoreOperand, {1});
    Value* BitWidth = Builder.CreateZExtOrTrunc(Builder.CreateExtractValue(StoreOperand, {2}), Type::getInt32Ty(F.getContext()));
    StoreInst* StoreInst = Builder.CreateStore(ValueOperand, PointerOperand);
    StoreInst->setAlignment(llvm::Align(cast<ConstantInt>(BitWidth)->getZExtValue()));
  }
  Builder.CreateRetVoid();

  // Set up the IRBuilder to insert instructions before the original instruction
  IRBuilder<> CallerBuilder(&I);

  // Create the call instruction
  FunctionCallee OracleCallee = OracleFunc;
  Value* OracleResult = CallerBuilder.CreateCall(OracleCallee, {StoreOperandsArray, StoreInstCount});

  // Insert the Oracle call before the original instruction
  CallerBuilder.Insert(OracleResult);
}


namespace sc::opt::store_to_oracle {
PreservedAnalyses StoreToOracle::run(Function &F, FunctionAnalysisManager &FAM) {
  DominatorTree &DT = FAM.getResult<DominatorTreeAnalysis>(F);
  for (BasicBlock &BB : F) {
    if (BB.empty())
      continue;
    // Part 1 : Move store instructions up
    for (Instruction &I : BB) {
      if (!dyn_cast<StoreInst>(&I))
        continue;
      
      StoreInst *storeInst = dyn_cast<StoreInst>(&I);
      Instruction *priorStoreInst = storeInst->getPrevNode();
      Value *storeVal = storeInst->getValueOperand();
      Value *storePtr = storeInst->getPointerOperand();

      while (priorStoreInst){
        if (dyn_cast<PHINode>(priorStoreInst) ||
            dyn_cast<CallInst>(priorStoreInst) ||
            priorStoreInst->mayReadOrWriteMemory())
          break;
        if (storeVal == priorStoreInst ||
            storePtr == priorStoreInst)
          break;
        storeInst->moveBefore(priorStoreInst);
        priorStoreInst = storeInst->getPrevNode();
      }
      
    }
    
    StructType* OperandStructType = StructType::get(
      F.getContext(),
      { Type::getInt64Ty(F.getContext()), // value operand
      Type::getInt64Ty(F.getContext()), // pointer operand
      Type::getInt8Ty(F.getContext())   // value type
      }
    );
    // Part 2 : replace consecutive store instructions with an oracle call 
    int storeInstCount = 0;
    Value* OperandsStruct = UndefValue::get(OperandStructType);
    std::vector<StoreInst*> storeInstArray;
    std::vector<Value*> storeOperandsArray;
    for (Instruction &I : BB){
      StoreInst *storeInst = dyn_cast<StoreInst>(&I);
      // replaceWithOracle if more than 3 consecutive store instructions (reduce cost)
      if (!dyn_cast<StoreInst>(&I)) {
        invalid_store:
        if (storeInstCount >= 3)
          storeInstCount = storeInstCount;
          //replaceWithOracle (F, I, storeInstCount, storeInstArray, storeOperandsArray);
        storeInstCount = 0;
        storeInstArray.clear();
        storeOperandsArray.clear();
        continue;
      }
      
      // Extract required information from storeInst
      Value *storeVal = storeInst->getValueOperand();
      Value *storePtr = storeInst->getPointerOperand();
      unsigned int storeWidth = (storeVal->getType())->getIntegerBitWidth();

      // Increment storeInstCount, Push storeInst, Cast storeVal, storePtr, storeWidth to i64, i64, i8 (extra cost)
      if (!storeVal || !storePtr || !storeWidth)
        goto invalid_store; // Invalid store instruction. Oracle call insertion place does not matter.
      storeInstCount++;
      storeInstArray.push_back(storeInst);
      storeVal = new ZExtInst(storeVal, Type::getInt64Ty(F.getContext()), "", &I);
      storePtr = new PtrToIntInst(storePtr, Type::getInt64Ty(F.getContext()), "", &I);

    
      // Create a constant for the value type, operand, and pointer operand
      Constant* storeWidthConst = ConstantInt::get(Type::getInt8Ty(F.getContext()), (uint64_t)storeWidth);
      Constant* storeValConst = ConstantInt::get(Type::getInt64Ty(F.getContext()), cast<ConstantInt>(storeVal)->getZExtValue());
      Constant* storePtrConst = ConstantExpr::getPtrToInt(cast<ConstantInt>(storePtr), Type::getInt64Ty(F.getContext()));

      OperandsStruct = ConstantStruct::get(OperandStructType, storeWidthConst, storeValConst, storePtrConst);
      storeOperandsArray.push_back(OperandsStruct);
    }
    
  }
    return PreservedAnalyses::none();
};

extern "C" ::llvm::PassPluginLibraryInfo llvmGetPassPluginInfo() {
    return {LLVM_PLUGIN_API_VERSION, "StoreToOracle", LLVM_VERSION_STRING,
            [](PassBuilder &PB) {
                PB.registerPipelineParsingCallback(
                        [](StringRef Name, FunctionPassManager &FPM,
                           ArrayRef <PassBuilder::PipelineElement>) {
                            if (Name == "store-to-oracle") {
                                FPM.addPass(StoreToOracle());
                                return true;
                            }
                            return false;
                        });
            }};
};
}