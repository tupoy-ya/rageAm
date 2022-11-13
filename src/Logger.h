#pragma once
#include "Template/atSingleton.h"
#include <string>
#include <fstream>
#include <vector>

class Logger : public atSingleton<Logger>
{
	std::string logFile = "rageAm.txt";
	std::vector<std::string> entries;
public:
	void Log(const std::string& msg, bool truncate = false)
	{
		auto fs = Open(truncate);
		fs << msg << std::endl;
		entries.push_back(msg);
		fs.close();
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
