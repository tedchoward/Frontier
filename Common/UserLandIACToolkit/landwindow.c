
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

#include "landinternal.h"





typedef struct tystatsrecord {

	long watchforthis; /*see comment in landopenstatswindow*/
	
	short lineheight;
	
	short h, v;
	
	short ctlines;
	
	byte bstitle [32];
	
	tydirection dirflush;
	
	boolean flactive: 1; /*if true the window is active, false if not*/
	
	boolean flincoming: 1; /*if true, stats shows verb being received, otherwise being sent*/
	
	tyverbclass verbclass; /*4-byte class, often same as receiver id*/
	
	tyverbtoken verbtoken; /*4-byte token, e.g. 'save', 'go  '*/
	
	tyapplicationid idother; /*signature of the sender or receiver*/
	
	short ctparams; /*number of items in the params array*/
	
	long ctverbsin; /*total number of verbs received by this app*/
	
	long ctverbsout; /*total number of verbs sent by this app*/
	
	/*remaining fields are for offscreen bitmaps*/
	
	BitMap offscreenbitmap, savedbitmap;

	Handle bitmapbasehandle;

	boolean flbitmapsenabled: 1;

	boolean flbitmapopen: 1;
	} tystatsrecord, *ptrstatsrecord, **hdlstatsrecord;
	

static boolean landopenbitmap (void) {

	register hdllandglobals hg = landgetglobals ();
	register WindowPtr w = (**hg).statswindow;
	register hdlstatsrecord hstats;
	register short nrowbytes;
	register long nboxbytes;
	register long sizehandle;	
	Rect r;
	
	if (w == nil) /*defensive driving*/
		return (false);
	
	r = (*w).portRect;
	
	hstats = (hdlstatsrecord) (*(WindowPeek) w).refCon;
	
	(**hstats).flbitmapopen = false;
	
	if (!(**hstats).flbitmapsenabled) /*application doesn't want any bitmaps*/
		return (false);
	
	nrowbytes = (r.right - r.left + 7) / 8;
	
	if ((nrowbytes % 2) == 1) /*odd number*/
		nrowbytes++;
	
	nboxbytes = (r.bottom - r.top) * nrowbytes;
	
	SetHandleSize ((**hstats).bitmapbasehandle, nboxbytes);

	sizehandle = GetHandleSize ((**hstats).bitmapbasehandle);

	if (sizehandle < nboxbytes) {
	
 		SetHandleSize ((**hstats).bitmapbasehandle, 10L);

		return (false);
		}

	HLock ((**hstats).bitmapbasehandle);

	(**hstats).offscreenbitmap.baseAddr = *(**hstats).bitmapbasehandle;

	(**hstats).offscreenbitmap.rowBytes = nrowbytes;

	(**hstats).offscreenbitmap.bounds = r;

	(**hstats).savedbitmap = (*w).portBits;

	CopyBits (&(**hstats).savedbitmap, &(**hstats).offscreenbitmap, &r, &r, srcCopy, nil);
	
	SetPortBits (&(**hstats).offscreenbitmap);
	
	(**hstats).flbitmapopen = true; /*remember in our own local global*/

	return (true);
	} /*landopenbitmap*/


static landclosebitmap (void) {

	register hdllandglobals hg = landgetglobals ();
	register WindowPtr w = (**hg).statswindow;
	register hdlstatsrecord hstats;
	
	if (w == nil) /*defensive driving*/
		return (false);
	
	hstats = (hdlstatsrecord) (*(WindowPeek) w).refCon;
	
	if ((**hstats).flbitmapsenabled && (**hstats).flbitmapopen) {
		
		(**hstats).flbitmapopen = false;
		
		SetPortBits (&(**hstats).savedbitmap);
	
		CopyBits (
			&(**hstats).offscreenbitmap, &((*w).portBits), &(**hstats).offscreenbitmap.bounds, 
			
			&(**hstats).offscreenbitmap.bounds, srcCopy, nil);
		
		HUnlock ((**hstats).bitmapbasehandle);
		
		(**hstats).offscreenbitmap.baseAddr = 0;
		
		SetHandleSize ((**hstats).bitmapbasehandle, 10L);
		}
	} /*landclosebitmap*/


static landinitbitmaps (void) {

	register hdllandglobals hg = landgetglobals ();
	register WindowPtr w = (**hg).statswindow;
	register hdlstatsrecord hstats;
	
	if (w == nil) /*defensive driving*/
		return (false);
	
	hstats = (hdlstatsrecord) (*(WindowPeek) w).refCon;
	
	(**hstats).flbitmapsenabled = true;
	
	if ((**hstats).flbitmapsenabled) {
	
		(**hstats).bitmapbasehandle = NewHandle (10L); 
		
		(**hstats).offscreenbitmap.baseAddr = 0;
		
		(**hstats).offscreenbitmap.rowBytes = 2;
		
		SetRect (&(**hstats).offscreenbitmap.bounds, 0, 0, 0, 0);
		}
	} /*landinitbitmaps*/


static boolean landsetstyle (fnum, fsize, fstyle, ascent, descent) short fnum, fsize, fstyle, *ascent, *descent; {
	
	FontInfo info;
	
	TextFont (fnum);
	
	TextSize (fsize);
	
	TextFace (fstyle);
	
	GetFontInfo (&info);
	
	*ascent = info.ascent;
	
	*descent = info.descent;
	
	return (true);
	} /*landsetstyle*/


static boolean landinitstatswindow (void) {
	
	/*
	preserves the port, does not preserve the font/size/style of the stats
	window port.  doesn't preserve the pen style.
	*/

	register hdllandglobals hg = landgetglobals ();
	register WindowPtr w = (**hg).statswindow;
	register hdlstatsrecord hstats;
	GrafPtr savedport;
	short ascent, descent;
	register short lh;
	register short ctvertpixels;
	Rect r;
	
	if (w == nil) /*defensive driving*/
		return (false);
	
	hstats = (hdlstatsrecord) (*(WindowPeek) w).refCon;
	
	savedport = quickdrawglobal (thePort); /*we preserve the port*/
	
	SetPort (w);
	
	landsetstyle (geneva, 9, 0, &ascent, &descent);
	
	lh = ascent + descent + 3;
	
	(**hstats).lineheight = lh;
	
	ctvertpixels = lh * (**hstats).ctlines + (lh / 2) + 2;
	
	r = (*w).portRect;
	
	r.top += lh / 2;
	
	r.bottom -= lh / 2;
	
	r.left += 5;
	
	r.right -= 5;
	
	switch ((**hstats).dirflush) {
		
		case up: /*flush with the top of the window*/
			r.bottom = r.top + ctvertpixels;
			
			break;
		
		case down: /*flush with the bottom of the window*/
			r.top = r.bottom - ctvertpixels;
			
			break;
			
		} /*switch*/
	
	EraseRect (&r);
	
	if ((**hstats).flactive)
		PenPat (quickdrawglobal (gray));
	else
		PenPat (quickdrawglobal (white));
	
	MoveTo (r.left, r.top);
	
	LineTo (r.left + 8, r.top);
	
	MoveTo (r.left + 8, r.top + 3);
		
	landsetstyle (geneva, 9, bold, &ascent, &descent);
	
	DrawString ((**hstats).bstitle);
	
	MoveTo (r.left + 8 + StringWidth ((**hstats).bstitle), r.top);
	
	LineTo (r.right, r.top);
	
	LineTo (r.right, r.bottom);
	
	LineTo (r.left, r.bottom);
	
	LineTo (r.left, r.top);
	
	(**hstats).v = r.top + ascent + (lh / 2) + 2;
	
	(**hstats).h = r.left + 8;
	
	SetPort (savedport); /*restore the quickdraw port*/
	} /*landinitstatswindow*/


pascal boolean landopenstatswindow (void) {
	
	/*
	after calling this guy, the Toolkit stats window is open and ready
	to display status information about the Toolkit layer.  the refcon
	of this window points to a statsrecord.
	
	if you're running thru your window list and come across a refcon whose
	first four bytes are 'LAND' -- it's our window, not one of yours.
	*/
	
	register hdllandglobals hg = landgetglobals ();
	register WindowPtr w = (**hg).statswindow;
	hdlstatsrecord hstats;
	bigstring bs;
	Rect r;
	
	if (w != nil) { /*window is already open*/
		
		SelectWindow (w);
		
		return (true);
		}
	
	if (!landnewclearhandle (longsizeof (tystatsrecord), (Handle *) &hstats))
		return (false);
	
	SetRect (&r, 100, 100, 425, 210);
	
	landcopystring ("\pUserLand IAC Toolkit", bs);
	
	w = NewWindow (nil, &r, bs, false, noGrowDocProc, (WindowPtr) -1, true, 0);
	
	if (w == nil) { /*failed to allocate window*/
		
		landdisposehandle ((Handle) hstats);
		
		return (false);
		}
		
	(**hg).statswindow = w;
	
	(*(WindowPeek) w).refCon = (long) hstats; /*window points at stats record*/
	
	(**hstats).watchforthis = 'LAND';
	
	(**hstats).flactive = true;
	
	landinitbitmaps ();
	
	ShowWindow (w);
	
	return (true);
	} /*landopenstatswindow*/


pascal boolean landclosestatswindow (void) {

	register hdllandglobals hg = landgetglobals ();
	register WindowPtr w = (**hg).statswindow;

	if (w == nil) /*it's already closed*/
		return (true);
	
	landdisposehandle ((Handle) (*(WindowPeek) w).refCon);
	
	DisposeWindow (w);
	
	(**hg).statswindow = nil;
	
	return (true);
	} /*landclosestatswindow*/


pascal boolean landisstatswindow (WindowPtr w) {
	
	/*
	return true if it's the stats window.  this is part of the Toolkit API because
	applications will encounter our window in a traversal of its window list.  if we
	return true it definitely is the stats window, and the application should make
	no assumptions about the handle hanging off the refcon field of the window.
	*/
	
	register hdllandglobals hg = landgetglobals ();
	register WindowPtr statswindow = (**hg).statswindow;

	if (statswindow == nil) /*the stats window isn't open*/
		return (false);
	
	return (w == statswindow);
	} /*landisstatswindow*/


boolean landstatswindowisopen (void) {

	register hdllandglobals hg = landgetglobals ();
	
	return ((**hg).statswindow != nil);
	} /*landstatswindowisopen*/


static boolean landstatswindowline (bigstring bs) {

	register hdllandglobals hg = landgetglobals ();
	register WindowPtr w = (**hg).statswindow;
	register hdlstatsrecord hstats;
	GrafPtr savedport;
	short ascent, descent;

	if (w == nil) /*defensive driving*/
		return (false);
	
	savedport = quickdrawglobal (thePort); /*we preserve the port*/
	
	SetPort (w);
	
	hstats = (hdlstatsrecord) (*(WindowPeek) w).refCon;

	landsetstyle (geneva, 9, 0, &ascent, &descent);
	
	MoveTo ((**hstats).h, (**hstats).v);
	
	DrawString (bs);
	
	(**hstats).v += (**hstats).lineheight;
	
	SetPort (savedport);
	
	return (true);
	} /*landstatswindowline*/


static boolean landupdatestatswindow (void) {

	register hdllandglobals hg = landgetglobals ();
	register WindowPtr w = (**hg).statswindow;
	register hdlstatsrecord hstats;
	bigstring bs, bsid;
	
	if (w == nil) /*defensive driving*/
		return (false);
	
	hstats = (hdlstatsrecord) (*(WindowPeek) w).refCon;
	
	/*top half of window*/ {
	
		(**hstats).dirflush = up;
		
		(**hstats).ctlines = 2;
		
		landcopystring ("\pIAC Messages:", (**hstats).bstitle);
			
		landinitstatswindow ();
		
		/*line #1*/ {
		
			landcopystring ("\pLast message was '", bs);
			
			land4bytestostring ((**hstats).verbclass, bsid);
			
			landpushstring (bsid, bs);
			
			landpushstring ("\p.", bs);
			
			land4bytestostring ((**hstats).verbtoken, bsid);
			
			landpushstring (bsid, bs);
			
			landpushstring ("\p' ", bs);
			
			if ((**hstats).flincoming)
				landpushstring ("\p received", bs);
			else
				landpushstring ("\p sent", bs);
		
			landpushstring ("\p with ", bs);
			
			landpushlong ((long) (**hstats).ctparams, bs);
			
			landpushstring ("\p params.", bs);
		
			landstatswindowline (bs);
			} /*line #1*/
			
		/*line #2*/ {
		
			setemptystring (bs);
			
			landpushlong ((**hstats).ctverbsout, bs);
			
			landpushstring ("\p verbs sent; ", bs);
			
			landpushlong ((**hstats).ctverbsin, bs);
			
			landpushstring ("\p verbs received.", bs);
			
			landstatswindowline (bs);
			} /*line #2*/
		} /*top half of stats window*/

	/*bottom half of stats window*/ {
	
		(**hstats).dirflush = down;
		
		(**hstats).ctlines = 2;
		
		landcopystring ("\pSystem Info:", (**hstats).bstitle);
		
		landinitstatswindow ();
		
		/*line #1*/ {
			register long freemem = FreeMem ();
			
			setemptystring (bs);
			
			landpushlong (freemem / 1024, bs);
			
			landpushstring ("\pK free; ", bs);
			
			landpushlong ((**hg).initialfreemem - freemem, bs);
			
			landpushstring ("\p bytes difference since LandInit.", bs);
			
			landstatswindowline (bs);
			}
			
		/*line #2*/ {

			setemptystring (bs);
			
			landpushstring ("\pIAC name = '", bs);
			
			land4bytestostring ((**hg).applicationid, bsid);
			
			landpushstring (bsid, bs);
			
			if ((**hg).transport == macsystem6) {
			
				landpushstring ("\p'; max queue depth = ", bs);
			
				landpushlong ((long) (**hg).maxqueuedepth, bs);
			
				landpushstring ("\p.", bs);
				}
			
			landstatswindowline (bs);
			}
		} /*bottom half of window*/
	
	return (true);	
	} /*landupdatestatswindow*/


static boolean landbitmapupdate (void) {
	
	register hdllandglobals hg = landgetglobals ();
	register WindowPtr w = (**hg).statswindow;
	GrafPtr savedport;

	savedport = quickdrawglobal (thePort); /*we preserve the port*/
	
	SetPort (w);
	
	landopenbitmap ();
	
	landupdatestatswindow ();
	
	landclosebitmap ();
	
	SetPort (savedport);
	
	return (true);
	} /*landbitmapupdate*/


boolean landsetstatsinfo (hdlverbrecord hverb, boolean flincoming) {

	register hdllandglobals hg = landgetglobals ();
	register WindowPtr w = (**hg).statswindow;
	register hdlstatsrecord hstats;
	register hdlverbrecord hv = hverb;
	
	if (w == nil) /*defensive driving*/
		return (false);
	
	hstats = (hdlstatsrecord) (*(WindowPeek) w).refCon;
	
	(**hstats).verbclass = (**hv).verbclass;
	
	(**hstats).verbtoken = (**hv).verbtoken;
	
	(**hstats).flincoming = flincoming;
	
	if (flincoming) {
	
		(**hstats).idother = (**hv).idsender;
		
		(**hstats).ctverbsin++;
		}
	else {
	
		(**hstats).idother = (**hv).idreceiver;		
		
		(**hstats).ctverbsout++;
		}

	(**hstats).ctparams = (**hv).ctparams;
		
	landbitmapupdate ();
	
	return (true);
	} /*landsetstatsinfo*/


landsetmemstats (void) {

	register hdllandglobals hg = landgetglobals ();

	(**hg).initialfreemem = FreeMem (); /*after everything alloc'd, for stats window*/
	} /*landsetmemstats*/


static landwindowmousedown (EventRecord *ev) {

	register short part;
	WindowPtr w;
	
	part = FindWindow ((*ev).where, &w);
	
	if (w != nil) {
	
		if (w != FrontWindow ()) { /*just like all other Mac programs*/
			
			SelectWindow (w);
							
			return (true); 
			}
		}
	
	switch (part) {
	
		case inGoAway:
			if (TrackGoAway (w, (*ev).where)) 
				landclosestatswindow ();
								
			break;
			
		case inDrag: {
			Rect r;

			r = quickdrawglobal (screenBits).bounds; 
			
			r.top += GetMBarHeight (); 
			
			r.left += 4;
			
			r.right -= 4;
			
			r.bottom -= 4;
			
			DragWindow (w, (*ev).where, &r);
			}
		} /*switch*/
		
	return (true);
	} /*landwindowmousedown*/


static boolean landactivatestatswindow (boolean flactive) {

	register hdllandglobals hg = landgetglobals ();
	register WindowPtr w = (**hg).statswindow;
	register hdlstatsrecord hstats;
	
	if (w == nil) /*defensive driving*/
		return (false);
	
	hstats = (hdlstatsrecord) (*(WindowPeek) w).refCon;
	
	(**hstats).flactive = flactive;
	
	landbitmapupdate ();
	
	return (true);
	} /*landactivatestatswindow*/


boolean landwindoweventfilter (EventRecord *ev) {
	
	/*
	return true if we consume the event.
	*/

	register hdllandglobals hg = landgetglobals ();
	register WindowPtr w = (**hg).statswindow;
	register WindowPtr eventwindow = (WindowPtr) (*ev).message;
	
	if (w == nil) /*there is no stats window, event can't be for us*/
		return (false);
		
	if (eventwindow == nil) { /*it's nil if it's a mousedown in closebox, at least*/
		
		WindowPtr x;
		
 		FindWindow ((*ev).where, &x);
 		
 		eventwindow = x;
 		}
		
	if (eventwindow != w) /*the event is for another window*/
		return (false);
		
	if (w == FrontWindow ()) /*we always have an arrow cursor*/
		landsetcursortype (0);
	
	switch ((*ev).what) {
	
		case mouseDown:
			landwindowmousedown (ev);
			
			return (true); 
			
		case updateEvt:
			BeginUpdate (w);
			
			landupdatestatswindow ();
			
			EndUpdate (w);
			
			return (true); 
			
		case activateEvt:
			landactivatestatswindow ((*ev).modifiers & activeFlag);
			
			return (true);
		} /*switch*/
	
	return (false); /*don't consume the event*/
	} /*landwindoweventfilter*/



