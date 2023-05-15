define i32 @sum_of_squares(i32 %n) {
; CHECK-LABEL: @sum_of_squares(i32 %n)
; CHECK:   entry:
; CHECK-NEXT:     %sum = alloca i32
; CHECK-NEXT:     store i32 0, i32* %sum
; CHECK-NEXT:     %i = alloca i32
; CHECK-NEXT:     store i32 0, i32* %i
; CHECK-NEXT:     br label %loop
; CHECK:   loop:
; CHECK-NEXT:     %cur_i = load i32, i32* %i
; CHECK-NEXT:     %cur_sum = load i32, i32* %sum
; CHECK-NEXT:     %cmp = icmp slt i32 %cur_i, %n
; CHECK-NEXT:     br i1 %cmp, label %loop_body1, label %loop_exit
; CHECK:   loop_body1:
; CHECK-NEXT:     %cur_i_1 = load i32, i32* %i
; CHECK-NEXT:     %cur_sum_1 = load i32, i32* %sum
; CHECK-NEXT:     %cur_i_sq1 = mul i32 %cur_i_1, %cur_i_1
; CHECK-NEXT:     %new_sum1 = add i32 %cur_sum_1, %cur_i_sq1
; CHECK-NEXT:     store i32 %new_sum1, i32* %sum
; CHECK-NEXT:     %next_i1 = add i32 %cur_i_1, 1
; CHECK-NEXT:     store i32 %next_i1, i32* %i
; CHECK-NEXT:     br label %loop_body2
; CHECK:   loop_body2:
; CHECK-NEXT:     %cur_i_2 = load i32, i32* %i
; CHECK-NEXT:     %cur_sum_2 = load i32, i32* %sum
; CHECK-NEXT:     %cur_i_sq2 = mul i32 %cur_i_2, %cur_i_2
; CHECK-NEXT:     %new_sum2 = add i32 %cur_sum_2, %cur_i_sq2
; CHECK-NEXT:     store i32 %new_sum2, i32* %sum
; CHECK-NEXT:     %next_i2 = add i32 %cur_i_2, 1
; CHECK-NEXT:     store i32 %next_i2, i32* %i
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

    %cmp = icmp slt i32 %cur_i, %n
    br i1 %cmp, label %loop_body, label %loop_exit

  loop_body:
    %cur_i_2 = load i32, i32* %i
    %cur_sum_2 = load i32, i32* %sum

    %cur_i_sq = mul i32 %cur_i_2, %cur_i_2
    %new_sum = add i32 %cur_sum_2, %cur_i_sq

    store i32 %new_sum, i32* %sum

    %next_i = add i32 %cur_i_2, 1
    store i32 %next_i, i32* %i

    br label %loop

  loop_exit:
    %final_sum = load i32, i32* %sum
    ret i32 %final_sum
}
