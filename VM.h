#ifndef VMM_VM_H
#define VMM_VM_H

#include "Util.h"
#include "CPU.h"
#include <memory>
#include <vector>
#include "Instruction.h"

class VM {
private:
    Config config;
    std::unique_ptr<CPU> cpu;
    std::vector<Instruction> assemblyLines;

public:
    VM(const Config& config);
    ~VM();
};


#endif //VMM_VM_H
