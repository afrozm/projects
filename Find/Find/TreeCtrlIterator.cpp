#include "StdAfx.h"
#include "TreeCtrlIterator.h"
#include <queue>
#include <vector>
#include <list>

CTreeCtrlIterator::CTreeCtrlIterator(CTreeCtrl *pTree, TreeIteratorCallBack tiCB, void *pUserParam)
: m_pTreeCtrl(pTree), mTreeIteratorCallBack(tiCB), m_pUserParam(pUserParam), m_bAbort(false)
{
}

CTreeCtrlIterator::~CTreeCtrlIterator(void)
{
}

void CTreeCtrlIterator::StartIteration()
{
    if (m_pTreeCtrl->GetSafeHwnd())
	    StartIterationEx(m_pTreeCtrl->GetRootItem());
}

void CTreeCtrlIterator::StartIterationEx(HTREEITEM hItem)
{
	if (hItem == NULL || m_bAbort)
		return;
	TreeIteratorCallBackData ticbData = {m_pTreeCtrl, hItem, hItem};
	std::queue <HTREEITEM> q;
	q.push(hItem);
	while (!q.empty()&& !m_bAbort) {
		hItem = q.front();
		q.pop();
		while (hItem && !m_bAbort) {
			ticbData.hItem = hItem;
			int retVal = mTreeIteratorCallBack(&ticbData, m_pUserParam);
			m_bAbort = m_bAbort || (retVal & TICB_ABORT);
			if (!(retVal & TICB_DONTITERATE_CHILREN))
				q.push(m_pTreeCtrl->GetChildItem(hItem));
			if (!(retVal & TICB_DONTITERATE_SIBLING))
				hItem = m_pTreeCtrl->GetNextSiblingItem(hItem);
			else hItem = NULL;
		}
	}
}

void CTreeCtrlIterator::SetTreeIteratorCallBack(TreeIteratorCallBack ticb)
{
	mTreeIteratorCallBack = ticb;
}