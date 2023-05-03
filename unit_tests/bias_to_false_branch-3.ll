; RUN: opt < %s -load-pass-plugin=./build/libBiasToFalseBranch.so -passes=bias-to-false-branch -S | FileCheck %s
; check BranchProbabilityAnalysis correctly computes branch probability

declare i32 @f()
declare i32 @g()

; due to pointer heuristics, branch prob becomes 0.625 / 0.375. So, swap conditions.
define i32 @cmp_ptr(ptr %0, ptr %1) {
; CHECK-LABEL: @cmp_ptr(
; CHECK-NEXT:    [[COND:%.*]] = icmp eq ptr [[TMP0:%.*]], [[TMP1:%.*]]
; CHECK-NEXT:    br i1 [[COND]], label %false_bb, label %true_bb
; CHECK:       true_bb:
; CHECK-NEXT:    [[X:%.*]] = call i32 @f()
; CHECK-NEXT:    br label %exit
; CHECK:       false_bb:
; CHECK-NEXT:    [[Y:%.*]] = call i32 @g()
; CHECK-NEXT:    br label %exit
; CHECK:       exit:
; CHECK-NEXT:    [[Z:%.*]] = phi i32 [ [[X]], %true_bb ], [ [[Y]], %false_bb ]
; CHECK-NEXT:    ret i32 [[Z]]
;
  %cond = icmp ne ptr %0, %1
  br i1 %cond, label %true_bb, label %false_bb

true_bb:                                                ; preds = %2
  %x = call i32 @f()
  br label %exit

false_bb:                                                ; preds = %2
  %y = call i32 @g()
  br label %exit

exit:                                                ; preds = %false_bb, %true_bb
  %z = phi i32 [ %x, %true_bb ], [ %y, %false_bb ]
  ret i32 %z
}

; when there is no information, branch prob becomes 0.5 / 0.5. So, do nothing.
define i32 @cmp_int(i32 %0, i32 %1) {
; CHECK-LABEL: @cmp_int(
; CHECK-NEXT:    [[COND:%.*]] = icmp ne i32 [[TMP0:%.*]], [[TMP1:%.*]]
; CHECK-NEXT:    br i1 [[COND]], label %true_bb, label %false_bb
; CHECK:       true_bb:
; CHECK-NEXT:    [[X:%.*]] = call i32 @f()
; CHECK-NEXT:    br label %exit
; CHECK:       false_bb:
; CHECK-NEXT:    [[Y:%.*]] = call i32 @g()
; CHECK-NEXT:    br label %exit
; CHECK:       exit:
; CHECK-NEXT:    [[Z:%.*]] = phi i32 [ [[X]], %true_bb ], [ [[Y]], %false_bb ]
; CHECK-NEXT:    ret i32 [[Z]]
;
  %cond = icmp ne i32 %0, %1
  br i1 %cond, label %true_bb, label %false_bb

true_bb:                                                ; preds = %2
  %x = call i32 @f()
  br label %exit

false_bb:                                                ; preds = %2
  %y = call i32 @g()
  br label %exit

exit:                                                ; preds = %false_bb, %true_bb
  %z = phi i32 [ %x, %true_bb ], [ %y, %false_bb ]
  ret i32 %z
}
