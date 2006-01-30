
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
#include "threads.h"
#include "shell.h"
#include "lang.h"

#include "CallMachOFrameWork.h"
#include "memory.h"
#include "fcntl.h" /* 2006-01-10 creedon */


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


/* prototypes */
static boolean unixshellcallinit (void);
static boolean unixshellcallbackgroundtask (void);


boolean unixshellcall (Handle hcommand, Handle hreturn) {

	/*
	2006-01-10 creedon: laid down the groundwork for a timeoutsecs parameter, what is a good default? longinfinity? several minutes? I know that some commands I've done have taken 15 - 30 minutes
					 fread now reads in non-blocking mode, no error checking
	
	2005-10-02 creedon: changed buf size from 256 to 32, makes envrionment more responsive
					 added call to new unixshellcallbackgroundtask function so kernel doesn't lock up while waiting for a lot of data to be read
	
	7.0b51: Call the UNIX popen command, which evaluates a string as if it were typed on the command line. Verb: sys.unixShellCommand.
		     Code adapted by Timothy Paustian from Apple sample code.
		     This routine by PBS.
	*/

	FILE *f;
	char buf [32];
	long ct = 0;
	// unsigned long timeoutsecs = 60 * 5;
	// long timeoutticks = gettickcount () + (timeoutsecs * 60);

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
		
		if (ct > 0) // {
				
			if (!insertinhandle (hreturn, gethandlesize (hreturn), buf, ct))
				break;
				
			// timeoutticks = gettickcount () + (timeoutsecs * 60);
			// }
		
		if (feoffunc (f))
			break;
			
		/* if (gettickcount () > timeoutticks)
			break; */
		
		if (!unixshellcallbackgroundtask ())
			break;
		
		} /*while*/
	
	pclosefunc (f); /*pclose*/
	
	return (true);	
	} /*unixshellcall*/


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
	

OSStatus LoadFrameworkBundle(CFStringRef framework, CFBundleRef *bundlePtr) /*Adapted from Apple sample code*/
{
	OSStatus 	err;
	FSRef 		frameworksFolderRef;
	CFURLRef	baseURL;
	CFURLRef	bundleURL;
	
	assert(bundlePtr != nil);
	
	*bundlePtr = nil;
	
	baseURL = nil;
	bundleURL = nil;
	
	err = FSFindFolder(kOnAppropriateDisk, kFrameworksFolderType, true, &frameworksFolderRef);
	if (err == noErr) {
		baseURL = CFURLCreateFromFSRef(kCFAllocatorSystemDefault, &frameworksFolderRef);
		if (baseURL == nil) {
			err = coreFoundationUnknownErr;
		}
	}
	if (err == noErr) {
		bundleURL = CFURLCreateCopyAppendingPathComponent(kCFAllocatorSystemDefault, baseURL, framework, false);
		if (bundleURL == nil) {
			err = coreFoundationUnknownErr;
		}
	}
	if (err == noErr) {
		*bundlePtr = CFBundleCreate(kCFAllocatorSystemDefault, bundleURL);
		if (*bundlePtr == nil) {
			err = coreFoundationUnknownErr;
		}
	}
	if (err == noErr) {
	    if ( ! CFBundleLoadExecutable( *bundlePtr ) ) {
			err = coreFoundationUnknownErr;
	    }
	}

	// Clean up.
	
	if (err != noErr && *bundlePtr != nil) {
		CFRelease(*bundlePtr);
		*bundlePtr = nil;
	}
	if (bundleURL != nil) {
		CFRelease(bundleURL);
	}	
	if (baseURL != nil) {
		CFRelease(baseURL);
	}	
	
	return err;
} /*LoadFrameworkBundle*/


void *getframeworkfuncptr (CFStringRef framework, CFStringRef functionname) {

	/*
	2005-01-15 aradke: give us the name of a framework and the name of a function
	in the framework and we will retrieve a callable pointer to that function.
	*/
	
	CFBundleRef bundlePtr;
	
	 if (LoadFrameworkBundle (framework, &bundlePtr) != noErr)
		return (nil);
	
	return (CFBundleGetFunctionPointerForName (bundlePtr, functionname));
	} /*getframeworkfuncptr*/


static UInt32 gluetemplate[6] = {0x3D800000, 0x618C0000, 0x800C0000, 0x804C0004, 0x7C0903A6, 0x4E800420};

void *convertcfmtomachofuncptr (void *cfmfp) {
	
	/*
	2004-11-28 aradke: adapted from Apple's CFM_MachO_CFM sample:
	
	http://developer.apple.com/samplecode/CFM_MachO_CFM/CFM_MachO_CFM.html
	
	allocate a block of cfm glue code with instructions for calling the CFM function.
	the caller is responsible for disposal via disposemachofuncptr 
	*/

    UInt32 *mfp = (UInt32*) NewPtr (sizeof(gluetemplate));

    mfp[0] = gluetemplate[0] | ((UInt32) cfmfp >> 16);
    mfp[1] = gluetemplate[1] | ((UInt32) cfmfp & 0xFFFF);
    mfp[2] = gluetemplate[2];
    mfp[3] = gluetemplate[3];
    mfp[4] = gluetemplate[4];
    mfp[5] = gluetemplate[5];

    MakeDataExecutable (mfp, sizeof(gluetemplate));

    return ((void *) mfp);
	} /*convertcfmtomachofuncptr*/


void disposemachofuncptr (void *mfp) {
	
	if (mfp != nil)
		DisposePtr (mfp);
	} /*disposemachofuncptr*/


typedef struct tvectorstruct {
	ProcPtr	procaddr;
	UInt32	toc;
	} tvectorstruct, *tvectorptr;


void *convertmachotocfmfuncptr (void *mfp) {
	
	/*
	2004-11-28 aradke: allocate a fake TVector and set its procptr entry
	to the mach procptr handed to us by the caller. the toc entry can
	safely be set to nil since it's ignored in this context.
	*/
	
    tvectorptr cfmfp = (tvectorptr) NewPtr (sizeof(tvectorstruct));
	
	if (MemError()  == noErr && cfmfp != nil) {
		
		cfmfp->procaddr = (ProcPtr) mfp;
		cfmfp->toc = 0; /*ignored*/
		}

    return ((void *) cfmfp);
	} /*convertmachotocfmfuncptr*/


void disposecfmfuncptr (void *cfmfp) {
	
	if (cfmfp != nil)
		DisposePtr (cfmfp);
	} /*disposecfmfuncptr*/


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

