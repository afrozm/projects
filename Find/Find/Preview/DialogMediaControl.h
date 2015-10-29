#pragma once


// CDialogMediaControl dialog
#include "Drawer.h"
#include "VLCWrapperImpl.h"
#include "ControlResizer.h"

class CDialogPreviewMedia;

class CDialogMediaControl : public CDrawer
{
	DECLARE_DYNAMIC(CDialogMediaControl)

public:
	CDialogMediaControl(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDialogMediaControl();
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnBnClickedButtonMediaPlayPause();
	afx_msg void OnStnClickedStaticMediaTime();
	void SetVLCPlayer(VLCWrapper *player)
	{
		m_VLCPlayer = player;
	}
	bool GetManualPositioning() const {return m_bManualPositioning;}
	void UpdatePosition();
	void ResetConrols();
	void StopControls();
	void SetFinished()
	{
		m_bPaused = false;
		OnBnClickedButtonMediaPlayPause();
		m_bFinished = true;
	}
// Dialog Data
	enum { IDD = IDD_DIALOG_MEDIA_CONTROL };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSizing(UINT nSize, LPRECT lpRect);
	void OnTimerShowHideControls();
	void UpdateTimeControls();
	DECLARE_MESSAGE_MAP()

	VLCWrapper		*m_VLCPlayer;
	bool			m_bManualPositioning;
	__int64			mTotalTime;
	bool			m_bShowTimeInReverse;
	bool			m_bPaused;
	bool			m_bFinished;
	CString			mStrTotalTime;
	CPoint			mMouseHoverPoint;
	int				m_iTimeOutCount;
	CControlResizer mControlResizer;
	DWORD			mLastUpdateTime;
	CDialogPreviewMedia *m_pDialogPreviewMedia;
	bool			m_bFullScreen;
	CRect			m_WindowSize;
	CToolTipCtrl	m_ToolTip;
public:
	afx_msg void OnBnClickedButtonMediaFullscreen();
};
