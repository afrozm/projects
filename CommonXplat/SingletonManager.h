#pragma once

#include "TMessageHandler.h"
#include <list>

// Base Singleton class for all singleton
class Singleton
{
public:
	// Used in GetInstance
	enum SingletonFlag {
		CreateInstance, // if singleton object does not exits - create new one and return
		ReturnExisting, // return existing pointer to single object - can be null
		FreeInstance	// free existing single object
	};
	int Priority() const { return m_iPriority; }
	const std::string& Name() const { return mName; }
protected:
	// private constructor and destructor
	Singleton();
	virtual ~Singleton();
	// Called before singleton is destroyed
	virtual void BeforeDestroy() {}

	typedef Singleton* PSingleton;
	// FreeSingletonInstance - Destroy single object
	static void FreeSingletonInstance(PSingleton &singleton);
	typedef STLUtils::TFunction<PSingleton, SingletonFlag, Singleton> InstanceFunction;

	// CreateSingletonInstance - template based Singleton object creation helper function
	template<typename ClassName>
	static void CreateSingletonInstance(ClassName* &outInsance, const char *name = nullptr, int priority = 0, InstanceFunction insFun = nullptr)
	{
		if (nullptr == outInsance) { // if singleton object is null, then create one
			if (insFun)			// call creator function if available
				outInsance = (ClassName*)insFun(CreateInstance);
			if (outInsance == nullptr) {// call new if creator function did noting
				outInsance = new ClassName;
				outInsance->mSingletonReference = (PSingleton*)&outInsance;
			}
			outInsance->mName = name ? name : "";
			outInsance->m_iPriority = priority;
                        if (insFun)
                            outInsance->mInstanceFunction = insFun;
			AddSingleton(outInsance);	// Register it to SingletonManager list
		}
	}
	static void AddSingleton(PSingleton singleton);
	PSingleton *mSingletonReference;	// Store the actual single reference object - like static pointer in GetInstance method. This is used only destructor to set it to null
	std::string mName;	// name of the singleton object - generally the class name
	int m_iPriority;	// priority of singleton object, lower the number higher the priority which means it will be destroyed later
	// mInstanceFunction - optional instance callback function to create and destroy singleton object. User own method managing singleton objects.
	// FreeInstance: mInstanceFunction method must call BeforeDestory method before deleting the single object and return null.
	//				if it not deleting the object, means want default implementation to handle, then it should not return null
	InstanceFunction mInstanceFunction;
};

// SM_DEFINE_GETINSTANCE - Defines the singleton class definition of GetInstance method
// cn - class name
// priority - priority (int), lower the priority, the singleton object will be destroyed later
// insF - InstanceFunction - can be nullptr
#define SM_DEFINE_GETINSTANCE_EX(cn,GetInstance,name,priority,insF) \
friend Singleton; \
static cn* GetInstance(SingletonFlag sf = CreateInstance) \
{ \
	static cn* pInstance = nullptr; \
	if (!pInstance && sf == CreateInstance) \
		CreateSingletonInstance(pInstance, name, priority, insF); \
	else if (sf == FreeInstance) \
		SM_CALL_SINGLETON_MGR_METHOD(GetInstanceByName, name, sf, (PSingleton*)&pInstance); \
	return pInstance; \
}
// Helper macros
#define SM_DEFINE_GETINSTANCE(cn,priority,insF) SM_DEFINE_GETINSTANCE_EX(cn,GetInstance,#cn,priority,insF)
#define SM_CALL_INSTANCE_METHOD_EX(CN,GetInstance,m,...) {CN *pIns = CN::GetInstance(Singleton::ReturnExisting); if (pIns) pIns->m(__VA_ARGS__);}
#define SM_CALL_INSTANCE_METHOD(CN,m,...) SM_CALL_INSTANCE_METHOD_EX(CN,GetInstance,m,__VA_ARGS__)
#define SM_RCALL_INSTANCE_METHOD(CN,m,r,...) {CN *pIns = CN::GetInstance(Singleton::ReturnExisting); if (pIns) r=pIns->m(__VA_ARGS__);}
#define SM_CALL_SINGLETON_MGR_METHOD(m,...) SM_CALL_INSTANCE_METHOD_EX(SingletonManager,Instance,m,__VA_ARGS__)
#define SM_GET_SINGLETON_MGR SingletonManager::Instance()
#define SM_GET_SINGLETON(c,sf) ((c*)SM_GET_SINGLETON_MGR->GetInstanceByName(#c,sf))

// SingletonManager class
// Manages the list of all singletons
// destroys them in priority order
class SingletonManager : public Singleton
{
public:
	SM_DEFINE_GETINSTANCE_EX(SingletonManager, Instance, nullptr, 0, nullptr);
	// AddSingleton - Register the singleton to list
	PSingleton AddSingleton(PSingleton pSingleton);
	// GetInstanceByName - lookup single by name
	// if pName is null, this object of SingletonManager is returned
	PSingleton GetInstanceByName(const char *pName, SingletonFlag sf = Singleton::ReturnExisting, PSingleton *outSingleton = nullptr);
protected:
	virtual void BeforeDestroy() override;
private:
	// Private construction and destructor
	SingletonManager();
	~SingletonManager();
	// RemoveAllSingletons - unregister all singletons and destroys them too
	void RemoveAllSingletons();
	// list of registered singletons
	typedef std::list<PSingleton> ListPSingleton;
	ListPSingleton mListPSingleton;

};
