; RUN: opt < %s -load-pass-plugin=./build/libFunctionInlining.so -passes=function-inlining -S | FileCheck %s

; Check if function inlining moves the static allocas up.

; @foo calls @bar, and @bar should be inlined with its static allocas up to the entry.
define i32 @foo(i32 %x) {
; CHECK-LABEL: @foo(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[TMP0:%.*]] = alloca i32, align 4
; CHECK-NEXT:    [[TMP1:%.*]] = alloca i32, align 4
; CHECK-NEXT:    [[ALLOCA_X:%.*]] = alloca i32, align 4
; CHECK-NEXT:    store i32 [[X:%.*]], i32* [[ALLOCA_X]], align 4
; CHECK-NEXT:    br label [[EXIT:%.*]]
; CHECK:       exit:
; CHECK-NEXT:    [[LOAD_X:%.*]] = load i32, i32* [[ALLOCA_X]], align 4
; CHECK-NEXT:    store i32 [[LOAD_X]], i32* [[TMP0]], align 4
; CHECK-NEXT:    store i32 [[LOAD_X]], i32* [[TMP1]], align 4
; CHECK-NEXT:    [[TMP2:%.*]] = load i32, i32* [[TMP0]], align 4
; CHECK-NEXT:    [[TMP3:%.*]] = add i32 [[TMP2]], 1
; CHECK-NEXT:    ret i32 [[TMP3]]
;
entry:
  %alloca_x = alloca i32, align 4
  store i32 %x, i32* %alloca_x, align 4
  br label %exit
exit:
  %load_x = load i32, i32* %alloca_x, align 4
  %call = call i32 @bar(i32 %load_x)
  ret i32 %call
}

; bar have two static allocas
define i32 @bar(i32 %y) {
; CHECK-LABEL: @bar(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[ALLOCA_Y:%.*]] = alloca i32, align 4
; CHECK-NEXT:    [[ALLOCA_Z:%.*]] = alloca i32, align 4
; CHECK-NEXT:    store i32 [[Y:%.*]], i32* [[ALLOCA_Y]], align 4
; CHECK-NEXT:    store i32 [[Y]], i32* [[ALLOCA_Z]], align 4
; CHECK-NEXT:    [[LOAD_Y:%.*]] = load i32, i32* [[ALLOCA_Y]], align 4
; CHECK-NEXT:    [[ADD:%.*]] = add i32 [[LOAD_Y]], 1
; CHECK-NEXT:    ret i32 [[ADD]]
;
entry:
  %alloca_y = alloca i32, align 4
  %alloca_z = alloca i32, align 4
  store i32 %y, i32* %alloca_y, align 4
  store i32 %y, i32* %alloca_z, align 4
  %load_y = load i32, i32* %alloca_y, align 4
  %add = add i32 %load_y, 1
  ret i32 %add
}
