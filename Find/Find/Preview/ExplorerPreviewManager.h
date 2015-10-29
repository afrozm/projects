#pragma once

#include "Path.h"

class CExplorerPreviewManager
{
	CExplorerPreviewManager(void);
public:
	static CExplorerPreviewManager& GetInstance();
	~CExplorerPreviewManager(void);
	void LoadSupportedExts();
	void SetPreviewWindow(CWnd *pWnd) {m_pPreviewWindow=pWnd;}
	CWnd* GetPreviewWindow() const {return m_pPreviewWindow;};
	const CString& GetPattern() const {return mPattern;}
	class CExpPreviewHandler {
	private:
		IPreviewHandler *m_pIP ;
		IInitializeWithFile *m_pIFile ;
		IInitializeWithStream *m_pIStream ;
		IStream *m_pStream;
		HRESULT LoadStream(LPCTSTR file = NULL);
	public:
		CExpPreviewHandler(const CString& clsid);
		~CExpPreviewHandler();
		BOOL ShowPreview(const CString &path);
		void StopPreview();
		void DoResize();
	};
private:
	CString mPattern;
	CMapStringToString mExtVsCLSID;
	CExpPreviewHandler *m_pCurPreviewHandler;
	CWnd *m_pPreviewWindow;
	void Unload();
public:
	CExpPreviewHandler* GetPreviewHandler(const CString &ext);
};

