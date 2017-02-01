// Preview\DialogPreviewText.cpp : implementation file
//

#include "stdafx.h"
#include "Find.h"
#include "DialogPreviewText.h"
#include "ThreadManager.h"
#include "AutoLock.h"
#include <Windows.h>
#include "PreviewController.h"

// CDialogPreviewText dialog

IMPLEMENT_DYNAMIC(CDialogPreviewText, CDialogPreviewBase)

CDialogPreviewText::CDialogPreviewText(CWnd* pParent /*=NULL*/)
	: CDialogPreviewBase(CDialogPreviewText::IDD, pParent), mResizer(this)
{
	mResizer.AddControl(IDC_EDIT_TEXT);
	mEditCtrl.SubclassDlgItem(IDC_EDIT_TEXT, this);
}

CDialogPreviewText::~CDialogPreviewText()
{
}

void CDialogPreviewText::DoDataExchange(CDataExchange* pDX)
{
	CDialogPreviewBase::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDialogPreviewText, CDialogPreviewBase)
	ON_WM_SIZE()
	ON_MESSAGE(WM_PBM_DATA_CHACNGED, OnDataChanged)
END_MESSAGE_MAP()


// CDialogPreviewText message handlers
#define FILE_LOADER_THREAD_ID 7896
static int LoadTextFile_ThreadProcFn(LPVOID pThreadData)
{
	return ((CDialogPreviewText*)pThreadData)->LoadFile();
}
bool CDialogPreviewText::UpdateText()
{
	CString curText;
	GetDlgItemText(IDC_EDIT_TEXT, curText);
	curText += mText;
	mText.Empty();
	const int maxSize(100*1024*1024); // 100MB limit
	int lenExceeding(curText.GetLength() - maxSize);
	if (lenExceeding > 0)
		curText = curText.Right(maxSize);
	lenExceeding = curText.GetLength();
	SetDlgItemText(IDC_EDIT_TEXT, curText);
	return true;
}
int CDialogPreviewText::LoadFile()
{
	DWORD tc = GetTickCount();
    const bool& bIsTerminated(ThreadManager::GetInstance().GetIsTerminatedFlag());
	while (!bIsTerminated) {
		CAutoLock ca(mLock);
		CString line = mTextReader.Read().c_str();
		if (line.IsEmpty())
			break;
		mText += line;
		DWORD nowTc = GetTickCount();
		if (!bIsTerminated && ((nowTc - tc) > 250)) {
			tc = nowTc;
			UpdateText();
		}
	}
	if (!bIsTerminated) {
		UpdateText();
		OnDataChanged(0, 0);
	}
	return 0;
}
BOOL CDialogPreviewText::ShowPreview(const CString &path)
{
	BOOL bSuccess = FALSE;
	{
		CAutoLock ca(mLock);
		mText.Empty();
		SetDlgItemText(IDC_EDIT_TEXT, mText);
		bSuccess = mTextReader.SetFile(path);
	}
	if (bSuccess) {
		ThreadManager &tm(ThreadManager::GetInstance());
		if ((int)tm.GetThread(FILE_LOADER_THREAD_ID) <= 0)
			bSuccess = tm.CreateThread(LoadTextFile_ThreadProcFn, this, FILE_LOADER_THREAD_ID) ? TRUE : FALSE;
	}
	return bSuccess;
}
void CDialogPreviewText::StopPreview()
{
	ThreadManager &tm(ThreadManager::GetInstance());
	DWORD threadID(tm.GetThread(FILE_LOADER_THREAD_ID));
	if ((int)threadID > 0)
		tm.TerminateThread(threadID);
	ShowPreview(_T(""));
}
void CDialogPreviewText::OnSize(UINT nType, int cx, int cy)
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

LRESULT CDialogPreviewText::OnDataChanged( WPARAM, LPARAM )
{
	CPreviewExtraDataFileContentSearch *pExtraData = (CPreviewExtraDataFileContentSearch *)GetPreviewContoller()->GetExtraData();
	if (pExtraData != NULL) {
		int nFirstLine(mEditCtrl.GetFirstVisibleLine());
		nFirstLine = (int)pExtraData->nLineToScroll-5 - nFirstLine;
		mEditCtrl.LineScroll(nFirstLine);
		nFirstLine = (int)pExtraData->nLineToScroll-1;
		int lineIndex = mEditCtrl.LineIndex(nFirstLine);
		if (lineIndex < 0)
			return 0;
		CString strText;
		{
			TCHAR lineText[1024];
			int nLineLenght = mEditCtrl.GetLine(nFirstLine, lineText, sizeof(lineText)/sizeof(lineText[0]));
			strText.SetString(lineText, nLineLenght); // null terminate
		}
		int pos = strText.FindOneOf(_T("\r\n"));
		if (pos > 0)
			strText = strText.Left(pos);
		mEditCtrl.SetSel(lineIndex, lineIndex+strText.GetLength());
	}
	return 0;
}
