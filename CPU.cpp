#include "CPU.h"
#include "Instruction.h"

CPU::CPU() : programCounter(0) {
    registers.fill(0);
}

uint32_t CPU::getRegister(int index) const {
    if (index < 0 || index >= 32) {
        std::cerr << "Wrong index given to getRegister" << std::endl;
        return 0;
    }
    return registers.at(index);
}

void CPU::setRegister(int index, uint32_t value) {
    if (index < 0 || index >= 32) {
        std::cerr << "Invalid index" << std::endl;
    }

    if (index != 0) { // Register 0 is always 0
        registers.at(index) = value;
    }
}

uint32_t CPU::getProgramCounter() const {
    return programCounter;
}

void CPU::setProgramCounter(uint32_t pc) {
    programCounter = pc;
}

void CPU::dumpState() const {
    std::cout << "Processor State: " << std::endl;

    for (int i = 0; i < 32; i++) {
        std::cout << "R" << i << ": " << registers.at(i) << std::endl;
    }
    std::cout << "PC: " << programCounter << std::endl;
}

// TODO - use switch stmnt for handling instruction types
void CPU::executeInstruction(Instruction &instruction) {

}
