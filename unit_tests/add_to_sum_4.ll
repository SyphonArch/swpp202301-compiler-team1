; Check that non-add instructions are handled correctly

define i64 @add_div_eight(i64 noundef %a, i64 noundef %b, i64 noundef %c, i64 noundef %d, i64 noundef %e, i64 noundef %f, i64 noundef %g, i64 noundef %h) {
;CHECK-LABEL: @add_div_eight(i64 noundef %a, i64 noundef %b, i64 noundef %c, i64 noundef %d, i64 noundef %e, i64 noundef %f, i64 noundef %g, i64 noundef %h)
;CHECK-NEXT:  %sum2 = call i64 @int_sum_i64(i64 %c, i64 %a, i64 %b, i64 0, i64 0, i64 0, i64 0, i64 0)
;CHECK-NEXT:  %sum3 = sdiv i64 %sum2, %d
;CHECK-NEXT:  %sum7 = call i64 @int_sum_i64(i64 %h, i64 %g, i64 %f, i64 %sum3, i64 %e, i64 0, i64 0, i64 0)
;CHECK-NEXT:  ret i64 %sum7

  %sum1 = add i64 %a, %b
  %sum2 = add i64 %sum1, %c
  %sum3 = sdiv i64 %sum2, %d
  %sum4 = add i64 %sum3, %e
  %sum5 = add i64 %sum4, %f
  %sum6 = add i64 %sum5, %g
  %sum7 = add i64 %sum6, %h

  ret i64 %sum7
}

declare i64 @int_sum_i64(i64, i64, i64, i64, i64, i64, i64, i64)
declare i32 @int_sum_i32(i32, i32, i32, i32, i32, i32, i32, i32)
