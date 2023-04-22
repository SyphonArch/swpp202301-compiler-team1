#ifndef SC_LIB_BACKEND_ASSEMBLY_H
#define SC_LIB_BACKEND_ASSEMBLY_H

#include "assembly/inst.h"
#include "assembly/int_t.h"
#include "assembly/register_t.h"
#include "assembly/width_t.h"

namespace sc::backend::assembly {
using AccessWidth = width_t::AccessWidth;
using BitWidth = width_t::BitWidth;
using IcmpCondition = inst::IcmpCondition;

using IntTy = int_t::IntTy;
using GeneralRegister = register_t::GeneralRegister;
using ArgumentRegister = register_t::ArgumentRegister;
using ValueTy = inst::ValueTy;

using FunctionStartInst = inst::FunctionStartInst;
using FunctionEndInst = inst::FunctionEndInst;
using BasicBlockInst = inst::BasicBlockInst;
using CommentInst = inst::CommentInst;
using ReturnInst = inst::ReturnInst;
using JumpInst = inst::JumpInst;
using BranchInst = inst::BranchInst;
using SwitchInst = inst::SwitchInst;
using MallocInst = inst::MallocInst;
using FreeInst = inst::FreeInst;
using LoadInst = inst::LoadInst;
using StoreInst = inst::StoreInst;
using IntAddInst = inst::IntAddInst;
using IntSubInst = inst::IntSubInst;
using IntMulInst = inst::IntMulInst;
using IntUDivInst = inst::IntUDivInst;
using IntSDivInst = inst::IntSDivInst;
using IntURemInst = inst::IntURemInst;
using IntSRemInst = inst::IntSRemInst;
using IntAndInst = inst::IntAndInst;
using IntOrInst = inst::IntOrInst;
using IntXorInst = inst::IntXorInst;
using IntShlInst = inst::IntShlInst;
using IntLShrInst = inst::IntLShrInst;
using IntAShrInst = inst::IntAShrInst;
using IntCompInst = inst::IntCompInst;
using SelectInst = inst::SelectInst;
using CallInst = inst::CallInst;
using AssertEqInst = inst::AssertEqInst;

using AsyncLoadInst = inst::AsyncLoadInst;
using IntSumInst = inst::IntSumInst;
using IntIncrInst = inst::IntIncrInst;
using IntDecrInst = inst::IntDecrInst;
} // namespace sc::backend::assembly
#endif // SC_LIB_BACKEND_ASSEMBLY_H
