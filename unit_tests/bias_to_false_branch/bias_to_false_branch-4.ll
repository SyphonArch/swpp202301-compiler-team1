; RUN: opt < %s -load-pass-plugin=./build/libBiasToFalseBranch.so -passes=bias-to-false-branch -S | FileCheck %s
; Check if the pass considers side effect for inverting condition
; when the condition is used by other instructions other than branch.

declare void @f(i1)

; False branch has low probability ~= 0.03. (looping edge)
; However, the condition is used as a function argument.
; So, create inverted condition.
define i32 @main() {
; CHECK-LABEL: @main(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    br label %while.cond
; CHECK:       while.cond:
; CHECK-NEXT:    [[I_0:%.*]] = phi i32 [ 0, %entry ], [ [[ADD:%.*]], %while.body ]
; CHECK-NEXT:    [[CMP:%.*]] = icmp slt i32 [[I_0]], 10
; CHECK-NEXT:    [[NOT_CMP:%.*]] = select i1 [[CMP]], i1 false, i1 true
; CHECK-NEXT:    br i1 [[NOT_CMP]], label %while.end, label %while.body
; CHECK:       while.body:
; CHECK-NEXT:    [[ADD]] = add nsw i32 [[I_0]], 1
; CHECK-NEXT:    br label %while.cond
; CHECK:       while.end:
; CHECK-NEXT:    call void @f(i1 [[CMP]])
; CHECK-NEXT:    ret i32 [[I_0]]
;
entry:
  br label %while.cond

while.cond:                                       ; preds = %while.body, %entry
  %i.0 = phi i32 [ 0, %entry ], [ %add, %while.body ]
  %cmp = icmp slt i32 %i.0, 10
  br i1 %cmp, label %while.body, label %while.end

while.body:                                       ; preds = %while.cond
  %add = add nsw i32 %i.0, 1
  br label %while.cond

while.end:                                        ; preds = %while.cond
  call void @f(i1 %cmp)
  ret i32 %i.0
}
