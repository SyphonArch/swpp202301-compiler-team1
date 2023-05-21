; RUN: opt < %s -load-pass-plugin=./build/libFunctionInlining.so -passes=function-inlining -S | FileCheck %s

; Check if the function inlining works in the most simple case.
; (Callee has 1 BasicBlock and 1 int return instruction)

define i32 @caller(i32 %a, i32 %b) {
; CHECK-LABEL: @caller(
; CHECK-NEXT:    [[ADD:%.*]] = add i32 [[A:%.*]], [[B:%.*]]
; CHECK-NEXT:    [[TMP1:%.*]] = sub i32 [[ADD]], 5
; CHECK-NEXT:    [[TMP2:%.*]] = mul i32 [[TMP1]], 3
; CHECK-NEXT:    ret i32 [[TMP2]]
;
  %add = add i32 %a, %b
  %call1 = call i32 @callee1(i32 %add)
  %call2 = call i32 @callee2(i32 %call1)
  ret i32 %call2
}

define i32 @callee1(i32 %x) {
; CHECK-LABEL: @callee1(
; CHECK-NEXT:    [[SUB:%.*]] = sub i32 [[X:%.*]], 5
; CHECK-NEXT:    ret i32 [[SUB]]
;
  %sub = sub i32 %x, 5
  ret i32 %sub
}

define i32 @callee2(i32 %y) {
; CHECK-LABEL: @callee2(
; CHECK-NEXT:    [[MUL:%.*]] = mul i32 [[Y:%.*]], 3
; CHECK-NEXT:    ret i32 [[MUL]]
;
  %mul = mul i32 %y, 3
  ret i32 %mul
}
