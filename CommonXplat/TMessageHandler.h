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
        typedef ReturnType (*CFunction)(const ArgumentType &, void *pUserData);
        typedef ReturnType(ClassName::*ClassFunction)(const ArgumentType &);
        typedef std::function<ReturnType(const ArgumentType &)> StdFunction;

        TFunction(CFunction f = nullptr, void *pUserData = nullptr) : m_pCaller((ClassName *)pUserData), mType(FT_CFuntion)
        {
            mFunction.mCFunction = f;
        }
        TFunction(ClassFunction cf, ClassName *pCaller) : m_pCaller(pCaller), mType(FT_ClassFuntion)
        {
            mFunction.mClassFunction = cf;
        }
        TFunction(const StdFunction &f) : mType(FT_StdFuntion)
        {
            mFunction.mStdFunction = new StdFunction(f);
        }
        TFunction(const TFunction &tFuntion) : mFunction(tFuntion.mFunction), m_pCaller(tFuntion.m_pCaller), mType(tFuntion.mType)
        {
            if (FT_StdFuntion == mType && mFunction.mStdFunction)
                mFunction.mStdFunction = new StdFunction(*mFunction.mStdFunction);
        }
        TFunction(TFunction&& tFuntion) : mFunction(tFuntion.mFunction), m_pCaller(tFuntion.m_pCaller), mType(tFuntion.mType)
        {
            tFuntion.mFunction.mStdFunction = nullptr;
        }
        TFunction& operator=(const TFunction& tFuntion)
        {
            Clear();
            mType = tFuntion.mType;
            mFunction = tFuntion.mFunction;
            m_pCaller = tFuntion.m_pCaller;
            if (FT_StdFuntion == mType && mFunction.mStdFunction)
                mFunction.mStdFunction = new StdFunction(*mFunction.mStdFunction);
            return *this;
        }
        TFunction& operator=(TFunction&& tFuntion)
        {
            Clear();
            mType = tFuntion.mType;
            mFunction = tFuntion.mFunction;
            m_pCaller = tFuntion.m_pCaller;
            tFuntion.mFunction.mStdFunction = nullptr;
            return *this;
        }
        ~TFunction()
        {
            Clear();
        }

        ReturnType operator()(const ArgumentType &inArg) const
        {
            if (*this) {
                switch (mType)
                {
                case FT_ClassFuntion:
                    return (m_pCaller->*mFunction.mClassFunction)(inArg);
                case FT_StdFuntion:
                    return (*mFunction.mStdFunction)(inArg);
                case FT_CFuntion:
                default:
                    return mFunction.mCFunction(inArg, (void*)m_pCaller);
                }
            }
            return ReturnType();
        }
        operator bool() const
        {
            return mFunction.mCFunction != nullptr;
        }
    protected:
        void Clear()
        {
            if (FT_StdFuntion == mType && mFunction.mStdFunction)
                delete mFunction.mStdFunction;
            mFunction.mCFunction = nullptr;
            m_pCaller = nullptr;
            mType = FT_None;
        }
        union FuntionPtr
        {
            CFunction mCFunction = nullptr;
            ClassFunction mClassFunction;
            StdFunction *mStdFunction;
        } mFunction;
        ClassName *m_pCaller = nullptr;
        enum FuntionType {
            FT_None,
            FT_CFuntion,
            FT_ClassFuntion,
            FT_StdFuntion
        } mType = FT_None;
    };
#define TFUNTION_DEFINE_CLASS_FN_(CN,CM,CO,TF) TF((TF::ClassFunction)&CN::CM, CO)
#define TFUNTION_DEFINE_CLASS_FN(CN,CM,CO,TF,TCN) TFUNTION_DEFINE_CLASS_FN_(CN, CM, (TCN*)CO, TF)
#define TFUNTION_TO_STRING(f) #f

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
        size_t size() const { return mMapHandlerFunction.size(); }
    protected:
        typedef std::map<std::string, HandlerFunction> MapHandlerFunction;
        MapHandlerFunction mMapHandlerFunction;
    };

#define TMESSAGE_HANDLER_ADD_CLASS_FN_WITH_NAME(mt,m,n,c,f,o) m.AddMessageHandler(n, TFUNTION_DEFINE_CLASS_FN_(c,f,o,mt::HandlerFunction))
#define TMESSAGE_HANDLER_ADD_CLASS_FN(mt,m,c,f,o) TMESSAGE_HANDLER_ADD_CLASS_FN_WITH_NAME(mt,m,TFUNTION_TO_STRING(f),c,f,o)

}
