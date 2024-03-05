#ifndef SOCKET_HPP
#define SOCKET_HPP
class Socket
{
private:
  int cli_fd;
public:
  void SetSocket(int cli_fd);
  int GetSocket();
  void Clear();
  Socket();
  Socket(int cli_fd);
  ~Socket();
public:
  int stop_CPU = 0;
  int stop_MEM = 0;
  int stop_HDD = 0;
  int stop_TES = 0;
};


#endif