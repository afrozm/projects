// DynamicLibraryTemplate.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "ILibraryExports.h"
#include <stdio.h>

#ifdef _WIN32
// windows symbols are exported from .def file
#define DLT_EXPORT
#else
// symbols exported from visibility default and from exp file
#define DLT_EXPORT __attribute__((visibility("default")))
#endif

// Rename _DLT with you own library name

extern "C" {
    DLT_EXPORT int Initialize_DLT()
    {
        ILibraryExports *pHandler(ILibraryExports::GetLibraryMessageHandler());
        if (pHandler)
            return pHandler->Initialize();
        return -1;
    }
    DLT_EXPORT int Finalize_DLT()
    {
        ILibraryExports *pHandler(ILibraryExports::GetLibraryMessageHandler());
        if (pHandler)
            return pHandler->Finalize();
        return -1;
    }
    DLT_EXPORT const char* ProcessMessage_DLT(const char *msg, const char *msgData)
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
            UNREFERENCED_PARAMETER(bHandled);
            if (outResult.length() == 0)
                return NULL;
            const size_t dataLen(outResult.length() + 1);
            char *outData = new char[dataLen];
            strcpy_s(outData, dataLen, outResult.c_str());
            return outData;
        }
        return NULL;
    }
    DLT_EXPORT void FreeMessage_DLT(const char *msgResult)
    {
        if (msgResult != NULL)
            delete[] msgResult;
    }
}