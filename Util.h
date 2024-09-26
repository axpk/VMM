#ifndef VMM_UTIL_H
#define VMM_UTIL_H

#include <string>
#include <vector>
#include "Instruction.h"

struct Config {
    int vm_exec_slice_in_instructions;
    std::string vm_binary;
};

bool parseConfigFile(const std::string& configPath, Config& config);
bool parseAssemblyFile(const std::string& assemblyPath, std::vector<Instruction>& assemblyLines);

#endif //VMM_UTIL_H
