#ifndef VMM_HYPERVISOR_H
#define VMM_HYPERVISOR_H

#include <vector>
#include <string>
#include "VM.h"
#include "Util.h"

class Hypervisor {
private:
    std::vector<VM> vms;

public:
    void createVM(const Config& config);
};


#endif //VMM_HYPERVISOR_H
