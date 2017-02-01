#pragma once

typedef struct {
	ULONG i[2];
	ULONG buf[4];
	unsigned char in[64];
	unsigned char digest[16];
} MD5_CTX;

class CMD5
{
public:
	CMD5();
	~CMD5(void);
	const unsigned char *GetMD5(LPCTSTR fileOrText);
private:
	MD5_CTX md5Ctx;
	HMODULE hMod;
};
