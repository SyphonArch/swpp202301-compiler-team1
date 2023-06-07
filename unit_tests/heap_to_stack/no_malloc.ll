; `my_malloc` should not even appear here

declare i8* @malloc(i64)
declare void @free(i8*)

define i32 @main() {
  ret i32 0
}

; CHECK-NOT: my_malloc