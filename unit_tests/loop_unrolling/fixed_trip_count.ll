; Test loop unrolling on a loop with a fixed trip count

define i32 @sum_of_values() {
; CHECK-LABEL: @sum_of_values()
; CHECK: entry:
; CHECK-NEXT:   %sum = alloca i32, align 4
; CHECK-NEXT:   store i32 0, i32* %sum, align 4
; CHECK-NEXT:   %i = alloca i32, align 4
; CHECK-NEXT:   store i32 0, i32* %i, align 4
; CHECK-NEXT:   %cur_i1 = load i32, i32* %i, align 4
; CHECK-NEXT:   %cur_sum2 = load i32, i32* %sum, align 4
; CHECK-NEXT:   %cmp3 = icmp slt i32 %cur_i1, 3
; CHECK-NEXT:   br i1 %cmp3, label %loop_body.lr.ph, label %loop_exit
; CHECK: loop_body.lr.ph:                                  ; preds = %entry
; CHECK-NEXT:   br label %loop_body
; CHECK: loop_body:                                        ; preds = %loop_body.7, %loop_body.lr.ph
; CHECK-NEXT:   %cur_sum5 = phi i32 [ %cur_sum2, %loop_body.lr.ph ], [ %cur_sum.7, %loop_body.7 ]
; CHECK-NEXT:   %cur_i4 = phi i32 [ %cur_i1, %loop_body.lr.ph ], [ %cur_i.7, %loop_body.7 ]
; CHECK-NEXT:   %cur_i_val = add i32 1, %cur_i4
; CHECK-NEXT:   %cur_sum_val = add i32 %cur_sum5, %cur_i_val
; CHECK-NEXT:   store i32 %cur_sum_val, i32* %sum, align 4
; CHECK-NEXT:   %next_i = add i32 %cur_i4, 1
; CHECK-NEXT:   store i32 %next_i, i32* %i, align 4
; CHECK-NEXT:   %cur_i = load i32, i32* %i, align 4
; CHECK-NEXT:   %cur_sum = load i32, i32* %sum, align 4
; CHECK-NEXT:   %cmp = icmp slt i32 %cur_i, 3
; CHECK-NEXT:   br i1 %cmp, label %loop_body.1, label %loop.loop_exit_crit_edge
; CHECK: loop_body.1:                                      ; preds = %loop_body
; CHECK-NEXT:   %cur_i_val.1 = add i32 1, %cur_i
; CHECK-NEXT:   %cur_sum_val.1 = add i32 %cur_sum, %cur_i_val.1
; CHECK-NEXT:   store i32 %cur_sum_val.1, i32* %sum, align 4
; CHECK-NEXT:   %next_i.1 = add i32 %cur_i, 1
; CHECK-NEXT:   store i32 %next_i.1, i32* %i, align 4
; CHECK-NEXT:   %cur_i.1 = load i32, i32* %i, align 4
; CHECK-NEXT:   %cur_sum.1 = load i32, i32* %sum, align 4
; CHECK-NEXT:   %cmp.1 = icmp slt i32 %cur_i.1, 3
; CHECK-NEXT:   br i1 %cmp.1, label %loop_body.2, label %loop.loop_exit_crit_edge
; CHECK: loop_body.2:                                      ; preds = %loop_body.1
; CHECK-NEXT:   %cur_i_val.2 = add i32 1, %cur_i.1
; CHECK-NEXT:   %cur_sum_val.2 = add i32 %cur_sum.1, %cur_i_val.2
; CHECK-NEXT:   store i32 %cur_sum_val.2, i32* %sum, align 4
; CHECK-NEXT:   %next_i.2 = add i32 %cur_i.1, 1
; CHECK-NEXT:   store i32 %next_i.2, i32* %i, align 4
; CHECK-NEXT:   %cur_i.2 = load i32, i32* %i, align 4
; CHECK-NEXT:   %cur_sum.2 = load i32, i32* %sum, align 4
; CHECK-NEXT:   %cmp.2 = icmp slt i32 %cur_i.2, 3
; CHECK-NEXT:   br i1 %cmp.2, label %loop_body.3, label %loop.loop_exit_crit_edge
; CHECK: loop_body.3:                                      ; preds = %loop_body.2
; CHECK-NEXT:   %cur_i_val.3 = add i32 1, %cur_i.2
; CHECK-NEXT:   %cur_sum_val.3 = add i32 %cur_sum.2, %cur_i_val.3
; CHECK-NEXT:   store i32 %cur_sum_val.3, i32* %sum, align 4
; CHECK-NEXT:   %next_i.3 = add i32 %cur_i.2, 1
; CHECK-NEXT:   store i32 %next_i.3, i32* %i, align 4
; CHECK-NEXT:   %cur_i.3 = load i32, i32* %i, align 4
; CHECK-NEXT:   %cur_sum.3 = load i32, i32* %sum, align 4
; CHECK-NEXT:   %cmp.3 = icmp slt i32 %cur_i.3, 3
; CHECK-NEXT:   br i1 %cmp.3, label %loop_body.4, label %loop.loop_exit_crit_edge
; CHECK: loop_body.4:                                      ; preds = %loop_body.3
; CHECK-NEXT:   %cur_i_val.4 = add i32 1, %cur_i.3
; CHECK-NEXT:   %cur_sum_val.4 = add i32 %cur_sum.3, %cur_i_val.4
; CHECK-NEXT:   store i32 %cur_sum_val.4, i32* %sum, align 4
; CHECK-NEXT:   %next_i.4 = add i32 %cur_i.3, 1
; CHECK-NEXT:   store i32 %next_i.4, i32* %i, align 4
; CHECK-NEXT:   %cur_i.4 = load i32, i32* %i, align 4
; CHECK-NEXT:   %cur_sum.4 = load i32, i32* %sum, align 4
; CHECK-NEXT:   %cmp.4 = icmp slt i32 %cur_i.4, 3
; CHECK-NEXT:   br i1 %cmp.4, label %loop_body.5, label %loop.loop_exit_crit_edge
; CHECK: loop_body.5:                                      ; preds = %loop_body.4
; CHECK-NEXT:   %cur_i_val.5 = add i32 1, %cur_i.4
; CHECK-NEXT:   %cur_sum_val.5 = add i32 %cur_sum.4, %cur_i_val.5
; CHECK-NEXT:   store i32 %cur_sum_val.5, i32* %sum, align 4
; CHECK-NEXT:   %next_i.5 = add i32 %cur_i.4, 1
; CHECK-NEXT:   store i32 %next_i.5, i32* %i, align 4
; CHECK-NEXT:   %cur_i.5 = load i32, i32* %i, align 4
; CHECK-NEXT:   %cur_sum.5 = load i32, i32* %sum, align 4
; CHECK-NEXT:   %cmp.5 = icmp slt i32 %cur_i.5, 3
; CHECK-NEXT:   br i1 %cmp.5, label %loop_body.6, label %loop.loop_exit_crit_edge
; CHECK: loop_body.6:                                      ; preds = %loop_body.5
; CHECK-NEXT:   %cur_i_val.6 = add i32 1, %cur_i.5
; CHECK-NEXT:   %cur_sum_val.6 = add i32 %cur_sum.5, %cur_i_val.6
; CHECK-NEXT:   store i32 %cur_sum_val.6, i32* %sum, align 4
; CHECK-NEXT:   %next_i.6 = add i32 %cur_i.5, 1
; CHECK-NEXT:   store i32 %next_i.6, i32* %i, align 4
; CHECK-NEXT:   %cur_i.6 = load i32, i32* %i, align 4
; CHECK-NEXT:   %cur_sum.6 = load i32, i32* %sum, align 4
; CHECK-NEXT:   %cmp.6 = icmp slt i32 %cur_i.6, 3
; CHECK-NEXT:   br i1 %cmp.6, label %loop_body.7, label %loop.loop_exit_crit_edge
; CHECK: loop_body.7:                                      ; preds = %loop_body.6
; CHECK-NEXT:   %cur_i_val.7 = add i32 1, %cur_i.6
; CHECK-NEXT:   %cur_sum_val.7 = add i32 %cur_sum.6, %cur_i_val.7
; CHECK-NEXT:   store i32 %cur_sum_val.7, i32* %sum, align 4
; CHECK-NEXT:   %next_i.7 = add i32 %cur_i.6, 1
; CHECK-NEXT:   store i32 %next_i.7, i32* %i, align 4
; CHECK-NEXT:   %cur_i.7 = load i32, i32* %i, align 4
; CHECK-NEXT:   %cur_sum.7 = load i32, i32* %sum, align 4
; CHECK-NEXT:   %cmp.7 = icmp slt i32 %cur_i.7, 3
; CHECK-NEXT:   br i1 %cmp.7, label %loop_body, label %loop.loop_exit_crit_edge
; CHECK: loop.loop_exit_crit_edge:                         ; preds = %loop_body.7, %loop_body.6, %loop_body.5, %loop_body.4, %loop_body.3, %loop_body.2, %loop_body.1, %loop_body
; CHECK-NEXT:   br label %loop_exit
; CHECK: loop_exit:                                        ; preds = %loop.loop_exit_crit_edge, %entry
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
