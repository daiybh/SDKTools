#pragma once
#include "easyThread.h"
#include <shared_mutex>
#include <functional>
#include <logLib.h>
using RisePoleFunc =  std::function<bool(std::string& ip)>;

class TCPServer:public EasyThread {
public:
	int start(simplyLogger _logger,int port, RisePoleFunc _func);
private:

	uint64_t           m_listenSocket;;
	
	uint64_t m_client[10];
	void closeClient(int cfd);
	std::shared_mutex		m_lock;

	RisePoleFunc m_RisePoleFunc;
	std::thread m_heartThread;

	void heartThread();
protected:
	struct ClientInfo
	{
		int socketCfd;
		int ipos;
	};
	void do_communication(ClientInfo cfd);

	void callBack() override;	

	SimplyLive::Logger * m_logger;
};