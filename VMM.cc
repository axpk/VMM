/**
 * C++ 20
*/
#include <iostream>
#include <utility>
#include <vector>
#include <string>
#include <array>
#include <memory>
#include <fstream>

enum class InstructionType {
    ADD,
    SUB,
    ADDI,
    MUL,
    AND,
    OR,
    XOR,
    SLL,
    SRL,
    LI,
    DUMP_PROCESSOR_STATE,
    INVALID
};

struct Instruction {
    InstructionType instructionType = InstructionType::INVALID;
    uint32_t dest = 0;
    uint32_t operand1 = 0;
    uint32_t operand2 = 0;
    uint32_t immediate = 0;
};

struct Config {
    int vm_exec_slice_in_instructions = 0;
    std::string vm_binary;
};

class CPU {
private:
    std::array<uint32_t, 32> registers;
    uint32_t pc;
public:
    CPU() : pc(0) {
        registers.fill(0);
    }
};

class VM {
private:
    Config config;
    std::unique_ptr<CPU> cpu;
public:
    VM(Config c) : config(std::move(c)){
        this->cpu = std::make_unique<CPU>();
    }
};

class Hypervisor {
private:
    std::vector<std::unique_ptr<VM>> vms;
public:
    Hypervisor() = default;
    void createVM(const Config& config) {
       std::unique_ptr<VM> vm = std::make_unique<VM>(config);
       vms.push_back(std::move(vm));
    }
};

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

int main(int argc, char* argv[]) {
    std::vector<std::string> vmFiles;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        std::cout << arg << std::endl;
        if (arg == "-v" && i + 1 < argc) { // VM Files
            vmFiles.push_back(argv[++i]);
        } else {
            std::cerr << "No filename given after -v flag" << std::endl;
        }
    }

    if (vmFiles.empty()) {
        std::cerr << "No VM files provided. Use -v to specify files." << std::endl;
        return 1;
    }

    Hypervisor hypervisor;

    for (const auto& configPath : vmFiles) {
        Config config;
        if (!parseConfigFile(configPath, config)) {
            std::cerr << "Error parsing config assembly file" << std::endl;
            return 1;
        }
        hypervisor.createVM(config);
    }

    return 0;
}