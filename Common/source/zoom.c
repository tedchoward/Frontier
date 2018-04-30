
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
#include "cursor.h"
#include "ops.h"
#include "dialogs.h"
#include "zoom.h"
#include "shell.h"
#include "shellprivate.h"



static long ctzoomdelayloops = 0;

static Rect rdefaultzoomfrom = {0, 0, 0, 0};

#define zoomfixer 65536L

static short zoomsteps = 14; /*decrease this number for faster zooming*/

static Point zoomratio = {5, 6}; /*must adjust to work with zoomsteps*/

Fixed zoomfract;




void zoomsetdefaultrect (WindowPtr w, Rect r) {
	
	rdefaultzoomfrom = r;
	
	localtoglobalrect (w, &rdefaultzoomfrom);
	} /*zoomsetdefaultrect*/


/*
void middlerect (Rect rorig, Rect *rmiddle) {
	
	/%
	compute a rectangle which is at or near the center of rorig of 
	minimum dimensions.
	%/
	
	Rect r;
	
	r.left = r.right = rorig.left + ((rorig.right - rorig.left) / 2);
	
	r.top = r.bottom = rorig.top + ((rorig.bottom - rorig.top) / 2);
	
	*rmiddle = r; /%return value%/
	} /%middlerect%/
*/


static short zoomblend (short i1, short i2) {

	Fixed smallfix, bigfix, tempfix;

	smallfix = zoomfixer * i1;
	
	bigfix = zoomfixer * i2;
	
	tempfix = FixMul (zoomfract, bigfix) + FixMul (zoomfixer - zoomfract, smallfix);
	
	return (FixRound (tempfix));
	} /*zoomblend*/
	

void zoomrect (Rect *rfrom, Rect *rto, boolean flzoomup) {
	
	/*
	11/16/90 DW: try to smooth out performance even on the fastest machines.
	
	9/16/91 dmb: compare rfrom.top to -1, not < 0; negative numbers can 
	be ligitimate when mutliple monitors are in use. 
	
	also, don't constrain rects to desktop, since we might want to zoom to the 
	menubar.
	
	12/27/91 dmb: don't do zoomrect if frontier isn't the frontmost app
     
    12/20/2005 kw: killing zoomrects
	*/

	return;

	} /*zoomrect*/
	
	
/*
void zoomfrommiddle (Rect r) {
	
	Rect rmiddle;
	
	middlerect (r, &rmiddle);
	
	zoomrect (&rmiddle, &r, true);
	} /%zoomfrommiddle%/
*/	

void zoomport (Rect rsource, WindowPtr w, boolean flzoomup) {
#	pragma unused(rsource)
#	pragma unused(w)
#	pragma unused(flzoomup)

	/*
	zooms the window from an invisible state to a visible 
	state (flzoomup == true) or vice versa (flzoomup == false).
	
	rsource is the place where the window "came from" -- ie
	if you're opening an about box, the source is a position
	just below the Apple menu.
	
	7.0b41 PBS: don't zoom in Carbon version.
	
	8.0fc3 PBS: Again, don't zoom in Carbon version.
	*/
	
	
		return;
		
	} /*zoomport*/


void zoomwindowfrom (Rect rsource, WindowPtr w) {

	/*
	animate the opening of a window.
	
	you tell us where you want the window to appear to be coming from.
	*/
	
	//Code change by Timothy Paustian Monday, August 21, 2000 4:31:49 PM
	//Must pass a CGrafPtr to pushport on OS X to avoid a crash
	{
	CGrafPtr	thePort;
	thePort = GetWindowPort(w);
		
	pushport (thePort);
	}
	
	zoomport (rsource, w, true);
	
	ShowWindow (w);
	
	popport ();
	} /*zoomwindowfrom*/
	
	
void zoomwindowto (Rect rsource, WindowPtr w) {

	/*
	animate the closing of a window.
	
	you tell us where you want the window to appear to be going to.
	*/
	
	//Code change by Timothy Paustian Monday, August 21, 2000 4:31:49 PM
	//Must pass a CGrafPtr to pushport on OS X to avoid a crash
	{
	CGrafPtr	thePort;
	thePort = GetWindowPort(w);
		
	pushport (thePort);
	}
	
	HideWindow (w);
	
	zoomport (rsource, w, false);
	
	popport ();
	} /*zoomwindowto*/
	

void zoomcenterrect (Rect *rcenter) {
	
	Rect r;
	
	r = *rcenter;
	
	r.left += (r.right - r.left) / 2;
	
	r.right = r.left;
	
	r.top += (r.bottom - r.top) / 2;
	
	r.bottom = r.top;
	
	*rcenter = r;
	} /*zoomcenterrect*/
	

void zoomwindowtocenter (Rect r, WindowPtr w) {
	
	/*
	shrink the rectangle down to the point at its center and zoom the window
	down to that point.
	*/
	
	if (r.top > 0) /*don't smash it if it's the default rect*/
		zoomcenterrect (&r);
	
	zoomwindowto (r, w);
	} /*zoomwindowtocenter*/
	

void zoomwindowfromcenter (Rect r, WindowPtr w) {
	
	/*
	shrink the rectangle down to the point at its center and zoom the window
	up from that point.
	*/
	
	if (r.top > 0) /*don't smash it if it's the default rect*/
		zoomcenterrect (&r);
	
	zoomwindowfrom (r, w);
	} /*zoomwindowfromcenter*/
	

void zoomfromorigin (WindowPtr w) {
	
	Rect r;
	
	r.top = r.left = r.bottom = r.right = 0;
	
	zoomwindowfrom (r, w);
	} /*zoomfromorigin*/
	

void zoomtoorigin (WindowPtr w) {
	
	Rect r;
	
	r.top = r.left = r.bottom = r.right = 0;
	
	zoomwindowto (r, w);
	} /*zoomtoorigin*/
	
/*
boolean zoomtempwindow (boolean flzoom, short height, short width, WindowPtr *w) {
	
	/%
	a temp window is like the one used in the About... command.
	
	10/18/91 DW: color!
	%/
	
	register WindowPtr x;
	Rect r, rsource;
	
	/%
	SetRect (&r, 0, 0, width, height);
	
	x = NewWindow (nil, &r, zerostring, false, altDBoxProc, (WindowPtr) -1L, false, 0);
	
	if (x == nil) {
		
		sysbeep (); 
		
		return;
		}
	%/
	
	if (!newcolorwindow (height, width, w)) {
	
		sysbeep (); /%instead of showing window%/
		
		return (false);
		}
	
	x = *w; /%copy into register%/
	
	pushport (x);
	
	positiondialogwindow (x);
	
	if (flzoom) {
	
		getsystemoriginrect (&rsource);
	
		zoomwindowfrom (rsource, x);
		}
	else
		ShowWindow (x);
	
	*w = x; /%return pointer to window record%/
	
	return (true);
	} /%zoomtempwindow%/
	
	
void closetempwindow (boolean flzoom, WindowPtr w) {

	register WindowPtr x = w;
	Rect rsource;
	
	if (x != nil) {
		
		getsystemoriginrect (&rsource);
		
		if (flzoom) 
			zoomwindowto (rsource, x);
		else
			HideWindow (x);
		
		DisposeWindow (x);
		
		popport ();
		}
	} /%closetempwindow%/


void modaltempwindow (WindowPtr w, void (*updateproc) ()) {
	
	register WindowPtr x = w; 
	EventRecord ev;
	
	setcursortype (cursorisarrow);
	
	while (true) {
		
		SystemTask ();
		
		if (!GetNextEvent (everyEvent, &ev))
			continue;
			
		switch (ev.what) {
		
			case keyDown: case autoKey: case mouseDown:		
				return;
				
			case updateEvt:	/%handle updates, he might be using Pyro!%/
				if ((WindowPtr) (ev.message) == x) {
				
					pushport (x);
					
					BeginUpdate (x);
					
					(*updateproc) (x);
					
					EndUpdate (x);
					
					popport ();
					}
									
				break;

			} /%switch%/
		} /%while%/
	} /%modaltempwindow%/
*/
	

void zoominit (void) {
	
	counttickloops (&ctzoomdelayloops);
	
	ctzoomdelayloops /= 6; /*burn a sixth of a tick each iteration of the loop*/
	
	if (ctzoomdelayloops < 60) { /*we're on a slow machine*/
		
		ctzoomdelayloops = 0;
		
		zoomsteps = 7;
		
		zoomratio.h = 32;
		
		zoomratio.v = 21;
		}
	} /*zoominit*/
