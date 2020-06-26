#pragma once
#include "Path.h"
#include <map>
#include <fstream>
#include "SingletonManager.h"
#include "StdCondVar.h"


// Macro for debugging purpose - dont ship while using this macro - use and then remove all occurrence of this macro
#ifndef __PRETTY_FUNCTION__
#define __PRETTY_FUNCTION__ __FUNCTION__
#endif // !__PRETTY_FUNCTION__

#define LOG_CONSOLE(...) printf("%s|%d|%s|\n", __FILE__, __LINE__, __PRETTY_FUNCTION__); printf(__VA_ARGS__); printf("\n")



class LogTarget {
public:
    LogTarget(const char *loggerName = nullptr);
	virtual void Log(const char *logMessage) = 0;
	virtual ~LogTarget();
};

class LogTargetFile : public LogTarget {
public:
	LogTargetFile(LPCTSTR filePath = NULL, bool bAddPID = false, const char *loggerName = nullptr);
	~LogTargetFile();
	int SetLogFile(LPCTSTR filePath = NULL, bool bAddPID = false);
    const Path& GetLogFile() const { return mLogFilePath; }
	void Log(const char *logMessage);
private:
	int OpenLogFile(LPCTSTR filePath = NULL);
    Path mLogFilePath;
    std::fstream mLogFile;
    bool mbAddPID;
};

class Logger : public Singleton
{
public:
	enum LogLevel {
        kLogLevelOff,
		kLogLevelFatal,
		kLogLevelError,
		kLogLevelWarning,
		kLogLevelInfo,
		kLogLevelDebug,
        kLogLevelDev    // Will log to console too
	};
	SM_DEFINE_GETINSTANCE(Logger, 1, nullptr);
	LogLevel SetLogLevel(LogLevel newLogLevel);
	LogLevel GetLogLevel() const { return mLogLevel; }
    void Log(const char *logMessage, LogLevel logLevel = kLogLevelInfo);
	void Log(LogLevel logLevel, const char *logMessage, ...);
	LogTarget* AddTarget(const std::string &loggerName, LogTarget *pTarget, bool bAddOnlyIfNotExists = false);
	LogTarget* RemoveTarget(const std::string &loggerName);
    std::string HasTarget(LogTarget *pTarget) const;
	LogTarget* RemoveTarget(LogTarget *pTarget);
	LogTarget* GetTarget(const std::string &loggerName) const;
    void LogSummary(bool bLogSummaryEvenWhenEmpty = true);
    void DisableLogSummary(bool bDisable = true) { mbDisableLogSummary = bDisable; }
    void DisableWriteLogLevel(bool bDisable = true) { mbWriteLogLevel = !bDisable; }
    void DisableWritePidAndTid(bool bDisable = true) { mbWritePidAndTid = !bDisable; }
    void DisableTempNewLine() { mbTempDisableNewLine = true; }
private:
    Logger(void);
	std::map<std::string, LogTarget*> mLogTargets;
	LogLevel mLogLevel;
	std::string mSummary, mProcessName;
    bool mbDisableLogSummary = false;
    bool mbWriteLogLevel = true;
    bool mbWritePidAndTid = true;
    bool mbTempDisableNewLine = false;
	int m_nErros[kLogLevelInfo - kLogLevelFatal];
    StdMutex mLock;
};


#define LOGGER_LOG_WITH_LEVEL(l, ...) Logger::GetInstance()->Log(l, __VA_ARGS__)
#define LOGGER_LOG(...) LOGGER_LOG_WITH_LEVEL(Logger::kLogLevelInfo, __VA_ARGS__)
#define LOGGER_LOG_ERROR(...) LOGGER_LOG_WITH_LEVEL(Logger::kLogLevelError, __VA_ARGS__)
#define LOGGER_LOG_WARN(...) LOGGER_LOG_WITH_LEVEL(Logger::kLogLevelWarning, __VA_ARGS__)
#define LOGGER_LOG_DEBUG(...) LOGGER_LOG_WITH_LEVEL(Logger::kLogLevelDebug, __VA_ARGS__)
#define LOGGER_LOG_DEV(...) LOGGER_LOG_WITH_LEVEL(Logger::kLogLevelDev, __VA_ARGS__)


// Should be called only when application quitting or library is unloading
#define LOGGER_SAFE_LOG_WITH_LEVEL(l, ...) SM_CALL_INSTANCE_METHOD(Logger, Log, l, __VA_ARGS__)
#define LOGGER_SAFE_LOG(...) LOGGER_SAFE_LOG_WITH_LEVEL(Logger::kLogLevelInfo, __VA_ARGS__)
#define LOGGER_SAFE_LOG_DEV(...) LOGGER_SAFE_LOG_WITH_LEVEL(Logger::kLogLevelDev, __VA_ARGS__)

// Enable either of the following macros to enable conditional logging - LOGGER_CLOG
//#define COMPILED_TIME_LOGS_ENABLED // Log to logger
//#define COMPILED_TIME_PRINTF_LOGS_ENABLED // log to console using printf
#ifdef COMPILED_TIME_LOGS_ENABLED
#define LOGGER_CLOG_WITH_LEVEL(l, ...) LOGGER_LOG_WITH_LEVEL(l, __VA_ARGS__)
#define LOGGER_CLOG(...) LOGGER_LOG(__VA_ARGS__)
#define LOGGER_CLOG_ERROR(...) LOGGER_LOG_ERROR(__VA_ARGS__)
#define LOGGER_CLOG_WARN(...) LOGGER_LOG_WARN(__VA_ARGS__)
#define LOGGER_CLOG_DEBUG(...) LOGGER_LOG_DEBUG(__VA_ARGS__)
#define LOGGER_CLOG_DEV(...) LOGGER_LOG_DEV(__VA_ARGS__)
#elif defined(COMPILED_TIME_PRINTF_LOGS_ENABLED)
#define LOGGER_CLOG_WITH_LEVEL(l, ...) printf("%s|", l); ADP_LOG(__VA_ARGS__)
#define LOGGER_CLOG(...) LOGGER_CLOG_WITH_LEVEL("INFO", __VA_ARGS__)
#define LOGGER_CLOG_ERROR(...) LOGGER_CLOG_WITH_LEVEL("ERROR", __VA_ARGS__)
#define LOGGER_CLOG_WARN(...) LOGGER_CLOG_WITH_LEVEL("WARN", __VA_ARGS__)
#define LOGGER_CLOG_DEBUG(...) LOGGER_CLOG_WITH_LEVEL("DEBUG", __VA_ARGS__)
#define LOGGER_CLOG_DEV(...) LOGGER_CLOG_WITH_LEVEL("DEV", __VA_ARGS__)
#else
#define LOGGER_CLOG_WITH_LEVEL(l, ...)
#define LOGGER_CLOG(...)
#define LOGGER_CLOG_ERROR(...)
#define LOGGER_CLOG_WARN(...)
#define LOGGER_CLOG_DEBUG(...)
#define LOGGER_CLOG_DEV(...)
#endif
