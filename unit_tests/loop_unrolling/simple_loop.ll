; Test a simple loop with phi nodes

define i32 @sumLoop(i32 %N) {
; CHECK-LABEL: @sumLoop(i32 %N)
; CHECK: entry:
; CHECK-NEXT:   br label %loop.preheader
; CHECK: loop.preheader:                                   ; preds = %entry
; CHECK-NEXT:   br label %loop
; CHECK: loop:                                             ; preds = %latch.7, %loop.preheader
; CHECK-NEXT:   %sum = phi i32 [ 0, %loop.preheader ], [ %next_sum.7, %latch.7 ]
; CHECK-NEXT:   %i = phi i32 [ 1, %loop.preheader ], [ %next_i.7, %latch.7 ]
; CHECK-NEXT:   %exit_cond1 = icmp sgt i32 %N, %i
; CHECK-NEXT:   br i1 %exit_cond1, label %latch, label %exit
; CHECK: latch:                                            ; preds = %loop
; CHECK-NEXT:   %next_sum = add i32 %sum, %i
; CHECK-NEXT:   %next_i = add nuw nsw i32 %i, 1
; CHECK-NEXT:   %exit_cond1.1 = icmp sgt i32 %N, %next_i
; CHECK-NEXT:   br i1 %exit_cond1.1, label %latch.1, label %exit
; CHECK: latch.1:                                          ; preds = %latch
; CHECK-NEXT:   %next_sum.1 = add i32 %next_sum, %next_i
; CHECK-NEXT:   %next_i.1 = add nuw nsw i32 %next_i, 1
; CHECK-NEXT:   %exit_cond1.2 = icmp sgt i32 %N, %next_i.1
; CHECK-NEXT:   br i1 %exit_cond1.2, label %latch.2, label %exit
; CHECK: latch.2:                                          ; preds = %latch.1
; CHECK-NEXT:   %next_sum.2 = add i32 %next_sum.1, %next_i.1
; CHECK-NEXT:   %next_i.2 = add nuw nsw i32 %next_i.1, 1
; CHECK-NEXT:   %exit_cond1.3 = icmp sgt i32 %N, %next_i.2
; CHECK-NEXT:   br i1 %exit_cond1.3, label %latch.3, label %exit
; CHECK: latch.3:                                          ; preds = %latch.2
; CHECK-NEXT:   %next_sum.3 = add i32 %next_sum.2, %next_i.2
; CHECK-NEXT:   %next_i.3 = add nuw nsw i32 %next_i.2, 1
; CHECK-NEXT:   %exit_cond1.4 = icmp sgt i32 %N, %next_i.3
; CHECK-NEXT:   br i1 %exit_cond1.4, label %latch.4, label %exit
; CHECK: latch.4:                                          ; preds = %latch.3
; CHECK-NEXT:   %next_sum.4 = add i32 %next_sum.3, %next_i.3
; CHECK-NEXT:   %next_i.4 = add nuw nsw i32 %next_i.3, 1
; CHECK-NEXT:   %exit_cond1.5 = icmp sgt i32 %N, %next_i.4
; CHECK-NEXT:   br i1 %exit_cond1.5, label %latch.5, label %exit
; CHECK: latch.5:                                          ; preds = %latch.4
; CHECK-NEXT:   %next_sum.5 = add i32 %next_sum.4, %next_i.4
; CHECK-NEXT:   %next_i.5 = add nuw nsw i32 %next_i.4, 1
; CHECK-NEXT:   %exit_cond1.6 = icmp sgt i32 %N, %next_i.5
; CHECK-NEXT:   br i1 %exit_cond1.6, label %latch.6, label %exit
; CHECK: latch.6:                                          ; preds = %latch.5
; CHECK-NEXT:   %next_sum.6 = add i32 %next_sum.5, %next_i.5
; CHECK-NEXT:   %next_i.6 = add i32 %next_i.5, 1
; CHECK-NEXT:   %exit_cond1.7 = icmp sgt i32 %N, %next_i.6
; CHECK-NEXT:   br i1 %exit_cond1.7, label %latch.7, label %exit
; CHECK: latch.7:                                          ; preds = %latch.6
; CHECK-NEXT:   %next_sum.7 = add i32 %next_sum.6, %next_i.6
; CHECK-NEXT:   %next_i.7 = add nuw nsw i32 %next_i.6, 1
; CHECK-NEXT:   br label %loop
; CHECK: exit:                                             ; preds = %latch.6, %latch.5, %latch.4, %latch.3, %latch.2, %latch.1, %latch, %loop
; CHECK-NEXT:   %sum_ = phi i32 [ %sum, %loop ], [ %next_sum, %latch ], [ %next_sum.1, %latch.1 ], [ %next_sum.2, %latch.2 ], [ %next_sum.3, %latch.3 ], [ %next_sum.4, %latch.4 ], [ %next_sum.5, %latch.5 ], [ %next_sum.6, %latch.6 ]
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
  br label %loop

exit:
  %sum_ = phi i32 [ %sum, %loop ]
  ret i32 %sum_
}
