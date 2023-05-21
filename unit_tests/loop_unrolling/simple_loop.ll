; Test a simple loop with phi nodes

define i32 @sumLoop(i32 %N) {
; CHECK-LABEL: @sumLoop(i32 %N)
; CHECK: entry:
; CHECK-NEXT:   br label %loop
; CHECK: loop:
; CHECK-NEXT:   %sum = phi i32 [ 0, %entry ], [ %next_sum.7, %latch.7 ]
; CHECK-NEXT:   %i = phi i32 [ 1, %entry ], [ %next_i.7, %latch.7 ]
; CHECK-NEXT:   %exit_cond1 = icmp sgt i32 %N, %i
; CHECK-NEXT:   br i1 %exit_cond1, label %latch, label %exit
; CHECK: latch:
; CHECK-NEXT:   %next_sum = add i32 %sum, %i
; CHECK-NEXT:   %next_i = add nuw nsw i32 %i, 1
; CHECK-NEXT:   %exit_cond2 = icmp sgt i32 %N, %next_i
; CHECK-NEXT:   br i1 %exit_cond2, label %loop.1, label %exit
; CHECK: loop.1:
; CHECK-NEXT:   br i1 true, label %latch.1, label %exit
; CHECK: latch.1:
; CHECK-NEXT:   %next_sum.1 = add i32 %next_sum, %next_i
; CHECK-NEXT:   %next_i.1 = add nuw nsw i32 %next_i, 1
; CHECK-NEXT:   %exit_cond2.1 = icmp sgt i32 %N, %next_i.1
; CHECK-NEXT:   br i1 %exit_cond2.1, label %loop.2, label %exit
; CHECK: loop.2:
; CHECK-NEXT:   br i1 true, label %latch.2, label %exit
; CHECK: latch.2:
; CHECK-NEXT:   %next_sum.2 = add i32 %next_sum.1, %next_i.1
; CHECK-NEXT:   %next_i.2 = add nuw nsw i32 %next_i.1, 1
; CHECK-NEXT:   %exit_cond2.2 = icmp sgt i32 %N, %next_i.2
; CHECK-NEXT:   br i1 %exit_cond2.2, label %loop.3, label %exit
; CHECK: loop.3:
; CHECK-NEXT:   br i1 true, label %latch.3, label %exit
; CHECK: latch.3:
; CHECK-NEXT:   %next_sum.3 = add i32 %next_sum.2, %next_i.2
; CHECK-NEXT:   %next_i.3 = add nuw nsw i32 %next_i.2, 1
; CHECK-NEXT:   %exit_cond2.3 = icmp sgt i32 %N, %next_i.3
; CHECK-NEXT:   br i1 %exit_cond2.3, label %loop.4, label %exit
; CHECK: loop.4:
; CHECK-NEXT:   br i1 true, label %latch.4, label %exit
; CHECK: latch.4:
; CHECK-NEXT:   %next_sum.4 = add i32 %next_sum.3, %next_i.3
; CHECK-NEXT:   %next_i.4 = add nuw nsw i32 %next_i.3, 1
; CHECK-NEXT:   %exit_cond2.4 = icmp sgt i32 %N, %next_i.4
; CHECK-NEXT:   br i1 %exit_cond2.4, label %loop.5, label %exit
; CHECK: loop.5:
; CHECK-NEXT:   br i1 true, label %latch.5, label %exit
; CHECK: latch.5:
; CHECK-NEXT:   %next_sum.5 = add i32 %next_sum.4, %next_i.4
; CHECK-NEXT:   %next_i.5 = add nuw nsw i32 %next_i.4, 1
; CHECK-NEXT:   %exit_cond2.5 = icmp sgt i32 %N, %next_i.5
; CHECK-NEXT:   br i1 %exit_cond2.5, label %loop.6, label %exit
; CHECK: loop.6:
; CHECK-NEXT:   br i1 true, label %latch.6, label %exit
; CHECK: latch.6:
; CHECK-NEXT:   %next_sum.6 = add i32 %next_sum.5, %next_i.5
; CHECK-NEXT:   %next_i.6 = add i32 %next_i.5, 1
; CHECK-NEXT:   %exit_cond2.6 = icmp sgt i32 %N, %next_i.6
; CHECK-NEXT:   br i1 %exit_cond2.6, label %loop.7, label %exit
; CHECK: loop.7:
; CHECK-NEXT:   br i1 true, label %latch.7, label %exit
; CHECK: latch.7:
; CHECK-NEXT:   %next_sum.7 = add i32 %next_sum.6, %next_i.6
; CHECK-NEXT:   %next_i.7 = add nuw nsw i32 %next_i.6, 1
; CHECK-NEXT:   %exit_cond2.7 = icmp sgt i32 %N, %next_i.7
; CHECK-NEXT:   br i1 %exit_cond2.7, label %loop, label %exit
; CHECK: exit:
; CHECK-NEXT:   %sum_ = phi i32 [ %sum, %loop ], [ %sum, %latch ], [ %next_sum, %loop.1 ], [ %next_sum, %latch.1 ], [ %next_sum.1, %loop.2 ], [ %next_sum.1, %latch.2 ], [ %next_sum.2, %loop.3 ], [ %next_sum.2, %latch.3 ], [ %next_sum.3, %loop.4 ], [ %next_sum.3, %latch.4 ], [ %next_sum.4, %loop.5 ], [ %next_sum.4, %latch.5 ], [ %next_sum.5, %loop.6 ], [ %next_sum.5, %latch.6 ], [ %next_sum.6, %loop.7 ], [ %next_sum.6, %latch.7 ]
; CHECK-NEXT:   ret i32 %sum_

entry:
  br label %loop
loop:
  %sum = phi i32 [ 0, %entry ], [ %next_sum, %latch ]
  %i = phi i32 [ 1, %entry ], [ %next_i, %latch ]
  %exit_cond1 = icmp sgt i32 %N, %i
  br i1 %exit_cond1, label %latch, label %exit

latch:
  %next_sum = add i32 %sum, %i
  %next_i = add i32 %i, 1
  %exit_cond2 = icmp sgt i32 %N, %next_i
  br i1 %exit_cond2, label %loop, label %exit

exit:
  %sum_ = phi i32 [ %sum, %loop ], [ %sum, %latch ]
  ret i32 %sum_
}
