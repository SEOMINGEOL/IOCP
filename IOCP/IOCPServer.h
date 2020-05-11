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
	void AddUser(User* user);
	void DeleteUser(SOCKET userSocket);
};
#endif // !__IOCP_SERVER__


