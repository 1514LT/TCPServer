#ifndef FTPSERVER_HPP
#define FTPSERVER_HPP
#include "Base.hpp"
struct Message {
    int FileSize;
    int FileNameSize;
    char FilelType;
    char *FileDate;
    char FileName[64];
    char DestFilePath[64];
    char DestFileName[64];
};
class FTPServer
{
private:
    /* data */
public:
    void communicate(int clientSock,Message &msg);
    int recvFileName(int clientSock,Message &msg);
    int getFileName(int clientSock,Message &msg);
    void receiveFile(int clientSock, Message &msg);
    void receiveFile(int clientSock, const char * filename);
    std::string extractFileName(const std::string& absolutePath);
    std::string readFirstLineFromFile(const std::string& filename);
};


#endif