// DialogPreviewPDF.cpp : implementation file
//

#include "stdafx.h"
#include "Find.h"
#include "DialogPreviewPDF.h"


// CDialogPreviewPDF dialog

IMPLEMENT_DYNAMIC(CDialogPreviewPDF, CDialogPreviewBase)

CDialogPreviewPDF::CDialogPreviewPDF(CWnd* pParent /*=NULL*/)
	: CDialogPreviewBase(CDialogPreviewPDF::IDD, pParent)
{
	OnInitDialog();
}

CDialogPreviewPDF::~CDialogPreviewPDF()
{
}

void CDialogPreviewPDF::DoDataExchange(CDataExchange* pDX)
{
	CDialogPreviewBase::DoDataExchange(pDX);
}
BOOL CDialogPreviewPDF::OnInitDialog()
{
	CDialogPreviewBase::OnInitDialog();

	RECT clientRect;
	GetClientRect(&clientRect);
	BOOL bCreated = mPDFCtrl.Create(_T("PDFPreview"), WS_CHILD | WS_VISIBLE, clientRect, this, 3001);
	if (!bCreated) {
		GetDlgItem(IDC_SYSLINK_GETREADER)->ShowWindow(SW_SHOW);
	}
	return TRUE;
}

BEGIN_MESSAGE_MAP(CDialogPreviewPDF, CDialogPreviewBase)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_GETREADER, &CDialogPreviewPDF::OnNMClickSyslinkGetreader)
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CDialogPreviewPDF message handlers
bool CDialogPreviewPDF::Load(LPCTSTR path)
{
	return mPDFCtrl.LoadFile(path) == TRUE;
}
BOOL CDialogPreviewPDF::ShowPreview(const CString &path)
{
	if (mPDFCtrl.m_hWnd == NULL)
		return TRUE;
	LRESULT lResult = theApp.GetMainWnd()->SendMessage(WM_LOAD_PREVIEW, (WPARAM)this, (LPARAM)(LPCTSTR)path);
	BOOL bRet(lResult != NULL);
	mPDFCtrl.setShowToolbar(FALSE);
	mPDFCtrl.setShowScrollbars(FALSE);
	return bRet;
}

void CDialogPreviewPDF::OnNMClickSyslinkGetreader(NMHDR *pNMHDR, LRESULT *pResult)
{
    UNREFERENCED_PARAMETER(pNMHDR);
	ShellExecute(NULL, _T("open"), _T("http://get.adobe.com/reader/"), NULL, NULL, SW_SHOWDEFAULT);
	*pResult = 0;
}
void CDialogPreviewPDF::OnSize(UINT nType, int cx, int cy)
{
	CDialogPreviewBase::OnSize(nType, cx, cy);
	switch (nType) {
	case SIZE_MINIMIZED:
		break;
	default:
		{
			RECT rc;
			GetClientRect(&rc);
			mPDFCtrl.MoveWindow(&rc);
		}
		break;
	}
}
