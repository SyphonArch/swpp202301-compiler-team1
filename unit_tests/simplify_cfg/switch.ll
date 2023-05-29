; RUN: opt < %s -load-pass-plugin=./build/libSimplifyCFG.so -passes=simplify-cfg -S | FileCheck %s

declare void @func2(i32)
declare void @func4(i32)
declare void @func8(i32)

; Check if simplifyCFGPass converts if-else tree to switch

define void @test1(i32 %N) nounwind uwtable {
; CHECK-LABEL: @test1(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    switch i32 [[N:%.*]], label [[IF_ELSE8:%.*]] [
; CHECK-NEXT:    i32 2, label [[IF_THEN:%.*]]
; CHECK-NEXT:    i32 4, label [[IF_THEN7:%.*]]
; CHECK-NEXT:    ]
; CHECK:       if.then:
; CHECK-NEXT:    call void @func2(i32 [[N]]) #[[ATTR1:[0-9]+]]
; CHECK-NEXT:    br label [[EXIT:%.*]]
; CHECK:       if.then7:
; CHECK-NEXT:    call void @func4(i32 [[N]]) #[[ATTR1]]
; CHECK-NEXT:    br label [[EXIT]]
; CHECK:       if.else8:
; CHECK-NEXT:    call void @func8(i32 [[N]]) #[[ATTR1]]
; CHECK-NEXT:    br label [[EXIT]]
; CHECK:       exit:
; CHECK-NEXT:    ret void
;
entry:
  %cmp = icmp eq i32 %N, 2
  br i1 %cmp, label %if.then, label %if.else

if.then:
  call void @func2(i32 %N) nounwind
  br label %exit

if.else:
  %cmp2 = icmp eq i32 %N, 4
  br i1 %cmp2, label %if.then4, label %if.else8

if.then4:
  call void @func4(i32 %N) nounwind
  br label %exit

if.else8:
  call void @func8(i32 %N) nounwind
  br label %exit

exit:
  ret void
}
