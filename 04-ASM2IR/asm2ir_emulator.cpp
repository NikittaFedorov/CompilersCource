#include "sim.h"
#include "isa.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <memory>
#include <unordered_map>

using ASM::CPUState;

// Глобальное состояние
CPUState cpu;

// Функции-эмуляторы для вызовов из IR
extern "C" void do_MOV(uint32_t rd, uint32_t imm) {
    if (rd < 16) cpu.regs[rd] = imm;
    printf("do_MOV: x%d = %d\n", rd, imm);
}

extern "C" void do_ADD(uint32_t rd, uint32_t rs1, uint32_t rs2) {
    if (rd < 16 && rs1 < 16 && rs2 < 16) {
        cpu.regs[rd] = cpu.regs[rs1] + cpu.regs[rs2];
        printf("do_ADD: x%d = x%d + x%d = %d\n", rd, rs1, rs2, cpu.regs[rd]);
    }
}

extern "C" void do_SUB(uint32_t rd, uint32_t rs1, uint32_t rs2) {
    if (rd < 16 && rs1 < 16 && rs2 < 16) {
        cpu.regs[rd] = cpu.regs[rs1] - cpu.regs[rs2];
        printf("do_SUB: x%d = x%d - x%d = %d\n", rd, rs1, rs2, cpu.regs[rd]);
    }
}

extern "C" void do_MUL(uint32_t rd, uint32_t rs1, uint32_t rs2) {
    if (rd < 16 && rs1 < 16 && rs2 < 16) {
        cpu.regs[rd] = cpu.regs[rs1] * cpu.regs[rs2];
        printf("do_MUL: x%d = x%d * x%d = %d\n", rd, rs1, rs2, cpu.regs[rd]);
    }
}

extern "C" void do_AND(uint32_t rd, uint32_t rs1, uint32_t rs2) {
    if (rd < 16 && rs1 < 16 && rs2 < 16) {
        cpu.regs[rd] = cpu.regs[rs1] & cpu.regs[rs2];
        printf("do_AND: x%d = x%d & x%d = %d\n", rd, rs1, rs2, cpu.regs[rd]);
    }
}

extern "C" void do_OR(uint32_t rd, uint32_t rs1, uint32_t rs2) {
    if (rd < 16 && rs1 < 16 && rs2 < 16) {
        cpu.regs[rd] = cpu.regs[rs1] | cpu.regs[rs2];
        printf("do_OR: x%d = x%d | x%d = %d\n", rd, rs1, rs2, cpu.regs[rd]);
    }
}

extern "C" void do_XOR(uint32_t rd, uint32_t rs1, uint32_t rs2) {
    if (rd < 16 && rs1 < 16 && rs2 < 16) {
        cpu.regs[rd] = cpu.regs[rs1] ^ cpu.regs[rs2];
        printf("do_XOR: x%d = x%d ^ x%d = %d\n", rd, rs1, rs2, cpu.regs[rd]);
    }
}

extern "C" void do_CMP(uint32_t rd, uint32_t rs1, uint32_t rs2) {
    if (rd < 16 && rs1 < 16 && rs2 < 16) {
        cpu.regs[rd] = (cpu.regs[rs1] == cpu.regs[rs2]) ? 1 : 0;
        printf("do_CMP: x%d = (x%d == x%d) -> %d\n", rd, rs1, rs2, cpu.regs[rd]);
    }
}

extern "C" void do_INIT_FIELD() {
    printf("do_INIT_FIELD\n");
    for (int y = 0; y < 64; y++) {
        for (int x = 0; x < 64; x++) {
            cpu.current[y][x] = 0;
            cpu.next[y][x] = 0;
        }
    }
}

extern "C" void do_RANDOM_INIT() {
    printf("do_RANDOM_INIT\n");
    for (int y = 0; y < 64; y++) {
        for (int x = 0; x < 64; x++) {
            if (rand() % 100 < 30) {
                cpu.current[y][x] = (rand() % 2) ? 2 : 1;
            }
        }
    }
}

extern "C" void do_CALC_NEIGHBORS(uint32_t y_reg, uint32_t x_reg) {
    // Получаем значения из регистров
    uint32_t y = cpu.regs[y_reg];
    uint32_t x = cpu.regs[x_reg];
    
    cpu.neighbors = 0;
    cpu.count1 = 0;
    cpu.count2 = 0;
    
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dy == 0 && dx == 0) continue;
            int ny = (y + dy + 64) % 64;
            int nx = (x + dx + 64) % 64;
            
            if (cpu.current[ny][nx] == 1) {
                cpu.neighbors++;
                cpu.count1++;
            } else if (cpu.current[ny][nx] == 2) {
                cpu.neighbors++;
                cpu.count2++;
            }
        }
    }
    printf("do_CALC_NEIGHBORS: x%d=%d, x%d=%d -> neighbors=%d\n", y_reg, y, x_reg, x, cpu.neighbors);
}

extern "C" void do_UPDATE_CELL(uint32_t y_reg, uint32_t x_reg) {
    // Получаем значения из регистров
    uint32_t y = cpu.regs[y_reg];
    uint32_t x = cpu.regs[x_reg];
    
    if (cpu.current[y][x] > 0) {
        if (cpu.neighbors == 2 || cpu.neighbors == 3) {
            cpu.next[y][x] = cpu.current[y][x];
        } else {
            cpu.next[y][x] = 0;
        }
    } else {
        if (cpu.neighbors == 3) {
            if (cpu.count1 > cpu.count2) {
                cpu.next[y][x] = 1;
            } else {
                cpu.next[y][x] = 2;
            }
        } else {
            cpu.next[y][x] = 0;
        }
    }
    printf("do_UPDATE_CELL: x%d=%d, x%d=%d %d -> %d\n", y_reg, y, x_reg, x, cpu.current[y][x], cpu.next[y][x]);
}

extern "C" void do_SWAP_BUFFERS() {
    printf("do_SWAP_BUFFERS\n");
    for (int y = 0; y < 64; y++) {
        for (int x = 0; x < 64; x++) {
            cpu.current[y][x] = cpu.next[y][x];
        }
    }
}

extern "C" void do_DRAW_FIELD() {
    printf("do_DRAW_FIELD\n");
    for (int y = 0; y < 64; y++) {
        for (int x = 0; x < 64; x++) {
            int color;
            if (cpu.current[y][x] == 1) {
                color = 0xC71585;
            } else if (cpu.current[y][x] == 2) {
                color = 0x00FF00;
            } else {
                color = 0x000000;
            }
            simFillRect(x * 4, y * 4, 4, 4, color);
        }
    }
}

extern "C" void do_DELAY(uint32_t ms) {
    printf("do_DELAY: %d ms\n", ms);
    simDelay(ms);
}

extern "C" void do_FLUSH() {
    printf("do_FLUSH\n");
    simFlush();
}

extern "C" void do_CHECK_FINISH(uint32_t result_reg) {
    cpu.regs[result_reg] = checkFinish();
    printf("do_CHECK_FINISH: x%d = %d\n", result_reg, cpu.regs[result_reg]);
}

// Парсер ассемблера
std::vector<std::unique_ptr<ASM::Instruction>> parseAssembly(const std::string& filename) {
    std::vector<std::unique_ptr<ASM::Instruction>> program;
    std::ifstream file(filename);
    std::string line;
    
    std::unordered_map<std::string, uint32_t> labels;
    
    // Первый проход: сбор меток
    uint32_t pc = 0;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string token;
        if (!(iss >> token)) continue;
        
        if (token.back() == ':') {
            std::string label = token.substr(0, token.size() - 1);
            labels[label] = pc;
            printf("Found label: %s at PC %d\n", label.c_str(), pc);
        } else if (token != "CMP") { // Пропускаем CMP - это псевдоинструкция
            pc++;
        }
    }
    
    // Второй проход: разбор инструкций
    file.clear();
    file.seekg(0);
    pc = 0;
    
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string opcode;
        if (!(iss >> opcode)) continue;
        
        if (opcode.back() == ':') continue;
        
        printf("Parsing: %s\n", line.c_str());
        
        if (opcode == "MOV") {
            uint8_t rd;
            uint32_t imm;
            std::string reg, imm_str;
            iss >> reg >> imm_str;
            
            // Убираем 'x' из имени регистра
            rd = std::stoi(reg.substr(1));
            
            // Парсим immediate значение
            imm = std::stoi(imm_str);
            
            program.push_back(std::make_unique<ASM::MOV>(rd, imm));
            printf("Added MOV x%d %d\n", rd, imm);
        }

        else if (opcode == "ADD") {
            uint8_t rd, rs1, rs2;
            std::string reg1, reg2, reg3;
            iss >> reg1 >> reg2 >> reg3;
            rd = std::stoi(reg1.substr(1));
            rs1 = std::stoi(reg2.substr(1));
            if (reg3[0] == 'x') {
                rs2 = std::stoi(reg3.substr(1));
                program.push_back(std::make_unique<ASM::ADD>(rd, rs1, rs2));
                printf("Added ADD x%d x%d x%d\n", rd, rs1, rs2);
            } else {
                // Если это число, создаем MOV + ADD
                uint32_t imm = std::stoi(reg3);
                program.push_back(std::make_unique<ASM::MOV>(15, imm)); // используем временный регистр x15
                program.push_back(std::make_unique<ASM::ADD>(rd, rs1, 15));
                printf("Added ADD x%d x%d %d (via MOV+ADD)\n", rd, rs1, imm);
            }
        }
        else if (opcode == "SUB") {
            uint8_t rd, rs1, rs2;
            std::string reg1, reg2, reg3;
            iss >> reg1 >> reg2 >> reg3;
            rd = std::stoi(reg1.substr(1));
            rs1 = std::stoi(reg2.substr(1));
            rs2 = std::stoi(reg3.substr(1));
            program.push_back(std::make_unique<ASM::SUB>(rd, rs1, rs2));
            printf("Added SUB x%d x%d x%d\n", rd, rs1, rs2);
        }
        else if (opcode == "MUL") {
            uint8_t rd, rs1, rs2;
            std::string reg1, reg2, reg3;
            iss >> reg1 >> reg2 >> reg3;
            rd = std::stoi(reg1.substr(1));
            rs1 = std::stoi(reg2.substr(1));
            rs2 = std::stoi(reg3.substr(1));
            program.push_back(std::make_unique<ASM::MUL>(rd, rs1, rs2));
            printf("Added MUL x%d x%d x%d\n", rd, rs1, rs2);
        }
        else if (opcode == "JMP") {
            std::string label;
            iss >> label;
            if (labels.find(label) != labels.end()) {
                program.push_back(std::make_unique<ASM::JMP>(labels[label]));
                printf("Added JMP %s (PC=%d)\n", label.c_str(), labels[label]);
            } else {
                printf("Warning: Label %s not found\n", label.c_str());
            }
        }
        else if (opcode == "JZ") {
            uint8_t cond;
            std::string reg, label;
            iss >> reg >> label;
            cond = std::stoi(reg.substr(1));
            if (labels.find(label) != labels.end()) {
                program.push_back(std::make_unique<ASM::JZ>(cond, labels[label]));
                printf("Added JZ x%d %s (PC=%d)\n", cond, label.c_str(), labels[label]);
            } else {
                printf("Warning: Label %s not found\n", label.c_str());
            }
        }
        else if (opcode == "JNZ") {
            uint8_t cond;
            std::string reg, label;
            iss >> reg >> label;
            cond = std::stoi(reg.substr(1));
            if (labels.find(label) != labels.end()) {
                program.push_back(std::make_unique<ASM::JNZ>(cond, labels[label]));
                printf("Added JNZ x%d %s (PC=%d)\n", cond, label.c_str(), labels[label]);
            } else {
                printf("Warning: Label %s not found\n", label.c_str());
            }
        }
        else if (opcode == "HALT") {
            program.push_back(std::make_unique<ASM::HALT>());
            printf("Added HALT\n");
        }
        else if (opcode == "INIT_FIELD") {
            program.push_back(std::make_unique<ASM::INIT_FIELD>());
            printf("Added INIT_FIELD\n");
        }
        else if (opcode == "RANDOM_INIT") {
            program.push_back(std::make_unique<ASM::RANDOM_INIT_INSTR>());
            printf("Added RANDOM_INIT\n");
        }
        else if (opcode == "CALC_NEIGHBORS") {
            uint8_t y, x;
            std::string reg1, reg2;
            iss >> reg1 >> reg2;
            y = std::stoi(reg1.substr(1));
            x = std::stoi(reg2.substr(1));
            program.push_back(std::make_unique<ASM::CALC_NEIGHBORS>(y, x));
            printf("Added CALC_NEIGHBORS x%d x%d\n", y, x);
        }
        else if (opcode == "UPDATE_CELL") {
            uint8_t y, x;
            std::string reg1, reg2;
            iss >> reg1 >> reg2;
            y = std::stoi(reg1.substr(1));
            x = std::stoi(reg2.substr(1));
            program.push_back(std::make_unique<ASM::UPDATE_CELL>(y, x));
            printf("Added UPDATE_CELL x%d x%d\n", y, x);
        }
        else if (opcode == "SWAP_BUFFERS") {
            program.push_back(std::make_unique<ASM::SWAP_BUFFERS>());
            printf("Added SWAP_BUFFERS\n");
        }
        else if (opcode == "DRAW_FIELD") {
            program.push_back(std::make_unique<ASM::DRAW_FIELD>());
            printf("Added DRAW_FIELD\n");
        }
        else if (opcode == "DELAY") {
            uint32_t ms;
            iss >> ms;
            program.push_back(std::make_unique<ASM::DELAY>(ms));
            printf("Added DELAY %d\n", ms);
        }
        else if (opcode == "FLUSH") {
            program.push_back(std::make_unique<ASM::FLUSH>());
            printf("Added FLUSH\n");
        }
        else if (opcode == "CHECK_FINISH") {
            uint8_t rd;
            std::string reg;
            iss >> reg;
            rd = std::stoi(reg.substr(1));
            program.push_back(std::make_unique<ASM::CHECK_FINISH>(rd));
            printf("Added CHECK_FINISH x%d\n", rd);
        }
        else if (opcode == "CMP") {
            uint8_t rd, rs1, rs2;
            std::string reg1, reg2, reg3;
            iss >> reg1 >> reg2 >> reg3;
            
            rd = std::stoi(reg1.substr(1));
            rs1 = std::stoi(reg2.substr(1));
            
            if (reg3[0] == 'x') {
                rs2 = std::stoi(reg3.substr(1));
            } else {
                // Если сравниваем с числом, используем временный регистр
                uint32_t imm = std::stoi(reg3);
                program.push_back(std::make_unique<ASM::MOV>(14, imm)); // x14 - временный
                rs2 = 14;
            }
            
            program.push_back(std::make_unique<ASM::CMP>(rd, rs1, rs2));
            printf("Added CMP x%d x%d x%d\n", rd, rs1, rs2);
        }
        else {
            printf("Unknown instruction: %s\n", opcode.c_str());
            continue;
        }
        
        pc++;
    }
    
    printf("Parsed %zu instructions\n", program.size());
    return program;
}

// Простая генерация LLVM IR только с вызовами функций
void generateIR(const std::vector<std::unique_ptr<ASM::Instruction>>& program, const std::string& outputFile) {
    llvm::LLVMContext context;
    llvm::Module module("game_of_life_ir", context);
    llvm::IRBuilder<> builder(context);
    
    // Типы
    llvm::Type* voidType = llvm::Type::getVoidTy(context);
    llvm::Type* i32Type = llvm::Type::getInt32Ty(context);
    
    // Функция main
    llvm::FunctionType* mainType = llvm::FunctionType::get(voidType, false);
    llvm::Function* mainFunc = llvm::Function::Create(mainType, llvm::Function::ExternalLinkage, "main", &module);
    
    llvm::BasicBlock* entryBB = llvm::BasicBlock::Create(context, "entry", mainFunc);
    builder.SetInsertPoint(entryBB);
    
    // Декларации функций-эмуляторов
    std::vector<llvm::Type*> tripleParams = {i32Type, i32Type, i32Type};
    std::vector<llvm::Type*> doubleParams = {i32Type, i32Type};
    std::vector<llvm::Type*> singleParams = {i32Type};
    
    llvm::FunctionType* tripleFuncType = llvm::FunctionType::get(voidType, tripleParams, false);
    llvm::FunctionType* doubleFuncType = llvm::FunctionType::get(voidType, doubleParams, false);
    llvm::FunctionType* singleFuncType = llvm::FunctionType::get(voidType, singleParams, false);
    llvm::FunctionType* voidFuncType = llvm::FunctionType::get(voidType, false);
    
    // Создаем declarations для всех функций
    llvm::Function::Create(tripleFuncType, llvm::Function::ExternalLinkage, "do_MOV", &module);
    llvm::Function::Create(tripleFuncType, llvm::Function::ExternalLinkage, "do_ADD", &module);
    llvm::Function::Create(tripleFuncType, llvm::Function::ExternalLinkage, "do_SUB", &module);
    llvm::Function::Create(tripleFuncType, llvm::Function::ExternalLinkage, "do_MUL", &module);
    llvm::Function::Create(tripleFuncType, llvm::Function::ExternalLinkage, "do_AND", &module);
    llvm::Function::Create(tripleFuncType, llvm::Function::ExternalLinkage, "do_OR", &module);
    llvm::Function::Create(tripleFuncType, llvm::Function::ExternalLinkage, "do_XOR", &module);
    llvm::Function::Create(doubleFuncType, llvm::Function::ExternalLinkage, "do_CALC_NEIGHBORS", &module);
    llvm::Function::Create(doubleFuncType, llvm::Function::ExternalLinkage, "do_UPDATE_CELL", &module);
    llvm::Function::Create(singleFuncType, llvm::Function::ExternalLinkage, "do_DELAY", &module);
    llvm::Function::Create(singleFuncType, llvm::Function::ExternalLinkage, "do_CHECK_FINISH", &module);
    llvm::Function::Create(voidFuncType, llvm::Function::ExternalLinkage, "do_INIT_FIELD", &module);
    llvm::Function::Create(voidFuncType, llvm::Function::ExternalLinkage, "do_RANDOM_INIT", &module);
    llvm::Function::Create(voidFuncType, llvm::Function::ExternalLinkage, "do_SWAP_BUFFERS", &module);
    llvm::Function::Create(voidFuncType, llvm::Function::ExternalLinkage, "do_DRAW_FIELD", &module);
    llvm::Function::Create(voidFuncType, llvm::Function::ExternalLinkage, "do_FLUSH", &module);
    llvm::Function::Create(tripleFuncType, llvm::Function::ExternalLinkage, "do_CMP", &module);

    
    // Генерация кода для каждой инструкции
    for (const auto& instr : program) {
        if (auto mov = dynamic_cast<ASM::MOV*>(instr.get())) {
            llvm::Value* args[] = {
                llvm::ConstantInt::get(i32Type, mov->rd),
                llvm::ConstantInt::get(i32Type, 0), // rs1 не используется
                llvm::ConstantInt::get(i32Type, mov->imm)
            };
            builder.CreateCall(module.getFunction("do_MOV"), args);
        }
        else if (auto add = dynamic_cast<ASM::ADD*>(instr.get())) {
            llvm::Value* args[] = {
                llvm::ConstantInt::get(i32Type, add->rd),
                llvm::ConstantInt::get(i32Type, add->rs1),
                llvm::ConstantInt::get(i32Type, add->rs2)
            };
            builder.CreateCall(module.getFunction("do_ADD"), args);
        }
        else if (auto sub = dynamic_cast<ASM::SUB*>(instr.get())) {
            llvm::Value* args[] = {
                llvm::ConstantInt::get(i32Type, sub->rd),
                llvm::ConstantInt::get(i32Type, sub->rs1),
                llvm::ConstantInt::get(i32Type, sub->rs2)
            };
            builder.CreateCall(module.getFunction("do_SUB"), args);
        }
        else if (auto mul = dynamic_cast<ASM::MUL*>(instr.get())) {
            llvm::Value* args[] = {
                llvm::ConstantInt::get(i32Type, mul->rd),
                llvm::ConstantInt::get(i32Type, mul->rs1),
                llvm::ConstantInt::get(i32Type, mul->rs2)
            };
            builder.CreateCall(module.getFunction("do_MUL"), args);
        }
        else if (dynamic_cast<ASM::INIT_FIELD*>(instr.get())) {
            builder.CreateCall(module.getFunction("do_INIT_FIELD"));
        }
        else if (dynamic_cast<ASM::RANDOM_INIT_INSTR*>(instr.get())) {
            builder.CreateCall(module.getFunction("do_RANDOM_INIT"));
        }
        else if (auto calc_neighbors = dynamic_cast<ASM::CALC_NEIGHBORS*>(instr.get())) {
            llvm::Value* args[] = {
                llvm::ConstantInt::get(i32Type, calc_neighbors->y_reg),
                llvm::ConstantInt::get(i32Type, calc_neighbors->x_reg)
            };
            builder.CreateCall(module.getFunction("do_CALC_NEIGHBORS"), args);
        }
        else if (auto update_cell = dynamic_cast<ASM::UPDATE_CELL*>(instr.get())) {
            llvm::Value* args[] = {
                llvm::ConstantInt::get(i32Type, update_cell->y_reg),
                llvm::ConstantInt::get(i32Type, update_cell->x_reg)
            };
            builder.CreateCall(module.getFunction("do_UPDATE_CELL"), args);
        }
        else if (dynamic_cast<ASM::SWAP_BUFFERS*>(instr.get())) {
            builder.CreateCall(module.getFunction("do_SWAP_BUFFERS"));
        }
        else if (dynamic_cast<ASM::DRAW_FIELD*>(instr.get())) {
            builder.CreateCall(module.getFunction("do_DRAW_FIELD"));
        }
        else if (auto delay = dynamic_cast<ASM::DELAY*>(instr.get())) {
            llvm::Value* args[] = {
                llvm::ConstantInt::get(i32Type, delay->ms)
            };
            builder.CreateCall(module.getFunction("do_DELAY"), args);
        }
        else if (dynamic_cast<ASM::FLUSH*>(instr.get())) {
            builder.CreateCall(module.getFunction("do_FLUSH"));
        }
        else if (auto check_finish = dynamic_cast<ASM::CHECK_FINISH*>(instr.get())) {
            llvm::Value* args[] = {
                llvm::ConstantInt::get(i32Type, check_finish->result_reg)
            };
            builder.CreateCall(module.getFunction("do_CHECK_FINISH"), args);
        }
        else if (dynamic_cast<ASM::HALT*>(instr.get())) {
            builder.CreateRetVoid();
            break; // После HALT дальше код не выполняется
        }
        else if (auto cmp = dynamic_cast<ASM::CMP*>(instr.get())) {
            llvm::Value* args[] = {
                llvm::ConstantInt::get(i32Type, cmp->rd),
                llvm::ConstantInt::get(i32Type, cmp->rs1),
                llvm::ConstantInt::get(i32Type, cmp->rs2)
            };
            builder.CreateCall(module.getFunction("do_CMP"), args);
        }
    }
    
    // Если не было HALT, добавляем возврат
    if (!entryBB->getTerminator()) {
        builder.CreateRetVoid();
    }
    
    // Вывод IR в файл
    std::error_code ec;
    llvm::raw_fd_ostream out(outputFile, ec);
    if (ec) {
        std::cerr << "Error opening output file: " << ec.message() << "\n";
        return;
    }
    
    module.print(out, nullptr);
    std::cout << "Generated LLVM IR in: " << outputFile << "\n";
    
    // Валидация
    bool verified = llvm::verifyModule(module, &llvm::outs());
    std::cout << "Module verification: " << (verified ? "FAILED" : "PASSED") << "\n";
}

// Простая интерпретация без IR
void interpret(const std::vector<std::unique_ptr<ASM::Instruction>>& program) {
    cpu.pc = 0;
    cpu.running = true;

    int max_cycles=1000;
    int i = 0;
    
    while (cpu.running && cpu.pc < program.size()) {
        std::cout << "PC: " << cpu.pc << " - ";
        program[cpu.pc]->execute(cpu);
        
        // Автоинкремент PC, если инструкция не изменила его
        if (!dynamic_cast<ASM::JMP*>(program[cpu.pc].get()) && 
            !dynamic_cast<ASM::JZ*>(program[cpu.pc].get()) && 
            !dynamic_cast<ASM::JNZ*>(program[cpu.pc].get())) {
            cpu.pc++;
        }

        i++;
        if (i >= max_cycles) {
            std::cout << "Max cycles reached, stopping interpretation.\n";
            break;
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <input.asm> <output.ll>\n";
        return 1;
    }
    
    // Парсинг ассемблера
    auto program = parseAssembly(argv[1]);
    if (program.empty()) {
        std::cerr << "No instructions parsed!\n";
        return 1;
    }
    
    // Генерация IR
    generateIR(program, argv[2]);
    
    // Интерпретация
    std::cout << "\n=== Starting Interpretation ===\n";
    simInit();
    interpret(program);
    simExit();
    
    std::cout << "\n=== Interpretation Finished ===\n";
    return 0;
}