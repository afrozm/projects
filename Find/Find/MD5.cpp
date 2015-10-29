#include "StdAfx.h"
#include "MD5.h"


static wchar_t ToHex(wchar_t c)
{
	c &= 0x0f;
	if (c > 9)
		c = 'a' + c - 0xa;
	else
		c += '0';
	return c;
}

CString ToHex(const unsigned char *buf, int len)
{
	CString str;
	for (int i = 0; i < len; i++) {
		char c = buf[i];
		str += ToHex(c >> 4);
		str += ToHex(c);
	}
	return str;
}


CMD5::CMD5(void)
{
	memset(&md5Ctx, 0, sizeof(md5Ctx));
}

CMD5::~CMD5(void)
{
}

int ToChar(LPCTSTR str, unsigned char *out, int inLen)
{
	int len = 0;
	if (!out) inLen = 0;
#if defined(UNICODE) || defined(_UNICODE)
	while (*str) {
		const char *cstr = (const char *)str++;
		for (int i = 0; i < sizeof(TCHAR); i++, cstr++) {
			if (*cstr) {
				if (inLen > 0)
					out[len] = *cstr;
				len++;
			}
		}
	}
#else
	len = lstrlen(str);
	if (inLen > len)
		lstrcpy(out, str);
	inLen -= len;
#endif
	if (inLen > 0)
		out[len] = 0;
	return len;
}

CString CMD5::GetMD5(LPCTSTR fileOrText)
{
	CString md5;
	const CMD5Init &md5Init(CMD5Init::GetInstance());
	if (!md5Init.IsValid())
		return md5;
	md5Init.MD5Init(&md5Ctx);
	HANDLE hFile = CreateFile(fileOrText, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	CArray<unsigned char> bufA;
	bufA.SetSize(4096*1024); // 4 MB
	unsigned char *buf = bufA.GetData();
	DWORD nRead = (DWORD)bufA.GetSize();
	//unsigned char buf[4096];
	if (hFile != INVALID_HANDLE_VALUE) {
		MD5CBData cbData = {m_pUserData, 0};
		LARGE_INTEGER li;
		GetFileSizeEx(hFile, &li);
		cbData.totalSize = li.QuadPart;
		mMD5CBFn(&cbData);
		while (ReadFile(hFile, buf, nRead, &nRead, NULL) && nRead > 0) {
			md5Init.MD5Update(&md5Ctx, buf, (unsigned int)nRead);
			cbData.done += nRead;
			if (mMD5CBFn(&cbData))
				break;
			nRead = (DWORD)bufA.GetSize();
		}
		CloseHandle(hFile);
	}
	else {
		nRead = ToChar(fileOrText, buf, nRead);
		md5Init.MD5Update(&md5Ctx, buf, (unsigned int)nRead);
	}
	md5Init.MD5Final(&md5Ctx);
	md5 = ToHex(md5Ctx.digest, sizeof(md5Ctx.digest)/sizeof(md5Ctx.digest[0]));
	return md5;
}
static int DefaultMD5CB(MD5CBData *pData)
{
    UNREFERENCED_PARAMETER(pData);
	return 0;
}
void CMD5::SetMD5CBFn(MD5CBFn cbFn, void *pUserData)
{
	if (cbFn != NULL) {
		mMD5CBFn = cbFn;
		m_pUserData = pUserData;
	}
	else {
		mMD5CBFn = DefaultMD5CB;
		m_pUserData = NULL;
	}
}
CMD5Init* CMD5Init::mInstance = NULL;// initialize pointer

CMD5Init::CMD5Init()
	: hMod(NULL), mMD5InitFn(NULL), mMD5UpdateFn(NULL), mMD5FinalFn(NULL)
{
	hMod = LoadLibrary(_T("Cryptdll.dll"));
	mMD5InitFn = (MD5Fn)GetProcAddress(hMod, "MD5Init");
	mMD5UpdateFn = (MD5UpdateFn)GetProcAddress(hMod, "MD5Update");
	mMD5FinalFn = (MD5Fn)GetProcAddress(hMod, "MD5Final");
}
CMD5Init::~CMD5Init()
{
	if (hMod)
		FreeLibrary(hMod);
	hMod = NULL;
	mMD5InitFn = mMD5FinalFn = NULL;
	mMD5UpdateFn = NULL;
}
const CMD5Init& CMD5Init::GetInstance()
{
	if (mInstance == NULL)
		mInstance = new CMD5Init;
	return *mInstance;
}
void CMD5Init::ReleaseInstance()
{
	if (mInstance != NULL)
		delete mInstance;
	mInstance = NULL;
}
void CMD5Init::MD5Init(MD5_CTX *md5Ctx) const
{
	if (mMD5InitFn)
		mMD5InitFn(md5Ctx);
}
void CMD5Init::MD5Update(MD5_CTX *md5Ctx, unsigned char *buf, unsigned int len) const
{
	if (mMD5UpdateFn)
		mMD5UpdateFn(md5Ctx, buf, len);
}
void CMD5Init::MD5Final(MD5_CTX *md5Ctx) const
{
	if (mMD5FinalFn)
		mMD5FinalFn(md5Ctx);
}
