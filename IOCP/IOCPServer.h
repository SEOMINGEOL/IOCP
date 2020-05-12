#pragma once
#ifndef __IOCP_SERVER_H__
#define __IOCP_SERVER_H__

#include "IOCPCommon.h"
#include "User.h"

class IOCPServer
{
private:
	HANDLE hIOCP;
public:
	IOCPServer();
	BOOL Start();
	static DWORD WINAPI workerThread(LPVOID hIOCP);
	static void AddUser(User* user);
	static void DeleteUser(SOCKET userSocket);
	static void SendAllClient(WSABUF* buf);
	static void Send_Data(SOCKET socket, WSABUF* buf);
	static void Read_Data(SOCKET socket, WSABUF* buf, WSAOVERLAPPED* overlapped);
};
#endif // !__IOCP_SERVER__


