; RUN: opt < %s -load-pass-plugin=./build/libBiasToFalseBranch.so -passes=bias-to-false-branch -S | FileCheck %s
; Check basic functionality of the pass, especially when condition is used for only one branch.

; true branch is looping edge, so inverting condition is needed
; basically, looping edge has prob ~= 97%.
define i32 @true_is_probable() {
; CHECK-LABEL: @true_is_probable(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    br label %while.cond
; CHECK:       while.cond:
; CHECK-NEXT:    [[I_0:%.*]] = phi i32 [ 0, %entry ], [ [[ADD:%.*]], %while.body ]
; CHECK-NEXT:    [[CMP:%.*]] = icmp sge i32 [[I_0]], 10
; CHECK-NEXT:    br i1 [[CMP]], label %while.end, label %while.body
; CHECK:       while.body:
; CHECK-NEXT:    [[ADD]] = add nsw i32 [[I_0]], 1
; CHECK-NEXT:    br label %while.cond
; CHECK:       while.end:
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
  ret i32 %i.0
}

; false branch is looping edge, so do nothing
define i32 @false_is_probable() {
; CHECK-LABEL: @false_is_probable(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    br label %while.cond
; CHECK:       while.cond:
; CHECK-NEXT:    [[I_0:%.*]] = phi i32 [ 0, %entry ], [ [[ADD:%.*]], %while.body ]
; CHECK-NEXT:    [[CMP:%.*]] = icmp eq i32 [[I_0]], 10
; CHECK-NEXT:    br i1 [[CMP]], label %while.end, label %while.body
; CHECK:       while.body:
; CHECK-NEXT:    [[ADD]] = add nsw i32 [[I_0]], 1
; CHECK-NEXT:    br label %while.cond
; CHECK:       while.end:
; CHECK-NEXT:    ret i32 [[I_0]]
;
entry:
  br label %while.cond

while.cond:                                       ; preds = %while.body, %entry
  %i.0 = phi i32 [ 0, %entry ], [ %add, %while.body ]
  %cmp = icmp eq i32 %i.0, 10
  br i1 %cmp, label %while.end, label %while.body

while.body:                                       ; preds = %while.cond
  %add = add nsw i32 %i.0, 1
  br label %while.cond

while.end:                                        ; preds = %while.cond
  ret i32 %i.0
}

; false branch has low probability (looping edge)
; However, the condition is not icmp. So, create inverted condition.
define i32 @condition_is_not_icmp(i1 %x, i1 %y) {
; CHECK-LABEL: @condition_is_not_icmp(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    br label %while.cond
; CHECK:       while.cond:
; CHECK-NEXT:    [[I_0:%.*]] = phi i32 [ 0, %entry ], [ [[ADD:%.*]], %while.body ]
; CHECK-NEXT:    [[CMP:%.*]] = or i1 [[X:%.*]], [[Y:%.*]]
; CHECK-NEXT:    [[NOT_CMP:%.*]] = select i1 %cmp, i1 false, i1 true
; CHECK-NEXT:    br i1 [[NOT_CMP]], label %while.end, label %while.body
; CHECK:       while.body:
; CHECK-NEXT:    [[ADD]] = add nsw i32 [[I_0]], 1
; CHECK-NEXT:    br label %while.cond
; CHECK:       while.end:
; CHECK-NEXT:    ret i32 [[I_0]]
;
entry:
  br label %while.cond

while.cond:                                       ; preds = %while.body, %entry
  %i.0 = phi i32 [ 0, %entry ], [ %add, %while.body ]
  %cmp = or i1 %x, %y
  br i1 %cmp, label %while.body, label %while.end

while.body:                                       ; preds = %while.cond
  %add = add nsw i32 %i.0, 1
  br label %while.cond

while.end:                                        ; preds = %while.cond
  ret i32 %i.0
}
