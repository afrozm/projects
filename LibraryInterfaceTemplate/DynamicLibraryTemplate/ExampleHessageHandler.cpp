#include "ExampleHessageHandler.h"
#include <stdio.h>

ILibraryExports* ILibraryExports::GetLibraryMessageHandler()
{
    return &ExampleHessageHandler::GetInstance();
}

ExampleHessageHandler& ExampleHessageHandler::GetInstance()
{
    static ExampleHessageHandler sExampleMessageHandler;
    return sExampleMessageHandler;
}

ExampleHessageHandler::ExampleHessageHandler()
{
    ADD_MESSAGE_HANDLER(ExampleHessageHandler, MessageHandler_TestMessage);
}


ExampleHessageHandler::~ExampleHessageHandler()
{
}

void ExampleHessageHandler::MessageHandler_TestMessage(const StdString & msg, Paramters & params, StdString & outResult)
{
    printf("got message %s with parameters\n", msg.c_str());
    const char * paramNames[] = { "param1", "param2", "testParam" };
    for (auto param : paramNames)
        printf("%s=%s\n", param, params.GetParamValue(param).c_str());
    outResult = "Success";
}
