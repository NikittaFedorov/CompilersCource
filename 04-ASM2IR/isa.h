#ifndef ISA_H
#define ISA_H

#include <cstdint>
#include <string>
#include <functional>
#include <unordered_map>
#include "sim.h"

namespace ASM {
    struct CPUState {
        // Регистры общего назначения
        uint32_t regs[16] = {0};
        
        // Специальные регистры
        uint32_t pc = 0;
        bool running = true;
        
        // Память для поля игры
        uint32_t current[64][64] = {0};
        uint32_t next[64][64] = {0};
        
        // Временные переменные для вычислений
        uint32_t temp = 0;
    };

    class Instruction {
    public:
        virtual ~Instruction() = default;
        virtual void execute(CPUState& cpu) = 0;
        virtual std::string toString() const = 0;
    };

    // ========== АРИФМЕТИЧЕСКИЕ ИНСТРУКЦИИ ==========

    class MOV : public Instruction {
    public:
        uint8_t rd, rs;
        MOV(uint8_t r, uint8_t s) : rd(r), rs(s) {}
        void execute(CPUState& cpu) override {
            cpu.regs[rd] = cpu.regs[rs];
        }
        std::string toString() const override {
            return "MOV x" + std::to_string(rd) + " x" + std::to_string(rs);
        }
    };

    class MOVI : public Instruction {
    public:
        uint8_t rd;
        uint32_t imm;
        MOVI(uint8_t r, uint32_t i) : rd(r), imm(i) {}
        void execute(CPUState& cpu) override {
            cpu.regs[rd] = imm;
        }
        std::string toString() const override {
            return "MOVI x" + std::to_string(rd) + " " + std::to_string(imm);
        }
    };

    class ADD : public Instruction {
    public:
        uint8_t rd, rs1, rs2;
        ADD(uint8_t d, uint8_t s1, uint8_t s2) : rd(d), rs1(s1), rs2(s2) {}
        void execute(CPUState& cpu) override {
            cpu.regs[rd] = cpu.regs[rs1] + cpu.regs[rs2];
        }
        std::string toString() const override {
            return "ADD x" + std::to_string(rd) + " x" + std::to_string(rs1) + " x" + std::to_string(rs2);
        }
    };

    class ADDI : public Instruction {
    public:
        uint8_t rd, rs;
        uint32_t imm;
        ADDI(uint8_t d, uint8_t s, uint32_t i) : rd(d), rs(s), imm(i) {}
        void execute(CPUState& cpu) override {
            cpu.regs[rd] = cpu.regs[rs] + imm;
        }
        std::string toString() const override {
            return "ADDI x" + std::to_string(rd) + " x" + std::to_string(rs) + " " + std::to_string(imm);
        }
    };

    class SUB : public Instruction {
    public:
        uint8_t rd, rs1, rs2;
        SUB(uint8_t d, uint8_t s1, uint8_t s2) : rd(d), rs1(s1), rs2(s2) {}
        void execute(CPUState& cpu) override {
            cpu.regs[rd] = cpu.regs[rs1] - cpu.regs[rs2];
        }
        std::string toString() const override {
            return "SUB x" + std::to_string(rd) + " x" + std::to_string(rs1) + " x" + std::to_string(rs2);
        }
    };

    class MUL : public Instruction {
    public:
        uint8_t rd, rs1, rs2;
        MUL(uint8_t d, uint8_t s1, uint8_t s2) : rd(d), rs1(s1), rs2(s2) {}
        void execute(CPUState& cpu) override {
            cpu.regs[rd] = cpu.regs[rs1] * cpu.regs[rs2];
        }
        std::string toString() const override {
            return "MUL x" + std::to_string(rd) + " x" + std::to_string(rs1) + " x" + std::to_string(rs2);
        }
    };

    class AND : public Instruction {
    public:
        uint8_t rd, rs1, rs2;
        AND(uint8_t d, uint8_t s1, uint8_t s2) : rd(d), rs1(s1), rs2(s2) {}
        void execute(CPUState& cpu) override {
            cpu.regs[rd] = cpu.regs[rs1] & cpu.regs[rs2];
        }
        std::string toString() const override {
            return "AND x" + std::to_string(rd) + " x" + std::to_string(rs1) + " x" + std::to_string(rs2);
        }
    };

    class OR : public Instruction {
    public:
        uint8_t rd, rs1, rs2;
        OR(uint8_t d, uint8_t s1, uint8_t s2) : rd(d), rs1(s1), rs2(s2) {}
        void execute(CPUState& cpu) override {
            cpu.regs[rd] = cpu.regs[rs1] | cpu.regs[rs2];
        }
        std::string toString() const override {
            return "OR x" + std::to_string(rd) + " x" + std::to_string(rs1) + " x" + std::to_string(rs2);
        }
    };

    class XOR : public Instruction {
    public:
        uint8_t rd, rs1, rs2;
        XOR(uint8_t d, uint8_t s1, uint8_t s2) : rd(d), rs1(s1), rs2(s2) {}
        void execute(CPUState& cpu) override {
            cpu.regs[rd] = cpu.regs[rs1] ^ cpu.regs[rs2];
        }
        std::string toString() const override {
            return "XOR x" + std::to_string(rd) + " x" + std::to_string(rs1) + " x" + std::to_string(rs2);
        }
    };

    // ========== ИНСТРУКЦИИ СРАВНЕНИЯ ==========

    class CMP : public Instruction {
    public:
        uint8_t rd, rs1, rs2;
        CMP(uint8_t d, uint8_t s1, uint8_t s2) : rd(d), rs1(s1), rs2(s2) {}
        void execute(CPUState& cpu) override {
            cpu.regs[rd] = (cpu.regs[rs1] == cpu.regs[rs2]) ? 1 : 0;
        }
        std::string toString() const override {
            return "CMP x" + std::to_string(rd) + " x" + std::to_string(rs1) + " x" + std::to_string(rs2);
        }
    };

    class CMPI : public Instruction {
    public:
        uint8_t rd, rs;
        uint32_t imm;
        CMPI(uint8_t d, uint8_t s, uint32_t i) : rd(d), rs(s), imm(i) {}
        void execute(CPUState& cpu) override {
            cpu.regs[rd] = (cpu.regs[rs] == imm) ? 1 : 0;
        }
        std::string toString() const override {
            return "CMPI x" + std::to_string(rd) + " x" + std::to_string(rs) + " " + std::to_string(imm);
        }
    };

    class CMP_LT : public Instruction {
    public:
        uint8_t rd, rs1, rs2;
        CMP_LT(uint8_t d, uint8_t s1, uint8_t s2) : rd(d), rs1(s1), rs2(s2) {}
        void execute(CPUState& cpu) override {
            cpu.regs[rd] = (cpu.regs[rs1] < cpu.regs[rs2]) ? 1 : 0;
        }
        std::string toString() const override {
            return "CMP_LT x" + std::to_string(rd) + " x" + std::to_string(rs1) + " x" + std::to_string(rs2);
        }
    };

    class CMP_GT : public Instruction {
    public:
        uint8_t rd, rs1, rs2;
        CMP_GT(uint8_t d, uint8_t s1, uint8_t s2) : rd(d), rs1(s1), rs2(s2) {}
        void execute(CPUState& cpu) override {
            cpu.regs[rd] = (cpu.regs[rs1] > cpu.regs[rs2]) ? 1 : 0;
        }
        std::string toString() const override {
            return "CMP_GT x" + std::to_string(rd) + " x" + std::to_string(rs1) + " x" + std::to_string(rs2);
        }
    };

    // ========== ИНСТРУКЦИИ ПЕРЕХОДА ==========

    class JMP : public Instruction {
    public:
        uint32_t target;
        JMP(uint32_t t) : target(t) {}
        void execute(CPUState& cpu) override {
            cpu.pc = target;
        }
        std::string toString() const override {
            return "JMP " + std::to_string(target);
        }
    };

    class JZ : public Instruction {
    public:
        uint8_t cond;
        uint32_t target;
        JZ(uint8_t c, uint32_t t) : cond(c), target(t) {}
        void execute(CPUState& cpu) override {
            if (cpu.regs[cond] == 0) {
                cpu.pc = target;
            } else {
                cpu.pc++;
            }
        }
        std::string toString() const override {
            return "JZ x" + std::to_string(cond) + " " + std::to_string(target);
        }
    };

    class JNZ : public Instruction {
    public:
        uint8_t cond;
        uint32_t target;
        JNZ(uint8_t c, uint32_t t) : cond(c), target(t) {}
        void execute(CPUState& cpu) override {
            if (cpu.regs[cond] != 0) {
                cpu.pc = target;
            } else {
                cpu.pc++;
            }
        }
        std::string toString() const override {
            return "JNZ x" + std::to_string(cond) + " " + std::to_string(target);
        }
    };

    // ========== ИНСТРУКЦИИ РАБОТЫ С ПАМЯТЬЮ ==========

    class LOAD : public Instruction {
    public:
        uint8_t rd;
        uint32_t addr;
        LOAD(uint8_t r, uint32_t a) : rd(r), addr(a) {}
        void execute(CPUState& cpu) override {
            // Для простоты - загружаем из фиксированной памяти
            uint32_t y = addr / 64;
            uint32_t x = addr % 64;
            cpu.regs[rd] = cpu.current[y][x];
        }
        std::string toString() const override {
            return "LOAD x" + std::to_string(rd) + " [" + std::to_string(addr) + "]";
        }
    };

    class STORE : public Instruction {
    public:
        uint32_t addr;
        uint8_t rs;
        STORE(uint32_t a, uint8_t r) : addr(a), rs(r) {}
        void execute(CPUState& cpu) override {
            uint32_t y = addr / 64;
            uint32_t x = addr % 64;
            cpu.next[y][x] = cpu.regs[rs];
        }
        std::string toString() const override {
            return "STORE [" + std::to_string(addr) + "] x" + std::to_string(rs);
        }
    };

    // ========== СПЕЦИАЛЬНЫЕ ИНСТРУКЦИИ ==========

    class HALT : public Instruction {
    public:
        void execute(CPUState& cpu) override {
            cpu.running = false;
        }
        std::string toString() const override {
            return "HALT";
        }
    };

    class CALL : public Instruction {
    public:
        std::string func;
        CALL(const std::string& f) : func(f) {}
        void execute(CPUState& cpu) override {
            if (func == "init_field") {
                for (int y = 0; y < 64; y++) {
                    for (int x = 0; x < 64; x++) {
                        cpu.current[y][x] = 0;
                        cpu.next[y][x] = 0;
                    }
                }
            }
            else if (func == "random_init") {
                for (int y = 0; y < 64; y++) {
                    for (int x = 0; x < 64; x++) {
                        if (rand() % 100 < 30) {
                            cpu.current[y][x] = (rand() % 2) ? 2 : 1;
                        }
                    }
                }
            }
            else if (func == "draw_field") {
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
            else if (func == "swap_buffers") {
                for (int y = 0; y < 64; y++) {
                    for (int x = 0; x < 64; x++) {
                        cpu.current[y][x] = cpu.next[y][x];
                    }
                }
            }
            else if (func == "flush") {
                simFlush();
            }
            else if (func == "delay") {
                simDelay(100);
            }
            else if (func == "check_finish") {
                cpu.regs[15] = checkFinish(); // Используем последний регистр для результата
            }
        }
        std::string toString() const override {
            return "CALL " + func;
        }
    };

    // ========== ИНСТРУКЦИИ ДЛЯ ИГРЫ ЖИЗНЬ ==========

    class CALC_NEIGHBORS : public Instruction {
    public:
        uint8_t y_reg, x_reg, result_reg;
        CALC_NEIGHBORS(uint8_t y, uint8_t x, uint8_t res) : y_reg(y), x_reg(x), result_reg(res) {}
        void execute(CPUState& cpu) override {
            int y = cpu.regs[y_reg];
            int x = cpu.regs[x_reg];
            
            int neighbors = 0;
            int count1 = 0;
            int count2 = 0;
            
            for (int dy = -1; dy <= 1; dy++) {
                for (int dx = -1; dx <= 1; dx++) {
                    if (dy == 0 && dx == 0) continue;
                    int ny = (y + dy + 64) % 64;
                    int nx = (x + dx + 64) % 64;
                    
                    if (cpu.current[ny][nx] == 1) {
                        neighbors++;
                        count1++;
                    } else if (cpu.current[ny][nx] == 2) {
                        neighbors++;
                        count2++;
                    }
                }
            }
            
            // Сохраняем результаты во временные переменные
            cpu.temp = neighbors;
            cpu.regs[result_reg] = count1; // Используем result_reg для count1
            cpu.regs[result_reg + 1] = count2; // А следующий регистр для count2
        }
        std::string toString() const override {
            return "CALC_NEIGHBORS x" + std::to_string(y_reg) + " x" + std::to_string(x_reg) + " x" + std::to_string(result_reg);
        }
    };

    class UPDATE_CELL : public Instruction {
    public:
        uint8_t y_reg, x_reg, count1_reg, count2_reg;
        UPDATE_CELL(uint8_t y, uint8_t x, uint8_t c1, uint8_t c2) : y_reg(y), x_reg(x), count1_reg(c1), count2_reg(c2) {}
        void execute(CPUState& cpu) override {
            int y = cpu.regs[y_reg];
            int x = cpu.regs[x_reg];
            int neighbors = cpu.temp;
            int count1 = cpu.regs[count1_reg];
            int count2 = cpu.regs[count2_reg];
            
            if (cpu.current[y][x] > 0) {
                if (neighbors == 2 || neighbors == 3) {
                    cpu.next[y][x] = cpu.current[y][x];
                } else {
                    cpu.next[y][x] = 0;
                }
            } else {
                if (neighbors == 3) {
                    if (count1 > count2) {
                        cpu.next[y][x] = 1;
                    } else {
                        cpu.next[y][x] = 2;
                    }
                } else {
                    cpu.next[y][x] = 0;
                }
            }
        }
        std::string toString() const override {
            return "UPDATE_CELL x" + std::to_string(y_reg) + " x" + std::to_string(x_reg) + " x" + std::to_string(count1_reg) + " x" + std::to_string(count2_reg);
        }
    };
}

#endif