
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
Implement right-click menus in the system tray icon or dock menu.
Broken out from FrontierWinMain.c
7.1b22 11/08/01 PBS
*/

#include "frontier.h"
#include "standard.h"

#include "menu.h"
#include "strings.h"
#include "cancoon.h"
#include "launch.h"
#include "tablestructure.h"
#include "popup.h"
#include "meprograms.h"
#include "dockmenu.h"
#include "opinternal.h"
#include "menuverbs.h"

#define idgetdockmenumenuaddresscallback 43

#define kmaxcommands 100
#define kbasecommandid 3000
#define maxsubmenus 40


#ifdef MACVERSION
	UInt32 menuid;
	short idsubmenu;
	hdlmenurecord hcurrmenurecord;
	boolean flstackneedsdisposing = false;
#endif


#ifdef WIN95VERSION
	short menuid;
#endif


#ifdef MACVERSION

	typedef struct typopupinfo {

		hdlmenu hmenu;
		
		short id;			
		} typopupinfo, *ptrpopupinfo, **hdlpopupinfo;

	typedef struct tydockmenustack {
		
		typopupinfo popupmenus [maxsubmenus];
		
		short currstackitem;
		} tydockmenustack, *ptrdockmenustack, **hdldockmenustack;


	tydockmenustack dockmenustack;

#endif


static boolean dockmenubuildpopupmenu (hdlheadrecord hnode, hdlmenu hmenu); /*forward*/

static void dockmenudisposemenusinstack (void);


static void dockmenuresetmenustack (void) {
	
	#ifdef MACVERSION
	
		dockmenudisposemenusinstack ();

		dockmenustack.currstackitem = -1;
		
		dockmenustack.popupmenus [0].hmenu = nil;
		
		dockmenustack.popupmenus [0].id = -1;
	
	#endif
	} /*dockmenuresetmenustack*/


static void dockmenudisposemenusinstack (void) {
	
	/*
	Dispose and delete all menus in popup menu stack.
	*/
	
	#ifdef MACVERSION
	
		short ix = dockmenustack.currstackitem;
		short i, id;
		hdlmenu hmenu;
		
		if (!flstackneedsdisposing)
			return;
		
		if (ix < 0)
			return;
		
		for (i = ix; i > -1; i--) {
			
			id = dockmenustack.popupmenus [i].id;
			
			if (id > -1)
			
				DeleteMenu (id);
				
			if (id != defaultpopupmenuid) { /*already disposed by system*/
			
				hmenu = dockmenustack.popupmenus [i].hmenu;
			
				if (hmenu != nil)
			
					disposemenu (hmenu);
				} /*if*/
			
			dockmenustack.popupmenus [i].hmenu = nil;
			
			dockmenustack.popupmenus [i].id = -1;
			
			} /*for*/
		
		flstackneedsdisposing = false;	
	#endif
	} /*dockmenudisposemenusinstack*/


static boolean dockmenuaddtomenustack (hdlmenu hmenu, short id) {

	#ifdef MACVERSION

		short ix = dockmenustack.currstackitem + 1;
		
		if (ix > maxsubmenus)	
			return (false);
		
		dockmenustack.currstackitem = ix;
		
		dockmenustack.popupmenus [ix].hmenu = hmenu;
		
		dockmenustack.popupmenus [ix].id = id;
	
	#endif

	return (true);	
	} /*dockmenuaddtomenustack*/


static void dockmenuinsertsubmenu (hdlmenu hmenu, short itemnumber, hdlheadrecord hnode) {

	/*
	7.1b23 PBS: build a sub-menu and attach it.
	*/

	hdlmenu hsubmenu;
	short id = defaultpopupmenuid;

	#ifdef MACVERSION
		idsubmenu++;
		
		id = idsubmenu;
	#endif
	
	hsubmenu = Newmenu (id, "");

	#ifdef MACVERSION	
		InsertMenu (hsubmenu, -1);
	#endif

	dockmenubuildpopupmenu (hnode, hsubmenu);

	dockmenuaddtomenustack (hsubmenu, id);

	sethierarchicalmenuitem (hmenu, itemnumber, hsubmenu, id);
	} /*dockmenuinsertsubmenu*/


static boolean dockmenugetaddresscallback (tyvaluerecord *val) {

	/*
	7.1b22 PBS: Run the callback script that returns the address
	of a menubar object to use.
	*/

	Handle htext = 0;
	boolean fl = false;
	bigstring bsscript;
	short idscript = idgetdockmenumenuaddresscallback;

	if (getsystemtablescript (idscript, bsscript)) {
	
		if (!newtexthandle (bsscript, &htext))
		
			return (false);

		grabthreadglobals ();
		
		oppushoutline (outlinedata);
		
		fl = langrun (htext, val);

		oppopoutline ();

		releasethreadglobals ();
		}

	return (fl);
	} /*dockmenugetaddresscallback*/


static boolean dockmenuinsertmenuitem (hdlmenu hmenu, short itemnumber, hdlheadrecord hnode) {

	/*
	Insert one menu item.

	7.1b42 PBS: check menu items that should be checked.
	*/
	
	bigstring bsheadstring;
	boolean flenabled = true;
	boolean flchecked = false;

	getheadstring (hnode, bsheadstring);

	mereduceformula (bsheadstring);
	
	mereducemenucodes (bsheadstring, &flenabled, &flchecked); /*7.0b23 PBS: items can be disabled.*/

	pushpopupitem (hmenu, bsheadstring, flenabled, menuid);

	#if TARGET_API_MAC_CARBON == 1
	
		SetMenuItemCommandID (hmenu, countmenuitems (hmenu), menuid);
		
	#endif

	if (flchecked) /*7.1b42 PBS: support for checked menu items.*/
		checkmenuitem (hmenu, countmenuitems (hmenu), flchecked);

	if (!opnosubheads (hnode)) /*has subs?*/
		dockmenuinsertsubmenu (hmenu, itemnumber, hnode);

	#ifdef MACVERSION
		flstackneedsdisposing = true;
	#endif

	return (true);
	} /*dockmenuinsertmenuitem*/


static boolean dockmenubuildpopupmenu (hdlheadrecord hnode, hdlmenu hmenu) {
	
	/*
	7.1b22 PBS: Build a popup menu from the outline pointed to by hnode.
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

		menuid++;
		
		if (!dockmenuinsertmenuitem (hmenu, itemnumber, hnode))
			return (false);
		
		if (opislastsubhead (hnode)) /*done with this menu*/
			return (true);
		
		hnode = (**hnode).headlinkdown; /*advance to next item*/
		} /*while*/
	} /*dockmenubuildpopupmenu*/


static boolean dockmenufillpopup (hdlmenu hmenu, hdlmenurecord *hmreturned) {

	/*
	7.1b44 PBS: call menuverbgetsize to make sure the menu is in memory.
	*/

	hdlhashtable htable;
	hdlhashnode hnode;
	hdlmenurecord hm;
	tyvaluerecord valaddress, val;
	hdlexternalhandle h;
	hdlheadrecord hsummit;
	bigstring bsaddress;
	boolean fl = false;
	long menusize = 0;

	if (roottable == nil) /*9.1b1 JES: If there's no system root, don't crash.*/
		return (false);
	
	if (!dockmenugetaddresscallback (&valaddress))
		return (false);
	
	if (valaddress.valuetype != addressvaluetype)
		goto exit;

	if (!getaddressvalue (valaddress, &htable, bsaddress))
		goto exit;

	if (!langsymbolreference (htable, bsaddress, &val, &hnode))
		goto exit;

	if (val.valuetype != externalvaluetype)
		goto exit;

	h = (hdlexternalhandle) val.data.externalvalue;

	if (!menuverbgetsize (h, &menusize)) /*7.1b43 PBS: We don't care about the size. We just want to read it into memory.*/
		goto exit;

	if ((**h).id != idmenuprocessor) /*it must be a menu*/
		goto exit;

	hm = (hdlmenurecord) (**h).variabledata;

	hsummit = (**(**hm).menuoutline).hsummit;

	#if TARGET_API_MAC_CARBON == 1
		menuid = kbasecommandid;
		idsubmenu = 5;
	#endif

	dockmenudisposemenusinstack ();
	
	dockmenubuildpopupmenu (hsummit, hmenu);
	
	*hmreturned = hm;

	fl = true;
	
	exit:

	disposevaluerecord (valaddress, false);

	return (fl);
	} /*dockmenufillpopup*/


static void dockmenuruncommand (hdlmenurecord hm, short itemhit) {
	
	hdlheadrecord hsummit;
	hdlheadrecord hmenuitem;
	bigstring bs;

	hsummit = (**(**hm).menuoutline).hsummit; /*Top level of the menu*/

	hmenuitem = oprepeatedbump (flatdown, itemhit, hsummit, false);
	
	oppushoutline ((**hm).menuoutline);

	getheadstring (hmenuitem, bs);

	meuserselected (hmenuitem);
	
	dockmenudisposemenusinstack ();

	oppopoutline ();
	} /*dockmenuruncommand*/


#if TARGET_API_MAC_CARBON == 1

pascal OSStatus dockcommandhandler (EventHandlerCallRef nextHandler, EventRef theEvent, void* userData) {
	
	#pragma unused(nextHandler) /*happy compiler*/
	#pragma unused(theEvent)
	#pragma unused(userData)

	HICommand commandstruct;
	UInt32 commandid;
	OSErr ec = eventNotHandledErr;
	
	GetEventParameter (theEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof (HICommand), NULL, &commandstruct);
	
	commandid = commandstruct.commandID;
	
	if ((commandid >= kbasecommandid) && (commandid <= kbasecommandid + kmaxcommands)) {
	
		dockmenuruncommand (hcurrmenurecord, commandid - kbasecommandid);
		
		ec = noErr; /*all's well*/
		} /*if*/
	
	dockmenudisposemenusinstack (); /*7.1b23 PBS: delete and dispose submenus*/
	
	return (ec);
	} /*dockcommandhandler*/


static void dockmenuinstallhandler (void) {
	
	EventTypeSpec myevents = {kEventClassCommand, kEventCommandProcess};
	
	InstallApplicationEventHandler (NewEventHandlerUPP (dockcommandhandler), 1, &myevents, 0, NULL);
	
	} /*dockmenuinstallhandler*/


pascal OSStatus dockmenuhandler (EventHandlerCallRef nextHandler, EventRef theEvent, void* userData) {
	
	/*
	7.1b22 PBS: called from the system when the dock icon is right-clicked.
	Build a contextual menu and return it.
	*/
	
	#pragma unused(nextHandler) /*happy compiler*/
	#pragma unused(theEvent)
	#pragma unused(userData)
	
	hdlmenu hmenu;
	hdlmenurecord hm;
	static boolean flinited = false;
	
	menuid = 0;
	
	dockmenuresetmenustack (); /*7.1b23 PBS: maintain a stack of submenus*/
	
	if (!flinited) { /*install command handler first time*/
		
		dockmenuinstallhandler ();
		
		flinited = true;
		} /*if*/
	
	hmenu = Newmenu (defaultpopupmenuid, "");
	
	if (!dockmenufillpopup (hmenu, &hm))
		goto exit;
	
	dockmenuaddtomenustack (hmenu, defaultpopupmenuid);

	hcurrmenurecord = hm;
	
	SetEventParameter (theEvent, kEventParamMenuRef, typeMenuRef, sizeof (MenuRef), &hmenu);
	
	exit:

	return (noErr); /*all's well in dock-menu-land*/
	} /*dockmenuhandler*/

#endif

#ifdef WIN95VERSION

void rundockmenu (void) {
	
	/*
	The system tray icon or dock icon has been right-clicked.
	Display a menu and handle the user's choice.
	*/

	hdlmenu hmenu;
	hdlmenurecord hm;
	POINT mousept;
	UINT flags = 0;
	
	hmenu = Newmenu (defaultpopupmenuid, "");
	
	menuid = 0;

	if (!dockmenufillpopup (hmenu, &hm))
		goto exit;

	SetMenuDefaultItem (GetSubMenu (hmenu, 0), 0, true); /*Top item is default*/

	GetCursorPos (&mousept); 
	
	SetForegroundWindow (shellframewindow); 

	if (TrackPopupMenuEx (GetSubMenu (hmenu, 0), flags, mousept.x, mousept.y, shellframewindow, NULL)) {
			
		MSG msg;
		short itemhit;
		hdlheadrecord hsummit;
		hdlheadrecord hmenuitem;
		bigstring bs;
		boolean flopencommand = false;

		if (PeekMessage (&msg, shellframewindow, WM_COMMAND, WM_COMMAND, PM_REMOVE)) {

			itemhit = LOWORD (msg.wParam) % 100;

			hsummit = (**(**hm).menuoutline).hsummit; /*Top level of the menu*/

			//hmenuitem = opnthsubhead (hsummit, itemhit);

			oppushoutline ((**hm).menuoutline);
	
			hmenuitem = oprepeatedbump (flatdown, itemhit, hsummit, false);

			getheadstring (hmenuitem, bs);

			if (equalidentifiers (bs, "\x0d" "Open Frontier"))
				flopencommand = true;

			else if (equalidentifiers (bs, "\x0a" "Open Radio"))
				flopencommand = true;

			if (flopencommand) { /*Intercept -- there's no script verb for this command.*/

				ShowWindow (shellframewindow, SW_SHOW); /*Show the window.*/

				activateapplication (NULL); /*bring to front*/
				} /*if*/
			
			else /*run the script*/
				meuserselected (hmenuitem);

			oppopoutline ();
			} /*if*/
		} /*if*/
	
	exit:

	disposemenu (hmenu);
	} /*rundockmenu*/

#endif