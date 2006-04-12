
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
#include "bitmaps.h"
#include "font.h"
#include "kb.h"
#include "launch.h"
#include "quickdraw.h"
#include "search.h"
#include "strings.h"
#include "frontierwindows.h"
#include "frontierconfig.h"
#include "scrollbar.h"
#include "zoom.h"
#include "file.h"
#include "resources.h"
#include "shell.h"
#include "shellbuttons.h"
#include "shellhooks.h"
#include "shellundo.h"
#include "shellprivate.h"
#include "lang.h"
#include "langinternal.h"
#include "tablestructure.h"
#include "cancoon.h"

#ifdef WIN95VERSION
#include "FrontierWinMain.h"
#endif



#ifdef xxxWIN95VERSION
	static byte bschangedwindowsuffix [] = "\x02" " *";
#endif


static hdlwindowinfo hfirstwindow = nil; /*head of the window list*/



void shellwindowinval (hdlwindowinfo hinfo) {
	
	windowinval ((**hinfo).macwindow);
	} /*shellwindowinval*/
	
	
void shellinvalcontent (hdlwindowinfo hinfo) {
	
	//Code change by Timothy Paustian Monday, August 21, 2000 4:31:49 PM
	//Must pass a CGrafPtr to pushport on OS X to avoid a crash
	CGrafPtr	thePort;
	#if TARGET_API_MAC_CARBON == 1
	thePort = GetWindowPort((**hinfo).macwindow);
	#else
	thePort = (CGrafPtr)(**hinfo).macwindow;
	#endif
		
	pushport (thePort);
	
	smashrect ((**hinfo).contentrect);
	
	popport ();
	} /*shellinvalcontent*/


boolean shellrectneedsupdate (const Rect *r) {

	hdlregion rgn = (**shellwindowinfo).drawrgn;
	
	if (rgn == nil)
		return (false);
	
//	#if WIN95VERSION
//		return (true);	// *** cover bug
//	#endif

	return (rectinregion (*r, (**shellwindowinfo).drawrgn)); 
	} /*shellrectneedsupdate*/


boolean shellgetgrowiconrect (hdlwindowinfo hinfo, Rect *r) {
	
	if (config.flgrowable) {
	
		*r = (**hinfo).growiconrect;
		
		return (true);
		}
	
	return (false);
	} /*shellgetgrowiconrect*/


void shelldrawgrowicon (hdlwindowinfo hinfo) {

#	ifdef MACVERSION
#		if TARGET_API_MAC_CARBON == 1
#			pragma unused (hinfo)

	return;

#	else
			
			Rect r;
			
			if (shellgetgrowiconrect (hinfo, &r)) {
				
				register WindowPtr w = (**hinfo).macwindow;
				boolean flnoframe; // 5.0a3 dmb: should add new config flag
				
				//Code change by Timothy Paustian Monday, August 21, 2000 4:31:49 PM
				//Must pass a CGrafPtr to pushport on OS X to avoid a crash
				{
				CGrafPtr	thePort;
				#if TARGET_API_MAC_CARBON == 1
				thePort = GetWindowPort(w);
				#else
				thePort = (CGrafPtr)w;
				#endif
					
				pushport (thePort);
				}
				
				flnoframe = (**hinfo).configresnum == idaboutconfig;
				
				if (flnoframe)
					insetrect (&r, 1, 1); /*don't wan't to erase its frame*/
				
				pushclip (r);
				
				if ((**hinfo).flwindowactive)
					DrawGrowIcon (w);
				
				else {
					
					if (!flnoframe) {
					
						framerect (r);
						
						insetrect (&r, 1, 1); /*don't wan't to erase its frame*/
						}
					
					eraserect (r);
					}
				
				validrect (r);
				
				popclip ();
				
				popport ();
				}
		#endif
	#endif

	} /*shelldrawgrowicon*/


void shellerasegrowicon (hdlwindowinfo hinfo) {
	
	Rect r;
	
	if (shellgetgrowiconrect (hinfo, &r)) 
		eraserect (r);
	} /*shellerasegrowicon*/


boolean getwindowinfo (WindowPtr w, hdlwindowinfo *hinfo) {
	
	register hdlwindowinfo h;
	
	*hinfo = nil;
	
	if (w == nil) /*defensive driving*/
		return (false);
	
	*hinfo = h = (hdlwindowinfo) getwindowrefcon (w);

	return (h != nil);
	} /*getwindowinfo*/


boolean isshellwindow (WindowPtr wptr) {
	
	/*
	return true if the indicated window is one of our windows, owned by 
	the shell.  see comments in frontshellwindow below.
	
	note: another method traverse our window list looking for a match
	
	3/26/93 dmb: that's really the best method, clean & reasonably fast.
	*/
	
	register WindowPtr w = wptr;
	register hdlwindowinfo nomad;
	
	if (w == nil)
		return (false);
	
#ifdef MACVERSION
	//Code change by Timothy Paustian Saturday, April 29, 2000 11:12:22 PM
	//Changed to Opaque call for Carbon
	if(GetWindowKind(w) <= 0)
		return false;
	//old code
	//if ((*(WindowPeek) w).windowKind <= 0)
	//	return (false);
#endif
	
	nomad = hfirstwindow;
	
	if (nomad == nil)
		return (false);
	
	while (true) {
		
		if ((**nomad).macwindow == (WindowPtr) w)
			return (true);
		
		nomad = (**nomad).nextwindow;
		
		if (nomad == hfirstwindow) /*wrapped around, didn't find it*/
			return (false);
		}

	} /*isshellwindow*/


boolean isfilewindow (WindowPtr w) {
	
	/*
	return true if the indicated window is a file window -- a root 
	window. this routine could also be called isrootwindow
	*/
	
	hdlwindowinfo hinfo;
	
	if (!getwindowinfo (w, &hinfo))
		return (false);
	
	return ((**hinfo).parentwindow == nil);
	} /*isfilewindow*/


boolean frontshellwindow (WindowPtr *wptr) {
	
	/*
	built to return the front window even when a modal dialog box is the
	front window.  we chase through the Mac's window list looking for a 
	window with a non-nil refcon field.  we assume (perhaps foolishly?)
	that the only windows we will encounter are ones of our own creation.
	
	this can be a problem when all our windows are hidden.
	
	7/27/90 dmb: can't assume non-nil refcon is ours...  test for 
	a comm event with a null event.
	
	12/28/90 dmb: also need to check windowKind
	*/

	register WindowPtr w;
	
	for (w = getfrontwindow (); w != nil; w = getnextwindow (w)) {
		
		if (isshellwindow ((WindowPtr) w)) {
			
			*wptr = (WindowPtr) w;
			
			return (true);
			}
		}
	
	*wptr = nil; /*fell off the end of the list*/
	
	return (false);
	} /*frontshellwindow*/


boolean shellfindwindow (short id, WindowPtr *w, hdlwindowinfo *hinfo, Handle *hdata) {
	
	/*
	find the window that has a config id equal to the indicated id.
	
	return all three records -- its window, its info record and a handle to its data.

	5.1.5 dmb: out callers expect hidden windows to be found. so we can't walk 
	the window manager's list. so the result may not be he fontmost. but our callers
	are all one-of-a-kind
	*/

	register hdlwindowinfo h = hfirstwindow;
	
	if (h == nil) /*no windows open*/
		return (false);
	
	while (true) {
		
		if ((**h).configresnum == id) {
			
			*w = (**h).macwindow;
			
			*hinfo = h;
			
			*hdata = (**h).hdata;
			
			return (true);
			}
		
		h = (**h).nextwindow; /*nomad might have been dealloc'd already*/
		
		if (h == hfirstwindow) /*wrapped around to head of list*/
			return (false);
		} /*while*/
	} /*shellfindwindow*/


boolean getfrontwindowinfo (hdlwindowinfo *hinfo) {
	
	/*
	get the windowinfo record for the frontmost window.
	*/
	
	WindowPtr w;
	
	if (!frontshellwindow (&w))
		return (false);
	
	return (getwindowinfo (w, hinfo));
	} /*getfrontwindowinfo*/
	

boolean shellgetglobalwindowrect (hdlwindowinfo hinfo, Rect *r) {
	
	return (getglobalwindowrect ((**hinfo).macwindow, r));
	} /*shellgetglobalwindowrect*/
	
	
boolean shellgetwindowrect (hdlwindowinfo hinfo, Rect *r) {
	
	if (hinfo == nil)
		return (false);
		
#ifdef MACVERSION
	
	//Code change by Timothy Paustian Saturday, April 29, 2000 11:16:53 PM
	//Changed to Opaque call for Carbon
	#if ACCESSOR_CALLS_ARE_FUNCTIONS == 1
	{
	CGrafPtr	thePort = GetWindowPort((**hinfo).macwindow);
	GetPortBounds(thePort, r);
	}
	#else
	//old code
	*r = (*(**hinfo).macwindow).portRect; 
	#endif
#endif

#ifdef WIN95VERSION
	
	GetClientRect ((**hinfo).macwindow, r);
#endif

	return (true);
	} /*shellgetwindowrect*/
	

boolean shellgetwindowcenter (hdlwindowinfo hinfo, Rect *rcenter) {
	
	Rect r;
	
	shellgetglobalwindowrect (hinfo, &r); 
	
	zerorect (rcenter); /*we zoom from a point*/
	
	centerrect (rcenter, r);
	
	return (true);
	} /*shellgetwindowcenter*/


hdlfilenum windowgetfnum (WindowPtr w) {
	
	hdlwindowinfo hinfo;
	
	if (!getwindowinfo (w, &hinfo))
		return (0);
		
	return ((**hinfo).fnum);
	} /*windowgetfnum*/


void windowsetfnum (WindowPtr w, hdlfilenum fnum) {
	
	hdlwindowinfo hinfo;
	
	if (!getwindowinfo (w, &hinfo))
		return;
	
	(**hinfo).fnum = fnum;
	} /*windowsetfnum*/


short windowgetrnum (WindowPtr w) {
	
	hdlwindowinfo hinfo;
	
	if (!getwindowinfo (w, &hinfo))
		return (0);
		
	return ((**hinfo).rnum);
	} /*windowgetrnum*/


void windowsetrnum (WindowPtr w, short rnum) {
	
	hdlwindowinfo hinfo;
	
	if (!getwindowinfo (w, &hinfo))
		return;
	
	(**hinfo).rnum = rnum;
	} /*windowsetrnum*/
	
	
short windowgetvnum (WindowPtr w) {
	
	hdlwindowinfo hinfo;
	
	if (!getwindowinfo (w, &hinfo))
		return (0);
	
#ifdef MACVERSION
	return ((**hinfo).fspec.vRefNum);
#endif

#ifdef WIN95VERSION
	return -1; // *** need new field?
#endif
	} /*windowgetvnum*/
	
	
boolean windowsetfspec (WindowPtr w, tyfilespec *fspec) {

	hdlwindowinfo hinfo;
	bigstring bstitle;
	
	if (!getwindowinfo (w, &hinfo))
		return (false);
	
	(**hinfo).fspec = *fspec;
	
	getfsfile (fspec, bstitle);
	
	shellsetwindowtitle (hinfo, bstitle); // 7.24.97 dmb: was windowsettitle

	return (true);
	} /*windowsetfspec*/
	

boolean windowgetfspec (WindowPtr w, tyfilespec *fspec) {

	/*
	6.18.97 dmb: return boolean value indicating whether or not the
	fspec actually points to an existing file
	*/
	
	hdlwindowinfo hinfo;
	long vnum;
	boolean flfolder;
	
	clearbytes (fspec, sizeof (tyfilespec));
	
	if (!getwindowinfo (w, &hinfo))
		return (false);
	
	*fspec = (**hinfo).fspec;
	
	if (!getfsvolume (fspec, &vnum)) //don't allow default vol to satisfy fileexists
		return (false);
	
	return (fileexists (fspec, &flfolder));
	} /*windowgetfspec*/


boolean windowgetpath (WindowPtr w, bigstring bspath) {
	
	tyfilespec fs;
	
	if (!windowgetfspec (w, &fs))
		return (false);
	
	return (filespectopath (&fs, bspath));
	} /*windowgetpath*/


#ifndef version42orgreater

// dmb 12/17/96: it's not clear that any of the resource saving
// code should be used any more. font/size and window positioning
// information should be stored in the op or wp data files directly.

boolean shellsavewindowresource (WindowPtr wptr, ptrfilespec fspec, short rnum) {
	
	/*
	save information about the window's size and position in the 
	wpos resource in the given file.
	*/
	
	register WindowPtr w = wptr;
	hdlwindowinfo hinfo;
	register hdlwindowinfo h;
	tywindowposition wpos;
	
	if (!getwindowinfo (w, &hinfo)) /*defensive driving*/
		return (false);
	
	h = hinfo; /*copy into register*/
	
	wpos = (**h).wpos; /*copy from window data structure*/
	
	getscrollbarinfo ((**h).vertscrollbar, &wpos.vertmin, &wpos.vertmax, &wpos.vertcurrent);
	
	getscrollbarinfo ((**h).horizscrollbar, &wpos.horizmin, &wpos.horizmax, &wpos.horizcurrent);
	
	wpos.configresnum = (**h).configresnum;
	
	getglobalwindowrect (w, &wpos.windowrect);
	
	wpos.flhidden = (**h).flhidden;
	
	(**h).wpos = wpos; /*copy it back into the window data structure*/
	
	return (saveresource (fspec, rnum, 'wpos', 128, nil, sizeof (wpos), &wpos));
	} /*shellsavewindowresource*/


boolean shellsavewindowposition (WindowPtr wptr) {
	
	/*
	save information about the window's size and position in the 
	wpos resource in the file.
	*/
	
	register WindowPtr w = wptr;
	tyfilespec fspec;
	
	if (windowgetfnum (w) == 0) /*no file to save window info into*/
		return (false);
	
	windowgetfspec (w, &fspec);
	
	return (shellsavewindowresource (w, &fspec, windowgetrnum (w)));
	} /*shellsavewindowposition*/


boolean shellsavefontresource (WindowPtr w, ptrfilespec fspec, short rnum) {
	
	hdlwindowinfo hinfo;
	register hdlwindowinfo h;
	tysavedfont savedfont;
	register long len;
	
	if (!getwindowinfo (w, &hinfo)) /*defensive driving*/
		return (false);
	
	h = hinfo; /*copy into register*/
	
	fontgetname ((**h).defaultfont, savedfont.fontname);
	
	savedfont.fontsize = (**h).defaultsize;
	
	savedfont.fontstyle = (**h).defaultstyle;
	
	len = sizeof (savedfont) - sizeof (bigstring) + stringlength (savedfont.fontname) + 1;
	
	return (saveresource (fspec, rnum, 'styl', 128, nil, len, &savedfont));
	} /*shellsavefontresource*/


boolean shellsavedefaultfont (register WindowPtr w) {
	
	tyfilespec fspec;
	
	if (windowgetfnum (w) == 0) /*no file to save window info into*/
		return (false);
	
	windowgetfspec (w, &fspec);
	
	return (shellsavefontresource (w, &fspec, windowgetrnum (w)));
	} /*shellsavedefaultfont*/

#endif	// MACVERSION


boolean loadwindowposition (ptrfilespec fspec, short rnum, tywindowposition *wpos) {
	
	return (loadresource (fspec, rnum, 'wpos', 128, nil, sizeof (tywindowposition), wpos, resourcefork)); /* 2005-09-02 creedon - added support for fork parameter, see resources.c: openresourcefile and pushresourcefile */ 
	} /*loadwindowposition*/


boolean loaddefaultfont (WindowPtr w) {

	hdlwindowinfo hinfo;
	register hdlwindowinfo h;
	tyfilespec fspec;
	tysavedfont savedfont;
	short fontnum;
	long resourceSize;
	if (!getwindowinfo (w, &hinfo)) /*defensive driving*/
		return (false);
	
	h = hinfo; /*copy into register*/
	
	if ((**h).fnum == 0) /*no file to save font info into*/
		return (false);
	
	windowgetfspec (w, &fspec);
	
	//bug notice by Timothy Paustian Friday, May 19, 2000 1:03:18 PM
	//the size of a style resource in the root in not the size of the savedfont
	//type. savedfont is 260 while the resource is 11 in the root. 
	//I think a more insidious problem was that the sizeof() was returning
	//a short that was not being picked up right when run under spotlight. 
	//This may be a spotlight bug though.
	resourceSize = sizeof (savedfont);
	if (!loadresource (&fspec, (short) (**h).rnum, 'styl', 128, nil, resourceSize, &savedfont, resourcefork)) { /* 2005-09-02 creedon - added support for fork parameter, see resources.c: openresourcefile and pushresourcefile */
		
		(**h).defaultfont = config.defaultfont;
		
		(**h).defaultsize = config.defaultsize;
		
		(**h).defaultstyle = config.defaultstyle;
		
		return (false);
		}
		
	(**h).defaultsize = savedfont.fontsize;
	
	(**h).defaultstyle = savedfont.fontstyle;
	
	fontgetnumber (savedfont.fontname, &fontnum);
	
	(**h).defaultfont = fontnum;
	
	return (true);
	} /*loaddefaultfont*/


void getdefaultwindowrect (Rect *rdefault) {
	
	hdlwindowinfo hinfo;
	Rect r;
	register short height, width;
	Rect rfront;
	Rect rdesktop;
	register short h, v;
	
#ifdef MACVERSION
	#if TARGET_API_MAC_CARBON == 1
		{
		BitMap	screenBits;
		GetQDGlobalsScreenBits(&screenBits);
		rdesktop = r = screenBits.bounds;
		}
		#else
		rdesktop = r = qd.screenBits.bounds;
	#endif
		
#endif

#ifdef WIN95VERSION
	getglobalwindowrect (shellframewindow, &rdesktop);

	r = rdesktop;
#endif
	
	width = r.right - r.left;
	
	height = r.bottom - r.top;
	
	r.left += (width / 6);
	
	r.right -= (width / 6);
	
	r.top += height / 7;
	
	r.bottom = r.top + (height / 3);
	
	*rdefault = r;
	
	if (!getfrontwindowinfo (&hinfo)) /*no front window to position relative to*/
		return;
	
	shellgetglobalwindowrect (hinfo, &rfront);
	
	h = rfront.left + 12;
	
	v = rfront.top + 12;
	
	offsetrect (&r, h - r.left, v - r.top);
	
	if (issubrect (r, rdesktop))
		*rdefault = r;
	} /*getdefaultwindowrect*/


void shellcalcgrowiconrect (Rect rwholewindow, hdlwindowinfo hinfo) {

	Rect r = rwholewindow;
	register short scrollbarwidth;
	
	scrollbarwidth = getscrollbarwidth ();
	
	r.left = r.right - scrollbarwidth;
	
	r.top = r.bottom - scrollbarwidth;
	
	offsetrect (&r, 1, 1);
	
	(**hinfo).growiconrect = r;
	} /*shellcalcgrowiconrect*/


void shellresetwindowrects (hdlwindowinfo hinfo) {
	
	/*
	resize all the rectangles stored in the window's information record.
	
	dmb 10/24/90: on entry, buttons rect is now already cleared, and the 
	contentrect is initialized to the portrect
	
	7.0b26 PBS: tweaks for having a message area but no horizontal scrollbar.
	*/
	
	register hdlwindowinfo h;
	register short messagewidth;
	register short scrollbarwidth;
	Rect messagerect;
	Rect rwholewindow;
	Rect r;
	Rect buttonsrect;
	
	h = hinfo; /*copy into register*/
	
	scrollbarwidth = getscrollbarwidth ();
	
	rwholewindow = (**h).contentrect;
	
	r = rwholewindow;
	
	if (config.flvertscroll) /*leave room for vertical scrollbar*/
		r.right -= scrollbarwidth;
	
	if (config.flhorizscroll || config.flmessagearea) /*leave room for horiz scrollbar*/ /*7.0b26 PBS: leave room for msg, even if no scrollbar.*/
		r.bottom -= scrollbarwidth;
	
	if ((shellglobals.buttonlist != nil) && (!(**h).flhidebuttons)) { /*window type has attached button list*/
		
		shellgetbuttonsrect (h, &buttonsrect);
		
		if (config.flvertscroll)
			buttonsrect.right -= scrollbarwidth - 1;
		
		r.top = buttonsrect.bottom;
		
		(**h).buttonsrect = buttonsrect;
		}
	
	if (config.flinsetcontentrect)
		insetrect (&r, 3, 3);
	
	(**h).contentrect = r;
	
	if (config.flgrowable)
		shellcalcgrowiconrect (rwholewindow, h);
	
	if (config.flmessagearea) {
		
		r = rwholewindow;
		
		//if (config.flhorizscroll) 
		
			messagewidth = (r.right - r.left - scrollbarwidth) / config.messageareafraction;
		
		//else
		
			//messagewidth = (r.right - r.left); /*7.0b26 PBS: if no horiz scrollbar, message gets entire space.*/
		
		messagerect.top = r.bottom - scrollbarwidth + 1;
		
		messagerect.left = r.left - 1;
		
		messagerect.bottom = r.bottom + 1;
		
		messagerect.right = r.left + messagewidth;		
		}
	else {
	
		messagewidth = 0;
		
		clearbytes (&messagerect, sizeof (messagerect));
		}
		
	(**h).messagerect = messagerect;
	
	if (config.flvertscroll) { /*position the vertical scrollbar*/
		
		register hdlscrollbar scrollbar = (**h).vertscrollbar;
		
		r = rwholewindow; 
	
		r.bottom -= scrollbarwidth - 1;
		
		scrollbarflushright (r, scrollbar);
		}
		
	if (config.flhorizscroll) { /*position the horizontal scrollbar*/
	
		register hdlscrollbar scrollbar = (**h).horizscrollbar;
		
		r = rwholewindow; 
	
		r.right -= scrollbarwidth - 2;
	
		r.left += messagewidth - 1; /*leave room for the message area*/
	
		scrollbarflushbottom (r, scrollbar);
		}
	} /*shellresetwindowrects*/


void windowresetrects (hdlwindowinfo hinfo) {
	
	/*
	(re)initialize all of the rectangles stored in the window's information 
	record.  start by clearing the buttons rect and setting the content rect 
	to be the owning window's portrect.  the resetwindowrects callback routine 
	is responsible for:
		1.  adjust the contentrect, if necessary
		2.  setting the buttons rect, if used
		3.  positioning any scrollbars
		4.  setting up the grow box rect
	*/
	
	register hdlwindowinfo h = hinfo;
	
	// (**h).contentrect = (*(**h).macwindow).portRect;
	
	getlocalwindowrect((**h).macwindow, &(**h).contentrect);
	
	zerorect (&(**h).buttonsrect); /*fill with zeros*/
	
	(*shellglobals.resetrectsroutine) (h);
	} /*windowresetrects*/


boolean emptywindowlist (void) {
	
	return (hfirstwindow == nil);
	} /*emptywindowlist*/
			
	
short countwindowlist (void) {
	
	/*
	return the number of windows in the window list.
	*/
	
	register hdlwindowinfo nomad, firstnomad, nextnomad;
	register short ct;
	
	nomad = hfirstwindow;
	
	if (nomad == nil) /*special case for empty list*/
		return (0);
	
	firstnomad = nomad;
	
	ct = 1; /*at least one element in the list*/
	
	while (true) {
	
		nextnomad = (**nomad).nextwindow;
		
		if (nextnomad == firstnomad) /*reached the end of the list*/
			return (ct);
			
		ct++;
		
		nomad = nextnomad;
		} /*while*/
	} /*countwindowlist*/
	

boolean indexwindowlist (short windownumber, hdlwindowinfo *hwindow) {
	
	/*
	finds the nth window in the window list.  n is 1-based.
	
	return false if there aren't that many windows.
	*/
	
	register hdlwindowinfo nomad, firstnomad, nextnomad;
	register short i, ct;
	
	*hwindow = nil;
	
	nomad = hfirstwindow;
	
	if (nomad == nil) /*special case for empty list*/
		return (false);
	
	firstnomad = nomad;
	
	ct = windownumber - 1; /*iterate n - 1 times*/
	
	for (i = 1; i <= ct; i++) {
	
		nextnomad = (**nomad).nextwindow;
		
		if (nextnomad == firstnomad) /*reached the end of the list, element doesn't exist*/
			return (false);
			
		nomad = nextnomad;
		} /*while*/
	
	*hwindow = nomad; /*loop terminated, it actually has that many elements*/
	
	return (true);
	} /*indexwindowlist*/


/*3/1/91 dmb: unused

boolean firstrootwindow (hdlwindowinfo *hinfo) {
	
	/%
	a root window is one with no parent.  we travel through the window
	list and return the info record for the first such window.
	
	return false if there aren't any root windows;
	%/
	
	*hinfo = hfirstwindow; /%start search with first window%/
	
	return (nextrootwindow (hinfo));
	} /%firstrootwindow%/
	

boolean nextrootwindow (hdlwindowinfo *hinfo) {
	
	/%
	a root window is one with no parent.  we travel through the window
	list and return the info record for the next such window.
	
	return false if there aren't any root windows;
	%/
	
	register hdlwindowinfo nomad = *hinfo;
	register hdlwindowinfo firstnomad = nomad;
	register hdlwindowinfo nextnomad;
	
	if (nomad == nil) /%empty list, no more root windows%/
		return (false);
	
	while (true) { /%check each node in the window list%/
		
		if ((**nomad).parentwindow == nil) { /%a root window%/
			
			*hinfo = nomad;
			
			return (true);
			}
	
		nextnomad = (**nomad).nextwindow;
		
		if (nextnomad == hfirstwindow) { /%reached end of list, no more root windows%/
			
			*hinfo = nil;
			
			return (false);
			}
			
		nomad = nextnomad;
		} /%while%/
	} /%nextrootwindow%/


boolean visitrootwindows (boolean (*visit) ()) {
	
	hdlwindowinfo nomad;
	
	if (!firstrootwindow (&nomad))
		return (false);
		
	while (true) {
		
		if (!(*visit) (nomad))
			return (true);
		
		nomad = (**nomad).nextwindow;
		
		if (nomad == hfirstwindow) /%wrapped around to head of list%/
			return (true);
		
		if (!nextrootwindow (&nomad)) /%finished visiting%/
			return (true);
		} /%while%/
	} /%visitrootwindows%/
*/


boolean getrootwindow (WindowPtr w, hdlwindowinfo *hrootinfo) {

	hdlwindowinfo hinfo;
	register hdlwindowinfo nomad, nextnomad;
	
	if (!getwindowinfo (w, &hinfo))
		return (false);
		
	nomad = hinfo;
	
	while (true) { /*traverse the parent links, if any*/
		
		nextnomad = (**nomad).parentwindow;
		
		if (nextnomad == nil) { /*at a root window*/
			
			*hrootinfo = nomad;
			
			return (true);
			}
		
		nomad = nextnomad; /*keep traversing for the root*/
		} /*while*/
	} /*getrootwindow*/
	
	
boolean frontrootwindow (hdlwindowinfo *hrootinfo) {

	/*
	a root window is one with no parent.  we look at the frontmost window, and
	if it's a root window, return its info record handle.
	
	otherwise we return a handle to the first root window we encounter traversing
	the parent links of the info record.
	
	10/3/92 dmb: try to handle case where user hides all windows
	*/
	
	WindowPtr w;
	
	if (!frontshellwindow (&w)) { /*none of our windows are open*/
		
		if (hfirstwindow == nil)
			return (false);
		
		w = (**hfirstwindow).macwindow; /*it's hidden, but it's the first window opened*/
		}
	
	return (getrootwindow (w, hrootinfo));
	} /*frontrootwindow*/


WindowPtr shellnextwindow (WindowPtr pwindow) {
	
	/*
	return the next window in the system window list that is a 
	shell window, or nil.
	*/
	
	register WindowPtr w = pwindow;
	
	while (true) {
		
		w = getnextwindow (w); /*advance to next window in the list*/
		
		if (w == nil) /*no more window in the list*/
			break;
		
		if (isshellwindow (w))
			break;
		}
	
	return (w);
	} /*shellnextwindow*/


boolean shellvisitwindowlayer (shellwindowvisitcallback visit, ptrvoid refcon) {
	
	/*
	visits all windows in front to back order.  to do this, we must use 
	the OS's window list, since ours has no particular order
	
	if the visit routine returns false, we return false immediately.  if all
	visits returned true, we return true.
	*/
	
	register WindowPtr w;
	
	w = getfrontwindow ();
	
	while (w != nil) {
		
		if (isshellwindow (w))
			if (!(*visit) (w, refcon))
				return (false);
		
		w = shellnextwindow (w); /*advance to next window in the list*/
		}
	
	return (true);
	} /*shellvisitwindowlayer*/


boolean visitownedwindows (WindowPtr w, short id, shellwindowvisitcallback visit, ptrvoid refcon) {
	
	/*
	w should be a root window, one whose parent is nil.
	
	we call the visit routine for each window whose root window is w whose type
	equals the indicated id.  if id == -1 then we visit all owned windows,
	regardless of type.
	
	if the visit routine returns false, we return false immediately.  if all
	visits returned true, we return true.
	*/
	
	register hdlwindowinfo nomad = hfirstwindow;
	hdlwindowinfo hparentinfo;
	
	if (!getwindowinfo (w, &hparentinfo))
		return (false);
	
	while (true) {
		
		hdlwindowinfo nextnomad = (**nomad).nextwindow;
		hdlwindowinfo hinfo;
		
		if (getrootwindow ((**nomad).macwindow, &hinfo)) {
			
			if (hinfo == hparentinfo)
				
				if (((**nomad).configresnum == id) || (id == -1))
			
					if (!(*visit) ((**nomad).macwindow, refcon))
						return (false);
			}
			
		nomad = nextnomad; /*nomad might have been dealloc'd already*/
		
		if (nomad == hfirstwindow) /*wrapped around to head of list*/
			return (true);
		} /*while*/
	} /*visitownedwindows*/
	
	
boolean shellvisittypedwindows (short id, shellwindowvisitcallback visit, ptrvoid refcon) {
	
	/*
	visit all windows whose configresum equals id.  
	
	to visit all windows, set id to -1.
	
	if the visit routine returns false, we return false immediately.  if all
	visits returned true, we return true.
	*/
	
	register hdlwindowinfo nomad = hfirstwindow;
	
	if (nomad == nil) /*no windows open*/
		return (true);
		
	while (true) {
		
		hdlwindowinfo nextnomad = (**nomad).nextwindow;
		
		if (((**nomad).configresnum == id) || (id == -1)) {
			
			if (!(*visit) ((**nomad).macwindow, refcon))
				return (false);
			}
		
		nomad = nextnomad; /*nomad might have been dealloc'd already*/
		
		if (nomad == hfirstwindow) /*wrapped around to head of list*/
			return (true);
		} /*while*/
	} /*shellvisittypedwindows*/


static boolean counttypedvisit (WindowPtr w,  ptrvoid refcon) {
#pragma unused(w)

	(*(short *)refcon)++;
	
	return (true);
	} /*counttypedvisit*/


short counttypedwindows (short id) {
	
	short windowcount = 0;
	
	shellvisittypedwindows (id, &counttypedvisit, &windowcount);
	
	return (windowcount);
	} /*counttypedwindows*/


typedef struct finddatainfo {
	
	Handle hdata;
	
	hdlwindowinfo hinfo;
	} tyfinddatainfo;


static boolean shellfinddatavisit (WindowPtr w, ptrvoid refcon) {
	
	tyfinddatainfo *findinfo = (tyfinddatainfo *) refcon;
	hdlwindowinfo hinfo;
	
	if (getwindowinfo (w, &hinfo)) {
		
		if ((**hinfo).hdata == (*findinfo).hdata) { /*found the handle we're looking for*/
			
			(*findinfo).hinfo = hinfo;
			
			return (false);
			}
		}
	
	return (true);
	} /*shellfinddatavisit*/


boolean shellfinddatawindow (Handle hdata, hdlwindowinfo *hinfo) {
	
	/*
	search the window list looking for a window that has the indicated data
	handle linked into it.  return true if such a window was found, false
	otherwise.
	*/
	
	tyfinddatainfo findinfo;

	if (hdata == nil)
		return (false);

	findinfo.hdata = hdata;
	
	findinfo.hinfo = nil;
	
	shellvisittypedwindows (-1, &shellfinddatavisit, &findinfo);
	
	*hinfo = findinfo.hinfo;
	
	return (findinfo.hinfo != nil);
	} /*shellfinddatawindow*/


typedef struct findfileinfo {
	
	ptrfilespec fsfind;
	
	WindowPtr wfound;
	} tyfindfileinfo;


static boolean shellfindfilevisit (WindowPtr w, ptrvoid refcon) {
	
	/*
	if the filespec for the given window is the one we're
	 looking for, select that window and stop visiting
	*/
	
	tyfindfileinfo *findinfo = (tyfindfileinfo *) refcon;
	tyfilespec fs;
	
	windowgetfspec (w, &fs);
	
	if (equalfilespecs ((*findinfo).fsfind, &fs)) {
		
		(*findinfo).wfound = w;
		
		return (false);
		}
	
	return (true);
	} /*shellfindfilevisit*/


WindowPtr shellfindfilewindow (ptrfilespec fs) {
	
	/*
	4.1b7 dmb: return the window of the root who's fspec
	matches fs, nil if not found.
	*/
	
	tyfindfileinfo findinfo;
	
	findinfo.fsfind = fs;
	
	if (!shellvisittypedwindows (idcancoonconfig, &shellfindfilevisit, &findinfo))
		return (findinfo.wfound);
	
	return (nil);
	} /*shellfindfilewindow*/


typedef struct findtitleinfo {
	
	ptrstring pwindowtitle;
	
	hdlwindowinfo *hinfo;
	} tyfindtitleinfo;


static boolean findpathvisit (WindowPtr w, ptrvoid refcon) {
	
	tyfindtitleinfo *findinfo = (tyfindtitleinfo *) refcon;
	bigstring bs;
	
	if (windowgetpath (w, bs) && equalidentifiers (bs, (*findinfo).pwindowtitle)) { /*found it -- set values and terminate visit*/
		
		getwindowinfo (w, (*findinfo).hinfo);
		
		return (false);
		}
	
	return (true); /*keep visiting*/
	} /*findpathvisit*/


static boolean findtitlevisit (WindowPtr w, ptrvoid refcon) {
	
	tyfindtitleinfo *findinfo = (tyfindtitleinfo *) refcon;
	hdlwindowinfo hinfo;
	bigstring bs;
	
	getwindowinfo (w, &hinfo);

	shellgetwindowtitle (hinfo, bs); // 7.24.97 dmb: was windowgettitle
	
	if (equalidentifiers (bs, (*findinfo).pwindowtitle)) { /*found it -- set values and terminate visit*/
		
		*(*findinfo).hinfo = hinfo;
		
		return (false);
		}
	
	return (true); /*keep visiting*/
	} /*findtitlevisit*/


boolean shellfindnamedwindow (const bigstring bsname, hdlwindowinfo *hinfo) {
	
	/*
	7.2.97 dmb: find the window whose title or file path is bsname
	*/
	
	tyfindtitleinfo findinfo;
	
	*hinfo = nil; /*default return if not found*/
	
	findinfo.pwindowtitle = (ptrstring) bsname; /*so visit routine can see it*/
	
	findinfo.hinfo = hinfo; /*so visit routine can set it*/
	
	if (shellvisittypedwindows (-1, &findpathvisit, &findinfo) &&
		shellvisittypedwindows (-1, &findtitlevisit, &findinfo))
		
		return (false);

	return (*hinfo != nil);
	} /*shellfindnamedwindow*/


#if isFrontier

boolean shellclosedatawindow (Handle hdata) {
	
	hdlwindowinfo hinfo;
	
	if (!shellfinddatawindow (hdata, &hinfo)) /*easy - already closed*/
		return (true);
	
	return (shellclosewindow ((**hinfo).macwindow));
	} /*shellclosedatawindow*/

#endif


boolean shellfirstchildwindow (hdlwindowinfo hparent, hdlwindowinfo *hchild) {
	
	/*
	return the first window in the window list whose parent is hparent.
	
	return true if we found one, false otherwise.
	*/
	
	register hdlwindowinfo nomad = hfirstwindow;
	register hdlwindowinfo firstnomad = nomad;
	
	if (nomad == nil) /*an empty list*/
		return (false);
		
	while (true) {
		
		if ((**nomad).parentwindow == hparent) {	
			
			*hchild = nomad;
			
			return (true);
			}
			
		nomad = (**nomad).nextwindow;
		
		if (nomad == firstnomad) /*cycled around*/
			return (false);
		} /*while*/
	} /*shellfirstchildwindow*/


static boolean graywindowvisit (WindowPtr w, ptrvoid refcon) {
#pragma unused (refcon)

	graywindow (w);
	
	return (true);
	} /*graywindowvisit*/


void grayownedwindows (WindowPtr w) {
	
	graywindow (w);
	
	visitownedwindows (w, -1, &graywindowvisit, nil);
	} /*grayownedwindows*/
	

static void insertwindowlist (hdlwindowinfo hwindow) {

	/*
	link the window in at the end of the window list.
	
	if the list is empty (hfirstwindow == nil) then set up a one-window
	list.
	
	the list is circular and doubly linked.
	*/
	
	register hdlwindowinfo hprev;
	
	shellwindowmenudirty ();
	
	if (hfirstwindow == nil) { /*no windows open -- this is the first window*/
	
		hfirstwindow = hwindow; /*doubly linked and circular list*/
		
		(**hwindow).nextwindow = hwindow;
		
		(**hwindow).prevwindow = hwindow;
		
		return;
		}
	
	hprev = (**hfirstwindow).prevwindow; /*link into a non-empty list*/
	
	(**hprev).nextwindow = hwindow;
	
	(**hwindow).prevwindow = hprev;
	
	(**hwindow).nextwindow = hfirstwindow;
	
	(**hfirstwindow).prevwindow = hwindow; /*new window becomes last in list*/
	} /*insertwindowlist*/


static void unlinkwindowlist (hdlwindowinfo hwindow) {

	/*
	unlink the window from the window list.
	
	if it's the only window, indicate that the list is now empty.
	
	if it's the first window, unlink it and set hfirstwindow to point at
	the next window.
	*/

	hdlwindowinfo hnext, hprev;
	
	hnext = (**hwindow).nextwindow;
	
	if (hnext == nil) //not in window list anymore
		return;
	
	(**hwindow).nextwindow = nil; //mark as not being in list
	
	shellwindowmenudirty ();
	
	if (hnext == hwindow) { /*one-element list*/
		
		hfirstwindow = nil; /*the list is now empty*/
		
		return;
		}
		
	hprev = (**hwindow).prevwindow; /*unlink from a >1 element list*/
	
	(**hprev).nextwindow = hnext;
	
	(**hnext).prevwindow = hprev;
	
	if (hfirstwindow == hwindow)
		hfirstwindow = hnext;
	} /*unlinkwindowlist*/


static void enforceminimumsize (Rect *rwindow) {
	
	/*
	be sure the window rect is at least as large as config's rmin
	*/
	
	register Rect *r = rwindow;
	register short minwidth, minheight;
	
	minwidth = config.rmin.right - config.rmin.left;
	
	minheight = config.rmin.bottom - config.rmin.top;
	
	(*r).right = max ((*r).right, (*r).left + minwidth);
	
	(*r).bottom = max ((*r).bottom, (*r).top + minheight);
	} /*enforceminimumsize*/


static void constrainwindowrect (WindowPtr w, Rect *rwindow) {
	
	/*
	make sure the rectangle is visible within the current desktop.
	
	then make sure it's at least the minimum required size for a 
	window, according to the current config record.
	
	9/27/91 dmb: now take window parameter and pass through to constraintodesktop
	
	2.1b2 dmb: contrain top-right of window to a particular screen, 
	supplementing desktop bounds constraint
	
	2.1b5 dmb: no; if top right of window isn't visible on its screen after 
	desktop constraint, constrain the entire window to its screen
	*/
	
	Rect r, rmin;
	
	r = *rwindow; /*copy into a local*/
	
	constraintodesktop (w, &r);
	
	/*make sure top-right corner (zoom box) is accessable*/
	
	setrect (&rmin, r.top, r.right - 50, r.top + 5, r.right);
	
	if (constraintoscreenbounds (w, false, &rmin))
		constraintoscreenbounds (w, false, &r);
	
	/*be sure the rect is at least as large as config's rmin*/
	
	enforceminimumsize (&r);
	
	*rwindow = r; /*copy back into parameter*/
	} /*constrainwindowrect*/


void disposeshellwindowinfo (hdlwindowinfo hinfo) {
	
	/*
	10/5/92 dmb: dispose hundostring (fixes small memory leak)
	
	4.1b1 dmb: new name; public; take card of window list here
	
	5.1.3 dmb: handle nil hinfo
	*/
	
	register hdlwindowinfo h = hinfo;
	
	if (h == nil)
		return;
	
	unlinkwindowlist (h);
	
	disposehandle ((Handle) (**h).hmessagestring);
	
	disposehandle ((Handle) (**h).hundostring);
	
	disposescrollbar ((**h).vertscrollbar);
	
	disposescrollbar ((**h).horizscrollbar);
	
	disposeundostack ((hdlundostack) (**h).hundostack);
	
	disposeundostack ((hdlundostack) (**h).hredostack);
	
	disposehandle ((Handle) (**h).hwindowtitle);

	disposehandle ((Handle) h);
	} /*disposeshellwindowinfo*/


boolean defaultselectioninfo (hdlwindowinfo hinfo) {
	
	/*
	6/28/91 dmb: now use union for style to make things a lot easier
	*/
	
	register hdlwindowinfo hw = hinfo;
	tyselectioninfo x;
	
	clearbytes (&x, sizeof (x)); /*initialize all fields to zero*/
	
	x.flcansetfont = true;
	
	x.flcansetsize = true;
	
	x.flcansetstyle = true;
	
	x.fontnum = (**hw).defaultfont; 
	
	x.fontsize = (**hw).defaultsize; 
	
	x.fontstyle = (**hw).defaultstyle;
	
	(**hw).selectioninfo = x;
	
	return (true);
	} /*defaultselectioninfo*/
	

boolean newshellwindowinfo (WindowPtr w, hdlwindowinfo *hinfo) {
	
	/*
	4.1b1 dmb: new name; public; take card of window list here
	*/
	
	hdlwindowinfo h;
	hdlundostack hundostack;
	hdlstring htitle;
	
	if (!newclearhandle (sizeof (tywindowinfo), (Handle *) hinfo))
		return (false);
	
	h = *hinfo;
	
	setwindowrefcon (w, (long) h); /*window points at info structure*/
	
	(**h).macwindow = w; /*the pointing is mutual*/
	
	insertwindowlist (h);
	
	if (!newundostack (&hundostack))
		goto error;
	
	(**h).hundostack = (Handle) hundostack;
	
	if (!newundostack (&hundostack))
		goto error;
	
	(**h).hredostack = (Handle) hundostack;
	
	if (!newheapstring (zerostring, &htitle))
		goto error;
	
	(**h).hwindowtitle = htitle;

	return (true);
	
	error:
	
		disposeshellwindowinfo (h);
		
		return (false);
	} /*newshellwindowinfo*/


boolean newshellwindow (WindowPtr *wnew, hdlwindowinfo *hnew, tywindowposition *wpos) {
	
	register WindowPtr w;
	register hdlwindowinfo hinfo = nil;
	hdlscrollbar vertbar = nil, horizbar = nil;
#ifdef WIN95VERSION
	tyconfigrecord origconfig = config;
	tyshellglobals origglobals = shellglobals;
	GrafPtr saveport = getport ();
#endif
	
	w = getnewwindow (config.templateresnum, config.fldialog, &(*wpos).windowrect);
	
#ifdef WIN95VERSION
	config = origconfig;
	shellglobals = origglobals;
	setport (saveport);
#endif
	
	*wnew = w; /*copy into returned value*/
	
	if (w == nil) 
		return (false);
	
	if (!newshellwindowinfo (w, hnew))
		goto error;
	
	hinfo = *hnew; /*copy into register*/
	
	(**hinfo).defaultfont = config.defaultfont;
	
	(**hinfo).defaultsize = config.defaultsize;
	
	(**hinfo).defaultstyle = config.defaultstyle;
	
	(**hinfo).processid = getcurrentprocessid ();

	defaultselectioninfo (hinfo);
	
	if (config.flvertscroll) {
		
		if (!newscrollbar (w, true, &vertbar)) 
			goto error;
		
		(**hinfo).vertscrollbar = vertbar;
		}
	
	if (config.flhorizscroll) {
		
		if (!newscrollbar (w, false, &horizbar)) 
			goto error;
			
		(**hinfo).horizscrollbar = horizbar;
		}
	
	return (true);
	
	error: /*goto here on an error, everything that's been allocated gets deallocated*/
	
	disposeshellwindowinfo (hinfo);
	
	if (w != nil)
		disposewindow (w);
	
	return (false);
	} /*newshellwindow*/

	
boolean newfilewindow (ptrfilespec fspec, hdlfilenum fnum, short rnum, boolean flhidden, WindowPtr *wnew) { 
	
	/*
	create a new window to display the contents of a file.
	
	12/1/92 dmb: if we're passed a full path and zero vnum, get vnum from path
	
	2.1b2 dmb: updated for filespecs
	*/
	
	register WindowPtr w;
	register hdlwindowinfo hinfo;
	register hdlscrollbar vertbar, horizbar;
	hdlwindowinfo hwindowinfo = nil;
	tywindowposition wpos;
	bigstring bspath;
	
	// 5.6.97 dmb: calc wpos first, and pass it to newshellwindow

	if ((fnum == 0) || !loadwindowposition (fspec, rnum, &wpos)) {
		
		clearbytes (&wpos, sizeof (wpos));
		
	//	getdefaultwindowrect (&wpos.windowrect);
		wpos.windowrect = config.defaultwindowrect;
		
		wpos.configresnum = iddefaultconfig;
		}
	
	wpos.ctopens++; /*it's been opened one more time*/
	
	if (!newshellwindow (wnew, &hwindowinfo, &wpos))
		return (false);
	
	hinfo = hwindowinfo; /*copy into register*/
	
	w = *wnew; /*copy into register*/
	
	//Code change by Timothy Paustian Monday, August 21, 2000 4:31:49 PM
	//Must pass a CGrafPtr to pushport on OS X to avoid a crash
	{
	CGrafPtr	thePort;
	#if TARGET_API_MAC_CARBON == 1
	thePort = GetWindowPort(w);
	#else
	thePort = (CGrafPtr)w;
	#endif
		
	pushport (thePort);
	}	
	vertbar = (**hinfo).vertscrollbar;
	
	horizbar = (**hinfo).horizscrollbar;
	
	(**hinfo).fspec = *fspec;
		
	(**hinfo).fnum = fnum;
	
	(**hinfo).rnum = rnum;
	
	(**hinfo).wpos = wpos; /*save it so application can refer to it*/
	
	(**hinfo).configresnum = wpos.configresnum;
	
	(**hinfo).flhidden = flhidden || wpos.flhidden;
	
	getfsfile (fspec, bspath);
	
	shellsetwindowtitle (hinfo, bspath);
	
	windowresetrects (hinfo);
	
	*shellglobals.infoholder = hinfo;
	
	#ifndef version5orgreater
		setscrollbarinfo (vertbar, wpos.vertmin, wpos.vertmax, wpos.vertcurrent);
		
		setscrollbarinfo (horizbar, wpos.horizmin, wpos.horizmax, wpos.horizcurrent);
	#endif
	
	loaddefaultfont (w);
	
	popport ();
	
	return (true);
	} /*newfilewindow*/
	

boolean newchildwindow (short idtype, hdlwindowinfo hparentinfo, Rect * rwindow, rectparam rzoom, bigstring bstitle, WindowPtr *wnew) {
	
	/*
	create a new window, and a windowinfo record linked into it.
	
	the window can be linked to a parent, hparentinfo is nil then the window is a 
	root, subordinate to no other window.
	
	rwindow describes the window's size and position on the desktop.
	
	rzoom is the rectangle that the window zooms from, but the zooming doesn't take
	place now, the rect is recorded in the windowinfo record and zooming occurs 
	when the application calls windowzoom.
	
	bstitle is the title of the window.  if it's nil, don't set the title.
	
	a pointer to the Macintosh window record is returned.
	
	10/29/91 dmb: only use default rect if top is -1, not any negative number
	
	10/3/92 dmb: setfontsizestyle after pushing new port

	5.0a8 dmb: set title at end; it can smash globals

	5.0a24 dmb: constrain rwindow
	*/
	
	register hdlwindowinfo hinfo;
	register hdlscrollbar vertbar, horizbar;
	hdlwindowinfo hwindowinfo;
	register WindowPtr w;
	short ix;
	tyconfigrecord origconfig;
	tyshellglobals origglobals;
	register boolean fl = false;
	tywindowposition wpos;
	
	if (!shellfindcallbacks (idtype, &ix))
		return (false);
		
	origconfig = config; /*save, we'll restore later*/
	
	origglobals = shellglobals; /*save*/
	
	shellglobals = globalsarray [ix];
	
	config = shellglobals.config;
	
	// 5.6.97 dmb: calc wpos first, and pass it to newshellwindow

	clearbytes (&wpos, sizeof (wpos));
	
	if ((*rwindow).top != -1) { /*-1 is a signal to use default rect*/
		
		constrainwindowrect (nil, rwindow);

		wpos.windowrect = *rwindow;
		}
	
	else {
		hdlwindowinfo hfrontinfo;
		
		wpos.windowrect = config.defaultwindowrect;
		
		if (getfrontwindowinfo (&hfrontinfo)) {
			
			if ((**hfrontinfo).configresnum == idtype) {
				
				Rect wrect;
				
				getglobalwindowrect ((**hfrontinfo).macwindow, &wrect);
				
				offsetrect (&wpos.windowrect, wrect.left - wpos.windowrect.left, wrect.top - wpos.windowrect.top);
				
				offsetrect (&wpos.windowrect, doctitlebarheight, doctitlebarheight);
				}
			}
		}
	
	wpos.configresnum = idtype;
	
	if (!newshellwindow (wnew, &hwindowinfo, &wpos))
		goto exit;
	
	hinfo = hwindowinfo; /*copy into register*/
	
	w = *wnew; /*copy into register*/
	
	//Code change by Timothy Paustian Monday, August 21, 2000 4:31:49 PM
	//Must pass a CGrafPtr to pushport on OS X to avoid a crash
	{
	CGrafPtr	thePort;
	#if TARGET_API_MAC_CARBON == 1
	thePort = GetWindowPort(w);
	#else
	thePort = (CGrafPtr)w;
	#endif
		
	pushport (thePort);
	}
	
	setfontsizestyle (config.defaultfont, config.defaultsize, config.defaultstyle);
	
	popport ();
	
	vertbar = (**hinfo).vertscrollbar; /*copy into register*/
	
	horizbar = (**hinfo).horizscrollbar; /*copy into register*/
	
	(**hinfo).parentwindow = hparentinfo;
	
	(**hinfo).configresnum = idtype;
	
	(**hinfo).wpos = wpos; /*save it so application can refer to it*/
	
	windowresetrects (hinfo);
	
	*shellglobals.infoholder = hinfo;
	
	#ifndef version42orgreater
		setscrollbarinfo (vertbar, wpos.vertmin, wpos.vertmax, wpos.vertcurrent);
		
		setscrollbarinfo (horizbar, wpos.horizmin, wpos.horizmax, wpos.horizcurrent);
	#endif
	
	(**hinfo).rzoomfrom = *rzoom; /*so that we can zoom the window later*/
	
	if (bstitle != nil) {
		
		shellsetwindowtitle (hinfo, bstitle); // 7.24.97 dmb: was windowsettitle
		
		copystring (bstitle, fsname (&(**hinfo).fspec));
		}
	
	fl = true;
	
	exit:
	
	config = origconfig; /*restore*/
	
	shellglobals = origglobals; /*restore*/
	
	return (fl);
	} /*newchildwindow*/
	
	
boolean windowinit (WindowPtr w) {
	
	if (!shellpushglobals (w))
		return (false);
		
	if ((*shellglobals.newrecordroutine) ()) {
		
		(**shellwindowinfo).hdata = *shellglobals.dataholder; /*link data to window*/
		}
	
	shellpopglobals ();
	
	return (true);
	} /*windowinit*/


boolean shellrunwindowconfirmationscript (WindowPtr pwindow, short idscript) {
	
	/*
	5.0a23 dmb: common code now shared by window close & window open

	5.1.2 dmb: better error handling
	*/
	
	tyvaluerecord val;
	bigstring bsname;
	bigstring bsscript;
	bigstring bsresult;
	boolean fl, flscriptwasrunning;
	
	if (cancoonglobals == nil)  /*not in a position to run scripts*/
		return (true);
	
	if (!getsystemtablescript (idscript, bsscript))
		return (true);
	
	if (!setwinvalue (pwindow, &val))
		return (false);
	
	if (!coercetostring (&val))
		return (false);
	
	pullstringvalue (&val, bsname);
	
	disposevaluerecord (val, true);
	
	parsedialogstring (bsscript, bsname, nil, nil, nil, bsscript);
	
	flscriptwasrunning = flscriptrunning;
	
	flscriptrunning = false;
	
	fl = langrunstringnoerror (bsscript, bsresult);
	
	flscriptrunning = flscriptwasrunning;
	
	if (!fl) /*couldn't find/run the script*/
		return (true);
	
	if (equalstrings (bsresult, BIGSTRING ("\x01" "3"))) /*yes/no/cancel was cancelled*/
		return (false);
	
	if (equalstrings (bsresult, bsfalse))
		return (false);
	
	return (true);
	} /*shellrunwindowconfirmationscript*/


boolean windowzoom (WindowPtr w) {
	
	/*
	after creating a new window, you may want to pick the moment when the
	user sees the window.  that's why we unbundled this functionality from
	newchildwindow.  it may be useful in other window creating operations.
	
	12/17/91 dmb: set searchparams.flwindowzoomed every time -- see tablefind.c.
	
	5.0a23 dmb: call new shellrunwindowconfirmationscript
	*/
	
	hdlwindowinfo hinfo;
	register hdlwindowinfo h;
	
	if (!getwindowinfo (w, &hinfo))
		return (false);
	
	searchparams.flwindowzoomed = true; /*it wants to know*/
	
	h = hinfo; /*move into register*/
	
	showscrollbar ((**h).vertscrollbar); 
	
	showscrollbar ((**h).horizscrollbar); 
	
	if (!shellrunwindowconfirmationscript (w, idopenwindowscript))
		(**h).flhidden = true;
	
	if ((**h).rzoomfrom.top == -2) /*window is supposed to be invisible*/
		(**h).flhidden = true;
	
	if (!(**h).flhidden)
		zoomwindowfromcenter ((**h).rzoomfrom, w);
	
	/*
	shelldrawgrowicon (h);
	*/
	
	return (true);
	} /*windowzoom*/
	

boolean zoomfilewindow (WindowPtr w) {
	
	/*
	after creating a new window, you may want to pick the moment when the
	user sees the window.  that's why we unbundled this functionality from
	newfilewindow.
	
	5.0a23 dmb: call new shellrunwindowconfirmationscript
	*/
	
	hdlwindowinfo hinfo;
	
	if (!getwindowinfo (w, &hinfo))
		return (false);
	
	if (!shellrunwindowconfirmationscript (w, idopenwindowscript))
		(**hinfo).flhidden = true;
	
	if ((**hinfo).flhidden) {
		
		#ifdef WIN95VERSION
			//windowsendtoback (w);
			windowbringtofront (getnextwindow (w));
		#endif
		}
	else {
		
		zoomfromorigin (w); /*zoom and then show the window*/
		
		#ifdef MACVERSION
			shelldrawgrowicon (hinfo);
			
			showscrollbar ((**hinfo).vertscrollbar); 
			
			showscrollbar ((**hinfo).horizscrollbar);
		#endif
		}
	
	return (true);
	} /*zoomfilewindow*/


void shelladjustaftergrow (WindowPtr wptr) {
	
	register WindowPtr w = wptr;
	Rect r;
	hdlwindowinfo hinfo;
	register hdlwindowinfo h;
	
	getwindowinfo (w, &hinfo);
	
	if (hinfo != NULL) {
		
		h = hinfo; /*copy into register*/
		
		(**h).oldcontentrect = (**h).contentrect; /*some handlers need this*/
		
		if (config.fleraseonresize) {
			
			shellgetwindowrect (h, &r);
			
			//eraserect (r);
			smashrect (r);
			}
		
		shellerasegrowicon (h);
		
		shellerasemessagearea (h);
		
		windowresetrects (h);
		
		(*shellglobals.resizeroutine) ();
		
		(*shellglobals.setscrollbarroutine) ();
		
	//	shellwindowinval (h); /*force update on whole window*/
		
//	#ifdef MACVERSION
		shellupdatenow (w);
//	#endif

		//zerorect (&(**h).zoomedrect);
		}
	} /*shelladjustaftergrow*/


boolean shellmoveandsizewindow (hdlwindowinfo hinfo, Rect r) {
	
	register WindowPtr w = (**hinfo).macwindow;
	
	shellpushglobals (w);
	
	constrainwindowrect (w, &r); /*make sure it isn't too big or too small*/
	
	moveandsizewindow (w, r);
	
	shelladjustaftergrow (w);
	
	shellpopglobals ();
	
	return (true);
	} /*shellmoveandsizewindow*/


boolean shellmovewindow (hdlwindowinfo hinfo, short h, short v) {
	
	register WindowPtr w = (**hinfo).macwindow;
	Rect r;
	
	shellpushglobals (w);
	
	shellgetglobalwindowrect (hinfo, &r);
	
	offsetrect (&r, h - r.left, v - r.top);
	
	constrainwindowrect (w, &r); /*make sure it isn't too big or too small*/
	
	movewindow (w, r.left, r.top);
	
	shellpopglobals ();
	
	return (true);
	} /*shellmovewindow*/


boolean shellmovewindowhidden (hdlwindowinfo hinfo, short h, short v) {
	
	register WindowPtr w = (**hinfo).macwindow;
	Rect r;
	
	shellpushglobals (w);
	
	shellgetglobalwindowrect (hinfo, &r);
	
	offsetrect (&r, h - r.left, v - r.top);
	
	constrainwindowrect (w, &r); /*make sure it isn't too big or too small*/
	
	movewindowhidden (w, r.left, r.top);
	
	shellpopglobals ();
	
	return (true);
	} /*shellmovewindowhidden*/


boolean shellsizewindow (hdlwindowinfo hinfo, short h, short v) {
	
	/*
	9/19/91 dmb: don't resize windows that aren't growable
	
	5.0.2b16 dmb: avoid integer overflow
	*/
	
	register WindowPtr w = (**hinfo).macwindow;
	Rect r;
	boolean fl = false;
	
	shellpushglobals (w);
	
	if (config.flgrowable) {
		
		shellgetglobalwindowrect (hinfo, &r);
		
		r.right = min ((long) r.left + h, infinity);
		
		r.bottom = min ((long) r.top + v, infinity);
		
		constrainwindowrect (w, &r); /*make sure it isn't too big or too small*/
		
		sizewindow (w, r.right - r.left, r.bottom - r.top);
		
		shelladjustaftergrow (w);
		
		fl = true;
		}
	
	shellpopglobals ();
	
	return (fl);
	} /*shellsizewindow*/


boolean shellsizewindowhidden (hdlwindowinfo hinfo, short h, short v) {
	
	/*
	9/19/91 dmb: don't resize windows that aren't growable
	
	5.0.2b16 dmb: avoid integer overflow
	*/
	
	register WindowPtr w = (**hinfo).macwindow;
	Rect r;
	boolean fl = false;
	
	shellpushglobals (w);
	
	if (config.flgrowable) {
		
		shellgetglobalwindowrect (hinfo, &r);
		
		r.right = min ((long) r.left + h, infinity);
		
		r.bottom = min ((long) r.top + v, infinity);
		
		constrainwindowrect (w, &r); /*make sure it isn't too big or too small*/
		
		sizewindowhidden (w, r.right - r.left, r.bottom - r.top);
		
		shelladjustaftergrow (w);
		
		fl = true;
		}
	
	shellpopglobals ();
	
	return (fl);
	} /*shellsizewindow*/


boolean shellzoomwindow (hdlwindowinfo hinfo, boolean flzoomin) {
#pragma unused (flzoomin)

	/*
	9/16/91 dmb: use the new getcontentsizeroutine to apply intelligent 
	window sizing when zooming window.
	
	9/19/91 dmb: unforce a minimum zoomed size of the title width, plus 
	space for the close box & zoom box.
	
	10/6/91 dmb: return false if config says it's not growable
	
	12/24/91 dmb: fixed potential integer overflow in rect calculation
	
	2.1b1 dmb: scroll everything that fits completely into view
	
	2.1b6 dmb: fixed integer overflow bug when subtracting negative 
	number from infinity. also, don't call constrainwindowrect here;
	that can limit height on account of menubar that isn't on our screen.
	*/

	register hdlwindowinfo h = hinfo;
	register WindowPtr w = (**h).macwindow;
	long width, height;
	Rect rwindow;
	Rect rcontent;
	Rect rzoom;
	Rect rprevstate;
	boolean flscrollleft = false;
	boolean flscrollup = false;
	//boolean flzoom;
	//Code change by Timothy Paustian Saturday, April 29, 2000 11:23:29 PM
	//Changed to Opaque call for Carbon

	
#ifdef MACVERSION
	Rect	rectToErase;
	short part;
	//WStateData **hstatedata;
	//Code change by Timothy Paustian Saturday, April 29, 2000 11:22:52 PM
	//Changed to Opaque call for Carbon
	//hstatedata = (WStateData **)GetWindowDataHandle(w);
	//old code
	//hstatedata = (WStateData **) (*(WindowPeek) w).dataHandle;

	//if (hstatedata == nil)
	//	return (false);
#endif
	
	shellpushglobals (w);
	
	if (!config.flgrowable) {
		
		shellpopglobals ();
		
		return (false);
		}
	
	getglobalwindowrect (w, &rwindow); /*current state*/
	
#ifdef MACVERSION
	//Code change by Timothy Paustian Saturday, April 29, 2000 11:30:25 PM
	//Changed to Opaque call for Carbon
	GetWindowStandardState(w, &rzoom);
	//old code
	//rzoom = (**hstatedata).stdState; /*last calculated "zoomed out" state*/
#else
	rzoom = (**h).zoomedrect;
#endif

//	if (rzoom.top > 0)
//		flzoom = false;
//	else
//		flzoom = true;

	rprevstate = rzoom;
	
	if ((*shellglobals.getcontentsizeroutine) (&width, &height)) { /*calculate new "zoomed out" state*/
		
		bigstring bstitle;
		short titlewidth;
		Rect rfull;
		
		pushstyle (systemFont, 12, normal);
		
		shellgetwindowtitle (shellwindowinfo, bstitle); // 7.24.97 dmb: was windowgettitle
		
		titlewidth = stringpixels (bstitle);
		
		popstyle ();
		
		rcontent = (**h).contentrect;
		
		#ifdef WIN95VERSION
			width += (rwindow.right - rwindow.left) - (rcontent.right - rcontent.left);

			height += (rwindow.bottom - rwindow.top) - (rcontent.bottom - rcontent.top);
		#endif

		width -= (rcontent.right - rcontent.left);
		
		height -= (rcontent.bottom - rcontent.top);
		
		width = min (width, (long) infinity - rwindow.right);
		
		height = min (height, (long) infinity - rwindow.bottom);
		
		rzoom = rwindow;
		
		rzoom.right = min (infinity - 6, (long) rzoom.right + width);
		
		rzoom.bottom = min (infinity - 6, (long) rzoom.bottom + height);
		
		rfull = rzoom;
		
		enforceminimumsize (&rzoom);
		
		rzoom.right = max (rzoom.right, rzoom.left + titlewidth + 64);
		
		if (keyboardstatus.floptionkey) { /*just get real big*/
			
			rzoom.right = rzoom.left + 6000;
			
			rzoom.bottom = rzoom.top + 6000;
			}
		
		insetrect (&rzoom, -3, -3); /*extra border*/
		
		constraintoscreenbounds (w, true, &rzoom);
		
		insetrect (&rzoom, 3, 3);
		
		if ((rfull.right - rfull.left) <= (rzoom.right - rzoom.left)) /*can fit full horiz*/
			flscrollleft = true;
		
		if ((rfull.bottom - rfull.top) <= (rzoom.bottom - rzoom.top)) /*can fit full vert*/
			flscrollup = true;
		
		#ifdef MACVERSION
			//Code change by Timothy Paustian Saturday, April 29, 2000 11:31:20 PM
			//Changed to Opaque call for Carbon
			SetWindowStandardState(w, &rzoom);
			//old code
			//(**hstatedata).stdState = rzoom;
		#else
			(**h).zoomedrect = rzoom;  /* 2006-03-28 SMD fixed, was rwindow */
		#endif
		}
	
	#ifdef MACVERSION
		if (equalrects (rwindow, rzoom)) /*we're zoomed out*/
			part = inZoomIn;
		else
			part = inZoomOut;
		//Code change by Timothy Paustian Saturday, April 29, 2000 11:25:04 PM
		//Changed to Opaque call for Carbon
		#if ACCESSOR_CALLS_ARE_FUNCTIONS == 1
		{
		CGrafPtr thePort = GetWindowPort(w);
		GetPortBounds(thePort, &rectToErase);
		eraserect (rectToErase);
		}
		#else
		#pragma unused(rectToErase)
		//old code
		eraserect ((*w).portRect);
		#endif
		
		ZoomWindow (w, part, false);
    
		shelladjustaftergrow (w);
		
		if (part == inZoomOut) {
			
			if (flscrollleft) /*can fit full horiz*/
				(*shellglobals.scrollroutine) (left, false, -(**h).horizscrollinfo.cur);
			
			if (flscrollup) /*can fit full vert*/
				(*shellglobals.scrollroutine) (up, false, -(**h).vertscrollinfo.cur);
			}
	#else
		if (!equalrects (rwindow, rzoom) || rprevstate.top == 0) { /*we're not already zoomed*/
//		if (flzoom) {
			shellmoveandsizewindow (h, rzoom);
			(**h).zoomedrect = rwindow;
			//(**h).zoomedrect.top += 20000;
			}
		else
			shellmoveandsizewindow (h, rprevstate);

		if (flscrollleft) /*can fit full horiz*/
			(*shellglobals.scrollroutine) (left, false, -(**h).horizscrollinfo.cur);
		
		if (flscrollup) /*can fit full vert*/
			(*shellglobals.scrollroutine) (up, false, -(**h).vertscrollinfo.cur);
	#endif
	
	shellpopglobals ();
	
	return (true);
	} /*shellzoomwindow*/


boolean getwindowmessage (WindowPtr w, bigstring bs) {
	
	hdlwindowinfo hinfo;
	register hdlstring hstring;
	
	setstringlength (bs, 0);
	
	if (!getwindowinfo (w, &hinfo))
		return (false);
		
	hstring = (**hinfo).hmessagestring;
	
	if (hstring != nil)
		copyheapstring (hstring, bs);
		
	return (true);
	} /*getwindowmessage*/
		

boolean drawwindowmessage (WindowPtr wptr) {

	register WindowPtr w = wptr;
	hdlwindowinfo hinfo;
	register hdlwindowinfo h;
	Rect r;
	bigstring bs;
	
	if (!config.flmessagearea) /*feature wired off*/
		return (true);
	
	if (!getwindowinfo (w, &hinfo))
		return (false);
		
	h = hinfo; /*copy into register*/
	
	//Code change by Timothy Paustian Monday, August 21, 2000 4:31:49 PM
	//Must pass a CGrafPtr to pushport on OS X to avoid a crash
	{
	CGrafPtr	thePort;
	#if TARGET_API_MAC_CARBON == 1
	thePort = GetWindowPort(w);
	#else
	thePort = (CGrafPtr)w;
	#endif
		
	pushport (thePort);
	}
	
	r = (**h).messagerect;
	
	pushclip (r);
	
	openbitmap (r, w);
	
	eraserect (r);
	
	framerect (r);
	
	insetrect (&r, 2, 0); /*leave room for the vertical boundary lines & 1 white pixel*/
	
	if ((**h).flwindowactive) { /*only active windows display their messages*/
	
		getwindowmessage (w, bs);
		
		pushstyle (geneva, 9, 0);
		
		centerstring (r, bs);
		
		popstyle ();
		}
	
	closebitmap (w);
    
	popclip ();
	
	popport ();
	
	return (true);
	} /*drawwindowmessage*/


boolean setwindowmessage (WindowPtr w, bigstring bs) {
	
	hdlwindowinfo hinfo;
	hdlstring hstring;
	register hdlstring hs;
	bigstring bsoldmessage;
	
	if (!getwindowinfo (w, &hinfo))
		return (false);
		
	hs = (**hinfo).hmessagestring;
	
	copyheapstring (hs, bsoldmessage);
	
	if (equalstrings (bsoldmessage, bs)) /*no change, save time and flicker*/
		return (true);
	
	disposehandle ((Handle) hs); /*get rid of the old message string*/
	
	if (stringlength (bs) == 0)
		hs = nil; 
		
	else {
		newheapstring (bs, &hstring);
		
		hs = hstring;
		}
		
	(**hinfo).hmessagestring = hs;
	
	return (true);
	} /*setwindowmessage*/
	
	
boolean shellsetwindowmessage (hdlwindowinfo hinfo, bigstring bs) {
	
	return (setwindowmessage ((**hinfo).macwindow, bs));
	} /*shellsetwindowmessage*/


boolean shelldrawwindowmessage (hdlwindowinfo hinfo) {
	
	return (drawwindowmessage ((**hinfo).macwindow));
	} /*shellsetwindowmessage*/


boolean lockwindowmessage (WindowPtr w, boolean fllocked) {
	
	hdlwindowinfo hinfo;
	
	if (!getwindowinfo (w, &hinfo))
		return (false);
		
	(**hinfo).flmessagelocked = bitboolean (fllocked);
	
	return (true);
	} /*lockwindowmessage*/


void shellerasemessagearea (hdlwindowinfo hinfo) {
	
	smashrect ((**hinfo).messagerect);
	} /*shellerasemessagearea*/


boolean shellsetwindowchanges (hdlwindowinfo hinfo, boolean fldirty) {
	
	/*
	use this instead of windowsetchanges when you don't want dirtyhooks called.
	
	set the flmadechanges field of the indicated info record.
	
	if the window has a parent and we're setting the bit, set its dirty bit.
	*/
	
	register hdlwindowinfo hw = hinfo;
	register boolean fl = bitboolean (fldirty);
	register hdlwindowinfo hparent;
	
	if (hw == nil) //defensive driving
		return (true);
	
	if ((**hw).flmadechanges != fl) { /*changing state*/
		
		#ifdef xxxWIN95VERSION
			bigstring bstitle;
			
			shellgetwindowtitle (hw, bstitle);
			
			if (fl)
				pushstring (bschangedwindowsuffix, bstitle);

			windowsettitle ((**hw).macwindow, bstitle);
		#endif
		
		(**hw).flmadechanges = fl;
		
		shellwindowmenudirty ();
		}
	
	if (!fl) /*clearing the bit doesn't percolate -- think about it...*/
		return (true);
	
	hparent = (**hw).parentwindow;
	
	if (hparent == nil) /*this is a root window*/
		return (true);
	
	return (shellsetwindowchanges (hparent, true)); /*percolate through recursion*/
	} /*shellsetwindowchanges*/


boolean windowsetchanges (WindowPtr w, boolean fldirty) {
	
	/*
	set the flmadechanges field of the info record for the indicated window.
	
	if the window has a parent and we're setting the bit, set it's dirty bit.
	
	7/18/90 DW: added call to routines hooked into dirty events.
	
	6/14/91 dmb: only call dirtyhooks when dirtying, not when cleaning.  also,
	make sure the windows menu is current
	*/
	
	hdlwindowinfo hinfo;
	register boolean fl = bitboolean (fldirty);
	
	if (fl)
		shellcalldirtyhooks ();
	
	if (!getwindowinfo (w, &hinfo))
		return (true);
	
	return (shellsetwindowchanges (hinfo, fl));
	} /*windowsetchanges*/


boolean windowmadechanges (WindowPtr w) {

	hdlwindowinfo hinfo;
	
	if (!getwindowinfo (w, &hinfo))
		return (false);
	
	return ((**hinfo).flmadechanges);
	} /*windowmadechanges*/
	
	
boolean windowgetcontentrect (WindowPtr w, Rect *rcontent) {

	hdlwindowinfo hinfo;
	
	if (!getwindowinfo (w, &hinfo))
		return (false);
	
	*rcontent = (**hinfo).contentrect;
	
	return (true);
	} /*windowgetcontentrect*/


void disposeshellwindow (WindowPtr wptr) {
	
	/*
	dmb 8/20/90:  return immediately if nil
	
	4.1b4 dmb: respect new fldontownwindow windowinfo field

	3/31/97 dmb: invalidate globals _before_ disposewindow to handle
	Windows messages cleanly

	5.0.1 dmb: if hinfo isn't nil, disconnect data before 
	invalidating globals (avoids Win crash).
	
	5.1.5b7 dmb: ctpushes, fldisposewhenpopped replace shellinvalidglobals
	*/
	
	register WindowPtr w = wptr;
	hdlwindowinfo hinfo;
	boolean fldontownwindow = false;
	
	if (!w)
		return;
	
	if (getwindowinfo (w, &hinfo)) { /*defensive driving*/
		
		if ((**hinfo).ctpushes > 0) {
			
			unlinkwindowlist (hinfo);
			
			(**hinfo).fldisposewhenpopped = true;
			
			return;
			}
		
		(**hinfo).hdata = nil;
		
		//shellinvalidglobals (w);
		
		fldontownwindow = (**hinfo).fldontownwindow;
		
		disposeshellwindowinfo (hinfo);
		}
//	else
//		shellinvalidglobals (w);
	
	setwindowrefcon (w, 0); // don't point to disposed structure
	
	if (!fldontownwindow)
		disposewindow (w);
	} /*disposeshellwindow*/


boolean shellhidewindow (hdlwindowinfo hinfo) {
	
	/*
	5.0a10 dmb: special case for MAC is now in hidewindow code

	5.1b23 dmb: for Win, make sure hinfo isn't disposed behind our back
	*/

	register hdlwindowinfo hw = hinfo;
	register WindowPtr w = (**hw).macwindow;
	
	hidewindow (w);
	
	if (*hw == nil)
		return (false);

	(**hw).flhidden = true;
	
	/*
	unlinkwindowlist (hw);
	*/
	
	shellwindowmenudirty ();
	
	return (true);
	} /*shellhidewindow*/


boolean shellunhidewindow (hdlwindowinfo hinfo) {
	
	/*
	5.1b23 dmb: for Win, make sure hinfo isn't disposed behind our back
	*/

	register hdlwindowinfo hw = hinfo;
	
	if ((hw == nil) || (*hw == nil))
		return (false);

	showwindow ((**hw).macwindow);
	
	if (*hw == nil)
		return (false);

	(**hw).flhidden = false;
	
	/*
	insertwindowlist (hinfo);
	*/
	
	shellwindowmenudirty ();
	
	return (true);
	} /*shellunhidewindow*/

	
boolean shellbringtofront (hdlwindowinfo hinfo) {
	
	/*
	4/20/93 dmb: handle windows that were opened in another process
	*/
	
	register hdlwindowinfo hw = hinfo;
	register WindowPtr w = (**hw).macwindow;
	typrocessid processid = (**hw).processid;
	boolean fl;
	
	if (!iscurrentapplication (processid))
		fl = activateapplicationwindow (processid, w);
	else
		fl = windowbringtofront (w);
	
	if (!fl)
		return (false);
	
	shellunhidewindow (hinfo);
	
	return (true);
	} /*shellbringtofront*/


boolean shellsetwindowtitle (hdlwindowinfo hinfo, bigstring bstitle) {
	
	setheapstring (bstitle, (**hinfo).hwindowtitle);
	
	#ifdef xxxWIN95VERSION
		if ((**hinfo).flmadechanges)
			pushstring (bschangedwindowsuffix, bstitle);
	#endif

	windowsettitle ((**hinfo).macwindow, bstitle);
	
	shellwindowmenudirty ();
	
	return (true);
	} /*shellsetwindowtitle*/


void shellgetwindowtitle (hdlwindowinfo hinfo, bigstring bstitle) {
	
	/*
	7.24.97 dmb: added hwindowtitle to the windowinfo record so we 
	don't need to get it from the window. Under Windows, getwindowtitle 
	calls back to the window message proc, forcing us to allow a thread
	swap. this was wreaking havoc which has only partially been handled.
	*/

	// windowgettitle ((**hinfo).macwindow, bstitle);
	
	if (hinfo == nil)
		setemptystring (bstitle);
	else
		copyheapstring ((**hinfo).hwindowtitle, bstitle);
	} /*shellgetwindowtitle*/


void closewindowfile (WindowPtr wptr) {
	
	/*
	close the file displayed in the indicated window, and set up his information
	handle to indicate that no file is open.
	
	dmb 8/20/90:  support resource fork
	dmb 10/15/90:  dispose file if never saved
	*/
	
	hdlwindowinfo hinfo;
	register hdlwindowinfo h;
	tyfilespec fspec;
	
	if (!getwindowinfo (wptr, &hinfo))
		return;
		
	h = hinfo; /*copy into register*/
	
	if ((**h).fnum != 0) {
	
		closefile ((**h).fnum);
		
		closeresourcefile ((**h).rnum);
		
		windowgetfspec (wptr, &fspec);
		
		if ((**h).flneversaved)
			deletefile (&fspec);
		
		flushvolumechanges (&fspec, (hdlfilenum)0);
		}
	
	(**h).fnum = 0; /*indicate no file open*/
	
	(**h).rnum = -1;
	} /*closewindowfile*/


void shellwindowmessage (bigstring bs) {
	
	if (!(**shellwindowinfo).flmessagelocked) 
		(*shellglobals.msgroutine) (bs, false);
	} /*shellwindowmessage*/


boolean shellfrontwindowmessage (bigstring bs) {
	
	/*
	display the message in the frontmost window.
	*/
	
	if (!shellpushfrontglobals ())
		return (false);
	
	shellwindowmessage (bs);
	
	shellpopglobals ();
	
	return (true);
	} /*shellfrontwindowmessage*/


boolean shellfrontrootwindowmessage (bigstring bs) {
	
	/*
	display the message in the frontmost root window.
	*/
	
	if (!shellpushfrontrootglobals ())
		return (false);
	
	shellwindowmessage (bs);
	
	shellpopglobals ();
	
	return (true);
	} /*shellfrontrootwindowmessage*/


boolean shellgettargetdata (short id, WindowPtr w) {
	
	boolean fl;
	
	shellpushglobals (w);
	
	fl = (*shellglobals.gettargetdataroutine) (id); /*sets target globals if possible*/
	
	shellpopglobals ();
	
	return (fl); /*keep looking*/
	} /*shellgettargetdata*/


typedef struct findtargetinfo {

	short targetdataid;

	WindowPtr targetwindow;
	} tyfindtargetinfo;


static boolean findtargetvisit (WindowPtr w, ptrvoid refcon) {
	
	tyfindtargetinfo *findinfo = (tyfindtargetinfo *) refcon;
	
	if (windowvisible (w)) { /*hidden windows aren't condidates for default target*/
		
		if (shellgettargetdata ((*findinfo).targetdataid, w)) {
			
			(*findinfo).targetwindow = w;
			
			return (false); /*stop visiting*/
			}
		}
	
	return (true); /*keep looking*/
	} /*findtargetvisit*/


boolean shellfindtargetwindow (short id, WindowPtr *targetwindow) {
	
	/*
	chase through the window list, looking for the frontmost window that contains
	data for a verb of the given type.
	
	the gettargetdata callback routine sets up globals and returns true if 
	it is able to handle this type of verb
	*/
	
	tyfindtargetinfo findinfo;
	
	findinfo.targetdataid = id; /*make visible to visit routine*/
	
	if (shellvisitwindowlayer (&findtargetvisit, &findinfo))
		return (false);
	
	*targetwindow = findinfo.targetwindow;
	
	return (true);
	} /*shellfindtargetwindow*/


boolean shellgetexternaldata (hdlwindowinfo hinfo, void *hdata) {
	
	boolean fl;
	
	*(Handle *) hdata = nil;
	
	shellpushglobals ((**hinfo).macwindow);
	
	fl = (*shellglobals.getvariableroutine) (hdata);
	
	shellpopglobals ();
	
	return (fl);
	} /*shellfindtargetwindow*/


boolean shellgetdatabase (WindowPtr w, hdldatabaserecord *hdatabase) {
	
	boolean fl;
	hdloutlinerecord ho = outlinedata;
	
	*hdatabase = nil;
	
	if (!shellpushrootglobals (w))
		return (false);
	
	fl = (*shellglobals.getdatabaseroutine) (hdatabase);

	shellpopglobals ();
	
	opsetoutline (ho);

	return (fl);
	} /*shellgetdatabase*/




