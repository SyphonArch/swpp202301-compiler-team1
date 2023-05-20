define void @nested_example() {
  entry:
    %i = alloca i32
    store i32 0, i32* %i
    br label %outer_loop

  outer_loop:
    %0 = load i32, i32* %i
    %1 = add i32 %0, 1
    store i32 %1, i32* %i
    %2 = icmp slt i32 %1, 5
    br i1 %2, label %inner_loop, label %exit

  inner_loop:
    %3 = load i32, i32* %i
    %4 = add i32 %3, 1
    store i32 %4, i32* %i
    %5 = icmp slt i32 %4, 3
    br i1 %5, label %inner_loop, label %outer_loop

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