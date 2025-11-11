; ModuleID = 'game_of_life_ir'
source_filename = "game_of_life_ir"

define void @main() {
entry:
  call void @do_INIT_FIELD()
  call void @do_RANDOM_INIT()
  call void @do_MOV(i32 10, i32 0, i32 1)
  call void @do_MOV(i32 11, i32 0, i32 64)
  call void @do_DRAW_FIELD()
  call void @do_FLUSH()
  call void @do_DELAY(i32 100)
  call void @do_MOV(i32 1, i32 0, i32 0)
  call void @do_MOV(i32 2, i32 0, i32 0)
  call void @do_CALC_NEIGHBORS(i32 1, i32 2)
  call void @do_UPDATE_CELL(i32 1, i32 2)
  call void @do_ADD(i32 2, i32 2, i32 10)
  call void @do_CMP_LT(i32 4, i32 2, i32 11)
  call void @do_ADD(i32 1, i32 1, i32 10)
  call void @do_CMP_LT(i32 4, i32 1, i32 11)
  call void @do_SWAP_BUFFERS()
  call void @do_CHECK_FINISH(i32 5)
  ret void
}

declare void @do_MOV(i32, i32, i32)

declare void @do_ADD(i32, i32, i32)

declare void @do_SUB(i32, i32, i32)

declare void @do_MUL(i32, i32, i32)

declare void @do_AND(i32, i32, i32)

declare void @do_OR(i32, i32, i32)

declare void @do_XOR(i32, i32, i32)

declare void @do_CALC_NEIGHBORS(i32, i32)

declare void @do_UPDATE_CELL(i32, i32)

declare void @do_DELAY(i32)

declare void @do_CHECK_FINISH(i32)

declare void @do_INIT_FIELD()

declare void @do_RANDOM_INIT()

declare void @do_SWAP_BUFFERS()

declare void @do_DRAW_FIELD()

declare void @do_FLUSH()

declare void @do_CMP(i32, i32, i32)

declare void @do_CMP_LT(i32, i32, i32)

declare void @do_CMP_GT(i32, i32, i32)
