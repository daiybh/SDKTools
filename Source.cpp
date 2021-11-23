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
#include <filesystem>
#include "config.h"

#include "ConsoleUtil.h"
#include "dbghelper.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <ctime>
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

class Worker
{
public:
	void doRestartDev(struct tm& local) {
		if (!config.m_RestartParam.isNeedRun(local))return;
		SPDLOG_INFO("##############Restart cam   begin##############");
		for (auto ip : config.m_RestartParam.m_IPs)
		{
			CCamera  ca;
			strcpy(ca.m_ipaddrstr, ip.c_str());
			ca.connect();
			int ret = ca.set_EYEST_NET_RESTART();
			SPDLOG_INFO("Restart cam[{}]  result:{},{}", ip, (ret == 0) ? "Sccuess" : "faild", ret);
		}

		SPDLOG_INFO("##############Restart cam   END##############");
	}
	void doWork()
	{
		struct tm local;
		time_t t;
		t = time(NULL);
		localtime_s(&local, &t);

		doRestartDev(local);

		for (int j = 0; j < 4; j++)
		{
			if (!config.cameraOBJ[j].isValid())continue;
			for (int i = 0; i < 4; i++)
			{
				if (!config.cameraOBJ[j].paramOBJ[i].isValid)
					continue;
				//SPDLOG_INFO("time[{}]---> camera[{},{}]  xxxxxx", i, j, config.cameraOBJ[j].ip);

				if (config.cameraOBJ[j].paramOBJ[i].isNeedRun(local))
				{
					SPDLOG_INFO("time[{}]---> camera[{},{}]  to setParam>>>begin", i, j, config.cameraOBJ[j].ip);
					config.cameraOBJ[j].camera.connect();
					int ret = config.cameraOBJ[j].camera.set_3A_PARAM_V1(config.cameraOBJ[j].paramOBJ[i].cameraParam); /**/
					SPDLOG_INFO("time[{}]---> camera[{},{}]  to setcameraParam>>> {},{}", i, j, config.cameraOBJ[i].ip, (ret == 0) ? "Sccuess" : "faild", ret);

					ret = config.cameraOBJ[j].camera.set_EYEST_NET_SET_LIGHT_PARAM(config.cameraOBJ[j].paramOBJ[i].lightParam); /**/
					SPDLOG_INFO("time[{}]---> camera[{},{}]  to setLightParam>>> {},{}", i, j, config.cameraOBJ[i].ip, (ret == 0) ? "Sccuess" : "faild", ret);

					bool bret = config.cameraOBJ[j].camera.chageVOLUME(config.cameraOBJ[j].paramOBJ[i].voice_volume);
					SPDLOG_INFO("time[{}]---> camera[{},{}]  to chageVOLUME>>> {},{}", i, j, config.cameraOBJ[i].ip, (bret) ? "Sccuess" : "faild", ret);					
				}
			}
		}
	}
	Config config;
	ConsoleUtil m_ConsoleUtil;
	uint64_t m_loopCnt = 0;

	void start(int argc)
	{

		config.load();
		
		
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
												for (int j = 0; j < 4; j++)
												{
													m_ConsoleUtil.gotoXY(0, baseLine++);
													printf("ip:%s ", config.cameraOBJ[j].ip.data());
												
												baseLine++;
												for (int i = 0; i < 4; i++)
												{
													m_ConsoleUtil.gotoXY(0, baseLine++);
													/*	fmt::print("param aemaxTime:{} AVGlight:{} AGain:{}   runtime: {:02d}:{:02d}:{:02d}",
															config.cameraOBJ[j].paramOBJ[i].param.AEMaxTime,
															config.cameraOBJ[j].paramOBJ[i].param.AVGLight,
															config.cameraOBJ[j].paramOBJ[i].param.AGain,
															config.cameraOBJ[j].paramOBJ[i].workTime.tm_hour,
															config.cameraOBJ[j].paramOBJ[i].workTime.tm_min,
															config.cameraOBJ[j].paramOBJ[i].workTime.tm_sec);*/
												}
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

void SetQuickEditMode(bool _enable)
{
	HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
	DWORD  mode;
	GetConsoleMode(hStdin, &mode);
	if (_enable)
		mode |= ENABLE_QUICK_EDIT_MODE;
	else
		mode &= ~ENABLE_QUICK_EDIT_MODE;
	SetConsoleMode(hStdin, mode);
}
BOOL WINAPI CtrlHandler(DWORD fdwCtrlType)
{
	switch (fdwCtrlType)
	{
		// Handle the CTRL-C signal.
	case CTRL_C_EVENT:
		printf("Ctrl-C event\n\n");
		Beep(750, 300);
		return TRUE;

		// CTRL-CLOSE: confirm that the user wants to exit.
	case CTRL_CLOSE_EVENT:
		Beep(600, 200);
		printf("Ctrl-Close event\n\n");
		return TRUE;

		// Pass other signals to the next handler.
	case CTRL_BREAK_EVENT:
		Beep(900, 200);
		printf("Ctrl-Break event\n\n");
		return TRUE;

	case CTRL_LOGOFF_EVENT:
		Beep(1000, 200);
		printf("Ctrl-Logoff event\n\n");
		return TRUE;

	case CTRL_SHUTDOWN_EVENT:
		Beep(750, 500);
		printf("Ctrl-Shutdown event\n\n");
		return TRUE;

	default:
		return FALSE;
	}
}
int main(int argc, char *argv[])
{
	HANDLE mutex = OpenMutexA(MUTEX_ALL_ACCESS, FALSE, "store.hyman.sdk_TOOLs.Mutex");
	if (mutex == nullptr)
		mutex = CreateMutexA(nullptr, FALSE, "store.hyman.sdk_TOOLs.Mutex");
	else
	{
		CloseHandle(mutex);
		return EXIT_FAILURE;
	}
	SetQuickEditMode(false);
	EnableMenuItem(GetSystemMenu(GetConsoleWindow(), FALSE), SC_CLOSE, MF_GRAYED);
	ShowWindow(GetConsoleWindow(), SW_MINIMIZE);
	if (SetConsoleCtrlHandler(CtrlHandler, TRUE))
	{

	}
	SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)ApplicationCrashHandler);
	initLogger(nullptr);
	printf("\n----------SDKTOOls  ----\n");
	Worker worker;
	worker.start(argc);
	return 0;
}