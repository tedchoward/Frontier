
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

#include "memory.h"
#include "font.h"
#include "cursor.h"
#include "quickdraw.h"
#include "scrap.h"
#include "frontierwindows.h"
#include "op.h"
#include "opinternal.h"
#include "menueditor.h"
#include "menuinternal.h"

	

boolean megetmenuiteminfo (hdlheadrecord hnode, tymenuiteminfo *item) {
	
	if (!opgetrefcon (hnode, item, sizeof (tymenuiteminfo)))
		return (false);
	
	disktomemlong ((*item).linkedscript.adrlink);
	
	return (true);
	} /*megetmenuiteminfo*/


boolean mesetmenuiteminfo (hdlheadrecord hnode, const tymenuiteminfo *item) {
	
	tymenuiteminfo info = *item;

	memtodisklong (info.linkedscript.adrlink);
	
	return (opsetrefcon (hnode, &info, sizeof (info)));
	} /*mesetmenuiteminfo*/


boolean mecopyrefconroutine (hdlheadrecord hsource, hdlheadrecord hdest) {
	
	/*
	5.0a3 dmb: use meloadscriptoutline, not meloadoutline. the latter loads
	the main menubar outline, not script outlines. I don't know why it didn't 
	break in 4.x, but it breaks now.
	*/
	
	hdloutlinerecord ho;
	tymenuiteminfo item;
	hdloutlinerecord hcopy;
	dbaddress adr;
	boolean flignore;
	
	(**hdest).hrefcon = nil; /*default*/
	
	if ((**hsource).hrefcon == nil)
		return (true);
	
	megetmenuiteminfo (hsource, &item);
	
	ho = item.linkedscript.houtline;
	
	if (ho != nil) { /*copy the in-memory version*/
			
		if (!opcopyoutlinerecord (ho, &hcopy))
			return (false);
		}
		
	else { /*load the original in from disk*/
		
		adr = item.linkedscript.adrlink;
		
		if (adr == nildbaddress) /*no script linked into this line*/
			hcopy = nil;
			
		else {
			hdlmenurecord hm = (hdlmenurecord) (**outlinedata).outlinerefcon;
			
			if (!meloadscriptoutline (hm, hsource, &hcopy, &flignore)) // 5.0a3 dmb - was: meloadoutline (adr, &hcopy)
				return (false);
			}
		}
	
	if (hcopy != nil)
		(**hcopy).fldirty = true; /*force save on this guy's script*/
	
	item.linkedscript.houtline = hcopy;
	
	item.linkedscript.adrlink = nildbaddress; /*hasn't been allocated yet*/
	
	mesetmenuiteminfo (hdest, &item);
	
	return (true);
	} /*mecopyrefconroutine*/


static boolean headleveloffsetvisit (hdlheadrecord hnode, ptrvoid refcon) {
	
	long *headleveloffset = (long *) refcon;
	
	(**hnode).headlevel += *headleveloffset;
	
	return (true);
	} /*headleveloffsetvisit*/


boolean metextualizerefconroutine (hdlheadrecord hnode, Handle htext) {
	
	/*
	5.1.5b16 dmb: get menu record from outlinedata, not global
	*/
	
	register hdloutlinerecord ho;
	register boolean fl;
	register hdlheadrecord hsummit;
	hdloutlinerecord houtline;
	boolean fljustloaded;
	long headleveloffset;
	hdlmenurecord hm = (hdlmenurecord) (**outlinedata).outlinerefcon;
	
	if (!meloadscriptoutline (hm, hnode, &houtline, &fljustloaded)) 
		return (false);
	
	ho = houtline; /*move into register*/
	
	if (ho == nil) /*no script linked into the menu item*/
		return (true);
	
	hsummit = (**ho).hsummit;
	
	headleveloffset = (**hnode).headlevel + 1; /*add extra indentation for clipboard*/
	
	opsiblingvisiter (hsummit, false, &headleveloffsetvisit, &headleveloffset);
	
	fl = opoutlinetotextscrap (ho, false, htext);
	
	if (fljustloaded)
		opdisposeoutline (ho, false);
	
	else {
		
		headleveloffset = -headleveloffset; /*prepare for reversal*/
		
		opsiblingvisiter (hsummit, false, &headleveloffsetvisit, &headleveloffset);
		}
	
	return (fl);
	} /*metextualizerefconroutine*/


boolean mereleaserefconroutine (hdlheadrecord hnode, boolean fldisk) {
	
	/*
	5.1.4 dmb: set the right database when fldisk
	*/

	tymenuiteminfo item;
	hdlmenurecord hm = (hdlmenurecord) (**outlinedata).outlinerefcon;

	megetmenuiteminfo (hnode, &item);
	
	opdisposeoutline (item.linkedscript.houtline, fldisk);
	
	if (fldisk) {
	
		dbpushdatabase (megetdatabase (hm));

		dbpushreleasestack (item.linkedscript.adrlink, outlinevaluetype);

		dbpopdatabase ();
		}
	
	return (true);
	} /*mereleaserefconroutine*/


typedef struct typackinfo {

	Handle hpackedscripts; /*for pack/unpackscriptvisit routines*/

	long ixpackedscripts; /*for unpacking only*/
	} typackinfo, *ptrpackinfo;


static boolean mesavescriptvisit (hdlheadrecord hnode, ptrvoid refcon) {
	
	register hdlheadrecord h = hnode;
	//ptrpackinfo packinfo = (ptrpackinfo) refcon;
	register hdloutlinerecord ho;
	tymenuiteminfo item;
	
	rollbeachball ();
	
	assert (menudata != nil);
	
	if (!megetmenuiteminfo (h, &item)) /*nothing linked, keep visiting*/
		return (true);
	
	ho = item.linkedscript.houtline; /*copy into register*/
	
	if (ho == nil) /*if nil, nothing to worry about saving*/
		return (true);
	
	if ((**ho).fldirty) { /*needs saving*/
		
		if (!mesaveoutline (ho, &item.linkedscript.adrlink)) /*memory or disk error, stop visiting*/
			return (false);
		}
	
	if (ho != (**menudata).scriptoutline) { /*don't reclaim the active script…*/
		
		opdisposeoutline (ho, false); /*reclaim the script*/
	
		item.linkedscript.houtline = nil; /*force us to look to disk*/
		}
		
	else { /*…just clear its madechanges bit*/
		
		windowsetchanges ((**menudata).scriptwindow, false);
		
		(**ho).fldirty = false;
		}
	
	return (mesetmenuiteminfo (h, &item));
	} /*mesavescriptvisit*/
	

static boolean mesaveasscriptvisit (hdlheadrecord hnode, ptrvoid refcon) {
	
	/*
	for save as, we need to write a packed version of every script to 
	the new file.  db.c takes care of redirecting reads & writes as 
	necessary
	*/
	
	register hdlheadrecord h = hnode;
	hdloutlinerecord ho;
	tymenuiteminfo item;
	dbaddress adr;
	boolean fltempload = false;
	
	rollbeachball ();
	
	if (!megetmenuiteminfo (h, &item)) /*nothing linked, keep visiting*/
		return (true);
	
	ho = item.linkedscript.houtline; /*copy into register*/
	
	if (flconvertingolddatabase)
		if (!meloadscriptoutline (menudata, h, &ho, &fltempload)) /*error loading script*/
			return (false);
	
	if (ho != nil) {
	
		if (!mesaveoutline (ho, &adr)) /*memory or disk error, stop visiting*/
			return (false);
		}
	else {
		if (!dbcopy (item.linkedscript.adrlink, &adr)) /*copy packed version to new file*/
			return (false);
		}
	
	if (fltempload)
		opdisposeoutline (ho, false);
	
	item.linkedscript.adrlink = adr; /*link in current database will be restored by caller*/
	
	return (mesetmenuiteminfo (h, &item));
	} /*mesaveasscriptvisit*/


static boolean mepackscriptvisit (hdlheadrecord hnode, ptrvoid refcon) {
	
	/*
	if there's a script attached to hnode, pack it onto the end of 
	the hpackedscripts handle.
	*/
	
	register hdlheadrecord h = hnode;
	ptrpackinfo packinfo = (ptrpackinfo) refcon;
	register hdloutlinerecord ho;
	tymenuiteminfo item;
	Handle hpackedoutline;
	register dbaddress adr;
	boolean fl;
	
	if (!megetmenuiteminfo (h, &item)) /*nothing linked, keep visiting*/
		return (true);
	
	ho = item.linkedscript.houtline; /*copy into register*/
	
	if (ho != nil)
		return (oppackoutline (ho, &(*packinfo).hpackedscripts));
	
	adr = item.linkedscript.adrlink;
	
	if (adr == nildbaddress) /*no linked script*/
		return (true);
	
	if (!dbrefhandle (adr, &hpackedoutline)) 
		return (false);
	
	fl = pushhandle (hpackedoutline, (*packinfo).hpackedscripts);
	
	disposehandle (hpackedoutline);
	
	return (fl);
	} /*mepackscriptvisit*/


static boolean mesavemenustructure (tysavedmenuinfo *info, dbaddress *adr) {
	
	/*
	everything has already been set up:  everything pushed and dehoisted.  
	save all of the data associated with the menubar, by visiting every 
	node in the menu structure, saving off all linked scripts, and then 
	saving the menubar outline itself
	
	after a script is saved, we dispose of the in-memory structure, unless it
	is the active script, or we're doing a Save As.
	*/
	
	hdlheadrecord hsummit;
	register boolean fl;
	
	opoutermostsummit (&hsummit);
	
	if (fldatabasesaveas)
		fl = opsiblingvisiter (hsummit, false, &mesaveasscriptvisit, nil);
	else
		fl = opsiblingvisiter (hsummit, false, &mesavescriptvisit, nil);
	
	assert (opvalidate (outlinedata));
	
	if (!fl)
		return (false);

	disktomemlong ((*info).adroutline);
	
	if (!mesaveoutline (outlinedata, &(*info).adroutline))
		return (false);
	
	memtodisklong ((*info).adroutline);
	
	return (dbassign (adr, sizeof (tysavedmenuinfo), info));
	} /*mesavemenustructure*/


boolean mepackmenustructure (tysavedmenuinfo *info, Handle *hpacked) {
	
	/*
	analogous to mesavemenustructure above, except we're packing everything 
	into memory.
	
	10/6/91 dmb: mergehandles now consumes both source handles.
	
	4.1b7 dmb: oppack was fixed so that it won't dispose of the handle we 
	allocated if it fails. so we must dispose it here on error.
	*/
	
	register boolean fl;
	Handle hpackedmenu;
	hdlheadrecord hsummit;
	typackinfo packinfo;
	
	if (!newemptyhandle (&packinfo.hpackedscripts)) /*handle where scripts will be saved*/
	 	return (false);
	
	opoutermostsummit (&hsummit);
	
	fl = opsiblingvisiter (hsummit, false, &mepackscriptvisit, &packinfo);
	
	assert (opvalidate (outlinedata));
	
	if (fl)
		fl = newfilledhandle (info, sizeof (tysavedmenuinfo), &hpackedmenu);
	
	if (fl) {
		
		fl = oppack (&hpackedmenu);
		
		if (fl)
			fl = mergehandles (hpackedmenu, packinfo.hpackedscripts, hpacked);
		else
			disposehandle (hpackedmenu); /*4.1b7 dmb*/
		
		packinfo.hpackedscripts = nil; /*it's been consumed*/
		}
	
	disposehandle (packinfo.hpackedscripts);
	
	return (fl);
	} /*mepackmenustructure*/


boolean mesavemenurecord (hdlmenurecord hmenurecord, boolean flpreservelinks, boolean flmemory, dbaddress *adr, Handle *hpacked) {
	
	/*
	save the menu record handle in the database.
	
	dmb 9/21/90:  unfortunately, menudata global is heavily entrenched, set it at the
	beginning of the routine.
	
	dmb 10/23/90: the new flmemory parameter determines whether we're saving to disk 
	(using adr), or packing to memory (using hpacked)
	*/
	
	register hdlmenurecord hm = hmenurecord;
	register hdloutlinerecord ho = (**hm).menuoutline;
	register boolean fl;
	hdlheadrecord hcursor;
	tysavedmenuinfo info;
	register WindowPtr w;
	Rect r;
	long lnumcursor;
	
	opvalidate (ho);
	
	menudata = hm; 
	
	clearbytes (&info, sizeof (info));
	
	info.versionnumber = conditionalshortswap (1);
	
	info.adroutline = conditionallongswap ((**hm).adroutline);
	
	info.vertmin = conditionalshortswap ((**ho).vertscrollinfo.min);
	
	info.vertmax = conditionalshortswap ((**ho).vertscrollinfo.max);
	
	info.vertcurrent = conditionalshortswap ((**ho).vertscrollinfo.cur);
	
	w = (**hm).scriptwindow;
	
	if (w == nil) 
		r = (**hm).scriptwindowrect;
	else
		getglobalwindowrect (w, &r);
	
	recttodiskrect (&r, &info.scriptwindowrect);
	
	if ((**hm).flautosmash)
		info.flags |= flautosmash_mask;
	
	recttodiskrect (&(**hm).menuwindowrect, &info.menuwindowrect);
	
	diskgetfontname ((**hm).defaultscriptfontnum, info.defaultscriptfontname);
	
	info.defaultscriptfontsize = conditionalshortswap ((**hm).defaultscriptfontsize);
	
	oppushoutline (ho);
	
	hcursor = (**ho).hbarcursor;
	
	opgetnodeline (hcursor, &lnumcursor);

	info.lnumcursor = conditionalshortswap (loword (lnumcursor));
	
	oppopallhoists (); /*pop hoists, save state to be restored after saving*/
	
	(**ho).hbarcursor = hcursor; /*scotch tape!*/
	
	fl = true;
	
	if (flpreservelinks) {
		
		/*
		we're saving to another file, and need to preserve the memory version 
		of the menubar.  we'll make a copy that we can traverse and save 
		with the refcon links set up for the destination file.
		since we don't want to copy the scripts, and don't want them disposed 
		when we dispose the copy, we need to patch in op's default refcon
		callbacks while we do the copy.
		*/
		
		hdloutlinerecord hcopy;
		
		(**ho).copyrefconcallback = &opcopyrefconroutine;
		
		(**ho).releaserefconcallback = &opdefaultreleaserefconroutine;
		
		opsaveeditbuffer ();
		
		fl = opcopyoutlinerecord (ho, &hcopy);
		
		oprestoreeditbuffer ();
		
		(**ho).copyrefconcallback = &mecopyrefconroutine;
		
		(**ho).releaserefconcallback = &mereleaserefconroutine;
		
		if (!fl)
			goto exit;
		
		ho = opsetoutline (hcopy); /*work on copy*/
		}
	
	if (fl) {
	
		if (flmemory) {
			
			dbpushdatabase (megetdatabase (hm));
			
			fl = mepackmenustructure (&info, hpacked);
			
			dbpopdatabase ();
			}
		else {
			fl = mesavemenustructure (&info, adr);
			
			if (fl && (!fldatabasesaveas))
				(**hm).adroutline = conditionallongswap(info.adroutline);
			}
		}
	
	if (flpreservelinks) {
		
		opdisposeoutline (ho, false); /*dispose copy*/
		
		opsetoutline ((**hm).menuoutline); /*restore original*/
		}
	
	exit:
	
	oprestorehoists (); /*restore the hoist state*/
	
	oppopoutline (); /*pop outline globals*/
	
	return (fl);
	} /*mesavemenurecord*/


boolean mesetupmenurecord (tysavedmenuinfo *info, hdloutlinerecord houtline, hdlmenurecord *hmenurecord) { 
	
	/*
	create a new menu record with the given outline and setup info
	
	2/26/91 dmb: commented-out assignment into outline's vertical scroll 
	position, and use of info.lnumcursor.  all this stuff should already 
	be saved with the outline.  note: if it turns out that we need to 
	assign lnumcursor into movecursorto, we need to defeat it during 
	searches
	
	4/22/91 dmb: menu outline now points to menu variable
	*/
	
	register tysavedmenuinfo *pi = info;
	register hdlmenurecord hm;
	register hdloutlinerecord ho;
	short fontnum;
	
	if (!newclearhandle (sizeof (tymenurecord), (Handle *) hmenurecord))
		return (false);
	
	hm = *hmenurecord; /*copy into register*/
	
	ho = houtline; /*copy into register*/
	
	mesetcallbacks (ho); /*link in callback routines*/
	
	(**hm).menuoutline = ho; 
	
	(**ho).outlinerefcon = (long) hm; /*pointing is mutual*/
	
	(**hm).adroutline = conditionallongswap((*pi).adroutline); /*keep address around for save*/
	
	diskrecttorect (&(*pi).scriptwindowrect, &(**hm).scriptwindowrect);
	
	(**hm).flautosmash = ((*pi).flags & flautosmash_mask) != 0;
	
	diskrecttorect (&(*pi).menuwindowrect, &(**hm).menuwindowrect);
	
	diskgetfontnum ((*pi).defaultscriptfontname, &fontnum);
	
	(**hm).defaultscriptfontnum = fontnum;
	
	(**hm).defaultscriptfontsize = conditionalshortswap((*pi).defaultscriptfontsize);
	
	(**hm).menuactiveitem = menuoutlineitem;
	
	return (true);
	} /*mesetupmenurecord*/


static boolean meunpackscriptvisit (hdlheadrecord hnode, ptrvoid refcon) {
	
	/*
	4/30/91 dmb: set dirty bit on outline to make sure it gets saved
	*/
	
	register hdlheadrecord h = hnode;
	ptrpackinfo packinfo = (ptrpackinfo) refcon;
	hdloutlinerecord houtline;
	tymenuiteminfo item;
	
	if (!megetmenuiteminfo (h, &item)) /*nothing linked, keep visiting*/
		return (true);
	
	if (!item.linkedscript.adrlink && !item.linkedscript.houtline) /*no linked script*/
		return (true);
	
	if (!opunpack ((*packinfo).hpackedscripts, &(*packinfo).ixpackedscripts, &houtline)) /*memory error, stop visiting*/
		return (false);
	
	(**houtline).fldirty = true; /*make sure it gets saved*/
	
	item.linkedscript.houtline = houtline; /*in memory*/
	
	item.linkedscript.adrlink = nildbaddress; /*never saved to disk*/
	
	return (mesetmenuiteminfo (h, &item));
	} /*meunpackscriptvisit*/


boolean meunpackmenustructure (Handle hpacked, hdlmenurecord *hmenurecord) {
	
	/*
	analagous to meloadmenurecord below, except the entire structure is 
	to be unpacked from the handle hpacked.
	
	9/25/91 dmb: use new testheapspace instead of haveheapspace for preflighting. 
	the latter doesn't generate an error, so we could have failed silently.
	*/
	
	Handle hpackedmenu;
	hdlheadrecord hsummit;
	hdloutlinerecord ho;
	register boolean fl;
	tysavedmenuinfo info;
	long ix = 0;
	typackinfo packinfo;
	
	assert (sizeof (tysavedmenuinfo) == sizeof (tyOLD42savedmenuinfo));
	
	if (!unmergehandles (hpacked, &hpackedmenu, &packinfo.hpackedscripts)) /*consumes hpacked*/
		return (false);
	
//	oppushoutline (nil); /*preserve global*/
	
	fl = loadfromhandle (hpackedmenu, &ix, sizeof (info), &info);
	
	info.adroutline = nildbaddress; /*never been saved*/
	
	if (fl)
		fl = opunpack (hpackedmenu, &ix, &ho);
	
//	ho = outlinedata; /*save in register.  may be nil*/
	
	disposehandle (hpackedmenu);
	
	if (fl)
		fl = mesetupmenurecord (&info, ho, hmenurecord);
	
	if (fl)
		fl = testheapspace (gethandlesize (packinfo.hpackedscripts)); /*a little preflighting*/
	
	if (fl) {
		
		packinfo.ixpackedscripts = 0; /*offset into hpackedscripts*/
		
		oppushoutline (ho);
		
		opoutermostsummit (&hsummit);
		
		fl = opsiblingvisiter (hsummit, false, &meunpackscriptvisit, &packinfo);
		
		oppopoutline ();
		}
	
	disposehandle (packinfo.hpackedscripts);
	
	if (!fl)
		opdisposeoutline (ho, false); /*checks for nil*/
	
//	oppopoutline (); /*restore global*/
	
	return (fl);
	} /*meunpackmenustructure*/


boolean meloadmenurecord (dbaddress adr, hdlmenurecord *hmenurecord) { 
	
	hdloutlinerecord houtline;
	tysavedmenuinfo info;
	
	if (!dbreference (adr, sizeof (info), &info))
		return (false);
	
	if (!meloadoutline (conditionallongswap (info.adroutline), &houtline))
		return (false);
	
	if (!mesetupmenurecord (&info, houtline, hmenurecord)) {
		
		opdisposeoutline (houtline, false);
		
		return (false);
		}
	
	return (true);
	} /*meloadmenurecord*/


static void medisposescrap (hdloutlinerecord houtline) {
	
	opdisposeoutline (houtline, false);
	} /*medisposescrap*/


static boolean meexportscrap (hdloutlinerecord houtline, tyscraptype totype, Handle *hexport, boolean *fltempscrap) {
	
	tysavedmenuinfo info;
	boolean fl;
	
	*fltempscrap = true; /*usually the case*/
	
	switch (totype) {
		
		case menuscraptype: /*export flat version for system scrap*/
			
			oppushoutline (houtline);
			
			clearbytes (&info, sizeof (info));
			
			fl = mepackmenustructure (&info, hexport);
			
			oppopoutline ();
			
			return (fl);
		
		case opscraptype:
			*hexport = (Handle) houtline; /*op and script scraps are the same*/
			
			*fltempscrap = false; /*it's the original, not a copy*/
			
			return (true);
		
		case textscraptype:
			return (opoutlinetonewtextscrap (houtline, hexport));
		
		default:
			return (false);
		}
	} /*meexportscrap*/


boolean mesetscraproutine (hdloutlinerecord houtline) {
	
	return (shellsetscrap ((Handle) houtline, menuscraptype,
								(shelldisposescrapcallback) &medisposescrap,
								(shellexportscrapcallback) &meexportscrap));
	} /*mesetscraproutine*/


boolean megetscraproutine (hdloutlinerecord *houtline, boolean *fltempscrap) {
	
	Handle hscrap;
	tyscraptype scraptype;
	
	if (!shellgetscrap (&hscrap, &scraptype))
		return (false);
	
	if (scraptype == menuscraptype) {
		
		*houtline = (hdloutlinerecord) hscrap;
		
		*fltempscrap = false; /*we're returning a handle to the actual scrap*/
		
		return (true);
		}
	
	return (shellconvertscrap (opscraptype, (Handle *) houtline, fltempscrap));
	} /*megetscraproutine*/


boolean mescraphook (Handle hscrap) {
	
	/*
	if our private type is on the external clipboard, set the internal 
	scrap to it.
	*/
	
	if (getscrap (menuscraptype, hscrap)) {
		
		hdlmenurecord hmenurecord;
		
		if (meunpackmenustructure (hscrap, &hmenurecord)) {
			
			mesetscraproutine ((**hmenurecord).menuoutline);
			
			(**hmenurecord).menuoutline = nil;
			
			medisposemenurecord (hmenurecord, false);
			}
		
		return (false); /*don't call any more hooks*/
		}
	
	return (true); /*keep going*/
	} /*mescraphook*/


