define i32 @foo(i32* %ptr1) {
entry:
  %ptr2 = alloca i32
  %elem = getelementptr i32, i32* %ptr1, i32 2
  %val = load i32, i32* %elem
  ret i32 0
}


; CHECK: define i32 @foo(i32* %ptr1) {
; CHECK-NEXT: entry:
; CHECK-NEXT:   ret i32 0
; CHECK-NEXT: }

; case 3: memory operations without side effects include unused alloca