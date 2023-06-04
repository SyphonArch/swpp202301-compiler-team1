define void @oracle(i32* %ptr) {
entry:
  %o = add i32 1, 3
  %p = add i32 1, 3
  %h = add i32 1, 3
  %j = add i32 1, 2
  %d = add i32 3, 4
  %e = add i32 5, 6
  %add1 = getelementptr i32, i32* %ptr, i64 1
  %add2 = getelementptr i32, i32* %ptr, i64 2
  %add3 = getelementptr i32, i32* %ptr, i64 3
  %a1 = load i32, i32* %add1
  %a2 = load i32, i32* %add2
  %a3 = load i32, i32* %add3
  ret void
}


; CHECK: define void @oracle(i32* %ptr) {
; CHECK-NEXT: entry:
; CHECK-NEXT:   %o = add i32 1, 3
; CHECK-NEXT:   %p = add i32 1, 3
; CHECK-NEXT:   %h = add i32 1, 3
; CHECK-NEXT:   %j = add i32 1, 2
; CHECK-NEXT:   %d = add i32 3, 4
; CHECK-NEXT:   %e = add i32 5, 6
; CHECK-NEXT:   %add1 = getelementptr i32, i32* %ptr, i64 1
; CHECK-NEXT:   %add2 = getelementptr i32, i32* %ptr, i64 2
; CHECK-NEXT:   %add3 = getelementptr i32, i32* %ptr, i64 3
; CHECK-NEXT:   %a1 = load i32, i32* %add1, align 4
; CHECK-NEXT:   %a2 = load i32, i32* %add2, align 4
; CHECK-NEXT:   %a3 = load i32, i32* %add3, align 4
; CHECK-NEXT:   ret void
; CHECK-NEXT: }


; case 6 : if the name of the function is oracle, do not optimize