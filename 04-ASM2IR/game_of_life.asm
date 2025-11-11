; Game of Life - Normal Assembly Version
start:
    CALL init_field
    CALL random_init
    
    ; Константы
    MOVI x0, 1    ; increment
    MOVI x1, 64   ; size
    MOVI x2, 3    ; neighbor threshold

main_loop:
    CALL draw_field
    CALL flush
    CALL delay

    ; Обработка всех ячеек
    MOVI x3, 0    ; y = 0

y_loop:
    MOVI x4, 0    ; x = 0

x_loop:
    ; Вычисляем соседей для текущей ячейки
    CALC_NEIGHBORS x3 x4 x5  ; x5 = count1, x6 = count2
    UPDATE_CELL x3 x4 x5 x6
    
    ; x++
    ADDI x4, x4, 1
    CMP_LT x7, x4, x1       ; x < 64?
    JNZ x7, x_loop

    ; y++ 
    ADDI x3, x3, 1
    CMP_LT x7, x3, x1       ; y < 64?
    JNZ x7, y_loop

    ; Обновляем буферы
    CALL swap_buffers

    ; Проверяем завершение
    CALL check_finish
    JNZ x15, exit           ; x15 содержит результат check_finish

    JMP main_loop

exit:
    HALT