; Test on a loop with an if-else structure
; Unrolling does not happen for this loop

define i32 @exampleLoop(i32 %N) {
; CHECK-LABEL: @exampleLoop(i32 %N)
; CHECK: entry:
; CHECK-NEXT:   %0 = icmp slt i32 %N, 0
; CHECK-NEXT:   br i1 %0, label %exit, label %loop
; CHECK: loop:
; CHECK-NEXT:   %i = phi i32 [ 0, %entry ], [ %next, %if.end ]
; CHECK-NEXT:   %even_sum = phi i32 [ 0, %entry ], [ %even_sum_next_, %if.end ]
; CHECK-NEXT:   %odd_sum = phi i32 [ 0, %entry ], [ %odd_sum_next_, %if.end ]
; CHECK-NEXT:   %is_even = srem i32 %i, 2
; CHECK-NEXT:   %is_even_zero = icmp eq i32 %is_even, 0
; CHECK-NEXT:   br i1 %is_even_zero, label %if.then, label %if.else
; CHECK: if.then:
; CHECK-NEXT:   %even_sum_next = add i32 %even_sum, %i
; CHECK-NEXT:   br label %if.end
; CHECK: if.else:
; CHECK-NEXT:   %odd_sum_next = add i32 %odd_sum, %i
; CHECK-NEXT:   br label %if.end
; CHECK: if.end:
; CHECK-NEXT:   %even_sum_next_ = phi i32 [ %even_sum, %if.else ], [ %even_sum_next, %if.then ]
; CHECK-NEXT:   %odd_sum_next_ = phi i32 [ %odd_sum_next, %if.else ], [ %odd_sum, %if.then ]
; CHECK-NEXT:   %next = add i32 %i, 1
; CHECK-NEXT:   %exit_cond = icmp slt i32 %next, %N
; CHECK-NEXT:   br i1 %exit_cond, label %loop, label %exit
; CHECK: exit:
; CHECK-NEXT:   %even_sum_ = phi i32 [ 0, %entry ], [ %even_sum, %if.end ]
; CHECK-NEXT:   %odd_sum_ = phi i32 [ 0, %entry ], [ %odd_sum, %if.end ]
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
