#include "sim.h"
#include "isa.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/raw_ostream.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <memory>
#include <unordered_map>

using ASM::CPUState;

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
        
        if (token == ";") continue;
        
        if (token.back() == ':') {
            std::string label = token.substr(0, token.size() - 1);
            labels[label] = pc;
        } else {
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
        
        if (opcode == ";") continue;
        
        if (opcode.back() == ':') {
            if (iss >> opcode) {
                // Обрабатываем инструкцию после метки
            } else {
                continue;
            }
        }
        
        if (opcode == "MOV") {
            uint8_t rd, rs;
            std::string reg1, reg2;
            iss >> reg1 >> reg2;
            rd = std::stoi(reg1.substr(1));
            rs = std::stoi(reg2.substr(1));
            program.push_back(std::make_unique<ASM::MOV>(rd, rs));
        }
        else if (opcode == "MOVI") {
            uint8_t rd;
            uint32_t imm;
            std::string reg, imm_str;
            iss >> reg >> imm_str;
            rd = std::stoi(reg.substr(1));
            imm = std::stoi(imm_str);
            program.push_back(std::make_unique<ASM::MOVI>(rd, imm));
        }
        else if (opcode == "ADD") {
            uint8_t rd, rs1, rs2;
            std::string reg1, reg2, reg3;
            iss >> reg1 >> reg2 >> reg3;
            rd = std::stoi(reg1.substr(1));
            rs1 = std::stoi(reg2.substr(1));
            rs2 = std::stoi(reg3.substr(1));
            program.push_back(std::make_unique<ASM::ADD>(rd, rs1, rs2));
        }
        else if (opcode == "ADDI") {
            uint8_t rd, rs;
            uint32_t imm;
            std::string reg1, reg2, imm_str;
            iss >> reg1 >> reg2 >> imm_str;
            rd = std::stoi(reg1.substr(1));
            rs = std::stoi(reg2.substr(1));
            imm = std::stoi(imm_str);
            program.push_back(std::make_unique<ASM::ADDI>(rd, rs, imm));
        }
        else if (opcode == "CMP_LT") {
            uint8_t rd, rs1, rs2;
            std::string reg1, reg2, reg3;
            iss >> reg1 >> reg2 >> reg3;
            rd = std::stoi(reg1.substr(1));
            rs1 = std::stoi(reg2.substr(1));
            rs2 = std::stoi(reg3.substr(1));
            program.push_back(std::make_unique<ASM::CMP_LT>(rd, rs1, rs2));
        }
        else if (opcode == "JMP") {
            std::string label;
            iss >> label;
            program.push_back(std::make_unique<ASM::JMP>(labels[label]));
        }
        else if (opcode == "JNZ") {
            uint8_t cond;
            std::string reg, label;
            iss >> reg >> label;
            cond = std::stoi(reg.substr(1));
            program.push_back(std::make_unique<ASM::JNZ>(cond, labels[label]));
        }
        else if (opcode == "CALL") {
            std::string func;
            iss >> func;
            program.push_back(std::make_unique<ASM::CALL>(func));
        }
        else if (opcode == "CALC_NEIGHBORS") {
            uint8_t y, x, res;
            std::string reg1, reg2, reg3;
            iss >> reg1 >> reg2 >> reg3;
            y = std::stoi(reg1.substr(1));
            x = std::stoi(reg2.substr(1));
            res = std::stoi(reg3.substr(1));
            program.push_back(std::make_unique<ASM::CALC_NEIGHBORS>(y, x, res));
        }
        else if (opcode == "UPDATE_CELL") {
            uint8_t y, x, c1, c2;
            std::string reg1, reg2, reg3, reg4;
            iss >> reg1 >> reg2 >> reg3 >> reg4;
            y = std::stoi(reg1.substr(1));
            x = std::stoi(reg2.substr(1));
            c1 = std::stoi(reg3.substr(1));
            c2 = std::stoi(reg4.substr(1));
            program.push_back(std::make_unique<ASM::UPDATE_CELL>(y, x, c1, c2));
        }
        else if (opcode == "HALT") {
            program.push_back(std::make_unique<ASM::HALT>());
        }
        
        pc++;
    }
    
    return program;
}

// Генерация нормального LLVM IR
void generateIR(const std::vector<std::unique_ptr<ASM::Instruction>>& program, const std::string& outputFile) {
    llvm::LLVMContext context;
    llvm::Module module("game_of_life", context);
    llvm::IRBuilder<> builder(context);
    
    // Типы
    llvm::Type* voidType = llvm::Type::getVoidTy(context);
    llvm::Type* i32Type = llvm::Type::getInt32Ty(context);
    llvm::Type* i1Type = llvm::Type::getInt1Ty(context);
    
    // Глобальные переменные для состояния процессора
    llvm::ArrayType* regsType = llvm::ArrayType::get(i32Type, 16);
    llvm::GlobalVariable* regs = new llvm::GlobalVariable(
        module, regsType, false, llvm::GlobalValue::CommonLinkage,
        llvm::ConstantAggregateZero::get(regsType), "regs");
    
    llvm::ArrayType* fieldType = llvm::ArrayType::get(llvm::ArrayType::get(i32Type, 64), 64);
    llvm::GlobalVariable* current = new llvm::GlobalVariable(
        module, fieldType, false, llvm::GlobalValue::CommonLinkage,
        llvm::ConstantAggregateZero::get(fieldType), "current");
    
    llvm::GlobalVariable* next = new llvm::GlobalVariable(
        module, fieldType, false, llvm::GlobalValue::CommonLinkage,
        llvm::ConstantAggregateZero::get(fieldType), "next");
    
    llvm::GlobalVariable* temp = new llvm::GlobalVariable(
        module, i32Type, false, llvm::GlobalValue::CommonLinkage,
        llvm::ConstantInt::get(i32Type, 0), "temp");
    
    // Внешние функции
    llvm::FunctionType* fillRectType = llvm::FunctionType::get(voidType, {i32Type, i32Type, i32Type, i32Type, i32Type}, false);
    llvm::FunctionCallee fillRectFunc = module.getOrInsertFunction("simFillRect", fillRectType);
    
    llvm::FunctionType* flushType = llvm::FunctionType::get(voidType, false);
    llvm::FunctionCallee flushFunc = module.getOrInsertFunction("simFlush", flushType);
    
    llvm::FunctionType* delayType = llvm::FunctionType::get(voidType, {i32Type}, false);
    llvm::FunctionCallee delayFunc = module.getOrInsertFunction("simDelay", delayType);
    
    llvm::FunctionType* checkFinishType = llvm::FunctionType::get(i32Type, false);
    llvm::FunctionCallee checkFinishFunc = module.getOrInsertFunction("checkFinish", checkFinishType);
    
    // Функция main
    llvm::FunctionType* mainType = llvm::FunctionType::get(voidType, false);
    llvm::Function* mainFunc = llvm::Function::Create(mainType, llvm::Function::ExternalLinkage, "main", &module);
    
    // Базовые блоки
    std::vector<llvm::BasicBlock*> blocks;
    for (size_t i = 0; i < program.size(); i++) {
        blocks.push_back(llvm::BasicBlock::Create(context, "block_" + std::to_string(i), mainFunc));
    }
    llvm::BasicBlock* exitBlock = llvm::BasicBlock::Create(context, "exit", mainFunc);
    
    // Вспомогательные функции для доступа к регистрам и памяти
    auto getRegPtr = [&](uint32_t reg) {
        std::vector<llvm::Value*> indices = {
            llvm::ConstantInt::get(i32Type, 0),
            llvm::ConstantInt::get(i32Type, reg)
        };
        return builder.CreateGEP(regsType, regs, indices);
    };
    
    auto getFieldPtr = [&](llvm::GlobalVariable* field, llvm::Value* y, llvm::Value* x) {
        std::vector<llvm::Value*> indices = {
            llvm::ConstantInt::get(i32Type, 0),
            y,
            x
        };
        return builder.CreateGEP(fieldType, field, indices);
    };
    
    // Начинаем генерацию кода
    builder.SetInsertPoint(blocks[0]);
    
    for (size_t i = 0; i < program.size(); i++) {
        builder.SetInsertPoint(blocks[i]);
        
        const auto& instr = program[i];
        
        if (auto mov = dynamic_cast<ASM::MOV*>(instr.get())) {
            llvm::Value* srcPtr = getRegPtr(mov->rs);
            llvm::Value* dstPtr = getRegPtr(mov->rd);
            llvm::Value* val = builder.CreateLoad(i32Type, srcPtr);
            builder.CreateStore(val, dstPtr);
        }
        else if (auto movi = dynamic_cast<ASM::MOVI*>(instr.get())) {
            llvm::Value* dstPtr = getRegPtr(movi->rd);
            llvm::Value* imm = llvm::ConstantInt::get(i32Type, movi->imm);
            builder.CreateStore(imm, dstPtr);
        }
        else if (auto add = dynamic_cast<ASM::ADD*>(instr.get())) {
            llvm::Value* src1Ptr = getRegPtr(add->rs1);
            llvm::Value* src2Ptr = getRegPtr(add->rs2);
            llvm::Value* dstPtr = getRegPtr(add->rd);
            
            llvm::Value* val1 = builder.CreateLoad(i32Type, src1Ptr);
            llvm::Value* val2 = builder.CreateLoad(i32Type, src2Ptr);
            llvm::Value* result = builder.CreateAdd(val1, val2);
            builder.CreateStore(result, dstPtr);
        }
        else if (auto addi = dynamic_cast<ASM::ADDI*>(instr.get())) {
            llvm::Value* srcPtr = getRegPtr(addi->rs);
            llvm::Value* dstPtr = getRegPtr(addi->rd);
            
            llvm::Value* val = builder.CreateLoad(i32Type, srcPtr);
            llvm::Value* imm = llvm::ConstantInt::get(i32Type, addi->imm);
            llvm::Value* result = builder.CreateAdd(val, imm);
            builder.CreateStore(result, dstPtr);
        }
        else if (auto cmp_lt = dynamic_cast<ASM::CMP_LT*>(instr.get())) {
            llvm::Value* src1Ptr = getRegPtr(cmp_lt->rs1);
            llvm::Value* src2Ptr = getRegPtr(cmp_lt->rs2);
            llvm::Value* dstPtr = getRegPtr(cmp_lt->rd);
            
            llvm::Value* val1 = builder.CreateLoad(i32Type, src1Ptr);
            llvm::Value* val2 = builder.CreateLoad(i32Type, src2Ptr);
            llvm::Value* cmp = builder.CreateICmpSLT(val1, val2);
            llvm::Value* result = builder.CreateZExt(cmp, i32Type);
            builder.CreateStore(result, dstPtr);
        }
        else if (auto jmp = dynamic_cast<ASM::JMP*>(instr.get())) {
            builder.CreateBr(blocks[jmp->target]);
            continue;
        }
        else if (auto jnz = dynamic_cast<ASM::JNZ*>(instr.get())) {
            llvm::Value* condPtr = getRegPtr(jnz->cond);
            llvm::Value* cond = builder.CreateLoad(i32Type, condPtr);
            llvm::Value* condBool = builder.CreateICmpNE(cond, llvm::ConstantInt::get(i32Type, 0));
            
            builder.CreateCondBr(condBool, blocks[jnz->target], blocks[i + 1]);
            continue;
        }
        else if (auto call = dynamic_cast<ASM::CALL*>(instr.get())) {
            if (call->func == "draw_field") {
                // Генерация кода для отрисовки поля
                llvm::BasicBlock* yLoopHeader = llvm::BasicBlock::Create(context, "draw_y_loop", mainFunc);
                llvm::BasicBlock* yLoopBody = llvm::BasicBlock::Create(context, "draw_y_body", mainFunc);
                llvm::BasicBlock* yLoopEnd = llvm::BasicBlock::Create(context, "draw_y_end", mainFunc);
                llvm::BasicBlock* xLoopHeader = llvm::BasicBlock::Create(context, "draw_x_loop", mainFunc);
                llvm::BasicBlock* xLoopBody = llvm::BasicBlock::Create(context, "draw_x_body", mainFunc);
                llvm::BasicBlock* xLoopEnd = llvm::BasicBlock::Create(context, "draw_x_end", mainFunc);
                
                // Внешний цикл по y
                llvm::Value* yPtr = getRegPtr(0); // Используем x0 как счетчик
                builder.CreateStore(llvm::ConstantInt::get(i32Type, 0), yPtr);
                builder.CreateBr(yLoopHeader);
                
                builder.SetInsertPoint(yLoopHeader);
                llvm::Value* y = builder.CreateLoad(i32Type, yPtr);
                llvm::Value* yCond = builder.CreateICmpSLT(y, llvm::ConstantInt::get(i32Type, 64));
                builder.CreateCondBr(yCond, yLoopBody, yLoopEnd);
                
                // Тело внешнего цикла
                builder.SetInsertPoint(yLoopBody);
                llvm::Value* xPtr = getRegPtr(1); // Используем x1 как счетчик
                builder.CreateStore(llvm::ConstantInt::get(i32Type, 0), xPtr);
                builder.CreateBr(xLoopHeader);
                
                // Внутренний цикл по x
                builder.SetInsertPoint(xLoopHeader);
                llvm::Value* x = builder.CreateLoad(i32Type, xPtr);
                llvm::Value* xCond = builder.CreateICmpSLT(x, llvm::ConstantInt::get(i32Type, 64));
                builder.CreateCondBr(xCond, xLoopBody, xLoopEnd);
                
                // Тело внутреннего цикла - отрисовка ячейки
                builder.SetInsertPoint(xLoopBody);
                
                // Получаем значение ячейки
                llvm::Value* cellPtr = getFieldPtr(current, y, x);
                llvm::Value* cellVal = builder.CreateLoad(i32Type, cellPtr);
                
                // Вычисляем цвет
                llvm::Value* isAlive1 = builder.CreateICmpEQ(cellVal, llvm::ConstantInt::get(i32Type, 1));
                llvm::Value* isAlive2 = builder.CreateICmpEQ(cellVal, llvm::ConstantInt::get(i32Type, 2));
                
                llvm::Value* color1 = llvm::ConstantInt::get(i32Type, 0xC71585);
                llvm::Value* color2 = llvm::ConstantInt::get(i32Type, 0x00FF00);
                llvm::Value* colorDead = llvm::ConstantInt::get(i32Type, 0x000000);
                
                llvm::Value* color = builder.CreateSelect(isAlive1, color1, 
                                    builder.CreateSelect(isAlive2, color2, colorDead));
                
                // Вычисляем координаты
                llvm::Value* xCoord = builder.CreateMul(x, llvm::ConstantInt::get(i32Type, 4));
                llvm::Value* yCoord = builder.CreateMul(y, llvm::ConstantInt::get(i32Type, 4));
                
                // Вызываем simFillRect
                llvm::Value* args[] = {xCoord, yCoord, 
                                      llvm::ConstantInt::get(i32Type, 4),
                                      llvm::ConstantInt::get(i32Type, 4),
                                      color};
                builder.CreateCall(fillRectFunc, args);
                
                // Инкремент x
                llvm::Value* xInc = builder.CreateAdd(x, llvm::ConstantInt::get(i32Type, 1));
                builder.CreateStore(xInc, xPtr);
                builder.CreateBr(xLoopHeader);
                
                // Конец внутреннего цикла
                builder.SetInsertPoint(xLoopEnd);
                
                // Инкремент y
                llvm::Value* yInc = builder.CreateAdd(y, llvm::ConstantInt::get(i32Type, 1));
                builder.CreateStore(yInc, yPtr);
                builder.CreateBr(yLoopHeader);
                
                // Конец внешнего цикла
                builder.SetInsertPoint(yLoopEnd);
            }
            else if (call->func == "flush") {
                builder.CreateCall(flushFunc);
            }
            else if (call->func == "delay") {
                llvm::Value* args[] = {llvm::ConstantInt::get(i32Type, 100)};
                builder.CreateCall(delayFunc, args);
            }
            else if (call->func == "check_finish") {
                llvm::Value* result = builder.CreateCall(checkFinishFunc);
                llvm::Value* resultPtr = getRegPtr(15); // x15 для результата
                builder.CreateStore(result, resultPtr);
            }
            else if (call->func == "swap_buffers") {
                // Генерация кода для обмена буферов
                llvm::BasicBlock* swapYHeader = llvm::BasicBlock::Create(context, "swap_y_loop", mainFunc);
                llvm::BasicBlock* swapYBody = llvm::BasicBlock::Create(context, "swap_y_body", mainFunc);
                llvm::BasicBlock* swapYEnd = llvm::BasicBlock::Create(context, "swap_y_end", mainFunc);
                llvm::BasicBlock* swapXHeader = llvm::BasicBlock::Create(context, "swap_x_loop", mainFunc);
                llvm::BasicBlock* swapXBody = llvm::BasicBlock::Create(context, "swap_x_body", mainFunc);
                llvm::BasicBlock* swapXEnd = llvm::BasicBlock::Create(context, "swap_x_end", mainFunc);
                
                // Внешний цикл по y
                llvm::Value* yPtr = getRegPtr(0);
                builder.CreateStore(llvm::ConstantInt::get(i32Type, 0), yPtr);
                builder.CreateBr(swapYHeader);
                
                builder.SetInsertPoint(swapYHeader);
                llvm::Value* y = builder.CreateLoad(i32Type, yPtr);
                llvm::Value* yCond = builder.CreateICmpSLT(y, llvm::ConstantInt::get(i32Type, 64));
                builder.CreateCondBr(yCond, swapYBody, swapYEnd);
                
                builder.SetInsertPoint(swapYBody);
                llvm::Value* xPtr = getRegPtr(1);
                builder.CreateStore(llvm::ConstantInt::get(i32Type, 0), xPtr);
                builder.CreateBr(swapXHeader);
                
                // Внутренний цикл по x
                builder.SetInsertPoint(swapXHeader);
                llvm::Value* x = builder.CreateLoad(i32Type, xPtr);
                llvm::Value* xCond = builder.CreateICmpSLT(x, llvm::ConstantInt::get(i32Type, 64));
                builder.CreateCondBr(xCond, swapXBody, swapXEnd);
                
                // Тело внутреннего цикла - копирование next в current
                builder.SetInsertPoint(swapXBody);
                llvm::Value* nextPtr = getFieldPtr(next, y, x);
                llvm::Value* currentPtr = getFieldPtr(current, y, x);
                llvm::Value* nextVal = builder.CreateLoad(i32Type, nextPtr);
                builder.CreateStore(nextVal, currentPtr);
                
                // Инкремент x
                llvm::Value* xInc = builder.CreateAdd(x, llvm::ConstantInt::get(i32Type, 1));
                builder.CreateStore(xInc, xPtr);
                builder.CreateBr(swapXHeader);
                
                builder.SetInsertPoint(swapXEnd);
                
                // Инкремент y
                llvm::Value* yInc = builder.CreateAdd(y, llvm::ConstantInt::get(i32Type, 1));
                builder.CreateStore(yInc, yPtr);
                builder.CreateBr(swapYHeader);
                
                builder.SetInsertPoint(swapYEnd);
            }
        }
        else if (auto calc = dynamic_cast<ASM::CALC_NEIGHBORS*>(instr.get())) {
            // Генерация кода для вычисления соседей
            // (аналогично draw_field, но с вычислением соседей)
            // Это сложная операция, для простоты оставим заглушку
            llvm::Value* resultPtr = getRegPtr(calc->result_reg);
            builder.CreateStore(llvm::ConstantInt::get(i32Type, 0), resultPtr);
        }
        else if (auto update = dynamic_cast<ASM::UPDATE_CELL*>(instr.get())) {
            // Генерация кода для обновления ячейки
            // (аналогично, оставим заглушку)
        }
        else if (dynamic_cast<ASM::HALT*>(instr.get())) {
            builder.CreateBr(exitBlock);
            continue;
        }
        
        // Переход к следующему блоку
        if (i < program.size() - 1) {
            builder.CreateBr(blocks[i + 1]);
        } else {
            builder.CreateBr(exitBlock);
        }
    }
    
    // Завершающий блок
    builder.SetInsertPoint(exitBlock);
    builder.CreateRetVoid();
    
    // Вывод IR
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

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <input.asm> <output.ll>\n";
        return 1;
    }
    
    auto program = parseAssembly(argv[1]);
    generateIR(program, argv[2]);
    return 0;
}