
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
#include "error.h"
#include "menu.h"
#include "memory.h"
#include "strings.h"
#include "threads.h"


#define idtempmenu 3000 /*use this ID to create a temporary menu*/


static boolean isseparatorstring (bigstring bs) {

	byte bsseparator [4];
	
	setstringwithchar ('-', bsseparator);

	return (equalstrings (bs, STR_menuseparator) || equalstrings (bs, bsseparator));
	} /*isseparatorstring*/



boolean getcommandkeystring (byte chkey, tykeyflags keyflags, bigstring bscmdkey) {
	
		setemptystring (bscmdkey);
	
		if (keyflags & keycommand)
			pushstring ("\pCmd-", bscmdkey);
		
		if (keyflags & keyshift)
			pushstring ("\pShift-", bscmdkey);
		
		if (keyflags & keyoption)
			pushstring ("\pOption-", bscmdkey);
		
		if (chkey != chnul)
			pushchar (chkey, bscmdkey);
	
	
	return  (true);
	} /*getcommandkeystring*/


void drawmenubar (void) {
	
		DrawMenuBar ();

	} /*drawmenubar*/


hdlmenu getresourcemenu (short id) {

	
		/*Code change by Timothy Paustian Saturday, July 22, 2000 10:26:47 PM
		/code to conditionalize the quit menu.*/
		
		
			if (id == 2 || id == 4) { /*PBS 7.1b5: conditionalize Window menu also*/
			
			/*we are loading the file menu.
			check to see if we are in OS X if so change the id
			of the file menu*/
			
				SInt32	result;
				
				Gestalt (gestaltMenuMgrAttr, &result);
				
				if (result & gestaltMenuMgrAquaLayoutMask) {
					
					if (id == 2) /*File menu*/
						id = 412;
						
					if (id == 4) /*Window menu*/
						id = 413;
					} /*if*/
			} /*if*/
			
		
		return (GetMenu (id));
		

	} /*getresourcemenu*/

//Code change by Timothy Paustian Saturday, April 29, 2000 9:33:29 PM
//Changed for UH 3.3.1 to avoid conflict with Menus.h
hdlmenu Newmenu (short idmenu, bigstring bstitle) {

	/*
	4.18.97 dmb: for win95, we must actually create a parent menu with 
	a single item that is the actual (pulldown or popup) popup menu that
	we will be using. this accomplishes two things:

	1. it matches the structure of getresourcemenu, which actually returns 
	a menubar

	2. it gives us a place to maintain the name and command number of the 
	new menu -- in the item info for the parent's 0th (and only) item

	All other entrypoints in this file must account for this two-tiered 
	structure.
	*/

		return (NewMenu (idmenu, bstitle));

	} /*newmenu*/


void disposemenu (hdlmenu hmenu) {
	
	/*
	4.18.97 dmb: for win95, mirror logic of newmenu
	*/

		DisposeMenu (hmenu);

	} /*disposemenu*/


hdlmenu getmenuhandle (short id) {

		return (GetMenuHandle (id));

	} /*getmenuhandle*/


boolean insertmenu (hdlmenu hmenu, long idbefore) {
	
		if (idbefore == insertatend)
			idbefore = 0;
		
		InsertMenu (hmenu, idbefore);

		return (true);

	} /*insertmenu*/


boolean inserthierarchicmenu (hdlmenu hmenu, short idmenu) {
#	pragma unused (idmenu)

	InsertMenu (hmenu, insertsubmenu);

	return (true);

	} /*inserthierarchicmenu*/


void removemenu (short idmenu) {
	
		DeleteMenu (idmenu); // (**hmenu).menuID

	} /*removemenu*/



long trackmenu (Point mousept) {
	
	/*
	11/20/91 dmb: track the user's menu selection.  return the result 
	encoded in the high and low words of the long result.
	
	if no valid item is selected, but the mouse was released over a 
	disabled item (possibly the parent of a hierarchical menu), return 
	the negative of the item value (encoded as usual w/the menu id)
	*/
	
	long menucode;
	short menu;
	short item;
	//Code change by Timothy Paustian Tuesday, August 29, 2000 1:32:04 PM
	//No using direct trap address in OS X or carbon. Use the MenuChoice 
	//routine, that's what it is for.
	//#define MenuDisable (*(long *)0x0B54)
	
	menucode = MenuSelect (mousept);
	
	item = LoWord (menucode);
	
	if (item == 0) { /*no enable item selected*/
		
		//old code
		//menucode = MenuDisable; 
		//new code, why was the old stuff left in?
		menucode = MenuChoice ();
		
		item = -LoWord (menucode);
		}
	
	menu = HiWord (menucode);
	
	return ((long) menu << 16) + (unsigned short) item;
	} /*trackmenu*/


boolean sethierarchicalmenuitem (hdlmenu hmenu, short itemnumber, hdlmenu hsubmenu, short idsubmenu) {

		#pragma unused (hsubmenu)	
		short cmdchar, markchar;

		if (hsubmenu != nil) {

			cmdchar = hMenuCmd; /*$1B*/
			
			markchar = idsubmenu;
			}
		else {
			
			cmdchar = noMark;
			
			markchar = 0;
			}
		
		SetItemCmd (hmenu, itemnumber, cmdchar);

		SetItemMark (hmenu, itemnumber, markchar);
		
		return (true);

	} /*sethierarchicalmenuitem*/


boolean gethierarchicalmenuitem (hdlmenu hmenu, short ixmenu, hdlmenu *hsubmenu) {

		
		short cmdchar;
		short idsub;
		
		GetItemCmd (hmenu, ixmenu, &cmdchar);
		
		if (cmdchar == hMenuCmd) {
			
			GetItemMark (hmenu, ixmenu, &idsub);
			
			*hsubmenu = GetMenuHandle (idsub);
			}


	return (*hsubmenu != NULL);
	} /*gethierarchicalmenuitem*/


void setmenutitleenable (hdlmenu hmenu, short idmenu, boolean fl) {
#	pragma unused (idmenu)
	/*
	enable or disable a menu or a menu item.  if fl is true we enable the item,
	if false we disable it.
	
	if item == 0 we enable or disable the entire menu.
	
	dmb 8/1/90:  check for dummy items (negative item numbers)
	*/
	//Code change by Timothy Paustian Friday, June 9, 2000 9:44:07 PM
	//Changed to Modern calls for Enabling and Disabling menu items
	if (fl) {
#	if ACCESSOR_CALLS_ARE_FUNCTIONS == 1
		EnableMenuItem (hmenu, 0);
#	else
		EnableItem (hmenu, 0);
#	endif
	} else {
#	if ACCESSOR_CALLS_ARE_FUNCTIONS == 1
		DisableMenuItem (hmenu, 0);
#	else
		DisableItem (hmenu, 0);
#	endif
	}

	} /*setmenutitleenable*/
	
	
void setmenuitemenable (hdlmenu hmenu, short item, boolean fl) {
	
	/*
	enable or disable a menu or a menu item.  if fl is true we enable the item,
	if false we disable it.
	
	if item == 0 we enable or disable the entire menu.
	
	dmb 8/1/90:  check for dummy items (negative item numbers)
	*/
	
		if (item < 0) /*this item has been dummied out -- do nothing*/
			return;
		//Code change by Timothy Paustian Monday, May 1, 2000 9:10:06 PM
		//Changed to Opaque call for Carbon
		//updated the call from EnableItem and DisableItem
		if (fl)
			#if ACCESSOR_CALLS_ARE_FUNCTIONS == 1
			EnableMenuItem (hmenu, item);
			#else
			EnableItem(hmenu,item);
			#endif
		else
			#if ACCESSOR_CALLS_ARE_FUNCTIONS == 1
			DisableMenuItem (hmenu, item);
			#else
			DisableItem(hmenu,item);
			#endif

	} /*setmenuitemenable*/
	
	
boolean getmenutitleenable (hdlmenu hmenu, short idmenu) {
	
	/*
	*/
	
#	pragma unused (idmenu)
#	if ACCESSOR_CALLS_ARE_FUNCTIONS == 1

	return IsMenuItemEnabled(hmenu, 0);

#	else

	//old code
	return (((**hmenu).enableFlags & (0x01)) != 0);

#	endif

	} /*getmenuitemenable*/
	
	
boolean getmenuitemenable (hdlmenu hmenu, short item) {
	
		assert (item >= 0);
		//Code change by Timothy Paustian Monday, May 1, 2000 9:16:32 PM
		//Changed to Opaque call for Carbon
		#if ACCESSOR_CALLS_ARE_FUNCTIONS == 1
		return IsMenuItemEnabled(hmenu, item);
		#else
		//old code
		return (((**hmenu).enableFlags & (0x01 << item)) != 0);
		#endif

	} /*getmenuitemenable*/
	
	
void disablemenuitem (hdlmenu hmenu, short item) {
	
	setmenuitemenable (hmenu, item, false);
	} /*disablemenuitem*/

void enablemenuitem (hdlmenu hmenu, short item) {
	
	setmenuitemenable (hmenu, item, true);
	} /*enablemenuitem*/


short countmenuitems (hdlmenu hmenu) {
	
		return (CountMenuItems (hmenu));

	} /*countmenuitems*/


void enableallmenuitems (hdlmenu hmenu, boolean fl) {
	
	register short lastitem;
	register short i;
	
	lastitem = countmenuitems (hmenu);
	
	for (i = 1; i <= lastitem; i++) 
		setmenuitemenable (hmenu, i, fl);
	} /*enableallmenuitems*/
	

void hilitemenu (short id) {

		HiliteMenu (id);
	} /*hilitemenu*/


void checkmenuitem (hdlmenu hmenu, short ixmenu, boolean fl) {
	
	/*
	5.0a23 dmb: preserve other fState bits [Win]
	*/
	
		//Code change by Timothy Paustian Friday, June 9, 2000 9:44:37 PM
		//Changed to Modern call.
		CheckMenuItem (hmenu, ixmenu, fl);

	} /*checkmenuitem*/
	
	
boolean menuitemmarked (hdlmenu hmenu, short ixmenu) {
	
		short mark = 0;
		
		GetItemMark (hmenu, ixmenu, &mark);
		
		return (mark != 0);

	} /*menuitemmarked*/


void markmenuitem (hdlmenu hmenu, short ixmenu, short mark) {
	

		/*
		5.0a23 dmb: preserve other fState bits [Win]
		*/

		SetItemMark (hmenu, ixmenu, mark);

	} /*markmenuitem*/


void stylemenuitem (hdlmenu hmenu, short ixmenu, short style) {
	
		SetItemStyle (hmenu, ixmenu, style);

	} /*stylemenuitem*/
	


	
boolean setmenutitle (hdlmenu hmenu, bigstring bs) {
	
		SetMenuItemText (hmenu, 0, bs);
		
		return (true);

	} /*setmenutitle*/


static boolean getmenutitle (hdlmenu hmenu, bigstring bs) {
	
		//Code change by Timothy Paustian Monday, May 1, 2000 9:15:51 PM
		//Changed to Opaque call for Carbon
		#if ACCESSOR_CALLS_ARE_FUNCTIONS == 1
		Str255	menuTitle;
		GetMenuTitle(hmenu, menuTitle);
		copystring (menuTitle, bs);
		#else
		//old code
		copystring ((ptrstring) (**hmenu).menuData, bs);
		#endif
		return (true);

	} /*getmenutitle*/


boolean setmenuitem (hdlmenu hmenu, short ixmenu, bigstring bs) {
	
	/*
	4/29/91 dmb: check for empty string

	5.0a23 dmb: maintain accelerator string [Win]
	*/
	
		if (ixmenu <= 0)
			return (false);
		
		if (isemptystring (bs))
			SetMenuItemText (hmenu, ixmenu, "\p ");
		else
			SetMenuItemText (hmenu, ixmenu, bs);
			
		return (true);

	} /*setmenuitem*/


boolean getmenuitem (hdlmenu hmenu, short ixmenu, bigstring bs) {
	

	if (ixmenu < 0)
		return (false);
	
	if (ixmenu == 0)
		return (getmenutitle (hmenu, bs));

		GetMenuItemText (hmenu, ixmenu, bs);
		
		return (true);

	} /*getmenuitem*/


boolean setmenuitemcommandkey (hdlmenu hmenu, short ixmenu, short key) {

	SetItemCmd (hmenu, ixmenu, key);

	return (true);

	} /*setmenuitemcommandkey*/


void getmenuitemcommandkey (hdlmenu hmenu, short ixmenu, short *key) {

		GetItemCmd (hmenu, ixmenu, key);

	} /*getmenuitemcommandkey*/


//Code change by Timothy Paustian Saturday, April 29, 2000 9:33:29 PM
//Changed for UH 3.3.1 to avoid conflict with Menus.h
boolean Insertmenuitem (hdlmenu hmenu, short ixmenu, bigstring bs) {
	
	/*
	3/26/91 dmb: added logic to handle meta-characters
	
	12/31/96 dmb: added idmenu parameter. under windows, the command
	for the new menu has the menu id in the high byte, the item number
	in the low byte
	*/
	
	 /*to allow meta-characters in bs, append blank item, then set item text*/
	bigstring bsspace;
	
	if (equalstrings (bs, STR_menuseparator)) /*take disabled seperator as is*/
		InsertMenuItem (hmenu, bs, ixmenu - 1);
		
	else {
		setstringwithchar (chspace, bsspace);
		
	//	AppendMenu (hmenu, bsspace);
		InsertMenuItem (hmenu, bsspace, ixmenu - 1);
		
		if (!isemptystring (bs))
			SetMenuItemText (hmenu, ixmenu, bs);
		}
	
	return (true);

	} /*insertmenuitem*/


boolean deletemenuitem (hdlmenu hmenu, short ixmenu) {

	DeleteMenuItem (hmenu, ixmenu);
	
	return (true);

	} /*deletemenuitem*/


boolean deleteallmenuitems (hdlmenu hmenu, short afteritem) {
	
	register short ct, i;
	
	ct = countmenuitems (hmenu);
	
	for (i = ct; i > afteritem; i--)
		if (!deletemenuitem (hmenu, i))
			return (false);
		
	return (true);
	} /*deleteallmenuitems*/
	
	
boolean deletelastmenuitem (hdlmenu hmenu) {
	
	return (deletemenuitem (hmenu, countmenuitems (hmenu)));
	} /*deletelastmenuitem*/


boolean pushmenuitem (hdlmenu hmenu, short idmenu, bigstring bs, short commandid) {
	
	/*
	3/26/91 dmb: added logic to handle meta-characters
	
	12/31/96 dmb: added idmenu parameter. under windows, the command
	for the new menu has the menu id in the high byte, the item number
	in the low byte

	7.1b23 PBS: new parameter commandid allows one to specify the commandid.
	If it's < 1, then calculate the commandid.
	*/
	
#	pragma unused (idmenu)
	bigstring bsspace;
	
	if (isseparatorstring (bs)) /*take disabled seperator as is*/
		AppendMenu (hmenu, bs);
	
	else { /*to allow meta-characters in bs, append blank item, then set item text*/
		
		setstringwithchar (chspace, bsspace);
		
		AppendMenu (hmenu, bsspace);
		
		if (!isemptystring (bs))
			SetMenuItemText (hmenu, countmenuitems (hmenu), bs);
		
		if (commandid > 0) /*7.1b23 PBS: allow caller to specify commandid*/
			SetMenuItemCommandID (hmenu, countmenuitems (hmenu), commandid);
			
		}
	
	return (true);

	} /*pushmenuitem*/


boolean pushresourcemenuitems (hdlmenu hmenu, short idmenu, OSType restype) {

#	pragma unused (idmenu)
	//Code change by Timothy Paustian Wednesday, June 28, 2000 4:27:19 PM
	//You can't do this in carbon.
	//#if !TARGET_API_MAC_CARBON	
	AppendResMenu (hmenu, restype);
	//#endif
		
	return (true);

	} /*pushresourcemenuitems*/


	
boolean pushdottedlinemenuitem (hdlmenu hmenu) {
	
	/*
	push a disabled dotted line at the end of the indicated menu.
	*/
	
	return (pushmenuitem (hmenu, 0, STR_menuseparator, 0));
	} /*pushdottedlinemenuitem*/
	

/*
boolean newtempmenu (hdlmenu *hmenu, short *idmenu) {
	
	register hdlmenu h;
	
	h = NewMenu (idtempmenu, (ptrstring) "\p¹"); /%name is irrelevent in a popup%/
	
	if (h == nil) { /%error allocating new menu%/
		
		sysbeep ();
		
		return (false);
		}
	
	*hmenu = h;
	
	*idmenu = idtempmenu;
	
	return (true);
	} /%newtempmenu%/
*/
	
	
short getprevmenuitem (hdlmenu hmenu) {
	
	register hdlmenu hm = hmenu;
	register short ct;
	register short i;
	
	ct = countmenuitems (hmenu);
	
	for (i = ct; i >= 1; i--) {
		
		if (menuitemmarked (hm, i)) {
			
			if (i == 1) /*first item checked, wrap to bottom*/
				return (ct);
			else
				return (i - 1);
			}
		} /*for*/
	
	return (1); /*no item checked*/
	} /*getprevmenuitem*/

	
short getnextmenuitem (hdlmenu hmenu) {
	
	register hdlmenu hm = hmenu;
	register short ct;
	register short i;
	
	ct = countmenuitems (hmenu);
	
	for (i = 1; i <= ct; i++) {
		
		if (menuitemmarked (hm, i)) {
			
			if (i == ct) /*last item checked, wrap to top*/
				return (1);
			else
				return (i + 1);
			}
		} /*for*/
		
	return (1); /*no item checked*/
	} /*getnextmenuitem*/


boolean initmenusystem (void) {


	return (true);
	} /*initmenusystem*/


boolean deletemenuitems (hdlmenu hmenu, short firstitem, short itemcount) {

	/*
	2005-10-01 creedon: created, delete menu items starting at firstitem for itemcount
	
	2006-02-19 aradke: switched to platform-agnostic code
	*/

	short ctitems = countmenuitems (hmenu);
	short i;
	
	if (itemcount > ctitems)
		itemcount = ctitems;
	
	for (i = itemcount; i >= firstitem; i--) {
		if (!deletemenuitem (hmenu, i)) {
			return (false);
			}			
		} /*for*/
		
	return (true);
	} /* deletemenuitems */


void disableallmenuitems (hdlmenu hmenu) {

	/*
	2005-10-01 creedon: created
	
	2006-02-19 aradke: switched to platform-agnostic code
	*/

	short ct = countmenuitems (hmenu);
	short i;

	for (i = ct; i > 0; i--)
		disablemenuitem (hmenu, i);
	
	} /* disableallmenuitems */

