; GVN test 1
; removes Redundant loads

; CHECK-LABEL: @foo(
; CHECK: [[val1:%.*]] = load i32, i32* [[ptr:%.*]], align 4
; CHECK-NEXT: [[sum:%.*]] = add i32 [[val1]], [[val1]]
; CHECK-NEXT: [[result:%.*]] = add i32 [[sum]], [[val1]]
; CHECK-NEXT: ret i32 [[result]]

define i32 @foo(i32* noundef %ptr) {
  %val1 = load i32, i32* %ptr
  %val2 = load i32, i32* %ptr
  %val3 = load i32, i32* %ptr
  %sum = add i32 %val1, %val2
  %result = add i32 %sum, %val3
  ret i32 %result
}