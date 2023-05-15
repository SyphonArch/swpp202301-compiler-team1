define i32 @foo() {
entry:
  %ptr1 = alloca i32
  store i32 42, i32* %ptr1
  %ptr2 = alloca i32
  store i32 42, i32* %ptr2
  %val1 = load i32, i32* %ptr1
  ret i32 0
}

; CHECK: define i32 @foo() {
; CHECK-NEXT: entry:
; CHECK-NEXT:  %ptr1 = alloca i32, align 4
; CHECK-NEXT:  store i32 42, i32* %ptr1, align 4
; CHECK-NEXT:  %ptr2 = alloca i32, align 4
; CHECK-NEXT:  store i32 42, i32* %ptr2, align 4
; CHECK-NEXT:  ret i32 0
; CHECK-NEXT: }

; case 2: removed unused load; alloca and store may have side effects