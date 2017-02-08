#include "StdAfx.h"
#include "LoggerFactory.h"

LoggerFacory::LoggerFacory()
{
}
LoggerFacory::~LoggerFacory()
{
	Remove();
}
void LoggerFacory::Remove(LPCTSTR loggerName)
{
	if (loggerName != NULL) {
		MapLogger::CPair *pair = mLoggerMap.PLookup(loggerName);
		LogTargetFile *pTargetFile((LogTargetFile*)pair->value->GetTarget()); // file
		delete pair->value;
		delete pTargetFile;
		mLoggerMap.RemoveKey(loggerName);
	}
	else {
		POSITION pos = mLoggerMap.GetStartPosition();
		while (pos != NULL)
		{
			CString nKey;
			Logger *logger(NULL);
			mLoggerMap.GetNextAssoc( pos, nKey, logger );
			Remove(nKey);
			pos = mLoggerMap.GetStartPosition();
		}
	}
}

LoggerFacory& LoggerFacory::GetInstance()
{
	static LoggerFacory loggerFactory;
	return loggerFactory;
}
Path LoggerFacory::GetLogFolderPath()
{
	std::wstring appPath;
	SystemUtils::GetSpecialFolderPath(CSIDL_APPDATA, true, appPath);
	Path logFolderPath(appPath.c_str());
	logFolderPath = logFolderPath.Append(Path(_T("\\Find\\Logs")));
	logFolderPath.CreateDir();
	return logFolderPath;
}
Logger& LoggerFacory::GetLogger(const CString &loggerName)
{
	if (loggerName.IsEmpty() && mLoggerMap.GetCount() > 0) {
		POSITION pos = mLoggerMap.GetStartPosition();
		CString key;
		Logger *logger(NULL);
		mLoggerMap.GetNextAssoc(pos, key, logger);
		return *logger;
	}
	MapLogger::CPair *pair = mLoggerMap.PLookup(loggerName);
	if (pair == NULL) {
		mLoggerMap[loggerName] = new Logger;
		pair = mLoggerMap.PLookup(loggerName);
		Path logFilePath(GetLogFolderPath());
		logFilePath = logFilePath.Append((LPCTSTR)loggerName);
		logFilePath = logFilePath.RenameExtension(_T(".log"));
		LogTargetFile *pFileTarget = new LogTargetFile();
		pFileTarget->SetLogFile(logFilePath);
		pair->value->AddTarget(pFileTarget);
	}
	return *pair->value;
}