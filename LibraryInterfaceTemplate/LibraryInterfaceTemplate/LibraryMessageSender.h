#pragma once

#include <string>
#ifdef _WIN32
#define DEFAULT_LIB_EXTN ".dll"
#define DEFAULT_LIB_PREFIX ""
#else
#define DEFAULT_LIB_EXTN ".dylib"
#define DEFAULT_LIB_PREFIX "lib"
#endif // _WIN32

class LibraryMessageSender
{
public:
    LibraryMessageSender(const char *libName = NULL, const char *functionNameSuffix = NULL);
    ~LibraryMessageSender();
    bool SetLibraryName(const std::string &libName);
    void SetFunctionNameInitialize(const std::string &fnName);
    void SetFunctionNameFinalize(const std::string &fnName);
    void SetFunctionNameProcessMessage(const std::string &fnName);
    void SetFunctionNameFreeMessage(const std::string &fnName);
    void SetFunctionNameSuffix(const std::string &suffix);
    bool SetLibraryHandle(void *libHandle);
    bool IsLoaded();

    bool ProcessMessage(const std::string &inMessage, const std::string &inMessageData, std::string &outResult);
    // Always send last argument as NULL to complete arguments
    bool ProcessMessageWithArguments(const std::string &inMessage, std::string &outResult, const char *arg = NULL, ...);
private:
    void SetFunctionName(std::string &outName, const std::string &fnName, const std::string &defName);
    void UnLoad();
    std::string mLibraryName, mInitialize, mFinalize, mProcessMessage, mFreeMessage;
    void *mhLibraryHandle;
    bool mbUnloadLibrary;
    int (*m_FnInitialize)();
    int (*m_FnFinalize)();
    const char* (*m_FnProcessMessage)(const char *msg, const char *msgData);
    void(*m_FnFreeMessage)(const char *msgResult);
};

