#ifndef SC_BACKEND_ANALYSIS_H
#define SC_BACKEND_ANALYSIS_H

#include "../../result.h"
#include "../../static_error.h"

#include "llvm/IR/Instructions.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"

namespace sc::backend::analysis {
class UnknownSizeTypeError : public Error<UnknownSizeTypeError> {
private:
  std::string message;

public:
  UnknownSizeTypeError(llvm::Type *ty) noexcept;
  const char *what() const noexcept { return message.c_str(); }
};

llvm::Instruction *isMoveInst(llvm::Value *v);
bool isReg(llvm::Value *v);
Result<uint64_t, UnknownSizeTypeError>
tryCalculateSize(llvm::Type *ty) noexcept;
} // namespace sc::backend::analysis
#endif // SC_BACKEND_ANALYSIS_H
