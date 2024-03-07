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


std::vector<DiskInfo> diskInfo::fetch_disk_info() {
    FILE* pipe = popen("df -h", "r");
    if (!pipe) return {};

    char buffer[256];
    std::vector<std::string> lines;

    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        lines.push_back(buffer);
    }
    pclose(pipe);

    std::vector<DiskInfo> disk_infos;
    for (int i = 1; i < lines.size(); ++i) { // skip first line
        std::istringstream iss(lines[i]);
        DiskInfo info;
        if (!(iss >> info.filesystem >> info.capacity >> info.used >> info.available 
                  >> info.used_percentage >> info.mount_point)) {
            break;
        }
        disk_infos.push_back(info);
    }
    return disk_infos;
}