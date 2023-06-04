; test 3. check oracle
; do notthing when the function name is oracle

; CHECK-LABEL: define i64 @oracle(i8* %a8, i16* %a16, i32* %a32, i64* %a64) {
; CHECK: entry:
; CHECK-NEXT:    %elem1 = getelementptr inbounds i8, i8* %a8, i64 1
; CHECK-NEXT:    %elem2 = getelementptr inbounds i16, i16* %a16, i64 2
; CHECK-NEXT:    %elem3 = getelementptr inbounds i32, i32* %a32, i64 3
; CHECK-NEXT:    %elem4 = getelementptr inbounds i64, i64* %a64, i64 4
; CHECK-NEXT:    %val1 = load i64, i64* %elem4, align 4
; CHECK-NEXT:    ret i64 %val1
; CHECK-NEXT:  }


define i64 @oracle(i8* %a8, i16* %a16, i32* %a32, i64* %a64) {
entry:
  %elem1 = getelementptr inbounds i8, i8* %a8, i64 1
  %elem2 = getelementptr inbounds i16, i16* %a16, i64 2
  %elem3 = getelementptr inbounds i32, i32* %a32, i64 3
  %elem4 = getelementptr inbounds i64, i64* %a64, i64 4
  %val1 = load i64, i64* %elem4
  ret i64 %val1
}