
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

/*
bitmaps.c -- routines which manage off-screen bitmaps.  In order to use these routines
the program must call "initbitmaps" before calling anything.

initbitmaps takes a boolean.  if false, then all subsequent calls to bitmap routines do
nothing but return.
*/

#include <land.h>

#include <Gestalt.h>
#include <qdoffscreen.h>
#include <standard.h> 
#include <QuickDraw.h>
#include "mac.h"
#include "quickdraw.h"
#include "font.h"
#include "bitmaps.h"
#include "shellhooks.h"


#define qd32trap 0xAB03
/*
0xAB1D
*/
#define unimptrap 0xA89F



static boolean flbitmapsenabled = false; /*turned on by initbitmaps*/

static boolean flbitmapopen = false; /*is one open right now?*/

//Code change by Timothy Paustian Saturday, April 29, 2000 10:35:03 PM
//Changed to Opaque call for Carbon
//I am getting rid of savedbitmap and using psavedport to do the stuff
static BitMap offscreenbitmap;

static Handle bitmapbasehandle;

static boolean flhas32bitqd = false; /*32-bit quickdraw implemented?*/

static CGrafPtr psavedport;

static GWorldPtr poffscreenworld;

static GDHandle hsavedgd;

static Rect offscreenrect;



static boolean openworld (Rect r, WindowPtr w) {
	
	/*
	2.1b13 dmb: set the noNewDeviceBit and useTempMemBit for NewGWorld 
	too; in 2.1, we're never actually calling UpdateGWorld anymore
	
	3.0.2b1 dmb: was using useTempMemBit instead of useTempMem
	
	3.0.2 dmb: don't set the noNewDeviceBit, because the ROM can leak memory
	*/
	
	short fontnum, fontsize, fontstyle;
	PenState pen;
	Rect			portRect;
	
	GetGWorld (&psavedport, &hsavedgd);
	
//	assert (w == (WindowPtr) psavedport);
	
	offscreenrect = r;
	
	localtoglobalrect (w, &r);
	//Code change by Timothy Paustian Friday, May 19, 2000 1:12:47 PM
	//If we get an empty rect, then just return. This prevents a paramErr
	//from update or NewGWorld
	if(EmptyRect(&r))
		return false;
	if (poffscreenworld != nil) {
		
		//Code change by Timothy Paustian Friday, May 19, 2000 12:44:38 PM
		//I think we don't want to use temp memory for this.
		//I checked this a bit and it fails when an empty rect is brought in
		//lets try checking for this first above
		if (UpdateGWorld (&poffscreenworld, 0, &r, nil, nil, 0) & gwFlagErr)
			return (false);
		}
	else {
		//Code change by Timothy Paustian Friday, May 19, 2000 12:44:38 PM
		//I think we don't want to use temp memory for this.
		if (NewGWorld (&poffscreenworld, 0, &r, nil, nil, 0) != noErr)
			return (false);
		}
	
	getfontsizestyle (&fontnum, &fontsize, &fontstyle);
	
	GetPenState (&pen);
	
	SetGWorld (poffscreenworld, nil);
	//Code change by Timothy Paustian Saturday, April 29, 2000 10:02:33 PM
	//Changed to Opaque call for Carbon
	#if ACCESSOR_CALLS_ARE_FUNCTIONS == 1
	//LockPortBits(poffscreenworld);
	//watch out for this one. I think it is working.
	{
	PixMapHandle	portPixMap = GetPortPixMap(poffscreenworld);
	LockPixels (portPixMap);
	}
	#else
	//old code
	LockPixels (poffscreenworld->portPixMap);
	#endif
	setfontsizestyle (fontnum, fontsize, fontstyle);
	
	SetPenState (&pen);
	
	SetOrigin (offscreenrect.left, offscreenrect.top);
	//Code change by Timothy Paustian Saturday, April 29, 2000 10:06:50 PM
	//Changed to Opaque call for Carbon
	#if ACCESSOR_CALLS_ARE_FUNCTIONS == 1
	GetPortBounds(poffscreenworld, &portRect);
	ClipRect (&portRect);
	#else
	//old code
	#pragma unused(portRect)
	ClipRect (&poffscreenworld->portRect); /*recommended by Apple DTS*/
	#endif
	/*
	CopyBits (&((GrafPtr) w)->portBits, &((GrafPtr) poffscreenworld)->portBits,
		&offscreenrect, &poffscreenworld->portRect, srcCopy, nil);
	
	EraseRect (&poffscreenworld->portRect);
	*/
	
	return (true);
	} /*openworld*/


static void closeworld (WindowPtr w) {
	
	/*
	2/8/91 dmb: experiments show that we get better performance tossing 
	the gworld every time that we do using updategworld on the same one.
	*/
	Rect	portRect;
	SetGWorld (psavedport, hsavedgd);
	//Code change by Timothy Paustian Saturday, April 29, 2000 10:12:08 PM
	//Changed to Opaque call for Carbon
	#if ACCESSOR_CALLS_ARE_FUNCTIONS == 1
	GetPortBounds(poffscreenworld, &portRect);
	CopyBits (GetPortBitMapForCopyBits(poffscreenworld), GetPortBitMapForCopyBits(psavedport),
		&portRect, &offscreenrect, srcCopy, nil);
	#else
	//old code
	#pragma unused(portRect)
	CopyBits (&((GrafPtr) poffscreenworld)->portBits, &((GrafPtr) psavedport)->portBits,
		&poffscreenworld->portRect, &offscreenrect, srcCopy, nil);
	#endif
	
	DisposeGWorld (poffscreenworld);
	
	poffscreenworld = nil;
	
	/*
	UnlockPixels (poffscreenworld->portPixMap);
	*/
	} /*closeworld*/


static void initworld (void) {
	
	poffscreenworld = nil;
	} /*initworld*/


static boolean openmono (Rect r, WindowPtr w) {
	
	register short nrowbytes;
	register long nboxbytes;
	register long sizehandle;
	
	nrowbytes = (r.right - r.left + 7) / 8;
	
   	if (odd (nrowbytes)) 
      	nrowbytes++;
	
	nboxbytes = (r.bottom - r.top) * nrowbytes;
	
	SetHandleSize (bitmapbasehandle, nboxbytes);
	
	sizehandle = GetHandleSize (bitmapbasehandle);
	
	if (sizehandle < nboxbytes) {
	
 		SetHandleSize (bitmapbasehandle, 10L);
		
      	return (false);
      	}
	
	HLock (bitmapbasehandle);
	
	offscreenbitmap.baseAddr = *bitmapbasehandle;
	
	offscreenbitmap.rowBytes = nrowbytes;
	
	offscreenbitmap.bounds = r;
	
	//Code change by Timothy Paustian Saturday, April 29, 2000 10:16:24 PM
	//Changed to Opaque call for Carbon
	//we no longer need savedbitmap
	//old code
	//savedbitmap = (*w).portBits;
	
	SetPortBits (&offscreenbitmap);
	
	return (true);
	} /*openmono*/


static void closemono (WindowPtr w) {
//Code change by Timothy Paustian Tuesday, May 16, 2000 10:06:53 PM
//Changed to Opaque call for Carbon
//don't need to worry about this one because it only gets called when 32bit quickdraw is not
//present. These machines won't run Mac OS X anyway.
#if ACCESSOR_CALLS_ARE_FUNCTIONS == 1
//a signal to show that this was called, it should not be
assert(false);
#else
	SetPortBits (psavedport);
	
   	CopyBits (
   		&offscreenbitmap, &((GrafPtr) w)->portBits, &offscreenbitmap.bounds, 
   	
   		&offscreenbitmap.bounds, srcCopy, nil);
   	//#endif
   	
	HUnlock (bitmapbasehandle);
	
  	offscreenbitmap.baseAddr = 0;
 #endif	
  	/*
	SetHandleSize (bitmapbasehandle, 10L);
	*/
	} /*closemono*/


static void initmono (void) {
	
  	bitmapbasehandle = NewHandle (10L); 
  		
  	offscreenbitmap.baseAddr = 0;
  		
  	offscreenbitmap.rowBytes = 2;
  		
  	SetRect (&offscreenbitmap.bounds, 0, 0, 0, 0);
  	} /*initmono*/


boolean openbitmap (Rect r, WindowPtr w) {
	
	boolean fl;
	
	if (flbitmapopen)
		return (false); /*can't nest offscreens*/
	
	if (!flbitmapsenabled) /*application doesn't want any bitmaps*/
		return (false);
	
	flbitmapopen = true; /*set now so growzone won't mess with handle*/
	
	if (flhas32bitqd)
		fl = openworld (r, w);
	else
		fl = openmono (r, w);
	
	if (!fl) {
	
		flbitmapopen = false; /*well, it didn't work out*/
	
		return (false);
		}
	
	return (true);
	} /*openbitmap*/


boolean openbitmapcopy (Rect r, WindowPtr w) {
	
	//Code change by Timothy Paustian Saturday, April 29, 2000 10:45:30 PM
	//Changed to Opaque call for Carbon
	
	if (!openbitmap (r, w))
		return (false);
	
	if (flhas32bitqd)
	{
		#if ACCESSOR_CALLS_ARE_FUNCTIONS == 1
		assert(false); //this never gets called in the latest version
		#else
		//old code
		CopyBits (&((GrafPtr) w)->portBits, &((GrafPtr) poffscreenworld)->portBits,
				&r, &poffscreenworld->portRect, srcCopy, nil);
		#endif
	}
	else
	{	
		
		#if ACCESSOR_CALLS_ARE_FUNCTIONS == 0
		//old code
		CopyBits (&((GrafPtr) psavedport)->portBits, &offscreenbitmap, &r, &r, srcCopy, nil);
		#endif
		
	}
	return (true);
	} /*openbitmapcopy*/


void closebitmap (WindowPtr w) {
	
	if (flbitmapsenabled && flbitmapopen) {
		
		flbitmapopen = false;
		
		if (flhas32bitqd)
			closeworld (w);
		else
			closemono (w);
		}
	} /*closebitmap*/

//Code change by Timothy Paustian Sunday, May 7, 2000 10:59:28 PM
//Changed to Opaque call for Carbon
//this is really old and it should be removed
static boolean trapimplemented (short trapnum) {
	
	//Code change by Timothy Paustian Sunday, May 7, 2000 10:56:54 PM
	//Changed to Opaque call for Carbon
	#if ACCESSOR_CALLS_ARE_FUNCTIONS == 1
	//always return false for traps since this not allowed in carbon
	//This really is not needed but what the heck
	return false;
	#else
	return (NGetTrapAddress (trapnum, ToolTrap) != NGetTrapAddress (unimptrap, ToolTrap));
	#endif
	} /*trapimplemented*/


static boolean flushbitmap (long *ctbytesneeded) {
	
	if (!flbitmapopen && (*ctbytesneeded > 0)) {
		
		*ctbytesneeded -= GetHandleSize (bitmapbasehandle);
		
		SetHandleSize (bitmapbasehandle, 0L);
		}
	
	return (true);
	} /*flushbitmap*/


void initbitmaps (boolean fl) {
	
	/*
	2/11/91 dmb: got the gWorld routines to work properly, bypassing the 
	GetGWorldPixMap access routine which seemed to be returning the high 
	word only of the pixmaphandle.  however, performance was about three 
	times slower than using bitmaps, so flhas32bitqd is wired off here.  
	to support color, restore the assignments using Gestalt/SysEnvirons, 
	and create color windows in newshellwindow ().
	
	12/4/91 dmb: added flushbitmap memoryhook
	*/
	
	long qdversion;
	
	flbitmapsenabled = fl;
	
	if (flbitmapsenabled) {
		
		if (Gestalt (gestaltQuickdrawVersion, &qdversion) == noErr)
			flhas32bitqd = qdversion >= gestalt32BitQD;
		else
//Code change by Timothy Paustian Sunday, May 7, 2000 10:59:15 PM
//Changed to Opaque call for Carbon
//this looks like really old code for pre system 7 and we should be able to remove it now.
			flhas32bitqd = gHasColorQD && trapimplemented (qd32trap);
		
		if (flhas32bitqd)
			initworld ();
		else
			initmono ();
		
		shellpushmemoryhook (&flushbitmap);
	  	}
	} /*initbitmaps*/



