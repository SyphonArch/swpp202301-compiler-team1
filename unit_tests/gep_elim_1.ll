; CHECK-LABEL: define i64 @sum_array(i64* %a, i64 %n) {
; CHECK: entry:
; CHECK-NEXT:   [[sum:*.%]] = alloca i64, align 8
; CHECK-NEXT:   store i64 0, i64* [[%sum]], align 4
; CHECK-NEXT:   br label [[%loop:*.%]]
;
; CHECK: loop:
; CHECK-NEXT:   [[%i_phi:*.%]] = phi i64 [ 0, [[%entry:*.%]] ], [ [[%i_next:*.%]], [[%loop_body:*.%]] ]
; CHECK-NEXT:   [[%cmp:*.%]] = icmp slt i64 [[%i_phi]], [[%n:*.%]]
; CHECK-NEXT:   br i1 [[%cmp]], label [[%loop_body]], label [[%loop_exit:*.%]]
;
; CHECK-NEXT: loop_body:
; CHECK-NEXT:   %0 = ptrtoint i64* %a to i64
; CHECK-NEXT:   %1 = mul i64 %i_phi, 8
; CHECK-NEXT:   %2 = add i64 %0, %1
; CHECK-NEXT:   %3 = inttoptr i64 %2 to i64*
; CHECK-NEXT:   %aiv = load i64, i64* %3, align 4
; CHECK-NEXT:   %sum_val = load i64, i64* %sum, align 4
; CHECK-NEXT:   %sum_new = add i64 %sum_val, %aiv
; CHECK-NEXT:   store i64 %sum_new, i64* %sum, align 4

define i64 @sum_array(i64* %a, i64 %n) {
entry:
  %sum = alloca i64
  store i64 0, i64* %sum
  br label %loop

loop:
  %i_phi = phi i64 [ 0, %entry ], [ %i_next, %loop_body ]
  %cmp = icmp slt i64 %i_phi, %n
  br i1 %cmp, label %loop_body, label %loop_exit

loop_body:
  %ai = getelementptr inbounds i64, i64* %a, i64 %i_phi
  %aiv = load i64, i64* %ai
  %sum_val = load i64, i64* %sum
  %sum_new = add i64 %sum_val, %aiv
  store i64 %sum_new, i64 *%sum

  %i_next1 = add i64 %i_phi, 1
  %i_next = add i64 %i_next1, 1
  br label %loop

loop_exit:
  %sum_final = load i64, i64* %sum
  ret i64 %sum_final
}