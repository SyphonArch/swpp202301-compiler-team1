define i32 @foo(i32* %arr, i32 %i) {
entry:
  %gep = getelementptr i32, i32* %arr, i32 %i
  %val = load i32, i32* %gep
  ret i32 %val
}