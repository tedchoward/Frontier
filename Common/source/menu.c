
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

#ifdef WIN95VERSION
	#include "font.h"
#endif

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


#if WIN95VERSION

extern HWND shellframewindow;
extern HINSTANCE shellinstance;
extern HANDLE hAccel; /*FrontierWinMain.c*/
static Handle haccelerators = NULL;


static void updateaccelerators (void) {
	
	HANDLE newAccel;
	long ctaccelerators = gethandlesize (haccelerators) / sizeof (ACCEL); 

	newAccel = CreateAcceleratorTable ((LPACCEL) *haccelerators, ctaccelerators);
	
	if (newAccel != NULL) {

		DestroyAcceleratorTable (hAccel);

		hAccel = newAccel;
		}
	} /*updateaccelerators*/


static void removeaccelerators (long idmenu, short ixstart) {
	
	/*
	remove all accelerators to items in the indicated menu.
	*/

	int ctaccelerators;
	int i;
	
	ctaccelerators = gethandlesize (haccelerators) / sizeof (ACCEL); 
	
	for (i = ctaccelerators; --i >= 0; ) {
		
		if (((LPACCEL) *haccelerators) [i].cmd / 100 * 100 == idmenu) {
			
			if (((LPACCEL) *haccelerators) [i].cmd % 100 >= ixstart)
				pullfromhandle (haccelerators, i * sizeof (ACCEL), sizeof (ACCEL), NULL);
			}
		}
	} /*removeaccelerators*/


static boolean findaccelerator (long id, short *ixaccel) {
	
	short ctaccelerators;
	short i;

	// search for a existing accelerator
	ctaccelerators = (short) gethandlesize (haccelerators) / sizeof (ACCEL); 
	
	for (i = 0; i < ctaccelerators; ++i) {
		
		if (((LPACCEL) *haccelerators) [i].cmd == id) {
			
			*ixaccel = i;

			return (true);
			}
		}

	return (false);
	} /*findaccelerator*/


static void setupaccelrecord (short key, long cmd, ACCEL *accel) {

	UINT virtcode = VkKeyScan ((TCHAR) key);

	if (virtcode == -1) { // didn't map to a virtual key
		
		(*accel).fVirt = FCONTROL;

		(*accel).key = getlower (key);
		}
	else {
		(*accel).fVirt = FCONTROL | FVIRTKEY;
		
		(*accel).key = virtcode & (0x00FF);
		}
	
	(*accel).cmd = (WORD) cmd;
	} /*setupaccelrecord*/


static boolean insertaccelerator (const ACCEL *accel) {
	
	/*
	5.0a25 dmb: insert at beginning of table, and insert 
	two copies, the second with the shift key
	*/
	
	ACCEL ac [2];
	
	ac [0] = *accel;
	
	ac [1] = *accel;
	
	ac [1].fVirt |= FSHIFT;
	
	return (insertinhandle (haccelerators, 0, ac, 2 * sizeof (ACCEL)));
	} /*insertaccelrecord*/


static boolean pushvirtkeystring (UINT virtcode, bigstring bs, boolean flpushplus) {
	
	char keybuf [32];
	UINT scancode;
	
	scancode = MapVirtualKey (virtcode, 0);
	
	GetKeyNameText (scancode << 16, keybuf, 32L);
	
	convertcstring (keybuf);
	
	if (isemptystring (keybuf))
		return (false);

	pushstring (keybuf, bs);

	if (flpushplus)
		pushchar ('+', bs);

	return (true);
	} /*pushvirtkeystring*/


static void pushaccelstring (ACCEL *ac, bigstring bs) {
	
	/*
	5.0a18 dmb: pull out this code now, for future enhancement. 
	someday, we might offer non-Control keys. Plus, we should use
	the system to get the string for the keystroke.
	*/

	nthword (bs, 1, chtab, bs);

	if ((*ac).key > 0) {
		
		pushchar (chtab, bs);
		
		if ((*ac).fVirt & FCONTROL)
			pushvirtkeystring (VK_CONTROL, bs, true);
		
		if ((*ac).fVirt & FALT)
			pushvirtkeystring (VK_MENU, bs, true);
		
		if ((*ac).fVirt & FSHIFT)
			pushvirtkeystring (VK_SHIFT, bs, true);

		if ((*ac).fVirt & FVIRTKEY)
			pushvirtkeystring ((*ac).key, bs, false);
		else
			pushchar ((byte) (*ac).key, bs);
		}
	} /*pushaccelstring*/


static boolean addaccelerators (hdlmenu hmenu, long idmenu, short ixstart) {
	
	ACCEL accel;
	short i, ctitems;
	MENUITEMINFO info;
	bigstring menuname;
	short ixaccel;
	
	info.cbSize = sizeof (info);
	info.fMask = MIIM_TYPE;
	
	ctitems = GetMenuItemCount (hmenu);
	
	for (i = ixstart - 1; i < ctitems; ++i) {
		
		info.dwTypeData = &menuname[0];
		info.cch = 128;

		GetMenuItemInfo (hmenu, i, true, &info);
		
		if (info.cch == 0)
			continue;
		
		convertcstring (menuname);
		
		if (stringfindchar (chtab, menuname)) {
			
			// add new item to the accelerator table
			setupaccelrecord ((short) lastchar (menuname), idmenu + i + 1, &accel);
			
			if (!findaccelerator (accel.cmd, &ixaccel)) {

				if (!insertaccelerator (&accel))
					return (false);
				}
			}
		}
	
	updateaccelerators ();
	
	return (true);
	} /*addaccelerators*/


boolean adjustitemcommands (hdlmenu hmenu, long idmenu, short ixmenu) {
	
	/*
	an item has been inserted or deleted from the menu. we must 
	adjust the command id for the following items to match thier
	new index in the menu.

	hmenu is the actual pulldown or popup, not the parent menu

	5.0b15 dmb: "ixmenu" is 1-based
	*/
	
	MENUITEMINFO info;
	short ix, ctitems;
	
	info.cbSize = sizeof (info);
	info.fMask = MIIM_ID;
	
	ctitems = GetMenuItemCount (hmenu);
	
	for (ix = ixmenu - 1; ix < ctitems; ++ix) {
		
		info.wID = idmenu + ix + 1;
		
		SetMenuItemInfo (hmenu, ix, true, &info); // get the actual popup and it's id
		}

	return (true);
	} /*adjustitemcommands*/

#endif

boolean getcommandkeystring (byte chkey, tykeyflags keyflags, bigstring bscmdkey) {
	
	#ifdef MACVERSION
		setemptystring (bscmdkey);
	
		if (keyflags & keycommand)
			pushstring ("\pCmd-", bscmdkey);
		
		if (keyflags & keyshift)
			pushstring ("\pShift-", bscmdkey);
		
		if (keyflags & keyoption)
			pushstring ("\pOption-", bscmdkey);
		
		if (chkey != chnul)
			pushchar (chkey, bscmdkey);
	#endif
	
	#ifdef WIN95VERSION
		ACCEL ac;
		
		setemptystring (bscmdkey);
		
		ac.fVirt = 0;
		
		if (keyflags & keycommand)
			ac.fVirt |= FCONTROL;
		
		if (keyflags & keyoption)
			ac.fVirt |= FALT;
		
		if (keyflags & keyshift)
			ac.fVirt |= FSHIFT;
		
		ac.key = chkey;
		
		pushaccelstring (&ac, bscmdkey);
		
		popleadingchars (bscmdkey, chtab);
	#endif
	
	return  (true);
	} /*getcommandkeystring*/


void drawmenubar (void) {
	
	#ifdef MACVERSION
		DrawMenuBar ();
	#endif

	#if WIN95VERSION
		releasethreadglobals ();
		
		DrawMenuBar (shellframewindow);
		
		grabthreadglobals ();
	#endif
	} /*drawmenubar*/


hdlmenu getresourcemenu (short id) {

	#ifdef MACVERSION
	
		/*Code change by Timothy Paustian Saturday, July 22, 2000 10:26:47 PM
		/code to conditionalize the quit menu.*/
		
		#if TARGET_API_MAC_CARBON == 1
		
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
			
		#endif
		
		return (GetMenu (id));
		
	#endif

	#ifdef WIN95VERSION
		hdlmenu hmenu;
		MENUITEMINFO info;
		
		hmenu = LoadMenu (shellinstance, MAKEINTRESOURCE (id));
		
		// set the id of the actual popup to match its resource id
		info.cbSize = sizeof (info);
		info.fMask = MIIM_ID;
		info.wID = id;
		
 		SetMenuItemInfo (hmenu, 0, true, &info);
		
		return (hmenu);
	#endif
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

	#ifdef MACVERSION
		return (NewMenu (idmenu, bstitle));
	#endif

	#ifdef WIN95VERSION
		hdlmenu hparent = CreateMenu ();

		if (hparent != NULL) {

			MENUITEMINFO info;
			char menuname [256];


			info.cbSize = sizeof (info);
			info.fMask = MIIM_TYPE | MIIM_SUBMENU | MIIM_ID;

			info.fType = MFT_STRING;
			info.dwTypeData = menuname;
			copyptocstring (bstitle, menuname);

			info.hSubMenu = CreatePopupMenu ();
			
			info.wID = idmenu;
			
			if (!InsertMenuItem (hparent, 0, true, &info)) {
				
				DestroyMenu (info.hSubMenu);

				DestroyMenu (hparent);

				return (NULL);
				}
			}
		
		return (hparent);
	#endif
	} /*newmenu*/


void disposemenu (hdlmenu hmenu) {
	
	/*
	4.18.97 dmb: for win95, mirror logic of newmenu
	*/

	#ifdef MACVERSION
		DisposeMenu (hmenu);
	#endif

	#ifdef WIN95VERSION
		DeleteMenu (hmenu, 0, MF_BYPOSITION); // destroy the actual menu
		
		DestroyMenu (hmenu); // destroy the menu holder
	#endif
	} /*disposemenu*/


hdlmenu getmenuhandle (short id) {

	#ifdef MACVERSION
		return (GetMenuHandle (id));
	#endif

	#if WIN95VERSION
		MENUITEMINFO info;
		HMENU menubar = GetMenu (shellframewindow);
		
		info.cbSize = sizeof (info);
		info.fMask = MIIM_SUBMENU; 
		
		GetMenuItemInfo (menubar, id, false, &info);

		return (info.hSubMenu);
	#endif
	} /*getmenuhandle*/


boolean insertmenu (hdlmenu hmenu, long idbefore) {
	
	#ifdef MACVERSION
		if (idbefore == insertatend)
			idbefore = 0;
		
		InsertMenu (hmenu, idbefore);

		return (true);
	#endif

	#if WIN95VERSION
		MENUITEMINFO info;
		HMENU menubar = GetMenu (shellframewindow);
		char menuname[128];
		boolean fl;
		DWORD err;

		if (idbefore == insertsubmenu) // under windows, hierarchicals aren't inserted into the menubar
			return (true);

		info.cbSize = sizeof (info);
		info.fMask = MIIM_TYPE | MIIM_SUBMENU | MIIM_ID;
		info.dwTypeData = menuname;
		info.cch = 128;
		
 		GetMenuItemInfo (hmenu, 0, true, &info); // get the actual popup and all of it's info
		
		addaccelerators (info.hSubMenu, info.wID, 1);
		
		if (idbefore == insertatend) {
			
			fl = InsertMenuItem (menubar, 99, true, &info);
		//	fl = AppendMenu (menubar, MF_POPUP, (UINT)info.hSubMenu, info.dwTypeData);
			}
		else {
		
			fl = InsertMenuItem (menubar, idbefore, false, &info);
			}
		
		if (!fl)
			err = GetLastError();

	//	drawmenubar ();

		return (fl);
	#endif
	} /*insertmenu*/


boolean inserthierarchicmenu (hdlmenu hmenu, short idmenu) {
	
	#ifdef MACVERSION
		InsertMenu (hmenu, insertsubmenu);

		return (true);
	#endif

	#if WIN95VERSION
		HMENU menubar = GetMenu (shellframewindow);
		MENUITEMINFO info;
		
	//	hmenu = GetSubMenu (hmenu, 0); // get actual popup
		info.cbSize = sizeof (info);
		info.fMask = MIIM_SUBMENU | MIIM_ID;
		
 		GetMenuItemInfo (hmenu, 0, true, &info); // get the actual popup and all of it's info
		
		addaccelerators (info.hSubMenu, info.wID, 1);

	//	info.fMask = MIIM_SUBMENU; 
		
		return (SetMenuItemInfo (menubar, idmenu, false, &info));
	#endif
	} /*inserthierarchicmenu*/


void removemenu (short idmenu) {
	
	#ifdef MACVERSION
		DeleteMenu (idmenu); // (**hmenu).menuID
	#endif

	#if WIN95VERSION
		HMENU menubar = GetMenu (shellframewindow);
		
		removeaccelerators (idmenu, 1);
		
		RemoveMenu (menubar, idmenu, MF_BYCOMMAND);
		
		updateaccelerators ();
	#endif
	} /*removemenu*/


#if MACVERSION && !flruntime

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

#endif

boolean sethierarchicalmenuitem (hdlmenu hmenu, short itemnumber, hdlmenu hsubmenu, short idsubmenu) {

	#ifdef MACVERSION
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
	#endif

	#ifdef WIN95VERSION

		MENUITEMINFO info;
		
		info.cbSize = sizeof (info);
		info.fMask = MIIM_SUBMENU; 
		info.hSubMenu = GetSubMenu (hsubmenu, 0); // get actual submenu
		
		hmenu = GetSubMenu (hmenu, 0); // get actual popup

		return (SetMenuItemInfo (hmenu, itemnumber - 1, true, &info));
	#endif
	} /*sethierarchicalmenuitem*/


boolean gethierarchicalmenuitem (hdlmenu hmenu, short ixmenu, hdlmenu *hsubmenu) {

	#ifdef MACVERSION
		
		short cmdchar;
		short idsub;
		
		GetItemCmd (hmenu, ixmenu, &cmdchar);
		
		if (cmdchar == hMenuCmd) {
			
			GetItemMark (hmenu, ixmenu, &idsub);
			
			*hsubmenu = GetMenuHandle (idsub);
			}
	#endif

	#ifdef WIN95VERSION

		MENUITEMINFO info;
		
		info.cbSize = sizeof (info);
		info.fMask = MIIM_SUBMENU; 
		
		hmenu = GetSubMenu (hmenu, 0); // get actual popup

		GetMenuItemInfo (hmenu, ixmenu - 1, true, &info);

		*hsubmenu = info.hSubMenu;

	#endif

	return (*hsubmenu != NULL);
	} /*gethierarchicalmenuitem*/


void setmenutitleenable (hdlmenu hmenu, short idmenu, boolean fl) {
	
	/*
	enable or disable a menu or a menu item.  if fl is true we enable the item,
	if false we disable it.
	
	if item == 0 we enable or disable the entire menu.
	
	dmb 8/1/90:  check for dummy items (negative item numbers)
	*/
	//Code change by Timothy Paustian Friday, June 9, 2000 9:44:07 PM
	//Changed to Modern calls for Enabling and Disabling menu items
	#ifdef MACVERSION
		if (fl)
			#if ACCESSOR_CALLS_ARE_FUNCTIONS == 1
			EnableMenuItem (hmenu, 0);
			#else
			EnableItem (hmenu, 0);
			#endif
		else
			#if ACCESSOR_CALLS_ARE_FUNCTIONS == 1
			DisableMenuItem (hmenu, 0);
			#else
			DisableItem (hmenu, 0);
			#endif
	#endif

	#if WIN95VERSION
		HMENU menubar = GetMenu (shellframewindow);

		assert (idmenu >= 0);
		
		EnableMenuItem (menubar, idmenu, MF_BYCOMMAND | (fl? MF_ENABLED : MF_DISABLED));
	#endif
	} /*setmenutitleenable*/
	
	
void setmenuitemenable (hdlmenu hmenu, short item, boolean fl) {
	
	/*
	enable or disable a menu or a menu item.  if fl is true we enable the item,
	if false we disable it.
	
	if item == 0 we enable or disable the entire menu.
	
	dmb 8/1/90:  check for dummy items (negative item numbers)
	*/
	
	#ifdef MACVERSION
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
	#endif

	#if WIN95VERSION
		if (item < 0) /*this item has been dummied out -- do nothing*/
			return;
		
		hmenu = GetSubMenu (hmenu, 0); // get actual popup

		EnableMenuItem (hmenu, item - 1, MF_BYPOSITION | (fl? MF_ENABLED : MF_GRAYED));
	#endif
	} /*setmenuitemenable*/
	
	
boolean getmenutitleenable (hdlmenu hmenu, short idmenu) {
	
	/*
	*/
	
	#ifdef MACVERSION
		#if ACCESSOR_CALLS_ARE_FUNCTIONS == 1
		return IsMenuItemEnabled(hmenu, 0);
		#else
		//old code
		return (((**hmenu).enableFlags & (0x01)) != 0);
		#endif
	#endif

	#if WIN95VERSION
		MENUITEMINFO info;
		HMENU menubar = GetMenu (shellframewindow);
		
		info.cbSize = sizeof (info);
		info.fMask = MIIM_STATE; 

		GetMenuItemInfo (menubar, idmenu, false, &info);

		return (info.fState | MFS_ENABLED != 0);
	#endif
	} /*getmenuitemenable*/
	
	
boolean getmenuitemenable (hdlmenu hmenu, short item) {
	
	#ifdef MACVERSION
		assert (item >= 0);
		//Code change by Timothy Paustian Monday, May 1, 2000 9:16:32 PM
		//Changed to Opaque call for Carbon
		#if ACCESSOR_CALLS_ARE_FUNCTIONS == 1
		return IsMenuItemEnabled(hmenu, item);
		#else
		//old code
		return (((**hmenu).enableFlags & (0x01 << item)) != 0);
		#endif
	#endif

	#ifdef WIN95VERSION
		MENUITEMINFO info;
		
		info.cbSize = sizeof (info);
		info.fMask = MIIM_STATE; 

		hmenu = GetSubMenu (hmenu, 0); // get actual popup

		GetMenuItemInfo (hmenu, item - 1, true, &info);

		return (info.fState | MFS_ENABLED != 0);
	#endif
	} /*getmenuitemenable*/
	
	
void disablemenuitem (hdlmenu hmenu, short item) {
	
	setmenuitemenable (hmenu, item, false);
	} /*disablemenuitem*/

void enablemenuitem (hdlmenu hmenu, short item) {
	
	setmenuitemenable (hmenu, item, true);
	} /*enablemenuitem*/


short countmenuitems (hdlmenu hmenu) {
	
	#ifdef MACVERSION
		return (CountMenuItems (hmenu));
	#endif

	#ifdef WIN95VERSION
		hmenu = GetSubMenu (hmenu, 0); // get actual popup

		return (GetMenuItemCount (hmenu));
	#endif
	} /*countmenuitems*/


void enableallmenuitems (hdlmenu hmenu, boolean fl) {
	
	register short lastitem;
	register short i;
	
	lastitem = countmenuitems (hmenu);
	
	for (i = 1; i <= lastitem; i++) 
		setmenuitemenable (hmenu, i, fl);
	} /*enableallmenuitems*/
	

void hilitemenu (short id) {

	#ifdef MACVERSION
		HiliteMenu (id);
	#endif
	} /*hilitemenu*/


void checkmenuitem (hdlmenu hmenu, short ixmenu, boolean fl) {
	
	/*
	5.0a23 dmb: preserve other fState bits [Win]
	*/
	
	#ifdef MACVERSION
		//Code change by Timothy Paustian Friday, June 9, 2000 9:44:37 PM
		//Changed to Modern call.
		CheckMenuItem (hmenu, ixmenu, fl);
	#endif

	#if WIN95VERSION
		MENUITEMINFO info;
		
		hmenu = GetSubMenu (hmenu, 0); // get actual popup

		info.cbSize = sizeof (info);
		info.fMask = MIIM_STATE;

		GetMenuItemInfo (hmenu, ixmenu - 1, true, &info);
		
		if (fl)
			info.fState |= MFS_CHECKED;
		else
			info.fState &= ~MFS_CHECKED;

		SetMenuItemInfo (hmenu, ixmenu - 1, true, &info);
	#endif
	} /*checkmenuitem*/
	
	
boolean menuitemmarked (hdlmenu hmenu, short ixmenu) {
	
	#ifdef MACVERSION
		short mark = 0;
		
		GetItemMark (hmenu, ixmenu, &mark);
		
		return (mark != 0);
	#endif

	#if WIN95VERSION
		MENUITEMINFO info;
		
		info.cbSize = sizeof (info);
		info.fMask = MIIM_STATE; 
		
		hmenu = GetSubMenu (hmenu, 0); // get actual popup

		GetMenuItemInfo (hmenu, ixmenu - 1, true, &info);

		return ((info.fState & MFS_CHECKED) != 0);

	#endif
	} /*menuitemmarked*/


void markmenuitem (hdlmenu hmenu, short ixmenu, short mark) {
	
	#ifdef MACVERSION

		/*
		5.0a23 dmb: preserve other fState bits [Win]
		*/

		SetItemMark (hmenu, ixmenu, mark);
	#endif

	#if WIN95VERSION
		MENUITEMINFO info;
		
		hmenu = GetSubMenu (hmenu, 0); // get actual popup

		info.cbSize = sizeof (info);
		info.fMask = MIIM_STATE;

		GetMenuItemInfo (hmenu, ixmenu - 1, true, &info);
		
		if (mark)
			info.fState |= MFS_CHECKED;
		else
			info.fState &= ~MFS_CHECKED;

		// *** need to set hbmpChecked for non-checkmarks
		
		SetMenuItemInfo (hmenu, ixmenu - 1, true, &info);
	#endif
	} /*markmenuitem*/


void stylemenuitem (hdlmenu hmenu, short ixmenu, short style) {
	
	#ifdef MACVERSION
		SetItemStyle (hmenu, ixmenu, style);
	#endif

	#if WIN95VERSION
		
	#endif
	} /*stylemenuitem*/
	

#ifdef WIN95VERSION
static short matchsubmenu (hdlmenu hmenu, hdlmenu hmatchmenu, hdlmenu * hreturn)
	{
	short cnt, i, ret;
	hdlmenu hm;

	cnt = GetMenuItemCount (hmenu);

	for (i = 0; i< cnt; i++)
		{
		hm = GetSubMenu (hmenu, i);
		if (hm == hmatchmenu)
			{
			*hreturn = hmenu;
			return (i);
			}
		if (hm != NULL)
			{
			ret = matchsubmenu (hm, hmatchmenu, hreturn);
			if (ret >= 0)
				return (ret);
			}
		}

	return (-1);
	} /*matchsubmenu*/
#endif

	
boolean setmenutitle (hdlmenu hmenu, bigstring bs) {
	
	#ifdef MACVERSION
		SetMenuItemText (hmenu, 0, bs);
		
		return (true);
	#endif

	#if WIN95VERSION
		hdlmenu mainmenu, hreturn;
		MENUITEMINFO mi;
		short itempos;

		hmenu = GetSubMenu (hmenu, 0); // get actual popup

		mainmenu = GetMenu (shellframewindow);
		itempos = matchsubmenu (mainmenu, hmenu, &hreturn);
		
		if (itempos == -1)
			return (false);

		mi.cbSize = sizeof (mi);
		mi.fMask = MIIM_ID;

		GetMenuItemInfo (hreturn, itempos, TRUE, &mi);
		
		ModifyMenu (hreturn, itempos, MF_BYPOSITION | MF_STRING, mi.wID, stringbaseaddress(bs));

		return (true);
	#endif
	} /*setmenutitle*/


static boolean getmenutitle (hdlmenu hmenu, bigstring bs) {
	
	#ifdef MACVERSION
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
	#endif

	#if WIN95VERSION
		hdlmenu mainmenu, hreturn;
		MENUITEMINFO info;
		char menuname[128];
		short itempos;

		hmenu = GetSubMenu (hmenu, 0); // get actual popup

		mainmenu = GetMenu (shellframewindow);
		itempos = matchsubmenu (mainmenu, hmenu, &hreturn);
		
		if (itempos == -1)
			return (false);
		
		info.cbSize = sizeof (info);
		info.fMask = MIIM_TYPE;
		info.dwTypeData = &menuname[0];
		info.cch = 128;
		
		GetMenuItemInfo (hreturn, itempos, TRUE, & info);
		
		assert (info.fType == MFT_STRING);

		copyctopstring (info.dwTypeData, bs);

		return (true);
	#endif
	} /*getmenutitle*/


boolean setmenuitem (hdlmenu hmenu, short ixmenu, bigstring bs) {
	
	/*
	4/29/91 dmb: check for empty string

	5.0a23 dmb: maintain accelerator string [Win]
	*/
	
	#ifdef MACVERSION
		if (ixmenu <= 0)
			return (false);
		
		if (isemptystring (bs))
			SetMenuItemText (hmenu, ixmenu, "\p ");
		else
			SetMenuItemText (hmenu, ixmenu, bs);
			
		return (true);
	#endif

	#if WIN95VERSION
		MENUITEMINFO info;
		char item [256];
		short ixaccel = -1;
		
		if (ixmenu <= 0)
			return (false);
		
		// get the item info
		info.cbSize = sizeof (info);
		info.fMask = MIIM_TYPE | MIIM_ID;
		info.dwTypeData = &item[0];
		info.cch = 128;
		
		hmenu = GetSubMenu (hmenu, 0); // get actual popup
		
		GetMenuItemInfo (hmenu, ixmenu - 1, true, &info);
		
		findaccelerator (info.wID, &ixaccel);
		
		if (ixaccel >= 0) { //an accelerator exists. modify the new item text accordingly
			
			ACCEL ac = ((LPACCEL) *haccelerators) [ixaccel];
			
			pushaccelstring (&ac, bs);
			}

		copyptocstring (bs, item);
		
		if (isseparatorstring (bs))
			info.fType = MFT_SEPARATOR;
		else {
		
			info.fType = MFT_STRING;
			
			info.dwTypeData = item;
			}
		
		return (SetMenuItemInfo (hmenu, ixmenu - 1, true, &info));
	#endif
	} /*setmenuitem*/


boolean getmenuitem (hdlmenu hmenu, short ixmenu, bigstring bs) {
	
	#if WIN95VERSION
		MENUITEMINFO info;
		char menuname[128];
	#endif

	if (ixmenu < 0)
		return (false);
	
	if (ixmenu == 0)
		return (getmenutitle (hmenu, bs));

	#ifdef MACVERSION
		GetMenuItemText (hmenu, ixmenu, bs);
		
		return (true);
	#endif

	#if WIN95VERSION
		info.cbSize = sizeof (info);
		info.fMask = MIIM_TYPE;
		info.dwTypeData = &menuname[0];
		info.cch = 128;
		
		hmenu = GetSubMenu (hmenu, 0); // get actual popup

		GetMenuItemInfo (hmenu, ixmenu - 1, true, &info);
		
		assert (info.fType == MFT_STRING);

		copyctopstring (info.dwTypeData, bs);

		return (true);
	#endif
	} /*getmenuitem*/


boolean setmenuitemcommandkey (hdlmenu hmenu, short ixmenu, short key) {

#ifdef MACVERSION
	SetItemCmd (hmenu, ixmenu, key);

	return (true);
#endif

#ifdef WIN95VERSION
	MENUITEMINFO info;
	char menuname[128];
	ACCEL accel;
	LPACCEL paccelerators;
	short ixaccel = -1;
	bigstring bs;

	if (ixmenu < 0)
		return (false);
	
	// get the item info
	info.cbSize = sizeof (info);
	info.fMask = MIIM_TYPE | MIIM_ID;
	info.dwTypeData = &menuname[0];
	info.cch = 128;
	
	hmenu = GetSubMenu (hmenu, 0); // get actual popup
	
	GetMenuItemInfo (hmenu, ixmenu - 1, true, &info);
	
	if (info.cch == 0)
		setemptystring (bs);
	else
		copyctopstring (info.dwTypeData, bs);
	
	// set up the accel structure
	setupaccelrecord (key, info.wID, &accel);
	
	// search for a existing accelerator
	findaccelerator (info.wID, &ixaccel);

	paccelerators = (LPACCEL) *haccelerators;
	
	// if it an accelerator exists, change it in place. or remove it
	if (ixaccel >= 0) {

		if (paccelerators [ixaccel].key == key)
			; // it isn't changing, we may still need to added accel item text
		
		else {
			
			if (key > 0) {	
				// we're setting an accelerator, change it
				paccelerators [ixaccel] = accel;
				
				++ixaccel;
				
				if (paccelerators [ixaccel].cmd == info.wID) { // 2nd copy, w/shift
					
					paccelerators [ixaccel] = accel;
					
					paccelerators [ixaccel].fVirt |= FSHIFT;
					}
				}
			
			else {			
				// remove the existing one
				pullfromhandle (haccelerators, ixaccel * sizeof (ACCEL), sizeof (ACCEL), NULL);
				
				if (paccelerators [ixaccel].cmd == info.wID) // 2nd copy, w/shift
					pullfromhandle (haccelerators, ixaccel * sizeof (ACCEL), sizeof (ACCEL), NULL);
				}
			}
		}
	// otherwise, add the new accelerator to the table and the item text
	else {
		// if we're not adding an accelerator, we're done
		if (key == 0)
			return (true);
		
		// add new item to the accelerator table
		if (!insertaccelerator (&accel))
			return (false);
		}

	// put the changed table and menu item into effect
	updateaccelerators ();

	// add the key to the menu item text
	pushaccelstring (&accel, bs);

	copyptocstring (bs, info.dwTypeData);
	
	return (SetMenuItemInfo (hmenu, ixmenu - 1, true, &info));
#endif
	} /*setmenuitemcommandkey*/


void getmenuitemcommandkey (hdlmenu hmenu, short ixmenu, short *key) {

	#ifdef MACVERSION
		GetItemCmd (hmenu, ixmenu, key);
	#endif

	#ifdef WIN95VERSION
		MENUITEMINFO info;
		short ixaccel;
		
		*key = 0; // default return
		
		// get the item info
		info.cbSize = sizeof (info);
		info.fMask = MIIM_ID;
		
		hmenu = GetSubMenu (hmenu, 0); // get actual popup
		
		GetMenuItemInfo (hmenu, ixmenu - 1, true, &info);
		
		if (findaccelerator (info.wID, &ixaccel)) {
			
			ACCEL ac = ((LPACCEL) *haccelerators) [ixaccel];
			
			if (ac.fVirt & FVIRTKEY)
				*key = MapVirtualKey (ac.key, 2) & 0x00ff;
			else
				*key = ac.key;
			}
	#endif
	} /*getmenuitemcommandkey*/


#if !flruntime	
//Code change by Timothy Paustian Saturday, April 29, 2000 9:33:29 PM
//Changed for UH 3.3.1 to avoid conflict with Menus.h
boolean Insertmenuitem (hdlmenu hmenu, short ixmenu, bigstring bs) {
	
	/*
	3/26/91 dmb: added logic to handle meta-characters
	
	12/31/96 dmb: added idmenu parameter. under windows, the command
	for the new menu has the menu id in the high byte, the item number
	in the low byte
	*/
	
#ifdef MACVERSION
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
#endif

#ifdef WIN95VERSION
	MENUITEMINFO info;
	char item [256];
	short idmenu;
	boolean fl;
	
	info.cbSize = sizeof (info);
	info.fMask = MIIM_SUBMENU | MIIM_ID;
	
 	GetMenuItemInfo (hmenu, 0, true, &info); // get the actual popup and it's id
	
	hmenu = info.hSubMenu;
	
	idmenu = info.wID;
	
	assert ((idmenu % 100) == 0);
	
	removeaccelerators (idmenu, ixmenu);
	
	info.wID += ixmenu;

	info.fMask = MIIM_TYPE | MIIM_ID;
	
	if (isseparatorstring (bs))
		info.fType = MFT_SEPARATOR;
	
	else {
		copyptocstring (bs, item);
		
		info.fType = MFT_STRING;
		info.dwTypeData = item;
		}
	
	fl = InsertMenuItem (hmenu, ixmenu - 1, true, &info);
	
	adjustitemcommands (hmenu, idmenu, ixmenu);

	addaccelerators (hmenu, idmenu, ixmenu);
	
	return (fl);
#endif
	} /*insertmenuitem*/


boolean deletemenuitem (hdlmenu hmenu, short ixmenu) {

#ifdef MACVERSION
	DeleteMenuItem (hmenu, ixmenu);
	
	return (true);
#endif

#if WIN95VERSION	
	MENUITEMINFO info;
	boolean fl;
	
//	hmenu = GetSubMenu (hmenu, 0); // get actual popup
	info.cbSize = sizeof (info);
	info.fMask = MIIM_SUBMENU | MIIM_ID;
	
 	GetMenuItemInfo (hmenu, 0, true, &info); // get the actual popup and all of it's info
	
	removeaccelerators (info.wID, ixmenu);
	
	fl = RemoveMenu (info.hSubMenu, ixmenu - 1, MF_BYPOSITION);
	
	adjustitemcommands (info.hSubMenu, info.wID, ixmenu);

	addaccelerators (info.hSubMenu, info.wID, ixmenu);
	
	return (fl);
#endif
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

#endif

boolean pushmenuitem (hdlmenu hmenu, short idmenu, bigstring bs, short commandid) {
	
	/*
	3/26/91 dmb: added logic to handle meta-characters
	
	12/31/96 dmb: added idmenu parameter. under windows, the command
	for the new menu has the menu id in the high byte, the item number
	in the low byte

	7.1b23 PBS: new parameter commandid allows one to specify the commandid.
	If it's < 1, then calculate the commandid.
	*/
	
#ifdef MACVERSION
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
#endif

#if WIN95VERSION
	boolean fl;
	
	hmenu = GetSubMenu (hmenu, 0); // get actual popup
	
	if (isseparatorstring (bs))
		fl = AppendMenu (hmenu, MF_SEPARATOR, 0, NULL);
	
	else {
		short command = idmenu + GetMenuItemCount (hmenu) + 1;
		char item [256];

		if (commandid > 0)			/*7.1b23 PBS: allow caller to specify commandid*/
			command = commandid;
		
		copyptocstring (bs, item);

		fl = AppendMenu (hmenu, MF_STRING, command, item);
		}

	return (fl);
#endif
	} /*pushmenuitem*/


boolean pushresourcemenuitems (hdlmenu hmenu, short idmenu, OSType restype) {

#ifdef MACVERSION
	//Code change by Timothy Paustian Wednesday, June 28, 2000 4:27:19 PM
	//You can't do this in carbon.
	//#if !TARGET_API_MAC_CARBON	
	AppendResMenu (hmenu, restype);
	//#endif
		
	return (true);
#endif

#if WIN95VERSION
	if (restype == 'FONT') {

		short i;
		bigstring bsfont;
		extern short globalFontCount;
		
		deleteallmenuitems (hmenu, 0);

		for (i = 1; i <= globalFontCount; ++i) {

			fontgetname (i, bsfont);

			pushmenuitem (hmenu, idmenu, bsfont, 0);
			}
		}

	return true;
#endif
	} /*pushresourcemenuitems*/


#if !flruntime
	
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

#endif

boolean initmenusystem (void) {

	#ifdef WIN95VERSION
		/*
		creator an in-memory version of the oringal, resource-loaded
		accelerator table. this allows us to modify it more easily.
		*/

		int ctaccelerators;

		ctaccelerators = CopyAcceleratorTable (hAccel, NULL, 0); 
		
		if (!newhandle (ctaccelerators * sizeof (ACCEL), &haccelerators))
			return (false);
		
		CopyAcceleratorTable (hAccel, (LPACCEL) *haccelerators, ctaccelerators);
		
		lockhandle (haccelerators);

		hAccel = CreateAcceleratorTable ((LPACCEL) *haccelerators, ctaccelerators);

		unlockhandle (haccelerators);
	#endif

	return (true);
	} /*initmenusystem*/




