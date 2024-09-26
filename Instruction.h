#ifndef VMM_INSTRUCTION_H
#define VMM_INSTRUCTION_H

#include <cstdint>

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

/**
 * MIPS Instruction
 */
struct Instruction {
    InstructionType instructionType = InstructionType::INVALID;
    uint32_t operand1 = 0;
    uint32_t operand2 = 0;
    uint32_t dest = 0;
};


#endif //VMM_INSTRUCTION_H
