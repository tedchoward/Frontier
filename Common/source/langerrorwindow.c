
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

#include "bitmaps.h"
#include "frontierconfig.h"
#include "cursor.h"
#include "font.h"
#include "icon.h"
#include "kb.h"
#include "memory.h"
#include "mouse.h"
#include "ops.h"
#include "popup.h"
#include "resources.h"
#include "quickdraw.h"
#include "scrap.h"
#include "strings.h"
#include "textedit.h"
#include "windowlayout.h"
#include "frontierwindows.h"
#include "shell.rsrc.h"
#include "shellhooks.h"
#include "lang.h"
#include "langinternal.h"
#include "cancoon.h"

static void langerroractivate (boolean flactivate); /*forward*/

#pragma pack(2)
typedef struct tylangerrorrecord {
	
	bigstring bserror; /*the error message*/
	
	bigstring bslocation; /*line number and character number of the error*/
	
	Rect iconrect; /*location of the icon, for mouse clicks, cursor adjustment*/
	
	Rect textrect; /*location of the error text, for updates, activates*/
	
	unsigned short lnum, cnum; /*location of error, passed to callback routine*/
	
	//callback errorcallback; /*routine to call when user hits script button*/
	
	//long errorrefcon; /*the info we pass him*/
	
	hdlerrorstack herrorstack;
	
	boolean flcallbackconsumed: 1; /*if true do nothing on script button hit*/
	
	boolean flactive: 1; /*determines whether message is drawn as active or inactive*/
	} tylangerrorrecord, *ptrlangerrorrecord, **hdllangerrorrecord;
#pragma options align=reset


WindowPtr langerrorwindow = nil;

hdlwindowinfo langerrorwindowinfo = nil;

hdllangerrorrecord langerrordata = nil;

static hdllangerrorrecord pendingerrordata = nil;

static long inhibiterrorclear = 0; // 5.0.2b3



static boolean langerrorgetwindowrect (Rect *rwindow) {
	
	return (ccgetwindowrect (ixlangerrorinfo, rwindow));
	} /*langerrorgetwindowrect*/
	
	
static boolean langerrorclose (void) {
	
	ccsubwindowclose (langerrorwindowinfo, ixlangerrorinfo);
	
	return (true);	// even if we couldn't save our position
	} /*langerrorclose*/


static boolean langerrorcopy (void) {
	
	/*
	9/11/92 dmb: users pointed out that this might be useful for reporting 
	bugs and problems...
	*/
	
	bigstring bserror;
	Handle htext;
	
	copystring ((**langerrordata).bserror, bserror);
	
	if (!newtexthandle (bserror, &htext))
		return (false);
	
	return (shellsetscrap (htext, textscraptype, (shelldisposescrapcallback) disposehandle, nil));
	} /*langerrorcopy*/


static boolean langerrornewwindow (void) {
	
	/*
	5.0.2b3 dmb: added inhibiterrorclear logic to make sure zooming doesn't 
	clear us
	
	6.2b6 AR: Extended inhibiterrorclear protection so that langerrorlcear doesn't interfer
	*/
	
	hdlwindowinfo hw;
	WindowPtr w;
	bigstring bstitle;
	Rect rzoom, rwindow;
	hdlwindowinfo hparent;
	tylangerrorrecord **hdata;
	boolean fl = false;
	
	++inhibiterrorclear; // protect us from other threads while we're still busy creating the window
	
	if (!shellgetfrontrootinfo (&hparent)) /*our parent is the frontmost root window*/
		goto exit;
			
	shellgetwindowcenter (hparent, &rzoom);
	
	if (pendingerrordata != nil) {
		
		hdata = pendingerrordata;
		
		pendingerrordata = nil;
		}
	else {
		
		if (!newclearhandle (sizeof (tylangerrorrecord), (Handle *) &hdata))
			goto exit;
		}
	
	langerrorgetwindowrect (&rwindow);
	
	getstringlist (langerrorlistnumber, langerrortitlestring, bstitle);
	
	if (!newchildwindow (idlangerrorconfig, hparent, &rwindow, &rzoom, bstitle, &w)) {
		
		disposehandle ((Handle) hdata);
		
		goto exit;
		}
		
	getwindowinfo (w, &hw);
	
	(**hw).hdata = (Handle) hdata;
	
	ccnewsubwindow (hw, ixlangerrorinfo);
	
	windowzoom (w);

	fl = true;
	
exit:
	
	--inhibiterrorclear;
	
	return (fl);
	} /*langerrornewwindow*/


static boolean langerrorsetrects (void) {
	
	register hdllangerrorrecord hle = langerrordata;
	Rect rcontent;
	Rect r;
	
	rcontent = (**langerrorwindowinfo).contentrect;
	
	r = rcontent;
	
	r.top = r.top + windowmargin;
	
	r.bottom = r.top + iconrectheight;
	
	r.right = r.right - windowmargin;
	
	r.left = r.right - iconrectwidth;
	
	(**hle).iconrect = r; 
	
	r.right = r.left - windowmargin;
	
	r.left = rcontent.left + windowmargin;
	
	r.bottom = rcontent.bottom - windowmargin;
	
	(**hle).textrect = r;
	
	return (true);
	} /*langerrorsetrects*/


static void langerrordrawicon (boolean flpressed) {
	
	register hdllangerrorrecord hle = langerrordata;
	register boolean flenabled;
	bigstring bs;
	
	flenabled = (**hle).flactive && !(**hle).flcallbackconsumed;
	
	getstringlist (langerrorlistnumber, scripticonstring, bs);
	
	drawlabeledwindoidicon ((**hle).iconrect, bs, flenabled, flpressed);
	} /*langerrordrawicon*/

	
static void langerrorframetext (void) {
	
	register hdllangerrorrecord hle = langerrordata;
	
	pushpen ();
	
	if (!(**hle).flactive) 
		setgraypen ();
	
	framerect ((**hle).textrect);
	
	poppen ();
	} /*langerrorframetext*/

static void langerrordrawtext (boolean flbitmap) {
	
	/*
	4/18/91 dmb: straighten fancy quotes when displaying in geneva 9
	*/
	
	register hdllangerrorrecord hle = langerrordata;
	register hdlwindowinfo hw = langerrorwindowinfo;
	Rect rbox;
	bigstring bserror;
	short font, size;
	
	copystring ((**hle).bserror, bserror);
	
	font = (**hw).defaultfont;
	
	size = (**hw).defaultsize;
	
	if ((font == geneva) && (size <= 9)) { /*get rid of fancy quotes; nasty in this font*/
		
		stringreplaceall ('Õ', '\'', bserror); /*straighten contractions*/
		
		if (!stringfindchar ('"', bserror)) { /*don't already have straight quotes*/
			
			stringreplaceall ('Ò', '"', bserror);
			
			stringreplaceall ('Ó', '"', bserror);
			}
		}
	
	rbox = (**hle).textrect;
	
	insetrect (&rbox, 3, 3);
	
	if (flbitmap)
		flbitmap = openbitmap (rbox, langerrorwindow);
	
	edittwostringbox (bserror, zerostring, rbox, font, size);
	
	/*
	edittwostringbox (bserror, (**hle).bslocation, rbox, font, size);
	*/
	
	if (flbitmap)
		closebitmap (langerrorwindow);
	} /*langerrordrawtext*/


static void langerrorupdate (void) {
	
	langerrorsetrects ();
	
	/*
	eraserect ((**hw).contentrect);
	*/
		drawthemeborder ((**langerrordata).textrect, (**langerrorwindowinfo).contentrect);

	langerrordrawicon (false);
	
	#ifdef gray3Dlook
		pushbackcolor (&whitecolor);

		eraserect ((**langerrordata).textrect);
	#endif
	
//	framerect ((**langerrordata).textrect);
	
	langerrorframetext ();
		
	langerrordrawtext (false);

	//Timothy Paustian 10/5/00
	//For some reason the buffer is not being flushed for this window.
	//This should fix it.
		QDFlushPortBuffer (GetWindowPort (langerrorwindow), nil);

	#ifdef gray3Dlook
		popbackcolor ();
	#endif
	} /*langerrorupdate*/


static void langerroractivate (boolean flactivate) {
	
	(**langerrordata).flactive = flactivate;
	
	/*
	return (langerrorupdate ());
	*/
	
	langerrorsetrects ();
	
	langerrorframetext ();
	
		drawthemeborder ((**langerrordata).textrect, (**langerrorwindowinfo).contentrect);

	langerrordrawicon (false);
	} /*langerroractivate*/


static boolean langerrorfindwindow (WindowPtr *w, hdllangerrorrecord *hdata) {
	
	hdlwindowinfo hinfo;
	
	return (shellfindwindow (idlangerrorconfig, w, &hinfo, (Handle *) hdata));
	} /*langerrorfindwindow*/


boolean clearlangerrordialog (void) {
	
	WindowPtr w;
	hdllangerrorrecord hle;
	
	if (inhibiterrorclear == 0)
		if (langerrorfindwindow (&w, &hle))
			shellclosewindow (w);
	
	return (true);
	} /*clearlangerrordialog*/


static boolean langerrorset (bigstring bserror, bigstring bslocation) {
#pragma unused (bslocation)

	/*
	2.1b2 dmb: if the langerror window isn't already there, don't try to 
	create it now -- the heap or stack may be dangerously low.  instead, 
	just build the data structure and wait for langerrorflush to be called
	*/
	
	hdllangerrorrecord hle;
	boolean flalreadythere;
	hdlerrorstack herrorstack;
	register hdlerrorstack hs;
	register boolean flcallbackconsumed;
	WindowPtr w;
	
	flalreadythere = langerrorfindwindow (&w, &hle);
	
	if (flalreadythere) {
		
		++inhibiterrorclear; /*6.1b18 AR: make sure other threads don't clear the error window while
											we release the thread globals (only relevant for Win32)*/
	
		shellpushglobals (w);  /*dmb 9/20/90: set langerror.c globals*/
		
		shellbringtofront (langerrorwindowinfo);

		--inhibiterrorclear;
		}
	else {
		
		if (!newclearhandle (sizeof (tylangerrorrecord), (Handle *) &pendingerrordata)) {
			
			sysbeep (); // some kind of feedback
			
			return (false);
			}
		
		hle = pendingerrordata;
		
		shellforcebackgroundtask (); /*mimimize delay before window opens*/
		}
	
	copystring (bserror, (**hle).bserror);
	
	/*
	copystring (bslocation, (**hle).bslocation);
	*/
	
	copyhandle ((Handle) langcallbacks.scripterrorstack, (Handle *) &herrorstack);
	
	hs = herrorstack; /*copy into register*/
	
	disposehandle ((Handle) (**hle).herrorstack); /*toss old error info*/
	
	(**hle).herrorstack = hs;
	
	if ((hs == nil) || ((**hs).toperror == 0))
		flcallbackconsumed = true;
	else
		flcallbackconsumed = (**hs).stack [(**hs).toperror - 1].errorcallback == nil;
	
	(**hle).flcallbackconsumed = flcallbackconsumed;
	
	(**hle).lnum = ctscanlines;
	
	(**hle).cnum = ctscanchars;
	
	if (flalreadythere) {
		
		langerrorupdate ();
		
		shellpopglobals ();
		}

	return (true);
	} /*langerrorset*/


static boolean langerrordispose (void) {
	
	/*
	11/21/91 dmb: fixed memory leak; must dispose herrorstack
	*/
	
	register hdllangerrorrecord hle = langerrordata;
	
	disposehandle ((Handle) (**hle).herrorstack);
	
	disposehandle ((Handle) hle);
	
	return (true);
	} /*langerrordispose*/
	

static boolean langerroradjustcursor (Point pt) {
		
	if (pointinrect (pt, (**langerrordata).iconrect)) {
		
		if (!(**langerrordata).flcallbackconsumed) {
		
			setcursortype (cursorisgo);
			
			return (true);
			}
		}
		
	setcursortype (cursorisarrow);
	
	return (true);
	} /*langerroradjustcursor*/


static void langerrorbuttonhit (void) {
	
	register hdllangerrorrecord hle = langerrordata;
	register hdlerrorstack hs = (**hle).herrorstack;
	tyerrorrecord errorrecord = (**hs).stack [(**hs).toperror - 1];
	
	(*errorrecord.errorcallback) (errorrecord.errorrefcon, (**hle).lnum, (**hle).cnum, nil, nil);
	} /*langerrorbuttonhit*/


#ifdef flnewfeatures	// flstacktrace

static boolean langerrorpopupselect (hdlmenu hmenu, short itemselected) {
#pragma unused (hmenu)

	register hdlerrorstack hs = (**langerrordata).herrorstack;
	short ix;
	tyerrorrecord *pe;
	
	if (hs == nil)
		return (false);
	
	ix = (**hs).toperror - itemselected;
	
	pe = &(**hs).stack [ix];
	
	return ((*(*pe).errorcallback) ((*pe).errorrefcon, (*pe).errorline, (*pe).errorchar, nil, nil));
	} /*langerrorpopupselect*/


static boolean langerrorfillerrorpopup (hdlmenu hmenu, short *checkeditem) {
	
	register hdlerrorstack hs = (**langerrordata).herrorstack;
	register short ix;
	register short ixtop;
	hdlhashtable htable;
	bigstring bsname;
	boolean flenabled;
	
	if (hs == nil)
		return (false);
	
	ixtop = (**hs).toperror;
	
	for (ix = ixtop - 1; ix >= 0; --ix) {
		
		tyerrorrecord *pe = &(**hs).stack [ix];
		
		if ((*pe).errorcallback == nil ||
			!(*(*pe).errorcallback) ((*pe).errorrefcon, 0, 0, &htable, bsname)) {
		
			langgetstringlist (anomynousthreadstring, bsname); 
			
			flenabled = false;
			}
		else {
			flenabled = true;
			}
		
		if (!pushpopupitem (hmenu, bsname, flenabled, 0)) /*terminate visit on error*/
			return (false);
		}
	
	*checkeditem = -1;
	
	return (true);
	} /*langerrorfillerrorpopup*/

static boolean langerrorpopupmenu (void) {

	/*
	we're called after a tracking delay. return true to indicate that 
	we've handled the mouse click
	*/
	
	register hdllangerrorrecord hle = langerrordata;
	register hdlerrorstack hs = (**hle).herrorstack;
	Rect r;
	
	#ifdef flnewfeatures	// flstacktrace
		r = (**hle).iconrect;
		
		if (hs != nil /* && (**hs).toperror > 1*/) {
		
			r.top = r.bottom - 10;
			
			popupmenuhit (r, true, &langerrorfillerrorpopup, &langerrorpopupselect);
			}
	#endif
	
	return (true);
	} /*langerrorpopupmenu*/

#endif


static boolean langerrormousedown (Point pt, tyclickflags flags) {
#pragma unused (flags)

	/*
	7/16/90 DW: wired off the consumption of the error callback, if you want
	it back on, just switch on the code that's commented out, and the script
	icon will go dim, the cursor adjustment won't happen, and no mouseclicks
	will be handled for clicking on the script button.
	
	4.1b4 dmb: use custom tracking code so we can popup a menu after mouse has
	been down a fraction of a second.
	*/
	
	register hdllangerrorrecord hle = langerrordata;
	Rect r;
	boolean flinrectnow, flwasinrect;
	boolean flconsumed = false;
	unsigned long timemoved;
	short threshhold = getmousedoubleclicktime () * 2 / 3;
	
	if ((**hle).flcallbackconsumed)
		return (false);
	
	r = (**hle).iconrect;
	
	if (pointinrect (pt, r)) {
		
		setcursortype (cursorisarrow); /*get rid of funny appendage*/
		
		langerrordrawicon (true); /*show icon in depressed state*/
		
	//	if (!mousetrack (r, langerrordrawicon)) /*if false, mouse didn't stay in rectangle*/
	//		return (false);
		
		flwasinrect = true; /*at least for the first iteration of loop*/
		
		timemoved = gettickcount ();
		
		while (mousestilldown ()) { /*stay in holding pattern*/
		
			getmousepoint (&pt);
			
			flinrectnow = pointinrect (pt, r);
			
			if (flinrectnow != flwasinrect)	{
			
				langerrordrawicon (flinrectnow);
				
				timemoved = gettickcount ();
				}
			else if (flinrectnow && gettickcount () - timemoved > (unsigned) threshhold) {
				
				langerrorpopupmenu ();
				
				flconsumed = true;
				}
			
			flwasinrect = flinrectnow;
			} /*while*/
		
		if (flwasinrect) {
		
			langerrordrawicon (false); /*show icon in un-depressed state*/
			
			if (!flconsumed)
				langerrorbuttonhit ();
			}
		}
	
	return (true);
	} /*langerrormousedown*/


static boolean langerrorkeystroke (void) {
	
	/*
	handle the enter key to dispatch the error routine, if not consumed
	*/
	
	register hdllangerrorrecord hle = langerrordata;
	
	if (keyboardstatus.chkb != chenter)
		return (false);
	
	if ((**hle).flcallbackconsumed)
		return (false);
	
	langerrordrawicon (true);
	
	delayticks (8); /*so you can actually see the button down*/
	
	langerrordrawicon (false);
	
	langerrorbuttonhit ();
	
	return (true);
	} /*langerrorkeystroke*/


boolean langerrordialog (bigstring bsmsg, ptrvoid refcon) {
#pragma unused (refcon)

	unsigned short savedlines, savedchars; 
	
	if (isemptystring (bsmsg)) /*don't post empty messages -- scipterrror ("") relies on this*/
		return (true);
	
	savedlines = ctscanlines; savedchars = ctscanchars; /*dialogs allow background tasks*/
	
	langerrorset (bsmsg, zerostring); /*bslocation*/
	
	ctscanlines = savedlines; ctscanchars = savedchars; /*restore*/
	
	return (true);
	} /*langerrordialog*/


boolean langerrorflush (void) {
	
	/*
	if there's a lang error pending, show the window now
	
	2.1b2 dmb: added this routine along w/change in langerrorset to make 
	error reporting more reliable.
	*/
	
	if (pendingerrordata == nil)
		return (false);
	
	return (langerrornewwindow ());
	} /*langerrorflush*/


#ifdef flnewfeatures

hdlerrorstack langerrorgetstack (void) {
	
	WindowPtr w;
	hdllangerrorrecord hle;
	
	if (!langerrorfindwindow (&w, &hle))
		return (nil);
	
	return ((**hle).herrorstack);
	} /*langerrorgetstack*/

#endif


static boolean langerrordirtyhook (void) {
	
	/*
	this is where we hook into events that dirty things.  if a langerror 
	window is open, we gray out its script button.  the theory is that we
	can't handle the world changing **that** much, and as soon as the user
	does some editing or cursor moving, we don't want to take any chances
	on the callback routine and the data linked into the langerror window.
	
	7/20/90 DW: it's important to keep the work to an absolute minimum 
	when the window is open because the system really bogs down if you
	constantly update the script icon in the langerror window.
	*/
	
	WindowPtr w;
	hdllangerrorrecord hle;
	
	if (flscriptrunning) /*don't disable icon if dirtyness originates from a script*/
		return (true);
	
	if (langerrorfindwindow (&w, &hle)) { /*window is open, something to do*/
		
		if (!hle || (**hle).flcallbackconsumed) /*7/20/90 DW: quick exit*/
			return (true);
		
		shellpushglobals (w);
		
		(**langerrordata).flcallbackconsumed = true;
		
		/*
		langerrorupdate ();
		*/
		
		langerrorsetrects ();
		
		langerrordrawicon (false);
		
		shellpopglobals ();
		}
	
	return (true);
	} /*langerrordirtyhook*/
	

boolean langerrorstart (void) {
	
	/*
	set up callback routines record, and link our data into the shell's 
	data structure.
	*/
	
	ptrcallbacks langerrorcallbacks;
	register ptrcallbacks cb;
	
	langcallbacks.errormessagecallback = &langerrordialog;
	
	langcallbacks.clearerrorcallback = &clearlangerrordialog;
	
	shellnewcallbacks (&langerrorcallbacks);
	
	cb = langerrorcallbacks; /*copy into register*/
	
	loadconfigresource (idlangerrorconfig, &(*cb).config);
	
	(*cb).configresnum = idlangerrorconfig;
	
	(*cb).windowholder = &langerrorwindow;
	
	(*cb).dataholder = (Handle *) &langerrordata;
	
	(*cb).infoholder = &langerrorwindowinfo;
	
	(*cb).updateroutine = &langerrorupdate;
	
	(*cb).activateroutine = &langerroractivate;
	
	(*cb).adjustcursorroutine = &langerroradjustcursor;
	
	(*cb).mouseroutine = &langerrormousedown;
	
	(*cb).keystrokeroutine = &langerrorkeystroke;
	
	(*cb).disposerecordroutine = &langerrordispose;
	
	(*cb).closeroutine = &langerrorclose;
	
	(*cb).cutroutine = &langerrorcopy;
	
	(*cb).copyroutine = &langerrorcopy;
	
	shellpushdirtyhook (&langerrordirtyhook);
	
	return (true);
	} /*langerrorstart*/




