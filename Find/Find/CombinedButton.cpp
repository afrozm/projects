#include "StdAfx.h"
#include "CombinedButton.h"


CCombinedButton::CCombinedButton(CDialog *pParent /* = NULL */, int prevID /* = 0 */, int nextID /* = 0 */)
: m_pParent(pParent), mPrevID(prevID), mNextID(nextID), mCurrentIndex(-1), mPrevIndex(-1), muFlags(0)
{
}


CCombinedButton::~CCombinedButton(void)
{
}
void CCombinedButton::Init()
{
	mCurrentIndex = -1;
	OnIndexChange();
	mPrevIndex = -1;
	mArrayContext.RemoveAll();
}
INT_PTR CCombinedButton::AddPage(CCombinedButtonContext *context)
{
	if (context == NULL)
		return -1;
	INT_PTR index(mArrayContext.AddUnique(context));
	if (mCurrentIndex < 0)
		mCurrentIndex = index;
	OnIndexChange();
	return index;
}
INT_PTR CCombinedButton::RemovePage(CCombinedButtonContext *context)
{
	if (context == NULL)
		return -1;
	INT_PTR index((INT_PTR)context);
	if (index >= mArrayContext.GetCount())
		index = mArrayContext.Find(context);
	if (index >= 0 && index < mArrayContext.GetCount()) {
		mArrayContext[index]->ShowWindow(SW_HIDE);
		mArrayContext.RemoveAt(index);
	}
	if (mCurrentIndex < 0 || mCurrentIndex >= mArrayContext.GetCount()) {
		mCurrentIndex = mArrayContext.GetCount()-1;
	}
	OnIndexChange();
	return index;
}
void CCombinedButton::OnButtonClickEvent(int buttonID)
{
	if (buttonID == mPrevID)
		--mCurrentIndex;
	else if (buttonID == mNextID)
		++mCurrentIndex;
	OnIndexChange();
}
void CCombinedButton::OnIndexChange()
{
	if (mPrevIndex != mCurrentIndex) {
		ShowIndex(mPrevIndex, SW_HIDE);
		ShowIndex(mCurrentIndex, mCurrentIndex >= 0 ? SW_SHOW : SW_HIDE);
		mPrevIndex = mCurrentIndex;
	}
	bool bNextEnable(mCurrentIndex < mArrayContext.GetCount()-1);
	bool bPrevEnable(mCurrentIndex > 0);
	EnableControl(NextButton, bNextEnable);
	EnableControl(PrevButton, bPrevEnable);
}
void CCombinedButton::EnableControl(CombinedControl control /* = PrevButton */, bool bEnable /* = true */)
{
	int bID(control == PrevButton ? mPrevID : mNextID);
	CWnd *button(m_pParent->GetDlgItem(bID));
	if (IsFlagSet(DisableControls)) {
		button->EnableWindow(bEnable);
	}
	else {
		button->ShowWindow(bEnable ? SW_SHOW : SW_HIDE);
	}
}
void CCombinedButton::ShowIndex(INT_PTR index, int cmdShow /* = SW_SHOW */)
{
	if (index >= 0  && index < mArrayContext.GetCount()) {
		CCombinedButtonContext *pContext(mArrayContext[index]);
		BOOL isVisible(pContext->IsWindowVisible());
		if (cmdShow == SW_SHOW) {
			if (!isVisible)
				pContext->ShowWindow();
		}
		else if (isVisible)
			pContext->ShowWindow(SW_HIDE);
	}
}