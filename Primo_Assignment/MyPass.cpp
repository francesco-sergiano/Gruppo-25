// MyPass.cpp
#include "llvm/IR/PassManager.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Passes/PassBuilder.h"

using namespace llvm;

namespace {

class MyPass : public PassInfoMixin<MyPass> {
public:
  // Funzione principale
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM) {
    bool changed = false;

    // Per ogni BasicBlock, ottimizzo
    for (BasicBlock &BB : F) {
      // 1) Algebrhaic Identity (x+0 => x, x*1 => x, etc.)
      changed |= runOnBasicBlock(BB);

      // 2) MultiIstruction Optimization (es. a=b+1, c=a-1)
      changed |= runMultiInstructionOptimization(BB);
    }

    return (changed ? PreservedAnalyses::none() : PreservedAnalyses::all());
  }

private:
  /// Esempio di ottimizzazione
  bool runOnBasicBlock(BasicBlock &BB) {
    bool changed = false;

    // Uso un iteratore manuale
    for (auto it = BB.begin(); it != BB.end();) {
      Instruction &I = *it++;

      // Provo l'ottimizzazione
      if (auto *binOp = dyn_cast<BinaryOperator>(&I)) {
        if (binOp->getOpcode() == Instruction::Add) {
          // Se uno dei due è a zero, sostituisco con l'altro
          changed |= optimizeAddZero(binOp);
        }
        else if (binOp->getOpcode() == Instruction::Mul) {
          changed |= optimizeMulPatterns(binOp);
        }
        else if (binOp->getOpcode() == Instruction::SDiv ||
                 binOp->getOpcode() == Instruction::UDiv) {
        }
      }
    }
    return changed;
  }

  /// x + 0 => x (o 0 + x => x)
  bool optimizeAddZero(BinaryOperator *binOp) {
    bool changed = false;
    Value *op1 = binOp->getOperand(0);
    Value *op2 = binOp->getOperand(1);

    // Controllo se op2 è zero
    if (auto *c2 = dyn_cast<ConstantInt>(op2)) {
      if (c2->isZero()) {
        binOp->replaceAllUsesWith(op1);
        binOp->eraseFromParent();
        return true;
      }
    }
    // Controllo se op1 è zero
    if (auto *c1 = dyn_cast<ConstantInt>(op1)) {
      if (c1->isZero()) {
        binOp->replaceAllUsesWith(op2);
        binOp->eraseFromParent();
        return true;
      }
    }
    return changed;
  }

  /// Esempio: x * 1 => x, x * 15 => (x << 4) - x, 1 * x => x, 15 * x => (x <<4) - x
  bool optimizeMulPatterns(BinaryOperator *binOp) {
    bool changed = false;
    Value *op1 = binOp->getOperand(0);
    Value *op2 = binOp->getOperand(1);

    // Se op2 è costante
    if (auto *c2 = dyn_cast<ConstantInt>(op2)) {
      int64_t val = c2->getSExtValue();
      if (val == 1) {
        // x*1 => x
        binOp->replaceAllUsesWith(op1);
        binOp->eraseFromParent();
        return true;
      } else if (val == 15) {
        // x*15 => (x << 4) - x
        IRBuilder<> builder(binOp);
        Value *shiftLeft = builder.CreateShl(op1, builder.getInt32(4));
        Value *res = builder.CreateSub(shiftLeft, op1);

        binOp->replaceAllUsesWith(res);
        binOp->eraseFromParent();
        return true;
      }
    }

    // Se op1 è costante
    if (auto *c1 = dyn_cast<ConstantInt>(op1)) {
      int64_t val = c1->getSExtValue();
      if (val == 1) {
        // 1*x => x
        binOp->replaceAllUsesWith(op2);
        binOp->eraseFromParent();
        return true;
      } else if (val == 15) {
        // 15*x => (x << 4) - x
        IRBuilder<> builder(binOp);
        Value *shiftLeft = builder.CreateShl(op2, builder.getInt32(4));
        Value *res = builder.CreateSub(shiftLeft, op2);

        binOp->replaceAllUsesWith(res);
        binOp->eraseFromParent();
        return true;
      }
    }

    return changed;
  }

  /// Esempio di “multi-istruzione”: 
  ///   %a = add i32 %b, 1
  ///   %c = sub i32 %a, 1
  /// => %c = %b
  bool runMultiInstructionOptimization(BasicBlock &BB) {
    bool changed = false;

    for (auto instIt = BB.begin(); instIt != BB.end(); ++instIt) {
      Instruction *I1 = &*instIt;
      if (I1->getOpcode() != Instruction::Add)
        continue;

      // Verifico che sia (b + 1)
      if (!isAddOne(I1))
        continue;

      // Controllo la successiva
      auto nextIt = std::next(instIt);
      if (nextIt == BB.end()) break;
      Instruction *I2 = &*nextIt;

      // Deve essere un sub con 1, che usa I1 come primo operando
      if (I2->getOpcode() != Instruction::Sub)
        continue;
      if (!isSubOne(I2, I1))
        continue;

      // Applico l'ottimizzazione: c = b (invece di c = a - 1)
      I2->replaceAllUsesWith(I1->getOperand(0));
      I2->eraseFromParent();
      changed = true;
    }
    return changed;
  }

  // Verifico se un'istruzione Add e' (qualcosa + 1) o (1 + qualcosa)
  bool isAddOne(Instruction *I) {
    if (I->getNumOperands() != 2)
      return false;
    Value *op1 = I->getOperand(0);
    Value *op2 = I->getOperand(1);

    if (auto *c = dyn_cast<ConstantInt>(op1))
      if (c->getSExtValue() == 1)
        return true;
    if (auto *c = dyn_cast<ConstantInt>(op2))
      if (c->getSExtValue() == 1)
        return true;

    return false;
  }

  // Verifico se un'istruzione Sub e' (I1 - 1)
  bool isSubOne(Instruction *I2, Instruction *I1) {
    if (I2->getNumOperands() != 2)
      return false;
    Value *op1 = I2->getOperand(0);
    Value *op2 = I2->getOperand(1);

    // op1 deve essere l'istruzione I1
    if (op1 != I1)
      return false;
    if (auto *c = dyn_cast<ConstantInt>(op2))
      if (c->getSExtValue() == 1)
        return true;

    return false;
  }

}; // end class

} // end anonymous namespace

// Plugin entry point col New Pass Manager
extern "C" ::llvm::PassPluginLibraryInfo LLVM_ATTRIBUTE_WEAK
llvmGetPassPluginInfo() {
  return {
    LLVM_PLUGIN_API_VERSION,
    "MyPass",
    "v0.1",
    [](PassBuilder &PB) {
      PB.registerPipelineParsingCallback(
        [&](StringRef Name, FunctionPassManager &FPM,
            ArrayRef<PassBuilder::PipelineElement>) {
          if (Name == "my-pass") {
            FPM.addPass(MyPass());
            return true;
          }
          return false;
        }
      );
    }
  };
}
