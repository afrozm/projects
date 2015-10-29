#pragma once

class CRefCountObj
{
public:
	CRefCountObj();
	virtual ~CRefCountObj(void);
	int IncrmentRefCount();
	int DecrementRefCount(CRefCountObj **objThis = NULL);
protected:
	unsigned int m_uRefCount;
	CMutex m_Lock;
};
