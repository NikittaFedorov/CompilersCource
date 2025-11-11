#ifndef ISA_DEFINITION_H
#define ISA_DEFINITION_H

#include <cstdint>
#include <string>
#include <map>
#include <vector>

// Набор инструкций на основе статистики из Game of Life
enum InsnId_t {
    // Арифметические операции
    ISA_ADD,        // r1 = r2 + r3
    ISA_SUB,        // r1 = r2 - r3  
    ISA_MUL,        // r1 = r2 * r3
    ISA_DIV,        // r1 = r2 / r3
    ISA_MOD,        // r1 = r2 % r3
    
    // Арифметика с непосредственными значениями
    ISA_ADDI,       // r1 = r2 + imm
    ISA_SUBI,       // r1 = r2 - imm
    ISA_MULI,       // r1 = r2 * imm
    ISA_DIVI,       // r1 = r2 / imm
    ISA_MODI,       // r1 = r2 % imm
    
    // Логические операции
    ISA_AND,        // r1 = r2 & r3
    ISA_OR,         // r1 = r2 | r3
    ISA_XOR,        // r1 = r2 ^ r3
    ISA_NOT,        // r1 = ~r2
    ISA_ANDI,       // r1 = r2 & imm
    ISA_ORI,        // r1 = r2 | imm
    ISA_XORI,       // r1 = r2 ^ imm
    
    // Операции сравнения
    ISA_CMP_EQ,     // r1 = (r2 == r3)
    ISA_CMP_NE,     // r1 = (r2 != r3)
    ISA_CMP_LT,     // r1 = (r2 < r3)
    ISA_CMP_LE,     // r1 = (r2 <= r3)
    ISA_CMP_GT,     // r1 = (r2 > r3)
    ISA_CMP_GE,     // r1 = (r2 >= r3)
    ISA_CMP_EQI,    // r1 = (r2 == imm)
    ISA_CMP_NEI,    // r1 = (r2 != imm)
    ISA_CMP_LTI,    // r1 = (r2 < imm)
    ISA_CMP_LEI,    // r1 = (r2 <= imm)
    ISA_CMP_GTI,    // r1 = (r2 > imm)
    ISA_CMP_GEI,    // r1 = (r2 >= imm)
    
    // Операции с памятью
    ISA_LOAD,       // r1 = memory[address]
    ISA_STORE,      // memory[address] = r1
    ISA_LOAD_IND,   // r1 = memory[r2]
    ISA_STORE_IND,  // memory[r1] = r2
    ISA_MOV,        // r1 = r2
    ISA_MOVI,       // r1 = imm
    
    // Операции сдвига
    ISA_SHL,        // r1 = r2 << r3
    ISA_SHR,        // r1 = r2 >> r3
    ISA_SHLI,       // r1 = r2 << imm
    ISA_SHRI,       // r1 = r2 >> imm
    
    // Управление потоком
    ISA_JMP,        // безусловный переход
    ISA_JMP_COND,   // if (r1) goto label
    ISA_JMP_EQ,     // if (r1 == r2) goto label
    ISA_JMP_NE,     // if (r1 != r2) goto label
    ISA_JMP_LT,     // if (r1 < r2) goto label
    ISA_JMP_LE,     // if (r1 <= r2) goto label
    ISA_JMP_GT,     // if (r1 > r2) goto label
    ISA_JMP_GE,     // if (r1 >= r2) goto label
    ISA_CALL,       // вызов подпрограммы
    ISA_RET,        // возврат из подпрограммы
    
    // Специальные инструкции для Game of Life
    ISA_INIT_FIELD, // инициализация игрового поля
    ISA_RAND,       // генерация случайного числа
    ISA_DRAW_CELL,  // отрисовка клетки
    ISA_FLUSH,      // обновление экрана
    ISA_DELAY,      // задержка
    ISA_CHECK_EXIT, // проверка завершения
    ISA_EXIT,       // завершение программы
    
    // Инструкции для отладки
    ISA_DUMP_REG,   // дамп регистра
    ISA_DUMP_MEM    // дамп памяти
};

using RegVal_t = uint32_t;
const int ISA_REG_FILE_SIZE = 16;
const int ISA_FIELD_WIDTH = 64;
const int ISA_FIELD_HEIGHT = 64;
const int ISA_FIELD_SIZE = ISA_FIELD_WIDTH * ISA_FIELD_HEIGHT;

struct MemoryAddress {
    uint32_t base_reg;
    uint32_t index_reg;
    uint32_t scale;
    uint32_t offset;
    bool has_index;
    
    MemoryAddress() : base_reg(0), index_reg(0), scale(1), offset(0), has_index(false) {}
};

struct ASMInstruction {
    InsnId_t id;
    std::string name;
    uint32_t rd;          // destination register
    uint32_t rs1;         // source register 1
    uint32_t rs2;         // source register 2
    uint32_t imm;         // immediate value
    std::string label;    // target label for jumps
    MemoryAddress mem_addr; // address for load/store
    
    ASMInstruction(InsnId_t i, const std::string& n, uint32_t d = 0, 
                uint32_t s1 = 0, uint32_t s2 = 0, uint32_t im = 0, 
                const std::string& lbl = "")
        : id(i), name(n), rd(d), rs1(s1), rs2(s2), imm(im), label(lbl) {}
};

// Таблицы для преобразования имен инструкций
extern std::map<std::string, InsnId_t> instructionMap;
extern std::map<InsnId_t, std::string> instructionNameMap;

void initializeInstructionMaps();
bool parseMemoryAddress(const std::string& addr_str, MemoryAddress& addr);
std::string memoryAddressToString(const MemoryAddress& addr);

#endif