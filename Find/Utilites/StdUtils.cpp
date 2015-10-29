#include "StdAfx.h"
#include "StdUtils.h"
#include <time.h>

lstring GetDateTimeFormat(struct tm &t, LPCTSTR fmt)
{
	TCHAR timeString[256] = {0};
	if (fmt == NULL)
		fmt =  _T("%A %d %B %Y %I:%M:%S %p");
	mktime(&t);
	_tcsftime(timeString, 256, fmt, &t);
	return timeString;
}
lstring GetDateTimeFormat(unsigned time, LPCTSTR fmt)
{
	__time32_t dt(time);
	struct tm t;
	_localtime32_s(&t, &dt);
	return GetDateTimeFormat(t, fmt);
}
lstring GetDateTimeFormat(LPCTSTR time, LPCTSTR fmt)
{
	TCHAR *endPtr(NULL);
	return GetDateTimeFormat(_tcstoul(time, &endPtr, 10), fmt);
}
lstring Get_YYYYMMDD_DateTimeFormat(LPCTSTR time, LPCTSTR fmt)
{
	if (time == NULL || *time == 0)
		return lstring();
	if (fmt == NULL)
		fmt = _T("%A %d %B %Y");
	TCHAR *endPtr(NULL);
	unsigned uTime(_tcstoul(time, &endPtr, 10));
	tm t = {0};
	t.tm_mday = uTime % 100;
	uTime /= 100;
	t.tm_mon = (uTime % 100) -1;
	uTime /= 100;
	t.tm_year = uTime - 1900;
	return GetDateTimeFormat(t, fmt);
}
lstring GetClipboardString()
{
	lstring clipBoardString;
	if (OpenClipboard(NULL)) {
		HANDLE hClip(GetClipboardData(CF_UNICODETEXT));
		if (hClip != NULL) {
			LPCTSTR clipStr((LPCTSTR)GlobalLock(hClip));
			if (clipStr != NULL) {
				clipBoardString = clipStr;
				GlobalUnlock(hClip);
			}
		}
		CloseClipboard();
	}
	return clipBoardString;
}

static lstring WildCardToRegExp(LPCTSTR wildCard)
{
	LPTSTR regExp = new TCHAR[6*lstrlen(wildCard)+1];
	unsigned len = 0;

	while (*wildCard) {
		TCHAR extraCharToAdd = 0;

		switch (*wildCard) {
		case '*':
			extraCharToAdd = '.';
			break;
		case '.':
		case '[':
		case '{':
		case ']':
		case '}':
		case '(':
		case '\\':
			extraCharToAdd = '\\';
			break;
		}
		if (extraCharToAdd)
			regExp[len++] = extraCharToAdd;
		regExp[len++] = *wildCard++;
	}
	regExp[len] = 0;
	lstring regExpStr(regExp);

	delete[] regExp;

	return regExpStr;
}

lstring WildCardExpToRegExp(LPCTSTR wildCardExp)
{
	TCHAR *exp = new TCHAR[lstrlen(wildCardExp)+1];
	lstrcpy(exp, wildCardExp);
	LPTSTR nexttoken(NULL);
	LPTSTR token = _tcstok_s(exp, _T(";"), &nexttoken);
	lstring regExp;
	while (token != NULL) {
		regExp += _T("(") + WildCardToRegExp(token) + _T(")");
		token = _tcstok_s(NULL, _T(";"), &nexttoken);
		if (token != NULL) {
			regExp +=_T("|");
		}
	}
	return regExp;
}
