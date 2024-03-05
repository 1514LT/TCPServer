#ifndef BASE_HPP
#define BASE_HPP
#include <iostream>
#include <vector>
#include <thread>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <fstream>
#include <regex>
#include <sys/stat.h>
#include <csignal>
#include <tuple>
#include "Poco/JSON/JSON.h"
#include "Poco/JSON/Object.h"
#include "Poco/JSON/Parser.h"
#include "Poco/JSON/Array.h"
#include "Poco/Types.h"
#include "Poco/Buffer.h"
#include "Poco/BasicEvent.h"
#include "Poco/Mutex.h"
#include "Poco/Clock.h"
#include "Poco/SharedPtr.h"
#include "Poco/Activity.h"
#include "Poco/Process.h"
#include "Poco/PipeStream.h"
#include "Poco/Types.h"
#include "Poco/ByteOrder.h"
#include "Poco/Net/SocketStream.h"
#include "Poco/Net/NetException.h"
#include "Poco/Net/TCPServerConnection.h"
#include "Poco/Util/ServerApplication.h"
#include "Poco/Tuple.h"

#include "Poco/Net/TCPServer.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/Net/StreamSocket.h"
#include "Poco/Net/SocketAddress.h"

#include "cpu-info.h"
#include "hdd-info.h"
#include "mem-info.h"

#define CONFIG_SAVA  "/config/FTPSavePath.txt"
#define SILENCE_FILE 819200
using namespace std;

class ProcessLauncher {
public:

  ProcessLauncher(){}
  ~ProcessLauncher(){}
  ProcessLauncher(const std::string& program): _program(program) {}
  Poco::ProcessHandle getHandle(); 
  int launch(const std::vector<std::string>& arguments);
  int launch(std::string _program,const std::vector<std::string>& arguments,Poco::Pipe &outpipe);
  void readFromPipe(Poco::Pipe& outpipe);
private:
  std::string _program;
  Poco::SharedPtr<Poco::ProcessHandle> PHandle;
};

#endif