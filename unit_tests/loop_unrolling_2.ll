define i32 @sum_of_values() {
; CHECK-LABEL: @sum_of_values()
; CHECK:   entry:
; CHECK-NEXT:     %sum = alloca i32
; CHECK-NEXT:     store i32 0, i32* %sum
; CHECK-NEXT:     %i = alloca i32
; CHECK-NEXT:     store i32 0, i32* %i
; CHECK-NEXT:     br label %loop
; CHECK:   loop:
; CHECK-NEXT:     %cur_i = load i32, i32* %i
; CHECK-NEXT:     %cur_sum = load i32, i32* %sum
; CHECK-NEXT:     %cmp = icmp slt i32 %cur_i, 3
; CHECK-NEXT:     br i1 %cmp, label %loop_body1, label %loop_exit
; CHECK:   loop_body1:
; CHECK-NEXT:     %cur_i_val1 = add i32 1, %cur_i
; CHECK-NEXT:     %cur_sum_val1 = add i32 %cur_sum, %cur_i_val1
; CHECK-NEXT:     store i32 %cur_sum_val1, i32* %sum
; CHECK-NEXT:     %next_i1 = add i32 %cur_i, 1
; CHECK-NEXT:     store i32 %next_i1, i32* %i
; CHECK-NEXT:     br label %loop_body2
; CHECK:   loop_body2:
; CHECK-NEXT:     %cur_i_val2 = add i32 2, %cur_i
; CHECK-NEXT:     %cur_sum_val2 = add i32 %cur_sum_val1, %cur_i_val2
; CHECK-NEXT:     store i32 %cur_sum_val2, i32* %sum
; CHECK-NEXT:     %next_i2 = add i32 %cur_i, 1
; CHECK-NEXT:     store i32 %next_i2, i32* %i
; CHECK-NEXT:     br label %loop_body3
; CHECK:   loop_body3:
; CHECK-NEXT:     %cur_i_val3 = add i32 3, %cur_i
; CHECK-NEXT:     %cur_sum_val3 = add i32 %cur_sum_val2, %cur_i_val3
; CHECK-NEXT:     store i32 %cur_sum_val3, i32* %sum
; CHECK-NEXT:     %next_i3 = add i32 %cur_i, 1
; CHECK-NEXT:     store i32 %next_i3, i32* %i
; CHECK-NEXT:     br label %loop
; CHECK:   loop_exit:
; CHECK-NEXT:     %final_sum = load i32, i32* %sum
; CHECK-NEXT:     ret i32 %final_sum

  entry:
    %sum = alloca i32
    store i32 0, i32* %sum

    %i = alloca i32
    store i32 0, i32* %i

    br label %loop

  loop:
    %cur_i = load i32, i32* %i
    %cur_sum = load i32, i32* %sum

    %cmp = icmp slt i32 %cur_i, 3
    br i1 %cmp, label %loop_body, label %loop_exit

  loop_body:
    %cur_i_val = add i32 1, %cur_i
    %cur_sum_val = add i32 %cur_sum, %cur_i_val

    store i32 %cur_sum_val, i32* %sum

    %next_i = add i32 %cur_i, 1
    store i32 %next_i, i32* %i

    br label %loop

  loop_exit:
    %final_sum = load i32, i32* %sum
    ret i32 %final_sum
}
