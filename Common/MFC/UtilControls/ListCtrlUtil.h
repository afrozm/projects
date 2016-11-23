#pragma once


// CListCtrlUtil

class CListCtrlUtil : public CListCtrl
{
	DECLARE_DYNAMIC(CListCtrlUtil)

public:
	CListCtrlUtil();
	virtual ~CListCtrlUtil();
	int GetColumnCount() { return GetHeaderCtrl()->GetItemCount(); }
	void AdjustColumnWidth();
	void SetAdjustCol(int iCol) {m_iColToAdjust=iCol;}
	int GetAdjustCol() const {return m_iColToAdjust;}
	BOOL EnsureColVisible(int col);
	virtual bool DisableNotification(bool bDisable = true);
	bool IsNotificationDisabled() const { return mbNotificationDisabled; }
	void SelectAllItems();
	void InvertSelection();
	void RemoveAllSelection();
	BOOL SetItemTextIfModified(int nItem, int nSubItem,  LPCTSTR lpszText);
	int GetItemIndex(LPVOID pItemData, int startIndex = 0, bool bIncr = true, bool bMatch = true);
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnSize(UINT nType, int cx, int cy);
	void OnKeyDown(UINT nChar, UINT nReptCnt, UINT nFlags);
private:
	bool mbNotificationDisabled;
	int m_iColToAdjust;
};


class CAutoDisableNotificaltion {
	CListCtrlUtil *m_pList;
public:
	CAutoDisableNotificaltion(CListCtrlUtil *pList)
		: m_pList(pList)
	{
		m_pList->DisableNotification(true);
	}
	~CAutoDisableNotificaltion()
	{
		m_pList->DisableNotification(false);
	}
};
