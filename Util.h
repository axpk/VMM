#ifndef VMM_UTIL_H
#define VMM_UTIL_H

#include <string>
#include <vector>

struct Config {
    int vm_exec_slice_in_instructions;
    std::vector<std::string> vm_binaries;
};

bool parseConfigFile(const std::string& configPath, Config& config);

#endif //VMM_UTIL_H
