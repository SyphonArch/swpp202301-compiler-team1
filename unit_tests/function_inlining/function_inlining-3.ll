; RUN: opt < %s -load-pass-plugin=./build/libFunctionInlining.so -passes=function-inlining -S | FileCheck %s

; Check if function inlining works
; when the Callee has multiple BasicBlocks.

define i32 @caller(i32 %a, i32 %b) {
; CHECK-LABEL: @caller(
; CHECK-NEXT:    [[ADD:%.*]] = add i32 [[A:%.*]], [[B:%.*]]
; CHECK-NEXT:    [[TMP1:%.*]] = mul i32 [[ADD]], 3
; CHECK-NEXT:    br label %[[TMP3:.*]]
; CHECK:       [[CALLEE2_EXIT:.*]]:
; CHECK-NEXT:    [[TMP2:%.*]] = sub i32 [[TMP4:%.*]], 10
; CHECK-NEXT:    ret i32 [[TMP2]]
; CHECK:       [[TMP3]]:
; CHECK-NEXT:    [[TMP4]] = sub i32 [[TMP1]], 5
; CHECK-NEXT:    br label %[[CALLEE2_EXIT]]
;
  %add = add i32 %a, %b
  %call1 = call i32 @callee1(i32 %add)
  %call2 = call i32 @callee2(i32 %call1)
  ret i32 %call2
}

define i32 @callee2(i32 %x) {
; CHECK-LABEL: @callee2(
; CHECK-NEXT:    [[SUB:%.*]] = sub i32 [[X:%.*]], 5
; CHECK-NEXT:    br label %exit
; CHECK:       exit:
; CHECK-NEXT:    [[SUB2:%.*]] = sub i32 [[SUB]], 10
; CHECK-NEXT:    ret i32 [[SUB2]]
;
  %sub = sub i32 %x, 5
  br label %exit

exit:
  %sub2 = sub i32 %sub, 10
  ret i32 %sub2
}

define i32 @callee1(i32 %y) {
; CHECK-LABEL: @callee1(
; CHECK-NEXT:    [[MUL:%.*]] = mul i32 [[Y:%.*]], 3
; CHECK-NEXT:    ret i32 [[MUL]]
;
  %mul = mul i32 %y, 3
  ret i32 %mul
}
