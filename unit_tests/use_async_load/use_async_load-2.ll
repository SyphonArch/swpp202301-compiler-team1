
define void @f(i32 %x, i32 %y) {
entry:
  %ptr = alloca i32
  store i32 1, i32* %ptr
  %a = load i32, i32* %ptr
  %b = add i32 %a, %a
  %c = add i32 %b, %a
  %d = add i32 0, 0
  %e = add i32 1, 1
  %f = sub i32 %a, %e
  %g = mul i32 %e, %f
  %h = mul i32 2, 2
  ret void
}

; CHECK: define void @f(i32 %x, i32 %y) {
; CHECK-NEXT: entry:
; CHECK-NEXT:   %ptr = alloca i32, align 4
; CHECK-NEXT:   store i32 1, i32* %ptr, align 4
; CHECK-NEXT:   %0 = call i32 @aload_i32(i32* %ptr)
; CHECK-NEXT:   %e = add i32 1, 1
; CHECK-NEXT:   %h = mul i32 2, 2
; CHECK-NEXT:   %d = add i32 0, 0
; CHECK-NEXT:   %b = add i32 %0, %0
; CHECK-NEXT:   %c = add i32 %b, %0
; CHECK-NEXT:   %f = sub i32 %0, %e
; CHECK-NEXT:   %g = mul i32 %e, %f
; CHECK-NEXT:   ret void
; CHECK-NEXT: }

; case 2 : single load replaced, CHECK-NEXT for cost before use
