; Check that `mul` instructions are expanded when possible

define i64 @mul_test(i64 %a, i64 %b, i64 %c, i64 %d) {
;CHECK-LABEL: @mul_test(i64 %a, i64 %b, i64 %c, i64 %d)
;CHECK-NEXT:  %sum3 = call i64 @int_sum_i64(i64 %c, i64 %a, i64 %b, i64 %d, i64 %d, i64 %d, i64 %d, i64 0)
;CHECK-NEXT:  ret i64 %sum3

  %sum1 = add i64 %a, %b
  %sum2 = add i64 %sum1, %c
  %dddd = mul i64 %d, 4
  %sum3 = add i64 %sum2, %dddd

  ret i64 %sum3
}

declare i64 @int_sum_i64(i64, i64, i64, i64, i64, i64, i64, i64)
declare i32 @int_sum_i32(i32, i32, i32, i32, i32, i32, i32, i32)
