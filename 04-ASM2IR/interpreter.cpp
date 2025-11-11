#include "sim.h"
#include "isa.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <memory>
#include <unordered_map>

using ASM::CPUState;

// Глобальное состояние
CPUState cpu;

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

// Интерпретатор
void interpret(const std::vector<std::unique_ptr<ASM::Instruction>>& program) {
    cpu.pc = 0;
    cpu.running = true;
    
    const int MAX_CYCLES = 1000000;
    int cycle_count = 0;
    
    while (cpu.running && cpu.pc < program.size() && cycle_count < MAX_CYCLES) {
        uint32_t current_pc = cpu.pc;
        
        // Выполняем инструкцию
        program[cpu.pc]->execute(cpu);
        
        // Увеличиваем PC, если инструкция не изменила его
        if (!dynamic_cast<ASM::JMP*>(program[current_pc].get()) && 
            !dynamic_cast<ASM::JZ*>(program[current_pc].get()) && 
            !dynamic_cast<ASM::JNZ*>(program[current_pc].get())) {
            cpu.pc++;
        }
        
        cycle_count++;
    }
    
    if (cycle_count >= MAX_CYCLES) {
        std::cout << "Max cycles reached\n";
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input.asm>\n";
        return 1;
    }
    
    auto program = parseAssembly(argv[1]);
    if (program.empty()) {
        std::cerr << "No instructions parsed!\n";
        return 1;
    }
    
    std::cout << "Starting interpretation...\n";
    simInit();
    interpret(program);
    simExit();
    
    std::cout << "Interpretation finished\n";
    return 0;
}