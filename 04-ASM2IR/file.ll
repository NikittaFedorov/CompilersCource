; ModuleID = 'game_of_life_ir'
source_filename = "game_of_life_ir"

define void @main() {
block_0:
  call void @do_INIT_FIELD()
  br label %block_1

block_1:                                          ; preds = %block_0
  call void @do_RANDOM_INIT()
  br label %block_2

block_2:                                          ; preds = %block_1
  call void @do_MOV(i32 10, i32 0, i32 1)
  br label %block_3

block_3:                                          ; preds = %block_2
  call void @do_MOV(i32 11, i32 0, i32 64)
  br label %block_4

block_4:                                          ; preds = %block_3
  call void @do_DRAW_FIELD()
  br label %block_5

block_5:                                          ; preds = %block_4
  call void @do_FLUSH()
  br label %block_6

block_6:                                          ; preds = %block_5
  call void @do_DELAY(i32 100)
  br label %block_7

block_7:                                          ; preds = %block_6
  call void @do_MOV(i32 1, i32 0, i32 0)
  br label %block_8

block_8:                                          ; preds = %block_7
  call void @do_MOV(i32 2, i32 0, i32 0)
  br label %block_9

block_9:                                          ; preds = %block_8
  call void @do_CALC_NEIGHBORS(i32 1, i32 2)
  br label %block_10

block_10:                                         ; preds = %block_9
  call void @do_UPDATE_CELL(i32 1, i32 2)
  br label %block_11

block_11:                                         ; preds = %block_10
  call void @do_ADD(i32 2, i32 2, i32 10)
  br label %block_12

block_12:                                         ; preds = %block_11
  call void @do_CMP_LT(i32 4, i32 2, i32 11)
  br label %block_13

block_13:                                         ; preds = %block_12
  call void @do_JNZ(i32 4, i32 9, i32 14)

block_14:                                         ; No predecessors!
  call void @do_ADD(i32 1, i32 1, i32 10)
  br label %block_15

block_15:                                         ; preds = %block_14
  call void @do_CMP_LT(i32 4, i32 1, i32 11)
  br label %block_16

block_16:                                         ; preds = %block_15
  call void @do_JNZ(i32 4, i32 8, i32 17)

block_17:                                         ; No predecessors!
  call void @do_SWAP_BUFFERS()
  br label %block_18

block_18:                                         ; preds = %block_17
  call void @do_CHECK_FINISH(i32 5)
  br label %block_19

block_19:                                         ; preds = %block_18
  call void @do_JNZ(i32 5, i32 21, i32 20)

block_20:                                         ; No predecessors!
  call void @do_JMP(i32 4)

block_21:                                         ; No predecessors!
  call void @do_HALT()
  br label %exit

exit:                                             ; preds = %block_21
  ret void
}

declare void @do_CALC_NEIGHBORS(i32, i32)

declare void @do_UPDATE_CELL(i32, i32)

declare void @do_MOV(i32, i32, i32)

declare void @do_ADD(i32, i32, i32)

declare void @do_SUB(i32, i32, i32)

declare void @do_MUL(i32, i32, i32)

declare void @do_AND(i32, i32, i32)

declare void @do_OR(i32, i32, i32)

declare void @do_XOR(i32, i32, i32)

declare void @do_CMP(i32, i32, i32)

declare void @do_CMP_LT(i32, i32, i32)

declare void @do_CMP_GT(i32, i32, i32)

declare void @do_JZ(i32, i32, i32)

declare void @do_JNZ(i32, i32, i32)

declare void @do_DELAY(i32)

declare void @do_CHECK_FINISH(i32)

declare void @do_JMP(i32)

declare void @do_INIT_FIELD()

declare void @do_RANDOM_INIT()

declare void @do_SWAP_BUFFERS()

declare void @do_DRAW_FIELD()

declare void @do_FLUSH()

declare void @do_HALT()
