; GVN test 3
; removes Loop-invariant code motion

; CHECK-LABEL: @baz(
; CHECK: [[i:%.*]] = alloca i32
; CHECK-NEXT: store i32 0, i32* [[i]], align 4
; CHECK-NEXT: [[elem1:%.*]] = load i32, i32* [[ptr1:%.*]], align 4
; CHECK-NEXT: [[elem2:%.*]] = load i32, i32* [[ptr2:%.*]], align 4
; CHECK: br label [[loop:%.*]]
; CHECK:  loop:
; CHECK-NEXT: [[index:%.*]] = phi i32 [ [[new_index:%.*]], [[loop]] ], [ 0, %0 ]
; CHECK-NEXT: [[mul:%.*]] = mul i32 [[elem1:%.*]], [[elem2:%.*]]
; CHECK-NEXT: [[result:%.*]] = add i32 %index, [[mul]]
; CHECK-NEXT: [[new_index]] = add i32 [[index]], 1
; CHECK-NEXT: store i32 [[new_index]], i32* [[i]], align 4
; CHECK-NEXT: [[cmp:%.*]] = icmp slt i32 [[new_index]], [[n:%.*]]
; CHECK-NEXT: br i1 [[cmp]], label [[loop]], label [[exit:%.*]]
; CHECK:  exit:
; CHECK-NEXT: ret void


define void @baz(i32 %n, i32* %ptr1, i32* %ptr2) {
  %i = alloca i32
  store i32 0, i32* %i
  br label %loop

loop:
  %index = load i32, i32* %i
  %elem1 = load i32, i32* %ptr1
  %elem2 = load i32, i32* %ptr2
  %mul = mul i32 %elem1, %elem2
  %result = add i32 %index, %mul
  %new_index = add i32 %index, 1
  store i32 %new_index, i32* %i
  %cmp = icmp slt i32 %new_index, %n
  br i1 %cmp, label %loop, label %exit

exit:
  ret void
}