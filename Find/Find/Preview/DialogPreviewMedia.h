#pragma once

#include "DialogPreviewBase.h"
#include "VLCWrapperImpl.h"
#include "ControlResizer.h"
#include "DialogMediaControl.h"
// CDialogPreviewMedia dialog

class CDialogPreviewMedia : public CDialogPreviewBase
{
	DECLARE_DYNAMIC(CDialogPreviewMedia)

public:
	CDialogPreviewMedia(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDialogPreviewMedia();
	BOOL ShowPreview(const CString &path);
	void StopPreview();
	virtual BOOL OnInitDialog();
	HBRUSH OnControlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBnClickedButtonLocate();
	afx_msg void OnNMClickSyslinkGetVLC(NMHDR *pNMHDR, LRESULT *pResult);

	void UpdatePosition();
	void MediaEndReached();
// Dialog Data
	enum { IDD = IDD_DIALOG_PREVIEW_MEDIA };


protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg LRESULT OnSetMediaTime(WPARAM wParam, LPARAM lParam);
	void SetMessage(LPCTSTR message = NULL, ...);
	bool VLC_InitLib(LPCTSTR vlcPath = NULL);
	DECLARE_MESSAGE_MAP()

	VLCWrapper		*m_VLCPlayer;
	int64_t			m_iLengthToSkip;
	int64_t			m_iLastPlayedLength;
	CBrush			m_BackGroundBrush;
	CControlResizer mControlResizer;
	CDialogMediaControl mDialogMediaControl;
};
