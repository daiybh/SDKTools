
#include "mainctrl.h"
#include "logLib.h"
#include <conio.h>
#include "httpClient.h"
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
		uint64_t m_nRestartTime = 0;
		while (!bExit)
		{
			TryReconnectCameras(_logger);
			Sleep(Config::instance().monitorTHreadTime);

			auto pRestartFun = [&]() {
				struct tm local;
				time_t t;
				t = time(NULL);
				localtime_s(&local, &t);
				bool bNeedRestart1 = false;// (local.tm_hour == 1 && local.tm_min == 0 && local.tm_sec == 0);
				//check if Now is the restart point.  01:00:xx
				bool bNeedRestart2 = 
				(local.tm_hour == Config::instance().reStartHour 
				&& local.tm_min == Config::instance().reStartMinute
				&& local.tm_sec == Config::instance().reStartSecond);
				
				// check if the app Had start.  
				// if no  then  (GetTickCount() - m_nRestartTime) will be a huge
				if ((bNeedRestart2) && (GetTickCount() - m_nRestartTime > 1000 * 60 * (2)))
				{
					m_nRestartTime = GetTickCount();
					return true;
				}
				return false;
			};
			if (pRestartFun())
			{
				_logger->error("============RERSATR  begin=============");
				RestartCameras(_logger);
				_logger->error("============RERSATR  DONE=============");
			}
			
		}

		});
	
		
	//while (!bExit)Sleep(1000);
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

void MainCtrl::RestartCameras(simplyLogger _logger)
{
	for (int i = 0; i < Config::instance().m_Cameras.size(); i++)
	{
		auto item = Config::instance().m_Cameras[i];
		{
			bool b = item->camera->set_EYEST_NET_RESTART();
			_logger->info("{}>>{} RestartCameras()={} ", i, item->ip, b);
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
	for (int i = 0; i < 1; i++)
	{
		HttpClient::ResponseItem response=HttpClient::isInDsBlack(SmartResultEx->camerIp, SmartResultEx->platenum);
		if (response.state == 0)
		{
			m_logger->error("state=0 errormsg={}",response.errmsg );
			continue;
		}
		if (response.state == 1)
		{
			//send to LED
			pCamera->LEDScreen_setText(response.ips, response.texts);
		}
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
