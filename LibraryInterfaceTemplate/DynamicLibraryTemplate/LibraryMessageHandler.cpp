#include "LibraryMessageHandler.h"



LibraryMessageHandler::LibraryMessageHandler()
{
}


LibraryMessageHandler::~LibraryMessageHandler()
{
}

int LibraryMessageHandler::Initialize()
{
    return 0;
}

int LibraryMessageHandler::Finalize()
{
    return 0;
}

bool LibraryMessageHandler::ProcessMessage(const StdString &msg, const StdString &msgData, StdString &outResult)
{
    const bool bFoundHandler(mMapMessageHandlerFn.find(msg) != mMapMessageHandlerFn.end());
    if (bFoundHandler) {
        MessageHandlerFn handler(mMapMessageHandlerFn[msg]);
        Paramters params(msgData);
        (this->*handler)(msg, params, outResult);
    }
    return bFoundHandler;
}

void LibraryMessageHandler::AddMessageHandler(const StdString &msg, MessageHandlerFn handler)
{
    if (handler)
        mMapMessageHandlerFn[msg] = handler;
    else
        mMapMessageHandlerFn.erase(msg);
}
