#include "FTPServer.hpp"
void FTPServer::communicate(int clientSock,Message &msg)
{
    recvFileName(clientSock,msg);
    getFileName(clientSock,msg);
    receiveFile(clientSock,msg);
    // printf("file name :%s\n",msg.DestFileName);
    // receiveFile(clientSock,"external.tar.gz");
    close(clientSock);
}

int FTPServer::recvFileName(int clientSock,Message &msg)
{
    int flage;
    char fileName[64];
    int flieNameLen;
    //deal name
    bzero(fileName,sizeof(fileName));   
    flage=read(clientSock,fileName,64);
    if (flage < 0)
    {
        std::cerr << "Error reading from fileName" << std::endl;
        return -1;
    }
    char *p=fileName;
    int *len=(int*)p;
    flieNameLen=*len;
    p+=4;
    int *binary_len=(int*)p;
    msg.FileSize=*binary_len;
    p+=4;
    msg.FileNameSize=flieNameLen;
    printf("fileNameSize:%d\n",flieNameLen);
    printf("fileName:%s\n",p);
    printf("BinarySize:%d\n",msg.FileSize);
    strcpy(msg.FileName,p);
    write(clientSock,"RECV File Name",32);
    return 0;
}

int FTPServer::getFileName(int clientSock,Message &msg)
{
    printf("---enter saveFlie function---\n");
    //将文件名分割出来
    std::string FlieName=extractFileName(msg.FileName);
    printf("File Name :%s\n",FlieName.c_str());
    //读取配置文件
    std::string DestFilePath=readFirstLineFromFile(CONFIG_SAVA);
    std::string DestFileName=DestFilePath + "/" + FlieName;
    strcpy(msg.DestFileName,DestFileName.c_str());
    printf("save File:%s\n",msg.DestFileName);
    return 0;
}
void FTPServer::receiveFile(int clientSock,const char* filename) {
    
    std::ofstream file(filename, std::ios::out | std::ios::binary);

    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return;
    }

    const int bufferSize = 1024;
    const int header = 9;
    char buffer[bufferSize + header];
    char date[bufferSize];
    ssize_t bytesRead;
    while ((bytesRead = recv(clientSock, buffer, bufferSize + header, 0)) > 0) {
        printf("recv :%ld\n",bytesRead);
        //拆包存储
        char *p=buffer;
        int *len=(int *)p;
        printf("len:%d\n",*len);
        p+=4;
        char *type=p;
        printf("type:%c\n",*type);
        p+=1;
        int *order=(int*)p;
        printf("order:%d\n",*order);
        p+=4;
        if(bytesRead == bufferSize + header)
        {
            memcpy(date,p,bufferSize);
        }
        else
        {
            memcpy(date,p,bytesRead);
        }
        file.write(date, bytesRead - header);
    }

    if (bytesRead < 0) {
        std::cerr << "Error receiving file data." << std::endl;
    }

    file.close();
    std::cout << "File received successfully: " <<filename << std::endl;
}

void FTPServer::receiveFile(int clientSock,Message &msg) {
    const char* filename=msg.DestFileName;
    // char *filename;
    // strcpy(filename,msg.DestFileName);
    // printf("filename:%s\n",filename);
    std::ofstream file(filename, std::ios::out | std::ios::binary);

    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return;
    }

    const int bufferSize = 1024;
    const int header = 9;
    char buffer[bufferSize + header];
    char date[bufferSize];
    ssize_t bytesRead;
    int index = 0;
    while ((bytesRead = recv(clientSock, buffer, bufferSize + header, 0)) > 0) {
        printf("recv :%ld\n",bytesRead);
        //拆包存储
        char *p=buffer;
        int *len=(int *)p;
        printf("len:%d\n",*len);
        p+=4;
        char *type=p;
        printf("type:%c\n",*type);
        p+=1;
        int *order=(int*)p;
        printf("order:%d\n",*order);
        printf("index:%d\n",index);
        if(*order!=index)
        {
            send(clientSock,"404",4,0);
            memset(buffer,0,bufferSize + header);
            if(recv(clientSock,buffer,bufferSize + header,0) > 0)
            {
                p=buffer;
                len=(int *)p;
                printf("len:%d\n",*len);
                p+=4;
                type=p;
                printf("type:%c\n",*type);
                p+=1;
                order=(int*)p;
                printf("order:%d\n",*order);
                printf("index:%d\n",index);
                if(*order!=index){
                    printf("rewrite\n");
                    send(clientSock,"400",4,0);
                    return;
                }
            }
            else
            {
                printf("RECV ERRO\n");
                return;
            }
        }
        else
        {
            send(clientSock,"200",4,0);
        }
        p+=4;
        if(bytesRead == bufferSize + header)
        {
            memcpy(date,p,bufferSize);
        }
        else
        {
            memcpy(date,p,bytesRead);
        }
        file.write(date, bytesRead - header);
        index+=1;
    }
}
std::string FTPServer::extractFileName(const std::string& absolutePath) {
    std::regex pattern("/([^/]+)$");

    std::smatch matches;
    if (std::regex_search(absolutePath, matches, pattern)) {
        return matches[1].str();
    }
    return "";
}

std::string FTPServer::readFirstLineFromFile(const std::string& filename) {
    std::ifstream inputFile(filename);

    if (!inputFile.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return "";
    }

    std::string firstLine;
    if (std::getline(inputFile, firstLine)) {
        inputFile.close();
        return firstLine;
    } else {
        inputFile.close();
        return "";
    }
}