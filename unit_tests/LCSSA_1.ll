define i32 @factorial(i32 %n) {
  entry:
    %cmp1 = icmp sle i32 %n, 1
    br i1 %cmp1, label %return_one, label %loop
    
  loop:

    %counter = phi i32 [ %n, %entry ], [ %next_value, %loop ]
    %result = phi i32 [ 1, %entry ], [ %mul_result, %loop ]
    %mul_result = mul i32 %result, %counter
    %next_value = sub i32 %counter, 1
    %cmp2 = icmp sgt i32 %next_value, 1
    br i1 %cmp2, label %loop, label %exit
    
  exit:
    %redundant = add i32 %counter, 1
    ret i32 %mul_result
    
  return_one:
    ret i32 1
}

; CHECK: define i32 @factorial(i32 %n) {
; CHECK: entry:
; CHECK-NEXT:   %cmp1 = icmp sle i32 %n, 1
; CHECK-NEXT:   br i1 %cmp1, label %return_one, label %loop
; CHECK: loop:                                             ; preds = %loop, %entry
; CHECK-NEXT:   %counter = phi i32 [ %n, %entry ], [ %next_value, %loop ]
; CHECK-NEXT:   %result = phi i32 [ 1, %entry ], [ %mul_result, %loop ]
; CHECK-NEXT:   %mul_result = mul i32 %result, %counter
; CHECK-NEXT:   %next_value = sub i32 %counter, 1
; CHECK-NEXT:   %cmp2 = icmp sgt i32 %next_value, 1
; CHECK-NEXT:   br i1 %cmp2, label %loop, label %exit
; CHECK: exit:                                             ; preds = %loop
; CHECK-NEXT:   %counter.lcssa = phi i32 [ %counter, %loop ]
; CHECK-NEXT:   %mul_result.lcssa = phi i32 [ %mul_result, %loop ]
; CHECK-NEXT:   %redundant = add i32 %counter.lcssa, 1
; CHECK-NEXT:   ret i32 %mul_result.lcssa
; CHECK: return_one:                                       ; preds = %entry
; CHECK-NEXT:   ret i32 1

; case 1: simple loop example: factorial