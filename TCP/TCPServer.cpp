#include "TCPServer.hpp"
JRLCServer::JRLCServer(int port)
{
  try
  {
    cpu = new cpuInfo();
    disk = new diskInfo();
    memory = new memoryInfo();
    // 设置监听地址和端口
    printf("server start\n");

    Poco::Net::SocketAddress sa(port);
    // 创建服务器套接字
    Poco::Net::ServerSocket server(sa);
    Poco::Timespan::TimeDiff timeout(POLLTIMEOUT);
    Poco::UInt32 bodySize;
    Poco::UInt8 msgType;
    Poco::UInt32 serialNum;
    // 创建任务管理线程
    std::thread(&JRLCServer::executeTasks, this).detach();
    while (true)
    {
      // 接受新的连接
      Poco::Net::StreamSocket *ss = new Poco::Net::StreamSocket(server.acceptConnection());
      std::cout << "New connection from: " << ss->peerAddress().toString() << std::endl;
      std::thread handler(&JRLCServer::handleMsg, this, msgType, bodySize, serialNum, ss);
      handler.detach();
    }
  }
  catch (const std::exception &e)
  {
    std::cerr << e.what() << '\n';
  }
}
void JRLCServer::handleMsg(Poco::UInt8 msgType, Poco::UInt32 bodySize, Poco::UInt32 serialNum, Poco::Net::StreamSocket *ss)
{
  try
  {
    while(1)
    {

      // if(ss->receiveBytes(&bodySize, sizeof(bodySize)) <= 0)
      // {
      //   perror("client closed");
      //   ss->close();
      //   removeTimedTask(socket_task_map[ss],ss);
      //   // return;
      //   break;
      // }
      int len;
      len = ss->receiveBytes(&bodySize, sizeof(bodySize));
      printf("len:%d\n",len);
      ss->receiveBytes(&msgType, sizeof(msgType));
      ss->receiveBytes(&serialNum, sizeof(serialNum));
      bodySize = Poco::ByteOrder::fromLittleEndian(bodySize);
      serialNum = Poco::ByteOrder::fromLittleEndian(serialNum);
      printf("enter handleMessage msgType:%d\n", msgType);
      handleBody(ss, msgType);
    }
        
  }
  catch (Poco::Net::ConnectionResetException &)
    {
      perror("client closed");
      ss->close();
      removeTimedTask(socket_task_map[ss],ss);
      return;
      // break;
    }
    catch (const Poco::Exception &e)
    {
      std::cerr << e.displayText() << '\n';
    }
    catch (const std::exception &e)
    {
      std::cerr << e.what() << '\n';
    }
    catch (...)
    {
      std::cerr << "unknow error!\n";
    }
  
}
void JRLCServer::handleBody(Poco::Net::StreamSocket *ss, Poco::UInt8 msgType)
{
  try
  {
    // 使用 StreamSocket 创建 SocketStream
    Poco::Net::SocketStream str(*ss);

    // 读取 jsonSize 与 payloadSize 的值
    int jsonSize = 0;
    int payloadSize = 0;
    str.read((char *)&jsonSize, 4);
    str.read((char *)&payloadSize, 4);

    // 根据读取的大小值分配内存
    std::string jsonStr(jsonSize, '\0');
    std::string payloadStr(payloadSize, '\0');

    // 读取 json 与 payload 的内容
    str.read(&jsonStr[0], jsonSize);
    str.read(&payloadStr[0], payloadSize);
    cout << "jsonSize:" << jsonSize << " payloadSize:" << payloadSize;
    printf("jsonString:%s", jsonStr.c_str());
    switch (msgType)
    {
    case PROCESSLAUNCHER:
      ProcessBinaryLaunch();
      break;
    case ADDTIMER:
      ProcessTimer(ss);
      break;
    case REMOVETIMER:
      RemoveTimer(ss);
      break;
    case CPUINFO:
      ProcessCpu(ss);
      break;
    case REMOVECPU:
      RemoveCpu(ss);
      break;
    case HDDINFO:
      ProcessHdd(ss);
      break;
    case REMOVEHDD:
      RemoveHdd(ss);
      break;
    case MEMINFO:
      ProcessMem(ss);
      break;
    case REMOVEMEM:
      RemoveMem(ss);
      break;
    default:
      break;
    }
  }
  catch (Poco::Exception &ex)
  {
    // 处理或记录异常
  }
}

void JRLCServer::executeTasks()
{
  while (true)
  {
    Poco::FastMutex::ScopedLock locker(m_queueMutex);
    auto now = Poco::Clock();

    for (auto it = m_taskQueue.begin(); it != m_taskQueue.end();)
    {
      if (it->first < now)
      {
        auto taskHandle = it->second;
        auto task = taskHandle.get<0>();
        auto interval = taskHandle.get<1>();
        auto single = taskHandle.get<2>();
        Poco::Net::StreamSocket* socket = taskHandle.get<3>();
        (this->*task)(socket);
        if (single)
        {
          Poco::Clock next = now;
          next += interval * 1000;
          m_taskQueue.insert({next, {task, interval, single, socket}});
        }
        it = m_taskQueue.erase(it);
      }
      else
      {
        ++it;
      }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
}

std::string JRLCServer::toJSONString(const Poco::JSON::Object &root)
{
  std::stringstream ss;
  Poco::JSON::Stringifier::stringify(root, ss);

  return ss.str();
}

Poco::JSON::Object::Ptr JRLCServer::toJSONObj(const std::string &jsonString)
{
  Poco::JSON::Parser parser;
  Poco::Dynamic::Var result = parser.parse(jsonString);
  Poco::JSON::Object::Ptr jsonObj = result.extract<Poco::JSON::Object::Ptr>();

  return jsonObj;
}

void JRLCServer::SendJson(Poco::Net::StreamSocket *ss, const Poco::JSON::Object obj, int Msg_Type)
{
  std::string result = toJSONString(obj);
  int jsonSize = result.size();
  char jsonData[jsonSize];
  strcpy(jsonData,result.c_str());
  printf("jsonData:%s\n", jsonData);
  int cli_fd = ss->impl()->sockfd();
  int payloadSize = 0;
  int headLen = 4 + 1 + 4;
  int bodyLen = 4 + 4 + jsonSize + payloadSize;
  char *header = (char*)malloc(headLen + bodyLen);
  char *p = header;
  int *bodySize = (int*)p;
  *bodySize = bodyLen;
  p += 4;
  char* msgType = (char*)p;
  *msgType = Msg_Type;
  p += 1;
  int *serialNum = (int*)p;
  *serialNum = 0;
  p += 4;
  int *jsonsize = (int*)p;
  *jsonsize = jsonSize;
  p += 4;
  int *payloadsize = (int*)p;
  *payloadsize = payloadSize;
  p += 4;
  memcpy(p,jsonData,jsonSize);
  send(cli_fd,header,headLen + bodyLen,0);
  free(header);
}

void JRLCServer::ProcessBinaryLaunch()
{
  Poco::Process::Args arguments;
  arguments.push_back("arg1");
  arguments.push_back("arg2");
  ProcessLauncher launcher;
  Poco::Pipe outpipe;
  launcher.launch("/home/lt/workspace/demo/test", arguments, outpipe);
  int pid = launcher.getHandle().id();
  printf("pid:%d\n", pid);
  launcher.getHandle().wait();
  // launcher.readFromPipe(outpipe);
}
void JRLCServer::test(Poco::Net::StreamSocket* socket)
{
  time_t currentTime = time(nullptr);
  struct tm *localTime = localtime(&currentTime);
  std::cout << "当前时间: " << (localTime->tm_hour < 10 ? "0" : "") << localTime->tm_hour << ":"
            << (localTime->tm_min < 10 ? "0" : "") << localTime->tm_min << ":"
            << (localTime->tm_sec < 10 ? "0" : "") << localTime->tm_sec << std::endl;
}
void JRLCServer::ProcessTimer(Poco::Net::StreamSocket *ss)
{
  socket_task_map[ss] = &JRLCServer::test;
  // addTimedTask(&JRLCServer::test, 1000, true);
  addTimedTask(&JRLCServer::test, ss, 1000, true);
}
void JRLCServer::RemoveTimer(Poco::Net::StreamSocket* socket)
{
  removeTimedTask(&JRLCServer::test,socket);
}
void JRLCServer::ProcessCpu(Poco::Net::StreamSocket *ss)
{
  socket_task_map[ss] = &JRLCServer::GetCpuInfo;
  addTimedTask(&JRLCServer::GetCpuInfo, ss,1000, true);
}
void JRLCServer::RemoveCpu(Poco::Net::StreamSocket* socket)
{
  removeTimedTask(&JRLCServer::GetCpuInfo,socket);
}
void JRLCServer::ProcessHdd(Poco::Net::StreamSocket *ss)
{
  socket_task_map[ss] = &JRLCServer::GetHddInfo;
  addTimedTask(&JRLCServer::GetHddInfo, ss,1000, true);
}
void JRLCServer::RemoveHdd(Poco::Net::StreamSocket* socket)
{
  removeTimedTask(&JRLCServer::GetHddInfo,socket);
}
void JRLCServer::ProcessMem(Poco::Net::StreamSocket *ss)
{
  socket_task_map[ss] = &JRLCServer::GetMemInfo;
  addTimedTask(&JRLCServer::GetMemInfo, ss,1000, true);
}
void JRLCServer::RemoveMem(Poco::Net::StreamSocket* socket)
{
  removeTimedTask(&JRLCServer::GetMemInfo,socket);
}

void JRLCServer::addTimedTask(JRLCTimedTask task, Poco::Net::StreamSocket* socket, int interval, bool single)
{
    Poco::FastMutex::ScopedLock locker(m_queueMutex);
    Poco::Clock clock;
    clock += interval * 1000;
    m_taskQueue.insert({clock, {task, interval, single, socket}});
}

void JRLCServer::removeTimedTask(JRLCTimedTask task, Poco::Net::StreamSocket* socket)
{
  Poco::FastMutex::ScopedLock locker(m_queueMutex);
  std::multimap<Poco::Clock, TaskHandle>::iterator it = m_taskQueue.begin();
  while (it != m_taskQueue.end())
  {
    if (it->second.get<0>() == task && it->second.get<3>() == socket)
    {
      it = m_taskQueue.erase(it);
    }
    else
    {
      it++;
    }
  }
}

void JRLCServer::removeTimedTask(JRLCTimedTask task, Poco::Net::StreamSocket* socket, int interval, bool single)
{
  Poco::FastMutex::ScopedLock locker(m_queueMutex);
  std::multimap<Poco::Clock, TaskHandle>::iterator it = m_taskQueue.begin();
  while (it != m_taskQueue.end())
  {
    if (it->second.get<0>() == task && it->second.get<3>() == socket)
    {
      it = m_taskQueue.erase(it);
    }
    else
    {
      it++;
    }
  }
}
void JRLCServer::clearTimedTask()
{
  Poco::FastMutex::ScopedLock locker(m_queueMutex);
  m_taskQueue.clear();
}

void JRLCServer::GetCpuInfo(Poco::Net::StreamSocket* socket)
{
  // 获取 CPU 信息
  // std::cout << "CPU 型号: " << cpu->getCPUModel() << std::endl;
  // std::cout << "CPU 架构: " << cpu->getCPUArchitecture();
  // std::cout << "逻辑 CPU 个数: " << cpu->getLogicalCPUCores();
  // std::cout << "CPU核数: " << cpu->getPhysicalCPUCores() << std::endl;
  // std::cout << "CPU的使用率为: " << cpu->getCPUUsage(5) << "%" << std::endl;
  Poco::JSON::Object root;
  root.set("Msg_Type", "CPUInfo");
  // root.set("CPU_Model", cpu->getCPUModel());
  // root.set("CPU_Architecture", cpu->getCPUArchitecture());
  // root.set("CPU_Logical_Count", cpu->getLogicalCPUCores());
  // root.set("CPU_Count", cpu->getPhysicalCPUCores());
  // root.set("CPU_Usage", std::to_string(cpu->getCPUUsage(5)) + "%");
  SendJson(socket,root,REPLAYCPU);
}

void JRLCServer::GetMemInfo(Poco::Net::StreamSocket* socket)
{
  // unsigned long long totalRAM = memory->getTotalRAM();
  // unsigned long long freeRAM = memory->getFreeRAM();
  // unsigned long long usedRAM = memory->getUsedRAM();

  // 打印内存信息
  // std::cout << "总内存大小: " << totalRAM << " 字节\n";
  // std::cout << "可用内存大小: " << freeRAM << " 字节\n";
  // std::cout << "已使用内存大小: " << usedRAM << " 字节\n";
  Poco::JSON::Object root;
  root.set("Msg_Type", "MemInfo");
  root.set("totalRAM","totalRAM");
  root.set("freeRAM","freeRAM");
  root.set("usedRAM","usedRAM");
  SendJson(socket,root,REPLAYMEM);
}

void JRLCServer::GetHddInfo(Poco::Net::StreamSocket* socket)
{
  // 获取硬盘信息
  // unsigned long long totalDiskSpace = disk->getTotalDiskSpace();
  // unsigned long long freeDiskSpace = disk->getFreeDiskSpace();
  // unsigned long long usedDiskSpace = disk->getUsedDiskSpace();

  // 打印硬盘信息
  // std::cout << "总磁盘空间大小: " << totalDiskSpace << " 字节\n";
  // std::cout << "可用磁盘空间大小: " << freeDiskSpace << " 字节\n";
  // std::cout << "已使用磁盘空间大小: " << usedDiskSpace << " 字节\n";
  Poco::JSON::Object root;
  root.set("totalDiskSpace","totalDiskSpace");
  root.set("freeDiskSpace","freeDiskSpace");
  root.set("usedDiskSpace","usedDiskSpace");
  SendJson(socket,root,REPLAYHDD);

}

std::string JRLCServer::readFirstLineFromFile(const std::string &filename)
{
  std::ifstream inputFile(filename);

  if (!inputFile.is_open())
  {
    std::cerr << "Error opening file: " << filename << std::endl;
    return "";
  }

  std::string firstLine;
  if (std::getline(inputFile, firstLine))
  {
    inputFile.close();
    return firstLine;
  }
  else
  {
    inputFile.close();
    return "";
  }
}

JRLCServer::~JRLCServer()
{
  close(sockfd);
  for (auto &clientSock : clients)
  {
    close(clientSock);
  }
}

std::string JRLCServer::extractFileName(const std::string &absolutePath)
{
  std::regex pattern("/([^/]+)$");

  std::smatch matches;
  if (std::regex_search(absolutePath, matches, pattern))
  {
    return matches[1].str();
  }
  return "";
}
void signalHandler(int signal_num)
{
  std::cout << "Interrupt signal is (" << signal_num << ").\n";
  exit(signal_num);
}

int main(int argc, char const *argv[])
{
  signal(SIGINT, signalHandler);
  int port = 9090;
  JRLCServer server(port);
  return 0;
}
