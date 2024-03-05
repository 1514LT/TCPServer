#ifndef MEMINFO_H
#define MEMINFO_H
#include <iostream>
#include <sys/sysinfo.h>

class memoryInfo {
public:
    memoryInfo();

    // 获取总内存大小（以字节为单位）
    unsigned long long getTotalRAM();

    // 获取可用内存大小（以字节为单位）
    unsigned long long getFreeRAM();

    // 获取已使用内存大小（以字节为单位）
    unsigned long long getUsedRAM();

private:
    struct sysinfo info;
};

#endif // ATLASINFORMATION_H