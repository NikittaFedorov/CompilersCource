#include "isa_definition.h"
#include <map>
#include <sstream>
#include <algorithm>
#include <cctype>

std::map<std::string, InsnId_t> instructionMap;
std::map<InsnId_t, std::string> instructionNameMap;

void initializeInstructionMaps() {
    // Арифметические операции
    instructionMap["ADD"] = ISA_ADD;
    instructionMap["SUB"] = ISA_SUB;
    instructionMap["MUL"] = ISA_MUL;
    instructionMap["DIV"] = ISA_DIV;
    instructionMap["MOD"] = ISA_MOD;
    
    instructionMap["ADDI"] = ISA_ADDI;
    instructionMap["SUBI"] = ISA_SUBI;
    instructionMap["MULI"] = ISA_MULI;
    instructionMap["DIVI"] = ISA_DIVI;
    instructionMap["MODI"] = ISA_MODI;
    
    // Логические операции
    instructionMap["AND"] = ISA_AND;
    instructionMap["OR"] = ISA_OR;
    instructionMap["XOR"] = ISA_XOR;
    instructionMap["NOT"] = ISA_NOT;
    instructionMap["ANDI"] = ISA_ANDI;
    instructionMap["ORI"] = ISA_ORI;
    instructionMap["XORI"] = ISA_XORI;
    
    // Операции сравнения
    instructionMap["CMP_EQ"] = ISA_CMP_EQ;
    instructionMap["CMP_NE"] = ISA_CMP_NE;
    instructionMap["CMP_LT"] = ISA_CMP_LT;
    instructionMap["CMP_LE"] = ISA_CMP_LE;
    instructionMap["CMP_GT"] = ISA_CMP_GT;
    instructionMap["CMP_GE"] = ISA_CMP_GE;
    instructionMap["CMP_EQI"] = ISA_CMP_EQI;
    instructionMap["CMP_NEI"] = ISA_CMP_NEI;
    instructionMap["CMP_LTI"] = ISA_CMP_LTI;
    instructionMap["CMP_LEI"] = ISA_CMP_LEI;
    instructionMap["CMP_GTI"] = ISA_CMP_GTI;
    instructionMap["CMP_GEI"] = ISA_CMP_GEI;
    
    // Операции с памятью
    instructionMap["LOAD"] = ISA_LOAD;
    instructionMap["STORE"] = ISA_STORE;
    instructionMap["LOAD_IND"] = ISA_LOAD_IND;
    instructionMap["STORE_IND"] = ISA_STORE_IND;
    instructionMap["MOV"] = ISA_MOV;
    instructionMap["MOVI"] = ISA_MOVI;
    
    // Операции сдвига
    instructionMap["SHL"] = ISA_SHL;
    instructionMap["SHR"] = ISA_SHR;
    instructionMap["SHLI"] = ISA_SHLI;
    instructionMap["SHRI"] = ISA_SHRI;
    
    // Управление потоком
    instructionMap["JMP"] = ISA_JMP;
    instructionMap["JMP_COND"] = ISA_JMP_COND;
    instructionMap["JMP_EQ"] = ISA_JMP_EQ;
    instructionMap["JMP_NE"] = ISA_JMP_NE;
    instructionMap["JMP_LT"] = ISA_JMP_LT;
    instructionMap["JMP_LE"] = ISA_JMP_LE;
    instructionMap["JMP_GT"] = ISA_JMP_GT;
    instructionMap["JMP_GE"] = ISA_JMP_GE;
    instructionMap["CALL"] = ISA_CALL;
    instructionMap["RET"] = ISA_RET;
    
    // Специальные инструкции
    instructionMap["INIT_FIELD"] = ISA_INIT_FIELD;
    instructionMap["RAND"] = ISA_RAND;
    instructionMap["DRAW_CELL"] = ISA_DRAW_CELL;
    instructionMap["FLUSH"] = ISA_FLUSH;
    instructionMap["DELAY"] = ISA_DELAY;
    instructionMap["CHECK_EXIT"] = ISA_CHECK_EXIT;
    instructionMap["EXIT"] = ISA_EXIT;
    
    // Отладочные инструкции
    instructionMap["DUMP_REG"] = ISA_DUMP_REG;
    instructionMap["DUMP_MEM"] = ISA_DUMP_MEM;
    
    // Обратное отображение
    for (const auto& pair : instructionMap) {
        instructionNameMap[pair.second] = pair.first;
    }
}

bool parseMemoryAddress(const std::string& addr_str, MemoryAddress& addr) {
    std::string str = addr_str;
    
    // Удаляем квадратные скобки
    if (str.front() == '[' && str.back() == ']') {
        str = str.substr(1, str.size() - 2);
    }
    
    std::istringstream iss(str);
    std::string token;
    
    addr = MemoryAddress();
    
    while (std::getline(iss, token, '+')) {
        // Удаляем пробелы
        token.erase(std::remove_if(token.begin(), token.end(), ::isspace), token.end());
        
        if (token.empty()) continue;
        
        // Проверяем на регистр (начинается с r)
        if (token[0] == 'r') {
            if (addr.base_reg == 0) {
                // Первый регистр - базовый
                addr.base_reg = std::stoi(token.substr(1));
            } else {
                // Второй регистр - индексный
                addr.index_reg = std::stoi(token.substr(1));
                addr.has_index = true;
            }
        } 
        // Проверяем на масштабирование (регистр * число)
        else if (token.find('*') != std::string::npos) {
            std::istringstream scale_iss(token);
            std::string reg_part, scale_part;
            std::getline(scale_iss, reg_part, '*');
            std::getline(scale_iss, scale_part, '*');
            
            if (reg_part[0] == 'r') {
                addr.index_reg = std::stoi(reg_part.substr(1));
                addr.has_index = true;
                addr.scale = std::stoi(scale_part);
            }
        }
        // Проверяем на непосредственное значение
        else if (std::isdigit(token[0])) {
            addr.offset = std::stoi(token);
        }
    }
    
    return true;
}

std::string memoryAddressToString(const MemoryAddress& addr) {
    std::stringstream ss;
    ss << "[r" << addr.base_reg;
    
    if (addr.has_index) {
        ss << " + r" << addr.index_reg;
        if (addr.scale != 1) {
            ss << " * " << addr.scale;
        }
    }
    
    if (addr.offset != 0) {
        if (addr.has_index || addr.base_reg != 0) {
            ss << " + ";
        }
        ss << addr.offset;
    }
    
    ss << "]";
    return ss.str();
}