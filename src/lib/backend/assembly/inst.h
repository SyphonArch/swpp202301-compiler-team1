#ifndef SC_LIB_BACKEND_ASSEMBLY_INST_H
#define SC_LIB_BACKEND_ASSEMBLY_INST_H

#include <map>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "../../../result.h"
#include "../../../static_error.h"
#include "int_t.h"
#include "register_t.h"
#include "width_t.h"

namespace super = sc::backend::assembly;

namespace sc::backend::assembly::inst {
using AccessWidth = super::width_t::AccessWidth;
using BitWidth = super::width_t::BitWidth;

using IntTy = super::int_t::IntTy;
using GeneralRegister = super::register_t::GeneralRegister;
using ArgumentRegister = super::register_t::ArgumentRegister;
using ValueTy = std::variant<IntTy, GeneralRegister, ArgumentRegister>;

template <typename T> class AbstractInst {
private:
  AbstractInst() noexcept {}
  friend T;

public:
  AbstractInst(AbstractInst &&) = default;
  AbstractInst &operator=(AbstractInst &&) = default;

  std::string getAssembly() const noexcept {
    return static_cast<T *>(this)->getAssembly();
  }
};

class InvalidFunctionArgcError : public Error<InvalidFunctionArgcError> {
public:
  const char *what() const noexcept {
    return "Number of arguments must not exceed 16";
  }
};

class FunctionStartInst : public AbstractInst<FunctionStartInst> {
private:
  std::string name;
  IntTy argc;
  FunctionStartInst(std::string &&name, const IntTy argc) noexcept;

public:
  static Result<FunctionStartInst, InvalidFunctionArgcError>
  tryCreate(std::string &&name, const IntTy argc) noexcept;
  std::string getAssembly() const noexcept;
};

class FunctionEndInst : public AbstractInst<FunctionEndInst> {
private:
  std::string name;
  FunctionEndInst(std::string &&name) noexcept;

public:
  static FunctionEndInst create(std::string &&name) noexcept;
  std::string getAssembly() const noexcept;
};

class BasicBlockInst : public AbstractInst<BasicBlockInst> {
private:
  std::string name;
  BasicBlockInst(std::string &&name) noexcept;

public:
  static BasicBlockInst create(std::string &&name) noexcept;
  std::string getAssembly() const noexcept;
};

class CommentInst : public AbstractInst<CommentInst> {
private:
  std::string message;
  CommentInst(std::string &&message) noexcept;

public:
  static CommentInst create(std::string &&message) noexcept;
  std::string getAssembly() const noexcept;
};

class ReturnInst : public AbstractInst<ReturnInst> {
private:
  ValueTy value;
  ReturnInst() noexcept;
  ReturnInst(ValueTy &&value) noexcept;

public:
  static ReturnInst createVoid() noexcept;
  static ReturnInst create(ValueTy &&value) noexcept;
  std::string getAssembly() const noexcept;
};

class JumpInst : public AbstractInst<JumpInst> {
private:
  std::string label;
  JumpInst(std::string &&message) noexcept;

public:
  static JumpInst create(std::string &&message) noexcept;
  std::string getAssembly() const noexcept;
};

class BranchInst : public AbstractInst<BranchInst> {
private:
  ValueTy condition;
  std::string label_true;
  std::string label_false;
  BranchInst(ValueTy &&condition, std::string &&label_true,
             std::string &&label_false) noexcept;

public:
  static BranchInst create(ValueTy &&condition, std::string &&label_true,
                           std::string &&label_false) noexcept;
  std::string getAssembly() const noexcept;
};

class SwitchInst : public AbstractInst<SwitchInst> {
private:
  ValueTy condition;
  std::map<IntTy, std::string> cases;
  std::string label_default;
  SwitchInst(ValueTy &&condition, std::string &&label_default) noexcept;
  void addCase(const IntTy value, std::string &&label) noexcept;

public:
  class MismatchingCaseError : public Error<MismatchingCaseError> {
  public:
    const char *what() const noexcept {
      return "Number of cases and labels does not match";
    }
  };

  static Result<SwitchInst, MismatchingCaseError>
  tryCreate(ValueTy &&cond, std::vector<IntTy> &&cases,
            std::vector<std::string> &&labels,
            std::string &&label_default) noexcept;
  std::string getAssembly() const noexcept;
};

class MallocInst : public AbstractInst<MallocInst> {
private:
  GeneralRegister target;
  ValueTy size;
  MallocInst(const GeneralRegister target, ValueTy &&size) noexcept;

public:
  static MallocInst create(const GeneralRegister target,
                           ValueTy &&size) noexcept;
  std::string getAssembly() const noexcept;
};

class FreeInst : public AbstractInst<FreeInst> {
private:
  ValueTy ptr;
  FreeInst(ValueTy &&ptr) noexcept;

public:
  static FreeInst create(ValueTy &&ptr) noexcept;
  std::string getAssembly() const noexcept;
};

class LoadInst : public AbstractInst<LoadInst> {
private:
  GeneralRegister target;
  AccessWidth size;
  ValueTy ptr;
  LoadInst(const GeneralRegister target, const AccessWidth size,
           ValueTy &&ptr) noexcept;

public:
  static LoadInst create(const GeneralRegister target, const AccessWidth size,
                         ValueTy &&ptr) noexcept;
  std::string getAssembly() const noexcept;
};

class StoreInst : public AbstractInst<StoreInst> {
private:
  AccessWidth size;
  ValueTy value;
  ValueTy ptr;
  StoreInst(const AccessWidth size, ValueTy &&value, ValueTy &&ptr) noexcept;

public:
  static StoreInst create(const AccessWidth size, ValueTy &&value,
                          ValueTy &&ptr) noexcept;
  std::string getAssembly() const noexcept;
};

template <typename T>
class IntBinaryOpInst : public AbstractInst<IntBinaryOpInst<T>> {
private:
  GeneralRegister target;
  BitWidth bw;
  ValueTy lhs;
  ValueTy rhs;

  IntBinaryOpInst(const GeneralRegister target, const BitWidth bw,
                  ValueTy &&lhs, ValueTy &&rhs) noexcept {
    this->target = target;
    this->bw = bw;
    this->lhs = std::move(lhs);
    this->rhs = std::move(rhs);
  }
  friend T;

public:
  std::string getAssembly() const noexcept {
    return static_cast<T *>(this)->getAssembly();
  }
};

class IntAddInst : public IntBinaryOpInst<IntAddInst> {
private:
  IntAddInst(const GeneralRegister target, const BitWidth bw, ValueTy &&lhs,
             ValueTy &&rhs) noexcept;

public:
  static IntAddInst create(const GeneralRegister target, const BitWidth bw,
                           ValueTy &&lhs, ValueTy &&rhs) noexcept;
  std::string getAssembly() const noexcept;
};

class IntSubInst : public IntBinaryOpInst<IntSubInst> {
private:
  IntSubInst(const GeneralRegister target, const BitWidth bw, ValueTy &&lhs,
             ValueTy &&rhs) noexcept;

public:
  static IntSubInst create(const GeneralRegister target, const BitWidth bw,
                           ValueTy &&lhs, ValueTy &&rhs) noexcept;
  std::string getAssembly() const noexcept;
};

class IntMulInst : public IntBinaryOpInst<IntMulInst> {
private:
  IntMulInst(const GeneralRegister target, const BitWidth bw, ValueTy &&lhs,
             ValueTy &&rhs) noexcept;

public:
  static IntMulInst create(const GeneralRegister target, const BitWidth bw,
                           ValueTy &&lhs, ValueTy &&rhs) noexcept;
  std::string getAssembly() const noexcept;
};

class IntUDivInst : public IntBinaryOpInst<IntUDivInst> {
private:
  IntUDivInst(const GeneralRegister target, const BitWidth bw, ValueTy &&lhs,
              ValueTy &&rhs) noexcept;

public:
  static IntUDivInst create(const GeneralRegister target, const BitWidth bw,
                            ValueTy &&lhs, ValueTy &&rhs) noexcept;
  std::string getAssembly() const noexcept;
};

class IntSDivInst : public IntBinaryOpInst<IntSDivInst> {
private:
  IntSDivInst(const GeneralRegister target, const BitWidth bw, ValueTy &&lhs,
              ValueTy &&rhs) noexcept;

public:
  static IntSDivInst create(const GeneralRegister target, const BitWidth bw,
                            ValueTy &&lhs, ValueTy &&rhs) noexcept;
  std::string getAssembly() const noexcept;
};

class IntURemInst : public IntBinaryOpInst<IntURemInst> {
private:
  IntURemInst(const GeneralRegister target, const BitWidth bw, ValueTy &&lhs,
              ValueTy &&rhs) noexcept;

public:
  static IntURemInst create(const GeneralRegister target, const BitWidth bw,
                            ValueTy &&lhs, ValueTy &&rhs) noexcept;
  std::string getAssembly() const noexcept;
};

class IntSRemInst : public IntBinaryOpInst<IntSRemInst> {
private:
  IntSRemInst(const GeneralRegister target, const BitWidth bw, ValueTy &&lhs,
              ValueTy &&rhs) noexcept;

public:
  static IntSRemInst create(const GeneralRegister target, const BitWidth bw,
                            ValueTy &&lhs, ValueTy &&rhs) noexcept;
  std::string getAssembly() const noexcept;
};

class IntAndInst : public IntBinaryOpInst<IntAndInst> {
private:
  IntAndInst(const GeneralRegister target, const BitWidth bw, ValueTy &&lhs,
             ValueTy &&rhs) noexcept;

public:
  static IntAndInst create(const GeneralRegister target, const BitWidth bw,
                           ValueTy &&lhs, ValueTy &&rhs) noexcept;
  std::string getAssembly() const noexcept;
};

class IntOrInst : public IntBinaryOpInst<IntOrInst> {
private:
  IntOrInst(const GeneralRegister target, const BitWidth bw, ValueTy &&lhs,
            ValueTy &&rhs) noexcept;

public:
  static IntOrInst create(const GeneralRegister target, const BitWidth bw,
                          ValueTy &&lhs, ValueTy &&rhs) noexcept;
  std::string getAssembly() const noexcept;
};

class IntXorInst : public IntBinaryOpInst<IntXorInst> {
private:
  IntXorInst(const GeneralRegister target, const BitWidth bw, ValueTy &&lhs,
             ValueTy &&rhs) noexcept;

public:
  static IntXorInst create(const GeneralRegister target, const BitWidth bw,
                           ValueTy &&lhs, ValueTy &&rhs) noexcept;
  std::string getAssembly() const noexcept;
};

class IntShlInst : public IntBinaryOpInst<IntShlInst> {
private:
  IntShlInst(const GeneralRegister target, const BitWidth bw, ValueTy &&lhs,
             ValueTy &&rhs) noexcept;

public:
  static IntShlInst create(const GeneralRegister target, const BitWidth bw,
                           ValueTy &&lhs, ValueTy &&rhs) noexcept;
  std::string getAssembly() const noexcept;
};

class IntLShrInst : public IntBinaryOpInst<IntLShrInst> {
private:
  IntLShrInst(const GeneralRegister target, const BitWidth bw, ValueTy &&lhs,
              ValueTy &&rhs) noexcept;

public:
  static IntLShrInst create(const GeneralRegister target, const BitWidth bw,
                            ValueTy &&lhs, ValueTy &&rhs) noexcept;
  std::string getAssembly() const noexcept;
};

class IntAShrInst : public IntBinaryOpInst<IntAShrInst> {
private:
  IntAShrInst(const GeneralRegister target, const BitWidth bw, ValueTy &&lhs,
              ValueTy &&rhs) noexcept;

public:
  static IntAShrInst create(const GeneralRegister target, const BitWidth bw,
                            ValueTy &&lhs, ValueTy &&rhs) noexcept;
  std::string getAssembly() const noexcept;
};

class IntCompInst : public IntBinaryOpInst<IntCompInst> {
public:
  enum class Condition { EQ, NE, UGT, UGE, ULT, ULE, SGT, SGE, SLT, SLE };

private:
  Condition cond;
  IntCompInst(const GeneralRegister target, const Condition cond,
              const BitWidth bw, ValueTy &&lhs, ValueTy &&rhs) noexcept;

public:
  static IntCompInst create(const GeneralRegister target, const Condition cond,
                            const BitWidth bw, ValueTy &&lhs,
                            ValueTy &&rhs) noexcept;
  std::string getAssembly() const noexcept;
};
using IcmpCondition = IntCompInst::Condition;

class SelectInst : public AbstractInst<SelectInst> {
private:
  GeneralRegister target;
  ValueTy cond;
  ValueTy lhs;
  ValueTy rhs;
  SelectInst(const GeneralRegister target, ValueTy &&cond, ValueTy &&lhs,
             ValueTy &&rhs) noexcept;

public:
  static SelectInst create(const GeneralRegister target, ValueTy &&cond,
                           ValueTy &&lhs, ValueTy &&rhs) noexcept;
  std::string getAssembly() const noexcept;
};

class CallInst : public AbstractInst<CallInst> {
private:
  std::optional<GeneralRegister> target;
  std::string name;
  std::vector<ValueTy> args;
  CallInst(std::string &&name) noexcept;
  CallInst(const GeneralRegister target, std::string &&name) noexcept;
  void addArg(ValueTy &&arg) noexcept;

public:
  static Result<CallInst, InvalidFunctionArgcError>
  tryCreateDiscarding(std::string &&name, std::vector<ValueTy> &&args) noexcept;
  static Result<CallInst, InvalidFunctionArgcError>
  tryCreateReturning(const GeneralRegister target, std::string &&name,
                     std::vector<ValueTy> &&args) noexcept;
  std::string getAssembly() const noexcept;
};

class AssertEqInst : public AbstractInst<AssertEqInst> {
private:
  ValueTy lhs;
  ValueTy rhs;
  AssertEqInst(ValueTy &&lhs, ValueTy &&rhs) noexcept;

public:
  static AssertEqInst create(ValueTy &&lhs, ValueTy &&rhs) noexcept;
  std::string getAssembly() const noexcept;
};

class AsyncLoadInst : public AbstractInst<AsyncLoadInst> {
private:
  GeneralRegister target;
  AccessWidth size;
  ValueTy ptr;
  AsyncLoadInst(const GeneralRegister target, const AccessWidth size,
                ValueTy &&ptr) noexcept;

public:
  static AsyncLoadInst create(const GeneralRegister target,
                              const AccessWidth size, ValueTy &&ptr) noexcept;
  std::string getAssembly() const noexcept;
};

class InvalidNumOperandsError : public Error<InvalidNumOperandsError> {
private:
  std::string message;

public:
  InvalidNumOperandsError(const size_t expected, const size_t actual) noexcept;
  const char *what() const noexcept { return message.c_str(); }
};

class IntSumInst : public AbstractInst<IntSumInst> {
private:
  GeneralRegister target;
  ValueTy v1;
  ValueTy v2;
  ValueTy v3;
  ValueTy v4;
  ValueTy v5;
  ValueTy v6;
  ValueTy v7;
  ValueTy v8;
  BitWidth bw;
  IntSumInst(const GeneralRegister target, ValueTy &&v1, ValueTy &&v2,
             ValueTy &&v3, ValueTy &&v4, ValueTy &&v5, ValueTy &&v6,
             ValueTy &&v7, ValueTy &&v8, const BitWidth bw) noexcept;

public:
  static Result<IntSumInst, InvalidNumOperandsError>
  tryCreate(const GeneralRegister target, std::vector<ValueTy> &&operands,
            const BitWidth bw) noexcept;
  std::string getAssembly() const noexcept;
};

class IntIncrInst : public AbstractInst<IntIncrInst> {
private:
  GeneralRegister target;
  BitWidth bw;
  ValueTy arg;
  IntIncrInst(const GeneralRegister target, const BitWidth bw,  ValueTy &&arg) noexcept;

public:
  static IntIncrInst create(const GeneralRegister target,
                            const BitWidth bw,  ValueTy &&arg) noexcept;
  std::string getAssembly() const noexcept;
};

class IntDecrInst : public AbstractInst<IntDecrInst> {
private:
  GeneralRegister target;
  BitWidth bw;
  ValueTy arg;
  IntDecrInst(const GeneralRegister target, const BitWidth bw, ValueTy &&arg) noexcept;

public:
  static IntDecrInst create(const GeneralRegister target,
                            const BitWidth bw, ValueTy &&arg) noexcept;
  std::string getAssembly() const noexcept;
};
} // namespace sc::backend::assembly::inst
#endif // SC_LIB_BACKEND_ASSEMBLY_INST_H
