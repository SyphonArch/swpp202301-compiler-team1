; ModuleID = '/tmp/a.ll'
source_filename = "friend/src/friend.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK: malloc
; Function Attrs: nounwind uwtable
define dso_local i8* @malloc_upto_8(i64 noundef %x) #0 {
entry:
  %add = add i64 %x, 7
  %div = udiv i64 %add, 8
  %mul = mul i64 %div, 8
  %call = call noalias i8* @malloc(i64 noundef %mul) #4
  ret i8* %call
}

; Function Attrs: nounwind allocsize(0)
declare noalias i8* @malloc(i64 noundef) #1

; Function Attrs: nounwind uwtable
define dso_local i32 @findSample(i32 noundef %n, i32* noundef %confidence, i32* noundef %host, i32* noundef %protocol) #0 {
entry:
  %conv = sext i32 %n to i64
  %mul = mul i64 8, %conv
  %call = call i8* @malloc_upto_8(i64 noundef %mul)
  %0 = bitcast i8* %call to i32*
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %i.0 = phi i32 [ 0, %entry ], [ %inc, %for.inc ]
  %cmp = icmp slt i32 %i.0, %n
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %idxprom = sext i32 %i.0 to i64
  %arrayidx = getelementptr inbounds i32, i32* %confidence, i64 %idxprom
  %1 = load i32, i32* %arrayidx, align 4
  %mul2 = mul nsw i32 2, %i.0
  %idxprom3 = sext i32 %mul2 to i64
  %arrayidx4 = getelementptr inbounds i32, i32* %0, i64 %idxprom3
  store i32 %1, i32* %arrayidx4, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %inc = add nsw i32 %i.0, 1
  br label %for.cond, !llvm.loop !5

for.end:                                          ; preds = %for.cond
  %sub = sub nsw i32 %n, 1
  br label %for.cond5

for.cond5:                                        ; preds = %for.inc200, %for.end
  %i.1 = phi i32 [ %sub, %for.end ], [ %dec, %for.inc200 ]
  %cmp6 = icmp sge i32 %i.1, 1
  br i1 %cmp6, label %for.body8, label %for.end201

for.body8:                                        ; preds = %for.cond5
  %idxprom9 = sext i32 %i.1 to i64
  %arrayidx10 = getelementptr inbounds i32, i32* %protocol, i64 %idxprom9
  %2 = load i32, i32* %arrayidx10, align 4
  switch i32 %2, label %sw.default [
    i32 0, label %sw.bb
    i32 1, label %sw.bb43
  ]

sw.bb:                                            ; preds = %for.body8
  %mul11 = mul nsw i32 2, %i.1
  %add = add nsw i32 %mul11, 1
  %idxprom12 = sext i32 %add to i64
  %arrayidx13 = getelementptr inbounds i32, i32* %0, i64 %idxprom12
  %3 = load i32, i32* %arrayidx13, align 4
  %idxprom14 = sext i32 %i.1 to i64
  %arrayidx15 = getelementptr inbounds i32, i32* %host, i64 %idxprom14
  %4 = load i32, i32* %arrayidx15, align 4
  %mul16 = mul nsw i32 2, %4
  %idxprom17 = sext i32 %mul16 to i64
  %arrayidx18 = getelementptr inbounds i32, i32* %0, i64 %idxprom17
  %5 = load i32, i32* %arrayidx18, align 4
  %add19 = add nsw i32 %5, %3
  store i32 %add19, i32* %arrayidx18, align 4
  %mul20 = mul nsw i32 2, %i.1
  %idxprom21 = sext i32 %mul20 to i64
  %arrayidx22 = getelementptr inbounds i32, i32* %0, i64 %idxprom21
  %6 = load i32, i32* %arrayidx22, align 4
  %mul23 = mul nsw i32 2, %i.1
  %add24 = add nsw i32 %mul23, 1
  %idxprom25 = sext i32 %add24 to i64
  %arrayidx26 = getelementptr inbounds i32, i32* %0, i64 %idxprom25
  %7 = load i32, i32* %arrayidx26, align 4
  %cmp27 = icmp sgt i32 %6, %7
  br i1 %cmp27, label %cond.true, label %cond.false

cond.true:                                        ; preds = %sw.bb
  %mul29 = mul nsw i32 2, %i.1
  %idxprom30 = sext i32 %mul29 to i64
  %arrayidx31 = getelementptr inbounds i32, i32* %0, i64 %idxprom30
  %8 = load i32, i32* %arrayidx31, align 4
  br label %cond.end

cond.false:                                       ; preds = %sw.bb
  %mul32 = mul nsw i32 2, %i.1
  %add33 = add nsw i32 %mul32, 1
  %idxprom34 = sext i32 %add33 to i64
  %arrayidx35 = getelementptr inbounds i32, i32* %0, i64 %idxprom34
  %9 = load i32, i32* %arrayidx35, align 4
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.true
  %cond = phi i32 [ %8, %cond.true ], [ %9, %cond.false ]
  %idxprom36 = sext i32 %i.1 to i64
  %arrayidx37 = getelementptr inbounds i32, i32* %host, i64 %idxprom36
  %10 = load i32, i32* %arrayidx37, align 4
  %mul38 = mul nsw i32 2, %10
  %add39 = add nsw i32 %mul38, 1
  %idxprom40 = sext i32 %add39 to i64
  %arrayidx41 = getelementptr inbounds i32, i32* %0, i64 %idxprom40
  %11 = load i32, i32* %arrayidx41, align 4
  %add42 = add nsw i32 %11, %cond
  store i32 %add42, i32* %arrayidx41, align 4
  br label %sw.epilog

sw.bb43:                                          ; preds = %for.body8
  %idxprom44 = sext i32 %i.1 to i64
  %arrayidx45 = getelementptr inbounds i32, i32* %host, i64 %idxprom44
  %12 = load i32, i32* %arrayidx45, align 4
  %mul46 = mul nsw i32 2, %12
  %idxprom47 = sext i32 %mul46 to i64
  %arrayidx48 = getelementptr inbounds i32, i32* %0, i64 %idxprom47
  %13 = load i32, i32* %arrayidx48, align 4
  %mul49 = mul nsw i32 2, %i.1
  %idxprom50 = sext i32 %mul49 to i64
  %arrayidx51 = getelementptr inbounds i32, i32* %0, i64 %idxprom50
  %14 = load i32, i32* %arrayidx51, align 4
  %mul52 = mul nsw i32 2, %i.1
  %add53 = add nsw i32 %mul52, 1
  %idxprom54 = sext i32 %add53 to i64
  %arrayidx55 = getelementptr inbounds i32, i32* %0, i64 %idxprom54
  %15 = load i32, i32* %arrayidx55, align 4
  %cmp56 = icmp sgt i32 %14, %15
  br i1 %cmp56, label %cond.true58, label %cond.false62

cond.true58:                                      ; preds = %sw.bb43
  %mul59 = mul nsw i32 2, %i.1
  %idxprom60 = sext i32 %mul59 to i64
  %arrayidx61 = getelementptr inbounds i32, i32* %0, i64 %idxprom60
  %16 = load i32, i32* %arrayidx61, align 4
  br label %cond.end67

cond.false62:                                     ; preds = %sw.bb43
  %mul63 = mul nsw i32 2, %i.1
  %add64 = add nsw i32 %mul63, 1
  %idxprom65 = sext i32 %add64 to i64
  %arrayidx66 = getelementptr inbounds i32, i32* %0, i64 %idxprom65
  %17 = load i32, i32* %arrayidx66, align 4
  br label %cond.end67

cond.end67:                                       ; preds = %cond.false62, %cond.true58
  %cond68 = phi i32 [ %16, %cond.true58 ], [ %17, %cond.false62 ]
  %add69 = add nsw i32 %13, %cond68
  %idxprom70 = sext i32 %i.1 to i64
  %arrayidx71 = getelementptr inbounds i32, i32* %host, i64 %idxprom70
  %18 = load i32, i32* %arrayidx71, align 4
  %mul72 = mul nsw i32 2, %18
  %add73 = add nsw i32 %mul72, 1
  %idxprom74 = sext i32 %add73 to i64
  %arrayidx75 = getelementptr inbounds i32, i32* %0, i64 %idxprom74
  %19 = load i32, i32* %arrayidx75, align 4
  %mul76 = mul nsw i32 2, %i.1
  %idxprom77 = sext i32 %mul76 to i64
  %arrayidx78 = getelementptr inbounds i32, i32* %0, i64 %idxprom77
  %20 = load i32, i32* %arrayidx78, align 4
  %add79 = add nsw i32 %19, %20
  %cmp80 = icmp sgt i32 %add69, %add79
  br i1 %cmp80, label %cond.true82, label %cond.false109

cond.true82:                                      ; preds = %cond.end67
  %idxprom83 = sext i32 %i.1 to i64
  %arrayidx84 = getelementptr inbounds i32, i32* %host, i64 %idxprom83
  %21 = load i32, i32* %arrayidx84, align 4
  %mul85 = mul nsw i32 2, %21
  %idxprom86 = sext i32 %mul85 to i64
  %arrayidx87 = getelementptr inbounds i32, i32* %0, i64 %idxprom86
  %22 = load i32, i32* %arrayidx87, align 4
  %mul88 = mul nsw i32 2, %i.1
  %idxprom89 = sext i32 %mul88 to i64
  %arrayidx90 = getelementptr inbounds i32, i32* %0, i64 %idxprom89
  %23 = load i32, i32* %arrayidx90, align 4
  %mul91 = mul nsw i32 2, %i.1
  %add92 = add nsw i32 %mul91, 1
  %idxprom93 = sext i32 %add92 to i64
  %arrayidx94 = getelementptr inbounds i32, i32* %0, i64 %idxprom93
  %24 = load i32, i32* %arrayidx94, align 4
  %cmp95 = icmp sgt i32 %23, %24
  br i1 %cmp95, label %cond.true97, label %cond.false101

cond.true97:                                      ; preds = %cond.true82
  %mul98 = mul nsw i32 2, %i.1
  %idxprom99 = sext i32 %mul98 to i64
  %arrayidx100 = getelementptr inbounds i32, i32* %0, i64 %idxprom99
  %25 = load i32, i32* %arrayidx100, align 4
  br label %cond.end106

cond.false101:                                    ; preds = %cond.true82
  %mul102 = mul nsw i32 2, %i.1
  %add103 = add nsw i32 %mul102, 1
  %idxprom104 = sext i32 %add103 to i64
  %arrayidx105 = getelementptr inbounds i32, i32* %0, i64 %idxprom104
  %26 = load i32, i32* %arrayidx105, align 4
  br label %cond.end106

cond.end106:                                      ; preds = %cond.false101, %cond.true97
  %cond107 = phi i32 [ %25, %cond.true97 ], [ %26, %cond.false101 ]
  %add108 = add nsw i32 %22, %cond107
  br label %cond.end120

cond.false109:                                    ; preds = %cond.end67
  %idxprom110 = sext i32 %i.1 to i64
  %arrayidx111 = getelementptr inbounds i32, i32* %host, i64 %idxprom110
  %27 = load i32, i32* %arrayidx111, align 4
  %mul112 = mul nsw i32 2, %27
  %add113 = add nsw i32 %mul112, 1
  %idxprom114 = sext i32 %add113 to i64
  %arrayidx115 = getelementptr inbounds i32, i32* %0, i64 %idxprom114
  %28 = load i32, i32* %arrayidx115, align 4
  %mul116 = mul nsw i32 2, %i.1
  %idxprom117 = sext i32 %mul116 to i64
  %arrayidx118 = getelementptr inbounds i32, i32* %0, i64 %idxprom117
  %29 = load i32, i32* %arrayidx118, align 4
  %add119 = add nsw i32 %28, %29
  br label %cond.end120

cond.end120:                                      ; preds = %cond.false109, %cond.end106
  %cond121 = phi i32 [ %add108, %cond.end106 ], [ %add119, %cond.false109 ]
  %idxprom122 = sext i32 %i.1 to i64
  %arrayidx123 = getelementptr inbounds i32, i32* %host, i64 %idxprom122
  %30 = load i32, i32* %arrayidx123, align 4
  %mul124 = mul nsw i32 2, %30
  %idxprom125 = sext i32 %mul124 to i64
  %arrayidx126 = getelementptr inbounds i32, i32* %0, i64 %idxprom125
  store i32 %cond121, i32* %arrayidx126, align 4
  %mul127 = mul nsw i32 2, %i.1
  %add128 = add nsw i32 %mul127, 1
  %idxprom129 = sext i32 %add128 to i64
  %arrayidx130 = getelementptr inbounds i32, i32* %0, i64 %idxprom129
  %31 = load i32, i32* %arrayidx130, align 4
  %idxprom131 = sext i32 %i.1 to i64
  %arrayidx132 = getelementptr inbounds i32, i32* %host, i64 %idxprom131
  %32 = load i32, i32* %arrayidx132, align 4
  %mul133 = mul nsw i32 2, %32
  %add134 = add nsw i32 %mul133, 1
  %idxprom135 = sext i32 %add134 to i64
  %arrayidx136 = getelementptr inbounds i32, i32* %0, i64 %idxprom135
  %33 = load i32, i32* %arrayidx136, align 4
  %add137 = add nsw i32 %33, %31
  store i32 %add137, i32* %arrayidx136, align 4
  br label %sw.epilog

sw.default:                                       ; preds = %for.body8
  %idxprom138 = sext i32 %i.1 to i64
  %arrayidx139 = getelementptr inbounds i32, i32* %host, i64 %idxprom138
  %34 = load i32, i32* %arrayidx139, align 4
  %mul140 = mul nsw i32 2, %34
  %idxprom141 = sext i32 %mul140 to i64
  %arrayidx142 = getelementptr inbounds i32, i32* %0, i64 %idxprom141
  %35 = load i32, i32* %arrayidx142, align 4
  %mul143 = mul nsw i32 2, %i.1
  %add144 = add nsw i32 %mul143, 1
  %idxprom145 = sext i32 %add144 to i64
  %arrayidx146 = getelementptr inbounds i32, i32* %0, i64 %idxprom145
  %36 = load i32, i32* %arrayidx146, align 4
  %add147 = add nsw i32 %35, %36
  %idxprom148 = sext i32 %i.1 to i64
  %arrayidx149 = getelementptr inbounds i32, i32* %host, i64 %idxprom148
  %37 = load i32, i32* %arrayidx149, align 4
  %mul150 = mul nsw i32 2, %37
  %add151 = add nsw i32 %mul150, 1
  %idxprom152 = sext i32 %add151 to i64
  %arrayidx153 = getelementptr inbounds i32, i32* %0, i64 %idxprom152
  %38 = load i32, i32* %arrayidx153, align 4
  %mul154 = mul nsw i32 2, %i.1
  %idxprom155 = sext i32 %mul154 to i64
  %arrayidx156 = getelementptr inbounds i32, i32* %0, i64 %idxprom155
  %39 = load i32, i32* %arrayidx156, align 4
  %add157 = add nsw i32 %38, %39
  %cmp158 = icmp sgt i32 %add147, %add157
  br i1 %cmp158, label %cond.true160, label %cond.false171

cond.true160:                                     ; preds = %sw.default
  %idxprom161 = sext i32 %i.1 to i64
  %arrayidx162 = getelementptr inbounds i32, i32* %host, i64 %idxprom161
  %40 = load i32, i32* %arrayidx162, align 4
  %mul163 = mul nsw i32 2, %40
  %idxprom164 = sext i32 %mul163 to i64
  %arrayidx165 = getelementptr inbounds i32, i32* %0, i64 %idxprom164
  %41 = load i32, i32* %arrayidx165, align 4
  %mul166 = mul nsw i32 2, %i.1
  %add167 = add nsw i32 %mul166, 1
  %idxprom168 = sext i32 %add167 to i64
  %arrayidx169 = getelementptr inbounds i32, i32* %0, i64 %idxprom168
  %42 = load i32, i32* %arrayidx169, align 4
  %add170 = add nsw i32 %41, %42
  br label %cond.end182

cond.false171:                                    ; preds = %sw.default
  %idxprom172 = sext i32 %i.1 to i64
  %arrayidx173 = getelementptr inbounds i32, i32* %host, i64 %idxprom172
  %43 = load i32, i32* %arrayidx173, align 4
  %mul174 = mul nsw i32 2, %43
  %add175 = add nsw i32 %mul174, 1
  %idxprom176 = sext i32 %add175 to i64
  %arrayidx177 = getelementptr inbounds i32, i32* %0, i64 %idxprom176
  %44 = load i32, i32* %arrayidx177, align 4
  %mul178 = mul nsw i32 2, %i.1
  %idxprom179 = sext i32 %mul178 to i64
  %arrayidx180 = getelementptr inbounds i32, i32* %0, i64 %idxprom179
  %45 = load i32, i32* %arrayidx180, align 4
  %add181 = add nsw i32 %44, %45
  br label %cond.end182

cond.end182:                                      ; preds = %cond.false171, %cond.true160
  %cond183 = phi i32 [ %add170, %cond.true160 ], [ %add181, %cond.false171 ]
  %idxprom184 = sext i32 %i.1 to i64
  %arrayidx185 = getelementptr inbounds i32, i32* %host, i64 %idxprom184
  %46 = load i32, i32* %arrayidx185, align 4
  %mul186 = mul nsw i32 2, %46
  %idxprom187 = sext i32 %mul186 to i64
  %arrayidx188 = getelementptr inbounds i32, i32* %0, i64 %idxprom187
  store i32 %cond183, i32* %arrayidx188, align 4
  %mul189 = mul nsw i32 2, %i.1
  %add190 = add nsw i32 %mul189, 1
  %idxprom191 = sext i32 %add190 to i64
  %arrayidx192 = getelementptr inbounds i32, i32* %0, i64 %idxprom191
  %47 = load i32, i32* %arrayidx192, align 4
  %idxprom193 = sext i32 %i.1 to i64
  %arrayidx194 = getelementptr inbounds i32, i32* %host, i64 %idxprom193
  %48 = load i32, i32* %arrayidx194, align 4
  %mul195 = mul nsw i32 2, %48
  %add196 = add nsw i32 %mul195, 1
  %idxprom197 = sext i32 %add196 to i64
  %arrayidx198 = getelementptr inbounds i32, i32* %0, i64 %idxprom197
  %49 = load i32, i32* %arrayidx198, align 4
  %add199 = add nsw i32 %49, %47
  store i32 %add199, i32* %arrayidx198, align 4
  br label %sw.epilog

sw.epilog:                                        ; preds = %cond.end182, %cond.end120, %cond.end
  br label %for.inc200

for.inc200:                                       ; preds = %sw.epilog
  %dec = add nsw i32 %i.1, -1
  br label %for.cond5, !llvm.loop !8

for.end201:                                       ; preds = %for.cond5
  %arrayidx202 = getelementptr inbounds i32, i32* %0, i64 0
  %50 = load i32, i32* %arrayidx202, align 4
  %arrayidx203 = getelementptr inbounds i32, i32* %0, i64 1
  %51 = load i32, i32* %arrayidx203, align 4
  %cmp204 = icmp sgt i32 %50, %51
  br i1 %cmp204, label %cond.true206, label %cond.false208

cond.true206:                                     ; preds = %for.end201
  %arrayidx207 = getelementptr inbounds i32, i32* %0, i64 0
  %52 = load i32, i32* %arrayidx207, align 4
  br label %cond.end210

cond.false208:                                    ; preds = %for.end201
  %arrayidx209 = getelementptr inbounds i32, i32* %0, i64 1
  %53 = load i32, i32* %arrayidx209, align 4
  br label %cond.end210

cond.end210:                                      ; preds = %cond.false208, %cond.true206
  %cond211 = phi i32 [ %52, %cond.true206 ], [ %53, %cond.false208 ]
  ret i32 %cond211
}

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #2

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #2

; Function Attrs: nounwind uwtable
define dso_local i32 @main() #0 {
entry:
  %call = call i64 (...) @read()
  %conv = trunc i64 %call to i32
  %conv1 = sext i32 %conv to i64
  %mul = mul i64 4, %conv1
  %call2 = call i8* @malloc_upto_8(i64 noundef %mul)
  %0 = bitcast i8* %call2 to i32*
  %conv3 = sext i32 %conv to i64
  %mul4 = mul i64 4, %conv3
  %call5 = call i8* @malloc_upto_8(i64 noundef %mul4)
  %1 = bitcast i8* %call5 to i32*
  %conv6 = sext i32 %conv to i64
  %mul7 = mul i64 4, %conv6
  %call8 = call i8* @malloc_upto_8(i64 noundef %mul7)
  %2 = bitcast i8* %call8 to i32*
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %i.0 = phi i32 [ 0, %entry ], [ %inc, %for.inc ]
  %cmp = icmp slt i32 %i.0, %conv
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %call10 = call i64 (...) @read()
  %conv11 = trunc i64 %call10 to i32
  %idxprom = sext i32 %i.0 to i64
  %arrayidx = getelementptr inbounds i32, i32* %0, i64 %idxprom
  store i32 %conv11, i32* %arrayidx, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %inc = add nsw i32 %i.0, 1
  br label %for.cond, !llvm.loop !9

for.end:                                          ; preds = %for.cond
  br label %for.cond12

for.cond12:                                       ; preds = %for.inc24, %for.end
  %i.1 = phi i32 [ 1, %for.end ], [ %inc25, %for.inc24 ]
  %cmp13 = icmp slt i32 %i.1, %conv
  br i1 %cmp13, label %for.body15, label %for.end26

for.body15:                                       ; preds = %for.cond12
  %call16 = call i64 (...) @read()
  %conv17 = trunc i64 %call16 to i32
  %idxprom18 = sext i32 %i.1 to i64
  %arrayidx19 = getelementptr inbounds i32, i32* %1, i64 %idxprom18
  store i32 %conv17, i32* %arrayidx19, align 4
  %call20 = call i64 (...) @read()
  %conv21 = trunc i64 %call20 to i32
  %idxprom22 = sext i32 %i.1 to i64
  %arrayidx23 = getelementptr inbounds i32, i32* %2, i64 %idxprom22
  store i32 %conv21, i32* %arrayidx23, align 4
  br label %for.inc24

for.inc24:                                        ; preds = %for.body15
  %inc25 = add nsw i32 %i.1, 1
  br label %for.cond12, !llvm.loop !10

for.end26:                                        ; preds = %for.cond12
  %call27 = call i32 @findSample(i32 noundef %conv, i32* noundef %0, i32* noundef %1, i32* noundef %2)
  %conv28 = sext i32 %call27 to i64
  call void @write(i64 noundef %conv28)
  ret i32 0
}

declare i64 @read(...) #3

declare void @write(i64 noundef) #3

attributes #0 = { nounwind uwtable "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { nounwind allocsize(0) "frame-pointer"="none" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #2 = { argmemonly nocallback nofree nosync nounwind willreturn }
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
