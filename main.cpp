#include <iostream>
#include <string>
#include <vector>
#include "main.h"
#include "Hypervisor.h"

int main(int argc, char* argv[]) {

    std::vector<std::string> vmFiles;
    std::string configFile;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        std::cout << arg << std::endl;
        if (arg == "-v" && i + 1 < argc) { // VM Files
            vmFiles.push_back(argv[++i]);
        } else if (arg == "-c" && i + 1 < argc) { // Config file
            configFile = argv[++i];
        }
    }

    if (vmFiles.empty() || configFile.empty()) {
        std::cerr << "No VM files or config provided. Use -v and -c to specify files." << std::endl;
        return 1;
    }

    return 0;
}
