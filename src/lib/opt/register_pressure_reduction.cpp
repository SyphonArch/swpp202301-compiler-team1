#include "register_pressure_reduction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"

using namespace llvm;
using namespace std;

namespace sc::opt::register_pressure_reduction {
PreservedAnalyses RegisterPressureReduction::run(Function &F,
                                                 FunctionAnalysisManager &FAM) {

  // Collect all instructions which don't have side effects
  std::vector<Instruction *> instructions;
  // Record the location of Instructions within their respective blocks
  DenseMap<Instruction *, int> loc_in_block;
  for (BasicBlock &BB : F) {
    int loc_in_block_tracker = 0;
    for (Instruction &I : BB) {
      if (!I.mayHaveSideEffects()) {
        instructions.push_back(&I);
      }
      loc_in_block[&I] = loc_in_block_tracker++;
    }
  }

  // Looping over the previously collected instructions,
  for (auto I : instructions) {
    // Collect all usages of the instruction in the same block.
    int curr_loc = loc_in_block[I];
    Instruction *first_usage = nullptr;
    int usage_loc;
    bool has_usage_in_block = false;
    for (auto user : I->users()) {
      if (auto usage_inst = dyn_cast<Instruction>(user)) {
        if (usage_inst->getParent() == I->getParent()) {
          has_usage_in_block = true;
          usage_loc = loc_in_block[usage_inst];
          if (usage_loc > curr_loc && (first_usage == nullptr ||
                                       usage_loc < loc_in_block[first_usage])) {
            first_usage = usage_inst;
          }
        }
      }
      if (first_usage != nullptr) {
        // TODO: Resolve issue with out-of-block Instruction moving
        //I->moveBefore(first_usage);
      }
    }

    if (!has_usage_in_block) {
      // If there aren't any usages in the same block
      if (I->hasOneUse()) {
        User *only_user = *I->user_begin();
        if (auto only_inst = dyn_cast<Instruction>(only_user)) {
          outs() << "Moving " << I->getName() << " to " << only_inst->getName() << "\n";
          // TODO: Resolve issue with out-of-block Instruction moving
          //I->moveBefore(only_inst);
        }
      }
    }
  }
  return PreservedAnalyses::none();
}

extern "C" ::llvm::PassPluginLibraryInfo llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "RegisterPressureReduction",
          LLVM_VERSION_STRING, [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "register-pressure-reduction") {
                    FPM.addPass(RegisterPressureReduction());
                    return true;
                  }
                  return false;
                });
          }};
}
} // namespace sc::opt::register_pressure_reduction