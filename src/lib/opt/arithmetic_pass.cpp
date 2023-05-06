#include "arithmetic_pass.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Operator.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

using namespace llvm;
using namespace std;

namespace sc::opt::arithmetic_pass {
PreservedAnalyses ArithmeticPass::run(Function &F,
                                      FunctionAnalysisManager &FAM) {

  // first preprocess
  // change add const %a -> add %a const
  for (BasicBlock &BB : F) {
    for (auto i = BB.begin(), en = BB.end(); i != en;) {
      auto temp = i++;
      Instruction &I = *temp;
      if (I.getOpcode() == Instruction::Add) {
        Value *Op0 = I.getOperand(0);
        Value *Op1 = I.getOperand(1);
        ConstantInt *C0 = dyn_cast<ConstantInt>(Op0);
        ConstantInt *C1 = dyn_cast<ConstantInt>(Op1);
        if (C0 && !C1) { // Check if operands are equal
          Instruction *NewInst =
              BinaryOperator::Create(Instruction::Add, Op1, Op0);
          ReplaceInstWithInst((&I), NewInst);
        }
      }
    }
  }
  // second preprocess
  // change add %a (-1 ~ -4) -> sub %a (1 ~ 4)
  // change sub %a (-1 ~ -4) -> add %a (1 ~ 4)

  for (BasicBlock &BB : F) {
    for (auto i = BB.begin(), en = BB.end(); i != en;) {
      auto temp = i++;
      Instruction &I = *temp;
      if (I.getOpcode() == Instruction::Add) {
        Value *Op0 = I.getOperand(0);
        Value *Op1 = I.getOperand(1);
        ConstantInt *C1 = dyn_cast<ConstantInt>(Op1);
        // getValue() cannot be compared, only == is allowed
        outs() << "negative num!!!\n";
        if (C1 && ((C1->getSExtValue() == -1) || (C1->getSExtValue() == -2) ||
                   (C1->getSExtValue() == -3) || (C1->getSExtValue() == -4))) {
          outs() << "negative num!!! in add!!!\n";
          Instruction *NewInst;
          if ((C1->getSExtValue() == -1))
            NewInst = BinaryOperator::Create(
                Instruction::Sub, Op0, ConstantInt::get(Op0->getType(), 1));
          else if ((C1->getSExtValue() == -2))
            NewInst = BinaryOperator::Create(
                Instruction::Sub, Op0, ConstantInt::get(Op0->getType(), 2));
          else if ((C1->getSExtValue() == -3))
            NewInst = BinaryOperator::Create(
                Instruction::Sub, Op0, ConstantInt::get(Op0->getType(), 3));
          else
            NewInst = BinaryOperator::Create(
                Instruction::Sub, Op0, ConstantInt::get(Op0->getType(), 4));
          ReplaceInstWithInst((&I), NewInst);
        }
      }
      if (I.getOpcode() == Instruction::Sub) {
        Value *Op0 = I.getOperand(0);
        Value *Op1 = I.getOperand(1);
        ConstantInt *C1 = dyn_cast<ConstantInt>(Op1);
        if (C1 && ((C1->getSExtValue() == -1) || (C1->getSExtValue() == -2) ||
                   (C1->getSExtValue() == -3) || (C1->getSExtValue() == -4))) {
          outs() << "negative num!!! in sub!!\n";
          Instruction *NewInst;
          if ((C1->getSExtValue() == -1))
            NewInst = BinaryOperator::Create(
                Instruction::Add, Op0, ConstantInt::get(Op0->getType(), 1));
          else if ((C1->getSExtValue() == -2))
            NewInst = BinaryOperator::Create(
                Instruction::Add, Op0, ConstantInt::get(Op0->getType(), 2));
          else if ((C1->getSExtValue() == -3))
            NewInst = BinaryOperator::Create(
                Instruction::Add, Op0, ConstantInt::get(Op0->getType(), 3));
          else
            NewInst = BinaryOperator::Create(
                Instruction::Add, Op0, ConstantInt::get(Op0->getType(), 4));
          ReplaceInstWithInst((&I), NewInst);
        }
      }
    }
  }

  for (BasicBlock &BB : F) {
    for (auto i = BB.begin(), en = BB.end(); i != en;) {
      auto temp = i++;
      Instruction &I = *temp;

      // change add %a %a -> mul %a 2
      if (I.getOpcode() == Instruction::Add) {
        Value *Op0 = I.getOperand(0);
        Value *Op1 = I.getOperand(1);
        if (Op0 == Op1) { // Check if operands are equal
          Instruction *NewInst = BinaryOperator::Create(
              Instruction::Mul, Op0, ConstantInt::get(Op0->getType(), 2));
          ReplaceInstWithInst((&I), NewInst);
        }
      }

      // change sub 0 %a -> mul %a -1
      if (I.getOpcode() == Instruction::Sub) {
        Value *Op0 = I.getOperand(0);
        Value *Op1 = I.getOperand(1);
        ConstantInt *C = dyn_cast<ConstantInt>(Op0);
        if (C && (C->getValue() == 0)) {
          Instruction *NewInst = BinaryOperator::Create(
              Instruction::Mul, Op1, ConstantInt::get(Op1->getType(), -1));
          ReplaceInstWithInst((&I), NewInst);
        }
      }

      // change shl %x c -> mul %x (2^c)
      if (I.getOpcode() == Instruction::Shl) {
        Value *Op0 = I.getOperand(0);
        ConstantInt *Op1 = dyn_cast<ConstantInt>(I.getOperand(1));
        uint32_t lval = Op1->getZExtValue();
        Instruction *NewInst = BinaryOperator::Create(
            Instruction::Mul, Op0,
            ConstantInt::get(Op0->getType(), (1 << lval)));
        ReplaceInstWithInst((&I), NewInst);
      }

      // change ashr %x c -> sdiv %x (2^c)
      if (I.getOpcode() == Instruction::AShr) {
        Value *Op0 = I.getOperand(0);
        ConstantInt *Op1 = dyn_cast<ConstantInt>(I.getOperand(1));
        uint32_t lval = Op1->getZExtValue();
        Instruction *NewInst = BinaryOperator::Create(
            Instruction::SDiv, Op0,
            ConstantInt::get(Op0->getType(), (1 << lval)));
        ReplaceInstWithInst((&I), NewInst);
      }

      // change lshr %x c -> udiv %x (2^c)
      if (I.getOpcode() == Instruction::LShr) {
        Value *Op0 = I.getOperand(0);
        ConstantInt *Op1 = dyn_cast<ConstantInt>(I.getOperand(1));
        uint32_t lval = Op1->getZExtValue();
        Instruction *NewInst = BinaryOperator::Create(
            Instruction::UDiv, Op0,
            ConstantInt::get(Op0->getType(), (1 << lval)));
        ReplaceInstWithInst((&I), NewInst);
      }

      // change add %a 1~4 -> call incr
      // change sub %a 1~4 -> call decr
      // for each type of operands -> different name of function should be
      // called for each constant being added, there are different numbers of
      // function calls

      if (I.getOpcode() == Instruction::Add ||
          I.getOpcode() == Instruction::Sub) {
        IRBuilder<> Builder(&I);
        Value *Op0 = I.getOperand(0);
        Value *Op1 = I.getOperand(1);
        ConstantInt *C = dyn_cast<ConstantInt>(Op1);

        // check if the const is 1 to 4
        if (C && (C->getValue() == 1 || C->getValue() == 2 ||
                  C->getValue() == 3 || C->getValue() == 4)) {
          LLVMContext &Ctx = I.getContext();
          FunctionType *FuncType;
          Module *M = I.getModule();
          FunctionCallee FC;

          // check the type of the operand
          if (I.getOpcode() == Instruction::Add ||
              I.getOpcode() == Instruction::Sub) {
            Type *intType = Op0->getType();
            Constant *zero = ConstantInt::get(intType, 0);
            std::string typeName;
            llvm::raw_string_ostream typeStream(typeName);
            intType->print(typeStream);
            FuncType = FunctionType::get(intType, {intType}, false);
            if (I.getOpcode() == Instruction::Add) {
              std::string funcName = "incr_" + typeName;
              FC = M->getOrInsertFunction(funcName, FuncType);
            } else {
              std::string funcName = "decr_" + typeName;
              FC = M->getOrInsertFunction(funcName, FuncType);
            }
          }

          // check the const
          if (C->getValue() == 1) {
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
          } else {
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
  return {LLVM_PLUGIN_API_VERSION, "ArithmeticPass", LLVM_VERSION_STRING,
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
}; // namespace sc::opt::arithmetic_pass