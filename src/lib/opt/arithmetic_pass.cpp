#include "llvm/IR/Dominators.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/iterator_range.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/PatternMatch.h"
#include <vector>
#include <algorithm>
#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Operator.h"
#include "arithmetic_pass.h"

using namespace llvm;
using namespace std;

namespace sc::opt::arithmetic_pass {
PreservedAnalyses
ArithmeticPass::run(Function &F, FunctionAnalysisManager &FAM) {
  DominatorTree &DT = FAM.getResult<DominatorTreeAnalysis>(F);
  LLVMContext &Ctx = F.getContext();
  vector<Value *> ptrOperand;
  for (BasicBlock &BB : F) {

    for(auto i = BB.begin(), en = BB.end(); i!=en;){
      auto temp = i++;
      Instruction &I = *temp;


      // change add %a %a -> mul %a 2
      if (I.getOpcode() == Instruction::Add) {
        Value *Op0 = I.getOperand(0);
        Value *Op1 = I.getOperand(1);
        if (Op0 == Op1) { // Check if operands are equal
          Instruction*  NewInst = BinaryOperator::Create(Instruction::Mul, Op0, ConstantInt::get(Op0->getType(),2));
          ReplaceInstWithInst((&I), NewInst);
        }
      }


      // change shl %x c -> mul %x (2^c)
      if (I.getOpcode() == Instruction::Shl) {
        Value *Op0 = I.getOperand(0);
        ConstantInt* Op1 = dyn_cast<ConstantInt> (I.getOperand(1));
        uint32_t lval = Op1->getZExtValue();
        Instruction*  NewInst = BinaryOperator::Create(Instruction::Mul, Op0, ConstantInt::get(Op0->getType(),(1<<lval)));
        ReplaceInstWithInst((&I), NewInst);
      }


      // change ashr %x c -> sdiv %x (2^c)
      if (I.getOpcode() == Instruction::AShr) {
        Value *Op0 = I.getOperand(0);
        ConstantInt* Op1 = dyn_cast<ConstantInt> (I.getOperand(1));
        uint32_t lval = Op1->getZExtValue();
        Instruction*  NewInst = BinaryOperator::Create(Instruction::SDiv, Op0, ConstantInt::get(Op0->getType(),(1<<lval)));
        ReplaceInstWithInst((&I), NewInst);
      }


      // change lshr %x c -> udiv %x (2^c)
      if (I.getOpcode() == Instruction::LShr) {
        Value *Op0 = I.getOperand(0);
        ConstantInt* Op1 = dyn_cast<ConstantInt> (I.getOperand(1));
        uint32_t lval = Op1->getZExtValue();
        Instruction*  NewInst = BinaryOperator::Create(Instruction::UDiv, Op0, ConstantInt::get(Op0->getType(),(1<<lval)));
        ReplaceInstWithInst((&I), NewInst);
      }


      // change add %a 1~4 -> call incr
      // for each type of operands -> different name of function should be called
      // for each constant being added, there are different numbers of function calls

      if (I.getOpcode() == Instruction::Add) {
        IRBuilder<> Builder(&I);
        Value *Op0 = I.getOperand(0);
        Value *Op1 = I.getOperand(1);
        ConstantInt *C = dyn_cast<ConstantInt>(Op1);
        if (C && (C->getValue() == 1 || C->getValue() == 2 || C->getValue() == 3 || C->getValue() == 4)) {
          LLVMContext &Ctx = I.getContext();
          FunctionType *FuncType;
          Module *M = I.getModule();
          FunctionCallee FC;
          if(Op0->getType()->isIntegerTy(1)) {
            Type *Int1Ty = Type::getInt1Ty(Ctx);
            FuncType = FunctionType::get(Int1Ty, {Int1Ty}, false);
            FC = M->getOrInsertFunction("incr_i1", FuncType);
          } else if(Op0->getType()->isIntegerTy(8)) {
            Type *Int8Ty = Type::getInt8Ty(Ctx);
            FuncType = FunctionType::get(Int8Ty, {Int8Ty}, false);
            FC = M->getOrInsertFunction("incr_i8", FuncType);
          } else if(Op0->getType()->isIntegerTy(16)) {
            Type *Int16Ty = Type::getInt16Ty(Ctx);
            FuncType = FunctionType::get(Int16Ty, {Int16Ty}, false);
            FC = M->getOrInsertFunction("incr_i16", FuncType);
          } else if(Op0->getType()->isIntegerTy(32)) {
            Type *Int32Ty = Type::getInt32Ty(Ctx);
            FuncType = FunctionType::get(Int32Ty, {Int32Ty}, false);
            FC = M->getOrInsertFunction("incr_i32", FuncType);
          } else if(Op0->getType()->isIntegerTy(64)) {
            Type *Int64Ty = Type::getInt64Ty(Ctx);
            FuncType = FunctionType::get(Int64Ty, {Int64Ty}, false);
            FC = M->getOrInsertFunction("incr_i64", FuncType);
          } else {
            continue;
          }
          if(C->getValue() == 1) {
            Value *Arg = Op0;
            Value *Call1 = Builder.CreateCall(FC, Arg);
            I.replaceAllUsesWith(Call1);
          } else if (C->getValue() == 2) {
            Value *Arg = Op0;
            Value *Call1 = Builder.CreateCall(FC, Arg);
            Value *Call2 = Builder.CreateCall(FC, Call1);
            I.replaceAllUsesWith(Call2);
          } else if (C->getValue() == 3) {
            Value *Arg = Op0;
            Value *Call1 = Builder.CreateCall(FC, Arg);
            Value *Call2 = Builder.CreateCall(FC, Call1);
            Value *Call3 = Builder.CreateCall(FC, Call2);
            I.replaceAllUsesWith(Call3);
          } else { // c->getvalue = 4
            Value *Arg = Op0;
            Value *Call1 = Builder.CreateCall(FC, Arg);
            Value *Call2 = Builder.CreateCall(FC, Call1);
            Value *Call3 = Builder.CreateCall(FC, Call2);
            Value *Call4 = Builder.CreateCall(FC, Call3);
            I.replaceAllUsesWith(Call4);
          }
          I.eraseFromParent();
        }
      }
    }
  }
  return PreservedAnalyses::all();
}



extern "C" ::llvm::PassPluginLibraryInfo llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "arithmetic-pass", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "arithmetic-pass") {
                    FPM.addPass(ArithmeticPass());
                    return true;
                  }
                  return false;
                });
          }};
}
};