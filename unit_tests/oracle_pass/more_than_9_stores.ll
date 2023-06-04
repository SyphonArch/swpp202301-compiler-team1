; RUN: opt < %s -load-pass-plugin=./build/libOraclePass.so -passes=oracle-pass -S | FileCheck %s

; Check if the pass splits the store group into chunks (8 stores per group)

@num_to_bits = external global [16 x i32], align 16

define i32 @main() {
; CHECK-LABEL: @main(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    store i32 0, i32* getelementptr inbounds ([16 x i32], [16 x i32]* @num_to_bits, i64 0, i64 0), align 16
; CHECK-NEXT:    store i32 1, i32* getelementptr inbounds ([16 x i32], [16 x i32]* @num_to_bits, i64 0, i64 1), align 4
; CHECK-NEXT:    [[TMP0:%.*]] = call i64 @oracle(i32 2, i32* getelementptr inbounds ([16 x i32], [16 x i32]* @num_to_bits, i64 0, i64 9), i32 1, i32* getelementptr inbounds ([16 x i32], [16 x i32]* @num_to_bits, i64 0, i64 8), i32 3, i32* getelementptr inbounds ([16 x i32], [16 x i32]* @num_to_bits, i64 0, i64 7), i32 2, i32* getelementptr inbounds ([16 x i32], [16 x i32]* @num_to_bits, i64 0, i64 6), i32 2, i32* getelementptr inbounds ([16 x i32], [16 x i32]* @num_to_bits, i64 0, i64 5), i32 1, i32* getelementptr inbounds ([16 x i32], [16 x i32]* @num_to_bits, i64 0, i64 4), i32 2, i32* getelementptr inbounds ([16 x i32], [16 x i32]* @num_to_bits, i64 0, i64 3), i32 1, i32* getelementptr inbounds ([16 x i32], [16 x i32]* @num_to_bits, i64 0, i64 2))
; CHECK-NEXT:    ret i32 0
;
entry:
  store i32 0, i32* getelementptr inbounds ([16 x i32], [16 x i32]* @num_to_bits, i64 0, i64 0), align 16
  store i32 1, i32* getelementptr inbounds ([16 x i32], [16 x i32]* @num_to_bits, i64 0, i64 1), align 4
  store i32 1, i32* getelementptr inbounds ([16 x i32], [16 x i32]* @num_to_bits, i64 0, i64 2), align 8
  store i32 2, i32* getelementptr inbounds ([16 x i32], [16 x i32]* @num_to_bits, i64 0, i64 3), align 4
  store i32 1, i32* getelementptr inbounds ([16 x i32], [16 x i32]* @num_to_bits, i64 0, i64 4), align 16
  store i32 2, i32* getelementptr inbounds ([16 x i32], [16 x i32]* @num_to_bits, i64 0, i64 5), align 4
  store i32 2, i32* getelementptr inbounds ([16 x i32], [16 x i32]* @num_to_bits, i64 0, i64 6), align 8
  store i32 3, i32* getelementptr inbounds ([16 x i32], [16 x i32]* @num_to_bits, i64 0, i64 7), align 4

  store i32 1, i32* getelementptr inbounds ([16 x i32], [16 x i32]* @num_to_bits, i64 0, i64 8), align 16
  store i32 2, i32* getelementptr inbounds ([16 x i32], [16 x i32]* @num_to_bits, i64 0, i64 9), align 4
  ret i32 0
}


; CHECK: @oracle(i32 %0, i32* %1, i32 %2, i32* %3, i32 %4, i32* %5, i32 %6, i32* %7, i32 %8, i32* %9, i32 %10, i32* %11, i32 %12, i32* %13, i32 %14, i32* %15) {
; CHECK-NEXT: entry:
; CHECK-NEXT:   store i32 %0, i32* %1, align 4
; CHECK-NEXT:   store i32 %2, i32* %3, align 4
; CHECK-NEXT:   store i32 %4, i32* %5, align 4
; CHECK-NEXT:   store i32 %6, i32* %7, align 4
; CHECK-NEXT:   store i32 %8, i32* %9, align 4
; CHECK-NEXT:   store i32 %10, i32* %11, align 4
; CHECK-NEXT:   store i32 %12, i32* %13, align 4
; CHECK-NEXT:   store i32 %14, i32* %15, align 4
; CHECK-NEXT:   ret i64 0
;