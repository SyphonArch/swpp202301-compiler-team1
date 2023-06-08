//===- LoopExtractor2.cpp - Extract each loop into a new function
//----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// A pass wrapper around the ExtractLoop() scalar transformation to extract each
// top-level loop into its own new function. If the loop is the ONLY loop in a
// given function, it is not touched. This is a pass most useful for debugging
// via bugpoint.
//
//===----------------------------------------------------------------------===//

#include "./loop_extractor.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/AssumptionCache.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/Utils.h"
#include "llvm/Transforms/Utils/CodeExtractor.h"

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
#define DEBUG_TYPE "loop-extract"

STATISTIC(NumExtracted, "Number of loops extracted");

namespace sc::opt::loop_extractor {
struct LoopExtractor2 {
  explicit LoopExtractor2(
      unsigned NumLoops,
      function_ref<DominatorTree &(Function &)> LookupDomTree,
      function_ref<LoopInfo &(Function &)> LookupLoopInfo,
      function_ref<AssumptionCache *(Function &)> LookupAssumptionCache)
      : NumLoops(NumLoops), LookupDomTree(LookupDomTree),
        LookupLoopInfo(LookupLoopInfo),
        LookupAssumptionCache(LookupAssumptionCache) {}
  bool runOnModule(Module &M);
  bool CanConvertToOracle(Function &F);

private:
  // The number of natural loops to extract from the program into functions.
  unsigned NumLoops;

  function_ref<DominatorTree &(Function &)> LookupDomTree;
  function_ref<LoopInfo &(Function &)> LookupLoopInfo;
  function_ref<AssumptionCache *(Function &)> LookupAssumptionCache;

  bool runOnFunction(Function &F);

  bool extractLoops(Loop::iterator From, Loop::iterator To, LoopInfo &LI,
                    DominatorTree &DT);
  bool extractLoop(Loop *L, LoopInfo &LI, DominatorTree &DT);
};

bool LoopExtractor2::runOnModule(Module &M) {
  if (M.empty())
    return false;

  if (!NumLoops)
    return false;

  bool Changed = false;

  // The end of the function list may change (new functions will be added at the
  // end), so we run from the first to the current last.
  auto I = M.begin(), E = --M.end();
  while (true) {
    Function &F = *I;

    Changed |= runOnFunction(F);

    if (!NumLoops)
      break;

    // If this is the last function.
    if (I == E)
      break;

    ++I;
  }
  return Changed;
}

bool LoopExtractor2::runOnFunction(Function &F) {
  // Do not modify `optnone` functions.
  if (F.hasOptNone())
    return false;

  if (F.empty())
    return false;

  bool Changed = false;
  LoopInfo &LI = LookupLoopInfo(F);

  // If there are no loops in the function.
  if (LI.empty())
    return Changed;

  DominatorTree &DT = LookupDomTree(F);

  // If there is more than one top-level loop in this function, extract all of
  // the loops.
  if (std::next(LI.begin()) != LI.end())
    return Changed | extractLoops(LI.begin(), LI.end(), LI, DT);

  // Otherwise there is exactly one top-level loop.
  Loop *TLL = *LI.begin();

  // If the loop is in LoopSimplify form, then extract it only if this function
  // is more than a minimal wrapper around the loop.
  if (TLL->isLoopSimplifyForm()) {
    bool ShouldExtractLoop = false;

    // Extract the loop if the entry block doesn't branch to the loop header.
    Instruction *EntryTI = F.getEntryBlock().getTerminator();
    if (!isa<BranchInst>(EntryTI) ||
        !cast<BranchInst>(EntryTI)->isUnconditional() ||
        EntryTI->getSuccessor(0) != TLL->getHeader()) {
      ShouldExtractLoop = true;
    } else {
      // Check to see if any exits from the loop are more than just return
      // blocks.
      SmallVector<BasicBlock *, 8> ExitBlocks;
      TLL->getExitBlocks(ExitBlocks);
      for (auto *ExitBlock : ExitBlocks)
        if (!isa<ReturnInst>(ExitBlock->getTerminator())) {
          ShouldExtractLoop = true;
          break;
        }
    }
    LLVM_DEBUG(outs() << "One Loop: " << *TLL << "\n"
                      << "from Function "
                      << TLL->getHeader()->getParent()->getName() << "\n");

    if (ShouldExtractLoop)
      return Changed | extractLoop(TLL, LI, DT);
  }

  // Okay, this function is a minimal container around the specified loop.
  // If we extract the loop, we will continue to just keep extracting it
  // infinitely... so don't extract it. However, if the loop contains any
  // sub-loops, extract them.
  return Changed | extractLoops(TLL->begin(), TLL->end(), LI, DT);
}

bool LoopExtractor2::extractLoops(Loop::iterator From, Loop::iterator To,
                                  LoopInfo &LI, DominatorTree &DT) {
  bool Changed = false;
  SmallVector<Loop *, 8> Loops;

  // Save the list of loops, as it may change.
  Loops.assign(From, To);
  for (Loop *L : Loops) {
    // If LoopSimplify form is not available, stay out of trouble.
    if (!L->isLoopSimplifyForm())
      continue;

    Changed |= extractLoop(L, LI, DT);
    if (!NumLoops)
      break;
  }
  return Changed;
}

bool LoopExtractor2::extractLoop(Loop *L, LoopInfo &LI, DominatorTree &DT) {
  assert(NumLoops != 0);

  bool ShouldExtractLoop = true;

  // Added here
  size_t loop_size = 0;
  for (auto loop_block : L->blocks()) {
    loop_size += loop_block->size();
  }

  // If the loop do not have load or store instructions, don't extract it.
  bool has_load_and_store = false;
  for (auto loop_block : L->blocks()) {
    for (auto &I : *loop_block) {
      if (isa<LoadInst>(I) || isa<StoreInst>(I)) {
        // if there is no declaration of intrinsic function, it will be
        has_load_and_store = true;
        break;
      }
    }
  }

  bool has_call = false;
  for (auto loop_block : L->blocks()) {
    for (auto &I : *loop_block) {
      if (isa<CallInst>(I)) {
        CallInst *CI = dyn_cast<CallInst>(&I);
        Function *called_func = CI->getCalledFunction();

        // check if called_func has no Declaration, or it has the name
        // as "read" or "write".
        if ((called_func->getName() == "read" ||
             called_func->getName() == "write" ||
             called_func->getName() == "malloc" ||
             called_func->getName() == "free") ||
            (called_func && !called_func->isDeclaration()))
          has_call = true;
      }
    }
  }

  if (loop_size > 47 || !has_load_and_store || has_call) {
    ShouldExtractLoop = false;
  }

  LLVM_DEBUG(outs() << loop_size << " size, load/store/call: "
                    << has_load_and_store << ' ' << has_call << '\n');

  if (!ShouldExtractLoop)
    return false;

  Function &Func = *L->getHeader()->getParent();
  AssumptionCache *AC = LookupAssumptionCache(Func);
  CodeExtractorAnalysisCache CEAC(Func);
  CodeExtractor Extractor(DT, *L, false, nullptr, nullptr, AC, "extracted");
  if (Extractor.extractCodeRegion(CEAC)) {
    LI.erase(L);
    --NumLoops;
    ++NumExtracted;
    return true;
  }
  return false;
}

bool LoopExtractor2::CanConvertToOracle(Function &F) {
  LLVM_DEBUG(outs() << "\tInvestigating function " << F.getName() << "\n");
  LLVM_DEBUG(outs() << F << "\n");
  if (F.hasOptNone())
    return false;

  if (F.empty())
    return false;

  if (F.isDeclaration())
    return false;

  LoopInfo &LI = LookupLoopInfo(F);

  LLVM_DEBUG(outs() << "Function " << F.getName() << " has " << (!LI.empty())
                    << " loops\n");
  // If there are no loops in the function.
  if (LI.empty())
    return false;

  DominatorTree &DT = LookupDomTree(F);
  bool ShouldExtractLoop = true;

  auto numI = 0;
  for (BasicBlock &BB : F) {
    for (auto it = BB.begin(); it != BB.end(); ++it) {
      numI++;
    }
  }

  // crash if oracle contains more than 50 LLVM IR instructions
  // function calls and aload instructions are checked by the
  // swpp-interpreter
  if (numI > 50)
    return false;

  // If the loop do not have load or store instructions, don't extract it.
  bool has_load_and_store = false;
  for (auto &loop_block : F) {
    for (auto &I : loop_block) {
      if (isa<LoadInst>(I) || isa<StoreInst>(I)) {
        has_load_and_store = true;
        break;
      }
    }
  }

  if (!has_load_and_store)
    return false;

  bool has_call = false;
  for (auto &loop_block : F) {
    for (auto &I : loop_block) {
      if (isa<CallInst>(I)) {
        LLVM_DEBUG(outs() << "Call Instruction: " << I << "\n");
        CallInst *CI = dyn_cast<CallInst>(&I);
        Function *called_func = CI->getCalledFunction();

        // check if called_func has no Declaration, or it has the name
        // as "read" or "write".
        if ((called_func->getName() == "read" ||
             called_func->getName() == "write" ||
             called_func->getName() == "malloc" ||
             called_func->getName() == "free") ||
            (called_func && !called_func->isDeclaration()))
          has_call = true;
      }
    }
  }

  if (has_call) {
    return false;
  }

  return true;
}

PreservedAnalyses LoopExtractor2Pass::run(Module &M,
                                          ModuleAnalysisManager &AM) {
  LLVM_DEBUG(dbgs() << "Loop Extractor!\n");
  auto &FAM = AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
  auto LookupDomTree = [&FAM](Function &F) -> DominatorTree & {
    return FAM.getResult<DominatorTreeAnalysis>(F);
  };
  auto LookupLoopInfo = [&FAM](Function &F) -> LoopInfo & {
    return FAM.getResult<LoopAnalysis>(F);
  };
  auto LookupAssumptionCache = [&FAM](Function &F) -> AssumptionCache * {
    return FAM.getCachedResult<AssumptionAnalysis>(F);
  };
  auto LE2 = LoopExtractor2(NumLoops, LookupDomTree, LookupLoopInfo,
                            LookupAssumptionCache);
  LE2.runOnModule(M);

  // outs() << "M\n";
  for (auto &F : M) {
    for (auto &BB : F) {
      for (auto it = BB.begin(), end = BB.end(); it != end;) {
        Instruction *I = &*it++;

        if (CallInst *callInst = dyn_cast<CallInst>(I)) {
          if (callInst->getCalledFunction()->getName().startswith(
                  "llvm.lifetime")) {
            callInst->eraseFromParent();
          }
        }
      }
    }
  }

  Function *old_F = nullptr;
  std::string old_F_name;

  std::map<Function *, int> extracted_funcs;
  for (auto &F : M) {
    if (LE2.CanConvertToOracle(F)) {
      // insert to map with size
      int size = F.size();
      extracted_funcs.insert(std::pair<Function *, int>(&F, size));
    }
  }

  // select largest size function
  for (auto &func : extracted_funcs) {
    if (old_F == nullptr) {
      old_F = func.first;
      old_F_name = old_F->getName().str();
    } else {
      if (func.second > extracted_funcs[old_F]) {
        old_F = func.first;
        old_F_name = old_F->getName().str();
      }
    }
  }

  // if there is no extracted function
  // investigate all other functions

  // for (auto &F : M) {
  //   if (LE2.CanConvertToOracle(F)) {
  //   // if (F.getName().endswith("extracted")) {
  //     outs() << "Function " << F.getName() << " is converted to oracle\n";
  //     // old_func_name = F.getName();
  //     old_F = &F;
  //     // Create new function
  //     s = F.getName().str();
  //     F.setName("oracle");
  //     break;
  //   }
  // }

  if (old_F == nullptr) {
    return PreservedAnalyses::all();
  }

  old_F->setName("oracle");

  for (auto &func : M) {
    for (auto &BB : func) {
      for (auto it = BB.begin(), end = BB.end(); it != end; ++it) {
        if (CallInst *callInst = dyn_cast<CallInst>(&*it)) {
          if (callInst->getCalledFunction()->getName() == old_F_name) {
            callInst->setCalledFunction(old_F);
          }
        }
      }
    }
  }

  // PreservedAnalyses PA;
  // PA.preserve<LoopAnalysis>();
  return PreservedAnalyses::none();
}

extern "C" ::llvm::PassPluginLibraryInfo llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "LoopExtractor2", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, ModulePassManager &MPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "loop-extractor") {
                    MPM.addPass(LoopExtractor2Pass());
                    return true;
                  }
                  return false;
                });
          }};
}
}; // namespace sc::opt::loop_extractor