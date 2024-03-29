; RUN: opt < %s -load-pass-plugin=./build/libSimplifyCFG.so -passes=simplify-cfg -S | FileCheck %s

; Check if SimplifyCFGPass hoists common instructions.

define void @foo(i1 %C, ptr %P) {
; CHECK-LABEL: @foo(
; CHECK-NEXT:  common.ret:
; CHECK-NEXT:    store i32 7, ptr [[P:%.*]], align 4
; CHECK-NEXT:    ret void
;
  br i1 %C, label %T, label %F
T:              ; preds = %0
  store i32 7, ptr %P
  ret void
F:              ; preds = %0
  store i32 7, ptr %P
  ret void
}
