// DialogPreviewError.cpp : implementation file
//

#include "stdafx.h"
#include "Find.h"
#include "DialogPreviewError.h"
#include "Path.h"

// CDialogPreviewError dialog

IMPLEMENT_DYNAMIC(CDialogPreviewError, CDialogPreviewBase)

CDialogPreviewError::CDialogPreviewError(CWnd* pParent /*=NULL*/)
	: CDialogPreviewBase(CDialogPreviewError::IDD, pParent)
{

}

CDialogPreviewError::~CDialogPreviewError()
{
}

void CDialogPreviewError::DoDataExchange(CDataExchange* pDX)
{
	CDialogPreviewBase::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDialogPreviewError, CDialogPreviewBase)
END_MESSAGE_MAP()


// CDialogPreviewError message handlers
BOOL CDialogPreviewError::ShowPreview(const CString &path)
{
    CString msg(_T("Preview not available for file "));
    msg += Path(path).FileName();
	SetDlgItemText(IDC_STATIC_MESSAGE, msg);
	return TRUE;
}

