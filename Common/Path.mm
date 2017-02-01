#include "Path.h"
#import <Cocoa/Cocoa.h>
#include <mach-o/dyld.h>
#include <dlfcn.h>
#include "STLUtils.h"
#include "ProcessUtil.h"

Path::Path()
{
}
Path::Path(const lstring &inPath)
	: lstring(inPath)
{
}
Path::Path(const Path& p)
: lstring(p.c_str())
{
}
Path::Path(LPCTSTR inPath)
: lstring(inPath ? inPath : _T(""))
{
}

#define CH_IS_PATH_SEP(c) ((c)=='/')


static void PathRemoveTralingSeps(TCHAR *path)
{
    if (path && *path) {
        size_t len(lstrlen(path));
        while (--len > 0)
            if (CH_IS_PATH_SEP(path[len]))
                path[len]=0;
            else break;
    }
}
static void PathRemoveFileSpec(TCHAR *path)
{
    PathRemoveTralingSeps(path);
    if (path && *path) {
        size_t len(lstrlen(path));
        while (--len > 0)
            if (!CH_IS_PATH_SEP(path[len]))
                path[len]=0;
            else break;
        PathRemoveTralingSeps(path);
    }
}

Path Path::Parent() const
{
	TCHAR *path = new TCHAR[length()+1];
	_tcscpy_s(path, length()+1, c_str());
	PathRemoveFileSpec(path);
	Path parent(path);
	delete []path;
	return parent;
}

static LPCTSTR PathFindFileName(LPCTSTR path)
{
    LPCTSTR startPath(path);
    if (path && *path) {
        path += lstrlen(path);
        while (path > startPath && CH_IS_PATH_SEP(*path)) --path;
        while (path > startPath && !CH_IS_PATH_SEP(*path)) --path;
        if (CH_IS_PATH_SEP(*path))
            ++path;
    }
    return path;
}

Path Path::FileName() const
{
	return Path(PathFindFileName(c_str()));
}

Path Path::FileNameWithoutExt() const
{
	return FileName().RenameExtension();
}

bool Path::Exists() const
{
    struct stat dStat;
    return stat(c_str(), &dStat) != -1;
    
}
bool Path::IsDir() const
{
    struct stat dStat;
    bool isDir(stat(c_str(), &dStat) != -1);
    
    if (isDir)
        isDir = S_ISDIR(dStat.st_mode);

	return isDir;
}
bool Path::CreateDir() const
{
	bool bSuccess(false);
	if (Exists()) {
		bSuccess = IsDir();
	}
	else {
        @autoreleasepool {
            NSFileManager *fileManager= [NSFileManager defaultManager];
            bSuccess = [fileManager createDirectoryAtPath:[NSString stringWithUTF8String:c_str()] withIntermediateDirectories:YES attributes:nil error:NULL];
        }
	}
	return bSuccess;
}
Path Path::Append(const Path &append) const
{
    const size_t len(length()+append.length()+10);
	TCHAR *newPath = new TCHAR[len];
	_tcscpy_s(newPath, len,c_str());
	if (*newPath == '.') {
		if (newPath[length() - 1] != '/'
			&& append.length() > 0 && append[0] != '/')
			lstrcat(newPath, _T("/"));
		lstrcat(newPath, append.c_str());
	}
    else {
        @autoreleasepool {
            NSString *str = [NSString stringWithUTF8String:c_str()];
            NSString *outStr = [str stringByAppendingPathComponent:[NSString stringWithUTF8String:append.c_str()]];
            _tcscpy_s(newPath, len, [outStr UTF8String]);
        }
    }
	Path appended(newPath);
	delete []newPath;
	return appended;
}
Path Path::CurrentDir()
{
    Path curPath;
    
    @autoreleasepool {
        NSFileManager *fileMan = [NSFileManager defaultManager];
        NSString *curDir = fileMan.currentDirectoryPath;

        curPath = [curDir UTF8String];
    }
    return curPath;
}
bool Path::GetFileTime(LPFILETIME lpCreationTime, LPFILETIME lpLastAccessTime, LPFILETIME lpLastWriteTime) const
{
    struct stat dStat;
    bool bSuccess(stat(c_str(), &dStat) != -1);

	if (bSuccess) {
        if (lpCreationTime)
            *lpCreationTime = dStat.st_ctimespec;
        if (lpLastAccessTime)
            *lpLastAccessTime = dStat.st_atimespec;
        if (lpLastWriteTime)
            *lpLastWriteTime = dStat.st_mtimespec;
	}
	return bSuccess;
}
Path Path::RenameExtension(LPCTSTR newExtn) const
{
    Path outPath;
    @autoreleasepool {
        NSString *str = [NSString stringWithUTF8String:c_str()];
        str = [str stringByDeletingPathExtension];
        if (newExtn && newExtn[0]=='.')
            ++newExtn;
        if (newExtn != NULL && newExtn[0] != 0)
            str = [str stringByAppendingPathExtension:[NSString stringWithUTF8String:newExtn]];
       outPath = [str UTF8String];
    }
	return outPath;
}
Path Path::GetExtension() const
{
    Path outPath;
    @autoreleasepool {
        NSString *str = [NSString stringWithUTF8String:c_str()];
        str = str.pathExtension;
        outPath = [str UTF8String];
        if (!outPath.empty()&&outPath[0]!='.')
            outPath = "."+outPath;
    }
	return outPath;
}
int Path::Compare(const Path &p) const
{
	Path p1(MakeFullPath());
	Path p2(p.MakeFullPath());
	return lstrcmpi(p1.c_str(), p2.c_str());
}
int Path::CompareExtension(LPCTSTR extn) const
{
    if (extn == NULL)
        return -1;
    Path thisExtn(GetExtension());
    LPCTSTR thisExtnStr(thisExtn.c_str());
    if (*thisExtnStr == '.')
        ++thisExtnStr;
    if (*extn == '.')
        ++extn;
    return lstrcmpi(thisExtnStr, extn);
}
Path Path::GetRoot() const
{
    Path rootPath;
    @autoreleasepool {
        NSString *str = [NSString stringWithUTF8String:c_str()];
        auto pathComs = str.pathComponents;
        if (pathComs && pathComs.count > 0) {
            NSMutableArray<NSString *> *newPathCom = [NSMutableArray arrayWithCapacity:1];
            [newPathCom addObject:pathComs.firstObject];
            str = [NSString pathWithComponents:newPathCom];
            rootPath = [str UTF8String];
        }
    }
	return rootPath;
}

bool operator == (const Path& p1, const Path& p2)
{
	return p1.Compare(p2) == 0;
}
bool operator != (const Path& p1, const Path& p2)
{
	return p1.Compare(p2) != 0;
}
Path Path::GetSpecialFolderPath(int inFolderID, bool inCreate)
{
    Path outPath;
    @autoreleasepool {
        NSSearchPathDomainMask domainMask(NSUserDomainMask);
        if (inFolderID & 0xffff0000) {
            domainMask = inFolderID >> 16;
            inFolderID &= 0xffff;
        }
        NSArray *paths = NSSearchPathForDirectoriesInDomains((NSSearchPathDirectory)inFolderID, domainMask, YES);
        if (paths && [paths count] > 0)
            outPath = [[paths firstObject] UTF8String];
    }

	return outPath;
}

Path Path::GetModuleFilePath(HMODULE hModule /*= NULL*/)
{
    Path modulePath;
    if (hModule == NULL) {
        uint32_t size = PATH_MAX;
        char szExeFile[PATH_MAX];
        _NSGetExecutablePath(szExeFile, &size);
        modulePath = szExeFile;

    }
    else {
        Dl_info dlInfo;
        if (dladdr(hModule, &dlInfo))
            modulePath = dlInfo.dli_fname;
    }
    return modulePath;
}

bool Path::IsRelativePath() const
{
	return length() == 0 || at(0) != '/';
}
Path Path::MakeFullPath() const
{
	Path outPath(*this);
	if (IsRelativePath()) {
		outPath = CurrentDir().Append(*this);
	}
	outPath = outPath.Canonicalize();
	return outPath;
}
Path Path::Canonicalize() const
{
    Path outPath;
    @autoreleasepool {
        NSString *str = [NSString stringWithUTF8String:c_str()];
        str = [str stringByStandardizingPath];
        outPath = [str UTF8String];
    }
	return outPath;
}
bool Path::IsPreFixOf(const Path &preFixPath) const
{
	Path curPath(MakeFullPath());
	Path preFix(preFixPath.MakeFullPath());
    bool isPrefix(false);
    @autoreleasepool {
        auto pathComs = [[NSString stringWithUTF8String:curPath.c_str()] pathComponents];
        auto prefixComs = [[NSString stringWithUTF8String:preFixPath.c_str()] pathComponents];
        isPrefix = pathComs.count >= prefixComs.count;
        for (NSUInteger i=0; i<prefixComs.count && isPrefix; ++i)
            isPrefix = [[prefixComs objectAtIndex:i] caseInsensitiveCompare:[pathComs objectAtIndex:i]] == NSOrderedSame;
    }
	return isPrefix;
}
bool
Path::DeleteDirectory() const
{
    bool bSuccess = IsDir();
    
    if (bSuccess) {
        @autoreleasepool {
            bSuccess = [[NSFileManager defaultManager] removeItemAtPath:[NSString stringWithUTF8String:c_str()] error:nil];
        }
    }
	return bSuccess;
}

bool 
Path::SetFileAttributes(DWORD inAttribute ) const
{
    bool bSuccess(false);
    @autoreleasepool {
        NSDictionary *attribs = nil;
        if (inAttribute & 0xffff)
            attribs = [NSDictionary dictionaryWithObject:[NSNumber numberWithShort:inAttribute]
                                                            forKey:NSFilePosixPermissions];
        else
            attribs = [NSDictionary dictionaryWithObject:[NSNumber numberWithShort:(inAttribute >> 16)]
                                                  forKey:NSFileBusy];
        
        bSuccess = [[NSFileManager defaultManager] setAttributes:attribs
                                         ofItemAtPath:[NSString stringWithUTF8String: c_str()] error:nil];
    }
	return bSuccess;
}

bool 
Path::DeleteDirectoryRecursive() const
{
    @autoreleasepool {
        NSFileManager *localFileManager=[[NSFileManager alloc] init];
        NSDirectoryEnumerator *dirEnum = [localFileManager enumeratorAtPath:[NSString stringWithUTF8String:c_str()]];
        
        NSString *file = nil;
        while ((file = [dirEnum nextObject])) {
            file = [file stringByAppendingPathComponent:file];
            Path filePath([file UTF8String]);
            if (filePath.IsDir())
                filePath.DeleteDirectoryRecursive();
            else
                filePath.DeleteFile();
        }
    }
	return DeleteDirectory();
}
bool
Path::DeleteFile() const
{
    bool bRet(false);
    
    @autoreleasepool {
        NSString *filePath = [NSString stringWithUTF8String:c_str()];
        NSFileManager *localFileManager=[[NSFileManager alloc] init];
        bRet = [localFileManager removeItemAtPath:filePath error:nil];
        if (!bRet)
            SetFileAttributes(0777); // set permission
        if (!bRet)
            bRet = [localFileManager removeItemAtPath:filePath error:nil];
        if (!bRet)
            SetFileAttributes(0); // remove read-only
        if (!bRet)
            bRet = [localFileManager removeItemAtPath:filePath error:nil];
    }
    
	return bRet;
}
static lstring IntToString(int no)
{
    lstring outStr;
    STLUtils::ChangeType(outStr, no);
	return outStr;
}

Path Path::GetUniqueFileName(int &statNum, LPCTSTR ext, LPCTSTR prefix) const
{
	Path fileName;
	while (1) {
		fileName.clear();
		if (prefix)
			fileName = prefix;
		fileName += IntToString(statNum);
		if (ext) {
			if (*ext != '.')
				fileName += lstring(_T("."));
			fileName += lstring(ext);
		}
		fileName = Append(fileName);
		++statNum;
		if (!fileName.Exists())
			break;
	}
	return fileName;
}

Path Path::GetNextUniqueFileName() const
{
	if (Exists()) {
		int nextFileName(0);
		return Parent().GetUniqueFileName(nextFileName, GetExtension().c_str(), FileNameWithoutExt().c_str());
	}
	return *this;
}

static int FindCallBack_DeleteFolder(FindData& findData, void *pUserParam)
{
	int &nFilesDeleted(*(int*)pUserParam);
    Path filePath(findData.fullPath);
	if (findData.pFindData == NULL) {// exit of dir
        if (filePath.DeleteDirectory())
			++nFilesDeleted;
	}
	else if (findData.fileMatched && !filePath.IsDir()) {
		if (filePath.DeleteFile())
			++nFilesDeleted;
	}
	return 0;
}
int Path::Delete(bool bRecusrive /*= false*/, LPCTSTR pattern /*= NULL*/, LPCTSTR excludePattern /*= NULL*/) const
{
	int nFilesDeleted(0);
	if (IsDir()) {
		if (bRecusrive) {
			Finder rd(FindCallBack_DeleteFolder, &nFilesDeleted, pattern, excludePattern);
			rd.StartFind(*this);
		}
		if (DeleteDirectory())
			++nFilesDeleted;
	}
	else if (DeleteFile())
		++nFilesDeleted;
	return nFilesDeleted;
}

static int FindCallBack_PathGetSize(FindData &findData, void *pUserParam)
{
	if (findData.pFindData != NULL) {
        Path path(findData.fullPath);
        if (!path.IsDir())
            *((INT64*)pUserParam) += path.GetSize();
	}
	return 0;
}
INT64 Path::GetSize() const
{
	INT64 fileSize(-1);
	if (IsDir()) {
		fileSize = 0;
		Path root(GetRoot());
		if (root == *this) {
            @autoreleasepool {
                NSNumber *nsfileSize = [[[NSFileManager defaultManager] attributesOfFileSystemForPath:[NSString stringWithUTF8String:c_str()] error:nil] valueForKey:NSFileSystemSize];
                fileSize = nsfileSize.longLongValue;
            }
		}
		else
			Finder(FindCallBack_PathGetSize, &fileSize).StartFind(c_str());
	}
	else {
        @autoreleasepool {
            NSNumber *nsfileSize = [[[NSFileManager defaultManager] attributesOfItemAtPath:[NSString stringWithUTF8String:c_str()] error:nil] valueForKey:NSFileSize];
            fileSize = nsfileSize.unsignedLongLongValue;
        }
	}
	return fileSize;
}

bool Path::CreateShortCut(const Path &shortCutPath, LPCTSTR pszTargetargs /*= NULL*/, LPCTSTR pszDescription /*= NULL*/, int iShowmode /*= 0*/, LPCTSTR pszCurdir /*= NULL*/, LPCTSTR pszIconfile /*= NULL*/, int iIconindex /*= 0*/) const
{
    bool bSuccess(false);
    @autoreleasepool {
        bSuccess = [[NSFileManager defaultManager] createSymbolicLinkAtPath:
                    [NSString stringWithUTF8String:shortCutPath.c_str()]
                    withDestinationPath:[NSString stringWithUTF8String:c_str()] error:nil];
    }
	return bSuccess;
}

ULONGLONG Path::GetFileTime(FileTimeType fileType) const
{
	FILETIME fileTime[3] = { {0} };
	GetFileTime(fileTime, fileTime + 1, fileTime + 2);
	if (fileType < CreationTime || fileType > ModifiedTime)
		fileType = CreationTime;
	ULONGLONG outFileTime(fileTime[fileType].tv_nsec);
	return outFileTime;
}

bool Path::Move(const Path & inNewLocation) const
{
    bool bSuccess(false);
    @autoreleasepool {
       bSuccess = [[NSFileManager defaultManager] moveItemAtPath:[NSString stringWithUTF8String:c_str()] toPath:[NSString stringWithUTF8String:inNewLocation.c_str()] error:nil];
    }
    return bSuccess;
}
bool Path::CopyFile(const Path &newFilePath) const
{
    if (IsDir())
        return false;
    if (!Exists())
        return false;
    Path inNewLocation(newFilePath);
    if (newFilePath.Exists()) {
        if (newFilePath.IsDir())
            inNewLocation =  newFilePath.Append(FileName());
    }
    if (inNewLocation.Exists())
        inNewLocation.DeleteFile();
    bool bSuccess(false);
    @autoreleasepool {
        bSuccess = [[NSFileManager defaultManager] copyItemAtPath:[NSString stringWithUTF8String:c_str()] toPath:[NSString stringWithUTF8String:inNewLocation.c_str()] error:nil];
    }
    return bSuccess;
}


lstring WildCardToRegExp(LPCTSTR wildCard)
{
	TCHAR *regExp = new TCHAR[6 * lstrlen(wildCard) + 1];
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
		if (isalpha(*wildCard)) {
			regExp[len++] = '[';
			regExp[len++] = tolower(*wildCard);
			regExp[len++] = toupper(*wildCard++);
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
    const size_t len(lstrlen(wildCardExp) + 1);
	TCHAR *exp = new TCHAR[len];
	_tcscpy_s(exp, len, wildCardExp);
	LPTSTR nexttoken(NULL);
	LPTSTR token = strtok_r(exp, _T(";"), &nexttoken);
	lstring regExp;
	while (token != NULL) {
		regExp += _T("(") + WildCardToRegExp(token) + _T(")");
		token = strtok_r(NULL, _T(";"), &nexttoken);
		if (token != NULL) {
			regExp += _T("|");
		}
	}
	return regExp;
}


long long FindData::GetFileSize() const
{
    return Path(fullPath).GetSize();
}

Finder::Finder(FindCallBack fcb, void *pUserParam, LPCTSTR inpattern, LPCTSTR excludePattern)
	: mExcludePattern(excludePattern)
{
	lstring pat;
	if (inpattern)
		pat = inpattern;
	if (pat.empty())
		pat = _T("*");
    mRegExp.assign(WildCardExpToRegExp(pat.c_str()).c_str(), std::regex_constants::icase);
	if (mExcludePattern)
		mExcludeRegExp.assign(WildCardExpToRegExp(mExcludePattern).c_str(), std::regex_constants::icase);
	m_pUserParam = pUserParam;
	mFindCallBack = fcb;
}
int Finder::StartFind(const Path &dir)
{
	int c = 0;
    @autoreleasepool {
        NSString *file = nil;
        Path srcDir(dir);
        bool bSrcIsFile((!srcDir.IsDir() && srcDir.Exists()));
        if (bSrcIsFile) {
            file = [NSString stringWithUTF8String:srcDir.FileName().c_str()];
            srcDir = srcDir.Parent();
        }
        NSFileManager *localFileManager=[[NSFileManager alloc] init];
        NSDirectoryEnumerator *dirEnum = [localFileManager enumeratorAtURL:[NSURL fileURLWithPath: [NSString stringWithUTF8String:dir.c_str()] isDirectory:!bSrcIsFile] includingPropertiesForKeys:nil options:NSDirectoryEnumerationSkipsSubdirectoryDescendants errorHandler:nil];
        
        if (!file)
            file = ((NSURL*)[dirEnum nextObject]).path.lastPathComponent;
        if (file != nil || bSrcIsFile) {
            do {
                lstring fileName([file UTF8String]);
                if (strcmp(fileName.c_str(), _T(".")) && strcmp(fileName.c_str(), _T(".."))) {
                    Path filePath = srcDir.Append(fileName);
                    std::smatch base_match;
                    bool bMatched = std::regex_match(fileName, base_match, mRegExp);
                    if (mExcludePattern)
                        bMatched = bMatched && std::regex_match(fileName, base_match, mExcludeRegExp);
                    FindData::PLAT_FIND_DATA pd = {};
                    FindData fd(&pd, filePath, bMatched);
                    int fcbRetVal(mFindCallBack(fd, m_pUserParam));
                    if (fcbRetVal == FCBRV_ABORT)
                        break;
                    if (filePath.IsDir()
                        && fcbRetVal != FCBRV_SKIPDIR) {
                        c += StartFind(filePath);
                        fd.pFindData = NULL;
                        fd.fileMatched = false;
                        mFindCallBack(fd, m_pUserParam);
                    }
                    if (bMatched)
                        c++;
                }
            } while ((file = ((NSURL*)[dirEnum nextObject]).path.lastPathComponent));
        }
    }
	return c;
}
