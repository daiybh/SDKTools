#include "logLib.h"
#include "globalConfig.h"
#ifdef _WIN32
#define SPDLOG_WCHAR_FILENAMES
#define SPDLOG_WCHAR_TO_UTF8_SUPPORT
#else
extern char* __progname;
#endif
#include <filesystem>
#include "spdlog/async.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/rotating_file_sink.h"

namespace SimplyLive
{
	using spdLogger = std::shared_ptr<spdlog::logger>;
	static std::atomic_uint32_t instanceCount = 0;
	static std::mutex           locker;
	static std::once_flag       flag;
	static GlobalConfig         globalConfig;

	spdLogger getLogger(std::wstring_view logPath, Logger::LogConfig* logConfig)
	{
		std::scoped_lock lock(locker);
		auto rotating_logger = spdlog::get(getAString(logPath));
		if (rotating_logger != nullptr)
			return rotating_logger;

		std::call_once(flag, [&] { globalConfig.loadGlobalConfig(); });

		Logger::LogConfig* logConfigToUse = &globalConfig.globalConfig;
		if (logConfig != nullptr)
			logConfigToUse = logConfig;

		std::wstring newPath;
		if (logConfigToUse->relativePath)
		{
			if (std::filesystem::path path(logPath); path.has_root_path())
			{
				path = path.relative_path();
				newPath = fmt::format(LR"({}{})", (globalConfig.relativePath), path.wstring());
			}
			else
				newPath = fmt::format(LR"({}{})", (globalConfig.relativePath), logPath);
		}
		else
			newPath = logPath;

		if (logConfigToUse->addProcessName && !globalConfig.processName.empty())
			newPath.insert(newPath.length() - 4, globalConfig.processName);

		if (logConfigToUse->asyncLog)
		{
			static auto tp = std::make_shared<spdlog::details::thread_pool>(8192, 1);
#ifdef _WIN32
			auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(newPath, logConfigToUse->maxLogFileSizeMB * 1024 * 1024, logConfigToUse->maxNumberOfRollingFile);
#else
			auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(getAString(newPath), logConfigToUse->maxLogFileSizeMB * 1024 * 1024, logConfigToUse->maxNumberOfRollingFile);
#endif
			rotating_logger = std::make_shared<spdlog::async_logger>(getAString(logPath), std::move(file_sink), std::move(tp), spdlog::async_overflow_policy::overrun_oldest);
			register_logger(rotating_logger);
		}
		else
		{
#ifdef _WIN32
			rotating_logger = spdlog::rotating_logger_mt(getAString(logPath), newPath, logConfigToUse->maxLogFileSizeMB * 1024 * 1024, logConfigToUse->maxNumberOfRollingFile);
#else
			rotating_logger = spdlog::rotating_logger_mt(getAString(logPath), getAString(newPath), logConfigToUse->maxLogFileSizeMB * 1024 * 1024, logConfigToUse->maxNumberOfRollingFile);
#endif
		}

		//https://github.com/gabime/spdlog/wiki/3.-Custom-formatting
		rotating_logger->set_pattern("%C/%m/%d %T.%e [%-5t][%-5!l]- %v");
		switch (logConfigToUse->logLevelThreshold)
		{
		case Logger::Level::debug: rotating_logger->set_level(spdlog::level::debug); break;
		case Logger::Level::warn: rotating_logger->set_level(spdlog::level::warn); break;
		case Logger::Level::error:rotating_logger->set_level(spdlog::level::err); break;
		case Logger::Level::fatal:rotating_logger->set_level(spdlog::level::critical); break;
		case Logger::Level::info:
		default:	rotating_logger->set_level(spdlog::level::info); break;
		}
		return rotating_logger;
	}

	template<typename T>
	void internalWriter(const std::any& logger, Logger::Level level, T log)
	{
		if (!logger.has_value() || log.empty())
			return;

		const spdLogger writer = std::any_cast<spdLogger>(logger);
		switch (level)
		{
		case Logger::Level::debug: writer->debug(log);     break;
		case Logger::Level::warn:  writer->warn(log);      break;
		case Logger::Level::error: writer->error(log);     break;
		case Logger::Level::fatal: writer->critical(log);  break;
		case Logger::Level::info:
		default:                   writer->info(log);      break;
		}
		writer->flush();
	}

	Logger::Logger(std::wstring_view path, LogConfig* logConfig)
	{
		++instanceCount;
		setPath(path, logConfig);
	}

	Logger::Logger()
	{
		++instanceCount;
	}

	Logger::~Logger()
	{
		if (--instanceCount == 0)
			spdlog::shutdown();
	}

	void Logger::log(Level level, std::string_view log) const
	{
		internalWriter(m_logger, level, log);
	}

	void Logger::log(Level level, std::wstring_view log) const
	{
#ifdef _WIN32
		internalWriter(m_logger, level, log);
#else
		internalWriter(m_logger, level, getAString(log));
#endif
	}

	void Logger::setPath(std::wstring_view path, LogConfig* logConfig)
	{
#ifdef _WIN32
		m_logger = getLogger(path, logConfig);
#else
		std::wstring newPath(path);
		std::replace(newPath.begin(), newPath.end(), '\\', '/');//covert to posix path format
		m_logger = getLogger(newPath, logConfig);
#endif
	}
}