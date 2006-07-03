
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

#include "font.h"
#include "quickdraw.h"
#include "scrollbar.h"
#include "shell.h"
#include "shellprivate.h"




/*
void shellsetscrollbarinfo (void) {

	register hdlwindowinfo h = shellwindowinfo;
	
	setscrollbarinfo (
		(**h).vertscrollbar, (**h).vertmin, (**h).vertmax, (**h).vertcurrent);

	setscrollbarinfo (
		(**h).horizscrollbar, (**h).horizmin, (**h).horizmax, (**h).horizcurrent);
	} /%shellsetscrollbarinfo%/
*/

void shellsetdefaultstyle (hdlwindowinfo hinfo) {
	
	/*
	set the defaultstyle field of the windowinfo record based on the bits set in
	the selectioninfo record.
	
	6/28/91 dmb: now use union for style to make things a lot easier
	*/
	
	register hdlwindowinfo hw = hinfo;
	register short x = (**hw).selectioninfo.fontstyle;
	register Style style;
	
	if (x == 0)
		style = normal;
	else
		style = x ^ (**hw).defaultstyle;
	
	(**hw).defaultstyle = style;
	} /*shellsetdefaultstyle*/


void shellsetselectioninfo (void) {
	
	/*
	2/12/92 dmb: keep window's font/size in sync with settings
	*/
	
	register hdlwindowinfo hw = shellwindowinfo;
	
	(*shellglobals.setselectioninforoutine) (); /*fill in selectioninfo record*/
	
	if (shellwindow != nil) {
		
		//Code change by Timothy Paustian Monday, August 21, 2000 4:31:49 PM
	//Must pass a CGrafPtr to pushport on OS X to avoid a crash
	CGrafPtr	thePort;
	#if TARGET_API_MAC_CARBON == 1
	thePort = GetWindowPort(shellwindow);
	#else
	thePort = (GrafPtr)shellwindow;
	#endif
		
	pushport (thePort);
		
		setfontsizestyle ((**hw).selectioninfo.fontnum, (**hw).selectioninfo.fontsize, 0);
		
		popport ();
		}
	
	(**hw).selectioninfo.fldirty = false; /*consume its dirtyness*/
	} /*shellsetselectioninfo*/





