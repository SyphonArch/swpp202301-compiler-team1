#include "oracle_pass.h"

#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/Utils/CodeExtractor.h"
#include "llvm/ADT/Optional.h"

using namespace llvm;

#define DEBUG_TYPE "oracle-pass"

struct StoreGroup {
  BasicBlock *ParentBlock;         // Parent BasicBlock
  std::vector<StoreInst *> Stores; // Vector to hold consecutive StoreInst

  // Constructor
  StoreGroup(BasicBlock *BB, std::vector<StoreInst *> &Stores)
      : ParentBlock(BB), Stores(Stores) {}
};

struct StoreGroups {
  std::vector<StoreGroup> Groups; // Vector to hold all StoreGroup objects

  // Helper function to handle the stores in a temporary storage.
  void handleTempStores(BasicBlock *BB, std::vector<StoreInst *> &TempStores) {
    if (TempStores.size() > 1 && TempStores.size() <= 8) {
      Groups.push_back(StoreGroup(BB, TempStores));
    }
    TempStores.clear();
  }

  // Function to gather all StoreGroups in the Module
  void gatherGroups(Module &M) {
    for (auto &F : M) {
      if (F.getName() != "oracle") {
        gatherGroups(F);
      }
    }
  }

  // Function to gather all StoreGroups in a Function
  void gatherGroups(Function &F) {
    std::vector<StoreInst *> TempStores;
    for (auto &BB : F) {
      for (auto &I : BB) {
        if (auto *SI = dyn_cast<StoreInst>(&I)) {
          TempStores.push_back(SI);
        } else {
          handleTempStores(&BB, TempStores);
        }
      }
      handleTempStores(&BB, TempStores);
    }
  }

  void printGroups() const {
    for (const auto &group : Groups) {
      LLVM_DEBUG(dbgs() << "Group in Function: "
                        << group.ParentBlock->getParent()->getName()
                        << ", BasicBlock: " << group.ParentBlock->getName()
                        << "\n");
      for (const auto *SI : group.Stores) {
        LLVM_DEBUG(dbgs() << *SI << "\n");
      }
      LLVM_DEBUG(dbgs() << "\n");
    }
  }

  Optional<StoreGroup> getMaxSizeGroup() const {
    if (Groups.empty()) {
      return llvm::None;
    }

    return *std::max_element(Groups.begin(), Groups.end(),
                             [](const StoreGroup &a, const StoreGroup &b) {
                               return a.Stores.size() < b.Stores.size();
                             });
  }
};

namespace sc::opt::oracle_pass {
void outline(StoreGroup &group) {
  BasicBlock *BB = group.ParentBlock;
  IRBuilder<> Builder(BB->getContext());
  Builder.SetInsertPoint(group.Stores[0]);

  std::vector<Type *> ArgTypes;
  std::vector<std::string> ArgNames;

  for (StoreInst *SI : group.Stores) {
    ArgTypes.push_back(SI->getValueOperand()->getType());
    ArgTypes.push_back(SI->getPointerOperand()->getType());

    // Check if the value operand has a name
    if (SI->getValueOperand()->hasName())
      ArgNames.push_back(SI->getValueOperand()->getName().str());
    else
      ArgNames.push_back("");

    // Check if the pointer operand has a name
    if (SI->getPointerOperand()->hasName())
      ArgNames.push_back(SI->getPointerOperand()->getName().str());
    else
      ArgNames.push_back("");
  }

  FunctionType *FT = FunctionType::get(Builder.getInt64Ty(), ArgTypes, false);
  Function *NewFunc = Function::Create(FT, GlobalValue::ExternalLinkage,
                                       "oracle", BB->getModule());

  // Assign names to the arguments of the new function
  size_t i = 0;
  for (auto &Arg : NewFunc->args()) {
    Arg.setName(ArgNames[i]);
    i++;
  }

  Builder.CreateCall(NewFunc, [&]() {
    SmallVector<Value *, 8> Args;
    for (StoreInst *SI : group.Stores) {
      Args.push_back(SI->getValueOperand());
      Args.push_back(SI->getPointerOperand());
    }
    return Args;
  }());

  BasicBlock *NewBB = BasicBlock::Create(BB->getContext(), "entry", NewFunc);
  Builder.SetInsertPoint(NewBB);
  Function::arg_iterator AI = NewFunc->arg_begin();
  for (StoreInst *SI : group.Stores) {
    Value *ValToStore = &*AI++;
    Value *PtrToStoreInto = &*AI++;
    Builder.CreateStore(ValToStore, PtrToStoreInto);
    SI->eraseFromParent();
  }
  // Create return instruction with constant 0
  Builder.CreateRet(Builder.getInt64(0));
}

PreservedAnalyses OraclePass::run(Module &M, ModuleAnalysisManager &MPM) {
  StoreGroups storeGroups;
  storeGroups.gatherGroups(M);

  // Debug print
  storeGroups.printGroups();

  // Get the StoreGroup with the maximum size
  auto maxGroupOpt = storeGroups.getMaxSizeGroup();

  // Check if a maxGroup was found
  if (maxGroupOpt.hasValue()) {
    // Outline the maxGroup
    outline(*maxGroupOpt);
  } else {
    // Log or handle the case when no group was found
    LLVM_DEBUG(llvm::dbgs() << "No StoreGroup was found to outline\n");
  }

  return PreservedAnalyses::none();
};

extern "C" ::llvm::PassPluginLibraryInfo llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "OraclePass", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, ModulePassManager &MPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "oracle-pass") {
                    MPM.addPass(OraclePass());
                    return true;
                  }
                  return false;
                });
          }};
};
} // namespace sc::opt::oracle_pass