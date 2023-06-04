; RUN: opt < %s -load-pass-plugin=./build/libOraclePass.so -passes=oracle-pass -S | FileCheck %s

; Check if the pass does not group stores when there might be
; side effects for defering stores.

@M = external global i32, align 4
@N = external global i32, align 4

define i64 @f() {
; CHECK-LABEL: @f(
; CHECK-NEXT:    [[TMP1:%.*]] = load i32, i32* @M, align 4
; CHECK-NEXT:    [[TMP2:%.*]] = load i32, i32* @N, align 4
; CHECK-NEXT:    ret i64 0
;
  %1 = load i32, i32* @M, align 4
  %2 = load i32, i32* @N, align 4
  ret i64 0
}

define i32 @main() {
; CHECK-LABEL: @main(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[CALL:%.*]] = call i64 @f()
; CHECK-NEXT:    [[CONV:%.*]] = trunc i64 [[CALL]] to i32
; CHECK-NEXT:    store i32 [[CONV]], i32* @N, align 4
; CHECK-NEXT:    [[CALL1:%.*]] = call i64 @f()
; CHECK-NEXT:    [[CONV2:%.*]] = trunc i64 [[CALL1]] to i32
; CHECK-NEXT:    store i32 [[CONV2]], i32* @M, align 4
; CHECK-NEXT:    ret i32 0
;
entry:
  %call = call i64 @f()
  %conv = trunc i64 %call to i32
  store i32 %conv, i32* @N, align 4
  %call1 = call i64 @f()
  %conv2 = trunc i64 %call1 to i32
  store i32 %conv2, i32* @M, align 4

  ret i32 0
}
