; CHECK-LABEL: define i64 @sum_array(i64* %a, i64 %n)
; CHECK: entry:
; CHECK-NEXT:   [[sum:%.*]] = alloca i64, align 8
; CHECK-NEXT:   store i64 0, i64* [[sum]], align 4
; CHECK-NEXT:   br label [[loop:%.*]]
;
; CHECK: loop:
; CHECK-NEXT:   [[i_phi:%.*]] = phi i64 [ 0, [[entry:%.*]] ], [ [[i_next:%.*]], [[loop_body:%.*]] ]
; CHECK-NEXT:   [[cmp:%.*]] = icmp slt i64 [[i_phi]], [[n:%.*]]
; CHECK-NEXT:   br i1 [[cmp]], label [[loop_body]], label [[loop_exit:%.*]]
;
; CHECK: loop_body:
; CHECK-NEXT:   [[AA:%.*]] = ptrtoint i64* [[a:%.*]] to i64
; CHECK-NEXT:   [[BB:%.*]] = mul i64 [[i_phi:%.*]], 8
; CHECK-NEXT:   [[CC:%.*]] = add i64 [[AA]], [[BB]]
; CHECK-NEXT:   [[DD:%.*]] = inttoptr i64 [[CC]] to i64*
; CHECK-NEXT:   [[aiv:%.*]] = load i64, i64* [[DD]], align 4
; CHECK-NEXT:   [[sum_val:%.*]] = load i64, i64* [[sum]], align 4
; CHECK-NEXT:   [[sum_new:%.*]] = add i64 [[sum_val]], [[aiv]]
; CHECK-NEXT:   store i64 [[sum_new]], i64* [[sum]], align 4
; CHECK-NEXT:   [[i_next1:%.*]] = add i64 [[i_phi]], 1
; CHECK-NEXT:   [[i_next]] = add i64 [[i_next1]], 1
; CHECK-NEXT:   br label [[loop]]
; CHECK:   loop_exit: 
; CHECK-NEXT:   [[sum_final:%.*]] = load i64, i64* [[sum]], align 4
; CHECK-NEXT:   ret i64 [[sum_final]]


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