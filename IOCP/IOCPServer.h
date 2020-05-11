#pragma once
#ifndef __IOCP_SERVER_H__
#define __IOCP_SERVER_H__

#include "IOCPCommon.h"

class IOCPServer
{
private:
	HANDLE hIOCP;
public:
	IOCPServer();
	BOOL Start();
};
#endif // !__IOCP_SERVER__


