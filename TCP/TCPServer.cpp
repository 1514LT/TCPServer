#include "TCPServer.hpp"
JRLCServer::JRLCServer(int port)
{
  printf("server start\n");
  // std::thread(&JRLCServer::start, this, port).detach();
  try
  {
    // 设置监听地址和端口
    cpu = new cpuInfo();
    disk = new diskInfo();
    memory = new memoryInfo();
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
      // sockets.push_back(ss);
      map_sockets[ss->impl()->sockfd()] = ss;
      std::cout << "New connection from: " << ss->peerAddress().toString() << std::endl;
      client = new Socket();
      client->SetSocket(ss->impl()->sockfd());
      // clients.push_back(client);
      map_clients[ss->impl()->sockfd()] = client;
      std::thread handler(&JRLCServer::handleMsg, this, msgType, bodySize, serialNum, ss, client);
      handler.detach();
    }
  }
  catch (const std::exception &e)
  {
    std::cerr << e.what() << '\n';
  }
}
void JRLCServer::handleMsg(Poco::UInt8 msgType, Poco::UInt32 bodySize, Poco::UInt32 serialNum, Poco::Net::StreamSocket *ss, Socket *client)
{
  try
  {
    while (1)
    {
      if (ss->receiveBytes(&bodySize, sizeof(bodySize)) == 0)
      {
        perror("=====>client closed 7777\n");
        ss->close();
        client->Clear();
        auto it_sock = map_sockets.find(ss->impl()->sockfd());
        if (it_sock != map_sockets.end())
        {
          // delete it_sock->second;
          map_sockets.erase(it_sock);
        }
        auto it_client = map_clients.find(ss->impl()->sockfd());
        if (it_client != map_clients.end())
        {
          // delete it_client->second;
          map_clients.erase(it_client);
        }
        return;
      }
      // ss->receiveBytes(&bodySize, sizeof(bodySize));
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
    perror("=====>client closed erro\n");
    ss->close();
    client->Clear();
    auto it_sock = map_sockets.find(ss->impl()->sockfd());
    if (it_sock != map_sockets.end())
    {
      // delete it_sock->second;
      map_sockets.erase(it_sock);
    }
    auto it_client = map_clients.find(ss->impl()->sockfd());
    if (it_client != map_clients.end())
    {
      // delete it_client->second;
      map_clients.erase(it_client);
    }
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
    printf("jsonString:%s\n", jsonStr.c_str());
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
  char header[headLen + bodyLen];
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
  strcpy(p, jsonData);
  int len = send(cli_fd, header, headLen + bodyLen, 0);
  if(len == 0)
  {
    printf("client %d closed\n",cli_fd);
    return;
  }
  return;
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
  printf("enter ProcessCpu\n");
  client->stop_CPU = 0;
  while (!client->stop_CPU)
  {
    printf("client->stop_CPU:%d\n", client->stop_CPU);
    GetCpuInfo(ss, client);
  }
}
void JRLCServer::RemoveCpu(Poco::Net::StreamSocket *socket, Socket *client)
{
  client->stop_CPU = 1;
}


void JRLCServer::GetCpuInfo(Poco::Net::StreamSocket *socket, Socket *client)
{
  Poco::JSON::Object MonitorInfo;
  Poco::JSON::Object ResourceInfo;
  Poco::JSON::Object resourceUsage;
  Poco::JSON::Object HDD_root;
  Poco::JSON::Object MemoryInfo;
  /*CPU*/
  double total = 0.000;
  Poco::JSON::Array CPU_arry;
  for (int i = 0; i < cpu->getPhysicalCPUCores(); i++)
  {
    Poco::JSON::Object CPU_child;
    double used = cpu->getCpuUsage(i);
    used = static_cast<double>(static_cast<int>(used * 1000)) / 1000;
    total += used;
    char tmp[32];
    sprintf(tmp, "CPU%d", i);
    std::string key = tmp;
    CPU_child.set("CPUName", key);
    CPU_child.set("usage", used);
    CPU_arry.add(CPU_child);
  }
  total = total / cpu->getPhysicalCPUCores();
  resourceUsage.set("CPUTotalUsage", total);
  resourceUsage.set("subCPUUsages", CPU_arry);
  /*内存*/
  unsigned long long totalRAM = memory->getTotalRAM();
  unsigned long long freeRAM = memory->getFreeRAM();
  unsigned long long usedRAM = memory->getUsedRAM();
  MemoryInfo.set("total", std::to_string(totalRAM));
  MemoryInfo.set("used", std::to_string(usedRAM));
  MemoryInfo.set("free", std::to_string(freeRAM));
  /*磁盘*/
  Poco::JSON::Array DiskInfoUsage;

  std::vector<DiskInfo> disk_infos = disk->fetch_disk_info();
  for (const auto &info : disk_infos)
  {
    Poco::JSON::Object Disk;
    Disk.set("fileSystem", info.filesystem);
    Disk.set("size", info.capacity);
    Disk.set("used", info.used);
    Disk.set("avail", info.available);
    Disk.set("usePercentage", info.used_percentage);
    Disk.set("mountedOn", info.mount_point);
    DiskInfoUsage.add(Disk);
  }

  double perRAM = ((double)usedRAM / (double)totalRAM);
  cout << "usedRAM:" << usedRAM << " totalRAM:" << totalRAM << " perRAM:" << perRAM << endl;
  resourceUsage.set("memoryUsage", std::to_string(perRAM));
  double perDISK = ((double)disk->getUsedDiskSpace() / (double)disk->getTotalDiskSpace());
  resourceUsage.set("diskUsage", std::to_string(perDISK));

  ResourceInfo.set("resourceUsage", resourceUsage);
  ResourceInfo.set("memoryInfo", MemoryInfo);

  MonitorInfo.set("data", ResourceInfo);

  MonitorInfo.set("diskData", DiskInfoUsage);
  SendJson(socket, MonitorInfo, REPLAYCPU);
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
  printf("析构函数\n");
  for (auto &map_client : map_clients)
  {
    map_client.second->Clear();
    close(map_client.second->GetSocket());
    if (map_client.second != NULL)
    {
      printf("释放套接字:%d\n", map_client.first);
      free(map_client.second);
      map_client.second = NULL;
    }
  }
  for (auto &map_socket : map_sockets)
  {
    if (map_socket.second != NULL)
    {
      free(map_socket.second);
      map_socket.second = NULL;
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
