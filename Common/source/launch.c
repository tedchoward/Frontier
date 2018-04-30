
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


	#include <land.h>
	#include "mac.h"


#include "error.h"
#include "memory.h"
#include "strings.h"
#include "file.h"
#include "launch.h"
#include "threads.h"

	#include "aeutils.h"


#pragma pack(2)
typedef struct typrocessvisitinfo { /*2002-11-14 AR: for providing context to visitprocesses callbacks*/
	OSType idprocess;
	short ctprocesses;
	short nthprocess;
	ptrstring bsprocess;
	typrocessid *psnprocess;
	ptrfilespec fsprocess;
	} typrocessvisitinfo;
#pragma options align=reset
	
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


tylaunchcallbacks launchcallbacks = {nil};

#define maxwait 120 /*never wait more than 2 seconds for a stransition to occur*/




	static boolean system7apprunning (OSType *, bigstring, typrocessid *);
	



boolean launchapplication ( const ptrfilespec fsapp, const ptrfilespec fsdoc, boolean flbringtofront ) {

	//
	// 2006-06-24 creedon: for Mac, FSRef-ized
	//
	
	setfserrorparam ( fsapp ); // in case error message takes a filename parameter
	
	
		LSLaunchFSRefSpec launchspec;
		FSRef fsrefapp, fsrefdoc;
		OSStatus status;
		
		clearbytes ( &launchspec, longsizeof ( launchspec ) );
		
		if ( macgetfsref ( fsapp, &fsrefapp ) != noErr )
			return ( false );
		
		launchspec.appRef = &fsrefapp;

		if ( fsdoc != NULL ) {
			
			if ( macgetfsref ( fsdoc, &fsrefdoc ) == noErr ) {
			
				launchspec.numDocs = 1;
				
				launchspec.itemRefs = &fsrefdoc;
				}
			} // fsdoc
			
		if ( ! flbringtofront )
			launchspec.launchFlags |= kLSLaunchDontSwitch;
			
		status = LSOpenFromRefSpec ( &launchspec, NULL );

		if ( status == noErr )
			return ( true );
		
		return ( false );
	
		
	
	} // launchapplication



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
	


	static boolean matchprocess (bigstring bsprocess, ProcessInfoRec *processinfo) {
		
		//
		// 2006-06-24 creedon: FSRef-ized
		//
		// 7.1b37 PBS: deal with .app extensions on OS X.
		//
		// 1992-10-02 dmb: if bsprocess is a string4, see if it matches the process id
		//
		// 1992-09-04 dmb: see if bsprocess is the name or full path of the given process
		//
		
		bigstring bspath;
		OSType id;
		
		if (stringfindchar (':', bsprocess)) { // it's a path
		
			tyfilespec fs;
			FSRef fsref;
					
			FSRefParam pb;
			
			pb.ioVRefNum = processinfo->processAppSpec->vRefNum;
			pb.ioDirID = processinfo->processAppSpec->parID;
			pb.ioNamePtr = processinfo->processAppSpec->name;
			pb.newRef = &fsref;
			
			PBMakeFSRefSync(&pb);
			
			if (macmakefilespec (&fsref, &fs) != noErr)
				return (false);
				
			if (!filespectopath (&fs, bspath))
				return (false);
			
			return (equalidentifiers (bsprocess, bspath));
			}
		
		if (equalidentifiers (bsprocess, ( *processinfo ).processName))
			return (true);
		
		
			{
			bigstring bsprocessminussuffix;
			bigstring bssuffix;
			
			copystring (bsprocess, bsprocessminussuffix);
			
			if (pullstringsuffix (bsprocessminussuffix, bssuffix, '.')) // has suffix?
				
				if (equalidentifiers (bssuffix, "\papp")) // .app suffix?
				
					if (equalidentifiers (bsprocessminussuffix, ( *processinfo ).processName)) // match?
						return (true);
			}
		
		
		if (stringtoostype (bsprocess, &id))
			return (id == ( *processinfo ).processSignature);
		
		return (false);
		} // matchprocess
	


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


static boolean system6activate (bigstring bsprogram) {
#		pragma unused (bsprogram)

#	ifdef flsystem6

	bigstring bs;
	
	if (bsprogram == nil)
		return (false);
		
	filefrompath (bsprogram, bs);
		
	OpenDeskAcc (bs);
	
	#endif
	
	return (true);
	} /*system6activate*/

boolean activateapplication (bigstring bsprogram) {
	
	/*
	10/3/91 dmb: handle the possibility of bsprogram being a full path

	7.0b33 PBS: Workaround for changes in Win98 and Win2000 -- use AttachThreadInput
	to attach the current thread to the foreground window, so that our thread
	has permission to set the foreground window.
	*/
	
	boolean flsystem7;
	
	//Code change by Timothy Paustian Friday, June 9, 2000 2:36:02 PM
	//Changed because using SysEnvisons and SysEnvRec is like Really old style
	//This was changed to Gestalt calls with two new globals see mac.c initmacintosh
	
	flsystem7 = (gSystemVersion >= 0x0700);
	
	if (flsystem7)
		return (system7activate (bsprogram));
	else
		return (system6activate (bsprogram));

	} /*activateapplication*/


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


#ifdef flsystem6

static boolean system6apprunning (OSType *appid, bigstring appname) {
	
	ostypetostring (*appid, appname); /*better than nothing*/
	
	return (true);
	} /*system6apprunning*/

#endif

boolean findrunningapplication (OSType *appid, bigstring appname, typrocessid *psn) {
	
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

	} /*findrunningapplication*/



	static boolean system7frontprogram (bigstring bsprogram, boolean flfullpath) {
		
		//
		// 2006-06-24 creedon: FSRef-ized
		//
		// 1991-10-03 dmb: if flfullpath is true, return the full path instead of the process name
		//
		
		ProcessInfoRec processinfo;
		ProcessSerialNumber psn;
		FSSpec fs;
		
		processinfo.processInfoLength = sizeof (processinfo);
		
		processinfo.processName = bsprogram; // place to store process name
		
		processinfo.processAppSpec = &fs;
		
		if (oserror (GetFrontProcess (&psn)))
			return (false);
		
		if (oserror (GetProcessInformation (&psn, &processinfo)))
			return (false);
		
		if (flfullpath) {
		
			tyfilespec fst;
			FSRef fsref;
			
			FSpMakeFSRef (&fs, &fsref);
			
			macmakefilespec (&fsref, &fst);
			
			filespectopath (&fst, bsprogram);
			}
		
		return (true);
		
		} // system7frontprogram
	


boolean getfrontapplication (bigstring bsprogram, boolean flfullpath) {


		return (system7frontprogram (bsprogram, flfullpath));
	

	} /*getfrontapplication*/


static boolean processcountvisit (typrocessvisitinfoptr visitinfo, ProcessInfoRec *processinfo) {
	
	visitinfo->ctprocesses++;
	
	if (visitinfo->ctprocesses == visitinfo->nthprocess) { /*found the one we're looking for*/
		
		copystring (processinfo->processName, visitinfo->bsprocess);
		
		return (false); /*stop visiting*/
		}
	
	return (true); /*keep visiting*/
	} /*processcountvisit*/


short countapplications (void) {

	/*
	2002-11-14 AR: Switch from using globals to a local struct for providing
	context to the processcountvisit callback routine
	*/
		
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

	} /*countapplications*/


boolean getnthapplication (short n, bigstring bsprogram) {
	
	/*
	2002-11-14 AR: Switch from using globals to a local struct for providing
	context to the processcountvisit callback routine
	*/
	
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

	} /*getnthapplication*/



	static boolean getprocesspathvisit ( typrocessvisitinfoptr visitinfo, ProcessInfoRec *processinfo ) {
		
		//
		// 2006-06-28 creedon: FSRef-ized
		//
		// 1992-07-14 dmb: created
		//
		
		if ( matchprocess ( visitinfo->bsprocess, processinfo ) ) { // found the process
		
			OSStatus status;
			FSRef fsref;
		
			status = GetProcessBundleLocation ( &processinfo->processNumber, &fsref );
			
			macmakefilespec ( &fsref, visitinfo->fsprocess );
		
			return ( false ); // stop visiting
			
			}
		
		return ( true ); // keep visiting
		
		} // getprocesspathvisit
		


boolean getapplicationfilespec (bigstring bsprogram, ptrfilespec fs) {
	
	//
	// 2006-06-18 creedon: for Mac, FSRef-ized
	//
	// 2006-02-17 aradke:	Clear filespec before doing our thing on Mac too, so the caller won't crash if he ignores a false
	//				return value and tries to access the filespec anyway. Our only caller with a non-nil bsprogram
	//				currently is sys.getAppPath (shellsysverbs.c) and it does exactly that.
	//
	// 2004-10-26 aradke:	New Carbon/Mach-O implementation for obtaining a filespec to our application bundle, i.e. the
	//				Frontier.app "folder".
	//
	// 2002-11-14 AR:	Switch from using globals to a local struct for providing context to the getprocesspathvisit callback
	//			routine
	//
	// 2000-06-09 Timothy Paustian:	Changed because using SysEnvisons and SysEnvRec is like Really old style. This was
	//						changed to Gestalt calls with two new globals see mac.c initmacintosh.	
	//
	// 5.0.2b21 dmb: clear unused fields on Win
	//

	
		typrocessvisitinfo info;
		boolean flsystem7;
		
		clearbytes ( fs, sizeof ( *fs ) );

		flsystem7 = (gSystemVersion >= 0x0700);
		
		if (!flsystem7)
			return (false);
		
		if (bsprogram == nil) { // get path to this process
			
			#if TARGET_RT_MAC_MACHO
			
				CFBundleRef mybundleref;
				CFURLRef myurlref;
				FSRef fsref;
				boolean res;
				
				mybundleref = CFBundleGetMainBundle();
				
                if (mybundleref == NULL) {
                    fprintf(stderr, "mybundleref == NULL");
					return (false);
                }
				
				myurlref = CFBundleCopyBundleURL(mybundleref);
				
                if (myurlref == NULL) {
                    fprintf(stderr, "myurlref == NULL");
					return (false);
                }
            
                CFStringRef urlStr = CFURLGetString(myurlref);
                const char *urlCstring = CFStringGetCStringPtr(urlStr, kCFStringEncodingMacRoman);
                fprintf(stderr, "bundle url = '%s'\n", urlCstring);

				res = CFURLGetFSRef ( myurlref, &fsref );
				
				CFRelease(myurlref);
				
				if (!res)
					return (false);
				
				return ( macmakefilespec ( &fsref, fs ) == noErr );
				
			#else
			
				ProcessInfoRec processinfo;
				ProcessSerialNumber psn;
				
				processinfo.processInfoLength = sizeof (processinfo);
				
				processinfo.processName = nil; // place to store process name
				
				processinfo.processAppSpec = fs; // place to store process filespec
				
				psn.highLongOfPSN = 0;
				
				psn.lowLongOfPSN = kCurrentProcess;
				
				if (GetProcessInformation (&psn, &processinfo) != noErr)
					return (false);
				
				return (true);
				
			#endif
			
			}
		
		initprocessvisitinfo (&info);
		
		info.bsprocess = bsprogram; // point to caller's storage
		
		info.fsprocess = fs; // can be same storage as bsprogram -- or not
		
		return (!visitprocesses (&getprocesspathvisit, &info));
		

	
	} // getapplicationfilespec


boolean executeresource (ResType type, short id, bigstring bsname) {
	
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
	
		(*(pascal void (*) (void)) hcode) ();
		
	#else
	
		(*(pascal void (*) (void)) hcode) ();
	
	#endif
	
	unlockhandle (hcode);
	
	ReleaseResource (hcode);
	
	return (true);

	} /*executeresource*/


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


typrocessid getcurrentprocessid (void) {
	
	ProcessSerialNumber psn;
	
	GetCurrentProcess (&psn);

	
	return (psn);
	} /*getcurrentprocessid*/


boolean iscurrentapplication (typrocessid psn) {
	
	ProcessSerialNumber currentpsn;
	
	Boolean flsame;
	OSErr err;

	GetCurrentProcess (&currentpsn);

	err = SameProcess (&psn, &currentpsn, &flsame);
	
	return ((err == noErr) && flsame);
	
	} /*iscurrentapplication*/

boolean isfrontapplication (typrocessid psn) {

	ProcessSerialNumber frontpsn;
	Boolean flsame;
	OSErr err;
	
	GetFrontProcess (&frontpsn);
	
	err = SameProcess (&psn, &frontpsn, &flsame);
	
	return ((err == noErr) && flsame);

	} /*isfrontapplication*/


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

/* above not supported for MS Windows */



