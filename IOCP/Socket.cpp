#include "Socket.h"
#include "Exception.h"

Socket::Socket()
{

}

Socket::Socket(int port)
{
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        throw SocketException(WinSock);
    }

    // 1. ���ϻ���  
    serverSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    if (serverSocket == INVALID_SOCKET)
    {
        throw SocketException(INVALID);
    }

    // �������� ��ü����
    memset(&serverAddr, 0, sizeof(SOCKADDR_IN));
    serverAddr.sin_family = PF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
}

Socket::~Socket()
{
    closesocket(serverSocket);
    WSACleanup();
}

void Socket::Bind()
{
    // ���ϼ���
    if (bind(serverSocket, (struct sockaddr*) & serverAddr, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
    {
        closesocket(serverSocket);
        WSACleanup();
        throw SocketException(BIND);
    }
}

void Socket::Listen()
{
    //��⿭ ���� ��⿭�ϻ� �ִ� ���������� �ƴ�
    if (listen(serverSocket, 128) == SOCKET_ERROR)
    {
        closesocket(serverSocket);
        WSACleanup();
        throw SocketException(LISTEN);
    }
}



User* Socket::Accept()
{
    SOCKET clientSocket;
    SOCKADDR_IN clientAddr;

    int addrLen = sizeof(SOCKADDR_IN);

    clientSocket = accept(serverSocket, (struct sockaddr*) &clientAddr, &addrLen);
    if (clientSocket == INVALID_SOCKET)
    {
        throw SocketException(ACCEPT);
    }

    return new User(clientSocket, clientAddr);
}