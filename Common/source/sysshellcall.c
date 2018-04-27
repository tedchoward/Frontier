
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
#include "error.h"
#include "langinternal.h" /*for error strings*/
#include "resources.h" /*for error strings*/
#include "shell.h"
#include "sysshellcall.h"



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
		on the command line. Verb: sys.unixShellCommand.
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




