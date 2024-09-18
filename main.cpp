#include <iostream>
#include <string>
#include <vector>
#include "main.h"
#include "Hypervisor.h"
#include "Util.h"

int main(int argc, char* argv[]) {

    std::vector<std::string> vmFiles;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        std::cout << arg << std::endl;
        if (arg == "-v" && i + 1 < argc) { // VM Files
            vmFiles.push_back(argv[++i]);
        } else {
            std::cerr << "No filename given after -v flag" << std::endl;
        }
    }

    if (vmFiles.empty()) {
        std::cerr << "No VM files provided. Use -v to specify files." << std::endl;
        return 1;
    }

    // TODO - change to for loop for multiple VM parsing
    std::string configPath = "input_files/assembly_file_vm1";
    Config config;

    if (!parseConfigFile(configPath, config)) {
        std::cerr << "Error parsing config assembly file" << std::endl;
        return 1;
    }

    Hypervisor hypervisor;

    return 0;
}
