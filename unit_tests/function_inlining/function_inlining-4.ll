; RUN: opt < %s -load-pass-plugin=./build/libFunctionInlining.so -passes=function-inlining -S | FileCheck %s

; Check if the function inlining pass works
; when Callee have calls in function body.

define i32 @add(i32 %x, i32 %y) {
; CHECK-LABEL: @add(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[SUM:%.*]] = add i32 [[X:%.*]], [[Y:%.*]]
; CHECK-NEXT:    ret i32 [[SUM]]
;
entry:
  %sum = add i32 %x, %y
  ret i32 %sum
}

define i32 @mul(i32 %x, i32 %y) {
; CHECK-LABEL: @mul(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[PRODUCT:%.*]] = mul i32 [[X:%.*]], [[Y:%.*]]
; CHECK-NEXT:    ret i32 [[PRODUCT]]
;
entry:
  %product = mul i32 %x, %y
  ret i32 %product
}

define i32 @mul_and_add(i32 %x, i32 %y, i32 %z) {
; CHECK-LABEL: @mul_and_add(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[TMP0:%.*]] = mul i32 [[X:%.*]], [[Y:%.*]]
; CHECK-NEXT:    [[TMP1:%.*]] = add i32 [[TMP0]], [[Z:%.*]]
; CHECK-NEXT:    ret i32 [[TMP1]]
;
entry:
  %mul_res = call i32 @mul(i32 %x, i32 %y)
  %add_res = call i32 @add(i32 %mul_res, i32 %z)
  ret i32 %add_res
}

define i32 @main() {
; CHECK-LABEL: @main(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[TMP0:%.*]] = mul i32 2, 3
; CHECK-NEXT:    [[TMP1:%.*]] = add i32 [[TMP0]], 4
; CHECK-NEXT:    ret i32 0
;
entry:
  %result = call i32 @mul_and_add(i32 2, i32 3, i32 4)
  ret i32 0
}
