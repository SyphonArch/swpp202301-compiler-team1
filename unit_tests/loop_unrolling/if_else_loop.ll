; Test on a loop with an if-else structure
; Runtime unrolling will happen on this loop

define i32 @exampleLoop(i32 %N) {
; CHECK-LABEL: @exampleLoop(i32 %N)
; CHECK: entry:
; CHECK-NEXT:   %0 = icmp slt i32 %N, 0
; CHECK-NEXT:   br i1 %0, label %exit, label %loop.preheader
; CHECK: loop.preheader:                                   ; preds = %entry
; CHECK-NEXT:   br label %loop
; CHECK: loop:                                             ; preds = %if.end.7, %loop.preheader
; CHECK-NEXT:   %i = phi i32 [ 0, %loop.preheader ], [ %next.7, %if.end.7 ]
; CHECK-NEXT:   %even_sum = phi i32 [ 0, %loop.preheader ], [ %even_sum_next_.7, %if.end.7 ]
; CHECK-NEXT:   %odd_sum = phi i32 [ 0, %loop.preheader ], [ %odd_sum_next_.7, %if.end.7 ]
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
; CHECK-NEXT:   %exit_cond = icmp slt i32 %next, %N
; CHECK-NEXT:   br i1 %exit_cond, label %loop.1, label %exit.loopexit
; CHECK: loop.1:                                           ; preds = %if.end
; CHECK-NEXT:   %is_even.1 = srem i32 %next, 2
; CHECK-NEXT:   %is_even_zero.1 = icmp eq i32 %is_even.1, 0
; CHECK-NEXT:   br i1 %is_even_zero.1, label %if.then.1, label %if.else.1
; CHECK: if.else.1:                                        ; preds = %loop.1
; CHECK-NEXT:   %odd_sum_next.1 = add i32 %odd_sum_next_, %next
; CHECK-NEXT:   br label %if.end.1
; CHECK: if.then.1:                                        ; preds = %loop.1
; CHECK-NEXT:   %even_sum_next.1 = add i32 %even_sum_next_, %next
; CHECK-NEXT:   br label %if.end.1
; CHECK: if.end.1:                                         ; preds = %if.then.1, %if.else.1
; CHECK-NEXT:   %even_sum_next_.1 = phi i32 [ %even_sum_next_, %if.else.1 ], [ %even_sum_next.1, %if.then.1 ]
; CHECK-NEXT:   %odd_sum_next_.1 = phi i32 [ %odd_sum_next.1, %if.else.1 ], [ %odd_sum_next_, %if.then.1 ]
; CHECK-NEXT:   %next.1 = add nuw nsw i32 %next, 1
; CHECK-NEXT:   %exit_cond.1 = icmp slt i32 %next.1, %N
; CHECK-NEXT:   br i1 %exit_cond.1, label %loop.2, label %exit.loopexit
; CHECK: loop.2:                                           ; preds = %if.end.1
; CHECK-NEXT:   %is_even.2 = srem i32 %next.1, 2
; CHECK-NEXT:   %is_even_zero.2 = icmp eq i32 %is_even.2, 0
; CHECK-NEXT:   br i1 %is_even_zero.2, label %if.then.2, label %if.else.2
; CHECK: if.else.2:                                        ; preds = %loop.2
; CHECK-NEXT:   %odd_sum_next.2 = add i32 %odd_sum_next_.1, %next.1
; CHECK-NEXT:   br label %if.end.2
; CHECK: if.then.2:                                        ; preds = %loop.2
; CHECK-NEXT:   %even_sum_next.2 = add i32 %even_sum_next_.1, %next.1
; CHECK-NEXT:   br label %if.end.2
; CHECK: if.end.2:                                         ; preds = %if.then.2, %if.else.2
; CHECK-NEXT:   %even_sum_next_.2 = phi i32 [ %even_sum_next_.1, %if.else.2 ], [ %even_sum_next.2, %if.then.2 ]
; CHECK-NEXT:   %odd_sum_next_.2 = phi i32 [ %odd_sum_next.2, %if.else.2 ], [ %odd_sum_next_.1, %if.then.2 ]
; CHECK-NEXT:   %next.2 = add nuw nsw i32 %next.1, 1
; CHECK-NEXT:   %exit_cond.2 = icmp slt i32 %next.2, %N
; CHECK-NEXT:   br i1 %exit_cond.2, label %loop.3, label %exit.loopexit
; CHECK: loop.3:                                           ; preds = %if.end.2
; CHECK-NEXT:   %is_even.3 = srem i32 %next.2, 2
; CHECK-NEXT:   %is_even_zero.3 = icmp eq i32 %is_even.3, 0
; CHECK-NEXT:   br i1 %is_even_zero.3, label %if.then.3, label %if.else.3
; CHECK: if.else.3:                                        ; preds = %loop.3
; CHECK-NEXT:   %odd_sum_next.3 = add i32 %odd_sum_next_.2, %next.2
; CHECK-NEXT:   br label %if.end.3
; CHECK: if.then.3:                                        ; preds = %loop.3
; CHECK-NEXT:   %even_sum_next.3 = add i32 %even_sum_next_.2, %next.2
; CHECK-NEXT:   br label %if.end.3
; CHECK: if.end.3:                                         ; preds = %if.then.3, %if.else.3
; CHECK-NEXT:   %even_sum_next_.3 = phi i32 [ %even_sum_next_.2, %if.else.3 ], [ %even_sum_next.3, %if.then.3 ]
; CHECK-NEXT:   %odd_sum_next_.3 = phi i32 [ %odd_sum_next.3, %if.else.3 ], [ %odd_sum_next_.2, %if.then.3 ]
; CHECK-NEXT:   %next.3 = add nuw nsw i32 %next.2, 1
; CHECK-NEXT:   %exit_cond.3 = icmp slt i32 %next.3, %N
; CHECK-NEXT:   br i1 %exit_cond.3, label %loop.4, label %exit.loopexit
; CHECK: loop.4:                                           ; preds = %if.end.3
; CHECK-NEXT:   %is_even.4 = srem i32 %next.3, 2
; CHECK-NEXT:   %is_even_zero.4 = icmp eq i32 %is_even.4, 0
; CHECK-NEXT:   br i1 %is_even_zero.4, label %if.then.4, label %if.else.4
; CHECK: if.else.4:                                        ; preds = %loop.4
; CHECK-NEXT:   %odd_sum_next.4 = add i32 %odd_sum_next_.3, %next.3
; CHECK-NEXT:   br label %if.end.4
; CHECK: if.then.4:                                        ; preds = %loop.4
; CHECK-NEXT:   %even_sum_next.4 = add i32 %even_sum_next_.3, %next.3
; CHECK-NEXT:   br label %if.end.4
; CHECK: if.end.4:                                         ; preds = %if.then.4, %if.else.4
; CHECK-NEXT:   %even_sum_next_.4 = phi i32 [ %even_sum_next_.3, %if.else.4 ], [ %even_sum_next.4, %if.then.4 ]
; CHECK-NEXT:   %odd_sum_next_.4 = phi i32 [ %odd_sum_next.4, %if.else.4 ], [ %odd_sum_next_.3, %if.then.4 ]
; CHECK-NEXT:   %next.4 = add nuw nsw i32 %next.3, 1
; CHECK-NEXT:   %exit_cond.4 = icmp slt i32 %next.4, %N
; CHECK-NEXT:   br i1 %exit_cond.4, label %loop.5, label %exit.loopexit
; CHECK: loop.5:                                           ; preds = %if.end.4
; CHECK-NEXT:   %is_even.5 = srem i32 %next.4, 2
; CHECK-NEXT:   %is_even_zero.5 = icmp eq i32 %is_even.5, 0
; CHECK-NEXT:   br i1 %is_even_zero.5, label %if.then.5, label %if.else.5
; CHECK: if.else.5:                                        ; preds = %loop.5
; CHECK-NEXT:   %odd_sum_next.5 = add i32 %odd_sum_next_.4, %next.4
; CHECK-NEXT:   br label %if.end.5
; CHECK: if.then.5:                                        ; preds = %loop.5
; CHECK-NEXT:   %even_sum_next.5 = add i32 %even_sum_next_.4, %next.4
; CHECK-NEXT:   br label %if.end.5
; CHECK: if.end.5:                                         ; preds = %if.then.5, %if.else.5
; CHECK-NEXT:   %even_sum_next_.5 = phi i32 [ %even_sum_next_.4, %if.else.5 ], [ %even_sum_next.5, %if.then.5 ]
; CHECK-NEXT:   %odd_sum_next_.5 = phi i32 [ %odd_sum_next.5, %if.else.5 ], [ %odd_sum_next_.4, %if.then.5 ]
; CHECK-NEXT:   %next.5 = add nuw nsw i32 %next.4, 1
; CHECK-NEXT:   %exit_cond.5 = icmp slt i32 %next.5, %N
; CHECK-NEXT:   br i1 %exit_cond.5, label %loop.6, label %exit.loopexit
; CHECK: loop.6:                                           ; preds = %if.end.5
; CHECK-NEXT:   %is_even.6 = srem i32 %next.5, 2
; CHECK-NEXT:   %is_even_zero.6 = icmp eq i32 %is_even.6, 0
; CHECK-NEXT:   br i1 %is_even_zero.6, label %if.then.6, label %if.else.6
; CHECK: if.else.6:                                        ; preds = %loop.6
; CHECK-NEXT:   %odd_sum_next.6 = add i32 %odd_sum_next_.5, %next.5
; CHECK-NEXT:   br label %if.end.6
; CHECK: if.then.6:                                        ; preds = %loop.6
; CHECK-NEXT:   %even_sum_next.6 = add i32 %even_sum_next_.5, %next.5
; CHECK-NEXT:   br label %if.end.6
; CHECK: if.end.6:                                         ; preds = %if.then.6, %if.else.6
; CHECK-NEXT:   %even_sum_next_.6 = phi i32 [ %even_sum_next_.5, %if.else.6 ], [ %even_sum_next.6, %if.then.6 ]
; CHECK-NEXT:   %odd_sum_next_.6 = phi i32 [ %odd_sum_next.6, %if.else.6 ], [ %odd_sum_next_.5, %if.then.6 ]
; CHECK-NEXT:   %next.6 = add nuw nsw i32 %next.5, 1
; CHECK-NEXT:   %exit_cond.6 = icmp slt i32 %next.6, %N
; CHECK-NEXT:   br i1 %exit_cond.6, label %loop.7, label %exit.loopexit
; CHECK: loop.7:                                           ; preds = %if.end.6
; CHECK-NEXT:   %is_even.7 = srem i32 %next.6, 2
; CHECK-NEXT:   %is_even_zero.7 = icmp eq i32 %is_even.7, 0
; CHECK-NEXT:   br i1 %is_even_zero.7, label %if.then.7, label %if.else.7
; CHECK: if.else.7:                                        ; preds = %loop.7
; CHECK-NEXT:   %odd_sum_next.7 = add i32 %odd_sum_next_.6, %next.6
; CHECK-NEXT:   br label %if.end.7
; CHECK: if.then.7:                                        ; preds = %loop.7
; CHECK-NEXT:   %even_sum_next.7 = add i32 %even_sum_next_.6, %next.6
; CHECK-NEXT:   br label %if.end.7
; CHECK: if.end.7:                                         ; preds = %if.then.7, %if.else.7
; CHECK-NEXT:   %even_sum_next_.7 = phi i32 [ %even_sum_next_.6, %if.else.7 ], [ %even_sum_next.7, %if.then.7 ]
; CHECK-NEXT:   %odd_sum_next_.7 = phi i32 [ %odd_sum_next.7, %if.else.7 ], [ %odd_sum_next_.6, %if.then.7 ]
; CHECK-NEXT:   %next.7 = add i32 %next.6, 1
; CHECK-NEXT:   %exit_cond.7 = icmp slt i32 %next.7, %N
; CHECK-NEXT:   br i1 %exit_cond.7, label %loop, label %exit.loopexit
; CHECK: exit.loopexit:                                    ; preds = %if.end.7, %if.end.6, %if.end.5, %if.end.4, %if.end.3, %if.end.2, %if.end.1, %if.end
; CHECK-NEXT:   %even_sum_.ph = phi i32 [ %even_sum, %if.end ], [ %even_sum_next_, %if.end.1 ], [ %even_sum_next_.1, %if.end.2 ], [ %even_sum_next_.2, %if.end.3 ], [ %even_sum_next_.3, %if.end.4 ], [ %even_sum_next_.4, %if.end.5 ], [ %even_sum_next_.5, %if.end.6 ], [ %even_sum_next_.6, %if.end.7 ]
; CHECK-NEXT:   %odd_sum_.ph = phi i32 [ %odd_sum, %if.end ], [ %odd_sum_next_, %if.end.1 ], [ %odd_sum_next_.1, %if.end.2 ], [ %odd_sum_next_.2, %if.end.3 ], [ %odd_sum_next_.3, %if.end.4 ], [ %odd_sum_next_.4, %if.end.5 ], [ %odd_sum_next_.5, %if.end.6 ], [ %odd_sum_next_.6, %if.end.7 ]
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
