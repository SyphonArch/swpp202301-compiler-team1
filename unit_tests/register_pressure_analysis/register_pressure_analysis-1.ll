; RUN: opt < %s -load-pass-plugin=./build/libRegisterPressurePrinterPass.so -passes=register-pressure-printer-pass -S -disable-output 2>&1 | FileCheck %s

; bar has maximum 2 live registers at any point in the function
; except the read-only registers (we don't count argument registers when calculating register pressure)
; CHECK: Approximate Register Pressure of @bar: 2
define i32 @bar(i32 noundef %a, i32 noundef %b, i32 noundef %c) {
  %mul1 = mul i32 %b, %c
  %add1 = add i32 %a, %mul1
  %mul2 = mul i32 %b, %c
  %add2 = add i32 %add1, %mul2
  ret i32 %add2
}