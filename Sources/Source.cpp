#define _CRT_SECURE_NO_WARNINGS
#pragma comment(lib, "Shlwapi.lib")

#include <fmt/format.h>
#include <filesystem>
#include "config.h"

#include "ConsoleUtil.h"
#include "dbghelper.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <ctime>

class Worker
{
public:
	
	Config config;
	ConsoleUtil m_ConsoleUtil;
	uint64_t m_loopCnt = 0;

	void start(int argc)
	{	
		
		
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
#include "mainctrl.h"
#include "httpClient.h"
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
	printf("\n----------SDKTOOls  ----\n");
	MainCtrl ctrl;
	ctrl.init();
	return 0;
}