#pragma once

typedef struct {
	ULONG i[2];
	ULONG buf[4];
	unsigned char in[64];
	unsigned char digest[16];
} MD5_CTX;

struct MD5CBData {
	void *pUserData;
	UINT64 done;
	UINT64 totalSize;
};

typedef int (*MD5CBFn)(MD5CBData *pData);

class CMD5Init {
public:
	static const CMD5Init& GetInstance();
	static void ReleaseInstance();
	void MD5Init(MD5_CTX *md5Ctx) const;
	void MD5Update(MD5_CTX *md5Ctx, unsigned char *buf, unsigned int len) const;
	void MD5Final(MD5_CTX *md5Ctx) const;
	class AutoReleaseInstance {
	public:
		AutoReleaseInstance()
		{
			CMD5Init::GetInstance();
		}
		~AutoReleaseInstance()
		{
			CMD5Init::ReleaseInstance();
		}
	};
	bool IsValid() const {return mMD5UpdateFn != NULL;}
private:
	CMD5Init();
	~CMD5Init();
	HMODULE hMod;
	typedef void (__stdcall *MD5Fn)(MD5_CTX*);
	typedef void (__stdcall*MD5UpdateFn)(MD5_CTX*, unsigned char*, unsigned int);
	MD5Fn mMD5InitFn;
	MD5UpdateFn mMD5UpdateFn;
	MD5Fn mMD5FinalFn;
	static CMD5Init *mInstance;
};

class CMD5
{
public:
	CMD5(void);
	~CMD5(void);
	CString GetMD5(LPCTSTR fileOrText, bool bString = false);
	void SetMD5CBFn(MD5CBFn cbFn, void *pUserData = NULL);
private:
	MD5_CTX md5Ctx;
	MD5CBFn mMD5CBFn;
	void *m_pUserData;
};

