; Check that replacement doesn't happen on recursive modules

define void @recursive_malloc(i64 %n) {
; CHECK-LABEL: @recursive_malloc(i64 %n)
; CHECK: entry:
; CHECK-NEXT:   %0 = icmp sgt i64 %n, 0
; CHECK-NEXT:   br i1 %0, label %allocate, label %finish
; CHECK: allocate:
; CHECK-NEXT:   %1 = call i8* @malloc(i64 %n)
; CHECK-NEXT:   %2 = add i64 %n, -1
; CHECK-NEXT:   call void @recursive_malloc(i64 %2)
; CHECK-NEXT:   call void @free(i8* %1)
; CHECK-NEXT:   br label %finish
; CHECK: finish:
; CHECK-NEXT:   ret void

entry:
  %0 = icmp sgt i64 %n, 0
  br i1 %0, label %allocate, label %finish

allocate:
  %1 = call i8* @malloc(i64 %n)
  %2 = add i64 %n, -1
  call void @recursive_malloc(i64 %2)
  call void @free(i8* %1)
  br label %finish

finish:
  ret void
}

declare i8* @malloc(i64)
declare void @free(i8*)
