#include "oracle_pass.h"

#include "llvm/ADT/Optional.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/MemoryLocation.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/Debug.h"

using namespace llvm;

#define DEBUG_TYPE "oracle-pass"

struct StoreGroup {
  BasicBlock *ParentBlock;         // Parent BasicBlock
  std::vector<StoreInst *> Stores; // Vector to hold consecutive StoreInst

  // Default Constructor
  explicit StoreGroup(BasicBlock *BB) : ParentBlock(BB) {}

  // Constructor
  StoreGroup(BasicBlock *BB, std::vector<StoreInst *> &Stores)
      : ParentBlock(BB), Stores(Stores) {}
};

bool isSafeToMoveDownFromBack(StoreInst *SI, BasicBlock::reverse_iterator start,
                              BasicBlock::reverse_iterator end, AAResults &AA) {
  MemoryLocation StoreLoc = MemoryLocation::get(SI);
  for (auto I = end; I != start; I++) {
    if (I->mayReadFromMemory()) {

      Optional<MemoryLocation> LocOpt = MemoryLocation::getOrNone(&*I);
      if (!LocOpt.hasValue()) {
        // Possibly a call instruction: Might be unsafe.
        if (auto *CI = dyn_cast<CallInst>(&*I)) {
          if (!CI->getCalledFunction() ||
              CI->getCalledFunction()->isDeclaration()) {
            // The called function is either null (calling intrinsic) or it
            // doesn't have a definition in the current module. Skip alias
            // analysis for this instruction.
            continue;
          }
        }

        return false;
      }

      // If the memory locations may alias, it's not safe to move the store
      // instruction down
      MemoryLocation Loc = LocOpt.getValue();
      if (AA.alias(StoreLoc, Loc) != AliasResult::Kind::NoAlias) {
        return false;
      }
    }
  }
  return true;
}

struct StoreGroups {
  std::vector<StoreGroup> Groups; // Vector to hold all StoreGroup objects

  // Helper function to handle the stores in a temporary storage.
  void processAndSaveStoreGroup(StoreGroup &currentGroup) {
    // Max size of a group is set to 8.
    // So, number of pointers and values <= 16.
    // This is because backend crashes when [number of argument > 16].
    const size_t MaxSize = 8;
    size_t groupSize = currentGroup.Stores.size();

    // If group size is within the bounds, simply add it to the Groups
    if (groupSize > 1 && groupSize <= MaxSize) {
      Groups.push_back(currentGroup);
    } else if (groupSize > MaxSize) {
      // Split larger groups into chunks of MaxSize
      for (size_t i = 0; i < groupSize; i += MaxSize) {
        size_t end = std::min(i + MaxSize, groupSize);

        // Create a new vector for the chunk
        std::vector<StoreInst *> chunkStores;
        for (size_t j = i; j < end; ++j) {
          chunkStores.push_back(currentGroup.Stores[j]);
        }

        // Create a new StoreGroup for the chunk
        StoreGroup chunkGroup(currentGroup.ParentBlock, chunkStores);

        // Add the chunk to the Groups
        Groups.push_back(chunkGroup);
      }
    }
    currentGroup.Stores.clear();
  }

  // Function to gather all StoreGroups in the Module
  void gatherGroups(Module &M,
                    function_ref<AAResults &(Function &)> GetAAResults) {
    for (auto &F : M) {
      if (F.getName() != "oracle" && !F.isDeclaration()) {
        // Get an alias analysis instance
        AAResults &AA = GetAAResults(F);
        // Gather all StoreGroups in the Function
        gatherGroups(F, AA);
      }
    }
  }

  // Function to gather all StoreGroups in a Function
  void gatherGroups(Function &F, AAResults &AA) {
    for (BasicBlock &BB : F) {
      StoreGroup currentGroup(&BB);

      // Iterate over the instructions in reverse order, to reduce time
      // complexity
      for (BasicBlock::reverse_iterator I = BB.rbegin(), E = BB.rend(); I != E;
           ++I) {
        if (auto *SI = dyn_cast<StoreInst>(&*I)) {
          // Check if Stores are mergable.
          if (currentGroup.Stores.empty() ||
              isSafeToMoveDownFromBack(
                  SI, I, currentGroup.Stores[0]->getReverseIterator(), AA)) {
            currentGroup.Stores.push_back(SI);
          } else {
            processAndSaveStoreGroup(currentGroup);
            currentGroup.Stores.push_back(SI);
          }
        }
      }
      // Handle remaining currentGroups
      processAndSaveStoreGroup(currentGroup);
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

  assert(ArgTypes.size() <= 16 &&
         "Oracle should have number of arguments <= 16");

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

PreservedAnalyses OraclePass::run(Module &M, ModuleAnalysisManager &MAM) {
  auto &FAM = MAM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
  auto GetAAResults = [&](Function &F) -> AAResults & {
    return FAM.getResult<AAManager>(F);
  };

  StoreGroups storeGroups;
  storeGroups.gatherGroups(M, GetAAResults);

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