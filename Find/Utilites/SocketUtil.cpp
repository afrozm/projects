#include "StdAfx.h"
#include "SocketUitl.h"
#include "SystemUtils.h"
#include <netlistmgr.h>

#pragma comment(lib, "Ws2_32.lib")

class AutoWSAStratup {
public:
	static void Init() {
		static AutoWSAStratup sAutoWSAStratup;
	}
private:
	AutoWSAStratup() {
		WSAStartup(MAKEWORD(2, 2), &wsadata);
	}
	~AutoWSAStratup() {
		WSACleanup();
	}
	WSADATA wsadata;
};

namespace SocketUtil {
DWORD GetHostName(LPTSTR outHostName, DWORD len)
{
	AutoWSAStratup::Init();
	char hostN[NI_MAXHOST];
	DWORD lenR(0);
	if( gethostname ( hostN, sizeof(hostN)) == 0) {
		lenR = (DWORD)strlen(hostN)+1;
		if (outHostName != NULL && len >= lenR)
			lstrcpy(outHostName, SystemUtils::UTF8ToUnicode(hostN).c_str());
	}
	return lenR;
}
CString GetHostName()
{
	TCHAR hostName[NI_MAXHOST];
	GetHostName(hostName, NI_MAXHOST);
	return hostName;
}
DWORD GetHostNameFromIp(LPCTSTR ipAddr, LPTSTR outHostName, DWORD len, int family)
{
	AutoWSAStratup::Init();
	struct sockaddr_in saGNI = {0};
	struct sockaddr_in6 saGNI6 = {0};
	SOCKADDR *pSock(family==AF_INET ? (SOCKADDR *) &saGNI : (SOCKADDR *)&saGNI6);
	socklen_t sockLen(family==AF_INET ? sizeof(saGNI) : sizeof(saGNI6));
	pSock->sa_family = (ADDRESS_FAMILY)family;
	InetPton(AF_INET6, ipAddr, &saGNI6.sin6_addr);
	InetPton(AF_INET, ipAddr, &saGNI.sin_addr);

	//-----------------------------------------
	// Call GetNameInfoW
	WCHAR servInfo[NI_MAXSERV];
	DWORD dwRetval = GetNameInfoW(pSock, sockLen, outHostName, len, servInfo, NI_MAXSERV, NI_NOFQDN | NI_NAMEREQD);
	if (dwRetval != 0)
		dwRetval = WSAGetLastError();
	return dwRetval;
}
DWORD  GetIPs(LPCTSTR name, VecWString &outIPs, int family)
{
	AutoWSAStratup::Init();
	//-----------------------------------------
	// Declare and initialize variables
	DWORD dwRetval(0);

	ADDRINFOW *result = NULL;
	ADDRINFOW *ptr = NULL;
	ADDRINFOW hints;

	//    struct sockaddr_in6 *sockaddr_ipv6;
	LPSOCKADDR sockaddr_ip;

	wchar_t ipstringbuffer[46];
	DWORD ipbufferlength = 46;

	// Validate the parameters


	//--------------------------------
	// Setup the hints address info structure
	// which is passed to the getaddrinfo() function
	ZeroMemory( &hints, sizeof(hints) );
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	//--------------------------------
	// Call GetAddrinfoW(). If the call succeeds,
	// the result variable will hold a linked list
	// of addrinfow structures containing response
	// information
	dwRetval = GetAddrInfoW(name, NULL, &hints, &result);
	if ( dwRetval != 0 ) {
		dwRetval = WSAGetLastError();
		return dwRetval;
	}

	wprintf(L"GetAddrInfoW returned success\n");

	// Retrieve each address and print out the hex bytes
	for(ptr=result; ptr != NULL ;ptr=ptr->ai_next) {
		if (family != ptr->ai_family)
			continue;
		switch (ptr->ai_family) {
		case AF_UNSPEC:
			break;
		case AF_INET:
			sockaddr_ip = (LPSOCKADDR) ptr->ai_addr;
			// The buffer length is changed by each call to WSAAddresstoString
			// So we need to set it for each iteration through the loop for safety
			ipbufferlength = 46;
			dwRetval = WSAAddressToString(sockaddr_ip, (DWORD) ptr->ai_addrlen, NULL, 
				ipstringbuffer, &ipbufferlength );
			if (dwRetval == 0)
				outIPs.push_back(ipstringbuffer);
			break;
		case AF_INET6:
			// the InetNtop function is available on Windows Vista and later
			// sockaddr_ipv6 = (struct sockaddr_in6 *) ptr->ai_addr;
			// printf("\tIPv6 address %s\n",
			//    InetNtop(AF_INET6, &sockaddr_ipv6->sin6_addr, ipstringbuffer, 46) );

			// We use WSAAddressToString since it is supported on Windows XP and later
			sockaddr_ip = (LPSOCKADDR) ptr->ai_addr;
			// The buffer length is changed by each call to WSAAddresstoString
			// So we need to set it for each iteration through the loop for safety
			ipbufferlength = 46;
			dwRetval = WSAAddressToString(sockaddr_ip, (DWORD) ptr->ai_addrlen, NULL, 
				ipstringbuffer, &ipbufferlength );
			if (dwRetval == 0)
				outIPs.push_back(ipstringbuffer);
			break;
		default:
			wprintf(L"Other %ld\n", ptr->ai_family);
			break;
		}
	}

	FreeAddrInfoW(result);
	if (dwRetval != 0)
		dwRetval = WSAGetLastError();

	return dwRetval;
}
CString GetIpV4StrFromAddr(DWORD ipv4Addr)
{
	AutoWSAStratup::Init();
	wchar_t ipstringbuffer[46];
	DWORD ipbufferlength = ARRAYSIZE(ipstringbuffer);
	struct sockaddr_in saGNI = {0};
	saGNI.sin_family = AF_INET;
	saGNI.sin_addr.S_un.S_addr = ToggleEndian<DWORD>(ipv4Addr);
	DWORD dwRetval = WSAAddressToString((LPSOCKADDR)&saGNI, sizeof(sockaddr_in), NULL, 
		ipstringbuffer, &ipbufferlength );
	CString ipStr;
	if (dwRetval == 0)
		ipStr = ipstringbuffer;
	return ipStr;
}
DWORD GetIpV4AddrFromString(LPCTSTR ipV4Str)
{
	AutoWSAStratup::Init();
	struct sockaddr_in saGNI = {0};
	if (ipV4Str && *ipV4Str)
		InetPton(AF_INET, ipV4Str, &saGNI.sin_addr);
	return ToggleEndian<DWORD>(saGNI.sin_addr.S_un.S_addr);
}
BOOL IsConnectedToLAN()
{
	CoInitialize(NULL);
	INetworkListManager *pNetworkListManager = NULL; 
	HRESULT hr = CoCreateInstance(CLSID_NetworkListManager, NULL,
		CLSCTX_ALL, IID_INetworkListManager,
		(LPVOID *)&pNetworkListManager);
	VARIANT_BOOL isConnected = VARIANT_TRUE;
	if (pNetworkListManager != NULL) {
		hr = pNetworkListManager->get_IsConnected(&isConnected);
		pNetworkListManager->Release();
	}
	CoUninitialize();
	return isConnected == VARIANT_TRUE;
}
BOOL IsConnectedToInternet()
{
	CoInitialize(NULL);
	INetworkListManager *pNetworkListManager = NULL; 
	HRESULT hr = CoCreateInstance(CLSID_NetworkListManager, NULL,
		CLSCTX_ALL, IID_INetworkListManager,
		(LPVOID *)&pNetworkListManager);
	VARIANT_BOOL isConnected = VARIANT_FALSE;
	if (pNetworkListManager != NULL) {
		hr = pNetworkListManager->get_IsConnectedToInternet(&isConnected);
		pNetworkListManager->Release();
	}
	CoUninitialize();
	return isConnected == VARIANT_TRUE;
}
}