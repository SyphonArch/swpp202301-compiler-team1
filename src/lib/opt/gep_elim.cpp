#include "gep_elim.h"

#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Operator.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"
#include "llvm/Pass.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include <vector>

using namespace llvm;
using namespace std;
using namespace std::string_literals;

namespace sc::opt::gep_elim {

PreservedAnalyses GEPEliminatePass::run(Function &F,
                                        FunctionAnalysisManager &FAM) {
  llvm::IntegerType *Int64Ty = llvm::Type::getInt64Ty(F.getContext());

  // we dont change anything when the function name is oracle
  // oracle function has line conditions, so we cannot change the number of
  // lines in that function in the backend, getelementptr is changed after
  // counting oracle instruction numbers

  StringRef functionName = F.getName(); // Get the name of the function
  if (functionName.equals("oracle")) {
    return PreservedAnalyses::all();
  }

  std::set<llvm::GetElementPtrInst *> trashBin;

  for (llvm::BasicBlock &BB : F) {
    for (llvm::Instruction &I : BB) {
      if (llvm::GetElementPtrInst *gepi =
              llvm::dyn_cast<llvm::GetElementPtrInst>(&I)) {

        llvm::Value *ptrOp = gepi->getPointerOperand();

        // if the number of index operands is bigger than 1, we cannot optimize
        unsigned int numIndices = gepi->getNumIndices();
        if (numIndices > 1)
          continue;

        llvm::Type *ptrElemType = ptrOp->getType()->getPointerElementType();

        // a vector that will keep changed instructions
        std::vector<llvm::Instruction *> v;

        // op is the index operand of Getelementptr instruction
        auto opIt = gepi->getOperand(1);
        llvm::Value *IdxOp = opIt;

        // change ck to 1 if we cannot optimize
        bool cannotOptimize = false;

        // get the size of the pointer operand
        uint64_t size;
        if (llvm::isa<llvm::PointerType>(ptrElemType)) {
          size = 8UL;
        } else if (llvm::isa<llvm::IntegerType>(ptrElemType)) {
          switch (ptrElemType->getIntegerBitWidth()) {
          case 1:
          case 8:
            size = 1UL;
            break;
          case 16:
            size = 2UL;
            break;
          case 32:
            size = 4UL;
            break;
          case 64:
            size = 8UL;
            break;
          default:
            cannotOptimize = true;
            break;
          }
          if (cannotOptimize)
            continue;
        } else {
          // if the type that we want to load is not pointer nor integer, we
          // cannot optimize
          continue;
        }

        // we check whether this getelement instruction can be optimized
        // only adding 0 to 3 const can be optimized
        // we first change getelement instruction using add, but later
        // arithmetic pass will change add into incr

        // for our benchmark programs, idx value only has type Int64
        // but in reality, there may be wrong inputs, so checking is needed to
        // make sure the idx value is Int64 type

        if (IdxOp->getType() ==
            llvm::ConstantInt::get(Int64Ty, size, true)->getType()) {
          if (llvm::ConstantInt *constInt =
                  llvm::dyn_cast<llvm::ConstantInt>(IdxOp)) {

            // if const value is not 0 to 4 -> cannot optimize
            uint64_t const_val = constInt->getZExtValue();
            if (const_val >= 4 || const_val < 0)
              continue;

            // make a ptrtoint instruction
            llvm::Instruction *pti = llvm::CastInst::CreateBitOrPointerCast(
                ptrOp, Int64Ty, "", gepi);

            v.push_back(pti);

            if (constInt->equalsInt(0)) {
              // do nothing
              // but it is actually optimizing by eliminating useless
              // getelementptr instruction

            } else if (constInt->equalsInt(1) || constInt->equalsInt(2) ||
                       constInt->equalsInt(3)) {

              // only add of constant 1, 2, 3 can be optimized
              // or else the change will increase cost

              uint64_t const_val = constInt->getZExtValue();

              // if size == 1, or i8 type, we dont have to use div and mul
              // because it will div and mul by 1
              if (size != 1) {
                llvm::Instruction *div = llvm::BinaryOperator::CreateUDiv(
                    v.back(), llvm::ConstantInt::get(Int64Ty, size, false), "",
                    gepi);
                llvm::Instruction *add = llvm::BinaryOperator::CreateAdd(
                    div, ConstantInt::get(div->getType(), const_val), "", gepi);
                llvm::Instruction *mul = llvm::BinaryOperator::CreateMul(
                    add, llvm::ConstantInt::get(Int64Ty, size, false), "",
                    gepi);
                v.push_back(mul);
              } else {
                llvm::Instruction *add = llvm::BinaryOperator::CreateAdd(
                    v.back(), ConstantInt::get(v.back()->getType(), const_val),
                    "", gepi);
                v.push_back(add);
              }
            }
          } else
            continue;
        } else
          continue;

        // make a inttoptr instruction
        llvm::Instruction *itp = llvm::CastInst::CreateBitOrPointerCast(
            v.back(), I.getType(), "", gepi);

        // we dont erase the origial instruction in the first loop. put it in a
        // trashcan and in a later loop, we erese it
        gepi->replaceAllUsesWith(itp);
        trashBin.insert(gepi);
      }
    }
  }
  for (llvm::GetElementPtrInst *I : trashBin)
    I->eraseFromParent();

  return llvm::PreservedAnalyses::all();
}

extern "C" ::llvm::PassPluginLibraryInfo llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "GEPEliminatePass", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "gep-elim") {
                    FPM.addPass(GEPEliminatePass());
                    return true;
                  }
                  return false;
                });
          }};
}
}; // namespace sc::opt::gep_elim

/*
              llvm::Instruction *mul = llvm::BinaryOperator::CreateMul(
                  op, llvm::ConstantInt::get(Int64Ty, size, true), "", GEPI);
              llvm::Instruction *add =
                  llvm::BinaryOperator::CreateAdd(v.back(), mul, "", GEPI);
              v.push_back(add);
*/

// below here is unfinished!
// it will go through instructions and find chains that should be copied and
// make a new variable

/*
static bool isInductionVariable(PHINode &PHI) {
  // Check if the PHI node has two incoming values, one of which is a constant
  // increment/decrement
  if (PHI.getNumIncomingValues() == 2) {
    Value *IncValue = nullptr;
    Value *OtherValue = nullptr;
    for (unsigned i = 0; i < 2; ++i) {
      Value *IncomingValue = PHI.getIncomingValue(i);
      if (isa<ConstantInt>(IncomingValue))
        IncValue = IncomingValue;
      else
        OtherValue = IncomingValue;
    }
    if (IncValue && OtherValue) {
      // Verify that the other incoming value is the PHI node itself
      return PHI.getOperand(0) == OtherValue || PHI.getOperand(1) == OtherValue;
    }
  }
  return false;
}
*/

/*
  // Perform loop analysis-based optimizations here
  LoopInfo &LI = FAM.getResult<LoopAnalysis>(F);

  //two whole iteration needed
  //first we have to loop all over and look for PHI nodes that are possible
  //second we loop again, and for the possible phi nodes, optimize
  std::vector<Instruction *> vec;
  // look for phi node
  for (Function::iterator BB = F.begin(), E = F.end(); BB != E; ++BB) {
    for (BasicBlock::iterator I = BB->begin(), E = BB->end(); I != E; ++I) {
      Instruction *inst = &*I;
      if (PHINode *phiNode = dyn_cast<PHINode>(inst)) {
        Instruction *chain = phiNode;
        bool is_first_getele = true;
        Instruction *J;
        bool is_opti_possible = true;
        for (Function::iterator BBB = BB, E = F.end(); BBB != E; ++BBB) {
          for (BasicBlock::iterator II = BBB->begin(), E = BBB->end(); II !=
  E;++II) { for (Use &operand : II->operands()) { Value *value = operand.get();
              if (Instruction *operandInst = dyn_cast<Instruction>(value)) {
                if (operandInst == chain) {
                  if (GetElementPtrInst *GEPI =
                          llvm::dyn_cast<llvm::GetElementPtrInst>(&II)) {
                    continue;
                  }
                  CallInst *callInst = dyn_cast<CallInst>(&II);
                  Function *calledFunction = callInst->getCalledFunction();
                  if (calledFunction && calledFunction->getName() == "incr_i64")
  { continue;
                  }
                  is_opti_possible = false;
                  break;
                }
              }
            }
            if(!is_opti_possible) break;
            if (GetElementPtrInst *GEPI =
                    llvm::dyn_cast<llvm::GetElementPtrInst>(&II)) {
              unsigned numOp = GEPI->getNumOperands();
              Value *idxOp;
              if (numOp >= 2) {
                // retrive i
                idxOp = GEPI->getOperand(1);
              } else {
                // if there are more than 1 index, just return, dont now how to
  deal with these continue;
              }
              if (idxOp != chain)
                continue;

              // found getele in chain
              if (is_first_getele) {
                J = GEPI;
              } else {
                // this is not the first getelement that uses i

              }
            }
            CallInst *callInst = dyn_cast<CallInst>(&II);
            Function *calledFunction = callInst->getCalledFunction();
            if (calledFunction && calledFunction->getName() == "incr_i64") {

            }
          }
          if(!is_opti_possible) break;
        }

        if(is_opti_possible) {
          vec.push_back(phiNode);
        }
      }
    }
  }
*/
// test for loop analysis
/*
for (Loop *L : LI) {
  BasicBlock *Header = L->getHeader();
  for (auto &PHI : Header->phis()) {
    if (isInductionVariable(PHI)) {
      llvm::errs() << "Induction variable found: " << PHI << "\n";
    }
  }
}
*/

/*
  //look for getelim instruction
  for (BasicBlock &BB : F) {
    for (Instruction &I : BB) {
      if (GetElementPtrInst *GEPI =
              llvm::dyn_cast<llvm::GetElementPtrInst>(&I)) {

        // if a getelip instruction is found, look for the instructions that
  uses the second operand of gep as operand for add Value *ptrOp =
  GEPI->getPointerOperand(); Type *curr = ptrOp->getType(); curr =
  curr->getPointerElementType();
        //curr is the type of pointer operand

        unsigned numOp = GEPI->getNumOperands();
        Value *idxOp;
        if(numOp >= 2) {
          //retrive i
          idxOp = GEPI->getOperand(1);
        } else {
          //if there are more than 1 index, just return, dont now how to deal
  with these continue;
        }

        //so we found operand i that is used in getele
        //look for instruction that uses idx as operand
        //if the chain is longer than 2, we ignore
        int chain_len = 0;
        int cannot_change = 0;

        for (Instruction &II : BB) {
          for (Use &operand : II.operands()) {

            // find instruction that uses idxOp
            if (operand.get() == idxOp) {
              if(chain_len != 0) {
                //the length of the chain is longer than 1
                //cannot change
                cannot_change = 1;
                break;
              }

              // we only look for II that is incr function call, and that incr
  should be i64 type if (CallInst *callInst = dyn_cast<CallInst>(&II)) {
                Function *calledFunction = callInst->getCalledFunction();
                if (calledFunction && calledFunction->getName() == "incr_i64")
  { chain_len++; idxOp = calledFunction; } else { cannot_change = 1; break;
                }
              } else {
                //i is not used in incr, but it is used differently
                //cannot change this code
                cannot_change = 1;
                break;
              }
            }
          }
          if(cannot_change == 1) break;
        }
        if(cannot_change == 1) continue;

        //in the running program comes here, it means the chain has length 1
  and can be optimized

        for (Instruction &II : BB) {
          for (Use &operand : II.operands()) {
            if (operand.get() == idxOp) {

              // we make a new variable j that uses
              if (CallInst *callInst = dyn_cast<CallInst>(&II)) {
                Function *calledFunction = callInst->getCalledFunction();

              }
            }
          }
        }
      }
    }
  }
*/
// deleting get element ptr
// only availble for int and point types

// now look for the values used in
/*
for (llvm::Function &F : M) {
  trashBin.clear();
  for (llvm::BasicBlock &BB : F) {
    std::vector<PtrToIntInst *> vec;
    for (llvm::Instruction &I : BB) {
      if (auto *ptrtoint = dyn_cast<PtrToIntInst>(&I)) {
        //          Value *v = ptrtoint->getOperand(0);
        vec.push_back(ptrtoint);
      }
    }
    std::vector<int> visit(vec.size(), 0);
    for (size_t i = 0; i < vec.size(); i++) {

      if (visit[i] == 1)
        continue;
      visit[i] = 1;
      PtrToIntInst *v1 = vec[i];

      // this variable checks whether if there is a chain that makes v1 +
      // whether the chain is valid or not the chain can be invaild for
      // following reasons
      // 1. the chain has other operation that is not add
      // 1. the chain length is longer than 2
      // 2. the chain has add operation that is bigger than 3

      int is_there_chain_for_v1 = 0;

      // this variable is used to check the length of the chain
      int incr_value = 0;

      for (size_t j = 0; j < vec.size(); j++) {
        PtrToIntInst *v2 = vec[j];
        if (v1->getOperand(0) == v2->getOperand(0)) {
          // they are looking at the same operands
          // and v1 will be the start of it
          // in this case, start from v1 and go backward
          BasicBlock *BB_of_v1 = v1->getParent();

          // this value checks whether the instruction os befor v1, these are
          // the instructions that might be in the chain that return operand
          // of v1
          int ck_if_I_is_before_v1 = 0;

          // save operand of v1
          Value *v1_operand = v1->getOperand(0);
          StringRef operand_name = v1_operand->getName();

          // we look backwards
          for (BasicBlock::reverse_iterator rit = BB_of_v1->rbegin(),
                                            ritEnd = BB_of_v1->rend();
               rit != ritEnd; ++rit) {
            Instruction &I = *rit;
            if (dyn_cast<PtrToIntInst>(&I) != v1 &&
                ck_if_I_is_before_v1 == 0) {
              continue;
            }
            ck_if_I_is_before_v1 = 1;

            // we are at the instruction that we are looking for
            if (I.getName() == operand_name) {
              // if I is the instruction that v1 needs
              if (I.getOpcode() == Instruction::Add) {
                // the opreand came from add!
                Value *Op0 = I.getOperand(0);
                Value *Op1 = I.getOperand(1);
                auto *C0 = dyn_cast<ConstantInt>(Op0);
                auto *C1 = dyn_cast<ConstantInt>(Op1);
                if (!C0 && C1) {
                  if (C1->getValue() == 0) {

                  } else if (C1->getValue() == 1) {
                    if (incr_value == 0) {
                      // if it is the first chian instruction, it can be added
                      incr_value++;

                      // update operand_name to the operand of this chain
                      // operand
                      operand_name = Op0->getName();


                      //should be a code here that add instructions below the
                      // original add instruction
                      //1. makes new instruction that div new-made chain
                      // varible by its type size. /4 /8 etc
                      //2. makes new instruction that added new-made chain
                      // varible 1
                      //3. makes new instruction that mul new-made chain
                      // varible by its type size. /4 /8 etc


                    } else {
                      // if there was already a chain that added bigger number
                      // than zero, the chain should not be optimized
                      is_there_chain_for_v1 = 1;
                      break;
                    }
                  } else if (C1->getValue() == 2) {
                    if (incr_value == 0) {
                      incr_value += 2;
                      operand_name = Op0->getName();


                    } else {
                      is_there_chain_for_v1 = 1;
                      break;
                    }
                  } else {
                    // cannot make a chain, too big...
                    // just adding the value will be more efficient
                    // because even though we are going to /8 and *8, there
                    // should be adding operations between them for example if
                    // ther is add 3 in the chain for i64 variable div 8,
                    // incr, incr, incr , mul8 is already 8 instructions,
                    // which is the same cost as adding
                    is_there_chain_for_v1 = 1;
                    break;
                  }
                }
              } else {
                // the chain cannot be held... there is an operation that is
                // not add
                is_there_chain_for_v1 = 1;
                break;
              }
            }
          }

          // if for loop ended with no problem, it means a vaild chain exists
          // we might have to the instructions into a vec form, and do batch
          // processing...
        }

        // v2 is now checked
        visit[j] = 1;
        if (is_there_chain_for_v1 == 1)
          break;
      }
    }
  }
}
*/

/*******plz ignore below*********


  for (llvm::Function &F : M) {
    for (Function::iterator bb = F.begin(), e = F.end(); bb != e; ++bb) {

        if (auto *loop = dyn_cast<Loop>(bb)) {

            BasicBlock *header = loop->getHeader();
            for (auto it = header->begin(); it != header->end(); ++it) {
                if (PHINode *phi = dyn_cast<PHINode>(&*it)) {
                    PHINode *newPhi = PHINode::Create(phi->getType(),
  phi->getNumIncomingValues(), "", &*phi); for (unsigned i = 0; i <
  phi->getNumIncomingValues(); ++i) {
                        newPhi->addIncoming(phi->getIncomingValue(i),
  phi->getIncomingBlock(i));
                    }
                }
            }
        }
    }
  }


      auto &MSSA = getAnalysis<MemorySSAWrapperPass>().getMSSA();

     // Get pointers to the variables we want to check
     auto *I1 = dyn_cast<Instruction>(F.getEntryBlock().getFirstNonPHI());
     auto *I2 = dyn_cast<Instruction>(F.getEntryBlock().getTerminator());

     // Get the memory locations accessed by each variable
     auto *Loc1 = MSSA.getMemoryAccess(I1)->getMemoryLocation();
     auto *Loc2 = MSSA.getMemoryAccess(I2)->getMemoryLocation();

     // Check if the memory locations are the same
     bool Aliased = Loc1 == Loc2;

********************************************/
