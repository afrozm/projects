#pragma once

#include "FindDataBase.h"
#include "SaveListResultCtrl.h"

#define CRTCF_FORCE FLAGBIT(0)
#define CRTCF_REMOVE FLAGBIT(1)

class CCommitResultTimer
{
public:
	CCommitResultTimer();
	~CCommitResultTimer(void);
	bool DoCommit(unsigned int uFlags = 0);
	bool Load();
	int ItrRecentSearchTableRowsCallbackFn(sqlite3_stmt *statement, void *pUserData);
	void SetListCtrl(CSaveListResultCtrl *pListCtrl) {mListCtrl = pListCtrl;}
private:
	CSaveListResultCtrl *mListCtrl;
	int miListIndexSaved;
	DWORD mLastSavedTime;
	bool mbInCommit;
};
