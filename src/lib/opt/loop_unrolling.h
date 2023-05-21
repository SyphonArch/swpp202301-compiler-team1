#ifndef SWPP_COMPILER_LOOP_UNROLLING_H
#define SWPP_COMPILER_LOOP_UNROLLING_H

#include "llvm/IR/PassManager.h"
#include <llvm/Analysis/LoopInfo.h>
#include "llvm/IR/PassManager.h"
#include "llvm/Analysis/LoopAnalysisManager.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Transforms/Utils/LoopUtils.h"

using namespace std;
using namespace llvm;

namespace sc::opt::loop_unrolling {
class LoopUnrolling : public PassInfoMixin<LoopUnrolling> {
public:
  PreservedAnalyses run(Function &val, FunctionAnalysisManager &FAM);
  bool runOnLoop(Loop *L, ScalarEvolution &SE);
};
} // namespace sc::opt::loop_unrolling

#endif // SWPP_COMPILER_LOOP_UNROLLING_H
