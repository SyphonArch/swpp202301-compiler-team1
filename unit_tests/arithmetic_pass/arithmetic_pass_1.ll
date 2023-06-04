; Arithmetic Pass test 1
; this test includes
; add %a %a -> mul %a 2
; shl %x c -> mul %x (2^c)
; ashr %x c -> sdiv %x (2^c)
; lshr %x c -> udiv %x (2^c)
; sub 0 %a -> mul %a -1
; add %a 0 -> mul %a 1
; and %a 2^c-1 -> urem %a 2^c

define i32 @main(i32 noundef %a, i32 noundef %b) {
; CHECK-LABEL: @main(i32 noundef %a, i32 noundef %b)
; CHECK-NEXT: [[C:%.*]] = mul i32 [[A:%.*]], 2
; CHECK-NEXT: [[D:%.*]] = mul i32 [[C]], 32
; CHECK-NEXT: [[E:%.*]] = sdiv i32 [[D]], 8
; CHECK-NEXT: [[F:%.*]] = udiv i32 [[E]], 32
; CHECK-NEXT: [[G:%.*]] = mul i32 [[F]], -1
; CHECK-NEXT: [[H:%.*]] = mul i32 [[G]], 1
; CHECK-NEXT: [[I:%.*]] = urem i32 [[H]], 16
; CHECK-NEXT: [[J:%.*]] = urem i32 [[I]], 8
; CHECK-NEXT: [[K:%.*]] = urem i32 [[J]], 1
; CHECK-NEXT: ret i32 [[K]]
;
  %c = add i32 %a, %a
  %d = shl i32 %c, 5
  %e = ashr i32 %d, 3
  %f = lshr i32 %e, 5
  %g = sub i32 0, %f
  %h = add i32 %g, 0
  %i = and i32 %h, 15
  %j = and i32 7, %i
  %k = and i32 %j, 0
  ret i32 %k
}