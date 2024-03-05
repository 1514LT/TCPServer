#include "mem-info.h"

int main() {
    memoryInfo memory;

    // 获取内存信息
    unsigned long long totalRAM = memory.getTotalRAM();
    unsigned long long freeRAM = memory.getFreeRAM();
    unsigned long long usedRAM = memory.getUsedRAM();

    // 打印内存信息
    std::cout << "总内存大小: " << totalRAM << " 字节\n";
    std::cout << "可用内存大小: " << freeRAM << " 字节\n";
    std::cout << "已使用内存大小: " << usedRAM << " 字节\n";

    return 0;
}