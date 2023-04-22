#include "emitter.h"

#include "assembly.h"

#include <charconv>
#include <map>
#include <numeric>
#include <optional>

using namespace std::string_literals;

namespace {
const sc::backend::symbol::SymbolMap *__SM;

using namespace sc::backend::assembly;

std::string collectStrings(const std::vector<std::string> &__lines) noexcept {
  const auto collection_len =
      std::accumulate(__lines.cbegin(), __lines.cend(), (size_t)0,
                      [](const size_t len, const auto &line) -> size_t {
                        // +1 for \n
                        return len + line.size() + 1;
                      });

  std::string collected_string;
  collected_string.reserve(collection_len);
  for (const auto &line : __lines) {
    collected_string.append(line + "\n"s);
  }

  return collected_string;
}

class UnresolvedSymbolError : public Error<UnresolvedSymbolError> {
private:
  std::string message;

public:
  UnresolvedSymbolError(llvm::Value *const __value) noexcept {
    message = "unresolved symbol error: "s.append(__value->getName())
                  .append(" does not exist in the symbol map"s);
  }

  const char *what() const noexcept { return message.c_str(); }
};

Result<std::string, UnresolvedSymbolError>
tryGetName(llvm::Value *const __value) noexcept {
  using namespace llvm;
  using RetType = Result<std::string, UnresolvedSymbolError>;

  if (!__value || isa<ConstantPointerNull>(__value) ||
      __value->getType()->isVoidTy()) {
    return RetType::Ok("0"s);
  } else if (isa<ConstantInt>(__value)) {
    // return the value itself.
    return RetType::Ok(
        std::to_string(dyn_cast<ConstantInt>(__value)->getZExtValue()));
  } else {
    const auto sym_ptr = __SM->getSymbol(__value);
    if (!sym_ptr) {
      return RetType::Err(UnresolvedSymbolError(__value));
    } else {
      return RetType::Ok(sym_ptr->getName());
    }
  }
}

class CalculateBitWidthError : public Error<CalculateBitWidthError> {
private:
  std::string message;

public:
  CalculateBitWidthError(llvm::Type *const __ty) noexcept {
    message = "bitwidth calculating error: unable to calculate Bitwidth from "s
                  .append(__ty->getStructName());
  }

  const char *what() const noexcept { return message.c_str(); }
};

Result<BitWidth, CalculateBitWidthError>
tryCalculateBitWidth(llvm::Type *const __ty) noexcept {
  using RetType = Result<BitWidth, CalculateBitWidthError>;

  if (__ty->isPointerTy()) {
    return RetType::Ok(BitWidth::QWORD);
  } else if (!__ty->isIntegerTy()) {
    return RetType::Err(CalculateBitWidthError(__ty));
  }

  switch (__ty->getIntegerBitWidth()) {
  case 1:
    return RetType::Ok(BitWidth::BIT);
  case 8:
    return RetType::Ok(BitWidth::BYTE);
  case 16:
    return RetType::Ok(BitWidth::WORD);
  case 32:
    return RetType::Ok(BitWidth::DWORD);
  case 64:
    return RetType::Ok(BitWidth::QWORD);
  default:
    return RetType::Err(CalculateBitWidthError(__ty));
  }
}

class CalculateAccessWidthError : public Error<CalculateAccessWidthError> {
private:
  std::string message;

public:
  CalculateAccessWidthError(llvm::Type *const __ty) noexcept {
    message =
        "access width calculating error: unable to calculate AccessWidth from "s
            .append(__ty->getStructName());
  }

  const char *what() const noexcept { return message.c_str(); }
};

Result<AccessWidth, CalculateAccessWidthError>
tryCalculateAccessWidth(llvm::Type *const __ty) noexcept {
  using RetType = Result<AccessWidth, CalculateAccessWidthError>;

  if (!__ty->isPointerTy()) {
    return RetType::Err(CalculateAccessWidthError(__ty));
  }

  // handle ptr of ptr
  const auto pety = __ty->getPointerElementType();
  if (pety->isPointerTy()) {
    return RetType::Ok(AccessWidth::QWORD);
  }

  switch (pety->getIntegerBitWidth()) {
  case 8:
    return RetType::Ok(AccessWidth::BYTE);
  case 16:
    return RetType::Ok(AccessWidth::WORD);
  case 32:
    return RetType::Ok(AccessWidth::DWORD);
  case 64:
    return RetType::Ok(AccessWidth::QWORD);
  default:
    return RetType::Err(CalculateAccessWidthError(__ty));
  }
}

const std::map<std::string, IcmpCondition, std::less<>> icmp_table = {
    {"eq"s, IcmpCondition::EQ},   {"ne"s, IcmpCondition::NE},
    {"ugt"s, IcmpCondition::UGT}, {"uge"s, IcmpCondition::UGE},
    {"ult"s, IcmpCondition::ULT}, {"ule"s, IcmpCondition::ULE},
    {"sgt"s, IcmpCondition::SGT}, {"sge"s, IcmpCondition::SGE},
    {"slt"s, IcmpCondition::SLT}, {"sle"s, IcmpCondition::SLE},
};

class ParseIcmpConditionError : public Error<ParseIcmpConditionError> {
private:
  std::string message;

public:
  ParseIcmpConditionError(const std::string_view __op_str) noexcept {
    message = "icmp condition parsing error: unable to parse "s.append(__op_str)
                  .append(" into IcmpCondition"s);
  }

  const char *what() const noexcept { return message.c_str(); }
};

Result<IcmpCondition, ParseIcmpConditionError>
tryParseIcmpCondition(const std::string_view __op_str) noexcept {
  using RetType = Result<IcmpCondition, ParseIcmpConditionError>;

  const auto itr = icmp_table.find(__op_str);
  if (itr == icmp_table.cend()) {
    return RetType::Err(ParseIcmpConditionError(__op_str));
  }
  return RetType::Ok(itr->second);
}

const std::map<std::string, GeneralRegister, std::less<>> general_reg_table = {
    {"r1"s, GeneralRegister::R1},   {"r2"s, GeneralRegister::R2},
    {"r3"s, GeneralRegister::R3},   {"r4"s, GeneralRegister::R4},
    {"r5"s, GeneralRegister::R5},   {"r6"s, GeneralRegister::R6},
    {"r7"s, GeneralRegister::R7},   {"r8"s, GeneralRegister::R8},
    {"r9"s, GeneralRegister::R9},   {"r10"s, GeneralRegister::R10},
    {"r11"s, GeneralRegister::R11}, {"r12"s, GeneralRegister::R12},
    {"r13"s, GeneralRegister::R13}, {"r14"s, GeneralRegister::R14},
    {"r15"s, GeneralRegister::R15}, {"r16"s, GeneralRegister::R16},
    {"r17"s, GeneralRegister::R17}, {"r18"s, GeneralRegister::R18},
    {"r19"s, GeneralRegister::R19}, {"r20"s, GeneralRegister::R20},
    {"r21"s, GeneralRegister::R21}, {"r22"s, GeneralRegister::R22},
    {"r23"s, GeneralRegister::R23}, {"r24"s, GeneralRegister::R24},
    {"r25"s, GeneralRegister::R25}, {"r26"s, GeneralRegister::R26},
    {"r27"s, GeneralRegister::R27}, {"r28"s, GeneralRegister::R28},
    {"r29"s, GeneralRegister::R29}, {"r30"s, GeneralRegister::R30},
    {"r31"s, GeneralRegister::R31}, {"r32"s, GeneralRegister::R32},
    {"sp"s, GeneralRegister::SP}};

class ParseGeneralRegisterError : public Error<ParseGeneralRegisterError> {
private:
  std::string message;

public:
  ParseGeneralRegisterError(const std::string_view __vreg_str) noexcept {
    message =
        "general register parsing error: unable to parse "s.append(__vreg_str)
            .append(" into GeneralRegister"s);
  }

  const char *what() const noexcept { return message.c_str(); }
};

Result<GeneralRegister, ParseGeneralRegisterError>
tryParseGeneralRegister(const std::string_view __vreg_str) noexcept {
  using RetType = Result<GeneralRegister, ParseGeneralRegisterError>;

  const auto itr = general_reg_table.find(__vreg_str);
  if (itr == general_reg_table.cend()) {
    return RetType::Err(ParseGeneralRegisterError(__vreg_str));
  }

  return RetType::Ok(itr->second);
}

const std::map<std::string, ArgumentRegister, std::less<>> argument_reg_table =
    {
        {"arg1"s, ArgumentRegister::A1},   {"arg2"s, ArgumentRegister::A2},
        {"arg3"s, ArgumentRegister::A3},   {"arg4"s, ArgumentRegister::A4},
        {"arg5"s, ArgumentRegister::A5},   {"arg6"s, ArgumentRegister::A6},
        {"arg7"s, ArgumentRegister::A7},   {"arg8"s, ArgumentRegister::A8},
        {"arg9"s, ArgumentRegister::A9},   {"arg10"s, ArgumentRegister::A10},
        {"arg11"s, ArgumentRegister::A11}, {"arg12"s, ArgumentRegister::A12},
        {"arg13"s, ArgumentRegister::A13}, {"arg14"s, ArgumentRegister::A14},
        {"arg15"s, ArgumentRegister::A15}, {"arg16"s, ArgumentRegister::A16},
};

class ParseArgumentRegisterError : public Error<ParseArgumentRegisterError> {
private:
  std::string message;

public:
  ParseArgumentRegisterError(const std::string_view __vreg_str) noexcept {
    message =
        "argument register parsing error: unable to parse "s.append(__vreg_str)
            .append(" into ArgumentRegister"s);
  }

  const char *what() const noexcept { return message.c_str(); }
};

Result<ArgumentRegister, ParseArgumentRegisterError>
tryParseArgumentRegister(const std::string_view __vreg_str) noexcept {
  using RetType = Result<ArgumentRegister, ParseArgumentRegisterError>;

  const auto itr = argument_reg_table.find(__vreg_str);
  if (itr == argument_reg_table.cend()) {
    return RetType::Err(ParseArgumentRegisterError(__vreg_str));
  }

  return RetType::Ok(itr->second);
}

class ParseIntError : public Error<ParseIntError> {
private:
  std::string message;

public:
  ParseIntError(const std::string_view __int_str) noexcept {
    message = "int parsing error: unable to parse "s.append(__int_str).append(
        " into IntTy"s);
  }

  const char *what() const noexcept { return message.c_str(); }
};

Result<IntTy, ParseIntError> tryParseInt(const std::string_view __int_str) {
  using RetType = Result<IntTy, ParseIntError>;

  const auto str_ptr = __int_str.data();
  IntTy value;
  const auto fc_res =
      std::from_chars(str_ptr, str_ptr + __int_str.size(), value);
  if (fc_res.ptr != str_ptr + __int_str.size()) {
    return RetType::Err(ParseIntError(__int_str));
  }

  return RetType::Ok(value);
}

class ParseValueError : public Error<ParseValueError> {
private:
  std::string message;

public:
  ParseValueError(const std::string_view __value_str) noexcept {
    message = "value parsing error: unable to parse "s.append(__value_str)
                  .append(" into any of the ValueTy"s);
  }

  ParseValueError(ParseGeneralRegisterError &&__err) noexcept {
    message = "value parsing error: failed to parse value expected to be "s
              "GeneralRegister\n("s.append(__err.what())
                  .append(")"s);
  }

  ParseValueError(ParseArgumentRegisterError &&__err) noexcept {
    message = "value parsing error: failed to parse value expected to be "s
              "ArgumentRegister\n("s.append(__err.what())
                  .append(")"s);
  }

  ParseValueError(ParseIntError &&__err) noexcept {
    message = "value parsing error: failed to parse value expected to be "s
              "IntTy\n("s.append(__err.what())
                  .append(")"s);
  }

  const char *what() const noexcept { return message.c_str(); }
};

Result<ValueTy, ParseValueError>
tryParseValue(const std::string_view __value_str) noexcept {
  using RetType = Result<ValueTy, ParseValueError>;

  const auto prefix = __value_str[0];
  if (prefix == 'r' || prefix == 's') {
    auto res = tryParseGeneralRegister(__value_str);
    auto res_mapped =
        res.map<ValueTy>([](auto &&reg) { return ValueTy(std::move(reg)); });
    return res_mapped.mapErr<ParseValueError>(
        [](auto &&err) { return ParseValueError(std::move(err)); });
  } else if (prefix == 'a') {
    auto res = tryParseArgumentRegister(__value_str);
    auto res_mapped =
        res.map<ValueTy>([](auto &&reg) { return ValueTy(std::move(reg)); });
    return res_mapped.mapErr<ParseValueError>(
        [](auto &&err) { return ParseValueError(std::move(err)); });
  } else if ('0' <= prefix && prefix <= '9') {
    auto res = tryParseInt(__value_str);
    auto res_mapped =
        res.map<ValueTy>([](auto &&reg) { return ValueTy(std::move(reg)); });
    return res_mapped.mapErr<ParseValueError>(
        [](auto &&err) { return ParseValueError(std::move(err)); });
  } else {
    return RetType::Err(ParseValueError(__value_str));
  }
}

class IllFormedInstError : public Error<IllFormedInstError> {
private:
  std::string message;

public:
  IllFormedInstError(const std::string_view __message) noexcept {
    message = "ill-formed instruction error: "s.append(__message);
  }

  const char *what() const noexcept { return message.c_str(); }
};

template <typename E>
class ErrorWithInstruction : public Error<ErrorWithInstruction<E>> {
private:
  std::string message;

public:
  ErrorWithInstruction(E &&__err, const llvm::Instruction &__inst) noexcept {
    std::string inst;
    llvm::raw_string_ostream rso(inst);
    rso << __inst;

    message =
        std::string(__err.what()).append("\n["s).append(inst).append("]"s);
  }

  const char *what() const noexcept { return message.c_str(); }
};

template <typename E>
class ErrorWithBasicBlock : public Error<ErrorWithBasicBlock<E>> {
private:
  std::string message;

public:
  ErrorWithBasicBlock(E &&__err, const llvm::BasicBlock &__bblock) noexcept {
    std::string bblock;
    llvm::raw_string_ostream rso(bblock);
    rso << __bblock;

    message =
        std::string(__err.what()).append("\n["s).append(bblock).append("]"s);
  }

  const char *what() const noexcept { return message.c_str(); }
};

template <typename E>
class ErrorWithFunction : public Error<ErrorWithFunction<E>> {
private:
  std::string message;

public:
  ErrorWithFunction(E &&__err, const llvm::Function &__func) noexcept {
    std::string func;
    llvm::raw_string_ostream rso(func);
    rso << __func;

    message =
        std::string(__err.what()).append("\n["s).append(func).append("]"s);
  }

  const char *what() const noexcept { return message.c_str(); }
};

template <typename T, typename E>
T unwrapOrThrowWithInst(Result<T, E> &&__res, const llvm::Instruction &__inst) {
  using ResType = Result<T, E>;

  if (__res.isErr()) {
    auto err = __res.inspect();
    throw ErrorWithInstruction(std::move(err), __inst);
  }

  return __res.unwrap();
}

template <typename T, typename E>
T unwrapOrThrowWithBB(Result<T, E> &&__res, const llvm::BasicBlock &__bblock) {
  using ResType = Result<T, E>;

  if (__res.isErr()) {
    auto err = __res.inspect();
    throw ErrorWithBasicBlock(std::move(err), __bblock);
  }

  return __res.unwrap();
}

template <typename T, typename E>
T unwrapOrThrowWithFunc(Result<T, E> &&__res, const llvm::Function &__func) {
  using ResType = Result<T, E>;

  if (__res.isErr()) {
    auto err = __res.inspect();
    throw ErrorWithFunction(std::move(err), __func);
  }

  return __res.unwrap();
}

class IllFormedIntrinsicError : public Error<IllFormedIntrinsicError> {
private:
  std::string message;

public:
  IllFormedIntrinsicError(llvm::CallInst &__inst) noexcept {
    message = "ill-formed intrinsic error: signature of "s
                  .append(__inst.getCalledFunction()->getName())
                  .append(" does not match the definition");
  }

  const char *what() const noexcept { return message.c_str(); }
};

bool isMallocIntrinsic(llvm::CallInst &__inst) noexcept {
  const auto fn_name = __inst.getCalledFunction()->getName();
  return (fn_name == "malloc");
}

std::string emitFromMallocIntrinsic(llvm::CallInst &__inst) {
  const auto target_str = unwrapOrThrowWithInst(tryGetName(&__inst), __inst);
  const auto target =
      unwrapOrThrowWithInst(tryParseGeneralRegister(target_str), __inst);

  const auto size_str =
      unwrapOrThrowWithInst(tryGetName(__inst.getArgOperand(0)), __inst);
  auto size = unwrapOrThrowWithInst(tryParseValue(size_str), __inst);

  const auto inst =
      sc::backend::assembly::MallocInst::create(target, std::move(size));
  return inst.getAssembly();
}

bool isFreeIntrinsic(llvm::CallInst &__inst) noexcept {
  const auto fn_name = __inst.getCalledFunction()->getName();
  return (fn_name == "free");
}

std::string emitFromFreeIntrinsic(llvm::CallInst &__inst) {
  const auto ptr_str =
      unwrapOrThrowWithInst(tryGetName(__inst.getArgOperand(0)), __inst);
  auto ptr = unwrapOrThrowWithInst(tryParseValue(ptr_str), __inst);

  const auto inst = sc::backend::assembly::FreeInst::create(std::move(ptr));
  return inst.getAssembly();
}

bool isDecrSPIntrinsic(llvm::CallInst &__inst) noexcept {
  const auto fn_name = __inst.getCalledFunction()->getName();
  return (fn_name == "$decr_sp");
}

std::string emitFromDecrSPIntrinsic(llvm::CallInst &__inst) {
  const auto arg_str =
      unwrapOrThrowWithInst(tryGetName(__inst.getArgOperand(0)), __inst);
  auto arg = unwrapOrThrowWithInst(tryParseValue(arg_str), __inst);

  const auto inst = sc::backend::assembly::IntSubInst::create(
      GeneralRegister::SP, BitWidth::QWORD, GeneralRegister::SP,
      std::move(arg));
  return inst.getAssembly();
}

bool isAsyncLoadIntrinsic(llvm::CallInst &__inst) noexcept {
  const auto fn_name = __inst.getCalledFunction()->getName();
  if (fn_name == "aload_i8" || fn_name == "aload_i16" ||
      fn_name == "aload_i32" || fn_name == "aload_i64") {
    return true;
  } else {
    return false;
  }
}

Result<AccessWidth, IllFormedIntrinsicError>
tryCalculateAsyncLoadAccessWidth(llvm::CallInst &__inst) noexcept {
  using RetType = Result<AccessWidth, IllFormedIntrinsicError>;

  const auto ret_ty = __inst.getType();
  if (ret_ty->isIntegerTy() && __inst.arg_size() == 1) {
    const auto arg_ty =
        llvm::dyn_cast<llvm::PointerType>(__inst.getArgOperand(0)->getType());
    if (arg_ty && !arg_ty->isOpaque() &&
        arg_ty->isOpaqueOrPointeeTypeMatches(ret_ty)) {
      auto aw_res = tryCalculateAccessWidth(arg_ty);
      return aw_res.mapErr<IllFormedIntrinsicError>(
          [&__inst](auto &&err) { return IllFormedIntrinsicError(__inst); });
    }
  }
  return RetType::Err(IllFormedIntrinsicError(__inst));
}

std::string emitFromAsyncLoadIntrinsic(llvm::CallInst &__inst) {
  const auto size =
      unwrapOrThrowWithInst(tryCalculateAsyncLoadAccessWidth(__inst), __inst);

  const auto target_str = unwrapOrThrowWithInst(tryGetName(&__inst), __inst);
  const auto target =
      unwrapOrThrowWithInst(tryParseGeneralRegister(target_str), __inst);

  const auto ptr_str =
      unwrapOrThrowWithInst(tryGetName(__inst.getArgOperand(0)), __inst);
  auto ptr = unwrapOrThrowWithInst(tryParseValue(ptr_str), __inst);

  const auto inst = sc::backend::assembly::AsyncLoadInst::create(
      target, size, std::move(ptr));
  return inst.getAssembly();
}

bool isIntSumIntrinsic(llvm::CallInst &__inst) noexcept {
  const auto fn_name = __inst.getCalledFunction()->getName();
  if (fn_name == "int_sum_i1" || fn_name == "int_sum_i8" ||
      fn_name == "int_sum_i16" || fn_name == "int_sum_i32" ||
      fn_name == "int_sum_i64") {
    return true;
  } else {
    return false;
  }
}

Result<BitWidth, IllFormedIntrinsicError>
tryCalculateIntSumBitWidth(llvm::CallInst &__inst) noexcept {
  using RetType = Result<BitWidth, IllFormedIntrinsicError>;

  const auto ret_ty = __inst.getType();
  if (ret_ty->isIntegerTy() && __inst.arg_size() == 8) {
    const auto is_tys_match =
        std::accumulate(__inst.arg_begin(), __inst.arg_end(), true,
                        [ret_ty](const bool acc, const auto &arg) {
                          return acc && (ret_ty == arg->getType());
                        });
    if (is_tys_match) {
      auto bw_res = tryCalculateBitWidth(ret_ty);
      return bw_res.mapErr<IllFormedIntrinsicError>(
          [&__inst](auto &&err) { return IllFormedIntrinsicError(__inst); });
    }
  }
  return RetType::Err(IllFormedIntrinsicError(__inst));
}

std::string emitFromIntSumIntrinsic(llvm::CallInst &__inst) {
  const auto bw =
      unwrapOrThrowWithInst(tryCalculateIntSumBitWidth(__inst), __inst);

  const auto target_str = unwrapOrThrowWithInst(tryGetName(&__inst), __inst);
  const auto target =
      unwrapOrThrowWithInst(tryParseGeneralRegister(target_str), __inst);

  std::vector<ValueTy> args;
  args.reserve(__inst.arg_size());
  std::transform(__inst.arg_begin(), __inst.arg_end(), std::back_inserter(args),
                 [&__inst](auto &arg) {
                   const auto arg_str =
                       unwrapOrThrowWithInst(tryGetName(arg), __inst);
                   return unwrapOrThrowWithInst(tryParseValue(arg_str), __inst);
                 });

  const auto inst = unwrapOrThrowWithInst(
      sc::backend::assembly::IntSumInst::tryCreate(target, std::move(args), bw),
      __inst);
  return inst.getAssembly();
}

bool isIntIncrIntrinsic(llvm::CallInst &__inst) noexcept {
  const auto fn_name = __inst.getCalledFunction()->getName();
  if (fn_name == "incr_i1" || fn_name == "incr_i8" || fn_name == "incr_i16" ||
      fn_name == "incr_i32" || fn_name == "incr_i64") {
    return true;
  } else {
    return false;
  }
}

Result<BitWidth, IllFormedIntrinsicError>
tryCalculateIntIncrDecrBitWidth(llvm::CallInst &__inst) noexcept {
  using RetType = Result<BitWidth, IllFormedIntrinsicError>;

  const auto ret_ty = __inst.getType();
  if (ret_ty->isIntegerTy() && __inst.arg_size() == 1) {
    const auto arg_ty = __inst.getArgOperand(0)->getType();
    if (ret_ty == arg_ty) {
      auto bw_res = tryCalculateBitWidth(ret_ty);
      return bw_res.mapErr<IllFormedIntrinsicError>(
          [&__inst](auto &&err) { return IllFormedIntrinsicError(__inst); });
    }
  }
  return RetType::Err(IllFormedIntrinsicError(__inst));
}

std::string emitFromIntIncrIntrinsic(llvm::CallInst &__inst) {
  const auto bw =
      unwrapOrThrowWithInst(tryCalculateIntIncrDecrBitWidth(__inst), __inst);

  const auto target_str = unwrapOrThrowWithInst(tryGetName(&__inst), __inst);
  const auto target =
      unwrapOrThrowWithInst(tryParseGeneralRegister(target_str), __inst);

  const auto arg_str =
      unwrapOrThrowWithInst(tryGetName(__inst.getArgOperand(0)), __inst);
  auto arg = unwrapOrThrowWithInst(tryParseValue(arg_str), __inst);

  const auto inst =
      sc::backend::assembly::IntIncrInst::create(target, bw, std::move(arg));
  return inst.getAssembly();
}

bool isIntDecrIntrinsic(llvm::CallInst &__inst) noexcept {
  const auto fn_name = __inst.getCalledFunction()->getName();
  if (fn_name == "decr_i1" || fn_name == "decr_i8" || fn_name == "decr_i16" ||
      fn_name == "decr_i32" || fn_name == "decr_i64") {
    return true;
  } else {
    return false;
  }
}

std::string emitFromIntDecrIntrinsic(llvm::CallInst &__inst) {
  const auto bw =
      unwrapOrThrowWithInst(tryCalculateIntIncrDecrBitWidth(__inst), __inst);

  const auto target_str = unwrapOrThrowWithInst(tryGetName(&__inst), __inst);
  const auto target =
      unwrapOrThrowWithInst(tryParseGeneralRegister(target_str), __inst);

  const auto arg_str =
      unwrapOrThrowWithInst(tryGetName(__inst.getArgOperand(0)), __inst);
  auto arg = unwrapOrThrowWithInst(tryParseValue(arg_str), __inst);

  const auto inst =
      sc::backend::assembly::IntDecrInst::create(target, bw, std::move(arg));
  return inst.getAssembly();
}

bool isIntAssertionIntrinsic(llvm::CallInst &__inst) {
  const auto fn_name = __inst.getCalledFunction()->getName();
  if (fn_name == "assert_eq_i1" || fn_name == "assert_eq_i8" ||
      fn_name == "assert_eq_i16" || fn_name == "assert_eq_i32" ||
      fn_name == "assert_eq_i64") {
    return true;
  } else {
    return false;
  }
}

Result<BitWidth, IllFormedIntrinsicError>
tryCalculateIntAssertionBitWidth(llvm::CallInst &__inst) noexcept {
  using RetType = Result<BitWidth, IllFormedIntrinsicError>;

  llvm::Type *assert_ty = nullptr;
  auto &ctx = __inst.getContext();

  const auto fn_name = __inst.getCalledFunction()->getName();
  if (fn_name == "assert_eq_i1") {
    assert_ty = llvm::Type::getInt1Ty(ctx);
  } else if (fn_name == "assert_eq_i8") {
    assert_ty = llvm::Type::getInt8Ty(ctx);
  } else if (fn_name == "assert_eq_i16") {
    assert_ty = llvm::Type::getInt16Ty(ctx);
  } else if (fn_name == "assert_eq_i32") {
    assert_ty = llvm::Type::getInt32Ty(ctx);
  } else if (fn_name == "assert_eq_i64") {
    assert_ty = llvm::Type::getInt64Ty(ctx);
  }

  if (assert_ty && __inst.arg_size() == 2) {
    const auto is_tys_match =
        std::accumulate(__inst.arg_begin(), __inst.arg_end(), true,
                        [assert_ty](const bool acc, const auto &arg) {
                          return acc && (assert_ty == arg->getType());
                        });
    if (is_tys_match) {
      auto bw_res = tryCalculateBitWidth(assert_ty);
      return bw_res.mapErr<IllFormedIntrinsicError>(
          [&__inst](auto &&err) { return IllFormedIntrinsicError(__inst); });
    }
  }
  return RetType::Err(IllFormedIntrinsicError(__inst));
}

std::string emitFromIntAssertionIntrinsic(llvm::CallInst &__inst) {
  const auto bw =
      unwrapOrThrowWithInst(tryCalculateIntAssertionBitWidth(__inst), __inst);

  const auto arg1_str =
      unwrapOrThrowWithInst(tryGetName(__inst.getArgOperand(0)), __inst);
  auto arg1 = unwrapOrThrowWithInst(tryParseValue(arg1_str), __inst);

  const auto arg2_str =
      unwrapOrThrowWithInst(tryGetName(__inst.getArgOperand(1)), __inst);
  auto arg2 = unwrapOrThrowWithInst(tryParseValue(arg2_str), __inst);

  const auto inst = sc::backend::assembly::AssertEqInst::create(
      std::move(arg1), std::move(arg2));
  return inst.getAssembly();
}

std::optional<sc::backend::assembly::FunctionEndInst> function_to_close;
} // namespace

namespace sc::backend::emitter {
AssemblyEmitter::AssemblyEmitter(const symbol::SymbolMap &__SM) noexcept {
  ::__SM = &__SM;
}

void AssemblyEmitter::visitFunction(llvm::Function &__function) {
  if (function_to_close.has_value()) {
    const auto end_fn = std::move(*function_to_close);
    assembly_lines.push_back(end_fn.getAssembly());
    function_to_close.reset();

    // insert an empty line between two functions
    assembly_lines.push_back("");
  }

  if (__function.isDeclaration()) {
    // skip if it is function declaration
    return;
  }

  auto fn_name = unwrapOrThrowWithFunc(tryGetName(&__function), __function);
  const auto inst =
      unwrapOrThrowWithFunc(assembly::FunctionStartInst::tryCreate(
                                std::string(fn_name), __function.arg_size()),
                            __function);

  assembly_lines.push_back(inst.getAssembly());
  function_to_close = assembly::FunctionEndInst::create(std::move(fn_name));
}

void AssemblyEmitter::visitBasicBlock(llvm::BasicBlock &__basic_block) {
  auto bb_name = unwrapOrThrowWithBB(tryGetName(&__basic_block), __basic_block);
  const auto inst = assembly::BasicBlockInst::create(std::move(bb_name));
  assembly_lines.push_back(inst.getAssembly());
}

void AssemblyEmitter::visitICmpInst(llvm::ICmpInst &__inst) {
  const auto target_str = unwrapOrThrowWithInst(tryGetName(&__inst), __inst);
  const auto target =
      unwrapOrThrowWithInst(tryParseGeneralRegister(target_str), __inst);

  const auto cond_str = __inst.getPredicateName(__inst.getPredicate()).str();
  const auto cond =
      unwrapOrThrowWithInst(tryParseIcmpCondition(cond_str), __inst);

  const auto lhs_str =
      unwrapOrThrowWithInst(tryGetName(__inst.getOperand(0)), __inst);
  auto lhs = unwrapOrThrowWithInst(tryParseValue(lhs_str), __inst);

  const auto rhs_str =
      unwrapOrThrowWithInst(tryGetName(__inst.getOperand(1)), __inst);
  auto rhs = unwrapOrThrowWithInst(tryParseValue(rhs_str), __inst);

  const auto bw = unwrapOrThrowWithInst(
      tryCalculateBitWidth(__inst.getOperand(0)->getType()), __inst);

  const auto inst = assembly::IntCompInst::create(
      target, cond, bw, std::move(lhs), std::move(rhs));
  assembly_lines.push_back(inst.getAssembly());
}

void AssemblyEmitter::visitLoadInst(llvm::LoadInst &__inst) {
  const auto target_str = unwrapOrThrowWithInst(tryGetName(&__inst), __inst);
  const auto target =
      unwrapOrThrowWithInst(tryParseGeneralRegister(target_str), __inst);

  const auto aw = unwrapOrThrowWithInst(
      tryCalculateAccessWidth(__inst.getOperand(0)->getType()), __inst);

  const auto ptr_str =
      unwrapOrThrowWithInst(tryGetName(__inst.getOperand(0)), __inst);
  auto ptr = unwrapOrThrowWithInst(tryParseValue(ptr_str), __inst);

  const auto inst = assembly::LoadInst::create(target, aw, std::move(ptr));
  assembly_lines.push_back(inst.getAssembly());
}

void AssemblyEmitter::visitStoreInst(llvm::StoreInst &__inst) {
  const auto aw = unwrapOrThrowWithInst(
      tryCalculateAccessWidth(__inst.getOperand(1)->getType()), __inst);

  const auto value_str =
      unwrapOrThrowWithInst(tryGetName(__inst.getOperand(0)), __inst);
  auto value = unwrapOrThrowWithInst(tryParseValue(value_str), __inst);

  const auto ptr_str =
      unwrapOrThrowWithInst(tryGetName(__inst.getOperand(1)), __inst);
  auto ptr = unwrapOrThrowWithInst(tryParseValue(ptr_str), __inst);

  const auto inst =
      assembly::StoreInst::create(aw, std::move(value), std::move(ptr));
  assembly_lines.push_back(inst.getAssembly());
}

void AssemblyEmitter::visitSExtInst(llvm::SExtInst &__inst) {
  auto beforeBits = __inst.getOperand(0)->getType()->getIntegerBitWidth();
  auto afterBits = __inst.getType()->getIntegerBitWidth();

  const auto target_str = unwrapOrThrowWithInst(tryGetName(&__inst), __inst);
  const auto target =
      unwrapOrThrowWithInst(tryParseGeneralRegister(target_str), __inst);

  auto bw =
      unwrapOrThrowWithInst(tryCalculateBitWidth(__inst.getType()), __inst);

  const auto arg1_str =
      unwrapOrThrowWithInst(tryGetName(__inst.getOperand(0)), __inst);
  auto arg1 = unwrapOrThrowWithInst(tryParseValue(arg1_str), __inst);

  llvm::Constant *arg2_raw = llvm::ConstantInt::get(
      __inst.getType(), 1llu << (afterBits - beforeBits));
  const auto arg2_str = unwrapOrThrowWithInst(tryGetName(arg2_raw), __inst);
  auto arg2 = unwrapOrThrowWithInst(tryParseValue(arg2_str), __inst);

  const auto inst1 = assembly::IntMulInst::create(target, bw, std::move(arg1),
                                                  std::move(arg2));
  assembly_lines.push_back(inst1.getAssembly());

  const auto arg3_str = unwrapOrThrowWithInst(tryGetName(&__inst), __inst);
  auto arg3 = unwrapOrThrowWithInst(tryParseValue(arg3_str), __inst);

  const auto inst2 = assembly::IntSDivInst::create(target, bw, std::move(arg3),
                                                   std::move(arg2));
  assembly_lines.push_back(inst2.getAssembly());
}

void AssemblyEmitter::visitSelectInst(llvm::SelectInst &__inst) {
  const auto target_str = unwrapOrThrowWithInst(tryGetName(&__inst), __inst);
  const auto target =
      unwrapOrThrowWithInst(tryParseGeneralRegister(target_str), __inst);

  const auto cond_str =
      unwrapOrThrowWithInst(tryGetName(__inst.getCondition()), __inst);
  auto cond = unwrapOrThrowWithInst(tryParseValue(cond_str), __inst);

  const auto value_true_str =
      unwrapOrThrowWithInst(tryGetName(__inst.getTrueValue()), __inst);
  auto value_true =
      unwrapOrThrowWithInst(tryParseValue(value_true_str), __inst);

  const auto value_false_str =
      unwrapOrThrowWithInst(tryGetName(__inst.getFalseValue()), __inst);
  auto value_false =
      unwrapOrThrowWithInst(tryParseValue(value_false_str), __inst);

  const auto inst = assembly::SelectInst::create(
      target, std::move(cond), std::move(value_true), std::move(value_false));
  assembly_lines.push_back(inst.getAssembly());
}

void AssemblyEmitter::visitCallInst(llvm::CallInst &__inst) {
  // handle swpp intrinsics firsthand
  if (isMallocIntrinsic(__inst)) {
    assembly_lines.push_back(emitFromMallocIntrinsic(__inst));
    return;
  } else if (isFreeIntrinsic(__inst)) {
    assembly_lines.push_back(emitFromFreeIntrinsic(__inst));
    return;
  } else if (isDecrSPIntrinsic(__inst)) {
    assembly_lines.push_back(emitFromDecrSPIntrinsic(__inst));
    return;
  } else if (isAsyncLoadIntrinsic(__inst)) {
    assembly_lines.push_back(emitFromAsyncLoadIntrinsic(__inst));
    return;
  } else if (isIntSumIntrinsic(__inst)) {
    assembly_lines.push_back(emitFromIntSumIntrinsic(__inst));
    return;
  } else if (isIntIncrIntrinsic(__inst)) {
    assembly_lines.push_back(emitFromIntIncrIntrinsic(__inst));
    return;
  } else if (isIntDecrIntrinsic(__inst)) {
    assembly_lines.push_back(emitFromIntDecrIntrinsic(__inst));
    return;
  } else if (isIntAssertionIntrinsic(__inst)) {
    assembly_lines.push_back(emitFromIntAssertionIntrinsic(__inst));
    return;
  }

  const auto fn = __inst.getCalledFunction();

  std::vector<ValueTy> args;
  args.reserve(__inst.arg_size());
  std::transform(__inst.arg_begin(), __inst.arg_end(), std::back_inserter(args),
                 [&__inst](auto &arg) {
                   const auto arg_str =
                       unwrapOrThrowWithInst(tryGetName(arg), __inst);
                   return unwrapOrThrowWithInst(tryParseValue(arg_str), __inst);
                 });

  if (fn->getReturnType()->isVoidTy()) {
    auto fn_name = unwrapOrThrowWithInst(tryGetName(fn), __inst);
    const auto inst =
        unwrapOrThrowWithInst(assembly::CallInst::tryCreateDiscarding(
                                  std::move(fn_name), std::move(args)),
                              __inst);
    assembly_lines.push_back(inst.getAssembly());
  } else {
    const auto target_str = unwrapOrThrowWithInst(tryGetName(&__inst), __inst);
    const auto target =
        unwrapOrThrowWithInst(tryParseGeneralRegister(target_str), __inst);

    auto fn_name = unwrapOrThrowWithInst(tryGetName(fn), __inst);
    const auto inst =
        unwrapOrThrowWithInst(assembly::CallInst::tryCreateReturning(
                                  target, std::move(fn_name), std::move(args)),
                              __inst);
    assembly_lines.push_back(inst.getAssembly());
  }
}

void AssemblyEmitter::visitReturnInst(llvm::ReturnInst &__inst) {
  const auto ret_value = __inst.getReturnValue();
  if (!ret_value) {
    const auto inst = assembly::ReturnInst::createVoid();
    assembly_lines.push_back(inst.getAssembly());
  } else {
    const auto value_str = unwrapOrThrowWithInst(tryGetName(ret_value), __inst);
    auto value = unwrapOrThrowWithInst(tryParseValue(value_str), __inst);

    const auto inst = assembly::ReturnInst::create(std::move(value));
    assembly_lines.push_back(inst.getAssembly());
  }
}

void AssemblyEmitter::visitBranchInst(llvm::BranchInst &__inst) {
  if (__inst.isConditional()) {
    if (__inst.getNumSuccessors() == 2) {
      const auto cond_str =
          unwrapOrThrowWithInst(tryGetName(__inst.getCondition()), __inst);
      auto cond = unwrapOrThrowWithInst(tryParseValue(cond_str), __inst);

      auto label_true =
          unwrapOrThrowWithInst(tryGetName(__inst.getSuccessor(0)), __inst);
      auto label_false =
          unwrapOrThrowWithInst(tryGetName(__inst.getSuccessor(1)), __inst);
      const auto inst = assembly::BranchInst::create(
          std::move(cond), std::move(label_true), std::move(label_false));
      assembly_lines.push_back(inst.getAssembly());
    } else {
      throw ErrorWithInstruction(
          IllFormedInstError("conditional branch must have two successors"s),
          __inst);
    }
  } else if (__inst.isUnconditional()) {
    if (__inst.getNumSuccessors() == 1) {
      auto label =
          unwrapOrThrowWithInst(tryGetName(__inst.getSuccessor(0)), __inst);
      const auto inst = assembly::JumpInst::create(std::move(label));
      assembly_lines.push_back(inst.getAssembly());
    } else {
      throw ErrorWithInstruction(
          IllFormedInstError(
              "unconditional branch must have only one successor"s),
          __inst);
    }
  } else {
    // Unreachable?
    throw ErrorWithInstruction(
        IllFormedInstError("invalid branch instruction"s), __inst);
  }
}

void AssemblyEmitter::visitSwitchInst(llvm::SwitchInst &__inst) {
  const auto cond_str =
      unwrapOrThrowWithInst(tryGetName(__inst.getCondition()), __inst);
  auto cond = unwrapOrThrowWithInst(tryParseValue(cond_str), __inst);

  std::vector<IntTy> cases;
  cases.reserve(__inst.getNumCases());
  std::vector<std::string> labels;
  labels.reserve(__inst.getNumCases());
  for (auto &c : __inst.cases()) {
    auto succ_name =
        unwrapOrThrowWithInst(tryGetName(c.getCaseSuccessor()), __inst);
    cases.push_back(c.getCaseValue()->getZExtValue());
    labels.push_back(std::move(succ_name));
  }
  auto label_default = unwrapOrThrowWithInst(
      tryGetName(__inst.case_default()->getCaseSuccessor()), __inst);

  const auto inst =
      unwrapOrThrowWithInst(assembly::SwitchInst::tryCreate(
                                std::move(cond), std::move(cases),
                                std::move(labels), std::move(label_default)),
                            __inst);
  assembly_lines.push_back(inst.getAssembly());
}

void AssemblyEmitter::visitBinaryOperator(llvm::BinaryOperator &__op) {
  const auto target_str = unwrapOrThrowWithInst(tryGetName(&__op), __op);
  const auto target =
      unwrapOrThrowWithInst(tryParseGeneralRegister(target_str), __op);

  auto bw = unwrapOrThrowWithInst(tryCalculateBitWidth(__op.getType()), __op);

  const auto arg1_str =
      unwrapOrThrowWithInst(tryGetName(__op.getOperand(0)), __op);
  auto arg1 = unwrapOrThrowWithInst(tryParseValue(arg1_str), __op);
  const auto arg2_str =
      unwrapOrThrowWithInst(tryGetName(__op.getOperand(1)), __op);
  auto arg2 = unwrapOrThrowWithInst(tryParseValue(arg2_str), __op);

  switch (__op.getOpcode()) {
  case llvm::Instruction::UDiv: {
    const auto inst = assembly::IntUDivInst::create(target, bw, std::move(arg1),
                                                    std::move(arg2));
    assembly_lines.push_back(inst.getAssembly());
    break;
  }
  case llvm::Instruction::SDiv: {
    const auto inst = assembly::IntSDivInst::create(target, bw, std::move(arg1),
                                                    std::move(arg2));
    assembly_lines.push_back(inst.getAssembly());
    break;
  }
  case llvm::Instruction::URem: {
    const auto inst = assembly::IntURemInst::create(target, bw, std::move(arg1),
                                                    std::move(arg2));
    assembly_lines.push_back(inst.getAssembly());
    break;
  }
  case llvm::Instruction::SRem: {
    const auto inst = assembly::IntSRemInst::create(target, bw, std::move(arg1),
                                                    std::move(arg2));
    assembly_lines.push_back(inst.getAssembly());
    break;
  }
  case llvm::Instruction::Mul: {
    const auto inst = assembly::IntMulInst::create(target, bw, std::move(arg1),
                                                   std::move(arg2));
    assembly_lines.push_back(inst.getAssembly());
    break;
  }
  case llvm::Instruction::Shl: {
    const auto inst = assembly::IntShlInst::create(target, bw, std::move(arg1),
                                                   std::move(arg2));
    assembly_lines.push_back(inst.getAssembly());
    break;
  }
  case llvm::Instruction::AShr: {
    const auto inst = assembly::IntAShrInst::create(target, bw, std::move(arg1),
                                                    std::move(arg2));
    assembly_lines.push_back(inst.getAssembly());
    break;
  }
  case llvm::Instruction::LShr: {
    const auto inst = assembly::IntLShrInst::create(target, bw, std::move(arg1),
                                                    std::move(arg2));
    assembly_lines.push_back(inst.getAssembly());
    break;
  }
  case llvm::Instruction::And: {
    const auto inst = assembly::IntAndInst::create(target, bw, std::move(arg1),
                                                   std::move(arg2));
    assembly_lines.push_back(inst.getAssembly());
    break;
  }
  case llvm::Instruction::Or: {
    const auto inst = assembly::IntOrInst::create(target, bw, std::move(arg1),
                                                  std::move(arg2));
    assembly_lines.push_back(inst.getAssembly());
    break;
  }
  case llvm::Instruction::Xor: {
    const auto inst = assembly::IntXorInst::create(target, bw, std::move(arg1),
                                                   std::move(arg2));
    assembly_lines.push_back(inst.getAssembly());
    break;
  }
  case llvm::Instruction::Add: {
    const auto inst = assembly::IntAddInst::create(target, bw, std::move(arg1),
                                                   std::move(arg2));
    assembly_lines.push_back(inst.getAssembly());
    break;
  }
  case llvm::Instruction::Sub: {
    const auto inst = assembly::IntSubInst::create(target, bw, std::move(arg1),
                                                   std::move(arg2));
    assembly_lines.push_back(inst.getAssembly());
    break;
  }
  default: {
    throw ErrorWithInstruction(IllFormedInstError("invalid binary operation"s),
                               __op);
  }
  }
}

void AssemblyEmitter::visitUnreachableInst(llvm::UnreachableInst &__op) {
  const auto comment = assembly::CommentInst::create("unreachable"s);
  assembly_lines.push_back(comment.getAssembly());
  const auto inst = assembly::AssertEqInst::create(IntTy(0), IntTy(1));
  assembly_lines.push_back(inst.getAssembly());

  auto bb_label = __op.getParent()->getName().str();
  const auto term_placeholder = assembly::JumpInst::create(std::move(bb_label));
  assembly_lines.push_back(term_placeholder.getAssembly());
}

std::string AssemblyEmitter::getAssembly() noexcept {
  if (function_to_close.has_value()) {
    const auto end_fn = std::move(*function_to_close);
    assembly_lines.push_back(end_fn.getAssembly());
    function_to_close.reset();
  }

  return collectStrings(assembly_lines);
}
} // namespace sc::backend::emitter
