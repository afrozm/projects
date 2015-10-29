// DialogPreviewBase.cpp : implementation file
//

#include "stdafx.h"
#include "Find.h"
#include "DialogPreviewBase.h"
#include "DialogPreview.h"


// CDialogPreviewBase dialog

IMPLEMENT_DYNAMIC(CDialogPreviewBase, CDialog)

CDialogPreviewBase::CDialogPreviewBase(UINT nIDTemplate, CWnd* pParent /*=NULL*/)
	: CDialog(nIDTemplate, pParent)
{
	Create(nIDTemplate, pParent);
}
void CDialogPreviewBase::Init()
{
	// Resize as per parent
	RECT pcr;
	GetParent()->GetClientRect(&pcr);
	SetWindowPos(NULL, 0, 0, pcr.right-pcr.left, pcr.bottom-pcr.top, SWP_NOZORDER|SWP_NOMOVE);

}

CDialogPreviewBase::~CDialogPreviewBase()
{
}

void CDialogPreviewBase::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDialogPreviewBase, CDialog)
END_MESSAGE_MAP()


// CDialogPreviewBase message handlers
HBRUSH CDialogPreviewBase::OnControlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	return CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
}
const CString& CDialogPreviewBase::GetFileToPreview() const
{
	CDialogPreview *pParentDialog((CDialogPreview *)m_pParentWnd);

	return pParentDialog->GetFileToPreview();
}

CPreviewController* CDialogPreviewBase::GetPreviewContoller() const
{
	if (m_pParentWnd != NULL)
		return ((CDialogPreview *)m_pParentWnd)->GetPreviewController();
	return NULL;
}
