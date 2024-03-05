#include "Socket.hpp"
Socket::Socket()
{
  this->cli_fd = -1;
}
Socket::Socket(int cli_fd):cli_fd(cli_fd){}
Socket::~Socket(){}
void Socket::SetSocket(int cli_fd)
{
  this->cli_fd=cli_fd;
}

int Socket::GetSocket()
{
  return this->cli_fd;
}



void Socket::Clear()
{
  this->stop_CPU = 1;
  this->stop_MEM = 1;
  this->stop_HDD = 1;
  this->stop_TES = 1;
}
