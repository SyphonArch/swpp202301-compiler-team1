; RUN: opt < %s -load-pass-plugin=./build/libBiasToFalseBranch.so -passes=bias-to-false-branch -S | FileCheck %s
; Test case for an icmp instruction used in two branches.

; The first branch probability ~= 0.03 (looping edge)
; and the second is 0.5.
; So the condition can be inverted without side effect.
define i32 @both_prob_less_or_equal_to_05() {
; CHECK-LABEL: @both_prob_less_or_equal_to_05(

entry:
  br label %while.cond

while.cond:                                       ; preds = %while.body, %entry
  %i.0 = phi i32 [ 0, %entry ], [ %add, %while.body.end ]
  %cmp = icmp slt i32 %i.0, 10
; CHECK: %cmp = icmp sge i32 %i.0, 10
  br i1 %cmp, label %while.body, label %while.end
; CHECK: i1 %cmp, label %while.end, label %while.body


while.body:                                       ; preds = %while.cond
  %add = add nsw i32 %i.0, 1
  br i1 %cmp, label %true_bb, label %false_bb
; CHECK: i1 %cmp, label %false_bb, label %true_bb


true_bb:                                                ; preds = %2
  br label %while.body.end

false_bb:                                                ; preds = %2
  br label %while.body.end

while.body.end:
  br label %while.cond

while.end:                                        ; preds = %while.cond
  ret i32 %i.0
}
