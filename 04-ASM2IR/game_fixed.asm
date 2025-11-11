; Game of Life in Assembly - CORRECTED VERSION
; Initialization
start:
    INIT_FIELD
    RANDOM_INIT
    
    ; Load constants
    MOV x10 1   ; increment value
    MOV x11 64  ; size constant

main_loop:
    ; Draw current state
    DRAW_FIELD
    FLUSH
    DELAY 100

    ; Process all cells for next generation
    MOV x1 0    ; y counter

y_loop:
    MOV x2 0    ; x counter

x_loop:
    ; Calculate neighbors and update cell
    CALC_NEIGHBORS x1 x2
    UPDATE_CELL x1 x2
    
    ; Increment x and check if x < 64
    ADD x2 x2 x10
    CMP_LT x4 x2 x11
    JNZ x4 x_loop   ; if x2 < 64, continue x loop

    ; Increment y and check if y < 64  
    ADD x1 x1 x10
    CMP_LT x4 x1 x11
    JNZ x4 y_loop   ; if y1 < 64, continue y loop

    ; After processing all cells, swap buffers
    SWAP_BUFFERS

    ; Check for exit condition
    CHECK_FINISH x5
    JNZ x5 exit

    JMP main_loop

exit:
    HALT