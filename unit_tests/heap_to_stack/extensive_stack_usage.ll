; This program uses too much stack.
; The pass will not modify the module.

declare i8* @malloc(i64)
declare void @free(i8*)

define i32 @main() {
    ; Allocate 102000 bytes on the stack
    %stackmem = alloca i8, i64 102000

    ; Allocate 1000 bytes on the heap
    ; CHECK: call i8* @malloc(i64 1000)
    %heapmem = call i8* @malloc(i64 1000)

    ret i32 0
}

; CHECK-NOT: @my_malloc
