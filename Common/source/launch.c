
/******************************************************************************

    UserLand Frontier(tm) -- High performance Web content management,
    object database, system-level and Internet scripting environment,
    including source code editing and debugging.

    Copyright (C) 1992-2004 UserLand Software, Inc.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

******************************************************************************/

#include "frontier.h"
#include "standard.h"

#ifdef MACVERSION
#include <land.h>
#include "mac.h"
#endif

#include "error.h"
#include "memory.h"
#include "strings.h"
#include "file.h"
#include "launch.h"
#include "threads.h"

#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/
	#include "aeutils.h"
#endif

#ifdef MACVERSION

typedef struct typrocessvisitinfo { /*2002-11-14 AR: for providing context to visitprocesses callbacks*/
	OSType idprocess;
	short ctprocesses;
	short nthprocess;
	ptrstring bsprocess;
	typrocessid *psnprocess;
	tyfilespec *fsprocess;
	} typrocessvisitinfo;
	
typedef struct typrocessvisitinfo *typrocessvisitinfoptr;

#define initprocessvisitinfo(x) \
	do {						\
	(x)->idprocess = 0L;		\
	(x)->ctprocesses = 0;		\
	(x)->nthprocess = 0;		\
	(x)->bsprocess = NULL;		\
	(x)->psnprocess = NULL;		\
	(x)->fsprocess = NULL;		\
	} while(0)

#endif

tylaunchcallbacks launchcallbacks = {nil};

#define maxwait 120 /*never wait more than 2 seconds for a stransition to occur*/

#ifdef flsystem6

typedef struct system6launchparamblock {
	StringPtr				name;
	unsigned short			reserved1;
	unsigned short			launchBlockID;
	unsigned long			launchEPBLength;
	unsigned short			launchFileFlags;
	LaunchFlags				launchControlFlags;
	unsigned short			launchVRefNum;
} system6launchparamblock;

#endif


#ifdef MACVERSION
typedef AppParameters **AppParametersHandle;
#endif


#ifdef WIN95VERSION
typedef struct tyWINPROCESSINFO
	{
	struct tyWINPROCESSINFO ** next;
	bigstring ModName;
	bigstring ExeName;
	DWORD processID;
	HWND windowHandle;
	boolean isWindowVisible;
	} WINPROCESSINFO;

typedef struct tyWINPROCESSHEADER
	{
	long count;
	long platform;			//0 for win95; 1 for winNT
	long windowCount;
	HWND windowList[1024];
	WINPROCESSINFO ** head;
	} WINPROCESSHEADER;


// Type definitions for pointers to call tool help functions. 
typedef BOOL (WINAPI *MODULEWALK)(HANDLE hSnapshot, LPMODULEENTRY32 lpme);
typedef BOOL (WINAPI *PROCESSWALK)(HANDLE hSnapshot, LPPROCESSENTRY32 lppe); 
typedef HANDLE (WINAPI *CREATESNAPSHOT)(DWORD dwFlags, DWORD th32ProcessID); 
 
// File scope globals. These pointers are declared because of the need 
// to dynamically link to the functions.  They are exported only by 
// the Windows 95 kernel. Explicitly linking to them will make this 
// application unloadable in Microsoft(R) Windows NT(TM) and will 
// produce an ugly system dialog box. 
static CREATESNAPSHOT pCreateToolhelp32Snapshot = NULL; 
static MODULEWALK  pModule32First  = NULL;
static MODULEWALK  pModule32Next   = NULL;
static PROCESSWALK pProcess32First = NULL; 
static PROCESSWALK pProcess32Next  = NULL; 
 
// Function that initializes tool help functions. 
static BOOL InitToolhelp32 (void) 
	{ 
    BOOL   bRet  = FALSE; 
    HANDLE hKernel = NULL; 
 
	if ((pCreateToolhelp32Snapshot != NULL) && 
		(pModule32First != NULL) && (pModule32Next != NULL) &&
		(pProcess32First != NULL) && (pProcess32Next != NULL))
		return (true);  /*already done*/


    // Obtain the module handle of the kernel to retrieve addresses of 
    // the tool helper functions. 
    hKernel = GetModuleHandle("KERNEL32.DLL"); 
 
    if (hKernel) { 
        pCreateToolhelp32Snapshot = (CREATESNAPSHOT)GetProcAddress(hKernel, "CreateToolhelp32Snapshot"); 
 
        pModule32First  = (MODULEWALK)GetProcAddress(hKernel, "Module32First");
        pModule32Next   = (MODULEWALK)GetProcAddress(hKernel, "Module32Next");

        pProcess32First = (PROCESSWALK)GetProcAddress(hKernel, "Process32First"); 
        pProcess32Next  = (PROCESSWALK)GetProcAddress(hKernel, "Process32Next"); 
 
        // All addresses must be non-NULL to be successful. 
        // If one of these addresses is NULL, one of 
        // the needed lists cannot be walked. 
        bRet =  pProcess32First && 
                pProcess32Next &&
				pModule32First &&
				pModule32Next &&
                pCreateToolhelp32Snapshot; 
		} 
    else 
        bRet = FALSE; // could not even get the module handle of kernel 
 
    return bRet; 
	} /*InitToolhelp32*/


// Type definitions for pointers to call PSAPI. 
typedef BOOL (WINAPI *PSAPIENUMPROCESS)(DWORD * lpidProcess, DWORD cb, DWORD * cbNeeded);
typedef BOOL (WINAPI *PSAPIENUMPROCESSMODULES)(HANDLE hProcess, HMODULE * lphModule, DWORD cb, LPDWORD lpcbNeeded); 
typedef DWORD (WINAPI *PSAPIGETMODULENAME)(HANDLE hProcess, HMODULE hModule, LPSTR lpName, DWORD nSize); 
 
// File scope globals. These pointers are declared because of the need 
// to dynamically link to the functions.  They are exported only by 
// the Windows NT PSAPI.DLL. Explicitly linking to them will make this 
// application unloadable in Microsoft(R) Windows 95(TM) and will 
// produce an ugly system dialog box. 
static PSAPIENUMPROCESS pEnumProcesses = NULL; 
static PSAPIENUMPROCESSMODULES  pEnumProcessModules  = NULL;
static PSAPIGETMODULENAME pGetModuleBaseName   = NULL;
static PSAPIGETMODULENAME pGetModuleFileNameEx = NULL; 

// Function that initializes Process Apps functions. 
static BOOL InitToolhelpNT (void) 
	{ 
    BOOL   bRet  = FALSE; 
    HANDLE hKernel = NULL; 
 
	if ((pEnumProcesses != NULL) && 
		(pEnumProcessModules != NULL) && (pGetModuleBaseName != NULL) &&
		(pGetModuleFileNameEx != NULL))
		return (true);  /*already done*/


    // Obtain the module handle of the kernel to retrieve addresses of 
    // the tool helper functions. 
    hKernel = LoadLibrary("PSAPI.DLL"); 
 
    if (hKernel){ 
        pEnumProcesses = (PSAPIENUMPROCESS)GetProcAddress(hKernel, "EnumProcesses"); 
        pEnumProcessModules  = (PSAPIENUMPROCESSMODULES)GetProcAddress(hKernel, "EnumProcessModules");

        pGetModuleBaseName = (PSAPIGETMODULENAME)GetProcAddress(hKernel, "GetModuleBaseNameA"); 
        pGetModuleFileNameEx  = (PSAPIGETMODULENAME)GetProcAddress(hKernel, "GetModuleFileNameExA"); 
 
        // All addresses must be non-NULL to be successful. 
        // If one of these addresses is NULL, one of 
        // the needed lists cannot be walked. 
        bRet =  pEnumProcesses && 
                pEnumProcessModules &&
				pGetModuleBaseName &&
				pGetModuleFileNameEx; 
    } 
    else 
        bRet = FALSE; // could not even get the module handle of kernel 
 
    return bRet; 
} 



//
//  FUNCTION: GetModuleNameFromExe(LPCSTR, LPSTR, WORD)
//
//  PURPOSE:  Retrieves the module name of a Win16 app or DLL from its
//            excutable file.
//
//  PARAMETERS:
//    szFileName   - Executable file (.EXE or .DLL) from which to retrieve 
//                   module name
//    szModuleName - Points to buffer that receives the module name
//    cbLen        - Specifies maximum length of szModuleName including NULL
//
//  RETURN VALUE:
//    TRUE if the module name was succesfully copied into szModuleName.
//    FALSE if it wasn't killed.
//
//  COMMENTS:
//    Works for Win16 New Executable files only.
//

static BOOL GetModuleNameFromExe (LPCSTR szFileName, LPSTR szModuleName, WORD cbLen)
{

#ifdef _MSC_VER

    BOOL              bResult      = FALSE;
    HANDLE            hFile        = NULL;
    HANDLE            hFileMapping = NULL;
    PIMAGE_OS2_HEADER pNEHdr       = NULL;
    PIMAGE_DOS_HEADER pDosExeHdr   = NULL;

    // Open the file as read-only.  (This file may be opened already by the
    // system if it is an application or DLL that is currently loaded.)
    // Create a read-only file mapping and map a read-only view of the file.
    // If we can't open the file for some reason, then return FALSE.
  
    hFile = CreateFile(szFileName,
                       GENERIC_READ,
                       FILE_SHARE_READ|FILE_SHARE_WRITE,
                       NULL,
                       OPEN_EXISTING,
                       0,
                       NULL);
 
    if (hFile == INVALID_HANDLE_VALUE)
        return FALSE;
 
    hFileMapping = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
    if (hFileMapping == NULL)
    {
        verify (CloseHandle(hFile));
        return FALSE;
    }
 
    pDosExeHdr = (PIMAGE_DOS_HEADER)MapViewOfFile (hFileMapping,
                                                   FILE_MAP_READ,
                                                   0,
                                                   0,
                                                   0);
    if (!pDosExeHdr)
    {
        verify (CloseHandle(hFileMapping));
        verify (CloseHandle(hFile));
        return FALSE;
    }
 
    __try
    {
        // Go to the beginning of the NE header.
        pNEHdr =
           (PIMAGE_OS2_HEADER)((LPSTR)pDosExeHdr + pDosExeHdr -> e_lfanew);
 
        // Check to make sure that the file has DOS and NE EXE headers
        if (pDosExeHdr -> e_magic == IMAGE_DOS_SIGNATURE
            && pNEHdr -> ne_magic == IMAGE_OS2_SIGNATURE)
        {
            lstrcpyn (szModuleName, (LPSTR)pNEHdr + pNEHdr -> ne_restab +1,  
                      min((BYTE)*((LPSTR)pNEHdr + pNEHdr -> ne_restab) + 1, 
                          cbLen));
            bResult = TRUE;
        }
        else
            bResult = FALSE;
 
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        // If an access violation occurs in the try block, we know the file
        // is not a NE file because it is too small to have a NE header, or
        // the offset of the NE header isn't close to being correct.
        bResult = FALSE;
    }
 
    // Clean up file mapping, all views of file mapping, and close the
    // file.
    UnmapViewOfFile(pDosExeHdr);
    verify (CloseHandle(hFileMapping));
    verify (CloseHandle(hFile));
 
    return bResult;

#else
	
	/*
	2002-11-10 AR: The Metrowerks Codewarrior compiler for x86 does not appear
	to support the __try/__except statements used in the implementation above.
	Since it is probably not a good idea to use the same file-mapped i/o code
	as above without being able to catch access violations, we simply rewrote
	the code to use simple SetFilePointer and ReadFile calls.
	*/

	BOOL				bResult		= FALSE;
	HANDLE				hFile		= NULL;
	IMAGE_DOS_HEADER	sDosHeader;
	IMAGE_OS2_HEADER	sOs2Header;
	DWORD				nResult;
	BYTE				nLength;
	long				nBytesToRead;
	long				nBytesRead;	

	// Open the file as read-only.  (This file may be opened already by the
	// system if it is an application or DLL that is currently loaded.)
	// If we can't open the file for some reason, then return FALSE.

	hFile = CreateFile(szFileName,
						GENERIC_READ,
						FILE_SHARE_READ|FILE_SHARE_WRITE,
						NULL,
						OPEN_EXISTING,
						0,
						NULL);
 
	if (hFile == INVALID_HANDLE_VALUE)
		return FALSE;

	/* read IMAGE_DOS_HEADER from beginning of file */
	
	nBytesToRead = sizeof(sDosHeader);
	
	nResult = ReadFile (hFile, &sDosHeader, nBytesToRead, &nBytesRead, NULL);
	
	if (!nResult || nBytesRead != nBytesToRead)
		goto EXIT;

	/* make sure the data has got the magic signature */
	
	if (sDosHeader.e_magic != IMAGE_DOS_SIGNATURE)
		goto EXIT;
	
	/* read IMAGE_OS2_HEADER */
	
	nResult = SetFilePointer (hFile, sDosHeader.e_lfanew, NULL, FILE_BEGIN);
	
	if (nResult == INVALID_SET_FILE_POINTER)
		goto EXIT;
	
	nBytesToRead = sizeof(sOs2Header);

	nResult = ReadFile (hFile, &sOs2Header, nBytesToRead, &nBytesRead, NULL);
	
	if (!nResult || nBytesRead != nBytesToRead)
		goto EXIT;

	/* make sure the data has got the magic signature */
	
	if (sOs2Header.ne_magic != IMAGE_OS2_SIGNATURE)
		goto EXIT;
	
	/* read module name from file, first byte is length of name excl. trailing nil */
	
	nResult = SetFilePointer (hFile, sDosHeader.e_lfanew + sOs2Header.ne_restab, NULL, FILE_BEGIN);
	
	if (nResult == INVALID_SET_FILE_POINTER)
		goto EXIT;
	
	nBytesToRead = 1;

	nResult = ReadFile (hFile, &nLength, nBytesToRead, &nBytesRead, NULL);
	
	if (!nResult || nBytesRead != nBytesToRead)
		goto EXIT;

	nBytesToRead = min(nLength+1, cbLen);
	
	nResult = ReadFile (hFile, &szModuleName, nBytesToRead, &nBytesRead, NULL);
	
	if (!nResult || nBytesRead != nBytesToRead)
		goto EXIT;
	
	bResult = TRUE;

EXIT:

	verify (CloseHandle(hFile));

	return bResult;

#endif
}
 

//
//  FUNCTION: GetProcessModule(DWORD, DWORD, LPMODULEENTRY32, DWORD)
//
//  PURPOSE:  Given a Process ID and module ID, return its module information.
//
//  PARAMETERS:
//    dwPID      - ID of process that owns the module we want information 
//                 about.
//    dwModuleID - ToolHelp32 ID of the module within the process
//    lpMe32     - Structure to return data about the module we want
//    cbMe32     - Size of the buffer pointed to by lpMe32--to make sure we 
//                 don't copy too much data into lpMe32.
//
//  RETURN VALUE:
//    TRUE if it returns information about the specifed module.
//    FALSE if it could not enumerate the modules in the process, or the
//          module is not found in the process.
//
//  COMMENTS:
//

static BOOL GetProcessModule (DWORD           dwPID, 
                       DWORD           dwModuleID, 
                       LPMODULEENTRY32 lpMe32, 
                       DWORD           cbMe32)
{   
    BOOL          bRet        = FALSE;
    BOOL          bFound      = FALSE;
    HANDLE        hModuleSnap = NULL;
    MODULEENTRY32 me32        = {0};

    // Take a snapshot of all modules in the specified process.
    hModuleSnap = pCreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwPID);
    if (hModuleSnap == (HANDLE)-1)
        return (FALSE);

    // Size of the MODULEENTRY32 structure must be initialized before use!
    me32.dwSize = sizeof(MODULEENTRY32);

    // Walk the module list of the process and find the module we are 
    // interested in.  Then, copy the information to the buffer pointed to
    // by lpMe32 so that we can return it to the caller.
    if (pModule32First(hModuleSnap, &me32))
    {
        do 
        {
            if (me32.th32ModuleID == dwModuleID)
            {
                CopyMemory (lpMe32, &me32, cbMe32);
                bFound = TRUE;
            }
        }
        while (!bFound && pModule32Next(hModuleSnap, &me32));

        bRet = bFound;   // If this sets bRet to FALSE, then dwModuleID 
                         // no longer exsists in the specified process.
    }
    else
        bRet = FALSE;    // Couldn't walk module list.

    // Don't forget to clean up the snapshot object...
    verify (CloseHandle (hModuleSnap));

    return (bRet);
	} /*GetProcessModule*/


static WINPROCESSINFO ** getprocessinfofrompid (WINPROCESSHEADER * ph, DWORD pid) {
	WINPROCESSINFO ** entry;

	entry = ph->head;

	while (entry != NULL) {
		if ((**entry).processID == pid)
			return (entry);

		entry = (**entry).next;
		}

	return (NULL);
	} /*getprocessinfofrompid*/


//static WINPROCESSINFO ** getprocessinfofromhwnd (WINPROCESSHEADER * ph, HWND hwnd) {
//	WINPROCESSINFO ** entry;
//
//	entry = ph->head;
//
//	while (entry != NULL) {
//		if ((**entry).windowHandle == hwnd)
//			return (entry);
//
//		entry = (**entry).next;
//		}
//
//	return (NULL);
//	} /*getprocessinfofromhwnd*/
//

static WINPROCESSINFO ** getprocessinfofromwintitle (WINPROCESSHEADER * ph, bigstring bsname) {
	WINPROCESSINFO ** entry;
	char wintitle[258];
	short i;
	DWORD pid;

	releasethreadglobals ();

	for (i = 0; i < ph->windowCount; i++) {
		if (GetWindowText (ph->windowList[i], stringbaseaddress (wintitle), 256)) {
			setstringlength (wintitle, strlen(stringbaseaddress(wintitle)));

			if (equalidentifiers (wintitle, bsname)) {
				GetWindowThreadProcessId (ph->windowList[i], &pid);

				entry = getprocessinfofrompid (ph, pid);

				if (entry != NULL) {
					(**entry).windowHandle = ph->windowList[i];
					(**entry).isWindowVisible = IsWindowVisible (ph->windowList[i]);
					}

				grabthreadglobals ();
				return (entry);
				}
			}
		}

	grabthreadglobals ();
	return (NULL);
	} /*getprocessinfofromwintitle*/


static WINPROCESSINFO ** getprocessinfofrommodname (WINPROCESSHEADER * ph, bigstring bsname) {
	WINPROCESSINFO ** entry;

	entry = ph->head;

	while (entry != NULL) {
		if (equalidentifiers ((**entry).ModName, bsname))
			return (entry);

		entry = (**entry).next;
		}

	return (NULL);
	} /*getprocessinfofrommodname*/


static WINPROCESSINFO ** getprocessinfofromexepath (WINPROCESSHEADER * ph, bigstring bspath) {
	WINPROCESSINFO ** entry;

	entry = ph->head;

	while (entry != NULL) {
		if (equalidentifiers ((**entry).ExeName, bspath))
			return (entry);

		entry = (**entry).next;
		}

	return (NULL);
	} /*getprocessinfofromexepath*/


static BOOL CALLBACK winHandleEnumProc (HWND hwnd, LPARAM lparam) {
	WINPROCESSHEADER * listheader;
	DWORD pid;
	WINPROCESSINFO ** pe;

	listheader = (WINPROCESSHEADER *) lparam;

//	releasethreadglobals ();
		
	if (listheader->windowCount < 1024) {
		listheader->windowList[listheader->windowCount] = hwnd;

		++listheader->windowCount;
		}

	GetWindowThreadProcessId (hwnd, &pid);

	pe = getprocessinfofrompid (listheader, pid);

	if (pe != NULL) {
		if ((**pe).windowHandle != NULL) {
			if (! (**pe).isWindowVisible) {
				if (IsWindowVisible (hwnd)) {
					(**pe).windowHandle = hwnd;

					(**pe).isWindowVisible = true;
					}
				}
			}
		else {
			(**pe).isWindowVisible = IsWindowVisible (hwnd);

			(**pe).windowHandle = hwnd;
			}
		}

//	grabthreadglobals ();

	return (true);  /* keep enumerating */
	} /*winHandleEnumProc*/


static WINPROCESSINFO ** addentrytolist (WINPROCESSHEADER * ph) {

	WINPROCESSINFO * entry;
	WINPROCESSINFO ** entryHandle;

	newhandle (sizeof(WINPROCESSINFO), (Handle *)&entryHandle);

	assert (entryHandle);

	lockhandle ((Handle)entryHandle);

	entry = *entryHandle;

	ph->count += 1;

	entry->next = ph->head;
	entry->windowHandle = NULL;
	entry->isWindowVisible = false;

	ph->head = entryHandle;

	unlockhandle ((Handle)entryHandle);

	return (entryHandle);
	} /*addentrytolist*/


static void addprocesstolist (WINPROCESSHEADER * ph, PROCESSENTRY32 * pe) {

	WINPROCESSINFO * entry;
	WINPROCESSINFO ** entryHandle;
	MODULEENTRY32 me32;
	BOOL bGotModule;

	entryHandle = addentrytolist (ph);

	assert (entryHandle);

	lockhandle ((Handle)entryHandle);

	entry = *entryHandle;

	entry->processID = pe->th32ProcessID;

	copyctopstring (pe->szExeFile, entry->ExeName);

    bGotModule = GetProcessModule(pe->th32ProcessID, 
                                  pe->th32ModuleID, 
                                  &me32, 
                                  sizeof(MODULEENTRY32));
	if (bGotModule)
		{

		// Test to see if the app is a Win16 or Win32 app.  If the
		// file name returned in the PROCESSENTRY32 and MODULEENTRY32
		// structures are equal, then we have a Win32 app, otherwise,
		// we have a Win16 app.  

		if (!lstrcmpi (pe->szExeFile, me32.szExePath))
			{
			// Win32 app, use MODULENETRY32 module name
			copyctopstring(me32.szModule, entry->ModName);
			}
		else
			{
			// Win16 app, get module name out of EXE header of file
			if (!GetModuleNameFromExe (pe->szExeFile, stringbaseaddress(entry->ModName),
									   sizeof(entry->ModName) - 1))
				{
				// If we can't get the module name for some reason, at
				// least put something in the module name.
				copyctopstring(me32.szModule, entry->ModName);
				}
			else {
				setstringlength (entry->ModName, strlen(stringbaseaddress(entry->ModName)));
				}
			}
		}

	unlockhandle ((Handle)entryHandle);
	} /*addprocesstolist*/

		
static void cleanProcessHeader (WINPROCESSHEADER * ph) {

	WINPROCESSINFO ** entry;
	WINPROCESSINFO ** next;

	entry = ph->head;

	while (entry != NULL) {
		next = (**entry).next;
		disposehandle((Handle)entry);
		entry = next;
		}

	ph->count = 0;
	ph->windowCount = 0;
	ph->head = NULL;
	} /*cleanProcessHeader*/

static void AddNTProcessInfo (WINPROCESSHEADER * listheader, DWORD processID) {
    char szProcessName[MAX_PATH] = "unknown";
    char szProcessPath[MAX_PATH] = "unknown";
	WINPROCESSINFO * entry;
	WINPROCESSINFO ** entryHandle;

    // Get a handle to the process.

    HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION |
                                   PROCESS_VM_READ,
                                   FALSE, processID );

    // Get the process name.

    if (hProcess)
		{
        HMODULE hMod;
        DWORD cbNeeded;

        if (pEnumProcessModules (hProcess, &hMod, sizeof(hMod), &cbNeeded) )
			{
            pGetModuleBaseName( hProcess, hMod, szProcessName, 
                               sizeof(szProcessName) );
            pGetModuleFileNameEx( hProcess, hMod, szProcessPath, 
                               sizeof(szProcessPath) );
			}
		else {
			if (processID == 2) { /* This is the System process */
				strcpy (szProcessName, "System.exe");
				strcpy (szProcessPath, "System.exe");
				}
			}

	    verify (CloseHandle( hProcess ));
		}
	else {
		if (processID == 0) { /* This is the System Idle process */
			processID = -2;  /* We alter it so zero is not used */
			strcpy (szProcessName, "System Idle Process");
			strcpy (szProcessPath, "Idle.exe");
			}
		}

	entryHandle = addentrytolist (listheader);

	assert (entryHandle);

	lockhandle ((Handle)entryHandle);

	entry = *entryHandle;

	entry->processID = processID;
	copyctopstring (szProcessPath, entry->ExeName);
	copyctopstring (szProcessName, entry->ModName);

	unlockhandle ((Handle)entryHandle);
	} /*AddNTProcessInfo*/

static boolean enumWinNTProcesses (WINPROCESSHEADER * listheader) {

    // Get the list of process identifiers.

    DWORD aProcesses[1024], cbNeeded, cProcesses;
    unsigned int i;
	boolean res;

	res = false;

	if (InitToolhelpNT() == false)
		return (false);

 	releasethreadglobals ();
	if (pEnumProcesses (aProcesses, sizeof(aProcesses), &cbNeeded ) ) {

		// Calculate how many process identifiers were returned.

		cProcesses = cbNeeded / sizeof(DWORD);

		// Add processes to list.

		for ( i = 0; i < cProcesses; i++ )
			AddNTProcessInfo(listheader, aProcesses[i]);


		EnumWindows (winHandleEnumProc, (LPARAM)listheader);

		res = true;
		}

	grabthreadglobals ();

	return (res);
	} /*enumWinNTProcesses*/


static boolean enumWin95Processes (WINPROCESSHEADER * listheader) {

	HANDLE snapHandle;
	PROCESSENTRY32 pe;

	if (InitToolhelp32() == false)
		return (false);

	snapHandle = pCreateToolhelp32Snapshot (TH32CS_SNAPPROCESS, 0);

	if (snapHandle == (HANDLE) -1)
		return (false);

	if (snapHandle != NULL) {
		pe.dwSize = sizeof(PROCESSENTRY32);

		if (pProcess32First (snapHandle, &pe)) {

			do {
				addprocesstolist (listheader, &pe);
				pe.dwSize = sizeof(PROCESSENTRY32);
				}
				while (pProcess32Next (snapHandle, &pe));

			releasethreadglobals ();
		
			EnumWindows (winHandleEnumProc, (LPARAM)listheader);
			
			grabthreadglobals ();

			verify (CloseHandle (snapHandle));
			return (true);
			}

		verify (CloseHandle (snapHandle));
		}

	return (false);
	} /*enumWin95Processes*/


static boolean enumWinProcesses (WINPROCESSHEADER * listheader) {
	OSVERSIONINFO vi;

	listheader->count = 0;
	listheader->windowCount = 0;
	listheader->head = NULL;

	vi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);

	GetVersionEx (&vi);

	if (vi.dwPlatformId == VER_PLATFORM_WIN32_NT) {
		listheader->platform = 1;
		return (enumWinNTProcesses (listheader));
		}

	listheader->platform = 0;
	return (enumWin95Processes (listheader));
	} /*enumWinProcesses*/

#endif

#ifdef MACVERSION
static OSErr getlaunchappparams (const tyfilespec *fsdoc, AppParametersHandle *appparams) {
	
	/*
	2.1a6 dmb: new feature, if bsdoc is the empty string, send ascr/noop event 
	instead of aevt/odoc. this should result in no empty document / standard file 
	dialog in most apps
	*/
	
	AEDesc addrdesc;
	AEDesc launchdesc;
	AEDescList doclist;
	AppleEvent event;
	AEEventID eventid;
	AEEventClass eventclass;
	OSErr errcode;
	AliasHandle halias;
	AEDesc docdesc;
	
	if (!haveheapspace (0x0200)) /*512 bytes should be plenty to avoid lots of error checking*/
		return (memFullErr);
	
	if (isemptystring ((*fsdoc).name)) {
		
		eventclass = 'ascr';
		
		eventid = 'noop';
		
		/*
		eventid = kAEOpenApplication;
		*/
		}
	else {
		
		eventclass = kCoreEventClass;
		
		eventid = kAEOpenDocuments;
		
		errcode = NewAliasMinimal (fsdoc, &halias);
		
		if (errcode != noErr)
			return (errcode);
		}
	
	
	#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/
	
		newdescnull (&addrdesc, typeNull);
	
	#else
	
		addrdesc.descriptorType = typeNull;
	
		addrdesc.dataHandle = nil;
	
	#endif
	
	AECreateAppleEvent (eventclass, eventid, &addrdesc, kAutoGenerateReturnID, kAnyTransactionID, &event);
	
	if (eventid == kAEOpenDocuments) { /*need to add document list parameter*/
		
		AECreateList (nil, 0, false, &doclist); /*create list for the fsspec*/
		
		//AEPutPtr (&doclist, 0, typeFSS, (Ptr) &fs, sizeof (fs)); /* put filespec on the list%/
				
		#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/
		
			newdescwithhandle (&docdesc, typeAlias, (Handle) halias);
			
		#else
		
			docdesc.descriptorType = typeAlias;
		
			docdesc.dataHandle = (Handle) halias;
		
		#endif
		
		AEPutDesc (&doclist, 0, &docdesc); /*put alias on the list*/
		
		AEDisposeDesc (&docdesc);
		
		AEPutParamDesc (&event, keyDirectObject, &doclist); /*put list on the event*/
		
		AEDisposeDesc (&doclist); /*dispose list*/
		}
	
	errcode = AECoerceDesc (&event, typeAppParameters, &launchdesc); /*this is what Launch wants*/
	
	AEDisposeDesc (&event); /*we're done with it*/
	
	if (errcode != noErr) /*shouldn't fail, but...*/
		return (errcode);
	
	#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/
		
		copydatahandle (&launchdesc, (Handle*) appparams);

	#else
	
		*appparams = (AppParametersHandle) launchdesc.dataHandle;
	
	#endif
	
	return (noErr);
	} /*getlaunchappparams*/
#endif


#ifdef MACVERSION
static boolean system7apprunning (OSType *, bigstring, typrocessid *);
#endif

#ifdef MACVERSION
static boolean system7launch (tyfilespec *fsapp, const tyfilespec *fsdoc, boolean flbringtofront) {
	
	/*
	11/10/90 DW: comment out call to MakeFSSpec, the linker can't find it, wire
	off routine.
	
	3/27/91 dmb: re-enabled routine.  no wonder file.launch didn't work!
	
	12/9/91 dmb: check for oserrors
	
	3/11/92 dmb: now accept bsdocument and set up launchAppParams
	
	8/13/92 dmb: test bsdocument for emptyness as well as nil
	
	9/4/92 dmb: wait for launch to happen
	
	2002-11-13 AR: Only append ".app" to the filename if there's enough room in the name string
	*/
	
	LaunchParamBlockRec sys7lpb;
	AppParametersHandle docparams = nil;
	OSErr errcode;
	long startticks;
	
	clearbytes (&sys7lpb, longsizeof (sys7lpb));
	
	#if TARGET_API_MAC_CARBON == 1 /*7.1b19 PBS: append ".app" to the filename if the file can't be found.*/
		{
		boolean flfolder = false;
		
		if (!fileexists (fsapp, &flfolder)) {

			unsigned char appext[] = ("\x04" ".app");
			const short oldlength = stringlength ((*fsapp).name);
			const short newlength = oldlength + stringlength (appext);
			const short maxlength = sizeof ((*fsapp).name) - 1;	/*minus one for length byte*/
			
			if (newlength <= maxlength) {

				pushstring (appext, (*fsapp).name);
							
				if (!fileexists (fsapp, &flfolder)) {

					setstringlength ((*fsapp).name, oldlength);	/*pop off the .app suffix*/

					} /*if*/
				} /*if*/
			} /*if*/
		}
		/*
		{
		boolean flfolder = false;
		short lenname = (*fsapp).name [0];
		
		if (!fileexists (fsapp, &flfolder)) {
		
			(*fsapp).name [0] = (*fsapp).name [0] + 4;
			
			(*fsapp).name [lenname + 1] = '.';
			(*fsapp).name [lenname + 2] = 'a';
			(*fsapp).name [lenname + 3] = 'p';
			(*fsapp).name [lenname + 4] = 'p';
			
			if (!fileexists (fsapp, &flfolder)) {
			
				(*fsapp).name [0] = lenname; /%pop off the .app suffix%/
				} /%if%/
			} /%if%/
		}
		*/
	#endif
	
	sys7lpb.launchAppSpec = (FSSpecPtr) fsapp;
	
	sys7lpb.launchBlockID = extendedBlock;
	
	sys7lpb.launchEPBLength = extendedBlockLen;
	
	sys7lpb.launchControlFlags = launchContinue | launchNoFileFlags;

	if (!flbringtofront)
		sys7lpb.launchControlFlags |= launchDontSwitch;
	
	if (fsdoc != nil) { /*set up docparam with launch event*/
		
		errcode = getlaunchappparams (fsdoc, &docparams);
		
		if (oserror (errcode))
			return (false);
		
		setoserrorparam ((ptrstring) (*fsapp).name); /*restore*/
		
		lockhandle ((Handle) docparams);
		
		sys7lpb.launchAppParameters = *docparams;
		}
	
	errcode = LaunchApplication (&sys7lpb);
	
	disposehandle ((Handle) docparams); /*checks for nil*/
	
	if ((fsdoc != nil) && isemptystring ((*fsdoc).name)) /*sent noop event*/
		if (errcode == errAEEventNotHandled)
			errcode = noErr;
	
	if (oserror (errcode))
		return (false);
	
	startticks = gettickcount ();
	
	while (true) { /*wait for launch to happen*/
		
		OSType id = 0;
		
		if (!(*launchcallbacks.waitcallback) ())
			return (false);
		
		if (system7apprunning (&id, (ptrstring) (*fsapp).name, nil))
			return (true);
		
		if (gettickcount () - startticks > maxwait)
			return (false);
		}
	} /*system7launch*/
#endif

#ifdef flsystem6

static boolean system6launch (bigstring bsprogram) {
	
	system6launchparamblock sys6lpb;
	bigstring bsname;
	bigstring bsfolder;
	register OSErr errcode;
	WDPBRec wdpb;
	CInfoPBRec cipb;
	short vnum;
	
	clearbytes (&sys6lpb, longsizeof (sys6lpb));
	
	copystring (bsprogram, bsname);
	
	/*get the volume and directory ids of the path*/
	
	/*
	if (!fileparsevolname (bsprogram, &vnum))
		return (false);
	*/
	
	folderfrompath (bsprogram, bsfolder);
	
	cipb.hFileInfo.ioNamePtr = bsfolder;
	
	cipb.hFileInfo.ioVRefNum = 0; /*vnum;*/
	
	cipb.hFileInfo.ioFDirIndex = 0;
	
	errcode = PBGetCatInfo (&cipb, false);
	
	if (oserror (errcode))
		return (false);
	
	/*now open a working directory there*/
	
	wdpb.ioNamePtr = bsfolder;
	
	wdpb.ioVRefNum = 0;
	
	wdpb.ioWDProcID = 'ERIK';
	
	wdpb.ioWDDirID = cipb.hFileInfo.ioDirID;
	
	errcode = PBOpenWD (&wdpb, false);
	
	if (oserror (errcode))
		return (false);
	
	vnum = wdpb.ioVRefNum;
	
	filefrompath (bsprogram, bsname);
	
	sys6lpb.launchFileFlags = cipb.hFileInfo.ioFlFndrInfo.fdFlags;
	
	/*make the (new) working directory the current one*/
	
	errcode = SetVol (nil, vnum);
	
	sys6lpb.name = bsname;
	
	sys6lpb.launchVRefNum = vnum;
	
	sys6lpb.launchBlockID = extendedBlock;
	
	sys6lpb.launchEPBLength = extendedBlockLen;
	
	sys6lpb.launchControlFlags = 0xC000;
	
	return (!oserror (LaunchApplication ((LaunchPBPtr) &sys6lpb)));
	} /*system6launch*/

#endif

#ifdef WIN95VERSION
static boolean systemWinLaunch (const tyfilespec *fsapp, const tyfilespec *fsdoc, boolean flbringtofront) {
	char appname[258];
	char commandline [514];
	STARTUPINFO si;
//	PROCESS_INFORMATION pi;


	copystring (fsname (fsapp), appname);

	nullterminate (appname);

	if (fsdoc != nil)
		copystring (fsname (fsdoc), commandline);
	else
		setemptystring (commandline);

	nullterminate (commandline);

	si.cb = sizeof(STARTUPINFO);
	si.lpReserved = NULL;
	si.lpDesktop = NULL;
	si.lpTitle = NULL;
	si.dwFlags = 0;
	si.cbReserved2 = 0;
	si.lpReserved2 = NULL;
//	return (CreateProcess (stringbaseaddress (appname), stringbaseaddress (commandline), NULL, NULL, false, CREATE_DEFAULT_ERROR_MODE,
//					NULL, NULL, &si, &pi));

	return (ShellExecute (NULL, "open", stringbaseaddress (appname), stringbaseaddress (commandline), "", SW_SHOWNORMAL) > (HINSTANCE)32); 
	}
#endif

boolean launchapplication (const tyfilespec *fsapp, const tyfilespec *fsdoc, boolean flbringtofront) {
	
	setoserrorparam ((ptrstring) fsname(fsapp)); /*in case error message takes a filename parameter*/
	
#ifdef MACVERSION
	#ifdef flsystem6
	
	flsystem7 = (gSystemVersion >= 0x0700);
	
	if (flsystem7)
		return (system7launch (bs, bsdocument, flbringtofront));
	else
		return (system6launch (bs));
	
	#else
	
	return (system7launch ((ptrfilespec) fsapp, fsdoc, flbringtofront));
	
	#endif
#endif
	
#ifdef WIN95VERSION
	return (systemWinLaunch (fsapp, fsdoc, flbringtofront));
#endif
	} /*launchapplication*/

#ifdef MACVERSION
static boolean visitprocesses (boolean (*visitroutine) (typrocessvisitinfoptr, ProcessInfoRec *), typrocessvisitinfoptr visitinfo) {
	
	ProcessInfoRec processinfo;
	ProcessSerialNumber psn;
	bigstring bsname;
	FSSpec fss;
	
	processinfo.processInfoLength = sizeof (processinfo);
	
	processinfo.processName = bsname; /*place to store process name*/
	
	processinfo.processAppSpec = &fss; /*place to store process filespec*/
	
	psn.highLongOfPSN = kNoProcess;
	
	psn.lowLongOfPSN = kNoProcess;
	
	while (GetNextProcess (&psn) == noErr) {
		
	 	processinfo.processInfoLength = sizeof (ProcessInfoRec);
	 	
		if (GetProcessInformation (&psn, &processinfo) != noErr)
			continue; /*keep going -- ignore error*/
		
		if (!(*visitroutine) (visitinfo, &processinfo))
			return (false);
		}
	
	return (true);
	} /*visitprocesses*/
#endif

#ifdef MACVERSION
static boolean matchprocess (bigstring bsprocess, ProcessInfoRec *processinfo) {
	
	/*
	9/4/92 dmb: see if bsprocess is the name or full path of the given process
	
	10/2/92 dmb: if bsprocess is a string4, see if it matches the process id
	
	7.1b37 PBS: deal with .app extensions on OS X.
	*/
	
	bigstring bspath;
	OSType id;
	
	if (stringfindchar (':', bsprocess)) { /*it's a path*/
		
		if (!filespectopath (processinfo->processAppSpec, bspath))
			return (false);
		
		return (equalidentifiers (bsprocess, bspath));
		}
	
	if (equalidentifiers (bsprocess, processinfo->processName))
		return (true);
	
	#if TARGET_API_MAC_CARBON
	
		{
		bigstring bsprocessminussuffix;
		bigstring bssuffix;
		
		copystring (bsprocess, bsprocessminussuffix);
		
		if (pullstringsuffix (bsprocessminussuffix, bssuffix, '.')) /*has suffix?*/
			
			if (equalidentifiers (bssuffix, "\papp")) /*.app suffix?*/
			
				if (equalidentifiers (bsprocessminussuffix, processinfo->processName)) /*match?*/
					return (true);
		}
	
	#endif
	
	if (stringtoostype (bsprocess, &id))
		return (id == processinfo->processSignature);
	
	return (false);
	} /*matchprocess*/
#endif


#ifdef MACVERSION
static boolean activateprocess (ProcessSerialNumber psn) {
	
	/*
	9/4/92 dmb: check for oserrors & wait for change to happen
	
	2.1b12 dmb: call the waitcallback at the end of the loop, instead 
	of at the beginning. the process may already be in the front.
	
	2.1b14 dmb: make sure we call the waitcallback before timing out.
	
	10.0a4 trt/karstenw: avoid -600 (no such process) error in Classic
	even if the app is already launched.
	*/
	
	long startticks;
	
#if defined(TARGET_API_MAC_OS8) && (TARGET_API_MAC_OS8 == 1)
	if (oserror (WakeUpProcess (&psn)))
		return (false);
#endif
	
	if (oserror (SetFrontProcess (&psn)))
		return (false);
	
	startticks = gettickcount ();
	
	while (true) { /*wait for it to happen*/
		
		ProcessSerialNumber frontpsn;
		Boolean flsame;
		
		if (oserror (GetFrontProcess (&frontpsn)))
			break;
		
		if (oserror (SameProcess (&psn, &frontpsn, &flsame)))
			break;
		
		if (flsame)
			break;
		
		if (!(*launchcallbacks.waitcallback) ())
			break;
		
		if (gettickcount () - startticks > maxwait)
			break;
		}
	
	return (true);
	} /*activateprocess*/
#endif


#ifdef MACVERSION
static boolean processactivatevisit (typrocessvisitinfoptr visitinfo, ProcessInfoRec *processinfo) {
	
	/*
	6/24/92 dmb: make comparison case-insensitive
	*/
	
	if (matchprocess (visitinfo->bsprocess, processinfo)) { /*found the process*/
		
		activateprocess (processinfo->processNumber);
		
		return (false); /*stop visiting*/
		}
	
	return (true); /*keep visiting*/
	} /*processactivatevisit*/
#endif


#ifdef MACVERSION
static boolean system7activate (bigstring bsprogram) {
	
	/*
	1/21/93 dmb: if bsprogram is nil, use current process. this is done 
	for component support
	
	2002-11-14 AR: Switch from using globals to a local struct for providing
	context to the processactivatevisit callback routine
	*/
	
	typrocessvisitinfo info;
	
	if (bsprogram == nil) {
		
		ProcessSerialNumber psn;
		
		GetCurrentProcess (&psn);
		
		return (activateprocess (psn));
		}
	
	initprocessvisitinfo (&info);
	
	info.bsprocess = bsprogram; /*set for visit routine*/
	
	return (!visitprocesses (&processactivatevisit, &info));
	} /*system7activate*/
#endif


#ifdef MACVERSION
static boolean system6activate (bigstring bsprogram) {
	
	#ifdef flsystem6
	
	bigstring bs;
	
	if (bsprogram == nil)
		return (false);
		
	filefrompath (bsprogram, bs);
		
	OpenDeskAcc (bs);
	
	#endif
	
	return (true);
	} /*system6activate*/
#endif

boolean activateapplication (bigstring bsprogram) {
	
	/*
	10/3/91 dmb: handle the possibility of bsprogram being a full path

	7.0b33 PBS: Workaround for changes in Win98 and Win2000 -- use AttachThreadInput
	to attach the current thread to the foreground window, so that our thread
	has permission to set the foreground window.
	*/
	
#ifdef MACVERSION
	boolean flsystem7;
	
	//Code change by Timothy Paustian Friday, June 9, 2000 2:36:02 PM
	//Changed because using SysEnvisons and SysEnvRec is like Really old style
	//This was changed to Gestalt calls with two new globals see mac.c initmacintosh
	
	flsystem7 = (gSystemVersion >= 0x0700);
	
	if (flsystem7)
		return (system7activate (bsprogram));
	else
		return (system6activate (bsprogram));
#endif

#ifdef WIN95VERSION
	WINPROCESSHEADER listheader;
	WINPROCESSINFO ** entry;
	boolean res;
	int currthreadid, winthreadid;
	HWND hcurrwnd;

	res = false;

	enumWinProcesses (&listheader);

	if ((bsprogram == NULL) || (stringlength (bsprogram) == 0)) {
		entry = getprocessinfofrompid (&listheader, GetCurrentProcessId());
		}
	else
		{
		entry = getprocessinfofrommodname (&listheader, bsprogram);

		if (entry == NULL)
			entry = getprocessinfofromwintitle (&listheader, bsprogram);

		if (entry == NULL) {
			entry = getprocessinfofromexepath (&listheader, bsprogram);
			}
		}

	if (entry != NULL) {
		WINDOWPLACEMENT wp;

		releasethreadglobals();

		wp.length = sizeof(WINDOWPLACEMENT);
		if (GetWindowPlacement ((**entry).windowHandle, &wp))
			if (wp.showCmd == SW_SHOWMINIMIZED) {
				wp.showCmd = SW_RESTORE;
				SetWindowPlacement ((**entry).windowHandle, &wp);
				}

		currthreadid = GetCurrentThreadId ();

		hcurrwnd = GetForegroundWindow ();
   
		winthreadid = GetWindowThreadProcessId (hcurrwnd, nil);

		if (currthreadid != winthreadid)

			AttachThreadInput (currthreadid, winthreadid, true);

		res = SetForegroundWindow((**entry).windowHandle);
		
		if (currthreadid != winthreadid)

			AttachThreadInput (currthreadid, winthreadid, false);

		grabthreadglobals();
		}

	cleanProcessHeader (&listheader);
	return (res);
#endif
	} /*activateapplication*/


#ifdef MACVERSION
static boolean processcreatorvisit (typrocessvisitinfoptr visitinfo, ProcessInfoRec *processinfo) {
	
	if (processinfo->processSignature == visitinfo->idprocess) {
		
		if (visitinfo->bsprocess != nil)
			copystring (processinfo->processName, visitinfo->bsprocess);
		
		goto match;
		}
	
	if (visitinfo->bsprocess != nil) { /*try to match name*/
		
		if (matchprocess (visitinfo->bsprocess, processinfo)) {
			
			visitinfo->idprocess = processinfo->processSignature;
			
			goto match;
			}
		}
	
	return (true); /*keep visiting*/
	
	match: {
	
		if (visitinfo->psnprocess != nil) {
			
			visitinfo->psnprocess->highLongOfPSN = processinfo->processNumber.highLongOfPSN;
			
			visitinfo->psnprocess->lowLongOfPSN = processinfo->processNumber.lowLongOfPSN;
			}
		
		return (false); /*stop visiting*/
		}
	} /*processcreatorvisit*/
#endif



#ifdef MACVERSION
static boolean system7apprunning (OSType *idapp, bigstring appname, typrocessid *psn) {

	/*
	2002-11-14 AR: Switch from using globals to a local struct for providing
	context to the processcreatorvisit callback routine
	*/
	
	typrocessvisitinfo info;
	
	initprocessvisitinfo (&info);
		
	info.idprocess = *idapp; /*set for visit routine*/
	
	info.bsprocess = appname;
	
	info.psnprocess = psn;
	
	if (visitprocesses (&processcreatorvisit, &info))
		return (false);
	
	*idapp = info.idprocess;
	
	return (true);
	} /*system7apprunning*/
#endif


#ifdef flsystem6

static boolean system6apprunning (OSType *appid, bigstring appname) {
	
	ostypetostring (*appid, appname); /*better than nothing*/
	
	return (true);
	} /*system6apprunning*/

#endif

boolean findrunningapplication (OSType *appid, bigstring appname, typrocessid *psn) {
	
#ifdef MACVERSION
	/*
	this is really a system7-only routine.  under system 6, we always return true.
	
	under system 7, we look for an application with the given id, and return 
	true if we find it, filling in appname.
	
	2.1a7 dmb: take psn param
	*/
	
	#ifdef flsystem6
	
	boolean flsystem7;
	
	flsystem7 = (macworld.systemVersion >= 0x0700);
	
	if (!flsystem7)
		return (system6apprunning (appid, appname));
	else
	
	#endif
	
		return (system7apprunning (appid, appname, psn));
#endif

#ifdef WIN95VERSION
	WINPROCESSHEADER listheader;
	WINPROCESSINFO ** entry;
	boolean res;

	res = false;

	enumWinProcesses (&listheader);

	entry = getprocessinfofrommodname (&listheader, appname);

	if (entry == NULL)
		entry = getprocessinfofromwintitle (&listheader, appname);

	if (entry == NULL) {
		entry = getprocessinfofromexepath (&listheader, appname);
		}

	if (entry != NULL) {
		res = true;
		}

	cleanProcessHeader (&listheader);
	return (res);
#pragma message ("WIN95 findrunningapplication does not handle appid")
#endif
	} /*findrunningapplication*/


#ifdef MACVERSION
static boolean system7frontprogram (bigstring bsprogram, boolean flfullpath) {
	
	/*
	10/3/91 dmb: if flfullpath is true, return the full path instead of the 
	process name
	*/
	
	ProcessInfoRec processinfo;
	ProcessSerialNumber psn;
	FSSpec fs;
	
	processinfo.processInfoLength = sizeof (processinfo);
	
	processinfo.processName = bsprogram; /*place to store process name*/
	
	processinfo.processAppSpec = &fs;
	
	if (oserror (GetFrontProcess (&psn)))
		return (false);
	
	if (oserror (GetProcessInformation (&psn, &processinfo)))
		return (false);
	
	if (flfullpath)
		filespectopath (&fs, bsprogram);
	
	return (true);
	} /*system7frontprogram*/
#endif


#ifdef  flsystem6

static boolean system6frontprogram (bigstring bsprogram) {
	
	copystring ((ptrstring) CurApName, bsprogram);
	
	return (true);
	} /*system6frontprogram*/

#endif


boolean getfrontapplication (bigstring bsprogram, boolean flfullpath) {

#ifdef MACVERSION	
	#ifdef flsystem6
	
		boolean flsystem7;
		
		flsystem7 = (macworld.systemVersion >= 0x0700);
		
		if (flsystem7)
			return (system7frontprogram (bsprogram, flfullpath));
		else
			return (system6frontprogram (bsprogram));
	
	#else
	
		return (system7frontprogram (bsprogram, flfullpath));
	
	#endif
#endif

#ifdef WIN95VERSION
	WINPROCESSHEADER listheader;
	WINPROCESSINFO ** entry;
	boolean res;
	DWORD pid;

	res = false;

	enumWinProcesses (&listheader);

	GetWindowThreadProcessId (GetForegroundWindow(), &pid);

	entry = getprocessinfofrompid (&listheader, pid);

	if (entry != NULL) {
		if (flfullpath) {
			copystring ((**entry).ExeName, bsprogram);
			}
		else
			{
			copystring ((**entry).ModName, bsprogram);
			}

		res = true;
		}

	cleanProcessHeader (&listheader);
	return (res);
#endif
	} /*getfrontapplication*/


#ifdef MACVERSION
static boolean processcountvisit (typrocessvisitinfoptr visitinfo, ProcessInfoRec *processinfo) {
	
	visitinfo->ctprocesses++;
	
	if (visitinfo->ctprocesses == visitinfo->nthprocess) { /*found the one we're looking for*/
		
		copystring (processinfo->processName, visitinfo->bsprocess);
		
		return (false); /*stop visiting*/
		}
	
	return (true); /*keep visiting*/
	} /*processcountvisit*/
#endif


short countapplications (void) {

	/*
	2002-11-14 AR: Switch from using globals to a local struct for providing
	context to the processcountvisit callback routine
	*/
		
#ifdef MACVERSION
	typrocessvisitinfo info;
	boolean flsystem7;
	//Code change by Timothy Paustian Friday, June 9, 2000 2:36:02 PM
	//Changed because using SysEnvisons and SysEnvRec is like Really old style
	//This was changed to Gestalt calls with two new globals see mac.c initmacintosh
	flsystem7 = (gSystemVersion >= 0x0700);
	
	if (!flsystem7)
		return (0);
	
	initprocessvisitinfo (&info);
	
	info.ctprocesses = 0; /*set for visit routine*/
	
	info.nthprocess = -1; /*so we'll never find a match*/
	
	visitprocesses (&processcountvisit, &info);
	
	return (info.ctprocesses);
#endif

#ifdef WIN95VERSION
	WINPROCESSHEADER listheader;
	short count;

	enumWinProcesses (&listheader);

	count = (short)listheader.count;

	cleanProcessHeader (&listheader);
	return (count);
#endif
	} /*countapplications*/


boolean getnthapplication (short n, bigstring bsprogram) {
	
	/*
	2002-11-14 AR: Switch from using globals to a local struct for providing
	context to the processcountvisit callback routine
	*/
	
#ifdef MACVERSION
	typrocessvisitinfo info;
	boolean flsystem7;
	//Code change by Timothy Paustian Friday, June 9, 2000 2:36:02 PM
	//Changed because using SysEnvisons and SysEnvRec is like Really old style
	//This was changed to Gestalt calls with two new globals see mac.c initmacintosh
	flsystem7 = (gSystemVersion >= 0x0700);
	
	if (!flsystem7)
		return (false);
	
	initprocessvisitinfo (&info);
	
	info.ctprocesses = 0; /*set for visit routine*/
	
	info.nthprocess = n; /*the one we want*/
	
	info.bsprocess = bsprogram; /*point to caller's storage*/
	
	return (!visitprocesses (&processcountvisit, &info));
#endif

#ifdef WIN95VERSION
	WINPROCESSHEADER listheader;
	WINPROCESSINFO ** entry;
	boolean res;

	res = false;

	enumWinProcesses (&listheader);

	entry = listheader.head;

	while ((n > 1) && (entry != NULL)) {
		entry = (**entry).next;
		--n;
		}

	setstringlength (bsprogram, 0);

	if (entry != NULL) {
		copystring ((**entry).ModName, bsprogram);

		if (stringlength(bsprogram) == 0)
			copystring ((**entry).ExeName, bsprogram);

		res = true;
		}

	cleanProcessHeader (&listheader);
	return (res);
#endif
	} /*getnthapplication*/


#ifdef MACVERSION
static boolean getprocesspathvisit (typrocessvisitinfoptr visitinfo, ProcessInfoRec *processinfo) {
	
	/*
	7/14/92 dmb: created
	*/
	
	if (matchprocess (visitinfo->bsprocess, processinfo)) { /*found the process*/
		
		*(visitinfo->fsprocess) = *(processinfo->processAppSpec);
		
		return (false); /*stop visiting*/
		}
	
	return (true); /*keep visiting*/
	} /*getprocesspathvisit*/
#endif


boolean getapplicationfilespec (bigstring bsprogram, tyfilespec *fs) {
	
	/*
	5.0.2b21 dmb: clear unused fields on Win
	
	2000-06-09 Timothy Paustian: Changed because using SysEnvisons
	and SysEnvRec is like Really old style. This was changed to Gestalt calls
	with two new globals see mac.c initmacintosh.
	
	2002-11-14 AR: Switch from using globals to a local struct for providing
	context to the getprocesspathvisit callback routine
	
	2004-10-26 aradke: New Carbon/Mach-O implementation for obtaining
	a filespec to our application bundle, i.e. the Frontier.app "folder".
	*/

#ifdef MACVERSION
	typrocessvisitinfo info;
	boolean flsystem7;
	
	flsystem7 = (gSystemVersion >= 0x0700);
	
	if (!flsystem7)
		return (false);
	
	if (bsprogram == nil) { /*get path to this process*/
		
		#if TARGET_RT_MAC_MACHO
			CFBundleRef mybundleref;
			CFURLRef myurlref;
			FSRef myfsref;
			boolean res;
			OSErr err;
			
			mybundleref = CFBundleGetMainBundle();
			
			if (mybundleref == NULL)
				return (false);
			
			myurlref = CFBundleCopyBundleURL(mybundleref);
			
			if (myurlref == NULL)
				return (false);

			res = CFURLGetFSRef(myurlref, &myfsref);
			
			CFRelease(myurlref);
			
			if (!res)
				return (false);
			
			err = FSGetCatalogInfo(&myfsref, kFSCatInfoNone, NULL, NULL, fs, NULL);
			
			return (err == noErr);
		#else
			ProcessInfoRec processinfo;
			ProcessSerialNumber psn;
			
			processinfo.processInfoLength = sizeof (processinfo);
			
			processinfo.processName = nil; /*place to store process name*/
			
			processinfo.processAppSpec = fs; /*place to store process filespec*/
			
			psn.highLongOfPSN = 0;
			
			psn.lowLongOfPSN = kCurrentProcess;
			
			if (GetProcessInformation (&psn, &processinfo) != noErr)
				return (false);
			
			return (true);
		#endif
		}
	
	initprocessvisitinfo (&info);
	
	info.bsprocess = bsprogram; /*point to caller's storage*/
	
	info.fsprocess = fs; /*can be same storage as bsprogram -- or not*/
	
	return (!visitprocesses (&getprocesspathvisit, &info));
#endif

#ifdef WIN95VERSION
	WINPROCESSHEADER listheader;
	WINPROCESSINFO ** entry;
	boolean res;
	char curpath[256];
	short len;
//	OSVERSIONINFO osinfo;

//	osinfo.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);

//	GetVersionEx (&osinfo);

//	if (osinfo.dwPlatformId == VER_PLATFORM_WIN32_NT)

	res = false;
	
	clearbytes (fs, sizeof (*fs));

	if ((bsprogram == NULL) || (stringlength (bsprogram) == 0)) {

		len = (short)GetModuleFileName (NULL, curpath, sizeof(curpath));

		if (len > 0) {
			curpath[len] = 0;  //ensure null termination
			copyctopstring (curpath, fsname(fs));
			res = true;
			return (res);
			}
		}


	enumWinProcesses (&listheader);

	if ((bsprogram == NULL) || (stringlength (bsprogram) == 0)) {
		entry = getprocessinfofrompid (&listheader, GetCurrentProcessId());
		}
	else
		{
		entry = getprocessinfofrommodname (&listheader, bsprogram);

		if (entry == NULL)
			entry = getprocessinfofromwintitle (&listheader, bsprogram);

		if (entry == NULL)
			entry = getprocessinfofromexepath (&listheader, bsprogram);
		}

	setstringlength (fsname(fs), 0);

	if (entry != NULL) {
		copystring ((**entry).ExeName, fsname(fs));
		res = true;
		}
//	else {
//		GetCurrentDirectory (256, curpath);
//		strcat (curpath, "\\Frontier.exe");
//		copyctopstring (curpath, fsname(fs));
//		res = true;
//		}

	cleanProcessHeader (&listheader);
	return (res);
#endif
	} /*getapplicationfilespec*/


boolean executeresource (ResType type, short id, bigstring bsname) {
	
#ifdef MACVERSION
	/*
	if bsname is nil, get the resource by id; otherwise, ignore the id.
	
	this may not belong in this file, but running an external resource is 
	somewhat analagous to launching a program, so....
	
	3.0.4 dmb: void prototype for 68K direct call
	*/
	
	Handle hcode;
	
	if (bsname == nil)
		hcode = GetResource (type, id);
	else
		hcode = GetNamedResource (type, bsname);
	
	if (hcode == nil)
		return (false);
	
	lockhandle (hcode);
	
	#if THINK_C
	
	CallPascal (*hcode);
	
	//Code change by Timothy Paustian Sunday, May 7, 2000 11:18:54 PM
	//we can just call the routine directly. I think.
	#elif TARGET_RT_MAC_CFM
	
	#if TARGET_API_MAC_CARBON == 1
		(*(pascal void (*) (void)) hcode) ();
	#else
	{
		UniversalProcPtr upp = NewRoutineDescriptor ((ProcPtr) *hcode, kPascalStackBased, kM68kISA);
		
		CallUniversalProc (upp, kPascalStackBased);
		
		DisposeRoutineDescriptor (upp);
	}
	#endif
		
	#else
	
		(*(pascal void (*) (void)) hcode) ();
	
	#endif
	
	unlockhandle (hcode);
	
	ReleaseResource (hcode);
	
	return (true);
#endif

#ifdef WIN95VERSION
	/* not supported */
	return (false);
#endif
	} /*executeresource*/


#ifdef MACVERSION
OSType getprocesscreator (void) {
	
	/*
	get the 4-character creator identifier for the application we're running 
	inside of.
	*/
	
	ProcessSerialNumber psn;
	ProcessInfoRec info;
	
	GetCurrentProcess (&psn);
	
	info.processInfoLength = (long) sizeof (info);
	
	info.processName = nil;
	
	info.processAppSpec = nil;
	
	GetProcessInformation (&psn, &info);
	
	return (info.processSignature);
	} /*getprocesscreator*/
#endif


typrocessid getcurrentprocessid (void) {
	
	ProcessSerialNumber psn;
	
#ifdef MACVERSION
	GetCurrentProcess (&psn);
#endif

#ifdef WIN95VERSION
	psn = GetCurrentProcessId();
#endif
	
	return (psn);
	} /*getcurrentprocessid*/


boolean iscurrentapplication (typrocessid psn) {
	
	ProcessSerialNumber currentpsn;
	
#ifdef MACVERSION
	Boolean flsame;
	OSErr err;

	GetCurrentProcess (&currentpsn);

	err = SameProcess (&psn, &currentpsn, &flsame);
	
	return ((err == noErr) && flsame);
#endif
	
#ifdef WIN95VERSION
	currentpsn = GetCurrentProcessId();

	return (currentpsn == psn);
#endif
	} /*iscurrentapplication*/

boolean isfrontapplication (typrocessid psn) {

#ifdef MACVERSION
	ProcessSerialNumber frontpsn;
	Boolean flsame;
	OSErr err;
	
	GetFrontProcess (&frontpsn);
	
	err = SameProcess (&psn, &frontpsn, &flsame);
	
	return ((err == noErr) && flsame);
#endif

#ifdef WIN95VERSION
	WINPROCESSHEADER listheader;
	WINPROCESSINFO ** entry;
	boolean res;
	DWORD pid;

	res = false;

	enumWinProcesses (&listheader);

	entry = getprocessinfofrompid (&listheader, psn);

	if (entry != NULL) {
		GetWindowThreadProcessId (GetForegroundWindow(), &pid);

		if ((**entry).processID == pid)
			res = true;
		}

	cleanProcessHeader (&listheader);
	return (res);
#endif
	} /*isfrontapplication*/


#ifdef MACVERSION
boolean activateapplicationwindow (typrocessid psn, WindowPtr w) {
	
	/*
	4/20/93 dmb: activate the application and ask it to select window w.
	
	the event we send corresponds to the handler installed by the window 
	sharing server (us).
	*/
	
	AEDesc addrdesc;
	AppleEvent event, reply;
	OSErr err;
	register hdllandglobals hlg;
	
	err = AECreateDesc (typeProcessSerialNumber, (Ptr) &psn, longsizeof (psn), &addrdesc);
	
	err = AECreateAppleEvent ('SHUI', 'selw', &addrdesc, kAutoGenerateReturnID, kAnyTransactionID, &event);
	
	AEDisposeDesc (&addrdesc);
	
	err = AEPutParamPtr (&event, '----', typeLongInteger, (Ptr) &w, sizeof (long));
	
	hlg = landgetglobals ();
	
	err = (*(**hlg).eventsendcallback) (&event, &reply, 
		
		(AESendMode) kAEWaitReply + kAEAlwaysInteract + kAECanSwitchLayer, (AESendPriority) kAEHighPriority, 60L, nil, nil);
	
	err = WakeUpProcess (&psn);
	
	err = SetFrontProcess (&psn);
	
	return (true); /***/
	} /*activateapplicationwindow*/
#endif

/* above not supported for MS Windows */



