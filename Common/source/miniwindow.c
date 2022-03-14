
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
#include "cursor.h"
#include "frontierconfig.h"
#include "kb.h"
#include "icon.h"
#include "font.h"
#include "memory.h"
#include "mouse.h"
#include "ops.h"
#include "popup.h"
#include "quickdraw.h"
#include "scrap.h"
#include "frontier_strings.h"
#include "windowlayout.h"
#include "frontierwindows.h"
#include "zoom.h"
#include "shell.h"
#include "shellundo.h"
#include "cancoon.h"
#include "miniwindow.h"
#include "wpengine.h"


#define flmsgactive flmassiveupdate

WindowPtr miniwindow = nil;

hdlwindowinfo miniwindowinfo = nil;

hdlminirecord minidata = nil;


/*
5.0d14 dmb: major rewrite:
	use wpengine, not the Dialog Manager.
	text fields are now handles, not bigstrings
*/


static boolean minisetwpglobals (short item) {
	
	/*
	set up the word processing engine's globals
	*/
	
	if ((item < 0) || (item >= (**minidata).cttextitems)) {
		
		wpdata = nil;
		
		return (false);
		}
	
	wpdata = (**minidata).textdata [item];
	
	wpwindow = miniwindow;
	
	wpwindowinfo = miniwindowinfo;
	
	return (true);
	} /*minisetwpglobals*/


static boolean minisetactivewpglobals (void) {
	
	if (minidata == nil)
		return (false);
	
	return (minisetwpglobals ((**minidata).activetextitem));
	} /*minisetactivewpglobals*/


boolean minisetstring (short item, Handle h) {
	
	if (!minisetwpglobals (item))
		return (false);
	
	return (wpsettexthandle (h));
	} /*minisetstring*/


boolean minigetstring (short item, Handle *h) {
	
	if (!minisetwpglobals (item))
		return (false);
	
	return (wpgettexthandle (h));
	} /*minigetstring*/


boolean minigetselstring (short item, bigstring bs) {
	
	if (!minisetwpglobals (item))
		return (false);
	
	wpgetseltext (bs);
	
	return (true);
	} /*minigetselstring*/

/*
boolean minisetpopupmessage (short popupnumber, bigstring bs) {
	
	hdlstring hstring;
	
	newheapstring (bs, &hstring);
	
	(**minidata).popupmessages [popupnumber] = hstring;
	
	invalrect ((**minidata).popuprects [popupnumber]);
	} /%minisetpopupmessage%/
*/

void minisetselect (short startsel, short endsel) {
	
	if (minisetactivewpglobals ())
		wpsetselection (startsel, endsel);
	
	(**minidata).flselectallpending = false; /*override*/
	} /*minisetselect*/


#if 0

static void minigetactivetextrect (Rect *r) {
	
	*r = (**minidata).textrects [(**minidata).activetextitem];
	} /*minigetactivetextrect*/


static void minigetactivetextitem (short *item) {
	
	*item = (**minidata).textitems [(**minidata).activetextitem];
	} /*minigetactivetextitem*/

#endif


static boolean miniselectallandactivate (short newactiveitem) {
	
	(**minidata).activetextitem = newactiveitem;
	
	if (!minisetwpglobals (newactiveitem))
		return (false);
	
	wpselectall ();
	
	return (true);
	} /*miniselectallandactivate*/


static boolean miniselectall (void) {
	
	miniselectallandactivate ((**minidata).activetextitem);
	
	return (true);
	} /*miniselectall*/


static boolean mininextwindowvisit (WindowPtr w, ptrvoid refcon) {
	
	WindowPtr *nextwindow = (WindowPtr *) refcon;
	
	if (*nextwindow == nil) { /*we passed our target*/
		
		*nextwindow = w;
		
		return (false); /*stop visiting*/
		}
	
	if (w == *nextwindow) /*we're enountering our target*/
		*nextwindow = nil;
	
	return (true); /*keep visiting*/
	} /*mininextwindowvisit*/


static boolean minisearch (void) {
	
	/*
	this routine could be called "passthebuck".  we don't want to handle search 
	commands, but don't want them to fail either.  instead, we'll find the window 
	behind us and let it handle it
	*/
	
	boolean fl;
	WindowPtr nextwindow;
	
	nextwindow = miniwindow;
	
	if (shellvisitwindowlayer (&mininextwindowvisit, &nextwindow))
		return (false);
	
	shellpushglobals (nextwindow);
	
	fl = (*shellglobals.searchroutine) ();
	
	shellpopglobals ();
	
	return (fl);
	} /*minisearch*/


static boolean minicheckselectall (void) {

	register hdlminirecord hm = minidata;
	
	if ((**hm).flactive && (**hm).flselectallpending) {
		
		(**hm).flselectallpending = false; /*must be reset every time*/
		
		miniselectall ();
		
		return (true);
		}
	
	return (false);
	} /*minicheckselectall*/


static boolean minisavestrings (void) {
	
	/*
	5.0b17 dmb: minigetstring result is ours to consume
	*/
	
	Handle h;
	register short i;
	
	for (i = 0; i < (**minidata).cttextitems; i++) {
		
		minigetstring (i, &h);
		
		(*(**minidata).savestringroutine) (i, h);
		
		disposehandle (h);
		} /*for*/
	
	return (true);
	} /*minisavestrings*/


static void minidirty (void) {
	
	windowsetchanges (miniwindow, true);
	} /*minidirty*/


static boolean miniloadstrings (void) {
	
	Handle h;
	Rect r;
	hdlwprecord wp;
	register short i;
	
	for (i = 0; i < (**minidata).cttextitems; i++) {
		
		h = nil; /*in case callback is a no-op*/
		
		(*(**minidata).loadstringroutine) (i, &h);
		
		r = (**minidata).textrects [i];
		
		insetrect (&r, 3, 3);
		
		minisetwpglobals (i);
		
		wp = wpnewbuffer (nil, &r, &r, wpneverscroll, true);
		
		if (wp == nil)
			return (false);
		
		(**wp).flwindowopen = true;
		
		assert ((**wp).flneverscroll == true);
		
		(**minidata).textdata [i] = wp;
		
		minisetstring (i, h);
		
		(**wp).dirtyroutine = &minidirty; // must do after text is initialized
		
		disposehandle (h);
		
		(**minidata).activetextitem = i;
		} /*for*/
		
	return (true);
	} /*miniloadstrings*/


static boolean minisetvalue (void) {
	
	return ((*(**minidata).setvalueroutine) ());
	} /*minisetvalue*/


static void minidrawicon (boolean flpressed) {
	
	register hdlminirecord hm = minidata;
	register boolean flenabled;
	
	flenabled = (**hm).fliconenabled && (**hm).flactive;
	
	drawlabeledwindoidicon ((**hm).iconrect, (**hm).iconlabel, flenabled, flpressed);
	} /*minidrawicon*/


static void minidrawpopups (void) {
	
	register hdlminirecord hm = minidata;
	register short i;
	bigstring bs;
	
	for (i = 0; i < (**hm).ctpopups; i++) {
		
		copyheapstring ((**hm).popupmessages [i], bs);
		
		drawpopup ((**hm).popuprects [i], bs, false);
		} /*for*/
	} /*minidrawpopups*/


static void minidrawtextitems (void) {
	
	register hdlminirecord hm = minidata;
	register short i;
	
	#ifdef gray3Dlook
		pushbackcolor (&whitecolor);
	#endif

	for (i = 0; i < (**hm).cttextitems; i++) {
		
		#ifdef gray3Dlook
			eraserect ((**hm).textrects [i]);
		#endif
		
		framerect ((**hm).textrects [i]);
		
		if (minisetwpglobals (i))
			wpupdate ();
		} /*for*/

	#ifdef gray3Dlook
		popbackcolor ();
	#endif
	} /*minidrawtextitems*/


static boolean minidrawmsg (void) {
	
	register hdlminirecord hm = minidata;
	register boolean flbitmap;
	bigstring bs;
	Rect r;
	
	r = (**hm).msgrect;
	
	flbitmap = openbitmap (r, miniwindow);
		
	eraserect (r);
	
	pushstyle (msgfont, msgsize, msgstyle);
	
	copystring ((**hm).bsmsg, bs);
	
	centerstring (r, bs);
	
	popstyle ();
	
	framerect (r);
	
	if ((**hm).flactive && (**hm).flmsgactive) {
		
		insetrect (&r, 1, 1);
		
		#ifdef MACVERSION
		LMSetHiliteMode(LMGetHiliteMode() & ~(1 << hiliteBit));
		#endif
		
		invertrect (r);
		}
	
	if (flbitmap)
		closebitmap (miniwindow);
	
	return (true);
	} /*minidrawmsg*/


static boolean minimessage (bigstring bs, boolean flbackgroundmsg) {
#pragma unused (flbackgroundmsg)

	copystring (bs, (**minidata).bsmsg);
	
	return (minidrawmsg ());
	} /*minimessage*/


static boolean minigettargetdata (short id) {
	
	return ((*(**minidata).gettargetdataroutine) (id));
	} /*minigettargetdata*/


#if 0

static void minizoomtexttoicon (void) {
	
	Rect rfrom, rto;
	
	minigetactivetextrect (&rfrom);
	
	zerorect (&rto);
	
	centerrect (&rto, (**minidata).iconrect);
	
	localtoglobalrect (miniwindow, &rfrom);
	
	localtoglobalrect (miniwindow, &rto);
	
	zoomrect (&rfrom, &rto, true);
	} /*minizoomtexttoicon*/


static void minizoomicontomsg (void) {
	
	Rect rfrom, rto;
	
	rto = (**minidata).msgrect;
	
	zerorect (&rfrom);
	
	centerrect (&rfrom, (**minidata).iconrect);
	
	localtoglobalrect (miniwindow, &rfrom);
	
	localtoglobalrect (miniwindow, &rto);
	
	zoomrect (&rfrom, &rto, true);
	} /*minizoomicontomsg*/

#endif


static boolean miniiconhit (boolean flanimate) {
	
	/*
	7/23/90 DW: if flanimate is false, we don't bother displaying the run
	icon -- the caller wants the fastest-possible execution, or it's already 
	been tracked with the mouse
	
	2.1b11 dmb: check iconenabled flag in here, after calling enableroutine
	*/
	
	register hdlminirecord hm = minidata;
	
	(*(**hm).iconenableroutine) (); /*make sure it's up to date*/
	
	if (!(**hm).fliconenabled)
		return (false);
	
	if (flanimate) {
		
		minidrawicon (true);
		
		delayticks (8); /*so you can actually see the button down*/
		
		minidrawicon (false);
		}
	
	(*(**hm).iconhitroutine) ();
	
	(**hm).flselectallpending = true; 
	
	minisavestrings ();
	
	return (true);
	} /*miniiconhit*/


static short minifindpopup (Point pt){
	
	/*
	search the popuprects array, and return the index of the item whose rect
	contains the indicated point.
	
	we only look at the portion of the rect that displays the popup arrow.
	
	if the point isn't in any of the rects, return -1.
	*/

	register hdlminirecord hm = minidata;
	register short i;
	Rect r;
	
	for (i = 0; i < (**hm).ctpopups; i++) {
		
		r = (**hm).popuprects [i];
		
		r.left = r.right - popuparrowwidth;
		
		if (pointinrect (pt, r))
			return (i);
		} /*for*/
		
	return (-1);
	} /*minifindpopup*/


static boolean minipopuphit (short popupnumber) {
	
	register hdlminirecord hm = minidata;
	Rect r;
	
	(**hm).popupnumber = popupnumber; /*allow callback to use this*/
	
	r = (**hm).popuprects [popupnumber];
	
	r.left = r.right - popuparrowwidth;
	
	return (popupmenuhit (r, false, (**hm).fillpopuproutine, (**hm).popupselectroutine));
	} /*minipopuphit*/


static short minifindtextobject (Point pt) {
	
	/*
	search the textrects array, and return the index of the item whose rect
	contains the indicated point.
	
	if the point isn't in any of the rects, return -1.
	*/

	register hdlminirecord hm = minidata;
	register short i;
	
	for (i = 0; i < (**hm).cttextitems; i++) {
	
		if (pointinrect (pt, (**hm).textrects [i])) 
			return (i);
		} /*for*/
		
	return (-1);
	} /*minifindtextobject*/
	

static void miniactivatemessage (boolean flactivate) {
	
	register hdlminirecord hm = minidata;

	if ((**hm).flmsgactive != flactivate) {

		(**hm).flmsgactive = flactivate;

		if (flactivate) {
			
			wpactivate (false);
			
			(**hm).activetextitem = -1;
			}
		else {
			(**hm).activetextitem = 0;

			minisetactivewpglobals ();
			
			wpactivate (true);
			}

		minidrawmsg ();
		}
	} /*miniactivatemessage*/


static boolean minimessageisactive (void) {
	
	return ((**minidata).flmsgactive);
	} /*minimessageisactive*/


static void miniselectnexttextitem (void) {
	
	miniactivatemessage (!minimessageisactive ());
	} /*miniselectnexttextitem*/


static boolean minimousedown (Point pt, tyclickflags flags) {
	
	register hdlminirecord hm = minidata;
	short item;	
	Rect r;	
	
	minicheckselectall (); /*if a selectall is pending, do it now*/
	
	item = minifindtextobject (pt);
	
	if (item >= 0) {
		
		miniactivatemessage (false);
		
		(**hm).activetextitem = item;
		
		if (!minisetwpglobals (item))
			return (false);
		
		wpclick (pt, flags);
		
		(*(**hm).texthitroutine) (pt);
		
		shellevent.when = gettickcount ();
		
		mouseup (shellevent.when, shellevent.where.h, shellevent.where.v, shellevent.what); /*textedit may swallow the mouseup event when tracking click*/
		
		return (true);
		}
	
	if (pointinrect (pt, (**hm).msgrect)) {
		
		if (minisetactivewpglobals ())
			wpactivate (false);
		
		(**hm).activetextitem = -1;

		miniactivatemessage (true);
		}

	r = (**hm).iconrect;
	
	if (pointinrect (pt, r)) {
		
		if ((**hm).fliconenabled)
			if (trackicon (r, minidrawicon))
				return (miniiconhit (false));
		
		return (true);
		}
	
	bundle { /*check for mousedown in a popup*/
		
		register short x;
		
		x = minifindpopup (pt);
		
		if (x >= 0) { /*mousedown was in a popup*/
			
			return (minipopuphit (x));
			}
		}
	
	/*
	ouch ();
	*/
	
	return (true);
	} /*minimousedown*/


static boolean minikeystroke (void) {
	
	/*
	7/23/90 DW: improve demo-ability.  if the user is still holding down the
	enter key, we instruct miniiconhit not to use the delay he does to improve
	the animation of the run icon.  this allows us to show how fast the compiler
	and interpreter are by holding down the enter key.
	*/
	
	register char chkb = keyboardstatus.chkb;
	
	minicheckselectall (); /*if a selectall is pending, do it now*/
	
	if (chkb == chenter) { /*if enter key is down, don't delay*/
		
		miniiconhit (!keyboardstatus.flautokey);
		
		return (true);
		}
	
	if (chkb == chtab)
		miniselectnexttextitem ();
	else {
		
		if (minisetactivewpglobals ())
			wpkeystroke ();
		else
			sysbeep ();
		}
	
	return (true);
	} /*minikeystroke*/


static boolean miniruncursor (void) {
	
	/*
	2.1b11: might as well allow run selection verb fire off QuickScript
	*/
	
	return (miniiconhit (true));
	} /*miniruncursor*/


static boolean minigetwindowrect (Rect *rwindow) {
	
	return (ccgetwindowrect ((**minidata).windowtype, rwindow));
	} /*minigetwindowrect*/
	
	
static boolean miniclose (void) {
	
	minisavestrings ();
	
	killundo (); /*must toss undos before they're stranded*/
	
	if (!ccsubwindowclose (miniwindowinfo, (**minidata).windowtype))
		return (false);
	
	miniwindow = nil;
	
	return (true);
	} /*miniclose*/


static void miniresizetextrects (Rect rtext) {
	
	/*
	rtext covers all the space available for text objects, we distribute the
	text objects inside this rect.
	*/
	
	register hdlminirecord hm = minidata;
	register short ct = (**hm).cttextitems;
	register short objheight;
	register short i;
	register short nexttop;
	register short spacebetween;
	
	spacebetween = windowmargin / 2;
	
	objheight = (rtext.bottom - rtext.top - ((ct - 1) * spacebetween)) / ct;
	
	nexttop = rtext.top;
	
	for (i = 0; i < ct; i++) {
		
		Rect r = rtext;
		
		r.top = nexttop;
		
		r.bottom = r.top + objheight;
		
		(**hm).textrects [i] = r;
		
		insetrect (&r, 3, 3); /*make up for dialog manager funkiness*/
		
		minisetwpglobals (i);
		
		wpsetbufferrect (r, r);
		
		nexttop += objheight + spacebetween;
		} /*for*/
	} /*miniresizetextrects*/


static boolean miniresizepopups (Rect *rmsg) {
	
	register hdlminirecord hm = minidata;
	Rect r = *rmsg;
	Rect rpopup;
	register short i;
	
	for (i = 0; i < (**hm).ctpopups; i++) {
		
		rpopup = r; /*set top, bottom, left*/
		
		rpopup.right = rpopup.left + popupwidth;
		
		(**hm).popuprects [i] = rpopup;
		
		r.left = rpopup.right + (windowmargin / 2);
		} /*for*/
		
	*rmsg = r;
	
	return (true);
	} /*miniresizepopups*/


static void miniresize (void) {
	
	register hdlwindowinfo hw = miniwindowinfo;
	register hdlminirecord hm = minidata;
	Rect rcontent;
	Rect r;
	
	rcontent = (**hw).contentrect;
	
	r.top = rcontent.top + windowmargin;
	
	r.bottom = r.top + iconrectheight;
	
	r.right = rcontent.right - windowmargin;
	
	r.left = r.right - iconrectwidth;
	
	(**hm).iconrect = r; 
	
	r.bottom = rcontent.bottom - windowmargin;
	
	r.top = r.bottom - popupheight;
	
	r.left = rcontent.left + windowmargin;
	
	r.right = (**hm).iconrect.left - windowmargin;
	
	miniresizepopups (&r); /*r returns with msgrect*/
	
	(**hm).msgrect = r;
	
	r.left = rcontent.left + windowmargin;
	
	r.right = (**hm).iconrect.left - windowmargin;

	r.top = rcontent.top + windowmargin;
	
	r.bottom = (**hm).msgrect.top - (windowmargin / 2);
	
	miniresizetextrects (r); /*left and right are the same as for msgrect*/
	
	#if TARGET_API_MAC_CARBON == 1
	
		drawthemeborder ((**hm).textrects [0], (**hw).contentrect); /*PBS 7.0b52: draw scan-lines border*/
		
	#endif
	
	//(**hm).flmassiveupdate = true; /*it don't get much more massive than this!*/
	} /*miniresize*/
	

static boolean minisetfontsize (void) {
	
	register hdlwindowinfo hw = miniwindowinfo;
	
	if (minisetactivewpglobals ()) {
		
		wpselectall ();
		
		wpsetfont ();
		
		wpsetsize ();
		}
	
	cccopywindowinfo (hw, (**minidata).windowtype); /*update cancoondata*/
	
	return (true);
	} /*minisetfontsize*/

	
static boolean mininewwindow (callback setuproutine) {
	
	/*
	5.0a18 dmb: use ccfindrootwindow instead of frontrootwindow, as
	we do in minifindwindow. not just for about, but for guest roots
	*/

	register hdlminirecord hm;
	bigstring bs;
	Rect rzoom, rwindow;
	hdlwindowinfo hinfo, hparent;
	WindowPtr w;
	
	if (!ccfindrootwindow (&hparent)) /*our parent is the active root window*/
		return (false);
	
	/*shellgetwindowcenter (hparent, &rzoom);*/
	
	rzoom.top = -1; /*zoom from default rect*/
	
	if (!newclearhandle (longsizeof (tyminirecord), (Handle *) &minidata))
		return (false);
		
	hm = minidata; /*copy into register*/
	
	(**hm).savestringroutine = (minisavestringcallback) &truenoop;
	
	(**hm).loadstringroutine = (miniloadstringcallback) &truenoop;
	
	(**hm).texthitroutine = (minitexthitcallback) &truenoop;
	
	(**hm).iconenableroutine = (shellvoidcallback) &truenoop;
	
	(**hm).iconhitroutine = &truenoop;
	
	(**hm).setvalueroutine = &truenoop;
	
	(**hm).fillpopuproutine = (fillpopupcallback) &truenoop;
	
	(**hm).popupselectroutine = (popupselectcallback) &truenoop;
	
	(**hm).gettargetdataroutine = (shellshortcallback) &falsenoop;
	
	#ifdef MACVERSION
		(**hm).forecolor = blackColor;
		
		(**hm).backcolor = whiteColor;
	#endif

	(**hm).activetextitem = -1;
	
	(*setuproutine) (); /*callback to fill in user-defined fields of minidata*/
	
	minigetwindowrect (&rwindow);
	
	copystring ((**hm).windowtitle, bs);
	
	if (!newchildwindow ((**hm).idconfig, hparent, &rwindow, &rzoom, bs, &w)) {
		
		disposehandle ((Handle) hm);
		
		return (false);
		}
	
	getwindowinfo (w, &hinfo);
	
	windowsetcolor (w, (**hm).forecolor, true);
	
	windowsetcolor (w, (**hm).backcolor, false);
	
	(**hinfo).hdata = (Handle) hm;
	
	ccnewsubwindow (hinfo, (**hm).windowtype);
	
	shellpushglobals (w); 
	
	setfontsizestyle ((**hinfo).defaultfont, (**hinfo).defaultsize, (**hinfo).defaultstyle);
		
	miniresize ();
	
	minisetfontsize ();
	
	minisetvalue ();
	
	miniloadstrings ();
	
	(**hm).flselectallpending = true; /*selectall on idle*/
		
	windowzoom (w);
	
	#if TARGET_API_MAC_CARBON == 1 /*Make sure the window is fully drawn.*/
		miniresize ();
	#endif

	shellpopglobals ();
	
	return (true);
	} /*mininewwindow*/


static void miniupdate (void) {
	
	register hdlminirecord hm = minidata;
	
	(*(**hm).iconenableroutine) ();
	
	minidrawicon (false);
	
	minidrawpopups ();
	
	minidrawtextitems ();
	
	minidrawmsg ();
	} /*miniupdate*/


static void miniactivate (boolean flactivate) {
	
	minicheckselectall (); /*if a selectall is pending, do it now*/
	
	(**minidata).flactive = flactivate;
	
	if (minimessageisactive ())
		;
	else
		wpactivate (flactivate);
	
	minidrawicon (false);
	} /*miniactivate*/


static boolean minifindvisit (WindowPtr w, ptrvoid refcon) {
	
	/*
	if we're called, the w is a window with the desired config, owned 
	the the current root.
	
	set up mini globals and return false to terminate the visit
	*/
	
	hdlwindowinfo *hinfo = (hdlwindowinfo *) refcon;
	 
	miniwindow = w;
	
	getwindowinfo (w, hinfo);
	
	return (false);
	} /*minifindvisit*/


static boolean minifindwindow (short idconfig, hdlwindowinfo *hinfo) {
	
	/*
	5/22/91 dmb: use customized visit to limit search to windows belonging to the 
	current root.  this allows us to distinguish between the quickscript window 
	for each open file.
	
	5.0a2 dmb: use ccfindrootwindow instead of frontrootwindow to handle about

	5.0a5 dmb: take hinfo parameter; don't use globals
	*/
	
	hdlwindowinfo hroot;
	
	// if (!frontrootwindow (&hroot))
	if (!ccfindrootwindow (&hroot))
		return (false);
	
	return (!visitownedwindows ((**hroot).macwindow, idconfig, &minifindvisit, hinfo));
	
	/*
	return (shellfindwindow (idconfig, &miniwindow, &miniwindowinfo, (Handle *) &minidata));
	*/
	} /*minifindwindow*/


boolean miniinvalicon (short idconfig) {
	
	hdlwindowinfo hinfo;

	if (!minifindwindow (idconfig, &hinfo))
		return (false);
	
	shellpushwindowglobals (hinfo);

	invalwindowrect (miniwindow, (**minidata).iconrect);
	
	shellpopglobals ();

	return (true);
	} /*miniinvalicon*/


boolean minisetwindowmessage (short idconfig, bigstring bsmessage) {
	
	/*
	use this routine to set the window message when you don't know if the 
	window is around
	*/
	
	hdlwindowinfo hinfo;

	if (!minifindwindow (idconfig, &hinfo))
		return (false);
	
	if (!shellpushwindowglobals (hinfo))
		return (false);
	
	shellwindowmessage (bsmessage);
	
	shellpopglobals ();
	
	return (true);
	} /*minisetwindowmessage*/


boolean startminidialog (short idconfig, callback setuproutine) {
	
	hdlwindowinfo hinfo;
	
	if (minifindwindow (idconfig, &hinfo)) { /*sets minidata, miniwindow*/
		
		shellbringtofront (hinfo);
		
		// minisetvalue ();
		
		// miniloadstrings ();
		
		shellpushwindowglobals (hinfo);

		(**minidata).flselectallpending = true; /*selectall on idle*/
		
		shellpopglobals ();

		return (true);
		}
	
	return (mininewwindow (setuproutine));
	} /*startminidialog*/


static boolean minidisposerecord (void) {
	
	register hdlminirecord hm = minidata;
	register short i;
	
	for (i = 0; i < (**hm).ctpopups; i++)
		disposehandle ((Handle) (**hm).popupmessages [i]);
	
	for (i = i; i < (**hm).cttextitems; i++)
		wpdisposerecord ((**hm).textdata [i]);
	
	disposehandle ((Handle) hm);
	
	return (true);
	} /*minidisposerecord*/


static boolean miniadjustcursor (Point pt){
	
	if (minifindtextobject (pt) >= 0) {
			
		setcursortype (cursorisibeam);
			
		return (true);
		}
		
	if (minifindpopup (pt) >= 0) {
		
		setcursortype (cursorispopup);
		
		return (true);
		}
		
	setcursortype (cursorisarrow);
	
	return (true);
	} /*miniadjustcursor*/
	

static boolean minisetfont (void) {
	
	hdlwindowinfo hw = miniwindowinfo;
	
	(**hw).defaultfont = (**hw).selectioninfo.fontnum;
	
	(**hw).selectioninfo.fldirty = true;
	
	return (minisetfontsize ());
	} /*minisetfont*/
	

static boolean minisetsize (void) {
	
	hdlwindowinfo hw = miniwindowinfo;
	
	(**hw).defaultsize = (**hw).selectioninfo.fontsize;
	
	(**hw).selectioninfo.fldirty = true;
	
	return (minisetfontsize ());
	} /*minisetsize*/


static boolean minisetselectioninfo (void) {
	
	shelldefaultselectioninfo ();
	
	(**miniwindowinfo).selectioninfo.flcansetstyle = false;
	
	return (true);
	} /*minisetselectioninfo*/


static void miniidle (void) {
	
	minicheckselectall ();
	
	wpidle ();
	} /*miniidle*/


static boolean minicopy (void) {
	
	if (minimessageisactive ()) {

		bigstring bsmsg;
		Handle htext;
		
		copystring ((**minidata).bsmsg, bsmsg);
		
		if (!newtexthandle (bsmsg, &htext))
			return (false);
		
		return (shellsetscrap (htext, textscraptype, (shelldisposescrapcallback) disposehandle, nil));
		}
	else
		return (wpcopy ());
	} /*minicopy*/


static boolean minipaste (void) {
	
	if (!minisetactivewpglobals ())
		return (false);

	return (wppaste (false));
	} /*minipaste*/


static boolean miniclear (void) {
	
	if (!minisetactivewpglobals ())
		return (false);

	return (wpclear ());
	} /*miniclear*/


static boolean minicut (void) {
	
	if (minimessageisactive ())
		return (minicopy ());

	if (!minisetactivewpglobals ())
		return (false);

	return (wpcut ());
	} /*minicut*/


static boolean minigetundoglobals (long *globals) {
	
	*globals = (long) (**minidata).activetextitem;
	
	return (true);
	} /*opeditgetundoglobals*/


static boolean minisetundoglobals (long globals, boolean flundo) {
	
	/*
	7/2/91 dmb: now takes flundo parameter.  since wpengine doesn't need its 
	globals set up to dispose an undo, we can just exit when flundo is 
	false.  if this weren't the case, we'd have to store the wpdata handle 
	in the undoglobals to avoid having to recreate the context when just 
	tossing an undo.
	*/
	
	if (!flundo)
		return (true);
	
	miniactivatemessage (false);
	
	return (minisetwpglobals (globals));
	} /*opeditsetundoglobals*/


boolean ministart (short idconfig) {
	
	/*
	set up callback routines record, and link our data into the shell's 
	data structure.
	
	2.1b11 dmb: hook up executeroutine
	*/
	
	ptrcallbacks minicallbacks;
	register ptrcallbacks cb;
	
	shellnewcallbacks (&minicallbacks);
	
	cb = minicallbacks; /*copy into register*/
	
	loadconfigresource (idconfig, &(*cb).config);
		
	(*cb).configresnum = idconfig;
	
	(*cb).windowholder = &miniwindow;
	
	(*cb).dataholder = (Handle *) &minidata;
	
	(*cb).infoholder = &miniwindowinfo; 
	
	(*cb).setglobalsroutine = &minisetactivewpglobals;
	
	(*cb).activateroutine = &miniactivate;
	
	(*cb).updateroutine = &miniupdate;
	
	(*cb).resizeroutine = &miniresize;
	
	(*cb).mouseroutine = &minimousedown;
	
	(*cb).keystrokeroutine = &minikeystroke;
	
	(*cb).executeroutine = &miniruncursor;
	
	(*cb).disposerecordroutine = &minidisposerecord;
	
	(*cb).closeroutine = &miniclose;
	
	(*cb).presaveroutine = &minisavestrings;
	
	(*cb).adjustcursorroutine = &miniadjustcursor;
	
	(*cb).idleroutine = &miniidle;
	
	(*cb).cutroutine = &minicut;
	
	(*cb).copyroutine = &minicopy;
	
	(*cb).pasteroutine = &minipaste;
	
	(*cb).clearroutine = &miniclear;
	
	(*cb).fontroutine = &minisetfont;
	
	(*cb).sizeroutine = &minisetsize;
	
	(*cb).setselectioninforoutine = &minisetselectioninfo;
	
	(*cb).selectallroutine = &miniselectall;
	
	(*cb).searchroutine = &minisearch;
	
	(*cb).msgroutine = &minimessage;
	
	(*cb).gettargetdataroutine = &minigettargetdata;
	
	(*cb).setundoglobalsroutine = &minisetundoglobals;
	
	(*cb).getundoglobalsroutine = &minigetundoglobals;
	
	return (true);
	} /*ministart*/


boolean minireloadstrings ( short idconfig ) {

	//
	// reload the strings if the window is around
	//
	// 2007-07-27 creedon: created
	//
	
	hdlwindowinfo hinfo;
	
	if ( minifindwindow ( idconfig, &hinfo ) ) {
	
		shellpushwindowglobals ( hinfo );
		
		miniloadstrings ( );
		
		( **minidata ).flselectallpending = true; // select all on idle
		
		shellpopglobals ( );
		
		return ( true );
		
		}
	
	return ( false );
	
	} // minireloadstrings

