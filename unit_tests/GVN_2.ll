; GVN test 2
; removes Common subexpressions

; CHECK-LABEL: @bar(i32 %a, i32 %b, i32 %c)
; CHECK: [[mul1:%.*]] = mul i32 [[b:%.*]], [[c:%.*]]
; CHECK-NEXT: [[add1:%.*]] = add i32 [[a:%.*]], [[mul1]]
; CHECK-NEXT: [[add2:%.*]] = add i32 [[add1]], [[mul1]]
; CHECK-NEXT: ret i32 [[add2]]

define i32 @bar(i32 %a, i32 %b, i32 %c) {
  %mul1 = mul i32 %b, %c
  %add1 = add i32 %a, %mul1
  %mul2 = mul i32 %b, %c
  %add2 = add i32 %add1, %mul2
  ret i32 %add2
}