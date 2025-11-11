#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "isa_definition.h"
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <sstream>
#include <random>

#include "sim.h"

using namespace llvm;

// Глобальные переменные для регистров и памяти
uint32_t REG_FILE[ISA_REG_FILE_SIZE] = {0};
uint32_t* CURRENT_FIELD = nullptr;
uint32_t* NEXT_FIELD = nullptr;

// Генератор случайных чисел
std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<uint32_t> dis(0, 0xFFFFFFFF);

// Функции-эмуляторы для всех инструкций
extern "C" void do_ADD(uint32_t rd, uint32_t rs1, uint32_t rs2) {
    REG_FILE[rd] = REG_FILE[rs1] + REG_FILE[rs2];
}

extern "C" void do_SUB(uint32_t rd, uint32_t rs1, uint32_t rs2) {
    REG_FILE[rd] = REG_FILE[rs1] - REG_FILE[rs2];
}

extern "C" void do_MUL(uint32_t rd, uint32_t rs1, uint32_t rs2) {
    REG_FILE[rd] = REG_FILE[rs1] * REG_FILE[rs2];
}

extern "C" void do_DIV(uint32_t rd, uint32_t rs1, uint32_t rs2) {
    if (REG_FILE[rs2] != 0) {
        REG_FILE[rd] = REG_FILE[rs1] / REG_FILE[rs2];
    }
}

extern "C" void do_MOD(uint32_t rd, uint32_t rs1, uint32_t rs2) {
    if (REG_FILE[rs2] != 0) {
        REG_FILE[rd] = REG_FILE[rs1] % REG_FILE[rs2];
    }
}

extern "C" void do_ADDI(uint32_t rd, uint32_t rs1, uint32_t imm) {
    REG_FILE[rd] = REG_FILE[rs1] + imm;
}

extern "C" void do_SUBI(uint32_t rd, uint32_t rs1, uint32_t imm) {
    REG_FILE[rd] = REG_FILE[rs1] - imm;
}

extern "C" void do_MULI(uint32_t rd, uint32_t rs1, uint32_t imm) {
    REG_FILE[rd] = REG_FILE[rs1] * imm;
}

extern "C" void do_DIVI(uint32_t rd, uint32_t rs1, uint32_t imm) {
    if (imm != 0) {
        REG_FILE[rd] = REG_FILE[rs1] / imm;
    }
}

extern "C" void do_MODI(uint32_t rd, uint32_t rs1, uint32_t imm) {
    if (imm != 0) {
        REG_FILE[rd] = REG_FILE[rs1] % imm;
    }
}

extern "C" void do_AND(uint32_t rd, uint32_t rs1, uint32_t rs2) {
    REG_FILE[rd] = REG_FILE[rs1] & REG_FILE[rs2];
}

extern "C" void do_OR(uint32_t rd, uint32_t rs1, uint32_t rs2) {
    REG_FILE[rd] = REG_FILE[rs1] | REG_FILE[rs2];
}

extern "C" void do_XOR(uint32_t rd, uint32_t rs1, uint32_t rs2) {
    REG_FILE[rd] = REG_FILE[rs1] ^ REG_FILE[rs2];
}

extern "C" void do_NOT(uint32_t rd, uint32_t rs1) {
    REG_FILE[rd] = ~REG_FILE[rs1];
}

extern "C" void do_ANDI(uint32_t rd, uint32_t rs1, uint32_t imm) {
    REG_FILE[rd] = REG_FILE[rs1] & imm;
}

extern "C" void do_ORI(uint32_t rd, uint32_t rs1, uint32_t imm) {
    REG_FILE[rd] = REG_FILE[rs1] | imm;
}

extern "C" void do_XORI(uint32_t rd, uint32_t rs1, uint32_t imm) {
    REG_FILE[rd] = REG_FILE[rs1] ^ imm;
}

extern "C" void do_CMP_EQ(uint32_t rd, uint32_t rs1, uint32_t rs2) {
    REG_FILE[rd] = (REG_FILE[rs1] == REG_FILE[rs2]) ? 1 : 0;
}

extern "C" void do_CMP_NE(uint32_t rd, uint32_t rs1, uint32_t rs2) {
    REG_FILE[rd] = (REG_FILE[rs1] != REG_FILE[rs2]) ? 1 : 0;
}

extern "C" void do_CMP_LT(uint32_t rd, uint32_t rs1, uint32_t rs2) {
    REG_FILE[rd] = (REG_FILE[rs1] < REG_FILE[rs2]) ? 1 : 0;
}

extern "C" void do_CMP_LE(uint32_t rd, uint32_t rs1, uint32_t rs2) {
    REG_FILE[rd] = (REG_FILE[rs1] <= REG_FILE[rs2]) ? 1 : 0;
}

extern "C" void do_CMP_GT(uint32_t rd, uint32_t rs1, uint32_t rs2) {
    REG_FILE[rd] = (REG_FILE[rs1] > REG_FILE[rs2]) ? 1 : 0;
}

extern "C" void do_CMP_GE(uint32_t rd, uint32_t rs1, uint32_t rs2) {
    REG_FILE[rd] = (REG_FILE[rs1] >= REG_FILE[rs2]) ? 1 : 0;
}

extern "C" void do_CMP_EQI(uint32_t rd, uint32_t rs1, uint32_t imm) {
    REG_FILE[rd] = (REG_FILE[rs1] == imm) ? 1 : 0;
}

extern "C" void do_CMP_NEI(uint32_t rd, uint32_t rs1, uint32_t imm) {
    REG_FILE[rd] = (REG_FILE[rs1] != imm) ? 1 : 0;
}

extern "C" void do_CMP_LTI(uint32_t rd, uint32_t rs1, uint32_t imm) {
    REG_FILE[rd] = (REG_FILE[rs1] < imm) ? 1 : 0;
}

extern "C" void do_CMP_LEI(uint32_t rd, uint32_t rs1, uint32_t imm) {
    REG_FILE[rd] = (REG_FILE[rs1] <= imm) ? 1 : 0;
}

extern "C" void do_CMP_GTI(uint32_t rd, uint32_t rs1, uint32_t imm) {
    REG_FILE[rd] = (REG_FILE[rs1] > imm) ? 1 : 0;
}

extern "C" void do_CMP_GEI(uint32_t rd, uint32_t rs1, uint32_t imm) {
    REG_FILE[rd] = (REG_FILE[rs1] >= imm) ? 1 : 0;
}

// extern "C" void do_LOAD(uint32_t rd, uint32_t base, uint32_t index, uint32_t scale, uint32_t offset) {
//     uint32_t addr = base + index * scale + offset;
//     if (addr < ISA_FIELD_SIZE) {
//         REG_FILE[rd] = CURRENT_FIELD[addr];
//     }
// }

// extern "C" void do_STORE(uint32_t base, uint32_t index, uint32_t scale, uint32_t offset, uint32_t rs) {
//     uint32_t addr = base + index * scale + offset;
//     if (addr < ISA_FIELD_SIZE) {
//         CURRENT_FIELD[addr] = REG_FILE[rs];
//     }
// }


extern "C" void do_LOAD_IND(uint32_t rd, uint32_t addr_reg) {
    uint32_t addr = REG_FILE[addr_reg];
    if (addr < ISA_FIELD_SIZE) {
        REG_FILE[rd] = CURRENT_FIELD[addr];
    }
}

extern "C" void do_STORE_IND(uint32_t addr_reg, uint32_t rs) {
    uint32_t addr = REG_FILE[addr_reg];
    if (addr < ISA_FIELD_SIZE) {
        CURRENT_FIELD[addr] = REG_FILE[rs];
    }
}

extern "C" void do_MOV(uint32_t rd, uint32_t rs) {
    REG_FILE[rd] = REG_FILE[rs];
}

extern "C" void do_MOVI(uint32_t rd, uint32_t imm) {
    REG_FILE[rd] = imm;
}

extern "C" void do_SHL(uint32_t rd, uint32_t rs1, uint32_t rs2) {
    REG_FILE[rd] = REG_FILE[rs1] << REG_FILE[rs2];
}

extern "C" void do_SHR(uint32_t rd, uint32_t rs1, uint32_t rs2) {
    REG_FILE[rd] = REG_FILE[rs1] >> REG_FILE[rs2];
}

extern "C" void do_SHLI(uint32_t rd, uint32_t rs1, uint32_t imm) {
    REG_FILE[rd] = REG_FILE[rs1] << imm;
}

extern "C" void do_SHRI(uint32_t rd, uint32_t rs1, uint32_t imm) {
    REG_FILE[rd] = REG_FILE[rs1] >> imm;
}

extern "C" void do_RAND(uint32_t rd) {
    REG_FILE[rd] = simRand();
}

// extern "C" void do_INIT_FIELD(uint32_t current_ptr_reg, uint32_t next_ptr_reg) {
//     CURRENT_FIELD = new uint32_t[ISA_FIELD_SIZE]();
//     NEXT_FIELD = new uint32_t[ISA_FIELD_SIZE]();
//     // Используем uintptr_t для корректного приведения указателя
//     REG_FILE[current_ptr_reg] = (uint32_t)(uintptr_t)CURRENT_FIELD;
//     REG_FILE[next_ptr_reg] = (uint32_t)(uintptr_t)NEXT_FIELD;
// }

// extern "C" void do_DRAW_CELL(uint32_t x_reg, uint32_t y_reg, uint32_t color_reg) {
//     int x = REG_FILE[x_reg];
//     int y = REG_FILE[y_reg];
//     int color = REG_FILE[color_reg];
//     // Рисуем клетку 4x4 пикселя
//     simFillRect(x, y, CELL_SIZE, CELL_SIZE, color);
// }

// extern "C" void do_FLUSH() {
//     simFlush();
// }

// extern "C" void do_DELAY(uint32_t ms) {
//     simDelay(ms);
// }

// extern "C" void do_CHECK_EXIT(uint32_t rd) {
//     REG_FILE[rd] = checkFinish();
// }

extern "C" void do_EXIT() {
    if (CURRENT_FIELD) delete[] CURRENT_FIELD;
    if (NEXT_FIELD) delete[] NEXT_FIELD;
    simExit();
}

// extern "C" void do_DUMP_REG(uint32_t reg) {
//     std::cout << "[DUMP] r" << reg << " = " << REG_FILE[reg] << std::endl;
// }

// extern "C" void do_DUMP_MEM(uint32_t addr, uint32_t count) {
//     std::cout << "[DUMP MEM] addr=" << addr << " count=" << count << std::endl;
//     for (uint32_t i = 0; i < count && (addr + i) < ISA_FIELD_SIZE; i++) {
//         std::cout << "  [" << (addr + i) << "] = " << CURRENT_FIELD[addr + i] << std::endl;
//     }
// }

extern "C" void do_LOAD(uint32_t rd, uint32_t base, uint32_t index, uint32_t scale, uint32_t offset) {
    // Вычисляем относительное смещение в массиве, а не абсолютный адрес
    uint32_t addr = index * scale + offset;
    
    if (CURRENT_FIELD && addr < ISA_FIELD_SIZE) {
        REG_FILE[rd] = CURRENT_FIELD[addr];
    } else {
        REG_FILE[rd] = 0;
    }
    std::cout << "[DEBUG] LOAD: r" << rd << " = memory[" << addr << "] = " << REG_FILE[rd] 
              << " (base=" << base << ", index=" << index << ", scale=" << scale << ", offset=" << offset << ")" << std::endl;
}

extern "C" void do_STORE(uint32_t base, uint32_t index, uint32_t scale, uint32_t offset, uint32_t rs) {
    // Вычисляем относительное смещение в массиве, а не абсолютный адрес
    uint32_t addr = index * scale + offset;
    
    // Определяем, в какое поле записывать на основе базового регистра
    uint32_t* target_field = CURRENT_FIELD;
    if (base == 1) {  // если base = r1, то пишем в NEXT_FIELD
        target_field = NEXT_FIELD;
    }
    
    if (target_field && addr < ISA_FIELD_SIZE) {
        target_field[addr] = REG_FILE[rs];
        std::cout << "[DEBUG] STORE: " << (base == 0 ? "CURRENT" : "NEXT") 
                  << "[" << addr << "] = r" << rs << " = " << REG_FILE[rs] 
                  << " (index=" << index << ", scale=" << scale << ", offset=" << offset << ")" << std::endl;
    } else {
        std::cout << "[DEBUG] STORE: INVALID ADDRESS " << addr 
                  << " (base=" << base << ", index=" << index << ")" << std::endl;
    }
}

extern "C" void do_INIT_FIELD(uint32_t current_ptr_reg, uint32_t next_ptr_reg) {
    std::cout << "[DEBUG] INIT_FIELD: current_ptr_reg=" << current_ptr_reg 
              << ", next_ptr_reg=" << next_ptr_reg << std::endl;
    
    CURRENT_FIELD = new uint32_t[ISA_FIELD_SIZE]();
    NEXT_FIELD = new uint32_t[ISA_FIELD_SIZE]();
    
    // Инициализируем нулями
    for (int i = 0; i < ISA_FIELD_SIZE; i++) {
        CURRENT_FIELD[i] = 0;
        NEXT_FIELD[i] = 0;
    }
    
    // В регистры записываем идентификаторы полей (0 для CURRENT, 1 для NEXT)
    REG_FILE[current_ptr_reg] = 0;
    REG_FILE[next_ptr_reg] = 1;
    
    std::cout << "[DEBUG] Fields initialized: CURRENT_FIELD=" << CURRENT_FIELD 
              << ", NEXT_FIELD=" << NEXT_FIELD << std::endl;
    std::cout << "[DEBUG] Registers set: r" << current_ptr_reg << " = 0, r" << next_ptr_reg << " = 1" << std::endl;
}

extern "C" void do_DRAW_CELL(uint32_t x_reg, uint32_t y_reg, uint32_t color_reg) {
    int x = REG_FILE[x_reg];
    int y = REG_FILE[y_reg];
    int color = REG_FILE[color_reg];
    
    // Проверяем границы
    if (x >= 0 && x < FIELD_WIDTH * CELL_SIZE && y >= 0 && y < FIELD_HEIGHT * CELL_SIZE) {
        std::cout << "[DEBUG] DRAW_CELL: x=" << x << ", y=" << y << ", color=0x" 
                  << std::hex << color << std::dec << std::endl;
        
        // Рисуем клетку
        simFillRect(x, y, CELL_SIZE, CELL_SIZE, color);
    } else {
        std::cout << "[DEBUG] DRAW_CELL: OUT OF BOUNDS x=" << x << ", y=" << y << std::endl;
    }
}

extern "C" void do_FLUSH() {
    std::cout << "[DEBUG] FLUSH" << std::endl;
    simFlush();
}

extern "C" void do_DELAY(uint32_t ms) {
    std::cout << "[DEBUG] DELAY: " << ms << "ms" << std::endl;
    simDelay(ms);
}

extern "C" void do_CHECK_EXIT(uint32_t rd) {
    REG_FILE[rd] = checkFinish();
    std::cout << "[DEBUG] CHECK_EXIT: r" << rd << " = " << REG_FILE[rd] << std::endl;
}

extern "C" void do_DUMP_REG(uint32_t reg) {
    std::cout << "[DUMP] r" << reg << " = " << REG_FILE[reg] << std::endl;
}

extern "C" void do_DUMP_MEM(uint32_t addr, uint32_t count) {
    std::cout << "[DUMP MEM] addr=" << addr << " count=" << count << std::endl;
    if (CURRENT_FIELD) {
        for (uint32_t i = 0; i < count && (addr + i) < ISA_FIELD_SIZE; i++) {
            std::cout << "  [" << (addr + i) << "] = " << CURRENT_FIELD[addr + i] << std::endl;
        }
    } else {
        std::cout << "  [ERROR] CURRENT_FIELD is null" << std::endl;
    }
}

// Вспомогательная функция для создания вызова функции
void createFunctionCall(IRBuilder<>& builder, Module* module, const std::string& funcName, 
                       const std::vector<Value*>& args, Type* retType = nullptr) {
    if (!retType) {
        retType = Type::getVoidTy(module->getContext());
    }
    
    std::vector<Type*> paramTypes;
    for (auto arg : args) {
        paramTypes.push_back(arg->getType());
    }
    
    FunctionType* funcType = FunctionType::get(retType, paramTypes, false);
    FunctionCallee callee = module->getOrInsertFunction(funcName, funcType);
    builder.CreateCall(callee, args);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        outs() << "[ERROR] Need 1 argument: file with assembler code\n";
        return 1;
    }

    std::cout << "Initializing simulation..." << std::endl;
    simInit();
    std::cout << "Simulation initialized successfully" << std::endl;

    std::ifstream input(argv[1]);
    if (!input.is_open()) {
        outs() << "[ERROR] Can't open " << argv[1] << '\n';
        return 1;
    }

    // Инициализация таблиц инструкций
    initializeInstructionMaps();

    LLVMContext context;
    Module *module = new Module("game_of_life_emulator", context);
    IRBuilder<> builder(context);

    // Глобальные переменные для регистрового файла
    ArrayType *regFileType = ArrayType::get(builder.getInt32Ty(), ISA_REG_FILE_SIZE);
    module->getOrInsertGlobal("regFile", regFileType);
    GlobalVariable *regFile = module->getNamedGlobal("regFile");

    // Функция main
    FunctionType *funcType = FunctionType::get(builder.getVoidTy(), false);
    Function *mainFunc = Function::Create(funcType, Function::ExternalLinkage, "main", module);

    // Чтение и разбор ASM файла
    std::string line;
    std::unordered_map<std::string, BasicBlock*> labelMap;
    std::vector<std::pair<std::string, ASMInstruction>> instructions;
    
    // Карта для подпрограмм (метка -> базовый блок)
    std::unordered_map<std::string, BasicBlock*> subroutineMap;

    // Первый проход: сбор всех меток и создание базовых блоков
    std::vector<std::string> labelsInOrder;
    while (std::getline(input, line)) {
        // Пропускаем пустые строки и комментарии
        if (line.empty() || line[0] == '#') continue;
        
        std::istringstream iss(line);
        std::string token;
        iss >> token;
        
        // Проверяем, является ли токен меткой (заканчивается на :)
        if (token.back() == ':') {
            std::string label = token.substr(0, token.size() - 1);
            labelsInOrder.push_back(label);
        }
    }

    // Создаем базовые блоки для всех меток
    BasicBlock *entryBB = BasicBlock::Create(context, "entry", mainFunc);
    labelMap["entry"] = entryBB;
    
    for (const auto& label : labelsInOrder) {
        labelMap[label] = BasicBlock::Create(context, label, mainFunc);
    }
    
    // Создаем финальный блок для выхода
    BasicBlock *exitBB = BasicBlock::Create(context, "exit", mainFunc);

    input.clear();
    input.seekg(0);

    // Второй проход: разбор инструкций
    BasicBlock *currentBB = entryBB;
    builder.SetInsertPoint(currentBB);
    
    // Стек для вызовов подпрограмм (для реализации CALL/RET)
    std::vector<BasicBlock*> callStack;

    while (std::getline(input, line)) {
        if (line.empty() || line[0] == '#') continue;
        
        std::istringstream iss(line);
        std::string token;
        iss >> token;
        
        // Обработка меток
        if (token.back() == ':') {
            std::string label = token.substr(0, token.size() - 1);
            
            // Если текущий блок не пуст и не имеет терминатора, добавляем переход к новой метке
            if (builder.GetInsertBlock() && !builder.GetInsertBlock()->getTerminator()) {
                builder.CreateBr(labelMap[label]);
            }
            
            currentBB = labelMap[label];
            builder.SetInsertPoint(currentBB);
            continue;
        }

        // Разбор инструкции
        if (instructionMap.find(token) != instructionMap.end()) {
            InsnId_t insnId = instructionMap[token];
            ASMInstruction insn(insnId, token);
            
            // Разбор операндов в зависимости от типа инструкции
            std::string operand;
            
            switch (insnId) {
                // Инструкции с тремя регистрами: OP rd, rs1, rs2
                case ISA_ADD: case ISA_SUB: case ISA_MUL: case ISA_DIV: case ISA_MOD:
                case ISA_AND: case ISA_OR: case ISA_XOR: 
                case ISA_CMP_EQ: case ISA_CMP_NE: case ISA_CMP_LT: case ISA_CMP_LE: case ISA_CMP_GT: case ISA_CMP_GE:
                case ISA_SHL: case ISA_SHR: {
                    iss >> operand; 
                    if (operand[0] == 'r') insn.rd = std::stoi(operand.substr(1));
                    
                    iss >> operand;
                    if (operand[0] == 'r') insn.rs1 = std::stoi(operand.substr(1));
                    
                    iss >> operand;
                    if (operand[0] == 'r') insn.rs2 = std::stoi(operand.substr(1));
                    
                    // Создание вызова функции-эмулятора
                    std::string funcName = "do_" + token;
                    Value* rd_val = builder.getInt32(insn.rd);
                    Value* rs1_val = builder.getInt32(insn.rs1);
                    Value* rs2_val = builder.getInt32(insn.rs2);
                    createFunctionCall(builder, module, funcName, {rd_val, rs1_val, rs2_val});
                    break;
                }
                
                // Инструкции с двумя регистрами и непосредственным значением: OP rd, rs1, imm
                case ISA_ADDI: case ISA_SUBI: case ISA_MULI: case ISA_DIVI: case ISA_MODI:
                case ISA_ANDI: case ISA_ORI: case ISA_XORI:
                case ISA_CMP_EQI: case ISA_CMP_NEI: case ISA_CMP_LTI: case ISA_CMP_LEI: case ISA_CMP_GTI: case ISA_CMP_GEI:
                case ISA_SHLI: case ISA_SHRI: {
                    iss >> operand;
                    if (operand[0] == 'r') insn.rd = std::stoi(operand.substr(1));
                    
                    iss >> operand;
                    if (operand[0] == 'r') insn.rs1 = std::stoi(operand.substr(1));
                    
                    iss >> operand;
                    insn.imm = std::stoi(operand);
                    
                    std::string funcName = "do_" + token;
                    Value* rd_val = builder.getInt32(insn.rd);
                    Value* rs1_val = builder.getInt32(insn.rs1);
                    Value* imm_val = builder.getInt32(insn.imm);
                    createFunctionCall(builder, module, funcName, {rd_val, rs1_val, imm_val});
                    break;
                }
                
                // Инструкции с двумя регистрами: OP rd, rs1
                case ISA_NOT: case ISA_MOV: {
                    iss >> operand;
                    if (operand[0] == 'r') insn.rd = std::stoi(operand.substr(1));
                    
                    iss >> operand;
                    if (operand[0] == 'r') insn.rs1 = std::stoi(operand.substr(1));
                    
                    std::string funcName = "do_" + token;
                    Value* rd_val = builder.getInt32(insn.rd);
                    Value* rs1_val = builder.getInt32(insn.rs1);
                    createFunctionCall(builder, module, funcName, {rd_val, rs1_val});
                    break;
                }
                
                // Инструкции с одним регистром и непосредственным значением: OP rd, imm
                case ISA_MOVI: {
                    iss >> operand;
                    if (operand[0] == 'r') insn.rd = std::stoi(operand.substr(1));
                    
                    iss >> operand;
                    insn.imm = std::stoi(operand);
                    
                    std::string funcName = "do_" + token;
                    Value* rd_val = builder.getInt32(insn.rd);
                    Value* imm_val = builder.getInt32(insn.imm);
                    createFunctionCall(builder, module, funcName, {rd_val, imm_val});
                    break;
                }
                
                // Инструкции LOAD с сложным адресом
                case ISA_LOAD: {
                    iss >> operand;
                    if (operand[0] == 'r') insn.rd = std::stoi(operand.substr(1));
                    
                    iss >> operand;
                    parseMemoryAddress(operand, insn.mem_addr);
                    
                    // Получаем значения из регистров для адреса
                    Value* base_val = builder.CreateLoad(builder.getInt32Ty(),
                        builder.CreateConstGEP2_32(regFileType, regFile, 0, insn.mem_addr.base_reg));
                    
                    Value* index_val = builder.getInt32(0);
                    if (insn.mem_addr.has_index) {
                        index_val = builder.CreateLoad(builder.getInt32Ty(),
                            builder.CreateConstGEP2_32(regFileType, regFile, 0, insn.mem_addr.index_reg));
                    }
                    
                    Value* scale_val = builder.getInt32(insn.mem_addr.scale);
                    Value* offset_val = builder.getInt32(insn.mem_addr.offset);
                    Value* rd_val = builder.getInt32(insn.rd);
                    
                    createFunctionCall(builder, module, "do_LOAD", 
                        {rd_val, base_val, index_val, scale_val, offset_val});
                    break;
                }
                
                // Инструкции STORE с сложным адресом
                case ISA_STORE: {
                    iss >> operand;
                    parseMemoryAddress(operand, insn.mem_addr);
                    
                    iss >> operand;
                    if (operand[0] == 'r') insn.rs1 = std::stoi(operand.substr(1));
                    
                    Value* base_val = builder.CreateLoad(builder.getInt32Ty(),
                        builder.CreateConstGEP2_32(regFileType, regFile, 0, insn.mem_addr.base_reg));
                    
                    Value* index_val = builder.getInt32(0);
                    if (insn.mem_addr.has_index) {
                        index_val = builder.CreateLoad(builder.getInt32Ty(),
                            builder.CreateConstGEP2_32(regFileType, regFile, 0, insn.mem_addr.index_reg));
                    }
                    
                    Value* scale_val = builder.getInt32(insn.mem_addr.scale);
                    Value* offset_val = builder.getInt32(insn.mem_addr.offset);
                    Value* rs_val = builder.getInt32(insn.rs1);
                    
                    createFunctionCall(builder, module, "do_STORE", 
                        {base_val, index_val, scale_val, offset_val, rs_val});
                    break;
                }
                
                // Инструкции с одним регистром
                case ISA_RAND: case ISA_CHECK_EXIT: case ISA_DUMP_REG: {
                    iss >> operand;
                    if (operand[0] == 'r') insn.rd = std::stoi(operand.substr(1));
                    
                    std::string funcName = "do_" + token;
                    Value* rd_val = builder.getInt32(insn.rd);
                    createFunctionCall(builder, module, funcName, {rd_val});
                    break;
                }
                
                // Инструкции перехода
                case ISA_JMP: {
                    iss >> operand;
                    insn.label = operand;
                    
                    // Создаем переход к метке
                    builder.CreateBr(labelMap[insn.label]);
                    
                    // Текущий блок завершен, создаем новый "мертвый" блок на случай если есть код после JMP
                    currentBB = BasicBlock::Create(context, "dead_after_jmp", mainFunc);
                    builder.SetInsertPoint(currentBB);
                    break;
                }
                
                case ISA_JMP_COND: {
                    iss >> operand;
                    if (operand[0] == 'r') insn.rs1 = std::stoi(operand.substr(1));
                    
                    iss >> operand;
                    insn.label = operand;
                    
                    // Загружаем значение условия из регистра
                    Value* cond_val = builder.CreateLoad(builder.getInt32Ty(),
                        builder.CreateConstGEP2_32(regFileType, regFile, 0, insn.rs1));
                    Value* cond_bool = builder.CreateICmpNE(cond_val, builder.getInt32(0));
                    
                    BasicBlock* thenBB = labelMap[insn.label];
                    
                    // Создаем блок для else ветки (продолжение текущего потока)
                    BasicBlock* elseBB = BasicBlock::Create(context, "else_" + insn.label, mainFunc);
                    
                    builder.CreateCondBr(cond_bool, thenBB, elseBB);
                    
                    // Переходим к else блоку для продолжения генерации кода
                    builder.SetInsertPoint(elseBB);
                    currentBB = elseBB;
                    break;
                }
                
                // Условные переходы с сравнением регистров
                case ISA_JMP_EQ: case ISA_JMP_NE: case ISA_JMP_LT: case ISA_JMP_LE: case ISA_JMP_GT: case ISA_JMP_GE: {
                    iss >> operand;
                    if (operand[0] == 'r') insn.rs1 = std::stoi(operand.substr(1));
                    
                    iss >> operand;
                    if (operand[0] == 'r') insn.rs2 = std::stoi(operand.substr(1));
                    
                    iss >> operand;
                    insn.label = operand;
                    
                    // Загружаем значения регистров
                    Value* rs1_val = builder.CreateLoad(builder.getInt32Ty(),
                        builder.CreateConstGEP2_32(regFileType, regFile, 0, insn.rs1));
                    Value* rs2_val = builder.CreateLoad(builder.getInt32Ty(),
                        builder.CreateConstGEP2_32(regFileType, regFile, 0, insn.rs2));
                    
                    // Создаем сравнение в зависимости от типа перехода
                    Value* cond = nullptr;
                    switch (insnId) {
                        case ISA_JMP_EQ: cond = builder.CreateICmpEQ(rs1_val, rs2_val); break;
                        case ISA_JMP_NE: cond = builder.CreateICmpNE(rs1_val, rs2_val); break;
                        case ISA_JMP_LT: cond = builder.CreateICmpSLT(rs1_val, rs2_val); break;
                        case ISA_JMP_LE: cond = builder.CreateICmpSLE(rs1_val, rs2_val); break;
                        case ISA_JMP_GT: cond = builder.CreateICmpSGT(rs1_val, rs2_val); break;
                        case ISA_JMP_GE: cond = builder.CreateICmpSGE(rs1_val, rs2_val); break;
                        default: break;
                    }
                    
                    BasicBlock* thenBB = labelMap[insn.label];
                    BasicBlock* elseBB = BasicBlock::Create(context, "else_" + insn.label, mainFunc);
                    
                    builder.CreateCondBr(cond, thenBB, elseBB);
                    builder.SetInsertPoint(elseBB);
                    currentBB = elseBB;
                    break;
                }
                
                // Вызов подпрограммы
                case ISA_CALL: {
                    iss >> operand;
                    insn.label = operand;
                    
                    // Сохраняем текущий блок в стеке (следующая инструкция после CALL)
                    BasicBlock* returnBB = BasicBlock::Create(context, "return_" + insn.label, mainFunc);
                    callStack.push_back(returnBB);
                    
                    // Переход к подпрограмме
                    builder.CreateBr(labelMap[insn.label]);
                    
                    // Переходим к блоку возврата для генерации кода после возврата
                    builder.SetInsertPoint(returnBB);
                    currentBB = returnBB;
                    break;
                }
                
                // Возврат из подпрограммы
                case ISA_RET: {
                    if (!callStack.empty()) {
                        BasicBlock* returnBB = callStack.back();
                        callStack.pop_back();
                        builder.CreateBr(returnBB);
                        
                        // Создаем новый блок на случай, если есть код после RET
                        currentBB = BasicBlock::Create(context, "dead_after_ret", mainFunc);
                        builder.SetInsertPoint(currentBB);
                    } else {
                        // Если стек пуст, завершаем программу
                        builder.CreateRetVoid();
                    }
                    break;
                }
                
                // Специальные инструкции
                case ISA_INIT_FIELD: {
                    iss >> operand;
                    if (operand[0] == 'r') insn.rd = std::stoi(operand.substr(1));
                    
                    iss >> operand;
                    if (operand[0] == 'r') insn.rs1 = std::stoi(operand.substr(1));
                    
                    Value* rd_val = builder.getInt32(insn.rd);
                    Value* rs1_val = builder.getInt32(insn.rs1);
                    createFunctionCall(builder, module, "do_INIT_FIELD", {rd_val, rs1_val});
                    break;
                }
                
                case ISA_DRAW_CELL: {
                    iss >> operand;
                    if (operand[0] == 'r') insn.rd = std::stoi(operand.substr(1));
                    
                    iss >> operand;
                    if (operand[0] == 'r') insn.rs1 = std::stoi(operand.substr(1));
                    
                    iss >> operand;
                    if (operand[0] == 'r') insn.rs2 = std::stoi(operand.substr(1));
                    
                    Value* rd_val = builder.getInt32(insn.rd);
                    Value* rs1_val = builder.getInt32(insn.rs1);
                    Value* rs2_val = builder.getInt32(insn.rs2);
                    createFunctionCall(builder, module, "do_DRAW_CELL", {rd_val, rs1_val, rs2_val});
                    break;
                }
                
                case ISA_DELAY: {
                    iss >> operand;
                    insn.imm = std::stoi(operand);
                    
                    Value* imm_val = builder.getInt32(insn.imm);
                    createFunctionCall(builder, module, "do_DELAY", {imm_val});
                    break;
                }
                
                // Инструкции без аргументов
                case ISA_FLUSH: {
                    createFunctionCall(builder, module, "do_FLUSH", {});
                    break;
                }
                
                case ISA_EXIT: {
                    createFunctionCall(builder, module, "do_EXIT", {});
                    builder.CreateRetVoid();
                    break;
                }
                
                default:
                    outs() << "[WARNING] Unhandled instruction: " << token << "\n";
                    break;
            }
            
            instructions.push_back({line, insn});
        }
    }

    // Добавляем терминаторы ко всем базовым блокам, которые их не имеют
    for (auto& BB : *mainFunc) {
        if (!BB.getTerminator()) {
            builder.SetInsertPoint(&BB);
            
            // Если это подпрограмма, добавляем возврат
            bool isSubroutine = false;
            for (const auto& label : labelsInOrder) {
                if (BB.getName() == label) {
                    isSubroutine = true;
                    break;
                }
            }
            
            if (isSubroutine && !callStack.empty()) {
                // Возврат из подпрограммы
                BasicBlock* returnBB = callStack.back();
                callStack.pop_back();
                builder.CreateBr(returnBB);
            } else {
                // Иначе завершаем программу
                builder.CreateRetVoid();
            }
        }
    }

    // Верификация и вывод
    outs() << "\n#[LLVM IR]:\n";
    module->print(outs(), nullptr);
    
    bool verified = verifyModule(*module, &outs());
    outs() << "[VERIFICATION] " << (verified ? "FAIL" : "OK") << "\n";

    // Запуск через ExecutionEngine
    InitializeNativeTarget();
    InitializeNativeTargetAsmPrinter();

    ExecutionEngine *ee = EngineBuilder(std::unique_ptr<Module>(module)).create();
    
    // Регистрация ВСЕХ функций-эмуляторов
    ee->InstallLazyFunctionCreator([](const std::string &fnName) -> void * {
        if (fnName == "do_ADD") return (void*)do_ADD;
        if (fnName == "do_SUB") return (void*)do_SUB;
        if (fnName == "do_MUL") return (void*)do_MUL;
        if (fnName == "do_DIV") return (void*)do_DIV;
        if (fnName == "do_MOD") return (void*)do_MOD;
        if (fnName == "do_ADDI") return (void*)do_ADDI;
        if (fnName == "do_SUBI") return (void*)do_SUBI;
        if (fnName == "do_MULI") return (void*)do_MULI;
        if (fnName == "do_DIVI") return (void*)do_DIVI;
        if (fnName == "do_MODI") return (void*)do_MODI;
        if (fnName == "do_AND") return (void*)do_AND;
        if (fnName == "do_OR") return (void*)do_OR;
        if (fnName == "do_XOR") return (void*)do_XOR;
        if (fnName == "do_NOT") return (void*)do_NOT;
        if (fnName == "do_ANDI") return (void*)do_ANDI;
        if (fnName == "do_ORI") return (void*)do_ORI;
        if (fnName == "do_XORI") return (void*)do_XORI;
        if (fnName == "do_CMP_EQ") return (void*)do_CMP_EQ;
        if (fnName == "do_CMP_NE") return (void*)do_CMP_NE;
        if (fnName == "do_CMP_LT") return (void*)do_CMP_LT;
        if (fnName == "do_CMP_LE") return (void*)do_CMP_LE;
        if (fnName == "do_CMP_GT") return (void*)do_CMP_GT;
        if (fnName == "do_CMP_GE") return (void*)do_CMP_GE;
        if (fnName == "do_CMP_EQI") return (void*)do_CMP_EQI;
        if (fnName == "do_CMP_NEI") return (void*)do_CMP_NEI;
        if (fnName == "do_CMP_LTI") return (void*)do_CMP_LTI;
        if (fnName == "do_CMP_LEI") return (void*)do_CMP_LEI;
        if (fnName == "do_CMP_GTI") return (void*)do_CMP_GTI;
        if (fnName == "do_CMP_GEI") return (void*)do_CMP_GEI;
        if (fnName == "do_LOAD") return (void*)do_LOAD;
        if (fnName == "do_STORE") return (void*)do_STORE;
        if (fnName == "do_LOAD_IND") return (void*)do_LOAD_IND;
        if (fnName == "do_STORE_IND") return (void*)do_STORE_IND;
        if (fnName == "do_MOV") return (void*)do_MOV;
        if (fnName == "do_MOVI") return (void*)do_MOVI;
        if (fnName == "do_SHL") return (void*)do_SHL;
        if (fnName == "do_SHR") return (void*)do_SHR;
        if (fnName == "do_SHLI") return (void*)do_SHLI;
        if (fnName == "do_SHRI") return (void*)do_SHRI;
        if (fnName == "do_RAND") return (void*)do_RAND;
        if (fnName == "do_INIT_FIELD") return (void*)do_INIT_FIELD;
        if (fnName == "do_DRAW_CELL") return (void*)do_DRAW_CELL;
        if (fnName == "do_FLUSH") return (void*)do_FLUSH;
        if (fnName == "do_DELAY") return (void*)do_DELAY;
        if (fnName == "do_CHECK_EXIT") return (void*)do_CHECK_EXIT;
        if (fnName == "do_EXIT") return (void*)do_EXIT;
        if (fnName == "do_DUMP_REG") return (void*)do_DUMP_REG;
        if (fnName == "do_DUMP_MEM") return (void*)do_DUMP_MEM;
        return nullptr;
    });

    ee->addGlobalMapping(regFile, (void*)REG_FILE);
    ee->finalizeObject();

    // Запуск
    outs() << "\n#[Running code]\n";
    ArrayRef<GenericValue> noargs;
    ee->runFunction(mainFunc, noargs);
    outs() << "#[Code execution completed]\n";

    std::cout << "=== ASM2IR Emulator Finished ===" << std::endl;
    return 0;
}