// DialogPreview.cpp : implementation file
//

#include "stdafx.h"
#include "Find.h"
#include "DialogPreview.h"
#include "Path.h"
#include "DialogPreviewImages.h"
#include "DialogPreviewMedia.h"
#include "DialogPreviewPDF.h"
#include "DialogPreviewError.h"
#include "ThreadManager.h"
#include "DialogPreviewExplorer.h"
#include "DialogPreviewText.h"
#include "DialogPreviewHtml.h"
#include "DialogPreviewZIP.h"

#define WM_SHOW_PREVIEW WM_USER+0x700

CPreviewContainer::CPreviewContainer()
:m_pPreviewDialog(NULL)
{
}
CPreviewContainer::CPreviewContainer(const CString &matchPattern)
: m_pPreviewDialog(NULL), mStringMatcher(matchPattern, true)
{
}
CPreviewContainer::~CPreviewContainer()
{
	if (m_pPreviewDialog != NULL)
		delete m_pPreviewDialog;
	m_pPreviewDialog = NULL;
}
bool CPreviewContainer::CanShowPreview(const CString &fileName)
{
	bool bMatched = mStringMatcher.Match(fileName);
	return bMatched;
}
CDialogPreviewBase* CPreviewContainer::GetPreviewDialogObject(CDialogPreview *pDialogPreview, int iPreviewID)
{
	if (m_pPreviewDialog == NULL)
        m_pPreviewDialog = (CDialogPreviewBase*)theApp.GetMainWnd()->SendMessage(WM_CREATE_PREVIEW, (WPARAM)pDialogPreview, iPreviewID);
	return m_pPreviewDialog;
}
// CDialogPreview dialog

IMPLEMENT_DYNAMIC(CDialogPreview, CDialog)


enum PreviewID { // *** Order is important ****
	PreviewIDImages,
	PreviewIDMedia,
	PreviewIDHTML,
    PreviewIDPDF,
    PreviewIDExplorer,
    PreviewZIP,
    PreviewIDText,
    PreviewIDError
};
static LPCTSTR GetPatternImages() {
	return _T("(.*\\.jpg)|(.*\\.jpe)|(.*\\.bmp)|(.*\\.dib)|(.*\\.gif)|(.*\\.tif)|(.*\\.png)|(.*\\.ico)");
}
static LPCTSTR GetPatternMedia() {
	return _T("(.*\\.m)|(.*\\.div)|(.*\\.avi)|(.*\\.vob)|(.*\\.vlc)|(.*\\.wmv)|(.*\\.flv)|(.*\\.3gp)");
}
static LPCTSTR GetPatternExplorer() {
	return CExplorerPreviewManager::GetInstance().GetPattern();
}
static LPCTSTR GetPatternHTML() {
	return _T("(.*\\.htm)");
}
static LPCTSTR GetPatternText() {
	return _T(".*"); //_T("(.*\\.txt)|(.*\\.log)|(.*\\.as)|(.*\\.mx)|(.*\\.sh)|(.*\\.bat)|(.*\\.c)|(.*\\.h)|(.*\\.xml)|(.*\\.xsd)|(.*\\.in)|(.*\\.reg)|(.*\\.java)|(.*\\.js)|(.*\\.mak)|(.*\\.pl)|(.*\\.py)|(.*\\.rc)|(.*\\.xsl)");
}
static LPCTSTR GetPatternPDF() {
	return _T("(.*\\.pdf)");
}
static LPCTSTR GetPatternZIP() {
    return _T("(.*\\.zip)|(.*\\.tar)|(.*\\.iso)|(.*\\.gz)|(.*\\.7z)|(.*\\.rar)|(.*\\.war)|(.*\\.jar)|(.*\\.ar)");
}
static LPCTSTR GetPatternError() {
	return _T(".*");
}
typedef LPCTSTR (*GetPattern)();

CDialogPreview::CDialogPreview(CWnd* pParent /*=NULL*/)
	: CDialog(CDialogPreview::IDD, pParent), m_pCurrentPreview(NULL),
	mControlResizer(this), m_iChildControlID(0)
{
	Create(CDialogPreview::IDD, pParent);
	GetPattern patterns[] = { // *** Order is important ****
		// images
		GetPatternImages,
		// movies
		GetPatternMedia,
		// HTML files
		GetPatternHTML,
        // pdf
        GetPatternPDF,
        // Explorer
		GetPatternExplorer,
        // ZIP files
        GetPatternZIP,
        // Text files
        GetPatternText,
        // any other - no preview
		GetPatternError
	};
	for (int i = 0; i < sizeof(patterns)/sizeof(patterns[0]); ++i)
		m_CArrayCPreviewContainer.Add(new CPreviewContainer(patterns[i]()));
}

CDialogPreview::~CDialogPreview()
{
	INT_PTR nPreviewCount(m_CArrayCPreviewContainer.GetCount());
	for (INT_PTR i = 0; i < nPreviewCount; ++i) {
		CPreviewContainer *previewContainer(m_CArrayCPreviewContainer.GetAt(i));
		delete previewContainer;
	}
}

void CDialogPreview::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDialogPreview, CDialog)
	ON_WM_SIZE()
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// CDialogPreview message handlers
BOOL CDialogPreview::PreTranslateMessage(MSG* pMsg)
{
	switch (pMsg->message) {
	case WM_KEYDOWN:
		if (pMsg->wParam == VK_SPACE)
			GetParent()->PostMessage(WM_TOGGLE_PREVIEW);
		break;
	}
	return CDialog::PreTranslateMessage(pMsg);
}

static int ThreadProcFn_CheckFileExistence(LPVOID pThreadData)
{
	CDialogPreview *pDlg((CDialogPreview *)pThreadData);
	bool bExists(Path(pDlg->GetFileToPreview()).Exists());
	if (!ThreadManager::GetInstance().IsThreadTerminated())
		pDlg->SendMessage(WM_SHOW_PREVIEW, bExists);
	return 0;
}

bool CDialogPreview::ShowPreview(const CString &path)
{
	if (m_FileToPreview == path)
		return false;
	ThreadManager::GetInstance().TerminateThreads(TM_PREVIEW_THREAD_CLASS);
	m_FileToPreview = path;
	CDialogPreviewBase *pPreviewDialog(NULL);
	CString extName(Path(path).GetExtension());
	INT_PTR nPreviewCount(m_CArrayCPreviewContainer.GetCount());
	for (INT_PTR i = 0; i < nPreviewCount; ++i) {
		CPreviewContainer &previewContainer(*m_CArrayCPreviewContainer.GetAt(i));
		if (previewContainer.CanShowPreview(extName)) {
			pPreviewDialog = previewContainer.GetPreviewDialogObject(this, (int)i);
			break;
		}
	}
	if (PathIsUNC(path)) {
		HideCurrentPreview(pPreviewDialog);
		ThreadManager::GetInstance().CreateThread(ThreadProcFn_CheckFileExistence, this, TM_PREVIEW_THREAD_CLASS);
	}
	else {
		ShowCurrentPreview(pPreviewDialog);
	}
	return true;
}
void CDialogPreview::ShowDialogControls(int iCmdShow)
{
	int ids[] = {IDC_PROGRESS_LOADING,IDC_STATIC_MESSAGE};
	for (int  i = 0; i < sizeof(ids)/sizeof(ids[0]); ++i)
		GetDlgItem(ids[i])->ShowWindow(iCmdShow);
}
void CDialogPreview::HideCurrentPreview(CDialogPreviewBase *pNewPreview)
{
	CString windowText;
	windowText.LoadString(IDS_STRING_PREVIEW);
	if (m_pCurrentPreview != NULL) {
		m_pCurrentPreview->ShowWindow(SW_HIDE);
		m_pCurrentPreview->StopPreview();
	}
	if (m_pCurrentPreview != pNewPreview) {
		m_pCurrentPreview = pNewPreview;
	}
	if (m_pCurrentPreview) {
		ShowDialogControls();
		((CProgressCtrl*)GetDlgItem(IDC_PROGRESS_LOADING))->SetMarquee(TRUE, 50);
		CString fileName(Path(m_FileToPreview).FileName());
		SetDlgItemText(IDC_STATIC_MESSAGE, _T("Loading ") + fileName);
		windowText += _T(" - ") + fileName;
	}
	SetWindowText(windowText);
}
void CDialogPreview::ShowCurrentPreview(CDialogPreviewBase *pNewPreview)
{
	if (pNewPreview != NULL)
		HideCurrentPreview(pNewPreview);
	if (m_pCurrentPreview != NULL) {
		if (!m_pCurrentPreview->ShowPreview(m_FileToPreview))
			ShowCurrentPreview(GetDialogPreviews(PreviewIDError));
		if (!m_pCurrentPreview->IsWindowVisible()) {
			ShowDialogControls(SW_HIDE);
			m_pCurrentPreview->ShowWindow(SW_SHOW);
		}
	}
}
void CDialogPreview::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	switch (nType) {
	case SIZE_MINIMIZED:
		break;
	default:
		mControlResizer.DoReSize();
		if (m_pCurrentPreview)
			m_pCurrentPreview->InvalidateRect(NULL, TRUE);
		break;
	}
}

CDialogPreviewBase* CDialogPreview::GetDialogPreviewsEx(int iPreviewID)
{
	CDialogPreviewBase *pDialogPreview(NULL);
	switch (iPreviewID) {
	case PreviewIDImages:
		pDialogPreview = new CDialogPreviewImages(this);
		break;
	case PreviewIDMedia:
		pDialogPreview = new CDialogPreviewMedia(this);
		break;
	case PreviewIDExplorer:
		pDialogPreview = new CDialogPreviewExplorer(this);
		break;
	case PreviewIDHTML:
		pDialogPreview = new CDialogPreviewHtml(this);
		break;
	case PreviewIDText:
		pDialogPreview = new CDialogPreviewText(this);
		break;
	case PreviewIDPDF:
		pDialogPreview = new CDialogPreviewPDF(this);
		break;
    case PreviewZIP:
        pDialogPreview = new CDialogPreviewZIP(this);
        break;
    case PreviewIDError:
	default:
		pDialogPreview = new CDialogPreviewError(this);
		break;
	}
	if (pDialogPreview != NULL) {
		pDialogPreview->Init();
		pDialogPreview->SetDlgCtrlID(++m_iChildControlID);
		mControlResizer.AddControl(m_iChildControlID);
	}
	return pDialogPreview;
}
CDialogPreviewBase* CDialogPreview::GetDialogPreviews(int iPreviewID)
{
    return m_CArrayCPreviewContainer.GetAt(iPreviewID)->GetPreviewDialogObject(this, iPreviewID);
}
BOOL CDialogPreview::ShowWindow(int nCmdShow)
{
	DWORD dwFlags = AW_CENTER;
	if (nCmdShow == SW_HIDE) {
		HideCurrentPreview(NULL);
		m_FileToPreview.Empty();
		dwFlags |= AW_HIDE;
	}
	else dwFlags |= AW_ACTIVATE;
	return AnimateWindow(200, dwFlags);
	//return CDialog::ShowWindow(nCmdShow);
}
HBRUSH CDialogPreview::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	if (m_pCurrentPreview != NULL)
		return m_pCurrentPreview->OnControlColor(pDC, pWnd, nCtlColor);
	return CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
}
LRESULT CDialogPreview::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
	case WM_SHOW_PREVIEW:
		if (wParam) { // success ful loaded
			ShowCurrentPreview();
		}
		else {
			ShowCurrentPreview(GetDialogPreviews(PreviewIDError));
		}
		break;
	}
	return CDialog::WindowProc(message, wParam, lParam);
}
void CDialogPreview::OnCancel()
{
	ShowWindow(SW_HIDE);
	CDialog::OnCancel();
}
