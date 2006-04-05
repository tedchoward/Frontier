
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

#include "scrollbar.h"
#include "quickdraw.h"
#include "shell.h"
#include "shellprivate.h"



void shellupdatescrollbars (hdlwindowinfo hinfo) {
	
	/*
	can be called by an application to update the scrollbars when they need changing
	and the shell isn't expecting a change.
	
	an example -- a range delete operation in a spreadsheet, or a dragging move
	in an outline.
	
	dw to dmb: this routine now takes a hldwindowinfo parameter, not a windowptr.
	*/
	
	register hdlwindowinfo h = hinfo;
	
	setscrollbarinfo ((**h).vertscrollbar, &(**h).vertscrollinfo);
	
	setscrollbarinfo ((**h).horizscrollbar, &(**h).horizscrollinfo);
	
	(**h).fldirtyscrollbars = false;
	} /*shellupdatescrollbars*/
	
	
void shellcheckdirtyscrollbars (void) {
	
	if (!shellpushfrontglobals ()) /*need at least one window open*/
		return;
	
	/*dmb to DW: why was this excluded for dialogs?  no longer valid, but am I breaking something?
	if (!config.fldialog)
	*/
		if ((**shellwindowinfo).fldirtyscrollbars) {
			
			shellupdatescrollbars (shellwindowinfo);
			}
	
	shellpopglobals ();
	} /*shellcheckdirtyscrollbars*/
	
	
void shellsetscrollbars (WindowPtr w) {
	
	/*
	call the driver to set the min, max and current values for the scroll 
	bars.  
	
	8/31/90 DW: we not longer set globals, we now push them.
	*/
	
	shellpushglobals (w);
	
	(*shellglobals.setscrollbarroutine) ();
	
	shellupdatescrollbars (shellwindowinfo);
		
	shellpopglobals ();
	} /*shellsetscrollbars*/


static tydirection scrolldirection (boolean flvert, boolean flup) {

	register tydirection dir;
	
	if (flvert)
		if (flup)
			dir = up;
		else
			dir = down;
	else
		if (flup)
			dir = left;
		else
			dir = right;
	
	return (dir);
	} /*scrolldirection*/




#ifdef WIN95VERSION

void winscroll (boolean isvertscroll, int scrolltype, long pos)
	{

	/*
	7.0b20 PBS: Fix Windows display glitch -- if scroll is finished,
	redraw the scrollbar.
	*/

	boolean flup, flpage;
	long distance;
	tyscrollinfo scrollinfo;
	tydirection dir;

	if (shellwindowinfo != NULL)
		{
		if (isvertscroll)
			{
			scrollinfo = (**shellwindowinfo).vertscrollinfo;
			}
		else
			{
			scrollinfo = (**shellwindowinfo).horizscrollinfo;
			}

		switch (scrolltype)
			{
			case SB_LINEUP:
			case SB_PAGEUP:
			case SB_TOP:
				flup = false;
				break;

			case SB_BOTTOM:
			case SB_LINEDOWN:
			case SB_PAGEDOWN:
				flup = true;
				break;

			case SB_THUMBPOSITION:
			case SB_THUMBTRACK:
				if (pos < scrollinfo.cur)
					flup = false;
				else
					flup = true;
				break;
			default:
				flup = false;
				break;
			}

		dir = scrolldirection (isvertscroll, flup);

		flpage = false;
		distance = 0;

		switch (scrolltype)
			{
			case SB_PAGEUP:
			case SB_PAGEDOWN:
				flpage = true;
				distance = 1;
				break;

			case SB_LINEUP:
			case SB_LINEDOWN:
				distance = 1;
				break;

			case SB_BOTTOM:
				distance = scrollinfo.max - scrollinfo.cur;
				break;

			case SB_TOP:
				distance = scrollinfo.cur - scrollinfo.min;
				break;

			case SB_THUMBPOSITION:
			case SB_THUMBTRACK:
				distance = scrollinfo.cur - pos;
				break;

			default:
				break;
			}

		distance = abs(distance);

		if (distance > 0)
			(*shellglobals.scrollroutine) (dir, flpage, distance);

		if (scrolltype == SB_ENDSCROLL) { /*7.0b20 PBS: Fix Windows display glitch -- redraw.*/

			if (isvertscroll)

				drawscrollbar ((**shellwindowinfo).vertscrollbar);
			
			else

				drawscrollbar ((**shellwindowinfo).horizscrollbar);
			} /*if*/

		}
	}

#endif


#ifdef MACVERSION
static pascal void shellhorizscroll (hdlscrollbar ctrl, short part) {
	
	register hdlwindowinfo h = shellwindowinfo;
	boolean flleft, flpage;
	register tydirection dir;
	
	if (!scrollbarhit (ctrl, part, &flleft, &flpage))
		return;
	
	/*	
	pushclip ((**h).contentrect);
	*/
	
	dir = scrolldirection (false, flleft);
	
	(*shellglobals.scrollroutine) (dir, flpage, 1);
	
	/*
	popclip ();
	*/
	
	setscrollbarinfo ((**h).horizscrollbar, &(**h).horizscrollinfo);
	} /*shellhorizscroll*/


static pascal void shellvertscroll (hdlscrollbar ctrl, short part) {
	
	register hdlwindowinfo h = shellwindowinfo;
	boolean flup, flpage;
	register tydirection dir;
	
	if (!scrollbarhit (ctrl, part, &flup, &flpage))
		return;
	
	/*
	pushclip ((**h).contentrect);
	*/
	
	dir = scrolldirection (true, flup);
		
	(*shellglobals.scrollroutine) (dir, flpage, 1);
	
	/*	
	popclip ();
	*/
	
	setscrollbarinfo ((**h).vertscrollbar, &(**h).vertscrollinfo);
	} /*shellvertscroll*/


//Code change by Timothy Paustian Friday, June 16, 2000 3:14:00 PM
//Changed to Opaque call for Carbon

#if TARGET_RT_MAC_MACHO
	#define shellvertscrollUPP	(&shellvertscroll)
	#define shellhorizscrollUPP	(&shellhorizscroll)
	#define	shelllivescrollupp	(&ScrollThumbActionProc)
#elif TARGET_RT_MAC_CFM
	#if TARGET_API_MAC_CARBON
		ControlActionUPP	shellvertscrollDesc;
		ControlActionUPP	shellhorizscrollDesc;
		ControlActionUPP	shelllivescrollupp;
		#define shellvertscrollUPP (shellvertscrollDesc)
		#define shellhorizscrollUPP (shellhorizscrollDesc)
	#else
		static RoutineDescriptor shellvertscrollDesc = BUILD_ROUTINE_DESCRIPTOR (uppControlActionProcInfo, shellvertscroll);
		static RoutineDescriptor shellhorizscrollDesc = BUILD_ROUTINE_DESCRIPTOR (uppControlActionProcInfo, shellhorizscroll);
		#define shellvertscrollUPP (&shellvertscrollDesc)
		#define shellhorizscrollUPP (&shellhorizscrollDesc)
	#endif
#endif

#if TARGET_API_MAC_CARBON == 1

/*The live scrolling code descends from Apple sample code, hence the different style of code.*/

enum
{
	// Misc scroll bar constants

	kScrollBarWidth = 16,
	kScrollBarWidthAdjust = kScrollBarWidth - 1,
	
	kScrollArrowWidth = 16,
	kScrollThumbWidth = 16,
	kTotalWidthAdjust = (kScrollArrowWidth * 2) + kScrollThumbWidth,
	
	kPageOverlap = 10,
	kThumbTrackWidthSlop = 50,
	kThumbTrackLengthSlop = 113

};


static ControlRef   gControl;
static SInt32		gValueSlop;
//static SInt32		gSaveValue;
static SInt32		gStartValue;

static SInt32	CalcValueFromPoint ( ControlRef theControl, Point thePoint );
static boolean flverticalscroll;
static RgnHandle	gSaveClip = nil;
static long lastpoint;

pascal void ScrollThumbActionProc (void);


static void DisableDrawing ( void ) {

	Rect	nullRect = { 0, 0, 0, 0 };

	GetClip ( gSaveClip );
	ClipRect ( &nullRect );
	
	return;
	} /*DisableDrawing*/


static void EnableDrawing ( void ) {

	SetClip ( gSaveClip );
	
	return;
	} /*EnableDrawing*/


SInt32 CalcValueFromPoint (ControlHandle hControl, Point thePoint) {
	
	/*Figure where we are in scroll bar terms.*/

	SInt32 theValue = 0, theRange, theDistance, thePin;
	Rect rectControl, indicatorbounds;
	WindowPtr lpWindow;
	long thumbheight = 16;
	long thumbwidth = 16;
	RgnHandle indicatorregion = NewRgn ();
	long gTotalVSizeAdjust, gTotalWidthAdjust;
	short baselineoffset;
	
	GetControlRegion (hControl, kControlIndicatorPart, indicatorregion);
	GetRegionBounds (indicatorregion, &indicatorbounds);
	
	thumbheight = indicatorbounds.bottom - indicatorbounds.top;
	
	thumbwidth = indicatorbounds.right - indicatorbounds.left;
	
	DisposeRgn (indicatorregion);
	
	gTotalVSizeAdjust = ((kScrollArrowWidth * 2) + thumbheight);
	
	gTotalWidthAdjust = ((kScrollArrowWidth * 2) + thumbwidth);

	lpWindow = shellwindow; // (*hControl)->contrlOwner;
	
	zerorect (&rectControl);
	
	GetBestControlRect (hControl, &rectControl, &baselineoffset);
	
	//rectControl = (*hControl)->contrlRect;
	
	theRange = GetControlMaximum ( hControl ) - GetControlMinimum ( hControl );
	
	if (flverticalscroll) {
	
		// Scroll distance adjusted for scroll arrows and the thumb
		theDistance = rectControl.bottom - rectControl.top - gTotalVSizeAdjust;
		// Pin thePoint to the middle of the thumb
		thePin = rectControl.top + (thumbheight / 2);
		thePin = lastpoint;
		theValue = ((thePoint.v - thePin) * theRange) / theDistance;
		} /*if*/
	
	else { /*horizontal scrolling*/
		
		theDistance = rectControl.right - rectControl.left - gTotalWidthAdjust;

		thePin = rectControl.left + (thumbwidth / 2);
		
		thePin = lastpoint;
		
		theValue = ((thePoint.h - thePin) * theRange) / theDistance;
		} /*else*/

	theValue += gValueSlop;
	
	if ( theValue < GetControlMinimum ( hControl ) )
		theValue = GetControlMinimum ( hControl );
	else if ( theValue > GetControlMaximum ( hControl ) )
		theValue = GetControlMaximum ( hControl );

	return theValue;
	} /*CalcValueFromPoint*/
	
	
pascal void ScrollThumbActionProc (void) {

	SInt32 theValue;
    hdlscrollbar hscrollbar;
    Point thePoint;
    Rect theRect;
    long ctscroll;
	tydirection dir;
	long currvalue;
	hdlwindowinfo h = shellwindowinfo;
	short baselineoffset;

    if (h == nil) /*defensive driving*/
    	return;

	hscrollbar = (**h).vertscrollbar;
	
	if (!flverticalscroll)
		hscrollbar = (**h).horizscrollbar;
	
	zerorect (&theRect);
	
	GetBestControlRect (hscrollbar, &theRect, &baselineoffset);
 	//theRect = (**hscrollbar).contrlRect;
 	
 	if (flverticalscroll)
		
		insetrect (&theRect, -kThumbTrackLengthSlop, -kThumbTrackWidthSlop);
		
 	else
		
		insetrect (&theRect, -kThumbTrackWidthSlop, -kThumbTrackLengthSlop);
	
    GetMouse (&thePoint);
        
    if (pointinrect (thePoint, theRect))
		
		theValue = CalcValueFromPoint (hscrollbar, thePoint);
		
	else
		
		theValue = gStartValue;
		
	currvalue = (**h).vertscrollinfo.cur;
	
	if (theValue != GetControlValue (hscrollbar)) {	// if we scrolled
		
		EnableDrawing ();

		ctscroll = theValue - GetControlValue (hscrollbar);
		
		dir = scrolldirection (flverticalscroll, ctscroll > 0);	
		
	 	(*shellglobals.scrollroutine) (dir, false, abs (ctscroll));
	 	
		(**h).vertscrollinfo.cur = theValue;

		DisableDrawing ();
		} /*if*/
	
	return;
	} /*ScrollThumbActionProc*/


static OSErr BeginThumbTracking ( ControlRef theControl ) {

	OSErr theErr = noErr;
	Point thePoint;

	gControl = theControl;
	gStartValue = GetControlValue ( theControl );
	
	gValueSlop = 0;
	GetMouse ( &thePoint );
	
	if (flverticalscroll)
		lastpoint = thePoint.v;
	else
		lastpoint = thePoint.h;
	
	gValueSlop = GetControlValue ( theControl ) - CalcValueFromPoint ( theControl, thePoint );
		
	gSaveClip = NewRgn ( );

	DisableDrawing ( );
	
	return theErr;
	} /*BeginThumbTracking*/
	

static void EndThumbTracking ( void ) {
	
//	hdlwindowinfo h = shellwindowinfo;

	EnableDrawing ();

	DisposeRgn ( gSaveClip );

//	if (flverticalscroll)
//		setscrollbarcurrent ((**h).vertscrollbar, (**h).vertscrollinfo.cur);
//	else
//		setscrollbarcurrent ((**h).horizscrollbar, (**h).horizscrollinfo.cur);

	return;
	} /*EndThumbTracking*/

#endif

extern void shellinitscroll ();

void shellinitscroll(void)
{
	//Code change by Timothy Paustian Saturday, July 22, 2000 12:04:35 AM
	//Needed in shellscroll
	#if TARGET_API_MAC_CARBON
		#if TARGET_RT_MAC_CFM
			shellvertscrollDesc = NewControlActionUPP(shellvertscroll);
			shellhorizscrollDesc = NewControlActionUPP(shellhorizscroll);
			shelllivescrollupp = NewControlActionUPP (ScrollThumbActionProc);
		#endif
	#endif
}

extern void shellshutdownscroll ();

void shellshutdownscroll(void)
{
	#if TARGET_API_MAC_CARBON
		#if TARGET_RT_MAC_CFM
			DisposeControlActionUPP(shellvertscrollDesc);
			DisposeControlActionUPP(shellhorizscrollDesc);
			DisposeControlActionUPP (shelllivescrollupp);
		#endif
	#endif
}


void shellscroll (boolean flvert, hdlscrollbar sb, short part, Point pt) {
	
//	register WindowPtr w = shellwindow;
	register long oldscrollbarcurrent;
	#if TARGET_API_MAC_CARBON != 1
		register long ctscroll;
		tydirection dir;
	#endif
	
	//pushclip ((*w).portRect); /*allow drawing in whole window%/
		
	if (part == kControlIndicatorPart) {
		
		oldscrollbarcurrent = getscrollbarcurrent (sb);
		
		#if TARGET_API_MAC_CARBON == 1
			
			gControl = sb;
			
			flverticalscroll = flvert;
			
			gStartValue = oldscrollbarcurrent;
			
			BeginThumbTracking (sb);
			
			TrackControl (sb, pt, (ControlActionUPP) shelllivescrollupp);
			
			EndThumbTracking ();			
		
		#else
		
			TrackControl (sb, pt, nil);
			
			ctscroll = getscrollbarcurrent (sb) - oldscrollbarcurrent;
		
			dir = scrolldirection (flvert, ctscroll > 0);
			
			(*shellglobals.scrollroutine) (dir, false, abs (ctscroll));

		#endif			
		}
	
	else {
	
		if (flvert)
		
			TrackControl (sb, pt, shellvertscrollUPP);
			
		else
		
			TrackControl (sb, pt, shellhorizscrollUPP);
		}
	
	/*
	popclip ();
	*/
	} /*shellscroll*/
	
#endif



