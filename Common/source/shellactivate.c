
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
#include "scrap.h"
#include "scrollbar.h"
#include "frontierwindows.h"
#include "shell.h"
#include "shellbuttons.h"
#include "shellprivate.h"


boolean shellactivatewindow (WindowPtr w, boolean flactivate) {
	
	/*
	activate or deactivate the indicated window.
	
	8/31/90 DW: we no longer set the globals at the end of the routine. 
	
	10/11/90 dmb: set superglobals up now
	
	3/30/93 dmb: make sure it's a shell window
	
	2.1a7 dmb: fixed shaging of display so newly-opened windows look good
	
	4.1b4 dmb: overload the config.fleraseonresize flag to avoid updating 
	iowa windows while activating

	5.0a10 dmb: dirty the window menu
	*/
	
	register hdlwindowinfo hinfo;
	register hdlscrollbar vertbar, horizbar;
	//Code change by Timothy Paustian Saturday, April 29, 2000 11:03:12 PM
	//Changed to Opaque call for Carbon
	//moved r to outside, since need for carbon call below
	Rect r;
	
#ifdef WIN95VERSION
	RECT winrect;
#endif
	
	if (!isshellwindow (w)) /*chase the refcon field of the mac window*/
		return (false);
	
	shellwindowmenudirty ();

	shellsetsuperglobals (); /*do everyone a favor - make superglobals current right away*/
	
	if (!shellpushglobals (w)) /*install globals of newly active or de-active window*/
		return (false);
	
	hinfo = shellwindowinfo; /*copy into register*/
	
	(**hinfo).flwindowactive = bitboolean (flactivate);

#ifdef MACVERSION
	#if ACCESSOR_CALLS_ARE_FUNCTIONS == 1
	{
	CGrafPtr	thePort;
	thePort = GetWindowPort(w);
	GetPortBounds(thePort, &r);	
	}
	#else
	r = w->portRect;
	#endif
	pushclip (r); /*make sure we can draw into whole window*/
#endif
	
#ifdef WIN95VERSION
	GetClientRect (w, &winrect);
	winrecttorect (&winrect, &r);
	pushclip (r);
#endif

	shelldrawbuttons (); /*if window has an attached button list, draw it*/
	
	vertbar = (**hinfo).vertscrollbar;
	
	horizbar = (**hinfo).horizscrollbar;
	
	if (flactivate) {
	
		shellsetscrollbars (w);
		
		displayscrollbar (vertbar);
		
		displayscrollbar (horizbar);
		
		(**hinfo).selectioninfo.fldirty = true; /*dmb: force menu update*/
		}
	
	else {

		disablescrollbar (vertbar);
		
		disablescrollbar (horizbar);
	
	#if TARGET_API_MAC_CARBON != 1
	
		if (isdeskaccessorywindow (getfrontwindow ())) /*DA is being activated*/

	#endif	
	
	shellwritescrap (anyscraptype);
		
		}
	
	shelldrawgrowicon (hinfo);
	
	drawwindowmessage (w);
	
	popclip ();
	
	if ((*shellglobals.dataholder != NULL)) { // 4/8.97 dmb: window has conent
		
		pushclip ((**hinfo).contentrect); /*driver limited to drawing in content rect*/
		
		(*shellglobals.activateroutine) (bitboolean (flactivate));
		
		popclip ();

		#ifdef WIN95VERSION
			if (config.fleraseonresize)
				shellupdatenow (w);
		#endif
		}
	
	shellpopglobals ();
	
	return (true);
	} /*shellactivatewindow*/




