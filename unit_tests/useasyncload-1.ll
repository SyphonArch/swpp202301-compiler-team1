define void @f(i32 %x, i32 %y) {
entry:
  %ptr = alloca i32
  store i32 1, i32* %ptr
  %a = load i32, i32* %ptr
  %b = add i32 %a, %a
  ret void
}

; CHECK: define void @f(i32 %x, i32 %y) {
; CHECK-NEXT: entry:
; CHECK-NEXT:   %ptr = alloca i32, align 4
; CHECK-NEXT:   store i32 1, i32* %ptr, align 4
; CHECK-NEXT:   %a = load i32, i32* %ptr, align 4
; CHECK-NEXT:   %b = add i32 %a, %a
; CHECK-NEXT:   ret void
; CHECK-NEXT: }
