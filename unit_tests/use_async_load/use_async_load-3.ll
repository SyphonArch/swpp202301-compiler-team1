define void @f(i32 %x, i32 %y) {
entry:
  %ptra = alloca i32
  %ptrb = alloca i32
  %ptrc = alloca i32
  store i32 1, i32* %ptra
  store i32 3, i32* %ptrb
  store i32 5, i32* %ptrc
  %a = load i32, i32* %ptra
  %b = load i32, i32* %ptrb
  %c = load i32, i32* %ptrc
  %d = add i32 %a, %a
  %e = add i32 %b, %a
  %ptrz = alloca i32
  %f = add i32 0, 0
  %g = add i32 %c, %c
  %h = mul i32 1, 1
  store i32 7, i32* %ptrz
  %z = load i32, i32* %ptrz
  %zz = mul i32 %z, %z
  store i32 %d, i32* %ptra
  ret void
}

; CHECK: define void @f(i32 %x, i32 %y) {
; CHECK-NEXT: entry:
; CHECK-NEXT:   %ptra = alloca i32, align 4
; CHECK-NEXT:   %ptrb = alloca i32, align 4
; CHECK-NEXT:   %ptrc = alloca i32, align 4
; CHECK-NEXT:   store i32 1, i32* %ptra, align 4
; CHECK-NEXT:   store i32 3, i32* %ptrb, align 4
; CHECK-NEXT:   store i32 5, i32* %ptrc, align 4
; CHECK-NEXT:   %0 = call i32 @aload_i32(i32* %ptra)
; CHECK-NEXT:   %1 = call i32 @aload_i32(i32* %ptrb)
; CHECK-NEXT:   %2 = call i32 @aload_i32(i32* %ptrc)
; CHECK-NEXT:   %ptrz = alloca i32, align 4
; CHECK-NEXT:   store i32 7, i32* %ptrz, align 4
; CHECK-NEXT:   %3 = call i32 @aload_i32(i32* %ptrz)
; CHECK-NEXT:   %g = add i32 %2, %2
; CHECK-NEXT:   %f = add i32 0, 0
; CHECK-NEXT:   %e = add i32 %1, %0
; CHECK-NEXT:   %h = mul i32 1, 1
; CHECK-NEXT:   %d = add i32 %0, %0
; CHECK-NEXT:   store i32 %d, i32* %ptra, align 4
; CHECK-NEXT:   %zz = mul i32 %3, %3
; CHECK-NEXT:   ret void
; CHECK-NEXT: }


; case 3: multiple load replaced, dependent instructions below.
