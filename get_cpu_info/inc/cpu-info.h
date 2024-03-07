#ifndef CPUINFO_H
#define CPUINFO_H
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
struct CpuTime {
    long user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;
};
typedef struct PACKED         //定义一个cpu occupy的结构体
{
    char name[20];      //定义一个char类型的数组名name有20个元素
    unsigned int user; //定义一个无符号的int类型的user
    unsigned int nice; //定义一个无符号的int类型的nice
    unsigned int system;//定义一个无符号的int类型的system
    unsigned int idle; //定义一个无符号的int类型的idle
}CPU_OCCUPY;

int ExecuteCMD_C(const char* cmd, std::string &result_);

class cpuInfo {
public:
    cpuInfo();

    // 获取 CPU 型号
    std::string getCPUModel();

    // 获取 CPU 架构
    std::string getCPUArchitecture();

    // 获取逻辑 CPU 个数
    std::string getLogicalCPUCores();

    // 获取物理核心数
    int getPhysicalCPUCores();
  
    // 获取 CPU 使用率
    void get_cpuoccupy (CPU_OCCUPY *cpust);
    float cal_cpuoccupy(CPU_OCCUPY *o, CPU_OCCUPY *n); 
    float getCPUSUsage (int time);
    CpuTime getCpuTime(int cpuId);
    double getCpuUsage(int cpuId);
    CpuTime getTotalCpuTime();
    double getTotalCpuUsage();


private:
    std::vector<unsigned long long> prevTotalCPUTime;
    std::vector<unsigned long long> currTotalCPUTime;

    std::string cpuModel;
    std::string cpuArchitecture;
    std::string logicalCPUCores;
    int physicalCPUCores;

    void readCPUInfo();
};

#endif // CPU_INFO_H
