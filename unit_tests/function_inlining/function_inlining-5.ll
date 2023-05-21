; RUN: opt < %s -load-pass-plugin=./build/libFunctionInlining.so -passes=function-inlining -S | FileCheck %s

; Check if the pass does not corrupt the code
; in the multiple returns case.

define i32 @conditional_add(i32 %x, i32 %y, i1 %cond) {
; CHECK-LABEL: @conditional_add(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[ADD:%.*]] = add i32 [[X:%.*]], [[Y:%.*]]
; CHECK-NEXT:    br i1 [[COND:%.*]], label [[ADD_RETURN:%.*]], label [[NO_ADD_RETURN:%.*]]
; CHECK:       add_return:
; CHECK-NEXT:    ret i32 [[ADD]]
; CHECK:       no_add_return:
; CHECK-NEXT:    ret i32 [[X]]
;
entry:
  %add = add i32 %x, %y
  br i1 %cond, label %add_return, label %no_add_return

add_return:
  ret i32 %add

no_add_return:
  ret i32 %x
}

define internal i32 @main() {
; CHECK-LABEL: @main(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[RESULT1:%.*]] = call i32 @conditional_add(i32 2, i32 3, i1 true)
; CHECK-NEXT:    [[RESULT2:%.*]] = call i32 @conditional_add(i32 5, i32 4, i1 false)
; CHECK-NEXT:    [[SUM:%.*]] = add i32 [[RESULT1]], [[RESULT2]]
; CHECK-NEXT:    ret i32 [[SUM]]
;
entry:
  %result1 = call i32 @conditional_add(i32 2, i32 3, i1 true)
  %result2 = call i32 @conditional_add(i32 5, i32 4, i1 false)
  %sum = add i32 %result1, %result2
  ret i32 %sum
}
