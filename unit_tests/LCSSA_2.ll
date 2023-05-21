define i32 @sum_values(i32 %start, i32 %end) {
  entry:
    %sum = alloca i32
    store i32 0, i32* %sum
    br label %loop

  loop:
    %i = phi i32 [ %start, %entry ], [ %next_i, %loop ]
    %current_value = add i32 %i, 1
    %new_sum = load i32, i32* %sum
    %updated_sum = add i32 %new_sum, %current_value
    store i32 %updated_sum, i32* %sum
    %next_i = add i32 %i, 1
    %compare = icmp slt i32 %next_i, %end
    br i1 %compare, label %loop, label %exit

  exit:
    %final_sum = load i32, i32* %sum
    ret i32 %final_sum
}

; CHECK: define i32 @sum_values(i32 %start, i32 %end)
; CHECK: entry:
; CHECK-NEXT:   %sum = alloca i32
; CHECK-NEXT:   store i32 0, i32* %sum
; CHECK-NEXT:   br label %loop
; CHECK: loop:
; CHECK-NEXT:   %i = phi i32 [ %start, %entry ], [ %next_i, %loop ]
; CHECK-NEXT:   %current_value = add i32 %i, 1
; CHECK-NEXT:   %new_sum = load i32, i32* %sum
; CHECK-NEXT:   %updated_sum = add i32 %new_sum, %current_value
; CHECK-NEXT:   store i32 %updated_sum, i32* %sum
; CHECK-NEXT:   %next_i = add i32 %i, 1
; CHECK-NEXT:   %compare = icmp slt i32 %next_i, %end
; CHECK-NEXT:   br i1 %compare, label %loop, label %exit
; CHECK: exit:
; CHECK-NEXT:   %final_sum = load i32, i32* %sum
; CHECK-NEXT:   ret i32 %final_sum

; case 2: no useage of innner value. no change