#pragma once

#include "Common.h"
#include "BinaryFind.h"
#include "stlutils.h"


class CHttpClient
{
public:
    CHttpClient();
    virtual ~CHttpClient();
    bool Request(const TCHAR *pURL = NULL);

    virtual bool ContentHandler(const BinaryData &inData);


    DEFINE_FUNCTION_SET_GET_FLAGBIT(muFlags, Https)

    lstring mUserAgent;
    lstring mServer;
    unsigned mPort;
    lstring mPath;
    lstring mUserName, mPassword;
    lstring mhttpVerb, mhttpHeaders;
    BinaryData mDataToSend;

private:
    enum {
        Https
    };
    unsigned muFlags;
};

