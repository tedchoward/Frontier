
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

#include "quickdraw.h"
#include "bitmaps.h"
#include "cursor.h"
#include "font.h"
#include "launch.h"
#include "kb.h"
#include "memory.h"
#include "resources.h"
#include "scrap.h"
#include "strings.h"
#include "frontierwindows.h"
#include "ops.h"
#include "shell.h"
#include "shellmenu.h"
#include "shell.rsrc.h"
#include "dialogs.h"
#include "lang.h"
#include "mac.h"

#ifdef flcomponent

	#include <SetUpA5.h>

#endif


#define windowevents (updateMask + activMask)


short dialogcountitems (DialogPtr pdialog) {
	
	//Code change by Timothy Paustian Sunday, April 30, 2000 9:40:09 PM
	//Changed to Opaque call for Carbon
	//switched to use CountDITL. This has been around since OS 7
	return CountDITL(pdialog);
	} /*dialogcountitems*/


static void dialoggeteditbuffer (DialogPtr pdialog, TEHandle *hbuffer) {
	
	//Code change by Timothy Paustian Sunday, April 30, 2000 9:43:36 PM
	//Changed to Opaque call for Carbon
	//check this I need to check to see if this is a memory leak or not. 
	#if ACCESSOR_CALLS_ARE_FUNCTIONS == 1
		//DialogItemType  	itemType = 0;
		//Handle 				h;
		//Rect 				r = {0,0,0,0};
		//SInt16				whichField = 0;

		*hbuffer = GetDialogTextEditHandle((DialogRef) pdialog);
	
	#else
		//old code
		*hbuffer = (*(DialogPeek) pdialog).textH;
	#endif
	} /*dialoggeteditbuffer*/


void boldenbutton (DialogPtr pdialog, short itemnumber) {

	/*
	draw a thick black ring around the OK button in the dialog.  
	*/
	
	PenState savePen;
	short itemtype;
	Handle itemhandle;
	Rect itemrect;
	
	//Code change by Timothy Paustian Wednesday, July 12, 2000 12:39:52 PM
	//pushport was doing an implicit cast from a dialogPtr to a GrafPtr
	//This is allowed in MacOS, but not in carbon. This was causing a crash.
	CGrafPtr	thePort;
	#if TARGET_API_MAC_CARBON == 1
	thePort = GetDialogPort(pdialog);
	#else
	thePort = (CGrafPtr)pdialog;
	#endif
		
	pushport (thePort);
	
	GetPenState (&savePen); /*save the old pen state*/
	
	GetDialogItem (pdialog, itemnumber, &itemtype, &itemhandle, &itemrect); /*get the item's rect*/
	
	insetrect (&itemrect, -4, -4);
	
	PenSize (3, 3); /*make the pen fatter*/
	
	FrameRoundRect (&itemrect, 16, 16); /*draw the ring*/

	SetPenState (&savePen); /*restore the pen state*/
	
	popport ();
	} /*boldenbutton*/


void positiondialogwindow (DialogPtr pdialog) {

	register short h, v;
	Rect rdialog, rscreen;
	CGrafPtr	thePort;
	
	getcurrentscreenbounds (&rscreen);
	//Code change by Timothy Paustian Sunday, April 30, 2000 9:44:58 PM
	//Changed to Opaque call for Carbon
	#if ACCESSOR_CALLS_ARE_FUNCTIONS == 1
	thePort = GetDialogPort(pdialog);
	GetPortBounds(thePort, &rdialog);
	#else
	//old code
	#pragma unused(thePort)
	rdialog = (*pdialog).portRect;
	#endif
	
	h = rscreen.left + (((rscreen.right - rscreen.left) - (rdialog.right - rdialog.left)) / 2);
	
	v = rscreen.top + (((rscreen.bottom - rscreen.top) - (rdialog.bottom - rdialog.top)) / 3);
	
	#if TARGET_API_MAC_CARBON == 1
	{
	WindowPtr theWind = GetDialogWindow(pdialog);
	movewindow(theWind, h, v);
	}
	#else
	movewindow (pdialog, h, v);
	#endif
	} /*positiondialogwindow*/


static boolean dialogitemtypeiscontrol (short itemtype) {
	
	register short x;
	
	x = itemtype % itemDisable; /*ignore enabledness*/
	
	return ((x >= ctrlItem) && (x <= (ctrlItem + resCtrl))); 
	} /*dialogitemtypeiscontrol*/


void disabledialogitem (DialogPtr pdialog, short itemnumber) {
	
	/*
	3/6/91 dmb: also dim if the item is a control
	*/
	
	short itemtype;
	Handle itemhandle;
	Rect itemrect;
	
	GetDialogItem (pdialog, itemnumber, &itemtype, &itemhandle, &itemrect);
	
	if (itemtype < itemDisable) { /*it is enabled, disable it*/
		
		SetDialogItem (pdialog, itemnumber, itemtype + itemDisable, itemhandle, &itemrect);
		
		if (dialogitemtypeiscontrol (itemtype))
			HiliteControl ((ControlHandle) itemhandle, 255);
		}
	} /*disabledialogitem*/


void enabledialogitem (DialogPtr pdialog, short itemnumber) {
	
	short itemtype;
	Handle itemhandle;
	Rect itemrect;
	
	GetDialogItem (pdialog, itemnumber, &itemtype, &itemhandle, &itemrect);
	
	if (itemtype >= itemDisable) { /*it is disabled, enable it*/
		
		SetDialogItem (pdialog, itemnumber, itemtype - itemDisable, itemhandle, &itemrect);
		
		if (dialogitemtypeiscontrol (itemtype))
			HiliteControl ((ControlHandle) itemhandle, 0);
		}
	} /*enabledialogitem*/


void hidedialogitem (DialogPtr pdialog, short itemnumber) {
	
	HideDialogItem (pdialog, itemnumber);
	} /*hidedialogitem*/
	

void showdialogitem (DialogPtr pdialog, short itemnumber) {
	
	ShowDialogItem (pdialog, itemnumber);
	} /*showdialogitem*/
	

void setdefaultitem (DialogPtr pdialog, short defaultitem) {
	
	//Code change by Timothy Paustian Sunday, April 30, 2000 9:47:39 PM
	//Changed to Opaque call for Carbon
	SetDialogDefaultItem(pdialog, defaultitem);
	//old code
	//(*(DialogPeek) pdialog).aDefItem = defaultitem; /*filter will bolden this*/
	} /*setdefaultitem*/


static boolean dialogitemisenabled (DialogPtr pdialog, short item) {
	
	short itemtype;
	Handle itemhandle;
	Rect itemrect;
	
	GetDialogItem (pdialog, item, &itemtype, &itemhandle, &itemrect);
	
	return ((itemtype & itemDisable) == 0);
	} /*dialogitemisenabled*/


boolean dialogitemisbutton (DialogPtr pdialog, short item) {
	
	short itemtype;
	Handle itemhandle;
	Rect itemrect;
	
	if (item <= 0)
		return (false);
	
	GetDialogItem (pdialog, item, &itemtype, &itemhandle, &itemrect);
	
	return (dialogitemtypeiscontrol (itemtype));
	} /*dialogitemisbutton*/
	

static boolean dialogitemisedittext (DialogPtr pdialog, short item) {
	
	short itemtype;
	Handle itemhandle;
	Rect itemrect;
	
	GetDialogItem (pdialog, item, &itemtype, &itemhandle, &itemrect);
	
	return (itemtype & editText); 
	} /*dialogitemisedittext*/
	

static boolean dialoghasedititems (DialogPtr pdialog) {
	
	register short i;
	register short ctitems;
	
	ctitems = dialogcountitems (pdialog);
	
	for (i = 1; i <= ctitems; i++) 
		if (dialogitemisedittext (pdialog, i))
			return (true);
	
	return (false);
	} /*dialoghasedititems*/


static void dialoggetbuttonstring (DialogPtr pdialog, short item, bigstring bs) {
	
	short itemtype;
	Handle itemhandle;
	Rect itemrect;
	//Code change by Timothy Paustian Sunday, April 30, 2000 9:57:08 PM
	//Changed to Opaque call for Carbon
	ControlRef	theControl;
	Str255		controlTitle;
	
	GetDialogItem (pdialog, item, &itemtype, &itemhandle, &itemrect);
	//Code change by Timothy Paustian Sunday, April 30, 2000 9:57:41 PM
	//Changed to Opaque call for Carbon
	//watch this one. What happens if this is a bad cast to a ControlRef?
	theControl = (ControlRef) itemhandle;
	GetControlTitle(theControl, controlTitle);
	copystring (controlTitle, bs);
	//old code
	//copystring ((**(ControlRef) itemhandle).contrlTitle, bs);
	} /*dialoggetbuttonstring*/


static void dialogsetbuttonstring (DialogPtr pdialog, short item, bigstring bs) {
	
	short itemtype;
	Handle itemhandle;
	Rect itemrect;

	GetDialogItem (pdialog, item, &itemtype, &itemhandle, &itemrect);
	
	SetControlTitle ((ControlHandle) itemhandle, bs);
	} /*dialogsetbuttonstring*/


DialogPtr newmodaldialog (short id, short defaultitem) {
	
	/*
	2/7/92 dmb: if dialoghasedititems, force shell to export scrap
	
	2/10/92 dmb: added call to new shellactivate
	
	3.0.1b1 dmb: don't call shellactivate if the dialog isn't of a modal 
	design, i.e. a dBoxProc window. currently, the only case where 
	this isn't so is Frontier's about window / splash screen.  having 
	the splash screen force Frontier to the front is bad news for 
	background launching. it even crashes! 
	*/
	
	register DialogPtr pdialog;
	long appA5;
	
	#ifdef fldebug
	
	if (GetResource ('DLOG', id) == nil)
		DebugStr ("\pmissing DLOG resource");
	
	#endif
	
	#ifdef flcomponent
	
	appA5 = SetUpCurA5 ();
	
	#endif
	
	pdialog = GetNewDialog (id, nil, (WindowRef) -1L);
	
	#ifdef flcomponent
	
	RestoreA5 (appA5);
	
	#endif
	
	if (pdialog == nil) 
		return (nil);
	
	positiondialogwindow (pdialog);
	
	setdefaultitem (pdialog, defaultitem);
	
	if (dialoghasedititems (pdialog))
		shellwritescrap (textscraptype);
	
	#if TARGET_API_MAC_CARBON == 1
	{
	WindowPtr	theWind = GetDialogWindow(pdialog);
	if (GetWVariant (theWind) == dBoxProc) /*make sure we're in front before posting modal dialog*/
		shellactivate ();
	}
	#else
	if (GetWVariant (pdialog) == dBoxProc) /*make sure we're in front before posting modal dialog*/
		shellactivate ();
	#endif
	return (pdialog);
	} /*newmodaldialog*/


void disposemodaldialog (DialogPtr pdialog) {
	
	/*
	7/20/92 dmb: don't do partial event loop for runtime.
	
	1/21/93 dmb: use langpartialeventloop when a script is running
	
	2.1b dmb: if this was an out-of-memory alert, the partial event loop is 
	bad news. it's always tende to cause problems, so and shouldn't really 
	be necessary. so let's just not do it!
	*/
	
	DisposeDialog (pdialog);
	
	#if 0 //ndef flruntime
	
		/*handle all pending activate & update events*/
		
		if (flscriptrunning)
			langpartialeventloop (windowevents);
		else
			shellpartialeventloop (windowevents);
	
	#endif
	} /*disposemodaldialog*/


void setdialogcheckbox (DialogPtr pdialog, short item, boolean fl) {

	/*
	change the value of the checkbox.
	*/

	short itemtype;
	Rect itemrect;
	Handle itemhandle;
	
	GetDialogItem (pdialog, item, &itemtype, &itemhandle, &itemrect);
		
	SetControlValue ((ControlHandle) itemhandle, fl);
	} /*setdialogcheckbox*/


boolean getdialogcheckbox (DialogPtr pdialog, short item) {

	/*
	get the value of the checkbox, either a 1 or a 0.
	*/

	short itemtype;
	Rect itemrect;
	Handle itemhandle;
	
	GetDialogItem (pdialog, item, &itemtype, &itemhandle, &itemrect);
	
	return (bitboolean (GetControlValue ((ControlHandle) itemhandle)));
	} /*getdialogcheckbox*/


void toggledialogcheckbox (DialogPtr pdialog, short item) {

	/*
	if the checkbox is on, turn it off, and vice versa.
	*/

	short itemtype;
	Rect itemrect;
	Handle itemhandle;
	
	GetDialogItem (pdialog, item, &itemtype, &itemhandle, &itemrect);
	
	SetControlValue ((ControlHandle) itemhandle, !GetControlValue ((ControlHandle) itemhandle));
	} /*toggledialogcheckbox*/


boolean setdialogradiovalue (DialogPtr pdialog, short firstitem, short lastitem, short val) {

	/*
	set the value of the radio button range.  val is zero-based; valid 
	values range from zero to (firstitem - lastitem)
	*/
	
	register short item;
	register boolean flon;
	
	for (item = firstitem; item <= lastitem; ++item) {
		
		flon = val == (item - firstitem);
		
		if (flon != getdialogcheckbox (pdialog, item))
			setdialogcheckbox (pdialog, item, flon);
		}
	
	return (true);
	} /*getdialogradiovalue*/


short getdialogradiovalue (DialogPtr pdialog, short firstitem, short lastitem) {
	
	/*
	set the value of the radio button range.  val is zero-based; returned 
	value will range from zero to (firstitem - lastitem).
	
	if none of the radio buttons are set, returns -1.
	*/
	
	register short item;
	
	for (item = firstitem; item <= lastitem; ++item) {
		
		if (getdialogcheckbox (pdialog, item))
			return (item - firstitem);
		}
	
	return (-1);
	} /*getdialogradiovalue*/


void getdialogtext (DialogPtr pdialog, short itemnumber, bigstring bs) {
	
	short itemtype;
	Handle itemhandle;
	Rect itemrect;
	
	GetDialogItem (pdialog, itemnumber, &itemtype, &itemhandle, &itemrect);
	GetDialogItemText (itemhandle, bs);
	
	} /*getdialogtext*/


static void dialogscanspecialchars (bigstring bs) {

	register short i, ct;
	
	ct = stringlength (bs);
	
	for (i = 1; i <= ct; i++) {
		
		register char ch = bs [i];
		
		if (ch == '¨') /*option-R maps to the return char*/
			ch = chreturn;
			
		bs [i] = ch;
		} /*for*/
	} /*dialogscanspecialchars*/
	
#if !flruntime

void setdialogicon (DialogPtr pdialog, short itemnumber, short iconnum) {
	
	Handle hicon;
	short itemtype;
	Handle itemhandle;
	Rect itemrect;

	hicon = GetIcon (iconnum); 
	
	if (hicon != nil) {
	
		GetDialogItem (pdialog, itemnumber, &itemtype, &itemhandle, &itemrect);
	
		SetDialogItem (pdialog, itemnumber, itemtype, hicon, &itemrect);
		}
	} /*setdialogicon*/
	
#endif

void setdialogtext (DialogPtr pdialog, short itemnumber, bigstring bs) {
	
	/*
	6/12/92 dmb: if the item is the active text item, select the new text
	*/
	
	OSStatus err = noErr; 	
	short itemtype;
	Handle itemhandle;
	Rect itemrect;
	char	s[256];
	ControlRef	C;
	dialogscanspecialchars (bs);
	
	
	GetDialogItem (pdialog, itemnumber, &itemtype, &itemhandle, &itemrect);
	SetDialogItemText (itemhandle, bs); 
err = GetDialogItemAsControl (pdialog,itemnumber,&C);
if (err==noErr) {
		X0_p2cstrcpy(s,bs);
	err = SetControlData (C,kControlEditTextPart,kControlEditTextTextTag,strlen(s),s);
	}

	
	//Code change by Timothy Paustian Sunday, April 30, 2000 10:02:10 PM
	//Changed to Opaque call for Carbon
	//change to use a new routine that finds the item in focus

	if (itemnumber == GetDialogKeyboardFocusItem(pdialog))
		dialogselectall (pdialog);


	//old code
	//if (itemnumber == (*(DialogPeek) pdialog).editField + 1) /*6/12/92*/
	//	dialogselectall (pdialog);
	
	/*closebitmap (pdialog);*/
	} /*setdialogtext*/


void selectdialogtext (DialogPtr pdialog, short itemnumber) {
	OSStatus err = noErr;
	ControlRef	C;
	ControlEditTextSelectionRec    selection = { 0, 32000 };
//	SelectDialogItemText (pdialog, itemnumber, 0, infinity); /*select all text*/
	
	err = GetDialogItemAsControl (pdialog,itemnumber,&C);
	if (err==noErr) {
		err = SetControlData(C, kControlEntireControl,kControlEditTextSelectionTag, sizeof(ControlEditTextSelectionRec),&selection);
	} else SelectDialogItemText (pdialog, itemnumber, 0, infinity); /*select all text*/
	
}



short getdialogint (DialogPtr pdialog, short itemnumber) {
	
	/*
	get an integer from an edittext item in the dialog.
	*/
	
	bigstring bs;
	long num;
	
	getdialogtext (pdialog, itemnumber, bs);
	
	stringtonumber (bs, &num);
	
	return ((short) num);
	} /*getdialogint*/
	
	
void setdialogint (DialogPtr pdialog, short itemnumber, short num) {
	
	bigstring bs;
	
	NumToString (num, bs);
	
	setdialogtext (pdialog, itemnumber, bs);
	} /*setdialogint*/
	
	
OSType getdialogostype (DialogPtr pdialog, short itemnumber) {
	
	/*
	get an OSType from an edittext item.
	*/
	
	bigstring bs;
	OSType type;
	
	getdialogtext (pdialog, itemnumber, bs);
	
	stringtoostype (bs, &type);
	
	return (type);
	} /*getdialogostype*/
	
	
void setdialogostype (DialogPtr pdialog, short itemnumber, OSType type) {
	
	bigstring bs;
	
	ostypetostring (type, bs);
	
	setdialogtext (pdialog, itemnumber, bs);
	} /*setdialogostype*/
	
	
void setdialogbutton (DialogPtr pdialog, short itemnumber, bigstring bs) {
	
	/*
	6/11/93 dmb: don't use bitmaps; doesn't work on SF buttons
	*/
	
	short itemtype;
	Handle itemhandle;
	Rect r;
	
	GetDialogItem (pdialog, itemnumber, &itemtype, &itemhandle, &r);
	
	/*
	openbitmapcopy (r, pdialog);
	*/
	
	SetControlTitle ((ControlHandle) itemhandle, bs);
	
	/*
	closebitmap (pdialog);
	
	validrect (r);
	*/
	} /*setdialogbutton*/
	

void dialoggetobjectrect (DialogPtr pdialog, short objectnumber, Rect *r) {

	short itemtype;
	Handle itemhandle;
	
	GetDialogItem (pdialog, objectnumber, &itemtype, &itemhandle, r);
	} /*dialoggetobjectrect*/
	
	
void dialogsetobjectrect (DialogPtr pdialog, short objectnumber, Rect r) {

	short itemtype;
	Handle itemhandle;
	Rect itemrect;
	
	GetDialogItem (pdialog, objectnumber, &itemtype, &itemhandle, &itemrect);
	
	SetDialogItem (pdialog, objectnumber, itemtype, itemhandle, &r);
	
	if (itemtype & editText) { /*it's an edittext item, set its rects and wordwrap*/
		
		TEHandle hbuffer;
		
		dialoggeteditbuffer (pdialog, &hbuffer);
		
		(**hbuffer).viewRect = r;
		
		(**hbuffer).destRect = r;
		
		TECalText (hbuffer);
		}
	} /*dialogsetobjectrect*/
	
#if !flruntime

boolean dialogsetfontsize (DialogPtr pdialog, short font, short size) {
	
	/*
	2.1b9 dmb: including leading in line height calculation
	*/
	
	TEHandle hbuffer;
	register TEHandle h;
	
	dialoggeteditbuffer (pdialog, &hbuffer);
	
	h = hbuffer; /*copy into register*/
	
	(**h).txFont = font;
	
	(**h).txSize = size;
	
	pushstyle (font, size, 0);
	
	(**h).fontAscent = globalfontinfo.ascent;
	
	(**h).lineHeight = globalfontinfo.ascent + globalfontinfo.descent + globalfontinfo.leading;
	
	popstyle ();
	
	TECalText (h);
	
	return (true);
	} /*dialogsetfontsize*/
	

boolean ptinuseritem (Point pt, DialogPtr pdialog, short item) {
	
	Rect r;
	
	dialoggetobjectrect (pdialog, item, &r);
	
	return (pointinrect (pt, r));
	} /*ptinuseritem*/
	
#endif

boolean setuseritemdrawroutine (DialogPtr pdialog, short item, dialogcallback drawroutine) {
	
	short itemtype;
	Handle itemhandle;
	Rect itemrect;
	
	GetDialogItem (pdialog, item, &itemtype, &itemhandle, &itemrect);
	
	assert (itemtype % itemDisable == userItem);
	
	SetDialogItem (pdialog, item, itemtype, (Handle) drawroutine, &itemrect);
	
	return (true);
	} /*setuseritemdrawroutine*/
	

static void highlightdialogbutton (DialogPtr pdialog, short itemnumber, boolean flon) {
	
	register DialogPtr p = pdialog;
	register short val;
	short itemtype;
	Handle itemhandle;
	Rect itembox;
	
	if (pdialog == nil) /*defensive driving*/
		return;
		
	GetDialogItem (p, itemnumber, &itemtype, &itemhandle, &itembox);
	
	if (flon)
		val = kControlButtonPart;
	else
		val = 0;
	
	HiliteControl ((ControlHandle) itemhandle, val); 
	} /*highlightdialogbutton*/


static boolean dialogsimulatehit (DialogPtr pdialog, short item) {
	
	/*
	8/26/91 dmb: block event during delay.  otherwise, silly behavior (like 
	handling updates) or even crashes (when running a scripted dialog) can
	occur
	*/
	
	if (dialogitemisbutton (pdialog, item) && dialogitemisenabled (pdialog, item)) {
	
		highlightdialogbutton (pdialog, item, true);
		
		shellblockevents (); /*limit background processing during delay*/
		
		delayticks (8);
		
		shellpopevents ();
		
		highlightdialogbutton (pdialog, item, false);
		
		return (true);
		}
	
	return (false);
	} /*dialogsimulatehit*/


static boolean dialogmapkeystroke (DialogPtr pdialog, bigstring bsmap, short *item) {
	
	/*
	map a keystroke onto a dialog item.  if no match, return false.
	
	for buttons, if the keystroke matches the first character of the button name,
	we return true with that item.  makes your choice of button names strategic.
	
	to get the number of items in the list, pull the first two bytes out of the
	dialog's itemhandle and add one.  an obvious problem if Apple changes the format
	of an item list handle.
	
	10/10/91 dmb: now take string to map instead of a character.  if bsmap is 
	longer than a single character, we look for an exact match (still ignoring case).
	
	2/7/92 dmb: use dialoghasedititems instead of looping through item list.
	*/
	
	register short i;
	register boolean flcmdkeyrequired;
	short ctitems;
	bigstring bs;
	boolean flsinglechar;
	
	alllower (bsmap); /*search is unicase*/
	
	ctitems = dialogcountitems (pdialog);
	
	flsinglechar = stringlength (bsmap) == 1;
	
	if (flsinglechar) { /*mapping a single character -- check command key*/
		
		flcmdkeyrequired = dialoghasedititems (pdialog);
		
		if (flcmdkeyrequired && !cmdkeydown ())
			return (false);
		}
	
	for (i = 1; i <= ctitems; i++) {
		
		if (dialogitemisbutton (pdialog, i)) {
			
			dialoggetbuttonstring (pdialog, i, bs);
			
			if (stringlength (bs) > 0) {
				
				if (flsinglechar)
					setstringlength (bs, 1);
				
				alllower (bs);
				
				if (equalstrings (bs, bsmap)) {
					
					*item = i;
					
					return (true);
					}
				}	
			}
		} /*for*/
	
	return (false); /*no mapping for the character*/
	} /*dialogmapkeystroke*/


static short passworditem = -1;


static void passwordprocesskey (DialogPtr pdialog, char chkb, EventRecord *ev, short *item) {
	
	TEHandle hte;
	char *buffer;
	short start, end;
	short len;
	//Code change by Timothy Paustian Sunday, April 30, 2000 10:07:30 PM
	//Changed to Opaque call for Carbon
	//I think this is what is desired, but this may break this code.
	//SInt16	dialItem = - 1;
	if( GetDialogKeyboardFocusItem(pdialog) != passworditem)
		return;
	//old code
	//dialItem = (*(DialogPeek) pdialog).editField;
	//if (dialItem != passworditem - 1) /*it's not the password item, nothing to do*/
	//	return;
	
	switch (chkb) {
		
		case chtab: case chleftarrow: case chrightarrow:
		case chuparrow: case chdownarrow:
			return;
			
		} /*switch*/
	
	#if TARGET_API_MAC_CARBON == 1
	{
	WindowPtr theWind = GetDialogWindow(pdialog);
	buffer = (char *) GetWRefCon (theWind);
	}
	#else
	buffer = (char *) GetWRefCon (pdialog);
	#endif
	//Code change by Timothy Paustian Sunday, April 30, 2000 10:13:01 PM
	//Changed to Opaque call for Carbon
	#if ACCESSOR_CALLS_ARE_FUNCTIONS == 1
	hte = GetDialogTextEditHandle(pdialog);
	#else
	//old code
	hte = (*(DialogPeek) pdialog).textH;
	#endif
	
	start = (**hte).selStart;
	
	end = (**hte).selEnd;
	
	if ((start != end) || (chkb == chbackspace)) {
	
		setstringlength (buffer, 0);
		
		TESetSelect (0, 32767, hte);
		
		TEDelete (hte);
		
		return;
		}
	
	len = stringlength (buffer) + 1;
	
	buffer [len] = chkb;
	
	setstringlength (buffer, len);
		
	(*ev).message = '\xA5';
	} /*passwordprocesskey*/
	

pascal boolean modaldialogcallback (DialogPtr pdialog, EventRecord *ev, short *item) {
 	
 	/*
 	standard dialog & alert event filtering.
 	
 	10/3/90 dmb: check result of dialogsimulatehit (which now checks enabledness). 
 	only process background tasks on null events; dialog events have priority. 
 	also, get default item instead of assuming item 1, and handle boldening.
 	
 	8/26/91 dmb: after backgrounding, make sure we're still in front.  otherwise, 
 	the dialog manager will crash
 	
 	10/14/91 dmb: set shellevent.when to deter overzealous agent processing
 	*/
 	//Code change by Timothy Paustian Sunday, April 30, 2000 10:13:28 PM
	//Changed to Opaque call for Carbon
	short defaultitem = GetDialogDefaultItem(pdialog);
 	//old codde
 	//short defaultitem = (*(DialogPeek) pdialog).aDefItem; /*was set by newmodaldialog*/
 	bigstring bsbutton;
 	short whatevent = (*ev).what;
 	//why was hit a short?
 	boolean eventHandled = false;
	
 	//register short hit = 0;
 	
 	#if !TARGET_API_MAC_CARBON && defined(flcomponent)
		long curA5 = SetUpAppA5 ();
 	#endif
 	
 	if (whatevent != nullEvent) { /*non-null event; set global event time for background logic*/
 		
 		shellevent.when = (*ev).when;
 		}
 	//I need to change this so it hands of key presses for default handling
	switch (whatevent) {
				
		case keyDown: case autoKey: {
			
 			register char chkb;
 			register boolean flcmdkey;
 	
			chkb = (*ev).message & charCodeMask;
			
			flcmdkey = ((*ev).modifiers & cmdKey) == cmdKey;
			
			if ((chkb == chreturn) || (chkb == chenter)) { /*user hit return or enter*/
				
				if (defaultitem == 0)
					break;
				
				if (!dialogsimulatehit (pdialog, defaultitem))
					break;
				
				*item = defaultitem;
				
				eventHandled = true; /*the dialog manager's version of true*/
				
				break;
				}
			
			if ((chkb == chescape) || ((chkb == '.') && flcmdkey)) { /*escape or cmd-period*/
				
				shellgetstring (cancelbuttonstring, bsbutton);
				}
			else
				setstringwithchar (chkb, bsbutton);
			
			if (passworditem > 0)
				passwordprocesskey (pdialog, chkb, ev, item);
				
			if (dialogmapkeystroke (pdialog, bsbutton, item)) {
				
				if (MenuKey ((*ev).message & charCodeMask)) { /*will be handled by system*/
					
					HiliteMenu (0);
					
					break;
					}
				
				if (!dialogsimulatehit (pdialog, *item))
					break;
				
				eventHandled = true;
				
				break;
				}
			break; /*keydown or autokey*/
			}
		
		case updateEvt:
		
			if ((defaultitem > 0) && ((*ev).message == (long) pdialog))
				boldenbutton (pdialog, defaultitem);
			#if TARGET_API_MAC_CARBON == 1
				{
				CGrafPtr thePort = GetDialogPort(pdialog);
				QDFlushPortBuffer(thePort, nil);	
				}
			#endif
			break;
		
		case nullEvent:
			
			/*
			shellblockevents (); /%all events are blocked when modal dialog is in front%/
			
			if (flscriptrunning)
				langbackgroundtask ();
			else
				shellbackgroundtask ();
			
			shellpopevents ();
			*/
			//Code change Timothy Paustian 9/16/00
			//This is a bit of a hack, but it works. The contents of the port
			//need to be flushed on screen. The only way I could figure to do this
			//was to do it at every null event. The whole thing should be rewritten
			//at some points to use ModalDialog Correctly.
			#if TARGET_API_MAC_CARBON == 1
				{
				WindowRef pWindow = GetDialogWindow(pdialog);
				CGrafPtr thePort = GetDialogPort(pdialog);
				
				SelectWindow(pWindow);
				QDFlushPortBuffer(thePort, nil);	
				}
			#else
				SelectWindow (pdialog); /*make sure no one has screwed around with us*/
			#endif
			break;
		
		} /*switch*/
	
	#if !TARGET_API_MAC_CARBON && defined(flcomponent)
		RestoreA5 (curA5);
	#endif
	return (eventHandled); /*the dialog manager's version of false*/
	} /*modaldialogcallback*/


//Code change by Timothy Paustian Sunday, May 7, 2000 11:07:09 PM
//Changed to Opaque call for Carbon

#if !TARGET_RT_MAC_CFM

static short runmodaldialog (void) {
	
	short itemnumber;
	
	setcursortype (cursorisarrow);
	
	#ifdef flcomponent
		{
		ProcPtr filter = (ProcPtr) modaldialogcallback;
		#if !TARGET_API_MAC_CARBON && defined(flcomponent)
			long appA5 = SetUpCurA5 (); /*for system*/
		#endif
		
		ModalDialog ((ModalFilterUPP) filter, &itemnumber);
		
		#if !TARGET_API_MAC_CARBON && defined(flcomponent)
			RestoreA5 (appA5);
		#endif
		}
	#else
	
		ModalDialog (&modaldialogcallback, &itemnumber);
		
	#endif
	
	return (itemnumber);
	} /*runmodaldialog*/

#else

#if !TARGET_API_MAC_CARBON
static RoutineDescriptor modalfilterdesc = BUILD_ROUTINE_DESCRIPTOR (uppModalFilterProcInfo, modaldialogcallback);
#endif

static short runmodaldialog (void) {
	
	short itemnumber;
	#if TARGET_API_MAC_CARBON == 1
	ModalFilterUPP filter = NewModalFilterUPP(modaldialogcallback);
	#else	
	ModalFilterUPP filter = &modalfilterdesc;
	#endif
		
	setcursortype (cursorisarrow);
	
	shellmodaldialogmenuadjust ();
	
	#if flcomponent && !TARGET_API_MAC_CARBON
		{
		long appA5 = SetUpCurA5 (); /*for system*/
		
		ModalDialog (filter, &itemnumber);
		
		RestoreA5 (appA5);
		}
	#else
	
//while(itemnumber != 1 && itemnumber != 2 && itemnumber != 3)
//	{
		ModalDialog (filter, &itemnumber);
//	}
	#endif
	#if TARGET_API_MAC_CARBON == 1
	DisposeModalFilterUPP(filter);
	#endif
	shellforcemenuadjust ();
	
	return (itemnumber);
	} /*runmodaldialog*/
	
#endif

#if !flruntime

void dialogupdate (EventRecord *event, DialogPtr pdialog) {
	
	/*
	for modeless dialogs, we can't use DialogSelect here, because we're 
	being called within a Begin/EndUpdate pair
	*/
	
	short item;
	//Code change by Timothy Paustian Sunday, April 30, 2000 10:16:35 PM
	//Changed to Opaque call for Carbon
	//who ever calls this, it is only references from langdialogupdate and that is not
	//called in Frontier as far as I can see. I bet it is called as a call back, but I 
	//haven't found out how.
	#if ACCESSOR_CALLS_ARE_FUNCTIONS == 1
	CGrafPtr	thePort;
	RgnHandle	dialRgn = NewRgn();
	thePort = GetDialogPort(pdialog);
	dialRgn = GetPortVisibleRegion(thePort, dialRgn);
	UpdateDialog (pdialog, dialRgn);
	//Code change by Timothy Paustian Friday, September 15, 2000 9:01:04 PM
	//Mac OS X uses double buffered windows. If you are not calling WaitNextEvent
	//you have to tell the OS to flush the contents of the offscreen buffer
	//to the screen. Use QDFlushPortBuffer to do this.
	QDFlushPortBuffer(thePort, dialRgn);
	DisposeRgn(dialRgn);
	#else
	//old code
	UpdateDialog (pdialog, (*pdialog).visRgn);
	#endif
	
	modaldialogcallback (pdialog, event, &item);
	} /*dialogupdate*/

	
boolean dialogevent (EventRecord *event, DialogPtr pdialog, short *itemnumber) {
	
	/*
	if the event refers to an item in pdialog's item list, return the item number
	and true, otherwise return false.
	*/
	
	DialogPtr dlog;
	
	if (pdialog == nil) /*no dialog open*/
		return (false);
	
	if (!IsDialogEvent (event)) 
		return (false);
	
	if (modaldialogcallback (pdialog, event, itemnumber)) /*special keystroke*/
		return (true);
	
	if (!DialogSelect (event, &dlog, itemnumber))
		return (false);
	
	return (dlog == pdialog); /*click in another dialog?*/
	} /*dialogevent*/


boolean dialogidle (DialogPtr pdialog) {
	
	TEHandle hbuffer;
	
	dialoggeteditbuffer (pdialog, &hbuffer);
	
	if (hbuffer != nil)
		TEIdle (hbuffer);
	
	return (true);
	} /*dialogidle*/
	
	
boolean dialogactivate (DialogPtr pdialog, boolean flactivate) {
	
	TEHandle hbuffer;
	
	dialoggeteditbuffer (pdialog, &hbuffer);
	
	if (hbuffer != nil) {
		
		if (flactivate)
			TEActivate (hbuffer);
		else
			TEDeactivate (hbuffer);
		}
	
	return (true);
	} /*dialogactivate*/


boolean dialoggetselect (DialogPtr pdialog, short *startsel, short *endsel) {
	
	TEHandle hbuffer;
	
	dialoggeteditbuffer (pdialog, &hbuffer);
	
	if (hbuffer != nil)  {
		
		*startsel = (**hbuffer).selStart + 1;
		
		*endsel = (**hbuffer).selEnd + 1;
		}
	
	return (true);
	} /*dialoggetselect*/

#endif

boolean dialogsetselect (DialogPtr pdialog, short startsel, short endsel) {
	
	TEHandle hbuffer;
	if (pdialog == nil) return false;


	
	dialoggeteditbuffer (pdialog, &hbuffer);
	
	if (hbuffer != nil) 
		TESetSelect (startsel, endsel, hbuffer);

	
	return (true);
	} /*dialogsetselect*/


boolean dialogselectall (DialogPtr pdialog) {
	
	OSStatus		err = noErr;

	#if TARGET_API_MAC_CARBON == 1
		WindowClass 	wclass;
		err = GetWindowClass ((WindowRef) pdialog, &wclass);
		if (err==noErr) {
			if (wclass == kModalWindowClass) {
				// it's a dialog
	return (dialogsetselect (pdialog, 0, infinity));
			} else if (wclass == kDocumentWindowClass) {
			
				// it's not a dialog, it's an olde-tyme window. The Find window, for example.
				return (dialogsetselect (GetDialogFromWindow((WindowRef) pdialog), 0, infinity));
			}
			
	} /*dialogselectall*/
		return 0;
	#else
		return (dialogsetselect (pdialog, 0, infinity));
	#endif
}

#if 1	// !flruntime -- we can rely on the linker to omit these now

short savedialog (bigstring bsfname) {
	
	/*
	returns 1, 2 or 3.
	
	if the user said yes, save the changes, the result is 1.
	
	if the user said no, discard the changes, the result is 2.
	
	if the user said cancel, continue editing, the result is 3.
	
	12/20/91 dmb: if file name is nil, just prompt for quit
	
	1/3/92 dmb: removed code for quit dialog
	*/
	
	register DialogPtr pdialog;
	register short itemnumber;
	
	/*
	register short id;
	
	if (bsfname == nil) /%file wasn't dirty; just want to confirm Quit%/
		id = quitdialogid;
	else
		id = savedialogid;
	
	if ((pdialog = newmodaldialog (id, saveyesitem)) == nil)
		return (1);
	*/
	
	if ((pdialog = newmodaldialog (savedialogid, saveyesitem)) == nil)
		return (1);
	
	ParamText (bsfname, nil, nil, nil);
	#if TARGET_API_MAC_CARBON == 1
	{
	WindowRef pWind = GetDialogWindow(pdialog);
	ShowWindow(pWind);
	}
	#else
	ShowWindow (pdialog);
	#endif	
	
	itemnumber = runmodaldialog ();
	
	disposemodaldialog (pdialog);
	
	switch (itemnumber) {
		
		case saveyesitem: 
			return (1);
		
		case savenoitem: 
			return (2);
		
		case savecancelitem: 
			return (3);
		} /*switch*/
	
	return (1);
	} /*savedialog*/


short replacevariabledialog (bigstring bsitem) {
	
	/*
	return 1 if the user clicked on Replace, 2 if Duplicate, 3 if Cancel.
	*/

	register DialogPtr pdialog;
	register short itemnumber;
	
	sysbeep ();
		
	ParamText (bsitem, nil, nil, nil);
	
	if ((pdialog = newmodaldialog (replacedialogid, replacereplaceitem)) == nil)
		return (1);
	
	#if TARGET_API_MAC_CARBON == 1
	{
	WindowRef pWind = GetDialogWindow(pdialog);
	ShowWindow(pWind);
	}
	#else
	ShowWindow (pdialog);
	#endif
	itemnumber = runmodaldialog ();
	
	disposemodaldialog (pdialog);
	
	switch (itemnumber) {
		
		case replacereplaceitem: 
			return (1);
		
		case replaceduplicateitem:
			return (2);
		
		case replacecancelitem: 
			return (3);
		} /*switch*/
	
	return (1);
	} /*replacevariabledialog*/


boolean revertdialog (bigstring bsfname) {
	
	/*
	put up the standard "revert" dialog, with the provided file name and return
	true if the user clicked on ok.  false if cancel was clicked.
	*/
	
	register DialogPtr pdialog;
	register short itemnumber;
	
	ParamText (bsfname, nil, nil, nil);
	
	if ((pdialog = newmodaldialog (revertdialogid, revertokitem)) == nil)
		return (false);
	#if TARGET_API_MAC_CARBON == 1
	{
	WindowRef pWind = GetDialogWindow(pdialog);
	ShowWindow(pWind);
	}
	#else
	ShowWindow (pdialog);	
	#endif
	itemnumber = runmodaldialog ();
	
	disposemodaldialog (pdialog);
	
	return (itemnumber == revertokitem);
	} /*revertdialog*/


boolean askdialog (bigstring bsprompt, bigstring bsanswer) {
	
	/*
	put up the standard "ask" dialog, with the provided prompt and return
	true if the user clicked on ok.  the answer is in bsanswer.
	*/
	
	register DialogPtr pdialog;
	register short itemnumber;
	
	ParamText (bsprompt, nil, nil, nil);
	
	if ((pdialog = newmodaldialog (askdialogid, askokitem)) == nil)
		return (false);
	
	setdialogtext (pdialog, askansweritem, bsanswer);
	
//	selectdialogtext (pdialog, askansweritem);
	
	#if TARGET_API_MAC_CARBON == 1
	{
	WindowRef pWind = GetDialogWindow(pdialog);
	ShowWindow(pWind);
	
	}
	#else
	ShowWindow (pdialog);
	#endif
	
	#if TARGET_API_MAC_CARBON == 1
	
		while (true) {
	
	#endif
	
	itemnumber = runmodaldialog ();
	
	#if TARGET_API_MAC_CARBON == 1
	
		if (itemnumber == askokitem || itemnumber == askcancelitem)
			break;
			
		} /*while*/
	
	#endif
	
	getdialogtext (pdialog, askansweritem, bsanswer);
	
	disposemodaldialog (pdialog);
	
	return (itemnumber == askokitem);
	} /*askdialog*/


boolean twowaydialog (bigstring bsprompt, bigstring okbutton, bigstring cancelbutton) {
	
	register DialogPtr pdialog;
	register short item;
	
	if ((pdialog = newmodaldialog (twowaydialogid, twowayokitem)) == nil)
		return (false);
	
	setdialogtext (pdialog, twowaymsgitem, bsprompt);
	
	dialogsetbuttonstring (pdialog, twowayokitem, okbutton);
	
	dialogsetbuttonstring (pdialog, twowaycancelitem, cancelbutton);
	
	#if TARGET_API_MAC_CARBON == 1
	{
	WindowRef pWind = GetDialogWindow(pdialog);
	ShowWindow(pWind);
	}
	#else
	ShowWindow (pdialog);
	#endif
	
	item = runmodaldialog ();
	
	disposemodaldialog (pdialog);
	
	return (item == twowayokitem);
	} /*twowaydialog*/


short threewaydialog (bigstring bsprompt, bigstring yesbutton, bigstring nobutton, bigstring cancelbutton) {
	
	register DialogPtr pdialog;
	register short item;
	
	if ((pdialog = newmodaldialog (threewaydialogid, threewayyesitem)) == nil)
		return (false);
	
	setdialogtext (pdialog, threewaymsgitem, bsprompt);
	
	dialogsetbuttonstring (pdialog, threewaycancelitem, cancelbutton);
	
	dialogsetbuttonstring (pdialog, threewaynoitem, nobutton);
	
	dialogsetbuttonstring (pdialog, threewayyesitem, yesbutton);
	
	#if TARGET_API_MAC_CARBON == 1
	{
	WindowRef pWind = GetDialogWindow(pdialog);
	ShowWindow(pWind);
	}
	#else
	ShowWindow (pdialog);
	#endif
	item = runmodaldialog ();
	
	disposemodaldialog (pdialog);
	
	switch (item) {
		
		case threewaycancelitem:
			return (3);
			
		case threewaynoitem:
			return (2);
			
		case threewayyesitem:
		default:
			return (1);
		} /*switch*/
	} /*threewaydialog*/
	
	
boolean intdialog (bigstring bsprompt, short *intval) {
	
	register DialogPtr pdialog;
	register short itemnumber;
	bigstring bs;
	
	if ((pdialog = newmodaldialog (intdialogid, intokitem)) == nil)
		return (false);
	
	setdialogtext (pdialog, intpromptitem, bsprompt);
	
	shorttostring (*intval, bs);
	
	setdialogtext (pdialog, intintitem, bs);
	
	selectdialogtext (pdialog, intintitem);
	
	#if TARGET_API_MAC_CARBON == 1
	{
	WindowRef pWind = GetDialogWindow(pdialog);
	ShowWindow(pWind);
	}
	#else
	ShowWindow (pdialog);
	#endif
	itemnumber = runmodaldialog ();

	getdialogtext (pdialog, intintitem, bs);
	
	stringtoshort (bs, intval);
	
	disposemodaldialog (pdialog);
	
	return (itemnumber == intokitem);
	} /*intdialog*/


boolean chardialog (bigstring bsprompt, short *charval) {
	
	register DialogPtr pdialog;
	register char ch;
	register short itemnumber;
	bigstring bs;
	
	if ((pdialog = newmodaldialog (chardialogid, charokitem)) == nil)
		return (false);
	
	setdialogtext (pdialog, charpromptitem, bsprompt);
	
	setstringwithchar (*charval, bs);
	
	setdialogtext (pdialog, charvalitem, bs);
	
	selectdialogtext (pdialog, charvalitem);
	
	#if TARGET_API_MAC_CARBON == 1
	{
	WindowRef pWind = GetDialogWindow(pdialog);
	ShowWindow(pWind);
	}
	#else
	ShowWindow (pdialog);
	#endif
	while (true) {
		
		itemnumber = runmodaldialog ();
		
		getdialogtext (pdialog, charvalitem, bs);
		
		if (isemptystring (bs))
			ch = chnul;
		else
			ch = bs [1];
		
		ch = uppercasechar (ch); /*cmd-keystrokes must be uppercase*/
		
		switch (itemnumber) {
			
			case charokitem: case charcancelitem:
				*charval = ch;
				
				disposemodaldialog (pdialog);
				
				return (itemnumber == charokitem);
			
			default:
				setstringwithchar (ch, bs);
				
				setdialogtext (pdialog, charvalitem, bs);
				
				selectdialogtext (pdialog, charvalitem);
				
				break;
			}
		}
	} /*chardialog*/

#endif


boolean msgdialog (bigstring bsprompt) {
	
	/*
	put up the standard "msg" dialog, with the provided prompt and return
	true if the user clicked on ok.  false if cancel was clicked.
	*/
	
	register DialogPtr pdialog;
	register short itemnumber;
	
	if ((pdialog = newmodaldialog (msgdialogid, msgokitem)) == nil) /*error; assume OK*/
		return (true);
	
	setdialogtext (pdialog, msgmsgitem, bsprompt);
	
	#if TARGET_API_MAC_CARBON == 1
	{
	WindowRef pWind = GetDialogWindow(pdialog);
	ShowWindow(pWind);
	}
	#else
	ShowWindow (pdialog);
	#endif
	itemnumber = runmodaldialog ();
	
	disposemodaldialog (pdialog);
	
	return (itemnumber == msgokitem);
	} /*msgdialog*/


short customalert (short id, bigstring bsprompt) {
	
	/*
	put up the dialog with the indicated id, and run it.
	
	**** currently using DisposDialog so it can be called from standard file;
	frontshellwindow should be smarter!
	*/
	
	register DialogPtr pdialog;
	register short itemnumber;
	
	if ((pdialog = newmodaldialog (id, 1)) == nil)
		return (0);
	
	setdialogtext (pdialog, 2, bsprompt);
	
	#if TARGET_API_MAC_CARBON == 1
	{
	WindowRef pWind = GetDialogWindow(pdialog);
	ShowWindow(pWind);
	}
	#else
	ShowWindow (pdialog); /*make sure it's visible now that values are set*/
	#endif
	itemnumber = runmodaldialog ();
	
	DisposeDialog (pdialog);
	
	return (itemnumber);
	} /*customalert*/


boolean alertdialog (bigstring bsprompt) {
	
	/*
	put up the standard "alert" dialog, with the provided prompt and return
	true if the user clicked on ok.  false if cancel was clicked.
	*/
	
	register DialogPtr pdialog;
	register short itemnumber;
	
	sysbeep ();
	
	if ((pdialog = newmodaldialog (alertdialogid, alertokitem)) == nil)
		return (false);
	
	setdialogtext (pdialog, alertmsgitem, bsprompt);
	
	#if TARGET_API_MAC_CARBON == 1
	{
	WindowRef pWind = GetDialogWindow(pdialog);
	ShowWindow(pWind);
	}
	#else
	ShowWindow (pdialog);
	#endif
	itemnumber = runmodaldialog ();
	
	disposemodaldialog (pdialog);
	
	return (itemnumber == alertokitem);
	} /*alertdialog*/


boolean alertstring (short iderror) {
	
	bigstring bs;
	
	getstringlist (alertstringlistnumber, iderror, bs);
	
	return (alertdialog (bs));
	} /*alertstring*/


boolean customdialog (short id, short defaultitem, dialogcallback itemhitcallback) {
	
	/*
	put up the dialog with the indicated id, and run it until the 
	callback says we're done.
	*/
	
	register DialogPtr pdialog;
	register short itemnumber;
	
	if ((pdialog = newmodaldialog (id, defaultitem)) == nil)
		return (false);
	
	if ((*itemhitcallback) (pdialog, -1)) { /*special value tells callback to initialize fields*/
		
		dialogselectall (pdialog); /*in case there's a text item*/
		
		#if TARGET_API_MAC_CARBON == 1
		{
		WindowRef pWind = GetDialogWindow(pdialog);
		ShowWindow(pWind);
		}
		#else
		ShowWindow (pdialog); /*make sure it's visible now that values are set*/
		#endif
				
		while (true) {
			
			itemnumber = runmodaldialog ();
			
			if (!(*itemhitcallback) (pdialog, itemnumber))
				break;
			}
		}
	
	disposemodaldialog (pdialog);
	
	return (true);
	} /*customdialog*/


boolean askpassword (bigstring passprompt, bigstring password) {
	
	/*
	3.0.1 dmb: if id is not the empty string, start with the password item selected
	*/
	
	register DialogPtr pdialog;
	register short itemnumber;
	
	ParamText (passprompt, nil, nil, nil);
	
	if ((pdialog = newmodaldialog (askdialogid, askokitem)) == nil)
		return (false);
		
	setemptystring (password);
	#if TARGET_API_MAC_CARBON == 1
	{
	WindowRef pWind = GetDialogWindow(pdialog);
	SetWRefCon (pWind, (long) password);
	}
	#else

	SetWRefCon (pdialog, (long) password);
	
	#endif
	
//	setdialogtext (pdialog, passworditem, password);	
//	setdialogtext (pdialog, iditem, id);
	
	setdialogtext (pdialog, askansweritem, password);
	
	//selectdialogtext (pdialog, askansweritem);
	#if TARGET_API_MAC_CARBON == 1
	{
	WindowRef pWind = GetDialogWindow(pdialog);
	ShowWindow(pWind);
	}
	#else
	ShowWindow (pdialog);
	#endif
	
	passworditem = askansweritem;
	
	#if TARGET_API_MAC_CARBON == 1
	
		while (true) {
	
	#endif

	itemnumber = runmodaldialog ();
	
	#if TARGET_API_MAC_CARBON == 1
	
		if (itemnumber == askokitem || itemnumber == askcancelitem)
			break;
			
		} /*while*/

	#endif
	
	passworditem = -1;
	
//	getdialogtext (pdialog, iditem, id);
	// xxx getdialogtext (pdialog, askansweritem, password);
	
	disposemodaldialog (pdialog);
	
	return (itemnumber == askokitem);
	} /*askpassword*/


boolean initdialogs (void) {
	
	#ifdef flcomponent
		//Code change by Timothy Paustian Sunday, May 7, 2000 11:10:48 PM
		//In Carbon this is not needed.
		#if !TARGET_CARBON
		RememberA5 ();
		#endif /*for filters, callbacks*/
	#endif
	
	return (true);
	} /*dialoginit*/


char X0_p2cstrcpy(char *dst, StringPtr src) 
{
	register short	len = src[0], i;
	
	for(i=0; i<len; i++)
		dst[i] = src[i+1];
		
	dst[i] = 0x00;
		
	return (*dst);
}




