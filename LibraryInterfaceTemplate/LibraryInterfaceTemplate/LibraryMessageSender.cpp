#include "stdafx.h"
#include "LibraryMessageSender.h"
#ifdef _WIN32
#include <windows.h>
#include <codecvt>
#define LM_LIB_SYMBOL(h,n) GetProcAddress((HMODULE)h, n)
#else
#define LM_LIB_SYMBOL(h,n) dlsym(h, n)
#include <dlfcn.h>
#endif // _WIN32

#include<stdarg.h>
#include "Paramters.h"


LibraryMessageSender::LibraryMessageSender(const char *libName /* = NULL */, const char *functionNameSuffix /* = NULL */)
    : mbUnloadLibrary(false), m_FnInitialize(NULL), m_FnFinalize(NULL), m_FnProcessMessage(NULL), m_FnFreeMessage(NULL),
    mInitialize("Initialize")
{
    void (LibraryMessageSender::*pFN[])(const std::string &) = { &LibraryMessageSender::SetFunctionNameInitialize,
        &LibraryMessageSender::SetFunctionNameFinalize , &LibraryMessageSender::SetFunctionNameProcessMessage ,
        &LibraryMessageSender::SetFunctionNameFreeMessage };
    for (auto fn : pFN)
        (this->*fn)("");
    SetFunctionNameSuffix(functionNameSuffix ? functionNameSuffix : "");
    SetLibraryName(libName ? libName : "");
}


LibraryMessageSender::~LibraryMessageSender()
{
    UnLoad();
}


void LibraryMessageSender::SetFunctionName(std::string &outName, const std::string &fnName, const std::string &defName)
{
    outName = fnName;
    if (outName.empty())
        outName = defName;
}

void LibraryMessageSender::UnLoad()
{
    if (IsLoaded()) {
        const bool bUnload(mbUnloadLibrary);
        mbUnloadLibrary = false;
        m_FnInitialize = NULL;
        m_FnFinalize = NULL;
        m_FnProcessMessage = NULL;
        m_FnFreeMessage = NULL;
        if (bUnload && mhLibraryHandle != NULL) {
#ifdef _WIN32
            FreeLibrary((HMODULE)mhLibraryHandle);
#else
            dlclose(mhLibraryHandle);
#endif 
        }
        mhLibraryHandle = NULL;
        mLibraryName.clear();
    }
}

bool LibraryMessageSender::SetLibraryName(const std::string &libName)
{
    if (mLibraryName != libName) {
        UnLoad();
        if (libName.length() > 0) {
            mLibraryName = libName;
#ifdef _WIN32
            std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
            mhLibraryHandle = LoadLibrary(myconv.from_bytes(mLibraryName).c_str());
#else
            mhLibraryHandle = dlopen(mLibraryName.c_str(), RTLD_LAZY | RTLD_LOCAL);
#endif
            SetLibraryHandle(mhLibraryHandle);
            mbUnloadLibrary = true;
        }
    }
    return IsLoaded();
}

void LibraryMessageSender::SetFunctionNameInitialize(const std::string & intialize)
{
    SetFunctionName(mInitialize, intialize, "Initialize");
}

void LibraryMessageSender::SetFunctionNameFinalize(const std::string & fnName)
{
    SetFunctionName(mFinalize, fnName, "Finalize");
}

void LibraryMessageSender::SetFunctionNameProcessMessage(const std::string &fnName)
{
    SetFunctionName(mProcessMessage, fnName, "ProcessMessage");
}

void LibraryMessageSender::SetFunctionNameFreeMessage(const std::string &fnName)
{
    SetFunctionName(mFreeMessage, fnName, "FreeMessage");
}

void LibraryMessageSender::SetFunctionNameSuffix(const std::string & suffix)
{
    std::string *fnNames[] = { &mInitialize, &mFinalize, &mProcessMessage, &mFreeMessage };
    for (auto name : fnNames)
        *name += suffix;
}

bool LibraryMessageSender::SetLibraryHandle(void * libHandle)
{
    mhLibraryHandle = libHandle;
    if (!IsLoaded() && mhLibraryHandle != NULL) {
        m_FnInitialize = (int(*)())LM_LIB_SYMBOL(mhLibraryHandle, mInitialize.c_str());
        m_FnFinalize = (int(*)())LM_LIB_SYMBOL(mhLibraryHandle, mFinalize.c_str());
        m_FnProcessMessage = (const char*(*)(const char *, const char *))LM_LIB_SYMBOL(mhLibraryHandle, mProcessMessage.c_str());
        m_FnFreeMessage = (void(*)(const char *))LM_LIB_SYMBOL(mhLibraryHandle, mFreeMessage.c_str());
    }
    return IsLoaded();
}

bool LibraryMessageSender::IsLoaded()
{
    return m_FnProcessMessage != NULL;
}

bool LibraryMessageSender::ProcessMessage(const std::string & inMessage, const std::string & inMessageData, std::string &outResult)
{
    outResult.clear();
    if (IsLoaded()) {
        const char *result(m_FnProcessMessage(inMessage.c_str(), inMessageData.c_str()));
        outResult = result ? result : "";
        m_FnFreeMessage(result);
    }
    return IsLoaded();
}

bool LibraryMessageSender::ProcessMessageWithArguments(const std::string &inMessage, std::string &outResult, const char *arg /*= NULL*/, ...)
{
    Paramters params;

    if (arg != NULL) {
        va_list ap;
        va_start(ap, arg);
        while (arg) {
            std::string key(arg);
            arg = va_arg(ap, const char *);
            std::string value(arg ? arg : "");
            params.SetParamValue(key, value);
            if (arg)
                arg = va_arg(ap, const char *);
        }
    }
    return ProcessMessage(inMessage, params.ToString(), outResult);
}
