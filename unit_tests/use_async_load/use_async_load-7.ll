define void @f(i32* %ptr) {
entry:
  %add1 = getelementptr i32, i32* %ptr, i64 1
  %a1 = load i32, i32* %add1
  %b1 = add i32 %a1, 1
  %add2 = getelementptr i32, i32* %ptr, i64 2
  %a2 = load i32, i32* %add2
  %b2 = add i32 %a2, 2
  %add3 = getelementptr i32, i32* %ptr, i64 3
  %a3 = load i32, i32* %add3
  %b3 = add i32 %a3, 3
  %dummy1 = add i32 1, 2
  %dummy2 = add i32 1, 2
  ret void
}


; CHECK: define void @f(i32* %ptr) {
; CHECK-NEXT: entry:
; CHECK-NEXT:   %add1 = getelementptr i32, i32* %ptr, i64 1
; CHECK-NEXT:   %add2 = getelementptr i32, i32* %ptr, i64 2
; CHECK-NEXT:   %add3 = getelementptr i32, i32* %ptr, i64 3
; CHECK-NEXT:   %0 = call i32 @aload_i32(i32* %add1)
; CHECK-NEXT:   %1 = call i32 @aload_i32(i32* %add2)
; CHECK-NEXT:   %2 = call i32 @aload_i32(i32* %add3)
; CHECK-NEXT:   %dummy1 = add i32 1, 2
; CHECK-NEXT:   %dummy2 = add i32 1, 2
; CHECK-NEXT:   %b2 = add i32 %1, 2
; CHECK-NEXT:   %b1 = add i32 %0, 1
; CHECK-NEXT:   %b3 = add i32 %2, 3
; CHECK-NEXT:   ret void
; CHECK-NEXT: }


; case 7 : general loop-unrolled case 