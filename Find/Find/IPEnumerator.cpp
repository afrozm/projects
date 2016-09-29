#include "StdAfx.h"
#include "IPEnumerator.h"
#include "SocketUitl.h"
#include "STLUtils.h"

static int Def_IPEnumerator_Callback(IPEnumCBData *ipData, void *pUSerData)
{
    UNREFERENCED_PARAMETER(ipData);
    UNREFERENCED_PARAMETER(pUSerData);
	return 0;
}

IPEnumerator::IPEnumerator(IPEnumerator_Callback callbackFn /* = NULL */, void *pUSerData /* = NULL */)
	: mCallback(callbackFn), mnCount(0), m_pUserData(pUSerData), mExitCode(0)
{
	SetCallBack(callbackFn, pUSerData);
}

void IPEnumerator::SetCallBack(IPEnumerator_Callback callbackFn, void *pUSerData /* = NULL */)
{
	mCallback = callbackFn ? callbackFn : Def_IPEnumerator_Callback;
	m_pUserData = pUSerData;
}

unsigned IPEnumerator::Enumerate(DWORD startIP, DWORD endIp)
{
	if (startIP > endIp)
        STLUtils::Swap(startIP, endIp);
	while (startIP < endIp) {
		if (FindHostName(startIP++))
			break;
	}
	return mnCount;
}

int IPEnumerator::FindHostName(DWORD ipV4)
{
	ipV4 = SocketUtil::ToggleEndian<DWORD>(ipV4);
	TCHAR ip[NI_MAXHOST];
	InetNtop(AF_INET, &ipV4, ip, NI_MAXHOST);
	TCHAR hostName[NI_MAXHOST];
	hostName[0] = 0;
	DWORD retVal(SocketUtil::GetHostNameFromIp(ip, hostName, NI_MAXHOST));
	if (retVal == ERROR_SUCCESS) {
		mnCount++;
	}
	IPEnumCBData ipData = {ip, hostName};
	mExitCode = mCallback(&ipData, m_pUserData);
	return mExitCode;
}