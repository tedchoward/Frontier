
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

#ifdef MACVERSION
#define GetDoubleClickTime() GetDblTime()
#endif

#include "quickdraw.h"
#include "mouse.h"



tymouserecord mousestatus = {0};

/*static long mouseuptime = 0; 

static long mousedowntime = 0; 

static Point mouseuppoint = {0, 0};

static Point mousedownpoint = {0, 0};

static boolean fldoubleclickdisabled = false;
*/


void setmousedoubleclickstatus (boolean fl) {
	mousestatus.fldoubleclick = fl;
	}


boolean mousebuttondown (void) {
	
	/*shellbackgroundtask ();*/ /*allow tasks to hook into this bottleneck*/
#ifdef MACVERSION	
	return (Button ());
#endif
#ifdef WIN95VERSION
	MSG msg;
	return (PeekMessage (&msg, NULL, WM_LBUTTONDOWN, WM_LBUTTONDOWN, PM_NOREMOVE));
#endif
	} /*mousebuttondown*/
	
	
void waitmousebutton (boolean fl) {
	
	/*
	wait for the mouse button to either go up or go down.
	
	if fl is true, we wait for it to go down.
	
	if fl is false, we wait for it to come back up.
	*/
#ifdef MACVERSION	
	while (true) {
		if (Button () == fl)
			return;
			
		/*shellbackgroundtask ();*/ /*wired off 5/9/90 -- simplifies debugging*/
		} /*while*/
#endif
#ifdef WIN95VERSION	
	MSG msg;

	while (true) {
		if (fl) {
			if (PeekMessage (&msg, NULL, WM_LBUTTONDOWN, WM_LBUTTONDOWN, PM_NOREMOVE)) {
				return;
				}
			}
		else {
			if (PeekMessage (&msg, NULL, WM_LBUTTONUP, WM_LBUTTONUP, PM_NOREMOVE)) {
				return;
				}
			}
						
		} /*while*/
#endif

	} /*waitmousebutton*/
	
	
void waitmouseclick (void) {
	
	waitmousebutton (false);
	
	waitmousebutton (true);
	} /*waitmouseclick*/
	

boolean mousestilldown (void) {
	
	/*
	wait for mouse button to come back up after a mousedown.  no background
	tasks while this is happening.  thank you!
	*/
#ifdef MACVERSION	
	return (StillDown ());
#endif

#ifdef WIN95VERSION

	/* getting the keystate of the mouse, gets the physical mouse, but the user may have swapped
	   the meaning of the buttons.  We are interested in the left logical button. */
	if (GetSystemMetrics(SM_SWAPBUTTON)) {
		return ((GetAsyncKeyState(VK_RBUTTON) & 0x8000) == 0x8000);
		}

	return ((GetAsyncKeyState(VK_LBUTTON) & 0x8000) == 0x8000);
#endif	
	} /*mousestilldown*/
	

boolean rightmousestilldown (void) {

	/*
	7.0b26 PBS: wait for the right-mouse-button to come back up after a mousedown.
	*/

#ifdef WIN95VERSION

	/* getting the keystate of the mouse, gets the physical mouse, but the user may have swapped
	   the meaning of the buttons.  We are interested in the left logical button. */
	if (GetSystemMetrics(SM_SWAPBUTTON)) {
		return ((GetAsyncKeyState(VK_LBUTTON) & 0x8000) == 0x8000);
		}

	return ((GetAsyncKeyState(VK_RBUTTON) & 0x8000) == 0x8000);
#endif
	
	return (false);
	} /*rightmousestilldown*/

void getmousepoint (Point *pt) {
	
	/*
	5.0a8 dmb: Win version must use current port, not the 
	window that happens to contain the mouse
	*/

#ifdef MACVERSION	
	GetMouse (pt);
#endif
#ifdef WIN95VERSION
	POINT winpt;
//	RECT winrect;
	HWND hwnd;
	
	GetCursorPos (&winpt);
	
//	hwnd = ChildWindowFromPointEx (hwndMDIClient , winpt, CWP_SKIPINVISIBLE | CWP_SKIPTRANSPARENT);
//	hwnd = WindowFromPoint (winpt);
	hwnd = getport ();
	
	if (hwnd != NULL)
		ScreenToClient (hwnd, &winpt);
	
	(*pt).h = (short)winpt.x;
	(*pt).v = (short)winpt.y;
#endif
	} /*getmousepoint*/
	

boolean getmousewindowpos (WindowPtr *w, Point *pt) {
	
	#ifdef MACVERSION
		short part;

		GetMouse (pt);

		LocalToGlobal (pt);

		part = FindWindow (*pt, w);
		
		//Code change by Timothy Paustian Monday, August 21, 2000 4:31:49 PM
		//Must pass a CGrafPtr to pushport on OS X to avoid a crash
		{
		CGrafPtr	thePort;
		#if TARGET_API_MAC_CARBON == 1
		thePort = GetWindowPort(*w);
		#else
		thePort = (CGrafPtr)(*w);
		#endif
			
		pushport (thePort);
		}		

		GlobalToLocal (pt);

		popport ();
	#endif

	#ifdef WIN95VERSION
		POINT winpt;
		
		GetCursorPos (&winpt);
		
		*w = WindowFromPoint (winpt);
		
		if (*w != NULL)
			ScreenToClient (*w, &winpt);
		
		(*pt).h = (short)winpt.x;
		(*pt).v = (short)winpt.y;
	#endif
	
	return (*w != nil);
	} /*findmousewindow*/


boolean mousetrack (Rect r, void (*displaycallback) (boolean)) {
	
	/*
	hang out in this routine until the mouse button comes up.  return true if
	the mouse point is in the indicated rectangle when we return.
	
	7/17/90 DW: if the mouse wasn't down when we enter, return true.  this is
	a heuristic that may allow some mouse-dependent routines to be driven by
	a script.  we figure that if the mouse isn't down now, it never was down,
	and whether it's inside any particular rectangle is irrelevent.
	
	7/17/90 DW: add callback routine to display the object being tracked as
	the mouse moves in and out of the rectangle.  set it to nil if you don't
	need this feature.
	
	7/17/90 DW: only call the callback when the state of the object changes,
	assume display is correct when we're entered.
	*/
	
	boolean flinrectnow;
	boolean flwasinrect;
	
	if (!mousestilldown ()) /*see comment above*/
		return (true);
	
	flwasinrect = true; /*at least for the first iteration of loop*/
	
	while (mousestilldown ()) { /*stay in holding pattern*/
	
		Point pt;
		
		getmousepoint (&pt);
	
		flinrectnow = pointinrect (pt, r);
		
		if (flinrectnow != flwasinrect) { /*state of object changed*/
		
			if (displaycallback != nil)
				(*displaycallback) (flinrectnow);
			}
			
		flwasinrect = flinrectnow;
		} /*while*/
	
	return (flwasinrect);
	} /*mousetrack*/


void mousedoubleclickdisable (void) { 
	
	/*
	call this if you just received a mouse click that can't be interpreted as
	part of a doubleclick.
	
	example: mousing down on a window to select it.
	*/
	
	mousestatus.fldoubleclickdisabled = true; /*next mouseup can't be part of a doubleclick*/
	} /*mousedoubleclickdisable*/
	
	
static boolean mousecheckdoubleclick (void) {
#ifdef MACVERSION
	/*
	using the globals mouseuptime and mouseuppoint determine if a
	mouseclick at pt, right now, is a double click.
	
	9/6/90 dmb:  pt parameter is no longer used.  superceeded by new 
	mousedown globals
	
	12/6/96 dmb: now is private routine, callers examind mousestatus
	*/
	
	register boolean fldoubleclick;
	register short diff;
	
	fldoubleclick = (mousestatus.mousedowntime - mousestatus.mouseuptime) < GetDoubleClickTime ();
	
	if (fldoubleclick) { /*qualifies so far*/
		
		diff = pointdist (mousestatus.mousedownpoint, mousestatus.mouseuppoint);
		
		fldoubleclick = diff < 5; /*keep it set if mouse hasn't wandered too far*/
		}
	
	if (fldoubleclick) { /*user must doubleclick again to get effect*/
		
		mousestatus.mouseuptime = 0L;
		
		mousestatus.mouseuppoint.h = mousestatus.mouseuppoint.v = 0;
		}
	
	mousestatus.fldoubleclickdisabled = fldoubleclick; /*copy into global*/
	
	return (fldoubleclick);
#endif

#ifdef WIN95VERSION
	return (mousestatus.fldoubleclick);
#endif
	} /*mousecheckdoubleclick*/


boolean mousedoubleclick (void) {

	return (mousestatus.fldoubleclick);
	} /*mousedoubleclick*/


boolean ismouseleftclick (void) {

	return (mousestatus.whichbutton == leftmousebuttonaction);
	} /*ismouseleftclick*/


boolean ismouserightclick (void) {

	return (mousestatus.whichbutton == rightmousebuttonaction);
	} /*ismouserightclick*/


boolean ismousecenterclick (void) {

	return (mousestatus.whichbutton == centermousebuttonaction);
	} /*ismousecenterclick*/


boolean ismousewheelclick (void) {

	return (mousestatus.whichbutton == wheelmousebuttonaction);
	} /*ismousewheelclick*/


static short translatemouseeventtype (long eventwhat) {
#ifdef MACVERSION
#	pragma unused (eventwhat)
#endif

#ifdef WIN95VERSION
	switch (eventwhat) {
		case rmouseDown:
		case rmouseUp:
			return (rightmousebuttonaction);

		case cmouseDown:
		case cmouseUp:
			return (centermousebuttonaction);

		case wmouseDown:
		case wmouseUp:
			return (wheelmousebuttonaction);

		default:
			break;
		}
#endif

	return (leftmousebuttonaction);
	} /*translatemouseeventtype*/
		

void mouseup (long eventwhen, long eventposx, long eventposy, long eventwhat) {
	
	/*
	call this when you receive an mouse up event.  if the last mouse down was
	a double click, we set things up so that the next single click will not
	be interpreted as a double click.
	*/
	
	if (!mousestatus.fldoubleclickdisabled) {
		
		mousestatus.mouseuptime = eventwhen;
		
		mousestatus.mouseuppoint.h = (short)eventposx;
		mousestatus.mouseuppoint.v = (short)eventposy;
		
		mousestatus.mousedowntime = 0L; /*hasn't happened yet*/

		mousestatus.whichbutton = translatemouseeventtype (eventwhat);
		}
	
	mousestatus.fldoubleclickdisabled = false; /*next mouse up is important*/
	} /*mouseup*/


void mousedown (long eventwhen, long eventposx, long eventposy, long eventwhat) {
	
	/*
	call this when you receive a mouse down event.  we set our globals so 
	the we can accurately detect a double click if requested
	
	12/6/96 dmb: set new fldoubleclick field
	*/
	
	mousestatus.mousedowntime = eventwhen; 
	
	mousestatus.mousedownpoint.h = (short)eventposx;
	
	mousestatus.mousedownpoint.v = (short)eventposy;
	
	mousestatus.fldoubleclick = mousecheckdoubleclick ();

	mousestatus.whichbutton = translatemouseeventtype (eventwhat);
	} /*mousedown*/


long getmousedoubleclicktime () {

#ifdef MACVERSION
	return (GetDoubleClickTime ());
#endif

#ifdef WIN95VERSION
	return ((GetDoubleClickTime ()*60L)/1000L);
#endif
	} /*getmousedoubleclicktime*/


#if 0

void smashmouse (Point pt) {
	
	/*
	reset the mouse position to the indicated Point, in global coordinates.
	
	downloaded from APPDEV on Compuserve, so we're not exactly sure how this
	works.
	*/
	
	register short h = pt.h, v = pt.v;
	register short *pint;
	register char *pchar;
	register short maxh, maxv;
	Rect rbounds;
	
	getcurrentscreenbounds (&rbounds);
	
	maxv = rbounds.bottom - 1;
	
	maxh = rbounds.right - 1;
	
	v = min (v, maxv); /*keep h and v within bounds of of screen*/
	
	v = max (v, 0);
		
	h = min (h, maxh);
	
	h = max (h, 0);
	
	pint = (short *) 0x828; /*point at a magic spot in memory*/
	
	*pint++ = v;
	
	*pint++ = h;
	
	*pint++ = v;
	
	*pint++ = h;
	
	*pint++ = v;
	
	*pint = h;
	
	pchar = (char *) 0x8ce; /*point at another magic spot*/

	*pchar = 0xff;
	} /*smashmouse*/

#endif

/*	
smashmousetester (void) {
	
	register short i, j;
	bigstring bs;
	Point pt;
	
	for (i = 0; i <= 100; i++) {
		
		for (j = 0; j <= 100; j++) {
			
			/%
			if (optionkeydown ())
				return;
			%/
				
			pt.h = i;
			
			pt.v = j;
			
			smashmouse (pt);
			
			/%
			copystring ("\p(h = ", bs);
			
			pushint (i, bs);
			
			pushstring ("\p, v = ", bs);
			
			pushint (j, bs);
			
			pushstring ("\p)", bs);
			
			shellfrontwindowmessage (bs);
			%/
			}
		}
	} /%smashmousetester%/
*/


void showmousecursor (void) {
#ifdef MACVERSION	
	ShowCursor ();
#endif
#ifdef WIN95VERSION
	ShowCursor (TRUE);
#endif
	} /*showmousecursor*/


void hidemousecursor (void) {
	
#ifdef MACVERSION	
	HideCursor ();
#endif
#ifdef WIN95VERSION
	ShowCursor (FALSE);
#endif
	} /*hidemousecursor*/


boolean mousecheckautoscroll (Point pt, Rect r, boolean flhoriz, tydirection *dir) {
	
	/*
	if the point pt is outside of the rect r in the indicated dimension (flhoriz), 
	return true and set dir to the corresponding scroll direction.
	*/
	
	register tydirection d = nodirection;
	
	if (flhoriz) {
	
		if (pt.h < r.left)
			d = right;
		
		else if (pt.h > r.right)
			d = left;
		}
	else {
		
		if (pt.v < r.top)
			d = down;
		
		else if (pt.v > r.bottom)
			d = up;
		}
	
	*dir = d;
	
	return (d != nodirection);
	} /*mousecheckautoscroll*/




