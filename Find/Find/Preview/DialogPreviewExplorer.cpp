// Preview\DialogPreviewExplorer.cpp : implementation file
//

#include "stdafx.h"
#include "Find.h"
#include "DialogPreviewExplorer.h"

// CDialogPreviewExplorer dialog

IMPLEMENT_DYNAMIC(CDialogPreviewExplorer, CDialogPreviewBase)

CDialogPreviewExplorer::CDialogPreviewExplorer(CWnd* pParent /*=NULL*/)
	: CDialogPreviewBase(CDialogPreviewExplorer::IDD, pParent), m_pPreviewExp(NULL)
{
	CExplorerPreviewManager::GetInstance().SetPreviewWindow(this);
}

CDialogPreviewExplorer::~CDialogPreviewExplorer()
{
}

void CDialogPreviewExplorer::DoDataExchange(CDataExchange* pDX)
{
	CDialogPreviewBase::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDialogPreviewExplorer, CDialogPreviewBase)
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CDialogPreviewExplorer message handlers
BOOL CDialogPreviewExplorer::ShowPreview(const CString &path)
{
	//LRESULT lResult = theApp.GetMainWnd()->SendMessage(WM_LOAD_PREVIEW, (WPARAM)this, (LPARAM)(LPCTSTR)path);
	//BOOL bRet(lResult != NULL);
	//return bRet;
	return Load(path);
}
bool CDialogPreviewExplorer::Load(LPCTSTR path)
{
	CExplorerPreviewManager &expMgr(CExplorerPreviewManager::GetInstance());
	Path filePath(path);
	m_pPreviewExp = expMgr.GetPreviewHandler(filePath.GetExtension());
	BOOL bSuccess(FALSE);
	if (m_pPreviewExp != NULL)
		bSuccess = m_pPreviewExp->ShowPreview(filePath);
	return bSuccess == TRUE;
}
void CDialogPreviewExplorer::StopPreview()
{
	if (m_pPreviewExp)
		m_pPreviewExp->StopPreview();
}
void CDialogPreviewExplorer::OnSize(UINT nType, int cx, int cy)
{
	CDialogPreviewBase::OnSize(nType, cx, cy);
	switch (nType) {
	case SIZE_MINIMIZED:
		break;
	default:
		theApp.GetMainWnd()->SendMessage(WM_RESIZE_PREVIEW);
		break;
	}
}
void CDialogPreviewExplorer::DoResize()
{
	if (m_pPreviewExp)
		m_pPreviewExp->DoResize();
}