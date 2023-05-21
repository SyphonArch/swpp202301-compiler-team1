; RUN: opt < %s -load-pass-plugin=./build/libRegisterPressurePrinterPass.so -passes=register-pressure-printer-pass -S -disable-output 2>&1 | FileCheck %s

; The register pressure is caculated in this case as:
; exit: add1, add2 (2)
; block1: add2, x1, y1 (3)
; block2: x2, z2, a, b (4)
; entry: cond, a, b (3)
; So the maximum is calculated as 4.
; Note that this might not be correct; the register pressure is calculated with heuristics.

; CHECK: Approximate Register Pressure of @main: 4
define i32 @main() {
entry:
  %a = alloca i32
  %b = alloca i32
  %c = alloca i32
  store i32 10, i32* %a
  store i32 20, i32* %b
  store i32 30, i32* %c

  %cond = icmp eq i32 1, 1
  br i1 %cond, label %block1, label %block2

block1:
  %x1 = load i32, i32* %a
  %y1 = load i32, i32* %b
  %add1 = add i32 %x1, %y1
  br label %exit

block2:
  %x2 = load i32, i32* %a
  %z2 = load i32, i32* %c
  %add2 = add i32 %x2, %z2
  br label %exit

exit:
  %result = phi i32 [ %add1, %block1 ], [ %add2, %block2 ]
  ret i32 %result
}
