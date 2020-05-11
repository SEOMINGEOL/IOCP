#include "User.h"
#include "Ws2tcpip.h"

User::User()
{

}

User::~User()
{
	std::cout << "User distory" << std::endl;
}

User::User(SOCKET user_sock, SOCKADDR_IN user_addr)
{
	this->user_sock = user_sock;
	this->user_address = user_addr;
	//socketInfo = (struct SOCKETINFO*)malloc(sizeof(struct SOCKETINFO));
	//memset((void*)socketInfo, 0x00, sizeof(struct SOCKETINFO));
}

SOCKET User::GetUserSocket()
{
	return this->user_sock;
}

std::string User::GetUserIp()
{
	char ip[32];
	inet_ntop(PF_INET, &this->user_address.sin_addr, ip, sizeof(ip));

	return ip;
}