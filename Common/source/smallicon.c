
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
#include "icon.h"
#include "memory.h"
#include "mouse.h"
#include "ops.h"
#include "dialogs.h"
#include "quickdraw.h"
#include "smallicon.h"
#include "frontierwindows.h"



static hdlsmalliconbits hmiscbits;
//static hdlsmalliconbits hleaderbits;

#ifndef version42orgreater
	#define trianglepolyheight 9 /*for popup icons*/
	#define trianglepolywidth 5

	hdlsmalliconbits hfinderbits, hdraggingbits, hcirclebits, htablebits;
#endif




boolean plotsmallicon (tysmalliconspec spec) {

#ifdef MACVERSION
	register hdlsmalliconbits hbits;
	short mode;
	BitMap iconbitmap;
	
	hbits = spec.hbits;
	
	if (hbits == nil) {
	
		switch (spec.iconlist) {
			
			case miscsmalliconlist:
				hbits = hmiscbits;
				
				break;
			
			/*
			case leadersmalliconlist:
				hbits = hleaderbits;
				
				break;
			
			case draggingsmalliconlist:
				hbits = hdraggingbits;
				
				break;
				
			case tablesmalliconlist:
				hbits = htablebits;
				
				break;
			
			case circlesmalliconlist:
				hbits = hcirclebits;
				
				break;
			
			case findersmalliconlist:
				hbits = hfinderbits;
				
				break;
			*/
			
			} /*switch*/
		} /*nil bits*/
		
	if (hbits == nil)
		return (false);
		
	iconbitmap.baseAddr = (Ptr) &(*hbits) [spec.iconnum];
	
	iconbitmap.rowBytes = 2;
	
	iconbitmap.bounds.top = iconbitmap.bounds.left = 0; 
	
	iconbitmap.bounds.bottom = spec.iconrect.bottom - spec.iconrect.top; 
	
	iconbitmap.bounds.right = spec.iconrect.right - spec.iconrect.left;
	
	if (spec.flinverted)
		mode = notSrcCopy;
		
	else {
		if (spec.flclearwhatsthere)
			mode = srcCopy;
		else
			mode = srcOr;
		}
	//Code change by Timothy Paustian Friday, May 5, 2000 10:26:38 PM
	//Changed to Opaque call for Carbon
	#if ACCESSOR_CALLS_ARE_FUNCTIONS == 1
	{//added the bracket to save on code spread
	CGrafPtr windPort = GetWindowPort(spec.iconwindow);
	CopyBits (
		&iconbitmap, GetPortBitMapForCopyBits(windPort), 
		
		&iconbitmap.bounds, &spec.iconrect, mode, nil);
	}
	#else
	CopyBits (
		&iconbitmap, &(*spec.iconwindow).portBits, 
		
		&iconbitmap.bounds, &spec.iconrect, mode, nil);
	#endif
	
	return (true);
#endif

#ifdef WIN95VERSION
	if (!ploticon (&spec.iconrect, spec.iconlist * 10 + spec.iconnum))
		return (false);

	if (spec.flinverted)
		invertrect (spec.iconrect);

	return (true);
#endif
	} /*plotsmallicon*/
	
	
boolean displaypopupicon (Rect r, boolean flenabled) {
	
	
		short themestate = kThemeStateActive;
		
		if (!flenabled)
			themestate = kThemeStateUnavailable;
			
		/*It's always too high.*/
			
		r.top = r.top + 4;
		
		r.bottom = r.bottom + 4;
	
		DrawThemePopupArrow (&r, kThemeArrowDown, kThemeArrow9pt, themestate, NULL, 0);

		return (true);
	
	} /*displaypopupicon*/
	
	
boolean loadsmallicon (short resnum, hdlsmalliconbits *hbits) {
	
#ifdef MACVERSION
	Handle h;
	boolean fl;
	SInt8 hState;
	
	h = GetResource ('SICN', resnum);
	
	LoadResource(h); /*in case resource was purged*/
	hState = HGetState(h);
	HNoPurge(h); /*in case resource is purgeable*/
	fl = copyhandle (h, (Handle *) hbits);
	HSetState(h, hState);
	
	return (fl);
#endif

#ifdef WIN95VERSION
	*hbits = NULL;	// we map to individual icons

	return (true);
#endif
	} /*loadsmallicon*/


boolean myMoof (short ticksbetweenframes, long howlong) {
	
	/*
	Meet myMoof, our tip o the hat to Mac DTS. 
	
	Lest anyone think that UserLand Software doesn't REALLY understand multimedia!
	
	3/17/93 dmb: added howlong parameter so user interaction isn't required
	*/
	
	WindowPtr w;
	tysmalliconspec sicn; 	
	short i;
	Rect rport;
	long tc;
	CGrafPtr	thePort;
	
	tc = gettickcount ();
	
	if (!loadsmallicon (moofsmalliconlist, &sicn.hbits))
		goto error;
	
	setrect (&rport, 0, 0, 32, 32);

	centerrectondesktop (&rport);
	
	w = getnewwindow (132, false, &rport);
	
	if (w == nil) 
		goto error;
	
	if (abs (howlong) < 0x2222222) /*convert to ticks, avoiding overflow*/
		howlong *= 60;
	
#ifdef MACVERSION
	//you cannot call this with a window ptr, the function needs a dialog ptr
	//its really not needed anyway, but I will leave it in for the non-carbon version
#endif

	showwindow (w);	
	
	//Code change by Timothy Paustian Monday, August 21, 2000 4:31:49 PM
	//Must pass a CGrafPtr to pushport on OS X to avoid a crash
	{
	thePort = GetWindowPort(w);
		
	pushport (thePort);
	}
	setrect (&rport, 0, 0, 32, 32); // rport = (*w).portRect;
	
	sicn.iconlist = moofsmalliconlist;
		
	sicn.iconwindow = w;
	
	sicn.flinverted = false;
	
	sicn.flclearwhatsthere = true;
	
	sicn.iconrect = rport;
	
	insetrect (&sicn.iconrect, 8, 8);
	
	while (true) {
	
		for (i = 0; i < 7; i++) {
		
			sicn.iconnum = i;
			
			plotsmallicon (sicn);
			QDFlushPortBuffer(thePort, nil);
			if (mousebuttondown ()) {
				
				while (mousebuttondown ()) {}
				
				/*
				pushstyle (geneva, 9, 0);
				
				eraserect (rport);
				
				centerstring (rport, "\pPOOF!");
				
				popstyle ();
				
				delayticks (30);
				*/
				
				goto endloop;
				}
			
			if (howlong != 0) {
				
				if (gettickcount () - tc > (unsigned long) howlong)
					goto endloop;
				}
			
			delayticks (ticksbetweenframes);
			} /*for*/
		} /*while*/
	
	endloop:
	
	popport ();
	
	disposewindow (w);
	
	disposehandle ((Handle) sicn.hbits);
	
	return (true);

	error:
		
	sysbeep ();
	
	return (false);
	} /*myMoof*/


boolean initsmallicons (void) {
	
	loadsmallicon (miscsmalliconlist, &hmiscbits);
	
	/*
	loadsmallicon (leadersmalliconlist, &hleaderbits);

	loadsmallicon (draggingsmalliconlist, &hdraggingbits);

	loadsmallicon (findersmalliconlist, &hfinderbits);
	
	loadsmallicon (circlesmalliconlist, &hcirclebits);
	
	loadsmallicon (tablesmalliconlist, &htablebits);
	*/
	
	return (true);
	} /*initsmallicons*/


