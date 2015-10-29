#include "StdAfx.h"
#include "PreviewController.h"
#include "AutoLock.h"

CPreviewController::CPreviewController()
	: m_pDialogPreview(NULL), m_dwThreadControlID((DWORD)-1), mAction(CPreviewController::CA_None), m_pExtraData(NULL), mbUpdateData(false)
{
}


CPreviewController::~CPreviewController(void)
{
	if ((int)m_dwThreadControlID > 0) {
		ThreadManager::GetInstance().TerminateThread(m_dwThreadControlID);
		ThreadManager::GetInstance().WaitForThread(m_dwThreadControlID);
	}
	m_dwThreadControlID = (DWORD)-1;
	if (m_pDialogPreview != NULL) {
		delete m_pDialogPreview;
		m_pDialogPreview = NULL;
	}
	SetExtraData();
}
static int ThreadProcFn_ProcessPreviewRequests(LPVOID pThreadData)
{
	CPreviewController *pPreviewController((CPreviewController *)pThreadData);
	return pPreviewController->PreviewControlHandler();
}
CDialogPreview* CPreviewController::GetPreviewDialog(CDialog *pParentDialog)
{
	if (m_pDialogPreview == NULL) {
		m_pDialogPreview = new CDialogPreview(pParentDialog);
		m_pDialogPreview->SetPreviewController(this);
		ThreadManager::GetInstance().CreateThread(ThreadProcFn_ProcessPreviewRequests, this, 596, &m_dwThreadControlID);
	}
	return m_pDialogPreview;
}
int CPreviewController::PreviewControlHandler()
{
	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	while (!ThreadManager::GetInstance().IsThreadTerminated(m_dwThreadControlID)) {
		ControllerAction ca(GetAction());
		switch (ca) {
		case CA_ShowPreview:
		{
			CString fileToPreview(GetFileToPreview());
			if (!fileToPreview.IsEmpty() && IsPreviewVisible()) {
				if (!m_pDialogPreview->ShowPreview(fileToPreview))
					if (mbUpdateData) {
						mbUpdateData = false;
						PerformAction(CA_DataChanged);
					}
			}
		}
		break;
		case CA_Terminate:
		case CA_HidePreview:
			if (IsPreviewVisible())
				m_pDialogPreview->ShowWindow(SW_HIDE);
		if (ca == CA_HidePreview)
			break;
		ThreadManager::GetInstance().TerminateThread(m_dwThreadControlID);
		break;
		case CA_ReSize:
			if (IsPreviewVisible())
				m_pDialogPreview->GetCurrentPreview()->DoResize();
			break;
		case CA_DataChanged:
			m_pDialogPreview->GetCurrentPreview()->PostMessage(WM_PBM_DATA_CHACNGED);
			break;
		default:
			Sleep(50);
		}
	}
	CoUninitialize();
	m_dwThreadControlID = (DWORD)-1;
	return 0;
}
BOOL CPreviewController::ShowPreview(const CString &fileToPreview)
{
	{
		CAutoLock autoLock(mLockFile);
		mFileToPreview = fileToPreview;
	}
	return PerformAction(CA_ShowPreview);
}
BOOL CPreviewController::HidePreview()
{
	return PerformAction(CA_HidePreview);
}
BOOL CPreviewController::IsPreviewVisible()  const
{
	return m_pDialogPreview != NULL && m_pDialogPreview->IsWindowVisible();
}
CString CPreviewController::GetFileToPreview()
{
	CString fileToPreview;
	{
		CAutoLock autoLock(mLockFile);
		fileToPreview = mFileToPreview;
		mFileToPreview.Empty();
	}
	return fileToPreview;
}
BOOL CPreviewController::PerformAction(ControllerAction ca)
{
	{
		CAutoLock autoLock(mLockFile);
		mAction = ca;
	}
	return TRUE;
}
CPreviewController::ControllerAction CPreviewController::GetAction()
{
	CAutoLock autoLock(mLockFile);
	ControllerAction action(mAction);
	mAction = CA_None;
	return action;
}
BOOL CPreviewController::IsPreviewTerminated()
{
	if (m_dwThreadControlID > 0)
		return ThreadManager::GetInstance().WaitForThread(m_dwThreadControlID, 10) != WAIT_TIMEOUT;
	return TRUE;
}

void CPreviewController::SetExtraData( CRefCountObj *pExtraData /*= NULL*/ )
{
	mbUpdateData = pExtraData != m_pExtraData;
	m_pExtraData->DecrementRefCount(&m_pExtraData);
	m_pExtraData = pExtraData;
}
