#pragma  once
#include <windows.h>
#include "Camera.h"
#include "config.h"
#include "tcpServer.h"
#include "tcpClient.h"

class MainCtrl
{
public:
	void init();
	void Load();

	void __stdcall ADD_LOG_CALLBACK(const char* log, void* UserParam);
	//״̬�ص�����
	void __stdcall 		NET_CONSTAUSCALLBACK(NET_DEV_STATUS* status, void* UserParam);

	// �����ʶ�����ص����� 
	void __stdcall 		NET_SMARTRECVCALLBACK_EX(NET_DEV_SMARTRECRESUT_EX* SmartResultEx, char* pJpeg, int* nLength, char* userdata, void* UserParam);
	bool RaisePole(std::string& ip);
private:
	Config m_config;
	TCPServer m_tcpServer;

};