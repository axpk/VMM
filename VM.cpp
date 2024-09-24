#include "VM.h"

VM::VM(const Config &config) {
    this->config = config;
    this->cpu = std::make_unique<CPU>();
    parseAssemblyFile(this->config.vm_binary, assemblyLines);
}