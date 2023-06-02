; Test on a loop with an if-else structure
; Runtime unrolling will happen on this loop

define i32 @exampleLoop(i32 %N) {
; CHECK-LABEL: @exampleLoop(i32 %N)
; CHECK: entry:
; CHECK-NEXT:   %0 = icmp slt i32 %N, 0
; CHECK-NEXT:   br i1 %0, label %exit, label %loop.preheader
; CHECK: loop.preheader:                                   ; preds = %entry
; CHECK-NEXT:   %smax = call i32 @llvm.smax.i32(i32 %N, i32 1)
; CHECK-NEXT:   %1 = add nsw i32 %smax, -1
; CHECK-NEXT:   %xtraiter = and i32 %smax, 7
; CHECK-NEXT:   %2 = icmp ult i32 %1, 7
; CHECK-NEXT:   br i1 %2, label %exit.loopexit.unr-lcssa, label %loop.preheader.new
; CHECK: loop.preheader.new:                               ; preds = %loop.preheader
; CHECK-NEXT:   %unroll_iter = sub i32 %smax, %xtraiter
; CHECK-NEXT:   br label %loop
; CHECK: loop:                                             ; preds = %if.end.7, %loop.preheader.new
; CHECK-NEXT:   %i = phi i32 [ 0, %loop.preheader.new ], [ %next.7, %if.end.7 ]
; CHECK-NEXT:   %even_sum = phi i32 [ 0, %loop.preheader.new ], [ %even_sum_next_.7, %if.end.7 ]
; CHECK-NEXT:   %odd_sum = phi i32 [ 0, %loop.preheader.new ], [ %odd_sum_next_.7, %if.end.7 ]
; CHECK-NEXT:   %niter = phi i32 [ 0, %loop.preheader.new ], [ %niter.next.7, %if.end.7 ]
; CHECK-NEXT:   %is_even = srem i32 %i, 2
; CHECK-NEXT:   %is_even_zero = icmp eq i32 %is_even, 0
; CHECK-NEXT:   br i1 %is_even_zero, label %if.then, label %if.else
; CHECK: if.then:                                          ; preds = %loop
; CHECK-NEXT:   %even_sum_next = add i32 %even_sum, %i
; CHECK-NEXT:   br label %if.end
; CHECK: if.else:                                          ; preds = %loop
; CHECK-NEXT:   %odd_sum_next = add i32 %odd_sum, %i
; CHECK-NEXT:   br label %if.end
; CHECK: if.end:                                           ; preds = %if.else, %if.then
; CHECK-NEXT:   %even_sum_next_ = phi i32 [ %even_sum, %if.else ], [ %even_sum_next, %if.then ]
; CHECK-NEXT:   %odd_sum_next_ = phi i32 [ %odd_sum_next, %if.else ], [ %odd_sum, %if.then ]
; CHECK-NEXT:   %next = add nuw nsw i32 %i, 1
; CHECK-NEXT:   %niter.next = add nuw nsw i32 %niter, 1
; CHECK-NEXT:   %is_even.1 = srem i32 %next, 2
; CHECK-NEXT:   %is_even_zero.1 = icmp eq i32 %is_even.1, 0
; CHECK-NEXT:   br i1 %is_even_zero.1, label %if.then.1, label %if.else.1
; CHECK: if.else.1:                                        ; preds = %if.end
; CHECK-NEXT:   %odd_sum_next.1 = add i32 %odd_sum_next_, %next
; CHECK-NEXT:   br label %if.end.1
; CHECK: if.then.1:                                        ; preds = %if.end
; CHECK-NEXT:   %even_sum_next.1 = add i32 %even_sum_next_, %next
; CHECK-NEXT:   br label %if.end.1
; CHECK: if.end.1:                                         ; preds = %if.then.1, %if.else.1
; CHECK-NEXT:   %even_sum_next_.1 = phi i32 [ %even_sum_next_, %if.else.1 ], [ %even_sum_next.1, %if.then.1 ]
; CHECK-NEXT:   %odd_sum_next_.1 = phi i32 [ %odd_sum_next.1, %if.else.1 ], [ %odd_sum_next_, %if.then.1 ]
; CHECK-NEXT:   %next.1 = add nuw nsw i32 %next, 1
; CHECK-NEXT:   %niter.next.1 = add nuw nsw i32 %niter.next, 1
; CHECK-NEXT:   %is_even.2 = srem i32 %next.1, 2
; CHECK-NEXT:   %is_even_zero.2 = icmp eq i32 %is_even.2, 0
; CHECK-NEXT:   br i1 %is_even_zero.2, label %if.then.2, label %if.else.2
; CHECK: if.else.2:                                        ; preds = %if.end.1
; CHECK-NEXT:   %odd_sum_next.2 = add i32 %odd_sum_next_.1, %next.1
; CHECK-NEXT:   br label %if.end.2
; CHECK: if.then.2:                                        ; preds = %if.end.1
; CHECK-NEXT:   %even_sum_next.2 = add i32 %even_sum_next_.1, %next.1
; CHECK-NEXT:   br label %if.end.2
; CHECK: if.end.2:                                         ; preds = %if.then.2, %if.else.2
; CHECK-NEXT:   %even_sum_next_.2 = phi i32 [ %even_sum_next_.1, %if.else.2 ], [ %even_sum_next.2, %if.then.2 ]
; CHECK-NEXT:   %odd_sum_next_.2 = phi i32 [ %odd_sum_next.2, %if.else.2 ], [ %odd_sum_next_.1, %if.then.2 ]
; CHECK-NEXT:   %next.2 = add nuw nsw i32 %next.1, 1
; CHECK-NEXT:   %niter.next.2 = add nuw nsw i32 %niter.next.1, 1
; CHECK-NEXT:   %is_even.3 = srem i32 %next.2, 2
; CHECK-NEXT:   %is_even_zero.3 = icmp eq i32 %is_even.3, 0
; CHECK-NEXT:   br i1 %is_even_zero.3, label %if.then.3, label %if.else.3
; CHECK: if.else.3:                                        ; preds = %if.end.2
; CHECK-NEXT:   %odd_sum_next.3 = add i32 %odd_sum_next_.2, %next.2
; CHECK-NEXT:   br label %if.end.3
; CHECK: if.then.3:                                        ; preds = %if.end.2
; CHECK-NEXT:   %even_sum_next.3 = add i32 %even_sum_next_.2, %next.2
; CHECK-NEXT:   br label %if.end.3
; CHECK: if.end.3:                                         ; preds = %if.then.3, %if.else.3
; CHECK-NEXT:   %even_sum_next_.3 = phi i32 [ %even_sum_next_.2, %if.else.3 ], [ %even_sum_next.3, %if.then.3 ]
; CHECK-NEXT:   %odd_sum_next_.3 = phi i32 [ %odd_sum_next.3, %if.else.3 ], [ %odd_sum_next_.2, %if.then.3 ]
; CHECK-NEXT:   %next.3 = add nuw nsw i32 %next.2, 1
; CHECK-NEXT:   %niter.next.3 = add nuw nsw i32 %niter.next.2, 1
; CHECK-NEXT:   %is_even.4 = srem i32 %next.3, 2
; CHECK-NEXT:   %is_even_zero.4 = icmp eq i32 %is_even.4, 0
; CHECK-NEXT:   br i1 %is_even_zero.4, label %if.then.4, label %if.else.4
; CHECK: if.else.4:                                        ; preds = %if.end.3
; CHECK-NEXT:   %odd_sum_next.4 = add i32 %odd_sum_next_.3, %next.3
; CHECK-NEXT:   br label %if.end.4
; CHECK: if.then.4:                                        ; preds = %if.end.3
; CHECK-NEXT:   %even_sum_next.4 = add i32 %even_sum_next_.3, %next.3
; CHECK-NEXT:   br label %if.end.4
; CHECK: if.end.4:                                         ; preds = %if.then.4, %if.else.4
; CHECK-NEXT:   %even_sum_next_.4 = phi i32 [ %even_sum_next_.3, %if.else.4 ], [ %even_sum_next.4, %if.then.4 ]
; CHECK-NEXT:   %odd_sum_next_.4 = phi i32 [ %odd_sum_next.4, %if.else.4 ], [ %odd_sum_next_.3, %if.then.4 ]
; CHECK-NEXT:   %next.4 = add nuw nsw i32 %next.3, 1
; CHECK-NEXT:   %niter.next.4 = add nuw nsw i32 %niter.next.3, 1
; CHECK-NEXT:   %is_even.5 = srem i32 %next.4, 2
; CHECK-NEXT:   %is_even_zero.5 = icmp eq i32 %is_even.5, 0
; CHECK-NEXT:   br i1 %is_even_zero.5, label %if.then.5, label %if.else.5
; CHECK: if.else.5:                                        ; preds = %if.end.4
; CHECK-NEXT:   %odd_sum_next.5 = add i32 %odd_sum_next_.4, %next.4
; CHECK-NEXT:   br label %if.end.5
; CHECK: if.then.5:                                        ; preds = %if.end.4
; CHECK-NEXT:   %even_sum_next.5 = add i32 %even_sum_next_.4, %next.4
; CHECK-NEXT:   br label %if.end.5
; CHECK: if.end.5:                                         ; preds = %if.then.5, %if.else.5
; CHECK-NEXT:   %even_sum_next_.5 = phi i32 [ %even_sum_next_.4, %if.else.5 ], [ %even_sum_next.5, %if.then.5 ]
; CHECK-NEXT:   %odd_sum_next_.5 = phi i32 [ %odd_sum_next.5, %if.else.5 ], [ %odd_sum_next_.4, %if.then.5 ]
; CHECK-NEXT:   %next.5 = add nuw nsw i32 %next.4, 1
; CHECK-NEXT:   %niter.next.5 = add nuw nsw i32 %niter.next.4, 1
; CHECK-NEXT:   %is_even.6 = srem i32 %next.5, 2
; CHECK-NEXT:   %is_even_zero.6 = icmp eq i32 %is_even.6, 0
; CHECK-NEXT:   br i1 %is_even_zero.6, label %if.then.6, label %if.else.6
; CHECK: if.else.6:                                        ; preds = %if.end.5
; CHECK-NEXT:   %odd_sum_next.6 = add i32 %odd_sum_next_.5, %next.5
; CHECK-NEXT:   br label %if.end.6
; CHECK: if.then.6:                                        ; preds = %if.end.5
; CHECK-NEXT:   %even_sum_next.6 = add i32 %even_sum_next_.5, %next.5
; CHECK-NEXT:   br label %if.end.6
; CHECK: if.end.6:                                         ; preds = %if.then.6, %if.else.6
; CHECK-NEXT:   %even_sum_next_.6 = phi i32 [ %even_sum_next_.5, %if.else.6 ], [ %even_sum_next.6, %if.then.6 ]
; CHECK-NEXT:   %odd_sum_next_.6 = phi i32 [ %odd_sum_next.6, %if.else.6 ], [ %odd_sum_next_.5, %if.then.6 ]
; CHECK-NEXT:   %next.6 = add nuw nsw i32 %next.5, 1
; CHECK-NEXT:   %niter.next.6 = add nuw nsw i32 %niter.next.5, 1
; CHECK-NEXT:   %is_even.7 = srem i32 %next.6, 2
; CHECK-NEXT:   %is_even_zero.7 = icmp eq i32 %is_even.7, 0
; CHECK-NEXT:   br i1 %is_even_zero.7, label %if.then.7, label %if.else.7
; CHECK: if.else.7:                                        ; preds = %if.end.6
; CHECK-NEXT:   %odd_sum_next.7 = add i32 %odd_sum_next_.6, %next.6
; CHECK-NEXT:   br label %if.end.7
; CHECK: if.then.7:                                        ; preds = %if.end.6
; CHECK-NEXT:   %even_sum_next.7 = add i32 %even_sum_next_.6, %next.6
; CHECK-NEXT:   br label %if.end.7
; CHECK: if.end.7:                                         ; preds = %if.then.7, %if.else.7
; CHECK-NEXT:   %even_sum_next_.7 = phi i32 [ %even_sum_next_.6, %if.else.7 ], [ %even_sum_next.7, %if.then.7 ]
; CHECK-NEXT:   %odd_sum_next_.7 = phi i32 [ %odd_sum_next.7, %if.else.7 ], [ %odd_sum_next_.6, %if.then.7 ]
; CHECK-NEXT:   %next.7 = add i32 %next.6, 1
; CHECK-NEXT:   %niter.next.7 = add i32 %niter.next.6, 1
; CHECK-NEXT:   %niter.ncmp.7 = icmp ne i32 %niter.next.7, %unroll_iter
; CHECK-NEXT:   br i1 %niter.ncmp.7, label %loop, label %exit.loopexit.unr-lcssa.loopexit
; CHECK: exit.loopexit.unr-lcssa.loopexit:                 ; preds = %if.end.7
; CHECK-NEXT:   %even_sum_.ph.ph.ph = phi i32 [ %even_sum_next_.6, %if.end.7 ]
; CHECK-NEXT:   %odd_sum_.ph.ph.ph = phi i32 [ %odd_sum_next_.6, %if.end.7 ]
; CHECK-NEXT:   %i.unr.ph = phi i32 [ %next.7, %if.end.7 ]
; CHECK-NEXT:   %even_sum.unr.ph = phi i32 [ %even_sum_next_.7, %if.end.7 ]
; CHECK-NEXT:   %odd_sum.unr.ph = phi i32 [ %odd_sum_next_.7, %if.end.7 ]
; CHECK-NEXT:   br label %exit.loopexit.unr-lcssa
; CHECK: exit.loopexit.unr-lcssa:                          ; preds = %exit.loopexit.unr-lcssa.loopexit, %loop.preheader
; CHECK-NEXT:   %even_sum_.ph.ph = phi i32 [ undef, %loop.preheader ], [ %even_sum_.ph.ph.ph, %exit.loopexit.unr-lcssa.loopexit ]
; CHECK-NEXT:   %odd_sum_.ph.ph = phi i32 [ undef, %loop.preheader ], [ %odd_sum_.ph.ph.ph, %exit.loopexit.unr-lcssa.loopexit ]
; CHECK-NEXT:   %i.unr = phi i32 [ 0, %loop.preheader ], [ %i.unr.ph, %exit.loopexit.unr-lcssa.loopexit ]
; CHECK-NEXT:   %even_sum.unr = phi i32 [ 0, %loop.preheader ], [ %even_sum.unr.ph, %exit.loopexit.unr-lcssa.loopexit ]
; CHECK-NEXT:   %odd_sum.unr = phi i32 [ 0, %loop.preheader ], [ %odd_sum.unr.ph, %exit.loopexit.unr-lcssa.loopexit ]
; CHECK-NEXT:   %lcmp.mod = icmp ne i32 %xtraiter, 0
; CHECK-NEXT:   br i1 %lcmp.mod, label %loop.epil.preheader, label %exit.loopexit
; CHECK: loop.epil.preheader:                              ; preds = %exit.loopexit.unr-lcssa
; CHECK-NEXT:   br label %loop.epil
; CHECK: loop.epil:                                        ; preds = %loop.epil.preheader
; CHECK-NEXT:   %is_even.epil = srem i32 %i.unr, 2
; CHECK-NEXT:   %is_even_zero.epil = icmp eq i32 %is_even.epil, 0
; CHECK-NEXT:   br i1 %is_even_zero.epil, label %if.then.epil, label %if.else.epil
; CHECK: if.else.epil:                                     ; preds = %loop.epil
; CHECK-NEXT:   %odd_sum_next.epil = add i32 %odd_sum.unr, %i.unr
; CHECK-NEXT:   br label %if.end.epil
; CHECK: if.then.epil:                                     ; preds = %loop.epil
; CHECK-NEXT:   %even_sum_next.epil = add i32 %even_sum.unr, %i.unr
; CHECK-NEXT:   br label %if.end.epil
; CHECK: if.end.epil:                                      ; preds = %if.then.epil, %if.else.epil
; CHECK-NEXT:   %even_sum_next_.epil = phi i32 [ %even_sum.unr, %if.else.epil ], [ %even_sum_next.epil, %if.then.epil ]
; CHECK-NEXT:   %odd_sum_next_.epil = phi i32 [ %odd_sum_next.epil, %if.else.epil ], [ %odd_sum.unr, %if.then.epil ]
; CHECK-NEXT:   %next.epil = add i32 %i.unr, 1
; CHECK-NEXT:   %epil.iter.cmp = icmp ne i32 1, %xtraiter
; CHECK-NEXT:   br i1 %epil.iter.cmp, label %loop.epil.1, label %exit.loopexit.epilog-lcssa
; CHECK: loop.epil.1:                                      ; preds = %if.end.epil
; CHECK-NEXT:   %is_even.epil.1 = srem i32 %next.epil, 2
; CHECK-NEXT:   %is_even_zero.epil.1 = icmp eq i32 %is_even.epil.1, 0
; CHECK-NEXT:   br i1 %is_even_zero.epil.1, label %if.then.epil.1, label %if.else.epil.1
; CHECK: if.else.epil.1:                                   ; preds = %loop.epil.1
; CHECK-NEXT:   %odd_sum_next.epil.1 = add i32 %odd_sum_next_.epil, %next.epil
; CHECK-NEXT:   br label %if.end.epil.1
; CHECK: if.then.epil.1:                                   ; preds = %loop.epil.1
; CHECK-NEXT:   %even_sum_next.epil.1 = add i32 %even_sum_next_.epil, %next.epil
; CHECK-NEXT:   br label %if.end.epil.1
; CHECK: if.end.epil.1:                                    ; preds = %if.then.epil.1, %if.else.epil.1
; CHECK-NEXT:   %even_sum_next_.epil.1 = phi i32 [ %even_sum_next_.epil, %if.else.epil.1 ], [ %even_sum_next.epil.1, %if.then.epil.1 ]
; CHECK-NEXT:   %odd_sum_next_.epil.1 = phi i32 [ %odd_sum_next.epil.1, %if.else.epil.1 ], [ %odd_sum_next_.epil, %if.then.epil.1 ]
; CHECK-NEXT:   %next.epil.1 = add i32 %next.epil, 1
; CHECK-NEXT:   %epil.iter.cmp.1 = icmp ne i32 2, %xtraiter
; CHECK-NEXT:   br i1 %epil.iter.cmp.1, label %loop.epil.2, label %exit.loopexit.epilog-lcssa
; CHECK: loop.epil.2:                                      ; preds = %if.end.epil.1
; CHECK-NEXT:   %is_even.epil.2 = srem i32 %next.epil.1, 2
; CHECK-NEXT:   %is_even_zero.epil.2 = icmp eq i32 %is_even.epil.2, 0
; CHECK-NEXT:   br i1 %is_even_zero.epil.2, label %if.then.epil.2, label %if.else.epil.2
; CHECK: if.else.epil.2:                                   ; preds = %loop.epil.2
; CHECK-NEXT:   %odd_sum_next.epil.2 = add i32 %odd_sum_next_.epil.1, %next.epil.1
; CHECK-NEXT:   br label %if.end.epil.2
; CHECK: if.then.epil.2:                                   ; preds = %loop.epil.2
; CHECK-NEXT:   %even_sum_next.epil.2 = add i32 %even_sum_next_.epil.1, %next.epil.1
; CHECK-NEXT:   br label %if.end.epil.2
; CHECK: if.end.epil.2:                                    ; preds = %if.then.epil.2, %if.else.epil.2
; CHECK-NEXT:   %even_sum_next_.epil.2 = phi i32 [ %even_sum_next_.epil.1, %if.else.epil.2 ], [ %even_sum_next.epil.2, %if.then.epil.2 ]
; CHECK-NEXT:   %odd_sum_next_.epil.2 = phi i32 [ %odd_sum_next.epil.2, %if.else.epil.2 ], [ %odd_sum_next_.epil.1, %if.then.epil.2 ]
; CHECK-NEXT:   %next.epil.2 = add i32 %next.epil.1, 1
; CHECK-NEXT:   %epil.iter.cmp.2 = icmp ne i32 3, %xtraiter
; CHECK-NEXT:   br i1 %epil.iter.cmp.2, label %loop.epil.3, label %exit.loopexit.epilog-lcssa
; CHECK: loop.epil.3:                                      ; preds = %if.end.epil.2
; CHECK-NEXT:   %is_even.epil.3 = srem i32 %next.epil.2, 2
; CHECK-NEXT:   %is_even_zero.epil.3 = icmp eq i32 %is_even.epil.3, 0
; CHECK-NEXT:   br i1 %is_even_zero.epil.3, label %if.then.epil.3, label %if.else.epil.3
; CHECK: if.else.epil.3:                                   ; preds = %loop.epil.3
; CHECK-NEXT:   %odd_sum_next.epil.3 = add i32 %odd_sum_next_.epil.2, %next.epil.2
; CHECK-NEXT:   br label %if.end.epil.3
; CHECK: if.then.epil.3:                                   ; preds = %loop.epil.3
; CHECK-NEXT:   %even_sum_next.epil.3 = add i32 %even_sum_next_.epil.2, %next.epil.2
; CHECK-NEXT:   br label %if.end.epil.3
; CHECK: if.end.epil.3:                                    ; preds = %if.then.epil.3, %if.else.epil.3
; CHECK-NEXT:   %even_sum_next_.epil.3 = phi i32 [ %even_sum_next_.epil.2, %if.else.epil.3 ], [ %even_sum_next.epil.3, %if.then.epil.3 ]
; CHECK-NEXT:   %odd_sum_next_.epil.3 = phi i32 [ %odd_sum_next.epil.3, %if.else.epil.3 ], [ %odd_sum_next_.epil.2, %if.then.epil.3 ]
; CHECK-NEXT:   %next.epil.3 = add i32 %next.epil.2, 1
; CHECK-NEXT:   %epil.iter.cmp.3 = icmp ne i32 4, %xtraiter
; CHECK-NEXT:   br i1 %epil.iter.cmp.3, label %loop.epil.4, label %exit.loopexit.epilog-lcssa
; CHECK: loop.epil.4:                                      ; preds = %if.end.epil.3
; CHECK-NEXT:   %is_even.epil.4 = srem i32 %next.epil.3, 2
; CHECK-NEXT:   %is_even_zero.epil.4 = icmp eq i32 %is_even.epil.4, 0
; CHECK-NEXT:   br i1 %is_even_zero.epil.4, label %if.then.epil.4, label %if.else.epil.4
; CHECK: if.else.epil.4:                                   ; preds = %loop.epil.4
; CHECK-NEXT:   %odd_sum_next.epil.4 = add i32 %odd_sum_next_.epil.3, %next.epil.3
; CHECK-NEXT:   br label %if.end.epil.4
; CHECK: if.then.epil.4:                                   ; preds = %loop.epil.4
; CHECK-NEXT:   %even_sum_next.epil.4 = add i32 %even_sum_next_.epil.3, %next.epil.3
; CHECK-NEXT:   br label %if.end.epil.4
; CHECK: if.end.epil.4:                                    ; preds = %if.then.epil.4, %if.else.epil.4
; CHECK-NEXT:   %even_sum_next_.epil.4 = phi i32 [ %even_sum_next_.epil.3, %if.else.epil.4 ], [ %even_sum_next.epil.4, %if.then.epil.4 ]
; CHECK-NEXT:   %odd_sum_next_.epil.4 = phi i32 [ %odd_sum_next.epil.4, %if.else.epil.4 ], [ %odd_sum_next_.epil.3, %if.then.epil.4 ]
; CHECK-NEXT:   %next.epil.4 = add i32 %next.epil.3, 1
; CHECK-NEXT:   %epil.iter.cmp.4 = icmp ne i32 5, %xtraiter
; CHECK-NEXT:   br i1 %epil.iter.cmp.4, label %loop.epil.5, label %exit.loopexit.epilog-lcssa
; CHECK: loop.epil.5:                                      ; preds = %if.end.epil.4
; CHECK-NEXT:   %is_even.epil.5 = srem i32 %next.epil.4, 2
; CHECK-NEXT:   %is_even_zero.epil.5 = icmp eq i32 %is_even.epil.5, 0
; CHECK-NEXT:   br i1 %is_even_zero.epil.5, label %if.then.epil.5, label %if.else.epil.5
; CHECK: if.else.epil.5:                                   ; preds = %loop.epil.5
; CHECK-NEXT:   %odd_sum_next.epil.5 = add i32 %odd_sum_next_.epil.4, %next.epil.4
; CHECK-NEXT:   br label %if.end.epil.5
; CHECK: if.then.epil.5:                                   ; preds = %loop.epil.5
; CHECK-NEXT:   %even_sum_next.epil.5 = add i32 %even_sum_next_.epil.4, %next.epil.4
; CHECK-NEXT:   br label %if.end.epil.5
; CHECK: if.end.epil.5:                                    ; preds = %if.then.epil.5, %if.else.epil.5
; CHECK-NEXT:   %even_sum_next_.epil.5 = phi i32 [ %even_sum_next_.epil.4, %if.else.epil.5 ], [ %even_sum_next.epil.5, %if.then.epil.5 ]
; CHECK-NEXT:   %odd_sum_next_.epil.5 = phi i32 [ %odd_sum_next.epil.5, %if.else.epil.5 ], [ %odd_sum_next_.epil.4, %if.then.epil.5 ]
; CHECK-NEXT:   %next.epil.5 = add i32 %next.epil.4, 1
; CHECK-NEXT:   %epil.iter.cmp.5 = icmp ne i32 6, %xtraiter
; CHECK-NEXT:   br i1 %epil.iter.cmp.5, label %loop.epil.6, label %exit.loopexit.epilog-lcssa
; CHECK: loop.epil.6:                                      ; preds = %if.end.epil.5
; CHECK-NEXT:   %is_even.epil.6 = srem i32 %next.epil.5, 2
; CHECK-NEXT:   %is_even_zero.epil.6 = icmp eq i32 %is_even.epil.6, 0
; CHECK-NEXT:   br i1 %is_even_zero.epil.6, label %if.then.epil.6, label %if.else.epil.6
; CHECK: if.else.epil.6:                                   ; preds = %loop.epil.6
; CHECK-NEXT:   br label %if.end.epil.6
; CHECK: if.then.epil.6:                                   ; preds = %loop.epil.6
; CHECK-NEXT:   br label %if.end.epil.6
; CHECK: if.end.epil.6:                                    ; preds = %if.then.epil.6, %if.else.epil.6
; CHECK-NEXT:   br label %exit.loopexit.epilog-lcssa
; CHECK: exit.loopexit.epilog-lcssa:                       ; preds = %if.end.epil.6, %if.end.epil.5, %if.end.epil.4, %if.end.epil.3, %if.end.epil.2, %if.end.epil.1, %if.end.epil
; CHECK-NEXT:   %even_sum_.ph.ph1 = phi i32 [ %even_sum.unr, %if.end.epil ], [ %even_sum_next_.epil, %if.end.epil.1 ], [ %even_sum_next_.epil.1, %if.end.epil.2 ], [ %even_sum_next_.epil.2, %if.end.epil.3 ], [ %even_sum_next_.epil.3, %if.end.epil.4 ], [ %even_sum_next_.epil.4, %if.end.epil.5 ], [ %even_sum_next_.epil.5, %if.end.epil.6 ]
; CHECK-NEXT:   %odd_sum_.ph.ph2 = phi i32 [ %odd_sum.unr, %if.end.epil ], [ %odd_sum_next_.epil, %if.end.epil.1 ], [ %odd_sum_next_.epil.1, %if.end.epil.2 ], [ %odd_sum_next_.epil.2, %if.end.epil.3 ], [ %odd_sum_next_.epil.3, %if.end.epil.4 ], [ %odd_sum_next_.epil.4, %if.end.epil.5 ], [ %odd_sum_next_.epil.5, %if.end.epil.6 ]
; CHECK-NEXT:   br label %exit.loopexit
; CHECK: exit.loopexit:                                    ; preds = %exit.loopexit.unr-lcssa, %exit.loopexit.epilog-lcssa
; CHECK-NEXT:   %even_sum_.ph = phi i32 [ %even_sum_.ph.ph, %exit.loopexit.unr-lcssa ], [ %even_sum_.ph.ph1, %exit.loopexit.epilog-lcssa ]
; CHECK-NEXT:   %odd_sum_.ph = phi i32 [ %odd_sum_.ph.ph, %exit.loopexit.unr-lcssa ], [ %odd_sum_.ph.ph2, %exit.loopexit.epilog-lcssa ]
; CHECK-NEXT:   br label %exit
; CHECK: exit:                                             ; preds = %exit.loopexit, %entry
; CHECK-NEXT:   %even_sum_ = phi i32 [ 0, %entry ], [ %even_sum_.ph, %exit.loopexit ]
; CHECK-NEXT:   %odd_sum_ = phi i32 [ 0, %entry ], [ %odd_sum_.ph, %exit.loopexit ]
; CHECK-NEXT:   %result = sub i32 %even_sum_, %odd_sum_
; CHECK-NEXT:   ret i32 %result

entry:
  %0 = icmp slt i32 %N, 0
  br i1 %0, label %exit, label %loop

loop:
  %i = phi i32 [ 0, %entry ], [ %next, %if.end ]
  %even_sum = phi i32 [ 0, %entry ], [ %even_sum_next_, %if.end ]
  %odd_sum = phi i32 [ 0, %entry ], [ %odd_sum_next_, %if.end ]
  %is_even = srem i32 %i, 2
  %is_even_zero = icmp eq i32 %is_even, 0
  br i1 %is_even_zero, label %if.then, label %if.else

if.then:
  ; Code to execute if 'i' is even
  %even_sum_next = add i32 %even_sum, %i
  br label %if.end

if.else:
  ; Code to execute if 'i' is odd
  %odd_sum_next = add i32 %odd_sum, %i
  br label %if.end

if.end:
  %even_sum_next_ = phi i32 [ %even_sum, %if.else ], [ %even_sum_next, %if.then ]
  %odd_sum_next_ = phi i32 [ %odd_sum_next, %if.else ], [ %odd_sum, %if.then ]

  %next = add i32 %i, 1
  %exit_cond = icmp slt i32 %next, %N
  br i1 %exit_cond, label %loop, label %exit

exit:
  %even_sum_ = phi i32 [ 0, %entry ], [ %even_sum, %if.end ]
  %odd_sum_ = phi i32 [ 0, %entry ], [ %odd_sum, %if.end ]
  %result = sub i32 %even_sum_, %odd_sum_
  ret i32 %result
}
