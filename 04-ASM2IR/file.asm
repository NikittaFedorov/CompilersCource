; Simple test program
start:
    INIT_FIELD
    RANDOM_INIT
    MOV x1 0
    MOV x2 0
    MOV x10 1

loop:
    CALC_NEIGHBORS x1 x2
    UPDATE_CELL x1 x2
    
    ; Increment x2
    ADD x2 x2 x10
    ; Compare x2 with 64
    MOV x3 64
    SUB x4 x2 x3
    JZ x4 next_row
    
    JMP loop

next_row:
    MOV x2 0
    ; Increment x1
    ADD x1 x1 x10
    ; Compare x1 with 64
    SUB x4 x1 x3
    JZ x4 finish
    
    JMP loop

finish:
    SWAP_BUFFERS
    DRAW_FIELD
    FLUSH
    DELAY 1000
    CHECK_FINISH x5
    JZ x5 start
    HALT