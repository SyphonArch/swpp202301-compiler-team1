; Arithmetic Pass test 5
; this test includes
; change add %a, const 1 ~ 4
; change add const 1 ~ 4, %a
; there should be no changes to add i32 %a, %b



; CHECK-LABEL: @main(i32 noundef %a)

; CHECK-NEXT:  [[A1:%.*]] = call i32 @incr_i32(i32 %a)
; CHECK-NEXT:  [[A2:%.*]] = call i32 @incr_i32(i32 [[A1]])
; CHECK-NEXT:  [[A3:%.*]] = call i32 @incr_i32(i32 [[A2]])
; CHECK-NEXT:  [[A4:%.*]] = call i32 @incr_i32(i32 [[A3]])
; CHECK-NEXT:  [[A5:%.*]] = call i32 @incr_i32(i32 [[A4]])
; CHECK-NEXT:  [[A6:%.*]] = add i32 [[A2]], [[A5]]
; CHECK-NEXT: ret i32 [[A6]]
;

define i32 @main(i32 noundef %a) {
  %b = add i32 %a, 2
  %c = add i32 3, %b
  %d = add i32 %b, %c
  ret i32 %d
}