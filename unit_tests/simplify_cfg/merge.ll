; RUN: opt < %s -load-pass-plugin=./build/libSimplifyCFG.so -passes=simplify-cfg -S | FileCheck %s

; Check if SimplifyCFGPass merges basic blocks with unconditional branches

declare void @use(i1)

define void @test(i1 %c) {
; CHECK-LABEL: @test(
; CHECK-NEXT:  T:
; CHECK-NEXT:    [[X:%.*]] = add i32 0, 0
; CHECK-NEXT:    call void @use(i1 [[C:%.*]])
; CHECK-NEXT:    ret void
;
  br label %T
T:
  %x = add i32 0, 0
  br label %F
F:
  call void @use( i1 %c )
  ret void
}
