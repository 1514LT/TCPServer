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
    while (true)
    {
      // 接受新的连接
      Poco::Net::StreamSocket *ss = new Poco::Net::StreamSocket(server.acceptConnection());
      std::cout << "New connection from: " << ss->peerAddress().toString() << std::endl;
      client = new Socket();
      client->SetSocket(ss->impl()->sockfd());
      clients.push_back(client);
      std::thread handler(&JRLCServer::handleMsg, this, msgType, bodySize, serialNum, ss, client);
      handler.detach();
    }
  }
  catch (const std::exception &e)
  {
    std::cerr << e.what() << '\n';
    free(client);
  }
}
void JRLCServer::handleMsg(Poco::UInt8 msgType, Poco::UInt32 bodySize, Poco::UInt32 serialNum, Poco::Net::StreamSocket *ss, Socket *client)
{
  try
  {
    while (1)
    {
      if(ss->receiveBytes(&bodySize, sizeof(bodySize)) < sizeof(bodySize))
      {
        perror("client closed");
        ss->close();
        client->Clear();
        break;
      }
      ss->receiveBytes(&msgType, sizeof(msgType));
      ss->receiveBytes(&serialNum, sizeof(serialNum));

      bodySize = Poco::ByteOrder::fromLittleEndian(bodySize);
      serialNum = Poco::ByteOrder::fromLittleEndian(serialNum);
      printf("enter handleMessage msgType:%d\n", msgType);
      handleBody(ss, msgType, client);
    }
  }
  catch (Poco::Net::ConnectionResetException &)
  {
    perror("client closed");
    ss->close();
    client->Clear();
    return;
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
void JRLCServer::handleBody(Poco::Net::StreamSocket *ss, Poco::UInt8 msgType, Socket *client)
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
      std::thread(&JRLCServer::ProcessTimer, this, ss, client).detach();
      break;
    case REMOVETIMER:
      RemoveTimer(ss, client);
      break;
    case CPUINFO:
      std::thread(&JRLCServer::ProcessCpu, this, ss, client).detach();
      break;
    case REMOVECPU:
      RemoveCpu(ss, client);
      break;
    case HDDINFO:
      std::thread(&JRLCServer::ProcessHdd, this, ss, client).detach();
      break;
    case REMOVEHDD:
      RemoveHdd(ss, client);
      break;
    case MEMINFO:
      std::thread(&JRLCServer::ProcessMem, this, ss, client).detach();
      break;
    case REMOVEMEM:
      RemoveMem(ss, client);
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
  strcpy(jsonData, result.c_str());
  printf("jsonData:%s\n", jsonData);
  int cli_fd = ss->impl()->sockfd();
  int payloadSize = 0;
  int headLen = 4 + 1 + 4;
  int bodyLen = 4 + 4 + jsonSize + payloadSize;
  char *header = (char *)malloc(headLen + bodyLen);
  char *p = header;
  int *bodySize = (int *)p;
  *bodySize = bodyLen;
  p += 4;
  char *msgType = (char *)p;
  *msgType = Msg_Type;
  p += 1;
  int *serialNum = (int *)p;
  *serialNum = 0;
  p += 4;
  int *jsonsize = (int *)p;
  *jsonsize = jsonSize;
  p += 4;
  int *payloadsize = (int *)p;
  *payloadsize = payloadSize;
  p += 4;
  memcpy(p, jsonData, jsonSize);
  send(cli_fd, header, headLen + bodyLen, 0);
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
}
void JRLCServer::test(Poco::Net::StreamSocket *socket, Socket *client)
{
  time_t currentTime = time(nullptr);
  struct tm *localTime = localtime(&currentTime);
  std::cout << "当前时间: " << (localTime->tm_hour < 10 ? "0" : "") << localTime->tm_hour << ":"
            << (localTime->tm_min < 10 ? "0" : "") << localTime->tm_min << ":"
            << (localTime->tm_sec < 10 ? "0" : "") << localTime->tm_sec << std::endl;
}
void JRLCServer::ProcessTimer(Poco::Net::StreamSocket *ss, Socket *client)
{
  client->stop_TES = 0;
  while (!client->stop_TES)
  {
    test(ss, client);
    sleep(1);
  }
}
void JRLCServer::RemoveTimer(Poco::Net::StreamSocket *socket, Socket *client)
{
  client->stop_TES = 1;
}
void JRLCServer::ProcessCpu(Poco::Net::StreamSocket *ss, Socket *client)
{
  client->stop_CPU = 0;
  while (!client->stop_CPU)
  {
    GetCpuInfo(ss, client);
    sleep(1);
  }
}
void JRLCServer::RemoveCpu(Poco::Net::StreamSocket *socket, Socket *client)
{
  client->stop_CPU = 1;
}
void JRLCServer::ProcessHdd(Poco::Net::StreamSocket *ss, Socket *client)
{
  client->stop_HDD = 0;
  while (!client->stop_HDD)
  {
    GetHddInfo(ss, client);
    sleep(1);
  }
}
void JRLCServer::RemoveHdd(Poco::Net::StreamSocket *socket, Socket *client)
{
  client->stop_HDD = 1;
}
void JRLCServer::ProcessMem(Poco::Net::StreamSocket *ss, Socket *client)
{
  client->stop_MEM = 0;
  while (!client->stop_MEM)
  {
    printf("client->stop_MEM:%d\n", client->stop_MEM);
    GetMemInfo(ss, client);
    sleep(1);
  }
}
void JRLCServer::RemoveMem(Poco::Net::StreamSocket *socket, Socket *client)
{
  client->stop_MEM = 1;
}

void JRLCServer::GetCpuInfo(Poco::Net::StreamSocket *socket, Socket *client)
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
  SendJson(socket, root, REPLAYCPU);
}

void JRLCServer::GetMemInfo(Poco::Net::StreamSocket *socket, Socket *client)
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
  root.set("totalRAM", "totalRAM");
  root.set("freeRAM", "freeRAM");
  root.set("usedRAM", "usedRAM");
  SendJson(socket, root, REPLAYMEM);
}

void JRLCServer::GetHddInfo(Poco::Net::StreamSocket *socket, Socket *client)
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
  root.set("totalDiskSpace", "totalDiskSpace");
  root.set("freeDiskSpace", "freeDiskSpace");
  root.set("usedDiskSpace", "usedDiskSpace");
  SendJson(socket, root, REPLAYHDD);
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
  for (auto &clientSock : clients)
  {
    clientSock->Clear();
    close(clientSock->GetSocket());
    if (clientSock != NULL)
    {
      free(clientSock);
    }
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
