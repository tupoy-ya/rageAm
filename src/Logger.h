#pragma once
#include <string>
#include <fstream>
#include <vector>

#include "format"

#include "FileHelper.h"

enum eLoggerLevel
{
	LOG_TRACE = 0x1,
	LOG_DEBUG = 0x2,
	LOG_ERROR = 0x4,

#ifdef _DEBUG
	LOG_DEFAULT = LOG_TRACE | LOG_DEBUG | LOG_ERROR
#else
	LOG_DEFAULT = LOG_TRACE | LOG_ERROR
#endif
};

class Logger
{
	std::vector<std::string> m_entries;

	std::ofstream m_fs;
	std::string m_logFile = "rageAm/rageAm.txt";
	std::string m_logFileBack = "rageAm/rageAm_backup.txt";

	std::mutex m_fileMutex;

	eLoggerLevel m_logLevel = LOG_DEFAULT;
public:
	Logger()
	{
		LogT("Logger()");

		CreateDataFolderIfNotExists();

		// Rename existing log
		MoveFileExA(m_logFile.c_str(), m_logFileBack.c_str(), MOVEFILE_REPLACE_EXISTING);

		m_fs.open(m_logFile, std::ios::trunc);
	}

	~Logger()
	{
		LogT("~Logger()");
		m_fs.close();
	}

	void Log(const std::string& msg)
	{
		m_fileMutex.lock();
		m_fs << msg << std::endl;
		m_entries.push_back(msg);
		m_fileMutex.unlock();
	}

	void LogLevel(eLoggerLevel level, const std::string& msg)
	{
		const char* prefix = nullptr;

		if (level & LOG_TRACE && m_logLevel & LOG_TRACE)
		{
			prefix = "[TRACE]";
		}

		if (level & LOG_DEBUG && m_logLevel & LOG_DEBUG)
		{
			prefix = "[DEBUG]";
		}

		if (level & LOG_ERROR && m_logLevel & LOG_ERROR)
		{
			prefix = "[ERROR]";
		}

		// Log level is not enabled
		if (prefix == nullptr)
			return;

		Log("{}: {}", prefix, msg);
	}

	template<typename... Args>
	void Log(std::_Fmt_string<Args...> fmt, Args&&... args)
	{
		// https://stackoverflow.com/questions/72795189/how-can-i-wrap-stdformat-with-my-own-template-function
		Log(std::format(fmt, std::forward<Args>(args)...));
	}

	template<typename... Args>
	void LogLevel(eLoggerLevel level, std::_Fmt_string<Args...> fmt, Args&&... args)
	{
		LogLevel(level, std::format(fmt, std::forward<Args>(args)...));
	}

	template<typename... Args>
	void LogT(std::_Fmt_string<Args...> fmt, Args&&... args)
	{
		LogLevel(LOG_TRACE, std::format(fmt, std::forward<Args>(args)...));
	}

	template<typename... Args>
	void LogD(std::_Fmt_string<Args...> fmt, Args&&... args)
	{
		LogLevel(LOG_DEBUG, std::format(fmt, std::forward<Args>(args)...));
	}

	template<typename... Args>
	void LogE(std::_Fmt_string<Args...> fmt, Args&&... args)
	{
		LogLevel(LOG_ERROR, std::format(fmt, std::forward<Args>(args)...));
	}

	std::vector<std::string> GetEntries()
	{
		// TODO: Doesn't this returns a copy?
		return m_entries;
	}
};

inline Logger g_Log;
