#ifndef HDDINFO_H
#define HDDINFO_H
#include <iostream>
#include <sys/statvfs.h>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
struct DiskInfo {
    std::string filesystem;
    std::string capacity;
    std::string used;
    std::string available;
    std::string used_percentage;
    std::string mount_point;
};
class diskInfo {
public:
    diskInfo();

    // 获取总磁盘空间大小（以字节为单位）
    unsigned long long getTotalDiskSpace();

    // 获取可用磁盘空间大小（以字节为单位）
    unsigned long long getFreeDiskSpace();

    // 获取已使用磁盘空间大小（以字节为单位）
    unsigned long long getUsedDiskSpace();

    std::vector<DiskInfo> fetch_disk_info();

private:
    struct statvfs disk_info;
    const char *mount_point = "/";  // 指定挂载点，通常是根目录
};

#endif // ATLASINFORMATION_H
