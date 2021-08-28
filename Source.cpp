#define _CRT_SECURE_NO_WARNINGS
#include "..\inc\Camera.h"
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

#include <fmt/format.h>
#define SPDLOG_ACTIVE_LEVEL 0
#include <spdlog/spdlog.h>
#include "spdlog/sinks/daily_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/rotating_file_sink.h"
inline void initLogger(const char *folderPath)
{
	if (folderPath == nullptr)
		folderPath = ".\\";
	auto pCreateLogger = [&](std::string loggerName, bool bDefault = false)
	{
		auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
		console_sink->set_level(spdlog::level::trace);

		std::string fileName = std::string(folderPath) + "/logs/" + loggerName + ".txt";
		auto file_sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(fileName, 0, 0, false, 10);
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
#include <filesystem>

static void CALLBACK addLog(const char *log, void *UserParam)
{
	SPDLOG_ERROR("addLog....{}", log);
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
		for (int i = 0; i < 4; i++)
		{
			sprintf_s(szKey, "ip_%d", i + 1);
			nret = getConfigString("main", szKey, szBuf1);
			if (nret > 0)
			{
				cameraOBJ[i].ip = szBuf1;
				strcpy(cameraOBJ[i].camera.m_ipaddrstr, szBuf1);
				ipValid = true;
			}
		}

		bool isvalidTime = false;
		for (int i = 0; i < 4; i++)
		{
			sprintf_s(szKey, "%d", i + 1);
			nret = getConfigString(szKey, "runtime", szBuf1);
			if (nret > 0)
			{
				std::string s(szBuf1);
				auto pos1 = s.find(":");
				if (pos1 == std::string::npos)
					continue;
				auto pos2 = s.find(":", pos1 + 1);
				if (pos2 == std::string::npos)
					continue;
				szBuf1[pos1] = '\0';
				szBuf1[pos1] = '\0';
				szBuf1[pos2] = '\0';

				paramOBJ[i].workTime.tm_hour = atoi(szBuf1);
				paramOBJ[i].workTime.tm_min = atoi(szBuf1 + pos1 + 1);
				paramOBJ[i].workTime.tm_sec = atoi(szBuf1 + pos2 + 1);
				isvalidTime = true;
				paramOBJ[i].isValid = true;
			}

			paramOBJ[i].param.AEMaxTime = getConfigInt(szKey, "AEMaxTime");
			paramOBJ[i].param.AVGLight = getConfigInt(szKey, "AVGLight");
			paramOBJ[i].param.AGain = getConfigInt(szKey, "AGain");
		}

		if (!isvalidTime)
		{
			SPDLOG_ERROR("read [main][timeX] error, no one was valid! exit");
			return false;
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
		for (int i = 0; i < 4; i++)
		{
			SPDLOG_ERROR("param aemaxTime:{} AVGlight:{} AGain:{}", paramOBJ[i].param.AEMaxTime, paramOBJ[i].param.AVGLight, paramOBJ[i].param.AGain);
		}
		return true;
	}
	int getConfigInt(const char *szApp, const char *szKey)
	{
		if (bWriteIni)
			WritePrivateProfileStringA(szApp, szKey, "", m_ConfigPathA.data());
		return GetPrivateProfileIntA(szApp, szKey, 0, m_ConfigPathA.data());
	}
	int getConfigString(const char *szApp, const char *szKey, char *retVal)
	{
		if (bWriteIni)
			WritePrivateProfileStringA(szApp, szKey, "", m_ConfigPathA.data());
		return GetPrivateProfileStringA(szApp, szKey, "", retVal, MAX_PATH, m_ConfigPathA.data());
	}
	struct CameraOBJ
	{
		std::string ip;
		bool isValid()
		{
			return !ip.empty();
		}
		CCamera camera;
		uint64_t lastRunTime = 0;
		CameraOBJ()
		{
			camera._ADD_LOG = addLog;
		}
	};

	struct ParamObj
	{
		struct MyTM
		{
			bool isValid = false;
			int tm_sec = 0;	 // seconds after the minute - [0, 60] including leap second
			int tm_min = 0;	 // minutes after the hour - [0, 59]
			int tm_hour = 0; // hours since midnight - [0, 23]
		};
		struct MyTM workTime;
		bool isValid = false;
		NET_DEV_CAMERAPARAM_V1 param;
	};
	ParamObj paramOBJ[4];
	CameraOBJ cameraOBJ[4];
	bool bWriteIni = false;

private:
	std::string m_ConfigPathA = ".\\sdkTool_Config.ini";
};
#include "ConsoleUtil.h"
class Worker
{
public:
	void doWork()
	{
		for (int i = 0; i < 4; i++)
		{
			if (!config.paramOBJ[i].isValid)
				continue;
			struct tm local;
			time_t t;
			t = time(NULL);
			localtime_s(&local, &t);
			bool bNeedRestart2 = (local.tm_hour == config.paramOBJ[i].workTime.tm_hour && local.tm_min == config.paramOBJ[i].workTime.tm_min);
			if (bNeedRestart2)
			{
				for (int j = 0; j < 4; j++)
				{
					if (!config.cameraOBJ[j].isValid())
						continue;
					if ((GetTickCount() - config.cameraOBJ[j].lastRunTime > 1000 * 60 * 2))
					{
						SPDLOG_INFO("time[{}]---> camera[{},{}]  to setParam>>>begin", i, j, config.cameraOBJ[j].ip);
						config.cameraOBJ[j].lastRunTime = GetTickCount();

						config.cameraOBJ[j].camera.connect();
						int ret = config.cameraOBJ[j].camera.set_3A_PARAM_V1(config.paramOBJ[i].param); /**/
						SPDLOG_INFO("time[{}]---> camera[{},{}]  to setParam>>> {},{}", i, j, config.cameraOBJ[i].ip, (ret == 0) ? "Sccuess" : "faild", ret);
					}
				}
			}
		}
	}
	Config config;
	ConsoleUtil m_ConsoleUtil;
	uint64_t m_loopCnt = 0;

	void start(int argc)
	{
		if (!config.isvalid)
			return;

		config.load();

		if (argc > 1)
		{
			for (int j = 0;; j++)
			{
				struct tm local;
				time_t t;
				t = time(NULL);
				localtime_s(&local, &t);
				for (int i = 0; i < 4; i++)
				{
					bool isValid = false;
					int tm_sec;	 // seconds after the minute - [0, 60] including leap second
					int tm_min;	 // minutes after the hour - [0, 59]
					int tm_hour; // hours since midnight - [0, 23]
					config.paramOBJ[i].workTime = {true, local.tm_sec, local.tm_min, local.tm_hour};
					;
					config.cameraOBJ[i].lastRunTime = 0;
				}
				doWork();
				printf("\n press any key to continue");
				getchar();
			}
		}
		auto updateScreen = std::thread([&]()
										{
											while (1)
											{
												int baseLine = 5;
												struct tm local;
												time_t t;
												t = time(NULL);
												localtime_s(&local, &t);

												m_ConsoleUtil.gotoXY(0, baseLine++);
												printf("loop:%I64d,curTick:%d cur:%02d:%02d:%02d", m_loopCnt, GetTickCount(), local.tm_hour, local.tm_min, local.tm_sec);
												for (int i = 0; i < 4; i++)
												{
													m_ConsoleUtil.gotoXY(0, baseLine++);
													printf("ip:%s lastDo:%I64d", config.cameraOBJ[i].ip.data(), config.cameraOBJ[i].lastRunTime);
												}
												baseLine++;
												for (int i = 0; i < 4; i++)
												{
													m_ConsoleUtil.gotoXY(0, baseLine++);
													fmt::print("param aemaxTime:{} AVGlight:{} AGain:{}   runtime: {:02d}:{:02d}:{:02d}",
															   config.paramOBJ[i].param.AEMaxTime,
															   config.paramOBJ[i].param.AVGLight,
															   config.paramOBJ[i].param.AGain,
															   config.paramOBJ[i].workTime.tm_hour,
															   config.paramOBJ[i].workTime.tm_min,
															   config.paramOBJ[i].workTime.tm_sec);
												}

												Sleep(1000);
												if (m_loopCnt % 1000 == 0)
													m_ConsoleUtil.clearscreen();
												doWork();
												m_loopCnt++;
											}
										});

		updateScreen.join();
	}
};
int main(int argc, char *argv[])
{
	initLogger(nullptr);
	printf("\n----------SDKTOOls  ----\n");
	Worker worker;
	worker.start(argc);
	return 0;
}