#include "stdafx.h"
#include "RegisterSelf.h"
#include "Path.h"
#include "cMD5.h"

bool CRegisterSelf::TerminatePreviousInstance()
{
	bool bTerminated(true);
	// Check if already running
	HWND hWnd = FindWindow(NULL, _T("com.xyz.SalT"));
	if (hWnd != NULL) {
		DWORD pid(0);
		GetWindowThreadProcessId(hWnd, &pid);
		HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
		if (hProcess != INVALID_HANDLE_VALUE && hProcess != NULL) {
			bTerminated = TerminateProcess(hProcess, 3) != FALSE;
			WaitForSingleObject(hProcess, 3000);
			CloseHandle(hProcess);
		}
	}
	return bTerminated;
}

CRegisterSelf::CRegisterSelf()
{
}


CRegisterSelf::~CRegisterSelf()
{
}

ULONGLONG GetFileVersion(LPCTSTR pFilePath)
{
	DWORD               dwSize = 0;
	BYTE                *pVersionInfo = NULL;
	VS_FIXEDFILEINFO    *pFileInfo = NULL;
	UINT                pLenFileInfo = 0;
	ULONGLONG outVer(0);

	/*getting the file version info size */
	dwSize = GetFileVersionInfoSize(pFilePath, NULL);
	if (dwSize == 0)
	{
		return outVer;
	}

	pVersionInfo = new BYTE[dwSize]; /*allocation of space for the verison size */

	if (!GetFileVersionInfo(pFilePath, 0, dwSize, pVersionInfo)) /*entering all info     data to pbVersionInfo*/
	{
		delete[] pVersionInfo;
		return outVer;
	}

	if (!VerQueryValue(pVersionInfo, TEXT("\\"), (LPVOID*)&pFileInfo, &pLenFileInfo))
	{
		delete[] pVersionInfo;
		return outVer;
	}

	outVer = pFileInfo->dwFileVersionMS;
	outVer <<= 32;
	outVer |= pFileInfo->dwFileVersionLS;
	delete[] pVersionInfo;
	return outVer;
}
int CRegisterSelf::Register()
{
	TCHAR _path[1024];
	GetModuleFileName(NULL, _path, ARRAYSIZE(_path));
	Path curExe(_path);
	Path installPath(Path().GetSpecialFolderPath(CSIDL_COMMON_APPDATA).Append(_T("SalT.exe")));
	ULONGLONG installVer(GetFileVersion(installPath.c_str()));
	ULONGLONG thisVersion(GetFileVersion(curExe.c_str()));
	bool bInstall(thisVersion > installVer); // higher version
	if (!bInstall) {
		if (thisVersion == installVer) { // same version
			// Md5 different
			bInstall = curExe.GetFileTime(Path::ModifiedTime) > installPath.GetFileTime(Path::ModifiedTime)
				|| curExe.GetFileTime() > installPath.GetFileTime();
		}
	}
	if (bInstall) {
		TerminatePreviousInstance();
		if (!installPath.Delete() && installPath.Exists()) { // Delete existing
			Path deleteFile(installPath.GetNextUniqueFileName());
			MoveFileEx(installPath.c_str(), deleteFile.c_str(), MOVEFILE_REPLACE_EXISTING);
			MoveFileEx(deleteFile.c_str(), NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
		}
		// Copy new
		CopyFile(curExe.c_str(), installPath.c_str(), FALSE);
	}
	// Register for startup
	Path shortCutPath(Path().GetSpecialFolderPath(CSIDL_STARTUP).Append(_T("SalT.lnk")));
	if (!shortCutPath.Exists())
		installPath.CreateShortCut(shortCutPath);
	int retVal(0);
	// Check if already running
	HWND hWnd = FindWindow(NULL, _T("com.xyz.SalT"));
	if (hWnd != NULL)
		return 1;
	return 0;
}
