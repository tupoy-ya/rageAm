#pragma once
#include "Template/atSingleton.h"
#include <string>
#include <fstream>

class Logger : public atSingleton<Logger>
{
private:
	std::string logFile = "rageAm.txt";

public:
	void Log(std::string msg, bool truncate = false)
	{
		std::ofstream fs;
		fs.open(logFile, truncate ? std::ios::trunc : std::ios::app);
		fs << msg << std::endl;
		fs.close();
	}
};

extern Logger* g_logger;
