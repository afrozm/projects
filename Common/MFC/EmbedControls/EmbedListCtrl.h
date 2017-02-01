#pragma once

#include "ListCtrlUtil.h"

// CEmbedListCtrl
// CEmbedListCtrl::EnableControl - uEnable flags
#define ECF_ENABLE FLAGBIT(0)
#define ECF_DONOT_COMMIT FLAGBIT(1)
#define ECF_ENABLE_NEXT FLAGBIT(2)
#define ECF_ENABLE_PREV FLAGBIT(3)

#define WM_ENABLE_EMBED_CONTROL WM_USER+310
#define LVN_COLUMNCLICK_ON_CHECK_BOX (LVN_LAST-1)

// Check box state to remove it
#define BST_REMOVE 3
#define BST_ADD FLAGBIT(30)

class CEmbedListCtrl;

typedef int (*EmbedControlCallBackFn)(CEmbedListCtrl *pList, int message, WPARAM wParam, LPARAM lParam, LPVOID pUserData);

enum EmbedControlType {
	ECT_NO_Control,
	ECT_Edit,
	ECT_ComboBox,
	ECT_DropDownComboBox,
	ECT_Slider,
	ECT_Spin,
	ECT_Progress,
	ECT_HotKey,
	ECT_DateTime,
	ECT_Calendar,
	ECT_IPAddress,
};

enum EmbedControlMessage {
	EMT_AddControl,
	EMT_InitControl,
	EMT_RemoveControl
};

struct EmbedControlInfo {
	EmbedControlInfo()
		: mControl(NULL), row(-1), col(-1), mType(ECT_NO_Control)
	{
	}
	CWnd *mControl;
	EmbedControlType mType;
	int row;
	int col;
};

class CEmbedListCtrl : public CListCtrlUtil
{
	DECLARE_DYNAMIC(CEmbedListCtrl)

public:
	CEmbedListCtrl();
	virtual ~CEmbedListCtrl();
	void SetEmbedControlCallBackFn(EmbedControlCallBackFn embedControlCallBackFn, LPVOID pUserData);
	BOOL PreTranslateMessage(MSG* pMsg);
	bool CheckRowColCheckBox(int iCol, int iRow = -1, int iChecked = BST_CHECKED);
	bool IsChecked(int iCol, int iRow = -1);
	bool HasCheckBox(int iCol, int iRow);
	BOOL GetItemRect(int nItem, int iCol, LPRECT lpRect, UINT nCode = LVIR_BOUNDS);
	bool EnableControl(int row, int col, unsigned uEnable = ECF_ENABLE);
	int AddIcon(HICON hIcon);
	BOOL SetIcon(int iconIndex = -1, int row = -1, int col = -1);
	int GetIcon(int row, int col);
	int SetFontSize(int fontSize);
	bool HeaderHasCheckBox(int iCol);
protected:
	bool AddCheckBox();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg LRESULT OnEnableControl(WPARAM wParam, LPARAM lParam);
	afx_msg void OnLButtonDown(UINT, CPoint);
	bool OnDeleteEmbedControl(unsigned int iIndex, unsigned uCommit = 0);
	void InitHeaderControl();
	DECLARE_MESSAGE_MAP()
	EmbedControlCallBackFn mEmbedControlCallBackFn;
	LPVOID m_pUserData;
	CArray<EmbedControlInfo> mEmbedControlsArray;
	CImageList mImageList;
	int		mCheckBoxIndex;
	CWnd *m_pHeaderControl;
	int m_iConSize;
	CFont mFont;
};


