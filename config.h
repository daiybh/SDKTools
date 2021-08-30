
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
		load();
	}
	bool isvalid = false;
	bool load()
	{
		char szBuf1[MAX_PATH];
		char szKey[MAX_PATH];
		char szApp[MAX_PATH];
		int nret = 0;

		bool ipValid = false;
		
		bool isvalidTime = false;
		for (int i = 0; i < 4; i++)
		{
			sprintf_s(szKey, "%d", i + 1);
			
			nret = getConfigString(szKey, fmt::format("ip_{}", i).data(), szBuf1);
			if (nret < 7)//1.1.1.1
			{
				SPDLOG_ERROR("read ip_{} error, skip",i);
				continue;;
			}
			cameraOBJ[i].ip = szBuf1;
			strcpy(cameraOBJ[i].camera.m_ipaddrstr, szBuf1);
			ipValid = true;

			for (int j = 0; j < 4; j++)
			{
				nret = getConfigString(szKey, fmt::format("stime{}", j).data(), szBuf1);
				auto getTimeFunc = [&](char* szBuf1, ParamObj::MyTM& workTime) {
					std::string s(szBuf1);
					auto pos1 = s.find(":");
					if (pos1 == std::string::npos)
						return false;
					auto pos2 = s.find(":", pos1 + 1);
					if (pos2 == std::string::npos)
						return false;
					szBuf1[pos1] = '\0';
					szBuf1[pos1] = '\0';
					szBuf1[pos2] = '\0';

					workTime.tm_hour = atoi(szBuf1);
					workTime.tm_min = atoi(szBuf1 + pos1 + 1);
					workTime.tm_sec = atoi(szBuf1 + pos2 + 1);
					return true;
				};

				if (nret <1 && !getTimeFunc(szBuf1,cameraOBJ[i].paramOBJ[j].stime))
				{					
					isvalidTime = false;
				}
				nret = getConfigString(szKey, fmt::format("etime{}", j).data(), szBuf1);
				if (nret < 0 && !getTimeFunc(szBuf1, cameraOBJ[i].paramOBJ[j].etime))
				{
					isvalidTime = false;
				}
				if (!isvalidTime)
				{
					SPDLOG_ERROR("read cam[{}]  time [{}] error, skip", i, j);
					continue;;
				}	
				cameraOBJ[i].paramOBJ[j].isValid = true;

					/*
					stime2 =
					etime2 =
					AEMaxTime2 =
					AVGLight2 =
					AGain2 =
					Dlight3 =*/
				cameraOBJ[i].paramOBJ[j].param.AEMaxTime = getConfigInt(szKey, "AEMaxTime");
				cameraOBJ[i].paramOBJ[j].param.AVGLight = getConfigInt(szKey, "AVGLight");
				cameraOBJ[i].paramOBJ[j].param.AGain = getConfigInt(szKey, "AGain");
			}
		}

		if (!ipValid)
		{
			SPDLOG_ERROR("read [main][ip_X] error, no one was valid! exit");
			return false;
		}
		isvalid = ipValid && isvalidTime;
		for (int i = 0; i < 4; i++)
		{
			SPDLOG_ERROR("IP: {}", cameraOBJ[i].ip);
		}
		/*for (int i = 0; i < 4; i++)
		{
			SPDLOG_ERROR("param aemaxTime:{} AVGlight:{} AGain:{}", paramOBJ[i].param.AEMaxTime, paramOBJ[i].param.AVGLight, paramOBJ[i].param.AGain);
		}*/
		return true;
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

	struct ParamObj
	{
		struct MyTM
		{
			bool isValid = false;
			int tm_sec = 0;	 // seconds after the minute - [0, 60] including leap second
			int tm_min = 0;	 // minutes after the hour - [0, 59]
			int tm_hour = 0; // hours since midnight - [0, 23]
		};
		struct MyTM stime;
		struct MyTM etime;
		struct tm lastRunTime;
		bool isFirstRun = true;
		bool isValid = false;
		NET_DEV_CAMERAPARAM_V1 param;
		ParamObj() {
			memset(&lastRunTime, 0, sizeof(tm));
		}
		bool isNeedRun(struct tm& local) {

			auto isInside = [&](struct tm&local) {
				bool bNeedRestart1 = (local.tm_hour >= stime.tm_hour && local.tm_min >= stime.tm_min && local.tm_sec >= stime.tm_sec);
				bool bNeedRestart2 = (local.tm_hour <= etime.tm_hour && local.tm_min <= etime.tm_min && local.tm_sec <= etime.tm_sec);
				return bNeedRestart1 && bNeedRestart2;
			};
			if (!isInside(local))return false;

			if (!isFirstRun) {
				if (isInside(lastRunTime))
				{
					if(lastRunTime.tm_year== local.tm_year && lastRunTime.tm_mon==local.tm_mon && lastRunTime.tm_mday == local.tm_mday)
						return false;
				}
			}

			lastRunTime = local;
			isFirstRun = false;
			return true;
		}
	};
	struct CameraOBJ
	{
		std::string ip;
		bool isValid()
		{
			return !ip.empty();
		}
		CCamera camera;
		ParamObj paramOBJ[4];
		CameraOBJ()
		{
			camera._ADD_LOG = addLog;
		}
	};
	CameraOBJ cameraOBJ[4];
	bool bWriteIni = false;

private:
	std::string m_ConfigPathA = ".\\sdkTool_Config.ini";
};