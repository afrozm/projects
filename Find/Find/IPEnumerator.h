#pragma once

struct IPEnumCBData {
	LPCTSTR ip;
	LPCTSTR hostname;
};
typedef int (*IPEnumerator_Callback)(IPEnumCBData *ipData, void *pUSerData);

class IPEnumerator {
public:
	IPEnumerator(IPEnumerator_Callback callbackFn = NULL, void *pUSerData = NULL);
	void SetCallBack(IPEnumerator_Callback callbackFn, void *pUSerData = NULL);
	unsigned Enumerate(DWORD startIP, DWORD endIp);
	int GetExitCode() const {return mExitCode;}
private:
	int FindHostName(DWORD ipV4);
	IPEnumerator_Callback mCallback;
	void* m_pUserData;
	unsigned mnCount;
	int mExitCode;
};