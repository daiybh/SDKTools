#pragma  once
#include <windows.h>
#include "Camera.h"
#include "config.h"
#include "tcpServer.h"
#include "tcpClient.h"
#include "logLib.h"
class MainCtrl
{
public:
	MainCtrl() {
		m_logger = std::make_shared<SimplyLive::Logger>();
		m_logger->setPath(L".\\logs\\mainctrl.log");
	}
	void Test();
	void init();
	void TryReconnectCameras(simplyLogger _logger);

    void RestartCameras(simplyLogger _logger);

    void __stdcall ADD_LOG_CALLBACK(const char *log, void *UserParam);
    //状态回掉函数
	void __stdcall 		NET_CONSTAUSCALLBACK(NET_DEV_STATUS* status, void* UserParam);

	// 精简的识别结果回调函数 
	void __stdcall 		NET_SMARTRECVCALLBACK_EX(NET_DEV_SMARTRECRESUT_EX* SmartResultEx, char* pJpeg, int* nLength, char* userdata, void* UserParam);
	bool RaisePole(std::string& ip);
private:
	TCPServer m_tcpServer;
	std::thread m_heartThread;
	std::thread m_moniterThread;
	simplyLogger m_logger;
	bool bExit=false;
};