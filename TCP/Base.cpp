#include "Base.hpp"
int ProcessLauncher::launch(const std::vector<std::string>& arguments)
{
  Poco::Pipe outPipe;
  Poco::ProcessHandle PH = Poco::Process::launch(_program, arguments, 0, &outPipe, 0);
  PHandle = new Poco::ProcessHandle(PH);
  return 0;
}

int ProcessLauncher::launch(std::string _program,const std::vector<std::string>& arguments,Poco::Pipe &outpipe)
{
  Poco::ProcessHandle PH = Poco::Process::launch(_program, arguments, 0, 0, 0);
  PHandle = new Poco::ProcessHandle(PH);
  return 0;
}
void ProcessLauncher::readFromPipe(Poco::Pipe& outpipe)
{
  Poco::PipeInputStream istr(outpipe);
  std::string line;
  while (std::getline(istr, line))
  {
      std::cout << line << std::endl;  // 写入到主进程的标准输出
  }
}

Poco::ProcessHandle ProcessLauncher::getHandle()
{
    if(PHandle)
    {
      return *this->PHandle;
    }
    else
    {
      throw Poco::NullPointerException("Process Handle Not Initialized");
    }    
}
