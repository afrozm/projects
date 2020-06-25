#include "StdAfx.h"
#include "RefCountObj.h"
#include "AutoLock.h"



CRefCountObj::CRefCountObj()
	: m_uRefCount(1)
{

}

CRefCountObj::~CRefCountObj( void )
{

}

int CRefCountObj::IncrmentRefCount()
{
	if (this == NULL)
		return 0;
	CAutoLock al(m_Lock);
	++m_uRefCount;
	return m_uRefCount;
}

int CRefCountObj::DecrementRefCount(CRefCountObj **objThis /* = NULL */)
{
	ASSERT(objThis == NULL || *objThis == this);
	if (this == NULL)
		return 0;
    CRefCountObj *pObjToDelte(nullptr);
    int outRefCount(0);
    {
        CAutoLock al(m_Lock);
        --m_uRefCount;
        outRefCount = m_uRefCount;
        if (m_uRefCount == 0) {
            if (objThis && *objThis == this)
                *objThis = NULL;
            pObjToDelte = this;
        }
    }
    if (pObjToDelte)
        delete pObjToDelte;
	return outRefCount;
}

