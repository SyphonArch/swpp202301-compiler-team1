; RUN: opt < %s -load-pass-plugin=./build/libOraclePass.so -passes=oracle-pass -S | FileCheck %s

; Check if the grouping of store isn't blocked by intrinsics or I/O call.

@M = external global i32, align 4
@N = external global i32, align 4

define i32 @main() {
; CHECK-LABEL: @main(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[CALL:%.*]] = call i64 (...) @read()
; CHECK-NEXT:    [[CONV:%.*]] = trunc i64 [[CALL]] to i32
; CHECK-NEXT:    [[CALL1:%.*]] = call i64 (...) @read()
; CHECK-NEXT:    [[CALL2:%.*]] = call i32 @incr_i32(i32 [[CONV]])
; CHECK-NEXT:    [[CONV2:%.*]] = trunc i64 [[CALL1]] to i32
; CHECK-NEXT:    [[TMP0:%.*]] = call i64 @oracle(i32 [[CONV2]], i32* @M, i32 [[CONV]], i32* @N)
; CHECK-NEXT:    ret i32 0
;
entry:
  %call = call i64 (...) @read()
  %conv = trunc i64 %call to i32
  store i32 %conv, i32* @N, align 4
  %call1 = call i64 (...) @read()
  %call2 = call i32 @incr_i32(i32 %conv)
  %conv2 = trunc i64 %call1 to i32
  store i32 %conv2, i32* @M, align 4

  ret i32 0
}


declare i64 @read(...)
declare i32 @incr_i32(i32)
