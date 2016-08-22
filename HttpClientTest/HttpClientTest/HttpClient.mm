#include "stdafx.h"
#include "HttpClient.h"
#include "Path.h"
#import <Cocoa/Cocoa.h>
#include "STLUtils.h"
#include "ProcessUtil.h"

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
    @autoreleasepool {
        NSURL *url = [NSURL URLWithString:[NSString stringWithUTF8String:pURL]];
        NSMutableURLRequest *request = [NSMutableURLRequest requestWithURL:url];
        if (!mhttpVerb.empty() && mhttpVerb != "GET")
            [request setHTTPMethod:[NSString stringWithUTF8String:mhttpVerb.c_str()]];
        if (!mUserName.empty()) {
            NSString *authStr = [NSString stringWithUTF8String: (mUserName+":"+mPassword).c_str()];
            NSData *authData = [authStr dataUsingEncoding:NSUTF8StringEncoding];
            NSString *authValue = [NSString stringWithFormat: @"Basic %@",[authData base64EncodedStringWithOptions:0]];
            [request setValue:authValue forHTTPHeaderField:@"Authorization"];
        }
        if (mDataToSend.DataSize() > 0) {
            lstring dataSize;
            STLUtils::ChangeType(mDataToSend.DataSize(), dataSize);
            [request setValue:[NSString stringWithUTF8String:dataSize.c_str()] forHTTPHeaderField:@"Content-Length"];
            [request setHTTPBody:[NSData dataWithBytesNoCopy:mDataToSend length:mDataToSend.DataSize() freeWhenDone:NO]];
        }
        __block NSError *nserror = nil;
        __block bool bDone(false);
        NSURLSession *session = [NSURLSession sharedSession];
        NSURLSessionDataTask *dataTask = [session dataTaskWithRequest:request completionHandler:
            ^(NSData *data, NSURLResponse *response, NSError *error)
            {
                nserror = error;
                if (error == nil || error.code == 0) {
                    BinaryData downloadData(data.bytes, data.length, false);
                    ContentHandler(downloadData);
                }
                bDone = true;
            }];
        [dataTask resume];
        while (!bDone)
            ProcessUtil::Sleep(10);
        bSuccess = nserror==nil || nserror.code == 0;
    }
    
    return bSuccess;
}

bool CHttpClient::ContentHandler(const BinaryData & inData)
{
    UNREFERENCED_PARAMETER(inData);

#ifdef DEBUG
    _tprintf(_T("%s\n"), inData.HexDump().c_str());
#endif // _DEBUG

    return true;
}
