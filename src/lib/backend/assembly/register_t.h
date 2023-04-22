#ifndef SC_LIB_BACKEND_ASSEMBLY_REGISTER_T_H
#define SC_LIB_BACKEND_ASSEMBLY_REGISTER_T_H

#include <string>

namespace sc::backend::assembly::register_t {
enum class GeneralRegister {
  R1,
  R2,
  R3,
  R4,
  R5,
  R6,
  R7,
  R8,
  R9,
  R10,
  R11,
  R12,
  R13,
  R14,
  R15,
  R16,
  R17,
  R18,
  R19,
  R20,
  R21,
  R22,
  R23,
  R24,
  R25,
  R26,
  R27,
  R28,
  R29,
  R30,
  R31,
  R32,
  SP,
};

enum class ArgumentRegister {
  A1,
  A2,
  A3,
  A4,
  A5,
  A6,
  A7,
  A8,
  A9,
  A10,
  A11,
  A12,
  A13,
  A14,
  A15,
  A16
};

std::string getToken(const GeneralRegister reg) noexcept;
std::string getToken(const ArgumentRegister reg) noexcept;
} // namespace sc::backend::assembly::register_t
#endif // SC_LIB_BACKEND_ASSEMBLY_REGISTER_T_H
