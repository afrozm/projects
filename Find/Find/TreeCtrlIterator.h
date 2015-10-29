#pragma once

#define TICB_CONTINUE 0
#define TICB_DONTITERATE_CHILREN 1
#define TICB_DONTITERATE_SIBLING 2
#define TICB_ABORT 4

typedef struct {
	CTreeCtrl *pTree;
	HTREEITEM hItem;
	HTREEITEM hStartItem;
} TreeIteratorCallBackData;

// Return true to abort
typedef int (*TreeIteratorCallBack)(TreeIteratorCallBackData*, void *pUserParam);

class CTreeCtrlIterator
{
	CTreeCtrl *m_pTreeCtrl;
	TreeIteratorCallBack mTreeIteratorCallBack;
	void *m_pUserParam;
	bool m_bAbort;
public:
	CTreeCtrlIterator(CTreeCtrl *pTree, TreeIteratorCallBack tiCB, void *pUserParam = NULL);
	void StartIterationEx(HTREEITEM hItem);
	void StartIteration();
	void SetTreeIteratorCallBack(TreeIteratorCallBack);
	~CTreeCtrlIterator(void);
	bool IsAborted() {return m_bAbort;}
};
