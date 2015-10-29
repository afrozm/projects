#pragma once


class CCombinedButtonContext {
public:
	virtual BOOL IsWindowVisible() = 0;
	virtual void ShowWindow(int cmdShow = SW_SHOW) = 0;
};

class CCombinedButton
{
public:
	CCombinedButton(CDialog *pParent = NULL, int prevID = 0, int nextID = 0);
	~CCombinedButton(void);
	void Init();
	INT_PTR AddPage(CCombinedButtonContext *context);
	INT_PTR RemovePage(CCombinedButtonContext *context);
	void OnButtonClickEvent(int buttonID);
	enum CBFlags {
		DisableControls = FLAGBIT(0) // when set, prev & next controls are disabled otherwise they are hidden if they are unavailable
	};
	void SetFlags(CBFlags flags, bool bSet = true) { SET_UNSET_FLAG(muFlags, flags, bSet); }
	bool IsFlagSet(CBFlags flag) const { return IS_FLAG_SET(muFlags, flag); }
	enum CombinedControl {
		PrevButton,
		NextButton
	};
private:
	void EnableControl(CombinedControl control = PrevButton, bool bEnable = true);
	void ShowIndex(INT_PTR index, int cmdShow = SW_SHOW);
	void OnIndexChange();
	int mPrevID;
	int mNextID;
	INT_PTR mCurrentIndex;
	INT_PTR mPrevIndex;
	UINT muFlags;
	CDialog *m_pParent;
	CArrayEx<CCombinedButtonContext*> mArrayContext;
};

