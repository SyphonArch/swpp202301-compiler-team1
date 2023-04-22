#include "inst.h"

#include "../../../result.h"

#include <algorithm>
#include <numeric>

using namespace std::string_literals;

namespace {
std::string joinTokens(std::vector<std::string> &&__tokens) noexcept {
  const auto collection_len = std::accumulate(
      __tokens.cbegin(), __tokens.cend(), static_cast<size_t>(0),
      [](const size_t len, const auto &line) -> size_t {
        return len + line.size() + 1; // trailing whitespace or colon
      });

  std::string joined_string;
  joined_string.reserve(collection_len);
  for (const auto &token : __tokens) {
    joined_string.append(token);
    joined_string.append(" "s);
  }

  // remove trailing whitespace
  joined_string.pop_back();
  return joined_string;
}

using namespace sc::backend::assembly::inst;

std::string getToken(const IcmpCondition __cond) noexcept {
  switch (__cond) {
  case IcmpCondition::EQ:
    return "eq"s;
  case IcmpCondition::NE:
    return "ne"s;
  case IcmpCondition::UGT:
    return "ugt"s;
  case IcmpCondition::UGE:
    return "uge"s;
  case IcmpCondition::ULT:
    return "ult"s;
  case IcmpCondition::ULE:
    return "ule"s;
  case IcmpCondition::SGT:
    return "sgt"s;
  case IcmpCondition::SGE:
    return "sge"s;
  case IcmpCondition::SLT:
    return "slt"s;
  case IcmpCondition::SLE:
    return "sle"s;
  default: // unreachable
    return ";"s;
  }
}

std::string getToken(const std::string_view __name) noexcept {
  return std::string(__name);
}

std::string getToken(const ValueTy &__value) noexcept {
  using super::int_t::getToken;
  using super::register_t::getToken;
  return std::visit([](const auto &val) { return getToken(val); }, __value);
}

template <typename T>
std::vector<std::string> __unpackOperandTokens(const T &__arg) noexcept {
  std::vector<std::string> tokens;
  tokens.reserve(4); // sufficient for most ops (except switch and call)

  tokens.push_back(getToken(__arg));
  return tokens;
}

template <typename T, typename... Ts>
std::vector<std::string> __unpackOperandTokens(const T &__arg,
                                               Ts... __args) noexcept {
  auto tokens = __unpackOperandTokens(__args...);
  tokens.push_back(getToken(__arg));
  return tokens;
}

template <typename... Ts>
std::vector<std::string> collectOperandTokens(Ts... __args) noexcept {
  auto operands_rev = __unpackOperandTokens(__args...);
  // operands are unpacked in reverse order
  std::reverse(operands_rev.begin(), operands_rev.end());
  return operands_rev;
}

template <typename... Ts>
std::vector<std::string> collectOpTokens(std::string &&__opname,
                                         Ts... __args) noexcept {
  auto operand_tokens = collectOperandTokens(__args...);

  std::vector<std::string> tokens = {std::move(__opname)};
  tokens.reserve(operand_tokens.size() + 1);
  std::move(operand_tokens.begin(), operand_tokens.end(),
            std::back_inserter(tokens));
  return tokens;
}

std::vector<std::string>
prependTarget(const GeneralRegister __target,
              std::vector<std::string> &&__tokens) noexcept {
  std::vector<std::string> tokens = {getToken(__target), "="s};
  tokens.reserve(tokens.size() + __tokens.size());
  std::move(__tokens.begin(), __tokens.end(), std::back_inserter(tokens));
  return tokens;
}

std::vector<std::string> getIntBinaryOpTokens(const GeneralRegister __target,
                                              std::string &&__op_name,
                                              const ValueTy &__lhs,
                                              const ValueTy &__rhs,
                                              const BitWidth __bw) noexcept {
  auto op_tokens = collectOpTokens(std::move(__op_name), __lhs, __rhs, __bw);
  return prependTarget(__target, std::move(op_tokens));
}

constexpr size_t max_argc = 16;
} // namespace

namespace sc::backend::assembly::inst {
//---------------------------------------------------------
// class FunctionStartInst
//---------------------------------------------------------
FunctionStartInst::FunctionStartInst(std::string &&__name,
                                     const IntTy __argc) noexcept
    : AbstractInst(), name(std::move(__name)), argc(__argc) {}

Result<FunctionStartInst, InvalidFunctionArgcError>
FunctionStartInst::tryCreate(std::string &&__name,
                             const IntTy __argc) noexcept {
  using RetType = Result<FunctionStartInst, InvalidFunctionArgcError>;

  if (__argc > max_argc) {
    return RetType::Err(InvalidFunctionArgcError());
  }

  return RetType::Ok(inst::FunctionStartInst(std::move(__name), __argc));
}

std::string FunctionStartInst::getAssembly() const noexcept {
  auto joined_tokens = joinTokens(collectOpTokens("start"s, name, argc));
  joined_tokens.append(":"s);
  return joined_tokens;
}

//---------------------------------------------------------
// class FunctionEndInst
//---------------------------------------------------------
FunctionEndInst::FunctionEndInst(std::string &&__name) noexcept
    : AbstractInst(), name(std::move(__name)) {}

FunctionEndInst FunctionEndInst::create(std::string &&__name) noexcept {
  return FunctionEndInst(std::move(__name));
}

std::string FunctionEndInst::getAssembly() const noexcept {
  return joinTokens(collectOpTokens("end"s, name));
}

//---------------------------------------------------------
// class BasicBlockInst
//---------------------------------------------------------
BasicBlockInst::BasicBlockInst(std::string &&__name) noexcept : AbstractInst() {
  name = "."s;
  name.reserve(__name.size() + 1);
  std::move(__name.begin(), __name.end(), std::back_inserter(name));
}

BasicBlockInst BasicBlockInst::create(std::string &&__name) noexcept {
  return BasicBlockInst(std::move(__name));
}

std::string BasicBlockInst::getAssembly() const noexcept { return name + ":"s; }

//---------------------------------------------------------
// class CommentInst
//---------------------------------------------------------
CommentInst::CommentInst(std::string &&__message) noexcept
    : AbstractInst(), message(std::move(__message)) {}

CommentInst CommentInst::create(std::string &&__message) noexcept {
  return CommentInst(std::move(__message));
}

std::string CommentInst::getAssembly() const noexcept {
  return joinTokens(collectOpTokens(";"s, message));
}

//---------------------------------------------------------
// class ReturnInst
//---------------------------------------------------------
ReturnInst::ReturnInst() noexcept
    : AbstractInst(), value(static_cast<IntTy>(0)) {}

ReturnInst::ReturnInst(ValueTy &&__value) noexcept
    : AbstractInst(), value(std::move(__value)) {}

ReturnInst ReturnInst::createVoid() noexcept { return ReturnInst(); }

ReturnInst ReturnInst::create(ValueTy &&__value) noexcept {
  return ReturnInst(std::move(__value));
}

std::string ReturnInst::getAssembly() const noexcept {
  return joinTokens(collectOpTokens("ret"s, value));
}

//---------------------------------------------------------
// class JumpInst
//---------------------------------------------------------
JumpInst::JumpInst(std::string &&__label) noexcept
    : AbstractInst(), label(std::move(__label)) {}

JumpInst JumpInst::create(std::string &&__label) noexcept {
  return JumpInst(std::move(__label));
}

std::string JumpInst::getAssembly() const noexcept {
  return joinTokens(collectOpTokens("br"s, "."s.append(label)));
}

//---------------------------------------------------------
// class BranchInst
//---------------------------------------------------------
BranchInst::BranchInst(ValueTy &&__condition, std::string &&__label_true,
                       std::string &&__label_false) noexcept
    : AbstractInst(), condition(std::move(__condition)),
      label_true(std::move(__label_true)),
      label_false(std::move(__label_false)) {}

BranchInst BranchInst::create(ValueTy &&__condition, std::string &&__label_true,
                              std::string &&__label_false) noexcept {
  return BranchInst(std::move(__condition), std::move(__label_true),
                    std::move(__label_false));
}

std::string BranchInst::getAssembly() const noexcept {
  return joinTokens(collectOpTokens("br"s, condition, "."s.append(label_true), "."s.append(label_false)));
}

//---------------------------------------------------------
// class SwitchInst
//---------------------------------------------------------
SwitchInst::SwitchInst(ValueTy &&__condition,
                       std::string &&__label_default) noexcept
    : AbstractInst(), condition(std::move(__condition)),
      label_default(std::move(__label_default)) {}

void SwitchInst::addCase(const IntTy __value, std::string &&__label) noexcept {
  cases.insert({__value, std::move(__label)});
}

Result<SwitchInst, SwitchInst::MismatchingCaseError>
SwitchInst::tryCreate(ValueTy &&__cond, std::vector<IntTy> &&__cases,
                      std::vector<std::string> &&__labels,
                      std::string &&__label_default) noexcept {
  using MismatchingCaseError = SwitchInst::MismatchingCaseError;
  using RetType = Result<SwitchInst, MismatchingCaseError>;

  if (__cases.size() != __labels.size()) {
    return RetType::Err(MismatchingCaseError());
  }

  auto inst = SwitchInst(std::move(__cond), std::move(__label_default));
  for (auto cs = __cases.crbegin(); cs != __cases.crend(); cs++) {
    inst.addCase(*cs, std::move(__labels.back()));
    __labels.pop_back();
  }

  return RetType::Ok(std::move(inst));
}

std::string SwitchInst::getAssembly() const noexcept {
  auto tokens = collectOpTokens("switch"s, condition);

  tokens.reserve(tokens.size() + cases.size() * 2 + 1);
  for (const auto &case_pair : cases) {
    const auto cs = case_pair.first;
    const auto label = case_pair.second;

    auto case_tokens = collectOperandTokens(cs, std::move("."s.append(label)));
    std::move(case_tokens.begin(), case_tokens.end(),
              std::back_inserter(tokens));
  }

  tokens.push_back("."s.append(label_default));
  return joinTokens(std::move(tokens));
}

//---------------------------------------------------------
// class MallocInst
//---------------------------------------------------------
MallocInst::MallocInst(const GeneralRegister __target,
                       ValueTy &&__size) noexcept
    : AbstractInst(), target(__target), size(std::move(__size)) {}

MallocInst MallocInst::create(const GeneralRegister __target,
                              ValueTy &&__size) noexcept {
  return MallocInst(__target, std::move(__size));
}

std::string MallocInst::getAssembly() const noexcept {
  return joinTokens(prependTarget(target, collectOpTokens("malloc"s, size)));
}

//---------------------------------------------------------
// class FreeInst
//---------------------------------------------------------
FreeInst::FreeInst(ValueTy &&__ptr) noexcept
    : AbstractInst(), ptr(std::move(__ptr)) {}

FreeInst FreeInst::create(ValueTy &&__ptr) noexcept {
  return FreeInst(std::move(__ptr));
}

std::string FreeInst::getAssembly() const noexcept {
  return joinTokens(collectOpTokens("free"s, ptr));
}

//---------------------------------------------------------
// class LoadInst
//---------------------------------------------------------
LoadInst::LoadInst(const GeneralRegister __target, const AccessWidth __size,
                   ValueTy &&__ptr) noexcept
    : AbstractInst(), target(__target), size(__size), ptr(std::move(__ptr)) {}

LoadInst LoadInst::create(const GeneralRegister __target,
                          const AccessWidth __size, ValueTy &&__ptr) noexcept {
  return LoadInst(__target, __size, std::move(__ptr));
}

std::string LoadInst::getAssembly() const noexcept {
  return joinTokens(prependTarget(target, collectOpTokens("load"s, size, ptr)));
}

//---------------------------------------------------------
// class StoreInst
//---------------------------------------------------------
StoreInst::StoreInst(const AccessWidth __size, ValueTy &&__value,
                     ValueTy &&__ptr) noexcept
    : AbstractInst(), size(__size), value(std::move(__value)),
      ptr(std::move(__ptr)) {}

StoreInst StoreInst::create(const AccessWidth __size, ValueTy &&__value,
                            ValueTy &&__ptr) noexcept {
  return StoreInst(__size, std::move(__value), std::move(__ptr));
}

std::string StoreInst::getAssembly() const noexcept {
  return joinTokens(collectOpTokens("store", size, value, ptr));
}

//---------------------------------------------------------
// class IntAddInst
//---------------------------------------------------------
IntAddInst::IntAddInst(const GeneralRegister __target, const BitWidth __bw,
                       ValueTy &&__lhs, ValueTy &&__rhs) noexcept
    : IntBinaryOpInst(__target, __bw, std::move(__lhs), std::move(__rhs)) {}

IntAddInst IntAddInst::create(const GeneralRegister __target,
                              const BitWidth __bw, ValueTy &&__lhs,
                              ValueTy &&__rhs) noexcept {
  return IntAddInst(__target, __bw, std::move(__lhs), std::move(__rhs));
}

std::string IntAddInst::getAssembly() const noexcept {
  return joinTokens(getIntBinaryOpTokens(target, "add"s, lhs, rhs, bw));
}

//---------------------------------------------------------
// class IntSubInst
//---------------------------------------------------------
IntSubInst::IntSubInst(const GeneralRegister __target, const BitWidth __bw,
                       ValueTy &&__lhs, ValueTy &&__rhs) noexcept
    : IntBinaryOpInst(__target, __bw, std::move(__lhs), std::move(__rhs)) {}

IntSubInst IntSubInst::create(const GeneralRegister __target,
                              const BitWidth __bw, ValueTy &&__lhs,
                              ValueTy &&__rhs) noexcept {
  return IntSubInst(__target, __bw, std::move(__lhs), std::move(__rhs));
}

std::string IntSubInst::getAssembly() const noexcept {
  return joinTokens(getIntBinaryOpTokens(target, "sub"s, lhs, rhs, bw));
}

//---------------------------------------------------------
// class IntMulInst
//---------------------------------------------------------
IntMulInst::IntMulInst(const GeneralRegister __target, const BitWidth __bw,
                       ValueTy &&__lhs, ValueTy &&__rhs) noexcept
    : IntBinaryOpInst(__target, __bw, std::move(__lhs), std::move(__rhs)) {}

IntMulInst IntMulInst::create(const GeneralRegister __target,
                              const BitWidth __bw, ValueTy &&__lhs,
                              ValueTy &&__rhs) noexcept {
  return IntMulInst(__target, __bw, std::move(__lhs), std::move(__rhs));
}

std::string IntMulInst::getAssembly() const noexcept {
  return joinTokens(getIntBinaryOpTokens(target, "mul"s, lhs, rhs, bw));
}

//---------------------------------------------------------
// class IntUDivInst
//---------------------------------------------------------
IntUDivInst::IntUDivInst(const GeneralRegister __target, const BitWidth __bw,
                         ValueTy &&__lhs, ValueTy &&__rhs) noexcept
    : IntBinaryOpInst(__target, __bw, std::move(__lhs), std::move(__rhs)) {}

IntUDivInst IntUDivInst::create(const GeneralRegister __target,
                                const BitWidth __bw, ValueTy &&__lhs,
                                ValueTy &&__rhs) noexcept {
  return IntUDivInst(__target, __bw, std::move(__lhs), std::move(__rhs));
}

std::string IntUDivInst::getAssembly() const noexcept {
  return joinTokens(getIntBinaryOpTokens(target, "udiv"s, lhs, rhs, bw));
}

//---------------------------------------------------------
// class IntSDivInst
//---------------------------------------------------------
IntSDivInst::IntSDivInst(const GeneralRegister __target, const BitWidth __bw,
                         ValueTy &&__lhs, ValueTy &&__rhs) noexcept
    : IntBinaryOpInst(__target, __bw, std::move(__lhs), std::move(__rhs)) {}

IntSDivInst IntSDivInst::create(const GeneralRegister __target,
                                const BitWidth __bw, ValueTy &&__lhs,
                                ValueTy &&__rhs) noexcept {
  return IntSDivInst(__target, __bw, std::move(__lhs), std::move(__rhs));
}

std::string IntSDivInst::getAssembly() const noexcept {
  return joinTokens(getIntBinaryOpTokens(target, "sdiv"s, lhs, rhs, bw));
}

//---------------------------------------------------------
// class IntURemInst
//---------------------------------------------------------
IntURemInst::IntURemInst(const GeneralRegister __target, const BitWidth __bw,
                         ValueTy &&__lhs, ValueTy &&__rhs) noexcept
    : IntBinaryOpInst(__target, __bw, std::move(__lhs), std::move(__rhs)) {}

IntURemInst IntURemInst::create(const GeneralRegister __target,
                                const BitWidth __bw, ValueTy &&__lhs,
                                ValueTy &&__rhs) noexcept {
  return IntURemInst(__target, __bw, std::move(__lhs), std::move(__rhs));
}

std::string IntURemInst::getAssembly() const noexcept {
  return joinTokens(getIntBinaryOpTokens(target, "urem"s, lhs, rhs, bw));
}

//---------------------------------------------------------
// class IntSRemInst
//---------------------------------------------------------
IntSRemInst::IntSRemInst(const GeneralRegister __target, const BitWidth __bw,
                         ValueTy &&__lhs, ValueTy &&__rhs) noexcept
    : IntBinaryOpInst(__target, __bw, std::move(__lhs), std::move(__rhs)) {}

IntSRemInst IntSRemInst::create(const GeneralRegister __target,
                                const BitWidth __bw, ValueTy &&__lhs,
                                ValueTy &&__rhs) noexcept {
  return IntSRemInst(__target, __bw, std::move(__lhs), std::move(__rhs));
}

std::string IntSRemInst::getAssembly() const noexcept {
  return joinTokens(getIntBinaryOpTokens(target, "srem"s, lhs, rhs, bw));
}

//---------------------------------------------------------
// class IntAndInst
//---------------------------------------------------------
IntAndInst::IntAndInst(const GeneralRegister __target, const BitWidth __bw,
                       ValueTy &&__lhs, ValueTy &&__rhs) noexcept
    : IntBinaryOpInst(__target, __bw, std::move(__lhs), std::move(__rhs)) {}

IntAndInst IntAndInst::create(const GeneralRegister __target,
                              const BitWidth __bw, ValueTy &&__lhs,
                              ValueTy &&__rhs) noexcept {
  return IntAndInst(__target, __bw, std::move(__lhs), std::move(__rhs));
}

std::string IntAndInst::getAssembly() const noexcept {
  return joinTokens(getIntBinaryOpTokens(target, "and"s, lhs, rhs, bw));
}

//---------------------------------------------------------
// class IntOrInst
//---------------------------------------------------------
IntOrInst::IntOrInst(const GeneralRegister __target, const BitWidth __bw,
                     ValueTy &&__lhs, ValueTy &&__rhs) noexcept
    : IntBinaryOpInst(__target, __bw, std::move(__lhs), std::move(__rhs)) {}

IntOrInst IntOrInst::create(const GeneralRegister __target, const BitWidth __bw,
                            ValueTy &&__lhs, ValueTy &&__rhs) noexcept {
  return IntOrInst(__target, __bw, std::move(__lhs), std::move(__rhs));
}

std::string IntOrInst::getAssembly() const noexcept {
  return joinTokens(getIntBinaryOpTokens(target, "or"s, lhs, rhs, bw));
}

//---------------------------------------------------------
// class IntXorInst
//---------------------------------------------------------
IntXorInst::IntXorInst(const GeneralRegister __target, const BitWidth __bw,
                       ValueTy &&__lhs, ValueTy &&__rhs) noexcept
    : IntBinaryOpInst(__target, __bw, std::move(__lhs), std::move(__rhs)) {}

IntXorInst IntXorInst::create(const GeneralRegister __target,
                              const BitWidth __bw, ValueTy &&__lhs,
                              ValueTy &&__rhs) noexcept {
  return IntXorInst(__target, __bw, std::move(__lhs), std::move(__rhs));
}

std::string IntXorInst::getAssembly() const noexcept {
  return joinTokens(getIntBinaryOpTokens(target, "xor"s, lhs, rhs, bw));
}

//---------------------------------------------------------
// class IntShlInst
//---------------------------------------------------------
IntShlInst::IntShlInst(const GeneralRegister __target, const BitWidth __bw,
                       ValueTy &&__lhs, ValueTy &&__rhs) noexcept
    : IntBinaryOpInst(__target, __bw, std::move(__lhs), std::move(__rhs)) {}

IntShlInst IntShlInst::create(const GeneralRegister __target,
                              const BitWidth __bw, ValueTy &&__lhs,
                              ValueTy &&__rhs) noexcept {
  return IntShlInst(__target, __bw, std::move(__lhs), std::move(__rhs));
}

std::string IntShlInst::getAssembly() const noexcept {
  return joinTokens(getIntBinaryOpTokens(target, "shl"s, lhs, rhs, bw));
}

//---------------------------------------------------------
// class IntLShrInst
//---------------------------------------------------------
IntLShrInst::IntLShrInst(const GeneralRegister __target, const BitWidth __bw,
                         ValueTy &&__lhs, ValueTy &&__rhs) noexcept
    : IntBinaryOpInst(__target, __bw, std::move(__lhs), std::move(__rhs)) {}

IntLShrInst IntLShrInst::create(const GeneralRegister __target,
                                const BitWidth __bw, ValueTy &&__lhs,
                                ValueTy &&__rhs) noexcept {
  return IntLShrInst(__target, __bw, std::move(__lhs), std::move(__rhs));
}

std::string IntLShrInst::getAssembly() const noexcept {
  return joinTokens(getIntBinaryOpTokens(target, "lshr"s, lhs, rhs, bw));
}

//---------------------------------------------------------
// class IntAShrInst
//---------------------------------------------------------
IntAShrInst::IntAShrInst(const GeneralRegister __target, const BitWidth __bw,
                         ValueTy &&__lhs, ValueTy &&__rhs) noexcept
    : IntBinaryOpInst(__target, __bw, std::move(__lhs), std::move(__rhs)) {}

IntAShrInst IntAShrInst::create(const GeneralRegister __target,
                                const BitWidth __bw, ValueTy &&__lhs,
                                ValueTy &&__rhs) noexcept {
  return IntAShrInst(__target, __bw, std::move(__lhs), std::move(__rhs));
}

std::string IntAShrInst::getAssembly() const noexcept {
  return joinTokens(getIntBinaryOpTokens(target, "ashr"s, lhs, rhs, bw));
}

//---------------------------------------------------------
// class IntCompInst
//---------------------------------------------------------
IntCompInst::IntCompInst(const GeneralRegister __target,
                         const IntCompInst::Condition __cond,
                         const BitWidth __bw, ValueTy &&__lhs,
                         ValueTy &&__rhs) noexcept
    : IntBinaryOpInst(__target, __bw, std::move(__lhs), std::move(__rhs)),
      cond(__cond) {}

IntCompInst IntCompInst::create(const GeneralRegister __target,
                                const IntCompInst::Condition __cond,
                                const BitWidth __bw, ValueTy &&__lhs,
                                ValueTy &&__rhs) noexcept {
  return IntCompInst(__target, __cond, __bw, std::move(__lhs),
                     std::move(__rhs));
}

std::string IntCompInst::getAssembly() const noexcept {
  return joinTokens(
      prependTarget(target, collectOpTokens("icmp"s, cond, lhs, rhs, bw)));
}

//---------------------------------------------------------
// class SelectInst
//---------------------------------------------------------
SelectInst::SelectInst(const GeneralRegister __target, ValueTy &&__cond,
                       ValueTy &&__lhs, ValueTy &&__rhs) noexcept
    : AbstractInst(), target(__target), cond(std::move(__cond)),
      lhs(std::move(__lhs)), rhs(std::move(__rhs)) {}

SelectInst SelectInst::create(const GeneralRegister __target, ValueTy &&__cond,
                              ValueTy &&__lhs, ValueTy &&__rhs) noexcept {
  return SelectInst(__target, std::move(__cond), std::move(__lhs),
                    std::move(__rhs));
}

std::string SelectInst::getAssembly() const noexcept {
  return joinTokens(
      prependTarget(target, collectOpTokens("select"s, cond, lhs, rhs)));
}

//---------------------------------------------------------
// class CallInst
//---------------------------------------------------------
CallInst::CallInst(std::string &&__name) noexcept
    : AbstractInst(), name(std::move(__name)) {}

CallInst::CallInst(const GeneralRegister __target,
                   std::string &&__name) noexcept
    : CallInst(std::move(__name)) {
  target = __target;
}

void CallInst::addArg(ValueTy &&__arg) noexcept {
  args.push_back(std::move(__arg));
}

Result<CallInst, InvalidFunctionArgcError>
CallInst::tryCreateDiscarding(std::string &&__name,
                              std::vector<ValueTy> &&__args) noexcept {
  using RetType = Result<CallInst, InvalidFunctionArgcError>;
  if (__args.size() > max_argc) {
    return RetType::Err(InvalidFunctionArgcError());
  }

  auto inst = CallInst(std::move(__name));
  for (auto arg = std::make_move_iterator(__args.begin());
       arg != std::make_move_iterator(__args.end()); arg++) {
    inst.addArg(*arg);
  }

  return RetType::Ok(std::move(inst));
}

Result<CallInst, InvalidFunctionArgcError>
CallInst::tryCreateReturning(const GeneralRegister __target,
                             std::string &&__name,
                             std::vector<ValueTy> &&__args) noexcept {
  using RetType = Result<CallInst, InvalidFunctionArgcError>;
  if (__args.size() > max_argc) {
    return RetType::Err(InvalidFunctionArgcError());
  }

  auto inst = CallInst(__target, std::move(__name));
  for (auto arg = std::make_move_iterator(__args.begin());
       arg != std::make_move_iterator(__args.end()); arg++) {
    inst.addArg(*arg);
  }

  return RetType::Ok(std::move(inst));
}

std::string CallInst::getAssembly() const noexcept {
  auto tokens = collectOpTokens("call", name);
  if (target.has_value()) {
    tokens = prependTarget(*target, std::move(tokens));
  }

  tokens.reserve(tokens.size() + args.size());
  for (const auto &arg : args) {
    tokens.push_back(getToken(arg));
  }

  return joinTokens(std::move(tokens));
}

//---------------------------------------------------------
// class AssertEqInst
//---------------------------------------------------------
AssertEqInst::AssertEqInst(ValueTy &&__lhs, ValueTy &&__rhs) noexcept
    : AbstractInst(), lhs(std::move(__lhs)), rhs(std::move(__rhs)) {}

AssertEqInst AssertEqInst::create(ValueTy &&__lhs, ValueTy &&__rhs) noexcept {
  return AssertEqInst(std::move(__lhs), std::move(__rhs));
}

std::string AssertEqInst::getAssembly() const noexcept {
  return joinTokens(collectOpTokens("assert_eq"s, lhs, rhs));
}

//---------------------------------------------------------
// class AsyncLoadInst
//---------------------------------------------------------
AsyncLoadInst::AsyncLoadInst(const GeneralRegister __target,
                             const AccessWidth __size, ValueTy &&__ptr) noexcept
    : AbstractInst(), target(__target), size(__size), ptr(std::move(__ptr)) {}

AsyncLoadInst AsyncLoadInst::create(const GeneralRegister __target,
                                    const AccessWidth __size,
                                    ValueTy &&__ptr) noexcept {
  return AsyncLoadInst(__target, __size, std::move(__ptr));
}

std::string AsyncLoadInst::getAssembly() const noexcept {
  return joinTokens(
      prependTarget(target, collectOpTokens("aload"s, size, ptr)));
}

//---------------------------------------------------------
// class InvalidNumOperandsError
//---------------------------------------------------------
InvalidNumOperandsError::InvalidNumOperandsError(
    const size_t __expected, const size_t __actual) noexcept {
  message = "Expected "s.append(std::to_string(__expected))
                .append(" operands; got "s)
                .append(std::to_string(__actual));
}

//---------------------------------------------------------
// class IntSumInst
//---------------------------------------------------------
IntSumInst::IntSumInst(const GeneralRegister __target, ValueTy &&__v1,
                       ValueTy &&__v2, ValueTy &&__v3, ValueTy &&__v4,
                       ValueTy &&__v5, ValueTy &&__v6, ValueTy &&__v7,
                       ValueTy &&__v8, const BitWidth __bw) noexcept
    : AbstractInst(), target(__target), v1(std::move(__v1)),
      v2(std::move(__v2)), v3(std::move(__v3)), v4(std::move(__v4)),
      v5(std::move(__v5)), v6(std::move(__v6)), v7(std::move(__v7)),
      v8(std::move(__v8)), bw(__bw) {}

Result<IntSumInst, InvalidNumOperandsError>
IntSumInst::tryCreate(const GeneralRegister __target,
                      std::vector<ValueTy> &&__operands,
                      const BitWidth __bw) noexcept {
  using RetType = Result<IntSumInst, InvalidNumOperandsError>;

  constexpr size_t allowed_num_operands = 8;
  const size_t num_operands = __operands.size();
  if (num_operands != allowed_num_operands) {
    return RetType::Err(
        InvalidNumOperandsError(allowed_num_operands, num_operands));
  }

  auto v1 = std::move(__operands[0]);
  auto v2 = std::move(__operands[1]);
  auto v3 = std::move(__operands[2]);
  auto v4 = std::move(__operands[3]);
  auto v5 = std::move(__operands[4]);
  auto v6 = std::move(__operands[5]);
  auto v7 = std::move(__operands[6]);
  auto v8 = std::move(__operands[7]);

  return RetType::Ok(IntSumInst(
      __target, std::move(v1), std::move(v2), std::move(v3), std::move(v4),
      std::move(v5), std::move(v6), std::move(v7), std::move(v8), __bw));
}

std::string IntSumInst::getAssembly() const noexcept {
  return joinTokens(prependTarget(
      target, collectOpTokens("sum"s, v1, v2, v3, v4, v5, v6, v7, v8, bw)));
}

//---------------------------------------------------------
// class IntIncrInst
//---------------------------------------------------------
IntIncrInst::IntIncrInst(const GeneralRegister __target,
                         const BitWidth __bw, ValueTy &&__arg) noexcept
    : AbstractInst(), target(__target), bw(__bw), arg(std::move(__arg)) {}

IntIncrInst IntIncrInst::create(const GeneralRegister __target,
                                const BitWidth __bw, ValueTy &&__arg) noexcept {
  return IntIncrInst(__target, __bw, std::move(__arg));
}

std::string IntIncrInst::getAssembly() const noexcept {
  return joinTokens(prependTarget(target, collectOpTokens("incr"s, arg, bw)));
}

//---------------------------------------------------------
// class IntDecrInst
//---------------------------------------------------------
IntDecrInst::IntDecrInst(const GeneralRegister __target,
                         const BitWidth __bw, ValueTy &&__arg) noexcept
    : AbstractInst(), target(__target), bw(__bw), arg(std::move(__arg)) {}

IntDecrInst IntDecrInst::create(const GeneralRegister __target,
                                const BitWidth __bw, ValueTy &&__arg) noexcept {
  return IntDecrInst(__target, __bw, std::move(__arg));
}

std::string IntDecrInst::getAssembly() const noexcept {
  return joinTokens(prependTarget(target, collectOpTokens("decr"s, arg, bw)));
}
} // namespace sc::backend::assembly::inst
