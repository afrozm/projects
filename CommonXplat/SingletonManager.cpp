#include "SingletonManager.h"

Singleton::Singleton()
    :mSingletonReference(nullptr), m_iPriority(0)
{
}

Singleton::~Singleton()
{
    if (mSingletonReference)
        *mSingletonReference = nullptr;
}


void Singleton::FreeSingletonInstance(PSingleton &singleton)
{
    if (singleton) {
        if (singleton->mInstanceFunction)
            singleton = singleton->mInstanceFunction(FreeInstance);
        if (singleton != nullptr) { // callback method did not free it - so free it
            singleton->BeforeDestroy();
            delete singleton;
            singleton = nullptr;
        }
    }
}

void Singleton::AddSingleton(PSingleton singleton)
{
    SM_GET_SINGLETON_MGR->AddSingleton(singleton);
}

Singleton::PSingleton SingletonManager::AddSingleton(PSingleton pSingleton)
{
    if (pSingleton != nullptr && pSingleton != this) {
        bool bAdd(true);
        // check if exists
        for (auto & cit : mListPSingleton) {
            if (cit == pSingleton
                || (!pSingleton->Name().empty() && cit->Name() == pSingleton->Name())) {
                bAdd = false;
                break;
            }
        }
        if (bAdd) {
            // Add in priority sorted order (descending)
            bAdd = false;
            for (auto cit = mListPSingleton.begin(); cit != mListPSingleton.end(); ++cit) {
                if (pSingleton->Priority() >= (*cit)->Priority()) {
                    mListPSingleton.insert(cit, pSingleton);
                    bAdd = true;
                    break;
                }
            }
            if (!bAdd)
                mListPSingleton.push_back(pSingleton);
        }
    }
    return pSingleton;
}

void SingletonManager::RemoveAllSingletons()
{
    for (auto & cit : mListPSingleton)
        Singleton::FreeSingletonInstance(cit);
    mListPSingleton.clear();
}

Singleton::PSingleton SingletonManager::GetInstanceByName(const char *pName, SingletonFlag sf /* = Singleton::ReturnExisting */, PSingleton *outSingleton /* = nullptr */)
{
    PSingleton tempVal(nullptr);
    PSingleton &pRet = outSingleton ? *outSingleton : tempVal;
    if (pName == nullptr) {
        if (pRet == nullptr)
            pRet = this;
    }
    if (pRet != this) {
        for (auto cit = mListPSingleton.begin(); cit != mListPSingleton.end(); ++cit) {
            if ((pRet && pRet == *cit)
                || (*cit)->Name() == pName) {
                if (nullptr == pRet)
                    pRet = *cit;
                if (sf == FreeInstance)
                    mListPSingleton.erase(cit);
                break;
            }
        }
    }
    if (pRet && sf == FreeInstance)
        FreeSingletonInstance(pRet);
    return pRet;
}

SingletonManager::SingletonManager()
{
}


SingletonManager::~SingletonManager()
{
}

void SingletonManager::BeforeDestroy()
{
    RemoveAllSingletons();
}

#ifdef _WIN32
class AutoReleaseSingletonManager
{
public:
    ~AutoReleaseSingletonManager();
};

AutoReleaseSingletonManager::~AutoReleaseSingletonManager()
{
    SingletonManager::Instance(Singleton::FreeInstance);
}
static AutoReleaseSingletonManager sAutoReleaseSingletonManager;
#else // Linux and mac
__attribute__((destructor(101))) // call this destructor as late as possible
static void ReleaseSingletonManager()
{
    SingletonManager::Instance(Singleton::FreeInstance);
}
#endif // _WIN32

