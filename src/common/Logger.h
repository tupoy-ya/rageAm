#pragma once
#include <string>
#include <fstream>
#include <shared_mutex>
#include <vector>
#include <format>

#include "FileHelper.h"

#define AM_TRACE(msg) g_Log.LogT(msg)
#define AM_TRACEF(fmt, ...) g_Log.LogT(fmt, __VA_ARGS__)
#define AM_ERR(msg) g_Log.LogE(msg)
#define AM_ERRF(fmt, ...) g_Log.LogE(fmt, __VA_ARGS__)

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

struct LogEntry
{
	eLoggerLevel Level;

	LogEntry(eLoggerLevel level)
	{
		Level = level;
	}
};

class Logger
{
	// For unknown reason if std::string is inside structure it causes heap exceptions
	// This doesn't happen in release target though. As solution use 2 separate lists
	// for log metadata and message.
	// TODO: Try struct again, it might have been related to thread safety

	std::vector<LogEntry> m_Entries;
	std::vector<std::string> m_Messages;

	std::ofstream m_fs;
	std::string m_logFile = "rageAm/rageAm.txt";
	std::string m_logFileBack = "rageAm/rageAm_backup.txt";

	std::shared_mutex m_fileMutex;

	eLoggerLevel m_logLevel = LOG_DEFAULT;

	bool m_HasError = false;

	void Log(const std::string& msg)
	{
#ifdef RAGE_STANDALONE
		printf("%s\n", msg.c_str());
#else
		m_fileMutex.lock();
		m_fs << msg << std::endl;
		m_fileMutex.unlock();
#endif
	}
public:
	Logger()
	{
		LogT("Logger()");

		CreateDefaultFolders();

		// Rename existing log
		MoveFileExA(m_logFile.c_str(), m_logFileBack.c_str(), MOVEFILE_REPLACE_EXISTING);

		m_fs.open(m_logFile, std::ios::trunc);
	}

	~Logger()
	{
		LogT("~Logger()");
		m_fs.close();
	}

	void LogLevel(eLoggerLevel level, const std::string msg)
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

		m_fileMutex.lock();

		if (level == LOG_ERROR)
			m_HasError = true;

		m_Entries.push_back(level);
		m_Messages.push_back(msg);

		m_fileMutex.unlock();
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

	int GetLogCount() const
	{
		return (int)m_Entries.size();
	}

	LogEntry* GetLogEntryAt(int index)
	{
		return &m_Entries[index];
	}

	std::string GetLogMessageAt(int index)
	{
		return m_Messages[index];
	}

	std::shared_mutex* GetMutex()
	{
		return &m_fileMutex;
	}

	bool GetHasError() const
	{
		return m_HasError;
	}

	void ResetError()
	{
		m_HasError = false;
	}
};

inline Logger g_Log;
