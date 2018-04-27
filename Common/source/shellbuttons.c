
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

#include "cursor.h"
#include "memory.h"
#include "quickdraw.h"
#include "bitmaps.h"
#include "frontierconfig.h"
#include "font.h"
#include "mouse.h"
#include "scrollbar.h"
#include "sounds.h"
#include "strings.h"
#include "shell.h"
#include "shellbuttons.h"
#include "shellprivate.h"
#include "oplist.h"


#define backgroundcolor lightblueindex

#define backgroundpattern 128


	#define flbitmapsallowed true /*bitmaps wreak havoc on debugging*/

static boolean flbuttonbitmap = false; /*if true, don't use bitmaps*/

static short mousebuttonnumber = 0; /*global for button tracking*/


static void shellgetbuttonlist (hdlwindowinfo hinfo, hdllistrecord *buttonlist) {

	/*
	7.1b18 PBS: get the button list for a window.
	*/

	if ((**hinfo).flsyntheticbuttons)

		*buttonlist = (hdllistrecord) (**hinfo).buttonlist;

	else

		*buttonlist = (hdllistrecord) shellglobals.buttonlist;
	} /*shellgetbuttonlist*/


static long shellgetbuttonlistcount (hdlwindowinfo hinfo) {
	
	/*
	7.1b18 PBS: get the count of buttons.
	*/

	hdllistrecord buttonlist;

	shellgetbuttonlist (hinfo, &buttonlist);

	if (buttonlist == nil)
		return (0L);

	return (opcountlistitems (buttonlist));
	} /*shellgetbuttonlistcount*/


static void shellrefreshbuttonstatus (void) {
	
	hdlwindowinfo hw = shellwindowinfo;
	hdllistrecord buttonlist; // = (hdllistrecord) shellglobals.buttonlist;
	register short ct; // = (**buttonlist).ctitems;
	tybuttonstatus buttonstatus;
	unsigned short buttonsenabled = 0;
	unsigned short buttonsdisplayed = 0;
	short i;
	unsigned short flag = 1;

	shellgetbuttonlist (shellwindowinfo, &buttonlist);

	ct = shellgetbuttonlistcount (shellwindowinfo);

	for (i = 1; i <= ct; i++) {
		
		(*shellglobals.buttonstatusroutine) (i, &buttonstatus);
		
		if (buttonstatus.flenabled)
			buttonsenabled |= flag;

		if (buttonstatus.fldisplay)
			buttonsdisplayed |= flag;

		flag <<= 1;
		}

	(**hw).buttonsenabled = buttonsenabled;

	(**hw).buttonsdisplayed = buttonsdisplayed;
	} /*shellrefreshbuttonstatus*/


static boolean buttonenabled (short ix) {
	
	unsigned short flag = 1L << (ix - 1);
	
	return (((**shellwindowinfo).buttonsenabled & flag) != 0);
	} /*buttonenabled*/


static boolean buttondisplayed (short ix) {
		
	unsigned short flag = 1L << (ix - 1);
	
	return (((**shellwindowinfo).buttonsdisplayed & flag) != 0);
	} /*buttondisplayed*/




#ifdef flbuttoncolor

static short getbuttoncolor (short ix) {
	
	/*
	if (colorenabled () && (**shellwindowinfo).flwindowactive)
		return (lightyellowindex);
	*/
	
	return (whiteindex);
	} /*getbuttoncolor*/

#endif

static short countbuttons (void) {
	
	hdllistrecord buttonlist; // = (hdllistrecord) shellglobals.buttonlist;
	register short ct; // = (**buttonlist).ctitems;
	register short i;
	register short ctdisplayed;

	shellgetbuttonlist (shellwindowinfo, &buttonlist);

	ct = shellgetbuttonlistcount (shellwindowinfo);
	
	ctdisplayed = 0;
	
	for (i = 1; i <= ct; i++) {
		
		if (buttondisplayed (i))
			ctdisplayed++;
		} /*for*/
		
	return (ctdisplayed);
	} /*countbuttons*/
	

static short getbuttonwidth (void) {
	
	/*
	return the width of each button, using the current window's button list
	and the button font, size and style.  it's the maximum string width of
	the names in the button.
	*/
	
	hdllistrecord buttonlist; // = (hdllistrecord) shellglobals.buttonlist;
	register short ct;
	register short i;
	register short maxpixels = 0;
	
	shellgetbuttonlist (shellwindowinfo, &buttonlist);

	ct = shellgetbuttonlistcount (shellwindowinfo);

	//ct = (**buttonlist).ctitems;
	
	pushstyle (buttonfont, buttonsize, boldbuttonstyle);
	
	for (i = 1; i <= ct; i++) {
		
		if (buttondisplayed (i)) {
		
			bigstring bs;
			register short ctpixels;
			
			opgetliststring (buttonlist, i, nil, bs);
			
			ctpixels = stringpixels (bs);
			
			maxpixels = max (maxpixels, ctpixels);
			}
		} /*for*/
		
	popstyle ();
	
	i = maxpixels + (2 * buttontextinset); /*allow extra pixels on each side*/
	
	i = max (minbuttonwidth, i);
	
	return (i);
	} /*getbuttonwidth*/
	

static short countdisplayedbefore (short ix) {
	
	register short ct = ix;
	register short ctdisplayed = 0;
	register short i;
	
	for (i = 1; i < ct; i++)
		if (buttondisplayed (i))
			ctdisplayed++;
			
	return (ctdisplayed);
	} /*countdisplayedbefore*/
	
	
void shellgetbuttonrect (short ix, Rect *rbutton) {
	
	hdllistrecord buttonlist;
	register short ct;
	register short totalwidth;
	register short leftmargin;
	register short buttonwidth;
	Rect r;
	
	if (!buttondisplayed (ix)) {
		
		zerorect (rbutton);
		
		return;
		}
	
	buttonwidth = getbuttonwidth ();
	
	//buttonlist = (hdllistrecord) shellglobals.buttonlist;

	shellgetbuttonlist (shellwindowinfo, &buttonlist);
	
	ct = countbuttons ();
	
	totalwidth = ((ct - 1) * pixelsbetweenbuttons) + (ct * buttonwidth);
	
	r = (**shellwindowinfo).buttonsrect;
	
	leftmargin = ((r.right - r.left) - totalwidth) / 2;
	
	leftmargin = max (leftmargin, pixelsbetweenbuttons);
	
	r.left += leftmargin;
	
	ct = countdisplayedbefore (ix);
	
	r.left += ct * (buttonwidth + pixelsbetweenbuttons);
	
	r.right = r.left + buttonwidth;
	
	r.top += buttontopinset;
	
	r.bottom -= buttonbottominset;
	
	*rbutton = r;
	} /*shellgetbuttonrect*/


static void drawbuttonbackground (Rect r) {
	
	/*
	2.1b3 dmb: use 'ppat' for button background; easier to modify, less code to draw.
	*/
	
	#ifdef gray3Dlook
		eraserect (r);
	#else
		//Code change by Timothy Paustian Friday, June 16, 2000 2:47:25 PM
		//Changed to Opaque call for Carbon
		if ((**shellwindowinfo).flwindowactive) {
			
			if (colorenabled ())
				
				
					DrawThemeWindowHeader (&r, kThemeStateActive);
					
								
			else
				{
				Pattern pat;
				GetQDGlobalsGray(&pat);
				FillRect (&r, &pat);
				}
			}
		else
			{
			//Pattern pat;
			//GetQDGlobalsWhite(&pat);
			//FillRect (&r, &pat);
			DrawThemeWindowHeader (&r, kThemeStateInactive);
			}
	#endif
	} /*drawbuttonbackground*/



#pragma pack(2)
	typedef struct tybuttoninfo {
		
		bigstring bslabel;
		
		boolean flenabled: 1;
		} tybuttoninfo, *ptrbuttoninfo, **hdlbuttoninfo;
#pragma options align=reset


static void
MyThemeButtonDrawCallback (
		const Rect					*bounds,
		ThemeButtonKind				 kind,
		const ThemeButtonDrawInfo	*info,
		UInt32						 userData,
		SInt16						 depth,
		Boolean						 isColorDev)
{
#pragma unused (kind, info, depth, isColorDev)
		
		/*
		7.0b48 PBS: draw the label for a popup menu.
		*/
		ptrbuttoninfo buttoninfo = (ptrbuttoninfo) userData;
		
		pushstyle (buttonfont, buttonsize, 0);

		//movepento ((*bounds).left, (*bounds).top + 10);
		
		centerbuttonstring (bounds, (*buttoninfo).bslabel, !((*buttoninfo).flenabled));

		popstyle ();
		} /*MyThemeButtonDrawCallback*/



void shelldrawbutton (short ix, boolean flpressed) {
	
	hdllistrecord buttonlist;
	Rect rbutton;
	bigstring bs;
	
	if (!buttondisplayed (ix))
		return;
	
	shellgetbuttonlist (shellwindowinfo, &buttonlist);
	
	opgetliststring (buttonlist, ix, nil, bs);
	
	shellgetbuttonrect (ix, &rbutton);
	
	
	
		{
		ThemeButtonDrawInfo drawinfo;
		ThemeButtonDrawUPP drawupp;
		tybuttoninfo buttoninfo;
		boolean flenabled = buttonenabled (ix);
		Handle h;

		copystring (bs, buttoninfo.bslabel);

		buttoninfo.flenabled = flenabled;
		
		drawupp = NewThemeButtonDrawUPP (MyThemeButtonDrawCallback);

		drawinfo.state = kThemeButtonOn;
		
		if (flenabled)
			drawinfo.value = kThemeStateActive;
		else
			drawinfo.value = kThemeStateInactive;
			
		if (flpressed)
			drawinfo.value = kThemeStatePressed;
		
		drawinfo.adornment = 0;
		
		//eraserect (rbutton);

		newtexthandle (bs, &h); 

		DrawThemeButton (&rbutton, kThemeLargeBevelButton, &drawinfo, nil, nil, drawupp, (unsigned long) &buttoninfo); 
		
		//DisposeThemeButtonDrawUPP (MyThemeButtonDrawCallback);
		DisposeThemeButtonDrawUPP (drawupp);
		}
	
	
	} /*shelldrawbutton*/
	

void shelldrawbuttons (void) {
	
	register hdlwindowinfo hw = shellwindowinfo;
	hdllistrecord buttonlist;
	Rect r;
	short i;
	short ctbuttons;
	
	//buttonlist = (hdllistrecord) shellglobals.buttonlist;

	if ((**hw).flhidebuttons) /*7.1b18 PBS: windows may have hidden buttons*/
		return;	

	shellgetbuttonlist (hw, &buttonlist);	

	if (buttonlist == nil) /*no buttons for this window type*/
		return;
	
	shellrefreshbuttonstatus ();
	
	r = (**hw).buttonsrect;
	
	if (flbitmapsallowed)
		flbuttonbitmap = openbitmap (r, shellwindow);
	
	drawbuttonbackground (r);
	
	#ifndef gray3Dlook
		
		//#if !TARGET_API_MAC_CARBON
			movepento (r.left, r.bottom - 1);
			
			pendrawline (r.right, r.bottom - 1);
			
			if ((**hw).contentrect.bottom <= r.top) { /*content is above the buttons rect*/
			
				movepento (r.left, r.top);
			
				pendrawline (r.right, r.top);
				}
		//#endif
	#endif
	
	ctbuttons = opcountlistitems (buttonlist);
	
	for (i = 1; i <= ctbuttons; i++)
		shelldrawbutton (i, false);
		
	if (flbuttonbitmap) {
		
		flbuttonbitmap = false; /*must be reset every time*/
		
		closebitmap (shellwindow);
		}
	
	validrect (r); /*no need to draw this again*/
	} /*shelldrawbuttons*/
	

static void buttontracker (boolean flpressed) {
	
	/*
	callback routine for the mouse tracker.
	*/
	
	pushclip ((**shellwindowinfo).buttonsrect);
	
	shelldrawbutton (mousebuttonnumber, flpressed);
	
	popclip ();
	} /*buttontracker*/


void shellbuttonhit (Point pt) {
	
	hdllistrecord buttonlist;
	register short i;
	short ct;
	
	shellrefreshbuttonstatus ();

	shellgetbuttonlist (shellwindowinfo, &buttonlist);

	ct = shellgetbuttonlistcount (shellwindowinfo);
	
	//buttonlist = (hdllistrecord) shellglobals.buttonlist;
	
	for (i = 1; i <= ct /*(**buttonlist).ctitems*/; i++) {
		
		Rect rbutton;
		
		shellgetbuttonrect (i, &rbutton);
		
		if (pointinrect (pt, rbutton)) {
			
			if (!buttonenabled (i)) {
				
				ouch ();
				
				return;
				}
				
			mousebuttonnumber = i;
			
			buttontracker (true); /*show button in its pressed state*/
			
			if (mousetrack (rbutton, &buttontracker)) {
				
				(*shellglobals.buttonroutine) (i);
				
				buttontracker (false); /*show button in its unpressed state*/
				
				(**shellwindowinfo).lastbuttonhit = i;
				}
			
			/*
			shelldrawbuttons ();
			*/
			
			return;
			}
		} /*for*/
		
	//ouch (); /*loop satisfied, click isn't in one of the buttons*/
	} /*shellbuttonhit*/
	

void shellgetbuttonsrect (hdlwindowinfo hinfo, Rect *buttonsrect) {
	
	/*
	we control the height of the buttonsrect from here.  externally, the buttonsrect
	may be less than the full width of the window, especially if there's a vertical
	scrollbar linked to the window.
	
	11/17/90 DW: called externally from shellwindow.c.
	*/
	
	Rect r;
	
	shellgetwindowrect (hinfo, &r);
	
	r.bottom = r.top + buttonsrectheight;
	
	*buttonsrect = r;
	} /*shellgetbuttonsrect*/


void shellinvalbuttons (void) {
	
	/*
	can be called by a handler when it knows that the buttons need updating.
	
	5.0a17 dmb: use new buttonsenabled/displayed fields to intelligently
	update buttons
	*/
	
	hdlwindowinfo hw = shellwindowinfo;
	unsigned short buttonsenabled = (**hw).buttonsenabled;
	unsigned short buttonsdisplayed = (**hw).buttonsdisplayed;
	
	shellrefreshbuttonstatus ();
	
	if (buttonsenabled != (**hw).buttonsenabled || 
		buttonsdisplayed != (**hw).buttonsdisplayed) {
	
		invalwindowrect (shellwindow, (**shellwindowinfo).buttonsrect);
		}
	} /*shellinvalbuttons*/


boolean shellgetbuttonstring (short ix, bigstring bs) {
	
	hdllistrecord buttonlist;
	
	//buttonlist = (hdllistrecord) shellglobals.buttonlist;

	shellgetbuttonlist (shellwindowinfo, &buttonlist);
	
	return (opgetliststring (buttonlist, ix, nil, bs));
	} /*shellgetbuttonstring*/


void shellbuttongetoptimalwidth (short *width) {
	
	/*
	provided for external callers who are willing to monkey with the width of
	their window to provide for a buttons list.
	
	we compute the optimum window width for the current window's button list.
	
	12/9/91 dmb: only consider displayed buttons
	*/
	
	register short x;
	register short ct;
	
	shellrefreshbuttonstatus ();
	
	/*
	buttonlist = (hdllistrecord) shellglobals.buttonlist;
	
	ct = (**buttonlist).ctitems;
	*/
	
	ct = countbuttons ();
	
	x = ct * getbuttonwidth ();
	
	x += (ct + 1) * pixelsbetweenbuttons; /*account for left and right margin*/
	
	*width = x;
	} /*shellbuttongetoptimalwidth*/


void shellbuttonadjustcursor (Point pt) {
#pragma unused (pt)

	setcursortype (cursorisarrow); /*no special button cursor, 4/23/90*/
	} /*shellbuttonadjustcursor*/
	
	
	
