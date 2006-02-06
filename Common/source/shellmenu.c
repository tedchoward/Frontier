
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

#ifdef MACVERSION
	#include <uisharing.h>
#endif

#include "memory.h"
#include "cursor.h"
#include "dialogs.h"
#include "font.h"
#include "menu.h"
#include "resources.h"
#include "sounds.h"
#include "strings.h"
#include "popup.h"
#include "quickdraw.h"
#include "frontierwindows.h"
#include "kb.h"
#include "about.h"
#include "shell.h"
#include "shell.rsrc.h"
#include "shellhooks.h"
#include "shellmenu.h"
#include "shellprint.h"
#include "shellprivate.h"
#include "cancoon.h"
#include "tablestructure.h"
#include "ops.h" /* 2005-09-25 creedon */
#include "langinternal.h" /* 2005-12-27 creedon */


tymenuinfo menustack [ctmenustack];

short topmenustack = -1;

static hdlmenu happlemenu;

static tymenustate menustate = dirtymenus;


#ifdef MACVERSION

static boolean parammenuitem (hdlmenu hmenu, short item) {
	
	/*
	in the indicated menu item, replace all occurrences of ^0 with the
	parameter string.
	*/
	
	register hdlmenu h = hmenu; 
	bigstring bsitem;
	
	getmenuitem (h, item, bsitem); 
	
	parseparamstring (bsitem);
	
	setmenuitem (h, item, bsitem);
	
	return (true);
	} /*parammenuitem*/

#endif
	
	
static boolean menudisablevisit (hdlmenu hmenu, short item) {
	
	if (hmenu != happlemenu) /*apple menu always fully enabled*/
		disablemenuitem (hmenu, item);
	
	return (true);
	} /*menudisablevisit*/


static boolean visitonemenu (short idmenu, boolean (*visitproc) (hdlmenu, short)) {
	
	register hdlmenu hmenu;
	register short i;
	register short lastitem;
	
	hmenu = shellmenuhandle (idmenu);
	
	if (hmenu == nil)
		return (false);
	
	lastitem = countmenuitems (hmenu);
	
	for (i = 1; i <= lastitem; i++) {
		
		if (!(*visitproc) (hmenu, i))
			return (false);
		} /*for*/

	return (true);
	} /*visitonemenu*/


static boolean visitmenuitems (boolean (*visitproc) (hdlmenu, short)) {
	
	register short lastmenu = topmenustack;
	register short i, j;
	register short lastitem;
	register hdlmenu hmenu;
	
	for (i = 0; i <= lastmenu; i++) {
		
		hmenu = menustack [i].macmenu;
		
		lastitem = countmenuitems (hmenu);
		
		for (j = 1; j <= lastitem; j++) {
			
			if (!(*visitproc) (hmenu, j))
				return (false);
			} /*for*/
		} /*for*/
		
	return (true);
	} /*visitmenuitems*/


#if 0 //TARGET_API_MAC_CARBON == 1

static void setfontmenustyles (void) {

	/*
	7.0b47 PBS: set font styles for the items in the Font menu.
	*/

	short i, lastitem, idfont;
	hdlmenu hfontmenu;
	bigstring bsitem;
	
	hfontmenu = shellmenuhandle (fontmenu);
	
	lastitem = countmenuitems (hfontmenu);
	
	for (i = 1; i <= lastitem; i++) {
	
		getmenuitem (hfontmenu, i, bsitem);
		
		fontgetnumber (bsitem, &idfont);
	
		SetMenuItemFontID (hfontmenu, i, idfont);	
		} /*for*/

	} /*setfontmenustyles*/
	
#endif


hdlmenu shellmenuhandle (short idmenu) {

	short i;
	register short lastmenu = topmenustack;

	for (i = 0; i <= lastmenu; i++) {
		
		if (menustack [i].idmenu == idmenu)
			return (menustack [i].macmenu);
		}

	return (nil);
	} /*shellmenuhandle*/


boolean shelltgetmainmenu (bigstring bsmenu, hdlmenu *hmenu, short *idmenu) {
	
	/*
	5.0a24 dmb: find a main menu with the given title
	*/

	short ctmainmenus = (lastmainmenu - firstmainmenu) / mainmenuincrement + 1;
	short ixmenu;
	
	for (ixmenu = 0; ixmenu < ctmainmenus; ++ixmenu) {
		
		register hdlmenu h = menustack [ixmenu].macmenu;
		bigstring bstitle;
		
		getmenuitem (h, 0, bstitle);
		
		popleadingchars (bstitle, (char) '&'); /*pop off keyboard accelerator mark*/
		
		if (equalstrings (bstitle, bsmenu)) {
			
			*hmenu = h;
			
			*idmenu = menustack [ixmenu].idmenu;
			
			return (true);
			}
		}
	
	#ifdef MACVERSION
		if (equalstrings (bsmenu, "\pHelp")) {
			
			#if TARGET_API_MAC_CARBON == 1
			//Code change by Timothy Paustian Friday, June 16, 2000 3:04:41 PM
			//Changed to Opaque call for Carbon
			//we will add the below code when this is implemented in carbon.
			{
				MenuItemIndex	theIndex;
				if (HMGetHelpMenu(hmenu, &theIndex) == noErr && *hmenu != nil) {
					
					*idmenu = kHMHelpMenuID;
					
					return (true);
					}
				}
			#else
			
				if (HMGetHelpMenuHandle (hmenu) == noErr && *hmenu != nil) {
					
					*idmenu = kHMHelpMenuID;
					
					return (true);
					}
				
			#endif
		}
	#endif

	return (false);
	} /*shelltgetmainmenu*/


static boolean pushmenustack (short idmenu, hdlmenu hmenu) {
	
	register short top = topmenustack + 1;
	
	if (top >= ctmenustack) 
		return (false);
	
	menustack [top].idmenu = idmenu;

	menustack [top].macmenu = hmenu;
	
	topmenustack = top;
	
	return (true);
	} /*pushmenustack*/
	
	
static boolean installmenu (short idmenu) {
	
	register hdlmenu h;
	
	h = getresourcemenu (idmenu); 
	
	if (h == nil) /*error in the menu manager*/
		return (false);
	
	if (!pushmenustack (idmenu, h))
		return (false);
		
	insertmenu (h, insertatend);
	
	return (true);
	} /*installmenu*/


static boolean installhierarchicmenu (short idmenu) {
	
	register hdlmenu h;
	
	h = getresourcemenu (idmenu);
	
	if (h == nil) /*error in the menu manager*/
		return (false);
	
	if (!pushmenustack (idmenu, h))
		return (false);
	
	inserthierarchicmenu (h, idmenu); /*insert it as a hierarchic menu*/
		
	return (true);
	} /*installhierarchicmenu*/


static boolean installresitems (short idmenu, OSType restype) {
	
	/*
	2/6/91 dmb: HierDA leaves ResError set after AddResMenu, so 
	we no longer check the result.  we probably wouldn't want to 
	treat that as a fatal error anyway.
	*/
	
	register hdlmenu h;
	
	h = shellmenuhandle (idmenu);
	
	if (h == nil) /*error in the menu manager*/
		return (false);
	
	pushresourcemenuitems (h, idmenu, restype); 
	
	return (true);
	} /*installresitems*/


boolean shellinitmenus (void) {
	
	register short idmenu;
	
	#ifdef MACVERSION
		bigstring bsprogramname; /*PBS 7.1b4: use ifdef because this variable isn't used on Windows.*/
	#endif
	
	topmenustack = -1; /*no items on the menu stack*/
	
	clearbytes (&menustack, sizeof (menustack)); /*clear it for neatness sake*/
	
	for (idmenu = firstmainmenu;  idmenu <= lastmainmenu;  idmenu += mainmenuincrement)
		if (!installmenu (idmenu))
			return (false);
		
	for (idmenu = firsthiermenu;  idmenu <= lasthiermenu;  idmenu += hiermenuincrement)
		if (!installhierarchicmenu (idmenu))
			return (false);
	
	happlemenu = shellmenuhandle (applemenu); /*set global*/

#ifdef MACVERSION
	if (!installresitems (applemenu, 'DRVR'))
		return (false);
#endif
	
	if (!installresitems (fontmenu, 'FONT'))
		return (false);
	
	//#if TARGET_API_MAC_CARBON == 1 /*PBS 7.1b15: too slow to open that menu.*/
	
		//setfontmenustyles (); /*PBS 7.0b46: wizzy font menu*/
		
	//#endif
	
#ifdef MACVERSION
	
	getprogramname (bsprogramname);
	
	setparseparams (bsprogramname, nil, nil, nil);
	
	visitmenuitems (&parammenuitem); /*perform ^0, ^1... substitutions*/

#endif

	visitmenuitems (&menudisablevisit); /*disable all menu items*/
	
	return (true);
	} /*shellinitmenus*/
	
	
void shellgetlastmenuid (short *id) {
	
	/*
	return the menu id of the last menu we insert into the menubar.
	*/
	
	*id = editmenu;
	} /*shellgetlastmenuid*/
	
#ifdef MACVERSION
boolean shellapplemenu (bigstring bsname) {
	
	/*
	can be called from one of the clients.  also there's a verb in the language
	that supports this.
	
	by convention, DA names may or may not begin with one or more nulls.  we take
	this cultural wierdness into account.
	
	return true if we got to call OpenDeskAcc, false if there was some problem we
	were able to detect.
	*/
	
	bigstring bs, bsorig, bscompare;
	short i, ct;
	
	copystring (bsname, bs); /*work with a copy*/
	
	if (isemptystring (bs)) /*nothing to do*/
		return (false);
	
	popleadingchars (bs, (char) 0); /*pop off any leading nulls*/
	
	ct = countmenuitems (happlemenu);
	
	for (i = 1; i <= ct; i++) {
		
		getmenuitem (happlemenu, i, bsorig);
		
		copystring (bsorig, bscompare);
		
		popleadingchars (bscompare, (char) 0); /*pop off any leading nulls*/
		
		if (equalstrings (bs, bscompare)) { /*strings match without any nulls*/
			
			pushstyle (systemFont, 12, 0);
			//Code change by Timothy Paustian Friday, June 16, 2000 3:01:02 PM
			//Changed to Opaque call for Carbon
			//we do not need to do this for carbon
			#if !TARGET_API_MAC_CARBON
			OpenDeskAcc (bsorig); /*use the string as the menu manager provided it*/
			#endif

			popstyle ();    
			
			return (true);  
			}
		} /*for*/
	
	return (false); /*no item with a matching name*/
	} /*shellapplemenu*/
#endif	

boolean shelleditcommand (tyeditcommand editcmd) {

	/*
	7.0b33 PBS: For some reason the HTML Control background on Windows version
	of Radio messes with the kernel's notion of when an editing window is in
	front. This can cause a crash. So a bit of defensive code has been placed here.
	Having spent hours on this bug, I'm going with the defensive code until I can get Bob's help.
	However, I'm limiting the defensive code to Radio/Win only.
	*/
	
	register boolean fl = false;

	#ifdef PIKE
		#ifdef WIN95VERSION

			if (shellwindow == nil) /*7.0b33 PBS: defensive code for Radio/Win to prevent a crash.*/

				return (false);

		#endif
	#endif
	
	switch (editcmd) {
		
		case undocommand:
			fl = (*shellglobals.undoroutine) ();
			
			break;
		
		case cutcommand:
			fl = (*shellglobals.cutroutine) ();
			
			break;
			
		case copycommand:
			fl = (*shellglobals.copyroutine) ();
			
			break;
			
		case pastecommand:
			
			shellreadscrap (); /*since we don't display scrap, now's the time to get current*/
			
			fl = (*shellglobals.pasteroutine) ();
			
			break;
			
		case clearcommand:
			fl = (*shellglobals.clearroutine) ();
			
			break;
		
		case selectallcommand:
			fl = (*shellglobals.selectallroutine) ();
		
		default:
			return (false);
		} /*switch*/
	
	return (true);
	} /*shelleditcommand*/
	

static boolean shellfontmenuchecker (hdlmenu hmenu, short itemnumber) {

	/*
	11/8/90 DW: turns out we CAN ligitimately check more than one font, if there
	is a conflict in font numbers.  the solution -- the user must straighten out
	his font resources, possibly using a font harmonzier so that there are not
	font number conflicts.
	*/
	
	register boolean fl;
	bigstring bs;
	short fontnum;
	
	getmenuitem (hmenu, itemnumber, bs);
	
	fontgetnumber (bs, &fontnum);
	
	fl = (fontnum == (**shellwindowinfo).selectioninfo.fontnum);
	
	checkmenuitem (hmenu, itemnumber, fl);
		
	return (true);
	} /*shellfontmenuchecker*/
	
	
static boolean shellsizemenuchecker (hdlmenu hmenu, short itemnumber) {
	
	register short checkeditem;
	register short fontsize;
	register short itemsize;
	register short style;
	register short fontnum;
	
	fontsize = (**shellwindowinfo).selectioninfo.fontsize;
	
	switch (fontsize) {
		
		case -1: /*no consistent size across selection*/
			checkmenuitem (hmenu, itemnumber, false);
			
			goto L1; /*skip to determining if it's a real font or not*/
		
		case 9:
			checkeditem = point9item;
			
			break;
			
		case 10:
			checkeditem = point10item;
			
			break;
			
		case 12:
			checkeditem = point12item;
			
			break;
			
		case 14:
			checkeditem = point14item;
			
			break;
			
		case 18:
			checkeditem = point18item;
			
			break;
			
		case 24:
			checkeditem = point24item;
			
			break;
			
		default:
			checkeditem = pointcustomitem;
			
			break;
		} /*switch*/
	
	checkmenuitem (hmenu, itemnumber, itemnumber == checkeditem);
	
	L1:
	
	fontnum = (**shellwindowinfo).selectioninfo.fontnum;
	
	if (fontnum == -1) { /*no consistent font across selection*/
		
		stylemenuitem (hmenu, itemnumber, 0); /*plain*/
		
		return (true);
		}
	
	itemsize = -1;
	
	switch (itemnumber) {
		
		case point9item:
			itemsize = 9;
			
			break;
			
		case point10item:
			itemsize = 10;
			
			break;
			
		case point12item:
			itemsize = 12;
			
			break;
			
		case point14item:
			itemsize = 14;
			
			break;
			
		case point18item:
			itemsize = 18;
			
			break;
			
		case point24item:
			itemsize = 24;
			
			break;
			
		} /*switch*/
	
	style = 0;
	
	if (itemsize != -1)
		if (realfont (fontnum, itemsize))
			style = outline;
		
	stylemenuitem (hmenu, itemnumber, style);
		
	return (true);
	} /*shellsizemenuchecker*/
	
	
static boolean shellstylemenuchecker (hdlmenu hmenu, short itemnumber) {
	
	register boolean flchecked = false;
	tyselectioninfo x;
	
	x = (**shellwindowinfo).selectioninfo;
	
	switch (itemnumber) {
		
		case plainitem:
			flchecked = x.fontstyle == 0; /*flplain;*/
			
			break;
			
		case bolditem:
			flchecked = (x.fontstyle & bold) != 0;
			
			break;
			
		case italicitem:
			flchecked = (x.fontstyle & italic) != 0;
			
			break;
			
		case underlineitem:
			flchecked = (x.fontstyle & underline) != 0;
			
			break;
			
		case outlineitem:
			flchecked = (x.fontstyle & outline) != 0;
			
			break;
			
		case shadowitem:
			flchecked = (x.fontstyle & shadow) != 0;
			
			break;

		/*
		case condenseditem:
			flchecked = x.fontstyle.flcondensed;
			
			break;
		
		case superscriptitem:
			flchecked = x.fontstyle.flsuperscript;
			
			break;
			
		case subscriptitem:
			flchecked = x.fontstyle.flsubscript;
			
			break;
		*/	
		} /*switch*/
	
	checkmenuitem (hmenu, itemnumber, flchecked);
	
	return (true);
	} /*shellstylemenuchecker*/
	

static boolean shellleadingmenuchecker (hdlmenu hmenu, short itemnumber) {
	
	register short leading = (**shellwindowinfo).selectioninfo.leading;
	register short checkeditem = -1;
	
	switch (leading) {
		
		case -1: /*no consistent leading across selection*/
			checkmenuitem (hmenu, itemnumber, false);
			
			return (true);
		
		case 0:
			checkeditem = leading0item;
			
			break;
		
		case 1:
			checkeditem = leading1item;
			
			break;
			
		case 2:
			checkeditem = leading2item;
			
			break;
			
		case 3:
			checkeditem = leading3item;
			
			break;
			
		case 4:
			checkeditem = leading4item;
			
			break;
			
		case 5:
			checkeditem = leading5item;
			
			break;
			
		default:
			checkeditem = leadingcustomitem;
			
			break;
		} /*switch*/
	
	checkmenuitem (hmenu, itemnumber, itemnumber == checkeditem);
	
	return (true);
	} /*shellleadingmenuchecker*/


static boolean shelljustifymenuchecker (hdlmenu hmenu, short itemnumber) {
	
	register tyjustification justification = (**shellwindowinfo).selectioninfo.justification;
	register short checkeditem = -1;
	
	switch (justification) {
		
		case leftjustified:
			checkeditem = leftjustifyitem;
			
			break;
			
		case centerjustified:
			checkeditem = centerjustifyitem;
			
			break;
			
		case rightjustified:
			checkeditem = rightjustifyitem;
			
			break;
			
		case fulljustified:
			checkeditem = fulljustifyitem;
			
			break;
			
		case unknownjustification:
			checkmenuitem (hmenu, itemnumber, false);
			
			return (true);
			
		} /*switch*/
	
	checkmenuitem (hmenu, itemnumber, itemnumber == checkeditem);
	
	return (true);
	} /*shelljustifymenuchecker*/


static void shellcheckfontsizestyle (void) {
	
	register hdlwindowinfo hw = shellwindowinfo;
	tyselectioninfo x;
	
	/*blidgey ();*/
	
	if (hw == nil) /*no windows open*/
		return;
	
	x = (**hw).selectioninfo;
	
	if (!x.fldirty) /*nothing to do*/
		return;
	
	shellsetselectioninfo ();
	
	x = (**hw).selectioninfo; /*get updated flags*/
	
	if (x.flcansetfont)
		visitonemenu (fontmenu, &shellfontmenuchecker);
	
	if (x.flcansetsize)
		visitonemenu (sizemenu, &shellsizemenuchecker);
	
	if (x.flcansetstyle)
		visitonemenu (stylemenu, &shellstylemenuchecker);
	
	if (x.flcansetleading)
		visitonemenu (leadingmenu, &shellleadingmenuchecker);
	
	if (x.flcansetjust)
		visitonemenu (justifymenu, &shelljustifymenuchecker);
	} /*shellcheckfontsizestyle*/


void shelladjustundo (void) {
	
	register hdlstring hstring = nil;
	register boolean flundoable = false;
	register hdlmenu hmenu;
	bigstring bs;
	
	if (shellwindow != nil) { /*there's at least one window open*/
	
		(*shellglobals.setundostatusroutine) ();
	
		hstring = (**shellwindowinfo).hundostring;
	
		flundoable = hstring != nil;
		}
	
	if (!flundoable) 
		getstringlist (undolistnumber, cantundoitem, bs);
	else
		copyheapstring (hstring, bs);
	
	hmenu = shellmenuhandle (editmenu);
	
	if (hmenu != nil) {
		
		setmenuitem (hmenu, undoitem, bs);
	
		setmenuitemenable (hmenu, undoitem, flundoable);
		}
	} /*shelladjustundo*/


void shellforcemenuadjust (void) {
	
	menustate = dirtymenus;
	} /*shellforcemenuadjust*/


void shellmodaldialogmenuadjust (void) {
	
	menustate = modaldialogmenus;
	
	shelladjustmenus ();
	} /*shellforcemenuadjust*/


static boolean shellsetmenuitemstring (hdlmenu hmenu, short ixmenu, short ixitemstring) {
	
	bigstring bs;
	
	return (shellgetstring (ixitemstring, bs) && setmenuitem (hmenu, ixmenu, bs));
	} /*shellsetmenuitemstring*/


#ifdef PIKE

static void pikesetfilemenuitemchecked (short ixmenu) {
	
	/*
	7.0b25 PBS: Run a Radio UserLand script that returns true if the item should get
	a check mark. If true, put a check next to the item.

	7.1b4 PBS: Get script from resource, don't hard-code.
	*/
	
	bigstring bsscript, bsitem, bsresult;
	
	if (roottable == nil)
		return;
	
	getfilemenuitemidentifier (ixmenu, bsitem);
	
	/*copystring ("\x20" "pike.isFileMenuItemChecked(\"^0\")", bsscript);*/

	getsystemtablescript (idpikeisfilemenuitemcheckedscript, bsscript);
	
	parsedialogstring (bsscript, bsitem, nil, nil, nil, bsscript);

	grabthreadglobals ();
	
	langrunstringnoerror (bsscript, bsresult);
	
	releasethreadglobals ();

	checkmenuitem (shellmenuhandle (filemenu), ixmenu, equalstrings (bsresult, bstrue));
	} /*pikesetfilemenuitemchecked*/


static void pikesetfilemenuitemenable (short ixmenu) {

	/*
	6.2a2 AR: Call the pike.isFileMenuItemEnabled script to determine whether
	the menu item should be enabled or disabled.

	7.1b4: Get script from resource, don't hard-code it.
	*/

	bigstring bsscript, bsitem, bsresult;

	if (roottable == nil)
		return;

	getfilemenuitemidentifier (ixmenu, bsitem);

	/*copystring ("\x20""pike.isFileMenuItemEnabled(\"^0\")", bsscript);*/

	getsystemtablescript (idpikeisfilemenuitemenabledscript, bsscript);

	parsedialogstring (bsscript, bsitem, nil, nil, nil, bsscript);

	grabthreadglobals ();
	
	langrunstringnoerror (bsscript, bsresult);
	
	releasethreadglobals ();
	
	setmenuitemenable (shellmenuhandle (filemenu), ixmenu, equalstrings (bsresult, bstrue));
	}/*ccpikesetfilemenuitemenable*/


static void pikesetfilemenuitemname (short ixmenu) {

	/*
	6.2a2 AR: Call the pike.getFileMenuItemName script to determine the name
	of the displayed name of the menu item.
	*/

	bigstring bsscript, bsitem, bsresult;

	if (roottable == nil)
		return;

	getfilemenuitemidentifier (ixmenu, bsitem);

	copystring ("\x1e""pike.getFileMenuItemName(\"^0\")", bsscript);

	parsedialogstring (bsscript, bsitem, nil, nil, nil, bsscript);

	grabthreadglobals ();
	
	langrunstringnoerror (bsscript, bsresult);

	releasethreadglobals ();
	
	setmenuitem (shellmenuhandle (filemenu), ixmenu, bsresult);
	}/*pikesetfilemenuitemname*/



boolean pikequit () {

	/*
	7.0 PBS: Called in Windows when the user clicks the X in the frame window.

	7.1b4 PBS: get script string from resource, don't hard-code.
	*/

	bigstring bsscript, bsitem, bsresult;

	if (roottable == nil)
		return (true);

	getfilemenuitemidentifier (quititem, bsitem);

	getsystemtablescript (idrunfilemenuscript, bsscript); /*7.1b4: get from resource.*/

	/*copystring ("\x1c""pike.runFileMenuScript(\"^0\")", bsscript);*/

	parsedialogstring (bsscript, bsitem, nil, nil, nil, bsscript);

	grabthreadglobals ();
	
	langrunstringnoerror (bsscript, bsresult);

	releasethreadglobals ();

	return (bsresult == bsfalse);
	} /*pikequit*/

#endif


void shelladjustmenus (void) {
	
	/*
	2005-10-26 creedon: the file menu item that provides save as functionality now reads Save As... for database or file-object save for Frontier
	
	2005-09-25 creedon: added open recent menu
					 changed to support calling script for some file/edit menu commands on all targets
	
	8/1/90 dmb: call menuhooks with menu & item set to zero to give 
	hooks a chance to update their menus
	
	2/22/91 dmb: use new flcansetxxx fields in selectioninfo to handle enabling 
	of the corresponding submenus
	
	6.19.97 dmb: added spaghetti code for modal dialog menus.
	
	5.0d19 dmb: added save/save database toggle, changed enabling logic to experts
	
	5.0.2b6 dmb: when setting modaldialog menus, set the window menu dirty

	7.0b32 PBS: Handle Windows case when to the user there are no windows open,
	but actually the main root window is open but hidden. In this case it should act
	as if no windows are open.
	*/
	
	register hdlmenu hmenu;
	register WindowPtr w = shellwindow;
	boolean flwindow = w != nil;
	boolean flchanges;
	boolean flanywindow = (getfrontwindow () != nil);
	hdlwindowinfo hrootinfo = nil;
	tyselectioninfo x;

#ifndef PIKE
	Handle hdata;
#endif

	/*7.0b32 PBS: if shellwindow is root window but it's hidden, act as
	if no windows are open -- because, to the user, no windows *are* open.*/

	if (shellwindowinfo == NULL) {

		flwindow = false;

		flanywindow = false;
		} /*if*/

	else {

		if ((**shellwindowinfo).configresnum == idcancoonconfig) {
			
			if ((**shellwindowinfo).flhidden) {

				flwindow = false;

				flanywindow = false;
				} /*if*/
			} /*if*/
		} /*else*/

	
	if (menustate == modaldialogmenus) {
		
		visitmenuitems (&menudisablevisit); // disable all menu items
		
		shellwindowmenudirty (); // make sure it gets updated later
		
		goto L1;  // do the edit menu
		}
	
	if (flwindow) {
		
		getrootwindow (w, &hrootinfo);
		
		shellcheckfontsizestyle (); /*update checks on font/size/style menus*/
		
		x = (**shellwindowinfo).selectioninfo;
		}
	else {
		if (ccinexpertmode ())
			ccfindrootwindow (&hrootinfo);
		
		clearbytes (&x, sizeof (tyselectioninfo));
		}
		
	flchanges = hrootinfo && (**hrootinfo).flmadechanges;
	
	hmenu = shellmenuhandle (filemenu);
	
	if (hmenu == nil) /*skip file menu adjusting*/
		goto L1;	

#ifdef PIKE

/*7.0b1 PBS: Radio UserLand has a functioning About item in the Apple menu.*/

/*#ifdef MACVERSION
	disablemenuitem (happlemenu, aboutitem); //disable this command until we have a splash screen
#endif*/

	/*
	6.2a2 AR: For Pike, we want to manage the state of the items
	in the File menu by calling a UserTalk script.

	7.0d6 PBS: Pike's File menu has changed. It's more like a standard
	File menu. The names, number, and order of items have changed.

	7.0d10 PBS: Radio UserLand now has Update Radio.root... in the File menu.
	*/

	enablemenuitem (hmenu, newitem);

	enablemenuitem (hmenu, openitem);
	
	pikesetfilemenuitemenable (openurlitem); /*7.0b17 PBS: enable/disable Open URL... menu item.*/
	
#ifndef OPMLEDITOR
	pikesetfilemenuitemenable (openmanilasiteitem); /*7.0b27 PBS: enable/disable Open Manila Site item.*/
#endif // OPMLEDITOR		
	pikesetfilemenuitemenable (closeitem);
		
	pikesetfilemenuitemenable (saveitem);

	pikesetfilemenuitemenable (saveasitem);
	
#ifndef OPMLEDITOR
	pikesetfilemenuitemenable (saveashtmlitem); /*7.0b32 PBS: Save As HTML*/
	
	pikesetfilemenuitemenable (saveasplaintextitem); /*7.0b32 PBS: Save As Plain Text*/
#endif // OPMLEDITOR
		
	pikesetfilemenuitemenable (revertitem);

	pikesetfilemenuitemenable (viewinbrowseritem);

	pikesetfilemenuitemenable (updateradiorootitem); /*7.0d10 PBS*/
	
	pikesetfilemenuitemenable (workofflineitem); /*7.0b25 PBS*/
	
	pikesetfilemenuitemchecked (workofflineitem); /*7.0b25 PBS*/

#else

	enablemenuitem (hmenu, newitem);
	
	enablemenuitem (hmenu, openitem);
	
#ifndef OPMLEDITOR
	enablemenuitem (hmenu, openrecentitem);
#endif // OPMLEDITOR
	
	setmenuitemenable (hmenu, closeitem, flwindow);

	/*3/30/90 DW -- saveitem is not dependent on flchanges.  this allows you to save
	even when there have been no changes -- needed because changes to the symbol table
	no longer dirty the window it lives in.
	
	11/8/90 DW -- it's nice to be able to save even if we haven't made any changes.
	
	setmenuitemenable (hmenu, saveitem, flchanges);
	
	5.0a18 dmb: only enable save runnable for scripts
	*/

	setmenuitemenable (hmenu, saveitem, flwindow || hrootinfo);

	if ((hrootinfo != nil) && (!flwindow || (**hrootinfo).configresnum == iddefaultconfig)) {
		
		shellsetmenuitemstring (hmenu, saveitem, savedatabaseitemstring); // "Save Database");
		
		// shellsetmenuitemstring (hmenu, saveasitem, saveacopyitemstring); // "Save a CopyÉ");
		}
	else {
		shellsetmenuitemstring (hmenu, saveitem, saveitemstring); // "\x07" "Save");
		
		// shellsetmenuitemstring (hmenu, saveasitem, saveasitemstring); // "\x07" "Save AsÉ");
		}
	
	setmenuitemenable (hmenu, saveasitem, flwindow || hrootinfo);
	
	setmenuitemenable (hmenu, saverunnableitem, 
		flwindow && 
		shellgetexternaldata (shellwindowinfo, &hdata) && 
		(**(hdlexternalhandle) hdata).id == idscriptprocessor);
	
	setmenuitemenable (hmenu, revertitem, flchanges && ((**hrootinfo).fnum != 0) && !(**hrootinfo).flneversaved);
	
	setmenuitemenable (hmenu, pagesetupitem, flwindow);
	
	setmenuitemenable (hmenu, printitem, flwindow);

#endif
	
	enablemenuitem (hmenu, quititem);
	
	L1: /*goto here to skip the adjustment of the file menu*/
	
	hmenu = shellmenuhandle (editmenu);
	
	if (hmenu == nil) /*skip edit menu adjusting*/
		goto L2;
	
	setmenuitemenable (hmenu, undoitem, flanywindow);
	
	setmenuitemenable (hmenu, cutitem, flanywindow);
	
	setmenuitemenable (hmenu, copyitem, flanywindow);
	
	setmenuitemenable (hmenu, pasteitem, flanywindow);
	
	setmenuitemenable (hmenu, clearitem, flanywindow);
	
	setmenuitemenable (hmenu, selectallitem, flwindow);
	
	setmenuitemenable (hmenu, insertdatetimeitem, flwindow); /* 2006-01-06 creedon: re-establish code, went missing in sf repository revision 1.9 */
	
#ifdef PIKE
#ifndef OPMLEDITOR
	setmenuitemenable (hmenu, opennotepaditem, true); /*7.0b27 PBS: enable/disable Open Notepad, Insert Date/Time.*/
#endif // !OPMLEDITOR	
#endif
	
	if (menustate == modaldialogmenus)
		return;
	
	/*
	setmenuitemenable (hmenu, windowsitem, flanywindow);
	*/

	setmenuitemenable (hmenu, findandreplaceitem, flwindow);

	setmenuitemenable (hmenu, commonstylesitem, (x.flcansetfont && x.flcansetsize));

	setmenuitemenable (hmenu, fontitem, x.flcansetfont);
	
	setmenuitemenable (hmenu, sizeitem, x.flcansetsize);
	
#ifndef PIKE

	setmenuitemenable (hmenu, styleitem, x.flcansetstyle);
	
	setmenuitemenable (hmenu, leadingitem, x.flcansetleading);
	
	setmenuitemenable (hmenu, justifyitem, x.flcansetjust);

#endif

	hmenu = shellmenuhandle (findandreplacemenu); /*Find and Replace sub-menu*/
				
	if (hmenu)
		enableallmenuitems (hmenu, flwindow);

	hmenu = shellmenuhandle (commonstylesmenu); /*Common Styles sub-menu*/

	if (hmenu) {

		short i = 0;

		enableallmenuitems (hmenu, (x.flcansetfont && x.flcansetsize));
		
		for (i = 1; i < 5; i++) {

			bigstring bsitem;

			setemptystring (bsitem);

			if (flwindow)

				getcommonstylesmenuitemtext (i, bsitem);
			
			setmenuitem (hmenu, i, bsitem); /*Set item text.*/
			} /*for*/

		} /*if*/

	L2: /*goto here to skip the adjustment of the edit menu*/
	
	if (!x.flcansetsize)
		goto L3;

	hmenu = shellmenuhandle (sizemenu);
	
	if (hmenu == nil) /*skip edit menu adjusting*/
		goto L3;
		
	enablemenuitem (hmenu, point9item);
	
	enablemenuitem (hmenu, point10item);
	
	enablemenuitem (hmenu, point12item);
	
	enablemenuitem (hmenu, point14item);
	
	enablemenuitem (hmenu, point18item);
	
	enablemenuitem (hmenu, point24item);
	
	enablemenuitem (hmenu, pointupitem);
	
	enablemenuitem (hmenu, pointdownitem);
	
	enablemenuitem (hmenu, pointcustomitem);
	
	L3: /*goto here to skip the adjustment of the size menu*/
	
	if (!x.flcansetstyle)
		goto L4;

#ifndef PIKE

	hmenu = shellmenuhandle (stylemenu);
	
	if (hmenu == nil) /*skip edit menu adjusting*/
		goto L4;
		
	enablemenuitem (hmenu, plainitem);
	
	enablemenuitem (hmenu, bolditem);
	
	enablemenuitem (hmenu, italicitem);
	
	enablemenuitem (hmenu, underlineitem);
	
	enablemenuitem (hmenu, outlineitem);
	
	enablemenuitem (hmenu, shadowitem);
	
	/*
	enablemenuitem (hmenu, condenseditem);
	*/
	
	enablemenuitem (hmenu, superscriptitem);
	
	enablemenuitem (hmenu, subscriptitem);

#endif	

	L4: /*goto here to skip the adjustment of the style menu*/
	
	if (!x.flcansetleading)
		goto L5;

#ifndef PIKE

	hmenu = shellmenuhandle (leadingmenu);
	
	if (hmenu == nil) /*skip edit menu adjusting*/
		goto L5;
	
	enablemenuitem (hmenu, leading0item);

	enablemenuitem (hmenu, leading1item);

	enablemenuitem (hmenu, leading2item);

	enablemenuitem (hmenu, leading3item);

	enablemenuitem (hmenu, leading4item);

	enablemenuitem (hmenu, leading5item);

	enablemenuitem (hmenu, leadingcustomitem);

#endif

	L5: /*goto here to skip the adjustment of the leading menu*/
	
	if (!x.flcansetjust)
		goto L6;

#ifndef PIKE

	hmenu = shellmenuhandle (justifymenu);
	
	if (hmenu == nil) /*skip edit menu adjusting*/
		goto L6;
		
	enablemenuitem (hmenu, leftjustifyitem);

	enablemenuitem (hmenu, centerjustifyitem);

	enablemenuitem (hmenu, rightjustifyitem);

	enablemenuitem (hmenu, fulljustifyitem);

#endif

	L6: /*goto here to skip the adjustment of the justify menu*/


	if (x.flcansetfont) {
		
		hmenu = shellmenuhandle (fontmenu);
		
		if (hmenu)
			enableallmenuitems (hmenu, flwindow);
		}

#ifndef PIKE

	hmenu = shellmenuhandle (newobjectmenu);
	
	if (hmenu)
		enableallmenuitems (hmenu, true);

	/* 2005-09-25 creedon - open recent menu */
	
	hmenu = shellmenuhandle (openrecentmenu);
	
	if (hmenu)
		enableallmenuitems (hmenu, true);
#endif

	#ifdef WIN95VERSION
		hmenu = shellmenuhandle (helpmenu);
		
		if (hmenu)
			enableallmenuitems (hmenu, true);
	#endif
	
	shelladjustundo ();
	
	shellcallmenuhooks (0, 0); /*let menu hooks update their menu items*/
	} /*shelladjustmenus*/


void shellupdatemenus (void) {
	
	tymenustate newstate;
	
	if (optionkeydown ())
		newstate = optionmenus;
	else
		newstate = normalmenus;
	
	if (menustate != newstate) {
        
	//	grabthreadglobals ();
		
		shelladjustmenus ();
        
	//	releasethreadglobals ();
			
		menustate = newstate;
        }
	
	if (shellwindowinfo && (**shellwindowinfo).selectioninfo.fldirty) /*update style menus*/
		shellcheckfontsizestyle ();
	
	shellupdatewindowmenu ();
	} /*shellupdatemenus*/
	
	
boolean shellhandlemenu (long menucode) {

	/*
	2005-12-31 creedon: for host databases that don't have a Frontier.tools table, provide some of the basic file commands so that folks can work
						with databases
	
	2005-09-25 creedon: changed so that all targets can call scripts for some file/edit menu commands
					 added open recent menu
	
	7.0d6 PBS: With Pike's now-standard File menu, it's necessary to use kernel routines for New and Open. Run scripts in Pike for everything else.

	5/19/93 dmb: closefunc uses getfrontwindow, not shellwindow
	*/
	
	register short idmenu, iditem;
	
	/*
	if there's an open window, it should already be pushed.   we don't want 
	to push it again, since some operations (like close) will dispose 
	of it, and popping globals would then fail
	*/
	
	iditem = LoWord (menucode);
	
	idmenu = HiWord (menucode); 
	
	// grabthreadglobals ();
	
	#ifdef WIN95VERSION
		iditem -= idmenu;

		if (idmenu == 0)
			idmenu = editmenu;
	#endif

	if (iditem == 0)
		goto exitmenu;
    
	if (!shellcallmenuhooks (idmenu, iditem)) /*intercepted by a hook*/
		goto exitmenu;
	
	if (iditem < 0) /*was a disabled item*/
		goto exitmenu;
	
	switch (idmenu) {
   
		#ifdef MACVERSION
		case applemenu: 
			switch (iditem) {
			
				case aboutitem:
					aboutcommand ();
					
					break;
				
				default: {
				
					bigstring bs;
					
					getmenuitem (happlemenu, iditem, bs);
					
					shellapplemenu (bs);
					
					break;
					}
				} /*switch*/
			
			break; /*applemenu*/
		#endif

		#ifdef WIN95VERSION
		case helpmenu: 
			switch (iditem) {
			
				case aboutitem:
					aboutcommand ();
					
					break;
				} /*switch*/
			
			break; /*helpmenu*/
		#endif

		case filemenu: {
		
			boolean flkernelhandledcommand = false, flfoundrootwindow = false;
		
			WindowPtr w = shellwindow;
			Handle hdata;
			hdlwindowinfo hinfo;
			
			if (w == nil) {
				flfoundrootwindow = ccfindrootwindow (&hinfo);
				if (flfoundrootwindow)
					w = (**hinfo).macwindow;
				}

			switch (iditem) {
				
				case closeitem: /* possibly close an Error Info window */
			
					if (shellfindwindow (idlangerrorconfig, &w, &hinfo, &hdata)) { /* is there an Error Info window? */
			
						if (w == getfrontwindow ()) { /* is the Error Info window in front? */
						
							shellclose (w, true); /* close the Error Info window */
			
							flkernelhandledcommand = true; /* the kernel handled the command */
							} /* if */
						} /* if */
			
					break;
			
				case quititem:
					
					if (!flfoundrootwindow) {
						shellquit ();
							
						flkernelhandledcommand = true;
						}
						
					break;
					
				#ifndef PIKE
			
				case newitem:
					shellnew ();
					
					flkernelhandledcommand = true;
				
					break;
			
				case pagesetupitem:
					shellpagesetup ();
					
					flkernelhandledcommand = true;
				
					break;
				
				case printitem:
					shellprint (w, true);
					
					flkernelhandledcommand = true;
				
					break;
			
				case saverunnableitem:
					shellsaveas (w, nil, true);
					
					flkernelhandledcommand = true;
				
					break;
				
				#endif
			
				} /*switch*/
			
			if (!flkernelhandledcommand) { /*Run the script if special cases weren't handled above. */
			
#ifndef PIKE
			
				bigstring bs;
				boolean flrunfilemenuscript = langrunstringnoerror ("\x2C" "Frontier.tools.windowTypes.runFileMenuScript", bs);				
				
				if (flrunfilemenuscript) {
			
#endif
			
					runfilemenuscript (iditem);
					
#ifndef PIKE
					}
				else { /* if the script is not defined then we try to do some menu items without the script */
				
					switch (iditem) {
					
						case openitem:
							shellopen ();
											
							break;
					
						case closeitem:
							if (keyboardstatus.floptionkey)
								shellcloseall (w, true);
							else
								shellclose (getfrontwindow (), true);
							
							break;
						
						case saveitem:
							shellsave (w);
							
							break;
					
						case saveasitem:
							shellsaveas (w, nil, false);
							
							break;
					
						case revertitem:
							shellrevert (w, true);
							
							break;
					
						} /*switch*/
					
					} /* else */
						
#endif
				} /* if(!flkernelhandledcommand) */
			
			break; /* file menu */
			}
		
		case editmenu:
			#if MACVERSION
			
			if (iditem <= clearitem) { /*standard edit menu command*/
				//Code change by Timothy Paustian Friday, June 16, 2000 3:02:01 PM
				//Changed to Opaque call for Carbon
				//we don't need this for carbon.
				#if !TARGET_API_MAC_CARBON
				if (SystemEdit (iditem - 1)) /*consumed by desk accessory*/
					break;
				#endif

				if (uisEdit (iditem - 1)) /*consumed by shared window*/
					break;
				}

			#endif
			
			if (iditem <= selectallitem) {

				shelleditcommand ((tyeditcommand) (iditem - undoitem));
				
				break;
				}
			
			runeditmenuscript (editmenu, iditem); /*7.0b27 PBS */
			
			break; /*edit menu*/
			
		case fontmenu: {
		
			bigstring bs;
			short fontnumber;
			
			getmenuitem (shellmenuhandle (fontmenu), iditem, bs);
			
			fontgetnumber (bs, &fontnumber);
			
			(**shellwindowinfo).selectioninfo.fontnum = fontnumber;
			
			(*shellglobals.fontroutine) ();
			
			break;
			} /*font menu*/
			
		case windowsmenu:
			shellwindowmenuselect (iditem);
			
			break;
			
		case sizemenu: {
			
			short size = 0;
			
			switch (iditem) {
				
				case point9item:
					size = 9;
					
					break;
					
				case point10item:
					size = 10;
					
					break;
					
				case point12item:
					size = 12;
					
					break;
					
				case point14item:
					size = 14;
					
					break;
					
				case point18item:
					size = 18;
					
					break;
					
				case point24item:
					size = 24;
					
					break;
				
				case pointupitem:
					size = (**shellwindowinfo).selectioninfo.fontsize + 1;
					
					break;
					
				case pointdownitem: 
					size = (**shellwindowinfo).selectioninfo.fontsize;
					
					if (size == 1) /*can't get any smaller*/
						ouch ();
					else
						size--;
			
					break;
					
				case pointcustomitem: {
				
					short customsize = (**shellwindowinfo).selectioninfo.fontsize;
					bigstring bs;
					
					shellgetstring (customsizestring, bs);
					
					if (!intdialog (bs, &customsize)) /*user hit Cancel*/
						goto exitmenu;
						
					size = customsize;
					
					break;
					}
					
				} /*switch*/
				
			if (size != 0) {
				
				(**shellwindowinfo).selectioninfo.fontsize = size;
			
				(*shellglobals.sizeroutine) ();
				}
				
			break;
			} /*size menu*/
			
		case stylemenu: {
			
			tyselectioninfo x;
			
			clearbytes (&x, sizeof (x)); /*set all flags to false*/
			
			switch (iditem) {
				
				case plainitem:
					/*
					x.flplain = true;
					*/
					
					break;
					
				case bolditem:
					x.fontstyle |= bold;
					
					break;
					
				case italicitem:
					x.fontstyle |= italic;
					
					break;
					
				case underlineitem:
					x.fontstyle |= underline;
					
					break;
					
				case outlineitem:
					x.fontstyle |= outline;
					
					break;
					
				case shadowitem:
					x.fontstyle |= shadow;
					
					break;
				/*
				case condenseditem:
					x.fontstyle |= condensed;
					
					break;
				
				case superscriptitem:
					x.fontstyle |= superscript;
					
					break;
					
				case subscriptitem:
					x.fontstyle |= subscript;
					
					break;
				*/	
				} /*switch*/
			
			(**shellwindowinfo).selectioninfo = x;
			
			(*shellglobals.styleroutine) ();
			
			break;
			} /*style menu*/
		
		case justifymenu: {
			
			register tyjustification justification = unknownjustification;
			
			switch (iditem) {
				
				case leftjustifyitem:
					justification = leftjustified;
					
					break;
					
				case centerjustifyitem:
					justification = centerjustified;
					
					break;
					
				case rightjustifyitem:
					justification = rightjustified;
					
					break;
					
				case fulljustifyitem:
					justification = fulljustified;
					
					break;
					
				} /*switch*/
				
			if (justification != unknownjustification) {
				
				(**shellwindowinfo).selectioninfo.justification = justification;
			
				(*shellglobals.justifyroutine) ();
				}
				
			break;
			} /*justify menu*/
			
		case leadingmenu: {
			
			register short leading = -1;
			
			switch (iditem) {
				
				case leading0item:
					leading = 0;
					
					break;
					
				case leading1item:
					leading = 1;
					
					break;
					
				case leading2item:
					leading = 2;
					
					break;
					
				case leading3item:
					leading = 3;
					
					break;
					
				case leading4item:
					leading = 4;
					
					break;
					
				case leading5item:
					leading = 5;
					
					break;
					
				case leadingcustomitem:{
				
					short customleading;
					bigstring bs;
					
					shellgetstring (customleadingstring, bs);
					
					leading = (**shellwindowinfo).selectioninfo.leading;
					
					if (leading < 0) /*-1 means there's no consistent leading*/
						leading = 0;
						
					customleading = leading;
					
					if (!intdialog (bs, &customleading)) /*user hit Cancel*/
						goto exitmenu;
						
					leading = customleading;
					
					break;
					}
					
				} /*switch*/
				
			if (leading >= 0) {
				
				leading = min (leading, 144); /*max 2" leading*/
				
				(**shellwindowinfo).selectioninfo.leading = leading;
			
				(*shellglobals.leadingroutine) ();
				}
				
			break;
			} /*leading menu*/
		
			case findandreplacemenu:
			case commonstylesmenu:

				runeditmenuscript (idmenu, iditem);

				break;

		#ifdef MACVERSION
		case virtualmenu: { /*special key on the extended keyboard*/
			
			switch (iditem) {
				
				case helpitem:
					aboutcommand (); /*until we have help...*/
					
					break;
				
				case homeitem:
					(*shellglobals.scrollroutine) (down, false, longinfinity);
					
					break;
				
				case enditem:
					(*shellglobals.scrollroutine) (up, false, longinfinity);
					
					break;
				
				case pageupitem:
					(*shellglobals.scrollroutine) (down, true, 1);
					
					break;
				
				case pagedownitem:
					(*shellglobals.scrollroutine) (up, true, 1);
					
					break;
				}
				
			break;
			} /* virtual menu */

		case openrecentmenu: {
		
			boolean flkernelhandledcommand = false;
		
			if (!flkernelhandledcommand) /* run the script if special cases weren't handled above */

				runopenrecentmenuscript (iditem);
				
			break;
			} /* openrecentmenu */

		#endif

	#ifdef WIN95VERSION
	//	case defaultpopupmenuid:
	//		(*winpopupselectroutine) (NULL, iditem);
	//		break;
	#endif
		} /*switching*/
	
	exitmenu:
	
	hilitemenu (0);
	
//	releasethreadglobals ();

	return (idmenu != 0);
	} /*shellhandlemenu*/


void runfilemenuscript (short ixmenu) {
	
	/*
	2005-09-14 creedon: changed name from pikerunfilemenuscript to runfilemenuscript, all targets can now run a script associated with some of
						the file menu commands
	
	7.1b4 PBS: get script string from resource, don't hard-code.

	6.2a2 AR: Call the pike.runFileMenuScript script to eventually run the script associated with the current menu command.
	*/

	bigstring bsscript, bsitem, bsresult;

	if (roottable == nil)
		return;

	getfilemenuitemidentifier (ixmenu, bsitem);

	getsystemtablescript (idrunfilemenuscript, bsscript); /*7.1b4: get from resource.*/
	
	/*copystring ("\x1c""pike.runFileMenuScript(\"^0\")", bsscript);*/

	parsedialogstring (bsscript, bsitem, nil, nil, nil, bsscript);

	grabthreadglobals ();
	
	langrunstringnoerror (bsscript, bsresult);

	releasethreadglobals ();
	}/* runfilemenuscript */


void getfilemenuitemidentifier (short ixmenu, bigstring bsitem) {

	/*
	2005-09-14 creedon: changed name from pikerunfilemenuscript to runfilemenuscript, all targets can now run a script associated with some of the file menu commands
	
	6.2a2 AR: Convert an index into Pike's File menu into the name of that item for use in a call to builtins.pike scripts.

	7.0d6 PBS: Pike's File menu has changed -- it's more like a standard File menu, with
	New and Open commands, no more What Is This? command.

	7.0d10 PBS: Added case for Radio UserLand's Update Radio.root... command.
	
	7.0b25 PBS: Added case for Radio UserLand's Work Offline command.
	
	7.0b32 PBS: Cases for Save As HTML and Save As Plain Text.
	*/

	switch (ixmenu) {

		case openitem:
			copystring ("\x04""open", bsitem);
			break;
		
		case closeitem:
			copystring ("\x05""close", bsitem);
			break;

		case saveitem:
			copystring ("\x04""save", bsitem);
			break;

		case saveasitem:
			copystring ("\x06""saveas", bsitem);
			break;
		
		case revertitem:
			copystring ("\x06""revert", bsitem);
			break;
		
		case quititem:
			copystring ("\x04""quit", bsitem);
			break;
			
		#ifdef PIKE

		case newitem:
			copystring ("\x03""new", bsitem);
			break;
		
		case openurlitem: /*7.0b17 PBS*/
			copystring ("\x07""openurl", bsitem);
			break;

		#ifndef OPMLEDITOR
			
		case openmanilasiteitem: /*7.0b27 PBS*/
			copystring ("\x0e""openmanilasite", bsitem);
			break;
		
		case saveashtmlitem:
			copystring ("\x0a""saveashtml", bsitem);
			break;
			
		case saveasplaintextitem:
			copystring ("\x0f""saveasplaintext", bsitem);
			break;
		
		#endif //!OPMLEDITOR
			
		case viewinbrowseritem:
			copystring ("\x0d""viewinbrowser", bsitem);
			break;

		case updateradiorootitem:		/*7.0d10 PBS*/
			copystring ("\x06""update", bsitem);
			break;
		
		case workofflineitem: /*7.0b25 PBS*/
			copystring ("\x0b""workoffline", bsitem); 
			break;

		#endif //PIKE

		default:
			assert (false);
			break;

		}/*switch*/

	}/* getfilemenuitemidentifier */


void runopenrecentmenuscript (short iditem) {
	
	/*
	2005-09-24 creedon: created
	*/

	bigstring bsscript, bsitem, bsresult;
	
	// getfilemenuitemidentifier (ixmenu, bsitem);

	getsystemtablescript (idrunopenrecentmenuscript, bsscript); /* get from resource */
	
	// copystring ("\x39""Frontier.tools.windowTypes.runOpenRecentMenuScript (\"^0\")", bsscript);

	numbertostring (iditem, bsitem);
	
	parsedialogstring (bsscript, bsitem, nil, nil, nil, bsscript);

	grabthreadglobals ();
	
	langrunstringnoerror (bsscript, bsresult);

	releasethreadglobals ();
	} /* runopenrecentmenuscript */


void getcommonstylesmenuitemtext (short ixmenu, bigstring bsmenuitem) {
	
	/*
	2005-09-25 creedon: changed name from pikegetcommonstylesmenuitemtext to getcommonstylesmenuitemtext, all targets can now run a script associated with some of the edit menu commands
	
	7.0b26 PBS: Get the text that appears in a Common Styles item.
	*/

	bigstring bsscript, bsitem;

	setemptystring (bsitem);

	switch (ixmenu) {

		case 1:

			copystring ("\x04""tiny", bsitem);

			break;

		case 2:

			copystring ("\x06""medium", bsitem);

			break;


		case 3:

			copystring ("\x08""readable", bsitem);

			break;


		case 4:

			copystring ("\x04""huge", bsitem);

			break;		
		} /*switch*/

	copystring ("\x22""menus.scripts.styleMenuItem (\"^0\")", bsscript);

	parsedialogstring (bsscript, bsitem, nil, nil, nil, bsscript);

	grabthreadglobals ();
	
	langrunstringnoerror (bsscript, bsmenuitem);

	releasethreadglobals ();
	} /* getcommonstylesmenuitemtext */


void runeditmenuscript (short idmenu, short iditem) {
		
	/*
	2005-09-25 creedon: changed name from pikeruneditmenuscript to runeditmenuscript, all targets can now run a script associated with some of the edit menu commands

	7.1b4 PBS: get script from resource, don't hard-code.

	7.0b26 PBS: Call pike.runEditMenuScript to run the script associated with
	the current menu command.
	*/

	bigstring bsscript, bsitem, bsresult;

	if (roottable == nil)
		return;

	geteditmenuitemidentifier (idmenu, iditem, bsitem);

	/*copystring ("\x1c""pike.runEditMenuScript(\"^0\")", bsscript);*/

	getsystemtablescript (idruneditmenuscript, bsscript); 

	parsedialogstring (bsscript, bsitem, nil, nil, nil, bsscript);

	grabthreadglobals ();
	
	langrunstringnoerror (bsscript, bsresult);

	releasethreadglobals ();
	} /* runeditmenuscript */

void geteditmenuitemidentifier (short ixmenu, short ixitem, bigstring bsitem) {

	/*
	2005-09-25 creedon: changed name from pikegeteditmenuitemidentifier to geteditmenuitemidentifier, all targets can now run a script associated with some of the edit menu commands

	7.0b26 PBS: Get the identifier for an Edit menu item in the Find and Replace or Common Styles menu.
	*/
	
	if (ixmenu == editmenu) {
		
		switch (ixitem) {

#ifdef PIKE
#ifndef OPMLEDITOR	

			case opennotepaditem:
				copystring ("\x0b""opennotepad", bsitem);
				break;

#endif // !OPMLEDITOR		
#endif // PIKE

			case insertdatetimeitem:
				copystring ("\x0e""insertdatetime", bsitem);
				break;
			} /*switch*/
		} /*if*/
			
	
	if (ixmenu == findandreplacemenu) {

		switch (ixitem) {

			case 1:
				copystring ("\x04""find", bsitem);
				break;

			case 2:
				copystring ("\x07""replace", bsitem);
				break;

			case 3:
				copystring ("\x08""findnext", bsitem);
				break;

			case 4:
				copystring ("\x12""replaceandfindnext", bsitem);
				break;

			} /*switch*/
		} /*if*/

	if (ixmenu == commonstylesmenu) {

		switch (ixitem) {

			case 1:
				copystring ("\x04""tiny", bsitem);
				break;

			case 2:
				copystring ("\x06""medium", bsitem);
				break;

			case 3:
				copystring ("\x08""readable", bsitem);
				break;

			case 4:
				copystring ("\x04""huge", bsitem);
				break;

			} /*switch*/
		} /*if*/
	}/* geteditmenuitemidentifier */


static boolean openrecentmenuitemsvisit (bigstring bs, hdlhashnode hnode, tyvaluerecord val, ptrvoid refcon) {

	/*
	2005-09-24 creedon: created
	*/
	
	register hdlmenu hmenu;
	
	deletestring (bs, 1, 9);
	
	hmenu = shellmenuhandle (openrecentmenu);
	
	// InsertMenuItemTextWithCFString (hmenu, CFStringCreateWithPascalString (NULL, bs, kCFStringEncodingMacRoman), 0, 0, 0);
	
	Insertmenuitem (hmenu, 1, bs);
	
	return (false); // always return false so that hashsortedinversesearch will visit all nodes
	} /* openrecentmenuitemsvisit */


void shellupdateopenrecentmenu (void) {

	/*
	2005-10-01 creedon: when no files are listed in the menu, disable all menu items
			   made more accessible to windows platform
	
	2005-09-22 creedon: created
	*/
	
	bigstring bs;
	boolean fl;
	hdlhashtable htable;
	register hdlmenu hmenu;

	hmenu = shellmenuhandle (openrecentmenu);

	deletemenuitems (hmenu, 1, countmenuitems (hmenu) - 2);
	
	getsystemtablescript (idopenrecentmenutable, bs); // "user.prefs.openrecentmenu.items"

	pushhashtable (roottable);

	disablelangerror ();

	fl = langexpandtodotparams (bs, &htable, bs);

	enablelangerror ();

	pophashtable ();

	if (fl)
		hashsortedinversesearch (htable, &openrecentmenuitemsvisit, nil);
	
	if (countmenuitems (hmenu) == 2)
		disableallmenuitems (hmenu);
		
	}

