#include "mem-info.h"

memoryInfo::memoryInfo() {
    if (sysinfo(&info) != 0) {
        std::cerr << "Error getting system info" << std::endl;
    }
}

unsigned long long memoryInfo::getTotalRAM() {
    return info.totalram * info.mem_unit;
}

unsigned long long memoryInfo::getFreeRAM() {
    return info.freeram * info.mem_unit;
}

unsigned long long memoryInfo::getUsedRAM() {
    return getTotalRAM() - getFreeRAM();
}