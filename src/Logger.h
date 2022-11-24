#pragma once
#include <cstdarg>
#include <string>
#include <fstream>
#include <vector>
#include "format"

#include "Template/atSingleton.h"

class Logger : public atSingleton<Logger>
{
	std::string logFile = "rageAm.txt";
	std::vector<std::string> entries;
public:
	void Init()
	{
		Open(true).close();
	}

	void Log(const std::string& msg)
	{
		auto fs = Open(false);
		fs << msg << std::endl;
		entries.push_back(msg);
		fs.close();
	}

	template<typename... Args>
	void Log(std::_Fmt_string<Args...> fmt, Args&&... args)
	{
		// https://stackoverflow.com/questions/72795189/how-can-i-wrap-stdformat-with-my-own-template-function
		Log(std::format(fmt, std::forward<Args>(args)...));
	}

	std::vector<std::string> GetEntries()
	{
		return entries;
	}

	std::ofstream Open(bool truncate = false)
	{
		std::ofstream fs;
		fs.open(logFile, truncate ? std::ios::trunc : std::ios::app);
		return fs;
	}
};

extern Logger* g_logger;
