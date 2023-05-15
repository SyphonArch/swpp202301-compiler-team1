define void @f(i32 %x, i32 %y) {
entry:
  %ptra = alloca i32
  store i32 1, i32* %ptra
  %ptrb = alloca i32
  store i32 1, i32* %ptrb
  %ptrc = alloca i32
  store i32 1, i32* %ptrc
  ret void
}

; CHECK: define void @f(i32 %x, i32 %y) {
; CHECK-NEXT: entry:
; CHECK-NEXT:   %ptra = alloca i32, align 4
; CHECK-NEXT:   store i32 1, i32* %ptra, align 4
; CHECK-NEXT:   %ptrb = alloca i32, align 4
; CHECK-NEXT:   store i32 1, i32* %ptrb, align 4
; CHECK-NEXT:   %ptrc = alloca i32, align 4
; CHECK-NEXT:   store i32 1, i32* %ptrc, align 4
; CHECK-NEXT:   ret void
; CHECK-NEXT: }

; case 1 : no replace done