; Arithmetic Pass test 3
; this test includes
; change add %a, const 1 ~ 4
; test for types i8, i16, i32, i64

; CHECK-LABEL: @main(i8 %a, i16 %b, i32 %c, i64 %d)
; CHECK-NEXT:  [[A1:%.*]] = call i8 @incr_i8(i8 %a)
; CHECK-NEXT:  [[B1:%.*]] = call i16 @incr_i16(i16 %b)
; CHECK-NEXT:  [[B2:%.*]] = call i16 @incr_i16(i16 [[B1]])
; CHECK-NEXT:  [[C1:%.*]] = call i32 @incr_i32(i32 %c)
; CHECK-NEXT:  [[C2:%.*]] = call i32 @incr_i32(i32 [[C1]])
; CHECK-NEXT:  [[C3:%.*]] = call i32 @incr_i32(i32 [[C2]])
; CHECK-NEXT:  [[D1:%.*]] = call i64 @incr_i64(i64 %d)
; CHECK-NEXT:  [[D2:%.*]] = call i64 @incr_i64(i64 [[D1]])
; CHECK-NEXT:  [[D3:%.*]] = call i64 @incr_i64(i64 [[D2]])
; CHECK-NEXT:  [[D4:%.*]] = call i64 @incr_i64(i64 [[D3]])
; CHECK-NEXT: ret i64 [[D4]]

define i64 @main(i8 %a, i16 %b, i32 %c, i64 %d) {
  %e = add i8 %a, 1
  %f = add i16 %b, 2
  %g = add i32 %c, 3
  %h = add i64 %d, 4
  ret i64 %h
}
