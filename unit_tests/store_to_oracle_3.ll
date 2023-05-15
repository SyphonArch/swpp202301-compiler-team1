define void @f(i32 %x, i32 %y) {
entry:
  %ptra = alloca i32
  %ptrb = alloca i32
  %ptrc = alloca i32
  store i32 1, i32* %ptra
  add %a = i32 1, 0
  store i32 1, i32* %ptrb
  add %b = i32 2, 0
  store i32 1, i32* %ptrc
  add %c = i32 3, 0
  ret void
}


; CHECK: define void @f(i32 %x, i32 %y) {
; CHECK-NEXT: entry:
; CHECK-NEXT:   %ptra = alloca i32, align 4
; CHECK-NEXT:   %ptrb = alloca i32, align 4
; CHECK-NEXT:   %ptrc = alloca i32, align 4
; CHECK-NEXT:   %1 = call void @Oracle(i32* %ptra, i32* %ptrb, i32* %ptrc, i32 3) ; Oracle intrinsic call
; CHECK-NEXT:   add %a = i32 1, 0
; CHECK-NEXT:   add %b = i32 2, 0
; CHECK-NEXT:   add %c = i32 3, 0
; CHECK-NEXT:   ret void
; CHECK-NEXT: }

; declare void @Oracle(i32*, i32*, i32*, i32)

; case 3 : replace to oracle, move store