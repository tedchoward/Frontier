
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

#include "quickdraw.h"
#include "memory.h"
#include "file.h"
#include "launch.h"

extern boolean equalidentifiers (bigstring bs1, bigstring bs2);
boolean pushmacport (CGrafPtr p);
boolean returnkeydown (void);
boolean popmacport (void);
void centerwindow (WindowPtr w, Rect rscreen);
boolean rectlessthan (Rect r1, Rect r2);
boolean unicaseequalstrings (void *pvoid1, void *pvoid2);

// kw - 2005-12-15 - compiler warning rename
boolean oldclutconverter (short idx, RGBColor *rgb);
boolean emptyrect (Rect r);
boolean pushhandleonhandle (Handle hsource, Handle hdest);
void deleteinhandle (Handle h, unsigned long ix, unsigned long ctdelete);
boolean pushathandlestart (ptrvoid, long, Handle);
boolean assureappisrunning (OSType serverid, boolean flbringtofront);
boolean findrunningapp (OSType id, void *info);
void appletsegment (void);
boolean ploticonsuite (Rect *r, short align, short transform, Handle iconsuite);


unsigned short ttLabel [8] = {0x0000, 0x0100, 0x0200, 0x0300, 0x0400, 0x0500, 0x0600, 0x0700};


boolean ploticonsuite (Rect *r, short align, short transform, Handle iconsuite) {

	return (PlotIconSuite (r, align, transform, iconsuite) == noErr);
	} /*ploticonsuite*/


boolean pushmacport (CGrafPtr p) {
	
	return pushport (p);
	} /*pushmacport*/
		

boolean popmacport (void) {
	return popport ();
	} /*popmacport*/

static boolean keydown (short keycode) {
#ifdef MACVERSION
	KeyMap keys;
	
	GetKeys (keys);
	
	return (BitTst (&keys, keycode) != 0);
#endif
#ifdef WIN95VERSION
	return ((GetKeyState (keycode) & 0x8000) == 0x8000);
#endif
	} /*keydown*/
	
boolean returnkeydown (void) {
	
	return (keydown (35)); 
	} /*returnkeydown*/
	
void centerwindow (WindowPtr w, Rect rscreen) {

	short h, v;
	Rect r;
	short minv;
	//Code change by Timothy Paustian Tuesday, May 16, 2000 9:21:16 PM
	//Changed to Opaque call for Carbon
	#if ACCESSOR_CALLS_ARE_FUNCTIONS == 1
	{
	CGrafPtr	thePort = GetWindowPort(w);
	GetPortBounds(thePort, &r);
	}
	#else
	r = (*w).portRect;
	#endif
	h = rscreen.left + (((rscreen.right - rscreen.left) - (r.right - r.left)) / 2);
	
	v = rscreen.top + (((rscreen.bottom - rscreen.top) - (r.bottom - r.top)) / 6);
	
	minv = getmenubarheight () + doctitlebarheight + 10;
	
	if (v < minv)
		v = minv;
	
	MoveWindow (w, h, v, false);
	} /*centerwindow*/


boolean rectlessthan (Rect r1, Rect r2) {
	
	/*
	return true if r1 is "less than" r2.
	
	we look only at the top-left corners of the rectangles. a rectangle is
	less than the other rectangle if it's above the other guy vertically.
	
	if they're both at the same vertical position, then the guy to the left
	of the other is less than.
	
	this helps us implement a rational sort of tabbing thru objects, we go
	from top to bottom and left to right.
	*/
	
	if (r1.top != r2.top) { /*comparison is based on tops*/
		
		return (r1.top < r2.top);
		}
	
	return (r1.left < r2.left);
	} /*rectlessthan*/


boolean unicaseequalstrings (void *pvoid1, void *pvoid2) {
	
	return (equalidentifiers (pvoid1, pvoid2));
	} /*unicaseequalstrings*/
	

static boolean getclut (short resid, CTabHandle *hdlctab) {
	
	Handle hdata;
	
	*hdlctab = nil;
	
	hdata = GetResource ('clut', resid);
	
	if (hdata == nil)
		return (false);
	
	*hdlctab = (CTabHandle) hdata;
	
	return (true);
	} /*getclut*/

boolean oldclutconverter (short idx, RGBColor *rgb) {

	CTabHandle hdlctab;

	getclut (128, &hdlctab);
	
	*rgb = (**hdlctab).ctTable [idx].rgb;

	return (true);
	} /*oldclutconverter*/
	
	
boolean emptyrect (Rect r) {
	
	/*
	return false if there are any pixels contained inside the rectangle.
	*/
	
	return ((r.top >= r.bottom) || (r.left >= r.right));
	} /*emptyrect*/
	
	
boolean assureappisrunning (OSType serverid, boolean flbringtofront) {
	
	/*
	if the application whose creator id is serverid is running, return true.
	
	if not, we look for the application and try to launch it. we wait until it's 
	actually running and ready to receive Apple Events.
	*/
	
	bigstring appname;
	typrocessid psn;
	FSSpec fs;
	
	if (findrunningapplication (&serverid, appname, &psn)) /*already running, nothing to do*/
		return (true);
		
	if (!findapplication (serverid, &fs))
		return (false);
		
	if (!launchapplication (&fs, nil, flbringtofront))
		return (false);
		
	return (true);
	} /*assureappisrunning*/
	
	
boolean pushhandleonhandle (Handle hsource, Handle hdest) {
	
	return (pushhandle (hsource, hdest));
	} /*pushhandleonhandle*/


void deleteinhandle (Handle h, unsigned long ix, unsigned long ctdelete) {
	
	pullfromhandle (h, ix, ctdelete, nil);
	} /*deleteinhandle*/


boolean pushathandlestart (ptrvoid pdata, long ctbytes, Handle h) {

	return (insertinhandle (h, 0, pdata, ctbytes));
	} /*pushathandlestart*/
	
	
boolean findrunningapp (OSType id, void *info) {
#pragma unused(info)
	return (findrunningapplication (&id, nil, nil));
	} /*findrunningapp*/


void appletsegment (void) {
	} /*appletsegment*/


