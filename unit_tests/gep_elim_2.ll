; CHECK-LABEL: define i64 @sum_array_unrolled(i64* %a, i64 %n, i64 %m)
; CHECK: entry:
; CHECK-NEXT:   [[sum:%.*]] = alloca i64, align 8
; CHECK-NEXT:   store i64 0, i64* [[sum]], align 4
; CHECK-NEXT:   [[idx1:%.*]] = add i64 0, [[m:%.*]]
; CHECK-NEXT:   [[idx2:%.*]] = add i64 [[idx1]], 1
; CHECK-NEXT:   [[idx3:%.*]] = add i64 [[idx2]], 2
; CHECK-NEXT:   [[idx4:%.*]] = add i64 [[idx3]], 3
; CHECK-NEXT:   [[A0:%.*]] = ptrtoint i64* [[a:%.*]] to i64
; CHECK-NEXT:   [[A1:%.*]] = mul i64 [[idx1]], 8
; CHECK-NEXT:   [[A2:%.*]] = add i64 [[A0]], [[A1]]
; CHECK-NEXT:   [[A3:%.*]] = inttoptr i64 [[A2]] to i64*
; CHECK-NEXT:   [[A4:%.*]] = ptrtoint i64* [[a]] to i64
; CHECK-NEXT:   [[A5:%.*]] = mul i64 [[idx2]], 8
; CHECK-NEXT:   [[A6:%.*]] = add i64 [[A4]], [[A5]]
; CHECK-NEXT:   [[A7:%.*]] = inttoptr i64 [[A6]] to i64*
; CHECK-NEXT:   [[A8:%.*]] = ptrtoint i64* [[a]] to i64
; CHECK-NEXT:   [[A9:%.*]] = mul i64 [[idx3]], 8
; CHECK-NEXT:   [[A10:%.*]] = add i64 [[A8]], [[A9]]
; CHECK-NEXT:   [[A11:%.*]] = inttoptr i64 [[A10]] to i64*
; CHECK-NEXT:   [[A12:%.*]] = ptrtoint i64* [[a]] to i64
; CHECK-NEXT:   [[A13:%.*]] = mul i64 [[idx4]], 8
; CHECK-NEXT:   [[A14:%.*]] = add i64 [[A12]], [[A13]]
; CHECK-NEXT:   [[A15:%.*]] = inttoptr i64 [[A14]] to i64*
; CHECK-NEXT:   [[val1:%.*]] = load i64, i64* [[A3]], align 4
; CHECK-NEXT:   [[val2:%.*]] = load i64, i64* [[A7]], align 4
; CHECK-NEXT:   [[val3:%.*]] = load i64, i64* [[A11]], align 4
; CHECK-NEXT:   [[val4:%.*]] = load i64, i64* [[A15]], align 4
; CHECK-NEXT:   [[sum1:%.*]] = add i64 [[val1]], [[val2]]
; CHECK-NEXT:   [[sum2:%.*]] = add i64 [[val3]], [[val4]]
; CHECK-NEXT:   [[sum3:%.*]] = add i64 [[sum1]], [[sum2]]
; CHECK-NEXT:   store i64 [[sum3]], i64* [[sum]], align 4
; CHECK-NEXT:   [[sum_final:%.*]] = load i64, i64* [[sum]], align 4
; CHECK-NEXT:   ret i64 [[sum_final]]

define i64 @sum_array_unrolled(i64* %a, i64 %n, i64 %m) {
entry:
  %sum = alloca i64
  store i64 0, i64* %sum
  %idx1 = add i64 0, %m
  %idx2 = add i64 %idx1, 1
  %idx3 = add i64 %idx2, 2
  %idx4 = add i64 %idx3, 3
  %elem1 = getelementptr inbounds i64, i64* %a, i64 %idx1
  %elem2 = getelementptr inbounds i64, i64* %a, i64 %idx2
  %elem3 = getelementptr inbounds i64, i64* %a, i64 %idx3
  %elem4 = getelementptr inbounds i64, i64* %a, i64 %idx4
  %val1 = load i64, i64* %elem1
  %val2 = load i64, i64* %elem2
  %val3 = load i64, i64* %elem3
  %val4 = load i64, i64* %elem4
  %sum1 = add i64 %val1, %val2
  %sum2 = add i64 %val3, %val4
  %sum3 = add i64 %sum1, %sum2
  store i64 %sum3, i64* %sum
  %sum_final = load i64, i64* %sum
  ret i64 %sum_final
}