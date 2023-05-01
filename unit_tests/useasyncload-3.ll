define void @f(i32 %x, i32 %y) {
entry:
  %ptra = alloca i32
  %ptrb = alloca i32
  store i32 1, ptr %ptra
  store i32 3, ptr %ptrb
  %a = load i32, ptr %ptra
  %b = load i32, ptr %ptrb
  %c = add i32 %a, %a
  %e = add i32 %b, %a
  %g = add i32 0, 0
  %d = add i32 %c, %c
  %z = sub i32 1, 1
  ret void
}

; CHECK: define void @f(i32 %x, i32 %y) {
; CHECK-NEXT: entry:
; CHECK-NEXT:   %ptra = alloca i32, align 4
; CHECK-NEXT:   %ptrb = alloca i32, align 4
; CHECK-NEXT:   store i32 1, ptr %ptra, align 4
; CHECK-NEXT:   store i32 3, ptr %ptrb, align 4
; CHECK-NEXT:   [[VAL1:%.*]] = call i32 @aload_i32(ptr %ptra)
; CHECK-NEXT:   [[VAL2:%.*]] = call i32 @aload_i32(ptr %ptrb)
; CHECK-NEXT:   %g = add i32 0, 0
; CHECK-NEXT:   %c = add i32 [[VAL1:%.*]], [[VAL1:%.*]]
; CHECK-NEXT:   %d = add i32 %c, %c
; CHECK-NEXT:   %z = sub i32 1, 1
; CHECK-NEXT:   %e = add i32 [[VAL2:%.*]], [[VAL1:%.*]]
; CHECK-NEXT:   ret void
; CHECK-NEXT: }
