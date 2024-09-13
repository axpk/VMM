#include <iostream>
#include <string>
#include <vector>
#include "main.h"

int main(int argc, char* argv[]) {
    std::vector<std::string> vmFiles;
    for (int i = 0; i < argc; i++) {
        std::string arg = argv[i];
        std::cout << arg << std::endl;
        if (arg == "-v" && i + 1 < argc) {
            vmFiles.push_back(argv[i++]);
        } else {
            std::cerr << "Invalid arguments, placed a -v without a following file" << arg << std::endl;
            return 1;
        }
    }

    if (vmFiles.empty()) {
        std::cerr << "No VM files provided. Use -v to specify files." << std::endl;
        return 1;
    }

    std::cout << "Hello, World!" << std::endl;
    return 0;
}
