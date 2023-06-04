; RUN: opt < %s -load-pass-plugin=./build/libOraclePass.so -passes=oracle-pass -S | FileCheck %s

; This unit tests checks that the oracle pass does not crash
; when only a single store instruction exists.

define i32 @f(i32* noundef %ptr_a, i32* noundef %ptr_b, i32* noundef %ptr_c) {
; CHECK-LABEL: @f(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[A:%.*]] = load i32, i32* [[PTR_A:%.*]], align 4
; CHECK-NEXT:    [[B:%.*]] = load i32, i32* [[PTR_B:%.*]], align 4
; CHECK-NEXT:    [[C:%.*]] = load i32, i32* [[PTR_C:%.*]], align 4
; CHECK-NEXT:    store i32 [[A]], i32* [[PTR_C]], align 4
; CHECK-NEXT:    ret i32 0
;
entry:
  %a = load i32, i32* %ptr_a, align 4
  %b = load i32, i32* %ptr_b, align 4
  %c = load i32, i32* %ptr_c, align 4
  store i32 %a, i32* %ptr_c, align 4

  ret i32 0
}
