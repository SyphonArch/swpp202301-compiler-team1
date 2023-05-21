; RUN: opt < %s -load-pass-plugin=./build/libFunctionInlining.so -passes=function-inlining -S | FileCheck %s

; Check if the inlining does not corrupt the code
; in the case of recursion.

define i64 @factorial(i64 %n) {
; CHECK-LABEL: @factorial(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[CMP:%.*]] = icmp eq i64 [[N:%.*]], 0
; CHECK-NEXT:    br i1 [[CMP]], label %base_case, label %recursive_case
; CHECK:       base_case:
; CHECK-NEXT:    br label %merge
; CHECK:       recursive_case:
; CHECK-NEXT:    [[N_MINUS_ONE:%.*]] = sub i64 [[N]], 1
; CHECK-NEXT:    [[FACTORIAL_N_MINUS_ONE:%.*]] = call i64 @factorial(i64 [[N_MINUS_ONE]])
; CHECK-NEXT:    [[RESULT:%.*]] = mul i64 [[N]], [[FACTORIAL_N_MINUS_ONE]]
; CHECK-NEXT:    br label %merge
; CHECK:       merge:
; CHECK-NEXT:    [[RETVAL:%.*]] = phi i64 [ 1, %base_case ], [ [[RESULT]], %recursive_case ]
; CHECK-NEXT:    ret i64 [[RETVAL]]
;
entry:
  %cmp = icmp eq i64 %n, 0
  br i1 %cmp, label %base_case, label %recursive_case

base_case:
  br label %merge

recursive_case:
  %n_minus_one = sub i64 %n, 1
  %factorial_n_minus_one = call i64 @factorial(i64 %n_minus_one)
  %result = mul i64 %n, %factorial_n_minus_one
  br label %merge

merge:
  %retval = phi i64 [ 1, %base_case ], [ %result, %recursive_case ]
  ret i64 %retval
}

define internal i32 @main() {
; CHECK-LABEL: @main(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    br label [[ENTRY1:%.*]]
; CHECK:       factorial.exit:
; CHECK-NEXT:    [[TMP0:%.*]] = phi i64 [ 1, %base_case ], [ [[TMP4:%.*]], %recursive_case ]
; CHECK-NEXT:    ret i32 0
; CHECK:       entry1:
; CHECK-NEXT:    [[TMP1:%.*]] = icmp eq i64 5, 0
; CHECK-NEXT:    br i1 [[TMP1]], label %base_case, label %recursive_case
; CHECK:       base_case:
; CHECK-NEXT:    br label [[FACTORIAL_EXIT:%.*]]
; CHECK:       recursive_case:
; CHECK-NEXT:    [[TMP2:%.*]] = sub i64 5, 1
; CHECK-NEXT:    [[TMP3:%.*]] = call i64 @factorial(i64 [[TMP2]])
; CHECK-NEXT:    [[TMP4]] = mul i64 5, [[TMP3]]
; CHECK-NEXT:    br label [[FACTORIAL_EXIT]]
;
entry:
  %factorial_5 = call i64 @factorial(i64 5)
  ret i32 0
}
