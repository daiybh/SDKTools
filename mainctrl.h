#pragma  once
#include <windows.h>
#include "Camera.h"
#include "config.h"

class CMainCtrl
{
public:
	void init() {
		for (auto& item : m_config.m_Cameras)
		{
			item.camera->_ADD_LOG = &CMainCtrl::ADD_LOG_CALLBACK;
			item.camera->_NET_SMARTRECVCALLBACK_EX = &CMainCtrl::NET_SMARTRECVCALLBACK_EX;
			item.camera->_NET_CONSTAUSCALLBACK = &CMainCtrl::NET_CONSTAUSCALLBACK;

			printf("\n%p", item);
		}
		Load();
		int nret =m_tcpServer.start(m_config.localPort, [&](std::string& ip) {
			
			m_config.m_Cameras
			});
		SPDLOG_LOGGER_INFO("tcpServer start({}) {}", m_config.localPort, nret == 0 ? "sucessd" : "failed");
	}
	void Load() {
		for (auto& item : m_config.m_Cameras)
		{
			if(item.camera->m_caminstance==0)
				item.camera->connect();
		}
	}

	void __stdcall ADD_LOG_CALLBACK(const char* log, void* UserParam)
	{
		//if (g_pthis)
		//	g_pthis->addLog(log, (CCamera*)UserParam);

	}
	//状态回掉函数
	void __stdcall 		NET_CONSTAUSCALLBACK(NET_DEV_STATUS* status, void* UserParam) {
		//if (g_pthis)
		//	g_pthis->addLog("NET_CONSTAUSCALLBACK ", (CCamera*)UserParam);


	}

	// 精简的识别结果回调函数 
	void __stdcall 		NET_SMARTRECVCALLBACK_EX(NET_DEV_SMARTRECRESUT_EX* SmartResultEx, char* pJpeg, int* nLength, char* userdata, void* UserParam)
	{
		//if (g_pthis)
		//		g_pthis->NET_SMARTRECVCALLBACK_EX(SmartResultEx, pJpeg, nLength, (CCamera*)UserParam);		
		CCamera* pCamera = (CCamera*)UserParam;
		if (pCamera->m_curID >= m_config.m_Cameras.size())
			return;
		CameraOBJ& cameraOBJ = m_config.m_Cameras[pCamera->m_curID];

		SPDLOG_LOGGER_INFO("地点:%s IP:%s 车牌:%s realbility:%.2f carstatus:%d curID:%d bIn:%d\n",
			SmartResultEx->DevName, SmartResultEx->camerIp,
			SmartResultEx->platenum, SmartResultEx->realbility, SmartResultEx->carstatus,
			pCamera->m_curID, cameraOBJ.isIn);
		TcpClient client;
		for (int i = 0; i < 3; i++)
		{
			bool bret = client.ConnectToHost(m_config.serverIP, m_config.serverPort);
			if (!bret)
			{
				SPDLOG_LOGGER_INFO("ConnectToHost failed.try again {} {}", m_config.serverIP, m_config.serverPort);
				continue;
			}
			bret = client.sendCarComing(SmartResultEx->platenum, SmartResultEx->camerIp);
			if (bret)break;
			SPDLOG_LOGGER_INFO("sendCarComing failed.try again {} {}", SmartResultEx->platenum, SmartResultEx->camerIp);
		}
	}

private:
	Config m_config;
	TCPServer m_tcpServer;

};