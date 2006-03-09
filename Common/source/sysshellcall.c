
/*	$Id$    */

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

#include "memory.h"
#include "strings.h"
#include "threads.h"
#include "sysshellcall.h"


#if defined(MACVERSION) && (TARGET_API_MAC_CARBON == 1)

#include "shell.h"
#include "lang.h"
#include "CallMachOFramework.h"
#include <fcntl.h> /* 2006-01-10 creedon */

/* 2006-01-29 creedon - define the following for CodeWarrior compilation because it doesn't have these defined in its headers, as Xcode does */
#ifdef __MWERKS__
	#define	F_GETFL		3		/* get file status flags */
	#define	F_SETFL		4		/* set file status flags */
	#define	O_NONBLOCK	0x0004	/* no delay */
#endif //__MWERKS__

/*System.framework functions: popen, pclose, fread, fcntl, feof, and fileno.*/

typedef FILE* (*popenptr) (const char* command, const char *type);
typedef int (*pcloseptr) (FILE* f);
typedef size_t (*freadptr) (void *ptr, size_t size, size_t nobj, FILE* f);
typedef int (*fcntlptr) (int fd, int cmd, int arg); /* 2006-01-10 creedon */
typedef int (*feofptr) (FILE *f); /* 2006-01-10 creedon */
typedef int (*filenoptr) (FILE *f); /* 2006-01-10 creedon */
	 
static popenptr popenfunc;
static pcloseptr pclosefunc;
static freadptr freadfunc;
static fcntlptr fcntlfunc; /* 2006-01-10 creedon */
static feofptr feoffunc; /* 2006-01-10 creedon */
static filenoptr filenofunc; /* 2006-01-10 creedon */

static boolean unixshellcallinited = false;
static CFBundleRef sysBundle = nil;


static boolean unixshellcallinit (void) {

	/*
	2006-01-10 creedon: added fcntlfunc, feoffunc and filenofunc
	
	7.0b51 PBS: load the bundle and get the function pointers the first time called.
	*/
	
	if (unixshellcallinited) /*already inited*/
		return (true);
		
	if (sysBundle == nil)
		 if (LoadFrameworkBundle (CFSTR ("System.framework"), &sysBundle) != noErr)
		 	return (false);
		 
	popenfunc = (popenptr) CFBundleGetFunctionPointerForName (sysBundle, CFSTR ("popen"));	

	if (popenfunc == nil)
		return (false);
		
	freadfunc = (freadptr) CFBundleGetFunctionPointerForName (sysBundle, CFSTR ("fread"));
	
	if (freadfunc == nil)
		return (false);
	
	pclosefunc = (pcloseptr) CFBundleGetFunctionPointerForName (sysBundle, CFSTR ("pclose"));
	
	if (pclosefunc == nil)
		return (false);

	fcntlfunc = (fcntlptr) CFBundleGetFunctionPointerForName (sysBundle, CFSTR ("fcntl"));
	
	if (fcntlfunc == nil)
		return (false);

	feoffunc = (feofptr) CFBundleGetFunctionPointerForName (sysBundle, CFSTR ("feof"));
	
	if (feoffunc == nil)
		return (false);

	filenofunc = (filenoptr) CFBundleGetFunctionPointerForName (sysBundle, CFSTR ("fileno"));
	
	if (filenofunc == nil)
		return (false);

	unixshellcallinited = true;
	
	return (true);	
	} /*unixshellcallinit*/


static boolean unixshellcallbackgroundtask (void) {

	/*
	2005-10-02 creedon: created, cribbed from OpenTransportNetEvents.c: fwsbackgroundtask
	*/

	boolean fl = true;

	if (inmainthread ()) {
		EventRecord ev;
		short mask = osMask|activMask|mDownMask|keyDownMask; // |highLevelEventMask|updateMask
		long sleepTime = 6;	// 1/10 of a second by default
		
		if (WaitNextEvent (mask, &ev, sleepTime, nil)) /* might return false to indicate a null event, but that's not an error */
			fl = shellprocessevent (&ev);
		}
	else
		fl = langbackgroundtask (true);
	
	return (fl);
	}/* unixshellcallbackgroundtask */


boolean unixshellcall (Handle hcommand, Handle hreturn) {

	/*
	2006-01-29 creedon: change buf from 32 to 1024 to increase performance of function,
		we can do this now because fread is now in non-blocking mode, kernel remains repsonsive
	
	2006-01-10 creedon: laid down the groundwork for a timeoutsecs parameter, what is a good default?
		longinfinity? several minutes? I know that some commands I've done have taken 15 - 30 minutes
		fread now reads in non-blocking mode, no error checking
	
	2005-10-02 creedon: changed buf size from 256 to 32, makes envrionment more responsive
		added call to new unixshellcallbackgroundtask function so kernel doesn't lock up
		while waiting for a lot of data to be read
	
	7.0b51: Call the UNIX popen command, which evaluates a string as if it were typed
		ôn the command line. Verb: sys.unixShellCommand.
		Code adapted by Timothy Paustian from Apple sample code.
		This routine by PBS.
	*/

	FILE *f;
	char buf [1024];
	long ct = 0;

	if (!unixshellcallinit ())
		return (false);
		
	if (!enlargehandle (hcommand, 1, "\0"))
		return (false);
	
	lockhandle (hcommand);
	
	f = popenfunc (*hcommand, "r"); /*popen*/
	
	unlockhandle (hcommand);
	
	fcntlfunc (filenofunc (f), F_SETFL, fcntlfunc (filenofunc (f), F_GETFL, 0) | O_NONBLOCK);

	while (true) {
	
		ct = freadfunc (buf, 1, sizeof buf, f); /*fread*/
		
		if (ct > 0)
			if (!enlargehandle (hreturn, ct, buf))
				break;
		
		if (feoffunc (f))
			break;
		
		if (!unixshellcallbackgroundtask ())
			break;
		
		} /*while*/
	
	pclosefunc (f); /*pclose*/
	
	return (true);	
	} /*unixshellcall*/

#endif //MACVERSION


#ifdef WIN95VERSION

static boolean getcmdshell (Handle *hshell) {

	/*
	2006-03-09 aradke: return name or path of command shell in hshell.
		check the COMSPEC environment variable first. if it does not exist, resort
		to guessing the name of the shell based on the current OS version.
		caller is responsible for disposing the handle if we return true.
	*/
	
	Handle h;
	long res;

	res = GetEnvironmentVariable("COMSPEC", nil, 0);

	if (res > 0) { /*var exists, allocate buffer and get its value*/
		
		if (!newclearhandle (res, &h))
			return (false);
		
		lockhandle (h);

		res = GetEnvironmentVariable("COMSPEC", *h, res);	/*FIXME: check result, dispose h*/
		
		unlockhandle (h);

		assert (res == gethandlesize (h) - 1);
		
		sethandlesize (h, res);	/*drop trailing nil char*/
		}
	else {
		OSVERSIONINFO osinfo;
		bigstring bs;

		osinfo.dwOSVersionInfoSize = sizeof (osinfo);
		
		GetVersionEx (&osinfo); /*FIXME: check result, report error*/

		if (osinfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) {
			copystring ("\x0b" "command.com", bs); /*Win95/98/ME: actual DOS command interpreter*/
			}
		else {
			copystring ("\x07" "cmd.exe", bs); /*WinNT and successors: DOS emulation*/
			}

		if (!newtexthandle (bs, &h))
			return (false);
		}	
	
	*hshell = h;

	return (true);
	} /*getcmdshell*/


boolean runcmdshell (Handle hshell, Handle hcommand, HANDLE *hout) {

	/*
	2006-03-09 aradke: launch the command shell as a child process.
	
	TO DO:
		- check length of hcmdline < MAX_PATH
		- make sure handles in processinfo struct are closed
		- before creating process, check whether hshell exists
		- if we can't access hshell, do a search path lookup
		- proper error checking and reporting
	*/
	
	Handle hcmdline = nil;
	SECURITY_ATTRIBUTES securityinfo;
	STARTUPINFO startupinfo;
	PROCESS_INFORMATION processinfo;
	HANDLE hpiperead = nil;
	HANDLE hpipewrite = nil;
	DWORD attr;
	boolean fl;
	
	/*create pipe for reading from command shell*/
	clearbytes (&securityinfo, sizeof (securityinfo));
	securityinfo.nLength				= sizeof (securityinfo);
	securityinfo.lpSecurityDescriptor	= nil;
	securityinfo.bInheritHandle			= true;
	
	fl = CreatePipe (&hpiperead, &hpipewrite, &securityinfo, nil);
	
	if (!fl)
		goto exit;
	
	SetHandleInformation (hpiperead, HANDLE_FLAG_INHERIT, 0);
	
	/*init structs for creating process*/
	
	clearbytes (&startupinfo, sizeof (startupinfo));
	startupinfo.cb = sizeof (startupinfo);
	startupinfo.dwFlags		= STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	startupinfo.hStdInput	= GetStdHandle (STD_INPUT_HANDLE);
	startupinfo.hStdOutput	= hpipewrite;
	startupinfo.hStdError	= GetStdHandle (STD_ERROR_HANDLE);
	startupinfo.wShowWindow	= SW_HIDE;
	
	clearbytes (&processinfo, sizeof (processinfo));
	
	/*synthesize command string*/
	
	if (!inserttextinhandle (hcommand, 0, "\x04" " /c "))
		goto exit;
	
	if (!concathandles (hshell, hcommand, &hcmdline))
		goto exit;
		
	if (!enlargehandle (hshell, 1, "\0"))
		goto exit;

	if (!enlargehandle (hcmdline, 1, "\0"))
		goto exit;
	
	/*check whether command shell can be accessed*/
	
	lockhandle (hshell);

	attr = GetFileAttributes(*hshell);

	unlockhandle (hshell);
	
	if (attr == INVALID_FILE_ATTRIBUTES)
		goto exit; /*not found, try searching PATH for command shell?*/
	
	/*create command shell process*/
	
	lockhandle (hshell);
	lockhandle (hcmdline);
	
	fl = CreateProcess (
			*hshell,		/*IN LPCSTR lpApplicationName*/
			*hcmdline,		/*IN LPSTR lpCommandLine*/
			nil,			/*IN LPSECURITY_ATTRIBUTES lpProcessAttributes*/
			nil,			/*IN LPSECURITY_ATTRIBUTES lpThreadAttributes*/
			true,			/*IN BOOL bInheritHandles*/
			0,				/*IN DWORD dwCreationFlags*/
			nil,			/*IN LPVOID lpEnvironment: use parent's*/
			nil,			/*IN LPCSTR lpCurrentDirectory: use parent's*/
			&startupinfo,	/*IN LPSTARTUPINFOA lpStartupInfo*/
			&processinfo);	/*OUT LPPROCESS_INFORMATION lpProcessInformation*/
	
	unlockhandle (hshell);
	unlockhandle (hcmdline);
	
	/*handle result*/
	
	if (!fl)
		goto exit;
	
	CloseHandle (hpipewrite);	/*inherited by child process*/
	CloseHandle (processinfo.hProcess);
	CloseHandle (processinfo.hThread);
	
	disposehandle (hcmdline);
	
	*hout = hpiperead;
	
	return (true);
	
exit:
	if (!hpiperead)
		CloseHandle (hpiperead);
	
	if (!hpipewrite)
		CloseHandle (hpipewrite);
	
	disposehandle (hcmdline);
	
	return (false);	
	} /*runcmdshell*/


boolean readcmdresult (HANDLE hread, Handle hreturn) {
	
	char buf [1024];
	long ct = 0;
	boolean fl;
	
	while (true) {
		
		releasethreadglobals ();
		
		fl = ReadFile (hread, buf, sizeof (buf), &ct, nil);
		
		grabthreadglobals ();
		
		if (!fl) /*an error occurred*/
			break;

		if (fl && (ct == 0)) /*end of file*/
			break;

		if (ct > 0)				
			if (!enlargehandle (hreturn, ct, buf))
				break;
		} /*while*/
	
	CloseHandle (hread);
	
	return (true);
	} /*readcmdresult*/


boolean winshellcall (Handle hcommand, Handle hreturn) {

	/*
	2006-03-09 aradke: call the Windows shell and execute the given command.
		our caller owns hcommand and hreturn, whether we succeed or not.
		this implementation relies on the win32 api only and does not
		require popen etc to be available in the c runtime library.
	*/

	Handle hshell = nil;
	HANDLE hread;

	if (!getcmdshell (&hshell))
		goto exit;
	
	if (!runcmdshell (hshell, hcommand, &hread))
		goto exit;

	if (!readcmdresult (hread, hreturn))
		goto exit;

	disposehandle (hshell);

	return (true);

exit:

	disposehandle (hshell);

	return (false);
	} /*winshellcall*/


#if 0 //!__MWERKS__

#include <stdio.h>
#include <stdlib.h>

boolean winshellcall (Handle hcommand, Handle hreturn) {

	/*
	2006-03-09 aradke: simple version relying on popen to be available
		from the c runtime library. that is the case with microsoft's
		runtime library, but not with metrowerks. see our own
		implementation above using only win32 api calls.
	*/

	FILE *f;
	char buf [1024];
	long ct = 0;
		
	if (!enlargehandle (hcommand, 1, "\0"))
		return (false);
	
	lockhandle (hcommand);
	
	releasethreadglobals ();

	f = _popen (*hcommand, "r");

	grabthreadglobals ();

	unlockhandle (hcommand);

	while (true) {
		
		releasethreadglobals ();

		ct = fread (buf, 1, sizeof (buf), f); /*fread*/
		
		grabthreadglobals ();

		if (ct > 0)				
			if (!enlargehandle (hreturn, ct, buf))
				break;
		
		if (feof (f))
			break;

		} /*while*/
	
	_pclose (f);
	
	return (true);	
	} /*winshellcall*/

#endif

#endif //WIN95VERSION

