; Check that blocks within loops are merged

define i32 @sumLoop(i32 %N) {
; CHECK-LABEL: @sumLoop(i32 %N)
; CHECK: entry:
; CHECK-NEXT:   %exit_cond11 = icmp sgt i32 %N, 1
; CHECK-NEXT:   br i1 %exit_cond11, label %latch.lr.ph, label %exit
; CHECK: latch.lr.ph:                                      ; preds = %entry
; CHECK-NEXT:   %0 = add i32 %N, -1
; CHECK-NEXT:   %1 = add i32 %N, -2
; CHECK-NEXT:   %xtraiter = and i32 %0, 7
; CHECK-NEXT:   %2 = icmp ult i32 %1, 7
; CHECK-NEXT:   br i1 %2, label %loop.exit_crit_edge.unr-lcssa, label %latch.lr.ph.new
; CHECK: latch.lr.ph.new:                                  ; preds = %latch.lr.ph
; CHECK-NEXT:   %unroll_iter = sub i32 %0, %xtraiter
; CHECK-NEXT:   br label %latch
; CHECK: latch:                                            ; preds = %latch, %latch.lr.ph.new
; CHECK-NEXT:   %i3 = phi i32 [ 1, %latch.lr.ph.new ], [ %next_i.7, %latch ]
; CHECK-NEXT:   %sum2 = phi i32 [ 0, %latch.lr.ph.new ], [ %next_sum.7, %latch ]
; CHECK-NEXT:   %niter = phi i32 [ 0, %latch.lr.ph.new ], [ %niter.next.7, %latch ]
; CHECK-NEXT:   %next_sum = add i32 %sum2, %i3
; CHECK-NEXT:   %next_i = add nuw nsw i32 %i3, 1
; CHECK-NEXT:   %niter.next = add nuw nsw i32 %niter, 1
; CHECK-NEXT:   %next_sum.1 = add i32 %next_sum, %next_i
; CHECK-NEXT:   %next_i.1 = add nuw nsw i32 %next_i, 1
; CHECK-NEXT:   %niter.next.1 = add nuw nsw i32 %niter.next, 1
; CHECK-NEXT:   %next_sum.2 = add i32 %next_sum.1, %next_i.1
; CHECK-NEXT:   %next_i.2 = add nuw nsw i32 %next_i.1, 1
; CHECK-NEXT:   %niter.next.2 = add nuw nsw i32 %niter.next.1, 1
; CHECK-NEXT:   %next_sum.3 = add i32 %next_sum.2, %next_i.2
; CHECK-NEXT:   %next_i.3 = add nuw nsw i32 %next_i.2, 1
; CHECK-NEXT:   %niter.next.3 = add nuw nsw i32 %niter.next.2, 1
; CHECK-NEXT:   %next_sum.4 = add i32 %next_sum.3, %next_i.3
; CHECK-NEXT:   %next_i.4 = add nuw nsw i32 %next_i.3, 1
; CHECK-NEXT:   %niter.next.4 = add nuw nsw i32 %niter.next.3, 1
; CHECK-NEXT:   %next_sum.5 = add i32 %next_sum.4, %next_i.4
; CHECK-NEXT:   %next_i.5 = add nuw nsw i32 %next_i.4, 1
; CHECK-NEXT:   %niter.next.5 = add nuw nsw i32 %niter.next.4, 1
; CHECK-NEXT:   %next_sum.6 = add i32 %next_sum.5, %next_i.5
; CHECK-NEXT:   %next_i.6 = add i32 %next_i.5, 1
; CHECK-NEXT:   %niter.next.6 = add nuw nsw i32 %niter.next.5, 1
; CHECK-NEXT:   %next_sum.7 = add i32 %next_sum.6, %next_i.6
; CHECK-NEXT:   %next_i.7 = add nuw nsw i32 %next_i.6, 1
; CHECK-NEXT:   %niter.next.7 = add i32 %niter.next.6, 1
; CHECK-NEXT:   %niter.ncmp.7 = icmp ne i32 %niter.next.7, %unroll_iter
; CHECK-NEXT:   br i1 %niter.ncmp.7, label %latch, label %loop.exit_crit_edge.unr-lcssa.loopexit
; CHECK: loop.exit_crit_edge.unr-lcssa.loopexit:           ; preds = %latch
; CHECK-NEXT:   %split.ph.ph = phi i32 [ %next_sum.7, %latch ]
; CHECK-NEXT:   %i3.unr.ph = phi i32 [ %next_i.7, %latch ]
; CHECK-NEXT:   %sum2.unr.ph = phi i32 [ %next_sum.7, %latch ]
; CHECK-NEXT:   br label %loop.exit_crit_edge.unr-lcssa
; CHECK: loop.exit_crit_edge.unr-lcssa:                    ; preds = %loop.exit_crit_edge.unr-lcssa.loopexit, %latch.lr.ph
; CHECK-NEXT:   %split.ph = phi i32 [ undef, %latch.lr.ph ], [ %split.ph.ph, %loop.exit_crit_edge.unr-lcssa.loopexit ]
; CHECK-NEXT:   %i3.unr = phi i32 [ 1, %latch.lr.ph ], [ %i3.unr.ph, %loop.exit_crit_edge.unr-lcssa.loopexit ]
; CHECK-NEXT:   %sum2.unr = phi i32 [ 0, %latch.lr.ph ], [ %sum2.unr.ph, %loop.exit_crit_edge.unr-lcssa.loopexit ]
; CHECK-NEXT:   %lcmp.mod = icmp ne i32 %xtraiter, 0
; CHECK-NEXT:   br i1 %lcmp.mod, label %latch.epil.preheader, label %loop.exit_crit_edge
; CHECK: latch.epil.preheader:                             ; preds = %loop.exit_crit_edge.unr-lcssa
; CHECK-NEXT:   br label %latch.epil
; CHECK: latch.epil:                                       ; preds = %merge.epil, %latch.epil.preheader
; CHECK-NEXT:   %i3.epil = phi i32 [ %i3.unr, %latch.epil.preheader ], [ %next_i.epil, %merge.epil ]
; CHECK-NEXT:   %sum2.epil = phi i32 [ %sum2.unr, %latch.epil.preheader ], [ %next_sum.epil, %merge.epil ]
; CHECK-NEXT:   %epil.iter = phi i32 [ 0, %latch.epil.preheader ], [ %epil.iter.next, %merge.epil ]
; CHECK-NEXT:   br label %merge.epil
; CHECK: merge.epil:                                       ; preds = %latch.epil
; CHECK-NEXT:   %next_sum.epil = add i32 %sum2.epil, %i3.epil
; CHECK-NEXT:   %next_i.epil = add i32 %i3.epil, 1
; CHECK-NEXT:   %exit_cond1.epil = icmp sgt i32 %N, %next_i.epil
; CHECK-NEXT:   %epil.iter.next = add i32 %epil.iter, 1
; CHECK-NEXT:   %epil.iter.cmp = icmp ne i32 %epil.iter.next, %xtraiter
; CHECK-NEXT:   br i1 %epil.iter.cmp, label %latch.epil, label %loop.exit_crit_edge.epilog-lcssa, !llvm.loop !0
; CHECK: loop.exit_crit_edge.epilog-lcssa:                 ; preds = %merge.epil
; CHECK-NEXT:   %split.ph4 = phi i32 [ %next_sum.epil, %merge.epil ]
; CHECK-NEXT:   br label %loop.exit_crit_edge
; CHECK: loop.exit_crit_edge:                              ; preds = %loop.exit_crit_edge.unr-lcssa, %loop.exit_crit_edge.epilog-lcssa
; CHECK-NEXT:   %split = phi i32 [ %split.ph, %loop.exit_crit_edge.unr-lcssa ], [ %split.ph4, %loop.exit_crit_edge.epilog-lcssa ]
; CHECK-NEXT:   br label %exit
; CHECK: exit:                                             ; preds = %loop.exit_crit_edge, %entry
; CHECK-NEXT:   %sum_ = phi i32 [ %split, %loop.exit_crit_edge ], [ 0, %entry ]
; CHECK-NEXT:   ret i32 %sum_

entry:
  br label %loop
loop:
  %sum = phi i32 [ 0, %entry ], [ %next_sum, %merge ]
  %i = phi i32 [ 1, %entry ], [ %next_i, %merge ]
  %exit_cond1 = icmp sgt i32 %N, %i
  br i1 %exit_cond1, label %latch, label %exit

latch:
  br label %merge

merge:
  %next_sum = add i32 %sum, %i
  %next_i = add i32 %i, 1
  br label %loop

exit:
  %sum_ = phi i32 [ %sum, %loop ]
  ret i32 %sum_
}
