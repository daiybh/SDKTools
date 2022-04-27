#pragma once
#include "handleCmd.h"
#include "tcpBase.h"
class TcpClient:public TCPBase
{
public:
	TcpClient();
	~TcpClient();
	bool ConnectToHost(const char* serverip, const int port);
	int Send(const char* buffer, const int buflen);
	int Recv(char* buffer, const int buflen);

	void  sendHeartBeat() {

	}
	bool sendCarComing(std::string carNo, std::string ip) {
		auto cmdstr = handleCmd.makeCarcomingCmd(carNo, ip);
		int nRet = Send(cmdstr.data(), cmdstr.length());
		auto retStr = ReadData(m_sockfd);
		printf("\n retStr \n %s", retStr);
		return true;
	}
private:

	
	void close(int fd);
	int m_sockfd;
	HandleCmd handleCmd;
};