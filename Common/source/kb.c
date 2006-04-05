
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


#ifdef MACVERSION
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
#endif

#ifdef WIN95VERSION
 /*keycodes for numeric keybad*/
#define keycodeclear -1
#define keycodeminus 0x4a
#define keycodeplus 0x4e
#define keycodedivide 0x135
#define keycodetimes 0x37
#define keycodeseven 0x47
#define keycodeeight 0x48
#define keycodenine 0x49
#define keycodefour 0x4b
#define keycodefive 0x4c
#define keycodesix 0x4d
#define keycodecomma -1
#define keycodeone 0x4f
#define keycodetwo 0x50
#define keycodethree 0x51
#define keycodeenter 0x11c
#define keycodezero 0x52
#define keycodeperiod 0x53

//#define keycodeoption 61
//#define keycodecommand VK_MENU
//#define keycodeshift	VK_SHIFT
//#define keycodecontrol VK_CONTROL
#define keycodeoption VK_MENU
#define keycodecommand VK_CONTROL
#define keycodeshift	VK_SHIFT
#define keycodecontrol 60
#endif



static boolean keydown (short keycode, boolean flasync) {
	
	/*
	5.0b6 dmb: added flasunc parameter
	*/

	#ifdef MACVERSION
#		pragma unused (flasync)
		KeyMap keys;
		
		GetKeys (keys);
		
		return (BitTst (&keys, keycode) != 0);
	#endif
	#ifdef WIN95VERSION
		if (flasync)
			return ((GetAsyncKeyState (keycode) & 0x8000) == 0x8000);
		else
			return ((GetKeyState (keycode) & 0x8000) == 0x8000);
	#endif
	} /*keydown*/
	


//////////#if !flruntime


tykeystrokerecord keyboardstatus; 

static boolean flescapepending = false;



boolean iscmdperiodevent (long eventmessage, long eventwhat, long eventmodifiers) {
#ifdef MACVERSION
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
#endif

#ifdef WIN95VERSION
	/* For the Windows version, let us look for ALT-. and CTRL-C */
	if ((eventwhat == 'C') || (eventwhat == 'c')) {
		if (GetAsyncKeyState(VK_CONTROL) & 0x8000) {
			return (true);
			}
		}

	if ((eventwhat == '.') || (eventwhat == VK_DECIMAL)) {
		if (GetAsyncKeyState(VK_MENU) & 0x8000) {
			return (true);
			}
		}

	return (false);

#endif
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
	
#ifdef MACVERSION
	kbcurrent.flshiftkey = (eventmodifiers & shiftKey) != 0;
	
	kbcurrent.flcmdkey = (eventmodifiers & cmdKey) != 0;
	
	kbcurrent.floptionkey = (eventmodifiers & optionKey) != 0;
	
	kbcurrent.flalphalock = (eventmodifiers & alphaLock) != 0;
	
	kbcurrent.flcontrolkey = (eventmodifiers & controlKey) != 0;
#endif
	

#ifdef WIN95VERSION
	kbcurrent.flshiftkey = keydown(keycodeshift, false);
	
	kbcurrent.flcmdkey = keydown(keycodecommand, false);
	
	kbcurrent.floptionkey = keydown(keycodeoption, false);
	
	kbcurrent.flalphalock = (GetKeyState (VK_CAPITAL) & 0x0001);  /*Use toggle state*/
	
	kbcurrent.flcontrolkey = keydown(keycodecontrol, false);
#endif
	

	ct = 0;
	
	if (kbcurrent.flshiftkey) ct++;
	
	if (kbcurrent.flcmdkey) ct++;
	
	if (kbcurrent.floptionkey) ct++;
	
	/*
	if (kbcurrent.flalphalock) ct++;
	*/
	
	if (kbcurrent.flcontrolkey) ct++;
	
	kbcurrent.ctmodifiers = ct;	
	
#ifdef MACVERSION
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
#endif

#ifdef WIN95VERSION
	if ((eventmessage == WM_LBUTTONDOWN) || (eventmessage == WM_RBUTTONDOWN)) {
		
		kbcurrent.chkb = chnul;
		
		kbcurrent.keycode = 0;
		
		kbcurrent.flautokey = false;
		
		kbcurrent.keydirection = nodirection;
		}
	else {
		
		kbcurrent.chkb = eventmessage;
		
		kbcurrent.flautokey = ((eventmodifiers & 0x40000000) == 0x40000000);
		
		kbcurrent.keycode = keycode = (eventmodifiers & 0x01FF0000) >> 16;
	    
		kbcurrent.keydirection = keystroketodirection (kbcurrent.chkb);

		if (keycode == keycodeenter)
			kbcurrent.chkb = chenter;
		
//		if ((kbcurrent.flcmdkey) && iscmdperiodevent (eventmessage, eventwhat, eventmodifiers)) { /*dmb 4.1b12*/
//		
//			kbcurrent.chkb = '.';
//			
//			kbcurrent.flshiftkey = false; /*dmb 4.1.1b1: prevent shellfilterfunctionkey menu mapping*/
//			
//			kbcurrent.ctmodifiers--;
//			}

#endif
		
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
#ifdef MACVERSION
	tykeystrokerecord kbcurrent;
	EventRecord ev;
#endif
#ifdef WIN95VERSION
	MSG msg;
#endif

	static unsigned long lastcheck = 0;
	
	if (flescapepending)
		return (true);
	
	tc = gettickcount ();
	
	if ((tc - 60) < lastcheck) /*check 1 time per second*/
		return (false);
	
	lastcheck = tc; /*remember for next time*/

#ifdef MACVERSION
	if (EventAvail (keyDownMask, &ev)) {
		
		kbsetstatus (ev.message, ev.what, ev.modifiers, &kbcurrent);
		
		if ((kbcurrent.chkb == chescape) || (kbcurrent.flcmdkey && (kbcurrent.chkb == '.'))) {
			
			GetNextEvent (keyDownMask, &ev); /*get rid of the cmd-period*/
			
			keyboardsetescape (); /*multimedia!*/
			
			return (true);
			}
		}
#endif
#ifdef WIN95VERSION
	if (PeekMessage(&msg, NULL, WM_KEYDOWN, WM_KEYDOWN, PM_NOREMOVE)) {
		if ((msg.wParam == VK_ESCAPE) || iscmdperiodevent (msg.message, msg.wParam, 0)) {
			PeekMessage(&msg, NULL, WM_KEYDOWN, WM_KEYDOWN, PM_REMOVE); /*get rid of command*/
			
			keyboardsetescape (); /*multimedia!*/
			
			return (true);
			}
		}
#endif
	
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

	#ifdef MACVERSION
	
		return (LMGetKeyThresh ());
	
	#endif
	
	#ifdef WIN95VERSION
	
		return (20);	// dmb to rab: what should this be?
	
	#endif
	} /*getkeyboardstartrepeattime*/




