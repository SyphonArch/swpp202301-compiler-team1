#include "arithmetic_pass.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstrTypes.h"
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

      //new change! also change order of And instruction!
      if (I.getOpcode() == Instruction::Add || I.getOpcode() == Instruction::And) {
        Value *Op0 = I.getOperand(0);
        Value *Op1 = I.getOperand(1);
        auto *C0 = dyn_cast<ConstantInt>(Op0);
        auto *C1 = dyn_cast<ConstantInt>(Op1);
        if (C0 && !C1) {
          Instruction *NewInst;
          if(I.getOpcode() == Instruction::Add) {
            NewInst = BinaryOperator::Create(Instruction::Add, Op1, Op0);
          } else {
            NewInst = BinaryOperator::Create(Instruction::And, Op1, Op0);
          }
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
      if (I.getOpcode() == Instruction::Add ||
          I.getOpcode() == Instruction::Sub) {
        Value *Op0 = I.getOperand(0);
        Value *Op1 = I.getOperand(1);
        auto *C0 = dyn_cast<ConstantInt>(Op0);
        auto *C1 = dyn_cast<ConstantInt>(Op1);
        // getValue() cannot be compared, only == is allowed
        if (!C0 && C1 &&
            ((C1->getSExtValue() == -1) || (C1->getSExtValue() == -2) ||
             (C1->getSExtValue() == -3) || (C1->getSExtValue() == -4))) {
          Instruction *NewInst;
          Instruction::BinaryOps target_op;
          if (I.getOpcode() == Instruction::Add) {
            target_op = Instruction::Sub;
          } else {
            target_op = Instruction::Add;
          }
          NewInst = BinaryOperator::Create(
              target_op, Op0,
              ConstantInt::get(Op0->getType(), -C1->getSExtValue()));
          ReplaceInstWithInst((&I), NewInst);
        }
      }
    }
  }

  for (BasicBlock &BB : F) {
    for (auto i = BB.begin(), en = BB.end(); i != en;) {
      auto temp = i++;
      Instruction &I = *temp;

      if (!(I.getOpcode() == Instruction::Add ||
            I.getOpcode() == Instruction::Sub ||
            I.getOpcode() == Instruction::Shl ||
            I.getOpcode() == Instruction::AShr ||
            I.getOpcode() == Instruction::LShr ||
            I.getOpcode() == Instruction::And)) {
        continue;
      }

      Value *Op0 = I.getOperand(0);
      Value *Op1 = I.getOperand(1);
      auto *C0 = dyn_cast<ConstantInt>(Op0);
      auto *C1 = dyn_cast<ConstantInt>(Op1);
      uint32_t lval;
      if (C1) {
        lval = C1->getZExtValue();
      }
      Instruction *NewInst = nullptr;

      // change add %a 0 -> mul %a 1
      if (I.getOpcode() == Instruction::Add) {
        if (!C0 && C1 && (C1->getValue() == 0)) {
          NewInst = BinaryOperator::Create(
              Instruction::Mul, Op0, ConstantInt::get(Op0->getType(), 1));
        }
      }

      // change add %a %a -> mul %a 2
      if (I.getOpcode() == Instruction::Add) {
        if (!C1 && !C0 && Op0 == Op1) { // Check if operands are equal
          NewInst = BinaryOperator::Create(Instruction::Mul, Op0,
                                           ConstantInt::get(Op0->getType(), 2));
        }
      }

      // change sub 0 %a -> mul %a -1
      if (I.getOpcode() == Instruction::Sub) {
        if (!C1 && C0 && (C0->getValue() == 0)) {
          NewInst = BinaryOperator::Create(
              Instruction::Mul, Op1, ConstantInt::get(Op1->getType(), -1));
        }
      }

      // change and %x 2^c-1 -> urem %x (2^c)
      if (I.getOpcode() == Instruction::And) {
        if (!C0 && C1) {
          if(lval >= 0) {
            uint32_t lvalp1 = lval + 1;
            bool is_lval_power_of_2 = false;
            while(1) {
              if(lvalp1 == 1 || lvalp1 == 0) {
                is_lval_power_of_2 = true;
                break;
              } 
              if(lvalp1 % 2 == 1) {
                is_lval_power_of_2 = false;
                break;
              }
              lvalp1 /= 2;
            }
            if(is_lval_power_of_2) {
              NewInst = BinaryOperator::Create(
                  Instruction::URem, Op0,
                  ConstantInt::get(Op0->getType(), (lval + 1)));
            }
          }
        }
      }

      // change shl %x c -> mul %x (2^c)
      if (I.getOpcode() == Instruction::Shl) {
        if (!C0 && C1) {
          NewInst = BinaryOperator::Create(
              Instruction::Mul, Op0,
              ConstantInt::get(Op0->getType(), (1 << lval)));
        }
      }

      // change ashr %x c -> sdiv %x (2^c)
      if (I.getOpcode() == Instruction::AShr) {
        if (!C0 && C1) {
          NewInst = BinaryOperator::Create(
              Instruction::SDiv, Op0,
              ConstantInt::get(Op0->getType(), (1 << lval)));
        }
      }

      // change lshr %x c -> udiv %x (2^c)
      if (I.getOpcode() == Instruction::LShr) {
        if (!C0 && C1) {
          NewInst = BinaryOperator::Create(
              Instruction::UDiv, Op0,
              ConstantInt::get(Op0->getType(), (1 << lval)));
        }
      }

      if (NewInst) {
        ReplaceInstWithInst((&I), NewInst);
        continue;
      }

      // change add %a 1~4 -> call incr
      // change sub %a 1~4 -> call decr
      // for each type of operands -> different name of function should be
      // called for each constant being added, there are different numbers of
      // function calls

      if (I.getOpcode() == Instruction::Add ||
          I.getOpcode() == Instruction::Sub) {
        IRBuilder<> Builder(&I);

        // check if the const is 1 to 4
        if (!C0 && C1 &&
            (C1->getValue() == 1 || C1->getValue() == 2 ||
             C1->getValue() == 3 || C1->getValue() == 4)) {
          LLVMContext &Ctx = I.getContext();
          FunctionType *FuncType;
          Module *M = I.getModule();
          FunctionCallee FC;

          // check the type of the operand
          Type *intType = Op0->getType();
          std::string typeName;
          llvm::raw_string_ostream typeStream(typeName);
          intType->print(typeStream);
          FuncType = FunctionType::get(intType, {intType}, false);
          std::string funcName;
          if (I.getOpcode() == Instruction::Add) {
            funcName = "incr_" + typeName;
          } else {
            funcName = "decr_" + typeName;
          }
          FC = M->getOrInsertFunction(funcName, FuncType);

          // check the const
          Value *Arg = Op0;
          for (int idx = 0; idx < C1->getSExtValue(); ++idx) {
            Arg = Builder.CreateCall(FC, Arg);
          }
          I.replaceAllUsesWith(Arg);
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
} // namespace sc::opt::arithmetic_pass

