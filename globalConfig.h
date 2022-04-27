#pragma once
#include "logLib.h"
#include "rapidjson/rapidjson.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/document.h"
#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#endif

class GlobalConfig
{
public:
	SimplyLive::Logger::LogConfig globalConfig;
	std::wstring processName;
	std::wstring relativePath;

	GlobalConfig()
	{
#ifndef _WIN32
		const char* homedir = getenv("HOME");
		if (homedir == nullptr)
			homedir = getpwuid(getuid())->pw_dir;
		m_configPath = fmt::format(R"({}/Config/LogConfig.json)", homedir);
		relativePath = fmt::format(R"({}/Logs/)", homedir);
#else
		wchar_t strModuleFileName[260];
		wchar_t strPath[260];
		GetModuleFileNameW(nullptr, strModuleFileName, 260);
		_wsplitpath_s(strModuleFileName, nullptr, 0, nullptr, 0, strPath + 1, std::size(strPath) - 1, nullptr, 0);

		wchar_t *p =wcsrchr(strModuleFileName, '\\');
		*(p + 1) = '\0';
		strPath[0] = '_';
		processName = strPath;
		relativePath = strModuleFileName;
#endif
	}

	void loadGlobalConfig()
	{
		FILE* myFile = fopen(m_configPath.data(), "r");
		if (myFile == nullptr)
			return;
		fseek(myFile, 0, SEEK_END);
		const uint32_t size = ftell(myFile);
		rewind(myFile);
		const auto jsonBuffer = std::make_unique<char[]>(size);
		rapidjson::FileReadStream inputStream(myFile, jsonBuffer.get(), size);
		m_json.ParseStream<0>(inputStream);
		fclose(myFile);

		if (m_json.HasParseError())
			return;

		globalConfig.asyncLog = getInt("Async", 1);
		globalConfig.logLevelThreshold = getInt("DebugLog", 0) == 0 ? SimplyLive::Logger::Level::info : SimplyLive::Logger::Level::debug;
		globalConfig.maxLogFileSizeMB = getInt("MaxLogFileSizeMB", 20);
		globalConfig.maxNumberOfRollingFile = getInt("MaxNumberOfRollingFile", 5);
		globalConfig.relativePath = getInt("RelativePath", 1);

#ifdef _WIN32
		globalConfig.addProcessName = getInt("AddProcessName", 0);
#else
		globalConfig.addProcessName = false;
#endif
	}

private:
	std::string         m_configPath = R"(C:\ProgramData\SimplyLive.TV\Vibox\Backend\LogConfig.json)";
	rapidjson::Document m_json;

	std::string getString(std::string_view pKey, const std::string& defaultValue)
	{
		if (!m_json.HasMember(pKey.data()))
			return defaultValue;
		if (m_json[pKey.data()].IsNull())
			return defaultValue;
		return m_json[pKey.data()].GetString();
	}

	int32_t getInt(std::string_view pKey, int32_t defaultValue)
	{
		if (!m_json.HasMember(pKey.data()))
			return defaultValue;
		if (m_json[pKey.data()].IsNull())
			return defaultValue;
		return m_json[pKey.data()].GetInt();
	}
};

inline std::wstring getWString(std::string_view fmt)
{
	wchar_t wBuffer[5 * 1024] = { 0 };
#ifdef  _MSC_VER
	size_t size = 0;
	mbstowcs_s(&size, wBuffer, std::size(wBuffer), fmt.data(), fmt.size());
	return wBuffer;
#else
	mbstowcs(wBuffer, fmt.data(), fmt.size());
	return wBuffer;
#endif
}

inline std::string getAString(std::wstring_view wstr)
{
	size_t bytesConverted;
	char buffer[5 * 1024] = { 0 };
#ifdef WINDOWS_SECURE_FUNCTIONS_AVAILABLE
	wcstombs_s(&bytesConverted, buffer, wstr.data(), wstr.size());
#else
	bytesConverted = wcstombs(buffer, wstr.data(), wstr.size());
#endif
	if (bytesConverted == size_t(-1) || bytesConverted == 0)
		return {};
	return std::string(buffer, bytesConverted);
}