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
    ANDI,
    OR,
    ORI,
    XOR,
    XORI,
    SLL,
    SRL,
    LI,
    DUMP_PROCESSOR_STATE,
    SNAPSHOT,
    MIGRATE,
    INVALID
};

struct VMFileConfig {
    std::string vmFile;
    std::string snapshotFile;
};

struct Instruction {
    InstructionType instructionType = InstructionType::INVALID;
    std::vector<int> operands;
    std::string snapshotPath;
    std::string migratePath;
};

struct Config {
    int vm_exec_slice_in_instructions = 0;
    std::string vm_binary;
    int vmID;
};

bool parseConfigFile(const std::string& configPath, Config& config) {
    std::ifstream file(configPath);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << configPath << std::endl;
        return false;
    }

    std::string line;
    while(std::getline(file, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }

        size_t equalPos = line.find('=');

        std::string key = line.substr(0, equalPos);
        std::string value = line.substr(equalPos + 1);

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

bool parseSnapshotFile(const std::string& snapshotPath, std::array<int, 32>& registers, uint32_t& pc, std::string& binaryFile) {
    std::ifstream file(snapshotPath);
    if (!file.is_open()) {
        std::cerr << "Failed to open snapshot file: " << snapshotPath << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }

        size_t equalPos = line.find('=');
        std::string key = line.substr(0, equalPos);
        std::string value = line.substr(equalPos + 1);

        if (key[0] == 'R') {
            int registerIndex = std::stoi(key.substr(1));
            registers[registerIndex] = std::stoi(value);
        } else if (key == "pc") {
            pc = static_cast<uint32_t>(std::stoi(value));
        } else if (key == "binary") {
            binaryFile = value;
        } else {
            std::cerr << "Unknown snapshot key: " << key << std::endl;
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
            {"andi", InstructionType::ANDI},
            {"mul", InstructionType::MUL},
            {"mult", InstructionType::MULT},
            {"div", InstructionType::DIV},
            {"or", InstructionType::OR},
            {"ori", InstructionType::ORI},
            {"sll", InstructionType::SLL},
            {"srl", InstructionType::SRL},
            {"sub", InstructionType::SUB},
            {"subu", InstructionType::SUBU},
            {"xor", InstructionType::XOR},
            {"xori", InstructionType::XORI}
    };

    auto it = opcodeMap.find(opcode);
    if (it != opcodeMap.end()) {
        return it->second;
    } else {
        std::cerr << "Couldn't parse MIPS opcode: " << opcode << std::endl;
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

    if (line.find("MIGRATE") != std::string::npos) {
        inst.instructionType = InstructionType::MIGRATE;
        auto keyLocation = line.find(' ');
        inst.migratePath = line.substr(keyLocation + 1);
        return inst;
    }

    inst.instructionType = getInstructionType(opcode);

    std::string operand;
    while(std::getline(iss, operand, ',')) {
        if (operand.find('$') != std::string::npos) {
            inst.operands.emplace_back(parseRegister(operand));
        } else {
            inst.operands.emplace_back(static_cast<int>(std::stoi(operand)));
            if (inst.instructionType == InstructionType::OR) { // Convert OR to ORI internally if immediate value
                inst.instructionType = InstructionType::ORI;
            } else if (inst.instructionType == InstructionType::XOR) {
                inst.instructionType = InstructionType::XORI;
            }
        }
    }

    return inst;
}

class CPU {
public:
    int VMID = 0;
    std::array<int, 32> registers;
    uint32_t hi; // mult special register
    uint32_t lo; // mult special register
    uint32_t pc;

    CPU(int vmID) : pc(0), VMID(vmID) {
        registers.fill(0);
    }
    CPU(const std::array<int, 32> regs, int vmID) : pc(0), VMID(vmID) {
        registers = regs;
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
            case InstructionType::ANDI:
                registers[inst.operands[0]] = registers[inst.operands[1]] & inst.operands[2];
                break;
            case InstructionType::OR:
                registers[inst.operands[0]] = registers[inst.operands[1]] | registers[inst.operands[2]];
                break;
            case InstructionType::ORI:
                registers[inst.operands[0]] = registers[inst.operands[1]] | inst.operands[2];
                break;
            case InstructionType::XOR:
                registers[inst.operands[0]] = registers[inst.operands[1]] ^ registers[inst.operands[2]];
                break;
            case InstructionType::XORI:
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
        std::cout << "==== VM: " << VMID << " =======" << std::endl;
        std::cout << "Processor State: " << std::endl;

        for (int i = 0; i < 32; i++) {
            std::cout << "R" << i << ": " << registers.at(i) << std::endl;
        }
        std::cout << "hi: " << hi << std::endl;
        std::cout << "lo: " << lo << std::endl;
        std::cout << "PC: " << pc << std::endl;

        std::cout << "======================" << std::endl;
        std::cout << "\n";
    }

};

class VM {
private:
    Config config;
    std::unique_ptr<CPU> cpu;
    std::vector<Instruction> instructions;
    int currentInstructionIndex;
public:
    VM(Config c) : config(std::move(c)), cpu(std::make_unique<CPU>(config.vmID)), currentInstructionIndex(0) {
        loadInstructions();
    }

    VM(Config c, std::unique_ptr<CPU> snapshotCPU) : cpu(std::move(snapshotCPU)), config(std::move(c)), currentInstructionIndex(0) {
        loadInstructions();
    }

    VM(Config c, std::unique_ptr<CPU> snapshotCPU, int current_instruction_index) : cpu(std::move(snapshotCPU)),
            config(std::move(c)), currentInstructionIndex(current_instruction_index) {
        loadInstructions();
    }

    void loadInstructions() {
        std::ifstream file(config.vm_binary);
        std::string line;
        while (std::getline(file, line)) {
            if (!line.empty()) {
                instructions.emplace_back(parseInstruction(line));
            }
        }
    }

    void snapshot(const std::string& outputPath) {
        std::cout << "Creating snapshot: " << outputPath << ", pc: " << cpu->pc << std::endl;
        std::ofstream outFile(outputPath);

        if (!outFile.is_open()) {
            std::cerr << "Couldn't write to file: " << outputPath << std::endl;
            return;
        }

        for (int i = 0; i < cpu->registers.size(); i++) {
            outFile << "R" << i << "=" << cpu->registers.at(i) << "\n";
        }

        outFile << "pc=" << cpu->pc << "\n";
        outFile << "binary=" << config.vm_binary << "\n";

        outFile.close();
        cpu->pc++;
    }

    void migrate(const std::string& outputPath) {
        // TODO - add migration logic to IP or hostname
    }

    bool run(int contextSwitch) {
        for (int i = 0; i < contextSwitch && currentInstructionIndex < instructions.size(); i++) {
            if (instructions.at(currentInstructionIndex).instructionType == InstructionType::SNAPSHOT) {
                snapshot(instructions.at(currentInstructionIndex).snapshotPath);
            } else if (instructions.at(currentInstructionIndex).instructionType == InstructionType::MIGRATE) {
                migrate(instructions.at(currentInstructionIndex).migratePath);
            } else {
                cpu->execute(instructions.at(currentInstructionIndex));
            }
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
       vms.emplace_back(std::move(vm));
    }
    void createVM(const Config& config, std::unique_ptr<CPU> cpu) {
        std::unique_ptr<VM> vm = std::make_unique<VM>(config, std::move(cpu));
        vms.emplace_back(std::move(vm));
    }
    void createVM(const Config& config, std::unique_ptr<CPU> cpu, int current_instruction_index) {
        std::unique_ptr<VM> vm = std::make_unique<VM>(config, std::move(cpu), current_instruction_index);
        vms.emplace_back(std::move(vm));
    }
    void run() {
        bool allVMSCompleted = false;
        while (!allVMSCompleted) {
            allVMSCompleted = true;
            for (int i = 0; i < vms.size(); i++) {
                bool vmHasMoreInstructions = vms.at(i)->run(vms.at(i)->getConfig().vm_exec_slice_in_instructions);
                if (vmHasMoreInstructions) {
                    allVMSCompleted = false;
                    std::cout << "(VM: " << i + 1 << " running)" << std::endl;
                }
            }
        }
        std::cout << "All VMs completed executing" << std::endl;
    }
};

int main(int argc, char* argv[]) {
    std::vector<VMFileConfig> vmFileConfigsVector;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        VMFileConfig vmFileConfig;
        if (arg == "-v" && i + 1 < argc) { // VM Files
            vmFileConfig.vmFile = argv[++i];

            if (i + 1 < argc && std::string(argv[i+1]) == "-s") { // Snapshot files
                if (i + 2 > argc) {
                    std::cerr << "No snapshot file provided after -s flag" << std::endl;
                    return 1;
                }
                vmFileConfig.snapshotFile = argv[i+2];
                i += 2;
            }
            vmFileConfigsVector.emplace_back(std::move(vmFileConfig));
        } else {
            std::cerr << "No filename given after -v flag" << std::endl;
        }
    }

    Hypervisor hypervisor;

    int vmID = 0;
    for (const auto& vmConfig : vmFileConfigsVector) {
        vmID++;
        Config config;
        config.vmID = vmID;
        if (!parseConfigFile(vmConfig.vmFile, config)) {
            std::cerr << "Error parsing config assembly file" << std::endl;
            return 1;
        }
        if (!vmConfig.snapshotFile.empty()) {
            std::array<int, 32> registers{};
            uint32_t pc;
            std::string binaryFile;
            parseSnapshotFile(vmConfig.snapshotFile, registers, pc, binaryFile);
            std::unique_ptr<CPU> cpu = std::make_unique<CPU>(registers, config.vmID);
            pc++;
            cpu->pc = pc;
            if (config.vm_binary == binaryFile) { // Same assembly file -> continue from snapshot point
                hypervisor.createVM(config, std::move(cpu), static_cast<int>(pc));
            } else { // otherwise just use registers
                hypervisor.createVM(config, std::move(cpu));
            }
        } else {
            hypervisor.createVM(config);
        }
    }

    hypervisor.run();

    return 0;
}