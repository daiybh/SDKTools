
#include "..\inc\Camera.h"

static void CALLBACK addLog(const char* log, void* UserParam)
{
	SPDLOG_ERROR("addLog....{}", log);
}
/*
[1]
IP_0=

stime1=
etime1=
AEMaxTime1=
AVGLight1=
AGain1=
Dlight1=

stime2=
etime2=
AEMaxTime2=
AVGLight2=
AGain2=
Dlight3=

stime3=
etime3=
AEMaxTime3=
AVGLight3=
AGain3=
Dlight3=

stime4=
etime4=
AEMaxTime4=
AVGLight4=
AGain4=
Dligh4t=
*/
class Config
{
public:
	Config()
	{
		char szBuf1[MAX_PATH];
		ZeroMemory(szBuf1, MAX_PATH);
		GetModuleFileNameA(nullptr, szBuf1, MAX_PATH);
		PathRemoveFileSpecA(szBuf1);
		strcat(szBuf1, "\\sdkTool_Config.ini");
		m_ConfigPathA = szBuf1;

		std::error_code error;
		if (!std::filesystem::exists(m_ConfigPathA, error)) //already have a file with the same name?
		{
			bWriteIni = true;
		}
		loadCameras();
	}
	
	int getConfigInt(const char* szApp, const char* szKey)
	{
		if (bWriteIni)
			WritePrivateProfileStringA(szApp, szKey, "", m_ConfigPathA.data());
		return GetPrivateProfileIntA(szApp, szKey, 0, m_ConfigPathA.data());
	}
	int getConfigString(const char* szApp, const char* szKey, char* retVal)
	{
		if (bWriteIni)
			WritePrivateProfileStringA(szApp, szKey, "", m_ConfigPathA.data());
		return GetPrivateProfileStringA(szApp, szKey, "", retVal, MAX_PATH, m_ConfigPathA.data());
	}

	struct CameraOBJ
	{
		std::string ip;
		bool isIn=false;
		int id = 0;
		bool isMaster = false;
		bool isValid()
		{
			return ip.length()>6 && (ip!="0.0.0.0");
		}
		CCamera *camera;
		CameraOBJ()
		{
			camera = new CCamera();
			camera->_ADD_LOG = addLog;
		}
	};
	std::vector<CameraOBJ> m_Cameras;
	
	bool bWriteIni = false;

private:
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

		//In door
		for (int i = 0; i < 4; i++)
		{
			sprintf_s(szKey, "ip_%d", i );

			nret = getConfigString("in", szKey, szBuf1);
			if (nret < 7)//1.1.1.1
			{
				SPDLOG_ERROR("read ip_{} error, skip", i);
				if (!bWriteIni)continue;;
			}
			std::vector<std::string> ipArr = pLoadIP(szBuf1);
			if(ipArr.empty())continue;
			int j = 0;
			for (auto item : ipArr)
			{
				CameraOBJ obj;
				obj.isIn = true;
				obj.isMaster = j==0;
				
				obj.ip = item;
				obj.camera->m_curID = m_Cameras.size();
				strcpy(obj.camera->m_ipaddrstr, item.data());
				m_Cameras.emplace_back(obj);
				j++;
			}
			ipValid = true;
		}
		//out door
		for (int i = 0; i < 4; i++)
		{
			sprintf_s(szKey, "ip_%d", i);

			nret = getConfigString("out", szKey, szBuf1);
			if (nret < 7)//1.1.1.1
			{
				SPDLOG_ERROR("read ip_{} error, skip", i);
				if (!bWriteIni)continue;;
			}
			std::vector<std::string> ipArr = pLoadIP(szBuf1);
			if (ipArr.empty())continue;
			int j = 0;
			for (auto item : ipArr)
			{
				CameraOBJ obj;
				obj.isIn = false;
				obj.isMaster = j == 0;
				
				obj.ip = item;
				obj.camera->m_curID = m_Cameras.size();
				strcpy(obj.camera->m_ipaddrstr, item.data());
				m_Cameras.emplace_back(obj);
				j++;
			}
			ipValid = true;
		}


		if (!ipValid)
		{
			SPDLOG_ERROR("read [main][ip_X] error, no one was valid! exit");
			return false;
		}
		for (int i=0;i< m_Cameras.size();i++)
		{
			CameraOBJ item = m_Cameras[i];

			SPDLOG_ERROR("in[{}] IP: {}", item.isIn,item.ip);
		}
		return true;
	}
};