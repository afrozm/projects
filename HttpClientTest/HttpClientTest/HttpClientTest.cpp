// HttpClientTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "HttpClient.h"

int _tmain(int argc, _TCHAR* argv[])
{
    CHttpClient httpClient;
    httpClient.Request(argc > 1 ? argv[1] : _T("http://www.google.com"));
    return 0;
}

