
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

#include "shelltypes.h"
#include <appletdefs.h>
#include <iac.h>
#include <SetupA5.h>
#include <appletquickdraw.h>
#include <appletmouse.h>
#include <appletfrontier.h>
#include <appletcursor.h>
#include <appletops.h>
#include <appletfiles.h>
#include <appletcardops.h>
#include <appletzoom.h>
#include <appletcard.h>
#include <appletkb.h>

#include "iowaparser.h"
#include "iowafrontier.h"
#include "iowaattributes.h"
#include "iowascript.h"

#include "quickdraw.h"


#ifndef cutCmd
#define	cutCmd		3
#define	copyCmd		4
#define	pasteCmd	5
#define clearCmd	6
#endif


#ifndef iowaRuntime /*in Card Editor, cmd-period kills a running card*/ 

	#define cmdPeriodKillsCard
	
#endif

hdlruntimerecord runtimedata = nil;

boolean flexitcardloop = false;

typedef struct cardwindow {

	WindowPtr w;
	
	hdlruntimerecord hw;
	
	} tycardwindow;

static tycardwindow **hwindowarray = nil; /*dynamic array of WindowPtrs managed by iowa*/
	
#define textticksrecalc 30 /*while typing text, wait this number of ticks before recalcing*/

boolean flquickrecalc = true; /*set false in cardrunner.c, under special circumstances*/


static short sizewindowarray (void) {
	
	if (hwindowarray == nil)
		return (0);
		
	return ((short) (GetHandleSize ((Handle) hwindowarray) / longsizeof (tycardwindow)));
	} /*sizewindowarray*/
	
	
static hdlruntimerecord getwindowdata (WindowPtr w) {
	
	/*return ((hdlruntimerecord) GetWRefCon (w));*/
	
	if (w != nil)
	{
		short ct, i;
		
		ct = sizewindowarray ();
		
		for (i = 0; i < ct; i++) { /*look for the window in the window array*/
			
			if ((*hwindowarray) [i].w == w)
				return ((*hwindowarray) [i].hw);
			}
	}
	
	return (nil); /*it's not in the window array*/
	} /*getwindowdata*/
	
	
static hdlcard getwindowcardrecord (WindowPtr w) {
		
	return ((**getwindowdata (w)).hc);
	} /*getwindowcardrecord*/
	

static hdlcard cardtofind;

static WindowPtr windowtofind = nil;


static boolean getcardrecordwindowvisit (WindowPtr w) {
	
	hdlcard hc = getwindowcardrecord (w);

	if (hc == cardtofind)	
		windowtofind = w;
		
	return (true);
	} /*getcardrecordwindowvisit*/
	
	
WindowPtr getcardrecordwindow (hdlcard card) {
	
	/*
	4.1b4 dmb: I added this routine a couple of builds back for modalcardeventfilter
	in langcard.c. I now see that we must preserve our globals, which visitCardWindows
	automatically sets.
	*/
	
	hdlruntimerecord saveruntimedata = runtimedata;
	hdlcard saveiowadata = iowadata;
	
	cardtofind = card;
	
	visitCardWindows (&getcardrecordwindowvisit);
	
	runtimedata = saveruntimedata;
	
	iowadata = saveiowadata;
	
	return (windowtofind);
	} /*getcardrecordwindow*/

	
short countCardWindows (void) {
	
	short ctcells = sizewindowarray ();
	short ct = 0;
	short i;
	
	for (i = 0; i < ctcells; i++)
		if ((*hwindowarray) [i].w != nil)
			ct++;
			
	return (ct);
	} /*countCardWindows*/


//#pragma global_optimizer off	//fixes CWPro2 compiler bug, hw would always be zero

static boolean addtowindowarray (WindowPtr w, hdlruntimerecord hw) {
	
	short ct, i;
	tycardwindow cw;
	
	assert (hw != nil);
	
	cw.w = w;
	
	cw.hw = hw;
	
	ct = sizewindowarray ();
	
	for (i = 0; i < ct; i++) { /*look for an empty slot in the window array*/
		
		if ((*hwindowarray) [i].w == nil) {
			
			(*hwindowarray) [i] = cw;
			
			return (true);
			}
		} /*for*/
	
	/*no empty slots, allocate a new array or make the array bigger*/
	
	if (hwindowarray == nil)
		return (newfilledhandle (&cw, longsizeof (tycardwindow), (Handle *) &hwindowarray));
	
	SetHandleSize ((Handle) hwindowarray, (ct + 1) * longsizeof (tycardwindow));

	if (MemError () != noErr)
		return (false);
	
	(*hwindowarray) [ct] = cw;
	
	return (true);
	} /*addtowindowarray*/

//#pragma global_optimizer reset

	
static boolean inwindowarray (WindowPtr w) {

	return (getwindowdata (w) != nil);
	} /*inwindowarray*/
	
	
static void removefromwindowarray (WindowPtr w) {

	short ct, i;
	
	ct = sizewindowarray ();
	
	for (i = 0; i < ct; i++) { /*look for an empty slot in the window array*/
		
		if ((*hwindowarray) [i].w == w) {
		
			(*hwindowarray) [i].w = nil;
			
			return;
			}
		} /*for*/
	} /*removefromwindowarray*/
	
	
boolean isCardWindow (WindowPtr w) {
	
	return (inwindowarray (w));
	} /*isCardWindow*/


boolean cardToFront (bigstring windowtitle) {

	short ct, i;
	
	ct = sizewindowarray ();
	
	for (i = 0; i < ct; i++) { 
		
		WindowPtr w = (*hwindowarray) [i].w;
		
		if (w != nil) {
			
			bigstring bs;
			
			GetWTitle (w, bs);
			
			if (equalstrings (bs, windowtitle)) {
				
				SelectWindow (w);
				
				return (true);
				}
			}
		} /*for*/
	
	return (false); /*we don't have a window with that name*/
	} /*cardToFront*/


static void runmodeactivate (boolean flactivating) {
	
	/*
	8/9/96 4.1b4 dmb: broke out of handleCardEvent's main switch statement
	
	1.0b19 dmb: call frontSetRuntimeCard when we switch out too.
	*/
	
	frontSetRuntimeCard (flactivating, true);
	
	/*DW 4/6/95: hack -- activate/deactivate the active text object directly*/ {
		
		hdlobject hobj = (**iowadata).activetextobject;
		
		if (hobj != nil) {
		
			hdleditrecord heditrecord = (hdleditrecord) (**hobj).objecteditrecord;
		
			editactivate (heditrecord, flactivating);
			}
		}
	} /*runmodeactivate*/


static void runmodeupdate (void) {
	
	hdlruntimerecord hw = runtimedata;
	WindowPtr w;
	hdlcard hc;
	boolean beginendupdate;
	
	if (hw == nil) // 7.3.97 dmb: can happen when multiple cards are started quickly
		return;
	
	w = (**hw).macwindow;
	hc = (**hw).hc;
	beginendupdate = (**hw).ownwindow;	/* || (**hc).macevent.what != updateEvt;*/
	
	assert ((**hc).drawwindow == w);
	//Code change by Timothy Paustian Monday, August 21, 2000 9:34:50 PM
	//have to use the port can't cast a window in OS X
	{
	CGrafPtr	thePort;
	#if TARGET_API_MAC_CARBON == 1
	thePort = GetWindowPort(w);
	#else
	thePort = (CGrafPtr)w;	
	#endif
	pushmacport (thePort);
	}
	if ((**hc).fullupdate) {
		//Code change by Timothy Paustian Friday, May 5, 2000 10:49:31 PM
		//Changed to Opaque call for Carbon
		//This may actually be useless in Carbon
		#if ACCESSOR_CALLS_ARE_FUNCTIONS == 1
		CGrafPtr	thePort;
		Rect		windBounds;
		thePort = GetWindowPort(w);
		GetPortBounds(thePort, &windBounds);
		InvalWindowRect (w, &windBounds);
		#else
		InvalRect (&(*w).portRect);
		#endif
		
		(**hc).fullupdate = false;
		}
	
	if (beginendupdate) 
		BeginUpdate (w);
	//Code change by Timothy Paustian Friday, May 5, 2000 10:51:00 PM
	//Changed to Opaque call for Carbon
	#if ACCESSOR_CALLS_ARE_FUNCTIONS == 1
	{
		CGrafPtr	thePort;
		RgnHandle	visRgn = NewRgn();
		thePort = GetWindowPort(w);
		visRgn = GetPortVisibleRegion(thePort, visRgn);
		GetRegionBounds(visRgn, &(**hc).updaterect);
		DisposeRgn(visRgn);
	}
	#else
	(**hc).updaterect = (**(*w).visRgn).rgnBBox;
	#endif
	/*preserve iowadata across an update*/ {
	
		hdlcard oldiowadata = iowadata;
	
		iowadata = hc;
	
		iowaupdate ();
		//Code change by Timothy Paustian Friday, September 15, 2000 9:01:04 PM
		//Mac OS X uses double buffered windows. If you are not calling WaitNextEvent
		//you have to tell the OS to flush the contents of the offscreen buffer
		//to the screen. Use QDFlushPortBuffer to do this.
		#if TARGET_API_MAC_CARBON == 1
		{
		CGrafPtr	thePort;
		thePort = GetWindowPort(w);
		QDFlushPortBuffer(thePort, nil);
		}
		#endif
		iowadata = oldiowadata;
		}
			
	if (beginendupdate)
		EndUpdate (w);
	
	popmacport ();
	} /*runmodeupdate*/
	
	
static boolean runtimeeditableobject (hdlobject h) {
	
	derefclone (&h);
	
	if (!(**h).objectenabled)
		return (false);
		
	return (calleditableinrunmode (h));
	} /*runtimeeditableobject*/
	
	
static boolean getfirsttextobject (hdlobject *hfirst) {
	
	/*
	return the first editable text object
	*/
	
	hdlcard hc = iowadata;
	hdlobject nomad = (**hc).objectlist;
	short lowesttag = infinity;
	
	*hfirst = nil;
	
	while (nomad != nil) {
		
		if (runtimeeditableobject (nomad)) {
			
			if ((**nomad).sorttag < lowesttag) {
				
				lowesttag = (**nomad).sorttag;
				
				*hfirst = nomad;
				}
			}

		nomad = (**nomad).nextinthread;
		} /*while*/
	
	return (*hfirst != nil);
	} /*getfirsttextobject*/
	

static void getnexttextobject (hdlobject h, hdlobject *hnext) {
	
	hdlcard hc = iowadata;
	hdlobject nomad = (**hc).objectlist;
	short n = (**h).sorttag + 1;
	short nearesttag = infinity;
	hdlobject nearestnomad = nil;
	short lowesttag = infinity;
	hdlobject lowestnomad = nil;

	while (nomad != nil) {
		
		if (runtimeeditableobject (nomad)) {
			
			short nomadtag = (**nomad).sorttag;
			
			if (nomadtag == n) { /*found it the easy way*/
				
				*hnext = nomad;
				
				return;
				}
			
			if (nomadtag < lowesttag) { /*in case we wrap around*/
				
				lowesttag = nomadtag;
				
				lowestnomad = nomad;
				}
				
			if ((nomadtag > n) && (nomadtag < nearesttag)) {
				
				nearestnomad = nomad;
				
				nearesttag = nomadtag;
				}
			}

		nomad = (**nomad).nextinthread;
		} /*while*/
	
	if (nearestnomad == nil) /*wrap around to the first guy*/
		*hnext = lowestnomad;
	else
		*hnext = nearestnomad;
	} /*getnexttextobject*/
	

static void getprevtextobject (hdlobject h, hdlobject *hnext) {
	
	hdlcard hc = iowadata;
	hdlobject nomad = (**hc).objectlist;
	short n = (**h).sorttag - 1;
	short nearesttag = 0;
	hdlobject nearestnomad = nil;
	short highesttag = 0;
	hdlobject highestnomad = nil;

	while (nomad != nil) {
		
		if (runtimeeditableobject (nomad)) {
			
			short nomadtag = (**nomad).sorttag;
			
			if (nomadtag == n) { /*found it the easy way*/
				
				*hnext = nomad;
				
				return;
				}
			
			if (nomadtag > highesttag) { /*in case we wrap around*/
				
				highesttag = nomadtag;
				
				highestnomad = nomad;
				}
				
			if ((nomadtag < n) && (nomadtag > nearesttag)) {
				
				nearestnomad = nomad;
				
				nearesttag = nomadtag;
				}
			}

		nomad = (**nomad).nextinthread;
		} /*while*/
	
	if (nearestnomad == nil) /*wrap around to the first guy*/
		*hnext = highestnomad;
	else
		*hnext = nearestnomad;
	} /*getprevtextobject*/
	
	
static boolean movetextboxcursor (boolean flbackward) {

	hdlcard hc = iowadata;
	hdlobject h;
	
	h = (**hc).activetextobject;
	
	if (h == nil) /*no text objects*/
		return (false);
	
	if (flbackward)
		getprevtextobject (h, &h);
	else
		getnexttextobject (h, &h);
		
	if (h == (**hc).activetextobject) /*only one text object*/
		return (false);
	
	derefclone (&h);
	
	calleditobject (h, true);
			
	validobject (h); /*1.0b15 DW -- part of the flicker avoidance scheme*/
	
	return (true);
	} /*movetextboxcursor*/
	
	
static void hotobject (boolean flpressed) {
	
	/*
	callback routine for the mouse tracker.
	*/
	
	hdlcard hc = iowadata;
	
	(**hc).trackerpressed = flpressed;
	
	invalobject ((**hc).trackerobject);
	
	iowaupdatenow ();
	} /*hotobject*/
	

boolean recalcCardWindow (WindowPtr w, boolean flmajorrecalc) {
	
	if (w == nil) 
		return (false);
		
	//Code change by Timothy Paustian Monday, August 21, 2000 9:34:50 PM
	//have to use the port can't cast a window in OS X
	{
	CGrafPtr	thePort;
	#if TARGET_API_MAC_CARBON == 1
	thePort = GetWindowPort(w);
	#else
	thePort = (CGrafPtr)w;	
	#endif
	pushmacport (thePort);
	}
		
	runtimedata = getwindowdata (w); /*set up global for runmode routines*/
	
	iowadata = getwindowcardrecord (w); /*set up global for card routines*/
	
	frontSetRuntimeCard (true, false); /*a minor switch, just make embedded objects global*/
	
	(**iowadata).recalcobject = nil;
	
	if (flmajorrecalc)
		visitobjects ((**iowadata).objectlist, &majorrecalcvisit);
	else
		visitobjects ((**iowadata).objectlist, &minorrecalcvisit);
	
	runmodeupdate ();
		
	popmacport ();
	
	return (true); /*keep visiting*/
	} /*recalcCardWindow*/
	
	
static boolean sameasbuttontype (tyobjecttype type) { 
	
	return (type == buttontype);
	} /*sameasbuttontype*/
	
	
static void filterevent (EventRecord *ev) {
	
	tycardeventcallback callback = (**runtimedata).callback;
	
	if (callback == nil) 
		return;
		
	if ((*ev).what == nullEvent) /*DW 8/22/93*/
		(*ev).message = (long) iowadata;
	
	else { /*DW /8/26/93: no klooging to set the global over in the filter, send 2 events*/ 
	
		EventRecord x;
		
		clearbytes (&x, longsizeof (x));
		
		x.what = iowaSetHandleEvent;
		
		x.message = (long) iowadata;
		
		(*callback) (&x);
		}
	
	//Code change by Timothy Paustian Monday, August 21, 2000 9:34:50 PM
	//have to use the port can't cast a window in OS X
	{
	CGrafPtr	thePort;
	#if TARGET_API_MAC_CARBON == 1
	thePort = GetWindowPort((**iowadata).drawwindow);
	#else
	thePort = (CGrafPtr)(**iowadata).drawwindow;	
	#endif
	pushmacport (thePort);
	}
	
	(*callback) (ev);
	
	popmacport ();
	} /*filterevent*/
	
	
static void sendinitmessage (void) {

	tycardeventcallback callback = (**runtimedata).callback;
	EventRecord ev;
	
	if (callback == nil) 
		return;
		
	ev.what = iowaInitEvent;
	
	ev.message = (long) iowadata;
	
	(*callback) (&ev);
	} /*sendinitmessage*/
	
	
static boolean sendbuttonhitmessage (hdlobject h) {
	
	/*
	return true if the buttonhit has been fully processed
	*/
	
	tycardeventcallback callback = (**runtimedata).callback;
	EventRecord ev;
		
	if (callback == nil)
		return (true);
	
	ev.what = iowaButtonHitEvent;
	
	ev.message = (long) h;
	
	(*callback) (&ev);
	
	if (ev.what == iowaRunScriptEvent) /*pass the buttonhit thru, run the script*/
		return (false);
	
	if (ev.what == iowaCloseEvent) /*the client is telling us to close the window*/
		closeCardWindow ((**runtimedata).macwindow);
		
	return (true); /*we consumed the buttonhit*/
	} /*sendbuttonhitmessage*/
	
	
static boolean sendclosemessage (void) {
	
	/*
	the user just clicked in the close box of a card window, or for some
	other reason the card is about to close. we call back to the client,
	giving him a chance to save data from the card.
	*/
	
	tycardeventcallback callback = (**runtimedata).callback;
	EventRecord ev;
	
	if (callback == nil)
		return (true);
	
	ev.what = iowaCloseEvent;
	
	ev.message = (long) iowadata;
	
	(*callback) (&ev);
	
	return (ev.what != iowaCancelEvent);
	} /*sendclosemessage*/
	
	
static void objectclick (hdlobject listhead, hdlobject h) {
	
	/*
	the user just clicked on this object in run mode. 
	*/
	
	derefclone (&h); /*click on a clone is the same as clicking on the object*/
	
	if (sameasbuttontype ((**h).objecttype)) {
		
		if (sendbuttonhitmessage (h)) {
		
			if (iowadata != nil) /*protect against a closed window*/
				schedulerecalc (h, 0);
			
			return;
			}
		}
		
	if (callclick (listhead, h, mousestatus.localpt, keyboardstatus.flshiftkey, mousestatus.fldoubleclick)) {
	
		if (iowadata == nil) /*protect against closed window*/
			return;
		
		if (!flquickrecalc) { /*1/25/93 DW: very rarely you need to disable the quick recalc*/
			
			flquickrecalc = true; /*must be reset for every call*/
			
			return;
			}
		
		/*1/24/93 DW: streamlined for quick response to click and in recalc*/
			
		runmodeupdate (); /*avoid lag in display during massive recalcs*/
	
		frontSetRuntimeCard (true, false); /*make this card's table global*/
		
		(**iowadata).recalcobject = h;
		
		visitobjects ((**iowadata).objectlist, &minorrecalcvisit); /*recalc now, avoid lag*/
	
		runmodeupdate (); /*avoid lag in display, agents might need to run now*/
		
		(**iowadata).runmodechanges = true;
		}
	} /*objectclick*/
	
	
static boolean returnkeyvisit (hdlobject h) {

	hdlcard hc = iowadata;
	
	if (!(**h).objectenabled) /*keep searching*/
		return (true);
		
	if (!callcatchreturn (h)) /*object doesn't want the Return key*/
		return (true);
		
	(**hc).tracking = true; /*this is what bold buttons do, for example*/

	(**hc).trackerobject = h;

	hotobject (true); /*display button in its hot state*/
	
	waitfornexttick (); waitfornexttick (); waitfornexttick ();
	
	while (returnkeydown ()) {};
	
	hotobject (false); /*display button in its un-hot state*/

	(**hc).tracking = false;
	
	objectclick ((**iowadata).objectlist, h);
	
	return (false); /*stop searching*/
	} /*returnkeyvisit*/
	

static char upperchar (char ch) {

	if ((ch >= 'a') && (ch <= 'z'))
		return (ch - 32);
	else
		return (ch);
	} /*upperchar*/


static boolean mapkeyvisit (hdlobject h) {
	
	/*
	dmb 1.0b20: map chescape to the cancel button
	*/
	
	hdlcard hc = iowadata;
	bigstring bs;
	
	if (!(**h).objectenabled) /*keep searching*/
		return (true);
		
	if ((**h).objecttype != buttontype) /*keep searching*/
		return (true);
		
	texthandletostring ((**h).objectvalue, bs);
	
	if (stringlength (bs) == 0)
		return (true);
	
	if (keyboardstatus.chkb == chescape) {
		
		if (!equalstrings (bs, "\pCancel"))
			return (true);
		}
	else {
	
		if (upperchar (bs [1]) != upperchar (keyboardstatus.chkb))
			return (true);
		}
	
	(**hc).tracking = true; 

	(**hc).trackerobject = h;

	hotobject (true); /*display button in its hot state*/
	
	waitfornexttick (); waitfornexttick (); waitfornexttick ();
	
	hotobject (false); /*display button in its un-hot state*/

	(**hc).tracking = false;
	
	objectclick ((**iowadata).objectlist, h);
	
	return (false); /*stop searching*/
	} /*mapkeyvisit*/


static boolean runmodekeystroke (void) {
	
	/*
	return true if the keystroke was consumed by runtime, otherwise it is 
	passed back to the eventhandler.
	
	dmb 1.0b20: map cmd-. and escape to the cancel button, if present
	and enabled. otherwise, close card as before.
	
	dmb 1.0b20: allow option-Return to type into text field
	
	dmb 1.0b21: on chescape, don't set flexitcardloop true if closeCardWindow
	returns false
	*/
	
	hdlcard hc = iowadata;
	char ch = keyboardstatus.chkb;
	hdlobject activetextobject = (**hc).activetextobject;
	
	if (keyboardstatus.flcmdkey) {
		
		#ifdef cmdPeriodKillsCard /*disabled for LSE on 1/27/95 by DW*/
	
			/*
			if (ch == '.') {
				
				flexitcardloop = true;
				
				closeCardWindow ((**runtimedata).macwindow);
				
				return (true);
				}
			*/
			if (ch == '.')
				keyboardstatus.chkb = ch = chescape;
			else

		#endif
			
		return (false); /*we pass on all other cmd-keystrokes*/
		}
		
	if (ch == chescape) { /*escape always kills the card*/
	
		visitobjects ((**hc).objectlist, &mapkeyvisit); /*dmb 1.0b20*/
		
		if (!flexitcardloop) { /*didn't map to enabled Cancel button. close anyway*/
			
			closeCardWindow ((**runtimedata).macwindow);
			}
		
		return (true);
		}
		
	if (ch == chtab) {
		
		movetextboxcursor (keyboardstatus.flshiftkey);
		
		return (true);
		}
		
	if ((ch == chreturn) || (ch == chenter)) {
	
		if (!keyboardstatus.floptionkey || activetextobject == nil) /*dmb 1.0b20*/
			if (!visitobjects ((**hc).objectlist, &returnkeyvisit))
				return (true);
		}
		
	if (activetextobject == nil) { /*3/18/93 DW -- map keystroke to button*/

		visitobjects ((**hc).objectlist, &mapkeyvisit);
		
		return (true);
		}
		
	callkeyobject (activetextobject, ch);
	
	schedulerecalc (activetextobject, textticksrecalc);
		
	return (true);
	} /*runmodekeystroke*/


static void runmodetracker (hdlobject listhead, hdlobject x) {

	hdlcard hc = iowadata;
	Rect r;
	boolean flclick;
	hdlobject htarget;
	
	htarget = x;
	
	if (!(**htarget).objectenabled)
		return;
	
	derefclone (&htarget);
	
	if (!(**htarget).objectenabled)
		return;
	
	if (callhandlesmousetrack (htarget)) { /*object handles its own mouse tracking*/
	
		objectclick (listhead, x);
			
		return;
		}
		
	getobjectrect (x, &r);
	
	(**hc).tracking = true;
	
	(**hc).trackerobject = x;
	
	hotobject (true); /*display object in its hot state*/
	
	flclick = false;
	
	if (mousetrack (r, &hotobject)) {
	
		hotobject (false); /*display object in its un-hot state*/
		
		flclick = true;
		}
	
	(**hc).tracking = false;
	
	if (flclick)
		objectclick (listhead, x);
	} /*runmodetracker*/
	
	
static boolean findrunmodeobject (Point pt, hdlobject listhead, hdlobject *hparent, hdlobject *hobject) {
	
	/*
	special object-finder for runmode. it passes right thru clones.
	*/
	
	hdlobject x;
	
	while (true) { 
		
		if (!findobject (pt, listhead, &x))
			return (false);
			
		switch ((**x).objecttype) {
			
			case grouptype:
				getchildobjectlist (x, &listhead);
				
				break;
				
			case clonetype:
				listhead = x;
				
				derefclone (&listhead);
				
				break;
				
			default:
				*hparent = listhead;
			
				*hobject = x;
			
				return (true);
			} /*switch*/
		} /*while*/
	} /*findrunmodeobject*/
	

static void runmodecontentclick (WindowPtr w, Point pt) {
			
	hdlobject listhead, x;
		
	if (w != FrontWindow ()) {
	
		if ((**runtimedata).ownwindow)
			SelectWindow (w);
		
		return;
		}
	//Code change by Timothy Paustian Wednesday, August 23, 2000 9:20:43 PM
	//
	{	
	#if TARGET_API_MAC_CARBON
	CGrafPtr	thePort = GetWindowPort(w);
	#else
	GrafPtr	thePort = (GrafPtr)w;
	#endif

	SetPort (thePort);
	}
	mousestatus.localpt = pt;
	
	globaltolocalpoint (w, &mousestatus.localpt);

	listhead = (**iowadata).objectlist;
	
	if (findrunmodeobject (mousestatus.localpt, listhead, &listhead, &x)) 
		runmodetracker (listhead, x);
	} /*runmodecontentclick*/
	

static void runmodemouseup (long when, Point where) {
	
	/*
	call this when you receive an mouse up event.  if the last mouse down was
	a double click, we set things up so that the next single click will not
	be interpreted as a double click.
	*/
	
	if (!mousestatus.fldoubleclickdisabled) {
		
		mousestatus.mouseuptime = when;
		
		mousestatus.mouseuppoint = where;
		
		mousestatus.mousedowntime = 0L; /*hasn't happened yet*/
		}
	
	mousestatus.fldoubleclickdisabled = false; /*next mouse up is important*/
	} /*runmodemouseup*/


static boolean runmodemousedown (long when, Point pt, boolean *flcloseallwindows) {
	
	/*
	1.0b20 dmb: made boolean, return false if we don't handle it.
	*/
	
	WindowPtr w;
	short part;
	boolean flhandled = true;
	
	mousestatus.mousedowntime = when; 
	
	mousestatus.mousedownpoint = pt;
	
	part = FindWindow (pt, &w);
	
	switch (part) {
		
		case inDrag: {
			Rect r;
			//Code change by Timothy Paustian Monday, June 26, 2000 3:15:37 PM
			//
			#if TARGET_API_MAC_CARBON == 1
			BitMap screenBits;
			GetQDGlobalsScreenBits(&screenBits);
			r = screenBits.bounds;
			#else	
			r = quickdrawglobal (screenBits).bounds; 
			#endif
		
			r.top = r.top + GetMBarHeight (); 
			
			InsetRect (&r, 4, 4);
			
			DragWindow (w, pt, &r);
			
			break;
			}
			
		case inContent:
			runmodecontentclick (w, pt);
			
			break;
			
		case inGoAway:
			if (TrackGoAway (w, pt)) {
			
				if (keyboardstatus.floptionkey) {
				
					closeAllCardWindows ();
					
					*flcloseallwindows = true;
					}
				else
					closeCardWindow (w); 
				}
				
			break;
		
		default:
			flhandled = false;
			
			break;
		} /*switch*/
	
	return (flhandled);
	} /*runmodemousedown*/


static void runmodecheckupdate (void) {

	hdlcard hc = iowadata;
	WindowPtr w = (**runtimedata).macwindow;
	Rect		portRect;
	
	if ((**hc).needsupdate && ((**hc).runmode)) {
		
		(**hc).needsupdate = false; /*consume it*/
				
		//Code change by Timothy Paustian Monday, August 21, 2000 9:34:50 PM
		//have to use the port can't cast a window in OS X
		{
		CGrafPtr	thePort;
		#if TARGET_API_MAC_CARBON == 1
		thePort = GetWindowPort(w);
		#else
		thePort = (CGrafPtr)w;	
		#endif
		pushmacport (thePort);
		}
		
		//Code change by Timothy Paustian Friday, May 5, 2000 10:55:27 PM
		//Changed to Opaque call for Carbon
		#if ACCESSOR_CALLS_ARE_FUNCTIONS == 1
		{
		CGrafPtr	thePort = GetWindowPort(w);
		GetPortBounds(thePort, &portRect);
		}
		#else
		portRect = w->portRect;
		#endif
		
		pushclip (portRect);
		
		iowapreupdate (); /*actually inval everything that's been logically inval'd*/
	
		runmodeupdate ();
		
		popclip ();
		
		popmacport ();
		}
	} /*runmodecheckupdate*/
	

static boolean alwaysidlevisit (hdlobject h) {
	
	if (callalwaysidle (h))
		callidleobject (h);
		
	return (true);
	} /*alwaysidlevisit*/
	
	
static void runmodeidle (void) {
	
	hdlcard hc = iowadata;
	boolean flcursorset = false;
	hdlobject hparent, h;

	GetMouse (&mousestatus.localpt);
	
	if (findrunmodeobject (mousestatus.localpt, (**hc).objectlist, &hparent, &h)) {
		
		if ((**h).objectenabled) 
			flcursorset = callsetcursor (h, mousestatus.localpt);
		}
	
	if (!flcursorset)
		setcursortype (cursorisarrow);

	callidleobject ((**hc).activetextobject);
	
	visitobjects ((**hc).objectlist, &alwaysidlevisit);

	runmodecheckupdate ();
	
	checkrecalc ();
	} /*runmodeidle*/
	
	
boolean closeCardWindow (WindowPtr w) {
	
	Boolean ownwindow;
	
	if (!isCardWindow (w))
		return (false);
	
	runtimedata = getwindowdata (w); /*set up global for runmode routines*/
	
	iowadata = getwindowcardrecord (w); /*set up global for card routines*/
	
	if (!sendclosemessage ()) /*abort the close*/
		return (false);
	
	ownwindow = (**runtimedata).ownwindow;
	
	#ifdef iowaRuntimeInApp /*12/5/93 DW -- baling wire*/
		iowadata = getwindowcardrecord (w); 
	#endif
	
	//Code change by Timothy Paustian Monday, August 21, 2000 9:34:50 PM
	//have to use the port can't cast a window in OS X
	{
	CGrafPtr	thePort;
	#if TARGET_API_MAC_CARBON == 1
	thePort = GetWindowPort(w);
	#else
	thePort = (CGrafPtr)w;	
	#endif
	pushmacport (thePort);
	} /*8/27/94 DW -- on quit, if one modeless card is open, the port isn't properly set*/
	
	clearactivetextobject ();
	
	popmacport ();
	
	if (ownwindow)
		HideWindow (w); /*12/16/93 -- immediate feedback*/
	
	iowadisposerecord (); /*disposes iowadata and its contents*/

	iowadata = nil; /*help out EvenBetterBusError*/

	disposehandle ((Handle) runtimedata);
	
	runtimedata = nil; /*help out EvenBetterBusError*/
	
	removefromwindowarray (w);
	
	if (ownwindow)
		DisposeWindow (w);
	
	flexitcardloop = true;
	
	return (true);
	} /*closeCardWindow*/
	
	
void closeAllCardWindows (void) {

	short ct, i;
	
	ct = sizewindowarray ();
	
	for (i = 0; i < ct; i++) 
		closeCardWindow ((*hwindowarray) [i].w);
	} /*closeAllCardWindows*/
	

static void initeditabletext (void) {
	
	/*
	activate the first edittext object we find.
	*/
	
	hdlobject h;
	
	if (getfirsttextobject (&h)) {
	
		derefclone (&h);
		
		calleditobject (h, true);
		
		validobject (h); 
		}
	} /*initeditabletext*/
	

static boolean nilvaluesvisit (hdlobject h) {
	
	/*
	if an object is editable, and it recalcs,  we dispose of the initial value, 
	which helped the card designer visualize what the running card would look 
	like, but has no value for the card user.
	
	example: if you have a text object initialized to user.name, without
	this hack, you'd see the designer's name first, then the user's name.
	*/
	
	if (!calleditableinrunmode (h))
		return (true);
		
	if ((**h).objectrecalcstatus == neverrecalc)
		return (true);
	
	disposehandle ((**h).objectvalue);
	
	(**h).objectvalue = nil;
	
	return (true);
	} /*nilvaluesvisit*/
	

static void cardcheckinit (void) {
	
	/*
	1.0b23 dmb: call startCard script directly from here, after doing 
	preliminary auto-initialization. also, detect when startCard sets 
	the active text and don't override it.
	*/
	
	if ((**runtimedata).initfields) {
		
		if (!(**iowadata).notscriptedcard) {
			
			hdlobject objectlist = (**iowadata).objectlist;
			Handle hscript;
			bigstring errorstring;
		
			frontSetRuntimeCard (true, false);
		
			visitobjects (objectlist, &nilvaluesvisit);
			
			visitobjects (objectlist, &initexclusiveobjectsvisit);
			
			turnonfirstexclusiveobject (objectlist);
			
			/*1.0b23 dmb: call startCard now that we've done default setup*/
			newtexthandle ("\pif defined (startCard) {startCard ()}", &hscript);
			
			if (!runcardscript (hscript, idfrontier, false, errorstring, nil)) {
				
				if ((**iowadata).runmode)
					alertdialog (errorstring);
				}
			
			visitobjects (objectlist, &majorrecalcvisit);
			}
		
		iowapreupdate ();
		
		runmodeupdate (); 
		
		if ((**iowadata).activetextobject == nil) /*startup script didn't init text*/
			initeditabletext ();
		
		(**runtimedata).initfields = false;
		}
	} /*cardcheckinit*/


#pragma global_optimizer off

Rect edittextrect;

static boolean objectVisit (hdlobject h) {

	/*
	7.0b54 PBS: Visit routine.
	*/
	
	if ((**h).objecttype == edittexttype) {
	
		edittextrect = (**h).objectrect;
		
		return (false); /*stop visit*/
		}
		
	return (true); /*continue visit*/
	} /*objectVisit*/


static boolean cardContainsEditTextObject (hdlcard h) {
	
	/*
	7.0b54 PBS: For OS X appearance we need to know if a card contains
	an editable text object. If not, we we can use the Theme background.
	If it does, we have to use a color background.*/
	
	if (!IOAvisitobjects ((**h).objectlist, &objectVisit))
		return (true);
	
	return (false);
	} /*cardContainsEditTextObject*/


static void frameedittextrect (Rect r, WindowPtr w) {
	
	/*
	draw a border, leaving out the right side of the rect.  makes it prettier
	if you're displaying a scrollbar to the right of the rect, for example.

	5.0a25 dmb: for Win vesion we now use this routine to frame the content
	areas
	
	2002-10-13 AR: Declared static to eliminate compiler warning about
	missing function prototype. Commented out variable declaration for depth
	to eliminate compiler warning about unused variable.
	*/
	
	--r.bottom;
	
	movepento (r.right, r.top); 
	
	pendrawline (r.left, r.top); /*draw top of box*/
	
	pendrawline (r.left, r.bottom); /*draw left side of box*/
	
	pendrawline (r.right, r.bottom); /*draw bottom of box*/
	
	#if TARGET_API_MAC_CARBON == 1
	
		{
		Rect rwindow;
		Rect rbackground;
		//short depth;
		CGrafPtr thePort = GetWindowPort (w);
		
		GetPortBounds (thePort, &rwindow);

		//depth = maxdepth (&rwindow);
		
		rwindow = r;
				
		pushpen ();
		
		setthemepen (kThemeBrushDialogBackgroundActive, rwindow, true);
		
		rbackground.top = 0;
		rbackground.left = 0;
		rbackground.bottom = r.top - 1;
		rbackground.right = rwindow.right;
		
		paintrect (rbackground);
		
		rbackground.right = r.left - 1;
		rbackground.bottom = rwindow.bottom;
		
		paintrect (rbackground);
		
		rbackground.left = r.right;
		rbackground.right = rwindow.right;
		
		paintrect (rbackground);
		
		rbackground.left = 0;
		rbackground.top = r.bottom + 1;
		rbackground.bottom = rwindow.bottom;
		
		paintrect (rbackground);
		
		poppen ();
		}
	
	
	#endif
	} /*frame3sides*/

WindowPtr newCardWindow (Point pt, boolean notscriptedcard, Handle hpackedcard) {
	
	/*
	create a new window to run the packed card.
	
	dmb 6/27/96: don't disposehandle (iowadata); must iowadisposerecord.
				 (fixes memory leak.)
	*/
	
	WindowPtr w = nil;
	hdlruntimerecord hw;
	hdlcard hc;
	
	iowadata = nil;
	
	if (!newclearhandle (longsizeof (tyruntimerecord), (Handle *) &runtimedata))
		return (false);
	
	if (!newclearhandle (longsizeof (tycard), (Handle *) &iowadata))
		goto error;
		
	hw = runtimedata; 
	
	hc = iowadata; 
		
	(**hw).hc = hc;
	
	(**hc).runmode = true; 
	
	if (!iowaunpack (hpackedcard))
		goto error;

	(**hc).runmode = true;
	
	(**hc).notscriptedcard = notscriptedcard;
	
	(**hc).updatecallback = &runmodeupdate;
	
	/*DW 10/17/93: transition from default window resources, to letting the card specify*/ {
		
		short resnum = (**hc).idwindow; /*first try, use the resnum specified by card editor*/
		
		if (systemhascolor ())
			w = GetNewCWindow (resnum, nil, (WindowPtr) -1);
		else
			w = GetNewWindow (resnum, nil, (WindowPtr) -1);
		
		if (w == nil)
			goto error;
		
		#if TARGET_API_MAC_CARBON == 1 /*Theme background on OS X*/
		
			if (!cardContainsEditTextObject (hc))
				
				SetThemeWindowBackground (w, kThemeBrushModelessDialogBackgroundActive, false);

			else {
			
				SetPort (GetWindowPort (w));

				frameedittextrect (edittextrect, w);
				} /*else*/

		#endif
		}
	
	SizeWindow (w, (**hc).rightborder, (**hc).bottomborder, false);
	
				
				ShowWindow (w);
	//Code change by Timothy Paustian Wednesday, August 23, 2000 9:20:56 PM
		//
	{
	#if TARGET_API_MAC_CARBON
	CGrafPtr	thePort = GetWindowPort(w);
	#else
	GrafPtr	thePort = (GrafPtr)w;
	#endif

	SetPort (thePort);
	}
	
	if ((pt.h != 0) && (pt.v != 0))
		MoveWindow (w, pt.h, pt.v, false);
	else
	{
		//Code change by Timothy Paustian Monday, June 26, 2000 3:15:51 PM
		//
		#if TARGET_API_MAC_CARBON == 1
		BitMap		screenBits;
		GetQDGlobalsScreenBits(&screenBits);
		centerwindow(w, screenBits.bounds);
		#else
		centerwindow (w, quickdrawglobal (screenBits).bounds);
		#endif
		
	}
	(**hw).macwindow = w;
	
	(**hw).ownwindow = true;
	
	(**hw).initfields = true;
	
	/*set the window title*/ {
		
		bigstring bs;
		
		texthandletostring ((**hc).windowtitle, bs);
		
		SetWTitle (w, bs);
		}
		
	(**hc).drawwindow = w; /*all drawing goes into this window*/
	
	IACglobals.waitroutine = nil; /*no callback when in runmode*/
	
	sortobjectlist (); /*sort objects based on window position*/
	
	laythread (); /*make sure flat threads are installed*/
	
	addtowindowarray (w, hw);
	
	/*SetWRefCon (w, (long) runtimedata);*/
	
	return (w);
	
	error:
	
	iowadisposerecord ();
	
	disposehandle ((Handle) runtimedata);
	
	if (w != nil)
		DisposeWindow (w);
	
	return (nil);
	} /*newCardWindow*/
	
#pragma global_optimizer reset

	
boolean visitCardWindows (tycardwindowvisitroutine visit) {

	short ct, i;
	
	ct = sizewindowarray ();
	
	for (i = 0; i < ct; i++) { /*look for an empty slot in the window array*/
		
		WindowPtr w = (*hwindowarray) [i].w;
		
		if (w != nil) {
			
			runtimedata = getwindowdata (w); /*set up global for runmode routines*/
	
			iowadata = getwindowcardrecord (w); /*set up global for card routines*/
			
			if (!(*visit) (w))
				return (false);
			}
		} /*for*/
		
	return (true);
	} /*visitCardWindows*/
	
	
void getCardData (WindowPtr w, hdlruntimerecord *hcardwindow) {
	
	/*
	after calling newCardWindow, you can use this entry to get the data handle
	linked into the window, so you can override the default settings.
	*/
	
	*hcardwindow = getwindowdata (w); 
	} /*getCardData*/
	
	
boolean cardEditCommand (short editcommand) {
	
	WindowPtr w = FrontWindow ();
	hdlcard hcard;
	hdlobject hobject;
	hdleditrecord hedit;
	
	if (!inwindowarray (w)) /*it's not one of our windows, app handles the command*/
		return (false);
	
	hcard = getwindowcardrecord (w); /*set up global for card routines*/
	
	hobject = (**hcard).activetextobject;
	
	if (hobject == nil) /*consume the command, but there's nothing to do*/
		return (true);
		
	hedit = (hdleditrecord) (**hobject).objecteditrecord;
		
	switch (editcommand) {
		
		case cutCmd:
			editcut (hedit);
			
			break;
			
		case copyCmd:
			editcopy (hedit);
			
			break;
			
		case pasteCmd:
			editpaste (hedit);
			
			(**hcard).runtimevaluechanged = true; /*DW 10/27/95*/
			
			break;
			
		case clearCmd:
			editreplace ("\p", hedit);
			
			break;
			
		} /*switch*/
		
	return (true);
	} /*cardEditCommand*/


static boolean showinvisiblesvisit (hdlobject h) {
	
	derefclone (&h);
	
	if (!(**h).objectvisible) {
		
		Rect r;
		
		getobjectrect (h, &r);
		
		pushpen ();
		//Code change by Timothy Paustian Monday, June 26, 2000 3:15:59 PM
		//
		#if TARGET_API_MAC_CARBON == 1
		{
		Pattern gray;
		GetQDGlobalsGray(&gray);
		PenPat(&gray);
		}
		#else
		PenPat (&quickdrawglobal (gray)); 
		#endif
		FrameRect (&r);
		
		poppen ();
		//Code change by Timothy Paustian Monday, June 26, 2000 3:16:06 PM
		//
		#if TARGET_API_MAC_CARBON == 1
		if((**iowadata).drawwindow != nil)
			InvalWindowRect((**iowadata).drawwindow, &r);
		#else
		InvalRect (&r);
		#endif
		
		}
		
	return (true);
	} /*showinvisiblesvisit*/
	

static void checkinvisibles (void) {
	
	boolean flfirstloop = true;
	
	if (iowadata == nil) /*defensive, it can happen*/
		return;
	
	while (cmdkeydown () && optionkeydown ()) {
		
		if (flfirstloop) {
		
			visitobjects ((**iowadata).objectlist, &showinvisiblesvisit);
			
			flfirstloop = false;
			}
		} /*while*/
		
	if (!flfirstloop) 
		runmodeupdate ();
	} /*checkinvisibles*/


boolean handleCardEvent (WindowPtr w, EventRecord *ev, boolean *flcloseallwindows) {
	
	/*
	returns true if the event has been handled. 
	
	if false, you should handle it.
	
	dmb 1.0b21: for iowaOkeyDokeyEvent, press default button if there is one,
	else the cancel	button. In any case, we'll end up calling closeCardWindow
	
	dmb 1.0b24: don't filter activate and update event that are ours. The client
	will see those events if they're not for us.
	*/
	
	boolean flhandled;
	CGrafPtr	thePort;
	
	*flcloseallwindows = false; 

	setkeyboardstatus ((*ev).message, (*ev).what, (*ev).modifiers); /*set fields of keyboardstatus record*/
		
	if (!inwindowarray (w)) /*it's not one of our windows, you handle the event*/
		return (false);

	runtimedata = getwindowdata (w); /*set up global for runmode routines*/
		
	iowadata = getwindowcardrecord (w); /*set up global for card routines*/
	
	if ((*ev).what != activateEvt && (*ev).what != updateEvt) { /*1.0b24 dmb: don't give them our updates & activate*/
		filterevent (ev); /*pass the event thru the callback routine if there is one*/
		}
	
	iowadata = getwindowcardrecord (w); /*set up global for card routines*/
	
	(**iowadata).macevent = *ev; /*for anyone with an interest in the event record*/
	
	releasetemphandles (); /*dispose of any temps created by the previous event*/
	
	//Code change by Timothy Paustian Monday, August 21, 2000 9:34:50 PM
	//have to use the port can't cast a window in OS X
	{
	#if TARGET_API_MAC_CARBON == 1
		thePort = GetWindowPort(w);
	#else
		thePort = (CGrafPtr)w;	
	#endif
	pushmacport (thePort);
	}
	
	cardcheckinit ();
	
	//iowavalidate (true); 
	
	flhandled = true; /*default, we handled the event*/
	
	switch ((*ev).what) {
	
		case keyDown: case autoKey:
			flhandled = runmodekeystroke ();
			#if TARGET_API_MAC_CARBON == 1
				//a key stoke was entered. We need to flush it to the screen on OS X
				if (!flhandled)
					QDFlushPortBuffer(thePort, nil);
			#endif
			break;
			
		case mouseUp:
			runmodemouseup ((*ev).when, (*ev).where);
			
			break;
		   
		case mouseDown:	
			flhandled = runmodemousedown ((*ev).when, (*ev).where, flcloseallwindows);
			
			break;	
			
		case activateEvt:
			runmodeactivate ((*ev).modifiers & activeFlag);
			
			break;
		
		case updateEvt:
			runmodeupdate ();
			
			break;
		
		/*4.1b4 dmb: must handle own activates*/
		case osEvt:
			runmodeactivate ((*ev).modifiers & resumeFlag);
			
			break;
		
		case nullEvent:

			if (w == FrontWindow ())
				runmodeidle ();
			else
				runmodecheckupdate (); /*DW 9/13/93: may need update even if it's not in front*/
			
			flhandled = false; /*let the client get a shot at null events too*/
			
			break;
		
		case iowaOkeyDokeyEvent: /*dmb 1.0b21*/
			flhandled = !visitobjects ((**iowadata).objectlist, &returnkeyvisit);
			
			if (!flhandled) {
				
				keyboardstatus.chkb = chescape;
				
				flhandled = runmodekeystroke ();
				}
			
			break;
		} /*switch*/

	checkinvisibles ();
	
	popmacport ();
	
	return (flhandled);
	} /*handleCardEvent*/
	
	
static boolean nullsforallvisit (WindowPtr w) {
	
	hdlruntimerecord hcw = getwindowdata (w);
	EventRecord ev;
	boolean flcloseallwindows;
	
	if ((**hcw).modeless) {
	
		clearbytes (&ev, longsizeof (ev));
	
		ev.what = nullEvent;
	
		handleCardEvent (w, &ev, &flcloseallwindows);
		}
	
	return (true);
	} /*nullsforallvisit*/
	
	
static boolean nullsforall (void) {
	
	/*
	send a null event to every modeless card that's open.
	*/
	
	hdlruntimerecord saveruntimedata = runtimedata;
	hdlcard saveiowadata = iowadata;
	GrafPtr oldport;
	
	GetPort (&oldport);

	visitCardWindows (&nullsforallvisit);

	runtimedata = saveruntimedata;
	iowadata = saveiowadata;
	SetPort (oldport);
	
	return (true);
	} /*nullsforall*/
	
	
boolean setObjectValue (hdlcard hcard, bigstring name, Handle hvalue) {

	hdlcard oldcard = iowadata;
	boolean fl;
	
	iowadata = hcard;
	
	fl = setCardValue (name, hvalue);
	
	iowadata = oldcard;
	
	return (fl);
	} /*setObjectValue*/
	
	
boolean getObjectValue (hdlcard hcard, bigstring name, Handle *hvalue) {
	
	hdlcard oldcard = iowadata;
	boolean fl;
	
	iowadata = hcard;
	
	fl = getCardValue (name, hvalue);
	
	iowadata = oldcard;
	
	return (fl);
	} /*getObjectValue*/


boolean getObjectHandle (hdlcard hcard, bigstring name, hdlobject *h) {
	
	hdlcard oldcard = iowadata;
	boolean fl;
	
	iowadata = hcard;
	
	fl = lookupname (name, h);
	
	iowadata = oldcard;
	
	return (fl);
	} /*getObjectHandle*/
	
	
boolean recalcObject (hdlobject h) {
	
	/*
	Card Editor has all of its recalculation done for it thru Iowa
	Runtime. the object handle has a handle to the card that contains
	it. its objecttmpbit is set true if it's a major recalc, false
	if its a minor one.
	*/
	
	hdlcard oldcard = iowadata;
	boolean fl;
	
	iowadata = (**h).owningcard;
	
	fl = recalcbottleneck (h, (**h).objecttmpbit);
	
	(**h).objecttmpbit = false;
	
	iowadata = oldcard;
	
	return (fl);
	} /*recalcObject*/
	
	
boolean updateCard (hdlcard hcard) {

	hdlcard oldcard = iowadata;
	
	iowadata = hcard;
	
	runtimedata = getwindowdata ((**iowadata).drawwindow); 
	
	runmodecheckupdate ();
	
	runmodeupdate ();
	
	iowadata = oldcard;
	
	if (iowadata == nil)
		runtimedata = nil;
	else
		runtimedata = getwindowdata ((**iowadata).drawwindow); 
	
	return (true);
	} /*updateCard*/


boolean cardIsModal (Handle hpackedcard) {

	tydiskheader header;
	
	moveleft (*hpackedcard, &header, longsizeof (header));
	
	switch (header.idwindow) {
		
		case 1024: case 1025:
			return (true);
			
		case 1026: case 1027:
			return (false);
			
		} /*switch*/
		
	#ifdef fldebug
	
		DebugStr ("\punexpected window resource id");
		
	#endif
	
	return (true);
	} /*cardIsModal*/
	
	
static boolean returnNamedValueVisit (hdlobject h) {
	
	/*
	a special AE that might only be useful in Card Editor's
	double-click dialog. but it makes it much faster.
	
	we return the values of all objects that have four-
	character names. only checkboxes, radio buttons and 
	edittexts.
	*/
	
	Handle hname, hvalue;
	bigstring bs;
	OSType key;
	
	getobjectname (h, &hname);
	
	if (hname == nil)
		return (true);
		
	texthandletostring (hname, bs);
		
	if (stringlength (bs) != 4) /*keep going*/
		return (true);
	
	BlockMove (&bs [1], &key, 4);
	
	switch ((**h).objecttype) {
		
		case checkboxtype: case radiobuttontype:
			IACpushbooleanparam ((**h).objectflag, key);
			
			break;
			
		case edittexttype:
			getobjectvalue (h, &hvalue);
			
			copyhandle (hvalue, &hvalue);
			
			IACpushtextparam (hvalue, key);
			
			break;
		} /*switch*/
		
	return (true); /*keep visiting*/
	} /*returnNamedValueVisit*/


static void getCardAttributesVerb (void) {
	
	Boolean returnedValue;
	
	returnedValue = IACgetcardattributes ();
	
	IACreturnboolean (returnedValue);
	} /*getCardAttributesVerb*/


static void setCardAttributesVerb (void) {
	
	Boolean returnedValue;
	
	returnedValue = IACsetcardattributes ();
	
	IACreturnboolean (returnedValue);
	} /*setCardAttributesVerb*/


static void getObjectAttributesVerb (void) {
	
	Handle param1;
	Boolean returnedValue;
	
	if (!IACgettextparam ('prm1', &param1))
		return;
	
	returnedValue = IACgetobjectattributes (param1);
	
	IACreturnboolean (returnedValue);
	} /*getObjectAttributesVerb*/


static void setObjectAttributesVerb (void) {
	
	Handle param1;
	Boolean returnedValue;
	
	if (!IACgettextparam ('prm1', &param1))
		return;
	
	returnedValue = IACsetobjectattributes (param1);
	
	IACreturnboolean (returnedValue);
	} /*setObjectAttributesVerb*/
	
	
static pascal OSErr appleEventHandler (const AppleEvent *event, AppleEvent *reply, long refcon) {
	
	/*
	4.1b14 dmb: since we're installing out handler in the client's process,
	we need to set up our A5 globals
	*/
	
	#if !TARGET_API_MAC_CARBON
	long curA5 = SetUpAppA5 ();
	#endif

	IACglobals.event = (AppleEvent *) event; 
	
	IACglobals.reply = reply;
	
	IACglobals.refcon = refcon;
		
	switch (IACgetverbtoken ()) {
	
		case 'clos':
			if (runtimedata != nil)
				(**runtimedata).closethiswindow = true;
			
			break;
			
		case 'gacd':
			getCardAttributesVerb (); 
			
			break;
		
		case 'sacd':
			setCardAttributesVerb (); 
			
			break;
		
		case 'gaob':
			getObjectAttributesVerb (); 
			
			break;
		
		case 'saob':
			setObjectAttributesVerb (); 
			
			break;
			
		case 'mema': {
			Size x;
	
			MaxMem (&x);
			
			IACreturnlong (FreeMem ());
			
			break;
			}
			
		case 'rnov':
			IACglobals.event = IACglobals.reply; /*push the values onto the reply record*/
			
			visitobjects ((**iowadata).objectlist, &returnNamedValueVisit); 
			
			break;
		} /*switch*/

	#if !TARGET_API_MAC_CARBON
	RestoreA5 (curA5);
	#endif
		
	return (noErr);
	} /*appleEventHandler*/

//Code change by Timothy Paustian Monday, June 26, 2000 3:11:06 PM
//switched to new conditional.
#if !TARGET_RT_MAC_CFM

	#define appleEventHandlerUPP (&appleEventHandler)
	
#else

	#if TARGET_API_MAC_CARBON == 1
	
	AEEventHandlerUPP appleEventHandlerDesc = nil;
	#define appleEventHandlerUPP (appleEventHandlerDesc)
	
	#else
		
	static RoutineDescriptor appleEventHandlerDesc = BUILD_ROUTINE_DESCRIPTOR (uppAEEventHandlerProcInfo, appleEventHandler);

	#define appleEventHandlerUPP (&appleEventHandlerDesc)
	#endif
		

#endif



void installRuntimeEventHandlers (void) {
	
	/*
	1.0b19 dmb: reinstall handlers every time we're called. It could be 
	the we're competing with handlers installed by other cards being 
	run from OSA scripts. along with this, we must be sure to create 
	the UPP just once, so we can't use IACinstallhandler.
	*/
	
	static boolean flinstalled = false;
	
	if (!flinstalled) {
		//Code change by Timothy Paustian Friday, July 21, 2000 10:36:08 PM
		//create the UPP for the apple event handler
		#if TARGET_API_MAC_CARBON && TARGET_RT_MAC_CFM
		if (appleEventHandlerDesc == nil)
			appleEventHandlerDesc = NewAEEventHandlerUPP(appleEventHandler);
		#endif
		AEInstallEventHandler ('card', typeWildCard, appleEventHandlerUPP, 0, false);
		
		/*
		IACinstallhandler ('card', typeWildCard, (ProcPtr) &appleEventHandler);
		
		flinstalled = true; // 1.0b19 dmb: commented out
		*/
		}
	} /*installRuntimeEventHandlers*/
	
	
static boolean passthruevent (short what) {
	
	return ((what == updateEvt) || (what == activateEvt) || (what == osEvt)  || (what == diskEvt));
	} /*passthruevent*/


boolean runModalCard (Handle hpackedcard, boolean flscriptedcard, short top, short left, tycardeventcallback callback) {
	
	/*
	dmb 1.0b24: changed flcallback logic. we no longer call back on activate
	and update events that we just handled
	*/
	
	hdlcard oldiowadata = iowadata; 
	hdlruntimerecord oldruntimedata = runtimedata;
	GrafPtr oldport;
	WindowPtr modalwindow;
	Point winpos;
	boolean flreturn;
	
	GetPort (&oldport);
	
	installRuntimeEventHandlers ();
	
	winpos.v = top;
	
	winpos.h = left;
	
	modalwindow = newCardWindow (winpos, !flscriptedcard, hpackedcard);
	
	flreturn = true;
		
	if (modalwindow == nil) {
		
		flreturn = false;
		
		goto exit;
		}
		
	(**runtimedata).callback = callback;
		
	(**runtimedata).modeless = false;
	
	if (flscriptedcard) {

		if (!frontStartCard (nil)) {
			
			flreturn = false;
			
			goto exit;
			}
		}
	//Code change by Timothy Paustian Wednesday, August 23, 2000 9:21:06 PM
	//
	{
	#if TARGET_API_MAC_CARBON
	CGrafPtr	thePort = GetWindowPort(modalwindow);
	#else
	GrafPtr	thePort = (GrafPtr)modalwindow;
	#endif

	SetPort (thePort);
	}

	sendinitmessage ();
		
	cardcheckinit (); /*1.0b23 dmb: init field before showing the card. more responsive*/
	
	flexitcardloop = false;
	
	ShowWindow (modalwindow);
	
	while (true) {
		
		EventRecord ev;
		WindowPtr eventwindow;
		boolean flcloseallwindows;
		boolean flcallback = false;
		
		WaitNextEvent (everyEvent, &ev, 1, nil);
		
		if (ev.what == nullEvent) /*send a null event to every modeless card*/
			nullsforall ();
			
		geteventwindow (&ev, &eventwindow);
		
		if (eventwindow == modalwindow)
			flcallback = !handleCardEvent (eventwindow, &ev, &flcloseallwindows);
		else
			flcallback = passthruevent (ev.what) || (eventwindow == nil);
		
		/*
		if (eventwindow == modalwindow) {
		
			flcallback = !handleCardEvent (eventwindow, &ev, &flcloseallwindows);
			}
		else {
			if ((passthruevent (ev.what)) || (eventwindow == nil)) {
				
				if (!handleCardEvent (eventwindow, &ev, &flcloseallwindows)) {
					
					flcallback = true;
					}
				}
			}
		*/
			
		if ((flcallback) && (callback != nil)) {
			
			hdlcard oldiowadata = iowadata;
			hdlruntimerecord oldruntimedata = runtimedata;
			
			(*callback) (&ev);
			
			iowadata = oldiowadata;
		
			runtimedata = oldruntimedata;
			//Code change by Timothy Paustian Wednesday, August 23, 2000 9:21:26 PM
			//
			{	
			#if TARGET_API_MAC_CARBON
			CGrafPtr	thePort = GetWindowPort(modalwindow);
			#else
			GrafPtr	thePort = (GrafPtr)modalwindow;
			#endif
		
			SetPort (thePort);
			}
			}
			
		if (flexitcardloop)
			break;
		
		if ((**runtimedata).closethiswindow) /*set true by an Apple Event*/
			closeCardWindow (modalwindow); 
		
		if (flexitcardloop) /*can be set by closeCardWindow*/
			break;
		} /*while*/
		
	exit:
		
	flexitcardloop = false; /*we consume it, modal cards can be nested*/
	
	/*restore saved globals*/ {
	
		iowadata = oldiowadata;
		
		runtimedata = oldruntimedata;
	
		SetPort (oldport);
		}
	
	return (flreturn);
	} /*runModalCard*/
	

boolean isModelessCardEvent (EventRecord *ev, boolean *flcloseallwindows) {
	
	hdlcard oldiowadata = iowadata; 
	hdlruntimerecord oldruntimedata = runtimedata;
	//Code change by Timothy Paustian Monday, June 26, 2000 3:16:19 PM
	//
	#if TARGET_API_MAC_CARBON == 1
	CGrafPtr oldport = GetQDGlobalsThePort();
	#else	
	GrafPtr oldport = quickdrawglobal (thePort);
	#endif
		boolean flreturn = false;
	WindowPtr w;
	
	if ((*ev).what == nullEvent) {
	
		nullsforall ();
		
		goto exit; /*return false -- let the main event loop process it too*/
		}
		
	geteventwindow (ev, &w);

	if (isCardWindow (w)) 
		flreturn = handleCardEvent (w, ev, flcloseallwindows);
	
	exit:
	
	/*restore saved globals*/ {
	
		iowadata = oldiowadata;
		
		runtimedata = oldruntimedata;
	
		SetPort (oldport);
		}
	
	return (flreturn);
	} /*isModelessCardEvent*/
	
	
boolean runModelessCard (Handle hpackedcard, boolean flscriptedcard, short top, short left, tycardeventcallback callback) {
	
	/*
	dmb 1.024: call cardcheckinit here, before showing window. 
	this accomplishes three things:
	
	1. It ensures that all fields are before the card is shown.
	
	2. Invoking a card is crisper; we don't need to wait for a null
	   event before running startup scripts.
	   
	3. It allows the calling script's context to be in force as our 
	   initial recalc scripts and starupScript run.
	*/
	
	hdlcard oldiowadata = iowadata; 
	hdlruntimerecord oldruntimedata = runtimedata;
	//Code change by Timothy Paustian Monday, June 26, 2000 3:15:15 PM
	//
	#if TARGET_API_MAC_CARBON == 1
	CGrafPtr oldport = GetQDGlobalsThePort();
	#else
	GrafPtr oldport = quickdrawglobal (thePort);
	#endif
		Point winpos;
	WindowPtr modelesswindow;
	boolean flreturn;
	
	installRuntimeEventHandlers ();
	
	winpos.v = top;
	
	winpos.h = left;
	
	modelesswindow = newCardWindow (winpos, !flscriptedcard, hpackedcard);
	
	flreturn = true;
	
	if (modelesswindow == nil) {
		
		flreturn = false;
		
		goto exit;
		}
		
	(**runtimedata).callback = callback;
		
	(**runtimedata).modeless = true;
	
	if (flscriptedcard) {
	
		if (!frontStartCard (nil)) {
		
			flreturn = false;
		
			goto exit;
			}
		}
	
	//Code change by Timothy Paustian Wednesday, August 23, 2000 9:21:06 PM
	//
	{
	#if TARGET_API_MAC_CARBON
	CGrafPtr	thePort = GetWindowPort(modelesswindow);
	#else
	GrafPtr	thePort = (GrafPtr)modelesswindow;
	#endif

	SetPort (thePort);
	}
	
	sendinitmessage ();
	
	cardcheckinit (); /*1.0b24 dmb: see comment above*/
	
	ShowWindow (modelesswindow);

	exit:
	
	/*restore saved globals*/ {
	
		iowadata = oldiowadata;
		
		runtimedata = oldruntimedata;
	
		SetPort (oldport);
		}
		
	return (flreturn);
	} /*runModelessCard*/
	 

boolean runCard (Handle hpackedcard, boolean flscriptedcard, short top, short left, tycardeventcallback callback) {
	
	boolean fl;
	
	if (cardIsModal (hpackedcard)) 
		fl = runModalCard (hpackedcard, flscriptedcard, top, left, callback);
	else
		fl = runModelessCard (hpackedcard, flscriptedcard, top, left, callback);
		
	return (fl);
	} /*runCard*/


#pragma global_optimizer off
	
static WindowPtr moveIntoCardWindow (WindowPtr hostwindow, boolean notscriptedcard, Handle hpackedcard) {
	
	/*
	use existing window to run the packed card.
	
	dmb 2.1.96: this could share plenty of code with newCardWindow
	*/
	
	WindowPtr w = hostwindow;
	hdlruntimerecord hw;
	hdlcard hc;
	
	iowadata = nil;
	
	if (!newclearhandle (longsizeof (tyruntimerecord), (Handle *) &runtimedata))
		return (false);
	
	if (!newclearhandle (longsizeof (tycard), (Handle *) &iowadata))
		goto error;
		
	hw = runtimedata; 
	
	hc = iowadata; 
	
	(**hw).hc = hc;
	
	(**hc).runmode = true; 
	
	if (!iowaunpack (hpackedcard))
		goto error;

	(**hc).runmode = true;
	
	(**hc).notscriptedcard = notscriptedcard;
	
	(**hc).updatecallback = &runmodeupdate;
	
	(**hw).macwindow = w;
	
	(**hw).ownwindow = false;
	
	(**hw).initfields = true;
	
	(**hc).drawwindow = w; /*all drawing goes into this window*/
	
	IACglobals.waitroutine = nil; /*no callback when in runmode*/
	
	sortobjectlist (); /*sort objects based on window position*/
	
	laythread (); /*make sure flat threads are installed*/
	
	addtowindowarray (w, hw);
	
	return (w);
	
	error:
	
	disposehandle ((Handle) iowadata);
	
	disposehandle ((Handle) runtimedata);
	
	iowadata = nil;
	
	runtimedata = nil;
	
	return (nil);
	} /*moveIntoCardWindow*/

#pragma global_optimizer reset
	
	
boolean runHostedCard (WindowPtr hostwindow, Handle hpackedcard, boolean flscriptedcard, tycardeventcallback callback) {
	
	hdlcard oldiowadata = iowadata; 
	hdlruntimerecord oldruntimedata = runtimedata;
	GrafPtr oldport;
	WindowPtr modelesswindow;
	boolean flreturn;
	
	GetPort (&oldport);
	
	installRuntimeEventHandlers ();
	
	modelesswindow = moveIntoCardWindow (hostwindow, !flscriptedcard, hpackedcard);
	
	flreturn = true;
	
	(**runtimedata).callback = callback;
	
	(**runtimedata).modeless = true;
	
	if (flscriptedcard) {
	
		if (!frontStartCard (nil)) {
		
			flreturn = false;
		
			goto exit;
			}
		}
	//Code change by Timothy Paustian Wednesday, August 23, 2000 9:21:06 PM
	//
	{
	#if TARGET_API_MAC_CARBON
	CGrafPtr	thePort = GetWindowPort(modelesswindow);
	#else
	GrafPtr	thePort = (GrafPtr)modelesswindow;
	#endif

	SetPort (thePort);
	}
	
	sendinitmessage ();

	exit:
	
	/*restore saved globals*/ {
	
		iowadata = oldiowadata;
		
		runtimedata = oldruntimedata;
	
		SetPort (oldport);
		}
		
	return (flreturn);
	} /*runHostedCard*/
	
	
boolean initIowaRuntime (void) {
	
	/*
	4.1b14 dmb: remember A5 for AE handler
	*/
	//Code change by Timothy Paustian Monday, June 26, 2000 3:16:32 PM
	//
	#if !TARGET_API_MAC_CARBON
	RememberA5 ();
	#endif
		
	return (true);
	} /*initIowaRuntime*/	
	
	
	
