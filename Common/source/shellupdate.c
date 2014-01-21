
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
#include "scrollbar.h"
#include "shell.h"
#include "shellbuttons.h"
#include "shellprivate.h"
#include "threads.h"
#include "frontierwindows.h"



#ifdef WIN95VERSION

static PAINTSTRUCT globalWinPaintstruct;

static void BeginUpdate (WindowPtr w) {
	HDC hdc;
	HWND hwnd;
	hdlwindowinfo hinfo;
	Rect r;
	
	assert (shellevent.what == updateEvt);
	
	getwindowinfo (w, &hinfo);
	
	if (hinfo != nil)
		(**hinfo).drawrgn = getupdateregion (w);
	
	hwnd = (HWND) w;
	hdc = BeginPaint (hwnd, &globalWinPaintstruct);
	winpushport (w, hdc);
	
	getlocalwindowrect (w, &r);

	/* if hbrBackground brush is NULL...
	pushbackcolor (&graycolor);
	eraserect (r);
	popbackcolor ();
	*/
	}

static void EndUpdate (WindowPtr w) {
	HWND hwnd;

	assert (shellevent.what == updateEvt);

	hwnd = (HWND) w;
	winpopport();
	EndPaint (hwnd, &globalWinPaintstruct);
	}

#endif


void shellupdatewindow (WindowPtr w) {
	
	/*
	1/17/91 dmb: draw scrollbars, buttons, etc. before window contents
	*/
	
	hdlwindowinfo hinfo;
	register hdlwindowinfo hw;
	
	getwindowinfo (w, &hinfo);
	
	hw = hinfo; /*copy into register*/
	
	if ((hinfo == nil) || (**hinfo).fldisposewhenpopped) {

		#ifdef MACVERSION
			BeginUpdate (w);
			
			EndUpdate (w);
		#endif
		
		return;
		}
	
	BeginUpdate (w);
	
	if (!shellpushglobals (w))
		goto exit;
	
	//Code change by Timothy Paustian Monday, August 21, 2000 4:31:49 PM
	//Must pass a CGrafPtr to pushport on OS X to avoid a crash
	{
	CGrafPtr	thePort;
	thePort = GetWindowPort(w);
		
	pushport (thePort);/*7/7/90 DW: this probably is not necessary, shellpushglobals does it*/
	}
#ifdef MACVERSION	
	if (!config.fldialog) /*if it's a dialog, the callback routine re-drew the controls*/
		DrawControls (w);
	
	if (config.flvertscroll)
		showscrollbar ((**hw).vertscrollbar); /*make sure it is visible*/
	
	shelldrawgrowicon (hw);
	
	if (config.flhorizscroll)
		showscrollbar ((**hw).horizscrollbar);
	//Code change by Timothy Paustian Saturday, April 29, 2000 11:07:46 PM
	//Changed to Opaque call for Carbon
	//This is working, but it is giving bact the coordinates in local not
	//global. It seems to make more sense to me so leave it.
	#if ACCESSOR_CALLS_ARE_FUNCTIONS == 1
	{
	CGrafPtr	thePort = GetWindowPort(w);
	(**hw).drawrgn = NewRgn(); //note we delete this below
	(**hw).drawrgn = GetPortVisibleRegion(thePort, (**hw).drawrgn);
	}
	#else
	//old code
	(**hw).drawrgn = (*w).visRgn; /*so updater knows what needs drawing*/
	#endif
#endif

#ifdef WIN95VERSION
//	(**hw).drawrgn = getvisregion (w);

	pushcliprgn ((**hw).drawrgn, false);
#endif

	if (shellrectneedsupdate (&(**hw).buttonsrect)) /*if window has an attached button list, draw it*/
		shelldrawbuttons ();
	
	drawwindowmessage (w);
	
	// 4.11.97 dmb: it appears that under Windows we need to push the clipk here.
	// Most likely, we should be establishing this wider clip soon arter window
	// creation, one time.

	if (shellrectneedsupdate (&(**hw).contentrect))
		(*shellglobals.updateroutine) ();
	
#ifdef WIN95VERSION
	popclip ();
	
	DeleteObject ((**hw).drawrgn);
#endif
#ifdef MACVERSION
	//Code change by Timothy Paustian Saturday, April 29, 2000 11:11:12 PM
	//Changed to Opaque call for Carbon
	//Get rid of the drawrgn to prevent a memory leak
	#if ACCESSOR_CALLS_ARE_FUNCTIONS == 1
	DisposeRgn((**hw).drawrgn);	
	#endif
#endif
	(**hw).drawrgn = nil; /*this is a temp, keep it nil normally*/
	
	popport (); /*7/7/90 DW: see comment for pusport, above*/
	
	shellpopglobals ();
	
	exit:
	
	EndUpdate (w);
	} /*shellupdatewindow*/


void shellupdatecontent (Rect contentrect) {
	
	/*
	update the portion of contectrect in shellwindow that needs updating, 
	and validate the area updated
	
	2/11/91 dmb: it turns out that we usually want to update the scroll 
	bars at this time too, before redrawing contectrect
	
	12/9/91 dmb: quick bail-out if update rgn is empty
	
	9/22/92 dmb: don't draw scrollbars when update rgn is empty
	*/
	
	register hdlwindowinfo hw = shellwindowinfo;
	register hdlregion contentrgn, updatergn;
	hdlregion actualupdatergn;
	
	actualupdatergn = getupdateregion (shellwindow);

	if (EmptyRgn (actualupdatergn)) {
		
		#ifdef WIN95VERSION
			DeleteObject (actualupdatergn);
		#endif

		return;
		}
	
	shellupdatescrollbars (hw);
	
	updatergn = NewRgn ();

	CopyRgn (actualupdatergn, updatergn); /*window's update rgn is global*/
	
	globaltolocalrgn (updatergn);
	
	#ifdef MACVERSION
		RectRgn (contentrgn = NewRgn (), &contentrect);
		#if ACCESSOR_CALLS_ARE_FUNCTIONS == 1
		//in carbon a copy of the update region is returned.
		DisposeRgn(actualupdatergn);
		actualupdatergn = nil;
		#endif
	#endif
	#ifdef WIN95VERSION
		DeleteObject (actualupdatergn);
		insetrect (&contentrect, -1, -1);
		contentrgn = CreateRectRgn (contentrect.left, contentrect.top, contentrect.right, contentrect.bottom);
	#endif
	
	SectRgn (contentrgn, updatergn, contentrgn); /*re-use contentrgn*/

	
#if defined(WIN95VERSION) && fldebug

/****** DEBUG CODE *********/
	if (false) {
		
		RECT rrr;

		GetClientRect (shellwindow, &rrr);

		DeleteObject(contentrgn);

		contentrgn = CreateRectRgn (rrr.left, rrr.top, rrr.right, rrr.bottom);

		FillRect (getport(), &rrr, GetStockObject (WHITE_BRUSH));
		}

#endif

	if (!EmptyRgn (contentrgn)) {
		
		(**hw).drawrgn = contentrgn; /*for display routines to refer to*/
		//Code change by Timothy Paustian Monday, June 19, 2000 3:09:46 PM
		//Changed to Opaque call for Carbon
		#ifdef MACVERSION
			ValidWindowRgn((WindowRef) hw, contentrgn);
		#endif
		#ifdef WIN95VERSION
			ValidateRgn (shellwindow, contentrgn);
		#endif
		
		pushcliprgn (contentrgn, false);
		
		(*shellglobals.updateroutine) ();
		
		popclip ();
		
		(**hw).drawrgn = nil; /*keep it neat, this guy is a temp*/
		}
	
	DisposeRgn (updatergn);
	
	DisposeRgn (contentrgn);
	} /*shellupdatecontent*/


boolean shellupdatenow (WindowPtr wupdate) {
	
	/*
	can be called from within one of the callbacks to force an update
	to happen immediately, without waiting for the OS to generate an
	update event.
	
	dmb 8/14/90:  can't we just call the normal update routine?  this
	should be fine from the toolbox's point of view, but will not limit 
	the update to contentrect.
	*/
#ifdef MACVERSION
	hdlregion rgn;

	rgn = getupdateregion (wupdate);

	if (!EmptyRgn (rgn))
		shellupdatewindow (wupdate);
	#if ACCESSOR_CALLS_ARE_FUNCTIONS == 1
	//Code change by Timothy Paustian Thursday, May 18, 2000 10:24:57 PM
	//Changed because calling getupdateregion returns a copy of the region
	//we have to dispose of it to prevent a memory leak
	DisposeRgn(rgn);
	#endif
#endif

#ifdef WIN95VERSION
	releasethreadglobals ();

	UpdateWindow ((HWND) wupdate);

	grabthreadglobals ();
#endif
	return (true);
	} /*shellupdatenow*/


static boolean updatenowvisit (WindowPtr wupdate, ptrvoid refcon) {
#pragma unused (refcon)

	shellupdatenow (wupdate);
	
	return (true);
	} /*updatenowvisit*/


/*updates all windows immediately*/
void shellupdateallnow (void) {
	
	shellvisittypedwindows (-1, &updatenowvisit, nil); /*visit all windows*/
	} /*shellupdateallnow*/
	
	
boolean shellhandleupdate (void) {
	
	/*
	3/30/93 dmb: make sure it's a shell window
	
	5.1.6 dmb: no, even for non-shell windows, we need to begin/end update
	*/
	
	WindowPtr w = (WindowPtr) shellevent.message;
	
	if (!isshellwindow (w)) {
		
		#ifdef MACVERSION
			BeginUpdate (w);
			EndUpdate (w);
		#endif
		
		return (false);
		}
		
	shellupdatewindow (w);
	
	return (true);
	} /*shellhandleupdate*/


