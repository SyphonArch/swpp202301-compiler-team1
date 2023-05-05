; This function adds together arguments like below:

; a  b c d e f
;  \/  \/   \/
;   g   h   i
;     \/  \/
;     j    k

; Because `d` is not used once, it is handled by the non-one-use expansion code.

define i64 @add_eight_overlapping(i64 %a, i64 %b, i64 %c, i64 %d, i64 %e, i64 %f) {
;CHECK-LABEL: @add_eight_overlapping(i64 %a, i64 %b, i64 %c, i64 %d, i64 %e, i64 %f)
;CHECK-NEXT:  %j = call i64 @int_sum_i64(i64 %a, i64 %b, i64 %c, i64 %d, i64 0, i64 0, i64 0, i64 0)
;CHECK-NEXT:  %k = call i64 @int_sum_i64(i64 %e, i64 %f, i64 %c, i64 %d, i64 0, i64 0, i64 0, i64 0)
;CHECK-NEXT:  %l = mul i64 %j, %k
;CHECK-NEXT:  ret i64 %l

  %g = add i64 %a, %b
  %h = add i64 %c, %d
  %i = add i64 %e, %f
  %j = add i64 %g, %h
  %k = add i64 %h, %i
  %l = mul i64 %j, %k
  ret i64 %l
}

declare i64 @int_sum_i64(i64, i64, i64, i64, i64, i64, i64, i64)
declare i32 @int_sum_i32(i32, i32, i32, i32, i32, i32, i32, i32)
