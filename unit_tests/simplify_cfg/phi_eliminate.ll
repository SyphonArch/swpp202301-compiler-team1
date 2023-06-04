; RUN: opt < %s -load-pass-plugin=./build/libSimplifyCFG.so -passes=simplify-cfg -S | FileCheck %s

; Check if SimplifyCFGPass merges empty basic blocks
; and replaces the phi node to select instruction.

declare void @use(i1)

define void @test(i1 %c) {
; CHECK-LABEL: @test(
; CHECK-NEXT:  F:
; CHECK-NEXT:    [[SPEC_SELECT:%.*]] = select i1 [[C:%.*]], i1 false, i1 true
; CHECK-NEXT:    call void @use(i1 [[SPEC_SELECT]])
; CHECK-NEXT:    ret void
;
  br i1 %c, label %T, label %F
T:
  br label %F
F:
  %B1 = phi i1 [ true, %0 ], [ false, %T ]
  call void @use( i1 %B1 )
  ret void
}
