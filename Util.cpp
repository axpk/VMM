#include "Util.h"
#include <fstream>
#include <iostream>

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
            config.vm_binaries.push_back(value);
        } else {
            std::cerr << "Unknown file key: " << key << std::endl;
        }
    }
    file.close();
    return true;
}