; Arithmetic Pass test 2
; this test includes
; change add %a, const 1 ~ 4
; add %a %a -> mul %a 2


; CHECK-LABEL: @main(i32 %a, i64 %b)

; CHECK-NEXT:  [[A1:%.*]] = call i32 @incr_i32(i32 %a)
; CHECK-NEXT:  [[A2:%.*]] = call i32 @incr_i32(i32 [[A1]])
; CHECK-NEXT:  [[A3:%.*]] = call i32 @incr_i32(i32 [[A2]])
; CHECK-NEXT:  [[A4:%.*]] = call i32 @incr_i32(i32 [[A3]])
; CHECK-NEXT:  [[A5:%.*]] = call i32 @incr_i32(i32 [[A4]])

; CHECK-NEXT:  [[B1:%.*]] = mul i64 %b, 2
; CHECK-NEXT:  [[B2:%.*]] = call i64 @incr_i64(i64 [[B1]])
; CHECK-NEXT:  [[B3:%.*]] = call i64 @incr_i64(i64 [[B2]])
; CHECK-NEXT:  [[B4:%.*]] = call i64 @incr_i64(i64 [[B3]])
; CHECK-NEXT:  [[B5:%.*]] = call i64 @incr_i64(i64 [[B4]])

; CHECK-NEXT: ret i64 [[B5]]
;
define i64 @main(i32 %a, i64 %b) {
  %c = add i32 %a, 2
  %d = add i32 %c, 3
  %e = add i64 %b, %b
  %f = add i64 %e, 4
  ret i64 %f
}
