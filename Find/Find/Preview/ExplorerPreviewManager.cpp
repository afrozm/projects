#include "StdAfx.h"
#include "ExplorerPreviewManager.h"
#include "Find.h"

CExplorerPreviewManager::CExplorerPreviewManager(void)
	: m_pPreviewWindow(NULL), m_pCurPreviewHandler(NULL)
{
	LoadSupportedExts();
}


CExplorerPreviewManager::~CExplorerPreviewManager(void)
{
	Unload();
}
CExplorerPreviewManager& CExplorerPreviewManager::GetInstance()
{
	static CExplorerPreviewManager sExplorerPreviewManager;
	return sExplorerPreviewManager;
}
void CExplorerPreviewManager::LoadSupportedExts()
{
	Unload();
	WCHAR wcName[ MAX_PATH ];	
	WCHAR wcData[ MAX_PATH ];
	CString csExt = NULL;	

	DWORD cName = sizeof( wcName)/ sizeof(wcName[0]);
	LONG cbData = sizeof( wcData);

	for( DWORD i =0; ; i ++)
	{
		cName = sizeof( wcName)/ sizeof(wcName[0]);
		if( ERROR_SUCCESS == RegEnumKey( HKEY_CLASSES_ROOT,
			i,
			wcName,
			cName) )			
		{
			if( L'.' != wcName[0] )
			{
				continue;
			}
			csExt = wcName ;
			for (int rt = 0; rt < 2; ++rt) {
				lstrcat( wcName, L"\\ShellEx\\{8895b1c6-b41f-4c1c-a562-0d564250836f}" );
				cbData = sizeof( wcData);
				// found the key, now find the GUID and match it with friendly name
				if( ERROR_SUCCESS == RegQueryValue(HKEY_CLASSES_ROOT,wcName,wcData, &cbData ) )
				{
					mExtVsCLSID[csExt] = wcData;
					break;
				}
				else {
					if( ERROR_SUCCESS == RegQueryValue(HKEY_CLASSES_ROOT,csExt,wcData, &cbData ) ) {
						lstrcpy(wcName, wcData);
					}
				}
			}
		}
		else
		{
			break;
		}
	}
    CString ownSupportedFormats(_T(".bat;.pdf;.cmd;.txt"));
	for (POSITION pos = mExtVsCLSID.GetStartPosition(); pos != NULL;) {
		CString ext;
		CString clsid;
		mExtVsCLSID.GetNextAssoc(pos, ext, clsid);
        ext += _T(";");
        if (ownSupportedFormats.Find(ext) < 0)
            mPattern += ext;
	}
	mPattern = WildCardExpToRegExp(mPattern);
}
void CExplorerPreviewManager::Unload()
{
	mPattern.Empty();
	mExtVsCLSID.RemoveAll();
	if (m_pCurPreviewHandler)
		delete m_pCurPreviewHandler;
	m_pCurPreviewHandler = NULL;
}
CExplorerPreviewManager::CExpPreviewHandler* CExplorerPreviewManager::GetPreviewHandler(const CString &ext)
{
	if (m_pCurPreviewHandler)
		delete m_pCurPreviewHandler;
	m_pCurPreviewHandler = NULL;
	CMapStringToString::CPair *pairExt(mExtVsCLSID.PLookup(ext));
	if (pairExt) {
		m_pCurPreviewHandler = new CExpPreviewHandler(pairExt->value);
	}
	return m_pCurPreviewHandler;
}
CExplorerPreviewManager::CExpPreviewHandler::CExpPreviewHandler(const CString& clsid)
	: m_pIP(NULL), m_pIFile(NULL), m_pIStream(NULL), m_pStream(NULL)
{
	HRESULT hr = E_FAIL;
	CLSID cls;

	hr = CLSIDFromString((LPWSTR)(LPCTSTR)clsid,&cls);
	if( S_OK == CoCreateInstance(cls,NULL,CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER,IID_IPreviewHandler,(LPVOID*)&m_pIP) ) {
		m_pIP->QueryInterface(IID_IInitializeWithFile,(LPVOID*)&m_pIFile );
		m_pIP->QueryInterface(IID_IInitializeWithStream,(LPVOID*)&m_pIStream );
	}
}
#define SAFERELEASE( X ) { if( X ) { X->Release(); X = NULL; } }
CExplorerPreviewManager::CExpPreviewHandler::~CExpPreviewHandler()
{
	StopPreview();
}
HRESULT CExplorerPreviewManager::CExpPreviewHandler::LoadStream(LPCTSTR file /* = NULL */)
{
	HRESULT hr = E_FAIL;
	if (m_pIP == NULL)
		return hr;
	if (file) {
		if( m_pIFile )
		{
			hr = m_pIFile->Initialize( file,STGM_READ);
		}
		else if( m_pIStream )
		{
			HANDLE hFile = CreateFile(file,FILE_READ_DATA,FILE_SHARE_READ,NULL,OPEN_EXISTING,0,NULL );
			if( INVALID_HANDLE_VALUE != hFile )
			{
				DWORD dwSize = GetFileSize( hFile,NULL );
				HGLOBAL hGlobal= GlobalAlloc(GPTR, dwSize );
				BYTE * pByte = (BYTE *)GlobalLock(hGlobal);

				if( pByte )
				{
					ReadFile(hFile,pByte,dwSize,&dwSize,NULL);	
					GlobalUnlock(hGlobal);
					CreateStreamOnHGlobal(hGlobal, TRUE, &m_pStream);	
					hr = m_pIStream->Initialize( m_pStream,STGM_READ);
				}
				CloseHandle( hFile );
			}
		}
	}
	return hr;
}
BOOL CExplorerPreviewManager::CExpPreviewHandler::ShowPreview(const CString &path)
{
	BOOL bSuccess(SUCCEEDED(LoadStream(path)));
	if (bSuccess) {
		CWnd *previewWnd(CExplorerPreviewManager::GetInstance().GetPreviewWindow());
		CRect rc;
		previewWnd->GetClientRect(rc);
		m_pIP->SetWindow(previewWnd->GetSafeHwnd(), &rc);
		HRESULT hr = m_pIP->DoPreview();
		bSuccess = SUCCEEDED(hr);
	}
	return bSuccess;
}
void CExplorerPreviewManager::CExpPreviewHandler::StopPreview()
{
	SAFERELEASE(m_pStream)
	SAFERELEASE( m_pIStream )
	SAFERELEASE( m_pIFile )
	if (m_pIP)
		m_pIP->Unload();
	SAFERELEASE(m_pIP);
}
void CExplorerPreviewManager::CExpPreviewHandler::DoResize()
{
	if (m_pIP) {
		CWnd *previewWnd(CExplorerPreviewManager::GetInstance().GetPreviewWindow());
		CRect rc;
		previewWnd->GetClientRect(rc);
		m_pIP->SetRect(&rc);
	}
}