
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

#include "landinternal.h"
#include <processes.h>




static landgetscriptid (tyapplicationid *id) {
	
	/*
	makes it easy to direct messages to the scripting system.
	
	7/3/91 dmb: removed call to landapprunning.  it justs adds IAC traffic, 
	and would doesn't add any additional error handle capabilities.  (besides, 
	none of the callers were checking the result of this fuction)
	*/
	
	*id = idappscriptrunner;
	
	/*
	return (landapprunning (*id));
	*/
	} /*landgetscriptid*/


static short landcountmenuarray (hdlmenuarray hmenuarray) {
	
	/*
	returns the number of menus in the menu array.
	*/
	
	if (hmenuarray == nil)
		return (0);
		
	return ((short) (landgethandlesize ((Handle) hmenuarray) / longsizeof (tyruntimemenurecord)));
	} /*landcountmenuarray*/


pascal boolean landinstallmenuarray (short idmenuafter, hdlmenuarray hmenuarray) {
	
	/*
	Calls the Macintosh Menu Manager InsertMenu routine once for each menu in the 
	hdlmenuarray structure. All the menus are inserted before the menu id passed in 
	as the first parameter according to the conventions of InsertMenu.  Pass in zero 
	for the first parameter to install the menus to the right of all other menus.
	
	Does not call DrawMenuBar.
	
	Always returns true.
	*/
	
	register hdllandglobals hg = landgetglobals ();
	register hdlmenuarray hm = hmenuarray;
	register short i, ct;
	tyruntimemenurecord item;
	
	ct = landcountmenuarray (hm);
	
	for (i = 0; i < ct; i++) {
		
		item = (**hm) [i];
		
		if (item.flhierarchic)
			InsertMenu (item.hmenu, -1);
		else
			InsertMenu (item.hmenu, idmenuafter);
		
		(**hm) [i].flinserted = true;
		} /*for*/
	
	(**hg).fldirtyruntimemenus = false;
	
	return (true);
	} /*landinstallmenuarray*/


static boolean landgetmenuhandles (hdlmenuarray hmenuarray) {
	
	register hdllandglobals hg = landgetglobals ();
	register short i, ct;
	register boolean fl;
	hdlverbrecord hverb;
	typaramrecord param;
	bigstring bserror;
	short iderror;
	tyapplicationid id;
	
	ct = landcountmenuarray (hmenuarray);
	
	landgetscriptid (&id);
	
	for (i = 0; i < ct; i++) {
		
		if (!landnewverb (id, id, getmenuhandletoken, 2, &hverb))
			return (false);
		
		if (!landpushlongparam (hverb, (**hg).applicationid, idmenuprogram))
			return (false);
		
		if (!landpushintparam (hverb, i, idarrayindex))
			return (false);
		
		if (!landsendverb (hverb, &param, bserror, &iderror))
			return (false);
		
		fl = false;
		
		if (param.type == binarytype) {
		
			(**hmenuarray) [i].hmenu = (MenuHandle) param.val.binaryvalue;
			
			param.val.binaryvalue = nil; /*keep it from being disposed*/
			
			fl = true;
			}
			
		landdisposeparamrecord (&param);
		
		if (!fl)
			return (false);
		} /*for*/
		
	return (true);
	} /*landgetmenuhandles*/


pascal boolean landgetmenuarray (short firstresource, hdlmenuarray *hmenuarray) {
	
	/*
	Sends an IAC-message to Frontier asking for an array of menu information about the 
	menu bar.  ItÕs passed back as a binary IAC message. The resulting array is returned
	in the second parameter.
	
	The first parameter is the starting resource number for menu allocation.  
	
	Returns false if Runtime wasnÕt running, or if there are no menus for your 
	application, or if one of the menus wasnÕt successfully loaded.
	*/
	
	register hdllandglobals hg = landgetglobals ();
	hdlverbrecord hverb;
	typaramrecord param;
	bigstring bserror;
	short iderror;
	register boolean fl;
	tyapplicationid id;
	
	iderror = 0;
	
	setemptystring (bserror);
	
	landgetscriptid (&id);
	
	if (!landnewverb (id, id, getmenuarraytoken, 2, &hverb))
		return (false);
	
	if (!landpushlongparam (hverb, (**hg).applicationid, idmenuprogram))
		return (false);
		
	if (!landpushintparam (hverb, firstresource, idstartingresource))
		return (false);
		
	if (!landsendverb (hverb, &param, bserror, &iderror))
		return (false);
	
	fl = false;
	
	if (param.type == binarytype) {
	
		*hmenuarray = (hdlmenuarray) param.val.binaryvalue;
		
		param.val.binaryvalue = nil; /*keep it from being disposed*/
		
		fl = true;
		}
		
	landdisposeparamrecord (&param);
	
	if (!fl)
		return (false);
	
	return (landgetmenuhandles (*hmenuarray));
	} /*landgetmenuarray*/


pascal boolean landremovemenuarray (hdlmenuarray hmenuarray) {
	
	/*
	Removes the menus in the specified menu array from the menu bar.  Does not 
	dispose of the menus or menu array. Returns true if it was able to remove the 
	menus.
	*/

	register hdlmenuarray hm = hmenuarray;
	register short i;
	register short ctmenus;
	
	ctmenus = landcountmenuarray (hm);
	
	for (i = 0; i < ctmenus; i++) {
		
		DeleteMenu ((**hm) [i].idmenu);
		
		(**hm) [i].flinserted = false;
		}
	
	return (true);
	} /*landremovemenuarray*/


pascal boolean landdisposemenuarray (hdlmenuarray hmenuarray) {
	
	/*
	Diposes of the menus in the specified menu array and frees the memory 
	associated with the menu array. Returns true if it was able to dispose 
	of the menus and menu array memory.
	*/
	
	register hdlmenuarray hm = hmenuarray;
	register short i;
	register short ctmenus;
	tyruntimemenurecord item;
	
	if (hm == nil) /*defensive driving*/
		return (false);
	
	ctmenus = landcountmenuarray (hm);
	
	for (i = 0; i < ctmenus; i++) {
		
		item = (**hm) [i];
		
		if (item.flinserted)
			DeleteMenu (item.idmenu);
		
		DisposeMenu (item.hmenu);
		} /*for*/
	
	DisposHandle ((Handle) hm);
	
	return (true);
	} /*landdisposemenuarray*/


pascal boolean landenablemenuarray (hdlmenuarray hmenuarray, boolean flenable) {
	
	/*
	7/3/91 dmb: Enables or disables the the menus in the specified menu array.
	
	Always returns true.
	*/
	
	register hdlmenuarray hm = hmenuarray;
	register short i;
	register short ctmenus;
	register MenuHandle hmenu;
	
	ctmenus = landcountmenuarray (hm);
	
	for (i = 0; i < ctmenus; i++) {
		
		hmenu = (**hm) [i].hmenu;
		
		if (flenable)
			EnableItem (hmenu, 0);
		else
			DisableItem (hmenu, 0);
		} /*for*/
	
	DrawMenuBar ();
	
	return (true);
	} /*landenablemenuarray*/


pascal boolean landrunmenuitem (idmenu, iditem, hmenuarray) short idmenu, iditem; hdlmenuarray hmenuarray; {
	
	/*
	Sends an IAC message to Frontier to run the script linked into the indicated 
	menu item.  The first parameter is the id of the menu, the second is the menu 
	item number. Returns true if it was able to send the message to Runtime.
	*/
	
	register hdllandglobals hg = landgetglobals ();
	register hdlverbrecord hv;
	register long idscript;
	hdlverbrecord hverb;
	typaramrecord param;
	bigstring bserror;
	short iderror;
	tyapplicationid id;
	
	(**hg).idmenuscript = 0; /*indicate no script running*/
	
	(**hg).hmenuarray = nil; 
	
	(**hg).flscriptcancelled = false; 
	
	iderror = 0;
	
	setemptystring (bserror);
	
	landgetscriptid (&id);
	
	if (!landnewverb (id, id, idrunmenuitem, 3, &hverb))
		return (false);
	
	hv = hverb; /*copy into register*/
	
	if (!landpushlongparam (hv, (**hg).applicationid, idmenuprogram))
		return (false);
	
	if (!landpushintparam (hv, idmenu, idmenuidvalue))
		return (false);
	
	if (!landpushintparam (hv, iditem, idmenuitemvalue))
		return (false);
	
	if (!landsendverb (hv, &param, bserror, &iderror))
		return (false);
		
	idscript = param.val.longvalue;
	
	(**hg).idmenuscript = idscript;
	
	if (idscript != 0) { /*a script is actually running*/
	
		HiliteMenu (0); /*un-highlight any highlighted menus*/
			
		landenablemenuarray (hmenuarray, false); /*disable menus while script running*/
		
		(**hg).hmenuarray = hmenuarray;
		}
		
	landdisposeparamrecord (&param); /*it might have heap-allocated stuff linked in*/
	
	return (true);
	} /*landrunmenuitem*/


pascal boolean landmenuhit (short idmenu, hdlmenuarray hmenuarray) {
	
	/*
	Returns true if the menu whose id (the first parameter) is in the menuarray 
	(the 2nd parameter).  Call this in your menu handling routine to decide if you 
	should pass the command to Frontier using landrunmenuitem.	
	*/
	
	register short ct, i;
	tyruntimemenurecord item;
	
	ct = landcountmenuarray (hmenuarray);
	
	for (i = 0; i < ct; i++) {
		
		item = (**hmenuarray) [i];
		
		if (item.idmenu == idmenu)
			return (true);
		} /*for*/
		
	return (false);
	} /*landmenuhit*/


pascal boolean landcancelmenuscript (void) {
	
	/*
	call this routine to kill the menu script that's running.   
	
	it just sets a flag in the toolkit globals record.  the next IAC verb that's 
	received returns with an error saying that the user cancelled the script.  
	Frontier will avoid putting up an error window on this particular error.
	
	returns false if no script is running.
	*/
	
	register hdllandglobals hg = landgetglobals ();
	
	if ((**hg).idmenuscript == 0) /*no script running*/
		return (false);
		
	(**hg).flscriptcancelled = true;
	
	return (true);
	} /*landcancelmenuscript*/
	
	
pascal boolean landmenuneedsupdate (void) {

	register hdllandglobals hg = landgetglobals ();
	
	return ((**hg).fldirtyruntimemenus);
	} /*landmenuneedsupdate*/


pascal boolean landmenuscriptcompleted (void) { 

	register hdllandglobals hg = landgetglobals ();
	
	if ((**hg).idmenuscript == 0) /*no script running*/
		return (false);
	
	(*(**hg).menuscriptcompleteroutine) ();
	
	(**hg).idmenuscript = 0; /*no script running anymore*/
	
	landenablemenuarray ((**hg).hmenuarray, true);
	
	(**hg).hmenuarray = nil;
	
	(**hg).flscriptcancelled = false; 
	
	return (true);
	} /*landmenuscriptcompleted*/
			


