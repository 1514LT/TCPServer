#include "hdd-info.h"

int main() {
    diskInfo disk;

    // 获取硬盘信息
    unsigned long long totalDiskSpace = disk.getTotalDiskSpace();
    unsigned long long freeDiskSpace = disk.getFreeDiskSpace();
    unsigned long long usedDiskSpace = disk.getUsedDiskSpace();

    // 打印硬盘信息
    std::cout << "总磁盘空间大小: " << totalDiskSpace << " 字节\n";
    std::cout << "可用磁盘空间大小: " << freeDiskSpace << " 字节\n";
    std::cout << "已使用磁盘空间大小: " << usedDiskSpace << " 字节\n";

    return 0;
}