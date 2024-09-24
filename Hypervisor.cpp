#include "Hypervisor.h"

void Hypervisor::createVM(const Config& config) {
    std::unique_ptr<VM> vm = std::make_unique<VM>(config);
    vms.push_back(std::move(vm));
}

