; Test runtime unrolling where the loop body is merged into a single block

%struct.HIP_vector_type = type {  %union.anon }
%union.anon = type { <2 x float> }

define void @pragma_unroll(ptr %queue, i32 %num_elements) {
; CHECK: entry:
; CHECK-NEXT:   br label %for.body.preheader
; CHECK: for.body.preheader:                               ; preds = %entry
; CHECK-NEXT:   %0 = add i32 %num_elements, -1
; CHECK-NEXT:   %xtraiter = and i32 %num_elements, 7
; CHECK-NEXT:   %1 = icmp ult i32 %0, 7
; CHECK-NEXT:   br i1 %1, label %exit.unr-lcssa, label %for.body.preheader.new
; CHECK: for.body.preheader.new:                           ; preds = %for.body.preheader
; CHECK-NEXT:   %unroll_iter = sub i32 %num_elements, %xtraiter
; CHECK-NEXT:   br label %for.body
; CHECK: for.body:                                         ; preds = %for.body, %for.body.preheader.new
; CHECK-NEXT:   %i.06 = phi i32 [ 0, %for.body.preheader.new ], [ %add.7, %for.body ]
; CHECK-NEXT:   %niter = phi i32 [ 0, %for.body.preheader.new ], [ %niter.next.7, %for.body ]
; CHECK-NEXT:   %add = add nuw nsw i32 %i.06, 1
; CHECK-NEXT:   %idxprom = zext i32 %add to i64
; CHECK-NEXT:   %arrayidx = getelementptr inbounds %struct.HIP_vector_type, ptr %queue, i64 %idxprom
; CHECK-NEXT:   store i64 %idxprom, ptr %arrayidx, align 8
; CHECK-NEXT:   %niter.next = add nuw nsw i32 %niter, 1
; CHECK-NEXT:   %add.1 = add nuw nsw i32 %add, 1
; CHECK-NEXT:   %idxprom.1 = zext i32 %add.1 to i64
; CHECK-NEXT:   %arrayidx.1 = getelementptr inbounds %struct.HIP_vector_type, ptr %queue, i64 %idxprom.1
; CHECK-NEXT:   store i64 %idxprom.1, ptr %arrayidx.1, align 8
; CHECK-NEXT:   %niter.next.1 = add nuw nsw i32 %niter.next, 1
; CHECK-NEXT:   %add.2 = add nuw nsw i32 %add.1, 1
; CHECK-NEXT:   %idxprom.2 = zext i32 %add.2 to i64
; CHECK-NEXT:   %arrayidx.2 = getelementptr inbounds %struct.HIP_vector_type, ptr %queue, i64 %idxprom.2
; CHECK-NEXT:   store i64 %idxprom.2, ptr %arrayidx.2, align 8
; CHECK-NEXT:   %niter.next.2 = add nuw nsw i32 %niter.next.1, 1
; CHECK-NEXT:   %add.3 = add nuw nsw i32 %add.2, 1
; CHECK-NEXT:   %idxprom.3 = zext i32 %add.3 to i64
; CHECK-NEXT:   %arrayidx.3 = getelementptr inbounds %struct.HIP_vector_type, ptr %queue, i64 %idxprom.3
; CHECK-NEXT:   store i64 %idxprom.3, ptr %arrayidx.3, align 8
; CHECK-NEXT:   %niter.next.3 = add nuw nsw i32 %niter.next.2, 1
; CHECK-NEXT:   %add.4 = add nuw nsw i32 %add.3, 1
; CHECK-NEXT:   %idxprom.4 = zext i32 %add.4 to i64
; CHECK-NEXT:   %arrayidx.4 = getelementptr inbounds %struct.HIP_vector_type, ptr %queue, i64 %idxprom.4
; CHECK-NEXT:   store i64 %idxprom.4, ptr %arrayidx.4, align 8
; CHECK-NEXT:   %niter.next.4 = add nuw nsw i32 %niter.next.3, 1
; CHECK-NEXT:   %add.5 = add nuw nsw i32 %add.4, 1
; CHECK-NEXT:   %idxprom.5 = zext i32 %add.5 to i64
; CHECK-NEXT:   %arrayidx.5 = getelementptr inbounds %struct.HIP_vector_type, ptr %queue, i64 %idxprom.5
; CHECK-NEXT:   store i64 %idxprom.5, ptr %arrayidx.5, align 8
; CHECK-NEXT:   %niter.next.5 = add nuw nsw i32 %niter.next.4, 1
; CHECK-NEXT:   %add.6 = add nuw nsw i32 %add.5, 1
; CHECK-NEXT:   %idxprom.6 = zext i32 %add.6 to i64
; CHECK-NEXT:   %arrayidx.6 = getelementptr inbounds %struct.HIP_vector_type, ptr %queue, i64 %idxprom.6
; CHECK-NEXT:   store i64 %idxprom.6, ptr %arrayidx.6, align 8
; CHECK-NEXT:   %niter.next.6 = add nuw nsw i32 %niter.next.5, 1
; CHECK-NEXT:   %add.7 = add nuw nsw i32 %add.6, 1
; CHECK-NEXT:   %idxprom.7 = zext i32 %add.7 to i64
; CHECK-NEXT:   %arrayidx.7 = getelementptr inbounds %struct.HIP_vector_type, ptr %queue, i64 %idxprom.7
; CHECK-NEXT:   store i64 %idxprom.7, ptr %arrayidx.7, align 8
; CHECK-NEXT:   %niter.next.7 = add i32 %niter.next.6, 1
; CHECK-NEXT:   %niter.ncmp.7 = icmp ne i32 %niter.next.7, %unroll_iter
; CHECK-NEXT:   br i1 %niter.ncmp.7, label %for.body, label %exit.unr-lcssa.loopexit
; CHECK: exit.unr-lcssa.loopexit:                          ; preds = %for.body
; CHECK-NEXT:   %i.06.unr.ph = phi i32 [ %add.7, %for.body ]
; CHECK-NEXT:   br label %exit.unr-lcssa
; CHECK: exit.unr-lcssa:                                   ; preds = %exit.unr-lcssa.loopexit, %for.body.preheader
; CHECK-NEXT:   %i.06.unr = phi i32 [ 0, %for.body.preheader ], [ %i.06.unr.ph, %exit.unr-lcssa.loopexit ]
; CHECK-NEXT:   %lcmp.mod = icmp ne i32 %xtraiter, 0
; CHECK-NEXT:   br i1 %lcmp.mod, label %for.body.epil.preheader, label %exit
; CHECK: for.body.epil.preheader:                          ; preds = %exit.unr-lcssa
; CHECK-NEXT:   br label %for.body.epil
; CHECK: for.body.epil:                                    ; preds = %for.body.epil.preheader
; CHECK-NEXT:   %add.epil = add nuw nsw i32 %i.06.unr, 1
; CHECK-NEXT:   %idxprom.epil = zext i32 %add.epil to i64
; CHECK-NEXT:   %arrayidx.epil = getelementptr inbounds %struct.HIP_vector_type, ptr %queue, i64 %idxprom.epil
; CHECK-NEXT:   store i64 %idxprom.epil, ptr %arrayidx.epil, align 8
; CHECK-NEXT:   %epil.iter.cmp = icmp ne i32 1, %xtraiter
; CHECK-NEXT:   br i1 %epil.iter.cmp, label %for.body.epil.1, label %exit.epilog-lcssa
; CHECK: for.body.epil.1:                                  ; preds = %for.body.epil
; CHECK-NEXT:   %add.epil.1 = add nuw nsw i32 %add.epil, 1
; CHECK-NEXT:   %idxprom.epil.1 = zext i32 %add.epil.1 to i64
; CHECK-NEXT:   %arrayidx.epil.1 = getelementptr inbounds %struct.HIP_vector_type, ptr %queue, i64 %idxprom.epil.1
; CHECK-NEXT:   store i64 %idxprom.epil.1, ptr %arrayidx.epil.1, align 8
; CHECK-NEXT:   %epil.iter.cmp.1 = icmp ne i32 2, %xtraiter
; CHECK-NEXT:   br i1 %epil.iter.cmp.1, label %for.body.epil.2, label %exit.epilog-lcssa
; CHECK: for.body.epil.2:                                  ; preds = %for.body.epil.1
; CHECK-NEXT:   %add.epil.2 = add nuw nsw i32 %add.epil.1, 1
; CHECK-NEXT:   %idxprom.epil.2 = zext i32 %add.epil.2 to i64
; CHECK-NEXT:   %arrayidx.epil.2 = getelementptr inbounds %struct.HIP_vector_type, ptr %queue, i64 %idxprom.epil.2
; CHECK-NEXT:   store i64 %idxprom.epil.2, ptr %arrayidx.epil.2, align 8
; CHECK-NEXT:   %epil.iter.cmp.2 = icmp ne i32 3, %xtraiter
; CHECK-NEXT:   br i1 %epil.iter.cmp.2, label %for.body.epil.3, label %exit.epilog-lcssa
; CHECK: for.body.epil.3:                                  ; preds = %for.body.epil.2
; CHECK-NEXT:   %add.epil.3 = add nuw nsw i32 %add.epil.2, 1
; CHECK-NEXT:   %idxprom.epil.3 = zext i32 %add.epil.3 to i64
; CHECK-NEXT:   %arrayidx.epil.3 = getelementptr inbounds %struct.HIP_vector_type, ptr %queue, i64 %idxprom.epil.3
; CHECK-NEXT:   store i64 %idxprom.epil.3, ptr %arrayidx.epil.3, align 8
; CHECK-NEXT:   %epil.iter.cmp.3 = icmp ne i32 4, %xtraiter
; CHECK-NEXT:   br i1 %epil.iter.cmp.3, label %for.body.epil.4, label %exit.epilog-lcssa
; CHECK: for.body.epil.4:                                  ; preds = %for.body.epil.3
; CHECK-NEXT:   %add.epil.4 = add nuw nsw i32 %add.epil.3, 1
; CHECK-NEXT:   %idxprom.epil.4 = zext i32 %add.epil.4 to i64
; CHECK-NEXT:   %arrayidx.epil.4 = getelementptr inbounds %struct.HIP_vector_type, ptr %queue, i64 %idxprom.epil.4
; CHECK-NEXT:   store i64 %idxprom.epil.4, ptr %arrayidx.epil.4, align 8
; CHECK-NEXT:   %epil.iter.cmp.4 = icmp ne i32 5, %xtraiter
; CHECK-NEXT:   br i1 %epil.iter.cmp.4, label %for.body.epil.5, label %exit.epilog-lcssa
; CHECK: for.body.epil.5:                                  ; preds = %for.body.epil.4
; CHECK-NEXT:   %add.epil.5 = add nuw nsw i32 %add.epil.4, 1
; CHECK-NEXT:   %idxprom.epil.5 = zext i32 %add.epil.5 to i64
; CHECK-NEXT:   %arrayidx.epil.5 = getelementptr inbounds %struct.HIP_vector_type, ptr %queue, i64 %idxprom.epil.5
; CHECK-NEXT:   store i64 %idxprom.epil.5, ptr %arrayidx.epil.5, align 8
; CHECK-NEXT:   %epil.iter.cmp.5 = icmp ne i32 6, %xtraiter
; CHECK-NEXT:   br i1 %epil.iter.cmp.5, label %for.body.epil.6, label %exit.epilog-lcssa
; CHECK: for.body.epil.6:                                  ; preds = %for.body.epil.5
; CHECK-NEXT:   %add.epil.6 = add nuw nsw i32 %add.epil.5, 1
; CHECK-NEXT:   %idxprom.epil.6 = zext i32 %add.epil.6 to i64
; CHECK-NEXT:   %arrayidx.epil.6 = getelementptr inbounds %struct.HIP_vector_type, ptr %queue, i64 %idxprom.epil.6
; CHECK-NEXT:   store i64 %idxprom.epil.6, ptr %arrayidx.epil.6, align 8
; CHECK-NEXT:   br label %exit.epilog-lcssa
; CHECK: exit.epilog-lcssa:                                ; preds = %for.body.epil.6, %for.body.epil.5, %for.body.epil.4, %for.body.epil.3, %for.body.epil.2, %for.body.epil.1, %for.body.epil
; CHECK-NEXT:   br label %exit
; CHECK: exit:                                             ; preds = %exit.unr-lcssa, %exit.epilog-lcssa
; CHECK-NEXT:   ret void

entry:
  br label %for.body.preheader
for.body.preheader:
  br label %for.body
for.body:
  %i.06 = phi i32 [ %add, %for.body ], [ 0, %for.body.preheader ]
  %add = add nuw nsw i32 %i.06, 1
  %idxprom = zext i32 %add to i64
  %arrayidx = getelementptr inbounds %struct.HIP_vector_type, ptr %queue, i64 %idxprom
  store i64 %idxprom, ptr %arrayidx, align 8
  %exitcond = icmp ne i32 %add, %num_elements
  br i1 %exitcond, label %for.body, label %exit
exit:
  ret void
}