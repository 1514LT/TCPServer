#ifndef TCPSERVER_HPP
#define TCPSERVER_HPP
#include "Base.hpp"
#include "FTPServer.hpp"
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
  // typedef void (JRLCServer::*JRLCTimedTask)();
  typedef void (JRLCServer::*JRLCTimedTask)(Poco::Net::StreamSocket*);
private:
  int sockfd;

  std::vector<int> clients;

  Poco::FastMutex m_queueMutex;

  // typedef Poco::Tuple<JRLCTimedTask,int,bool> TaskHandle;
  typedef Poco::Tuple<JRLCTimedTask, int, bool, Poco::Net::StreamSocket*> TaskHandle;

  std::multimap<Poco::Clock, TaskHandle> m_taskQueue;
public:
  JRLCServer(int port);

  ~JRLCServer();

public:
  FTPServer* FTP;

  cpuInfo* cpu;

  diskInfo* disk;

  memoryInfo* memory;

  std::map<Poco::Net::StreamSocket*, JRLCTimedTask> socket_task_map;
  
  std::string extractFileName(const std::string& absolutePath);

  std::string readFirstLineFromFile(const std::string &filename);

  void handleMsg(Poco::UInt8 msgType, Poco::UInt32 bodySize, Poco::UInt32 serialNum, Poco::Net::StreamSocket *ss);

  void handleBody(Poco::Net::StreamSocket *ss, Poco::UInt8 msgType);

  // void addTimedTask(JRLCTimedTask task, int interval, bool single);
  void addTimedTask(JRLCTimedTask task, Poco::Net::StreamSocket* socket, int interval, bool single);

  // void removeTimedTask(JRLCTimedTask task);
  void removeTimedTask(JRLCTimedTask task, Poco::Net::StreamSocket* socket);

  // void removeTimedTask(JRLCTimedTask task, int interval, bool single);
  void removeTimedTask(JRLCTimedTask task, Poco::Net::StreamSocket* socket, int interval, bool single);

  void clearTimedTask();

  std::string toJSONString(const Poco::JSON::Object &root);

  Poco::JSON::Object::Ptr toJSONObj(const std::string &jsonString);

  void SendJson(Poco::Net::StreamSocket *ss,const Poco::JSON::Object obj,int Msg_Type);

public:
  void executeTasks();

  void ProcessBinaryLaunch();

  void ProcessTimer(Poco::Net::StreamSocket *ss);

  void ProcessCpu(Poco::Net::StreamSocket *ss);

  void ProcessHdd(Poco::Net::StreamSocket *ss);

  void ProcessMem(Poco::Net::StreamSocket *ss);

  void RemoveTimer(Poco::Net::StreamSocket* socket);

  void RemoveCpu(Poco::Net::StreamSocket* socket);

  void RemoveHdd(Poco::Net::StreamSocket* socket);

  void RemoveMem(Poco::Net::StreamSocket* socket);

  void test(Poco::Net::StreamSocket* socket);

  void GetCpuInfo(Poco::Net::StreamSocket* socket);

  void GetMemInfo(Poco::Net::StreamSocket* socket);

  void GetHddInfo(Poco::Net::StreamSocket* socket);
  
};
#endif