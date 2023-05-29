#ifndef REGISTER_PRESSURE_REDUCTION_H
#define REGISTER_PRESSURE_REDUCTION_H

#include "llvm/IR/PassManager.h"

using namespace std;
using namespace llvm;

namespace sc::opt::register_pressure_reduction {
class RegisterPressureReduction
    : public PassInfoMixin<RegisterPressureReduction> {
public:
  static PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM);
};
} // namespace sc::opt::register_pressure_reduction
#endif