#include "Logger.h"
#include "STLUtils.h"
#include <stdarg.h>
#include "ProcessUtil.h"

LogTargetFile::LogTargetFile(LPCTSTR filePath /* = NULL */, bool bAddPID /* = false */, const char *loggerName /* = nullptr */)
    :LogTarget(loggerName), mLogFilePath(filePath), mbAddPID(bAddPID)
{
}
LogTargetFile::~LogTargetFile()
{
    if (mLogFile.is_open())
        SM_CALL_INSTANCE_METHOD(Logger, LogSummary);
    SetLogFile();
}
int LogTargetFile::SetLogFile(LPCTSTR filePath, bool bAddPID /* = false */)
{
    mbAddPID = bAddPID;
    return OpenLogFile(filePath);
}
int LogTargetFile::OpenLogFile(LPCTSTR filePath)
{
    int retVal(0);
    if (filePath != NULL && *filePath != 0) {
        Path inFilePath(filePath);
        if (mbAddPID) {
            Path logFileName(inFilePath.FileNameWithoutExt());
            std::string strPID;
            STLUtils::ChangeType(ProcessUtil::GetCurrentProcessId(), strPID);
            logFileName = std::string(logFileName) + "_" + strPID;
            logFileName = logFileName.RenameExtension(inFilePath.GetExtension());
            inFilePath = inFilePath.Parent().Append(logFileName);
        }
        if (!mLogFile.is_open() || mLogFilePath != inFilePath) {
            OpenLogFile(NULL); // close existing one
            mLogFilePath = inFilePath;
			if (Logger::GetInstance()->GetLogLevel() > Logger::kLogLevelOff) {
				mLogFilePath.Parent().CreateDir();
				try {
					mLogFile.open(mLogFilePath.c_str(), std::ios_base::out | std::ios_base::trunc | std::ios_base::binary);
                    if (!mLogFile.is_open()) {
                        mLogFilePath.DeleteFile();
                        mLogFile.open(mLogFilePath.c_str(), std::ios_base::out | std::ios_base::trunc | std::ios_base::binary);
                    }
					// Write UTF8 BOM
					mLogFile << "\xef\xbb\xbf";
                    Logger::GetInstance()->AddTarget(mLogFilePath.FileNameWithoutExt(), this, true);
				}
				catch (...) {
					retVal = 1;
				}
			}
        }
    }
	else if (mLogFile.is_open()) {
		mLogFile.close();
		mLogFilePath.clear();
	}
    return retVal;
}
void LogTargetFile::Log(const char *logMessage)
{
    if (logMessage && *logMessage) {
        if (!mLogFile.is_open() || !(mLogFilePath.empty() || mLogFilePath.Exists()))
            OpenLogFile(mLogFilePath.c_str());
        if (mLogFile.is_open()) {
            mLogFile << logMessage;
            mLogFile.flush();
        }
    }
}


static Logger::LogLevel Logger_GetDefaultLogLevel(Logger::LogLevel newLogLevel = (Logger::LogLevel)-1)
{
    Logger::LogLevel outLogLevel = newLogLevel;
    if ((int)outLogLevel < 0) {
        outLogLevel =
#ifdef _DEBUG
    Logger::kLogLevelDebug
#else
    Logger::kLogLevelInfo
#endif
    ;
    }
    return outLogLevel;

}

Logger::Logger(void) : mLogLevel(Logger_GetDefaultLogLevel()),
    mProcessName(Path::GetModuleFilePath().FileNameWithoutExt())
{
    memset(m_nErros, 0, sizeof(m_nErros));
}
void Logger::ResetDisableTempNewLine()
{
    mbOldValueOfTempDisableNewLine = mbTempDisableNewLine;
    mbTempDisableNewLine = false;
}

void Logger::LogSummary(bool bLogSummaryEvenWhenEmpty /* = true */)
{
    if (mbDisableLogSummary);
    else if (bLogSummaryEvenWhenEmpty || !mSummary.empty()) {
        Log("-------------------------------------- Summary --------------------------------------");
        Log(mSummary.c_str());
        Log(kLogLevelInfo, " - %d fatal error(s), %d error(s), %d warning(s)", m_nErros[0], m_nErros[1], m_nErros[2]);
        if (!mSummary.empty())
            Log("Please search the above error/warning string(s) to find when the error occurred.");
    }
	mSummary.clear();
	memset(m_nErros, 0, sizeof(m_nErros));
}


Logger::LogLevel Logger::SetLogLevel(Logger::LogLevel newLogLevel)
{
	LogLevel logLevel(mLogLevel);
    mLogLevel = Logger_GetDefaultLogLevel(newLogLevel);
	return logLevel;
}
LogTarget* Logger::AddTarget(const std::string &loggerName, LogTarget *pTarget, bool bAddOnlyIfNotExists /* = false */)
{
	LogTarget *pOldTarget(GetTarget(loggerName));
    if (pTarget == nullptr)
        mLogTargets.erase(loggerName);
    else if (!bAddOnlyIfNotExists || HasTarget(pTarget).empty()) {
        RemoveTarget(pTarget); // Remove existing one
        mLogTargets[loggerName] = pTarget;
    }
	return pOldTarget;
}
LogTarget* Logger::RemoveTarget(const std::string &loggerName)
{
    if (loggerName.empty())
        return nullptr;
    return AddTarget(loggerName, nullptr);
}
std::string Logger::HasTarget(LogTarget *pTarget) const
{
    for (auto &cit : mLogTargets)
        if (cit.second == pTarget)
            return cit.first;
    return "";
}

LogTarget* Logger::RemoveTarget(LogTarget *pTarget)
{
    return RemoveTarget(HasTarget(pTarget));
}

LogTarget* Logger::GetTarget(const std::string &loggerName) const
{
    auto cit(mLogTargets.find(loggerName));
    if (cit != mLogTargets.end()) {
        return cit->second;
    }
	return nullptr;
}
static const char* GetLogLevelString(Logger::LogLevel logLevel)
{
    static const char* logLevelStrings[] = {
        "",
        "FATAL",
        "ERROR",
        "WARN",
        "INFO",
        "DEBUG",
        "DEV"
    };
    if (logLevel >= 0 && logLevel < sizeof(logLevelStrings) / sizeof(logLevelStrings[0]))
        return logLevelStrings[logLevel];
    return "UNKNOWN";
}
void Logger::Log(const char *logMessage, LogLevel logLevel /* = kLogLevelInfo */)
{
	if (mLogLevel == kLogLevelOff || logLevel > mLogLevel || logMessage == NULL || *logMessage == 0)
		return;

    std::string strLogMessage;
    if (mbWriteLogLevel || mbWritePidAndTid)
        ResetDisableTempNewLine();
    if (!mbTempDisableNewLine && mbOldValueOfTempDisableNewLine) {
        strLogMessage += "\r\n";
        mbOldValueOfTempDisableNewLine = false;
    }
    if (mbWriteLogLevel) {
        strLogMessage += GetLogLevelString(logLevel);
        strLogMessage += " | ";
    }
    if (!mbTempDisableNewLine)
         strLogMessage += ProcessUtil::GetLocalTimeString(ProcessUtil::Time_MicroSeconds) + " | ";
    if (mbWritePidAndTid) {
        strLogMessage += " | exe: " + mProcessName;
        std::string tempString;
        STLUtils::ChangeType(ProcessUtil::GetCurrentProcessId(), tempString);
        strLogMessage += " | pid:" + tempString;
        STLUtils::ChangeType(ProcessUtil::GetCurrentThreadId(), tempString);
        strLogMessage += " | tid:" + tempString + " | ";
    }
    strLogMessage += logMessage;
    if (!mbTempDisableNewLine)
        strLogMessage += "\r\n";
    ResetDisableTempNewLine();
    {
        {
            StdAutoMutexLock autoLock(mLock);

            if (!mbDisableLogSummary && logLevel < kLogLevelInfo) {
                if (!mSummary.empty())
                    mSummary += "\r\n";
                mSummary += std::string(GetLogLevelString(logLevel)) + ": " + logMessage;
                if (mSummary.length() > 4096 * 2) {
                    mSummary.erase(mSummary.begin(), mSummary.begin() + 4096);
                }
                m_nErros[logLevel - 1]++;
            }
            for (auto logTarget : mLogTargets)
                logTarget.second->Log(strLogMessage.c_str());
        }

    }
}

void Logger::Log(LogLevel logLevel, const char *fmt, ...)
{
    if (mLogLevel == kLogLevelOff || logLevel > mLogLevel || fmt == NULL || *fmt == 0)
        return;
    if (fmt != NULL) {
        va_list args1;
        va_start(args1, fmt);
        va_list args2;
        va_copy(args2, args1);
        std::vector<char> buf(1 + std::vsnprintf(NULL, 0, fmt, args1));
        va_end(args1);
        std::vsnprintf(buf.data(), buf.size(), fmt, args2);
        va_end(args2);
        Log(buf.data(), logLevel);
    }
}


LogTarget::LogTarget(const char *loggerName /*= nullptr*/)
{
    if (loggerName && *loggerName)
        Logger::GetInstance()->AddTarget(loggerName, this);
}

LogTarget::~LogTarget()
{
    SM_CALL_INSTANCE_METHOD(Logger, RemoveTarget, this);
}
