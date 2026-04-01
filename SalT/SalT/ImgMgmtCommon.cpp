#ifdef _WIN32
#include "stdafx.h"
#endif

#include "ImgMgmt.h"
#include <set>
#include <chrono>
#include <thread>
#include <atomic>

// --- Helpers ---

static void ReplaceStr(lstring& str, const lstring& oldStr, const lstring& newStr)
{
	for (auto cit(oldStr.begin()); cit != oldStr.end();) {
		bool bMatchFound(false);
		size_t pos = 0;
		while ((pos = str.find(*cit, pos)) != lstring::npos)
		{
			str.replace(pos, 1, newStr);
			pos += newStr.length();
			bMatchFound = true;
		}
		if (!bMatchFound)
			++cit;
	}
}

static lstring SanitizePrefix(const lstring &windowTitle)
{
	Path imgPath(GetImagePath());
	int remLength = 250 - (int)imgPath.length() - 12;
	lstring prefix(_T("SS"));
	if (remLength > 0 && !windowTitle.empty()) {
		lstring morePrefix(windowTitle);
		if ((int)morePrefix.length() > remLength)
			morePrefix.erase(remLength);
		ReplaceStr(morePrefix, _T("\\/:*?\"<>|"), _T(""));
		if (morePrefix.length() > 0)
			prefix += _T(".") + morePrefix;
	}
	return prefix;
}

// --- Image Path ---

Path GetImagePath()
{
#ifdef _WIN32
	const int kFolderID = 0x0023; // CSIDL_COMMON_APPDATA
#else
	const int kFolderID = 14; // NSApplicationSupportDirectory
#endif
	Path outPath(Path::GetSpecialFolderPath(kFolderID));
	outPath = outPath.Append(_T("salt"));
	outPath = outPath.Append(_T("ss"));
	outPath.CreateDir();
	return outPath;
}

// --- Cleanup ---

const INT64 kMaxSize(8 * 1024 * 1024 * 1024LL); // 8 GB

struct CleanupFileInfo {
	ULONGLONG timestamp;
	INT64 fileSize;
	Path filePath;
	CleanupFileInfo() : timestamp(0), fileSize(0) {}
	bool operator<(const CleanupFileInfo &rt) const { return timestamp < rt.timestamp; }
};

static int FindCallback_Cleanup(FindData &findData, void *pUserParam)
{
	if (findData.pFindData != NULL) {
		Path filePath(findData.fullPath);
		if (!filePath.IsDir()) {
			CleanupFileInfo info;
			info.filePath = filePath;
			info.fileSize = filePath.GetSize();
			FILETIME ct = {};
			filePath.GetFileTime(&ct, nullptr, nullptr);
#ifdef _WIN32
			ULARGE_INTEGER ul = { ct.dwLowDateTime, ct.dwHighDateTime };
			info.timestamp = ul.QuadPart;
#else
			info.timestamp = (ULONGLONG)ct.tv_sec * 1000000000ULL + (ULONGLONG)ct.tv_nsec;
#endif
			((std::multiset<CleanupFileInfo>*)pUserParam)->insert(info);
		}
	}
	return 0;
}

static std::atomic<bool> sCleanupRunning(false);

static void CleanupThreadProc()
{
	Path imgPath(GetImagePath());
	INT64 folderSize(imgPath.GetSize());
	if (folderSize > kMaxSize) {
		std::multiset<CleanupFileInfo> fileSet;
		Finder(FindCallback_Cleanup, &fileSet).StartFind(imgPath);
		for (auto it = fileSet.begin(); it != fileSet.end() && (folderSize > (kMaxSize >> 1)); ++it) {
			folderSize -= it->fileSize;
			it->filePath.Delete();
		}
	}
	sCleanupRunning = false;
}

void CleanupImgFolder()
{
	using clock = std::chrono::steady_clock;
	static auto lastCleanup = clock::time_point();
	auto now = clock::now();
	auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastCleanup).count();
	if (elapsedMs < 60 * 60 * 1000) {
		if (elapsedMs < 5 * 60 * 1000)
			return;
		if (GetImagePath().GetSize() < kMaxSize)
			return;
	}
	lastCleanup = now;
	if (sCleanupRunning)
		return;
	sCleanupRunning = true;
	std::thread(CleanupThreadProc).detach();
}

// --- Save Window Image ---

bool SaveWindowImage(const lstring &windowTitle, bool bThrottle)
{
	using clock = std::chrono::steady_clock;
	static auto lastSavedTime = clock::time_point();
	auto now = clock::now();
	if (bThrottle && std::chrono::duration_cast<std::chrono::seconds>(now - lastSavedTime).count() < 60)
		return false;

	lstring prefix(SanitizePrefix(windowTitle));
	int imgNum(0);
	Path imgPath = GetImagePath().GetUniqueFileName(imgNum, _T(".jpg"), prefix.c_str());

	if (CaptureAndSaveWindowImage(imgPath)) {
		lastSavedTime = now;
		CleanupImgFolder();
		return true;
	}
	return false;
}
