# Game of Life - исправленная версия с правильной адресацией
# Регистры:
# r0: идентификатор current field (0)  
# r1: идентификатор next field (1)
# r2: координата y (индекс)
# r3: координата x (индекс)

entry:
    # Инициализация полей
    INIT_FIELD r0, r1
    DUMP_REG 0
    DUMP_REG 1
    
    # Простая инициализация - несколько живых клеток
    MOVI r2, 32      # y = 32
    MOVI r3, 32      # x = 32
    MOVI r4, 1       # значение = 1 (живая)
    STORE [r0 + r2*64 + r3], r4
    
    MOVI r2, 33
    MOVI r3, 32
    MOVI r4, 1
    STORE [r0 + r2*64 + r3], r4
    
    MOVI r2, 34
    MOVI r3, 32
    MOVI r4, 1
    STORE [r0 + r2*64 + r3], r4
    
    # Дамп памяти вокруг наших клеток для проверки
    MOVI r12, 32*64 + 30
    DUMP_MEM r12, 10

main_loop:
    # Отрисовка поля
    XOR r2, r2, r2      # y = 0
    
draw_y_loop:
    XOR r3, r3, r3      # x = 0
    
draw_x_loop:
    # Загрузка состояния клетки из CURRENT поля (r0 = 0)
    LOAD r4, [r0 + r2*64 + r3]
    
    # Определение цвета
    CMP_EQI r8, r4, 1
    JMP_COND r8, draw_alive
    
    # Мертвая - черный
    MOVI r9, 0x000000
    JMP draw_cell
    
draw_alive:
    # Живая - белый (для отладки)
    MOVI r9, 0xFFFFFF
    
draw_cell:
    # Вычисление координат пикселей (индексы -> пиксели)
    MOV r10, r3
    SHLI r10, r10, 2    # x * 4
    
    MOV r11, r2
    SHLI r11, r11, 2    # y * 4
    
    # Отрисовка клетки
    DRAW_CELL r10, r11, r9
    
    ADDI r3, r3, 1
    CMP_NEI r8, r3, 64
    JMP_COND r8, draw_x_loop
    
    ADDI r2, r2, 1
    CMP_NEI r8, r2, 64
    JMP_COND r8, draw_y_loop
    
    # Отладочный вывод
    DUMP_REG 2
    DUMP_REG 3
    
    # Задержка и обновление
    DELAY 1000
    FLUSH
    
    # Проверка выхода
    CHECK_EXIT r8
    JMP_COND r8, program_exit
    
    JMP main_loop

program_exit:
    EXIT