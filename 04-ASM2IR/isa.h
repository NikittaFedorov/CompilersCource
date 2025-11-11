#ifndef ISA_H
#define ISA_H

#include <cstdint>
#include <string>
#include <functional>
#include <unordered_map>
#include "sim.h"

// Переименовываем, чтобы не конфликтовать с LLVM
namespace ASM {
    struct CPUState {
        uint32_t regs[16] = {0};
        uint32_t pc = 0;
        bool running = true;
        uint32_t current[64][64] = {0};
        uint32_t next[64][64] = {0};
        uint32_t neighbors = 0;
        uint32_t count1 = 0;
        uint32_t count2 = 0;
        uint32_t last_flush_time = 0;
    };

    class Instruction {
    public:
        virtual ~Instruction() = default;
        virtual void execute(CPUState& cpu) = 0;
        virtual std::string toString() const = 0;
    };

    class MOV : public Instruction {
    public:  // Делаем публичными
        uint8_t rd, imm;
        MOV(uint8_t r, uint32_t i) : rd(r), imm(i) {}
        void execute(CPUState& cpu) override {
            cpu.regs[rd] = imm;
            printf("MOV: x%d = %d\n", rd, imm);
        }
        std::string toString() const override {
            return "MOV x" + std::to_string(rd) + " " + std::to_string(imm);
        }
    };

    class ADD : public Instruction {
    public:
        uint8_t rd, rs1, rs2;
        ADD(uint8_t d, uint8_t s1, uint8_t s2) : rd(d), rs1(s1), rs2(s2) {}
        void execute(CPUState& cpu) override {
            cpu.regs[rd] = cpu.regs[rs1] + cpu.regs[rs2];
            printf("ADD: x%d = x%d + x%d = %d\n", rd, rs1, rs2, cpu.regs[rd]);
        }
        std::string toString() const override {
            return "ADD x" + std::to_string(rd) + " x" + std::to_string(rs1) + " x" + std::to_string(rs2);
        }
    };

    class SUB : public Instruction {
    public:
        uint8_t rd, rs1, rs2;
        SUB(uint8_t d, uint8_t s1, uint8_t s2) : rd(d), rs1(s1), rs2(s2) {}
        void execute(CPUState& cpu) override {
            cpu.regs[rd] = cpu.regs[rs1] - cpu.regs[rs2];
            printf("SUB: x%d = x%d - x%d = %d\n", rd, rs1, rs2, cpu.regs[rd]);
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
            printf("MUL: x%d = x%d * x%d = %d\n", rd, rs1, rs2, cpu.regs[rd]);
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
            printf("AND: x%d = x%d & x%d = %d\n", rd, rs1, rs2, cpu.regs[rd]);
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
            printf("OR: x%d = x%d | x%d = %d\n", rd, rs1, rs2, cpu.regs[rd]);
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
            printf("XOR: x%d = x%d ^ x%d = %d\n", rd, rs1, rs2, cpu.regs[rd]);
        }
        std::string toString() const override {
            return "XOR x" + std::to_string(rd) + " x" + std::to_string(rs1) + " x" + std::to_string(rs2);
        }
    };

    class JMP : public Instruction {
    public:
        uint32_t target;
        JMP(uint32_t t) : target(t) {}
        void execute(CPUState& cpu) override {
            cpu.pc = target;  // Устанавливаем PC напрямую
            printf("JMP: -> %d\n", target);
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
                cpu.pc = target;  // Прыгаем на target
                printf("JZ: x%d == 0 -> jump to %d\n", cond, target);
            } else {
                cpu.pc++;  // Переходим к следующей инструкции
                printf("JZ: x%d != 0 -> continue to next instruction\n", cond);
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
                cpu.pc = target;  // Прыгаем на target
                printf("JNZ: x%d != 0 -> jump to %d\n", cond, target);
            } else {
                cpu.pc++;  // Переходим к следующей инструкции
                printf("JNZ: x%d == 0 -> continue to next instruction\n", cond);
            }
        }
        std::string toString() const override {
            return "JNZ x" + std::to_string(cond) + " " + std::to_string(target);
        }
    };

    class HALT : public Instruction {
    public:
        void execute(CPUState& cpu) override {
            cpu.running = false;
            printf("HALT\n");
        }
        std::string toString() const override {
            return "HALT";
        }
    };

    class INIT_FIELD : public Instruction {
    public:
        void execute(CPUState& cpu) override {
            printf("INIT_FIELD: Clearing field\n");
            for (int y = 0; y < 64; y++) {
                for (int x = 0; x < 64; x++) {
                    cpu.current[y][x] = 0;
                    cpu.next[y][x] = 0;
                }
            }
        }
        std::string toString() const override {
            return "INIT_FIELD";
        }
    };

    class RANDOM_INIT_INSTR : public Instruction {  // Переименовываем
    public:
        void execute(CPUState& cpu) override {
            printf("RANDOM_INIT: Randomizing field\n");
            for (int y = 0; y < 64; y++) {
                for (int x = 0; x < 64; x++) {
                    if (rand() % 100 < 30) {
                        cpu.current[y][x] = (rand() % 2) ? 2 : 1;
                    }
                }
            }
        }
        std::string toString() const override {
            return "RANDOM_INIT";
        }
    };

    class CALC_NEIGHBORS : public Instruction {
    public:
        uint8_t y_reg, x_reg;
        CALC_NEIGHBORS(uint8_t y, uint8_t x) : y_reg(y), x_reg(x) {}
        void execute(CPUState& cpu) override {
            int y = cpu.regs[y_reg];
            int x = cpu.regs[x_reg];
            
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
            printf("CALC_NEIGHBORS: (%d,%d) -> neighbors=%d, count1=%d, count2=%d\n", 
                   y, x, cpu.neighbors, cpu.count1, cpu.count2);
        }
        std::string toString() const override {
            return "CALC_NEIGHBORS x" + std::to_string(y_reg) + " x" + std::to_string(x_reg);
        }
    };

    class UPDATE_CELL : public Instruction {
    public:
        uint8_t y_reg, x_reg;
        UPDATE_CELL(uint8_t y, uint8_t x) : y_reg(y), x_reg(x) {}
        void execute(CPUState& cpu) override {
            int y = cpu.regs[y_reg];
            int x = cpu.regs[x_reg];
            
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
            printf("UPDATE_CELL: (%d,%d) %d -> %d\n", y, x, cpu.current[y][x], cpu.next[y][x]);
        }
        std::string toString() const override {
            return "UPDATE_CELL x" + std::to_string(y_reg) + " x" + std::to_string(x_reg);
        }
    };

    class SWAP_BUFFERS : public Instruction {
    public:
        void execute(CPUState& cpu) override {
            printf("SWAP_BUFFERS: Swapping current and next\n");
            for (int y = 0; y < 64; y++) {
                for (int x = 0; x < 64; x++) {
                    cpu.current[y][x] = cpu.next[y][x];
                }
            }
        }
        std::string toString() const override {
            return "SWAP_BUFFERS";
        }
    };

    class DRAW_FIELD : public Instruction {
    public:
        void execute(CPUState& cpu) override {
            printf("DRAW_FIELD: Drawing field\n");
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
        std::string toString() const override {
            return "DRAW_FIELD";
        }
    };

    class DELAY : public Instruction {
    public:
        uint32_t ms;
        DELAY(uint32_t m) : ms(m) {}
        void execute(CPUState& cpu) override {
            printf("DELAY: %d ms\n", ms);
            simDelay(ms);
        }
        std::string toString() const override {
            return "DELAY " + std::to_string(ms);
        }
    };

    class FLUSH : public Instruction {
    public:
        void execute(CPUState& cpu) override {
            printf("FLUSH\n");
            simFlush();
        }
        std::string toString() const override {
            return "FLUSH";
        }
    };

    class CHECK_FINISH : public Instruction {
    public:
        uint8_t result_reg;
        CHECK_FINISH(uint8_t r) : result_reg(r) {}
        void execute(CPUState& cpu) override {
            cpu.regs[result_reg] = checkFinish();
            printf("CHECK_FINISH: %d\n", cpu.regs[result_reg]);
        }
        std::string toString() const override {
            return "CHECK_FINISH x" + std::to_string(result_reg);
        }
    };

    class CMP : public Instruction {
    public:
        uint8_t rd, rs1, rs2;
        CMP(uint8_t d, uint8_t s1, uint8_t s2) : rd(d), rs1(s1), rs2(s2) {}
        void execute(CPUState& cpu) override {
            cpu.regs[rd] = (cpu.regs[rs1] == cpu.regs[rs2]) ? 1 : 0;
            printf("CMP: x%d = (x%d == x%d) -> %d\n", rd, rs1, rs2, cpu.regs[rd]);
        }
        std::string toString() const override {
            return "CMP x" + std::to_string(rd) + " x" + std::to_string(rs1) + " x" + std::to_string(rs2);
        }
    };

    class CMP_LT : public Instruction {
    public:
        uint8_t rd, rs1, rs2;
        CMP_LT(uint8_t d, uint8_t s1, uint8_t s2) : rd(d), rs1(s1), rs2(s2) {}
        void execute(CPUState& cpu) override {
            cpu.regs[rd] = (cpu.regs[rs1] < cpu.regs[rs2]) ? 1 : 0;
            printf("CMP_LT: x%d = (x%d < x%d) -> %d\n", rd, rs1, rs2, cpu.regs[rd]);
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
            printf("CMP_GT: x%d = (x%d > x%d) -> %d\n", rd, rs1, rs2, cpu.regs[rd]);
        }
        std::string toString() const override {
            return "CMP_GT x" + std::to_string(rd) + " x" + std::to_string(rs1) + " x" + std::to_string(rs2);
        }
    };
}

#endif