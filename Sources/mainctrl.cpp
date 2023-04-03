#include "mainctrl.h"
#include "logLib.h"
#include <conio.h>
MainCtrl* g_pthis = nullptr;
namespace StaticClass {

	static void __stdcall ADD_LOG_CALLBACK(const char* log, void* UserParam)
	{
		//if (g_pthis)			g_pthis->addLog(log, (CCamera*)UserParam);

	}
	//状态回掉函数
	static void __stdcall 		NET_CONSTAUSCALLBACK(NET_DEV_STATUS* status, void* UserParam) {
		if (g_pthis)
			g_pthis->NET_CONSTAUSCALLBACK(status, UserParam);


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
void MainCtrl::Test()
{
	std::thread t[30];

	for (int j = 0; j < Config::instance().m_Cameras.size(); j++)
	{
		t[j] = std::thread([&](int i) {

			NET_DEV_SMARTRECRESUT_EX ndes;
			strcpy(ndes.DevName, "test");
			strcpy(ndes.camerIp, "19.19.19.19");
			strcpy(ndes.platenum, "platenum");
			ndes.realbility = 9;
			ndes.carstatus = 10;
			CameraOBJ* obj = Config::instance().m_Cameras[i];
			strcpy(ndes.camerIp, obj->ip.data());
			ndes.carstatus = i;
			while (1)
			{
				this->NET_SMARTRECVCALLBACK_EX(&ndes, nullptr, 0, 0, (void*)obj->camera);
				Sleep(1000);
			}
			}, j);
	}
	
	t[0].join();
	
	
}
void MainCtrl::init()
{
	m_logger->info("init");
	g_pthis = this;
	for (int i = 0; i < Config::instance().m_Cameras.size(); i++)
	{
		auto item = Config::instance().m_Cameras[i];

		item->camera->_ADD_LOG = &StaticClass::ADD_LOG_CALLBACK;
		item->camera->_NET_SMARTRECVCALLBACK_EX = &StaticClass::NET_SMARTRECVCALLBACK_EX;
		item->camera->_NET_CONSTAUSCALLBACK = &StaticClass::NET_CONSTAUSCALLBACK;

		printf("\n%p", item);
	}

	m_moniterThread = std::thread([&]() {	

		simplyLogger _logger = std::make_shared<SimplyLive::Logger>();
		_logger->setPath(L"\\logs\\moniter.log");
		while (!bExit)
		{
			TryReconnectCameras(_logger);
			Sleep(Config::instance().monitorTHreadTime);
		}

		});
	m_heartThread = std::thread([&]() {
		simplyLogger m_logger = std::make_shared<SimplyLive::Logger>();
		m_logger->setPath(L"\\logs\\main_heartThread.log");
		while (!bExit) {
			Sleep(10 * 1000);
			TcpClient client;
			bool bret = client.ConnectToHost(Config::instance().serverIP.data(), Config::instance().serverPort);
			if (!bret)			
				m_logger->error("ConnectToHost failed. {} {}", Config::instance().serverIP, Config::instance().serverPort);
			else
				client.sendHeartBeat();
		}
		});
	int nret = m_tcpServer.start(m_logger,Config::instance().localPort, std::bind(&MainCtrl::RaisePole,this,  std::placeholders::_1));
	m_logger->info("tcpServer start({}) {}", Config::instance().localPort, nret == 0 ? "sucessd" : "failed");
	int  key = 0;
	uint64_t tloop = 0;
	
	while (!bExit)
	{
		Sleep(10);
		
		if (_kbhit() == 0)
			continue;

		key = _getch();
		if (key == 'Q') {
			bExit = true;
			break;
		}
		else if (key == 'T') {
			//Test();
		}

	}
	m_heartThread.join();
	m_moniterThread.join();
}

void MainCtrl::TryReconnectCameras(simplyLogger _logger)
{
	for (int i = 0; i < Config::instance().m_Cameras.size(); i++)
	{
		auto item = Config::instance().m_Cameras[i];
		if (item->camera->m_caminstance <= 0)
		{
			bool b = item->camera->tryReconnect();
			_logger->info("{}>>{} reconnect()={} ", i, item->ip, b);
		}
	}
}

void __stdcall MainCtrl::ADD_LOG_CALLBACK(const char* log, void* UserParam)
{
	//if (g_pthis)
	//	g_pthis->addLog(log, (CCamera*)UserParam);
}

void __stdcall MainCtrl::NET_CONSTAUSCALLBACK(NET_DEV_STATUS* status, void* UserParam)
{
	CCamera* pCamera = (CCamera*)UserParam;
	if (pCamera == nullptr)return;
	m_logger->info("NET_CONSTAUSCALLBACK {} status:{}",pCamera->m_ipaddrstr,status->status);
}

void __stdcall MainCtrl::NET_SMARTRECVCALLBACK_EX(NET_DEV_SMARTRECRESUT_EX* SmartResultEx, char* pJpeg, int* nLength, char* userdata, void* UserParam)
{
	//if (g_pthis)
	//		g_pthis->NET_SMARTRECVCALLBACK_EX(SmartResultEx, pJpeg, nLength, (CCamera*)UserParam);		
	CCamera* pCamera = (CCamera*)UserParam;
	if (pCamera->m_curID >= Config::instance().m_Cameras.size())
		return;
	CameraOBJ* cameraOBJ = Config::instance().m_Cameras[pCamera->m_curID];

	m_logger->info("地点:{} IP:{} 车牌:{} realbility:{} carstatus:{} curID:{} bIn:{}",
		SmartResultEx->DevName, SmartResultEx->camerIp,
		SmartResultEx->platenum, SmartResultEx->realbility, SmartResultEx->carstatus,
		pCamera->m_curID, cameraOBJ->isIn);
	TcpClient client;
	for (int i = 0; i < 3; i++)
	{
		bool bret = client.ConnectToHost(Config::instance().serverIP.data(), Config::instance().serverPort);
		if (!bret)
		{
			m_logger->error("ConnectToHost failed.try again {} {}", Config::instance().serverIP, Config::instance().serverPort);
			continue;
		}
		bret = client.sendCarComing(m_logger,SmartResultEx->platenum, SmartResultEx->camerIp);
		if (bret)break;
		m_logger->error("sendCarComing failed.try again {} {}", SmartResultEx->platenum, SmartResultEx->camerIp);
	}
}

bool MainCtrl::RaisePole(std::string& ip)
{
	CameraOBJ* obj = nullptr;
	if (!Config::instance().m_cameraMap.find(ip, obj))
	{
		m_logger->error("raisePole  {} failed .don't found correct IP", ip);
		return false;
	}
	
	bool tryReconnect = obj->masterObj->camera->tryReconnect();
	bool b = obj->masterObj->camera->openDoor();
	m_logger->info("raisePole  {} ,reconnet({}) openDoor={}", ip, tryReconnect, b);
	return true;
} 
