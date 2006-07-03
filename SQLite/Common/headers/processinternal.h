
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

#define processinternalinclude

#ifdef MACVERSION
	#ifndef __THREADS__
		//#include <Threads.h>
	#endif

	#ifndef landinclude
		#include <land.h>
	#endif
#endif

#ifndef threadsinclude
	#include "threads.h"
#endif

#ifndef cancooninclude
	#include "cancoon.h"
#endif

#ifndef opinternalinclude
	#include "opinternal.h"
#endif

#if !flruntime
	#ifndef shellprivateinclude
		#include "shellprivate.h"
	#endif
	
	#ifndef shellhooksinclude
		#include "shellhooks.h"
	#endif
#endif


#define ctprocesses 5 /*we can remember nested processes up to 5 levels deep*/

typedef struct typrocessstackrecord {
	
	hdlprocessrecord hprocess;
	
	langerrormessagecallback errormessagecallback;
	
	langerrormessagecallback debugerrormessagecallback;
	
	langvoidcallback clearerrorcallback;
	
	hdlerrorstack herrorstack;
	
	hdltablestack htablestack;
	} typrocessstackrecord;


typedef struct typrocessstack {
	
	short top;
	
	typrocessstackrecord stack [ctprocesses];
	} typrocessstack;


typedef struct tythreadglobals {
	
	struct tythreadglobals **hnextglobals;
	
	hdlthread idthread;
	
	#if !flruntime
	
	hdlcancoonrecord hccglobals;
	
	#endif
	
	typrocessstack processstack;
	
	hdlprocessrecord hprocess;
	
	hdlhashtable htable;
	
	hdltablestack htablestack;
	
	tylangcallbacks langcallbacks;
	
	#if !flruntime
	
	short cterrorhooks;
	
	callback errorhooks [maxerrorhooks];
	
	tyglobalsstack globalsstack;
	
	short topoutlinestack;
	
	hdloutlinerecord outlinestack [ctoutlinestack];
	
	hdloutlinerecord outlinedata;

	WindowPtr shellwindow;
	
	DialogPtr pmodaldialog;
	
	#endif
	
	unsigned short ctscanlines;
	
	unsigned short ctscanchars;
	
	hdltreenode herrornode;
	
	unsigned long timestarted;

	unsigned long sleepticks;

	unsigned long timebeginsleep;
	
	unsigned long timetowake;
	
	unsigned long timeswappedin;

	unsigned long timesliceticks;

	#ifdef flcomponent
	
	pascal OSErr (*eventcreatecallback) (AEEventClass, AEEventID, const AEAddressDesc *, short, long, AppleEvent *);
	
	pascal OSErr (*eventsendcallback) (const AppleEvent *, AppleEvent *, AESendMode, AESendPriority, long, AEIdleUPP, AEFilterUPP);
	
	OSType applicationid;
	
	tymaceventsettings eventsettings; /*not just for OSA; new features*/
	
	#endif
	
	boolean flreturn;
	
	boolean flbreak;
	
	boolean flcontinue;
	
	boolean flscriptresting;
	
	boolean flretryagent;
	
	boolean flthreadkilled;
	
	boolean fllangerror;
	
	boolean flscriptrunning;
	
	unsigned short langerrordisable; /*6.1.1b2 AR*/
	
	Handle tryerror;

	Handle tryerrorstack;
	
	unsigned short fldisableyield;
	
	long debugthreadingcookie; /*6.2b11 AR: for debugging hung threads*/
	
	#if TARGET_API_MAC_CARBON == 1
	
	ThreadSwitchUPP threadInCallbackUPP;
	
	ThreadSwitchUPP threadOutCallbackUPP;
	
	ThreadTerminationUPP threadTerminateUPP;
	
	ThreadEntryUPP threadEntryCallbackUPP;
	
	#endif
	} tythreadglobals, *ptrthreadglobals, **hdlthreadglobals;


/*globals*/

extern boolean flthreadkilled;


/*prototypes*/

extern void disposethreadglobals (hdlthreadglobals);

extern boolean newthreadglobals (hdlthreadglobals *);

extern hdlthreadglobals getcurrentthreadglobals (void);

extern void copythreadglobals (hdlthreadglobals);

extern void swapinthreadglobals (hdlthreadglobals);

