// LibraryInterfaceTemplate.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "LibraryMessageSender.h"
#include "Paramters.h"


#ifdef _WIN32
#define DEFAULT_LIB_EXTN ".dll"
#define DEFAULT_LIB_PREFIX ""
#else
#define DEFAULT_LIB_EXTN ".dylib"
#define DEFAULT_LIB_PREFIX "lib"
#include <dlfcn.h>
#endif // _WIN32


int main()
{
    LibraryMessageSender msgSender(DEFAULT_LIB_PREFIX "DynamicLibraryTemplate" DEFAULT_LIB_EXTN, "_DLT");
    Paramters parameters;
    const char * paramNames[] = { "param1", "param2", "testParam" };
    for (auto param : paramNames)
        parameters.SetParamValue(param, param);
    std::string result;
    const std::string msg("MessageHandler_TestMessage"), params(parameters.ToString());
    printf("Sending message %s with param\n%s\n", msg.c_str(), params.c_str());
    msgSender.ProcessMessage(msg, params, result);
    printf("result=%s\n", result.c_str());
    msgSender.ProcessMessageWithArguments(msg, result, "param1", "help", "param2", "p2val", "testParam", "hell", NULL);
    printf("result=%s\n", result.c_str());
    msgSender.ProcessMessageWithArguments("unknown_message", result, "param1", "help", "param2", "p2val", "testParam", "hell", NULL);
    return 0;
}

