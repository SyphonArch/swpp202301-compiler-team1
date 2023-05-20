define void @example() {
  entry:
    %i = alloca i32
    store i32 0, i32* %i
    br label %loop

  loop:
    %0 = load i32, i32* %i
    %1 = add i32 %0, 1
    store i32 %1, i32* %i
    %2 = load i32, i32* %i
    %3 = icmp slt i32 %2, 10
    br i1 %3, label %loop, label %exit

  exit:
    ret void
}

; CHECK: define i32 @bar(i32 %x) {
; CHECK-NEXT:entry:
; CHECK-NEXT:  %y = add i32 %x, 5
; CHECK-NEXT:  %z = mul i32 %x, 3
; CHECK-NEXT:  %unused = sub i32 %z, %y
; CHECK-NEXT:  ret i32 %y
; CHECK-NEXT: }

; case 1: removed unused arith value