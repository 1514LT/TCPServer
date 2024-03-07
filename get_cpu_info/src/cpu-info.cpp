#include <cstring>
#include <iostream>
#include <unistd.h>
#include "cpu-info.h"

#define CMD_RESULT_BUF_SIZE 1024

int ExecuteCMD_C(const char* cmd, std::string &result_)
{
    char result[CMD_RESULT_BUF_SIZE] = {0};
    char buf_temp[CMD_RESULT_BUF_SIZE] = {0};
    FILE *ptr = NULL;
    int iRet = -1;

    //popen: 开启子进程，建立管道，并运行指令，'r':从子进程获取结果，'w':向子进程写数据
    if((ptr = popen(cmd, "r")) != NULL)  //popen
    {
        while(fgets(buf_temp, sizeof(buf_temp), ptr) != NULL)
        {
            if(strlen(result) + strlen(buf_temp) > CMD_RESULT_BUF_SIZE){
                break;
            }
           strcat(result, buf_temp);  //字符串拼接
        }
        result_ = result;
        pclose(ptr);
        ptr = NULL;
        iRet = 0;  // 处理成功
    }
    else
    {
        printf("popen %s error\n", cmd);
        iRet = -1; // 处理失败
    }

    return iRet;
}

cpuInfo::cpuInfo() {
    readCPUInfo();
}

std::string cpuInfo::getCPUModel() {
    return cpuModel;
}

std::string cpuInfo::getCPUArchitecture() {
    ExecuteCMD_C("uname -m", cpuArchitecture);
    return cpuArchitecture;
}

std::string cpuInfo::getLogicalCPUCores() {
    ExecuteCMD_C("cat /proc/cpuinfo | grep 'processor' | sort | uniq | wc -l", logicalCPUCores);
    return logicalCPUCores;
}

int cpuInfo::getPhysicalCPUCores() {
    physicalCPUCores = sysconf(_SC_NPROCESSORS_CONF);
    return physicalCPUCores;
}

void cpuInfo::readCPUInfo() {
    std::ifstream cpuinfoFile("/proc/cpuinfo");

    if (!cpuinfoFile.is_open()) {
        std::cerr << "Error opening /proc/cpuinfo" << std::endl;
        return;
    }

    std::string line;
    while (std::getline(cpuinfoFile, line)) {
        std::istringstream iss(line);
        std::string key, value;

        if (std::getline(iss, key, ':') && std::getline(iss, value)) {
            // Trim leading and trailing whitespaces
            key.erase(key.find_last_not_of(" \t") + 1);
            key.erase(0, key.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t"));

            if (key == "model name") {
                cpuModel = value;
            }
        }
    }
    cpuinfoFile.close();
}


void cpuInfo::get_cpuoccupy(CPU_OCCUPY *cpust) //对无类型get函数含有一个形参结构体类弄的指针O  
{       
        FILE *fd;            
        int n;              
        char buff[256];    
        CPU_OCCUPY *cpu_occupy;     
        cpu_occupy=cpust;           
        fd = fopen ("/proc/stat", "r");    
        fgets (buff, sizeof(buff), fd);          
        sscanf (buff, "%s %u %u %u %u", cpu_occupy->name, &cpu_occupy->user, &cpu_occupy->nice,&cpu_occupy->system, &cpu_occupy->idle);   
        fclose(fd);     
 }  

float cpuInfo::cal_cpuoccupy(CPU_OCCUPY *o, CPU_OCCUPY *n)  
{      
    unsigned long od, nd;       
    unsigned long id, sd;     
    float cpu_use = 0;           
    od = (unsigned long) (o->user + o->nice + o->system +o->idle);//第一次(用户+优先级+系统+空闲)的时间再赋给od    
    nd = (unsigned long) (n->user + n->nice + n->system +n->idle);//第二次(用户+优先级+系统+空闲)的时间再赋给nd      
    id = (unsigned long) (n->user - o->user);    //用户第一次和第二次的时间之差再赋给id    
    sd = (unsigned long) (n->system - o->system);//系统第一次和第二次的时间之差再赋给sd  
    if((nd-od) != 0)
    {
         cpu_use = (float)((sd+id)*100)/(nd-od); //((用户+系统)乖100)除(第一次和第二次的时间差)再赋给g_cpu_used   
    }     
    else
    {   cpu_use = 0;          
       
    }    
    return cpu_use; 
}

/*获取cpu使用率*/
float cpuInfo::getCPUSUsage (int time)
{
    CPU_OCCUPY cpu_stat1;    
    CPU_OCCUPY cpu_stat2;
    float cpu;  
    get_cpuoccupy((CPU_OCCUPY *)&cpu_stat1);  
    usleep(time*1000);
    get_cpuoccupy((CPU_OCCUPY *)&cpu_stat2);  
    cpu = cal_cpuoccupy((CPU_OCCUPY *)&cpu_stat1, (CPU_OCCUPY *)&cpu_stat2);
    return cpu;
}



// 获取具体某个CPU的时间
CpuTime cpuInfo::getCpuTime(int cpuId) {
    std::ifstream procStat("/proc/stat");
    std::string line;
    CpuTime cpuTime{};

    for (int i = 0; i <= cpuId + 1; ++i) {  
        std::getline(procStat, line);
    }
    
    std::sscanf(line.c_str(), "cpu%*d %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld", &cpuTime.user, &cpuTime.nice, &cpuTime.system, &cpuTime.idle, &cpuTime.iowait, &cpuTime.irq, &cpuTime.softirq, &cpuTime.steal, &cpuTime.guest, &cpuTime.guest_nice);
    return cpuTime;
}

// 获取CPU的使用率
double cpuInfo::getCpuUsage(int cpuId) {
    CpuTime cpuTime1 = getCpuTime(cpuId);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    CpuTime cpuTime2 = getCpuTime(cpuId);

    long idle1 = cpuTime1.idle + cpuTime1.iowait;
    long idle2 = cpuTime2.idle + cpuTime2.iowait;

    long nonIdle1 = cpuTime1.user + cpuTime1.nice + cpuTime1.system + cpuTime1.irq + cpuTime1.softirq + cpuTime1.steal;
    long nonIdle2 = cpuTime2.user + cpuTime2.nice + cpuTime2.system + cpuTime2.irq + cpuTime2.softirq + cpuTime2.steal;

    long total1 = idle1 + nonIdle1;
    long total2 = idle2 + nonIdle2;

    return static_cast<double>(total2 - total1 - (idle2 - idle1)) / (total2 - total1);
}

// 获取CPU总时间
CpuTime cpuInfo::getTotalCpuTime() {
    std::ifstream procStat("/proc/stat");
    std::string line;
    std::getline(procStat, line);
    CpuTime cpuTime{};
    
    std::sscanf(line.c_str(), "cpu %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld", &cpuTime.user, &cpuTime.nice, &cpuTime.system, &cpuTime.idle, &cpuTime.iowait, &cpuTime.irq, &cpuTime.softirq, &cpuTime.steal, &cpuTime.guest, &cpuTime.guest_nice);
    return cpuTime;
}

// 获取总CPU使用率
double cpuInfo::getTotalCpuUsage() {
    CpuTime cpuTime1 = getTotalCpuTime();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    CpuTime cpuTime2 = getTotalCpuTime();

    long idle1 = cpuTime1.idle + cpuTime1.iowait;
    long idle2 = cpuTime2.idle + cpuTime2.iowait;

    long nonIdle1 = cpuTime1.user + cpuTime1.nice + cpuTime1.system + cpuTime1.irq + cpuTime1.softirq + cpuTime1.steal;
    long nonIdle2 = cpuTime2.user + cpuTime2.nice + cpuTime2.system + cpuTime2.irq + cpuTime2.softirq + cpuTime2.steal;

    long total1 = idle1 + nonIdle1;
    long total2 = idle2 + nonIdle2;

    return static_cast<double>(total2 - total1 - (idle2 - idle1)) / (total2 - total1);
}