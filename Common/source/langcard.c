
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
#include <uisharing.h>
#include <uisinternal.h>
#endif

#include "launch.h"
#include "memory.h"
#include "ops.h"
#include "strings.h"
#include "frontierwindows.h"
#include "shell.h"
#include "scrap.h"
#include "shellprivate.h"
#include "shellhooks.h"
#include "shellmenu.h"
#include "lang.h"
#include "langinternal.h"
#include "langipc.h"
#include "process.h"
#ifdef iowaRuntime
	#include "iowainit.h"
#endif




#if !defined (fliowa) && !defined (iowaRuntime)

static boolean langruncard (hdltreenode hparam1, boolean flmodal, tyvaluerecord *vreturned) {
	
	return (false);
	} /*langruncard*/

static boolean langismodalcard (hdltreenode hparam1, tyvaluerecord *vreturned) {
	
	return (false);
	} /*langismodalcard*/

#else


typedef void (*tycardeventcallback)(EventRecord *);
typedef long hdlcard;

extern boolean runCard (Handle hpackedcard, boolean flscriptedcard, short top, short left, void * callback);
extern boolean runModalCard (Handle, boolean, short, short, tycardeventcallback);
extern boolean runModelessCard (Handle, boolean, short, short, tycardeventcallback);
extern boolean cardIsModal (Handle hpackedcard);
extern WindowPtr getcardrecordwindow (hdlcard card);
extern boolean cardEditCommand (short editcommand);
extern boolean closeCardWindow (WindowPtr w);
extern void appletsegment (void);

#define iowaInitEvent 1000 /*constants for (*ev).what in callbacks*/
#define iowaButtonHitEvent 1001
#define iowaCloseEvent 1002
#define iowaRunScriptEvent 1003
#define iowaCancelEvent 1004 
#define iowaSetHandleEvent 1005
#define iowaOkeyDokeyEvent 1006


static WindowPtr langcardwindow = nil;

static hdlwindowinfo langcardwindowinfo = nil;

static Handle langcarddata = nil;

static boolean flshellclosingwindow = false;

static unsigned long modalcardstarttime;

static unsigned long modalcardtimeout = -1; /*maxulong*/


static void modalcardeventfilter (EventRecord *ev) {
	
	/*
	4.1b3 dmb: iowaruntime sends us all events, not just ones it does handle,
	do we have to be more selective about what we do.
	
	4.1b13 dmb: use new processevent langcallback for handling events.
	
	4.1b13 dmb: intercept cmd-XCV for Cut/Copy/Paste.
	
	4.1b13 dmb: use IdleTime gestalt when checking for card timeout
	*/
	
	switch ((*ev).what) {
		
		case iowaInitEvent:
			modalcardstarttime = gettickcount ();
			
			break;
		
		case iowaButtonHitEvent:
			(*ev).what = iowaRunScriptEvent;
			
			break;
		
		case keyDown:
			if (((*ev).modifiers & cmdKey) != 0) {
				
				boolean handled = true;
				
				switch ((*ev).message & charCodeMask) {
				
					case 'x':
						cardEditCommand (cutitem); break;
					
					case 'c':
						cardEditCommand (copyitem); break;
					
					case 'v':
						shellwritescrap (textscraptype);
						cardEditCommand (pasteitem); break;
					
					default:
						handled = false; break;
					}
				
				if (handled)
					(*ev).what = nullEvent; /*prevent it from coming around again*/
				}
			
			break;
		
		case nullEvent: {
			long cardticks = (*ev).when - modalcardstarttime;
			long idleticks;
			
			if (gestalt ('idle', &idleticks))
				cardticks = min (cardticks, idleticks);
			
			if (cardticks / 60 > modalcardtimeout) {
			
				(*ev).what = iowaOkeyDokeyEvent;
			
				break;
				}
			
			(*langcallbacks.processeventcallback) (ev);
			
			break;
			}
		
		case activateEvt:
		case updateEvt:
			/*
			if (*(long *)&(*ev).where == 'LAND')
				(*langcallbacks.processeventcallback) (ev);
			else
				*(long *)&(*ev).where = 'LAND';
			
			break;
			*/
		case osEvt:
		case diskEvt:
		case kHighLevelEvent:
			(*langcallbacks.processeventcallback) (ev);
			
			break;
		
		default:
			break;
		}  /*switch*/
	} /*modalcardeventfilter*/


static void modelesscardeventfilter (EventRecord *ev) {
	
	/*
	4.1b1 dmb: maintain a shellwindowinfo record attached to the card window
	
	4.1b4 dmb: set new fldontownwindow field so that shellclose doesn't 
	try to tass Iowa's window.
	
	5.0b13 dmb: fixed bug setting window title when creating card windowinfo
	*/
	
	switch ((*ev).what) {
		
		case iowaInitEvent: {
			hdlcard hcard = (hdlcard) (*ev).message;
			WindowPtr w = getcardrecordwindow (hcard);
			hdlwindowinfo hinfo, hparent;
			bigstring bstitle;
			
			if (newshellwindowinfo (w, &hinfo)) {
				
				(**hinfo).configresnum = idiowaconfig;
				
				if (shellgetfrontrootinfo (&hparent))
					(**hinfo).parentwindow = hparent;
				
				(**hinfo).processid = getcurrentprocessid ();
				
				(**hinfo).fldontownwindow = true;
				
				windowgettitle (w, bstitle);
				
				shellsetwindowtitle (hinfo, bstitle); // 7.24.97 dmb: was windowsettitle
				}
			
			break;
			}
		
		case iowaCloseEvent: {
			hdlcard hcard = (hdlcard) (*ev).message;
			WindowPtr w = getcardrecordwindow (hcard);
			hdlwindowinfo hinfo;
			
			if (!flshellclosingwindow && getwindowinfo (w, &hinfo))
				disposeshellwindowinfo (hinfo);
			
			break;
			}
		
		case iowaButtonHitEvent:
			(*ev).what = iowaRunScriptEvent;
			
			break;
		
		case activateEvt:
			shellwindowmenudirty ();
			
			shellforcemenuadjust ();
			
			break;
		
		default:
			break;
		}
	} /*modelesscardeventfilter*/


static boolean getlinefromhandle (Handle h, unsigned long lnum, bigstring bs) { /*DW 12/31/95*/
	
	unsigned long ix = 0;
	unsigned long ct = gethandlesize (h);
	unsigned long linecount = 0;
	unsigned char ch;
	short lenstring;
	
	while (true) {
		
		if (linecount >= (lnum - 1))
			break;
			
		if (ix >= ct) /*ran out of chars, there aren't that many lines*/
			return (false);
		
		ch = (*h) [ix++];
		
		if (ch == '\r') 
			linecount++;
		} /*while*/
	
	lenstring = 0;
	
	bs [0] = 0;
	
	while (true) {
		
		if (ix >= ct)
			break;
			
		ch = (*h) [ix++];
		
		if (ch == '\r') 
			break;
		
		if (lenstring < 255) {
			
			lenstring++;
			
			bs [0] = lenstring;
			
			bs [lenstring] = ch;
			}
		} /*while*/
	
	return (true);
	} /*getlinefromhandle*/


static boolean getcardparam (hdltreenode hparam1, short pnum, Handle *hcard, bigstring bscard) {

	hdlhashtable htable;
	tyvaluerecord val;
	bigstring line1, line2;
	Handle hpacked;
	hdlhashnode hnode;
	
	if (!getvarvalue (hparam1, pnum, &htable, bscard, &val, &hnode))
		return (false);
	
	if ((val.valuetype != binaryvaluetype) || (getbinarytypeid (val.data.binaryvalue) != 'CARD')) {
		
		langparamerror (notcarderror, bscard);
		
		return (false);
		}
	
	if (!copyhandle (val.data.binaryvalue, hcard))
		return (false);
	
	hpacked = *hcard;
	
	stripbinarytypeid (hpacked);
	
	getlinefromhandle (hpacked, 1, line1);
	
	getlinefromhandle (hpacked, 2, line2);
	
	if (equalstrings (line1, "\pMacBird 1.0 -- 12/31/95")) {
	
		if (equalstrings (line2, "\prun maconly")) {
		
			pullfromhandle (hpacked, 0, stringlength (line1) + stringlength (line2) + 2, nil);
			}
		}
		
	return (true);
	} /*getcardparam*/


boolean langruncard (hdltreenode hparam1, boolean flmodal, tyvaluerecord *vreturned) {
	
	/*
	4.0b7 dmb: use new embedded birdruncard
	*/
	
	Handle hcard;
	bigstring bscard;
	boolean fl;
	
	flnextparamislast = true;
	
	if (!getcardparam (hparam1, 1, &hcard, bscard))
		return (false);
	
	/*
	if (wsGlobals.windowserver == nil) {
		
		if (!uisInit (nil, 0, 'LAND', noMenuSharing)) {
			
			disposehandle (hcard);
			
			langerror (0); //***
			
			return (false);
			}
		}
	
	if (flmodal)
		fl = uisRunModalHandle (hcard, true, bscard, 0, 0, &modalcardeventfilter);
	else
		fl = uisOpenHandle (hcard, true, bscard, 0, 0, nil);
	
	/*
	uisClose ();
	*/
	
	if (cardIsModal (hcard) || !iscurrentapplication (langipcself)) {
		
		shellactivate (); /*4.1.1b1 dmb*/
		
		shellmodaldialogmenuadjust ();
		
		fl = runModalCard (hcard, true, 0, 0, modalcardeventfilter);
		
		shellforcemenuadjust ();
		}
	else
		fl = runModelessCard (hcard, true, 0, 0, modelesscardeventfilter);
	
	disposehandle (hcard);
	
	if (!fl) {
		
		langparamerror (cantruncarderror, bscard);
		
		return (false);
		}
	
	setbooleanvalue (true, vreturned);
	
	return (true);
	} /*langruncard*/


boolean langismodalcard (hdltreenode hparam1, tyvaluerecord *vreturned) {
	
	Handle hcard;
	bigstring bscard;
	
	flnextparamislast = true;
	
	if (!getcardparam (hparam1, 1, &hcard, bscard))
		return (false);
	
	return (setbooleanvalue (cardIsModal (hcard), vreturned));
	} /*langismodalcard*/


boolean langsetmodalcardtimeout (hdltreenode hparam1, tyvaluerecord *vreturned) {
	
	flnextparamislast = true;
	
	if (!getlongvalue (hparam1, 1, (long *) &modalcardtimeout))
		return (false);
	
	return (setbooleanvalue (true, vreturned));
	} /*langismodalcard*/


static boolean langcardgettargetdata (short id) {
	
	return (id == -1); /*true if target type is generic -- a shell verb*/
	} /*langcardgettargetdata*/


static boolean langcardcut (void) {
	
	return (cardEditCommand (cutitem));
	} /*langcardcut*/


static boolean langcardcopy (void) {
	
	return (cardEditCommand (copyitem));
	} /*langcardcopy*/


static boolean langcardpaste (void) {
	
	shellwritescrap (textscraptype);
	
	return (cardEditCommand (pasteitem));
	} /*langcardpaste*/


static boolean langcardclear (void) {
	
	return (cardEditCommand (clearitem));
	} /*langcardclear*/


static boolean langcardselectall (void) {
	
	return (true);
	} /*langcardselectall*/


static boolean langcardclose (void) {
	
	flshellclosingwindow = true;
	
	closeCardWindow (shellwindow);
	
	flshellclosingwindow = false;
	
	return (true);
	} /*langcardclose*/


boolean langcardstart (void) {
	
	/*
	set up callback routines record, and link our data into the shell's 
	data structure.
	
	4.1b4 dmb: set closeroutine so that we call closeCardWindow
	*/
	
	ptrcallbacks langcardcallbacks;
	register ptrcallbacks cb;
	
	if (!iowaInit ())
		return (false);
	
	appletsegment (); /*4.1b7 dmb*/
	
	shellnewcallbacks (&langcardcallbacks);
	
	cb = langcardcallbacks; /*copy into register*/
	
	loadconfigresource (idiowaconfig, &(*cb).config);
	
	(*cb).configresnum = idiowaconfig;
	
	(*cb).windowholder = &langcardwindow;
	
	(*cb).dataholder = (Handle *) &langcarddata;
	
	(*cb).infoholder = &langcardwindowinfo; 
	
	(*cb).gettargetdataroutine = &langcardgettargetdata;
	
	(*cb).cutroutine = &langcardcut;
	
	(*cb).copyroutine = &langcardcopy;
	
	(*cb).pasteroutine = &langcardpaste;
	
	(*cb).clearroutine = &langcardclear;
	
	(*cb).selectallroutine = &langcardselectall;
	
	(*cb).closeroutine = &langcardclose;
	
	/*
	(*cb).activateroutine = (callback) &langcardactivate;
	
	(*cb).updateroutine = &langcardupdate;
	
	(*cb).mouseroutine = &langcardmousedown;
	
	(*cb).keystrokeroutine = &langcardkeystroke;
	
	(*cb).adjustcursorroutine = (callback) &langcardadjustcursor;
	*/
	
	return (true);
	} /*langcardstart*/

#endif




