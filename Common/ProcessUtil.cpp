// CreateProcessAsUser.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "ProcessUtil.h"
#include "Common.h"
#include <Shellapi.h>
#pragma comment(lib, "advapi32.lib")
//#pragma comment(lib, "Shell32.lib")

static BOOL GetLogonSID (HANDLE hToken, PSID *ppsid) 
{
   BOOL bSuccess = FALSE;
   DWORD dwIndex;
   DWORD dwLength = 0;
   PTOKEN_GROUPS ptg = NULL;

// Verify the parameter passed in is not NULL.
    if (NULL == ppsid)
        goto Cleanup;

// Get required buffer size and allocate the TOKEN_GROUPS buffer.

   if (!GetTokenInformation(
         hToken,         // handle to the access token
         TokenGroups,    // get information about the token's groups 
         (LPVOID) ptg,   // pointer to TOKEN_GROUPS buffer
         0,              // size of buffer
         &dwLength       // receives required buffer size
      )) 
   {
      if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) 
         goto Cleanup;

      ptg = (PTOKEN_GROUPS)HeapAlloc(GetProcessHeap(),
         HEAP_ZERO_MEMORY, dwLength);

      if (ptg == NULL)
         goto Cleanup;
   }

// Get the token group information from the access token.

   if (!GetTokenInformation(
         hToken,         // handle to the access token
         TokenGroups,    // get information about the token's groups 
         (LPVOID) ptg,   // pointer to TOKEN_GROUPS buffer
         dwLength,       // size of buffer
         &dwLength       // receives required buffer size
         )) 
   {
      goto Cleanup;
   }

// Loop through the groups to find the logon SID.

   for (dwIndex = 0; dwIndex < ptg->GroupCount; dwIndex++) 
      if ((ptg->Groups[dwIndex].Attributes & SE_GROUP_LOGON_ID)
             ==  SE_GROUP_LOGON_ID) 
      {
      // Found the logon SID; make a copy of it.

         dwLength = GetLengthSid(ptg->Groups[dwIndex].Sid);
         *ppsid = (PSID) HeapAlloc(GetProcessHeap(),
                     HEAP_ZERO_MEMORY, dwLength);
         if (*ppsid == NULL)
             goto Cleanup;
         if (!CopySid(dwLength, *ppsid, ptg->Groups[dwIndex].Sid)) 
         {
             HeapFree(GetProcessHeap(), 0, (LPVOID)*ppsid);
             goto Cleanup;
         }
         break;
      }

   bSuccess = TRUE;

Cleanup: 

// Free the buffer for the token groups.

   if (ptg != NULL)
      HeapFree(GetProcessHeap(), 0, (LPVOID)ptg);

   return bSuccess;
}
static VOID FreeLogonSID (PSID *ppsid) 
{
    HeapFree(GetProcessHeap(), 0, (LPVOID)*ppsid);
}


#define DESKTOP_ALL (DESKTOP_READOBJECTS | DESKTOP_CREATEWINDOW | \
DESKTOP_CREATEMENU | DESKTOP_HOOKCONTROL | DESKTOP_JOURNALRECORD | \
DESKTOP_JOURNALPLAYBACK | DESKTOP_ENUMERATE | DESKTOP_WRITEOBJECTS | \
DESKTOP_SWITCHDESKTOP | STANDARD_RIGHTS_REQUIRED)

#define WINSTA_ALL (WINSTA_ENUMDESKTOPS | WINSTA_READATTRIBUTES | \
WINSTA_ACCESSCLIPBOARD | WINSTA_CREATEDESKTOP | \
WINSTA_WRITEATTRIBUTES | WINSTA_ACCESSGLOBALATOMS | \
WINSTA_EXITWINDOWS | WINSTA_ENUMERATE | WINSTA_READSCREEN | \
STANDARD_RIGHTS_REQUIRED)

#define GENERIC_ACCESS (GENERIC_READ | GENERIC_WRITE | \
GENERIC_EXECUTE | GENERIC_ALL)

BOOL AddAceToWindowStation(HWINSTA hwinsta, PSID psid);

BOOL AddAceToDesktop(HDESK hdesk, PSID psid);

static BOOL StartInteractiveClientProcess (
    LPTSTR lpszUsername,    // client to log on
    LPTSTR lpszDomain,      // domain of client's account
    LPTSTR lpszPassword,    // client's password
    LPTSTR lpCommandLine    // command line to execute
) 
{
   HANDLE      hToken;
   HDESK       hdesk = NULL;
   HWINSTA     hwinsta = NULL, hwinstaSave = NULL;
   PROCESS_INFORMATION pi = {0};
   PSID pSid = NULL;
   STARTUPINFO si = {sizeof(STARTUPINFO), 0};
   BOOL bResult = FALSE;

// Log the client on to the local computer.

   if (!LogonUser(
           lpszUsername,
           lpszDomain,
           lpszPassword,
           LOGON32_LOGON_INTERACTIVE,
           LOGON32_PROVIDER_DEFAULT,
           &hToken) ) 
   {
      goto Cleanup;
   }

// Save a handle to the caller's current window station.

   if ( (hwinstaSave = GetProcessWindowStation() ) == NULL)
      goto Cleanup;

// Get a handle to the interactive window station.

   hwinsta = OpenWindowStation(
       _T("winsta0"),                   // the interactive window station 
       FALSE,                       // handle is not inheritable
       READ_CONTROL | WRITE_DAC);   // rights to read/write the DACL

   if (hwinsta == NULL) 
      goto Cleanup;

// To get the correct default desktop, set the caller's 
// window station to the interactive window station.

   if (!SetProcessWindowStation(hwinsta))
      goto Cleanup;

// Get a handle to the interactive desktop.

   hdesk = OpenDesktop(
      _T("default"),     // the interactive window station 
      0,             // no interaction with other desktop processes
      FALSE,         // handle is not inheritable
      READ_CONTROL | // request the rights to read and write the DACL
      WRITE_DAC | 
      DESKTOP_WRITEOBJECTS | 
      DESKTOP_READOBJECTS);

// Restore the caller's window station.

   if (!SetProcessWindowStation(hwinstaSave)) 
      goto Cleanup;

   if (hdesk == NULL) 
      goto Cleanup;

// Get the SID for the client's logon session.

   if (!GetLogonSID(hToken, &pSid)) 
      goto Cleanup;

// Allow logon SID full access to interactive window station.

   if (! AddAceToWindowStation(hwinsta, pSid) ) 
      goto Cleanup;

// Allow logon SID full access to interactive desktop.

  // if (! AddAceToDesktop(hdesk, pSid) ) 
   //   goto Cleanup;

// Impersonate client to ensure access to executable file.

   if (! ImpersonateLoggedOnUser(hToken) ) 
      goto Cleanup;

// Initialize the STARTUPINFO structure.
// Specify that the process runs in the interactive desktop.

   ZeroMemory(&si, sizeof(STARTUPINFO));
   si.cb= sizeof(STARTUPINFO);
   si.lpDesktop = TEXT("winsta0\\default");

// Launch the process in the client's logon session.

   bResult = CreateProcessAsUser(
      hToken,            // client's access token
      NULL,              // file to execute
      lpCommandLine,     // command line
      NULL,              // pointer to process SECURITY_ATTRIBUTES
      NULL,              // pointer to thread SECURITY_ATTRIBUTES
      FALSE,             // handles are not inheritable
      NORMAL_PRIORITY_CLASS | CREATE_NEW_CONSOLE,   // creation flags
      NULL,              // pointer to new environment block 
      NULL,              // name of current directory 
      &si,               // pointer to STARTUPINFO structure
      &pi                // receives information about new process
   ); 
   DWORD err = GetLastError();
// End impersonation of client.

   RevertToSelf();

   if (bResult && pi.hProcess != INVALID_HANDLE_VALUE) 
   { 
      WaitForSingleObject(pi.hProcess, INFINITE); 
      CloseHandle(pi.hProcess); 
   } 

   if (pi.hThread != INVALID_HANDLE_VALUE)
      CloseHandle(pi.hThread);  

Cleanup: 

   if (hwinstaSave != NULL)
      SetProcessWindowStation (hwinstaSave);

// Free the buffer for the logon SID.

   if (pSid)
      FreeLogonSID(&pSid);

// Close the handles to the interactive window station and desktop.

   if (hwinsta)
      CloseWindowStation(hwinsta);

   if (hdesk)
      CloseDesktop(hdesk);

// Close the handle to the client's access token.

   if (hToken != INVALID_HANDLE_VALUE)
      CloseHandle(hToken);  

   return bResult;
}

static BOOL AddAceToWindowStation(HWINSTA hwinsta, PSID psid)
{
   ACCESS_ALLOWED_ACE   *pace;
   ACL_SIZE_INFORMATION aclSizeInfo;
   BOOL                 bDaclExist;
   BOOL                 bDaclPresent;
   BOOL                 bSuccess = FALSE;
   DWORD                dwNewAclSize;
   DWORD                dwSidSize = 0;
   DWORD                dwSdSizeNeeded;
   PACL                 pacl;
   PACL                 pNewAcl;
   PSECURITY_DESCRIPTOR psd = NULL;
   PSECURITY_DESCRIPTOR psdNew = NULL;
   PVOID                pTempAce;
   SECURITY_INFORMATION si = DACL_SECURITY_INFORMATION;
   unsigned int         i;

   __try
   {
      // Obtain the DACL for the window station.

      if (!GetUserObjectSecurity(
             hwinsta,
             &si,
             psd,
             dwSidSize,
             &dwSdSizeNeeded)
      )
      if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
      {
         psd = (PSECURITY_DESCRIPTOR)HeapAlloc(
               GetProcessHeap(),
               HEAP_ZERO_MEMORY,
               dwSdSizeNeeded);

         if (psd == NULL)
            __leave;

         psdNew = (PSECURITY_DESCRIPTOR)HeapAlloc(
               GetProcessHeap(),
               HEAP_ZERO_MEMORY,
               dwSdSizeNeeded);

         if (psdNew == NULL)
            __leave;

         dwSidSize = dwSdSizeNeeded;

         if (!GetUserObjectSecurity(
               hwinsta,
               &si,
               psd,
               dwSidSize,
               &dwSdSizeNeeded)
         )
            __leave;
      }
      else
         __leave;

      // Create a new DACL.

      if (!InitializeSecurityDescriptor(
            psdNew,
            SECURITY_DESCRIPTOR_REVISION)
      )
         __leave;

      // Get the DACL from the security descriptor.

      if (!GetSecurityDescriptorDacl(
            psd,
            &bDaclPresent,
            &pacl,
            &bDaclExist)
      )
         __leave;

      // Initialize the ACL.

      ZeroMemory(&aclSizeInfo, sizeof(ACL_SIZE_INFORMATION));
      aclSizeInfo.AclBytesInUse = sizeof(ACL);

      // Call only if the DACL is not NULL.

      if (pacl != NULL)
      {
         // get the file ACL size info
         if (!GetAclInformation(
               pacl,
               (LPVOID)&aclSizeInfo,
               sizeof(ACL_SIZE_INFORMATION),
               AclSizeInformation)
         )
            __leave;
      }

      // Compute the size of the new ACL.

      dwNewAclSize = aclSizeInfo.AclBytesInUse +
            (2*sizeof(ACCESS_ALLOWED_ACE)) + (2*GetLengthSid(psid)) -
            (2*sizeof(DWORD));

      // Allocate memory for the new ACL.

      pNewAcl = (PACL)HeapAlloc(
            GetProcessHeap(),
            HEAP_ZERO_MEMORY,
            dwNewAclSize);

      if (pNewAcl == NULL)
         __leave;

      // Initialize the new DACL.

      if (!InitializeAcl(pNewAcl, dwNewAclSize, ACL_REVISION))
         __leave;

      // If DACL is present, copy it to a new DACL.

      if (bDaclPresent)
      {
         // Copy the ACEs to the new ACL.
         if (aclSizeInfo.AceCount)
         {
            for (i=0; i < aclSizeInfo.AceCount; i++)
            {
               // Get an ACE.
               if (!GetAce(pacl, i, &pTempAce))
                  __leave;

               // Add the ACE to the new ACL.
               if (!AddAce(
                     pNewAcl,
                     ACL_REVISION,
                     MAXDWORD,
                     pTempAce,
                    ((PACE_HEADER)pTempAce)->AceSize)
               )
                  __leave;
            }
         }
      }

      // Add the first ACE to the window station.

      pace = (ACCESS_ALLOWED_ACE *)HeapAlloc(
            GetProcessHeap(),
            HEAP_ZERO_MEMORY,
            sizeof(ACCESS_ALLOWED_ACE) + GetLengthSid(psid) -
                  sizeof(DWORD));

      if (pace == NULL)
         __leave;

      pace->Header.AceType  = ACCESS_ALLOWED_ACE_TYPE;
      pace->Header.AceFlags = CONTAINER_INHERIT_ACE |
                   INHERIT_ONLY_ACE | OBJECT_INHERIT_ACE;
      pace->Header.AceSize  = (WORD)(sizeof(ACCESS_ALLOWED_ACE) +
                   GetLengthSid(psid) - sizeof(DWORD));
      pace->Mask            = GENERIC_ACCESS;

      if (!CopySid(GetLengthSid(psid), &pace->SidStart, psid))
         __leave;

      if (!AddAce(
            pNewAcl,
            ACL_REVISION,
            MAXDWORD,
            (LPVOID)pace,
            pace->Header.AceSize)
      )
         __leave;

      // Add the second ACE to the window station.

      pace->Header.AceFlags = NO_PROPAGATE_INHERIT_ACE;
      pace->Mask            = WINSTA_ALL;

      if (!AddAce(
            pNewAcl,
            ACL_REVISION,
            MAXDWORD,
            (LPVOID)pace,
            pace->Header.AceSize)
      )
         __leave;

      // Set a new DACL for the security descriptor.

      if (!SetSecurityDescriptorDacl(
            psdNew,
            TRUE,
            pNewAcl,
            FALSE)
      )
         __leave;

      // Set the new security descriptor for the window station.

      if (!SetUserObjectSecurity(hwinsta, &si, psdNew))
         __leave;

      // Indicate success.

      bSuccess = TRUE;
   }
   __finally
   {
      // Free the allocated buffers.

      if (pace != NULL)
         HeapFree(GetProcessHeap(), 0, (LPVOID)pace);

      if (pNewAcl != NULL)
         HeapFree(GetProcessHeap(), 0, (LPVOID)pNewAcl);

      if (psd != NULL)
         HeapFree(GetProcessHeap(), 0, (LPVOID)psd);

      if (psdNew != NULL)
         HeapFree(GetProcessHeap(), 0, (LPVOID)psdNew);
   }

   return bSuccess;

}

static BOOL AddAceToDesktop(HDESK hdesk, PSID psid)
{
   ACL_SIZE_INFORMATION aclSizeInfo;
   BOOL                 bDaclExist;
   BOOL                 bDaclPresent;
   BOOL                 bSuccess = FALSE;
   DWORD                dwNewAclSize;
   DWORD                dwSidSize = 0;
   DWORD                dwSdSizeNeeded;
   PACL                 pacl;
   PACL                 pNewAcl;
   PSECURITY_DESCRIPTOR psd = NULL;
   PSECURITY_DESCRIPTOR psdNew = NULL;
   PVOID                pTempAce;
   SECURITY_INFORMATION si = DACL_SECURITY_INFORMATION;
   unsigned int         i;

   __try
   {
      // Obtain the security descriptor for the desktop object.

      if (!GetUserObjectSecurity(
            hdesk,
            &si,
            psd,
            dwSidSize,
            &dwSdSizeNeeded))
      {
         if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
         {
            psd = (PSECURITY_DESCRIPTOR)HeapAlloc(
                  GetProcessHeap(),
                  HEAP_ZERO_MEMORY,
                  dwSdSizeNeeded );

            if (psd == NULL)
               __leave;

            psdNew = (PSECURITY_DESCRIPTOR)HeapAlloc(
                  GetProcessHeap(),
                  HEAP_ZERO_MEMORY,
                  dwSdSizeNeeded);

            if (psdNew == NULL)
               __leave;

            dwSidSize = dwSdSizeNeeded;

            if (!GetUserObjectSecurity(
                  hdesk,
                  &si,
                  psd,
                  dwSidSize,
                  &dwSdSizeNeeded)
            )
               __leave;
         }
         else
            __leave;
      }

      // Create a new security descriptor.

      if (!InitializeSecurityDescriptor(
            psdNew,
            SECURITY_DESCRIPTOR_REVISION)
      )
         __leave;

      // Obtain the DACL from the security descriptor.

      if (!GetSecurityDescriptorDacl(
            psd,
            &bDaclPresent,
            &pacl,
            &bDaclExist)
      )
         __leave;

      // Initialize.

      ZeroMemory(&aclSizeInfo, sizeof(ACL_SIZE_INFORMATION));
      aclSizeInfo.AclBytesInUse = sizeof(ACL);

      // Call only if NULL DACL.

      if (pacl != NULL)
      {
         // Determine the size of the ACL information.

         if (!GetAclInformation(
               pacl,
               (LPVOID)&aclSizeInfo,
               sizeof(ACL_SIZE_INFORMATION),
               AclSizeInformation)
         )
            __leave;
      }

      // Compute the size of the new ACL.

      dwNewAclSize = aclSizeInfo.AclBytesInUse +
            sizeof(ACCESS_ALLOWED_ACE) +
            GetLengthSid(psid) - sizeof(DWORD);

      // Allocate buffer for the new ACL.

      pNewAcl = (PACL)HeapAlloc(
            GetProcessHeap(),
            HEAP_ZERO_MEMORY,
            dwNewAclSize);

      if (pNewAcl == NULL)
         __leave;

      // Initialize the new ACL.

      if (!InitializeAcl(pNewAcl, dwNewAclSize, ACL_REVISION))
         __leave;

      // If DACL is present, copy it to a new DACL.

      if (bDaclPresent)
      {
         // Copy the ACEs to the new ACL.
         if (aclSizeInfo.AceCount)
         {
            for (i=0; i < aclSizeInfo.AceCount; i++)
            {
               // Get an ACE.
               if (!GetAce(pacl, i, &pTempAce))
                  __leave;

               // Add the ACE to the new ACL.
               if (!AddAce(
                  pNewAcl,
                  ACL_REVISION,
                  MAXDWORD,
                  pTempAce,
                  ((PACE_HEADER)pTempAce)->AceSize)
               )
                  __leave;
            }
         }
      }

      // Add ACE to the DACL.

      if (!AddAccessAllowedAce(
            pNewAcl,
            ACL_REVISION,
            DESKTOP_ALL,
            psid)
      )
         __leave;

      // Set new DACL to the new security descriptor.

      if (!SetSecurityDescriptorDacl(
            psdNew,
            TRUE,
            pNewAcl,
            FALSE)
      )
         __leave;

      // Set the new security descriptor for the desktop object.

      if (!SetUserObjectSecurity(hdesk, &si, psdNew))
         __leave;

      // Indicate success.

      bSuccess = TRUE;
   }
   __finally
   {
      // Free buffers.

      if (pNewAcl != NULL)
         HeapFree(GetProcessHeap(), 0, (LPVOID)pNewAcl);

      if (psd != NULL)
         HeapFree(GetProcessHeap(), 0, (LPVOID)psd);

      if (psdNew != NULL)
         HeapFree(GetProcessHeap(), 0, (LPVOID)psdNew);
   }

   return bSuccess;
}

bool ProcessUtil::IsUserAdmin()
	/*++ 
	Routine Description: This routine returns TRUE if the caller's
	process is a member of the Administrators local group. Caller is NOT
	expected to be impersonating anyone and is expected to be able to
	open its own process and process token. 
	Arguments: None. 
	Return Value: 
	TRUE - Caller has Administrators local group. 
	FALSE - Caller does not have Administrators local group. --
	*/ 
{
	SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
	PSID AdministratorsGroup; 
	BOOL b = AllocateAndInitializeSid(
		&NtAuthority,
		2,
		SECURITY_BUILTIN_DOMAIN_RID,
		DOMAIN_ALIAS_RID_ADMINS,
		0, 0, 0, 0, 0, 0,
		&AdministratorsGroup); 
	if(b) 
	{
		if (!CheckTokenMembership( NULL, AdministratorsGroup, &b)) 
		{
			b = FALSE;
		} 
		FreeSid(AdministratorsGroup); 
	}

	return b != FALSE;
}

typedef	 BOOL 
	(__stdcall *CreateProcessWithTokenWFn)(
	__in        HANDLE hToken,
	__in        DWORD dwLogonFlags,
	__in_opt    LPCWSTR lpApplicationName,
	__inout_opt LPWSTR lpCommandLine,
	__in        DWORD dwCreationFlags,
	__in_opt    LPVOID lpEnvironment,
	__in_opt    LPCWSTR lpCurrentDirectory,
	__in        LPSTARTUPINFOW lpStartupInfo,
	__out       LPPROCESS_INFORMATION lpProcessInformation
	);

static CreateProcessWithTokenWFn Get_CreateProcessWithTokenWFn()
{
	CreateProcessWithTokenWFn fn((CreateProcessWithTokenWFn)GetProcAddress(LoadLibrary(L"Advapi32.dll"), "CreateProcessWithTokenW"));

	return fn;
}

static LPCTSTR GetExecutableArgFromCommandLine(LPCTSTR commandLine, LPTSTR outExecutable = NULL, DWORD *pExeLength = NULL)
{
    if (commandLine == NULL)
        return commandLine;
    TCHAR skiptTill = ' ';
    if (*commandLine == '"') {
        ++commandLine;
        skiptTill = '"';
    }
    DWORD len(0), srcLen(pExeLength ? *pExeLength : MAX_PATH);
    while (*commandLine && *commandLine != skiptTill) {
        ++len;
        if (outExecutable && srcLen > 1) {
            *outExecutable = *commandLine;
            ++outExecutable;
            --srcLen;
        }
        ++commandLine;
    }
    ++len;
    if (outExecutable) {
        *outExecutable = 0;
        if (pExeLength)
            *pExeLength = MAX_PATH - srcLen;
    }
    else if (pExeLength)
        *pExeLength = len;
    while (*commandLine && *commandLine == skiptTill)
        ++commandLine;
    return *commandLine ? commandLine : NULL;
}

bool ProcessUtil::RunApplication(LPCTSTR commandLine,
    unsigned uRAF /*= RAF_BLOCKING*/, unsigned long *outExitCode /*= NULL*/)
{
	bool isSuccessful(false);
	STARTUPINFO	StartupInfo = {sizeof(STARTUPINFO), 0};
	PROCESS_INFORMATION ProcessInfo = {0};
	unsigned long exitCode=2;



	DWORD creationFlag = 0;
	if (uRAF & RAF_NOWINDOW) {
		creationFlag |= CREATE_NO_WINDOW;
	}
	CreateProcessWithTokenWFn fnCreateProcessWithTokenW(Get_CreateProcessWithTokenWFn());
	if (!(uRAF & RAF_ADMIN)  && IsUserAdmin() && fnCreateProcessWithTokenW) {
		HANDLE hTokenHandle=NULL;
		HWND hwndShell = FindWindow(_T("Progman"), NULL);
		DWORD dwProcessId(0);
		GetWindowThreadProcessId(hwndShell, &dwProcessId);
		HANDLE hProcessShell = OpenProcess(MAXIMUM_ALLOWED, FALSE, dwProcessId);
		bool bSuccess(false);
		if  (hProcessShell != NULL) {
			bSuccess = ::OpenProcessToken(hProcessShell,TOKEN_DUPLICATE | TOKEN_ASSIGN_PRIMARY | TOKEN_QUERY,&hTokenHandle) == TRUE;
			CloseHandle(hProcessShell);
		}
		if (bSuccess)
		{
			if (hTokenHandle)
			{
				HANDLE hTokenShell(NULL);
				bSuccess = DuplicateTokenEx(hTokenHandle, MAXIMUM_ALLOWED, NULL, SecurityDelegation, TokenPrimary, &hTokenShell) == TRUE;
				CloseHandle(hTokenHandle);
				if (bSuccess)
				{
					if (fnCreateProcessWithTokenW(hTokenShell, 0, NULL, (LPWSTR)commandLine,creationFlag,NULL,NULL, &StartupInfo, &ProcessInfo) )
					{
						isSuccessful = true;					
					}
					exitCode = GetLastError();
					CloseHandle(hTokenShell);
				}
			}
		}
	}
    else if ((uRAF & RAF_ADMIN) && !IsUserAdmin()) {
        TCHAR exePath[MAX_PATH] = { 0 };
        LPCTSTR arg = GetExecutableArgFromCommandLine(commandLine, exePath);
        SHELLEXECUTEINFO shInfo = { sizeof(SHELLEXECUTEINFO) };
        shInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
        shInfo.lpVerb = _T("runas");
        shInfo.lpFile = exePath;
        shInfo.lpParameters = arg;
        shInfo.nShow = creationFlag & CREATE_NO_WINDOW ? SW_HIDE : SW_SHOWDEFAULT;
        isSuccessful = ShellExecuteEx(&shInfo) == TRUE;
        ProcessInfo.hProcess = shInfo.hProcess;
    }
	else if(0 != CreateProcess( NULL,(LPWSTR) commandLine, NULL, NULL, FALSE, creationFlag,
		NULL, NULL, &StartupInfo, &ProcessInfo))
	{
		isSuccessful = true;
	}
	if (isSuccessful) {
		if (uRAF & RAF_BLOCKING) {
			WaitForSingleObject(ProcessInfo.hProcess, INFINITE);
			GetExitCodeProcess( ProcessInfo.hProcess, &exitCode);
		}
	}

	// Close process and thread handles. 	
    if (ProcessInfo.hProcess)
	    CloseHandle( ProcessInfo.hProcess );
    if (ProcessInfo.hThread)
	    CloseHandle( ProcessInfo.hThread );

    if (outExitCode)
        *outExitCode = exitCode;

	return isSuccessful;
}

bool ProcessUtil::RunApplication(int argc, LPCTSTR * argv,
    unsigned uRAF /*= RAF_BLOCKING*/, unsigned long *outExitCode /*= NULL*/)
{
    lstring commandLine;
    for (int i = 0;i < argc; ++i) {
        lstring arg(argv[i]);
        if (arg.length() > 0) {
            lstring quote;
            if (arg[0] != '"' && arg.find(' ') != lstring::npos)
                quote = _T("\"");
            if (commandLine.length() > 0)
                commandLine += _T(" ");
            commandLine += quote + arg + quote;
        }
    }
    return RunApplication(commandLine.c_str(), uRAF, outExitCode);
}

int ProcessUtil::GetCurrentProcessId()
{
    return (int)::GetCurrentProcessId();
}

int ProcessUtil::GetCurrentThreadId()
{
    return (int)::GetCurrentThreadId();
}

void ProcessUtil::Sleep(unsigned milliSeconds)
{
    ::Sleep(milliSeconds);
}

unsigned long long ProcessUtil::GetTickCount()
{
    return ::GetTickCount64();
}
