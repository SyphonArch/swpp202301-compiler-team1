; RUN: opt < %s -load-pass-plugin=./build/libBiasToFalseBranch.so -passes=bias-to-false-branch -S | FileCheck %s
; Test case for an icmp instruction used in two branches.

; The first branch probability ~= 0.03 (looping edge)
; and the second is 0.97 (looping edge)
; So the the invert condition needs to be created to avoid side effect.
define i32 @each_prob_less_than_03_and_higher_than_07() {
; CHECK-LABEL: @each_prob_less_than_03_and_higher_than_07(

entry:
  br label %while.cond

while.cond:
  %i.0 = phi i32 [ 0, %entry ], [ %add, %while.body ]
  %cmp = icmp slt i32 %i.0, 10
; CHECK: %cmp = icmp slt i32 %i.0, 10
  br i1 %cmp, label %while.body, label %while2.cond
; CHECK: %not_cmp = select i1 %cmp, i1 false, i1 true
; CHECK-NEXT: br i1 %not_cmp, label %while2.cond, label %while.body

while.body:
  %add = add nsw i32 %i.0, 1
  br label %while.cond

while2.cond:
  br i1 %cmp, label %while2.end, label %while2.body
; CHECK-NOT: %not_cmp
; CHECK: br i1 %cmp, label %while2.end, label %while2.body


while2.body:
  %add2 = add nsw i32 %i.0, 1
  br label %while2.cond

while2.end:
  ret i32 %i.0
}
