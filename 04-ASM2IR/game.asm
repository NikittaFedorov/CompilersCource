; Game of Life in Assembly
; Initialization
start:
    INIT_FIELD
    RANDOM_INIT
    
main_loop:
    ; Reset counters
    MOV x1 0    ; y counter
    MOV x2 0    ; x counter
    MOV x10 1   ; increment value
    
    DRAW_FIELD
    FLUSH
    DELAY 10

y_loop:
    MOV x2 0    ; reset x counter

x_loop:
    ; Calculate neighbors and update cell
    CALC_NEIGHBORS x1 x2
    UPDATE_CELL x1 x2
    
    ; Increment x
    ADD x2 x2 x10
    
    ; Check if x < 64
    MOV x3 64
    CMP x4 x2 x3
    JZ x4 next_y
    
    JMP x_loop

next_y:
    ; Increment y
    ADD x1 x1 x10
    
    ; Check if y < 64
    CMP x4 x1 x3
    JZ x4 swap_buffers
    
    JMP y_loop

swap_buffers:
    SWAP_BUFFERS
    
    ; Check if should exit
    CHECK_FINISH x5
    JNZ x5 exit
    
    JMP main_loop

exit:
    HALT