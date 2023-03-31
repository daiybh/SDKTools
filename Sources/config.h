#pragma  once
#include "..\inc\Camera.h"
#include "syncVector.h"
#include "SyncMap.h"
#include <filesystem>
#include <Shlwapi.h>
#include "logLib.h"
static void CALLBACK addLog(const char* log, void* UserParam)
{
	//SPDLOG_ERROR("addLog....{}", log);
}

struct CameraOBJ
{
	std::string ip;
	bool isIn = false;
	int id = 0;
	bool isMaster = false;
	bool isValid()
	{
		return ip.length() > 6 && (ip != "0.0.0.0");
	}
	CCamera* camera = nullptr;
	CameraOBJ* masterObj=nullptr;
	CameraOBJ()
	{
		camera = new CCamera();
		camera->_ADD_LOG = addLog;
	}
};
class Config
{
public:
	static Config& instance() {
		static Config c;
		return c;
	}
	Config()
	{
		char szBuf1[MAX_PATH];
		ZeroMemory(szBuf1, MAX_PATH);
		GetModuleFileNameA(nullptr, szBuf1, MAX_PATH);
		PathRemoveFileSpecA(szBuf1);
		strcat(szBuf1, "\\sdkTool_Config.ini");
		m_ConfigPathA = szBuf1;
		m_logger = std::make_shared<SimplyLive::Logger>();
		m_logger->setPath(L".\\logs\\config.log");

		std::error_code error;
		if (!std::filesystem::exists(m_ConfigPathA, error)) //already have a file with the same name?
		{
			bWriteIni = true;
		}
		localPort = getConfigInt("main", "localListenPort","2345");
		serverPort = getConfigInt("server", "ListenPort","1983");

		getConfigString("server", "IP",szBuf1,serverIP.data());
		serverIP = szBuf1;

		getConfigString("server", "serverSN", szBuf1,"BS20220001");
		serverSN = szBuf1;
		loadCameras();

		monitorTHreadTime = getConfigInt("main", "monitor", "1") *1000;
		if (monitorTHreadTime < 1)
			monitorTHreadTime = 1000;
	}
	std::string serverSN;
	SyncVector<CameraOBJ*> m_Cameras;
	SyncMap<std::string,CameraOBJ*> m_cameraMap;
	std::string serverIP="127.0.0.1";
	int         serverPort=1983;
	int monitorTHreadTime = 1000;
	int localPort=2345;

private:
	bool bWriteIni = false;

	int getConfigInt(const char* szApp, const char* szKey,char*defaultValue="")
	{
		if (bWriteIni)
			WritePrivateProfileStringA(szApp, szKey, defaultValue, m_ConfigPathA.data());
		return GetPrivateProfileIntA(szApp, szKey, 0, m_ConfigPathA.data());
	}
	int getConfigString(const char* szApp, const char* szKey, char* retVal,char*defaultValue="")
	{
		if (bWriteIni)
			WritePrivateProfileStringA(szApp, szKey, defaultValue, m_ConfigPathA.data());
		return GetPrivateProfileStringA(szApp, szKey, defaultValue, retVal, MAX_PATH, m_ConfigPathA.data());
	}
	std::string m_ConfigPathA = ".\\sdkTool_Config.ini";

	bool loadCameras()
	{
		char szBuf1[MAX_PATH];
		char szKey[MAX_PATH];
		char szApp[MAX_PATH];
		int nret = 0;

		bool ipValid = false;
		auto isValidIP = [](std::string_view sIP) {
			if (sIP.length() < 7)return false;
			if (sIP.compare("0.0.0.0") == 0)return false;
			if (sIP.find(".") == sIP.npos)return false;

			return true;
		};
		auto pLoadIP = [&](std::string xIP) {
			std::vector<std::string> ipArr;
			int npos = xIP.find(',');
			if (npos == -1)
			{
				if (isValidIP(xIP))
					ipArr.emplace_back(xIP);

			}
			else
			{
				std::string ip0 = xIP.substr(0, npos);
				std::string ip1 = xIP.substr(npos + 1);
				if (isValidIP(ip0))
					ipArr.emplace_back(ip0);
				if (isValidIP(ip1))
					ipArr.emplace_back(ip1);
			}
			return ipArr;

		};
		int cameraCount = getConfigInt("main", "cameraCount");
		if (cameraCount < 1)cameraCount = 10;

		//In door
		for (int i = 0; i < cameraCount; i++)
		{
			sprintf_s(szKey, "ip_%d", i );

			nret = getConfigString("in", szKey, szBuf1);
			if (nret < 7)//1.1.1.1
			{
				m_logger->info("read ip_{} error, skip", i);
				if (!bWriteIni)continue;;
			}
			std::vector<std::string> ipArr = pLoadIP(szBuf1);
			if(ipArr.empty())continue;
			int j = 0;
			CameraOBJ* masterObj = nullptr;
			for (auto item : ipArr)
			{
				CameraOBJ *obj= new CameraOBJ();
				obj->isIn = true;
				if (j == 0)
					masterObj = obj;
				obj->isMaster = j==0;
				obj->masterObj = masterObj;
				obj->ip = item;
				obj->camera->m_curID = m_Cameras.size();
				strcpy(obj->camera->m_ipaddrstr, item.data());
				m_Cameras.emplace_back(obj);
				m_cameraMap.emplace(item, obj);
				j++;
			}
			ipValid = true;
		}
		//out door
		for (int i = 0; i < cameraCount; i++)
		{
			sprintf_s(szKey, "ip_%d", i);

			nret = getConfigString("out", szKey, szBuf1);
			if (nret < 7)//1.1.1.1
			{
				m_logger->info("read ip_{} error, skip", i);
				if (!bWriteIni)continue;;
			}
			std::vector<std::string> ipArr = pLoadIP(szBuf1);
			if (ipArr.empty())continue;
			int j = 0;
			CameraOBJ* masterObj = nullptr;

			for (auto item : ipArr)
			{
				CameraOBJ* obj = new CameraOBJ();
				obj->isIn = false;
				if (j == 0)
					masterObj = obj;
				obj->isMaster = j == 0;

				obj->masterObj = masterObj;
				obj->ip = item;
				obj->camera->m_curID = m_Cameras.size();
				strcpy(obj->camera->m_ipaddrstr, item.data());
				m_Cameras.emplace_back(obj);
				m_cameraMap.emplace(item, obj);

				j++;
			}
			ipValid = true;
		}


		if (!ipValid)
		{
			m_logger->info("read [main][ip_X] error, no one was valid! exit");
			return false;
		}
		for (int i=0;i< m_Cameras.size();i++)
		{
			CameraOBJ *item = m_Cameras[i];

			m_logger->info("in[{}] IP: {}", item->isIn,item->ip);
		}
		return true;
	}

	simplyLogger m_logger;
};