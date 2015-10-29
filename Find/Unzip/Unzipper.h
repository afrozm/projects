
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "windows.h"
#include <tchar.h>
const UINT MAX_COMMENT = 255;

// create our own fileinfo struct to hide the underlying implementation
struct UZ_FileInfo
{
	TCHAR szFileName[MAX_PATH + 1];
	TCHAR szComment[MAX_COMMENT + 1];
	
	DWORD dwVersion;  
	DWORD dwVersionNeeded;
	DWORD dwFlags;	 
	DWORD dwCompressionMethod; 
	DWORD dwDosDate;	
	DWORD dwCRC;   
	UINT64 dwCompressedSize; 
	UINT64 dwUncompressedSize;
	DWORD dwInternalAttrib; 
	DWORD dwExternalAttrib; 
	bool bFolder;
};

#include "ZLibCommon.h"

class CUnzipper  
{
public:
	CUnzipper(LPCTSTR szFileName = NULL);
	virtual ~CUnzipper();
	
	// simple interface
	BOOL Unzip(LPCTSTR szFileName, LPCTSTR szFolder = NULL, BOOL bIgnoreFilePath = FALSE);
	
	// works with prior opened zip
	BOOL Unzip(BOOL bIgnoreFilePath = FALSE); // unzips to output folder or sub folder with zip name 
	BOOL UnzipTo(LPCTSTR szFolder, BOOL bIgnoreFilePath = FALSE); // unzips to specified folder

	// extended interface
	BOOL OpenZip(LPCTSTR szFileName);
	BOOL CloseZip(); // for multiple reuse
	BOOL SetOutputFolder(LPCTSTR szFolder); // will try to create
	
	// unzip by file index
	UINT64 GetFileCount();
	BOOL GetFileInfo(int nFile, UZ_FileInfo& info);
	BOOL UnzipFile(int nFile, LPCTSTR szFolder = NULL, BOOL bIgnoreFilePath = FALSE);
	
	// unzip current file
	BOOL GotoFirstFile(LPCTSTR szChildPath = NULL);
	BOOL GotoNextFile(LPCTSTR szChildPath = NULL);
	BOOL GetFileInfo(UZ_FileInfo& info);
	BOOL UnzipFile(LPCTSTR szFolder = NULL, BOOL bIgnoreFilePath = FALSE);

	// helpers
	BOOL GotoFile(LPCTSTR szFileName, BOOL bIgnoreFilePath = TRUE);
	BOOL GotoFile(int nFile);
	void SetUnzipCallbackFn(ZLibCallback unzipCallbackFn, void *pUserData = NULL);
	UINT64 GetUnzipSize();
	static LONGLONG GetUnzipSize(LPCTSTR zipFilePath);
	BOOL SetUnzipPassword(const char *password);
	BOOL IsOpen() const {return m_uzFile != NULL;}
protected:
	void* m_uzFile;
	UINT64 muUncompressedSize;
	ZLibCBData mUnzipData;
	ZLibCallback mCallbackFn;
	TCHAR m_szOutputFolder[MAX_PATH + 1];

protected:
	BOOL CreateFolder(LPCTSTR szFolder);
	BOOL CreateFilePath(LPCTSTR szFilePath); // truncates from the last '\'
	BOOL SetFileModTime(LPCTSTR szFilePath, DWORD dwDosDate);

};
