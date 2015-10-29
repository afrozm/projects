#include "StdAfx.h"
#include "Logger.h"
#include "StdUtils.h"
#include "AutoLock.h"
#include "SystemUtils.h"
#include "LoggerFactory.h"

Logger::Logger(void)
: 
#ifdef _DEBUG
mLogLevel(Logger::kLogLevelDebug)
#else
mLogLevel(Logger::kLogLevelInfo)
#endif
{
	ZeroMemory(m_nErros, sizeof(m_nErros));
	Path finddebug(LoggerFacory::GetLogFolderPath().Parent());
	finddebug = finddebug.Append(Path(_T("finddebug")));
	if (finddebug.Exists())
		mLogLevel = kLogLevelDebug;
}

Logger::~Logger(void)
{
	LogSummary();
}
LogTargetFile::LogTargetFile()
	: m_hLogFile(INVALID_HANDLE_VALUE)
{

}
LogTargetFile::~LogTargetFile()
{
	SetLogFile();
}
int LogTargetFile::SetLogFile(LPCTSTR filePath)
{
	CAutoLock autoLock(mLock);
	return OpenLogFile(filePath);
}
int LogTargetFile::OpenLogFile(LPCTSTR filePath)
{
	int retVal(0);
	if (filePath != NULL) {
		OpenLogFile(NULL);
		Path logFilePath(filePath);
		logFilePath.Parent().CreateDir();
		m_hLogFile = CreateFile(filePath, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (m_hLogFile == INVALID_HANDLE_VALUE)
			retVal = GetLastError();
		else {
			mFilePath = logFilePath;
		}
	}
	else if (m_hLogFile != INVALID_HANDLE_VALUE) {
		CloseHandle(m_hLogFile);
		m_hLogFile = INVALID_HANDLE_VALUE;
	}
	return retVal;
}
void LogTargetFile::Log(LPCTSTR logMessage)
{
	if (m_hLogFile == INVALID_HANDLE_VALUE || !(mFilePath.IsEmpty() || mFilePath.Exists())) {
		OpenLogFile(mFilePath);
	}
	if (m_hLogFile != INVALID_HANDLE_VALUE) {
		DWORD nBytes((DWORD)(lstrlen(logMessage)*sizeof(TCHAR)));
		SetFilePointer(m_hLogFile, 0, NULL, FILE_END);
		WriteFile(m_hLogFile, logMessage, nBytes, &nBytes, NULL);
	}
}
void Logger::LogSummary()
{
	Log(_T("-------------------------------------- Summary --------------------------------------"));
	Log(mSummary.c_str());
	Log(kLogLevelInfo, _T(" - %d fatal error(s), %d error(s),  warning(s)"), m_nErros[0], m_nErros[1], m_nErros[2]);
	if (!mSummary.empty())
		Log(_T("Please search the above error/warning string(s) to find when the error occurred."));
	mSummary.clear();
	ZeroMemory(m_nErros, sizeof(m_nErros));
}
Logger::LogLevel Logger::SetLogLevel(Logger::LogLevel newLogLevel)
{
	LogLevel logLevel(mLogLevel);

	if (newLogLevel <= kLogLevelDebug) {
		CAutoLock autoLock(mLock);
		mLogLevel = newLogLevel;
	}

	return logLevel;
}
void Logger::AddTarget(LogTarget *pTarget)
{
	CAutoLock autoLock(mLock);
	mLogTargets.AddUnique(pTarget);
}
void Logger::RemoveTarget(LogTarget *pTarget)
{
	CAutoLock autoLock(mLock);
	mLogTargets.Remove(pTarget);
}
LogTarget* Logger::GetTarget(INT_PTR index)
{
	CAutoLock autoLock(mLock);
	if (index >= 0 && index < mLogTargets.GetCount())
		return mLogTargets[index];
	return NULL;
}
void Logger::Log(LPCTSTR logMessage, LogLevel logLevel)
{
	if (logLevel > mLogLevel || logMessage == NULL || *logMessage == 0)
		return;
	CAutoLock autoLock(mLock);
	time_t ltime;
	 _time64( &ltime ); 
	struct tm today;
	_localtime64_s( &today, &ltime );
	const LPCTSTR logLevelStrings[] = {
		_T("FATAL"),
		_T("ERROR"),
		_T("WARN"),
		_T("INFO"),
		_T("DEBUG"),
	};
	lstring preFixString = _T("[") + lstring((LPCTSTR)SystemUtils::IntToString(GetCurrentThreadId())) + _T("] ")
		+ GetDateTimeFormat(today, _T("%d-%m-%Y %H:%M:%S"))
		+ _T(" [") + logLevelStrings[logLevel] + _T("] ");
	lstring lsMessage;
	if (mPrefixString != preFixString) {
		mPrefixString = preFixString;
		lsMessage += mPrefixString;
		lsMessage += _T("\r\n");
	}
	lsMessage += logMessage;
	lsMessage += _T("\r\n");
	if (logLevel < kLogLevelInfo) {
		mSummary += logMessage;
		mSummary += _T("\r\n");
		if (mSummary.length() > 4096*2) {
			mSummary.erase(mSummary.begin(), mSummary.begin()+4096);
		}
		m_nErros[logLevel]++;
	}
	for (INT_PTR i = 0; i < mLogTargets.GetCount(); ++i) {
		mLogTargets[i]->LogSimple(logMessage);
		mLogTargets[i]->Log(lsMessage.c_str());
	}
#ifdef _DEBUG
	OutputDebugString(lsMessage.c_str());
#endif
}
void Logger::Log(LogLevel logLevel, LPCTSTR msg, ...)
{
	if (msg != NULL) {
		va_list arg;
		va_start(arg, msg);
		int len = _vsctprintf(msg, arg) + 4*sizeof(TCHAR); // _vscprintf doesn't count + 1; terminating '\0'
		TCHAR *buf = new TCHAR[len];
		_vstprintf_s(buf, len, msg, arg);
		Log(buf, logLevel);
		delete buf;
	}
}