#include "llvm/IR/Dominators.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;
using namespace std;

namespace {
class DemoPropagateIntegerEquality
    : public PassInfoMixin<DemoPropagateIntegerEquality> {
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM) {
    int replace_counts = 0;
    DominatorTree &DT = FAM.getResult<DominatorTreeAnalysis>(F);

    for (Function::iterator StartBB = F.begin(), E = F.end(); StartBB != E;
         ++StartBB) {
      // for every basic block BB,
      for (BasicBlock::iterator I = StartBB->begin(), IE = StartBB->end();
           I != IE; ++I) {
        // for every instruction I,
        BranchInst *branch_inst = dyn_cast<BranchInst>(&*I);

        // If not a branch instruction, ignore.
        if (!branch_inst) {
          continue;
        }
        // branch_inst is a branch instruction.

        // If not a conditional branch, ignore.
        if (!branch_inst->isConditional()) {
          outs() << "\tUnconditional branch ignored.\n";
          continue;
        }
        // branch_inst is a conditional branch instruction.

        // If branch condition is not an instruction, ignore.
        if (!isa<Instruction>(branch_inst->getOperand(0))) {
          if (isa<Constant>(branch_inst->getOperand(0))) {
            outs() << "\tBranch on constant ignored.\n";
          } else {
            outs()
                << "\tBranch condition is not an instruction nor a constant!\n";
          }
          continue;
        }
        // branch_inst is a conditional branch instruction on an instruction
        // condition.

        ICmpInst *icmp_inst =
            dyn_cast<ICmpInst>(&*(branch_inst->getOperand(0)));

        // If branch_inst's branch condition is not an icmp instruction, ignore.
        if (!icmp_inst) {
          errs() << "\tBranch condition register is not a icmp instruction!\n";
          continue;
        }
        // branch_inst is a conditional branch instruction on an icmp
        // instruction condition, icmp_inst.

        // If branch_inst's branch condition is not an `icmp eq` instruction,
        // ignore.
        if (icmp_inst->getPredicate() != ICmpInst::Predicate::ICMP_EQ) {
          outs()
              << "\tBranch condition register is not a icmp eq instruction!\n";
          continue;
        }
        // branch_inst is a conditional branch instruction on an `icmp eq`
        // instruction condition, icmp_inst.

        Value *Op1 = icmp_inst->getOperand(0); // left operand
        Value *Op2 = icmp_inst->getOperand(1); // right operand

        // Check that icmp_inst does not compare to constants.
        if (ConstantInt *test1 = dyn_cast<ConstantInt>(Op1)) {
          errs() << "\ticmp eq instruction has a constant operand for Op1!\n";
          continue;
        } else if (ConstantInt *test2 = dyn_cast<ConstantInt>(Op2)) {
          errs() << "\ticmp eq instruction has a constant operand for Op2!\n";
          continue;
        }
        // branch_inst is a conditional branch instruction on an `icmp eq`
        // instruction condition, icmp_inst, which in turn has two non-constant
        // operands.

        // Check that operands are integers.
        if (!Op1->getType()->isIntegerTy()) {
          errs() << "\ticmp eq instruction compares non-integer types!\n";
          continue;
        } else if (!Op2->getType()->isIntegerTy()) {
          errs() << "\ticmp eq instruction compares non-integer types!\n";
          continue;
        }

        // Here we can finally get our job done.
        Value *ReplaceThis;
        Value *ReplaceWith;
        // Determine which one to replace with which one.
        if (isa<Instruction>(Op1) && isa<Instruction>(Op2)) {
          Instruction *Op1_I = dyn_cast<Instruction>(Op1);
          Instruction *Op2_I = dyn_cast<Instruction>(Op2);
          if (DT.dominates(Op1_I, Op2_I)) {
            ReplaceThis = Op2;
            ReplaceWith = Op1;
          } else if (DT.dominates(Op2_I, Op1_I)) {
            ReplaceThis = Op1;
            ReplaceWith = Op2;
          } else {
            outs() << "\t\tTwo instructions don't dominate each other!\n";
            ReplaceThis = Op2;
            ReplaceWith = Op1;
          }
        } else if (isa<Instruction>(Op1) && isa<Argument>(Op2)) {
          ReplaceThis = Op1;
          ReplaceWith = Op2;
        } else if (isa<Instruction>(Op2) && isa<Argument>(Op1)) {
          ReplaceThis = Op2;
          ReplaceWith = Op1;
        } else if (isa<Argument>(Op1) && isa<Argument>(Op2)) {
          if (dyn_cast<Argument>(Op1)->getArgNo() <
              dyn_cast<Argument>(Op2)->getArgNo()) {
            ReplaceThis = Op2;
            ReplaceWith = Op1;
          } else {
            ReplaceThis = Op1;
            ReplaceWith = Op2;
          }
        } else {
          errs() << "\t\tOperands not instruction nor argument.\n";
          continue;
        }

        if (branch_inst->getNumSuccessors() != 2) {
          errs() << "\t\tBranch does not have two successors!\n";
          continue;
        }

        outs() << "\tReplacing " << ReplaceThis->getName() << " with "
               << ReplaceWith->getName() << ":\n";

        BasicBlock *TrueBranchBB = branch_inst->getSuccessor(0);
        BasicBlockEdge TrueBranchEdge(&*StartBB, TrueBranchBB);

        for (BasicBlock &TargetBB : F) {
          if (DT.dominates(TrueBranchEdge, &TargetBB)) {
            outs() << "\t\tEdge (" << StartBB->getName() << ", "
                   << TrueBranchBB->getName() << ") dominates "
                   << TargetBB.getName() << "!\n";
            // We can finally do the replacing.
            for (auto &TargetInst : TargetBB) {
              for (Use &U : TargetInst.operands()) {
                if (U.get() == ReplaceThis) {
                  U.set(ReplaceWith);
                  ++replace_counts;
                }
              }
            }
          }
        }
      }
    }
    outs() << '\t' << replace_counts << " replacements made.\n";
    return PreservedAnalyses::none();
  }
};
} // namespace

extern "C" ::llvm::PassPluginLibraryInfo llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "DemoPropagateIntegerEquality", "v0.1",
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "demo-prop-int-eq") {
                    FPM.addPass(DemoPropagateIntegerEquality());
                    return true;
                  }
                  return false;
                });
          }};
}
