
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

/*
oppopup.c -- handle right-click popup menus in outlines.

7.0b10 PBS
*/

#include "frontier.h"
#include "standard.h"

#include "quickdraw.h"
#include "op.h"
#include "opdisplay.h"
#include "opinternal.h"
#include "opicons.h"
#include "oplineheight.h"
#include "frontierwindows.h"
#include "popup.h"
#include "lang.h"
#include "menueditor.h"
#include "meprograms.h"
#include "tablestructure.h"
#include "threads.h"
#include "strings.h"
#include "mouse.h"
#include "dockmenu.h"
#include "menu.h"
#include "menuverbs.h"

#define idgetmenuaddresscallback 39

#define maxsubmenus 40

static hdlmenurecord oppopupmenubar; /*current popup menu*/

static boolean opbuildpopupmenu (hdlheadrecord hnode, hdlmenu hmenu);

static boolean oppopupinsertmenuitem (hdlmenu hmenu, short itemnumber, hdlheadrecord hnode);

static boolean oppopupruncallbackscript (tyvaluerecord *val);

short currmenuid;

#ifdef WIN95VERSION

	static unsigned long popupstartticks = 0; /*7.0b27 PBS: when menu was started. Used to make sure menu appeared minimum amount of ticks.*/

	static unsigned long popupticksminimum = 20; /*7.0b27 PBS*/

#endif

#if MACVERSION

#pragma pack(2)
	typedef struct typopupinfo {

		hdlmenu hmenu;
		
		short id;
			
		} typopupinfo, *ptrpopupinfo, **hdlpopupinfo;

	typedef struct typopupmenustack {
		
		typopupinfo popupmenus [maxsubmenus];
		
		short currstackitem;
		} typopupmenustack, *ptrpopupmenustack, **hdlpopupmenustack;
#pragma options align=reset

	typopupmenustack popupmenustack;

#endif


static void oppopupresetmenustack (void) {
	
	#if MACVERSION

		popupmenustack.currstackitem = -1;
		
		popupmenustack.popupmenus [0].hmenu = nil;
		
		popupmenustack.popupmenus [0].id = 0;
	
	#endif
	} /*oppopupresetmenustack*/


static void oppopupdisposemenusinstack (void) {
	
	/*
	Dispose and delete all menus in popup menu stack.
	*/
	
	#if MACVERSION
	
		short ix = popupmenustack.currstackitem;
		short i, id;
		hdlmenu hmenu;
		
		if (ix < 0)
			return;
		
		for (i = ix; i > -1; i--) {
			
			id = popupmenustack.popupmenus [i].id;
			
			if (id > -1)
			
				DeleteMenu (id);
			
			hmenu = popupmenustack.popupmenus [i].hmenu;
			
			if (hmenu != nil)
			
				disposemenu (hmenu);
			} /*for*/
			
	#endif
	} /*oppopupdisposemenusinstack*/


static boolean oppopupaddtomenustack (hdlmenu hmenu, short idsubmenu) {

	#if MACVERSION

		short ix = popupmenustack.currstackitem + 1;
		
		if (ix > maxsubmenus)	
			return (false);
		
		popupmenustack.currstackitem = ix;
		
		popupmenustack.popupmenus [ix].hmenu = hmenu;
		
		popupmenustack.popupmenus [ix].id = idsubmenu;
	
	#endif

	return (true);	
	} /*oppopupaddtomenustack*/


static void oppopupinsertsubmenu (hdlmenu hmenu, short itemnumber, hdlheadrecord hnode) {

	/*
	7.1b23 PBS: build a sub-menu and attach it.
	*/

	hdlmenu hsubmenu;
	short idsubmenu = currmenuid;

	hsubmenu = Newmenu (idsubmenu, BIGSTRING (""));
	
#if MACVERSION
	InsertMenu (hsubmenu, -1);
#endif
	
	opbuildpopupmenu (hnode, hsubmenu);
	
	oppopupaddtomenustack (hsubmenu, idsubmenu);
	
	sethierarchicalmenuitem (hmenu, itemnumber, hsubmenu, idsubmenu);
	} /*oppopupinsertsubmenu*/


static boolean oppopupinsertmenuitem (hdlmenu hmenu, short itemnumber, hdlheadrecord hnode) {
	
	/*
	7.0b23 PBS: allow right-click menu items to be disabled.
	*/

	bigstring bsheadstring;
	boolean flenabled = true;
	boolean flchecked = false; /*it's ignored anyway*/

	getheadstring (hnode, bsheadstring);

	mereduceformula (bsheadstring);
	
	mereducemenucodes (bsheadstring, &flenabled, &flchecked); /*7.0b23 PBS: items can be disabled.*/

	pushpopupitem (hmenu, bsheadstring, flenabled, currmenuid);

	if (ophassubheads (hnode))
		oppopupinsertsubmenu (hmenu, itemnumber, hnode);

	return (true);
	} /*oppopupinsertmenuitem*/


static boolean opbuildpopupmenu (hdlheadrecord hnode, hdlmenu hmenu) {
	
	/*
	7.0b10 PBS: Build a popup menu from the outline pointed to by hnode.
	Recurse to handle a menu that has a submenu.

	Based on mebuildmenu, but different enough to need its own routine.	
	*/
	
	short itemnumber;
	bigstring bs;
	
	getheadstring (hnode, bs); /*get the menu title*/
	
	if (opnosubheads (hnode))
		return (true);
	
	hnode = (**hnode).headlinkright; /*move to first subhead*/
	
	for (itemnumber = 1; ; itemnumber++) {

		currmenuid++;

		if (!oppopupinsertmenuitem (hmenu, itemnumber, hnode))
			return (false);

		if (opislastsubhead (hnode)) /*done with this menu*/
			return (true);
		
		hnode = (**hnode).headlinkdown; /*advance to next item*/
		} /*while*/
	} /*opbuildpopupmenu*/


static boolean oppopupruncallbackscript (tyvaluerecord *val) {

	/*
	7.0b11 PBS: run the callback script that returns the address
	of a menubar object to use.
	*/

	Handle htext;
	boolean fl = false; /*assume it didn't work*/
	bigstring bsscript;
	short idscript = idgetmenuaddresscallback;

	if (getsystemtablescript (idscript, bsscript)) {
	
		if (!newtexthandle (bsscript, &htext))
		
				return (false);

		grabthreadglobals ();

		oppushoutline (outlinedata); /*7.0b10 PBS: make sure the current outline gets saved.*/
		
		fl = langrun (htext, val);

		oppopoutline (); /*7.0b10 PBS: restore the current outline*/

		releasethreadglobals ();
		}

	return (fl);
	} /*oppopupruncallbackscript*/


static boolean oppopupgetaddresscallback (tyvaluerecord *val) {

	/*
	7.0b11 PBS: get the address of a menubar object to use
	in creating the popup menu. Call a callback script.
	*/

	if (!oppopupruncallbackscript (val))
		return (false);

	return (true);
	} /*oppopupgetaddresscallback*/


static boolean opfillpopup (hdlmenu hmenu, short *checkeditem) {

	/*
	7.1b43 PBS: make sure the menu object is in memory, so the
	app doesn't crash.
	*/

	hdlhashtable htable;
	hdlhashnode hnode;
	tyvaluerecord valaddress;
	tyvaluerecord val;
	hdlmenurecord hm;
	hdlexternalhandle h;
	hdlheadrecord hsummit;
	bigstring bsaddress;
	boolean fl = false;
	long menusize = 0;

	if (!oppopupgetaddresscallback (&valaddress))
		return (false);

	if (valaddress.valuetype != addressvaluetype)
		goto exit;

	if (!getaddressvalue (valaddress, &htable, bsaddress))
		goto exit;

	if (!langsymbolreference (htable, bsaddress, &val, &hnode))
		goto exit;

	if ((val.valuetype) != externalvaluetype)
		goto exit;

	h = (hdlexternalhandle) val.data.externalvalue;

	if (!menuverbgetsize (h, &menusize)) /*7.1b43 PBS: We don't care about the size. We just want to read it into memory.*/
		goto exit;

	if ((**h).id != idmenuprocessor) /*it must be a menu*/
		goto exit;

	hm = (hdlmenurecord) (**h).variabledata;

	oppopupmenubar = hm;

	hsummit = (**(**hm).menuoutline).hsummit;

	opbuildpopupmenu (hsummit, hmenu);

	fl = true;
	
	exit:

	disposevaluerecord (valaddress, false);

	*checkeditem = -1; /*Get rid of random checked item bug*/

	return (fl);
	} /*opfillpopup*/


static boolean oppopupselect (hdlmenu hmenu, short itemselected) {
	
	/*
	Run a popup menu command.

	7.0b26 PBS: on Windows, don't run the menu item in a new thread. When two right-click
	menu items run at the same time as the result of rapid right-clicking, something gets
	stepped on and the app crashes. So, instead, allow only one command to run at a time.

	7.0b27 PBS: make sure menu appears for a minimum amount of ticks on Windows, so you
	don't get the case where you choose a menu item without even seeing the menu.
	*/
	
	#pragma unused(hmenu)
	
	hdlmenurecord hm = oppopupmenubar;
	hdlheadrecord hsummit;
	hdlheadrecord hmenuitem;

	if (oppopupmenubar == nil)
		return (false);

	#ifdef WIN95VERSION /*7.0b27 PBS: enforce minimum menu display time.*/

		if ((gettickcount () - popupticksminimum) < popupstartticks)
			return (false);

	#endif

	oppopupmenubar = nil;

	hsummit = (**(**hm).menuoutline).hsummit; /*Top level of the menu*/
	
	//hmenuitem = opnthsubhead (hsummit, itemselected);

	hmenuitem = oprepeatedbump (flatdown, itemselected, hsummit, false);

	oppushoutline ((**hm).menuoutline);

	meuserselected (hmenuitem);

	oppopoutline ();

	return (true);
	} /*oppopupselect*/


boolean oprmousedown (Point pt, tyclickflags flags) {
	
	/*
	7.0b10 PBS: handle right-clicks in an outline.
	Possibly display and track a menu.
	*/
	
	#pragma unused(flags)
	
	register hdloutlinerecord ho = outlinedata;
	Rect linerect;
	hdlheadrecord hcursor;
	long i;
	
	if (!pointinrect (pt, (**ho).outlinerect))
		return (false);
	
	currmenuid = 0;
	
	oppopupresetmenustack (); /*7.1b23 PBS: maintain a stack of submenus*/

	/*determine the line number and rectangle for the mouse click*/ {
	
	long screenlines = opgetcurrentscreenlines (false) + 1;
		
	for (i = 0; i < screenlines; i++) {
			
		opgetlinerect (i, &linerect);
			
		if (pt.v <= linerect.bottom) /*we've reached the right line*/
			goto L2;
		} /*for*/
					
	return (true);;
	}
	
	L2:
	
	hcursor = oprepeatedbump (flatdown, i, (**ho).hline1, true);

	opmoveto (hcursor); /*potentially re-position the bar cursor*/ 
		
	opupdatenow (); /*be sure any changes are visible now*/

	if (!pointinrect (pt, linerect)) /*Must be right-click on a line*/
		return (false);

	linerect.left = pt.h - 5; /*relative to cursor*/

	#ifdef WIN95VERSION /*7.0b26 PBS: wait for the mouse to come back up -- Windows UI standard*/

		while (rightmousestilldown ()) {

			true;
			} /*while*/

		popupstartticks = gettickcount ();
	#endif
	
	popupmenuhit (linerect, false, &opfillpopup, &oppopupselect);
	
	oppopupdisposemenusinstack (); /*7.1b23 PBS: delete and dispose submenus*/
	
	return (true);
	} /*oprmousedown*/