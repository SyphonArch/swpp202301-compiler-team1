; RUN: opt < %s -load-pass-plugin=./build/libFunctionInlining.so -passes=function-inlining -S | FileCheck %s

; Check if the pass does not inline oracle function.

define i64 @oracle(i32* %ptr1, i32 %value1, i32* %ptr2, i32 %value2) {
; CHECK-LABEL: @oracle(
; CHECK-NEXT:    store i32 42, i32* [[PTR1:%.*]], align 4
; CHECK-NEXT:    store i32 84, i32* [[PTR2:%.*]], align 4
; CHECK-NEXT:    ret i64 0
;
  store i32 42, i32* %ptr1
  store i32 84, i32* %ptr2
  ret i64 0
}

define i64 @test() {
; CHECK-LABEL: @test(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[PTR1:%.*]] = alloca i32, align 4
; CHECK-NEXT:    [[PTR2:%.*]] = alloca i32, align 4
; CHECK-NEXT:    [[RET:%.*]] = call i64 @oracle(i32* [[PTR1]], i32 1, i32* [[PTR2]], i32 2)
; CHECK-NEXT:    ret i64 [[RET]]
;
entry:
  %ptr1 = alloca i32
  %ptr2 = alloca i32
  %ret = call i64 @oracle(i32* %ptr1, i32 1, i32* %ptr2, i32 2)
  ret i64 %ret
}
