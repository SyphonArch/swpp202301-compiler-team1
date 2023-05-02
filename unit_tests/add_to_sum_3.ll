; Check when more than 8 elements are being added together
; Should condense into two sums.

define i64 @add_ten(i64 %a, i64 %b, i64 %c, i64 %d, i64 %e, i64 %f, i64 %g, i64 %h, i64 %i, i64 %j) {
;CHECK-LABEL: @add_ten(i64 %a, i64 %b, i64 %c, i64 %d, i64 %e, i64 %f, i64 %g, i64 %h, i64 %i, i64 %j)
;CHECK-NEXT:  %sum7 = call i64 @int_sum_i64(i64 %h, i64 %g, i64 %f, i64 %e, i64 %d, i64 %c, i64 %a, i64 %b)
;CHECK-NEXT:  %sum9 = call i64 @int_sum_i64(i64 %j, i64 %i, i64 %sum7, i64 0, i64 0, i64 0, i64 0, i64 0)
;CHECK-NEXT:  ret i64 %sum9
  %sum1 = add i64 %a, %b
  %sum2 = add i64 %sum1, %c
  %sum3 = add i64 %sum2, %d
  %sum4 = add i64 %sum3, %e
  %sum5 = add i64 %sum4, %f
  %sum6 = add i64 %sum5, %g
  %sum7 = add i64 %sum6, %h
  %sum8 = add i64 %sum7, %i
  %sum9 = add i64 %sum8, %j

  ret i64 %sum9
}

; Should only use one sum, followed by an add.
define i64 @add_nine(i64 %a, i64 %b, i64 %c, i64 %d, i64 %e, i64 %f, i64 %g, i64 %h, i64 %i) {
;CHECK-LABEL: @add_nine(i64 %a, i64 %b, i64 %c, i64 %d, i64 %e, i64 %f, i64 %g, i64 %h, i64 %i)
;CHECK-NEXT:  %sum7 = call i64 @int_sum_i64(i64 %h, i64 %g, i64 %f, i64 %e, i64 %d, i64 %c, i64 %a, i64 %b)
;CHECK-NEXT:  %sum8 = add i64 %sum7, %i
;CHECK-NEXT:  ret i64 %sum8
  %sum1 = add i64 %a, %b
  %sum2 = add i64 %sum1, %c
  %sum3 = add i64 %sum2, %d
  %sum4 = add i64 %sum3, %e
  %sum5 = add i64 %sum4, %f
  %sum6 = add i64 %sum5, %g
  %sum7 = add i64 %sum6, %h
  %sum8 = add i64 %sum7, %i

  ret i64 %sum8
}

declare i64 @int_sum_i64(i64, i64, i64, i64, i64, i64, i64, i64)
declare i32 @int_sum_i32(i32, i32, i32, i32, i32, i32, i32, i32)
