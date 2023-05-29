define i32 @add_and_return(i32* %ptr, i32 %n) {
; CHECK-LABEL @add_and_return(i32* %ptr, i32 %n)
; CHECK: entry:
entry:
  %loaded_value = load i32, i32* %ptr
  br label %loop

loop:
  %index = phi i32 [ 1, %entry ], [ %next_index, %loop ]
  %sum = phi i32 [ 0, %entry ], [ %new_sum, %loop ]
  %new_sum = add i32 %sum, %index
  %next_index = add i32 %index, 1
  %cmp = icmp slt i32 %next_index, %n
  br i1 %cmp, label %loop, label %exit

exit:
  %result = add i32 %loaded_value, %sum
  ret i32 %result
}
