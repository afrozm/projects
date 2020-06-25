// Preview\DialogHtmlPreviewBase.cpp : implementation file
//

#include "stdafx.h"
#include "Find.h"
#include "DialogHtmlPreviewBase.h"
#include "Path.h"

// CDialogHtmlPreviewBase dialog

IMPLEMENT_DYNCREATE(CDialogHtmlPreviewBase, CDHtmlDialog)

CDialogHtmlPreviewBase::CDialogHtmlPreviewBase(CWnd* pParent /*=NULL*/)
	: CDHtmlDialog(CDialogHtmlPreviewBase::IDD, CDialogHtmlPreviewBase::IDH, pParent)
{
	Create(CDialogHtmlPreviewBase::IDD, pParent);
	// Resize as per parent
	RECT pcr;
	pParent->GetClientRect(&pcr);
	SetWindowPos(NULL, 0, 0, pcr.right-pcr.left, pcr.bottom-pcr.top, SWP_NOZORDER|SWP_NOMOVE);
}

CDialogHtmlPreviewBase::~CDialogHtmlPreviewBase()
{
}

void CDialogHtmlPreviewBase::DoDataExchange(CDataExchange* pDX)
{
	CDHtmlDialog::DoDataExchange(pDX);
}

BOOL CDialogHtmlPreviewBase::OnInitDialog()
{
	SetHostFlags(DOCHOSTUIFLAG_FLAT_SCROLLBAR|DOCHOSTUIFLAG_NO3DOUTERBORDER);
	CDHtmlDialog::OnInitDialog();
	return TRUE;  // return TRUE  unless you set the focus to a control
}

BEGIN_MESSAGE_MAP(CDialogHtmlPreviewBase, CDHtmlDialog)
END_MESSAGE_MAP()

BEGIN_DHTML_EVENT_MAP(CDialogHtmlPreviewBase)
END_DHTML_EVENT_MAP()



// CDialogHtmlPreviewBase message handlers

BOOL CDialogHtmlPreviewBase::ShowPreview(const CString &path, bool pathIsURL)
{
	Path urlPath(path);
	if (!pathIsURL)
		urlPath = urlPath.GetURL();
	Navigate(urlPath, navNoHistory);
	return TRUE;
}
