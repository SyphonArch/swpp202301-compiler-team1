#include "llvm/IR/Instructions.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/InstrTypes.h"

using namespace llvm;

//cost reduced by aload, oracle is not implemented

//set option = 0 for higher cost in
//  conditional branch (true branch)
//  load, store (heaps)
int getStrangeCost(Instruction* I, int option = -1){
    unsigned Opcode = I->getOpcode();

    switch(Opcode){
        case Instruction::Ret:
            return 1;

        case Instruction::Br:
            return option? 1: 6;

        case Instruction::Switch:
            return 4;

        case Instruction::Load:
        case Instruction::Store:
            return option? 20: 30;

        case Instruction::UDiv:
        case Instruction::SDiv:
        case Instruction::URem:
        case Instruction::SRem:
        case Instruction::Mul:
            return 1;

        case Instruction::Shl:
        case Instruction::LShr:
        case Instruction::AShr:
        case Instruction::And:
        case Instruction::Or:
        case Instruction::Xor:
            return 4;

        case Instruction::Add:
        case Instruction::Sub:
            return 5;

        case Instruction::Select:
            return 1;

        case Instruction::ICmp:
            return 1;

        case Instruction::Call:
            if(auto* call = llvm::dyn_cast<llvm::CallInst>(I)){
                llvm::Function* fun = call->getCalledFunction();
                if(!fun) {
                    //null function; should be unreachable
                    return -1;
                }
                StringRef name = fun->getName();
                int argNum = fun->getNumOperands();
                if(name == "aload"){
                    return option? 24: 34;
                }
                else if(name == "sum"){
                    return 10;
                }
                else if(name == "incr" || name == "decr"){
                    return 1;
                }
                else if(name == "llvm.assume"){
                    return 0;
                }
                else if(name == "oracle"){
                    return 40;
                }
                else{
                    return 2 + argNum;
                }
            }
            else {
                //not a function call; is unreachable
                return -1;
            }

        default:
        //invalid instruction; should be unreachable
            return -1;
    }
}