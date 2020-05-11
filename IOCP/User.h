#pragma once
#ifndef __USER_H__
#define __USER_H__

#include <iostream>
#include "IOCPCommon.h"

class User
{
private:
	SOCKET user_sock;
	SOCKADDR_IN user_address;
public:
	User();
	~User();
	User(SOCKET user_sock, SOCKADDR_IN user_addr);
	SOCKET GetUserSocket();
	std::string GetUserIp();
	int GetuserPort();
};

#endif // !__USER__

