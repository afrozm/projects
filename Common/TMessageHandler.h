//
// (C) Copyright 2017 by Autodesk, Inc.
//
// The information contained herein is confidential, proprietary
// to Autodesk, Inc., and considered a trade secret as defined
// in section 499C of the penal code of the State of California.
// Use of this information by anyone other than authorized
// employees of Autodesk, Inc. is granted only under a written
// non-disclosure agreement, expressly prescribing the scope
// and manner of such use.
//

#pragma once

#include <functional>
#include <string>
#include <map>

namespace STLUtils {

    template<typename ReturnType, typename ArgumentType, typename ClassName>
    class TFunction
    {
    public:
        typedef ReturnType (*CFunction)(const ArgumentType &);
        typedef ReturnType(ClassName::*ClassFunction)(const ArgumentType &);
        typedef std::function<ReturnType(const ArgumentType &)> StdFunction;

        TFunction(CFunction f = NULL) : mCFunction(f), mClassFunction(nullptr), m_pCaller(nullptr) {}
        TFunction(ClassFunction cf, ClassName *pCaller) : mCFunction(nullptr), mClassFunction(cf), m_pCaller(pCaller) {}
        TFunction(const StdFunction &f) : mCFunction(nullptr), mClassFunction(nullptr), m_pCaller(nullptr), mStdFunction(f) {}

        ReturnType operator()(const ArgumentType &inArg) const
        {
            if (mCFunction)
                return mCFunction(inArg);
            if (mClassFunction)
                return (m_pCaller->*mClassFunction)(inArg);
            if (mStdFunction)
                return mStdFunction(inArg);
            return ReturnType();
        }
        operator bool() const
        {
            if (mCFunction)
                return true;
            if (mClassFunction)
                return true;
            if (mStdFunction)
                return true;
            return false;
        }
    protected:
        CFunction mCFunction;
        ClassFunction mClassFunction;
        ClassName *m_pCaller;
        StdFunction mStdFunction;
    };


    template<typename ReturnType, typename ArgumentType, typename ClassName>
    class TMessageHandler {
    public:
        typedef TFunction<ReturnType, ArgumentType, ClassName> HandlerFunction;
        void AddMessageHandler(const std::string &inMessage, HandlerFunction inFunction)
        {
            if (inFunction)
                mMapHandlerFunction[inMessage] = inFunction;
            else
                mMapHandlerFunction.erase(inMessage);
        }
        ReturnType CallHandler(const std::string &inMessage, const ArgumentType &inArg, bool *outFnCalled = nullptr)
        {
            bool bCalled(false);
            bool &bFnCalled(outFnCalled ? *outFnCalled : bCalled);
            bFnCalled = false;
            auto cit(mMapHandlerFunction.find(inMessage));
            if (cit != mMapHandlerFunction.end()) {
                bFnCalled = true;
                return cit->second(inArg);
            }
            return ReturnType();
        }
    protected:
        typedef std::map<std::string, HandlerFunction> MapHandlerFunction;
        MapHandlerFunction mMapHandlerFunction;
    };

#define TMESSAGE_HANDLER_ADD_CLASS_FN_WITH_NAME(mt,m,n,c,f,o) m.AddMessageHandler(n, mt::HandlerFunction((mt::HandlerFunction::ClassFunction)&c::f,o))
#define TMESSAGE_HANDLER_ADD_CLASS_FN(mt,m,c,f,o) TMESSAGE_HANDLER_ADD_CLASS_FN_WITH_NAME(mt,m,#f,c,f,o)

}
