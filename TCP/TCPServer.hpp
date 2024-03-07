#ifndef TCPSERVER_HPP
#define TCPSERVER_HPP
#include "Base.hpp"
#include "FTPServer.hpp"
#include "Socket.hpp"
#define SILENCE_FILE 819200
#define POLLTIMEOUT 2500 * 1000
#define NETTIMEOUT 100 * 1000 * 1000

#define PROCESSLAUNCHER 26
#define ADDTIMER 27
#define REMOVETIMER 28

#define CPUINFO 30
#define REMOVECPU 31
#define HDDINFO 32
#define REMOVEHDD 33
#define MEMINFO 34
#define REMOVEMEM 35

#define REPLAYCPU 50
#define REPLAYHDD 52
#define REPLAYMEM 54
class JRLCServer : public Poco::Util::ServerApplication
{
private:
  std::map<int,Socket*> map_clients;
  std::map<int,Poco::Net::StreamSocket *> map_sockets;
  Poco::FastMutex m_queueMutex;

public:
  JRLCServer(int port);

  ~JRLCServer();

public:
  FTPServer* FTP;

  cpuInfo* cpu;

  diskInfo* disk;

  memoryInfo* memory;

  Socket* client;

  
  std::string extractFileName(const std::string& absolutePath);

  std::string readFirstLineFromFile(const std::string &filename);

  void handleMsg(Poco::UInt8 msgType, Poco::UInt32 bodySize, Poco::UInt32 serialNum, Poco::Net::StreamSocket *ss,Socket* client);

  void handleBody(Poco::Net::StreamSocket *ss, Poco::UInt8 msgType,Socket* client);


  std::string toJSONString(const Poco::JSON::Object &root);

  Poco::JSON::Object::Ptr toJSONObj(const std::string &jsonString);

  void SendJson(Poco::Net::StreamSocket *ss,const Poco::JSON::Object obj,int Msg_Type);

  // void start(int port);

public:

  void ProcessBinaryLaunch();

  void ProcessTimer(Poco::Net::StreamSocket *ss,Socket* client);

  void ProcessCpu(Poco::Net::StreamSocket *ss,Socket* client);

  void RemoveTimer(Poco::Net::StreamSocket* socket,Socket* client);

  void RemoveCpu(Poco::Net::StreamSocket* socket,Socket* client);

  void test(Poco::Net::StreamSocket* socket,Socket* client);

  void GetCpuInfo(Poco::Net::StreamSocket* socket,Socket* client);

};
#endif