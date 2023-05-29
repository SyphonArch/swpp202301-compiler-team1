declare i8* @malloc(i64)
declare void @free(i8*)

define i32 @main() {
; CHECK:   %new_alloca = alloca i8, i64 16, align 1
; CHECK-NEXT:   %1 = bitcast i8* %new_alloca to i32*
; CHECK-NEXT:   store i32 42, i32* %1, align 4
; CHECK-NEXT:   %2 = getelementptr i32, i32* %1, i64 1
; CHECK-NEXT:   store i32 24, i32* %2, align 4
; CHECK-NEXT:   %3 = load i32, i32* %1, align 4
; CHECK-NEXT:   %4 = load i32, i32* %2, align 4
; CHECK-NEXT:   call void @print(i32 %3)
; CHECK-NEXT:   call void @print(i32 %4)
; CHECK-NEXT:   ret i32 0

  ; Allocate memory using malloc
  %1 = call i8* @malloc(i64 16)

  ; Store some values in the allocated memory
  %2 = bitcast i8* %1 to i32*
  store i32 42, i32* %2
  %3 = getelementptr i32, i32* %2, i64 1
  store i32 24, i32* %3

  ; Load and print the values from the allocated memory
  %4 = load i32, i32* %2
  %5 = load i32, i32* %3
  call void @print(i32 %4)
  call void @print(i32 %5)

  ; Free the allocated memory using free
  call void @free(i8* %1)

  ret i32 0
}

declare void @print(i32)
