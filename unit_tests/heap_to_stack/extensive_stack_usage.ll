; This program uses too much stack.
; Interception will still happen, but the START-HEAP constant will be high.

declare i8* @malloc(i64)
declare void @free(i8*)

define i32 @main() {
    ; Allocate 102000 bytes on the stack
    %stackmem = alloca i8, i64 102000

    ; Allocate 1000 bytes on the heap
    %heapmem = call i8* @malloc(i64 1000)

    ret i32 0
}

; CHECK-LABEL: define i8* @my_malloc(i64 %size) {
; CHECK-NEXT:  %heap_start_int = add i64 0, 103048
