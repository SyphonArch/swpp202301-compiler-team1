; Add the 8 arguments together using `add` and return the sum.
; Except, `sum3` is used again at the end.
; Therefore, we cannot convert everything to a single sum.
; `sum3` is therefore left out.

define i64 @add_eight(i64 noundef %a, i64 noundef %b, i64 noundef %c, i64 noundef %d, i64 noundef %e, i64 noundef %f, i64 noundef %g, i64 noundef %h) {
;CHECK-LABEL: @add_eight(i64 noundef %a, i64 noundef %b, i64 noundef %c, i64 noundef %d, i64 noundef %e, i64 noundef %f, i64 noundef %g, i64 noundef %h)
;CHECK-NEXT:  [[SUM3:%.*]] = call i64 @int_sum_i64(i64 %d, i64 %c, i64 %a, i64 %b, i64 0, i64 0, i64 0, i64 0)
;CHECK-NEXT:  [[SUM8:%.*]] = call i64 @int_sum_i64(i64 %h, i64 %g, i64 %f, i64 %e, i64 [[SUM3]], i64 [[SUM3]], i64 0, i64 0)
;CHECK-NEXT:  ret i64 [[SUM8]]

  %sum1 = add i64 %a, %b
  %sum2 = add i64 %sum1, %c
  %sum3 = add i64 %sum2, %d
  %sum4 = add i64 %sum3, %e
  %sum5 = add i64 %sum4, %f
  %sum6 = add i64 %sum5, %g
  %sum7 = add i64 %sum6, %h

  %sum8 = add i64 %sum7, %sum3
  ret i64 %sum8
}

declare i64 @int_sum_i64(i64, i64, i64, i64, i64, i64, i64, i64)
declare i32 @int_sum_i32(i32, i32, i32, i32, i32, i32, i32, i32)
