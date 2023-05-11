#include "gep_eliminate.h"

#include "llvm/IR/Instructions.h"
#include "llvm/IR/Operator.h"
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
using namespace std::string_literals;


namespace sc::opt::gep_elim {
PreservedAnalyses GEPEliminatePass::run(llvm::Module &M, llvm::ModuleAnalysisManager &MAM) {
  llvm::IntegerType *Int64Ty = llvm::Type::getInt64Ty(M.getContext());
  std::set<llvm::GetElementPtrInst *> trashBin;
  
  for (llvm::Function &F : M) {
    trashBin.clear();
    for (llvm::BasicBlock &BB : F)
      for (llvm::Instruction &I : BB)
        if (llvm::GetElementPtrInst *GEPI =
                llvm::dyn_cast<llvm::GetElementPtrInst>(&I)) {
          
          llvm::Value *ptrOp = GEPI->getPointerOperand();
          llvm::Type *curr = ptrOp->getType();
          curr = curr->getPointerElementType();

          llvm::Instruction *pti =
              llvm::CastInst::CreateBitOrPointerCast(ptrOp, Int64Ty, "", GEPI);

          std::vector<llvm::Instruction *> v;
          v.push_back(pti);

          int ck = 0;

          for (auto opIt = GEPI->idx_begin(); opIt != GEPI->idx_end(); ++opIt) {
            llvm::Value *op = *opIt;
            
            uint64_t size;
            if (llvm::isa<llvm::PointerType>(curr)) {
              size = 8UL;
            } else if (llvm::isa<llvm::IntegerType>(curr)) {
              switch (curr->getIntegerBitWidth()) {
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
                ck = 1;
                break;
              } 
              if(ck == 1) break;
            } else {
                ck = 1;
                break;
            }

//            const auto sizeconst = size;
            llvm::Instruction *mul = llvm::BinaryOperator::CreateMul(
                op, llvm::ConstantInt::get(Int64Ty, 8UL, true), "", GEPI);
/*
            llvm::Instruction *add =
                llvm::BinaryOperator::CreateAdd(v.back(), mul, "", GEPI);
            v.push_back(add);
*/
          }

          if(ck == 1) continue;
          
          llvm::Instruction *itp = llvm::CastInst::CreateBitOrPointerCast(
              v.back(), I.getType(), "", GEPI);
          GEPI->replaceAllUsesWith(itp);
          trashBin.insert(GEPI);
          
        }
    for (llvm::GetElementPtrInst *I : trashBin)
      I->eraseFromParent();
  }
  
  return llvm::PreservedAnalyses::all();
}


extern "C" ::llvm::PassPluginLibraryInfo llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "GEPEliminatePass", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, ModulePassManager &MPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "gep_elim") {
                    MPM.addPass(GEPEliminatePass());
                    return true;
                  }
                  return false;
                });
          }};
}
}; 
