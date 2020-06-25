#pragma once

#include <string>


class ILibraryExports
{
public:
    static ILibraryExports* GetLibraryMessageHandler();
    virtual int Initialize() = 0;
    virtual int Finalize() = 0;
    typedef std::string StdString;
    virtual bool ProcessMessage(const StdString &msg, const StdString &msgData, StdString &outResult) = 0;
};
