#include "stdafx.h"
#include "HttpClient.h"
#include "Path.h"
#include <Wininet.h>

#pragma comment(lib, "Wininet.lib")


CHttpClient::CHttpClient()
    : mPort(0), mhttpVerb(_T("GET")), muFlags(0)
{
    mUserAgent = Path::GetModuleFilePath().RenameExtension();
}


CHttpClient::~CHttpClient()
{
}
#define GET_PTR_FROM_STR(s) s.empty() ? NULL : s.c_str()

bool CHttpClient::Request(const TCHAR * pURL)
{
    bool bSuccess(false);
    if (pURL != NULL) {
        URL_COMPONENTS urlCom = {sizeof(URL_COMPONENTS),
            NULL, 1, INTERNET_SCHEME_DEFAULT,
            NULL, 1, 0,
            NULL, 1,
            NULL, 1,
            NULL, 1,
            NULL, 1,
        };
        InternetCrackUrl(pURL, lstrlen(pURL), 0, &urlCom);
        mServer = urlCom.lpszHostName ? lstring(urlCom.lpszHostName, urlCom.dwHostNameLength) : pURL;
        mPort = urlCom.nPort;
        if (urlCom.lpszUrlPath)
            mPath = lstring(urlCom.lpszUrlPath, urlCom.dwUrlPathLength);
        if (urlCom.lpszUserName)
            mUserName = lstring(urlCom.lpszUserName, urlCom.dwUserNameLength);
        if (urlCom.lpszPassword)
            mPassword = lstring(urlCom.lpszPassword, urlCom.dwPasswordLength);
        SetHttps(urlCom.nScheme == INTERNET_SCHEME_HTTPS);
    }
    HINTERNET hi = InternetOpen(mUserAgent.c_str(), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if (hi != NULL) {
        if (!mPort)
            mPort = IsHttps() ? INTERNET_DEFAULT_HTTPS_PORT : INTERNET_DEFAULT_HTTP_PORT;
        HINTERNET hc = InternetConnect(hi, mServer.c_str(), mPort,
            GET_PTR_FROM_STR(mUserName), GET_PTR_FROM_STR(mPassword),
            INTERNET_SERVICE_HTTP, 0, 0);
        if (hc != NULL) {
            const TCHAR *rgpszAcceptTypes[] = { _T("*/*"), NULL};
            DWORD dwDword(INTERNET_FLAG_NO_UI);
            if (IsHttps())
                dwDword |= INTERNET_FLAG_SECURE;
            HINTERNET http = HttpOpenRequest(hc, GET_PTR_FROM_STR(mhttpVerb), mPath.c_str(), NULL, NULL, rgpszAcceptTypes,
                dwDword, NULL);
            if (http != NULL) {
                if (HttpSendRequest(http, GET_PTR_FROM_STR(mhttpHeaders), (DWORD)mhttpHeaders.length(), mDataToSend, (DWORD)mDataToSend.DataSize())) {
                    int statusCode(0);
                    {
                        wchar_t responseText[1024]; // change to wchar_t for unicode
                        DWORD responseTextSize = sizeof(responseText);

                        //Check existence of page (for 404 error)
                        if (!HttpQueryInfo(http,
                            HTTP_QUERY_STATUS_CODE,
                            &responseText,
                            &responseTextSize,
                            NULL))
                            statusCode = GetLastError();
                        else
                            STLUtils::ChangeType(lstring(responseText), statusCode);
                    }
                    BinaryData inDataRead(NULL, 1024*1024*4); // 4MB
                    while (InternetReadFile(http, inDataRead, (DWORD)inDataRead.Size(), &dwDword) && dwDword > 0)
                    {
                        bSuccess = true;
                        inDataRead.SetDataSize(dwDword);
                        if (!ContentHandler(inDataRead))
                            break;
                    }
                }
                InternetCloseHandle(http);
            }
            InternetCloseHandle(hc);
        }
        InternetCloseHandle(hi);
    }
    return bSuccess;
}

bool CHttpClient::ContentHandler(const BinaryData & inData)
{
    UNREFERENCED_PARAMETER(inData);

#ifdef _DEBUG
    _tprintf(_T("%s\n"), inData.HexDump().c_str());
#endif // _DEBUG

    return true;
}
