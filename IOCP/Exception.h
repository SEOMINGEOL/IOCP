#pragma once
#ifndef __EXCETPION__H__
#define __EXCETPION__H__

#include "IOCPCommon.h"

class CommonException
{
public:
	virtual void ShowException() = 0;
};

class SocketException : public CommonException
{
private:
	int type;
public:
	SocketException(int type) : type(type)
	{}
	void ShowException()
	{
		switch (type)
		{
		case WinSock:
			Log_printf(Error, "Can not find winsock.dll file");
			break;
		case INVALID:
			Log_printf(Error, "Invalid socket");
			break;
		case BIND:
			Log_printf(Error, "Fail bind");
			break;
		case LISTEN:
			Log_printf(Error, "Fail listen");
			break;
		case ACCEPT:
			Log_printf(Error, "Accept Failure");
			break;
		}
	}
};

#endif // !__EXCETPION__H__
