; ModuleID = 'game_of_life'
source_filename = "game_of_life"

@regs = common global [16 x i32] zeroinitializer
@current = common global [64 x [64 x i32]] zeroinitializer
@next = common global [64 x [64 x i32]] zeroinitializer
@temp = common global i32 0

declare void @simFillRect(i32, i32, i32, i32, i32)

declare void @simFlush()

declare void @simDelay(i32)

declare i32 @checkFinish()

define void @main() {
block_0:
  br label %block_1

block_1:                                          ; preds = %block_0
  br label %block_2

block_2:                                          ; preds = %block_1
  store i32 1, i32* getelementptr inbounds ([16 x i32], [16 x i32]* @regs, i32 0, i32 0), align 4
  br label %block_3

block_3:                                          ; preds = %block_2
  store i32 64, i32* getelementptr inbounds ([16 x i32], [16 x i32]* @regs, i32 0, i32 1), align 4
  br label %block_4

block_4:                                          ; preds = %block_3
  store i32 3, i32* getelementptr inbounds ([16 x i32], [16 x i32]* @regs, i32 0, i32 2), align 4
  br label %block_5

block_5:                                          ; preds = %block_21, %block_4
  store i32 0, i32* getelementptr inbounds ([16 x i32], [16 x i32]* @regs, i32 0, i32 0), align 4
  br label %draw_y_loop

block_6:                                          ; preds = %draw_y_end
  call void @simFlush()
  br label %block_7

block_7:                                          ; preds = %block_6
  call void @simDelay(i32 100)
  br label %block_8

block_8:                                          ; preds = %block_7
  store i32 0, i32* getelementptr inbounds ([16 x i32], [16 x i32]* @regs, i32 0, i32 3), align 4
  br label %block_9

block_9:                                          ; preds = %block_17, %block_8
  store i32 0, i32* getelementptr inbounds ([16 x i32], [16 x i32]* @regs, i32 0, i32 4), align 4
  br label %block_10

block_10:                                         ; preds = %block_14, %block_9
  store i32 0, i32* getelementptr inbounds ([16 x i32], [16 x i32]* @regs, i32 0, i32 5), align 4
  br label %block_11

block_11:                                         ; preds = %block_10
  br label %block_12

block_12:                                         ; preds = %block_11
  %0 = load i32, i32* getelementptr inbounds ([16 x i32], [16 x i32]* @regs, i32 0, i32 4), align 4
  %1 = add i32 %0, 1
  store i32 %1, i32* getelementptr inbounds ([16 x i32], [16 x i32]* @regs, i32 0, i32 4), align 4
  br label %block_13

block_13:                                         ; preds = %block_12
  %2 = load i32, i32* getelementptr inbounds ([16 x i32], [16 x i32]* @regs, i32 0, i32 4), align 4
  %3 = load i32, i32* getelementptr inbounds ([16 x i32], [16 x i32]* @regs, i32 0, i32 1), align 4
  %4 = icmp slt i32 %2, %3
  %5 = zext i1 %4 to i32
  store i32 %5, i32* getelementptr inbounds ([16 x i32], [16 x i32]* @regs, i32 0, i32 7), align 4
  br label %block_14

block_14:                                         ; preds = %block_13
  %6 = load i32, i32* getelementptr inbounds ([16 x i32], [16 x i32]* @regs, i32 0, i32 7), align 4
  %7 = icmp ne i32 %6, 0
  br i1 %7, label %block_10, label %block_15

block_15:                                         ; preds = %block_14
  %8 = load i32, i32* getelementptr inbounds ([16 x i32], [16 x i32]* @regs, i32 0, i32 3), align 4
  %9 = add i32 %8, 1
  store i32 %9, i32* getelementptr inbounds ([16 x i32], [16 x i32]* @regs, i32 0, i32 3), align 4
  br label %block_16

block_16:                                         ; preds = %block_15
  %10 = load i32, i32* getelementptr inbounds ([16 x i32], [16 x i32]* @regs, i32 0, i32 3), align 4
  %11 = load i32, i32* getelementptr inbounds ([16 x i32], [16 x i32]* @regs, i32 0, i32 1), align 4
  %12 = icmp slt i32 %10, %11
  %13 = zext i1 %12 to i32
  store i32 %13, i32* getelementptr inbounds ([16 x i32], [16 x i32]* @regs, i32 0, i32 7), align 4
  br label %block_17

block_17:                                         ; preds = %block_16
  %14 = load i32, i32* getelementptr inbounds ([16 x i32], [16 x i32]* @regs, i32 0, i32 7), align 4
  %15 = icmp ne i32 %14, 0
  br i1 %15, label %block_9, label %block_18

block_18:                                         ; preds = %block_17
  store i32 0, i32* getelementptr inbounds ([16 x i32], [16 x i32]* @regs, i32 0, i32 0), align 4
  br label %swap_y_loop

block_19:                                         ; preds = %swap_y_end
  %16 = call i32 @checkFinish()
  store i32 %16, i32* getelementptr inbounds ([16 x i32], [16 x i32]* @regs, i32 0, i32 15), align 4
  br label %block_20

block_20:                                         ; preds = %block_19
  %17 = load i32, i32* getelementptr inbounds ([16 x i32], [16 x i32]* @regs, i32 0, i32 15), align 4
  %18 = icmp ne i32 %17, 0
  br i1 %18, label %block_22, label %block_21

block_21:                                         ; preds = %block_20
  br label %block_5

block_22:                                         ; preds = %block_20
  br label %exit

exit:                                             ; preds = %block_22
  ret void

draw_y_loop:                                      ; preds = %draw_x_end, %block_5
  %19 = load i32, i32* getelementptr inbounds ([16 x i32], [16 x i32]* @regs, i32 0, i32 0), align 4
  %20 = icmp slt i32 %19, 64
  br i1 %20, label %draw_y_body, label %draw_y_end

draw_y_body:                                      ; preds = %draw_y_loop
  store i32 0, i32* getelementptr inbounds ([16 x i32], [16 x i32]* @regs, i32 0, i32 1), align 4
  br label %draw_x_loop

draw_y_end:                                       ; preds = %draw_y_loop
  br label %block_6

draw_x_loop:                                      ; preds = %draw_x_body, %draw_y_body
  %21 = load i32, i32* getelementptr inbounds ([16 x i32], [16 x i32]* @regs, i32 0, i32 1), align 4
  %22 = icmp slt i32 %21, 64
  br i1 %22, label %draw_x_body, label %draw_x_end

draw_x_body:                                      ; preds = %draw_x_loop
  %23 = getelementptr [64 x [64 x i32]], [64 x [64 x i32]]* @current, i32 0, i32 %19, i32 %21
  %24 = load i32, i32* %23, align 4
  %25 = icmp eq i32 %24, 1
  %26 = icmp eq i32 %24, 2
  %27 = select i1 %26, i32 65280, i32 0
  %28 = select i1 %25, i32 13047173, i32 %27
  %29 = mul i32 %21, 4
  %30 = mul i32 %19, 4
  call void @simFillRect(i32 %29, i32 %30, i32 4, i32 4, i32 %28)
  %31 = add i32 %21, 1
  store i32 %31, i32* getelementptr inbounds ([16 x i32], [16 x i32]* @regs, i32 0, i32 1), align 4
  br label %draw_x_loop

draw_x_end:                                       ; preds = %draw_x_loop
  %32 = add i32 %19, 1
  store i32 %32, i32* getelementptr inbounds ([16 x i32], [16 x i32]* @regs, i32 0, i32 0), align 4
  br label %draw_y_loop

swap_y_loop:                                      ; preds = %swap_x_end, %block_18
  %33 = load i32, i32* getelementptr inbounds ([16 x i32], [16 x i32]* @regs, i32 0, i32 0), align 4
  %34 = icmp slt i32 %33, 64
  br i1 %34, label %swap_y_body, label %swap_y_end

swap_y_body:                                      ; preds = %swap_y_loop
  store i32 0, i32* getelementptr inbounds ([16 x i32], [16 x i32]* @regs, i32 0, i32 1), align 4
  br label %swap_x_loop

swap_y_end:                                       ; preds = %swap_y_loop
  br label %block_19

swap_x_loop:                                      ; preds = %swap_x_body, %swap_y_body
  %35 = load i32, i32* getelementptr inbounds ([16 x i32], [16 x i32]* @regs, i32 0, i32 1), align 4
  %36 = icmp slt i32 %35, 64
  br i1 %36, label %swap_x_body, label %swap_x_end

swap_x_body:                                      ; preds = %swap_x_loop
  %37 = getelementptr [64 x [64 x i32]], [64 x [64 x i32]]* @next, i32 0, i32 %33, i32 %35
  %38 = getelementptr [64 x [64 x i32]], [64 x [64 x i32]]* @current, i32 0, i32 %33, i32 %35
  %39 = load i32, i32* %37, align 4
  store i32 %39, i32* %38, align 4
  %40 = add i32 %35, 1
  store i32 %40, i32* getelementptr inbounds ([16 x i32], [16 x i32]* @regs, i32 0, i32 1), align 4
  br label %swap_x_loop

swap_x_end:                                       ; preds = %swap_x_loop
  %41 = add i32 %33, 1
  store i32 %41, i32* getelementptr inbounds ([16 x i32], [16 x i32]* @regs, i32 0, i32 0), align 4
  br label %swap_y_loop
}
