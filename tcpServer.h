#pragma once
#include "easyThread.h"
#include <shared_mutex>
#include <functional>

using RisePoleFunc =  std::function<bool(std::string& ip)>;

class TCPServer:public EasyThread {
public:
	int start(int port, RisePoleFunc _func);
private:

	uint64_t           m_listenSocket;;
	
	uint64_t m_client[10];
	std::shared_mutex		m_lock;

	RisePoleFunc m_RisePoleFunc;
	std::thread m_heartThread;

	void heartThread();
protected:
	void do_communication(int cfd);

	void callBack() override;	
};