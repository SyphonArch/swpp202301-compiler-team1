; test 2. check the changes for many types possible
; we do not change when the value != 0 to 3
; for i8, we don't put div and mul instructions


; CHECK-LABEL: define i64 @manytype_getelechange(i8* %a8, i16* %a16, i32* %a32, i64* %a64) {
; CHECK: entry:
; CHECK-NEXT:   %0 = ptrtoint i8* %a8 to i64
; CHECK-NEXT:   %1 = add i64 %0, 1
; CHECK-NEXT:   %2 = inttoptr i64 %1 to i8*
; CHECK-NEXT:   %3 = ptrtoint i16* %a16 to i64
; CHECK-NEXT:   %4 = udiv i64 %3, 2
; CHECK-NEXT:   %5 = add i64 %4, 2
; CHECK-NEXT:   %6 = mul i64 %5, 2
; CHECK-NEXT:   %7 = inttoptr i64 %6 to i16*
; CHECK-NEXT:   %8 = ptrtoint i32* %a32 to i64
; CHECK-NEXT:   %9 = udiv i64 %8, 4
; CHECK-NEXT:   %10 = add i64 %9, 3
; CHECK-NEXT:   %11 = mul i64 %10, 4
; CHECK-NEXT:   %12 = inttoptr i64 %11 to i32*
; CHECK-NEXT:   %13 = ptrtoint i64* %a64 to i64
; CHECK-NEXT:   %elem4 = getelementptr inbounds i64, i64* %a64, i64 4
; CHECK-NEXT:   %val1 = load i64, i64* %elem4, align 4
; CHECK-NEXT:   ret i64 %val1
; CHECK-NEXT: }

define i64 @manytype_getelechange(i8* %a8, i16* %a16, i32* %a32, i64* %a64) {
entry:
  %elem1 = getelementptr inbounds i8, i8* %a8, i64 1
  %elem2 = getelementptr inbounds i16, i16* %a16, i64 2
  %elem3 = getelementptr inbounds i32, i32* %a32, i64 3
  %elem4 = getelementptr inbounds i64, i64* %a64, i64 4
  %val1 = load i64, i64* %elem4
  ret i64 %val1
}