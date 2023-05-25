define void @f(i32* %ptr) {
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

; CHECK: define void @f(i32* %ptr) {
; CHECK-NEXT: entry:
; CHECK-NEXT:   %o = add i32 1, 3
; CHECK-NEXT:   %add1 = getelementptr i32, i32* %ptr, i64 1
; CHECK-NEXT:   %add2 = getelementptr i32, i32* %ptr, i64 2
; CHECK-NEXT:   %add3 = getelementptr i32, i32* %ptr, i64 3
; CHECK-NEXT:   %0 = call i32 @aload_i32(i32* %add1)
; CHECK-NEXT:   %1 = call i32 @aload_i32(i32* %add2)
; CHECK-NEXT:   %2 = call i32 @aload_i32(i32* %add3)
; CHECK-NEXT:   %e = add i32 5, 6
; CHECK-NEXT:   %d = add i32 3, 4
; CHECK-NEXT:   %j = add i32 1, 2
; CHECK-NEXT:   %h = add i32 1, 3
; CHECK-NEXT:   %p = add i32 1, 3
; CHECK-NEXT:   ret void
; CHECK-NEXT: }


; case 4 : moving both getelementptr and load, but stops when no more moving is needed to reduce cost