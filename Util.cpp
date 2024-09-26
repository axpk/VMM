#include "Util.h"
#include <fstream>
#include <iostream>

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

// TODO - Change to parse directly to instruction
bool parseAssemblyFile(const std::string &assemblyPath, std::vector<Instruction>& assemblyLines) {
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

        if (line == "DUMP_PROCESSOR_STATE") {
            instruction.instructionType = InstructionType::DUMP_PROCESSOR_STATE;
            assemblyLines.push_back(instruction);
            continue;
        }

        // TODO - handle all other instruction types


    }
    file.close();
    return true;
}
