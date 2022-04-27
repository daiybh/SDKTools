#include "mainctrl.h"
#include "myLogger.h"
MainCtrl* g_pthis = nullptr;
namespace StaticClass {

	static void __stdcall ADD_LOG_CALLBACK(const char* log, void* UserParam)
	{
		//if (g_pthis)			g_pthis->addLog(log, (CCamera*)UserParam);

	}
	//状态回掉函数
	static void __stdcall 		NET_CONSTAUSCALLBACK(NET_DEV_STATUS* status, void* UserParam) {
		//if (g_pthis)			g_pthis->addLog("NET_CONSTAUSCALLBACK ", (CCamera*)UserParam);


	}

	// 精简的识别结果回调函数 
	static void __stdcall 		NET_SMARTRECVCALLBACK_EX(NET_DEV_SMARTRECRESUT_EX* SmartResultEx, char* pJpeg, int* nLength, char* userdata, void* UserParam)
	{
		if (g_pthis)
		{
			g_pthis->NET_SMARTRECVCALLBACK_EX(SmartResultEx,pJpeg,nLength,userdata, (CCamera*)UserParam);
		}
	}
};
void MainCtrl::init()
{
	g_pthis = this;
	for (int i = 0; i < m_config.m_Cameras.size(); i++)
	{
		auto item = m_config.m_Cameras[i];

		item->camera->_ADD_LOG = &StaticClass::ADD_LOG_CALLBACK;
		item->camera->_NET_SMARTRECVCALLBACK_EX = &StaticClass::NET_SMARTRECVCALLBACK_EX;
		item->camera->_NET_CONSTAUSCALLBACK = &StaticClass::NET_CONSTAUSCALLBACK;

		printf("\n%p", item);
	}
	Load();
	int nret = m_tcpServer.start(m_config.localPort, std::bind(&MainCtrl::RaisePole,this,  std::placeholders::_1));
	SPDLOG_INFO("tcpServer start({}) {}", m_config.localPort, nret == 0 ? "sucessd" : "failed");
}

void MainCtrl::Load()
{
	for (int i = 0; i < m_config.m_Cameras.size(); i++)
	{
		auto item = m_config.m_Cameras[i];
		if (item->camera->m_caminstance == 0)
			item->camera->connect();
	}
}

void __stdcall MainCtrl::ADD_LOG_CALLBACK(const char* log, void* UserParam)
{
	//if (g_pthis)
	//	g_pthis->addLog(log, (CCamera*)UserParam);
}

void __stdcall MainCtrl::NET_CONSTAUSCALLBACK(NET_DEV_STATUS* status, void* UserParam)
{
	//if (g_pthis)
	//	g_pthis->addLog("NET_CONSTAUSCALLBACK ", (CCamera*)UserParam);
}

void __stdcall MainCtrl::NET_SMARTRECVCALLBACK_EX(NET_DEV_SMARTRECRESUT_EX* SmartResultEx, char* pJpeg, int* nLength, char* userdata, void* UserParam)
{
	//if (g_pthis)
	//		g_pthis->NET_SMARTRECVCALLBACK_EX(SmartResultEx, pJpeg, nLength, (CCamera*)UserParam);		
	CCamera* pCamera = (CCamera*)UserParam;
	if (pCamera->m_curID >= m_config.m_Cameras.size())
		return;
	CameraOBJ* cameraOBJ = m_config.m_Cameras[pCamera->m_curID];

	SPDLOG_INFO("地点:%s IP:%s 车牌:%s realbility:%.2f carstatus:%d curID:%d bIn:%d\n",
		SmartResultEx->DevName, SmartResultEx->camerIp,
		SmartResultEx->platenum, SmartResultEx->realbility, SmartResultEx->carstatus,
		pCamera->m_curID, cameraOBJ->isIn);
	TcpClient client;
	for (int i = 0; i < 3; i++)
	{
		bool bret = client.ConnectToHost(m_config.serverIP.data(), m_config.serverPort);
		if (!bret)
		{
			SPDLOG_INFO("ConnectToHost failed.try again {} {}", m_config.serverIP, m_config.serverPort);
			continue;
		}
		bret = client.sendCarComing(SmartResultEx->platenum, SmartResultEx->camerIp);
		if (bret)break;
		SPDLOG_INFO("sendCarComing failed.try again {} {}", SmartResultEx->platenum, SmartResultEx->camerIp);
	}
}

bool MainCtrl::RaisePole(std::string& ip)
{
	CameraOBJ* obj = nullptr;
	if (!m_config.m_cameraMap.find(ip, obj))
	{

		SPDLOG_ERROR("raisePole  {} failed .don't found correct IP", ip);
		return false;
	}

	bool b = obj->masterObj->camera->openDoor();
	SPDLOG_INFO("raisePole  {}  openDoor={}", ip, b);
	return true;
}
