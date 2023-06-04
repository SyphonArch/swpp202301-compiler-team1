; RUN: opt < %s -load-pass-plugin=./build/libSimplifyCFG.so -passes=simplify-cfg -S | FileCheck %s

; Check if SimplifyCFGPass folds branch to a common destination.

define void @fold_branch(i32 %a, i32 %b) {
; CHECK-LABEL: @fold_branch(
; CHECK-NEXT:    [[CMP1:%.*]] = icmp eq i32 [[A:%.*]], [[B:%.*]]
; CHECK-NEXT:    [[CMP2:%.*]] = icmp ugt i32 [[A]], 0
; CHECK-NEXT:    [[OR_COND:%.*]] = and i1 [[CMP1]], [[CMP2]]
; CHECK-NEXT:    br i1 [[OR_COND]], label [[ELSE:%.*]], label [[COMMON_RET:%.*]]
; CHECK:       common.ret:
; CHECK-NEXT:    ret void
; CHECK:       else:
; CHECK-NEXT:    call void @foo()
; CHECK-NEXT:    br label [[COMMON_RET]]
;
  %cmp1 = icmp eq i32 %a, %b
  br i1 %cmp1, label %taken, label %untaken

taken:
  %cmp2 = icmp ugt i32 %a, 0
  br i1 %cmp2, label %else, label %untaken

else:
  call void @foo()
  ret void

untaken:
  ret void
}

declare void @foo()
