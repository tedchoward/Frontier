
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
#include "kb.h"
#include "shell.h"


#define keycodeclear 71 /*keycodes for numeric keybad*/
#define keycodeminus 78
#define keycodeplus 69
#define keycodetimes 67
#define keycodeseven 89
#define keycodeeight 91
#define keycodenine 92
#define keycodedivide 77
#define keycodefour 86
#define keycodefive 87
#define keycodesix 88
#define keycodecomma 72
#define keycodeone 83
#define keycodetwo 84
#define keycodethree 85
#define keycodeenter 76
#define keycodezero 82
#define keycodeperiod 65

#define keycodeoption 61
#define keycodecommand 48
#define keycodeshift	63
#define keycodecontrol 60




static boolean keydown (short keycode, boolean flasync) {
	
	/*
	5.0b6 dmb: added flasunc parameter
	*/

#		pragma unused (flasync)
		KeyMap keys;
		
		GetKeys (keys);
		
		return (BitTst (&keys, keycode) != 0);
	} /*keydown*/
	


//////////#if !flruntime


tykeystrokerecord keyboardstatus; 

static boolean flescapepending = false;



boolean iscmdperiodevent (long eventmessage, long eventwhat, long eventmodifiers) {
	/*
	Date: Tue, 17 Sep 1996 14:34:15 +0200
	To: FRONTIER-TALK@DMC.COM
	From: jegues@iol.it (Fabrizio Oddone)
	Reply-To: frontier-talk@DMC.COM
	Subject: Re: International canceling with Cmd-period
	
	You wrote:
	> I wasn't comfortable with the code you sent me. Was that yours, or is it
	> from Apple?
	
	It's almost from Apple.
	That is, it's from "Macintosh Programming Secrets" by Scott Knaster & Keith
	Rollin - the latter guy worked for years in Apple's Mac DTS.
	Page 484-486, FYI.
	
	I slightly improved the original code; it works at least on my Italian
	keyboard, US keyboards, and since one of my betatesters lives in Japan,
	even on a Japanese system.
	I've included this routine in every software I wrote, and I never got a
	complaint from anywhere.
	
	> I think there used to be a technote about the right way to
	> implement this. Do you know which it is?
	
	Yes, I have just checked it out.
	It's TN TE 23: International Canceling.
	Thanks for pointing this out: the note suggests the very same code, but in
	the end it mentions an extra trick not mentioned in the book (and the book
	was written two years after the technote... <g>).
	I have implemented the extra trick (it saves the GetResource() under System
	7.0 or later) as suggested and lo! the updated C code is below (I hope the
	tabs pass through fine). Note: it still works on my Italian kbd! 8-)
	*/
	
	#define	kModifiersMask	(0xFF00 & ~cmdKey)
	
	Handle	hKCHR = nil;
	Ptr		KCHRPtr;
	UInt32	state;
	long	keyInfo;
	UInt16	keyCode;
	Boolean	result = false;
	
	if ((unsigned short)eventwhat == keyDown || (unsigned short)eventwhat == autoKey) {
	
		if ((unsigned short)eventmodifiers & cmdKey) {
		
			keyCode = ((unsigned short)eventmodifiers & kModifiersMask) | ((unsigned short)eventmessage >> 8);
			state = 0;
			KCHRPtr = (Ptr) GetScriptManagerVariable (smKCHRCache);
			
			if (KCHRPtr == nil) {
				
				hKCHR = GetResource('KCHR', GetScriptVariable (GetScriptManagerVariable (smKeyScript), smScriptKeys));
				
				KCHRPtr = *hKCHR;
				}
			
			keyInfo = KCHRPtr ? KeyTranslate(KCHRPtr, keyCode, &state) : eventmessage;
			
			if (hKCHR)
				ReleaseResource (hKCHR);
			
			if (((char)keyInfo == '.') || ((char)(keyInfo >> 16) == '.'))
				result = true;
			}
		}
	
	return (result);

	} /*iscmdperiodevent*/


boolean arrowkey (char chkb) {
	
	/*
	return true if the indicated key character is an arrow key.
	*/
	
	register char ch = chkb;
	
	return (
		(ch == chuparrow) || (ch == chdownarrow) || 
		
		(ch == chleftarrow) || (ch == chrightarrow));
	} /*arrowkey*/


tydirection keystroketodirection (char ch) {
	
	switch (ch) {
		
		case chleftarrow:
			return (left);
			
		case chrightarrow:
			return (right);
			
		case chuparrow:
			return (up);
			
		case chdownarrow:
			return (down);
		} /*switch*/
		
	return (nodirection);
	} /*keystroketodirection*/
	

static void kbsetstatus (long eventmessage, long eventwhat, long eventmodifiers, tykeystrokerecord *kbs) {
	
	/*
	12/11/90 dmb: don't count the caps lock as a modifier in ctmodifiers.
	
	2/21/91 dmb: handle calls for mousedown events.  we just want to 
	record modifier keys
	
	4.1b12 dmb: use new international-aware iscmdperiodevent from Fabrizio Oddone
	*/
	
	register short ct;
	short keycode;
	tykeystrokerecord kbcurrent;
	
	kbcurrent.flshiftkey = (eventmodifiers & shiftKey) != 0;
	
	kbcurrent.flcmdkey = (eventmodifiers & cmdKey) != 0;
	
	kbcurrent.floptionkey = (eventmodifiers & optionKey) != 0;
	
	kbcurrent.flalphalock = (eventmodifiers & alphaLock) != 0;
	
	kbcurrent.flcontrolkey = (eventmodifiers & controlKey) != 0;
	

	

	ct = 0;
	
	if (kbcurrent.flshiftkey) ct++;
	
	if (kbcurrent.flcmdkey) ct++;
	
	if (kbcurrent.floptionkey) ct++;
	
	/*
	if (kbcurrent.flalphalock) ct++;
	*/
	
	if (kbcurrent.flcontrolkey) ct++;
	
	kbcurrent.ctmodifiers = ct;	
	
	if (eventwhat == mouseDown) {
		
		kbcurrent.chkb = chnul;
		
		kbcurrent.keycode = 0;
		
		kbcurrent.flautokey = false;
		
		kbcurrent.keydirection = nodirection;
		}
	else {
		
		kbcurrent.chkb = eventmessage & charCodeMask; /*get the keystroke*/
		
		kbcurrent.flautokey = (eventwhat == autoKey);
		
		kbcurrent.keycode = keycode = (eventmessage & keyCodeMask) >> 8;
	    
		kbcurrent.keydirection = keystroketodirection (kbcurrent.chkb);
		
		if ((kbcurrent.flcmdkey) && iscmdperiodevent (eventmessage, eventwhat, eventmodifiers)) { /*dmb 4.1b12*/
		
			kbcurrent.chkb = '.';
			
			kbcurrent.flshiftkey = false; /*dmb 4.1.1b1: prevent shellfilterfunctionkey menu mapping*/
			
			kbcurrent.ctmodifiers--;
			}

		if (kbcurrent.flcmdkey  &&  kbcurrent.floptionkey) {
	
			/*
			we don't want to option character, so find the normal character from 
			the keymap.  see IM V-195
			*/
			
			Handle hkchr;
			unsigned long state = 0;
			
			if ((hkchr = GetResource ('KCHR', 0)) != nil)
				kbcurrent.chkb = KeyTranslate (*hkchr, kbcurrent.keycode, &state) & 0x000000ff;
			};

		
		kbcurrent.flkeypad = /*true if it is a keystroke from the numeric keypad*/
			
			(keycode == keycodeclear) 		|| (keycode == keycodeminus) 	|| 
			
			(keycode == keycodeplus) 		|| (keycode == keycodetimes) 	||
			
			(keycode == keycodeseven) 		|| (keycode == keycodeeight) 	||
			
			(keycode == keycodenine) 		|| (keycode == keycodedivide) 	||
			
			(keycode == keycodefour) 		|| (keycode == keycodefive) 	||
			
			(keycode == keycodesix) 		|| (keycode == keycodecomma) 	||
			
			(keycode == keycodeone) 		|| (keycode == keycodetwo) 		||
			
			(keycode == keycodethree) 		|| (keycode == keycodeenter) 	||
			
			(keycode == keycodezero) 		|| (keycode == keycodeperiod);
		}
	
	*kbs = kbcurrent; /*set for caller*/
	} /*kbsetstatus*/
	

void setkeyboardstatus (long eventmessage, long eventwhat, long eventmodifiers) {
	
	/*
	sets a global that may be referenced by anyone.
	
	should be called every time a new event is received.
	*/
	
	kbsetstatus (eventmessage, eventwhat, eventmodifiers, &keyboardstatus); /*use global*/
	} /*setkeyboardstatus*/


void keyboardclearescape (void) {
	
	flescapepending = false;
	} /*keyboardclearescape*/


void keyboardsetescape (void) {
	
	/*ouch (); %*audible feedback asap, multimedia!*/
	
	/*
	shellfrontrootwindowmessage ("\pCancelled.");
	*/
	
	flescapepending = true;
	} /*keyboardsetescape*/


boolean keyboardescape (void) {
	
	/*
	check to see if the user has pressed cmd-period.  if not, no effect on
	the event queue and we return false.
	
	otherwise, we remove the keystroke and return true.  the caller is expected
	to return quickly!
	
	10/29/91 dmb: treat Escape key the same as cmd-period.
	*/
	
	register unsigned long tc;
	tykeystrokerecord kbcurrent;
	EventRecord ev;

	static unsigned long lastcheck = 0;
	
	if (flescapepending)
		return (true);
	
	tc = gettickcount ();
	
	if ((tc - 60) < lastcheck) /*check 1 time per second*/
		return (false);
	
	lastcheck = tc; /*remember for next time*/

	if (EventAvail (keyDownMask, &ev)) {
		
		kbsetstatus (ev.message, ev.what, ev.modifiers, &kbcurrent);
		
		if ((kbcurrent.chkb == chescape) || (kbcurrent.flcmdkey && (kbcurrent.chkb == '.'))) {
			
			GetNextEvent (keyDownMask, &ev); /*get rid of the cmd-period*/
			
			keyboardsetescape (); /*multimedia!*/
			
			return (true);
			}
		}
	
	/*motorsound ();*/
	
	return (false);
	} /*keyboardescape*/
	
////#endif

/*
boolean enterkeydown (void) {	
	
	return (keydown (75));
	} %*enterkeydown*/
	
	
boolean optionkeydown (void) {
	
	return (keydown (keycodeoption, true));
	} /*optionkeydown*/
	
	
boolean cmdkeydown (void) {
		
	return (keydown (keycodecommand, true));
	} /*cmdkeydown*/
	
	
boolean shiftkeydown (void) {
		
	return (keydown (keycodeshift, true));
	} /*shiftkeydown*/

boolean controlkeydown (void) {
		
	return (keydown (keycodecontrol, true));
	} /*shiftkeydown*/



void keyboardpeek (tykeystrokerecord *kbrecord) {
	
	/*
	10/29/91 dmb: set ctmodifiers too
	*/
	
	register ptrkeystrokerecord p = kbrecord;
	register short ct = 0;
	
	clearbytes (p, longsizeof (tykeystrokerecord));
		
	if (shiftkeydown()) {
		
		(*p).flshiftkey = true;
		
		ct = 1;
		}
	
	if (cmdkeydown()) {
		
		(*p).flcmdkey = true;
		
		++ct;
		}
	
	if (optionkeydown()) {
		
		(*p).floptionkey = true;
		
		++ct;
		}
	
	if (controlkeydown()) {
		
		(*p).flcontrolkey = true;
		
		++ct;
		}
	
	(*p).ctmodifiers = ct;
	} /*keyboardpeek*/


short getkeyboardstartrepeattime (void) {

	
		return (LMGetKeyThresh ());
	
	
	} /*getkeyboardstartrepeattime*/




