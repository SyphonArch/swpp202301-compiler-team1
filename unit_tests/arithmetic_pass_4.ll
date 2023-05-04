; Arithmetic Pass test 4
; this test includes
; change add %a, const 1 ~ 4
; change sub %a, const 1 ~ 4



; CHECK-LABEL: @main(i32 %a)

; CHECK-NEXT:  [[A1:%.*]] = call i32 @incr_i32(i32 %a)
; CHECK-NEXT:  [[A2:%.*]] = call i32 @incr_i32(i32 [[A1]])
; CHECK-NEXT:  [[A3:%.*]] = call i32 @incr_i32(i32 [[A2]])
; CHECK-NEXT:  [[A4:%.*]] = call i32 @incr_i32(i32 [[A3]])
; CHECK-NEXT:  [[A5:%.*]] = call i32 @incr_i32(i32 [[A4]])

; CHECK-NEXT:  [[A6:%.*]] = call i32 @decr_i32(i32 [[A5]])
; CHECK-NEXT:  [[A7:%.*]] = call i32 @decr_i32(i32 [[A6]])
; CHECK-NEXT:  [[A8:%.*]] = call i32 @decr_i32(i32 [[A7]])
; CHECK-NEXT:  [[A9:%.*]] = call i32 @decr_i32(i32 [[A8]])

; CHECK-NEXT: ret i32 [[A9]]
;
define i32 @main(i32 %a) {
  %b = add i32 %a, 2
  %c = add i32 %b, 3
  %d = sub i32 %c, 4
  ret i32 %d
}