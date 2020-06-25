// mp3info.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#define MAX_ID3V1_STR_LEN 30

struct ID3V1 {
	char header[3];
	char title[MAX_ID3V1_STR_LEN];
	char artist[MAX_ID3V1_STR_LEN];
	char album[MAX_ID3V1_STR_LEN];
	char year[4];
	char comment[MAX_ID3V1_STR_LEN];
	char genere;
};
const char *generes[] = {
	"Blues","Alternative","AlternRock","Top",
	"Classic","Rock","Ska","Bass","Christian","Rap",
	"Country","Death","Metal","Soul","Pop/Funk",
	"Dance","Pranks","Punk","Jungle",
	"Disco","Soundtrack","Space","Native","American",
	"Funk","Euro-Techno","Meditative","Cabaret",
	"Grunge","Ambient","Instrumental","Pop","New","Wave",
	"Hip-Hop","Trip-Hop","Instrumental","Rock","Psychadelic",
	"Jazz","Vocal","Ethnic","Rave",
	"Metal","Jazz+Funk","Gothic","Showtunes",
	"New","Age","Fusion","Darkwave","Trailer",
	"Oldies","Trance","Techno-Industrial","Lo-Fi",
	"Other","Classical","Electronic","Tribal",
	"Pop","Instrumental","Pop-Folk","Acid","Punk",
	"R&B","Acid","Eurodance","Acid","Jazz",
	"Rap","House","Dream","Polka",
	"Reggae","Game","Southern","Rock","Retro",
	"Rock","Sound","Clip","Comedy","Musical",
	"Techno","Gospel","Cult","Rock & Roll",
	"Industrial","Noise","Gangsta","Hard","Rock",

	"Folk","Progressive","Rock","Chamber","Music","Ballad",
	"Folk-Rock","Psychedelic","Rock","Sonata","Poweer","Ballad",
	"National","Folk","Symphonic","Rock","Symphony","Rhytmic","Soul",
	"Swing","Slow","Rock","Booty","Brass","Freestyle",
	"Fast","Fusion","Big","Band","Primus","Duet",
	"Bebob","Chorus","Porn","Groove","Punk","Rock",
	"Latin","Easy","Listening","Satire","Drum","Solo",
	"Revival","Acoustic","Slow","Jam","A","Capela",
	"Celtic","Humour","Club","Euro-House",
	"Bluegrass","Speech","Tango","Dance","Hall",
	"Avantgarde","Chanson","Samba",
	"Gothic","Rock","Opera","Folklore"
};
char GetGenereFromString(const char *cArgVal)
{
	char genere(0);
	if (isdigit(*cArgVal)) {
		genere = atoi(cArgVal);
		if (genere < 0 || genere > ARRAY_SIZE(generes))
			genere = 0;
	}
	else for(int i = 0; i <ARRAY_SIZE(generes); ++i) {
		if (_stricmp(cArgVal, generes[i]) == 0) {
			genere = i;
			break;
		}
	}
	return genere;
}
const char* GetStringFromGenere(char genere)
{
	if (genere < 0 || genere > ARRAY_SIZE(generes))
		genere = 0;
	return generes[genere];
}
std::wstring
UTF8ToUnicodeString(const std::string &sUTF8String)
{
	std::wstring		sRet;
	if (!sUTF8String.empty())
	{
		int	kAllocate = MultiByteToWideChar(CP_UTF8, 0, sUTF8String.c_str(), sUTF8String.length(), NULL, 0);
		if (kAllocate)
		{
			std::vector<wchar_t> vecWide(kAllocate);
			
			int kCopied = MultiByteToWideChar(CP_UTF8, 0, sUTF8String.c_str(), sUTF8String.length(), &vecWide[0], vecWide.size());
			if (kCopied)
			{
				sRet.assign(&vecWide[0], vecWide.size());
			}
		}
	}
	return sRet;
}
std::string	
UnicodeToUTF8String(const std::wstring &swUnicodeString)
{
	std::string sRet;
	if (!swUnicodeString.empty())
	{
		int kMultiByteLength = WideCharToMultiByte(CP_UTF8, 0, swUnicodeString.data(), swUnicodeString.length(), 0, 0, NULL, NULL);
		std::vector<char> vecChar(kMultiByteLength);
		if( WideCharToMultiByte(CP_UTF8, 0, swUnicodeString.data(), swUnicodeString.length(), &vecChar[0], vecChar.size(), NULL, NULL))
		{
			sRet.assign(&vecChar[0], vecChar.size());
		}
	}
	return sRet;
}

lstring WildCardToRegExp(LPCTSTR wildCard)
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
			extraCharToAdd = '\\';
			break;
		}
		if (extraCharToAdd)
			regExp[len++] = extraCharToAdd;
		if (_istalpha(*wildCard)) {
			regExp[len++] = '[';
			regExp[len++] = _totlower(*wildCard);
			regExp[len++] = _totupper(*wildCard++);
			regExp[len++] = ']';
		}
		else
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
struct FindData {
	WIN32_FIND_DATA *pFindData;
	const lstring &fullPath;
	bool fileMatched;
	FindData(WIN32_FIND_DATA *pFD, const lstring &fp, bool fm)
		: pFindData(pFD), fullPath(fp), fileMatched(fm)
	{}
};
#define FCBRV_CONTINUE 0
#define FCBRV_ABORT 1
#define FCBRV_SKIPDIR 2
#define FCBRV_STOP 3
typedef int (*FindCallBack)(FindData&, void *pUserParam);
struct Finder {
	Finder(FindCallBack fcb, void *pUserParam = NULL, LPCTSTR pattern = NULL, LPCTSTR excludePattern = NULL);
	int StartFind(const lstring &dir);
	CAtlRegExp<> mRegExp;
	CAtlRegExp<> mExcludeRegExp;
	LPCTSTR mExcludePattern;
	FindCallBack mFindCallBack;
	void *m_pUserParam;
};

Finder::Finder(FindCallBack fcb, void *pUserParam, LPCTSTR inpattern, LPCTSTR excludePattern)
: mExcludePattern(excludePattern)
{
	lstring pat;
	if (inpattern)
		pat = inpattern;
	if (pat.empty())
		pat = _T("*");
	mRegExp.Parse(WildCardExpToRegExp(pat.c_str()).c_str(), FALSE);
	if (mExcludePattern)
		mExcludeRegExp.Parse(WildCardExpToRegExp(mExcludePattern).c_str(), FALSE);
	m_pUserParam = pUserParam;
	mFindCallBack = fcb;
}
int Finder::StartFind(const lstring &dir)
{
	WIN32_FIND_DATA findFileData;
	HANDLE hFind = FindFirstFile((dir+_T("\\*")).c_str(), &findFileData);
	int c = 0;
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if (lstrcmp(findFileData.cFileName, _T(".")) && lstrcmp(findFileData.cFileName, _T(".."))) {
				CAtlREMatchContext<> mc;
				lstring file = dir + _T("\\");
				file += findFileData.cFileName;
				bool bMatched = mRegExp.Match(findFileData.cFileName, &mc) == TRUE;
				if (mExcludePattern)
					bMatched = bMatched && mExcludeRegExp.Match(findFileData.cFileName, &mc) == FALSE;
				int fcbRetVal(mFindCallBack(FindData(&findFileData, file, bMatched), m_pUserParam));
				while (fcbRetVal == FCBRV_STOP) {
					fcbRetVal = mFindCallBack(FindData(&findFileData, file, bMatched), m_pUserParam);
					Sleep(10);
				}
				if (fcbRetVal == FCBRV_ABORT)
					break;
				if ((findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
					&& fcbRetVal != FCBRV_SKIPDIR) {
					c += StartFind(file);
					mFindCallBack(FindData(NULL, file, false), m_pUserParam);
				}
				if (bMatched)
					c++;
			}
		} while (FindNextFile(hFind, &findFileData));
		FindClose(hFind);
	}
	return c;
}

class Path : public lstring
{
public:
	Path()
	{
	}
	Path(const lstring &inPath)
		: lstring(inPath)
	{
	}
	Path Parent() const
	{
		TCHAR *path = new TCHAR[length()+1];
		lstrcpy(path, c_str());
		PathRemoveFileSpec(path);
		Path parent(path);
		delete []path;
		return parent;
	}
	Path FileName() const
	{
		return Path(PathFindFileName(c_str()));
	}
	bool Exists() const
	{
		return PathFileExists(c_str()) == TRUE;
	}
	bool IsDir() const
	{
		DWORD dwRet(::GetFileAttributes(c_str()));
		return (INVALID_FILE_ATTRIBUTES != dwRet)
			&& ((FILE_ATTRIBUTE_DIRECTORY & dwRet) > 0);
	}
	bool CreateDir() const
	{
		bool bSuccess(false);
		if (Exists()) {
			bSuccess = IsDir();
		}
		else {
			bSuccess = SHCreateDirectoryEx(NULL, c_str(), NULL) == ERROR_SUCCESS;
		}
		return bSuccess;
	}
	Path Append(const Path &append) const
	{
		TCHAR *newPath = new TCHAR[length()+append.length()+10];
		lstrcpy(newPath, c_str());
		PathAppend(newPath, append.c_str());
		Path appended(newPath);
		delete []newPath;
		return appended;
	}
	static Path CurrentDir()
	{
		TCHAR curDir[4*MAX_PATH];
		curDir[0] = 0;
		GetCurrentDirectory(4*MAX_PATH, curDir);
		return Path(curDir);
	}
	bool GetFileTime(LPFILETIME lpCreationTime, LPFILETIME lpLastAccessTime, LPFILETIME lpLastWriteTime) const
	{
		bool bSuccess(false);
		HANDLE hFile(CreateFile(c_str(), GENERIC_READ, FILE_SHARE_READ, NULL,
			OPEN_EXISTING, 0, NULL));
		if (hFile != INVALID_HANDLE_VALUE) {
			bSuccess = ::GetFileTime(hFile, lpCreationTime, lpLastAccessTime, lpLastWriteTime) == TRUE;
			CloseHandle(hFile);
		}
		return bSuccess;
	}
	Path RemoveExtension() const
	{
		TCHAR *newPath = new TCHAR[length()+1];
		lstrcpy(newPath, c_str());
		::PathRemoveExtension(newPath);
		Path path(newPath);
		delete []newPath;
		return path;
	}
};


int FindArg(int argc, _TCHAR* argv[], LPCTSTR arg)
{
	int len(lstrlen(arg));
	while (--argc > 0) {
		if (_tcsncicmp(argv[argc], arg, len) == 0)
			break;
	}
	return argc;
}
LPCTSTR FindArgValue(int argc, _TCHAR* argv[], LPCTSTR arg, LPCTSTR seperator = _T("="))
{
	LPCTSTR argValue(NULL);
	argc = FindArg(argc, argv, arg);
	if (argc != 0) {
		argValue = _T("");
		LPCTSTR argVal(argv[argc]+lstrlen(arg));
		int lenSep(lstrlen(seperator));
		if (lstrlen(argVal) > lenSep) {
			argValue = argVal+lenSep;
		}
	}
	return argValue;
}
int FindNextArg(int argc, _TCHAR* argv[], int startArg)
{
	while (++startArg < argc) {
		if (argv[startArg][0] != '-')
			break;
	}
	if (startArg >= argc)
		startArg =0;
	return startArg;
}
void Help()
{
	_tprintf(_T("mp3info <path> [-t[=title]] [-a=artist] [-b[=album]] [-y=year] [-c=comment] [-g=genere]\n"));
	_tprintf(_T("-t[=title] set the title name. if no title is given the file name is used\n"));
	_tprintf(_T("[-a=artist] set the artist name.\n"));
	_tprintf(_T("[-b[=album]] set the album name. if no album name is given the folder name is used\n"));
	_tprintf(_T("[-y=year] set the year xxxx e.g 1998.\n"));
	_tprintf(_T("[-a=artist] Update comment.\n"));
	_tprintf(_T("[-g=genere] genere is either 0-125 or any string.\n"));
}
#define FLAGBIT(n) (1<<(n))
#define UMF_TITLE FLAGBIT(0)
#define UMF_ARTIST FLAGBIT(1)
#define UMF_ALBUM FLAGBIT(2)
#define UMF_YEAR FLAGBIT(3)
#define UMF_COMMENT FLAGBIT(4)

struct UpdateID3V1Data {
	ID3V1 id3v1;
	unsigned updateflag;
	UpdateID3V1Data()
		: updateflag(0)
	{
		ZeroMemory(&id3v1, sizeof(ID3V1));
	}
};

static std::string GetStringFromArray(const char *str, int maxCount)
{
	char outStr[256];
	strcpy_s(outStr, ARRAY_SIZE(outStr), str);
	while (outStr[maxCount-1] == ' ')
		--maxCount;
	outStr[maxCount] = 0;
	return std::string(outStr);
}
static void SetStringToArray(char *dstString, const char *srcString, int maxCount)
{
	strcpy_s(dstString, maxCount, srcString);
	int srcLen = strlen(srcString);
	if (srcLen < maxCount) {
		memset(dstString+srcLen, 0, maxCount-srcLen);
	}
}
#define GetString(a) GetStringFromArray(a, ARRAY_SIZE(a))
#define SetString(d,s) SetStringToArray(d, s, ARRAY_SIZE(d))

void PrintID3V1(const Path &path, const ID3V1 &id3v1)
{
	_tprintf(_T("file:%s\n"), path.c_str());
	printf(" title:%s\n artist:%s\n album:%s\n year:%s\n comment:%s\n genere:%s\n",
		GetString(id3v1.title).c_str(), GetString(id3v1.artist).c_str(),
		GetString(id3v1.album).c_str(), GetString(id3v1.year).c_str(),
		GetString(id3v1.comment).c_str(), GetStringFromGenere(id3v1.genere));
}

bool GetMP3InfoInFile(const Path &path, ID3V1 &outID3V1)
{
	bool bSuccess(false);
	FILE *fp(NULL);
	_tfopen_s(&fp, path.c_str(), _T("rb"));
	if (fp != NULL) {
		if (_fseeki64(fp, -(int)sizeof(ID3V1), SEEK_END) != -1) { // read last 128 bytes
			if (fread(&outID3V1, sizeof(ID3V1), 1, fp) == 1
				&& !memcmp(outID3V1.header, "TAG", 3)) { // correct tag
				bSuccess = true;
			}
		}
		fclose(fp);
	}
	return bSuccess;
}
bool SetMP3InfoInFile(const Path &path, const ID3V1 &inID3V1)
{
	bool bSuccess(!strncmp(inID3V1.header, "TAG", 3));
	if (bSuccess) {
		bSuccess = false;
		FILE *fp(NULL);
		_tfopen_s(&fp, path.c_str(), _T("rb+"));
		if (fp != NULL) {
			if (_fseeki64(fp, -(int)sizeof(ID3V1), SEEK_END) != -1) { // write last 128 bytes
				if (fwrite(&inID3V1, sizeof(ID3V1), 1, fp) == 1) {
					bSuccess = true;
				}
			}
			fclose(fp);
		}
	}
	return bSuccess;
}

int UpdateMP3InfoInFile(const Path &path, UpdateID3V1Data *pUpdateID3V1Data = NULL)
{
	ID3V1 id3v1 = {0};
	if (GetMP3InfoInFile(path, id3v1)) {
		PrintID3V1(path, id3v1);
		if (pUpdateID3V1Data != NULL && pUpdateID3V1Data->updateflag) {
			for (int i = 0; i < 6; ++i) {
				if (!(FLAGBIT(i) & pUpdateID3V1Data->updateflag))
					continue;
				switch(i) {
				case 0: // title
					{
						std::string title(pUpdateID3V1Data->id3v1.title);
						if (title.empty()) {
							title = UnicodeToUTF8String(path.FileName().RemoveExtension());
						}
						SetString(id3v1.title, title.c_str());
					}
					break;
				case 1: // artist
					SetString(id3v1.artist, pUpdateID3V1Data->id3v1.artist);
					break;
				case 2: // album
					{
						std::string album(pUpdateID3V1Data->id3v1.album);
						if (album.empty()) {
							album = UnicodeToUTF8String(path.Parent().FileName());
						}
						SetString(id3v1.album, album.c_str());
					}
					break;
				case 3: // year
					SetString(id3v1.year, pUpdateID3V1Data->id3v1.year);
					break;
				case 4: // comment
					SetString(id3v1.comment, pUpdateID3V1Data->id3v1.comment);
					break;
				case 5: // genere
					id3v1.genere = pUpdateID3V1Data->id3v1.genere;
					break;
				}
			}
			if (!SetMP3InfoInFile(path, id3v1)) {
				_tprintf(_T("Cannot update info in file %s\nerror:%d"), path.c_str(), GetLastError());
			}
		}
	}
	return 0;
}

int FindCallBack_UpdateMP3Info(FindData &fd, void *pUserParam)
{
	if (fd.fileMatched && !(fd.pFindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
		UpdateMP3InfoInFile(fd.fullPath, (UpdateID3V1Data *)pUserParam);
	}
	return 0;
}

int UpdateMP3Info(const Path &path, UpdateID3V1Data *pUpdateID3V1Data = NULL)
{
	int retVal(0);
	if (path.IsDir()) {
		Finder finder(FindCallBack_UpdateMP3Info, pUpdateID3V1Data, _T(".mp3"));
		retVal = finder.StartFind(path);
	}
	else {
		retVal = UpdateMP3InfoInFile(path, pUpdateID3V1Data);
	}
	return 0;
}
int _tmain(int argc, _TCHAR* argv[])
{
	if (argc < 2 || !lstrcmpi(argv[1], _T("/?"))) {
		Help();
		return -1;
	}
	int nextArg = FindNextArg(argc, argv, 0);
	if (nextArg == 0) {
		_tprintf(_T("file/folder no given\n"));
		Help();
		return -2;
	}
	LPCTSTR path(argv[nextArg]);
	UpdateID3V1Data updateID3V1data;
	LPCTSTR argNeeded[] = {_T("-t"), _T("-a"), _T("-b"), _T("-y"), _T("-c"), _T("-g")};
	for (int i = 0; i < ARRAY_SIZE(argNeeded); ++i) {
		LPCTSTR argVal = FindArgValue(argc, argv, argNeeded[i]);
		if (argVal == NULL)
			continue;
		std::string argValue(UnicodeToUTF8String(argVal));
		const char *cArgVal(argValue.c_str());
		updateID3V1data.updateflag |= FLAGBIT(i);
		switch(i) {
		case 0: // title
			strncpy_s(updateID3V1data.id3v1.title, cArgVal, ARRAY_SIZE(updateID3V1data.id3v1.title));
			break;
		case 1: // artist
			strncpy_s(updateID3V1data.id3v1.artist, cArgVal, ARRAY_SIZE(updateID3V1data.id3v1.artist));
			break;
		case 2: // album
			strncpy_s(updateID3V1data.id3v1.album, cArgVal, ARRAY_SIZE(updateID3V1data.id3v1.album));
			break;
		case 3: // year
			strncpy_s(updateID3V1data.id3v1.year, cArgVal, ARRAY_SIZE(updateID3V1data.id3v1.year));
			break;
		case 4: // comment
			strncpy_s(updateID3V1data.id3v1.comment, cArgVal, ARRAY_SIZE(updateID3V1data.id3v1.comment));
			break;
		case 5: // genere
			updateID3V1data.id3v1.genere = GetGenereFromString(cArgVal);
			break;
		}
	}
	return UpdateMP3Info(Path(path), &updateID3V1data);
}

