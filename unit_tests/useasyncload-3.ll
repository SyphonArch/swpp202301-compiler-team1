define void @f(i32 %x, i32 %y) {
entry:
  %ptra = alloca i32
  %ptrb = alloca i32
  %ptrc = alloca i32
  store i32 1, ptr %ptra
  store i32 3, ptr %ptrb
  store i32 5, ptr %ptrc
  %a = load i32, ptr %ptra
  %b = load i32, ptr %ptrb
  %c = load i32, ptr %ptrc
  %d = add i32 %a, %a
  %e = add i32 %b, %a
  %ptrz = alloca i32
  %f = add i32 0, 0
  %g = add i32 %c, %c
  %h = mul i32 1, 1
  store i32 7, ptr %ptrz
  %z = load i32, ptr %ptrz
  %zz = mul i32 %z, %z
  store i32 %d, ptr %ptra
  ret void
}

; CHECK: define void @f(i32 %x, i32 %y) {
; CHECK-NEXT: entry:
; CHECK-NEXT:   %ptra = alloca i32, align 4
; CHECK-NEXT:   %ptrb = alloca i32, align 4
; CHECK-NEXT:   %ptrc = alloca i32, align 4
; CHECK-NEXT:   store i32 1, ptr %ptra, align 4
; CHECK-NEXT:   store i32 3, ptr %ptrb, align 4
; CHECK-NEXT:   store i32 5, ptr %ptrc, align 4
; CHECK-NEXT:   [[VAL0:%.*]] = call i32 @aload_i32(ptr %ptra)
; CHECK-NEXT:   %b = load i32, ptr %ptrb, align 4
; CHECK-NEXT:   [[VAL1:%.*]] = call i32 @aload_i32(ptr %ptrc)
; CHECK-NEXT:   %e = add i32 %b, [[VAL0]]
; CHECK-NEXT:   %h = mul i32 1, 1
; CHECK-NEXT:   %f = add i32 0, 0
; CHECK-NEXT:   %ptrz = alloca i32, align 4
; CHECK-NEXT:   store i32 7, ptr %ptrz, align 4
; CHECK-NEXT:   [[VAL2:%.*]] = call i32 @aload_i32(ptr %ptrz)
; CHECK-NEXT:   %g = add i32 [[VAL1]], [[VAL1]]
; CHECK-NEXT:   %d = add i32 [[VAL0]], [[VAL0]]
; CHECK-NEXT:   store i32 %d, ptr %ptra, align 4
; CHECK-NEXT:   %zz = mul i32 [[VAL2]], [[VAL2]]
; CHECK-NEXT:   ret void
; CHECK-NEXT:   }

declare i32 @aload_i32(ptr)

; case 3: multiple load replaced, dependent instructions below. check for unchanged load