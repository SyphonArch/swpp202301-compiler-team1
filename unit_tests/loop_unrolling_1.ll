define i32 @sum_of_squares(i32 %n) {
; CHECK-LABEL: @sum_of_squares(i32 %n)
; CHECK: entry:
; CHECK-NEXT:   %sum = alloca i32, align 4
; CHECK-NEXT:   store i32 0, i32* %sum, align 4
; CHECK-NEXT:   %i = alloca i32, align 4
; CHECK-NEXT:   store i32 0, i32* %i, align 4
; CHECK-NEXT:   br label %loop
; CHECK: loop:
; CHECK-NEXT:   %cur_i = load i32, i32* %i, align 4
; CHECK-NEXT:   %cmp = icmp slt i32 %cur_i, %n
; CHECK-NEXT:   br i1 %cmp, label %loop_body, label %loop_exit
; CHECK: loop_body:
; CHECK-NEXT:   %cur_i_2 = load i32, i32* %i, align 4
; CHECK-NEXT:   %cur_sum_2 = load i32, i32* %sum, align 4
; CHECK-NEXT:   %cur_i_sq = mul i32 %cur_i_2, %cur_i_2
; CHECK-NEXT:   %new_sum = add i32 %cur_sum_2, %cur_i_sq
; CHECK-NEXT:   store i32 %new_sum, i32* %sum, align 4
; CHECK-NEXT:   %next_i = add i32 %cur_i_2, 1
; CHECK-NEXT:   store i32 %next_i, i32* %i, align 4
; CHECK-NEXT:   %cur_i.1 = load i32, i32* %i, align 4
; CHECK-NEXT:   %cmp.1 = icmp slt i32 %cur_i.1, %n
; CHECK-NEXT:   br i1 %cmp.1, label %loop_body.1, label %loop_exit
; CHECK: loop_body.1:
; CHECK-NEXT:   %cur_i_2.1 = load i32, i32* %i, align 4
; CHECK-NEXT:   %cur_sum_2.1 = load i32, i32* %sum, align 4
; CHECK-NEXT:   %cur_i_sq.1 = mul i32 %cur_i_2.1, %cur_i_2.1
; CHECK-NEXT:   %new_sum.1 = add i32 %cur_sum_2.1, %cur_i_sq.1
; CHECK-NEXT:   store i32 %new_sum.1, i32* %sum, align 4
; CHECK-NEXT:   %next_i.1 = add i32 %cur_i_2.1, 1
; CHECK-NEXT:   store i32 %next_i.1, i32* %i, align 4
; CHECK-NEXT:   %cur_i.2 = load i32, i32* %i, align 4
; CHECK-NEXT:   %cmp.2 = icmp slt i32 %cur_i.2, %n
; CHECK-NEXT:   br i1 %cmp.2, label %loop_body.2, label %loop_exit
; CHECK: loop_body.2:
; CHECK-NEXT:   %cur_i_2.2 = load i32, i32* %i, align 4
; CHECK-NEXT:   %cur_sum_2.2 = load i32, i32* %sum, align 4
; CHECK-NEXT:   %cur_i_sq.2 = mul i32 %cur_i_2.2, %cur_i_2.2
; CHECK-NEXT:   %new_sum.2 = add i32 %cur_sum_2.2, %cur_i_sq.2
; CHECK-NEXT:   store i32 %new_sum.2, i32* %sum, align 4
; CHECK-NEXT:   %next_i.2 = add i32 %cur_i_2.2, 1
; CHECK-NEXT:   store i32 %next_i.2, i32* %i, align 4
; CHECK-NEXT:   %cur_i.3 = load i32, i32* %i, align 4
; CHECK-NEXT:   %cmp.3 = icmp slt i32 %cur_i.3, %n
; CHECK-NEXT:   br i1 %cmp.3, label %loop_body.3, label %loop_exit
; CHECK: loop_body.3:
; CHECK-NEXT:   %cur_i_2.3 = load i32, i32* %i, align 4
; CHECK-NEXT:   %cur_sum_2.3 = load i32, i32* %sum, align 4
; CHECK-NEXT:   %cur_i_sq.3 = mul i32 %cur_i_2.3, %cur_i_2.3
; CHECK-NEXT:   %new_sum.3 = add i32 %cur_sum_2.3, %cur_i_sq.3
; CHECK-NEXT:   store i32 %new_sum.3, i32* %sum, align 4
; CHECK-NEXT:   %next_i.3 = add i32 %cur_i_2.3, 1
; CHECK-NEXT:   store i32 %next_i.3, i32* %i, align 4
; CHECK-NEXT:   %cur_i.4 = load i32, i32* %i, align 4
; CHECK-NEXT:   %cmp.4 = icmp slt i32 %cur_i.4, %n
; CHECK-NEXT:   br i1 %cmp.4, label %loop_body.4, label %loop_exit
; CHECK: loop_body.4:
; CHECK-NEXT:   %cur_i_2.4 = load i32, i32* %i, align 4
; CHECK-NEXT:   %cur_sum_2.4 = load i32, i32* %sum, align 4
; CHECK-NEXT:   %cur_i_sq.4 = mul i32 %cur_i_2.4, %cur_i_2.4
; CHECK-NEXT:   %new_sum.4 = add i32 %cur_sum_2.4, %cur_i_sq.4
; CHECK-NEXT:   store i32 %new_sum.4, i32* %sum, align 4
; CHECK-NEXT:   %next_i.4 = add i32 %cur_i_2.4, 1
; CHECK-NEXT:   store i32 %next_i.4, i32* %i, align 4
; CHECK-NEXT:   %cur_i.5 = load i32, i32* %i, align 4
; CHECK-NEXT:   %cmp.5 = icmp slt i32 %cur_i.5, %n
; CHECK-NEXT:   br i1 %cmp.5, label %loop_body.5, label %loop_exit
; CHECK: loop_body.5:
; CHECK-NEXT:   %cur_i_2.5 = load i32, i32* %i, align 4
; CHECK-NEXT:   %cur_sum_2.5 = load i32, i32* %sum, align 4
; CHECK-NEXT:   %cur_i_sq.5 = mul i32 %cur_i_2.5, %cur_i_2.5
; CHECK-NEXT:   %new_sum.5 = add i32 %cur_sum_2.5, %cur_i_sq.5
; CHECK-NEXT:   store i32 %new_sum.5, i32* %sum, align 4
; CHECK-NEXT:   %next_i.5 = add i32 %cur_i_2.5, 1
; CHECK-NEXT:   store i32 %next_i.5, i32* %i, align 4
; CHECK-NEXT:   %cur_i.6 = load i32, i32* %i, align 4
; CHECK-NEXT:   %cmp.6 = icmp slt i32 %cur_i.6, %n
; CHECK-NEXT:   br i1 %cmp.6, label %loop_body.6, label %loop_exit
; CHECK: loop_body.6:
; CHECK-NEXT:   %cur_i_2.6 = load i32, i32* %i, align 4
; CHECK-NEXT:   %cur_sum_2.6 = load i32, i32* %sum, align 4
; CHECK-NEXT:   %cur_i_sq.6 = mul i32 %cur_i_2.6, %cur_i_2.6
; CHECK-NEXT:   %new_sum.6 = add i32 %cur_sum_2.6, %cur_i_sq.6
; CHECK-NEXT:   store i32 %new_sum.6, i32* %sum, align 4
; CHECK-NEXT:   %next_i.6 = add i32 %cur_i_2.6, 1
; CHECK-NEXT:   store i32 %next_i.6, i32* %i, align 4
; CHECK-NEXT:   %cur_i.7 = load i32, i32* %i, align 4
; CHECK-NEXT:   %cmp.7 = icmp slt i32 %cur_i.7, %n
; CHECK-NEXT:   br i1 %cmp.7, label %loop_body.7, label %loop_exit
; CHECK: loop_body.7:
; CHECK-NEXT:   %cur_i_2.7 = load i32, i32* %i, align 4
; CHECK-NEXT:   %cur_sum_2.7 = load i32, i32* %sum, align 4
; CHECK-NEXT:   %cur_i_sq.7 = mul i32 %cur_i_2.7, %cur_i_2.7
; CHECK-NEXT:   %new_sum.7 = add i32 %cur_sum_2.7, %cur_i_sq.7
; CHECK-NEXT:   store i32 %new_sum.7, i32* %sum, align 4
; CHECK-NEXT:   %next_i.7 = add i32 %cur_i_2.7, 1
; CHECK-NEXT:   store i32 %next_i.7, i32* %i, align 4
; CHECK-NEXT:   br label %loop
; CHECK: loop_exit:
; CHECK-NEXT:   %final_sum = load i32, i32* %sum, align 4
; CHECK-NEXT:   ret i32 %final_sum
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
