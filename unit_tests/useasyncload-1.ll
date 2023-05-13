define void @f(i32 %x, i32 %y) {
entry:
  %c = add i32 1, 2
  %d = add i32 3, 4
  %e = add i32 5, 6
  %ptr = alloca i32
  store i32 1, ptr %ptr
  %a = load i32, ptr %ptr
  %b = mul i32 %a, %a
  ret void
}

; CHECK: define void @f(i32 %x, i32 %y) {
; CHECK-NEXT: entry:
; CHECK-NEXT:   %c = add i32 1, 2
; CHECK-NEXT:   %d = add i32 3, 4
; CHECK-NEXT:   %e = add i32 5, 6
; CHECK-NEXT:   %ptr = alloca i32, align 4
; CHECK-NEXT:   store i32 1, ptr %ptr, align 4
; CHECK-NEXT:   %a = load i32, ptr %ptr, align 4
; CHECK-NEXT:   %b = mul i32 %a, %a
; CHECK-NEXT:   ret void
; CHECK-NEXT: }

; case 1 : no replace done