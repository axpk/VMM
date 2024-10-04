/**
 * C++ 20
 * Tested with g++ -std=c++20
*/
#include <iostream>
#include <utility>
#include <vector>
#include <string>
#include <array>
#include <memory>
#include <fstream>
#include <unordered_map>
#include <sstream>

enum class InstructionType {
    ADD,
    ADDU,
    SUB,
    SUBU,
    ADDI,
    ADDIU,
    MUL,
    MULT,
    DIV,
    AND,
    OR,
    ORI,
    XOR,
    SLL,
    SRL,
    LI,
    DUMP_PROCESSOR_STATE,
    SNAPSHOT,
    INVALID
};

struct Instruction {
    InstructionType instructionType = InstructionType::INVALID;
    std::vector<int> operands;
    std::string snapshotPath;
};

struct Config {
    int vm_exec_slice_in_instructions = 0;
    std::string vm_binary;
};

bool parseConfigFile(const std::string& configPath, Config& config) {
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

        std::cout << "Key: " << key << ", Value: " << value << std::endl;

        if (key == "vm_exec_slice_in_instructions") {
            try {
                config.vm_exec_slice_in_instructions = std::stoi(value);
            } catch (std::exception& e) {
                std::cerr << "Error stoi exec_slice_in_instructions in parseConfigFile" << std::endl;
            }
        } else if (key == "vm_binary") {
            config.vm_binary = value;
        } else {
            std::cerr << "Unknown file key: " << key << std::endl;
        }
    }
    file.close();
    return true;
}

InstructionType getInstructionType(const std::string& opcode) {
    static const std::unordered_map<std::string, InstructionType> opcodeMap = {
            {"li", InstructionType::LI},
            {"add", InstructionType::ADD},
            {"addi", InstructionType::ADDI},
            {"addu", InstructionType::ADDU},
            {"addiu", InstructionType::ADDIU},
            {"and", InstructionType::AND},
            {"mul", InstructionType::MUL},
            {"mult", InstructionType::MULT},
            {"div", InstructionType::DIV},
            {"or", InstructionType::OR},
            {"ori", InstructionType::ORI},
            {"sll", InstructionType::SLL},
            {"srl", InstructionType::SRL},
            {"sub", InstructionType::SUB},
            {"subu", InstructionType::SUBU},
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

int parseRegister(const std::string& operand) {
    auto keyLocation = operand.find('$');
    return static_cast<int>(std::stoi(operand.substr(keyLocation + 1))); // Register value
}

Instruction parseInstruction(const std::string& line) {
    std::istringstream iss(line);
    Instruction inst;

    std::string opcode;
    iss >> opcode;

    if (opcode.find("DUMP_PROCESSOR_STATE") != std::string::npos) {
        inst.instructionType = InstructionType::DUMP_PROCESSOR_STATE;
        return inst;
    }

    if (line.find("SNAPSHOT") != std::string::npos) {
        inst.instructionType = InstructionType::SNAPSHOT;
        auto keyLocation = line.find(' ');
        inst.snapshotPath = line.substr(keyLocation + 1);
        return inst;
    }

    inst.instructionType = getInstructionType(opcode);

    std::string operand;
    while(std::getline(iss, operand, ',')) {
        if (operand.find('$') != std::string::npos) {
            inst.operands.push_back(parseRegister(operand));
        } else {
            inst.operands.push_back(static_cast<int>(std::stoi(operand)));
            if (inst.instructionType == InstructionType::OR) { // Convert OR to ORI internally if immediate value
                inst.instructionType = InstructionType::ORI;
            }
        }
    }

    return inst;
}

class CPU {
private:
    std::array<int, 32> registers;
    uint32_t hi; // mult special register
    uint32_t lo; // mult special register
    uint32_t pc;
public:
    CPU() : pc(0) {
        registers.fill(0);
    }
    void execute(const Instruction& inst) {
        switch(inst.instructionType) {
            // ARITHMETIC
            case InstructionType::ADD:
                registers[inst.operands[0]] = registers[inst.operands[1]] + registers[inst.operands[2]];
                break;
            case InstructionType::SUB:
                registers[inst.operands[0]] = registers[inst.operands[1]] - registers[inst.operands[2]];
                break;
            case InstructionType::ADDI:
                registers[inst.operands[0]] = registers[inst.operands[1]] + inst.operands[2];
                break;
            case InstructionType::ADDU: {
                uint32_t op1 = static_cast<uint32_t>(registers[inst.operands[1]]);
                uint32_t op2 = static_cast<uint32_t>(registers[inst.operands[2]]);
                registers[inst.operands[0]] = static_cast<int>(op1) + static_cast<int>(op2);
                break;
            }
            case InstructionType::SUBU: {
                uint32_t op1 = static_cast<uint32_t>(registers[inst.operands[1]]);
                uint32_t op2 = static_cast<uint32_t>(registers[inst.operands[2]]);
                registers[inst.operands[0]] = static_cast<int>(op1) - static_cast<int>(op2);
            }
            case InstructionType::ADDIU: {
                uint32_t op1 = static_cast<uint32_t>(registers[inst.operands[1]]);
                uint32_t op2 = static_cast<uint32_t>(inst.operands[2]);
                registers[inst.operands[0]] = static_cast<int>(op1) + static_cast<int>(op2);
                break;
            }
            case InstructionType::MUL: { // Result in 32-bit integer
                int32_t ans = registers[inst.operands[1]] * registers[inst.operands[2]];
                registers[inst.operands[0]] = ans;
                break;
            }
            case InstructionType::MULT: {
                int64_t res = registers[inst.operands[1]] * registers[inst.operands[2]];
                this->hi = static_cast<int32_t>((res >> 32)); // upper 32 bits
                this->lo = static_cast<int32_t>(res); // lower 32 bits
                break;
            }
            case InstructionType::DIV: {
                if (registers[inst.operands[2]] == 0) {
                    std::cerr << "Divide by 0 error" << std::endl;
                    break;
                }

                this->lo = registers[inst.operands[1]] / registers[inst.operands[2]];
                this->hi = registers[inst.operands[1]] % registers[inst.operands[2]];
                break;
            }

            // LOGICAL
            case InstructionType::AND:
                registers[inst.operands[0]] = registers[inst.operands[1]] & registers[inst.operands[2]];
                break;
            case InstructionType::OR:
                registers[inst.operands[0]] = registers[inst.operands[1]] | registers[inst.operands[2]];
                break;
            case InstructionType::ORI:
                registers[inst.operands[0]] = registers[inst.operands[1]] | inst.operands[2];
                break;
            case InstructionType::XOR:
                registers[inst.operands[0]] = registers[inst.operands[1]] ^ inst.operands[2];
                break;
            case InstructionType::SLL:
                registers[inst.operands[0]] = registers[inst.operands[1]] << inst.operands[2];
                break;
            case InstructionType::SRL:
                registers[inst.operands[0]] = registers[inst.operands[1]] >> inst.operands[2];
                break;

            // DATA
            case InstructionType::LI:
                registers[inst.operands[0]] = inst.operands[1];
                break;

            // SPECIAL
            case InstructionType::DUMP_PROCESSOR_STATE:
                this->dumpState();
                break;

            default:
                std::cerr << "Invalid MIPS instruction executed" << std::endl;
        }
        pc++;
    }

    void dumpState() const {
        std::cout << "Processor State: " << std::endl;

        for (int i = 0; i < 32; i++) {
            std::cout << "R" << i << ": " << registers.at(i) << std::endl;
        }
        std::cout << "hi: " << hi << std::endl;
        std::cout << "lo: " << lo << std::endl;
        std::cout << "PC: " << pc << std::endl;
    }
};

class VM {
private:
    Config config;
    std::unique_ptr<CPU> cpu;
    std::vector<Instruction> instructions;
    int currentInstructionIndex;
public:
    VM(Config c) : config(std::move(c)), cpu(std::make_unique<CPU>()), currentInstructionIndex(0) {
        loadInstructions();
    }

    void loadInstructions() {
        std::ifstream file(config.vm_binary);
        std::string line;
        while (std::getline(file, line)) {
            instructions.push_back(parseInstruction(line));
        }
    }

    void snapshot(const std::string& outputPath) {
        std::ofstream outFile(outputPath, std::ios::out);

        if (!outFile.is_open()) {
            std::cerr << "Couldn't write to file: " << outputPath << std::endl;
            return;
        }

        // TODO - Finish here

    }

    bool run(int contextSwitch) {
        for (int i = 0; i < contextSwitch && currentInstructionIndex < instructions.size(); i++) {
            if (instructions.at(currentInstructionIndex).instructionType == InstructionType::SNAPSHOT) {
                snapshot(instructions.at(currentInstructionIndex).snapshotPath);
                continue;
            }

            cpu->execute(instructions.at(currentInstructionIndex));
            currentInstructionIndex++;
        }
        return currentInstructionIndex < instructions.size();
    }

    Config getConfig() {
        return this->config;
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
    void run() {
        bool allVMSCompleted = false;
        while (!allVMSCompleted) {
            allVMSCompleted = true;
            for (int i = 0; i < vms.size(); i++) {
                bool vmHasMoreInstructions = vms.at(i)->run(vms.at(i)->getConfig().vm_exec_slice_in_instructions);
                if (vmHasMoreInstructions) {
                    allVMSCompleted = false;
                }
                std::cout << "Context switching from VM: " << i << std::endl;
            }
        }
        std::cout << "All VMs completed executing" << std::endl;
    }
};

int main(int argc, char* argv[]) {
    std::vector<std::string> vmFiles;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        std::cout << arg << std::endl;
        if (arg == "-v" && i + 1 < argc) { // VM Files
            vmFiles.push_back(argv[++i]);
        } else if (arg == "-s" && i + 1 < argc) { // TODO - Handle snapshots
            // Should start VM from snapshot position

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

    hypervisor.run();

    return 0;
}