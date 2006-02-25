
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
	#include "langipc.h"
#endif

#include "cursor.h"
#include "memory.h"
#include "kb.h"
#include "launch.h"
#include "menu.h"
#include "ops.h"
#include "strings.h"
#include "shellmenu.h"
#include "menubar.h"
#include "process.h"
#include "meprograms.h"
/*
12/27/96 dmb: windows port. use menu.c routines exclusively.
*/


/*
id of menu at stays to right of all custom menus.  set to zero if no main 
menu stays to the right
*/
#ifdef MACVERSION
	#define rightmainmenu windowsmenu
#endif

#ifdef WIN95VERSION
	static long rightmainmenu = -2;
#endif

/*
rules for resource numbers -- all menu ids generated here must be no greater 
than 255 -- since the hierarchic links in the Mac menu manager are stored as
a character.  Actaully, the range is further limited to 0-235; the rest are 
reserved for desk accessories. Plus, Frontier uses 128 - 133 or so. So we'll 
us 136 - 235. For menu sharing, a different base menu id can be pass in.
*/
#ifdef MACVERSION
	#define defaultbasemenuid (lasthiermenu + hiermenuincrement)
#endif

#ifdef WIN95VERSION
	/*
	2006-02-25 aradke: on windows, the menu with the highest id can be
		a main menu or a hierarchic (sub)menu. deal with it.
	*/
	#define defaultbasemenuid ((max(lastmainmenu, lasthiermenu) / hiermenuincrement) + 1)
#endif

hdlmenubarlist menubarlist = nil;

tymenubarcallbacks menubarcallbacks = {nil, nil};


static hdlmenubarstack menubardata = nil;

static boolean fldirtymenubar = false;


#define ctmenubarglobalsstack 3 /*we can remember menubar contexts up to 3 levels deep*/

static short topmenubarglobals = 0;

static hdlmenubarstack menubarglobalsstack [ctmenubarglobalsstack];




#ifdef WIN95VERSION

	static boolean BitTst (char *x, long n) {

		x += n / 8;

		return ((*x & (0x0080 >> (n % 8))) != 0);
		} /*BitTst*/


	static void BitSet (char *x, long n) {

		x += n / 8;

		*x |= (0x0080 >> (n % 8));
		} /*BitTst*/


	static void BitClr (char *x, long n) {

		x += n / 8;

		*x &= ~(0x0080 >> (n % 8));
		} /*BitTst*/

#endif


boolean pushmenubarglobals (hdlmenubarstack hstack) {
	
	/*
	4/22/91 dmb: this routine replaces the old mesetglobals calls; it 
	establishes the indicated menubar stack as the current one, and 
	sets up the corresponding outline globals.
	
	with the addition of menubaroutline to the menubarstack structure, and 
	the use of this routine, we are now reasonable ignorant of menudata
	
	4/21/93 dmb: two years later... make this a true push/pop, up to 3 levels
	*/
	
	assert (topmenubarglobals < ctmenubarglobalsstack);
	
	menubarglobalsstack [topmenubarglobals++] = menubardata;
	
	menubardata = hstack;
	
	if (hstack != nil)
		oppushoutline ((**hstack).menubaroutline);
	
	return (true);
	} /*pushmenubarglobals*/


boolean popmenubarglobals (void) {
	
	if (menubardata != nil)
		oppopoutline ();
	
	assert (topmenubarglobals > 0);
	
	menubardata = menubarglobalsstack [--topmenubarglobals];
	
	return (true);
	} /*popmenubarglobals*/


static boolean pushmenubarlist (hdlmenubarstack hstack) {
	
	/*
	link the indicated menubar into the menubarlist
	*/
	
	return (listlink ((hdllinkedlist) menubarlist, (hdllinkedlist) hstack));
	} /*pushmenubarlist*/


static boolean popmenubarlist (hdlmenubarstack hstack) {
	
	/*
	unlink the indicated menubar from the menubarlist
	*/
	
	return (listunlink ((hdllinkedlist) menubarlist, (hdllinkedlist) hstack));
	} /*popmenubarlist*/


static boolean meallocmenuid (short *id) {
	
	/*
	12/27/96 dmb: windows port
	*/

	register short i;
	register hdlmenubarlist hlist = menubarlist;
	register byte *pbitmap;
	boolean	flFound = false;
	#ifdef MACVERSION
		//9/1/00 Timothy Paustian
		//got rid of nasty use of direct memory access.
		//not allowed in carbon
		#if TARGET_API_MAC_CARBON == 1
		MenuBarHandle MenuList = nil;
		MenuList = GetMenuBar();
		#else
		#define MenuList (*(Handle *)0xA1C)
		#endif		
	#endif

	if (hlist == nil)
		return (false);
	
	pbitmap = (**hlist).menubitmap;
	
	for (i = 0; i < maxmenus; i++) {
		
		if (!BitTst (pbitmap, i)) { /*found one not in use*/
			
			BitSet (pbitmap, i);
			
			*id = (**hlist).basemenuid + i;
			
			#ifdef MACVERSION
				if ((MenuList != nil) && (GetMenuHandle (*id) != nil)) /*2.1a6 dmb: in use by someone!*/
					continue;
			#endif
			
			#ifdef WIN95VERSION
				*id *= 100; // 4.18.97 dmb: leave room for 99 items with their own, sequenced ids
			#endif
			
			flFound = true;
			break;
			}
		} /*for*/
	#if MACVERSION && TARGET_API_MAC_CARBON
		//Code change by Timothy Paustian Tuesday, September 5, 2000 9:27:35 PM
		//Only dispose of in carbon
		if(MenuList != nil)
			DisposeMenuBar(MenuList);
	#endif
	return (flFound); /*all menu ids are in use*/
	} /*meallocmenuid*/


static void mefreemenuid (short id) {
	
	register hdlmenubarlist hlist = menubarlist;
	
	#ifdef WIN95VERSION
		id /= 100;
	#endif

	if ((hlist != nil) && (id > 0))
		BitClr ((**hlist).menubitmap, id - (**hlist).basemenuid);
	} /*mefreemenuid*/


static void meremovemenu (const tymenubarstackelement *menuinfo) {

	if ((*menuinfo).flbuiltin)
		deleteallmenuitems ((*menuinfo).hmenu, (*menuinfo).ctbaseitems);
	
	else
		removemenu ((*menuinfo).idmenu);
	} /*meremovemenu*/


static short medeletemenu (hdlmenubarstack hstack, short ix, boolean flactive) {
	
	/*
	delete the menubarstack element at index ix.
	
	xxx 2.1b5 dmb: for some reason, on my machine (IIfx, 7.1), the code for 
	DisposeMenu seems to be patched, event when booting w/extension disabled. 
	the patched code always calls ReleaseResource, while the ROM code makes 
	sure it's a resource first. The menus we dispose of here are never made 
	from resources, and this is resuling in a hang in ReleaseResource under 
	certain conditions. To easier way to avoid the problem is to just dispose 
	the handle instead of calling DisposeMenu.
	
	update: I think it's a Talking Moose conflict, and affects much more than 
	just this. bleah!
	*/
	
	tymenubarstackelement *pitem = (**hstack).stack + ix;
	
	if ((*pitem).idmenu != -1) { /*a menu is allocated*/

		if (flactive)
			meremovemenu (pitem);
			
		if (!(*pitem).flbuiltin)
			disposemenu ((*pitem).hmenu);
		}
	
	return ((*pitem).idmenu);
	} /*medeletemenu*/


static void medeletemenubarentry (short ix) {
	
	/*
	delete the menubarstack element at index ix.
	
	1/24/91 dmb: adjust ixdeletedmenu if it slides down
	*/
	
	register hdlmenubarstack hs = menubardata;
	register short i;
	register short topstack;
	register short idmenu;
	
	idmenu = medeletemenu (hs, ix, true);
	
	topstack = --(**hs).topstack; /*one less item in the stack*/
	
	for (i = ix; i < topstack; i++) /*slide all elements to the right of ix down*/
		(**hs).stack [i] = (**hs).stack [i + 1];
	
	if ((**hs).ixdeletedmenu > ix)
		--(**hs).ixdeletedmenu;
	
	mefreemenuid (idmenu); /*allow the menu id to be re-used*/
	} /*medeletemenubarentry*/


static boolean mepushmenubarstack (const tymenubarstackelement *pitem) {
	
	register hdlmenubarstack hs = menubardata;
	
	if ((**hs).topstack >= ctmenubarstack) /*too many menus*/
		return (false);
	
	(**hs).stack [(**hs).topstack++] = *pitem; /*assign and bump stack top*/
	
	return (true);
	} /*mepushmenubarstack*/


void medirtymenubar (void) {
	
	fldirtymenubar = true;
	
	shellforcemenuadjust ();
	} /*medirtymenubar*/


void meupdatemenubar (void) {
	
	drawmenubar ();
	
	fldirtymenubar = false;
	} /*meupdatemenubar*/


void mecheckmenubar (void) {
	
	if (fldirtymenubar)
		meupdatemenubar ();
	} /*mecheckemenubar*/


static boolean mefindinmenubar (hdlheadrecord hnode, boolean flfindtitle, short *ixstack, short *itemnumber) {
	
	/*
	search the menubar data structure for the indicated outline node.  effectively
	maps a node onto the machine's menubar data structure.
	
	return an index into the menubar stack, and the item number in that menu for
	the item.  if the item number is 0, then the node generated the entire menu.
	
	if flallowhierarchic is false then we return the menu item for a hierarchic
	menu, not the menu itself.  think about it -- when you're editing text you want
	to edit the menu item that stands for the hierarchic menu, not the title of
	the hierarchic menu (which isn't even displayed anywhere!)...
	
	this is necessary because hierarchic menus have dual lives -- they are menus
	but they also appear as menu items.
	
	6/25/90 dmb:  replaced flallowhierarchic with flfindtitle.  now, searching for 
	titles vs. items is explicit and mutually exclusive
	
	2.1b3 dmb: never traverse the outline structure of the deleted item

	7.0b30 PBS: check for nil handle -- the menu outline may have changed behind our back.
	*/
	
	register short i, ct;
	register hdlheadrecord h, hnext;
	register short topstack;
	register hdlmenubarstack hs = menubardata;
	
	topstack = (**hs).topstack;
	
	for (i = 0; i < topstack; i++) {
		
		h = (**hs).stack [i].hnode;

		if (flfindtitle) {
		
			if (h == hnode) { /*found match -- it's a whole menu*/

				*ixstack = i;
			
				*itemnumber = 0; /*indicate the menu itself*/
			
				return (true);
				}
			}
		else {
			
			if (i == (**hs).ixdeletedmenu) /*2.1b3: deleted, outline structure isn't valid*/
				continue;
			
			ct = 1; /*indicate the first item in the menu*/
			
			hnext = (**h).headlinkright; /*start with the first sub*/
			
			while (h != hnext) { /*haven't reached the end of the menu outline*/
				
				if (hnext == nil) /*7.0b30 PBS: defensive driving -- the menu outline may have changed behind our back*/
					break;

				h = hnext;
				
				if (h == hnode) { /*found match on one of the menu items*/
					
					*ixstack = i;
					
					*itemnumber = ct + (**hs).stack [i].ctbaseitems;
					
					return (true);
					}
				
				ct++;

				hnext = (**h).headlinkdown;
				} /*while*/
			}
		} /*for*/
		
	return (false); /*search failed*/
	} /*mefindinmenubar*/


static boolean mebuildmenu (hdlheadrecord, boolean, tymenubarstackelement *, boolean);

static void meactivatemenus (hdlmenubarstack hstack, boolean flactivate) {
	
	/*
	5.0b9 dmb: for builtin menus, call meremovemenu to remove the items.
	when activating, rebuild the source node to recreate the main menu
	items. submenus are still in the stack, so we don't want to recurse.
	*/

	register short i;
	register short topstack;
	register hdlmenubarstack hs = hstack;
	tymenubarstackelement item;
	hdlheadrecord hnode;
	short ixstack;
	short itemnumber;
	
	if (hs == nil)
		return;
	
	if ((**hs).flactive == flactivate)
		return;
	
	pushmenubarglobals (hs);
	
	if (flactivate) { /*insert main menus in outline order*/
		
		opoutermostsummit (&hnode);
		
		while (true) {
			
			if (mefindinmenubar (hnode, true, &ixstack, &itemnumber)) {

				item = (**hs).stack [ixstack];

				if (item.flbuiltin)
					mebuildmenu (item.hnode, false, &item, false);
				else
					insertmenu (item.hmenu, rightmainmenu);
				}
			
			if (opislastsubhead (hnode)) /*all the menus are loaded*/
				break;
			
			hnode = (**hnode).headlinkdown; /*advance to next menu definition*/
			}
		}
	
	topstack = (**hs).topstack;
	
	for (i = 0; i < topstack; i++) {
		
		item = (**hs).stack [i];
		
	//	if (item.flbuiltin)
	//		continue;

		if (item.idmenu != -1)
		
			if (flactivate) {
				
				if (item.flhierarchic)
					inserthierarchicmenu (item.hmenu, item.idmenu);
				}
			else
				meremovemenu (&item);
		}
	
	(**hs).flactive = flactivate;
	
	medirtymenubar ();
	
	popmenubarglobals ();
	} /*meactivatemenus*/


static void medisposemenus (hdlmenubarstack hstack) {
	
	/*
	11/4/90 DW: save and restore flactive around call to clearhandle.  this was 
	breaking the menu.buildmenubar () verb.  this is the stupid way to do it,
	but the guy who put this in should have checked that the buildmenubar verb
	was still working.  (1/21/91 dmb: who, me?)
	
	1/21/91 dmb: use mefreemenuid instead of clearbytes, which can wipe 
	out menus that belong to another menubar stack
	*/
	
	register short i;
	register short topstack;
	register hdlmenubarstack hs;
	register boolean flactive;
	
	hs = hstack; /*move into register*/
	
	if (hs == nil)
		return;
	
	flactive = (**hs).flactive;
	
	topstack = (**hs).topstack;
	
	for (i = 0; i < topstack; i++)
		mefreemenuid (medeletemenu (hs, i, flactive));
	
	(**hs).topstack = 0;
	
	(**hs).ixdeletedmenu = -1;
	
	if (flactive)
		medirtymenubar ();
	} /*medisposemenus*/


static boolean memainmenunode (hdlheadrecord hnode) {

	/*
	return true if hnode correspondes to a main (as opposed to 
	hierarchical) menu title; that is, if it is among the first level 
	of subheads below the menubar layer's summit.

	7/14/90 DW to DMB: add commentary, use parens in tests, no buried assignments,
	one symbol defined per line, why not use registers?
	
	7/19/90 dmb to DW:  gotcha.  however, can't use register for hmenubar since 
	its address is taken, and using a register for hleftlink would increase code 
	size since it's only referenced twice and no other registers are needed.
	
	10/23/90 dmb: now that layers are gone, this is quite simple
	*/
	
	return ((**hnode).headlinkleft == hnode);
	} /*memainmenunode*/


static byte mecmdkey (hdlheadrecord hnode) {
	
	return ((*menubarcallbacks.getcmdkeyroutine) (hnode));
	} /*mecmdkey*/


static boolean melinkparentitem (hdlheadrecord hnode, hdlmenu hsubmenu, short idsubmenu) {
	
	/*
	set the item mark in the parent menu of hnode's hierarchical menu
	
	5.0a4 dmb: don't set the command key if we just set a hierarchical menu item
	*/
	
	register hdlmenu hmenu;
	short ixstack, itemnumber;
	
	if (!mefindinmenubar (hnode, false, &ixstack, &itemnumber))
		return (false);
	
	hmenu = (**menubardata).stack [ixstack].hmenu; /*move into register*/
	
	sethierarchicalmenuitem (hmenu, itemnumber, hsubmenu, idsubmenu);
	
	if (hsubmenu == nil)
		setmenuitemcommandkey (hmenu, itemnumber, mecmdkey (hnode));
	
	return (true);
	} /*melinkparentitem*/
	
	
static boolean meshouldhavemenu (hdlheadrecord hnode) {
	
	return (ophassubheads (hnode) || memainmenunode (hnode));
	} /*meshouldhavemenu*/


static boolean megetmenuinfo (hdlheadrecord hnode, tymenubarstackelement *menuinfo) {
	
	/*
	hnode should be the title of a menu.  get the id of that menu.
	
	return false if the menu isn't found
	*/
	
	short ixstack;
	short titlenumber;
	
	if (!mefindinmenubar (hnode, true, &ixstack, &titlenumber))
		return (false);
	
	*menuinfo = (**menubardata).stack [ixstack];
	
	return (true);
	} /*megetmenuinfo*/


void mereducemenucodes (bigstring bs, boolean *flenabled, boolean *flchecked) {

	/*
	4.1b8 dmb: common code for mebuildmenu, memenuitemchanged, and mecheckformulas
	
	handles item disabling with '(', and checkmark with '!'
	
	7.0b23 PBS: no longer static -- used in oppopup.c for right-click menus.
	*/
	
	*flenabled = (stringlength (bs) > 1) || (getstringcharacter (bs, 0) != '-');
		// !equalstrings (bs, (ptrstring) "\p-"); /*all dotted lines are disabled*/
	
	if (getstringcharacter (bs, 0) == '(' && lastchar (bs) != ')') { /*4.1b6 dmb*/
	
		deletestring (bs, 1, 1);
		
		*flenabled = false;
		}
	
	*flchecked = false;
	
	if ((stringlength (bs) > 1) && (getstringcharacter (bs, 0) == '!')) { /*4.1b8 dmb*/
	
		deletestring (bs, 1, 1);
		
		*flchecked = true;
		}
	} /*mereducemenucodes*/


boolean mereduceformula (bigstring bs) {
	
	/*
	5.0a2 dmb: must preserve outlinedata in case a formual messes with it. 
	this can happen if the globals of an outline window are push/popped
	
	7.0b12 PBS: no longer static: used by oppopup.c for right-click menus.
	*/
	
	boolean fl;
	hdloutlinerecord ho = outlinedata;
	
	fl = langreduceformula (bs);
	
	opsetoutline (ho);
	
	return (fl);
	} /*mereduceformula*/


static boolean meinsertmenuitem (hdlmenu hmenu, short itemnumber, hdlheadrecord h, boolean flrecurse) {

	boolean flitemenabled, flitemchecked;
	char cmdchar;
	bigstring bs;
	tymenubarstackelement submenuinfo;
	
	cmdchar = 0;
	
	submenuinfo.idmenu = -1; /*force a menu id to be allocated, if we recurse*/

	if (ophassubheads (h)) { /*a hierarchic menu is called for*/
		
		if (flrecurse) {
		
			if (!mebuildmenu (h, true, &submenuinfo, true)) /*recurse*/
				return (false);
			
			if (!mepushmenubarstack (&submenuinfo))
				return (false);
			
			if ((**menubardata).flactive)
				insertmenu (submenuinfo.hmenu, insertsubmenu); /*insert it as a submenu*/
			}
		else {
			
			if (!megetmenuinfo (h, &submenuinfo)) /*didn't find as title of hierarchical menu*/
				return (false);
			}
		} /*has a submenu*/
		
	else { /*a terminal menu item*/
		
		cmdchar = mecmdkey (h);
		
		if (cmdchar == (char) ' ') /*a space is equal to nothing*/
			cmdchar = (char) 0;
		}
	
	getheadstring (h, bs);
	
	mereduceformula (bs); /*filter formulas*/
	
	mereducemenucodes (bs, &flitemenabled, &flitemchecked); /*4.1b8 dmb*/
	
	//pushmenuitem (hmenu, id, bs);

	Insertmenuitem (hmenu, itemnumber, bs);
	
	if (!flitemenabled)
		disablemenuitem (hmenu, itemnumber);
	
	if (submenuinfo.idmenu != -1)
		sethierarchicalmenuitem (hmenu, itemnumber, submenuinfo.hmenu, submenuinfo.idmenu);
	else
		if (flitemchecked)
			checkmenuitem (hmenu, itemnumber, true); /*4.1b8 dmb*/

	if (cmdchar != 0)
		setmenuitemcommandkey (hmenu, itemnumber, cmdchar);
	
	return (true);
	} /*meinsertmenuitem*/


static boolean getmenutobuild (bigstring bsmenu, boolean flhierarchic, short *id, hdlmenu *hmenu, boolean *flbuiltin) {
	
	/*
	5.0a24 dmb: before allocating a new menu, see if we want to 
	build onto a builtin menu
	*/
	
	boolean flinfrontier;

	#ifdef flcomponent
		THz savezone;
		#if TARGET_API_MAC_CARBON == 1
		savezone = LMGetApplZone();
		#else
		 savezone = GetZone ();
		 #endif
		
		#endif
	
	#ifdef MACVERSION
		flinfrontier = iscurrentapplication (langipcself);
	#else
		flinfrontier = true;
	#endif
	
	if (flinfrontier && !flhierarchic && shelltgetmainmenu (bsmenu, hmenu, id)) {
		
		*flbuiltin = true;
		
		#ifdef WIN95VERSION
			// about needs to be last, but resource compiler doesn't allow empty menus
			if (*id == helpmenu)
				deletemenuitem (*hmenu, aboutitem);
		#endif
	
		return (true);
		}
	
	*flbuiltin = false;

	if (*id <= 0) { /*allocate a new menu id*/
		
		if (!meallocmenuid (id))
			return (false);
		}
	
	#ifdef flcomponent
		#if TARGET_API_MAC_CARBON == 1
		//Code change by Timothy Paustian Monday, June 26, 2000 9:29:46 PM
		//This code makes no sense to me.
		LMSetApplZone(LMGetApplZone());
		#else		
		SetZone (ApplicationZone ());
		#endif
		
	#endif
	
	*hmenu = Newmenu (*id, bsmenu);
	
	#ifdef flcomponent
		#if TARGET_API_MAC_CARBON == 1
		LMSetApplZone(savezone);
		#else
		SetZone (savezone);
		#endif
		
		
	#endif
	
	return (hmenu != nil);
	} /*getmenutobuild*/


static boolean mebuildmenu (hdlheadrecord hnode, boolean flhierarchic, tymenubarstackelement *menuinfo, boolean flrecurse) {
	
	/*
	load a menu from the outline pointed to by hnode.  we return a handle to
	the menu data structure and the menu id.
	
	we recurse to handle a menu that has a submenu.
	
	4.1b2 dmb: call Newmenu in the (client) application zone
	
	4.1b6 dmb: if item begins with a paren, and doesn't end with one, disabled it
	*/
	
	hdlmenu hmenu;
	short itemnumber;
	short id;
	bigstring bs;
	boolean flbuiltin;

	rollbeachball (); /*if operative, roll it*/
	
	if (!meshouldhavemenu (hnode))
		return (false);
	
	getheadstring (hnode, bs); /*get the menu title*/
	
	mereduceformula (bs); /*filter formulas*/
	
	id = (*menuinfo).idmenu;
	
	if (!getmenutobuild (bs, flhierarchic, &id, &hmenu, &flbuiltin))
		return (false);

	clearbytes (menuinfo, sizeof (tymenubarstackelement)); // set all field to zero

	(*menuinfo).hnode = hnode;
	
	(*menuinfo).idmenu = id; /*caller gets this returned value*/
	
	(*menuinfo).hmenu = hmenu; /*and this one too*/
	
	(*menuinfo).flenabled = true;
	
	(*menuinfo).flhierarchic = flhierarchic;
	
	(*menuinfo).flbuiltin = flbuiltin;
	
	(*menuinfo).ctbaseitems = countmenuitems (hmenu);
	
	if (opnosubheads (hnode))
		return (true);
	
	hnode = (**hnode).headlinkright; /*move to first subhead*/
	
	for (itemnumber = (*menuinfo).ctbaseitems + 1; ; itemnumber++) {
		
		if (!meinsertmenuitem (hmenu, itemnumber, hnode, flrecurse))
			return (false);
		
		if (opislastsubhead (hnode)) /*done with this menu*/
			return (true);
		
		hnode = (**hnode).headlinkdown; /*advance to next item*/
		} /*while*/
	} /*mebuildmenu*/


boolean newmenubarlist (hdlmenubarlist *hlist) {
		
	if (!newclearhandle (longsizeof (tymenubarlist), (Handle *) hlist))
		return (false);
	
	#ifdef MACVERSION
		assert (defaultbasemenuid + maxmenus < 236);
	#endif
	
	(***hlist).basemenuid = defaultbasemenuid;
	
	return (true);
	} /*newmenubarlist*/


void setcurrentmenubarlist (hdlmenubarlist hlist) {
	
	menubarlist = hlist;
	} /*setcurrentmenubarlist*/


boolean activatemenubarlist (hdlmenubarlist hlist, boolean flactivate) {
	
	register hdlmenubarstack hstack;
	
	if (hlist == nil)
		return (true);
	
	hstack = (**hlist).hfirst;
	
	while (hstack != nil) {
		
		meactivatemenus (hstack, flactivate); /*checks for no change*/
		
		hstack = (**hstack).hnext;
		}
	
	(**hlist).flactive = flactivate;
	
	return (true);
	} /*activatemenubarlist*/


boolean disposemenubarlist (hdlmenubarlist hmenubarlist) {
	
	register hdlmenubarlist hlist = hmenubarlist;
	register hdlmenubarstack h;
	register hdlmenubarstack hnext;
	
	if (hlist == nil) /*empty list*/
		return (true);
	
	h = (**hlist).hfirst;
	
	while (h != nil) {
		
		hnext = (**h).hnext;
		
		medisposemenubar (h);
		
		h = hnext; /*advance to next node*/
		} /*while*/
	
	disposehandle ((Handle) hlist);
	
	if (hlist == menubarlist)
		menubarlist = nil;
	
	return (true);
	} /*disposemenubarlist*/


boolean medisposemenubar (hdlmenubarstack hstack) {
	
	register hdlmenubarstack hs = hstack;
	
	if ((hs != nil) && !(**hs).flclientowned)
		medisposemenus (hstack);
	
	popmenubarlist (hstack);
	
	disposehandle ((Handle) hstack);
	
	return (true);
	} /*medisposemenubar*/


boolean menewmenubar (hdloutlinerecord houtline, hdlmenubarstack *hstack) {
	
	/*
	create a new, empty menubar data structure, linked into menubarlist.

	4.1b2 dmb: xxx - create stack in the (client) application zone
	*/
	
	register hdlmenubarstack hs;
	
	#if 0
	
		THz savezone = GetZone ();
		
		SetZone (ApplicationZone ());
		
		*hstack = (hdlmenubarstack) NewHandleClear (sizeof (tymenubarstack));
		
		SetZone (savezone);
		
		hs = *hstack; /*move into register*/
		
		if (hs == nil) {
		
			memoryerror ();
			
			return (false);
			}
		
	#else
		
		if (!newclearhandle (sizeof (tymenubarstack), (Handle *) hstack))
			return (false);
	
		hs = *hstack; /*move into register*/
		
	#endif
	
	(**hs).menubaroutline = houtline;
	
	(**hs).ixdeletedmenu = -1; /*no halfway deleted menu*/
	
	/*
	if (pushmenubarlist (hs)) /%add it to the end of our list%/
		
		(**hs).flactive = (**menubarlist).flactive;
	*/
	
	return (true);
	} /*menewmenubar*/


boolean mebuildmenubar (hdlmenubarstack hstack) {
	
	/*
	build a menubar out of the summits in the menu outline structure.  each summit
	represents a menu, each sub under each summit is a first-level menu item.
	
	the menubar is a hierarchic structure, we map the outline hierarchy onto a 
	hierarchy in the menubar.
	
	7/14/90 DW: redraw the menubar after disposing of it.  better staging for
	the user when it's being called from the menu.buildmenubar verb.
	
	3/31/91 DW: added param to firstmenuid param.
	
	4/26/91 dmb: removed firstmenuid param; now covered by menubarlist field
	*/
	
	register hdlheadrecord h; 
	hdlheadrecord hnode;
	tymenubarstackelement menuinfo;
	boolean fl = false;
	
	#ifdef WIN95VERSION

	hdlmenu hwindowmenu;
	MENUITEMINFO info;

	hwindowmenu = shellmenuhandle (windowsmenu);

	info.cbSize = sizeof (info);
	info.fMask = MIIM_ID;
	
 	GetMenuItemInfo (hwindowmenu, 0, true, &info); // get the actual popup and all of it's info

	rightmainmenu = info.wID; //GetMenuItemID (hmenu, 0);

	#endif

	pushmenubarglobals (hstack); /*make sure menubardata, outlinedata are set up*/
	
	medisposemenus (hstack); /*get rid of any menus in the stack*/
	
	opoutermostsummit (&hnode);
	
	h = hnode; /*copy into register*/
	
	while (true) {
		
		menuinfo.idmenu = -1; /*allocate a new id*/
		
		if (mebuildmenu (h, false, &menuinfo, true)) { /*no error building the menu*/
			
			if (!mepushmenubarstack (&menuinfo)) 
				break;
		
			if ((**menubardata).flactive)
				if (!menuinfo.flbuiltin)
					insertmenu (menuinfo.hmenu, rightmainmenu); /*add to menubar, after all others*/
			}
		
		if (opislastsubhead (h)) { /*all the menus are loaded*/
			
			medirtymenubar ();
			
			fl = true;
			
			break;
			}
		
		h = (**h).headlinkdown; /*advance to next menu definition*/
		} /*while*/
	
	popmenubarglobals ();
	
	return (fl);
	} /*mebuildmenubar*/


static boolean meinsertmenu (const tymenubarstackelement *menuinfo) {
	
	hdlheadrecord hnode = (*menuinfo).hnode;
	hdlheadrecord hnext;
	tymenubarstackelement nextinfo;
	hdlmenubarstack hnextstack;
	
	if ((*menuinfo).flbuiltin)
		return (true);

	if ((*menuinfo).flhierarchic) 
		nextinfo.idmenu = -1;
	
	else {
		
		hnext = (**hnode).headlinkdown;
		
		if (hnext == hnode) { /*last menu in this menubarstack*/
			
			hnextstack = (**menubardata).hnext;
			
			if (hnextstack == nil) /*it's the last user-defined menu, insert at end of list*/
				nextinfo.idmenu = (short) rightmainmenu;
			
			else {
				
				pushmenubarglobals (hnextstack);
				
				opoutermostsummit (&hnext);
				
				megetmenuinfo (hnext, &nextinfo);
				
				popmenubarglobals ();
				}
			}
		else {
			
			megetmenuinfo (hnext, &nextinfo);
			}
		}
	
	return (insertmenu ((*menuinfo).hmenu, nextinfo.idmenu)); /*add to menubar, insert before idnext*/
	} /*meinsertmenu*/

/*
static void menubarchanged (hdlmenubarstack hstack) {
	
	(*menubarcallbacks.menubarchangedroutine) (hstack);
	} /%menubarchanged%/
*/

boolean memenuitemchanged (hdlmenubarstack hstack, hdlheadrecord hnode) {
	
	/*
	called when the text or command key of hnode has changed.  it saves code
	to have one routine that does both, although usually only on or the 
	other has changed.
	
	returns true if we successfully changed the text of the indicated item.
	*/
	
	register hdlmenubarstack hs = hstack;
	tymenubarstackelement menuinfo;
	bigstring bs;
	boolean flitemenabled, flitemchecked;
	short ixstack, itemnumber;
	hdlmenu hmenu;
	boolean fltitle;
	register boolean fl = false;
	
	if (hs == nil) //nothing to do
		return (true);
	
	pushmenubarglobals (hs);
	
	/*first locate as the title of a menu*/

	fltitle = mefindinmenubar (hnode, true, &ixstack, &itemnumber);

	if (fltitle) {
		
		/*
		since the menu manager doesn't let us change the title of a menu, we 
		need to recreate the menu from scratch.  however, we want to reuse the 
		same id so that our parent (if we have one) will still point to us.
		*/
		
		short idmenu;
	//	boolean flhierarchic;
		
		menuinfo = (**hs).stack [ixstack];

		idmenu = medeletemenu (hs, ixstack, true);
		
		assert (idmenu == menuinfo.idmenu);

		if (!mebuildmenu (hnode, menuinfo.flhierarchic, &menuinfo, false)) { /*error building the menu*/
			
			mefreemenuid (idmenu); /*allow it to be reused*/
			
			goto exit;
			}
		
		(**hs).stack [ixstack] = menuinfo; /*update to new menu*/
		
		meinsertmenu (&menuinfo); /*add to menubar, in correct place*/
		
		if (!menuinfo.flhierarchic) { /*its name doesn't appear in a menu*/
			
			meupdatemenubar ();

			fl = true;
			
			goto exit;
			}
		}

	/*now locate the node as a menu item*/

	if (!mefindinmenubar (hnode, false, &ixstack, &itemnumber))
		goto exit;

	getheadstring (hnode, bs); 
	
	mereduceformula (bs); /*filter formulas*/

	mereducemenucodes (bs, &flitemenabled, &flitemchecked); /*4.1b8 dmb*/
	
	hmenu = (**hs).stack [ixstack].hmenu;
	
	setmenuitem (hmenu, itemnumber, bs);
	
	setmenuitemenable (hmenu, itemnumber, flitemenabled);
	// setmenuitemenable (hmenu, itemnumber, !equalstrings (bs, (ptrstring) "\p-"));
	
	if (!fltitle) { /*4.1b11 dmb: don't set check if we're a title*/
	
		checkmenuitem (hmenu, itemnumber, flitemchecked); /*4.1b8 dmb*/
		
		setmenuitemcommandkey (hmenu, itemnumber, mecmdkey (hnode));
		}
	
	fl = true;
	
	exit:
	
	/*
	menubarchanged (hs);
	*/
	
	popmenubarglobals ();
	
	return (fl);
	} /*memenuitemchanged*/


static short megetmenuindex (hdlmenu hmenu /*short id*/) {
	
	/*
	search the menubar data structure for the indicated menu id.
	
	return an index into the menubar stack, or -1 in not found.
	*/
	
	register hdlmenubarstack hs = menubardata;
	register short i;
	register short topstack;
	
	topstack = (**hs).topstack;
	
	for (i = 0; i < topstack; i++) {
		
		//if ((**hs).stack [i].idmenu == id) /*found match -- it's a whole menu*/
		if ((**hs).stack [i].hmenu == hmenu)
			return (i);
		} /*for*/
	
	return (-1); /*search failed*/
	} /*megetmenuindex*/


static void medeleteallmenubarentries (short ixstack) {
	
	/*
	delete the indicated entry from the menubarstack, as well as the entries 
	for any heirarchical menus subordinate to the menu of this entry.
	*/
	
	register hdlmenubarstack hs = menubardata;
	register short ixmenu;
	register hdlmenu hmenu;
	register short ctitems;
	hdlmenu hsubmenu;
	/*
	short cmdchar;
	short idsub;
	*/

	if (ixstack < 0) /*defensive driving*/
		return;
	
	hmenu = (**hs).stack [ixstack].hmenu; /*move into register*/
	
	ctitems = countmenuitems (hmenu);
	
	for (ixmenu = 1; ixmenu <= ctitems; ++ixmenu) {
		
		/*
		GetItemCmd (hmenu, ixmenu, &cmdchar);
		
		if (cmdchar == hMenuCmd) {
			
			GetItemMark (hmenu, ixmenu, &idsub);
			
			medeleteallmenubarentries (megetmenuindex (idsub));
			}
		*/
		
		if (gethierarchicalmenuitem (hmenu, ixmenu, &hsubmenu))
			medeleteallmenubarentries (megetmenuindex (hsubmenu));
		}
	
	medeletemenubarentry (megetmenuindex (hmenu)); // was (**hmenu).menuID
	} /*medeleteallmenubarentries*/


short mecheckdeletedmenu (short ixstack, boolean fldrawmenubar) {

	/*
	if ixstack is the deletedmenu, it is being reused.  otherwise, it needs to 
	be discarded.  return ixstack, adjusted for any deletion
	
	12/21/91 dmb: use new medeleteallmenubarentries to make sure that we don't 
	leave bogus entries in the stack for submenus of the deleted one.
	*/
	
	register hdlmenubarstack hs = menubardata;
	register short ixdeletedmenu;
	
	if (hs == nil)
		return (ixstack);
	
	ixdeletedmenu = (**hs).ixdeletedmenu;
	
	if (ixdeletedmenu >= 0) {
		
		if (ixdeletedmenu != ixstack) { /*menu is not being re-inserted now*/
			
			fldrawmenubar = (fldrawmenubar && !(**hs).stack [ixdeletedmenu].flhierarchic);
			
			(**hs).ixdeletedmenu = ixstack; /*temp -- medeletemenubarentry will maintain position*/
			
			medeleteallmenubarentries (ixdeletedmenu); 
			
			ixstack = (**hs).ixdeletedmenu; /*grab adjusted value*/
			
			if (fldrawmenubar)
				meupdatemenubar ();
			}
		
		(**hs).ixdeletedmenu = -1; /*no pending deletion*/
		}
	
	return (ixstack);
	} /*mecheckdeletedmenu*/


static boolean mefindvisit (hdlheadrecord h, ptrvoid refcon) {

	return (h != (hdlheadrecord) refcon);
	} /*mefindvisit*/


boolean memenuitemadded (hdlmenubarstack hstack, hdlheadrecord hnode) {
	
	/*
	called when something has been added in the menubar structure.
	*/
	
	register hdlmenubarstack hs = hstack;
	short ixstack, itemnumber;
	tymenubarstackelement menuinfo;
	boolean flhierarchic, flwashierarchic;
	hdlheadrecord hmenubar;
	register boolean fl = false;
	
	if (hs == nil) //nothing to do
		return (true);
	
	pushmenubarglobals (hs);
	
	flhierarchic = !memainmenunode (hnode);

	flwashierarchic = true;
	
	/*
	look for hnode as the title of a menu.  if it is found, then this is the 
	second half of a move operation -- an unlink left it in the menu structure, 
	with ixdeletedmenu set accordingly to its index.
	*/
	if (mefindinmenubar (hnode, true, &ixstack, &itemnumber)) { /*found as title?*/
		
		/*
		menu probably moved in the menu structure.  it may have even changed 
		from being a main menu to a heirarchical, or vice versa
		*/
		register tymenubarstackelement *pitem;
		
		assert (ixstack == (**hs).ixdeletedmenu);
		
		ixstack = mecheckdeletedmenu (ixstack, false); /*should match*/
		
		pitem = (**hs).stack + ixstack;
		
		flwashierarchic = (*pitem).flhierarchic;
		
		/*
		if hnode should still have a menu, we'll need to remove it from the 
		menu manager's menubar structure, so we can re-insert it according to 
		its new status (i.e. hierarchical or not) below.  if hnode shouldn't have
		a menu, dispose of the menu and its related data
		*/
		if (flhierarchic  &&  opnosubheads (hnode)) { /*shouldn't be a menu title anymore*/
			
			medeletemenubarentry (ixstack); /*dispose completely*/
			
			menuinfo.idmenu = -1;
			}
		else {
			(*pitem).flhierarchic = flhierarchic; /*update menubardata to new status*/

			menuinfo = (*pitem); /*this is the menu we'll re-insert below...*/
			
			meremovemenu (&menuinfo); /*remove from menu manager's structure*/
			}
		
		/*
		now look for hnode as a menu item
		*/
		if (!mefindinmenubar (hnode, false, &ixstack, &itemnumber)) /*found as item?*/
			itemnumber = -1;
		}
	else {
		
		if (!mefindinmenubar (hnode, false, &ixstack, &itemnumber)) { /*found as item?*/
			
			/*
			didn't find under existing titles; see if this node belongs in the 
			menu bar at all
			*/
			opoutermostsummit (&hmenubar);
			
			if (opsiblingvisiter (hmenubar, false, mefindvisit, (ptrvoid) hnode)) { /*not found in menubar outline*/
				
				mecheckdeletedmenu (-1, true); /*no match, draw menu bar*/
				
				fl = true;
				
				goto exit;
				}
			
			itemnumber = -1;
			}
		
		ixstack = mecheckdeletedmenu (ixstack, false); /*no match*/
		
		/*
		we didn't find hnode as the title of a menu.  see if we need to build 
		a new hierarchy of menus for the suboutline
		*/
		
		menuinfo.idmenu = -1; /*allocate a new id, if a menu is built*/
		
		if (meshouldhavemenu (hnode)) {
			
			if (!mebuildmenu (hnode, flhierarchic, &menuinfo, true)) /*error building the menu*/
				goto exit;
			
			if (!mepushmenubarstack (&menuinfo)) /*error inserting*/
				goto exit;
			}
		}
	
	/*
	if hnode has a menu, it should be available for insertion at this point.  if 
	idmenu is non-negative, it's ready to go
	*/
	assert ((menuinfo.idmenu != -1) == (ophassubheads (hnode) || !flhierarchic));
	
	if (menuinfo.idmenu != -1)
		meinsertmenu (&menuinfo);
	
	/*if a main menu was moved or created, redraw the menu bar*/
	
	if (!flhierarchic  ||  !flwashierarchic)
		meupdatemenubar ();
	
	/*if hnode is a main menu, we're done!*/
	
	if (!flhierarchic) {
		
		fl = true;
		
		goto exit;
		}
	
	/*now deal with the parent menu, which may or may not already exist*/
	
	if (itemnumber > 0) { /*appears as an item in existing menu*/
		
		assert ((**hs).stack [ixstack].hnode == (**hnode).headlinkleft);
	
		fl = meinsertmenuitem ((**hs).stack [ixstack].hmenu, itemnumber, hnode, false);
		
		goto exit;
		}
	else
		flhierarchic = true; /*if it was a main menu, it would already exist*/
	
	hnode = (**hnode).headlinkleft;
	
	menuinfo.idmenu = -1;
	
	if (mebuildmenu (hnode, flhierarchic, &menuinfo, false)) { /*no error building the menu*/
		
		if (!mepushmenubarstack (&menuinfo)) 
			goto exit;
		
		meinsertmenu (&menuinfo); /*add to menubar, in correct place*/
		
		if (flhierarchic)
			melinkparentitem (hnode, menuinfo.hmenu, menuinfo.idmenu);
		}
	
	fl = true;
	
	exit:
	
	/*
	menubarchanged (hs);
	*/
	
	popmenubarglobals ();
	
	return (fl);
	} /*memenuitemadded*/


boolean memenuitemdeleted (hdlmenubarstack hstack, hdlheadrecord hnode) {
	
	/*
	called when something has been deleted in the menubar structure.
	
	2.1b3: since this is a deletion, it can't be the second helf of a 
	move, and any previosly-deleted menu should be flushed now.

	5.0a24 dmb: fixed bug when emptying a hierarchical menu; must 
	delete item before tossing menu [Win]
	*/
	
	register hdlmenubarstack hs = hstack;
	short ixstack, itemnumber;
	hdlmenu hmenu;
	
	if (hs == nil) //nothing to do
		return (true);
	
	pushmenubarglobals (hs);
	
	mecheckdeletedmenu (-1, true); /*2.1b3 dmb: flush deleted menu unconditionally*/
	
	if (mefindinmenubar (hnode, true, &ixstack, &itemnumber)) { /*deleting a whole menu*/
		
		//2.1b3 mecheckdeletedmenu (ixstack, false); /*note match or flush previous deletion*/
		
		(**hs).ixdeletedmenu = ixstack; /*save for later, if not reinserted*/
		
		/*
		medeletemenubarentry (ixstack); 
		
		drawmenubar ();
		*/
		}
	
	if (mefindinmenubar (hnode, false, &ixstack, &itemnumber)) { /*deleting a menu item, not a whole menu*/
		
		hmenu = (**hs).stack [ixstack].hmenu;
		
		deletemenuitem (hmenu, itemnumber);
		
		if ((**hs).stack [ixstack].flhierarchic  &&  countmenuitems (hmenu) == 0) {
			
			melinkparentitem ((**hs).stack [ixstack].hnode, nil, -1);
			
			medeletemenubarentry (ixstack);
			}
		}
	
	/*
	menubarchanged (hs);
	*/
	
	popmenubarglobals ();
	
	return (true);
	} /*memenuitemdeleted*/	


boolean meinsertmenubar (hdlmenubarstack hstack) {
	
	/*
	add the indicated menubar stack to the menubarlist.
	*/
	
	if (!pushmenubarlist (hstack))
		return (false);
	
	meactivatemenus (hstack, (**menubarlist).flactive);
	
	return (true);
	} /*meinsertmenubar*/


boolean medeletemenubar (hdlmenubarstack hstack) {
	
	/*
	remove the indicated menubar stack from the menubarlist.
	*/
	
	if (!popmenubarlist (hstack))
		return (false);
	
	meactivatemenus (hstack, false);
	
	return (true);
	} /*medeletemenubar*/


boolean purgefrommenubarlist (long refcon) {
	
	/*
	5.1.5 dmb: remove all menubar stacks associated with the given refcon
	*/
	
	register hdlmenubarlist hlist = menubarlist;
	register hdlmenubarstack hstack, hnext;
	
	if (hlist == nil)
		return (true);
	
	hstack = (**hlist).hfirst;
	
	while (hstack != nil) {
		
		hnext = (**hstack).hnext;
		
		if ((**hstack).refcon == refcon)
			medeletemenubar (hstack);
		
		hstack = hnext;
		}
	
	return (true);
	} /*purgefrommenubarlist*/


boolean rebuildmenubarlist (void) {
	
	/*
	traverse the menubarlist, re-building all menubars from scratch.
	
	normally shouldn't need to be called, but we offer a verb that calls 
	this to force a refresh in case of bugs.
	*/
	
	register hdlmenubarlist hlist = menubarlist;
	register hdlmenubarstack hstack;
	boolean fl = true;
	
	if (hlist == nil)
		return (true);
	
	hstack = (**hlist).hfirst;
	
	while (hstack != nil) {
		
		if (!mebuildmenubar (hstack))
			fl = false;
		else
			meactivatemenus (hstack, (**hlist).flactive); /*checks for no change*/
		
		hstack = (**hstack).hnext;
		}
	
	return (fl);
	} /*rebuildmenubarlist*/


boolean melocatemenubarnode (hdlheadrecord hnode, hdloutlinerecord *houtline) {
	
	/*
	traverse the menubarlist, looking for the menu that contains hnode.
	
	set houtline to the outline associated with that menu's menubar.
	
	return true if successful, false if no menu was found containing hnode
	*/
	
	register hdlmenubarlist hlist = menubarlist;
	register hdlmenubarstack hstack;
	short ixstack, itemnumber;
	
	if (hlist == nil)
		return (false);
	
	hstack = (**hlist).hfirst;
	
	while (hstack != nil) {
		
		pushmenubarglobals (hstack);
		
		if (mefindinmenubar (hnode, false, &ixstack, &itemnumber)) { /*found as item*/
			
			*houtline = outlinedata;
			
			popmenubarglobals ();
			
			return (true);
			}
		
		popmenubarglobals ();
		
		hstack = (**hstack).hnext;
		}
	
	return (false);
	} /*melocatemenubarnode*/


boolean mecheckformulas (short ixstack) {
	
	/*
	search the menubar data structure for formula items.
	
	whenever a formula is encountered, update the item text.
	
	4.1b6 dmb: if item begins with a paren, and doesn't end with one, disabled it
	*/
	
	register hdlmenubarstack hs = menubardata;
	short i = ixstack;
	hdlheadrecord hnode;
	short ixmenu;
	bigstring bs;
	short ctitems;
	boolean flitemenabled, flitemchecked;
	tymenubarstackelement info;
	
	info = (**hs).stack [i];
	
	ctitems = countmenuitems (info.hmenu);
	
	hnode = (**info.hnode).headlinkright;
	
	for (ixmenu = info.ctbaseitems + 1; ixmenu <= ctitems; ++ixmenu) {
		
		getheadstring (hnode, bs); 
		
		if (mereduceformula (bs))  { /*item was a formula*/
			
			mereducemenucodes (bs, &flitemenabled, &flitemchecked); /*4.1b8 dmb*/
			
			setmenuitem (info.hmenu, ixmenu, bs);
			
			setmenuitemenable (info.hmenu, ixmenu, flitemenabled); /*4.1b6 dmb*/
			
			if (!ophassubheads (hnode))
				checkmenuitem (info.hmenu, ixmenu, flitemchecked); /*4.1b8 dmb*/
			}
		
		hnode = (**hnode).headlinkdown;
		}
	
	return (true);
	} /*mecheckformulas*/	


/*
boolean memenuupdate (void) {
	
	/%
	respond to a menubarlist menu selection
	%/
	
	register hdlmenubarlist hlist = menubarlist;
	register hdlmenubarstack h;
	
	if (hlist != nil) {
		
		h = (**hlist).hfirst;
		
		while (h != nil) {
			
			mecheckformulas (h);
			
			h = (**h).hnext;
			} /%while%/
		}
	
	return (false); /%didn't handle the menu%/
	} /%memenuupdate%/
*/


boolean memenuhit (short idmenu, short ixmenu, hdlheadrecord *hnode) {
	
	/*
	search the menubar data structure for a menu with the indicated id.
	
	locate the node that it came from in the menu and run its script, or 
	display the headline if the option key is down.
	
	12/2/91 dmb: don't call medirtymenubar from here, since we're called to 
	clear them up; set fldirtymenubar directly to force menubar redraw.
	
	5.1.4 dmb: don't dim menus when processbusy
	*/
	
	register short i;
	register short topstack;
	
	mecheckdeletedmenu (-1, true); /*8/13/92 dmb: flush deleted menu*/
	
	topstack = (**menubardata).topstack;
	
	for (i = 0; i < topstack; i++) { /*search for the menu*/
		
		tymenubarstackelement mi = (**menubardata).stack [i];

		if (idmenu == 0) { /*enable/disable all menus*/
			boolean flold, flnew;
			
			flold = getmenutitleenable (mi.hmenu, mi.idmenu);
			
			flnew = true; //!processbusy ()
			
			if (flnew != flold) {
				
				setmenutitleenable (mi.hmenu, mi.idmenu, flnew);
				
				fldirtymenubar = true;
				}
			
			if (flnew)
				mecheckformulas (i);
			}
		else {
			
			if (mi.idmenu == idmenu) {
				
				register hdlheadrecord h = mi.hnode;
				
				ixmenu -= mi.ctbaseitems;

				if (h == nil || ixmenu <= 0)
					return (false);
				
				h = opnthsubhead (h, (short) (ixmenu));
				
				if (h == nil) 
					return (false);
				
				*hnode = h;
				
				return (true);
				}
			}
		} /*for*/
	
	return (false); /*failed to find the id in our structure*/
	} /*memenuhit*/	


boolean memenu (short idmenu, short ixmenu) {
	
	/*
	respond to a menubarlist menu selection
	
	11/20/91 dmb: handle disabled (negative) items
	*/
	
	register hdlmenubarlist hlist = menubarlist;
	register hdlmenubarstack h;
	boolean fljustshownode;
	hdlheadrecord hnode;
	
	fljustshownode = keyboardstatus.floptionkey || optionkeydown ();
	
	if (ixmenu < 0) { /*disabled item*/
		
		if (!fljustshownode)
			return (false);
		
		ixmenu = -ixmenu;
		}
	
	if (hlist != nil) {
		
		h = (**hlist).hfirst;
		
		while (h != nil) {
			
			pushmenubarglobals (h);
			
			if (memenuhit (idmenu, ixmenu, &hnode)) {
				
				if (fljustshownode)
					meshownode (hnode);
				else
					meuserselected (hnode);
				
				popmenubarglobals ();
				
				return (true);
				}
			
			popmenubarglobals ();
			
			h = (**h).hnext;
			} /*while*/
		}
	
	if (idmenu == 0) /*updating menus; see if menubar needs to be redrawn*/
		mecheckmenubar ();
	
	return (false); /*didn't handle the menu*/
	} /*memenu*/	


static unsigned char menullchar (hdlheadrecord hn) {
	
	return (chnul);
	} /*menullchar*/


void menubarinit (void) {
	
	if (menubarcallbacks.menubarchangedroutine == nil) /*no one has set is so far*/
		menubarcallbacks.menubarchangedroutine = (menubarchangedcallback) &truenoop;
	
	if (menubarcallbacks.getcmdkeyroutine == nil)
		menubarcallbacks.getcmdkeyroutine = &menullchar;
	} /*menubarinit*/




