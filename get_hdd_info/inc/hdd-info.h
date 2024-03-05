#ifndef HDDINFO_H
#define HDDINFO_H
#include <iostream>
#include <sys/statvfs.h>

class diskInfo {
public:
    diskInfo();

    // 获取总磁盘空间大小（以字节为单位）
    unsigned long long getTotalDiskSpace();

    // 获取可用磁盘空间大小（以字节为单位）
    unsigned long long getFreeDiskSpace();

    // 获取已使用磁盘空间大小（以字节为单位）
    unsigned long long getUsedDiskSpace();

private:
    struct statvfs disk_info;
    const char *mount_point = "/";  // 指定挂载点，通常是根目录
};

#endif // ATLASINFORMATION_H
