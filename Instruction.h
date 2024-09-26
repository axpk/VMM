#ifndef VMM_INSTRUCTION_H
#define VMM_INSTRUCTION_H

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
class Instruction {
public:
    Instruction();
};


#endif //VMM_INSTRUCTION_H
