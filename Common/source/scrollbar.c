
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

#include "frontierconfig.h"
#include "memory.h"
#include "shell.rsrc.h"
#include "scrollbar.h"
#include "quickdraw.h"
#include "threads.h"


#ifdef MACVERSION
#define scrollbarpopclip() popclip()
#endif
#ifdef WIN95VERSION
#define scrollbarpopclip()
#endif

static boolean flmacproportionalthumbs = false; /*7.0b18 PBS*/

static boolean scrollbarpushclip (hdlscrollbar hscrollbar) {
#ifdef MACVERSION	
	/*
	11/19/90 DW: patch things up for the table displayer, and perhaps others in
	the future.  if the scrollbar is of trivial height, we disable the drawing
	that's about to happen.  we were getting one-pixel high scrollbars being
	drawn.
	
	2/28/91 dmb: now that pushclip does true clip nesting, we use superpushclip 
	to ensure that the scrollbar gets drawn
	*/
	
	//Code change by Timothy Paustian Friday, May 5, 2000 10:10:47 PM
	//Changed to Opaque call for Carbon
	Rect r;
	#if ACCESSOR_CALLS_ARE_FUNCTIONS == 1
	GetControlBounds(hscrollbar, &r);
	#else
	r = (**hscrollbar).contrlRect;
	#endif
	
	if ((r.bottom - r.top) == 1)
		r.bottom = r.top;
		
	return (superpushclip (r));
#endif

#ifdef WIN95VERSION
	return (true);
#endif
	} /*scrollbarpushclip*/
	
	
void validscrollbar (hdlscrollbar hscrollbar) {
#ifdef MACVERSION
	register hdlscrollbar h = hscrollbar;
	//Code change by Timothy Paustian Friday, May 5, 2000 10:13:48 PM
	//Changed to Opaque call for Carbon
	
	#if ACCESSOR_CALLS_ARE_FUNCTIONS == 1
	CGrafPtr	contrlOwner = GetControlOwner(h);
	Rect		contrlRect;
	
	//assert (h == nil || GetQDGlobalsThePort() == contrlOwner);
	if ((h != nil) && (GetQDGlobalsThePort() == contrlOwner)) 
	{
		GetControlBounds(h, &contrlRect);
		validrect (contrlRect);
	}
	#else
	assert (h == nil || qd.thePort == (**h).contrlOwner);
	if (h != nil) 
		validrect ((**h).contrlRect);
	#endif
		
	
#endif
	} /*validscrollbar*/
	
	
static void invalscrollbar (hdlscrollbar hscrollbar) {
#ifdef MACVERSION
	register hdlscrollbar h = hscrollbar;
	//Code change by Timothy Paustian Friday, May 5, 2000 10:16:35 PM
	//Changed to Opaque call for Carbon
	//This routine is never called in the PPC OT version.
	if (h != nil) 
	{	
		#if ACCESSOR_CALLS_ARE_FUNCTIONS == 1
		Rect contrlRect;
		GetControlBounds(h, &contrlRect);
		invalrect (contrlRect);
		#else
		invalrect((**h).contrlRect);
		#endif
	}
#endif
	} /*invalscrollbar*/
	
	
boolean pointinscrollbar (Point pt, hdlscrollbar hscrollbar) {
#ifdef MACVERSION
	register hdlscrollbar h = hscrollbar;
	Rect contrlRect;
	
	if (h == nil) /*defensive driving*/
		return (false);
	//Code change by Timothy Paustian Friday, May 5, 2000 10:18:27 PM
	//Changed to Opaque call for Carbon
	//This routine is never called in the PPC OT version.
	#if ACCESSOR_CALLS_ARE_FUNCTIONS == 1
	GetControlBounds(h, &contrlRect);	
	#else
	contrlRect = (**h).contrlRect;
	#endif
	return (pointinrect (pt, contrlRect));
	
#endif
#ifdef WIN95VERSION
	return (false);
#endif
	} /*pointinscrollbar*/
	
	
void enablescrollbar (hdlscrollbar hscrollbar) {
	
	if (hscrollbar == nil) /*defensive driving*/
		return;

#ifdef MACVERSION
	scrollbarpushclip (hscrollbar);
		
	HiliteControl (hscrollbar, 0);
	
	validscrollbar (hscrollbar);
	
	scrollbarpopclip ();
#endif
#ifdef WIN95VERSION
	EnableScrollBar ((*hscrollbar)->hWnd, (*hscrollbar)->item, ESB_ENABLE_BOTH);	
#endif
	} /*enablescrollbar*/
	
	
void disablescrollbar (hdlscrollbar hscrollbar) {
	
	if (hscrollbar == nil) /*defensive driving*/
		return;
		
#ifdef MACVERSION

	#if TARGET_API_MAC_CARBON == 1
		
		DeactivateControl (hscrollbar);

	#else	
		scrollbarpushclip (hscrollbar);
			
		HiliteControl (hscrollbar, -1);
		
		validscrollbar (hscrollbar);
		
		scrollbarpopclip ();
		
	#endif
	
#endif
#ifdef WIN95VERSION
	EnableScrollBar ((*hscrollbar)->hWnd, (*hscrollbar)->item, ESB_DISABLE_BOTH);	
#endif
	} /*disablescrollbar*/
	
	
void getscrollbarinfo (hdlscrollbar hscrollbar, tyscrollinfo *scrollinfo) {
	
	register hdlscrollbar h = hscrollbar;
#ifdef WIN95VERSION
	SCROLLINFO si;
#endif
	
	if (h == nil) { /*defensive driving*/
		
		clearbytes (scrollinfo, sizeof (tyscrollinfo));
		
		return;
		}
#ifdef MACVERSION	
	(*scrollinfo).min = GetControlMinimum (h);
	
	(*scrollinfo).max = GetControlMaximum (h);
	
	(*scrollinfo).cur = GetControlValue (h);

	(*scrollinfo).pag = 0;
#endif
#ifdef WIN95VERSION
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_ALL;

	if (GetScrollInfo ((*hscrollbar)->hWnd, (*hscrollbar)->item, &si)) {
		(*scrollinfo).cur = si.nPos;
		(*scrollinfo).min = si.nMin;
		(*scrollinfo).max = si.nMax - si.nPage + 1;
		(*scrollinfo).pag = si.nPage;
		}
	else  {
		clearbytes (scrollinfo, sizeof (tyscrollinfo));
		}
#endif
	} /*getscrollbarinfo*/
	
	
long getscrollbarcurrent (hdlscrollbar hscrollbar) {
	
	tyscrollinfo scrollinfo;
	
	getscrollbarinfo (hscrollbar, &scrollinfo);
	
	return (scrollinfo.cur);
	} /*getscrollbarcurrent*/
	
	
void showscrollbar (hdlscrollbar hscrollbar) {

	if (hscrollbar == nil) /*defensive driving*/
		return;
		
#ifdef MACVERSION	
	scrollbarpushclip (hscrollbar);
		
	ShowControl (hscrollbar); /*no effect if scrollbar is already visible, according to IM-1*/
	
	scrollbarpopclip ();
	
	/*don't validate the scrollbar rect, ShowControl might not draw it...*/
#endif
#ifdef WIN95VERSION
	releasethreadglobals ();

	ShowScrollBar ((*hscrollbar)->hWnd, (*hscrollbar)->item, TRUE);	
	
	grabthreadglobals ();
#endif
	} /*showscrollbar*/
	
	
void hidescrollbar (hdlscrollbar hscrollbar) {

	if (hscrollbar == nil) /*defensive driving*/
		return;
		
	#ifdef MACVERSION	

		{
		Rect contrlRect;
		scrollbarpushclip (hscrollbar);
			
		releasethreadglobals ();

		HideControl (hscrollbar);
		
		grabthreadglobals ();
		
		scrollbarpopclip ();
		
		/*Code change by Timothy Paustian Sunday, May 21, 2000 9:06:31 PM
		Changed to Opaque call for Carbon
		This routine is never called in the PPC OT version*/

		#if ACCESSOR_CALLS_ARE_FUNCTIONS == 1
			GetControlBounds(hscrollbar, &contrlRect);	
		#else
			contrlRect = (**hscrollbar).contrlRect;
		#endif
		
		invalrect (contrlRect);
			}

	#endif

	#ifdef WIN95VERSION

		releasethreadglobals ();

		ShowScrollBar ((*hscrollbar)->hWnd, (*hscrollbar)->item, FALSE);	
		
		grabthreadglobals ();

	#endif
	} /*hidescrollbar*/
	

void drawscrollbar (hdlscrollbar hscrollbar) {

	/*
	7.0b20 PBS: Now works for Windows too. Fixes display glitch.
	*/

	register hdlscrollbar h = hscrollbar;
	
	if (h == nil) /*defensive driving*/
		return;

#ifdef MACVERSION	
		
	scrollbarpushclip (h);
	
	Draw1Control (h);
	
	scrollbarpopclip ();
	
	validscrollbar (h);
#endif

#ifdef WIN95VERSION

	releasethreadglobals (); /*7.0b20 PBS: release and grab thread globals*/

	SendMessage ((**hscrollbar).hWnd, WM_NCPAINT, 1, 0);

	grabthreadglobals ();

#endif
	} /*drawscrollbar*/
	
	
void displayscrollbar (hdlscrollbar hscrollbar) {
	/*
	the caller is saying that we should enable it if there is enough 
	material to enable scrolling.

	7.0b17 PBS: Scroll if min < max, not if min <= max.
	*/
	
	register hdlscrollbar h = hscrollbar;
	tyscrollinfo scrollinfo;
	
	if (h == nil) /*defensive driving*/
		return;
		
	getscrollbarinfo (h, &scrollinfo);
	
/*	if (scrollinfo.min <= scrollinfo.max) /*there's something to scroll to*/

	/*7.0b17 PBS: scroll if min < max, not if min <= max.*/

	if (scrollinfo.min < scrollinfo.max) /*there's something to scroll to*/
		enablescrollbar (h);
	else
		disablescrollbar (h);
		
	drawscrollbar (h);
	} /*displayscrollbar*/
	
	
void setscrollbarinfo (hdlscrollbar hscrollbar, const tyscrollinfo *scrollinfo) {
	/*
	set the bounds of the scrollbar, and its current value.
	
	if the current info agrees with the parameters we do nothing, 
	avoiding unpleasant flicker.
	
	disables drawing while setting the values, and draws it when 
	all three values have been set.  avoids inconsistent displays.
	
	7.0b18 PBS: On Macs, if the control manager supports proportional thumbs,
	use them.
	*/
	
	register hdlscrollbar h = hscrollbar;
	tyscrollinfo curinfo;
	
#ifdef WIN95VERSION
	SCROLLINFO si;
#endif
	
	if (h == nil) /*defensive driving*/
		return;
	
	getscrollbarinfo (h, &curinfo);
	
	if (((*scrollinfo).min == curinfo.min) && ((*scrollinfo).max == curinfo.max) && ((*scrollinfo).cur == curinfo.cur))
		return; /*nothing to do*/
	
#ifdef MACVERSION
	pushemptyclip (); /*disable drawing*/
	
	SetControlMaximum (h, min (infinity, (*scrollinfo).max));
	
	SetControlMinimum (h, (*scrollinfo).min);
	
	SetControlValue (h, min (infinity, (*scrollinfo).cur));
	
#if __powerc

	if (flmacproportionalthumbs) /*7.0b18 PBS: proportional thumbs on Macs*/
	
		SetControlViewSize (h, (*scrollinfo).pag);

#endif
	
	scrollbarpopclip ();
	
	displayscrollbar (h);
#endif

#ifdef WIN95VERSION
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_RANGE | SIF_POS | SIF_DISABLENOSCROLL | SIF_PAGE;
	si.nPos = (*scrollinfo).cur;
	si.nMin = (*scrollinfo).min;
	si.nMax = (*scrollinfo).max + (*scrollinfo).pag - 1;
	si.nPage = (*scrollinfo).pag;

	releasethreadglobals ();

	SetScrollInfo ((*hscrollbar)->hWnd, (*hscrollbar)->item, &si, TRUE);
	
	grabthreadglobals ();
#endif
	} /*setscrollbarinfo*/
	

void setscrollbarcurrent (hdlscrollbar hscrollbar, long current) {
#ifdef MACVERSION
	register hdlscrollbar h = hscrollbar;
	
	if (h == nil) /*defensive driving*/
		return;
	
	scrollbarpushclip (h);
	
	SetControlValue (h, min (infinity, current));
	
	scrollbarpopclip ();
#endif
#ifdef WIN95VERSION
	SCROLLINFO si;

	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_POS;
	si.nPos = current;

	releasethreadglobals ();

	SetScrollInfo ((*hscrollbar)->hWnd, (*hscrollbar)->item, &si, TRUE);
	
	grabthreadglobals ();
#endif
	} /*setscrollbarcurrent*/


short getscrollbarwidth (void) {
#ifdef MACVERSION	
	if (config.flwindoidscrollbars)
		return (13);
	else
		return (16); /*standard Macintosh scrollbars*/
#endif
#ifdef WIN95VERSION
	return (0);  /*handled by system */
#endif
	} /*getscrollbarwidth*/



boolean newscrollbar (WindowPtr w, boolean flvert, hdlscrollbar *hscrollbar) {
#ifdef MACVERSION
	
	/*
	create a new scroll bar linked into the control list of the indicated
	window.  
	
	we need to know if it is a vertical or horizontal scrollbar so we can
	choose the proper resource template.  we use a CDEF that has different
	specs for horiz and vertical scrollbars.
	*/
	
	register short resnum;
	
	if (config.flwindoidscrollbars) {
	
		if (flvert)
			resnum = idvertbaroid;
		else
			resnum = idhorizbaroid;
		}
	else {
		if (flvert)
			resnum = idvertbar;
		else
			resnum = idhorizbar;
		}
	
	*hscrollbar = GetNewControl (resnum, w);
#endif
#ifdef WIN95VERSION
	hdlscrollbar h;
	h = (hdlscrollbar) NewHandle (sizeof(scrollbarrecord));
	if (h != NULL) {
		(*h)->hWnd = w;
		(*h)->item = flvert?SB_VERT:SB_HORZ;
		}
	*hscrollbar = h;
#endif
	return ((*hscrollbar) != nil);
	} /*newscrollbar*/
	
	
void disposescrollbar (hdlscrollbar hscrollbar) {
#ifdef MACVERSION	
	register hdlscrollbar h = hscrollbar;
	
	if (h != nil)
		DisposeControl (h);
#endif
#ifdef WIN95VERSION
	DisposeHandle ((Handle) hscrollbar);
#endif
	} /*disposescrollbar*/
	

#ifdef MACVERSION	
void getscrollbarrect (hdlscrollbar hscrollbar, Rect *r) {

	register hdlscrollbar h = hscrollbar;
	
	if (h != nil) 
	{
		//Code change by Timothy Paustian Sunday, May 21, 2000 9:07:18 PM
		//Changed to Opaque call for Carbon
		//This routine is never called in the PPC OT version.
		Rect contrlRect;
		#if ACCESSOR_CALLS_ARE_FUNCTIONS == 1
		GetControlBounds(h, &contrlRect);	
		#else
		contrlRect = (**h).contrlRect;
		#endif
		*r = contrlRect;
	}
	else
		zerorect (r);
	} /*getscrollbarrect*/
#endif	
	
	
void setscrollbarrect (hdlscrollbar hscrollbar, Rect r) {

#ifdef MACVERSION	
	register hdlscrollbar h = hscrollbar;
	
	if (h != nil) {
		
		HideControl (h);
		
		SizeControl (h, r.right - r.left, r.bottom - r.top);
		
		MoveControl (h, r.left, r.top);
		}
#endif
	} /*setscrollbarrect*/
	
void scrollbarflushright (Rect r, hdlscrollbar hscrollbar) {
#ifdef MACVERSION	
	register hdlscrollbar h = hscrollbar;
	register short width;
	
	if (h == nil) /*defensive driving*/
		return;
		
	width = getscrollbarwidth ();
	
	HideControl (h);
	
	SizeControl (h, width, r.bottom - r.top + 2);
	
	MoveControl (h, r.right - width + 1, r.top - 1);
#endif
	} /*scrollbarflushright*/
	
	
void scrollbarflushbottom (Rect r, hdlscrollbar hscrollbar) {
#ifdef MACVERSION	
	register hdlscrollbar h = hscrollbar;
	register short width;
	
	if (h == nil) /*defensive driving*/
		return;
		
	width = getscrollbarwidth ();
		
	HideControl (h);
	
	SizeControl (hscrollbar, r.right - r.left, width);
		
	MoveControl (hscrollbar, r.left, r.bottom - width + 1);
#endif
	} /*scrollbarflushbottom*/
	

#ifdef MACVERSION	
boolean findscrollbar (Point pt, WindowPtr w, hdlscrollbar *hscrollbar, short *scrollbarpart) {
	
	*scrollbarpart = FindControl (pt, w, hscrollbar);
	
	return (*hscrollbar != nil);
	} /*findscrollbar*/
	

long gpreviousscrollvalue = 0;

boolean scrollbarhit (hdlscrollbar hscrollbar, short part, boolean *flup, boolean *flpage) {
	
	/*
	can be called by a routine that tracks a mouse hit in a scroll bar.
	
	return true if it represents a valid scroll action, according to the info
	stored in the scrollbar handle.
	
	return false if the indicated action would move beyond the min or max for
	the scrollbar.
	
	set flup and flpage according to the indicated scrollbar part.
	*/
	
	register hdlscrollbar h = hscrollbar;
	tyscrollinfo scrollinfo;
	
	if (h == nil) /*defensive driving*/
		return (false);
	
	getscrollbarinfo (h, &scrollinfo);
	
	switch (part) {
	
		case kControlUpButtonPart:
			*flup = false; /*scroll text down*/
			
			*flpage = false;
			
			return (scrollinfo.cur > scrollinfo.min);
			
		case kControlDownButtonPart:
			*flup = true; /*scroll text up*/
			
			*flpage = false;
			
			return (scrollinfo.cur < scrollinfo.max);
			
		case kControlPageUpPart:
			*flup = false; /*scroll text down*/
			
			*flpage = true; 
			
			return (scrollinfo.cur > scrollinfo.min);
			
		case kControlPageDownPart:
			*flup = true; /*scroll text up*/
			
			*flpage = true;
			
			return (scrollinfo.cur < scrollinfo.max);
		case kControlIndicatorPart:
			*flpage = false;
			if (scrollinfo.cur  == gpreviousscrollvalue)
				return (false);
			if (scrollinfo.cur > gpreviousscrollvalue)
				*flup = true;
			else
				*flup = false;
			gpreviousscrollvalue = scrollinfo.cur;
			return (true);
		} /*switch*/
		
	return (false); /*fell through the switch statement*/
	} /*scrollbarhit*/
	
#endif

boolean initscrollbars (void) {
	
	/*
	7.0b18 PBS: check for 8.5 control manager on Macs -- proportional thumbs.
	*/
	
	#if __powerc

		OSErr err;
		long response;
	
		err = Gestalt (gestaltControlMgrAttr, &response);
	
		if ((err == noErr) && (response & gestaltControlMgrPresent))
			flmacproportionalthumbs = true;

	#endif
	
	return (true);
	} /*initscrollbars*/
