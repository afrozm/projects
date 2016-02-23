// rf.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Progress.h"
#include "Path.h"
#include "FileMapping.h"
#include "RecoverManager.h"
#include "CountTimer.h"

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
            if (progress.UpdateProgress(currentDone))
                cp.ShowPercentage(progress.GetCurrentPercentageDone());
            recoverManger.EndRecover();
            progress.UpdateProgress(totalSize);
            cp.ShowPercentage(progress.GetCurrentPercentageDone());
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

