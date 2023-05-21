#ifndef SC_OPT_REGISTER_PRESSURE_ANALYSIS
#define SC_OPT_REGISTER_PRESSURE_ANALYSIS

#include "llvm/IR/Instructions.h"
#include "llvm/IR/PassManager.h"

using namespace std;
using namespace llvm;

namespace sc::opt::register_pressure_analysis {
class RegisterPressureAnalysis
    : public PassInfoMixin<RegisterPressureAnalysis> {
public:
  uint64_t run(Function &F, FunctionAnalysisManager &FAM);
};

class RegisterPressurePrinterPass
    : public PassInfoMixin<RegisterPressurePrinterPass> {
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM);
};
} // namespace sc::opt::register_pressure_analysis
#endif // SC_OPT_REGISTER_PRESSURE_ANALYSIS