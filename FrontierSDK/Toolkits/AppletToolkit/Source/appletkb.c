
/*	$Id$    */

/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include "appletmemory.h"
#include "appletcursor.h"
#include "appletkb.h"



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


tykeystrokerecord keyboardstatus; 

static boolean flescapepending = false;




boolean arrowkey (char chkb) {
	
	/*
	return true if the indicated key character is an arrow key.
	*/
	
	char ch = chkb;
	
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
	

static void kbsetstatus (EventRecord ev, tykeystrokerecord *kbs) {
	
	/*
	12/11/90 dmb: don't count the caps lock as a modifier in ctmodifiers.
	
	2/21/91 dmb: handle calls for mousedown events.  we just want to 
	record modifier keys
	*/
	
	short ct;
	char chkb;
	short keycode;
	tykeystrokerecord kbcurrent;
	
	kbcurrent.flshiftkey = (ev.modifiers & shiftKey) != 0;
	
	kbcurrent.flcmdkey = (ev.modifiers & cmdKey) != 0;
	
	kbcurrent.floptionkey = (ev.modifiers & optionKey) != 0;
	
	kbcurrent.flalphalock = (ev.modifiers & alphaLock) != 0;
	
	kbcurrent.flcontrolkey = (ev.modifiers & controlKey) != 0;
	
	ct = 0;
	
	if (kbcurrent.flshiftkey) ct++;
	
	if (kbcurrent.flcmdkey) ct++;
	
	if (kbcurrent.floptionkey) ct++;
	
	/*
	if (kbcurrent.flalphalock) ct++;
	*/
	
	if (kbcurrent.flcontrolkey) ct++;
	
	kbcurrent.ctmodifiers = ct;	
	
	if (ev.what == mouseDown) {
		
		kbcurrent.chkb = chnul;
		
		kbcurrent.keycode = 0;
		
		kbcurrent.flautokey = false;
		
		kbcurrent.keydirection = nodirection;
		}
	else {
		
		kbcurrent.chkb = chkb = ev.message & charCodeMask; /*get the keystroke*/
		
		kbcurrent.flautokey = (ev.what == autoKey);
		
		kbcurrent.keycode = keycode = (ev.message & keyCodeMask) >> 8;
	    
		kbcurrent.keydirection = keystroketodirection (chkb);
		
		if (kbcurrent.flcmdkey  &&  kbcurrent.floptionkey) {
	
			/*
			we don't want to option character, so find the normal character from 
			the keymap.  see IM V-195
			*/
			
			Handle hkchr;
			unsigned long state = 0;
			
			if ((hkchr = GetResource ('KCHR', 0)) != NULL)
				kbcurrent.chkb = KeyTrans (*hkchr, kbcurrent.keycode, &state) & 0x000000ff;
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
	

void setkeyboardstatus (EventRecord ev) {
	
	/*
	sets a global that may be referenced by anyone.
	
	should be called every time a new event is received.
	*/
	
	kbsetstatus (ev, &keyboardstatus); /*use global*/
	} /*setkeyboardstatus*/


void keyboardclearescape (void) {
	
	flescapepending = false;
	} /*keyboardclearescape*/


boolean keyboardescape (void) {
	
	/*
	check to see if the user has pressed cmd-period.  if not, no effect on
	the event queue and we return false.
	
	otherwise, we remove the keystroke and return true.  the caller is expected
	to return quickly!
	
	DW 8/18/93: also handle Esc key.
	
	DW 8/30/93: the manual can say that you can press cmd-period or Esc 
	whenever you see the beachball rolling.
	
	DW 9/16/93: check once per second.
	*/
	
	if (flescapepending)
		return (true);
		
	rollbeachball (); /*DW 8/30/93*/
		
	/*return false if it's been less than 1 second since we checked*/ {
		
		static unsigned long tclastcheck = 0;
		unsigned long tc;
		
		tc = TickCount ();
		
		if ((tc - tclastcheck) < 60)
			return (false);
		
		tclastcheck = tc; /*... and fall thru*/
		}
	
	/*check the event queue for an Esc or cmd-period*/ {
	
		EventRecord ev;
		
		if (EventAvail (keyDownMask, &ev)) {
			
			tykeystrokerecord kbcurrent;
	
			kbsetstatus (ev, &kbcurrent);
			
			if (kbcurrent.chkb == chescape) /*Esc key*/
				flescapepending = true;
				
			if (kbcurrent.flcmdkey && (kbcurrent.chkb == '.'))
				flescapepending = true;
				
			if (flescapepending) 		
				GetNextEvent (keyDownMask, &ev); /*get rid of the Esc keystroke*/
			} 
		}
	
	return (flescapepending);
	} /*keyboardescape*/


void keyboardpeek (tykeystrokerecord *kbrecord) {
	
	ptrkeystrokerecord p = kbrecord;
	KeyMap keys;
	
	clearbytes (p, longsizeof (tykeystrokerecord));
	
	GetKeys (keys);
	
	(*p).flshiftkey = BitTst (&keys, 63);
		
	(*p).flcmdkey = BitTst (&keys, 48);
		
	(*p).floptionkey = BitTst (&keys, 61);
		
	(*p).flcontrolkey = BitTst (&keys, 60);
	} /*keyboardpeek*/


static boolean keydown (short keycode) {

	KeyMap keys;
	
	GetKeys (keys);
	
	return (BitTst (&keys, keycode));
	} /*keydown*/
	
	
boolean enterkeydown (void) {	
	
	return (keydown (75));
	} /*enterkeydown*/
	
	
boolean optionkeydown (void) {
	
	return (keydown (61));
	} /*optionkeydown*/
	
	
boolean cmdkeydown (void) {
		
	return (keydown (48));
	} /*cmdkeydown*/
	
	
boolean shiftkeydown (void) {
		
	return (keydown (63));
	} /*shiftkeydown*/
	
	
boolean returnkeydown (void) {
	
	/*
	short i; 
	bigstring bs;
	
	for (i = 1; i <= 200; i++) {
	
		if (keydown (i)) {
		
			NumToString (i, bs);
			
			DebugStr (bs);
			}
		}
	*/
		
	return (keydown (35)); 
	} /*returnkeydown*/
	
	
	
