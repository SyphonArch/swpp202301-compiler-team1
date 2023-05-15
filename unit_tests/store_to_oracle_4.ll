define void @f(i32 %x, i32 %y) {
entry:
  %ptra = alloca i32
  %ptrb = alloca i32
  %ptrc = alloca i32
  store i32 1, i32* %ptra
  store i32 1, i32* %ptrb
  store i32 1, i32* %ptrc
  ret void
}


; CHECK: define void @f(i32 %x, i32 %y) {
; CHECK-NEXT: entry:
; CHECK-NEXT:   %ptra = alloca i32, align 4
; CHECK-NEXT:   %ptrb = alloca i32, align 4
; CHECK-NEXT:   %ptrc = alloca i32, align 4
; CHECK-NEXT:   %1 = call void @Oracle(i32* %ptra, i32* %ptrb, i32* %ptrc, i32 3) ; Oracle intrinsic call
; CHECK-NEXT:   ret void
; CHECK-NEXT: }

; declare void @Oracle(i32*, i32*, i32*, i32)

; case 2 : replace to oracle