
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
#include "font.h"
#include "mouse.h"
#include "quickdraw.h"
#include "frontier_strings.h"
#include "scrap.h"
#include "kb.h"
#include "op.h"
#include "opinternal.h"
#include "shell.rsrc.h"
#include "shellundo.h"
#include "wpengine.h"




static boolean fleditingnow = false;



boolean opeditsetglobals (void) {
	
	/*
	set up the word processing engine's globals

	5.0b15 dmb: handle nil outlinedata
	*/
	
	if (outlinedata == nil)
		return (false);

	wpdata = (hdlwprecord) (**outlinedata).hbuffer;
	
	wpwindow = outlinewindow;
	
	wpwindowinfo = outlinewindowinfo;
	
	return (wpdata != nil);
	} /*opeditsetglobals*/


boolean opeditingtext (hdlheadrecord hnode) {
	
	/*
	return true if the indicated node is being edited by the text editor
	*/
	
	register hdloutlinerecord ho = outlinedata;
	
	if ((**ho).hbuffer && (hnode == (**ho).heditcursor)) {
		
		//assert ((**ho).fltextmode);
		
		assert ((**ho).hbarcursor == (**ho).heditcursor);
		
		return (true);
		}
	
	return (false);
	} /*opeditingtext*/


boolean opdefaultgetedittextrect (hdlheadrecord hnode, const Rect *linerect, Rect *textrect) {
	
	/*
	5.0b7 dmb: add our width to the default text rect here, cleanly
	*/
	
	opdefaultgettextrect (hnode, linerect, textrect);
	
	if (!opisfatheadlines (outlinedata))
		(*textrect).right += 1600; /*leave room for long lines; not too large, but enough*/
	
	(*textrect).top += textvertinset;
	
	return (true);
	} /*opdefaultgetedittextrect*/


boolean opdefaultsetwpedittext (hdlheadrecord hnode) {
	
	/*
	2/7/97 dmb: we are called with the wp globals set up. 
	our job it to set the wp to contain the right text.
	*/
	
	return (wpsettexthandle ((**hnode).headstring));
	} /*opdefaultsetwpedittext*/


boolean opdefaultgetwpedittext (hdlheadrecord hnode, boolean flunload) {
#pragma unused (flunload)

	/*
	2/7/97 dmb: we are called with the wp globals set up. 
	our job it to extract the current text from the wp.
	
	return true if the text actually changed
	
	6.0a1 dmb: use handles  [xxx bypassing opsetheadstring bigstrings and special cases]
	*/
	
	Handle htext;
	
	if (!wpgettexthandle (&htext)) 
		return (false);
	
	opsetheadtext (hnode, htext);
	
	/*
	if (!equalhandles (htext, (**hnode).headstring)) {
		
		(**hnode).fldirty = true;
		
		opdirtyoutline ();
		}
	
	disposehandle ((**hnode).headstring);
	
	(**hnode).headstring = htext;
	*/
	
	return (true);
	} /*opdefaultgetwpedittext*/


static boolean opgettextbufferrect (hdlheadrecord hnode, Rect *rclip, Rect *rtext) {
	
	/*
	6.0a12 dmb: clip to linerect _and_ outlinerect. linerect may extend above display.
	
	6.0b2 dmb: if line is off screen, return a rect above the screen, not false.
	*/
	
	register hdloutlinerecord ho = outlinedata;
	Rect rline;
	
	if (!opgetnoderect (hnode, &rline)) {
	
		rline = (**ho).outlinerect;
		
		rline.bottom = rline.top;
		
		rline.top -= opgetlineheight (hnode);
		
		// return (false);
		}
	
	intersectrect ((**ho).outlinerect, rline, rclip);
	
	(*(**ho).getedittextrectcallback) (hnode, &rline, rtext);
	
	#ifdef WIN95VERSION
		(*rtext).top += 1;
	#endif
	
	return (true);
	} /*opgettextbufferrect*/


boolean opseteditbufferrect (void) {
	
	/*
	5.0d1 dmb: return false if setglobals fails. can happen if outline was
	just unpacked
	*/
	
	register hdloutlinerecord ho = outlinedata;
	register hdlheadrecord hcursor = (**ho).heditcursor;
	Rect r, rclip;
	
	if (!(**ho).fltextmode || !opeditsetglobals ())
		return (true);
	
	opgettextbufferrect (hcursor, &rclip, &r);
	
	intersectrect (rclip, r, &rclip);
	
	wpsetbufferrect (rclip, r);
	
	return (true);
	} /*opseteditbufferrect*/


static boolean oppreedit (void) {
	
	fleditingnow = true;
	
	return (true);
	} /*oppreedit*/


void oppostedit (void) {
	
	/*
	6.0a1 dmb: for outline text, wpdata maintains text measurements, which we need
	to pass on to the headline

	7.0b16 PBS: no longer static, needed by opdisplay.c to fix display glitch
	when resizing window after editing.

	7.0b19 PBS: Fix a major Windows display glitch. When computing the difference
	needed to scroll, there was no check that the vertical height of the current line
	had been dirtied. If dirtied, then there will always be a difference, and
	a scroll will occur.
	*/
	
	hdloutlinerecord ho = outlinedata;
	hdlwprecord hwp = wpdata;
	hdlheadrecord hcursor = (**ho).heditcursor;
	
	fleditingnow = false;
	
	if ((**hwp).fltextchanged)
		opschedulevisi ();
	
	(**hcursor).hpixels = (**hwp).hpixels;
	
	if (!(**hwp).floneline && (**hwp).vpixels) {
	
		short vdiff;
		Rect r;
		long lnum, oldlines, linediff;

		if ((**hcursor).vpixels == opdirtymeasurevalue) /*7.0b19 PBS: if dirty, measure*/
			opeditmeasuretext (hcursor);

		vdiff = (**hwp).vpixels - (**hcursor).vpixels; //vertical growth
		
		if (vdiff != 0) {
			
 			opgetscreenline (hcursor, &lnum);
			
			// scroll headlines below up or down
			if (opdisplayenabled () && (lnum >= 0) && opgetnoderect (hcursor, &r)) {
				
				r.top = r.bottom;
				
				if (vdiff < 0)
					r.top += vdiff;
				
				r.bottom = (**ho).outlinerect.bottom;
				
				wphidecursor ();
				
				opscrollrect (r, 0, vdiff);
				}
			
			// adjust expansion count
			oldlines = opgetnodelinecount (hcursor);
			
			(**hcursor).vpixels = (**hwp).vpixels;
			
			linediff = opgetnodelinecount (hcursor) - oldlines;
			
			(**ho).ctexpanded += linediff;
			
			if (lnum < 0)
				(**ho).vertscrollinfo.cur += linediff;
			
			opresetscrollbars (); // scroll range depends on ctexpanded
			
			if (opdisplayenabled ()) {
				
				opseteditbufferrect ();
				
				opupdatenow (); //fill in behind scroll
				
				opvisibarcursor ();
				
				(**ho).flcheckvisi = false; //undo opschedulevisi above
				}
			}
		}
	} /*oppostedit*/


static pascal void opedittrackclick (hdlwprecord wp, Point pt) {
#pragma unused(wp)

	/*
	5.0a3 dmb: check outlinerect, now windowinfo's contentrect (via shellcheckautoscroll)
	*/
	
	tydirection dir;
	
	if (mousecheckautoscroll (pt, (**outlinedata).outlinerect, true, &dir)) {
		
	//	ClipRect (&(**outlinedata).outlinerect);

		opscroll (dir, false, 1);
		}
	} /*opedittrackclick*/


//Code change by Timothy Paustian Friday, June 16, 2000 1:23:53 PM
//Changed to Opaque call for Carbon
//try just a plain proc ptr, this may not work

#if !TARGET_RT_MAC_CFM
	
	#define opedittrackclickUPP (&opedittrackclick)

#else
	 #if TARGET_API_MAC_CARBON == 1
		#define opedittrackclickUPP opedittrackclick
	 #else
	static RoutineDescriptor opedittrackclickDesc = BUILD_ROUTINE_DESCRIPTOR (uppTrackClickProcInfo, opedittrackclick);
	#define opedittrackclickUPP (&opedittrackclickDesc)
	#endif
#endif



boolean oploadeditbuffer (void) {
	
	/*
	10/1/91 dmb: don't hook up the dirtyroutine callback until after 
	we've done the wpsettext.
	
	12/25/91 dmb: added recovery from wpsettext failure
	
	2/13/92 dmb: make sure opstyle is still pushed when opsetdisplaypixels is called
	
	5.0.2b4 dmb: don't reference hbuffer after setting it to nil
	*/
	
	register hdloutlinerecord ho = outlinedata;
	register hdlwprecord hbuffer;
	register hdlheadrecord hcursor = (**ho).hbarcursor;
	Rect rbounds, rclip;
	tywpflags flags;
	
	if (opeditsetglobals ())
		wpdispose ();
	
	(**ho).hbuffer = nil;
	
	if (!(**ho).fltextmode)
		return (true);
	
	if (!(**ho).flwindowopen)
		return (false);
	
	opgettextbufferrect (hcursor, &rclip, &rbounds);
	
	intersectrect (rclip, rbounds, &rclip);
	
	oppushheadstyle (hcursor);
	
	flags = wpalwaysmeasure;
	
	if (!(**ho).flfatheadlines)
		flags |= wponeline;
	
//	if (!(**ho).flhorizscrolldisabled) /*7.0b26 PBS: fix display glitch when horizontal scrolling is disabled.*/
		flags |= wpneverscroll;
	
	hbuffer = wpnewbuffer (nil, &rclip, &rbounds, flags, true);
	
	(**ho).hbuffer = (Handle) hbuffer;
	
	(**ho).heditcursor = hcursor;
	
	if (hbuffer != nil) {
		
		opeditsetglobals ();
		
		(**hbuffer).preeditroutine = &oppreedit;
		
		(**hbuffer).posteditroutine = &oppostedit;
		
		(**hbuffer).flinhibitdisplay = (**ho).flinhibitdisplay;
		
		(**hbuffer).flwindowopen = (**ho).flwindowopen;
		
		if ((!(**ho).flactive) && (!(**ho).flalwaysshowtextselection))
			wpactivate (false);
		
		if (!(*(**outlinedata).setwpedittextcallback) (hcursor)) { /*out of memory?*/
			
			wpdisposerecord (hbuffer);
			
			(**ho).hbuffer = nil;
			
			hbuffer = nil;
			}
		else
			wpsetselection (longinfinity, longinfinity);
		}
	
	if (hbuffer == nil) { /*allocation error*/
		
		(**ho).fltextmode = false; /*keep things consistant*/
		
		popstyle ();
		
		return (false);
		}
	
	if ((**hbuffer).flneverscroll) /*callback may have changed it*/
		(**hbuffer).trackclickroutine = opedittrackclickUPP;
	
	(**hbuffer).flstartedtyping = false; /*for undo -- haven't started typing here yet*/
	
	(**hbuffer).dirtyroutine = &opdirtyoutline;
	
	popstyle ();
	
	return (true);
	} /*oploadeditbuffer*/


boolean opwriteeditbuffer (void) {
	
	/*
	returns true if the text changed.
	
	7/9/91 dmb: don't set edit globals when hbuffer is nil, so we can be 
	called indiscriminantly from oppack.c (or whereever) and not smash 
	existing wp globals
	*/
	
	register hdloutlinerecord ho = outlinedata;
	register hdlheadrecord hcursor = (**ho).heditcursor;
	
	if ((**ho).hbuffer == nil)
		return (false);
	
	opeditsetglobals ();
	
	return ((*(**outlinedata).getwpedittextcallback) (hcursor, false));
	} /*opwriteeditbuffer*/


boolean opunloadeditbuffer (void) {

	hdloutlinerecord ho = outlinedata;
	hdlwprecord hbuffer = (hdlwprecord) (**ho).hbuffer;
	hdlheadrecord hcursor = (**ho).heditcursor;
	
	if (hbuffer == nil)
		return (true);
	
	opeditsetglobals ();
	
	if (!(*(**outlinedata).getwpedittextcallback) (hcursor, true))
		return (false);
	
	wpactivate (-1);
	
	wpdispose ();
	
	(**ho).hbuffer = nil;
	
	return (true);
	} /*opunloadeditbuffer*/


boolean opsaveeditbuffer (void) {
	
	register hdloutlinerecord ho = outlinedata;
	tytextinfo textinfo;
	
	if ((**ho).hbuffer == nil) /*no buffer open*/
		return (false);
	
	opeditsetglobals ();
	
	textinfo.flvalid = wpgetselection (&textinfo.selStart, &textinfo.selEnd);
	
	(**ho).textinfo = textinfo;
	
	opunloadeditbuffer ();
	
	return (true);
	} /*opsaveeditbuffer*/


boolean oprestoreeditbuffer (void) {
	
	register hdloutlinerecord ho = outlinedata;
	tytextinfo textinfo;
	
	if ((**ho).hbuffer != nil) /*already loaded -- don't smash selection*/
		return (true);
	
	if (!oploadeditbuffer ())
		return (false);
	
	if ((**ho).hbuffer == nil) 
		return (true);
	
	textinfo = (**ho).textinfo;
	
	if (textinfo.flvalid) {
		
		opeditsetglobals ();
		
		wpsetselection (textinfo.selStart, textinfo.selEnd);
		
		textinfo.flvalid = false;
		}
	
	return (true);
	} /*oprestoreeditbuffer*/


boolean opeditmeasuretext (hdlheadrecord hnode) {
	
	/*
	6.0a1 dmb: measure the height of the node's text using the current
	font, size, outlinerect, etc.
	*/
	
	register hdloutlinerecord ho = outlinedata;
	Rect rbounds, rclip;
	tywpflags flags = 0;
	long width, height;
	
	if ((**hnode).hpixels >= 0 && (**hnode).vpixels >= 0) //already measured
		return (true);
	
	if (opeditingtext (hnode)) {
		
		opeditsetglobals ();
		
		wpgetcontentsize (&width, &height);
		}
	else {
		
		rclip = (**ho).outlinerect;
		
		opgettextrect (hnode, &rclip, &rbounds);
		
		//intersectrect (rclip, rbounds, &rclip);
		
		oppushheadstyle (hnode);
		
		if (!(**ho).flfatheadlines)
			flags = wponeline;
		
		if ((**ho).flprinting)
			flags |= wpprinting;
		
		wpmeasuretext ((**hnode).headstring, &rbounds, flags);
		
		popstyle ();
		
		width = rbounds.right - rbounds.left;
		
		height = rbounds.bottom - rbounds.top;
		}
	
	if (height == 0)
		height = (**ho).defaultlineheight;
	
	(**hnode).hpixels = width;
	
	(**hnode).vpixels = height;
	
	return (true);
	} /*opeditmeasuretext*/


boolean opeditdrawtext (hdlheadrecord hnode, const Rect *rtext) {
	
	/*
	6.0a1 dmb: measure the height of the node's text using the current
	font, size, outlinerect, etc.
	*/
	
	register hdloutlinerecord ho = outlinedata;
	Rect r, rclip;
	tywpflags flags = 0;
	
	if (opeditingtext (hnode)) {
		
		opeditupdate ();
		
		return (true);
		}
	
	rclip = (**ho).outlinerect;
	
	r = *rtext;
	
	#ifdef WIN95VERSION
		r.top += 1;
	#endif
	
	intersectrect (rclip, r, &rclip);
	
	oppushheadstyle (hnode);
	
	if (!(**ho).flfatheadlines)
		flags = wponeline;
	
	if ((**ho).flprinting)
		flags |= wpprinting;

	wpdrawtext ((**hnode).headstring, &rclip, &r, flags);
	
	popstyle ();
	
	return (true);
	} /*opeditdrawtext*/


void opeditgetmaxpos (long *maxpos) {
	
	opeditsetglobals ();
	
	wpgetmaxpos (maxpos);
	} /*opeditgetmaxpos*/


void opeditgetselection (long *startsel, long *endsel) {
	
	opeditsetglobals ();
	
	wpgetselection (startsel, endsel);
	} /*opeditgetselection*/


void opeditsetselection (long startsel, long endsel) {
	
	/*
	2/26/91 dmb: to support searching things that aren't in windows (yet), 
	we need to handle the case where the buffer hasn't been loaded
	*/
	
	if (opeditsetglobals ())
		wpsetselection (startsel, endsel);
	
	else {
		tytextinfo textinfo;
		
		textinfo.selStart = startsel;
		
		textinfo.selEnd = endsel;
		
		textinfo.flvalid = true;
		
		(**outlinedata).textinfo = textinfo;
		}
	} /*opeditsetselection*/


void opeditgetseltext (bigstring bs) {
	
	opeditsetglobals ();
	
	wpgetseltext (bs);
	} /*opeditgetseltext*/


void opeditgetselrect (Rect * r) {
	
	opeditsetglobals ();
	
	wpgetselrect (r);
	} /*opeditgetselrect*/


void opeditgetselpoint (Point *pt) {
	
	opeditsetglobals ();
	
	wpgetselpoint (pt);
	} /*opeditgetselpoint*/


void opeditresetselpoint (void) {
	
	/*
	the next time the up or down cursor key is pressed, a new horizontal 
	offset should be maintained from that point.  we just clear our field.
	*/
	
	(**outlinedata).selpoint.h = -1;
	} /*opeditresetselpoint*/


void opeditsetselpoint (Point pt) {
	
	register hdloutlinerecord ho = outlinedata;
	
	opeditsetglobals ();
	
	if ((**ho).selpoint.h < 0) /*we already have established a horizontal position*/
		(**ho).selpoint = pt;
	
	wpsetselpoint ((**ho).selpoint);
	
	opschedulevisi ();
	} /*opeditsetselpoint*/


boolean opeditcango (tydirection dir) {
	
	/*
	6.0a1 dmb: return true if the current arrow key should be handled in this headline
	
	6.0b2 dmb: don't use wprect; it's clipped. get the textbufferrect from scratch.
	*/
	
	long startsel, endsel, maxpos;
	Rect rwhole, rsel, rclip;
	boolean fl;
	
	opeditsetglobals ();
	
	wpgetselection (&startsel, &endsel);
	
	wpgetselrect (&rsel);
	
	//rwhole = (**wpdata).wprect;
	opgettextbufferrect ((**outlinedata).heditcursor, &rclip, &rwhole);
	
	insetrect (&rwhole, texthorizinset, textvertinset);
	
	switch (dir) {
		
		case up: case flatup:
			fl = (rsel.top > rwhole.top);
			
			break;
		
		case down: case flatdown:
			fl = (rsel.bottom < rwhole.bottom);
			
			break;
		
		case left:
			fl = ((startsel != 0) || (endsel != 0));
			
			break;
		
		case right:
			wpgetmaxpos (&maxpos);
			
			fl = ((startsel != maxpos) || (endsel != maxpos));
			
			break;
		
		default:
			fl = false;
		}
	
	return (fl);
	} /*opeditcango*/


static boolean opeditrecalcheadline (void) {

	/*
	7.0b28 PBS: The headline may need to be redrawn to apply
	HTML styles.

	Return true if the headline was redrawn, false otherwise.

	7.0b33 PBS: Because redrawing the headline may collapse it to
	fewer lines, the headlines below the current headline should
	also be redrawn.
	*/

	if (outlinedata != NULL) {

		if ((**outlinedata).flhtml) {

			long startsel, endsel;

			wpgetselection (&startsel, &endsel);

			opunloadeditbuffer ();

			oploadeditbuffer ();

			wpsetselection (startsel, endsel);

			opinvalafter ((**outlinedata).heditcursor); /*7.0b33 PBS: redraw below current headline.*/

			return (true);
			} /*if*/
		} /*if*/

	return (false);
	} /*opeditrecalcheadline*/


boolean opeditkey (void) {

	char chkb = keyboardstatus.chkb;

	opeditsetglobals ();
	
	wpkeystroke ();

	if (chkb == '>')
		opeditrecalcheadline ();
	
	(**outlinedata).blocksupersmartvisi = true;
	
	opschedulevisi ();
	
	opeditresetselpoint ();
	
	return (true);
	} /*opeditkey*/


boolean opeditcopy (void) {

	opeditsetglobals ();
	
	return (wpcopy ());
	} /*opeditcopy*/


boolean opeditcut (void) {

	opeditsetglobals ();
	
	return (wpcut ());
	} /*opeditcut*/


boolean opeditpaste (void) {
	
	opeditsetglobals ();
	
	return (wppaste (false));
	} /*opeditpaste*/


boolean opeditclear (void) {
	
	opeditsetglobals ();
	
	return (wpclear ());
	} /*opeditclear*/


boolean opeditinsert (bigstring bs) {
	
	opeditsetglobals ();
	
	return (wpinsert (bs));
	} /*opeditinsert*/


boolean opeditclick (Point pt, tyclickflags flags) {
	
	//opclearallmarks (); /*9/11/91*/
	
	opeditsetglobals ();
	
	wpclick (pt, flags);
	
	opeditresetselpoint ();
	
	return (true);
	} /*opeditclick*/


void opeditselectall (void) {
	
	opeditsetglobals ();
	
	wpselectall ();
	} /*opeditselectall*/


void opeditactivate (boolean flactivate) {
	
	if (flactivate) {
		
		oprestoreeditbuffer ();
		
		if (opeditsetglobals ())
			wpactivate (true);
		}
	else {
	//	if (!(**outlinedata).flalwaysshowtextselection) { /*DW 8/16/93*/
		
			if (opeditsetglobals ())
				wpactivate (false);
	//		}
		
		opwriteeditbuffer (); /*update text handle*/
		}
	} /*opeditactivate*/


void opeditupdate (void) {
	
	opeditsetglobals ();
	
	wpupdate ();
	} /*opeditupdate*/


void opeditidle (void) {
	
	opeditsetglobals ();
	
	wpidle ();
	} /*opeditidle*/


void opeditdispose (void) {
	
	/*
	7/20/91 dmb: as a disposal routine, this call get called at odd times, 
	perhaps when a wp window is active or some other outline.  we need to 
	preserve all wp globals that we touch
	*/
	
	hdlwprecord savewpdata;
	WindowPtr savewpwindow;
	hdlwindowinfo savewpwindowinfo;
	
	savewpdata = wpdata;
	
	savewpwindow = wpwindow;
	
	savewpwindowinfo = wpwindowinfo;
	
	opeditsetglobals ();
	
	wpdispose ();
	
	wpdata = savewpdata;
	
	wpwindow = savewpwindow;
	
	wpwindowinfo = savewpwindowinfo;
	} /*opeditdispose*/


boolean opeditgetundoglobals (long *globals) {
	
	register hdloutlinerecord ho = outlinedata;
	
	if (fleditingnow && (**ho).hbuffer)
		*globals = (long) (**ho).heditcursor;
	else
		*globals = 0L;
	
	return (true);
	} /*opeditgetundoglobals*/


boolean opeditsetundoglobals (long globals, boolean flundo) {
	
	/*
	7/2/91 dmb: now takes flundo parameter.  since wpengine doesn't need its 
	globals set up to dispose an undo, we can just exit when flundo is 
	false.  if this weren't the case, we'd have to store the wpdata handle 
	in the undoglobals to avoid having to recreate the context when just 
	tossing an undo.
	*/
	
	register hdloutlinerecord ho = outlinedata;
	register hdlheadrecord hnode = (hdlheadrecord) globals;
	
	if (!globals || !flundo)
		return (true);
	
	if ((**ho).hbarcursor == hnode) { /*cursor is already on the node*/
		
		opsettextmode (true);
		
		opvisinode (hnode, false);
		}
	
	else {
		(**ho).fltextmode = true;
		
		opclearallmarks ();
		
		opexpandto (hnode);
		}
	
	opeditsetglobals ();
	
	return (true);
	} /*opeditsetundoglobals*/



