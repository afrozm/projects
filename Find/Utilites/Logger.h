#pragma once
#include "Path.h"

class LogTarget {
public:
	virtual void LogSimple(LPCTSTR /*logMessage*/) {}
	virtual void Log(LPCTSTR logMessage) = 0;
};

class LogTargetFile : public LogTarget {
public:
	LogTargetFile();
	~LogTargetFile();
	int SetLogFile(LPCTSTR filePath = NULL);
	void Log(LPCTSTR logMessage);
private:
	int OpenLogFile(LPCTSTR filePath = NULL);
	HANDLE m_hLogFile;
	Path mFilePath;
	CMutex mLock;
};

class Logger
{
public:
	enum LogLevel {
		kLogLevelFatal,
		kLogLevelError,
		kLogLevelWarning,
		kLogLevelInfo,
		kLogLevelDebug
	};
	Logger(void);
	~Logger(void);
	LogLevel SetLogLevel(LogLevel newLogLevel);
	LogLevel GetLogLevel() const { return mLogLevel; }
	void Log(LPCTSTR logMessage, LogLevel logLevel = kLogLevelInfo);
	void Log(LogLevel logLevel, LPCTSTR logMessage, ...);
	void AddTarget(LogTarget *pTarget);
	void RemoveTarget(LogTarget *pTarget);
	LogTarget* GetTarget(INT_PTR index = 0);
private:
	CArrayEx<LogTarget*> mLogTargets;
	void LogSummary();
	LogLevel mLogLevel;
	lstring mPrefixString;
	lstring mSummary;
	int m_nErros[3];
	CMutex mLock;
};
