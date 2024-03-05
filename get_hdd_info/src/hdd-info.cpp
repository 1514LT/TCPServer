#include "hdd-info.h"

diskInfo::diskInfo() {
    if (statvfs(mount_point, &disk_info) != 0) {
        perror("Error getting disk information");
    }
}

unsigned long long diskInfo::getTotalDiskSpace() {
    return disk_info.f_blocks * disk_info.f_frsize;
}

unsigned long long diskInfo::getFreeDiskSpace() {
    return disk_info.f_bfree * disk_info.f_frsize;
}

unsigned long long diskInfo::getUsedDiskSpace() {
    return (disk_info.f_blocks - disk_info.f_bfree) * disk_info.f_frsize;
}