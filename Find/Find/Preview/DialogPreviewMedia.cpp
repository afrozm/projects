// DialogPreviewMedia.cpp : implementation file
//

#include "stdafx.h"
#include "Find.h"
#include "DialogPreviewMedia.h"
#include "SystemUtils.h"
#include "Path.h"
#include "FindDataBase.h"

// CDialogPreviewMedia dialog
const int64_t VLC_TRAILER_TIME = 30*1000; // 30 sec
const int64_t VLC_SCENE_TIME = 5*1000; // 5 seconds each scene
const int64_t VLC_NUM_SCENE = (VLC_TRAILER_TIME)/(VLC_SCENE_TIME);

IMPLEMENT_DYNAMIC(CDialogPreviewMedia, CDialogPreviewBase)

CDialogPreviewMedia::CDialogPreviewMedia(CWnd* pParent /*=NULL*/)
	: CDialogPreviewBase(CDialogPreviewMedia::IDD, pParent),
	m_iLastPlayedLength(0), m_VLCPlayer(NULL), m_iLengthToSkip(0),
	mControlResizer(this), mDialogMediaControl(this)
{
	OnInitDialog();
}

CDialogPreviewMedia::~CDialogPreviewMedia()
{
	if (m_VLCPlayer != NULL)
		delete m_VLCPlayer;
}

void CDialogPreviewMedia::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

#define WM_SET_MEDIA_TIME WM_USER+0x789A

BEGIN_MESSAGE_MAP(CDialogPreviewMedia, CDialogPreviewBase)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BUTTON_LOCATE, &CDialogPreviewMedia::OnBnClickedButtonLocate)
	ON_WM_SIZE()
	ON_MESSAGE(WM_SET_MEDIA_TIME, &CDialogPreviewMedia::OnSetMediaTime)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK_GET_VLC, &CDialogPreviewMedia::OnNMClickSyslinkGetVLC)
	ON_NOTIFY(NM_RETURN, IDC_SYSLINK_GET_VLC, &CDialogPreviewMedia::OnNMClickSyslinkGetVLC)
END_MESSAGE_MAP()
static CString GetVLCNotFoundErrorMsg() {
	LPCTSTR bits[] = {_T("32"), _T("64")};
	LPCTSTR bit = bits[SystemUtils::IsModule64()];
	CString error;
	error.Format(_T("VLC Media player not installed\nor VLC player %s-bit version equal or higher than 2.0 not found.\n \n \nCannot generate preview"), bit);
	return error;
}
// CDialogPreviewMedia message handlers
BOOL CDialogPreviewMedia::ShowPreview(const CString &path)
{
	m_iLastPlayedLength = m_iLengthToSkip = 0;
	if (m_VLCPlayer != NULL) {
		mDialogMediaControl.ResetConrols();
		SetMessage(_T("Buffering ") + Path(path).FileName());
		m_VLCPlayer->OpenMedia(SystemUtils::UnicodeToUTF8(path).c_str());
		m_VLCPlayer->Play();
	}
	else {
		SetMessage(GetVLCNotFoundErrorMsg() + _T(" for ") + Path(path).FileName());
	}
	return TRUE;
}

static void HandleVLCEvents(const VLCEvent* pEvt, void* pUserData)
{
	CDialogPreviewMedia* pDlg=reinterpret_cast<CDialogPreviewMedia*>(pUserData); 

	switch(pEvt->type)
	{
	case libvlc_MediaPlayerTimeChanged:
		TRACE(_T("VLC_EVT_TIME_CHANGED: new_time %lld[ms]\n"), pEvt->u.media_player_time_changed.new_time);
		pDlg->UpdatePosition();
		break;
	case libvlc_MediaPlayerEndReached:
		TRACE(_T("VLC_EVT_END_REACHED:\n"));
		pDlg->MediaEndReached();
		break;
	}
}


BOOL CDialogPreviewMedia::OnInitDialog()
{
	BOOL bRet(CDialogPreviewBase::OnInitDialog());

	mControlResizer.AddControl(IDC_STATIC_MESSAGE);
	mControlResizer.AddControl(IDC_BUTTON_LOCATE, RSZF_SIZE_FIXED|RSZF_RIGHT_FIXED | RSZF_BOTTOM_FIXED);
	mControlResizer.AddControl(IDC_SYSLINK_GET_VLC, RSZF_SIZE_FIXED|RSZF_BOTTOM_FIXED);
	if (VLC_InitLib()) {
		mDialogMediaControl.Init(IDD_DIALOG_MEDIA_CONTROL, this);
		mDialogMediaControl.SetVLCPlayer(m_VLCPlayer);
		SetMessage();
	}
	else  { // VLC not installed
		SetMessage(GetVLCNotFoundErrorMsg()+_T("."));
	}
	return bRet;
}

void CDialogPreviewMedia::UpdatePosition()
{
	//if (m_VLCPlayer->IsUpdateLocked())
	//	return;
	//CAutoLock autoLock(m_VLCPlayer->GetUpdateMutex());
	int64_t llNewPosition=m_VLCPlayer->GetTime();
	if (m_iLengthToSkip == 0) {
		m_iLengthToSkip = (m_VLCPlayer->GetLength()-VLC_TRAILER_TIME)/VLC_NUM_SCENE;
		if (m_VLCPlayer->HasVOut() == 0) {
			SetMessage(_T("Playing ") + Path(GetFileToPreview()).FileName());
		}
		else SetMessage();
	}
	mDialogMediaControl.UpdatePosition();
	if (!mDialogMediaControl.GetManualPositioning()) {
		if ((llNewPosition-m_iLastPlayedLength) > VLC_SCENE_TIME) {
			llNewPosition += m_iLengthToSkip;
			m_iLastPlayedLength = llNewPosition;
			PostMessage(WM_SET_MEDIA_TIME);
		}
	}
}
void CDialogPreviewMedia::MediaEndReached()
{
	mDialogMediaControl.SetFinished();
}
void CDialogPreviewMedia::StopPreview()
{
	mDialogMediaControl.ResetConrols();
	mDialogMediaControl.StopControls();
	if (mDialogMediaControl.IsOpen())
		mDialogMediaControl.Close();
	if (m_VLCPlayer != NULL)
		m_VLCPlayer->Stop();
}

HBRUSH CDialogPreviewMedia::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr =  CDialogPreviewBase::OnCtlColor(pDC, pWnd, nCtlColor);
	if (m_iLengthToSkip && nCtlColor == CTLCOLOR_DLG
		&& !GetDlgItem(IDC_STATIC_MESSAGE)->IsWindowVisible()) {
		if ((HBRUSH)m_BackGroundBrush == NULL) {
			m_BackGroundBrush.CreateSolidBrush(RGB(0,0,0)); // black
		}
		hbr = (HBRUSH)m_BackGroundBrush;
	}
	return hbr;
}
HBRUSH CDialogPreviewMedia::OnControlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	return OnCtlColor(pDC, pWnd, nCtlColor);
}
void CDialogPreviewMedia::SetMessage(LPCTSTR message, ...)
{
	if (message == NULL || *message == 0) {
		GetDlgItem(IDC_STATIC_MESSAGE)->ShowWindow(SW_HIDE);
		SetDlgItemText(IDC_STATIC_MESSAGE, _T(""));
	}
	else {
		va_list args;
		// retrieve the variable arguments
		va_start( args, message );
		CString str;
		str.FormatV(message, args);
		va_end(args);
		SetDlgItemText(IDC_STATIC_MESSAGE, str);
		GetDlgItem(IDC_STATIC_MESSAGE)->ShowWindow(SW_SHOW);
	}
}
void CDialogPreviewMedia::OnBnClickedButtonLocate()
{
	CFileDialog cf(TRUE, NULL, NULL, 4|2, _T("Excutable Files (*.exe)|*.exe|All Files (*.*)|*.*||"), this);
	if (cf.DoModal() == IDOK) {
		Path pathName = cf.GetPathName();
		pathName = pathName.Parent();
		if (VLC_InitLib(pathName)) {
			FindDataBase::SSetProperty(_T("VLCPath"), pathName);
			ShowPreview(GetFileToPreview());
		}
	}
}

bool CDialogPreviewMedia::VLC_InitLib(LPCTSTR vlcPath)
{
	CString vlcPathSaved;
	if (vlcPath == NULL) {
		vlcPathSaved = FindDataBase::SGetProperty(_T("VLCPath"));
		if (!vlcPathSaved.IsEmpty())
			vlcPath = vlcPathSaved;
	}
	bool bInit = ::VLC_InitLib(vlcPath);
	if (!bInit && vlcPath != NULL) {
		bInit = ::VLC_InitLib();
		if (bInit)
			FindDataBase::SRemoveProperty(_T("VLCPath"));
	}
	if (bInit) {
		if (m_VLCPlayer == NULL) {
			m_VLCPlayer = new VLCWrapper();
			m_VLCPlayer->SetOutputWindow((void*)GetSafeHwnd());
			m_VLCPlayer->SetEventHandler(&HandleVLCEvents, libvlc_MediaPlayerTimeChanged, this);
			m_VLCPlayer->SetEventHandler(&HandleVLCEvents, libvlc_MediaPlayerEndReached, this);
		}
		GetDlgItem(IDC_BUTTON_LOCATE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_SYSLINK_GET_VLC)->ShowWindow(SW_HIDE);
	}
	else {
		GetDlgItem(IDC_BUTTON_LOCATE)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_SYSLINK_GET_VLC)->ShowWindow(SW_SHOW);
	}
	return bInit;
}
void CDialogPreviewMedia::OnSize(UINT nType, int cx, int cy)
{
	CDialogPreviewBase::OnSize(nType, cx, cy);
	switch (nType) {
	case SIZE_MINIMIZED:
		break;
	default:
		mControlResizer.DoReSize();
		InvalidateRect(NULL, FALSE);
		break;
	}
}
LRESULT CDialogPreviewMedia::OnSetMediaTime(WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);
	m_VLCPlayer->SetTime(m_iLastPlayedLength);
	return 0;
}
void CDialogPreviewMedia::OnNMClickSyslinkGetVLC(NMHDR *pNMHDR, LRESULT *pResult)
{
	 PNMLINK pNMLink = (PNMLINK)pNMHDR;
	ShellExecute(NULL, _T("open"), pNMLink->item.szUrl, NULL, NULL, SW_SHOWDEFAULT);
	*pResult = 0;
}
