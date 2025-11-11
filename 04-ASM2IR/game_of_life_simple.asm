# Game of Life - упрощенная реализация без подпрограмм
# Регистры:
# r0: указатель на current field
# r1: указатель на next field  
# r2: координата y
# r3: координата x
# r4: временные вычисления
# r5: счетчик соседей
# r6: count1 (соседи типа 1)
# r7: count2 (соседи типа 2)
# r8: временные значения
# r9: цвет клетки

entry:
    # Инициализация полей
    INIT_FIELD r0, r1
    
    # Инициализация случайными значениями
    XOR r2, r2, r2      # y = 0

init_y_loop:
    XOR r3, r3, r3      # x = 0

init_x_loop:
    # Генерация случайного числа и проверка вероятности
    # if (rand() % 100 < 30)
    RAND r4
    MODI r4, r4, 100
    CMP_LTI r8, r4, 30
    JMP_COND r8, init_alive
    
    # Мертвая клетка
    MOVI r4, 0
    STORE [r0 + r2*64 + r3], r4
    JMP init_continue
    
init_alive:
    RAND r4
    ANDI r8, r4, 1
    CMP_EQI r9, r8, 0
    JMP_COND r9, init_alive1
    
    # ALIVE2
    MOVI r4, 2
    STORE [r0 + r2*64 + r3], r4
    JMP init_continue
    
init_alive1:
    # ALIVE1  
    MOVI r4, 1
    STORE [r0 + r2*64 + r3], r4
    
init_continue:
    ADDI r3, r3, 1
    CMP_NEI r8, r3, 64
    JMP_COND r8, init_x_loop
    
    ADDI r2, r2, 1
    CMP_NEI r8, r2, 64
    JMP_COND r8, init_y_loop

main_loop:
    # Отрисовка поля
    XOR r2, r2, r2      # y = 0
    
draw_y_loop:
    XOR r3, r3, r3      # x = 0
    
draw_x_loop:
    # Загрузка состояния клетки
    LOAD r4, [r0 + r2*64 + r3]
    
    # Определение цвета клетки
    CMP_EQI r8, r4, 1
    JMP_COND r8, draw_alive1
    
    CMP_EQI r8, r4, 2
    JMP_COND r8, draw_alive2
    
    # DEAD - черный
    MOVI r9, 0x000000
    JMP draw_cell
    
draw_alive1:
    # ALIVE1 - фиолетовый
    MOVI r9, 0xC71585
    JMP draw_cell
    
draw_alive2:
    # ALIVE2 - зеленый
    MOVI r9, 0x00FF00
    
draw_cell:
    # Вычисление координат для отрисовки (x * 4, y * 4)
    MOV r10, r3
    SHLI r10, r10, 2    # x * 4
    
    MOV r11, r2
    SHLI r11, r11, 2    # y * 4
    
    # Отрисовка клетки 4x4 пикселя
    DRAW_CELL r10, r11, r9
    
    ADDI r3, r3, 1
    CMP_NEI r8, r3, 64
    JMP_COND r8, draw_x_loop
    
    ADDI r2, r2, 1
    CMP_NEI r8, r2, 64
    JMP_COND r8, draw_y_loop
    
    # Задержка между поколениями
    DELAY 10
    
    # Обновление экрана
    FLUSH
    
    # Вычисление следующего поколения
    XOR r2, r2, r2      # y = 0
    
update_y_loop:
    XOR r3, r3, r3      # x = 0
    
update_x_loop:
    # Подсчет соседей
    XOR r5, r5, r5      # neighbors = 0
    XOR r6, r6, r6      # count1 = 0
    XOR r7, r7, r7      # count2 = 0
    
    # Проверка всех 8 соседей с циклическими границами
    
    # Сосед (-1, -1)
    MOV r12, r2
    ADDI r12, r12, 63   # y-1 mod 64
    ANDI r12, r12, 63
    
    MOV r13, r3
    ADDI r13, r13, 63   # x-1 mod 64  
    ANDI r13, r13, 63
    
    LOAD r4, [r0 + r12*64 + r13]
    CALL count_neighbor
    
    # Сосед (-1, 0)
    MOV r13, r3         # x mod 64
    ANDI r13, r13, 63
    
    LOAD r4, [r0 + r12*64 + r13]
    CALL count_neighbor
    
    # Сосед (-1, 1)
    MOV r13, r3
    ADDI r13, r13, 1    # x+1 mod 64
    ANDI r13, r13, 63
    
    LOAD r4, [r0 + r12*64 + r13]
    CALL count_neighbor
    
    # Сосед (0, -1)
    MOV r12, r2         # y mod 64
    ANDI r12, r12, 63
    
    MOV r13, r3
    ADDI r13, r13, 63   # x-1 mod 64
    ANDI r13, r13, 63
    
    LOAD r4, [r0 + r12*64 + r13]
    CALL count_neighbor
    
    # Сосед (0, 1)
    MOV r13, r3
    ADDI r13, r13, 1    # x+1 mod 64
    ANDI r13, r13, 63
    
    LOAD r4, [r0 + r12*64 + r13]
    CALL count_neighbor
    
    # Сосед (1, -1)
    MOV r12, r2
    ADDI r12, r12, 1    # y+1 mod 64
    ANDI r12, r12, 63
    
    MOV r13, r3
    ADDI r13, r13, 63   # x-1 mod 64
    ANDI r13, r13, 63
    
    LOAD r4, [r0 + r12*64 + r13]
    CALL count_neighbor
    
    # Сосед (1, 0)
    MOV r13, r3         # x mod 64
    ANDI r13, r13, 63
    
    LOAD r4, [r0 + r12*64 + r13]
    CALL count_neighbor
    
    # Сосед (1, 1)
    MOV r13, r3
    ADDI r13, r13, 1    # x+1 mod 64
    ANDI r13, r13, 63
    
    LOAD r4, [r0 + r12*64 + r13]
    CALL count_neighbor
    
    # Применение правил Game of Life
    LOAD r4, [r0 + r2*64 + r3]  # текущее состояние
    
    CMP_GTI r8, r4, 0           # если клетка жива
    JMP_COND r8, alive_rules
    
dead_rules:
    # Правило для мертвой клетки: оживает если 3 соседа
    CMP_EQI r8, r5, 3
    JMP_COND r8, dead_birth
    
    # Остается мертвой
    MOVI r4, 0
    STORE [r1 + r2*64 + r3], r4
    JMP update_continue
    
dead_birth:
    # Решаем какой тип клетки родится
    CMP_GT r8, r6, r7
    JMP_COND r8, birth_type1
    
    # ALIVE2
    MOVI r4, 2
    STORE [r1 + r2*64 + r3], r4
    JMP update_continue
    
birth_type1:
    # ALIVE1
    MOVI r4, 1
    STORE [r1 + r2*64 + r3], r4
    JMP update_continue
    
alive_rules:
    # Правило для живой клетки: выживает если 2 или 3 соседа
    CMP_EQI r8, r5, 2
    JMP_COND r8, alive_survive
    
    CMP_EQI r8, r5, 3
    JMP_COND r8, alive_survive
    
    # Умирает
    MOVI r4, 0
    STORE [r1 + r2*64 + r3], r4
    JMP update_continue
    
alive_survive:
    # Остается живой (тот же тип)
    STORE [r1 + r2*64 + r3], r4
    
update_continue:
    ADDI r3, r3, 1
    CMP_NEI r8, r3, 64
    JMP_COND r8, update_x_loop
    
    ADDI r2, r2, 1
    CMP_NEI r8, r2, 64
    JMP_COND r8, update_y_loop
    
    # Копирование next в current
    # Вместо CALL copy_field - инлайним код
    XOR r2, r2, r2
    
copy_y_loop:
    XOR r3, r3, r3
    
copy_x_loop:
    # Загрузка из next
    LOAD r4, [r1 + r2*64 + r3]
    
    # Сохранение в current
    STORE [r0 + r2*64 + r3], r4
    
    ADDI r3, r3, 1
    CMP_NEI r12, r3, 64
    JMP_COND r12, copy_x_loop
    
    ADDI r2, r2, 1
    CMP_NEI r12, r2, 64
    JMP_COND r12, copy_y_loop
    
    # Проверка завершения
    CHECK_EXIT r8
    JMP_COND r8, program_exit
    
    JMP main_loop

program_exit:
    EXIT

# Подпрограмма для подсчета соседа
count_neighbor:
    CMP_EQI r14, r4, 1
    JMP_COND r14, count_alive1
    
    CMP_EQI r14, r4, 2
    JMP_COND r14, count_alive2
    
    RET
    
count_alive1:
    ADDI r5, r5, 1  # neighbors++
    ADDI r6, r6, 1  # count1++
    RET
    
count_alive2:
    ADDI r5, r5, 1  # neighbors++
    ADDI r7, r7, 1  # count2++
    RET