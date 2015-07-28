#pragma once

#define _ReadBufSize 0x1000000

typedef unsigned long long MD5ULL;

class MD5Callback {
public:
	MD5Callback() : mTotalBytes(0), mCurrentDone(0) {}
	virtual int Status() = 0;
	const MD5ULL& GetTotal() const {return mTotalBytes;}
	const MD5ULL& GetCurrent() const {return mCurrentDone;}
protected:
	virtual void SetTotal(MD5ULL total);
private:
	MD5ULL mTotalBytes;
	MD5ULL mCurrentDone;
	int UpdateCurrent(MD5ULL currentAdd);
	friend class cMD5;
};

class cMD5
{
public:
    char* CalcMD5FromString(const char *s8_Input);
    char* CalcMD5FromFile  (LPCTSTR s8_Path, bool bReset = true);

    void FreeBuffer();
    cMD5(MD5Callback *pMD5Callback = NULL);
    virtual ~cMD5();
	void SetMD5Callback(MD5Callback *pMD5Callback)
	{
		m_pMD5Callback = pMD5Callback;
	}
    char* MD5FinalToString();
private:
    struct MD5Context
    {
        unsigned long buf[4];
        unsigned long bits[2];
        unsigned char in[64];
    };

    void MD5Init();
    void MD5Update(unsigned char *buf, unsigned len);
    void MD5Final (unsigned char digest[16]);

    void MD5Transform(unsigned long buf[4], unsigned long in[16]);

    void byteReverse (unsigned char *buf, unsigned longs);

    char *mp_s8ReadBuffer;
    MD5Context ctx;
    char   ms8_MD5[40]; // Output buffer
	MD5Callback *m_pMD5Callback;
};

