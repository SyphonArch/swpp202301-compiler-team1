; Add the 8 arguments together using `add` and return the sum.
; Should be replaced by a single `sum` instruction.

define i64 @add_eight(i64 %a, i64 %b, i64 %c, i64 %d, i64 %e, i64 %f, i64 %g, i64 %h) {
;CHECK-LABEL: @add_eight(i64 %a, i64 %b, i64 %c, i64 %d, i64 %e, i64 %f, i64 %g, i64 %h)
;CHECK-NEXT:   [[SUM:%.*]] = call i64 @int_sum_i64(i64 %h, i64 %g, i64 %f, i64 %e, i64 %d, i64 %c, i64 %a, i64 %b)
;CHECK-NEXT:   ret i64 [[SUM]]
  %sum1 = add i64 %a, %b
  %sum2 = add i64 %sum1, %c
  %sum3 = add i64 %sum2, %d
  %sum4 = add i64 %sum3, %e
  %sum5 = add i64 %sum4, %f
  %sum6 = add i64 %sum5, %g
  %sum7 = add i64 %sum6, %h
  ret i64 %sum7
}

; Same as above, except with i32 type.
; Should be replaced by a single `sum` instruction

define i32 @add_eight_i32(i32 %a, i32 %b, i32 %c, i32 %d, i32 %e, i32 %f, i32 %g, i32 %h) {
;CHECK-LABEL: @add_eight_i32(i32 %a, i32 %b, i32 %c, i32 %d, i32 %e, i32 %f, i32 %g, i32 %h)
;CHECK-NEXT:   [[SUM:%.*]] = call i32 @int_sum_i32(i32 %h, i32 %g, i32 %f, i32 %e, i32 %d, i32 %c, i32 %a, i32 %b)
;CHECK-NEXT:   ret i32 [[SUM]]
  %sum1 = add i32 %a, %b
  %sum2 = add i32 %sum1, %c
  %sum3 = add i32 %sum2, %d
  %sum4 = add i32 %sum3, %e
  %sum5 = add i32 %sum4, %f
  %sum6 = add i32 %sum5, %g
  %sum7 = add i32 %sum6, %h
  ret i32 %sum7
}

; Now instead of accumulating by repeatedly adding to a new `sum` variable,
; this function does a pairwise addition reduction, like a binary tree.
; Again, should be replaced by a single `sum` instruction

define i64 @add_eight_pairs(i64 %a, i64 %b, i64 %c, i64 %d, i64 %e, i64 %f, i64 %g, i64 %h) {
;CHECK-LABEL: @add_eight_pairs(i64 %a, i64 %b, i64 %c, i64 %d, i64 %e, i64 %f, i64 %g, i64 %h)
;CHECK-NEXT:   [[SUM:%.*]] = call i64 @int_sum_i64(i64 %a, i64 %b, i64 %c, i64 %d, i64 %e, i64 %f, i64 %g, i64 %h)
;CHECK-NEXT:   ret i64 [[SUM]]

  %sum1 = add i64 %a, %b
  %sum2 = add i64 %c, %d
  %sum3 = add i64 %e, %f
  %sum4 = add i64 %g, %h
  %sum5 = add i64 %sum1, %sum2
  %sum6 = add i64 %sum3, %sum4
  %sum7 = add i64 %sum5, %sum6
  ret i64 %sum7
}

declare i64 @int_sum_i64(i64, i64, i64, i64, i64, i64, i64, i64)
declare i32 @int_sum_i32(i32, i32, i32, i32, i32, i32, i32, i32)
