define i32 @bar(i32 %x) {
entry:
  %y = add i32 %x, 5
  %z = mul i32 %x, 3
  %unused = sub i32 %z, %y
  ret i32 %y
}

; CHECK: define i32 @bar(i32 %x) {
; CHECK-NEXT:entry:
; CHECK-NEXT:  %y = add i32 %x, 5
; CHECK-NEXT:  %z = mul i32 %x, 3
; CHECK-NEXT:  %unused = sub i32 %z, %y
; CHECK-NEXT:  ret i32 %y
; CHECK-NEXT: }

; case 1: removed unused arith values