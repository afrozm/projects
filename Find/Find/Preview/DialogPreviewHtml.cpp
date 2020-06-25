// Preview\DialogPreviewHtml.cpp : implementation file
//

#include "stdafx.h"
#include "Find.h"
#include "DialogPreviewHtml.h"


// CDialogPreviewHtml dialog

IMPLEMENT_DYNAMIC(CDialogPreviewHtml, CDialogPreviewBase)

CDialogPreviewHtml::CDialogPreviewHtml(CWnd* pParent /*=NULL*/)
	: CDialogPreviewBase(CDialogPreviewHtml::IDD, pParent), mControlResizer(this), m_HtmlDialog(this)
{
	OnInitDialog();
}

CDialogPreviewHtml::~CDialogPreviewHtml()
{
}

void CDialogPreviewHtml::DoDataExchange(CDataExchange* pDX)
{
	CDialogPreviewBase::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDialogPreviewHtml, CDialogPreviewBase)
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CDialogPreviewHtml message handlers
BOOL CDialogPreviewHtml::ShowPreview(const CString &path)
{
	return m_HtmlDialog.ShowPreview(path);
}
BOOL CDialogPreviewHtml::OnInitDialog()
{
	BOOL bRet(__super::OnInitDialog());
	m_HtmlDialog.SetDlgCtrlID(CDialogHtmlPreviewBase::IDD);
	mControlResizer.AddControl(CDialogHtmlPreviewBase::IDD);
	//m_HtmlDialog.ShowWindow(SW_SHOW);
	return bRet;
}
void CDialogPreviewHtml::OnSize(UINT nType, int cx, int cy)
{
	__super::OnSize(nType, cx, cy);
	switch (nType) {
	case SIZE_MINIMIZED:
		break;
	default:
		mControlResizer.DoReSize();
		break;
	}
}