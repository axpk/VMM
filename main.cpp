#include <iostream>
#include <string>
#include <vector>
#include "main.h"
#include "Hypervisor.h"

int main(int argc, char* argv[]) {

    std::vector<std::string> vmFiles;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        std::cout << arg << std::endl;
        if (arg == "-v" && i + 1 < argc) { // VM Files
            vmFiles.push_back(argv[++i]);
        }
    }

    if (vmFiles.empty()) {
        std::cerr << "No VM files provided. Use -v to specify files." << std::endl;
        return 1;
    }

    return 0;
}
