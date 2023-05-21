; Test `sub` instruction handling

define i64 @add_sub_pairs(i64 noundef %a, i64 noundef %b, i64 noundef %c, i64 noundef %d, i64 noundef %e, i64 noundef %f, i64 noundef %g, i64 noundef %h) {
;CHECK-LABEL: @add_sub_pairs(i64 noundef %a, i64 noundef %b, i64 noundef %c, i64 noundef %d, i64 noundef %e, i64 noundef %f, i64 noundef %g, i64 noundef %h)
;CHECK-NEXT:  %neg.g = mul i64 %g, -1
;CHECK-NEXT:  %neg.f = mul i64 %f, -1
;CHECK-NEXT:  %sum7 = call i64 @int_sum_i64(i64 %a, i64 %b, i64 %c, i64 %d, i64 %e, i64 %neg.f, i64 %neg.g, i64 %h)
;CHECK-NEXT:  ret i64 %sum7

  %sum1 = add i64 %a, %b
  %sum2 = add i64 %c, %d
  %sum3 = sub i64 %e, %f
  %sum4 = sub i64 %g, %h
  %sum5 = add i64 %sum1, %sum2
  %sum6 = sub i64 %sum3, %sum4
  %sum7 = add i64 %sum5, %sum6
  ret i64 %sum7
}

define i64 @constant_sub(i64 noundef %a, i64 noundef %b) {
;CHECK-LABEL: @constant_sub(i64 noundef %a, i64 noundef %b)
;CHECK-NEXT:  %g = call i64 @int_sum_i64(i64 %b, i64 -10, i64 %a, i64 0, i64 0, i64 0, i64 0, i64 0)
;CHECK-NEXT:  ret i64 %g
  %c = sub i64 %a, 1
  %d = sub i64 %c, 2
  %e = sub i64 %d, 3
  %f = sub i64 %e, 4
  %g = add i64 %f, %b
  ret i64 %g

}

define i64 @sub_test(i64 noundef %a, i64 noundef %b, i64 noundef %c) {
;CHECK-LABEL: @sub_test(i64 noundef %a, i64 noundef %b, i64 noundef %c)
;CHECK-NEXT:  %neg.b = mul i64 %b, -1
;CHECK-NEXT:  %neg.c = mul i64 %c, -1
;CHECK-NEXT:  %z = call i64 @int_sum_i64(i64 3, i64 %neg.c, i64 %a, i64 %neg.b, i64 0, i64 0, i64 0, i64 0)
;CHECK-NEXT:  ret i64 %z

  %x = sub i64 %a, %b
  %y = sub i64 %x, %c
  %z = add i64 %y, 3
  ret i64 %z
}

declare i64 @int_sum_i64(i64, i64, i64, i64, i64, i64, i64, i64)
declare i32 @int_sum_i32(i32, i32, i32, i32, i32, i32, i32, i32)
