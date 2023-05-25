define i32 @sum_values(i32 %start, i32 %end) {
  entry:
    %sum = alloca i32
    store i32 0, i32* %sum
    br label %loop

  loop:
    %i = load i32, i32* %sum
    %compare = icmp slt i32 %i, %end
    br i1 %compare, label %body, label %exit

  body:
    %current_value = add i32 %i, 1
    %updated_sum = add i32 %current_value, %i
    store i32 %updated_sum, i32* %sum
    br label %loop

  exit:
    %final_sum = load i32, i32* %sum
    %i_value = add i32 %i, 0
    ret i32 %final_sum
}

; CHECK: define i32 @sum_values(i32 %start, i32 %end) {
; CHECK: entry:
; CHECK-NEXT:  %sum = alloca i32, align 4
; CHECK-NEXT:  store i32 0, i32* %sum, align 4
; CHECK-NEXT:  br label %loop

; CHECK: loop:                                             ; preds = %body, %entry
; CHECK-NEXT:  %i = load i32, i32* %sum, align 4
; CHECK-NEXT:  %compare = icmp slt i32 %i, %end
; CHECK-NEXT:  br i1 %compare, label %body, label %exit

; CHECK: body:                                             ; preds = %loop
; CHECK-NEXT:  %current_value = add i32 %i, 1
; CHECK-NEXT:  %updated_sum = add i32 %current_value, %i
; CHECK-NEXT:  store i32 %updated_sum, i32* %sum, align 4
; CHECK-NEXT:  br label %loop

; CHECK: exit:                                             ; preds = %loop
; CHECK-NEXT:  %i.lcssa = phi i32 [ %i, %loop ]
; CHECK-NEXT:  %final_sum = load i32, i32* %sum, align 4
; CHECK-NEXT:  %i_value = add i32 %i.lcssa, 0
; CHECK-NEXT:  ret i32 %final_sum
; CHECK-NEXT: }

; case 3: no phi nodes, usage of inner value.