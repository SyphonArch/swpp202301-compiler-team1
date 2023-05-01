
define void @f(i32 %x, i32 %y) {
entry:
  %ptr = alloca i32
  store i32 1, ptr %ptr
  %a = load i32, ptr %ptr
  %b = add i32 %a, %a
  %c = add i32 0, 0
  %d = add i32 1, 1
  %e = add i32 %b, %a
  ret void
}

; CHECK: define void @f(i32 %x, i32 %y) {
; CHECK: %ptr = alloca i32, align 4
; CHECK: store i32 1, ptr %ptr, align 4
; CHECK: [[VAL:%.*]] = call i32 @aload_i32(ptr %ptr)
; CHECK: %c = add i32 0, 0
; CHECK: %d = add i32 1, 1
; CHECK: %b = add i32 [[VAL:%.*]], [[VAL:%.*]]
; CHECK: %e = add i32 %b, [[VAL:%.*]]
; CHECK: ret void

declare i32 @aload_i32(i32*)
