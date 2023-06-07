; ModuleID = '/tmp/a.ll'
source_filename = "jenkins_hash/src/jenkins_hash.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK: malloc
; Function Attrs: nounwind uwtable
define dso_local i8* @malloc_upto_8(i64 noundef %x) #0 {
entry:
  %add = add i64 %x, 7
  %div = udiv i64 %add, 8
  %mul = mul i64 %div, 8
  %call = call noalias i8* @malloc(i64 noundef %mul) #5
  ret i8* %call
}

; Function Attrs: nounwind allocsize(0)
declare noalias i8* @malloc(i64 noundef) #1

; Function Attrs: nounwind uwtable
define dso_local i32 @jenkins_one_at_a_time_hash(i32 noundef %value, i8* noundef %key, i64 noundef %length) #0 {
entry:
  br label %while.cond

while.cond:                                       ; preds = %while.body, %entry
  %i.0 = phi i64 [ 0, %entry ], [ %inc, %while.body ]
  %hash.0 = phi i32 [ %value, %entry ], [ %xor, %while.body ]
  %cmp = icmp ne i64 %i.0, %length
  br i1 %cmp, label %while.body, label %while.end

while.body:                                       ; preds = %while.cond
  %inc = add i64 %i.0, 1
  %arrayidx = getelementptr inbounds i8, i8* %key, i64 %i.0
  %0 = load i8, i8* %arrayidx, align 1
  %conv = zext i8 %0 to i32
  %add = add i32 %hash.0, %conv
  %shl = shl i32 %add, 10
  %add1 = add i32 %add, %shl
  %shr = lshr i32 %add1, 6
  %xor = xor i32 %add1, %shr
  br label %while.cond, !llvm.loop !5

while.end:                                        ; preds = %while.cond
  %shl2 = shl i32 %hash.0, 3
  %add3 = add i32 %hash.0, %shl2
  %shr4 = lshr i32 %add3, 11
  %xor5 = xor i32 %add3, %shr4
  %shl6 = shl i32 %xor5, 15
  %add7 = add i32 %xor5, %shl6
  ret i32 %add7
}

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #2

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #2

; Function Attrs: nounwind uwtable
define dso_local i32 @main() #0 {
entry:
  %call = call i64 (...) @read()
  %cmp = icmp eq i64 %call, 0
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  call void @write(i64 noundef 0)
  br label %cleanup

if.end:                                           ; preds = %entry
  %call1 = call i8* @malloc_upto_8(i64 noundef %call)
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %if.end
  %i.0 = phi i32 [ 0, %if.end ], [ %inc, %for.inc ]
  %conv = sext i32 %i.0 to i64
  %cmp2 = icmp ult i64 %conv, %call
  br i1 %cmp2, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond
  br label %for.end

for.body:                                         ; preds = %for.cond
  %call4 = call i64 (...) @read()
  %conv5 = trunc i64 %call4 to i8
  %idxprom = sext i32 %i.0 to i64
  %arrayidx = getelementptr inbounds i8, i8* %call1, i64 %idxprom
  store i8 %conv5, i8* %arrayidx, align 1
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %inc = add nsw i32 %i.0, 1
  br label %for.cond, !llvm.loop !8

for.end:                                          ; preds = %for.cond.cleanup
  br label %for.cond7

for.cond7:                                        ; preds = %for.inc13, %for.end
  %value.0 = phi i32 [ 0, %for.end ], [ %call12, %for.inc13 ]
  %i6.0 = phi i32 [ 0, %for.end ], [ %inc14, %for.inc13 ]
  %cmp8 = icmp slt i32 %i6.0, 10
  br i1 %cmp8, label %for.body11, label %for.cond.cleanup10

for.cond.cleanup10:                               ; preds = %for.cond7
  br label %for.end15

for.body11:                                       ; preds = %for.cond7
  %call12 = call i32 @jenkins_one_at_a_time_hash(i32 noundef %value.0, i8* noundef %call1, i64 noundef %call)
  br label %for.inc13

for.inc13:                                        ; preds = %for.body11
  %inc14 = add nsw i32 %i6.0, 1
  br label %for.cond7, !llvm.loop !9

for.end15:                                        ; preds = %for.cond.cleanup10
  %conv16 = zext i32 %value.0 to i64
  call void @write(i64 noundef %conv16)
  call void @free(i8* noundef %call1) #6
  br label %cleanup

cleanup:                                          ; preds = %for.end15, %if.then
  ret i32 0
}

declare i64 @read(...) #3

declare void @write(i64 noundef) #3

; Function Attrs: nounwind
declare void @free(i8* noundef) #4

attributes #0 = { nounwind uwtable "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { nounwind allocsize(0) "frame-pointer"="none" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #2 = { argmemonly nocallback nofree nosync nounwind willreturn }
attributes #3 = { "frame-pointer"="none" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #4 = { nounwind "frame-pointer"="none" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #5 = { nounwind allocsize(0) }
attributes #6 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{!"clang version 15.0.7 (https://github.com/llvm/llvm-project.git 8dfdcc7b7bf66834a761bd8de445840ef68e4d1a)"}
!5 = distinct !{!5, !6, !7}
!6 = !{!"llvm.loop.mustprogress"}
!7 = !{!"llvm.loop.unroll.disable"}
!8 = distinct !{!8, !6, !7}
!9 = distinct !{!9, !6, !7}
