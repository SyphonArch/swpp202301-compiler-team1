; RUN: opt < %s -load-pass-plugin=./build/libOraclePass.so -passes=oracle-pass -S | FileCheck %s

; Check the case when the stores are interleaved with load instructions.

; In this case, outlining to oracle should happen, as we know
; the pointer for load instruction(%arrayidx2 = %arrayidx + 4) is different
; from the pointer for store instruction (%arrayidx), which is no alias.

define void @no_alias_pointers(i32* noundef %ptr_a) {
; CHECK-LABEL: @no_alias_pointers(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[ARRAYIDX:%.*]] = getelementptr inbounds i32, i32* [[PTR_A:%.*]], i32 0
; CHECK-NEXT:    [[A:%.*]] = load i32, i32* [[ARRAYIDX]], align 4
; CHECK-NEXT:    [[ARRAYIDX2:%.*]] = getelementptr inbounds i32, i32* [[PTR_A]], i32 1
; CHECK-NEXT:    [[B:%.*]] = load i32, i32* [[ARRAYIDX2]], align 4
; CHECK-NEXT:    [[TMP0:%.*]] = call i64 @oracle(i32 [[B]], i32* [[ARRAYIDX2]], i32 [[A]], i32* [[ARRAYIDX]])
; CHECK-NEXT:    ret void
;
entry:
  %arrayidx = getelementptr inbounds i32, i32* %ptr_a, i32 0
  %a = load i32, i32* %arrayidx, align 4
  store i32 %a, i32* %arrayidx, align 4
  %arrayidx2 = getelementptr inbounds i32, i32* %ptr_a, i32 1
  %b = load i32, i32* %arrayidx2, align 4
  store i32 %b, i32* %arrayidx2, align 4
  ret void
}

