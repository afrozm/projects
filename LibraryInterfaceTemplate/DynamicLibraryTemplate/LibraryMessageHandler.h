#pragma once
#include "ILibraryExports.h"
#include "Paramters.h"

class LibraryMessageHandler :
    public ILibraryExports
{

public:
    virtual int Initialize() override;
    virtual int Finalize() override;
    virtual bool ProcessMessage(const StdString &msg, const StdString &msgData, StdString &outResult) override;

    typedef void (LibraryMessageHandler::*MessageHandlerFn)(const StdString &msg, Paramters &params, StdString &outResult);
    void AddMessageHandler(const StdString &msg, MessageHandlerFn handler);
protected:
    LibraryMessageHandler();
    ~LibraryMessageHandler();
    typedef std::map<StdString, MessageHandlerFn> MapMessageHandlerFn;
    MapMessageHandlerFn mMapMessageHandlerFn;
};

#define ADD_MESSAGE_HANDLER_WITH_NAME(s,c,m) AddMessageHandler(s, (LibraryMessageHandler::MessageHandlerFn)&c::m)
#define ADD_MESSAGE_HANDLER(c,m) ADD_MESSAGE_HANDLER_WITH_NAME(#m, c, m)
