#ifndef VMM_CPU_H
#define VMM_CPU_H

#include <array>
#include <cstdint>
#include <iostream>

/**
 * MIPS CPU State Representation
 */
class CPU {
public:
    CPU();
    uint32_t getRegister(int index) const;
    void setRegister(int index, uint32_t value);
    uint32_t getProgramCounter() const;
    void setProgramCounter(uint32_t pc);
    void dumpState() const;

private:
    std::array<uint32_t, 32> registers;
    uint32_t programCounter;
};


#endif //VMM_CPU_H
