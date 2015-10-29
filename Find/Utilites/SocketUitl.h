#pragma once

#include <winsock2.h>
#include <Ws2tcpip.h>
#include <string>
#include <vector>

namespace SocketUtil {
	typedef std::vector<std::wstring> VecWString;
	DWORD GetHostName(LPTSTR outHostName, DWORD len);
	CString GetHostName();
	DWORD GetHostNameFromIp(LPCTSTR ipAddr, LPTSTR outHostName, DWORD len, int family = AF_INET);
	DWORD  GetIPs(LPCTSTR name, VecWString &outIPs, int family = AF_INET);
	template <class T>
	T ToggleEndian(T t, unsigned nBits = 3 /* 1 << nBits - number if bits to swap*/)
	{
		unsigned sizeInBytes = sizeof(T);
		while (nBits) { // Sanity check - normalize the no. of bits if it greater than total number of bits
			if ((sizeInBytes << 3) <= ((unsigned)1 << nBits))
				--nBits;
			else break;
		}
		const unsigned mask = (1 << (1 << nBits)) -1 ;	// Set all 1s with number of bits to swap
		const unsigned loopCount = (sizeInBytes << 3) / ((1 << nBits) << 1);
		sizeInBytes = (sizeInBytes << 3) / (1 << nBits); // Reset size in bytes multiple of bit length
		T retVal(0); // initialize output value to zero
		for (unsigned i = 0; i < loopCount; ++i) {
			T bitDiff = (sizeInBytes-1 - (i<<1)) << nBits;
			T higherBits = (sizeInBytes-1 - i) << nBits;
			retVal |= (t & (mask << (i << nBits))) << bitDiff; // Store lower bits of input value to higher bits of output
			retVal |= (t & (mask << higherBits)) >> bitDiff; // Store higher bits of input value to lower bits of output
		}
		return retVal;
	}

	CString GetIpV4StrFromAddr(DWORD ipv4Addr);
	DWORD GetIpV4AddrFromString(LPCTSTR ipV4Str);
	BOOL IsConnectedToLAN();
	BOOL IsConnectedToInternet();
}