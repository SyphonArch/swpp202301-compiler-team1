// store_to_oracle.cpp

#include "store_to_oracle.h"

#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/Intrinsics.h"
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

namespace sc::opt::store_to_oracle {


/*
llvm::Function* declareOracleIntrinsic(llvm::Module* module, llvm::LLVMContext& context) {

  // Argument of oracle intrinsic is variadic
  llvm::Type* retType = llvm::Type::getVoidTy(context);
  llvm::FunctionType* oracleFnType = llvm::FunctionType::get(retType, true);

  // Create the oracle intrinsic function
  llvm::Function* oracleFn = llvm::Function::Create(oracleFnType, llvm::Function::ExternalLinkage, "oracle", module);

  // Never inline the oracle intrinsic
  oracleFn->addFnAttr(llvm::Attribute::NoInline);

  return oracleFn;
}

void defineOracleIntrinsic(llvm::Function* oracleFn, llvm::LLVMContext& context) {
  llvm::BasicBlock* entryBlock = llvm::BasicBlock::Create(context, "entry", oracleFn);
  llvm::IRBuilder<> builder(entryBlock);

  llvm::Type* i8PtrTy = llvm::Type::getInt8PtrTy(context);
  llvm::Type* i64Ty = llvm::Type::getInt64Ty(context);
  llvm::Type* i32Ty = llvm::Type::getInt32Ty(context);

  llvm::Value* iterationCount = oracleFn->arg_begin();

  // Create loop structure
  llvm::BasicBlock* loopCondBlock = llvm::BasicBlock::Create(context, "loop.cond", oracleFn);
  llvm::BasicBlock* loopBodyBlock = llvm::BasicBlock::Create(context, "loop.body", oracleFn);
  llvm::BasicBlock* loopExitBlock = llvm::BasicBlock::Create(context, "loop.exit", oracleFn);

  builder.CreateBr(loopCondBlock);
  builder.SetInsertPoint(loopCondBlock);
  llvm::PHINode* iterationPhi = builder.CreatePHI(i32Ty, 2);
  iterationPhi->addIncoming(iterationCount, entryBlock);
  llvm::Value* isLoopExit = builder.CreateICmpEQ(iterationPhi, llvm::ConstantInt::get(i32Ty, 0));
  builder.CreateCondBr(isLoopExit, loopExitBlock, loopBodyBlock);

  // Loop body
  builder.SetInsertPoint(loopBodyBlock);
  llvm::Value* index = builder.CreateSub(iterationPhi, llvm::ConstantInt::get(i32Ty, 1));

  // Iterate over the variadic arguments
  llvm::Function::arg_iterator args = std::next(oracleFn->arg_begin());
  for (llvm::Function::arg_iterator arg = args; arg != oracleFn->arg_end(); ++arg) {
    llvm::Value* addr = builder.CreateGEP(i8PtrTy, &*arg, llvm::ArrayRef<llvm::Value*>({ index }), "");
    ++arg;
    llvm::Value* val = builder.CreateGEP(i64Ty, &*arg, llvm::ArrayRef<llvm::Value*>({ index }), "");
    ++arg;
    llvm::Value* bitwidth = builder.CreateGEP(i32Ty, &*arg, llvm::ArrayRef<llvm::Value*>({ index }), "");

    // Create a switch statement based on the bitwidth argument
    llvm::SwitchInst* switchInst = builder.CreateSwitch(bitwidth, nullptr, 4);

    // Create case 8
    llvm::BasicBlock* case8Block = llvm::BasicBlock::Create(context, "case8", oracleFn);
    builder.SetInsertPoint(case8Block);
    llvm::Value* val8 = builder.CreateBitCast(val, llvm::Type::getInt8Ty(context));
    builder.CreateStore(val8, addr);
    builder.CreateBr(loopCondBlock);
    switchInst->addCase(llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 8), case8Block);

    // Create case 16
    llvm::BasicBlock* case16Block = llvm::BasicBlock::Create(context, "case16", oracleFn);
    builder.SetInsertPoint(case16Block);
    llvm::Value* ptr16 = builder.CreateBitCast(addr, llvm::Type::getInt16PtrTy(context));
    llvm::Value* val16 = builder.CreateBitCast(val, llvm::Type::getInt16Ty(context));
    builder.CreateStore(val16, ptr16);
    builder.CreateBr(loopCondBlock);
    switchInst->addCase(llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 16), case16Block);

    // Create case 32
    llvm::BasicBlock* case32Block = llvm::BasicBlock::Create(context, "case32", oracleFn);
    builder.SetInsertPoint(case32Block);
    llvm::Value* ptr32 = builder.CreateBitCast(addr, llvm::Type::getInt32PtrTy(context));
    llvm::Value* val32 = builder.CreateBitCast(val, llvm::Type::getInt32Ty(context));
    builder.CreateStore(val32, ptr32);
    builder.CreateBr(loopCondBlock);
    switchInst->addCase(llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 32), case32Block);

    // Create case 64
    llvm::BasicBlock* case64Block = llvm::BasicBlock::Create(context, "case64", oracleFn);
    builder.SetInsertPoint(case64Block);
    llvm::Value* ptr64 = builder.CreateBitCast(addr, llvm::Type::getInt64PtrTy(context));
    builder.CreateStore(val, ptr64);
    builder.CreateBr(loopCondBlock);
    switchInst->addCase(llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 64), case64Block);
  }

  // Insert return instruction
  builder.SetInsertPoint(loopExitBlock);
  builder.CreateRetVoid();

  // Update the PHI node in the loop condition block
  iterationPhi->addIncoming(index, loopBodyBlock);
  
}

*/

PreservedAnalyses StoreToOracle::run(Function &F, FunctionAnalysisManager &FAM) {
  DominatorTree &DT = FAM.getResult<DominatorTreeAnalysis>(F);

  // Declare and define oracle intrinsic
  LLVMContext &c = F.getContext();
  Module *m = F.getParent();
  //Function *oracleFn = declareOracleIntrinsic(m, c);
  //defineOracleIntrinsic(oracleFn, c);

  for (BasicBlock &BB : F) {
    // Part 1 : Move storeInst up
    //for (Instruction &I : BB) {

    //}

    // Part 2 : Fetch arguments {storePtr[], storeVal[], storeType[], storeNum} for consecutive storeInst
    for (Instruction &I : BB) {
      std::vector<StoreInst*> storeInstVec;
      std::vector<Value*> argsVec;
      StoreInst *storeInst = dyn_cast<StoreInst>(&I);
      if (!storeInst) {
        // Part 3 : If beneficial, call oracle intrinsic
        if (!storeInstVec.empty()) {
          // complete argsVec = {storeCount, addr_1, val_1, bitwidth_1, ... , addr_n, val_n, bitwidth_n}
          //argsVec.insert(argsVec.begin(), llvm::ConstantInt::get(llvm::Type::getInt32Ty(c), storeCount));

          //CallInst *oracleCall = CallInst::Create(oracleFn, argsVec);
          //oracleCall->setCallingConv(oracleFn->getCallingConv());
          //oracleCall->insertAfter(dyn_cast<Instruction>(storeInstVec[storeCount - 1]));
          //for (int i = 0; i < storeInstVec.size(); i++) {
            //storeInstVec[i]->removeFromParent();
          //}
        }
        //argsVec.clear();
        //storeInstVec.clear();
      } else {
        //storeInstVec.push_back(storeInst);
        if (storeInst->getParent()) {
          storeInst->removeFromParent();
        }
        //Value* addrArg = storeInst->getPointerOperand();
        //Value* valArg = storeInst->getValueOperand();
        //llvm::IntegerType* intType = llvm::dyn_cast<llvm::IntegerType>(storeInst->getValueOperand()->getType());
        //Value* bitwidthArg = llvm::ConstantInt::get(llvm::Type::getInt32Ty(c), intType->getBitWidth());

        // argsVec += {addr_i, val_i, bitwidth_i}
        //argsVec.push_back(addrArg);
        //argsVec.push_back(valArg);
        //argsVec.push_back(bitwidthArg);
      }
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