; ModuleID = '/tmp/a.ll'
source_filename = "binary_tree/src/binary_tree.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@root = external global i64*, align 8

; CHECK: insert
; Function Attrs: nounwind uwtable
define dso_local i64 @insert(i64 noundef %data) #0 {
entry:
  %0 = load i64*, i64** @root, align 8
  %cmp = icmp eq i64* %0, null
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %call = call noalias i8* @malloc(i64 noundef 24) #5
  %1 = bitcast i8* %call to i64*
  store i64 %data, i64* %1, align 8
  %add.ptr = getelementptr inbounds i64, i64* %1, i64 1
  store i64 0, i64* %add.ptr, align 8
  %add.ptr1 = getelementptr inbounds i64, i64* %1, i64 2
  store i64 0, i64* %add.ptr1, align 8
  store i64* %1, i64** @root, align 8
  br label %return

if.end:                                           ; preds = %entry
  %2 = load i64*, i64** @root, align 8
  br label %while.cond

while.cond:                                       ; preds = %cleanup, %if.end
  %curr.0 = phi i64* [ %2, %if.end ], [ %curr.1, %cleanup ]
  %retval.0 = phi i64 [ 0, %if.end ], [ %retval.1, %cleanup ]
  br label %while.body

while.body:                                       ; preds = %while.cond
  %3 = load i64, i64* %curr.0, align 8
  %cmp2 = icmp ugt i64 %3, %data
  br i1 %cmp2, label %if.then3, label %if.else

if.then3:                                         ; preds = %while.body
  %add.ptr4 = getelementptr inbounds i64, i64* %curr.0, i64 1
  %4 = load i64, i64* %add.ptr4, align 8
  %5 = inttoptr i64 %4 to i64*
  %cmp5 = icmp eq i64* %5, null
  br i1 %cmp5, label %if.then6, label %if.end12

if.then6:                                         ; preds = %if.then3
  %call8 = call noalias i8* @malloc(i64 noundef 24) #5
  %6 = bitcast i8* %call8 to i64*
  store i64 %data, i64* %6, align 8
  %add.ptr9 = getelementptr inbounds i64, i64* %6, i64 1
  store i64 0, i64* %add.ptr9, align 8
  %add.ptr10 = getelementptr inbounds i64, i64* %6, i64 2
  store i64 0, i64* %add.ptr10, align 8
  %7 = ptrtoint i64* %6 to i64
  %add.ptr11 = getelementptr inbounds i64, i64* %curr.0, i64 1
  store i64 %7, i64* %add.ptr11, align 8
  br label %cleanup

if.end12:                                         ; preds = %if.then3
  br label %cleanup, !llvm.loop !5

if.else:                                          ; preds = %while.body
  %cmp13 = icmp ult i64 %3, %data
  br i1 %cmp13, label %if.then14, label %if.else24

if.then14:                                        ; preds = %if.else
  %add.ptr15 = getelementptr inbounds i64, i64* %curr.0, i64 2
  %8 = load i64, i64* %add.ptr15, align 8
  %9 = inttoptr i64 %8 to i64*
  %cmp16 = icmp eq i64* %9, null
  br i1 %cmp16, label %if.then17, label %if.end23

if.then17:                                        ; preds = %if.then14
  %call19 = call noalias i8* @malloc(i64 noundef 24) #5
  %10 = bitcast i8* %call19 to i64*
  store i64 %data, i64* %10, align 8
  %add.ptr20 = getelementptr inbounds i64, i64* %10, i64 1
  store i64 0, i64* %add.ptr20, align 8
  %add.ptr21 = getelementptr inbounds i64, i64* %10, i64 2
  store i64 0, i64* %add.ptr21, align 8
  %11 = ptrtoint i64* %10 to i64
  %add.ptr22 = getelementptr inbounds i64, i64* %curr.0, i64 2
  store i64 %11, i64* %add.ptr22, align 8
  br label %cleanup

if.end23:                                         ; preds = %if.then14
  br label %cleanup, !llvm.loop !5

if.else24:                                        ; preds = %if.else
  br label %cleanup

cleanup:                                          ; preds = %if.else24, %if.end23, %if.then17, %if.end12, %if.then6
  %curr.1 = phi i64* [ %curr.0, %if.then6 ], [ %5, %if.end12 ], [ %curr.0, %if.then17 ], [ %9, %if.end23 ], [ %curr.0, %if.else24 ]
  %cleanup.dest.slot.0 = phi i32 [ 1, %if.then6 ], [ 2, %if.end12 ], [ 1, %if.then17 ], [ 2, %if.end23 ], [ 1, %if.else24 ]
  %retval.1 = phi i64 [ 1, %if.then6 ], [ %retval.0, %if.end12 ], [ 1, %if.then17 ], [ %retval.0, %if.end23 ], [ 0, %if.else24 ]
  switch i32 %cleanup.dest.slot.0, label %cleanup26 [
    i32 2, label %while.cond
  ]

cleanup26:                                        ; preds = %cleanup
  br label %return

return:                                           ; preds = %cleanup26, %if.then
  %retval.2 = phi i64 [ 1, %if.then ], [ %retval.1, %cleanup26 ]
  ret i64 %retval.2
}

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: nounwind allocsize(0)
declare noalias i8* @malloc(i64 noundef) #2

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: nounwind uwtable
define dso_local i64 @adjust(i64* noundef %node) #0 {
entry:
  %add.ptr = getelementptr inbounds i64, i64* %node, i64 1
  %0 = load i64, i64* %add.ptr, align 8
  %1 = inttoptr i64 %0 to i64*
  br label %while.cond

while.cond:                                       ; preds = %cleanup.cont, %entry
  %curr.0 = phi i64* [ %1, %entry ], [ %curr.1, %cleanup.cont ]
  %parent.0 = phi i64* [ %node, %entry ], [ %parent.1, %cleanup.cont ]
  %retval.0 = phi i64 [ 0, %entry ], [ %retval.1, %cleanup.cont ]
  br label %while.body

while.body:                                       ; preds = %while.cond
  %2 = load i64, i64* %curr.0, align 8
  %add.ptr1 = getelementptr inbounds i64, i64* %curr.0, i64 1
  %3 = load i64, i64* %add.ptr1, align 8
  %4 = inttoptr i64 %3 to i64*
  %add.ptr2 = getelementptr inbounds i64, i64* %curr.0, i64 2
  %5 = load i64, i64* %add.ptr2, align 8
  %6 = inttoptr i64 %5 to i64*
  %cmp = icmp eq i64* %6, null
  br i1 %cmp, label %if.then, label %if.end8

if.then:                                          ; preds = %while.body
  %add.ptr3 = getelementptr inbounds i64, i64* %parent.0, i64 1
  %7 = load i64, i64* %add.ptr3, align 8
  %8 = inttoptr i64 %7 to i64*
  %cmp4 = icmp eq i64* %curr.0, %8
  br i1 %cmp4, label %if.then5, label %if.else

if.then5:                                         ; preds = %if.then
  %9 = ptrtoint i64* %4 to i64
  %add.ptr6 = getelementptr inbounds i64, i64* %parent.0, i64 1
  store i64 %9, i64* %add.ptr6, align 8
  br label %if.end

if.else:                                          ; preds = %if.then
  %10 = ptrtoint i64* %6 to i64
  %add.ptr7 = getelementptr inbounds i64, i64* %parent.0, i64 2
  store i64 %10, i64* %add.ptr7, align 8
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then5
  %11 = bitcast i64* %curr.0 to i8*
  call void @free(i8* noundef %11) #6
  br label %cleanup

if.end8:                                          ; preds = %while.body
  br label %cleanup

cleanup:                                          ; preds = %if.end8, %if.end
  %curr.1 = phi i64* [ %curr.0, %if.end ], [ %6, %if.end8 ]
  %parent.1 = phi i64* [ %parent.0, %if.end ], [ %curr.0, %if.end8 ]
  %retval.1 = phi i64 [ %2, %if.end ], [ %retval.0, %if.end8 ]
  %cleanup.dest.slot.0 = phi i32 [ 1, %if.end ], [ 0, %if.end8 ]
  switch i32 %cleanup.dest.slot.0, label %cleanup11 [
    i32 0, label %cleanup.cont
  ]

cleanup.cont:                                     ; preds = %cleanup
  br label %while.cond, !llvm.loop !7

cleanup11:                                        ; preds = %cleanup
  ret i64 %retval.1
}

; Function Attrs: nounwind
declare void @free(i8* noundef) #3

; Function Attrs: nounwind uwtable
define dso_local i64 @remove(i64 noundef %data) #0 {
entry:
  %0 = load i64*, i64** @root, align 8
  %cmp = icmp eq i64* %0, null
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  br label %return

if.end:                                           ; preds = %entry
  %1 = load i64*, i64** @root, align 8
  br label %while.cond

while.cond:                                       ; preds = %cleanup, %if.end
  %curr.0 = phi i64* [ %1, %if.end ], [ %curr.1, %cleanup ]
  %parent.0 = phi i64* [ null, %if.end ], [ %parent.1, %cleanup ]
  %retval.0 = phi i64 [ 0, %if.end ], [ %retval.1, %cleanup ]
  br label %while.body

while.body:                                       ; preds = %while.cond
  %cmp1 = icmp eq i64* %curr.0, null
  br i1 %cmp1, label %if.then2, label %if.end3

if.then2:                                         ; preds = %while.body
  br label %cleanup42

if.end3:                                          ; preds = %while.body
  %2 = load i64, i64* %curr.0, align 8
  %add.ptr = getelementptr inbounds i64, i64* %curr.0, i64 1
  %3 = load i64, i64* %add.ptr, align 8
  %4 = inttoptr i64 %3 to i64*
  %add.ptr4 = getelementptr inbounds i64, i64* %curr.0, i64 2
  %5 = load i64, i64* %add.ptr4, align 8
  %6 = inttoptr i64 %5 to i64*
  %cmp5 = icmp ult i64 %data, %2
  br i1 %cmp5, label %if.then6, label %if.end8

if.then6:                                         ; preds = %if.end3
  %add.ptr7 = getelementptr inbounds i64, i64* %curr.0, i64 1
  %7 = load i64, i64* %add.ptr7, align 8
  %8 = inttoptr i64 %7 to i64*
  br label %cleanup, !llvm.loop !8

if.end8:                                          ; preds = %if.end3
  %cmp9 = icmp ugt i64 %data, %2
  br i1 %cmp9, label %if.then10, label %if.end12

if.then10:                                        ; preds = %if.end8
  %add.ptr11 = getelementptr inbounds i64, i64* %curr.0, i64 2
  %9 = load i64, i64* %add.ptr11, align 8
  %10 = inttoptr i64 %9 to i64*
  br label %cleanup, !llvm.loop !8

if.end12:                                         ; preds = %if.end8
  %cmp13 = icmp eq i64* %4, null
  br i1 %cmp13, label %if.then14, label %if.end25

if.then14:                                        ; preds = %if.end12
  %cmp15 = icmp eq i64* %parent.0, null
  br i1 %cmp15, label %if.then16, label %if.else

if.then16:                                        ; preds = %if.then14
  store i64* %6, i64** @root, align 8
  br label %if.end24

if.else:                                          ; preds = %if.then14
  %add.ptr17 = getelementptr inbounds i64, i64* %parent.0, i64 1
  %11 = load i64, i64* %add.ptr17, align 8
  %12 = inttoptr i64 %11 to i64*
  %cmp18 = icmp eq i64* %curr.0, %12
  br i1 %cmp18, label %if.then19, label %if.else21

if.then19:                                        ; preds = %if.else
  %13 = ptrtoint i64* %6 to i64
  %add.ptr20 = getelementptr inbounds i64, i64* %parent.0, i64 1
  store i64 %13, i64* %add.ptr20, align 8
  br label %if.end23

if.else21:                                        ; preds = %if.else
  %14 = ptrtoint i64* %6 to i64
  %add.ptr22 = getelementptr inbounds i64, i64* %parent.0, i64 2
  store i64 %14, i64* %add.ptr22, align 8
  br label %if.end23

if.end23:                                         ; preds = %if.else21, %if.then19
  br label %if.end24

if.end24:                                         ; preds = %if.end23, %if.then16
  %15 = bitcast i64* %curr.0 to i8*
  call void @free(i8* noundef %15) #6
  br label %cleanup

if.end25:                                         ; preds = %if.end12
  %cmp26 = icmp eq i64* %6, null
  br i1 %cmp26, label %if.then27, label %if.end39

if.then27:                                        ; preds = %if.end25
  %cmp28 = icmp eq i64* %parent.0, null
  br i1 %cmp28, label %if.then29, label %if.else30

if.then29:                                        ; preds = %if.then27
  store i64* %4, i64** @root, align 8
  br label %if.end38

if.else30:                                        ; preds = %if.then27
  %add.ptr31 = getelementptr inbounds i64, i64* %parent.0, i64 1
  %16 = load i64, i64* %add.ptr31, align 8
  %17 = inttoptr i64 %16 to i64*
  %cmp32 = icmp eq i64* %curr.0, %17
  br i1 %cmp32, label %if.then33, label %if.else35

if.then33:                                        ; preds = %if.else30
  %18 = ptrtoint i64* %4 to i64
  %add.ptr34 = getelementptr inbounds i64, i64* %parent.0, i64 1
  store i64 %18, i64* %add.ptr34, align 8
  br label %if.end37

if.else35:                                        ; preds = %if.else30
  %19 = ptrtoint i64* %4 to i64
  %add.ptr36 = getelementptr inbounds i64, i64* %parent.0, i64 2
  store i64 %19, i64* %add.ptr36, align 8
  br label %if.end37

if.end37:                                         ; preds = %if.else35, %if.then33
  br label %if.end38

if.end38:                                         ; preds = %if.end37, %if.then29
  %20 = bitcast i64* %curr.0 to i8*
  call void @free(i8* noundef %20) #6
  br label %cleanup

if.end39:                                         ; preds = %if.end25
  %call = call i64 @adjust(i64* noundef %curr.0)
  store i64 %call, i64* %curr.0, align 8
  br label %cleanup

cleanup:                                          ; preds = %if.end39, %if.end38, %if.end24, %if.then10, %if.then6
  %cleanup.dest.slot.0 = phi i32 [ 2, %if.then6 ], [ 2, %if.then10 ], [ 1, %if.end24 ], [ 1, %if.end38 ], [ 1, %if.end39 ]
  %curr.1 = phi i64* [ %8, %if.then6 ], [ %10, %if.then10 ], [ %curr.0, %if.end24 ], [ %curr.0, %if.end38 ], [ %curr.0, %if.end39 ]
  %parent.1 = phi i64* [ %curr.0, %if.then6 ], [ %curr.0, %if.then10 ], [ %parent.0, %if.end24 ], [ %parent.0, %if.end38 ], [ %parent.0, %if.end39 ]
  %retval.1 = phi i64 [ %retval.0, %if.then6 ], [ %retval.0, %if.then10 ], [ 1, %if.end24 ], [ 1, %if.end38 ], [ 1, %if.end39 ]
  switch i32 %cleanup.dest.slot.0, label %cleanup42 [
    i32 2, label %while.cond
  ]

cleanup42:                                        ; preds = %cleanup, %if.then2
  %retval.2 = phi i64 [ 0, %if.then2 ], [ %retval.1, %cleanup ]
  br label %return

return:                                           ; preds = %cleanup42, %if.then
  %retval.3 = phi i64 [ 0, %if.then ], [ %retval.2, %cleanup42 ]
  ret i64 %retval.3
}

; Function Attrs: nounwind uwtable
define dso_local void @traverse(i64* noundef %node) #0 {
entry:
  %cmp = icmp eq i64* %node, null
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  br label %return

if.end:                                           ; preds = %entry
  %0 = load i64, i64* %node, align 8
  %add.ptr = getelementptr inbounds i64, i64* %node, i64 1
  %1 = load i64, i64* %add.ptr, align 8
  %2 = inttoptr i64 %1 to i64*
  %add.ptr1 = getelementptr inbounds i64, i64* %node, i64 2
  %3 = load i64, i64* %add.ptr1, align 8
  %4 = inttoptr i64 %3 to i64*
  call void @traverse(i64* noundef %2)
  call void @write(i64 noundef %0)
  call void @traverse(i64* noundef %4)
  br label %return

return:                                           ; preds = %if.end, %if.then
  ret void
}

declare void @write(i64 noundef) #4

; Function Attrs: nounwind uwtable
define dso_local i32 @main() #0 {
entry:
  store i64* null, i64** @root, align 8
  %call = call i64 (...) @read()
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %i.0 = phi i64 [ 0, %entry ], [ %inc, %for.inc ]
  %cmp = icmp ult i64 %i.0, %call
  br i1 %cmp, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond
  br label %for.end

for.body:                                         ; preds = %for.cond
  %call1 = call i64 (...) @read()
  %call2 = call i64 (...) @read()
  %cmp3 = icmp eq i64 %call1, 0
  br i1 %cmp3, label %if.then, label %if.else

if.then:                                          ; preds = %for.body
  %call4 = call i64 @insert(i64 noundef %call2)
  br label %if.end

if.else:                                          ; preds = %for.body
  %call5 = call i64 @remove(i64 noundef %call2)
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  br label %for.inc

for.inc:                                          ; preds = %if.end
  %inc = add i64 %i.0, 1
  br label %for.cond, !llvm.loop !9

for.end:                                          ; preds = %for.cond.cleanup
  %0 = load i64*, i64** @root, align 8
  call void @traverse(i64* noundef %0)
  ret i32 0
}

declare i64 @read(...) #4

attributes #0 = { nounwind uwtable "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { argmemonly nocallback nofree nosync nounwind willreturn }
attributes #2 = { nounwind allocsize(0) "frame-pointer"="none" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { nounwind "frame-pointer"="none" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #4 = { "frame-pointer"="none" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #5 = { nounwind allocsize(0) }
attributes #6 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{!"clang version 15.0.7 (https://github.com/llvm/llvm-project.git 8dfdcc7b7bf66834a761bd8de445840ef68e4d1a)"}
!5 = distinct !{!5, !6}
!6 = !{!"llvm.loop.unroll.disable"}
!7 = distinct !{!7, !6}
!8 = distinct !{!8, !6}
!9 = distinct !{!9, !10, !6}
!10 = !{!"llvm.loop.mustprogress"}
