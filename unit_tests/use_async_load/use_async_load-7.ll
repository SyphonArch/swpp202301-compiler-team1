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
  %add4 = getelementptr i32, i32* %ptr, i64 4
  %a4 = load i32, i32* %add4
  %b4 = add i32 %a4, 4
  %add5 = getelementptr i32, i32* %ptr, i64 5
  %a5 = load i32, i32* %add5
  %b5 = add i32 %a5, 5
  %dummy1 = add i32 1, 3
  %add6 = getelementptr i32, i32* %ptr, i64 6
  %a6 = load i32, i32* %add6
  %b6 = add i32 %a6, 6
  %add7 = getelementptr i32, i32* %ptr, i64 7
  %a7 = load i32, i32* %add7
  %b7 = add i32 %a7, 7
  %dummy2 = add i32 1, 3
  %dummy3 = add i32 1, 3
  ret void
}

; CHECK: define void @f(i32* %ptr) {
; CHECK-NEXT:entry:
; CHECK-NEXT:  %add1 = getelementptr i32, i32* %ptr, i64 1
; CHECK-NEXT:  %0 = call i32 @aload_i32(i32* %add1)
; CHECK-NEXT:  %add2 = getelementptr i32, i32* %ptr, i64 2
; CHECK-NEXT:  %1 = call i32 @aload_i32(i32* %add2)
; CHECK-NEXT:  %add3 = getelementptr i32, i32* %ptr, i64 3
; CHECK-NEXT:  %2 = call i32 @aload_i32(i32* %add3)
; CHECK-NEXT:  %add4 = getelementptr i32, i32* %ptr, i64 4
; CHECK-NEXT:  %3 = call i32 @aload_i32(i32* %add4)
; CHECK-NEXT:  %add5 = getelementptr i32, i32* %ptr, i64 5
; CHECK-NEXT:  %4 = call i32 @aload_i32(i32* %add5)
; CHECK-NEXT:  %add6 = getelementptr i32, i32* %ptr, i64 6
; CHECK-NEXT:  %dummy1 = add i32 1, 3
; CHECK-NEXT:  %5 = call i32 @aload_i32(i32* %add6)
; CHECK-NEXT:  %add7 = getelementptr i32, i32* %ptr, i64 7
; CHECK-NEXT:  %b1 = add i32 %0, 1
; CHECK-NEXT:  %6 = call i32 @aload_i32(i32* %add7)
; CHECK-NEXT:  %dummy3 = add i32 1, 3
; CHECK-NEXT:  %dummy2 = add i32 1, 3
; CHECK-NEXT:  %b2 = add i32 %1, 2
; CHECK-NEXT:  %b3 = add i32 %2, 3
; CHECK-NEXT:  %b4 = add i32 %3, 4
; CHECK-NEXT:  %b5 = add i32 %4, 5
; CHECK-NEXT:  %b6 = add i32 %5, 6
; CHECK-NEXT:  %b7 = add i32 %6, 7
; CHECK-NEXT:  ret void
; CHECK-NEXT:}


; case 7 : general loop-unrolled case added (requested by review)