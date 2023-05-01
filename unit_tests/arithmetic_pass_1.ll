
define i32 @main(i32 %a, i32 %b) {
; CHECK-LABEL: @main(i32 %a, i32 %b)
; CHECK-NEXT: [[C:%.*]] = mul i32 [[A:%.*]], 2
; CHECK-NEXT: [[D:%.*]] = mul i32 [[C]], 16
; CHECK-NEXT: [[E:%.*]] = sdiv i32 [[D]], 8
; CHECK-NEXT: [[F:%.*]] = udiv i32 [[E]], 32
; CHECK-NEXT: ret i32 [[F]]
;
  %c = add i32 %a, %a
  %d = shl i32 %c, 4
  %e = ashr i32 %d, 3
  %f = lshr i32 %e, 5
  ret i32 %f
}