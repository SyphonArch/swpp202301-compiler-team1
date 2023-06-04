; test 1. check the changes for the given constants 
; we do not change when the value != 0 to 3
; including variables

; CHECK-LABEL: define i64 @i64_getelechange(i64* %a) {
; CHECK: entry:
; CHECK-NEXT:   %ss = add i64 7, 7
; CHECK-NEXT:   %0 = ptrtoint i64* %a to i64
; CHECK-NEXT:   %1 = inttoptr i64 %0 to i64*
; CHECK-NEXT:   %2 = ptrtoint i64* %a to i64
; CHECK-NEXT:   %3 = udiv i64 %2, 8
; CHECK-NEXT:   %4 = add i64 %3, 1
; CHECK-NEXT:   %5 = mul i64 %4, 8
; CHECK-NEXT:   %6 = inttoptr i64 %5 to i64*
; CHECK-NEXT:   %7 = ptrtoint i64* %a to i64
; CHECK-NEXT:   %8 = udiv i64 %7, 8
; CHECK-NEXT:   %9 = add i64 %8, 2
; CHECK-NEXT:   %10 = mul i64 %9, 8
; CHECK-NEXT:   %11 = inttoptr i64 %10 to i64*
; CHECK-NEXT:   %12 = ptrtoint i64* %a to i64
; CHECK-NEXT:   %13 = udiv i64 %12, 8
; CHECK-NEXT:   %14 = add i64 %13, 3
; CHECK-NEXT:   %15 = mul i64 %14, 8
; CHECK-NEXT:   %16 = inttoptr i64 %15 to i64*
; CHECK-NEXT:   %elem5 = getelementptr inbounds i64, i64* %a, i64 4
; CHECK-NEXT:   %elem6 = getelementptr inbounds i64, i64* %a, i64 %ss
; CHECK-NEXT:   %val1 = load i64, i64* %1, align 4
; CHECK-NEXT:   ret i64 %val1
; CHECK-NEXT: }


define i64 @i64_getelechange(i64* %a) {
entry:
  %ss = add i64 7, 7
  %elem1 = getelementptr inbounds i64, i64* %a, i64 0
  %elem2 = getelementptr inbounds i64, i64* %a, i64 1
  %elem3 = getelementptr inbounds i64, i64* %a, i64 2
  %elem4 = getelementptr inbounds i64, i64* %a, i64 3
  %elem5 = getelementptr inbounds i64, i64* %a, i64 4
  %elem6 = getelementptr inbounds i64, i64* %a, i64 %ss
  %val1 = load i64, i64* %elem1
  ret i64 %val1
}