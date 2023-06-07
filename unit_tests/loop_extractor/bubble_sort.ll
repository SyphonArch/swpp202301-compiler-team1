; ModuleID = '/tmp/a.ll'
source_filename = "bubble_sort/src/bubble_sort.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK: get_inputs
; Function Attrs: nounwind uwtable
define dso_local i64* @get_inputs(i64 noundef %n) #0 {
entry:
  %cmp = icmp eq i64 %n, 0
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  br label %return

if.end:                                           ; preds = %entry
  %mul = mul i64 %n, 8
  %call = call noalias i8* @malloc(i64 noundef %mul) #4
  %0 = bitcast i8* %call to i64*
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %if.end
  %i.0 = phi i64 [ 0, %if.end ], [ %inc, %for.inc ]
  %cmp1 = icmp ult i64 %i.0, %n
  br i1 %cmp1, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond
  br label %for.end

for.body:                                         ; preds = %for.cond
  %call2 = call i64 (...) @read()
  %arrayidx = getelementptr inbounds i64, i64* %0, i64 %i.0
  store i64 %call2, i64* %arrayidx, align 8
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %inc = add i64 %i.0, 1
  br label %for.cond, !llvm.loop !5

for.end:                                          ; preds = %for.cond.cleanup
  br label %return

return:                                           ; preds = %for.end, %if.then
  %retval.0 = phi i64* [ null, %if.then ], [ %0, %for.end ]
  ret i64* %retval.0
}

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: nounwind allocsize(0)
declare noalias i8* @malloc(i64 noundef) #2

declare i64 @read(...) #3

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: nounwind uwtable
define dso_local void @sort(i64 noundef %n, i64* noundef %ptr) #0 {
entry:
  %cmp = icmp eq i64 %n, 0
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  br label %for.end15

if.end:                                           ; preds = %entry
  br label %for.cond

for.cond:                                         ; preds = %for.inc14, %if.end
  %i.0 = phi i64 [ 0, %if.end ], [ %inc, %for.inc14 ]
  %cmp1 = icmp ult i64 %i.0, %n
  br i1 %cmp1, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond
  br label %for.end15

for.body:                                         ; preds = %for.cond
  %sub = sub i64 %n, 1
  br label %for.cond2

for.cond2:                                        ; preds = %for.inc, %for.body
  %j.0 = phi i64 [ %sub, %for.body ], [ %dec, %for.inc ]
  %cmp3 = icmp ugt i64 %j.0, %i.0
  br i1 %cmp3, label %for.body5, label %for.cond.cleanup4

for.cond.cleanup4:                                ; preds = %for.cond2
  br label %for.end

for.body5:                                        ; preds = %for.cond2
  %arrayidx = getelementptr inbounds i64, i64* %ptr, i64 %j.0
  %0 = load i64, i64* %arrayidx, align 8
  %sub6 = sub i64 %j.0, 1
  %arrayidx7 = getelementptr inbounds i64, i64* %ptr, i64 %sub6
  %1 = load i64, i64* %arrayidx7, align 8
  %cmp8 = icmp ult i64 %0, %1
  br i1 %cmp8, label %if.then9, label %if.end13

if.then9:                                         ; preds = %for.body5
  %arrayidx10 = getelementptr inbounds i64, i64* %ptr, i64 %j.0
  store i64 %1, i64* %arrayidx10, align 8
  %sub11 = sub i64 %j.0, 1
  %arrayidx12 = getelementptr inbounds i64, i64* %ptr, i64 %sub11
  store i64 %0, i64* %arrayidx12, align 8
  br label %if.end13

if.end13:                                         ; preds = %if.then9, %for.body5
  br label %for.inc

for.inc:                                          ; preds = %if.end13
  %dec = add i64 %j.0, -1
  br label %for.cond2, !llvm.loop !8

for.end:                                          ; preds = %for.cond.cleanup4
  br label %for.inc14

for.inc14:                                        ; preds = %for.end
  %inc = add i64 %i.0, 1
  br label %for.cond, !llvm.loop !9

for.end15:                                        ; preds = %for.cond.cleanup, %if.then
  ret void
}

; Function Attrs: nounwind uwtable
define dso_local void @put_inputs(i64 noundef %n, i64* noundef %ptr) #0 {
entry:
  %cmp = icmp eq i64 %n, 0
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  br label %for.end

if.end:                                           ; preds = %entry
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %if.end
  %i.0 = phi i64 [ 0, %if.end ], [ %inc, %for.inc ]
  %cmp1 = icmp ult i64 %i.0, %n
  br i1 %cmp1, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond
  br label %for.end

for.body:                                         ; preds = %for.cond
  %arrayidx = getelementptr inbounds i64, i64* %ptr, i64 %i.0
  %0 = load i64, i64* %arrayidx, align 8
  call void @write(i64 noundef %0)
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %inc = add i64 %i.0, 1
  br label %for.cond, !llvm.loop !10

for.end:                                          ; preds = %for.cond.cleanup, %if.then
  ret void
}

declare void @write(i64 noundef) #3

; Function Attrs: nounwind uwtable
define dso_local i32 @main() #0 {
entry:
  %call = call i64 (...) @read()
  %cmp = icmp eq i64 %call, 0
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  br label %cleanup

if.end:                                           ; preds = %entry
  %call1 = call i64* @get_inputs(i64 noundef %call)
  call void @sort(i64 noundef %call, i64* noundef %call1)
  call void @put_inputs(i64 noundef %call, i64* noundef %call1)
  br label %cleanup

cleanup:                                          ; preds = %if.end, %if.then
  ret i32 0
}

attributes #0 = { nounwind uwtable "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { argmemonly nocallback nofree nosync nounwind willreturn }
attributes #2 = { nounwind allocsize(0) "frame-pointer"="none" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { "frame-pointer"="none" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #4 = { nounwind allocsize(0) }

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
!10 = distinct !{!10, !6, !7}
