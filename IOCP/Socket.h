#pragma once
#ifndef __IOCP_SOCKET_H__
#define __IOCP_SOCKET_H__

#pragma comment(lib, "Ws2_32.lib")

#include "IOCPCommon.h"
#include "Ws2tcpip.h"
#include "User.h"

struct SOCKETINFO
{
    WSAOVERLAPPED overlapped;
    WSABUF dataBuffer;
    SOCKET socket;
    char messageBuffer[MAX_BUF_SIZE];
    int receiveBytes;
    int sendBytes;
};

class Socket
{
private:
    WSAData wsaData;
    SOCKET serverSocket;
    SOCKADDR_IN serverAddr;
public:
    Socket();
    Socket(int port);
    ~Socket();

    void Bind();
    void Listen();
    User* Accept();
};

#endif // !__IOCP_SOCKET__
