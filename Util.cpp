#include "Util.h"
#include <fstream>
#include <iostream>
#include <unordered_map>

bool parseConfigFile(const std::string& configPath, Config& config) {
    std::cout << "Config path: " << configPath << std::endl; // Debugging

    std::ifstream file(configPath);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << configPath << std::endl;
        return false;
    }

    std::string line;
    while(getline(file, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }

        size_t equalPos = line.find('=');

        std::string key = line.substr(0, equalPos);
        std::string value = line.substr(equalPos + 1);

        if (key == "vm_exec_slice_in_instructions") {
            config.vm_exec_slice_in_instructions = std::stoi(value);
        } else if (key == "vm_binary") {
            config.vm_binary = value;
        } else {
            std::cerr << "Unknown file key: " << key << std::endl;
        }
    }
    file.close();
    return true;
}

bool parseAssemblyFile(const std::string &assemblyPath, /* output */ std::vector<Instruction>& assemblyLines) {
    std::ifstream file(assemblyPath);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << assemblyPath << std::endl;
        return false;
    }

    std::string line;
    while(getline(file, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }

        Instruction instruction {};

        if (line == "DUMP_PROCESSOR_STATE") { // Because this line doesn't contain a space.
            instruction.instructionType = InstructionType::DUMP_PROCESSOR_STATE;
            assemblyLines.push_back(instruction);
            continue;
        }

        size_t spacePos = line.find(' ');
        std::string opcode = line.substr(0, spacePos);
        std::string operands = line.substr(spacePos + 1);
        instruction.instructionType = getInstructionType(opcode);
        // TODO - Change, some ops are out of order.
        if (instruction.instructionType != InstructionType::INVALID) {
            if (instruction.instructionType == InstructionType::LI) { // TODO - change, since mul etc are like li
                size_t commaPos = operands.find(',');
                parseRegisterNumber(operands.substr(0, commaPos), instruction.operand1);
                try {
                    instruction.operand2 = std::stoi(operands.substr(commaPos + 1));
                } catch (const std::exception&) {
                    std::cerr << "Couldn't parse value for li instr: " << operands.substr(commaPos + 1) << std::endl;
                }
            } else {
                // TODO - do rest of regular operations
            }
        }
        assemblyLines.push_back(instruction);
    }
    file.close();
    return true;
}

InstructionType getInstructionType(const std::string& opcode) {
    static const std::unordered_map<std::string, InstructionType> opcodeMap = {
            {"li", InstructionType::LI},
            {"add", InstructionType::ADD},
            {"addi", InstructionType::ADDI},
            {"and", InstructionType::AND},
            {"mul", InstructionType::MUL},
            {"or", InstructionType::OR},
            {"sll", InstructionType::SLL},
            {"srl", InstructionType::SRL},
            {"sub", InstructionType::SUB},
            {"xor", InstructionType::XOR}
    };

    auto it = opcodeMap.find(opcode);
    if (it != opcodeMap.end()) {
        return it->second;
    } else {
        std::cerr << "Couldn't parse MIPS opcode: " << it->first << std::endl;
        return InstructionType::INVALID;
    }
}

bool parseRegisterNumber(const std::string &reg, uint32_t &regNum) {
    if (reg.empty() || reg[0] != '$') {
        return false;
    }
    try {
        regNum = std::stoi(reg.substr(1));
        return true;
    } catch (const std::exception&) {
        return false;
    }
}
