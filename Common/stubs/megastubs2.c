
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
	#include <IAC.h>
	#include <uisharing.h>
	#include "filealias.h"
#endif

#include "bitmaps.h"
#include "dialogs.h"
#include "memory.h"
#include "error.h"
#include "file.h"
#include "launch.h"
#include "fileloop.h"
#include "strings.h"
#include "frontierwindows.h"
#include "zoom.h"
#include "lang.h"
#include "langinternal.h"
#include "langipc.h"
#include "langexternal.h"
#include "langsystem7.h"
#include "scripts.h"
#include "shell.h"
#include "shellbuttons.h"
#include "shellhooks.h"
#include "shellprint.h"

#ifdef MACVERSION
#include "osacomponent.h"
#endif

#include "process.h"



// tyshellglobals shellglobals;


// bitmaps.c

extern HWND currentport;
extern HDC currentportDC;

static HDC offscreenDC = NULL;
static HBITMAP oldbitmap;
static Rect offscreenrect;
static HWND saveDC;


void initbitmaps (boolean fl) {
	} /*initbitmaps*/


boolean openbitmap (Rect r, WindowPtr w) {
	
	/*
	5.0fc2 rab: plugged memory leak calling GetDC
	*/

	HDC hdc = NULL; // = GetDC (w);
	HBITMAP bitmap;
	
	assert (w == currentport);

	offscreenDC = NULL; //*** disable: CreateCompatibleDC (hdc);
	
	if (offscreenDC == NULL)
		return (false);

	bitmap = CreateCompatibleBitmap (hdc, r.right - r.left, r.bottom - r.top);
	
	if (bitmap == NULL) {
		
		DeleteDC (offscreenDC);
		
		offscreenDC = NULL;

		return (false);
		}
	
	oldbitmap = SelectObject (offscreenDC, bitmap);
	
	SetViewportOrgEx (offscreenDC, r.left, r.top, NULL);
	
	offscreenrect = r;
	
	saveDC = (HWND) currentportDC;

	currentportDC = offscreenDC;

	return (true);
	} /*openbitmap*/


//boolean openbitmapcopy (Rect r, WindowPtr w) {return false;}

void closebitmap (WindowPtr w) {
	
	HBITMAP bitmap;
	Rect *r = &offscreenrect;

	if (offscreenDC != NULL) {

		currentport = w;
		
		currentportDC = (HDC) saveDC;
		
		BitBlt ((HDC) saveDC, r->left, r->top, r->right - r->left, r->bottom - r->top, offscreenDC, 0,0, SRCAND);
		
		bitmap = SelectObject (offscreenDC, oldbitmap);
		
		DeleteObject (bitmap);

		DeleteDC (offscreenDC);

		offscreenDC = NULL;
		}
	} /*closebitmap*/


// launch.c

boolean activateapplicationwindow (typrocessid id, WindowPtr w) {return (false);}


// zoom.c

void zoominit (void) {}

void zoomtoorigin (WindowPtr w) {hidewindow (w);}

void zoomfromorigin (WindowPtr w) {
	
	/*
	5.0.1 dmb: call SetFocus. Fixes the "Deaf QuickScript" bug.
	*/

	releasethreadglobals ();

	SetWindowPos (w, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	ShowWindow (w, SW_SHOWNORMAL);
	SetFocus (w);

	grabthreadglobals ();
	
	/*
	config = origconfig;
	shellglobals = origglobals;
	setport (saveport);
	*/

	shellactivatewindow (w, true);
	} /*zoomfromorigin*/

void zoomwindowfromcenter (Rect r, WindowPtr w) {
	
	zoomfromorigin (w);
	} /*zoomwindowfromcenter*/

void zoomwindowtocenter (Rect r, WindowPtr w) {
	
	hidewindow (w);
	} /*zoomwindowtocenter*/

void zoomsetdefaultrect (WindowPtr w, Rect r) {}


#ifndef isFrontier

// script.c

boolean scriptrunsuspendscripts (void) {return true;}

boolean scriptrunresumescripts (void) {return true;}


// process.c

unsigned short fldisableyield = 0;

hdlprocessrecord currentprocess = nil;

boolean initprocess (void) {return true;}

unsigned long processstackspace (void) {
	
#if MACVERSION
	return (StackSpace ());
#endif

#if WIN95VERSION
	return (0x7fff); // 3/10/97 dmb: no equivalent measure (stackavail) under Windows
#endif
	} /*processstackspace*/

#if winhybrid

boolean debuggingcurrentprocess (void) {return false;}

boolean processruntext (Handle htext) {bigstring bs; return langrunhandle (htext, bs);}

boolean processisoneshot (boolean fl) {return true;}

#endif

boolean processagentsleep (long ctseconds) {return true;}

boolean pushprocess (hdlprocessrecord hp) {return true;}

boolean popprocess (void) {return true;}

boolean processbusy (void) {return true;}

boolean processyield (void) {return true;}

boolean processdisposecode (hdltreenode hcode) {return false;}

void processchecktimeouts (void) {}

void processclose (void) {}

void processinvalidglobals (WindowPtr w) {}

boolean processrunning (void) {return true;}


// land.c

//pascal boolean landeventfilter (EventRecord *ev) {return false;}


// lang.c

boolean flscriptrunning = false;

boolean langzoomobject (const bigstring bs) {return (false);}

boolean initlang (void) {return true;}



// langcallbacks.c

boolean langbackgroundtask (boolean fl) {return true;}

boolean langpartialeventloop (short mask) {

	return shellpartialeventloop (mask);
	} /*langpartialeventloop*/

// langexternal.c

boolean langexternalzoomfrom (tyvaluerecord val, hdlhashtable ht, bigstring bs, Rect r) {return false;}

boolean langexternalwindowopen (tyvaluerecord val, hdlwindowinfo *hw) {return false;}

boolean tablezoomtoname (hdlhashtable ht, bigstring bs) {return false;}

boolean initscripts (void) {return true;}


#endif // isFrontier

// osacomponent.c

void osacomponentshutdown (void) {};


// langdialog.c

boolean langdialogrunning (void) {return false;}


// langmodeless.c

boolean langrunmodeless (hdltreenode hparam1, tyvaluerecord *vreturned) {return false;}


// langipc.c

typrocessid langipcself;
static boolean fltoolkitinitialized = false;

#ifdef MACVERSION

static pascal Boolean openfilespec (ptrfilespec pfs) {
	
	if (!shellopenfile (pfs)) {
		
		IACreturnerror (getoserror (), nil);
		
		return (false);
		}
		
	return (true);
	} /*openfilespec*/
	

static pascal OSErr handleopen (AppleEvent *event, AppleEvent *reply, long refcon) {
	
	IACglobals.event = event;
	
	IACglobals.reply = reply;
	
	IACglobals.refcon = refcon;
	
	return (IACdrivefilelist (&openfilespec));
	} /*handleopen*/

		
static pascal OSErr handlequit (AppleEvent *event, AppleEvent *reply, long refcon) {
	
	if (!shellcloseall (nil, true)) /*user hit Cancel button in save dialog*/
		return (userCanceledErr);
	
	shellexitmaineventloop (); /*sets flag for next iteration*/
	
	return (noErr);
	} /*handlequit*/
	
	
static pascal OSErr handleopenapp (const AppleEvent *event, AppleEvent *reply, long refcon) {
	
	if (!shellopendefaultfile ())
		return (getoserror ());
	
	return (noErr);
	} /*handleopenapp*/


boolean landeventfilter (EventRecord *ev) {
	
	/*
	watch for high level events (AppleEvents)
	
	return true if we consume the event, false otherwise.
	*/
	
	if ((*ev).what == kHighLevelEvent) {
	
		AEProcessAppleEvent (ev);
		
		return (true); /*consume the event*/		
		}
	
	return (false); /*don't consume the event*/
	} /*landeventfilter*/

#endif

boolean langipcinit (void) {
	
	return (true);
	} /*langipcinit*/


boolean langipcstart (void) {
	
	/*
	register with the IAC toolkit, using the identifier defined in land.h, 
	and registering the verbs that scriptrunners must implement.
	*/
	
#ifdef MACVERSION
	
	GetCurrentProcess (&langipcself);
	
	if (!IACinstallhandler (kCoreEventClass, kAEOpenApplication, (ProcPtr) &handleopenapp))
		goto error;
	
	if (!IACinstallhandler (kCoreEventClass, kAEOpenDocuments, (ProcPtr) &handleopen))
		goto error;
	
	if (!IACinstallhandler (kCoreEventClass, kAEQuitApplication, (ProcPtr) &handlequit))
		goto error;

	error:
	
#endif

	return (true);
	} /*langipcstart*/

void langipcshutdown () {}

// misc

//void initsegment (void) {}


#ifdef MACVERSION

// uiSharing

Boolean isModelessCardEvent (EventRecord *ev);
Boolean isModelessCardEvent (EventRecord *ev) {return false;}

Boolean uisIsSharedWindow (WindowPtr w) {return false;}

Boolean uisCloseSharedWindow (WindowPtr w) {return false;}

void uisCloseAllSharedWindows (void) {}

Boolean uisEdit (short item) {return false;}
	

// launch.c

typrocessid getcurrentprocessid (void) {
	
	ProcessSerialNumber psn;
	
	GetCurrentProcess (&psn);
	
	return (psn);
	} /*getcurrentprocessid*/


boolean getapplicationfilespec (bigstring bsprogram, tyfilespec *fs) {
	
	ProcessInfoRec processinfo;
	ProcessSerialNumber psn;
	
	assert (bsprogram == nil);
	
	processinfo.processInfoLength = sizeof (processinfo);
	processinfo.processName = nil; /*place to store process name*/
	processinfo.processAppSpec = fs; /*place to store process filespec*/
	
	psn.highLongOfPSN = 0;
	psn.lowLongOfPSN = kCurrentProcess;
	
	if (GetProcessInformation (&psn, &processinfo) != noErr)
		return (false);
	
	return (true);
	} /*getapplicationfilespec*/


// about.c

void aboutsegment (void) {}

boolean openabout (boolean fl, long n) {return false;}


#endif

#ifdef WIN95VERSION

short dialogcountitems (DialogPtr pdialog) {return 0;}

void disabledialogitem (DialogPtr pdialog, short itemnumber) {}
	
void enabledialogitem (DialogPtr pdialog, short itemnumber) {}

void hidedialogitem (DialogPtr pdialog, short itemnumber) {}

void showdialogitem (DialogPtr pdialog, short itemnumber) {}

boolean customdialog (short id, short defaultitem, dialogcallback itemhitcallback) {return false;}

long FreeMem (void) {
	MEMORYSTATUS ms;

	ms.dwLength = sizeof (MEMORYSTATUS);
	GlobalMemoryStatus (&ms);

	return (ms.dwAvailPageFile);
	}

void DrawPicture (Handle hpicture, const Rect *frame) {}

void UpdateDialog (WindowPtr w) {}


#endif
