; RUN: opt < %s -load-pass-plugin=./build/libFunctionInlining.so -passes=function-inlining -S | FileCheck %s

; Check if the inlining does not corrupt the code
; in the case of recursion.

define i64 @factorial(i64 %n) {
; CHECK-LABEL: @factorial(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[CMP:%.*]] = icmp eq i64 [[N:%.*]], 0
; CHECK-NEXT:    br i1 [[CMP]], label [[BASE_CASE:%.*]], label [[RECURSIVE_CASE:%.*]]
; CHECK:       base_case:
; CHECK-NEXT:    br label [[MERGE:%.*]]
; CHECK:       recursive_case:
; CHECK-NEXT:    [[N_MINUS_ONE:%.*]] = sub i64 [[N]], 1
; CHECK-NEXT:    [[FACTORIAL_N_MINUS_ONE:%.*]] = call i64 @factorial(i64 [[N_MINUS_ONE]])
; CHECK-NEXT:    [[RESULT:%.*]] = mul i64 [[N]], [[FACTORIAL_N_MINUS_ONE]]
; CHECK-NEXT:    br label [[MERGE]]
; CHECK:       merge:
; CHECK-NEXT:    [[RETVAL:%.*]] = phi i64 [ 1, [[BASE_CASE]] ], [ [[RESULT]], [[RECURSIVE_CASE]] ]
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

define dso_local i32 @main() {
; CHECK-LABEL: @main(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[FACTORIAL_5:%.*]] = call i64 @factorial(i64 5)
; CHECK-NEXT:    ret i32 0
;
entry:
  %factorial_5 = call i64 @factorial(i64 5)
  ret i32 0
}
