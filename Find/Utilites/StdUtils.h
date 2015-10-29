#pragma once
#include <time.h>

lstring GetDateTimeFormat(struct tm &t, LPCTSTR fmt = NULL);
lstring GetDateTimeFormat(unsigned time, LPCTSTR fmt = NULL);
lstring GetDateTimeFormat(LPCTSTR time, LPCTSTR fmt = NULL);
lstring Get_YYYYMMDD_DateTimeFormat(LPCTSTR time, LPCTSTR fmt = NULL);
lstring GetClipboardString();
//lstring WildCardExpToRegExp(LPCTSTR wildCardExp);
