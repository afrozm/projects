// DynamicLibraryTemplate.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "ILibraryExports.h"
#include <stdio.h>

// Rename _DLT with you own library name

extern "C" {
    int Initialize_DLT()
    {
        ILibraryExports *pHandler(ILibraryExports::GetLibraryMessageHandler());
        if (pHandler)
            return pHandler->Initialize();
        return -1;
    }
    int Finalize_DLT()
    {
        ILibraryExports *pHandler(ILibraryExports::GetLibraryMessageHandler());
        if (pHandler)
            return pHandler->Finalize();
        return -1;
    }
    const char* ProcessMessage_DLT(const char *msg, const char *msgData)
    {
        if (msg == NULL)
            return NULL;
        ILibraryExports *pHandler(ILibraryExports::GetLibraryMessageHandler());
        if (pHandler) {
            if (NULL == msgData)
                msgData = "";
            ILibraryExports::StdString outResult;
            bool bHandled((pHandler->ProcessMessage(msg, msgData, outResult)));
#if _DEBUG
            if (!bHandled)
                printf("No message handler for %s\n", msg);
#endif
            if (outResult.length() == 0)
                return NULL;
            const size_t dataLen(outResult.length() + 1);
            char *outData = new char[dataLen];
            strcpy_s(outData, dataLen, outResult.c_str());
            return outData;
        }
        return NULL;
    }
    void FreeMessage_DLT(const char *msgResult)
    {
        if (msgResult != NULL)
            delete[] msgResult;
    }
}