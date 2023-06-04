#include "heap_to_stack.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Linker/Linker.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/SourceMgr.h"
#include <map>

using namespace llvm;
using namespace std;

#define SAFETY_BUFFER 1024
#define ABORT_ON_RECURSION true

namespace sc::opt::heap_to_stack {

// Custom functions `my_malloc` and `my_free` to be injected
std::string my_functions_code =
    "declare i8* @malloc(i64)\n"
    "declare void @free(i8*)\n"
    "@current_pos = global i8* null\n"
    "define i8* @my_malloc(i64 %size) {\n"
    "  %heap_start_int = add i64 0, [HEAP-START]\n"
    "  %heap_end_int = add i64 0, 102400\n"
    "  %heap_end = inttoptr i64 %heap_end_int to i8*\n"
    "  ; Check if current_pos is null, i.e., this is the first allocation\n"
    "  %current_pos_val = load i8*, i8** @current_pos\n"
    "  %first_alloc = icmp eq i8* %current_pos_val, null\n"
    "  br i1 %first_alloc, label %init_heap, label %alloc\n"
    "init_heap:\n"
    "  ; Set current_pos to the start of the heap\n"
    "  %new_current_pos = inttoptr i64 %heap_start_int to i8*\n"
    "  store i8* %new_current_pos, i8** @current_pos\n"
    "  br label %alloc\n"
    "alloc:\n"
    "  %current_pos.1 = load i8*, i8** @current_pos\n"
    "  %next_pos = getelementptr i8, i8* %current_pos.1, i64 %size\n"
    "  %overflow = icmp ugt i8* %next_pos, %heap_end\n"
    "  br i1 %overflow, label %fallback, label %update_pos\n"
    "update_pos:\n"
    "  store i8* %next_pos, i8** @current_pos\n"
    "  ret i8* %current_pos.1\n"
    "fallback:\n"
    "  %malloc_ptr = call i8* @malloc(i64 %size)\n"
    "  ret i8* %malloc_ptr\n"
    "}\n"
    "define void @my_free(i8* %ptr) {\n"
    "  %heap_end_int = add i64 0, 102400\n"
    "  %heap_end = inttoptr i64 %heap_end_int to i8*\n"
    "  %is_on_stack = icmp ugt i8* %ptr, %heap_end\n"
    "  br i1 %is_on_stack, label %stack, label %heap\n"
    "heap:\n"
    "  ret void\n"
    "stack:\n"
    "  call void @free(i8* %ptr)\n"
    "  ret void\n"
    "}";

// Checks whether there are cycles in the call graph
bool hasCycle(const Function *current, std::vector<const Function *> &path,
              std::set<const Function *> &visited, CallGraph &CG) {
  if (visited.count(current))
    return false;
  if (std::find(path.begin(), path.end(), current) != path.end())
    return true;

  path.push_back(current);
  CallGraphNode *node = CG[current];
  for (auto &calledFunction : *node) {
    if (calledFunction.second->getFunction() &&
        hasCycle(calledFunction.second->getFunction(), path, visited, CG)) {
      return true;
    }
  }
  path.pop_back();
  visited.insert(current);

  return false;
}

PreservedAnalyses HeapToStack::run(Module &M, ModuleAnalysisManager &MAM) {
  Function *mallocFunc = M.getFunction("malloc");
  Function *freeFunc = M.getFunction("free");

  // All functions should be found!
  if (!mallocFunc || !freeFunc) {
    return PreservedAnalyses::none();
  }

  // Check if module contains `malloc` usages
  // Abort if no `malloc`s found
  bool malloc_found = false;
  for (auto &F : M)
    for (auto &BB : F)
      for (auto &I : BB)
        if (auto *CI = dyn_cast<CallInst>(&I))
          if (CI->getCalledFunction() == mallocFunc) {
            malloc_found = true;
            break;
          }
  if (!malloc_found) {
    outs() << "No malloc usage\n";
    return PreservedAnalyses::all();
  }

  // Check if recursive calls exist
  // Recursive calls invalidate the runtime stack usage upper bound
  if (ABORT_ON_RECURSION) {
    CallGraph CG(M);
    for (auto &function : M) {
      if (function.isDeclaration())
        continue;
      std::vector<const Function *> path;
      std::set<const Function *> visited;
      if (hasCycle(&function, path, visited, CG)) {
        outs() << "Recursive call cycle detected: ";
        for (auto *f : path)
          outs() << f->getName() << " -> ";
        outs() << function.getName() << "\n";
        return PreservedAnalyses::all();
      }
    }
  }

  // Calculate the upper bound of runtime stack usage
  int totalStackSize = SAFETY_BUFFER;
  for (auto &F : M) {
    for (auto &BB : F) {
      for (auto &I : BB) {
        totalStackSize += 8; // Add 8 bytes for each instruction

        // If the instruction is an alloca, add its allocated size
        if (auto *AI = dyn_cast<AllocaInst>(&I)) {
          Type *alloc_type = AI->getAllocatedType();
          auto *array_size = dyn_cast<ConstantInt>(AI->getArraySize());
          assert(array_size);
          totalStackSize +=
              (int)(array_size->getZExtValue() *
                    M.getDataLayout().getTypeAllocSize(alloc_type));
        }
      }
    }
  }

  // Replace the [HEAP-START] constant in the code to be inserted
  string heap_start_token = "[HEAP-START]";
  string heap_start = itostr(totalStackSize);
  size_t replace_pos = my_functions_code.find(heap_start_token);
  assert(replace_pos != string::npos);
  my_functions_code.replace(replace_pos, heap_start_token.length(), heap_start);

  // Parse the string into a new module
  SMDiagnostic error;
  LLVMContext &context = M.getContext();
  IRBuilder<> builder(context);
  std::unique_ptr<Module> newModule =
      parseIR(MemoryBuffer::getMemBuffer(my_functions_code)->getMemBufferRef(),
              error, context);

  // Link the new module into the original one
  if (Linker::linkModules(M, std::move(newModule))) {
    outs() << "Linking failed\n";
  }

  Function *myMallocFunc = M.getFunction("my_malloc");
  Function *myFreeFunc = M.getFunction("my_free");

  // All functions should be found!
  if (!myMallocFunc || !myFreeFunc) {
    exit(1);
  }

  SmallVector<CallInst *> toErase;

  // Replace `malloc` and `free` with `my_malloc` and `my_free` respectively
  for (auto &F : M) {
    // Replacement should not happen within `my_malloc` and `my_free`
    if (&F != myMallocFunc && &F != myFreeFunc) {
      for (auto &BB : F) {
        for (auto &I : BB) {
          if (auto *CI = dyn_cast<CallInst>(&I)) {
            // Set targets
            Function *replace_with = nullptr;
            if (CI->getCalledFunction() == mallocFunc) {
              replace_with = myMallocFunc;
            } else if (CI->getCalledFunction() == freeFunc) {
              replace_with = myFreeFunc;
            }
            // Do the replacement
            if (replace_with) {
              builder.SetInsertPoint(CI);
              Value *arg = CI->getArgOperand(0);
              CallInst *newCall = builder.CreateCall(replace_with, {arg});
              CI->replaceAllUsesWith(newCall);
              toErase.push_back(CI);
            }
          }
        }
      }
    }
  }
  // Remove original `malloc` and `free` calls
  for (auto CI : toErase) {
    CI->eraseFromParent();
  }
  return PreservedAnalyses::none();
}

extern "C" ::llvm::PassPluginLibraryInfo llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "HeapToStack", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, ModulePassManager &MPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "heap-to-stack") {
                    MPM.addPass(HeapToStack());
                    return true;
                  }
                  return false;
                });
          }};
}
} // namespace sc::opt::heap_to_stack