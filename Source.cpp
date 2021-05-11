#define _CRT_SECURE_NO_WARNINGS 
#include "..\inc\Camera.h"
#include <Shlwapi.h>
#pragma comment(lib,"Shlwapi.lib")

#define SPDLOG_ACTIVE_LEVEL 0
#include <spdlog/spdlog.h>
#include "spdlog/sinks/daily_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/rotating_file_sink.h"
inline void initLogger(const char *folderPath)
{
	if (folderPath == nullptr)
		folderPath = ".\\";
	auto pCreateLogger = [&](std::string loggerName,bool bDefault=false) {

		auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
		console_sink->set_level(spdlog::level::trace);

		std::string fileName = std::string(folderPath)+"/logs/" + loggerName + ".txt";
		auto file_sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(fileName, 0, 0,false,10);
		file_sink->set_level(spdlog::level::trace);

		std::vector<spdlog::sink_ptr> sinks;
		sinks.push_back(console_sink);
		sinks.push_back(file_sink);

		auto logger = std::make_shared<spdlog::logger>(loggerName, sinks.begin(), sinks.end());
		//{ console_sink, file_sink }
		//logger->flush_on(spdlog::level::info);
		logger->set_level(spdlog::level::debug);
		logger->set_pattern("[%x %X] %L %t [%s:%#-%!]>> %v");
		logger->info("########################STARt##############################");

		spdlog::register_logger(logger);
		if (bDefault)
			spdlog::set_default_logger(logger);
		//logger->flush();
	};
	/**/

	pCreateLogger("mainLog", true);
	spdlog::set_pattern("[%x %X] %L %t [%s:%#-%!]>> %v");
	spdlog::flush_every(std::chrono::seconds(5));
	spdlog::flush_on(spdlog::level::info);

}

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

		load();
	}
	
	void load() {
		char szBuf1[MAX_PATH];
		char szKey[MAX_PATH];
		int nret = 0;
		for (int i = 0; i < 4; i++)
		{
			sprintf_s(szKey, "time%d", i + 1);
			nret = getConfigString("main", szKey,szBuf1);
			if (nret > 0)
			{
				std::string s(szBuf1);
				auto pos1 = s.find(":");if(pos1 == std::string::npos)continue;
				auto pos2 = s.find(":",pos1+1); if (pos2 == std::string::npos)continue;				
				szBuf1[pos1] = '\0';
				szBuf1[pos1] = '\0';
				szBuf1[pos2] = '\0';

				cameraOBJ[i].workTime.tm_hour = atoi(szBuf1);
				cameraOBJ[i].workTime.tm_min = atoi(szBuf1 + pos1 + 1);
				cameraOBJ[i].workTime.tm_sec = atoi(szBuf1 + pos2 + 1);
				
			}
		}
		for (int i = 0; i < 4; i++)
		{
			sprintf_s(szKey, "ip_%d", i + 1);
			nret = getConfigString("main", szKey, szBuf1);
			if (nret > 0)
				cameraOBJ[i].ip = szBuf1;
		}

		for (int i = 0; i < 3; i++)
		{
			sprintf_s(szKey, "%d", i + 1);
			cameraOBJ[i].param.AEMaxTime= getConfigInt(szKey, "AEMaxTime");
			cameraOBJ[i].param.AVGLight = getConfigInt(szKey, "AVGLight" );
			cameraOBJ[i].param.AGain =    getConfigInt(szKey, "AGain"    );
		}
	}
	int getConfigInt(const char*szApp, const char*szKey)
	{
		if (bWriteIni)
			WritePrivateProfileStringA(szApp,szKey,"",m_ConfigPathA.data());
		return GetPrivateProfileIntA(szApp, szKey, 0, m_ConfigPathA.data());
	}
	int getConfigString(const char*szApp, const char*szKey,char*retVal)
	{
		if (bWriteIni)
			WritePrivateProfileStringA(szApp, szKey, "", m_ConfigPathA.data());
		return  GetPrivateProfileStringA(szApp, szKey, "", retVal, MAX_PATH, m_ConfigPathA.data());
	}
	struct CameraOBJ {
		std::string ip;
		bool isValid() {
			return !ip.empty() && workTime.isValid;
		}
		struct MyTM {
			bool isValid = false;
			int tm_sec;   // seconds after the minute - [0, 60] including leap second
			int tm_min;   // minutes after the hour - [0, 59]
			int tm_hour;  // hours since midnight - [0, 23]
		};
		struct MyTM workTime;		
		NET_DEV_CAMERAPARAM_V1 param;
		uint64_t lastRunTime;
	};
	CameraOBJ cameraOBJ[4];
	bool bWriteIni = false;
private:
	std::string m_ConfigPathA = ".\\sdkTool_Config.ini";
};

class Worker 
{
public:
	void doWork()
	{
		for (int i = 0; i < 4; i++)
		{
			if (!config.cameraOBJ[i].isValid())continue;
			
			
			struct tm local;
			time_t t;
			t = time(NULL);
			localtime_s(&local, &t);			
			bool bNeedRestart2 = (local.tm_hour == config.cameraOBJ[i].workTime.tm_hour 
				&& local.tm_min == config.cameraOBJ[i].workTime.tm_min);
			if (bNeedRestart2 && (GetTickCount() - config.cameraOBJ[i].lastRunTime > 1000 * 60 * 2))
			{
				SPDLOG_INFO("{} setParam",i);
				config.cameraOBJ[i].lastRunTime = GetTickCount();
				CCamera camera;
				strcpy(camera.m_ipaddrstr, config.cameraOBJ[i].ip.data());
				camera.connect();
				camera.set_3A_PARAM_V1(config.cameraOBJ[i].param);/**/
			}
		}
	}
	Config config;
	void start(int argc) {
		
		if (argc > 1)
			config.bWriteIni = true;

		config.load();
		auto a =std::thread([&]() {

			while (1)
			{
				Sleep(1000);
				doWork();
			}
			});

		a.join();
	}
};
int main(int argc,char*argv[])
{
	initLogger(nullptr);
	printf("\n----------SDKTOOls  ----\n");
	Worker worker;
	worker.start(argc);
	return 0;
}