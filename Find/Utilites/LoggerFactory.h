#pragma once
#include "Logger.h"

class LoggerFacory
{
public:
	Logger& GetLogger(const CString &loggerName);
	static LoggerFacory& GetInstance();
	~LoggerFacory();
	void Remove(LPCTSTR loggerName = NULL);
	static Path GetLogFolderPath();
private:
	LoggerFacory();
	typedef CMap <CString, LPCTSTR, Logger*, Logger*&> MapLogger;
	MapLogger mLoggerMap;
};
