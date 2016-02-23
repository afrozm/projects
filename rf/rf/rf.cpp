// rf.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Progress.h"
#include "Path.h"
#include "FileMapping.h"
#include "RecoverManager.h"

COORD GetConsolePos(void)
{
	CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE),
		&csbiInfo);
	return csbiInfo.dwCursorPosition;
}

void SetConsolePos(COORD newPos)
{
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), newPos);
}
void MoveConsoleCursor(SHORT x, SHORT y)
{
	COORD cp = GetConsolePos();
	cp.X += x;
	cp.Y += y;
	SetConsolePos(cp);
}
class AutoConsolePos {
	COORD mCurPos;
public:
	AutoConsolePos(PCOORD setPos = NULL)
	{
		mCurPos = GetConsolePos();
		if (setPos)
			SetConsolePos(*setPos);
	}
	~AutoConsolePos()
	{
		SetConsolePos(mCurPos);
	}
};
class ConsolePrinter
{
	COORD printPos;
	TCHAR mStr[256];
	int strLen;
/*	void AllocateSpaceForStr(int newLen)
	{
		if (newLen > strLen) {
			if (mStr)
				delete[] mStr;
			mStr = new TCHAR[newLen+1];
			mStr[0] = 0;
			strLen = newLen+1;
		}
		else if (newLen <= 0 && mStr) {
			delete[] mStr;
			mStr = NULL;
			strLen = 0;
		}
	}*/
	void CopyStr(const TCHAR *str)
	{
	//	AllocateSpaceForStr(lstrlen(str));
		lstrcpy(mStr, str);
	}
public:
	ConsolePrinter()
	{
		printPos = GetConsolePos();
		mStr[0] = 0;
		strLen = 0;
	}
	void Print(const TCHAR *str)
	{
		if (lstrcmp(mStr, str)) {
			Erase();
			CopyStr(str);
			AutoConsolePos acp(&printPos);
			lprintf(mStr);
		}
	}
	void Erase()
	{
		int len = lstrlen(mStr);
		AutoConsolePos acp(&printPos);
		while (len-- > 0) {
			lprintf(_T(" "));
		}
	}
	~ConsolePrinter(void)
	{
//		AllocateSpaceForStr(0); // Free space
	}
};

struct ConsoleProgress {
	COORD printPos;
	COORD printDotPos;
	double m_iPercentDone, m_dLastPercentage;
	ConsoleProgress() {
		lprintf(_T("\r\t\t\t\t\t"));
		printPos = GetConsolePos();
		printDotPos = printPos;
		printDotPos.X = 0;
		m_iPercentDone = 0;
		m_dLastPercentage = 0;
		lprintf(_T("\r"));
	}
	~ConsoleProgress() {
	}
	void ShowPercentage(double percentDone)
	{
		if (percentDone > 100)
			percentDone = 100;
		/*if (m_iPercentDone < percentDone)*/ {
			m_iPercentDone = percentDone;
			AutoConsolePos acp;
			if (m_iPercentDone - m_dLastPercentage > 5) {
				m_dLastPercentage = m_iPercentDone;
				SetConsolePos(printDotPos);
				lprintf(_T("."));
				printDotPos.X++;
			}
			SetConsolePos(printPos);
			lprintf(_T(" %02.02f%% "), percentDone);
		}
	}
};
#define TIMER_SIZE 7
static const TCHAR *sktimeNames[] = {
	_T("year"),
	_T("month"),
	_T("day"),
	_T("hour"),
	_T("minute"),
	_T("second"),
	_T("millisecond"),
};
static const __int64 skTimeDuration[] = {
	1000LL*60LL*60LL*24LL*365LL,
	1000*60*60*24*30UL,
	1000*60*60*24,
	1000*60*60,
	1000*60,
	1000,
	1
};
struct STime {
	__int64 times[TIMER_SIZE]; // index 0 - timeNames[0], 1 timeNames[1]
	STime(__int64 milliSecs)
	{
		for (int i = 0; i < TIMER_SIZE; i++) {
			times[i] = milliSecs / skTimeDuration[i];
			milliSecs %= skTimeDuration[i];
		}
	}
};
class CountTimer {
	DWORD mLastTime;
	DWORD mCurTime;
	__int64 mTimeDuration;
	DWORD mTimerUpdateDuration;
	bool m_bCountDownWards;
	ConsolePrinter m_CP;
public:
	CountTimer(bool bCountDownWards = false, DWORD timerUpdateDuration = 1000)
	{
		m_bCountDownWards = bCountDownWards;
		mLastTime = GetTickCount();
		mTimeDuration = 0;
		mTimerUpdateDuration = timerUpdateDuration;
	}
	void SetTimeDuration(__int64 timeDuration)
	{
		m_bCountDownWards = true; // its a count down timer
		mTimeDuration = timeDuration;
	}
	__int64 GetTimeDuration(void)
	{
		UpdateTimeDuration();
		return mTimeDuration;
	}

	void GetString(TCHAR *str, int size)
	{
		STime sTime(mTimeDuration);
		int i;
		for (i = 0; i < TIMER_SIZE -2; i++) {
			if (sTime.times[i])
				break;
		}
		int len = _sntprintf_s(str, size, size, _T("%d %s%s"), (int)sTime.times[i], sktimeNames[i],
			(int)sTime.times[i] > 1 ? _T("s") : _T(""));
		i++;
		if (i < TIMER_SIZE -1 && sTime.times[i] > 0) {
			len = _sntprintf_s(str+len, size-len, size-len, _T(" %d %s%s"), (int)sTime.times[i], sktimeNames[i],
			(int)sTime.times[i] > 1 ? _T("s") : _T(""));
		}
	}
	bool UpdateTimeDuration()
	{
		DWORD curTime = GetTickCount();
		if (curTime - mCurTime > mTimerUpdateDuration) {
			DWORD interval = curTime - mLastTime;
			if (m_bCountDownWards) {
				mTimeDuration -= interval;
				if (mTimeDuration < 0)
					mTimeDuration = 0;
				mLastTime = curTime;
			}
			else {
				mTimeDuration = interval;
			}
			mCurTime = curTime;
			return true;
		}
		return false;
	}
	void PrintTimeDuration()
	{
		if (UpdateTimeDuration()) {
			TCHAR str[32];
			GetString(str, 32);
			m_CP.Print(str);
		}
	}
};


void printf_Buffer(const unsigned char *pBuffer, int len)
{
	for (int i = 0; i < len; pBuffer++) {
		++i;
		_tprintf(_T("%02x "), *pBuffer);
		if ((i & 0x7) == 0)
			_tprintf(_T(" "));
		if ((i & 0xf) == 0)
			_tprintf(_T("\n"));
	}
}



int _tmain(int argc, _TCHAR* argv[])
{
	if (argc < 3) {
		_tprintf(_T("Usage: rf <source file/disk path> <desnitantion file/folder>\nFor disk use \\\\.\\H: as source path"));
		return -1;
	}
	HANDLE hFile = CreateFile(argv[1], GENERIC_READ, FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (hFile != INVALID_HANDLE_VALUE) {
		UINT64 currentDone(0), totalSize(0);
		LARGE_INTEGER fileSize = {0};
		GetFileSizeEx(hFile, &fileSize);
		totalSize = fileSize.QuadPart;
		if (totalSize == 0) { // Disk Path
			ULARGE_INTEGER li = {0};
			GetDiskFreeSpaceEx(argv[1]+4, NULL, &li, NULL);
			totalSize = li.QuadPart;
		}
		Progress progress;
		progress.SetTask(totalSize);
		ConsoleProgress cp;
		Path dstPath(argv[2]);
		DWORD updateWSTime = GetTickCount(); // Update Writing speed time
		lprintf(_T("\nTime Elapsed: "));
		CountTimer timeElapsed;
		lprintf(_T("\t\t\t\tTime Remaining: "));
		CountTimer timeRemaining(true);
		if (dstPath.IsDir()) {
            CRecoverManager recoverManger;
            recoverManger.SetInputFileHandle(hFile);
            recoverManger.SetSavePath(dstPath);
            recoverManger.SetTotal(totalSize);
            recoverManger.Initialize();
            recoverManger.BeginRecover();
			while (recoverManger.ProcessRecover()) {
                currentDone = recoverManger.GetCurrentDone();
					if (progress.UpdateProgress(currentDone))
						cp.ShowPercentage(progress.GetCurrentPercentageDone());
					DWORD curTime = GetTickCount();
					if (curTime - updateWSTime >= 5000) { // 5 secs passed
						timeRemaining.SetTimeDuration((timeElapsed.GetTimeDuration() * totalSize) / currentDone);
						updateWSTime = curTime;
					}
					timeElapsed.PrintTimeDuration();
					timeRemaining.PrintTimeDuration();
			}
            recoverManger.EndRecover();
		}
		else {
			CFileMapping fileDstMap;
			CFileMapping fileSrcMap;
			//fileSrcMap.GetFileMapping(hFile);
			fileSrcMap.SetFileHandle(hFile);
			//HANDLE hDestFile = CreateFile(argv[2], GENERIC_ALL, FILE_SHARE_READ, NULL, CREATE_ALWAYS, 0, NULL);
			//if (hDestFile != INVALID_HANDLE_VALUE) {
			//	fileDstMap.SetFileHandle(hDestFile);
			if (fileDstMap.GetFileMapping(argv[2], totalSize) != NULL) {
				std::vector<char> buffer;
				buffer.resize(1024*1024);
				size_t size = buffer.size();
				char *buf = &buffer[0];
				while (currentDone < totalSize) {
					DWORD nBytesRead(fileSrcMap.Read(buf, (UINT)size));
					if (nBytesRead > 0) {
						fileDstMap.Write(buf, nBytesRead);
						currentDone += nBytesRead;
						if (progress.UpdateProgress(currentDone))
							cp.ShowPercentage(progress.GetCurrentPercentageDone());
						DWORD curTime = GetTickCount();
						if (curTime - updateWSTime >= 5000) { // 5 secs passed
							double p = progress.GetCurrentPercentageDone();
							if (p < 0.01)
								p = 0.01;
							timeRemaining.SetTimeDuration(__int64((timeElapsed.GetTimeDuration() * (100-p)) / p));
							updateWSTime = curTime;
						}
						timeElapsed.PrintTimeDuration();
						timeRemaining.PrintTimeDuration();
					}
					else break;
				}
				//CloseHandle(hDestFile);
			}
			else _tprintf(_T("Cannot open file: '%s'\nError: %d"), argv[2], GetLastError());
		}
		CloseHandle(hFile);
	}
	else _tprintf(_T("Cannot open file: '%s'\nError: %d"), argv[1], GetLastError());
	return 0;
}

