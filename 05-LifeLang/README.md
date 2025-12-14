# LifeLang (Flex + Bison to LLVM IR to ExecutionEngine)

## Синтаксис языка

### Лексика
- Идентификаторы: `[A-Za-z_][A-Za-z0-9_]*`
- Числа: десятичные (`123`) и hex (`0xC71585`)
- Комментарии: `// ...` и `/* ... */`

### Типы
Поддерживаются:
- `i32`
- `void`

Все переменные — `i32` или массивы `i32` фиксированных размеров.

### Верхний уровень
- константы: `const NAME = 123;`
- функции:
  - `func name() { ... }`  (по умолчанию `void`)
  - `func name() : i32 { ... }`

### Операторы/выражения
- арифметика: `+ - * / %`
- сравнения: `== != < <= > >=` (результат `i32` 0/1)
- логика: `&& || !`
- скобки: `( ... )`
- доступ к массиву: `a[i][j]`
- вызов: `foo()`, `rand()`, `fill_rect(...)` и т.д.

### Операторы/стейтменты
- блок: `{ ... }`
- объявление:
  - `var x;`
  - `var x = 5;`
  - `var a[64][64];`
- присваивание: `x = expr;`, `a[i][j] = expr;`
- if/else:
  - `if (cond) { ... }`
  - `if (cond) { ... } else { ... }`
- while: `while (cond) { ... }`
- for: `for (init; cond; step) { ... }`
- `break;`, `continue;`
- `return;` или `return expr;` (в `: i32` функции)

## 2) Builtins под sim.h 

| LifeLang | C функция | Сигнатура |
|---|---|---|
| `fill_rect(x,y,w,h,color)` | `simFillRect` | `void(i32,i32,i32,i32,i32)` |
| `flush()` | `simFlush` | `void()` |
| `delay(ms)` | `simDelay` | `void(i32)` |
| `ticks()` | `simGetTicks` | `i32()` |
| `rand()` | `simRand` | `i32()` |
| `finish()` | `checkFinish` | `i32()` |
| `put_pixel(x,y,color)` | `simPutPixel` | `void(i32,i32,i32)` |
| `mouse_x()` | `simGetMouseX` | `i32()` |
| `mouse_y()` | `simGetMouseY` | `i32()` |
| `mouse_btn(btn)` | `simIsMouseButtonDown` | `i32(i32)` |
| `key_down(code)` | `simIsKeyDown` | `i32(i32)` |

Для тестов используется `runtime/sim_stub.c` (без SDL).  
Для графики — `runtime/sim_sdl.c` (с SDL2, компиляция по умолчанию).

## 3) Сборка

Требования:
- clang++
- bison, flex
- LLVM
- SDL2

### Сборка компилятора (без sdl, только для тестов)
```bash
make life_sdl
```

### Прогон тестов
```bash
make test
```

### Запуск Game of Life 
```bash
make life_sdl
./life_sdl examples/game_of_life.lf
```

### Генерация LLVM IR в файл
```bash
./life_sdl examples/game_of_life.lf -o out.ll --no-run
```