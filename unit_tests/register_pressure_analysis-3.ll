; RUN: opt < %s -load-pass-plugin=./build/libRegisterPressurePrinterPass.so -passes=register-pressure-printer-pass -S -disable-output 2>&1 | FileCheck %s

; One valid post-order visit sequence: while.end -> while.cond -> while.body -> entry
; In this case, the register pressure is calculated as 2 (in while.cond, there is %i.0 and %cmp)
; The other possible sequence can be: while.body -> while.end-> while.cond -> entry
; In this case, the register pressure is calculated as 2 
; (same as the previous. in while.cond, there is %i.0 and %cmp)

; CHECK: Approximate Register Pressure of @f: 2
define i32 @f() {
entry:
  br label %while.cond

while.cond:                                       ; preds = %while.body, %entry
  %i.0 = phi i32 [ 0, %entry ], [ %add, %while.body ]
  %cmp = icmp slt i32 %i.0, 10
  br i1 %cmp, label %while.body, label %while.end

while.body:                                       ; preds = %while.cond
  %add = add nsw i32 %i.0, 1
  br label %while.cond

while.end:                                        ; preds = %while.cond
  ret i32 %i.0
}
