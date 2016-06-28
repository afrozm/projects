#pragma once
#include "LibraryMessageHandler.h"

// Rename this ExampleHessageHandler class and start handling messages here
class ExampleHessageHandler :
    public LibraryMessageHandler
{
public:
    static ExampleHessageHandler& ExampleHessageHandler::GetInstance();

private:
    ExampleHessageHandler();
    ~ExampleHessageHandler();
    void MessageHandler_TestMessage(const StdString &msg, Paramters &params, StdString &outResult);
};

