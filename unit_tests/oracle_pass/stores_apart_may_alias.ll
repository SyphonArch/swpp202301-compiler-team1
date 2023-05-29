; RUN: opt < %s -load-pass-plugin=./build/libOraclePass.so -passes=oracle-pass -S | FileCheck %s

; Check the case when the stores are interleaved with load instructions.

; Here, we don't know ptr_a is the same with ptr_b (so it may be alias).
; So in this case, no outlining to oracle should happen.

; CHECK-LABEL: @may_alias_pointers
define void @may_alias_pointers(i32* noundef %ptr_a, i32* noundef %ptr_b) {
; CHECK-LABEL: @may_alias_pointers(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[A:%.*]] = load i32, i32* [[PTR_A:%.*]], align 4
; CHECK-NEXT:    store i32 [[A]], i32* [[PTR_A]], align 4
; CHECK-NEXT:    [[B:%.*]] = load i32, i32* [[PTR_B:%.*]], align 4
; CHECK-NEXT:    store i32 [[B]], i32* [[PTR_B]], align 4
; CHECK-NEXT:    ret void
;
entry:
  %a = load i32, i32* %ptr_a, align 4
  store i32 %a, i32* %ptr_a, align 4
  %b = load i32, i32* %ptr_b, align 4
  store i32 %b, i32* %ptr_b, align 4
  ret void
}
