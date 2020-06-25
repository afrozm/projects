// Preview\DialogPreviewZIP.cpp : implementation file
//

#include "stdafx.h"
#include "Find.h"
#include "DialogPreviewZIP.h"
#include "unzip/Unzipper.h"
#include "Path.h"

// CDialogPreviewZIP dialog

IMPLEMENT_DYNAMIC(CDialogPreviewZIP, CDialogPreviewBase)

CDialogPreviewZIP::CDialogPreviewZIP(CWnd* pParent /*=NULL*/)
	: CDialogPreviewBase(CDialogPreviewZIP::IDD, pParent), mResizer(this)
{
	mResizer.AddControl(IDC_TREE_DOMAIN);
    mTreeCtrl.SubclassDlgItem(IDC_TREE_DOMAIN, this);
    mTreeCtrl.SetSearchInZip();
    mTreeCtrl.SetFileListing();
}

CDialogPreviewZIP::~CDialogPreviewZIP()
{
}

void CDialogPreviewZIP::DoDataExchange(CDataExchange* pDX)
{
	CDialogPreviewBase::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDialogPreviewZIP, CDialogPreviewBase)
	ON_WM_SIZE()
    ON_NOTIFY(TVN_ITEMEXPANDING, IDC_TREE_DOMAIN, &CDialogPreviewZIP::OnTvnItemexpandingTreeDomain)
END_MESSAGE_MAP()

void CDialogPreviewZIP::OnTvnItemexpandingTreeDomain(NMHDR *pNMHDR, LRESULT *pResult)
{
    mTreeCtrl.OnTvnItemexpanding(pNMHDR, pResult);
}


BOOL CDialogPreviewZIP::ShowPreview(const CString &path)
{
    mTreeCtrl.DeleteAllTreeItem();
	BOOL bSuccess = FALSE;
    {
        CUnzipper unzipper(path);
        bSuccess = unzipper.IsOpen();
    }
    if (bSuccess) {
        Path zipFilePath(path);
        mTreeCtrl.SetRootPath(CString(zipFilePath.Parent()));
        mTreeCtrl.Expand(mTreeCtrl.InsertNewItem(NULL, zipFilePath.FileName(), 1), TVE_EXPAND, false);
    }
	return bSuccess;
}
void CDialogPreviewZIP::OnSize(UINT nType, int cx, int cy)
{
	CDialogPreviewBase::OnSize(nType, cx, cy);
	switch (nType) {
	case SIZE_MINIMIZED:
		break;
	default:
		mResizer.DoReSize();
		break;
	}
}

