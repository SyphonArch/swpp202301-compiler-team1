; RUN: opt < %s -load-pass-plugin=./build/libOraclePass.so -passes=oracle-pass -S | FileCheck %s

; Check if the oracle pass manages the base case
; Currently, the implementation outlines the largest store group to oracle call
; where the store group is defined as "the set of stores that can be reduced to a single call"

define void @cyclic_swap(i32* noundef %ptr_a, i32* noundef %ptr_b, i32* noundef %ptr_c) {
; CHECK-LABEL: @cyclic_swap(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[A:%.*]] = load i32, i32* [[PTR_A:%.*]], align 4
; CHECK-NEXT:    [[B:%.*]] = load i32, i32* [[PTR_B:%.*]], align 4
; CHECK-NEXT:    [[C:%.*]] = load i32, i32* [[PTR_C:%.*]], align 4
; CHECK-NEXT:    [[TMP0:%.*]] = call i64 @oracle(i32 [[A]], i32* [[PTR_C]], i32 [[A]], i32* [[PTR_C]], i32 [[C]], i32* [[PTR_B]], i32 [[B]], i32* [[PTR_A]])
; CHECK-NEXT:    [[A2:%.*]] = load i32, i32* [[PTR_A]], align 4
; CHECK-NEXT:    [[B2:%.*]] = load i32, i32* [[PTR_B]], align 4
; CHECK-NEXT:    [[C2:%.*]] = load i32, i32* [[PTR_C]], align 4
; CHECK-NEXT:    store i32 [[B2]], i32* [[PTR_A]], align 4
; CHECK-NEXT:    store i32 [[C2]], i32* [[PTR_B]], align 4
; CHECK-NEXT:    store i32 [[A2]], i32* [[PTR_C]], align 4
; CHECK-NEXT:    ret void
;
entry:
  %a = load i32, i32* %ptr_a, align 4
  %b = load i32, i32* %ptr_b, align 4
  %c = load i32, i32* %ptr_c, align 4
  store i32 %b, i32* %ptr_a, align 4
  store i32 %c, i32* %ptr_b, align 4
  store i32 %a, i32* %ptr_c, align 4
  store i32 %a, i32* %ptr_c, align 4

  %a2 = load i32, i32* %ptr_a, align 4
  %b2 = load i32, i32* %ptr_b, align 4
  %c2 = load i32, i32* %ptr_c, align 4
  store i32 %b2, i32* %ptr_a, align 4
  store i32 %c2, i32* %ptr_b, align 4
  store i32 %a2, i32* %ptr_c, align 4
  ret void
}
