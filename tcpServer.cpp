
#include <winsock2.h>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <WS2tcpip.h>
#include "tcpServer.h"
//#include <windows.h>
#pragma comment(lib, "Ws2_32.lib")
#define addLog(format,...) {}

int TCPServer::start(int _port, RisePoleFunc _func)
{
	//---------------
// create socket
	m_RisePoleFunc = _func;
	ADDRINFOA hints;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	ADDRINFOA* socketAddrInfo = nullptr;
	char port[20];
	sprintf_s(port, 20, "%d", _port);
	getaddrinfo(nullptr, port, &hints, &socketAddrInfo); // Resolve the local address and port to be used by the server
	std::shared_ptr<ADDRINFOA> socketAdd(socketAddrInfo, [](auto* item) { freeaddrinfo(item); });
	m_listenSocket = socket(socketAdd->ai_family, socketAdd->ai_socktype, socketAdd->ai_protocol); // Create a SOCKET for the server to listen for client connections

	const char enable = 1;
	//addLog("begin openPort(%d)", _port);
	if (setsockopt(m_listenSocket, SOL_SOCKET, SO_REUSEADDR, &enable, 1) == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		addLog("setsockopt failed.lasterror:(%d)", err);
		return err;
	}
	if (::bind(m_listenSocket, socketAdd->ai_addr, (int)socketAdd->ai_addrlen) == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		addLog("bind failed.lasterror:(%d)", err);
		return err;
	}
	//---------------
// listen socket

	if (listen(m_listenSocket, SOMAXCONN) == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		addLog("listen failed.lasterror:(%d)", err);
		return err;
	}
	for (int i = 0; i < 10; i++)
	{
		m_client[i] = -1;
	}
	startThread(EasyThread::Priority::normal);

	m_heartThread = std::thread(&TCPServer::heartThread,this);
	return 0;
}
#include "tcpBase.h"
#include "handleCmd.h"
void TCPServer::do_communication(int cfd)
{
	char buf[1024] = { 0 };
	int recvlen;
	TCPBase base;
	HandleCmd  handleCmd;
	while (1) {

		memset(buf, 0, sizeof(buf));
		int nret = base.ReadData(cfd, buf, 1024);
		if (nret<1)break;

		Header* pHeader = (Header*)buf;
		if(pHeader->cmdFlag!='C')continue;
		printf("\n\nrecvstr,\n%s", buf);

		/*
		 %CLOD0027RISEPOLE00
 BS20220001,192.168.0.243,20220407163254
 25@

 �ͻ�����Ӧ������:
 %RLOD0027RISEPOLE00
 BS20220001,192.168.0.243,20220407191532
 2A@
		*/
		if(!handleCmd.parseCmd(buf, nret))
			continue;

		RisePole* rp = (RisePole*)buf;
		if(!rp->isValid())continue;
		std::string body = rp->body;
		size_t pos = body.find(',');
		if (pos == std::string::npos)continue;
		size_t pos2 = body.find(',', pos + 1);
		if (pos2 == std::string::npos)continue;
		std::string ip = body.substr(pos + 1, pos2 - pos - 1);
		if (m_RisePoleFunc==nullptr)continue;

		bool bReet =m_RisePoleFunc(ip);

	}

	closesocket(cfd);
	return ;
}
#include "handleCmd.h"
void TCPServer::heartThread()
{
	HandleCmd  cmd;
	while (isRunning())
	{
		std::this_thread::sleep_for(std::chrono::seconds(10));
		std::string cmdStr = cmd.makeHeartCmd();
		for (int i = 0; i < 10; i++)
		{
			if (m_client[i] != -1)
			{
				int nret = send(m_client[i], cmdStr.data(), cmdStr.length(), 0);
				if (nret == SOCKET_ERROR)
				{
					std::lock_guard<std::shared_mutex> lock(m_lock);
					m_client[i] = -1;
				}
			}
		}
	}
}

void TCPServer::callBack()
{
	SOCKET clientSocket;
	std::thread clientThread[5];
	do
	{
		SOCKADDR clientAddr;
		int size = sizeof(clientAddr);
		clientSocket = accept(m_listenSocket, &clientAddr, &size);
		if (clientSocket != SOCKET_ERROR && INVALID_SOCKET != clientSocket)
		{
			SOCKADDR_IN sin;
			memcpy(&sin, &clientAddr, sizeof(sin));
			char clientIP[20] = { 0 };
			inet_ntop(sin.sin_family, &sin.sin_addr, clientIP, sizeof(clientIP));

			std::lock_guard<std::shared_mutex> lock(m_lock);
			for (int i = 0; i < 10; i++)
			{
				if (m_client[i] == -1)
				{
					m_client[i] = clientSocket; 
					std::thread t = std::thread(&TCPServer::do_communication, this, (int)clientSocket);
					t.detach();
					break;
				}
			}

		}
		/*else
		{
			int err = WSAGetLastError();
			printf_s("accept Error %ld socket %ld\n", err, (int)m_listenSocket);
		}*/
	} while (clientSocket != SOCKET_ERROR && isRunning());
}
