
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
	void loadResartParam() {
		
			char szBuf1[MAX_PATH];
		char szKey[MAX_PATH];
		char szApp[MAX_PATH];
		int nret = 0;

		bool ipValid = false;


		nret = getConfigString("restart", "time", szBuf1);
		m_RestartParam.stime.getTimeFunc(szBuf1);
		for (int i = 0; i < 10; i++)
		{
			sprintf_s(szKey, "IP%d", i + 1);
			nret = getConfigString("restart", szKey, szBuf1);
			if (nret < 7) {
				SPDLOG_ERROR("restart ->read ip_{} error, skip", i);
				continue;
			}
			m_RestartParam.m_IPs.emplace_back(szBuf1);
		}

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
	}struct MyTM
	{
		bool isValid = false;
		int tm_sec = 0;	 // seconds after the minute - [0, 60] including leap second
		int tm_min = 0;	 // minutes after the hour - [0, 59]
		int tm_hour = 0; // hours since midnight - [0, 23]
		
		bool getTimeFunc (char* szBuf1) {
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

			this->tm_hour = atoi(szBuf1);
			this->tm_min = atoi(szBuf1 + pos1 + 1);
			this->tm_sec = atoi(szBuf1 + pos2 + 1);
			this->isValid = true;


			return true;
		};
	};
	struct RestartParam {
		struct MyTM stime;
		struct tm lastRunTime;
		std::vector<std::string> m_IPs;
		bool isNeedRun(struct tm& local) {
			//       精确到分钟
			//判断 stime<当前时间
			//      lastRunTime
			auto isEqual = [&](MyTM& a) {
				if (local.tm_hour != a.tm_hour)return false;
				if (local.tm_min != a.tm_min)return false;
				if (local.tm_sec != a.tm_sec)return false;
				return true;
			};
			if (lastRunTime.tm_year == local.tm_year
				&& lastRunTime.tm_mon == local.tm_mon
				&& lastRunTime.tm_mday == local.tm_mday)
				return false;
			if (!isEqual(stime))return false;

			lastRunTime = local;
			//isFirstRun = false;
			return true;
		}

	};
	struct ParamObj
	{
		
		struct MyTM stime;
		struct MyTM etime;
		struct tm lastRunTime;
		bool isFirstRun = true;
		bool isValid = false;
		NET_DEV_CAMERAPARAM_V1 cameraParam;
		NET_DEV_LIGHT_CFG  lightParam;
		int voice_volume = 0;
		ParamObj() {
			memset(&lastRunTime, 0, sizeof(tm));
		}
		bool isNeedRun(struct tm& local) {

			auto isInside = [&](struct tm&local) {
				auto pbigger = [&](MyTM& a) {
					if (local.tm_hour < a.tm_hour)return false;
					if (local.tm_hour > a.tm_hour)return true;
					if (local.tm_min < a.tm_min)return false;
					if (local.tm_min > a.tm_min)return true;
					if (local.tm_sec < a.tm_sec)return false;
					return true;
				};
				auto pletter = [&](MyTM& a) {
					if (local.tm_hour > a.tm_hour)return false;
					if (local.tm_hour < a.tm_hour)return true;
					if (local.tm_min > a.tm_min)return false;
					if (local.tm_min < a.tm_min)return true;
					if (local.tm_sec > a.tm_sec)return false;
					return true;
				};
				bool bNeedRestart1 = pbigger(stime);
				bool bNeedRestart2 = pletter(etime);
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
	RestartParam m_RestartParam;

private:
	std::string m_ConfigPathA = ".\\sdkTool_Config.ini";

	bool load()
	{
		char szBuf1[MAX_PATH];
		char szKey[MAX_PATH];
		char szApp[MAX_PATH];
		int nret = 0;

		bool ipValid = false;


		loadResartParam();

		for (int i = 0; i < 4; i++)
		{
			sprintf_s(szKey, "%d", i + 1);

			nret = getConfigString(szKey, "ip", szBuf1);
			if (nret < 7)//1.1.1.1
			{
				SPDLOG_ERROR("read ip_{} error, skip", i);
				if (!bWriteIni)continue;;
			}
			cameraOBJ[i].ip = szBuf1;
			strcpy(cameraOBJ[i].camera.m_ipaddrstr, szBuf1);
			ipValid = true;

			for (int j = 0; j < 4; j++)
			{
				nret = getConfigString(szKey, fmt::format("stime{}", j).data(), szBuf1);


				if (nret < 1 || !cameraOBJ[i].paramOBJ[j].stime.getTimeFunc(szBuf1))
				{
					SPDLOG_ERROR("read cam[{}]  sstime [{}] error, skip", i, j);
					if (!bWriteIni)continue;;
				}
				nret = getConfigString(szKey, fmt::format("etime{}", j).data(), szBuf1);
				if (nret < 0 || !cameraOBJ[i].paramOBJ[j].etime.getTimeFunc(szBuf1))
				{
					SPDLOG_ERROR("read cam[{}] eetime [{}] error, skip", i, j);
					if (!bWriteIni)continue;;
				}

				cameraOBJ[i].paramOBJ[j].isValid = true;

				/*
				stime2 =
				etime2 =
				AEMaxTime2 =
				AVGLight2 =
				AGain2 =
				Dlight3 =*/
				cameraOBJ[i].paramOBJ[j].cameraParam.AEMaxTime = getConfigInt(szKey, fmt::format("AEMaxTime{}", j).data());
				cameraOBJ[i].paramOBJ[j].cameraParam.AVGLight = getConfigInt(szKey, fmt::format("AVGLight{}", j).data());
				cameraOBJ[i].paramOBJ[j].cameraParam.AGain = getConfigInt(szKey, fmt::format("AGain{}", j).data());
				cameraOBJ[i].paramOBJ[j].lightParam.light = getConfigInt(szKey, fmt::format("Dlight{}", j).data());
				cameraOBJ[i].paramOBJ[j].voice_volume = getConfigInt(szKey, fmt::format("voice{}", j).data());

			}
		}

		if (!ipValid)
		{
			SPDLOG_ERROR("read [main][ip_X] error, no one was valid! exit");
			return false;
		}
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
};