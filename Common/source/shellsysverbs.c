
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

#ifdef MACVERSION 
#include <land.h>
#define wsprintf sprintf
#endif

#include "ops.h"
#include "memory.h"
#include "error.h"
#include "file.h"
#include "resources.h"
#include "scrap.h"
#include "strings.h"
#include "launch.h"
#include "notify.h"
#include "shell.h"
#include "shellmenu.h"
#include "lang.h"
#include "langexternal.h"
#include "langinternal.h"
#include "langipc.h"
#include "kernelverbs.h"
#include "kernelverbdefs.h"
#include "tablestructure.h"
#if flruntime
	#include "outpost.h"
#else
	#include "shellprivate.h"
#endif
#include "process.h"
#include "processinternal.h"
#if TARGET_API_MAC_CARBON == 1
	#include "CallMachOFrameWork.h"
#endif

#include "langsystem7.h"  //6.1b7 AR: we need coercetolist
#include "tableverbs.h"  //6.1b7 AR: we need gettablevalue
#include "tableinternal.h" //6.1b7 AR: we need tablepacktable and tableunpacktable
#include "serialnumber.h" //7.1b34 dmb: new isvalidserialnumber verb

#define systemevents (osMask | activMask)

boolean frontierversion (tyvaluerecord *v); /* 2002-10-13 AR: also used in langhtml.c */
	
boolean sysos (tyvaluerecord *v); /* 2002-10-13 AR: also used in langhtml.c */


static tyfilespec programfspec;

static bigstring bsfrontierversion;


typedef enum tysystoken { /*verbs that are processed by sys*/
	
	systemversionfunc,
	
	systemtaskfunc,
	
	browsenetworkfunc,
	
	apprunningfunc,
	
	frontappfunc,
	
	bringapptofrontfunc,
	
	countappsfunc,
	
	getnthappfunc,
	
	getapppathfunc,
	
	memavailfunc,
	
	machinefunc,

	osfunc,

	getenvironmentvariablefunc,

	setenvironmentvariablefunc,
	
	unixshellcommandfunc,

	ctsysverbs
	} tysystoken;


typedef enum tylaunchtoken { /*verbs that are processed by launch*/
	
	applemenufunc,
	
	launchappfunc,
	
	launchappwithdocfunc,
	
	executeresourcefunc,
	
	anythingfunc,
	
	ctlaunchverbs
	} tylaunchtoken;


typedef enum tyfrontiertoken { /*verbs that are processed by frontier*/
	
	programpathfunc,
	
	filepathfunc,
	
	#if !flruntime
	
	agentsenablefunc,
	
	#endif
	
	requesttofrontfunc,
	
	isruntimefunc,
	
	countthreadsfunc,
	
	isnativefunc,
	
	reclaimmemoryfunc,
	
	frontierversionfunc,

	hashstatsfunc,

	gethashloopcountfunc,

	hideapplicationfunc,

	isvalidserialnumberfunc,

	showapplicationfunc,

	ctfrontierverbs
	} tyfrontiertoken;


typedef enum tyclipboardtoken { /*verbs that are processed by clipboard*/
	
	getscrapfunc,
	
	putscrapfunc,
	
	ctclipboardverbs
	} tyclipboardtoken;


#if threadverbs

typedef enum tythreadtoken {
	
	existsfunc,
	
	evaluatefunc,

	callscriptfunc,
	
	getcurrentfunc,
	
	getcountfunc,
	
	getnththreadfunc,
	
	sleepfunc,
	
	sleepforfunc,

	sleepticksfunc,
	
	issleepingfunc,
	
	wakefunc,
	
	killfunc,
	
	gettimeslicefunc,
	
	settimeslicefunc,
	
	getdefaulttimeslicefunc,
	
	setdefaulttimeslicefunc,
	/*
	begincriticalfunc,
	
	endcriticalfunc,
	*/
	statsfunc,
	
	ctthreadverbs
	
	} tythreadtoken;

#endif


static boolean getscrapverb (hdltreenode hparam1, tyvaluerecord *v) {
	
	/*
	5.0a10 dmb: must open/close clipboard. byte swap binary type
	*/

	OSType type;
	Handle hscrap;
	hdlhashtable htable;
	bigstring bs;
	boolean fl = false;
		
	setbooleanvalue (false, v); /*default return*/
	
	if (!getostypevalue (hparam1, 1, &type))
		return (false);
	
	flnextparamislast = true;
	
	if (!getvarparam (hparam1, 2, &htable, bs)) /*returned handle holder*/
		return (false);
	
	shellwritescrap (anyscraptype); /*export our private scrap, in necessary*/
	
	if (!newemptyhandle (&hscrap))
		return (false);
	
	if (openclipboard ()) {
		
		fl = getscrap (type, hscrap);
		
		closeclipboard ();
		}
	
	if (!fl) {
		
		disposehandle (hscrap);
		
		return (true); /*not a runtime error; return value is false*/
		}
	
	memtodisklong (type);
	
	if (!insertinhandle (hscrap, 0L, &type, sizeof (type))) {
		
		disposehandle (hscrap);
		
		return (false);
		}
	
	if (!langsetbinaryval (htable, bs, hscrap)) /*probably a memory error*/
		return (false);
	
	(*v).data.flvalue = true;
	
	return (true);
	} /*getscrapverb*/


static boolean putscrapverb (hdltreenode hparam1, tyvaluerecord *v) {
	
	/*
	5.0a10 dmb: must open/close clipboard
	*/

	OSType type;
	Handle hbinary;
	OSType bintype;
	
	if (!getostypevalue (hparam1, 1, &type))
		return (false);
	
	flnextparamislast = true;
	
	if (!getbinaryvalue (hparam1, 2, false, &hbinary))
		return (false);
	
	pullfromhandle (hbinary, 0L, sizeof (bintype), &bintype);
	
	if (openclipboard ()) {
		
 		releasethreadglobals ();
		
		resetscrap ();

		grabthreadglobals ();
		
		(*v).data.flvalue = putscrap (type, hbinary);

		closeclipboard ();
		}
	
	return (true);
	} /*putscrapverb*/


static boolean shellsysverbwaitroutine (void) {
	
	/*
	12/24/92 dmb: added special case for when yield is disabled
	
	2.1b9 dmb: systemevents are now just osMask. waiting for all updates 
	is dangerous, 'cause they may not be servicable
	
	3.0b15 dmb: systemevents need to include activate event too. otherwise, 
	shelleventavail (EventAvail) can return false when a juggle is indeed 
	pending.
	*/
	
	boolean fl;
	
	if (flscriptrunning)
		fl = langpartialeventloop ((short) systemevents);
	else
		fl = shellpartialeventloop ((short) systemevents);
	
	return (fl);
	} /*shellsysverbwaitroutine*/


boolean frontierversion (tyvaluerecord *v) { //6.1d1 AR: needed in langhtml.c
	
	if (stringlength (bsfrontierversion) == 0)
		filegetprogramversion (bsfrontierversion);
	
	return (setstringvalue (bsfrontierversion, v));
	} /*frontierversion*/


boolean sysos (tyvaluerecord *v) { //6.1d1 AR: needed in langhtml.c

	#ifdef MACVERSION
		//#if TARGET_API_MAC_CARBON == 1
		//return (setstringvalue(osCarbon, v));
		//#else
		return (setstringvalue (osMacOS, v));
	#endif
	
	//#endif
	
	#ifdef WIN95VERSION
		OSVERSIONINFO osinfo;

		osinfo.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);

		GetVersionEx (&osinfo);

		if (osinfo.dwPlatformId == VER_PLATFORM_WIN32_NT)
			return (setstringvalue (osWinNT, v));

		return (setstringvalue (osWin95, v));
	#endif
	} /*sysos*/


static boolean sysfunctionvalue (short token, hdltreenode hparam1, tyvaluerecord *vreturned, bigstring bserror) {
	
	/*
	2/12/92 dmb: do partialeventloop on systemtask & bringapptofrontfunc
	
	5/20/92 dmb: do processyield directly on systemtaskfunc
	
	8/11/92 dmb: make apprunningfunc accept a string or an ostype
	
	1/18/93 dmb: in systemtaskfunc, don't call processyield directly; use langbackgroundtask

	5.0b12 dmb: in systemtaskfunc, set flresting to false to make sure we don't slow
	down too much
	
	5.0b16 dmb: undo that change. it affect performance adversely if many threads do it.
	*/
	
	register tyvaluerecord *v = vreturned;
	
	setbooleanvalue (false, v); /*assume the worst*/
	
	switch (token) { /*these verbs don't need any special globals pushed*/
		
		case systemversionfunc: {
			bigstring bs;
			
			getsystemversionstring (bs, nil);
			
			if (!langcheckparamcount (hparam1, 0))
				return (false);
						
			return (setstringvalue (bs, v));
			}
		
		case systemtaskfunc:
			if (!langcheckparamcount (hparam1, 0)) /*shouldn't have any parameters*/
				return (false);
			
			shellsysverbwaitroutine ();
			
			/*
			if (!processyield ())
				return (false);
			*/
			
			if (!langbackgroundtask (true))
				return (false);
			
			(*v).data.flvalue = true;
			
			return (true);
		
		case browsenetworkfunc:
			#ifdef MACVERSION
				return (langipcbrowsenetwork (hparam1, v));
			#endif
			#ifdef WIN95VERSION
			#pragma message ("WIN95: browsenetworkfunc - not yet implemented!")
				break;
			#endif

		case apprunningfunc: {
			OSType appid;
			bigstring bsapp;
			tyvaluerecord val;
			
			flnextparamislast = true;
			
			/*
			if (!getostypevalue (hparam1, 1, &appid))
				return (false);
			
			(*v).data.flvalue = findrunningapplication (&appid, nil);
			*/
			
			if (!getparamvalue (hparam1, 1, &val))
				return (false);
			
			if (val.valuetype == ostypevaluetype) {
				
				setemptystring (bsapp);
				
				appid = val.data.ostypevalue;
				}
			else {
				if (!coercetostring (&val))
					return (false);
				
				pullstringvalue (&val, bsapp);
				
				if (!stringtoostype (bsapp, &appid))
					appid = 0;
				}
			
			(*v).data.flvalue = findrunningapplication (&appid, bsapp, nil);
			
			return (true);
			}
		
		case frontappfunc: {
			bigstring bs;
			
			if (!langcheckparamcount (hparam1, 0))
				return (false);
			
			if (!getfrontapplication (bs, false))
				return (false);
			
			return (setstringvalue (bs, v));
			}
		
		case bringapptofrontfunc: {
			bigstring bs;
			
			flnextparamislast = true;
			
			if (!getstringvalue (hparam1, 1, bs))
				return (false);
			
			(*v).data.flvalue = activateapplication (bs);
			
			return (true);
			}
		
		case countappsfunc:
			if (!langcheckparamcount (hparam1, 0))
				return (false);
			
			return (setlongvalue (countapplications (), v));
		
		case getnthappfunc: {
			short n;
			bigstring bs;
			
			if (!getintvalue (hparam1, 1, &n))
				return (false);
			
			if (!getnthapplication (n, bs))
				setemptystring (bs);
			
			return (setstringvalue (bs, v));
			}
		
		case getapppathfunc: {
			bigstring bs;
			tyfilespec fs;
			
			flnextparamislast = true;
			
			if (!getstringvalue (hparam1, 1, bs))
				return (false);
			
			if (!getapplicationfilespec (bs, &fs))	/* 2006-02-17 aradke: initializes fs even if it fails */
				setemptystring (bs);
			
			return (setfilespecvalue (&fs, v));
			}
		
		case memavailfunc:
			{
			unsigned long memavail;

			#ifdef WIN95VERSION
				MEMORYSTATUS meminfo;

				meminfo.dwLength = sizeof (MEMORYSTATUS);

				GlobalMemoryStatus (&meminfo);

				memavail = meminfo.dwAvailVirtual;
			#endif

			#ifdef MACVERSION
				memavail = TempFreeMem();
			#endif
			
			if (!langcheckparamcount (hparam1, 0)) /*shouldn't have any parameters*/
				return (false);
			
			return (setlongvalue (memavail, v));
			}
		
		case machinefunc:
			
			#ifdef MACVERSION
				//Code change by Timothy Paustian Friday, June 16, 2000 3:13:09 PM
				//Changed to Opaque call for Carbon
				//Carbon only runs on PPC
				#if TARGET_API_MAC_CARBON
				return (setstringvalue (machinePPC, v));
				#else
				#if GENERATINGPOWERPC
				return (setstringvalue (machinePPC, v));
				#endif
				#if GENERATING68K
				return (setstringvalue (machine68K, v));
				#endif
				#endif

				
			#endif
			
			#ifdef WIN95VERSION
				return (setstringvalue (machinex86, v));
			#endif

			break;

		case osfunc:
			return (sysos (v));
			break;

		#ifdef WIN95VERSION
			case getenvironmentvariablefunc: {
				bigstring bsenvname;
				bigstring buf;
				DWORD res;
				
				flnextparamislast = true;
				
				if (!getstringvalue (hparam1, 1, bsenvname))
					return (false);

				nullterminate(bsenvname);

				res = GetEnvironmentVariable (stringbaseaddress(bsenvname), stringbaseaddress(buf), sizeof(buf)-2);

				if (res > sizeof(buf) - 2) {
					return (setbooleanvalue (false, v));		//safety valve
					}

				setstringlength (buf, res);

				return (setstringvalue (buf, v));
				}
				break;

			case setenvironmentvariablefunc: {
				bigstring bsenvname;
				bigstring bsenvval;
				bigstring bserror, bserror2;
				boolean res;
				
				if (!getstringvalue (hparam1, 1, bsenvname))
					return (false);

				flnextparamislast = true;
				
				if (!getstringvalue (hparam1, 2, bsenvval))
					return (false);

				nullterminate(bsenvname);
				nullterminate(bsenvval);

				res = SetEnvironmentVariable (stringbaseaddress(bsenvname), stringbaseaddress(bsenvval));

				if (res) {
					return (setbooleanvalue (true, v));	
					}

				getsystemerrorstring (GetLastError(), bserror);

				nullterminate(bserror);

				wsprintf (bserror2, "Can't set environment variable \"%s\" to \"%s\" because %s", stringbaseaddress (bsenvname), stringbaseaddress (bsenvval), stringbaseaddress (bserror));

				setstringlength (bserror2, strlen(stringbaseaddress(bserror2)));

				shellerrormessage (bserror2);

				return (setbooleanvalue (false, v));	
				}
				break;
		#endif
		
		#if TARGET_API_MAC_CARBON == 1
		
			case unixshellcommandfunc: { /*7.0b51 PBS: call shell on OS X*/
			
				Handle hcommand, hreturn;
				
				newemptyhandle (&hreturn);
				
				flnextparamislast = true;
				
				if (!getexempttextvalue (hparam1, 1, &hcommand))
					return (false);
										
				if (!unixshellcall (hcommand, hreturn)) {
				
					disposehandle (hreturn);
					
					disposehandle (hcommand);
					
					return (false);
					} /*if*/
				
				disposehandle (hcommand);
					
				return (setheapvalue (hreturn, stringvaluetype, v));
				}
		
		
		#endif
		
		default:
			break;
		}

	getstringlist (langerrorlist, unimplementedverberror, bserror);

	return (false);
	} /*sysfunctionvalue*/


static boolean launchfunctionvalue (short token, hdltreenode hparam1, tyvaluerecord *vreturned, bigstring bserror) {
	
	/*
	3/10/92 dmb: added launch.appWithDocument.  check default paths
	
	5/6/93 dmb: make sure that we return false is an oserror has occurred
	*/
	
	register tyvaluerecord *v = vreturned;
	
	setbooleanvalue (false, v); /*assume the worst*/
	
	oserror (noErr); /*clear now so we can check it at end*/
	
	switch (token) { /*these verbs don't need any special globals pushed*/
		
		#ifdef MACVERSION
		case applemenufunc: {
			bigstring bs;
			
			flnextparamislast = true;
			
			if (!getstringvalue (hparam1, 1, bs))
				return (false);
			
			(*v).data.flvalue = shellapplemenu (bs);
			
			break;
			}
		#endif
		
		case launchappfunc: {
			tyfilespec fsapp;
			
			flnextparamislast = true;
			
			if (!getfilespecvalue (hparam1, 1, &fsapp))
				return (false);
			
			(*v).data.flvalue = launchapplication (&fsapp, nil, false);
			
			break;
			}
		
		case launchappwithdocfunc: {
			tyfilespec fsapp, fsdoc;
			
			if (!getfilespecvalue (hparam1, 1, &fsapp))
				return (false);
			
			flnextparamislast = true;
			
			if (!getfilespecvalue (hparam1, 2, &fsdoc))
				return (false);
			
			(*v).data.flvalue = launchapplication (&fsapp, &fsdoc, false);
			
			break;
			}
		
		case executeresourcefunc: {
			ResType type;
			short id;
			
			if (!getostypevalue (hparam1, 1, &type))
				return (false);
			
			flnextparamislast = true;
			
			if (!getintvalue (hparam1, 2, &id))
				return (false);
			
			(*v).data.flvalue = executeresource (type, id, nil);
			
			break;
			}
		
		case anythingfunc:
			#ifdef MACVERSION
				return (filelaunchanythingverb (hparam1, v));
			#endif
			#ifdef WIN95VERSION
			#pragma message ("WIN95: anythingfunc - not yet implemented! (uses filelaunchanthingverb)")
				getstringlist (langerrorlist, unimplementedverberror, bserror);

				return (false);
			#endif
		
		default:
			getstringlist (langerrorlist, unimplementedverberror, bserror);

			return (false);
		}
	
	return (getoserror () == noErr);
	} /*launchfunctionvalue*/


extern long fullpathloopcount;

extern boolean hashstatsverb (tyvaluerecord *v);

static boolean frontierfunctionvalue (short token, hdltreenode hparam1, tyvaluerecord *vreturned, bigstring bserror) {
	
	/*
	6/5/92 dmb: added isruntime func
	
	6/1/93 dmb: when vreturned is nil, return whether or not verb token must 
	be run in the Frontier process
	*/
	
	register tyvaluerecord *v = vreturned;
	
	if (v == nil) { /*need Frontier process?*/
		
		switch (token) {
			
			case requesttofrontfunc:
				return (true);
			
			default:
				return (false);
			}
		}
	
	setbooleanvalue (false, v); /*assume the worst*/
	
	switch (token) { /*these verbs don't need any special globals pushed*/
		
		case programpathfunc: {
			
			if (!langcheckparamcount (hparam1, 0))
				return (false);
			
			return (setfilespecvalue (&programfspec, v));
			}
		
		case filepathfunc: {
			tyfilespec fs;
			
			if (!langcheckparamcount (hparam1, 0))
				return (false);
			
			#if flruntime
			
			fs = currentfile.fspec;
			
			#else
			
			shellpushfrontrootglobals ();
			
			windowgetfspec (shellwindow, &fs);
			
			shellpopglobals ();
			
			#endif
			
			return (setfilespecvalue (&fs, v));
			}
		
		#if !flruntime
		
		case agentsenablefunc: {
			boolean fl;
			
			flnextparamislast = true;
			
			if (!getbooleanvalue (hparam1, 1, &fl))
				return (false);
			
			(*v).data.flvalue = setagentsenable (fl);
			
			return (true);
			}
		
		#endif
		
		case requesttofrontfunc: {
			bigstring bsmessage;
			
			flnextparamislast = true;
			
			if (!getstringvalue (hparam1, 1, bsmessage))
				return (false);

			#ifdef MACVERSION		
				(*v).data.flvalue = shellisactive () || notifyuser (bsmessage);
			#endif

			#ifdef WIN95VERSION
			#pragma message ("WIN95: case requesttofrontfunc,  windows case currently forced.")
				(*v).data.flvalue = shellisactive() || SetForegroundWindow (hwndActive);
			#endif

			return (true);
			}
		
		case isruntimefunc: {
			if (!langcheckparamcount (hparam1, 0))
				return (false);
			
			#if flruntime
			
			(*v).data.flvalue = true;
			
			#endif
			
			return (true);
			}
		
		case countthreadsfunc: {
			if (!langcheckparamcount (hparam1, 0))
				return (false);
			
			return (setlongvalue (processthreadcount (), v));
			}
		
		case isnativefunc:
			#if __powerc || __GNUC__
				(*v).data.flvalue = true;
			#else
			
				(*v).data.flvalue = false;
			#endif
			
			return (true);
		
		case reclaimmemoryfunc: {
			long ctbytes = longinfinity;

			if (!langcheckparamcount (hparam1, 0))
				return (false);
			
			hashflushcache (&ctbytes);

			return (setlongvalue (longinfinity - ctbytes, v));
			}
		
		case frontierversionfunc:
			return (frontierversion (v));
		
		case hashstatsfunc:{
			if (!langcheckparamcount (hparam1, 0))
				return (false);

			return (hashstatsverb (v));
			}

		case gethashloopcountfunc:{
			long myx;

			if (!langcheckparamcount (hparam1, 0))
				return (false);
				
			myx = fullpathloopcount;

			fullpathloopcount = 0;

			return (setlongvalue (myx, v));
			}

		case hideapplicationfunc: { /*7.1b9 PBS: minimize to system tray*/
			
			#ifdef WIN95VERSION
				
				releasethreadglobals ();

				ShowWindow (shellframewindow, SW_HIDE);

				grabthreadglobals ();
			
			#endif

			return (setbooleanvalue (true, v));
			}

		case isvalidserialnumberfunc: { /*7.1b34 dmb: expose the functionality as a verb*/
			bigstring bssn;
			
			flnextparamislast = true;
			
			if (!getstringvalue (hparam1, 1, bssn))
				return (false);

			(*v).data.flvalue = isvalidserialnumber (bssn);

			return (true);
			}
		
		case showapplicationfunc: {	/*2004-11-28 aradke: re-emerge from system tray*/

			#ifdef WIN95VERSION
				
				releasethreadglobals ();

				ShowWindow (shellframewindow, SW_SHOW);

				grabthreadglobals ();
			
			#endif

			return (setbooleanvalue (true, v));
			}

		default:
			return (false);
		}
	} /*frontierfunctionvalue*/


static boolean clipboardfunctionvalue (short token, hdltreenode hparam1, tyvaluerecord *vreturned, bigstring bserror) {
	
	register tyvaluerecord *v = vreturned;
	#ifdef MACVERSION
		typrocessid processid;
	#endif
	
	setbooleanvalue (false, v); /*assume the worst*/
	
	#ifdef MACVERSION
		processid = getcurrentprocessid ();
		
		if (!isfrontapplication (processid)) {
			
			getstringlist (langerrorlist, cantbackgroundclipboard, bserror);
			
			return (false);
			}
	#endif
	
	switch (token) { /*these verbs don't need any special globals pushed*/
		
		case getscrapfunc:
			return (getscrapverb (hparam1, v));
		
		case putscrapfunc:
			return (putscrapverb (hparam1, v));
		
		default:
			return (false);
		}
	} /*clipboardfunctionvalue*/


static boolean getthreadvalue (hdltreenode hfirst, short pnum, hdlprocessthread *hthread) {
	
	long id;
	
	if (!getlongvalue (hfirst, pnum, &id))
		return (false);
	
	*hthread = getprocessthread (id);
	
	if (*hthread == nil) {
		
		langlongparamerror (badthreadiderror, id);
		
		return (false);
		}
	
	return (true);
	} /*getthreadvalue*/


static boolean threadverbprocessstarted (void) {
	
	/*
	we don't want Frontier's menus to dim when thread.evaluate's newly-added 
	process start.
	*/
	
	processnotbusy ();
	
	return (true);
	} /*threadverbprocessstarted*/


static boolean threaddisposecontext (void) {
	/*
	6.1b7 AR: For thread.callScript, dispose the context table.
	*/

	register hdlprocessrecord hp = currentprocess;

	if (hp != nil && (**hp).hcontext != nil)
		disposehashtable ((**hp).hcontext, true);

	return (true);
	}/**/


static boolean threadcallscriptverb (bigstring bsscriptname, tyvaluerecord vparams, hdlhashtable hcontext, tyvaluerecord *v) {

	/*
	8.0.4 dmb: handle running code values
	
	9.1b3 AR: copy bsscriptname to processrecord so the thread can be more easily
	identified in the system.compiler.threads table
	*/
	
	hdlprocessrecord hp;
	hdlprocessthread hthread;

	bigstring bsverb;
	boolean fl = false;
	boolean flchained = false;
	tyvaluerecord val;
	hdltreenode hfunctioncall;
	hdltreenode hparamlist;
	hdltreenode hcode;
	hdlhashtable htable;
	tyvaluerecord vhandler;
	hdlhashnode handlernode;
	
	/*build code tree, see langrunscript*/

	pushhashtable (roottable);
	
	fl = langexpandtodotparams (bsscriptname, &htable, bsverb);

	if (fl && htable == nil)
		langsearchpathlookup (bsverb, &htable);

	pophashtable();
	
	if (!fl)
		goto exit;
	
	if (!hashtablelookupnode (htable, bsverb, &handlernode)) {
		
		langparamerror (unknownfunctionerror, bsverb);
		
		goto exit;
		}
	
	vhandler = (**handlernode).val;
	
	/*build a code tree and call the handler, with our error hook in place*/
	
	hcode = nil;
	
	if (vhandler.valuetype == codevaluetype) {

		hcode = vhandler.data.codevalue;
	}
	else if ((**htable).valueroutine == nil) { /*not a kernel table*/
		
		if (!langexternalvaltocode (vhandler, &hcode)) {

			langparamerror (notfunctionerror, bsverb);

			goto exit;
			}
		
		if (hcode == nil) { /*needs compilation*/
			
			if (!langcompilescript (handlernode, &hcode))
				goto exit;
			}
		}
	
	if (!setaddressvalue (htable, bsverb, &val))
		goto exit;
	
	if (!pushfunctionreference (val, &hfunctioncall))
		goto exit;
	
	if (hcontext != nil) {
		
		flchained = (**hcontext).flchained;
		
		if (flchained)
			pushhashtable (hcontext);
		else
			chainhashtable (hcontext); /*establishes outer local context*/
		}

	fl = langbuildparamlist (&vparams, &hparamlist);
	
	if (hcontext != nil) {
		
		if (flchained)
			pophashtable ();
		else
			unchainhashtable ();
		}
	
	if (!fl) {
		
		langdisposetree (hfunctioncall);
		
		goto exit;
		}
	
	if (!pushfunctioncall (hfunctioncall, hparamlist, &hcode)) /*consumes input parameters*/
		goto exit;

	if (!pushbinaryoperation (moduleop, hcode, nil, &hcode)) /*needs this level???*/
		goto exit;

	/*launch separate process, see processruntext*/
	
	newlyaddedprocess = nil; //process manager global

	if (!addnewprocess (hcode, true, nil, (long) 0)) {
		
		langdisposetree (hcode);
		
		goto exit;
		}
	
	/*return thread id*/

	hp = newlyaddedprocess; //process.c global; will be nil if a process wasn't just added
			
	if ((hp == nil) || !scheduleprocess (hp, &hthread))
		return (setlongvalue (0, v));
			
	(**hp).processstartedroutine = &threadverbprocessstarted; //don't dim the menu bar

	copystring (bsscriptname, (**hp).bsname);	/* 9.1b3 AR */

	if (hcontext != nil) {

		Handle hpacked;
		boolean fldummy;

		/*make a copy of the context table*/

		if (!tablepacktable (hcontext, true, &hpacked, &fldummy))
			goto exit;

		if (!tableunpacktable (hpacked, true, &hcontext))
			goto exit;		

		/*set the child thread's context to the copy of the context table*/

		(**hp).hcontext = hcontext;

		/*make sure the copy of the context table will be disposed*/

		(**hp).processkilledroutine = &threaddisposecontext;
		}

	return (setlongvalue (getthreadid (hthread), v));

exit:

	return (false);
	}/*threadcallscriptverb*/


static boolean threadstatsverb (hdltreenode hparam1, tyvaluerecord *v) {
	
	/*
	6.2b6 AR: New verb for debugging thread-related problems
	*/

	hdlhashtable htable, hstatstable;
	bigstring bs;

	flnextparamislast = true;

	if (!getvarparam (hparam1, 1, &htable, bs))
		return (false);

	if (!langsuretablevalue (htable, bs, &hstatstable))
		return (false);
		
	if (!processgetstats (hstatstable))
		return (false);
		
	return (setbooleanvalue (true, v));
	}/*threadstatsverb*/


static boolean threadfunctionvalue (short token, hdltreenode hparam1, tyvaluerecord *vreturned, bigstring bserror) {
	
	/*
	4.1b3 dmb: new verbs
	
	4.1b5 dmb: added thread.sleep
	
	4.1b6 dmb: make thread.sleepFor take seconds, not ticks
	
	5.0d13 dmb: added v == nil check
	*/
	
	register tyvaluerecord *v = vreturned;
	typrocessid processid;
	unsigned long ticks;
	
	if (v == nil) { /*need Frontier process?*/
		
		switch (token) {
			
			case evaluatefunc:
			case callscriptfunc:
			case sleepfunc:
			case sleepforfunc:
			case sleepticksfunc:
			case issleepingfunc:
			case wakefunc:
			case killfunc:
			/*
			case begincriticalfunc:
			case endcriticalfunc:
			*/
			case statsfunc:
				return (true);
			
			case existsfunc:
			case getcurrentfunc:
			case getcountfunc:
			case getnththreadfunc:
			case gettimeslicefunc:
			case getdefaulttimeslicefunc:
			case settimeslicefunc:
			case setdefaulttimeslicefunc:
			default:
				return (false);
			}
		}
	
	setbooleanvalue (false, v); // assume the worst
	
	processid = getcurrentprocessid ();
	
	if (!iscurrentapplication (processid)) {
		
		getstringlist (langerrorlist, cantbackgroundclipboard, bserror);	// ***
		
		return (false);
		}
	
	switch (token) {
		
		case existsfunc: {
			long id;
			
			flnextparamislast = true;
			
			if (!getlongvalue (hparam1, 1, &id))
				return (false);
			
			return (setbooleanvalue (getprocessthread (id) != nil, v));
			}
		
		case evaluatefunc: {
			Handle htext;
			hdlprocessrecord hp;
			hdlprocessthread hthread;
			
			flnextparamislast = true;
			
			if (!getexempttextvalue (hparam1, 1, &htext))
				return (false);
			
			newlyaddedprocess = nil; //process manager global
			
			if (!processruntext (htext))
				return (false);
			
			hp = newlyaddedprocess; //process.c global; will be nil if a process wasn't just added
			
			if ((hp == nil) || !scheduleprocess (hp, &hthread))
				return (setlongvalue (0, v));
			
			(**hp).processstartedroutine = &threadverbprocessstarted;

			return (setlongvalue (getthreadid (hthread), v));
			}
		
		case callscriptfunc: {

			bigstring bsscriptname;
			tyvaluerecord vparams;
			hdlhashtable hcontext = nil;
			boolean fl;
	
			if (!getstringvalue (hparam1, 1, bsscriptname))
				return (false);
	
			if (!getparamvalue (hparam1, 2, &vparams))
				return (false);
	
			if (vparams.valuetype != recordvaluetype)
				if (!coercetolist (&vparams, listvaluetype))
					return (false);
	
			if (langgetparamcount (hparam1) > 2) {
		
				flnextparamislast = true;

				if (!gettablevalue (hparam1, 3, &hcontext))
					return (false);
				}
				
			(**(getcurrentthreadglobals ())).debugthreadingcookie = token;

			fl = threadcallscriptverb (bsscriptname, vparams, hcontext, v);
			
			(**(getcurrentthreadglobals ())).debugthreadingcookie = 0;

			return (fl);
			}

		case getcurrentfunc:
			if (!langcheckparamcount (hparam1, 0))
				return (false);
			
			return (setlongvalue (getthreadid (getcurrentthread ()), v));
		
		case getcountfunc:
			if (!langcheckparamcount (hparam1, 0))
				return (false);
			
			return (setlongvalue (processthreadcount (), v));
		
		case getnththreadfunc: {
			short n;
			
			flnextparamislast = true;
			
			if (!getintvalue (hparam1, 1, &n))
				return (false);
			
			return (setlongvalue (getthreadid (nthprocessthread (n)), v));
			}
		
		case sleepfunc: {
			hdlprocessthread hthread;
			
			flnextparamislast = true;
			
			if (!getthreadvalue (hparam1, 1, &hthread))
				return (false);
			
			return (setbooleanvalue (processsleep (hthread, -1), v));
			}
		
		case sleepforfunc: {
			long n;
			boolean fl;
			
			flnextparamislast = true;
			
			if (!getlongvalue (hparam1, 1, &n))
				return (false);
			
			(**(getcurrentthreadglobals ())).debugthreadingcookie = token;

			fl = processsleep (getcurrentthread (), n * 60);
			
			(**(getcurrentthreadglobals ())).debugthreadingcookie = 0;

			return (setbooleanvalue (fl, v));
			}
		
		case sleepticksfunc: {
			long n;
			boolean fl;
			
			flnextparamislast = true;
			
			if (!getlongvalue (hparam1, 1, &n))
				return (false);
			
			(**(getcurrentthreadglobals ())).debugthreadingcookie = token;

			fl = processsleep (getcurrentthread (), n);
			
			(**(getcurrentthreadglobals ())).debugthreadingcookie = 0;

			return (setbooleanvalue (fl, v));
			}

		case issleepingfunc: {
			hdlprocessthread hthread;
			
			flnextparamislast = true;
			
			if (!getthreadvalue (hparam1, 1, &hthread))
				return (false);
			
			return (setbooleanvalue (processissleeping (hthread), v));
			}
		
		case wakefunc: {
			hdlprocessthread hthread;
			
			flnextparamislast = true;
			
			if (!getthreadvalue (hparam1, 1, &hthread))
				return (false);
			
			return (setbooleanvalue (wakeprocessthread (hthread), v));
			}
		
		case killfunc: {
			hdlprocessthread hthread;
			
			flnextparamislast = true;
			
			if (!getthreadvalue (hparam1, 1, &hthread))
				return (false);
			
			return (setbooleanvalue (killprocessthread (hthread), v));
			}
		
		case gettimeslicefunc:
			if (!langcheckparamcount (hparam1, 0))
				return (false);
			
			getprocesstimeslice (&ticks);

			return (setlongvalue (ticks, v));
		
		case settimeslicefunc:
			flnextparamislast = true;

			if (!getlongvalue (hparam1, 1, &ticks))
				return (false);
			
			return (setbooleanvalue (setprocesstimeslice (ticks), v));
	
		case getdefaulttimeslicefunc:
			if (!langcheckparamcount (hparam1, 0))
				return (false);
			
			getdefaulttimeslice (&ticks);

			return (setlongvalue (ticks, v));
		
		case setdefaulttimeslicefunc:
			flnextparamislast = true;

			if (!getlongvalue (hparam1, 1, &ticks))
				return (false);
			
			return (setbooleanvalue (setdefaulttimeslice (ticks), v));
		/*
		case begincriticalfunc:
			if (!langcheckparamcount (hparam1, 0))
				return (false);
			
			++fldisableyield;
			
			return (setbooleanvalue (true, v));
		
		case endcriticalfunc:
			if (!langcheckparamcount (hparam1, 0))
				return (false);
			
			if (fldisableyield > 0) {
			
				--fldisableyield;
				
				(*v).data.flvalue = true;
				}
			
			return (true);
		*/
		case statsfunc:
			return (threadstatsverb (hparam1, v));

		default:
			return (false);
		}
	} /*threadfunctionvalue*/


boolean sysinitverbs (void) {
	
	getapplicationfilespec (nil, &programfspec);
	
	launchcallbacks.waitcallback = &shellsysverbwaitroutine;
	
	if (!loadfunctionprocessor (idsysverbs, &sysfunctionvalue))
		return (false);
	
	if (!loadfunctionprocessor (idlaunchverbs, &launchfunctionvalue))
		return (false);
	
	if (!loadfunctionprocessor (idclipboardverbs, &clipboardfunctionvalue))
		return (false);
	
	if (!loadfunctionprocessor (idfrontierverbs, &frontierfunctionvalue))
		return (false);
	
	if (!loadfunctionprocessor (idthreadverbs, &threadfunctionvalue))
		return (false);
	
	return (true);
	} /*sysinitverbs*/




