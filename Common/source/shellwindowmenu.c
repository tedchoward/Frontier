
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

#include "menu.h"
#include "sounds.h"
#include "frontierwindows.h"
#include "shell.h"
#include "shellmenu.h"
#include "shellprivate.h"
#include "cancoon.h"
	#include "launch.h" /*For OS X Bring All to Front command*/




static hdlmenu hwindowsmenu = nil;

static boolean flwindowmenudirty = true;

static boolean fllastwasdottedline = false;

static short ixsearch;

static hdlwindowinfo hsearch;




static boolean pushwindowmenuvisit (WindowPtr w, ptrvoid ptr) {
#pragma unused (ptr)

	/*
	3/8/91 dmb: mark the current root item with an asterix, if not checked
	
	4/5/91 dmb: italicize hidden windows

	5.1b23 dmb: for Win threading, check getwindowinfo result

	7.0b26 PBS: mark the current root only on Macintosh. This is because the mark
	used on Windows is a checkmark, which makes for two checkmarks in the Window
	menu, which is confusing.
	*/
	
	bigstring bs;
	WindowPtr wfront;
	register short ix;
	hdlwindowinfo hinfo;
	Style itemstyle = 0;

	#ifdef MACVERSION
		hdlwindowinfo hroot;
	#endif
	
	if (!getwindowinfo (w, &hinfo))
		return (false);
	
#ifdef xxxPIKE
	/* Pike only displays visible .root windows */
	if ((hinfo != nil) && ((**hinfo).configresnum == idcancoonconfig) && (**hinfo).flhidden)
		return (true);
#endif

	shellgetwindowtitle (hinfo, bs); // 7.24.97 dmb: was windowgettitle
	
	if (isemptystring (bs))
		return (true);
	
	if (!pushmenuitem (hwindowsmenu, windowsmenu, bs, 0))
		return (false);
	
	fllastwasdottedline = false;

	frontshellwindow (&wfront);
	
	ix = countmenuitems (hwindowsmenu);
	
	if (w == wfront)
		checkmenuitem (hwindowsmenu, ix, true);
	
	#ifdef MACVERSION /*7.0b26: mark the current root only if this is the Mac version.*/

		else {
			if (frontrootwindow (&hroot) && (hinfo == hroot)) /*we're the active root*/
				markmenuitem (hwindowsmenu, ix, '×');
			}
	#endif
	
	if ((**hinfo).flmadechanges)
		itemstyle += underline;
	
	if ((**hinfo).flhidden)
		itemstyle += italic;
	
	if (itemstyle != 0)
		stylemenuitem (hwindowsmenu, ix, itemstyle);
	
	if (ix == ixsearch) /*side-effect for window menu selection*/
		hsearch = hinfo;
	
	return (true);
	} /*pushwindowmenuvisit*/
	
	
static boolean shellpushmajortype (short id) {
	
	/*
	there can be more than one window of a "major" type, so if there's more than
	zero of them, we push all their names and push a dotted line after the names.
	*/
	
	register boolean fl = true;
	
	if (counttypedwindows (id) == 0) 
		return (true);
		
	if (!shellvisittypedwindows (id, &pushwindowmenuvisit, nil))
		return (false);
	
	if (!fllastwasdottedline) {

		fl = pushdottedlinemenuitem (hwindowsmenu);
	
		fllastwasdottedline = true;
		}

	return (fl);
	} /*shellpushmajortype*/
	
	
static boolean shellpushminortype (short id) {
	
	/*
	there can be only one window of a "minor" type, so we push it on without the
	ceremonious dotted line.
	*/
	
	register boolean fl;
	
	if (counttypedwindows (id) == 0) 
		return (true);
		
	fl = shellvisittypedwindows (id, &pushwindowmenuvisit, nil);
	
	fllastwasdottedline = false;
	
	return (fl);
	} /*shellpushminortype*/
	
	
boolean shellupdatewindowmenu (void) {
	
	/*
	call this when a window is opened or closed, we traverse the window list, building
	the window menu, sorted by window type.
	
	8/30/90 dmb: check for nil windowsmenu
	
	2/21/91 dmb: window menu is now a main menu; need to disable when empty
	
	9/11/91 dmb: added idlangdialogconfig minor type
	
	5.0d16 dmb: push all odb windows as minor types; kill extra dotted lines
	
	5.0d17 dmb: push idcancoonconfig in export mode _or_ if the root is visible
	
	5.0a2 dmb: new hidewindow item
	
	7.0B44 PBS: new Minimize Window menu item.
	*/
	
	boolean flwasempty;
	hdlwindowinfo hinfo;
	
	hwindowsmenu = shellmenuhandle (windowsmenu);
	
	if (hwindowsmenu == nil)
		return (true);
	
	setmenuitemenable (hwindowsmenu, hidewindowitem, isshellwindow (getfrontwindow ()));
	
	
		setmenuitemenable (hwindowsmenu, minimizewindowitem, isshellwindow (getfrontwindow ()));
		
		setmenuitemenable (hwindowsmenu, bringalltofrontwindowitem, true);
		
	
	if (!flwindowmenudirty)
		return (true);
	
	flwindowmenudirty = false; /*consume it*/
	
	flwasempty = countmenuitems (hwindowsmenu) == 0;

	fllastwasdottedline = true; /*default*/
	
	
		deleteallmenuitems (hwindowsmenu, hidewindowitem + 4);


#ifndef PIKE	
	if (ccinexpertmode () || (ccfindrootwindow (&hinfo) && !(**hinfo).flhidden))
		shellpushmajortype (idcancoonconfig);
#endif

	shellpushminortype (idaboutconfig);
	
#ifdef MACVERSION
	
	shellpushminortype (idplayerconfig); /*7.0b4 PBS: QuickTime Player window*/
	
#endif
	
	shellpushminortype (idcommandconfig);
	
	shellpushminortype (idlangerrorconfig);
	
	/*
	shellpushminortype (idmessageconfig);
	
	shellpushminortype (idprogressconfig);
	*/
	
	shellpushminortype (idstatsconfig);
	
	shellpushminortype (idlangdialogconfig);
	
	if (countmenuitems (hwindowsmenu) > 2 && !fllastwasdottedline) {
		
		pushdottedlinemenuitem (hwindowsmenu);
		
		fllastwasdottedline = true;
		}
	
	shellpushminortype (idtableconfig);
	
	shellpushminortype (idmenueditorconfig);
	
	shellpushminortype (idscriptconfig);
	
	shellpushminortype (idoutlineconfig);
	
	shellpushminortype (idwpconfig);
	
	shellpushminortype (idpictconfig);
	
	shellpushminortype (idiowaconfig);

#ifdef PIKE	
	if (countmenuitems (hwindowsmenu) > 2 && !fllastwasdottedline) {
		
		pushdottedlinemenuitem (hwindowsmenu);
		
		fllastwasdottedline = true;
		}

	if (ccinexpertmode () || (ccfindrootwindow (&hinfo) && !(**hinfo).flhidden))
		shellpushmajortype (idcancoonconfig);
#endif

	if (countmenuitems (hwindowsmenu) > 2 && fllastwasdottedline) /*eliminate extraneous dotted line*/
		deletelastmenuitem (hwindowsmenu);
	
	if (flwasempty != (countmenuitems (hwindowsmenu) == 0)) {
		
		setmenuitemenable (hwindowsmenu, 0, flwasempty);
		
		drawmenubar ();
		}
	
	return (true);
	} /*shellupdatewindowmenu*/
	
	
void shellwindowmenudirty (void) {
	
	flwindowmenudirty = true; /*set local flag*/
	} /*shellwindowmenudirty*/
	

void shellwindowmenuselect (short ixmenu) {
	
	/*
	implemented using a side-effect we programmed into the update process for
	the window menu.
	
	as the menu is being built pushwindowmenuvisit watches for the indicated
	item number, when found it sets the global hsearch.  that's the one we
	want to bring to the front.
	
	5.0a2 dmb: new hidewindow item
	
	7.0b44 PBS: Minimize Window item on Mac OS X.
	*/
	
	boolean flcommand = false;
	
	flwindowmenudirty = true; /*force update*/
	
	if (ixmenu == hidewindowitem) {
		hdlwindowinfo hinfo;
		
		if (getfrontwindowinfo (&hinfo))
			shellhidewindow (hinfo);
		}
	else {
	
		
			if (ixmenu == minimizewindowitem) {
				
				hdlwindowinfo hinfo;
		
				flcommand = true;
							
				if (getfrontwindowinfo (&hinfo))
				
					CollapseWindow ((**hinfo).macwindow, true);				
				} /*if*/
			
			if (ixmenu == bringalltofrontwindowitem) {
				
				activateapplication (nil); /*nil means self*/
				
				flcommand = true;
				} /*if*/
				
		
		if (!flcommand) {

			ixsearch = ixmenu; /*set global*/
			
			hsearch = nil; /*indicate it wasn't found*/
			
			shellupdatewindowmenu (); /*for side-effect*/
			
			if (hsearch != nil) /*it was found*/
				shellbringtofront (hsearch);
			else
				ouch ();
			} /*if*/
		}
	} /*shellwindowmenuselect*/




