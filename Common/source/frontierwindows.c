
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

#ifdef MACVERSION
	#include <standard.h>
#endif

#ifdef WIN95VERSION
	#include "standard.h"
	#include <commctrl.h>
#endif

#include "quickdraw.h"
#include "strings.h"
#include "threads.h"
#include "frontierwindows.h"
#include "shell.h"



#ifdef WIN95VERSION

#define titlebarheight 20

static boolean setwindowpos (HWND hWnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags) {
	
	/*
	5.0.2b18 dmb: restore minimized windows before changing their position
	*/

	POINT winpt;
	boolean fl;
	WINDOWPLACEMENT wp;

	/*
	if (cy)
		cy += titlebarheight;
		
	if (Y)
		Y -= titlebarheight;
	*/

	winpt.x = X;
	winpt.y = Y;
	
//	ScreenToClient (shellframewindow, &winpt);
	
	releasethreadglobals ();
	
	wp.length = sizeof(WINDOWPLACEMENT);

	if (GetWindowPlacement (hWnd, &wp) && (wp.showCmd == SW_SHOWMINIMIZED)) {
		
		wp.showCmd = SW_RESTORE;
		
		SetWindowPlacement (hWnd, &wp);
		}
	
	fl = SetWindowPos (hWnd, hWndInsertAfter, winpt.x, winpt.y, cx, cy, uFlags);
	
	grabthreadglobals ();
	
	return (fl);
	} /*setwindowpos*/

#endif



WindowPtr getnewwindow (short id, boolean fldialog, Rect *r) {

	WindowPtr w = nil;
	
#ifdef MACVERSION
	//code change by Timothy Paustian 9/17/00 dialogPtrs and WindowPtrs
	//are not the save thing. Do a cast
	//This may cause trouble in other things that assume DialogPtrs
	//and WindowPtrs are the same, but I have to return a windowPtr
	//If needed this can be cast down to a DialogPtr
	if (fldialog)
	{
		DialogPtr	theDial = GetNewDialog (id, nil, (DialogPtr) -1L);
		w = GetDialogWindow(theDial);
	}
	else {
		if (config.flcolorwindow && havecolorquickdraw ())
			w = GetNewCWindow (id, nil, (WindowPtr) -1L); 
		else
			w = GetNewWindow (id, nil, (WindowPtr) -1L); 
		}

	if (w != nil && r != nil) {

		constraintodesktop (w, r);
		
		/*make sure top-right corner (zoom box) is accessable*/ {
		
			Rect rmin;
			
			setrect (&rmin, (*r).top, (*r).right - 50, (*r).top + 5, (*r).right);
			
			if (constraintoscreenbounds (w, false, &rmin))
				constraintoscreenbounds (w, false, r);
			}
		
		sizewindow (w,  (*r).right - (*r).left, (*r).bottom - (*r).top);
		
		movewindow (w, (*r).left, (*r).top);
		}

#endif

#ifdef WIN95VERSION
	HWND hwnd;
	CHAR            sz[160];
	MDICREATESTRUCT mcs;
	#ifdef USINGSTATUSBAR
		HWND hwndStatusBar;
	#endif

  //      LoadString (hInst, IDS_UNTITLED, sz, sizeof(sz));
	strcpy (sz, "Untitled");
	mcs.szTitle = (LPSTR)sz;
	mcs.szClass = szChild1;

	if (config.filetype == 'root')
		mcs.szClass = szChildRoot;

	else if (config.filetype == 'ftop')
		mcs.szClass = szChildFTop;

	else if (config.filetype == 'ftwp')
		mcs.szClass = szChildFTwp;

	else if (config.filetype == 'fttb')
		mcs.szClass = szChildFTtb;

	else if (config.filetype == 'ftmb')
		mcs.szClass = szChildFTmb;

	else if (config.filetype == 'ftsc')
		mcs.szClass = szChildFTsc;


	mcs.hOwner  = hInst;

	/* Use the default size for the window */
	if (r == NULL) {

		mcs.x = mcs.cx = CW_USEDEFAULT;
		mcs.y = mcs.cy = CW_USEDEFAULT;
		}
	else {
		mcs.x = (*r).left - 4;
		mcs.y = (*r).top - 23;

		mcs.cx = (*r).right - (*r).left + 5;
		mcs.cy = (*r).bottom - (*r).top + 24;
		}

	/* Set the style DWORD of the window to default */
	mcs.style = styleDefault;

	releasethreadglobals ();

	/* tell the MDI Client to create the child */
	hwnd = (HWND)SendMessage (hwndMDIClient,
                              WM_MDICREATE,
                              0,
                              (LONG)(LPMDICREATESTRUCT)&mcs);
	
	
	#ifdef USINGSTATUSBAR
		//RAB:  1/20/98 added call to create the status bar
		hwndStatusBar = CreateStatusWindow (WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | CCS_BOTTOM | SBARS_SIZEGRIP, "Ready", hwnd, 2);

		SetWindowLong (hwnd, 8, (long) hwndStatusBar);
	#endif

	grabthreadglobals ();

    w = hwnd;

#endif
	
	return (w);
	} /*getnewwindow*/

	
void disposewindow (WindowPtr w) {
	
	#ifdef MACVERSION
		DisposeWindow (w);
	#endif

	#ifdef WIN95VERSION
		releasethreadglobals ();

		SendMessage (hwndMDIClient, WM_MDIDESTROY, (WPARAM) w, 0L);

		grabthreadglobals ();
	#endif
	} /*disposewindow*/


void windowgettitle (WindowPtr w, bigstring bs) {
	
	#ifdef MACVERSION
		GetWTitle (w, bs);
	#endif

	#ifdef WIN95VERSION
		releasethreadglobals ();
		
		GetWindowText(w, bs, lenbigstring);
		
		grabthreadglobals ();
		
		convertcstring (bs);
	#endif
 	} /*windowgettitle*/
	

#if !flruntime

void windowsettitle (WindowPtr w, bigstring bs) {
	
	#ifdef MACVERSION
		bigstring bsorig;
		
		GetWTitle (w, bsorig);
		
		if (!equalstrings (bs, bsorig)) /*save a little flickering*/
			SetWTitle (w, bs);
	#endif

	#ifdef WIN95VERSION
	#ifdef PASCALSTRINGVERSION
		bs[stringlength(bs)+1] = 0;
	#endif
		
		releasethreadglobals ();

		SetWindowText(w, stringbaseaddress(bs));

		grabthreadglobals ();
	#endif
	} /*windowsettitle*/
	

void windowinval (WindowPtr w) {
	
	Rect r;

	getlocalwindowrect (w, &r);

	invalwindowrect (w, r);

	/*
	#ifdef MACVERSION
		invalwindowrect (w, (*w).portRect);
	#endif
	
	#ifdef WIN95VERSION
		releasethreadglobals ();

		ShowWindow (w, SW_SHOWNA);

		grabthreadglobals ();

	#endif
	*/
	} /*windowinval*/


boolean graywindow (WindowPtr w) {
	
	Rect r;
	//Code change by Timothy Paustian Monday, August 21, 2000 4:21:47 PM
	//Cannot do an implicit cast, pushport requires a CGrafPtr
	CGrafPtr	thePort;
	#if TARGET_API_MAC_CARBON == 1
	thePort = GetWindowPort(w);
	#else
	thePort = (CGrafPtr)w;
	#endif
		
	pushport (thePort);
	
	getlocalwindowrect (w, &r);
	
	grayrect (r);
	
	invalrect (r);
	
	popport ();

	return (true);
	} /*graywindow*/
	
#endif
	
void movewindow (WindowPtr w, short h, short v) {

	#ifdef MACVERSION
		MoveWindow (w, h, v, false); /*don't bring it to the front*/
	#endif

	#ifdef WIN95VERSION
		setwindowpos (w, NULL, h - 4, v - 23, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
	#endif

	} /*movewindow*/


void movewindowhidden (WindowPtr w, short h, short v) {

	#ifdef MACVERSION
		MoveWindow (w, h, v, false); /*don't bring it to the front*/
	#endif

	#ifdef WIN95VERSION
		setwindowpos (w, NULL, h - 4, v - 23, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_HIDEWINDOW | SWP_NOACTIVATE);
	#endif

	} /*movewindowhidden*/
	

void sizewindow (WindowPtr w, short h, short v) {

	#ifdef MACVERSION
		SizeWindow (w, h, v, true); /*add to the update region*/
	#endif

	#ifdef WIN95VERSION
		// dmb: must account for NC area
		/*long style = GetWindowLong (w, GWL_STYLE);
		
		if (style & WS_VSCROLL)
			h += 16;
		
		if (style & WS_HSCROLL)
			v += 16;
		*/

		setwindowpos (w, NULL, 0, 0, h + 5, v + 24, SWP_NOMOVE | SWP_NOZORDER);
	#endif

	} /*sizewindow*/

	
void sizewindowhidden (WindowPtr w, short h, short v) {

	#ifdef MACVERSION
		SizeWindow (w, h, v, true); /*add to the update region*/
	#endif

	#ifdef WIN95VERSION
		// dmb: must account for NC area
		/*long style = GetWindowLong (w, GWL_STYLE);
		
		if (style & WS_VSCROLL)
			h += 16;
		
		if (style & WS_HSCROLL)
			v += 16;
		*/

		setwindowpos (w, NULL, 0, 0, h + 5, v + 24, SWP_NOMOVE | SWP_NOZORDER | SWP_HIDEWINDOW | SWP_NOACTIVATE);
	#endif

	} /*sizewindowhidden*/


void moveandsizewindow (WindowPtr w, Rect r) {
	
	sizewindow (w, r.right - r.left, r.bottom - r.top);
	
	movewindow (w, r.left, r.top);
	} /*moveandsizewindow*/


WindowPtr getfrontwindow (void) {
	
	/*
	5.1.5b15 dmb: return invisible window if in front, not NULL [Win]
	*/

	#ifdef MACVERSION
		return (FrontWindow ());
	#endif

	#ifdef WIN95VERSION
		HWND w;

		w = GetTopWindow (hwndMDIClient);
		
		while (w && !IsWindowVisible (w))
			w = GetNextWindow (w, GW_HWNDNEXT);
		
		return (w);
	#endif

	} /*getfrontwindow*/


WindowPtr getnextwindow (WindowPtr w) {
	
	#ifdef MACVERSION
		WindowPtr	nextWindow;
		//Code change by Timothy Paustian Monday, May 1, 2000 9:18:45 PM
		//Changed to Opaque call for Carbon
		#if ACCESSOR_CALLS_ARE_FUNCTIONS == 1
		nextWindow = GetNextWindow(w);
		#else
		//old code
		nextWindow = ((WindowPtr) (*(WindowPeek) w).nextWindow);
		#endif
		return nextWindow;
	#endif

	#ifdef WIN95VERSION
		w = GetNextWindow (w, GW_HWNDNEXT);
		
		while (w && !IsWindowVisible (w))
			w = GetNextWindow (w, GW_HWNDNEXT);
		
		return (w);
	#endif

	} /*getfrontwindow*/


boolean windowbringtofront (WindowPtr w) {
	
	if (w == nil)
		return (false);
	
	#ifdef MACVERSION
		SelectWindow (w);
	#endif

	#ifdef WIN95VERSION
		setwindowpos (w, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	#endif

	shellwindowmenudirty (); /*the checked item changes*/
	
	return (true);
	} /*windowbringtofront*/


boolean windowsendtoback (WindowPtr w) {

	if (w == nil)
		return (false);
	
	#ifdef MACVERSION
		SendBehind (w, nil);
	#endif

	#ifdef WIN95VERSION
		{
		WindowPtr wnext = getnextwindow (w);
		
		setwindowpos (w, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		
		if (wnext != nil)
			windowbringtofront (wnext);
		}
	#endif
	
	shellwindowmenudirty (); /*the checked item changes*/
		
	return (true);
	} /*windowsendtoback*/


boolean getlocalwindowrect (WindowPtr w, Rect *r) {
	
	if (w == nil) { /*defensive driving*/
		
		zerorect (r);
		
		return (false);
		}
		
	#ifdef MACVERSION
		//Code change by Timothy Paustian Monday, May 1, 2000 9:19:35 PM
		//Changed to Opaque call for Carbon
		//we are assuming we want the entire window rectangle here, this may be wrong.
		//All defined in MacWindows.h
		//I had to do it this way to get the quick script window to work.
		//this may be moot because I am going to need to use a different WDEF
		//for the quick script window anyway.
		#if ACCESSOR_CALLS_ARE_FUNCTIONS == 1
		{
		CGrafPtr	thePort;
		
		thePort = GetWindowPort(w);
		GetPortBounds(thePort, r);
		}
		#else
		//old code
		*r = (*w).portRect;
		#endif
	#endif

	#ifdef WIN95VERSION
		{
		RECT winrect;

		GetClientRect (w, &winrect);
		
		setrect (r, winrect.top, winrect.left, winrect.bottom, winrect.right);
		}
	#endif
	
	return (true);
	} /*getlocalwindowrect*/
	

boolean getglobalwindowrect (WindowPtr w, Rect *r) {
	
	if (!getlocalwindowrect (w, r))
		return (false);
	
	localtoglobalrect (w, r);
	
	#ifdef WIN95VERSION
		{
		long style = GetWindowLong (w, GWL_STYLE);

		if (style & WS_VSCROLL)
			(*r).right += 16;
		else
			(*r).right += 3;
		
		if (style & WS_HSCROLL)
			(*r).bottom += 16;
		else
			(*r).bottom += 3;
		}
	#endif
	
	return (true);
	} /*getglobalwindowrect*/


#if !flruntime

boolean windowsendbehind (WindowPtr w, WindowPtr wbehind) {
	
	/*
	5.0a10 dmb: call dirty windowmenu for all platforms
	*/

	#ifdef MACVERSION
		boolean flmovingup;
		
		if (w == nil)
			return (false);
		
		flmovingup = w != getfrontwindow (); /*maybe moving up*/
		
		SendBehind (w, wbehind);
		
		if (flmovingup) { /*IM I-286*/
			//Code change by Timothy Paustian Monday, May 1, 2000 9:28:35 PM
			//Changed to Opaque call for Carbon
			//note this routine is never called. The compiler strips it out.
			#if ACCESSOR_CALLS_ARE_FUNCTIONS == 1
			#else
			//old code
			register WindowPeek wpeek = (WindowPeek) w;
			
			PaintOne ((WindowPtr) wpeek, (*wpeek).strucRgn);
			
			CalcVis ((WindowPtr) wpeek);
			#endif
			}
	#endif

	#ifdef WIN95VERSION
		setwindowpos (w, wbehind, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	#endif
	
	shellwindowmenudirty (); /*the checked item changes*/
	
	return (true);
	} /*windowsendbehind*/


boolean findmousewindow (Point globalpt, WindowPtr *w, short *part) {
	
	#ifdef MACVERSION
		*part = FindWindow (globalpt, w);
	#endif

	#ifdef WIN95VERSION
		POINT winPt;
		winPt.x = globalpt.h;
		winPt.y = globalpt.v;
		*w = ChildWindowFromPoint (hwndMDIClient, winPt);	// *** can't pass NULL?
	#endif
	
	return (*w != nil);
	} /*findmousewindow*/


boolean windowsetcolor (WindowPtr w, long color, boolean flforeground) {
	
	if (w == nil)
		return (false);
		
	#ifdef MACVERSION
		//Code change by Timothy Paustian Monday, August 21, 2000 4:23:48 PM
		//pushport needs a CGrafPtr, passing windowPtr causes a crash on OS X
		{
		CGrafPtr	thePort;
		#if TARGET_API_MAC_CARBON == 1
		thePort = GetWindowPort(w);
		#else
		thePort = (CGrafPtr)w;
		#endif
		
		pushport (thePort);
		}
		if (flforeground)
			ForeColor (color);
		else
			BackColor (color);
		
		popport ();
	#endif

	#ifdef WIN95VERSION
	#endif
	
	return (true);
	} /*windowsetcolor**/


boolean isdeskaccessorywindow (WindowPtr w) {
	
	#ifdef MACVERSION
		//Code change by Timothy Paustian Monday, May 1, 2000 9:31:55 PM
		//Changed to Opaque call for Carbon
		//I doubt we need this but what the hey.
		return (GetWindowKind(w) < 0);
		//old code
		//return ((*(WindowPeek) w).windowKind < 0);
	#endif

	#ifdef WIN95VERSION
		return (false);
	#endif
	} /*isdeskaccessorywindow*/

#endif


void showwindow (WindowPtr w) {
	
	#ifdef MACVERSION
		ShowWindow (w);
	#endif

	#ifdef WIN95VERSION
		releasethreadglobals ();

		ShowWindow (w, SW_SHOWNA);

		grabthreadglobals ();
	#endif
	} /*showwindow*/


void hidewindow (WindowPtr w) {
	
	/*
	5.0a5 dmb: apparantly, we need to do more to fully activate the next
	window if w is in front now. Using SetWindowPos didn't work, so we 
	make two calls

	5.0a10 dmb: fixed above code. also, added SendBehind code for Mac here, 
	so it doesn't need to be done at the shell level

	5.1.5b15 dmb: still wasn't right. hiding the window leaves it in front.
	we need to activate the frontmost visible window.
	*/

	#ifdef MACVERSION
		HideWindow (w);
		
		if (w == getfrontwindow ()) /*don't allow hidden window to remain active*/
			SendBehind (w, getfrontwindow ());
	#endif

	#ifdef WIN95VERSION
		WindowPtr wfront = getfrontwindow ();
		
		if (!IsWindowVisible (w))
			return;

		releasethreadglobals ();
		
		SetWindowPos (w, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_HIDEWINDOW);

	//	ShowWindow (w, SW_HIDE);
		
		if (w == wfront) { // we were in front; activate first visible
			
			w = GetTopWindow (hwndMDIClient);
			
			while (w && !IsWindowVisible (w))
				w = GetNextWindow (w, GW_HWNDNEXT);

			if (w)
				SetWindowPos (w, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
				//SetActiveWindow (w);
			}
		
		grabthreadglobals ();
	#endif
	} /*hidewindow*/

boolean windowvisible (WindowPtr w) {
	
	#ifdef MACVERSION
		//Code change by Timothy Paustian Monday, May 1, 2000 9:33:54 PM
		//Changed to Opaque call for Carbon
		return IsWindowVisible(w);
		//old code
		//return ((*(WindowPeek) w).visible);
	#endif

	#ifdef WIN95VERSION
		return (IsWindowVisible (w));
	#endif
	} /*windowvisible*/


hdlregion getupdateregion (WindowPtr w) {
#ifdef MACVERSION
	//Code change by Timothy Paustian Monday, May 1, 2000 9:35:02 PM
	//Changed to Opaque call for Carbon
	//watch out here. Make sure we are not making a memory leak.
	//I wonder if the windows version leaks memory or if all these calls
	//are mapped out and disposed of in the windows version.
	//ask Andre
	#if ACCESSOR_CALLS_ARE_FUNCTIONS == 1
	hdlregion	rgn;
	//I think I found a bug in carbon. Calling get window region looking for
	//the update region is always returning 0,0,0,0. For now use the 
	//content region. I will ask about this on the carbon mailing list.
	rgn = NewRgn();
	GetWindowRegion(w, kWindowContentRgn, rgn);
	return (rgn);
	#else
	//old code
	hdlregion retRegion = (*(WindowPeek) w).updateRgn;
	return (retRegion);
	#endif
#endif

#ifdef WIN95VERSION
	hdlregion rgn;
	rgn = (hdlregion) NewRgn();
	GetUpdateRgn (w, (HRGN) rgn, false);
	return (rgn);
#endif
	} /*getupdateregion*/


hdlregion getvisregion (WindowPtr w) {
#ifdef MACVERSION
	//Code change by Timothy Paustian Monday, May 1, 2000 9:35:02 PM
	//Changed to Opaque call for Carbon
	//watch out here. Make sure we are not making a memory leak.
	//I wonder if the windows version leaks memory or if all these calls
	//are mapped out and disposed of in the windows version.
	//ask Andre
	//This is also not called in the PPC version, dead code.
	#if ACCESSOR_CALLS_ARE_FUNCTIONS == 1
	hdlregion	rgn;
	CGrafPtr	thePort;
	//we have to change to a port to get the visible region
	thePort = GetWindowPort(w);
	rgn = NewRgn();
	rgn = GetPortVisibleRegion(thePort, rgn);
	return (rgn);
	#else
	//old code
	return ((*w).visRgn);
	#endif
#endif

#ifdef WIN95VERSION
	hdlregion rgn;
	HDC hdc;

//	hdc = GetDC (w);
	assert (w == getport ());
	hdc = getcurrentDC ();

	rgn = (hdlregion) NewRgn();
	GetClipRgn (hdc, (HRGN) rgn);
//	ReleaseDC(w, hdc);
	return (rgn);
#endif
	} /*getupdateregion*/


void setwindowrefcon (WindowPtr w, long refcon) {

	#ifdef MACVERSION
		//Code change by Timothy Paustian Monday, May 1, 2000 9:42:05 PM
		//Changed to Opaque call for Carbon
		SetWRefCon(w, refcon);
		//old code
		//(*(WindowPeek) w).refCon = refcon;
	#endif

	#ifdef WIN95VERSION
		SetWindowLong (w, 0, (LONG) refcon);
	#endif
	} /*setwindowrefcon*/


long getwindowrefcon (WindowPtr w) {

	#ifdef MACVERSION
		//Code change by Timothy Paustian Monday, May 1, 2000 9:42:44 PM
		//Changed to Opaque call for Carbon
		return GetWRefCon(w);
		/*old code
		return ((*(WindowPeek) w).refCon);*/
	#endif

	#ifdef WIN95VERSION
		return (GetWindowLong (w, 0));
	#endif
	} /*getwindowrefcon*/



