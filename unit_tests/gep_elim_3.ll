; CHECK-LABEL: define i32 @get_element_ptr_example(i32* %arr_32, i64 %idx_32, i16* %arr_16, i64 %idx_16, i8* %arr_8, i64 %idx_8)
; CHECK-NEXT:   [[A1:%.*]] = ptrtoint i32* [[arr_32:%.*]] to i64
; CHECK-NEXT:   [[A2:%.*]] = mul i64 [[idx_32:%.*]], 4
; CHECK-NEXT:   [[A3:%.*]] = add i64 [[A1]], [[A2]]
; CHECK-NEXT:   [[A4:%.*]] = inttoptr i64 [[A3]] to i32*
; CHECK-NEXT:   [[val_32:%.*]] = load i32, i32* [[A4]], align 4
; CHECK-NEXT:   [[A5:%.*]] = ptrtoint i16* [[arr_16:%.*]] to i64
; CHECK-NEXT:   [[A6:%.*]] = mul i64 [[idx_16:%.*]], 2
; CHECK-NEXT:   [[A7:%.*]] = add i64 [[A5]], [[A6]]
; CHECK-NEXT:   [[A8:%.*]] = inttoptr i64 [[A7]] to i16*
; CHECK-NEXT:   [[val_16:%.*]] = load i16, i16* [[A8]], align 2
; CHECK-NEXT:   [[A9:%.*]] = ptrtoint i8* [[arr_8:%.*]] to i64
; CHECK-NEXT:   [[A10:%.*]] = mul i64 [[idx_8:%.*]], 1
; CHECK-NEXT:   [[A11:%.*]] = add i64 [[A9]], [[A10]]
; CHECK-NEXT:   [[A12:%.*]] = inttoptr i64 [[A11]] to i8*
; CHECK-NEXT:   [[val_8:%.*]] = load i8, i8* [[A12]], align 1
; CHECK-NEXT:   ret i32 [[val_32]]




define i32 @get_element_ptr_example(i32* %arr_32, i64 %idx_32, i16* %arr_16, i64 %idx_16, i8* %arr_8, i64 %idx_8) {
  %element_ptr_32 = getelementptr i32, i32* %arr_32, i64 %idx_32
  %val_32 = load i32, i32* %element_ptr_32
  %element_ptr_16 = getelementptr i16, i16* %arr_16, i64 %idx_16
  %val_16 = load i16, i16* %element_ptr_16
  %element_ptr_8 = getelementptr i8, i8* %arr_8, i64 %idx_8
  %val_8 = load i8, i8* %element_ptr_8
  ret i32 %val_32
}