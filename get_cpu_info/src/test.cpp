#include "cpu-info.h"
#include <unistd.h>

int main() {
    cpuInfo cpu;

    // 获取 CPU 信息
    std::cout << "CPU 型号: " << cpu.getCPUModel() << std::endl;
    std::cout << "CPU 架构: " << cpu.getCPUArchitecture();
    std::cout << "逻辑 CPU 个数: " << cpu.getLogicalCPUCores();
    std::cout << "CPU核数: " << cpu.getPhysicalCPUCores() << std::endl;
    std::cout << "CPU的使用率为: " << cpu.getCPUSUsage(5) << "%" << std::endl;

    return 0;
}
