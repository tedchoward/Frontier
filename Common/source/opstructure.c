
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

#include "quickdraw.h"
#include "memory.h"
#include "frontier_strings.h"
#include "scrap.h"
#include "cursor.h"
#include "kb.h"
#include "ops.h"
#include "search.h"
#include "shellundo.h"
#include "shell.rsrc.h"
#include "op.h"
#include "opinternal.h"
#include "oplineheight.h"
#include "lang.h" // for flscriptrunning
#include "tablestructure.h" /*7.0b6 PBS: for opCursorMoved callback*/
#include "process.h" /*7.0b8 PBS: check to see if we're debugging*/



typedef struct tycopyinfo {
	
	hdlheadrecord hnode;
	
	short level;
	} tycopyinfo, *ptrcopyinfo;


static hdlscreenmap hundomap; /*temporary screen map shared by opbefore/afterundo*/



typedef struct tydepositinfo {

	hdlheadrecord hpre, hdeposit;
	
	tydirection dir;
	} tydepositinfo, *ptrdepositinfo, **hdldepositinfo;


void opstartinternalchange (void) {
	
	/*
	5.1.3 dmb: flag is now a byte, make nestable
	*/
	
	if (outlinedata)
		(**outlinedata).flinternalchange++;
	} /*opstartinternalchange*/


void opendinternalchange (void) {
	
	if (outlinedata)
		(**outlinedata).flinternalchange--;
	} /*opendinternalchange*/


boolean opinternalchange (void) {
	
	/*
	5.0d11 dmb: flinternalchange is now outline-specific
	*/
	
	return (outlinedata && (**outlinedata).flinternalchange);
	} /*opendinternalchange*/


boolean opnodechanged (hdlheadrecord hnode) {
	
	/*
	5.0a24 dmb: I don't know why we should have to do something as
	display intensive as invalidate a node every time it's "changed",
	something that happens all over the place during structure 
	changes. Maybe clay basket needed it, but as far as I can tell
	it's totally unnecessary, and clearly there's a performance hit.
	
	most importantly, this caused a bug in scrollrect under NT, where
	the invalidated area fouled up its basic operation
	*/

	/*
	opnodecallback cb = (**outlinedata).nodechangedcallback;
	
	if (cb != nil) { // we can get called before the top level has been able to init this, workaround problem
		
		opinvalnode (hnode);
		
		return ((*cb) (hnode));
		}
	*/
	
	(**hnode).hpixels = (**hnode).vpixels = opdirtymeasurevalue;
	
	return (true);
	} /*opnodechanged*/


void opsetline1 (hdlheadrecord hline1) {
	
	register hdloutlinerecord ho = outlinedata;
	
	(**ho).hline1 = hline1;
	
	(**ho).line1linesabove = 0;
	} /*opsetline1*/


static boolean opundounmark (hdlheadrecord hnode, boolean flundo) {
	
	/*
	8/28/92 dmb: the undo process must clear all marks in case the user has 
	made a new selection since the original change. this routine is used to 
	ensure that the original selection is restored
	*/
	
	if (flundo) {
		
		if (!(**hnode).flmarked) {
			
			(**hnode).flmarked = true;
			
			(**outlinedata).ctmarked++;
			}
		
		oppushundo (&opundounmark, hnode);
		}
	
	return (true);
	} /*opundounmark*/


boolean oppushunmarkundo (hdlheadrecord hnode) {
	
	/*
	8/28/92 dmb: setup undo to maintain selection for reorg operation.
	
	this undo should be pushed while hnode is still in the outline; 
	not between an unlink and a deposit
	*/
	
	if ((**hnode).flmarked)
		oppushundo (&opundounmark, hnode);
	
	return (true);
	} /*oppushunmarkundo*/


static boolean opundodeposit (hdlheadrecord hnode, boolean flundo) {

	if (flundo) {
		
		opunlink (hnode);
		
		if ((**hnode).flmarked)
			(**outlinedata).ctmarked--;
		}
	
	return (true);
	} /*opundodeposit*/


static boolean oppushdepositundo (hdlheadrecord hnode) {
	
	return (oppushundo (&opundodeposit, hnode));
	} /*oppushdepositundo*/


static boolean opdeletelinecallback (hdlheadrecord hnode) {

	if (!outlinedata || opinternalchange ())
		return (false);
	
	return ((*(**outlinedata).deletelinecallback) (hnode));
	} /*opdeletelinecallback*/


static boolean opinsertlinecallback (hdlheadrecord hnode) {
	
	if (!outlinedata || opinternalchange ())
		return (false);
	
	return ((*(**outlinedata).insertlinecallback) (hnode));
	} /*opinsertlinecallback*/


static void opchoosesafenodes (hdlheadrecord hdangerous, hdlheadrecord hsafe) {

	register hdloutlinerecord ho = outlinedata;
	
	if (hdangerous == (**ho).hline1) /*unlinking first line in display*/
		opsetline1 (hsafe);
		
	if (hdangerous == (**ho).hbarcursor) /*unlinking the bar cursor line, pretty good*/
		(**ho).hbarcursor = hsafe;
		
	if (hdangerous == (**ho).hsummit) /*unlinking the summit, awesome*/
		(**ho).hsummit = hsafe;
	} /*opchoosesafenodes*/


boolean oppushundo (opundocallback pundo, hdlheadrecord hnode) {
	
	/*
	2/19/91 dmb: this used to be a macro to simply cast hnode as a Handle.  
	now, all outline undo goes through here, and we short circuit when 
	appropriate.
	*/
	
	#if !fljustpacking
	
		if (outlinedata == nil) /*PBS 7.0b49: this can happen when getting OSA code, which doesn't require set global*/
			return (false);

		if (opinternalchange () || !(**outlinedata).flbuildundo) {
			
			(*pundo) (hnode, false);
			
			return (true);
			}
		else
			return (pushundostep ((undocallback) pundo, (Handle) hnode));
		
	#endif
	} /*oppushundo*/


static boolean opundounlink (hdldepositinfo hdepositinfo, boolean flundo) {
	
	/*
	1/24/91 dmb: when undoing, we need to determine whether or not we need 
	reset levels.
	*/
	
	if (flundo) {
		
		register hdldepositinfo hdi = hdepositinfo;
		register hdlheadrecord hdeposit = (**hdi).hdeposit;
		register short level = (**hdeposit).headlevel;
		
		opdeposit ((**hdi).hpre, (**hdi).dir, (**hdi).hdeposit);
		
		if ((**hdeposit).headlevel != level)
			opresetlevels (hdeposit);
		
		if ((**hdeposit).flmarked) {
			
			opexpandto (hdeposit);
			
			(**outlinedata).ctmarked++;
			}
		}
	
	disposehandle ((Handle) hdepositinfo);
	
	return (true);
	} /*opundounlink*/


void opunlink (hdlheadrecord hnode) {
	
	/*
	point all elements of the structure around the indicated node, and 
	set his "outbound" pointers (ie everything but his right pointer)  
	to point at himself.

	6/20/90 dmb: callback to deleteline
	6/27/90 dmb: added undo
	*/
	
	register hdlheadrecord h = hnode;
	register hdlheadrecord hdown = (**h).headlinkdown;
	register hdlheadrecord hup = (**h).headlinkup;
	register hdlheadrecord hleft = (**h).headlinkleft;
	tydepositinfo depositinfo;
	Handle hundo;
	
	if (h == nil) /*defensive driving*/
		return;
	
	opnodechanged (h);
	
	opnodechanged (hup); 
	
	opnodechanged (hdown);
	
	opdeletelinecallback (h);

	depositinfo.hdeposit = h;
	
	if (hup == h) { /*unlinking first guy at this level*/

		if (hleft == h) { /*has no parent -- undo by depositing up from downlink*/
		
			depositinfo.hpre = hdown;
			
			depositinfo.dir = up;
			
			assert (hdown != h);
			}
		else { /*has a parent -- undo by depositing right from leftlink*/
		
			depositinfo.hpre = hleft;

			depositinfo.dir = right;
			}

		if (hdown == h) { /*only child*/

			opchoosesafenodes (h, hleft);

			(**hleft).headlinkright = hleft; /*eliminate parent's child list*/
			
			goto L1;
			}

		opchoosesafenodes (h, hdown);

		if (hleft != h) /*has a parent*/
			(**hleft).headlinkright = hdown;
		
		(**hdown).headlinkup = hdown; /*he's now the 1st kid*/
		
		goto L1;
		}

	/*has a sibling above -- undo by depositing down from uplink*/

	depositinfo.hpre = hup;

	depositinfo.dir = down;

	opchoosesafenodes (h, hup);

	if (hdown == h) { /*last guy in his list*/
	
		(**hup).headlinkdown = hup; /*he's now the last kid*/

		goto L1;
		}

	(**hup).headlinkdown = hdown; /*point around me*/
	
	(**hdown).headlinkup = hup;
	
	L1: /*goto here to exit*/
	
	if (newfilledhandle ((ptrchar) &depositinfo, longsizeof (tydepositinfo), &hundo))
		oppushundo ((opundocallback) &opundounlink, (hdlheadrecord) hundo);

	(**h).headlinkup = (**h).headlinkdown = (**h).headlinkleft = h;
	} /*opunlink*/


static void opdepositdown (hdlheadrecord hpre, hdlheadrecord hdeposit) {
	
	/*
	deposit the indicated node down from hpre.
	
	modified 11/9/88 to support multiple level-0 nodes DW.
	
	11/10/88 handles moving the first summit down, updates 
	(**outlinedata).hsummit.
	
	12/23/88 more efficient code, use registers better.

	6/20/90 dmb: callback to insertline
	
	12/29/91 dmb: make sure flexpanded is consistent with hpre
	*/
	
	register hdlheadrecord hp = hpre;
	register hdlheadrecord hd = hdeposit;
	register short headlevel = (**hp).headlevel;
	
	opnodechanged (hp);
	
	if (headlevel == 0)
		(**hd).headlinkleft = hd; /*points back at himself*/
	else
		(**hd).headlinkleft = (**hp).headlinkleft; /*inherits parent from pre*/
	
	(**hd).headlevel = headlevel; /*inherits level from pre*/
	
	(**hd).flexpanded = (**hp).flexpanded; /*must be consistent*/
	
	if ((**hp).headlinkdown == hp) /*inserting as the last in the list*/
	
		(**hd).headlinkdown = hd; /*point back at himself*/
		
	else { /*inserting in the middle of the list*/
		
		register hdlheadrecord x = (**hp).headlinkdown;
		
		(**hd).headlinkdown = x;
		
		(**x).headlinkup = hd;
		
		opnodechanged (x);
		}
	
	(**hd).headlinkup = hp;
	
	(**hp).headlinkdown = hd;

	opinsertlinecallback (hd);
	
	#if !fljustpacking
	
		oppushdepositundo (hd);
	
	#endif
	
	opnodechanged ((**hd).headlinkleft); /*the parent's list changed*/
	
	opnodechanged ((**hd).headlinkdown); /*the next guy's prev pointer changed*/
	} /*opdepositdown*/


static void opdepositright (hdlheadrecord hparent, hdlheadrecord hdeposit) {

	/*
	6/20/90 dmb: callback to insertline
	
	12/29/91 dmb: make sure flexpanded is consistent with sibling
	
	8/28/92 dmb: if only child of non-expanded parent, clear expanded bit
	*/

	register hdlheadrecord hp = hparent;
	register hdlheadrecord hd = hdeposit;
	register hdlheadrecord hr = (**hp).headlinkright;
	
	opnodechanged (hp);
	
	(**hp).fldirty = true;
	
	(**hd).headlevel = (**hp).headlevel + 1;
	
	(**hd).headlinkleft = hp;
	
	(**hd).headlinkup = hd; /*points at himself*/
	
	if (hr == hp) { /*first kid*/
		
		(**hd).headlinkdown = hd; /*points at himself*/
		
		(**hp).fldirty = true;
		
		if (!(**hp).flexpanded)
			(**hd).flexpanded = false; /*must be consistent*/
		}
	else {
		(**hd).headlinkdown = hr;
		
		(**hr).headlinkup = hd;
		
		(**hd).flexpanded = (**hr).flexpanded; /*must be consistent*/
		
		opnodechanged (hr); /*the former first kid*/
		}
	
	(**hp).headlinkright = hd;
	
	opinsertlinecallback (hd);
	
	#if !fljustpacking
	
		oppushdepositundo (hd);
		
	#endif
	} /*opdepositright*/


static void opdepositup (hdlheadrecord hpre, hdlheadrecord hdeposit) {
	
	/*
	deposit the indicated node up from hpre.  special case considerations if
	we're moving at the very topmost level of the structure.

	6/20/90 dmb: callback to insertline
	
	12/29/91 dmb: make sure flexpanded is always set
	*/
	
	register hdlheadrecord hp = hpre;
	register hdlheadrecord hd = hdeposit;
	register hdlheadrecord hleft;
	register hdloutlinerecord ho = outlinedata;
	
	opnodechanged (hp);
			
	if (!opfirstinlist (hp)) { /*simple in every case*/
	
		opdepositdown ((**hp).headlinkup, hd);
		
		return;
		}
	
	hleft = (**hp).headlinkleft;
	
	if (hleft != hp) { /*pre has a parent*/
	
		opdepositright (hleft, hd); /*insert to right of pre's parent*/
		}
		
	else { /*insert as the new 0-level summit*/
		
		(**hd).headlevel = 0;
		
		(**hd).headlinkup = hd; 
		
		(**hd).headlinkdown = hp;
		
		(**hd).flexpanded = true; /*summits are always expanded*/
		
		(**hp).headlinkup = hd;
		
		if (hp == (**ho).hsummit) /*becomes the new summit*/
			(**ho).hsummit = hd;
		
		opinsertlinecallback (hd);
		
		#if !fljustpacking
		
			oppushdepositundo (hd);
		
		#endif
		}
		
	if (hp == (**ho).hline1) /*becomes the new first line*/
		opsetline1 (hd);
	} /*opdepositup*/


boolean opdeposit (hdlheadrecord hpre, tydirection dir, hdlheadrecord hdeposit) {
	
	switch (dir) {
	
		case down:
			opdepositdown (hpre, hdeposit);
			
			break;
			
		case right:
			opdepositright (hpre, hdeposit);
			
			break;
			
		case up:
			opdepositup (hpre, hdeposit);
			
			break;
			
		case sorted:
			#if false /*!fljustpacking*/
			
				opsorteddeposit (hpre, right, hdeposit);
			
			#endif
			
			break;
		
		default:
			return (false); /*invalid direction for deposit*/
		} /*switch*/
	
	return (true);
	} /*opdeposit*/


boolean opmoveto (hdlheadrecord hnode) {
	
	/*
	move the structure cursor to the indicated node.
	
	the flcursorneedsdisplay flag lets an external user insist that no matter
	what the cursor line must be displayed.
	
	returns true if it had to scroll vertically to make the cursor visible.
	
	automatically adjust the horizontal scrollbar to make the new cursor line
	fully visible horizontally (if possible).
	
	5/4/93 dmb: don't need to docursor if we just visiscrolled
	*/
	
	register hdloutlinerecord ho = outlinedata;
	register hdlheadrecord h = hnode;
	long hscroll, vscroll;
	register boolean flvisiscroll;
	
	if (((**ho).hbarcursor == hnode) && (!(**ho).flcursorneedsdisplay))
		return (false);
	
	opdirtyview ();
	
	(**ho).flcursorneedsdisplay = false; /*must be reset every time*/
	
	opunloadeditbuffer ();
	
	if ((**ho).flbarcursoron) /*un-highlight the old bar cursor line*/
		opdocursor (false); 
	
	(**ho).hbarcursor = h;
	
	flvisiscroll = opneedvisiscroll (h, &hscroll, &vscroll, false);
	
	if (flvisiscroll)
		opdovisiscroll (hscroll, vscroll);
	
	oploadeditbuffer ();
	
	if (!(/*dmb 11/11/96 - flvisiscroll ||*/ (**ho).fltextmode)) /*if in text mode, drawing already happened*/
		opdocursor (true);
	
	opschedulevisi (); /*may need horizontal scroll, once idle*/
	
	if (!debuggingcurrentprocess () && opdisplayenabled ()) /*7.0b8: don't run callbacks if debugging*/
	
		langopruncallbackscripts (idopcursormovedscript); /*7.0b6 PBS: call callback when cursor moves*/
	
	return (flvisiscroll);
	} /*opmoveto*/
	
	
#if false

static boolean oldopjumpto (hdlheadrecord hnode) {
	
	/*
	a special way to move the cursor when you know it's going a long 
	distance. no point erasing the old cursor. no point trying to scroll
	or erase the display. everything is going to be updated. do it all
	in one shot for nice staging.
	*/
	
	long hscroll, vscroll;
	
	opinvaldisplay (); /*everything will get redrawn*/
	
	(**outlinedata).hbarcursor = hnode;
	
	if (opneedvisiscroll (hnode, &hscroll, &vscroll, false)) {
	
		opvertscrollrecord (&vscroll); 

		/*ophorizscrollrecord (&vscroll);*/
		}
	} /*oldopjumpto*/

#endif
	
	
boolean opjumpto (hdlheadrecord hnode) {
	
	/*
	dmb 11/8/96: keep it simple here, let opdisplay take care of all the updating
	*/
	
	hdlheadrecord oldbarcursor = (**outlinedata).hbarcursor;
	
	(**outlinedata).hbarcursor = hnode;
	
	opjumpdisplayto (oldbarcursor, hnode);
	
	return (true);
	} /*opjumpto*/


boolean opflatfind (boolean flfromtop, boolean flwrap) {
	
	/*
	search for the search string, bumping our way flatdown as we go and
	wrapping around to the top when we reach the last node.
	
	return true if the search was successful, with the outline cursor on the
	node.
	
	8/21/90 DW: add flwrap parameter -- if false the search stops at the last
	node, doesn't wrap around to the first summit.
	
	7/15/91 dmb: never return true when doing a replace all; keep going 
	until we fail
	
	9/12/91 dmb: fixed replace all logic bugs.
	
	4/26/96 dmb: never automatically bump; it not in text mode, search from 
	beginning of current headline
	*/
	
	register hdloutlinerecord ho = outlinedata;
	register hdlheadrecord nomad;
	register hdlheadrecord orignomad;
	register hdlheadrecord nextnomad;
	register short ix;
	Handle hstring;
	long startsel, endsel;
	boolean flbump;
	long ixfind = 0, lenfind;
	
	flbump = !flscriptrunning;	// 4/26/96 dmb: was true
	
	if (flfromtop) {
		
		nomad = (**ho).hsummit;
		
		flbump = false;
		}
	else {
		
		nomad = (**ho).hbarcursor;
		
		if (opeditingtext (nomad)) {
			
			opeditgetselection (&startsel, &endsel);
			
			ixfind = max (startsel, endsel);
			
			opwriteeditbuffer (); /*make sure we search the current text*/
			
			flbump = false;
			}
		}
	
	orignomad = nomad;
	
	while (true) {		
		
		if (!flbump)
			flbump = true; /*set for next time*/
		
		else {
			nextnomad = opbumpflatdown (nomad, false);
			
			if (nextnomad == nomad) { /*no way flatdown, maybe wrap around to top*/
				
				if (!flwrap)
					return (false);
				
				nextnomad = (**ho).hsummit;
				}
			
			if (nextnomad == orignomad) /*back where we started, search failed*/
				return (false);
			
			nomad = nextnomad; /*advance to next node flatdown with wrap*/
			
			ixfind = 0;
			}
		
		rollbeachball ();
	
		hstring = (**nomad).headstring;
		
		if (handlesearch (hstring, &ixfind, &lenfind)) { /*found match, stop searching*/
			
			if (keyboardescape ()) /*use didn't want this to happen*/
				return (false);
			
			opexpandto (nomad);
			
			opsettextmode (true);
			
			ix = ixfind;
			
			opeditsetselection (ix, ix + lenfind);
			
			if (!searchparams.flreplaceall) { /*we're done*/
				
				opschedulevisi (); /*make sure new selection is visible*/
				
				return (true);
				}
			
			 /*do replacement while we have control*/
		 	
			if (oprestoreeditbuffer ()) {
				
				opeditinsert (searchparams.bsreplace);
				
				opwriteeditbuffer ();
				}
			else {
				
				if (!copyhandle (hstring, &hstring))
					return (false);
				
				if (!mungehandle (hstring, ix, lenfind,
							stringbaseaddress (searchparams.bsreplace),
							stringlength (searchparams.bsreplace)))
					return (false);
				
				opsetheadtext (nomad, hstring);
				}
			
			++searchparams.ctreplaced;
			
			flbump = false;
			
			ixfind += stringlength (searchparams.bsreplace);
			
			continue; /*next iteration of loop*/
			}
		
		if ((*(opnodecallback)(**ho).searchrefconcallback) (nomad)) {
			
			if (!searchparams.flreplaceall)
				return (true);
			}
		} /*while*/
	} /*opflatfind*/


boolean opgetsafenode (hdlheadrecord *hnode) {
	
	/*
	look for a new place to put the node.  first try going up, then down.
	then left.  right isn't an option since we're assuming that the line that
	the node is on is about to be unlinked from the structure and therefore
	it isn't a "safe" place.
	*/
	
	register hdlheadrecord hdangerous = *hnode;
	register hdlheadrecord h;
	
	h = (**hdangerous).headlinkup;
	
	if (h != hdangerous) /*found a safe place*/
		goto L1;
		
	h = (**hdangerous).headlinkdown;
	
	if (h != hdangerous) /*found a safe place*/
		goto L1;
		
	h = (**hdangerous).headlinkleft;
	
	if (h != hdangerous) /*found a safe place*/
		goto L1;
		
	return (false); /*couldn't move down, up or left*/
	
	L1:
	
	*hnode = h;
	
	return (true);
	} /*opgetsafenode*/


static boolean opchecksafenode (hdlheadrecord hdelete, hdlheadrecord *hsafe) {
	
	/*
	check to see if hsafe or any subelement is the first line in the
	display.  if so, change the first line to something nearby.
	
	return true if hsafe needed to be moved.
	*/
	
	while (true) {
		
		if (*hsafe == hdelete) { /*danger*/
			
			return (opgetsafenode (hsafe)); /*should always be true*/
			}
		
		if (!opchaseleft (hsafe)) /*safe*/
			return (false);
		}
	} /*opchecksafenode*/


static boolean opcheckline1 (hdlheadrecord hdelete) {
	
	/*
	check to see if hdelete or any subelement is the first line in the
	display.  if so, change the first line to something nearby.
	*/
	
	hdlheadrecord hline1 = (**outlinedata).hline1;
	
	if (opchecksafenode (hdelete, &hline1))
		opsetline1 (hline1);
	
	return (true);
	} /*opcheckline1*/


static boolean opsafebarcursor (hdlheadrecord hdelete) {
	
	/*
	look for a new place to put the bar cursor.  first try going up, then down.
	then left.  right isn't an option since we're assuming that the line that
	the bar cursor is on is about to be unlinked from the structure.
	*/
	
	hdlheadrecord hcursor = (**outlinedata).hbarcursor;
	
	if (opchecksafenode (hdelete, &hcursor))
		(**outlinedata).hbarcursor = hcursor;
	
	return (true);
	} /*opsafebarcursor*/


boolean opmovecursor (hdlheadrecord hpre, tydirection dirmove, long units, hdlheadrecord *hnew) {
	
	/*
	return with *hnew pointing to the node in the indicated
	direction from hpre.
	
	if there was no way to move in the indicated direction, 
	return false.
	*/
	
	register tydirection dir = dirmove;
	register hdlheadrecord h = hpre;
	register hdlheadrecord horig;
	register boolean flmoved = false;
	register long ct = units;
	
	while (--ct >= 0) {
		
		horig = h;
		
		switch (dir) {
		
			case right: {
				
				register hdlheadrecord hright = (**h).headlinkright;
				
				if ((**hright).flexpanded) /*only move into expanded nodes*/
					h = hright;
					
				break;
				}
				
			case left:
				h = (**h).headlinkleft;
				
				break;
				
			case up:
				h = (**h).headlinkup;
				
				break;
				
			case down:
				h = (**h).headlinkdown;
				
				break;
				
			case flatup:
				h = opbumpflatup (h, true);
				
				break;
				
			case flatdown:
				h = opgetnextexpanded (h);
				
				break;
			
			default:
				/* nothing to do */
				break;
				
			} /*switch*/
			
		if (h == horig) /*didn't move, break out of loop*/
			goto L1;
			
		flmoved = true;
		} /*for*/
	
	L1:
	
	*hnew = h; /*return new position for caller*/
		
	return (flmoved);
	} /*opmovecursor*/
	

boolean opcopyrefconroutine (hdlheadrecord hsource, hdlheadrecord hdest) {
	
	/*
	the default version of the callback that copies the refcon handle linked
	into the original node and links the copy into the destination.
	*/
	
	Handle hrefcon = (**hsource).hrefcon;
	Handle hcopy;
	
	(**hdest).hrefcon = nil; /*default*/
		
	if (hrefcon == nil)	/*very easy to copy a nil refcon*/
		return (true);

	if (!copyhandle (hrefcon, &hcopy)) 
		return (false);
		
	(**hdest).hrefcon = hcopy;
	
	return (true);
	} /*opcopyrefconroutine*/
	

static hdlheadrecord opcopyheadrecord (hdlheadrecord horig) {
	
	register hdlheadrecord hcopy;
	hdlheadrecord hnewnode;
	Handle hstring;
	
	if (!copyhandle ((**horig).headstring, &hstring))
		return (false);
	
	if (!opnewheadrecord (hstring, &hnewnode)) 
		return (nil);
	
	hcopy = hnewnode; /*copy into register*/
	
	(**hcopy).headlevel = (**horig).headlevel; /*inherits level*/
	
	(**hcopy).flexpanded = (**horig).flexpanded; /*inherits expanded state*/
	
	(**hcopy).flcomment = (**horig).flcomment; 

	(**hcopy).fldynamic = (**horig).fldynamic; /*7.0b30 PBS: copy fldynamic flag*/
	
	(**hcopy).appbit0 = (**horig).appbit0; /*7/30/93 DW: for forking in Clay Basket*/
	
	(**hcopy).appbit1 = (**horig).appbit1; 
	
	(**hcopy).appbit2 = (**horig).appbit2; 
	
	(**hcopy).appbit3 = (**horig).appbit3; 
	
	(**hcopy).appbit4 = (**horig).appbit4; 
	
	(**hcopy).appbit5 = (**horig).appbit5; 
	
	(**hcopy).appbit6 = (**horig).appbit6; 
	
	(**hcopy).appbit7 = (**horig).appbit7; 
	
	if ((**horig).hrefcon != nil) { /*the original has a refcon handle attached*/
		
		boolean fl;
		hdloutlinerecord x = outlinedata; /*we preserve x, y and z*/
		
		fl = (*(**outlinedata).copyrefconcallback) (horig, hcopy);
		
		opsetoutline (x); /*restore to original*/
		
		if (!fl) {
		
			opreleasenode (hcopy, false); /*dispose of the partial copy*/
			
			return (nil);
			}
		}
	
	return (hcopy);
	} /*opcopyheadrecord*/
	

static boolean opmoveoutlineup (void) {
	
	register hdlheadrecord hcursor = (**outlinedata).hbarcursor;
	register hdlheadrecord hpre = (**hcursor).headlinkup;
	
	if (hpre == hcursor) /*he's the first in the list*/
		return (false);
	
	opunlink (hcursor);
	
	opdepositup (hpre, hcursor);
	
	return (true);
	} /*opmoveoutlineup*/
	
	
static boolean opmoveoutlinedown (void) {
	
	/*
	in this operation we must preserve the cursor, in case there are 
	multiple moves involved.  the unlink operation keeps the barcursor
	pointing at a valid headline.
	*/
	
	register hdloutlinerecord ho = outlinedata;
	register hdlheadrecord hcursor = (**ho).hbarcursor;
	register hdlheadrecord hpre = (**hcursor).headlinkdown;
	
	if (hpre == hcursor) /*he's the last guy in the list*/
		return (false);
	
	opunlink (hcursor);
	
	opdepositdown (hpre, hcursor);
	
	(**ho).hbarcursor = hcursor; /*restore...*/
	
	return (true);
	} /*opmoveoutlinedown*/
	

static boolean opmoveoutlineleft (void) {
	
	register hdloutlinerecord ho = outlinedata;
	register hdlheadrecord hcursor = (**ho).hbarcursor;
	register hdlheadrecord hpre = (**hcursor).headlinkleft;
	
	if (hpre == hcursor) /*trying to move a summit to the left*/
		return (false);
	
	opunlink (hcursor);
	
	opdepositdown (hpre, hcursor);
	
	(**hpre).fldirty = true;
	
	opresetlevels (hcursor);
	
	(**hcursor).fldirty = true;
	
	return (true);
	} /*opmoveoutlineleft*/
	
	
static boolean opmoveoutlineright (void) {
	
	register hdloutlinerecord ho = outlinedata;
	register hdlheadrecord hcursor = (**ho).hbarcursor;
	register hdlheadrecord hpre = (**hcursor).headlinkup;
	
	if (hpre == hcursor) /*he's the first guy at his level, can't move right*/
		return (false);
		
	opexpand (hpre, 1, false); /*make any subheads visible*/
	
	opunlink (hcursor);
	
	if (opnosubheads (hpre))
		opdepositright (hpre, hcursor);
	else
		opdepositdown (opgetlastsubhead (hpre), hcursor);
		
	opresetlevels (hcursor);
	
	return (true);
	} /*opmoveoutlineright*/


static boolean opbeforeundo (hdlheadrecord hnode, boolean flundo) {
	
	if (flundo) {
		
		opclearallmarks ();
		
		opexpandto (hnode);
		
		opmoveto (hnode);
		
		opbeforestrucchange (&hundomap, false);

		opdisabledisplay (); //5.0b18
		}
	
	return (true);
	} /*opbeforeundo*/


static boolean opafterundo (hdlheadrecord hnode, boolean flundo) {

	if (flundo) {
		
		opexpandto (hnode);
		
		opmoveto (hnode);
		
		if (opanymarked ())
			opsettextmode (false);
		
		openabledisplay ();
		
		opafterstrucchange (hundomap, false);
		}
	
	return (true);
	} /*opafterundo*/


boolean opbeforestrucchange (hdlscreenmap *hmap, boolean flsaveeditbuffer) {
	
	/*
	this is what you do before restructuring the outline.
	*/
	
	opnewscreenmap (hmap); /*nil if it failed*/
	
	if (flsaveeditbuffer)
		opsaveeditbuffer ();
	else
		opunloadeditbuffer ();
	
	oppushundo (&opafterundo, (**outlinedata).hbarcursor);
	
	return (true);
	} /*opbeforestrucchange*/


boolean opafterstrucchange (hdlscreenmap hmap, boolean flvisisubs) {
	
	/*
	this is what you do after restructuring the outline.
	
	9.1b3 AR: Check currentprocess before even considering whether
	to run the opCursorMoved callbacks. This fixes a crashing bug that
	would occur if we tried to run the callbacks while removing the
	current thread's entry from the system.compiler.threads table
	since at that point all the infrastructure for running scripts
	has already been disposed.
	*/

	register hdloutlinerecord ho = outlinedata;
	
	opsetctexpanded (ho); /*don't bother maintaining this, we re-compute*/
	
	opsetscrollpositiontoline1 ();
	
	opresetscrollbars (); /*show changes in scroll bars, update data structures*/
	
	opinvalscreenmap (hmap); /*only inval those lines that changed*/
	
	opupdatenow (); /*provide immediate feedback*/
	
	if ((**ho).textinfo.flvalid)
		oprestoreeditbuffer ();
	else
		oploadeditbuffer ();
	
	if (flvisisubs)
		opvisisubheads ((**ho).hbarcursor);
	else
		opvisinode ((**ho).hbarcursor, true);
	
	opdirtyoutline ();
	
	if (currentprocess != nil				/* 9.1b3 AR */
			&& !debuggingcurrentprocess ()	/*7.0b8: don't run callbacks if debugging*/
			&& opdisplayenabled ()) {
			 
		langopruncallbackscripts (idopcursormovedscript); /*7.0b6 PBS: call callback when cursor moves*/
		}
		
	return (oppushundo (&opbeforeundo, (**ho).hbarcursor));
	} /*opafterstrucchange*/


boolean opsortlevel (hdlheadrecord hnode) {
	
	/*
	sort the nodes at the same level as the indicated node.
	
	9/1/92 dmb: roll the beachball cursor
	*/
	
	hdlheadrecord houter, hinner;
	hdlheadrecord h;
	bigstring bslowest;
	hdlheadrecord hlowest;
	bigstring bs;
	long ct, i;
	hdlscreenmap hmap;
	
	if (opanymarked ())
		return (false);
	
	pushundoaction (undosortstring);
	
	opbeforestrucchange (&hmap, false);
	
	houter = oprepeatedbump (up, longinfinity, hnode, true);
	
	ct = opcountatlevel (houter);
	
	initbeachball (right);
	
	for (i = 0; i < ct; i++) {
		
		hlowest = houter;
		
		opgetsortstring (hlowest, bslowest);
		
		hinner = (**houter).headlinkdown;
		
		if (hinner == houter) /*the list is sorted*/
			break;
		
		while (true) {
			
			rollbeachball ();
			
			opnodechanged (hinner); /*make sure all lines get refreshed*/
			
			(**hinner).fldirty = true;
			
			opgetsortstring (hinner, bs);
			
			if (stringlessthan (bs, bslowest)) {
			
				hlowest = hinner;
				
				copystring (bs, bslowest);
				}
				
			h = (**hinner).headlinkdown;
			
			if (h == hinner) /*finished scanning for lowest*/
				break;
				
			hinner = h; /*advance to next node*/
			} /*while*/
		
		if (hlowest != houter) {
		
			opunlink (hlowest);

			opdepositup (houter, hlowest);

			houter = hlowest;
			}

		houter = (**houter).headlinkdown;
		} /*while*/
	
	opafterstrucchange (hmap, false);
	
	return (true);
	} /*opsortlevel*/


/*
static boolean opdefaultcanmove (hdlheadrecord hnode, tydirection dir) {
	
	register hdlheadrecord h = hnode;
	boolean fl;
	
	switch (dir) {
		
		case up:
		case right:
			fl = (**h).headlinkup != h;
			
			break;
		
		case down:
			fl = (**h).headlinkdown != h;
			
			break;
			
		case left:
			fl = (**h).headlinkleft != h;
			
			break;
		
		default:
			fl = false;
		}
	
	return (fl);
	} /%opdefaultcanmove%/
*/


typedef struct tymoveinfo {
	
	hdlheadrecord hpre;
	
	tydirection dir;
	} tymoveinfo;


static boolean opvalidatecanmove (hdlheadrecord hnode, ptrvoid refcon) {
	
	/*
	5.0a18 dmb: added moveinfo parameter, so we can test move against
	correct parent when there's a multiple selection
	*/
	
	hdloutlinerecord ho = outlinedata;
	tymoveinfo *moveinfo = (tymoveinfo *) refcon;
	hdlheadrecord hpre = (*moveinfo).hpre;
	tydirection dir = (*moveinfo).dir;
	
	if (hpre == nil) { // we're first, we need to set hpre, dir
	
		switch (dir) {
			
			case up:
			case right:
				hpre = (**hnode).headlinkup;
				
				break;
			
			case down:
				hpre = (**hnode).headlinkdown;
				
				break;
				
			case left:
				hpre = (**hnode).headlinkleft;
				
				dir = down;
				
				break;
			
			default:
				return (false);
			}
		
		(*moveinfo).hpre = hpre;
		
		(*moveinfo).dir = dir;
		}
	
	return ((*(**ho).validatedragcallback) (hnode, hpre, dir));
	} /*opvalidatecanmove*/


static boolean opmoveoutlinevisit (hdlheadrecord hnode, ptrvoid dir) {
	
	boolean fl;
	
	(**outlinedata).hbarcursor = hnode; /*will be restored by caller later*/
	
	oppushunmarkundo (hnode);
	
	switch ((tydirection) dir) {
		
		case up:
			fl = opmoveoutlineup ();
			
			break;
		
		case down:
			fl = opmoveoutlinedown ();
			
			break;
			
		case left:
			fl = opmoveoutlineleft ();
			
			break;
			
		case right:					
			fl = opmoveoutlineright ();
			
			break;
		
		default:
			fl = false;
		}
	
	return (fl);
	} /*opmoveoutlinevisit*/

		
boolean opreorgcursor (tydirection dir, long units) {
	
	/*
	reorganize the structure so that the cursor line is moved in the 
	indicated direction, the indicated number of times.
	
	return false if the move couldn't be done.
	
	supported directions are up, down, left and right.
	
	if it's a one level structure, return false if direction is left
	or right.
	
	1/24/91 dmb: must restore hcursor every time through loop
	*/
	
	register hdloutlinerecord ho = outlinedata;
	register long i;
	register boolean fl;
	register boolean flmovedsomething = false;
	hdlscreenmap hmap;
	hdlheadrecord horigcursor = (**ho).hbarcursor;
	tydirection visitdir;
	tymoveinfo moveinfo;
	
	pushundoaction (undomovestring);
	
	opbeforestrucchange (&hmap, true);
	
	if ((dir == up) || (dir == right))
		visitdir = down;
	else
		visitdir = up;
	
	for (i = 1; i <= units; i++) {
		
		moveinfo.hpre = nil;
		moveinfo.dir = dir;
		
		fl = opvisitmarked (visitdir, &opvalidatecanmove, &moveinfo);
		
		if (fl)
			fl = opvisitmarked (visitdir, &opmoveoutlinevisit, (ptrvoid) dir);
		
		if (!fl)
			break;
		
		(**ho).hbarcursor = horigcursor; /*unlink changed it*/
		
		flmovedsomething = true;
		} /*for*/
	
	if (!flmovedsomething) {
		
		popundoaction ();
		
		oprestoreeditbuffer ();
		
		disposehandle ((Handle) hmap);
		
		return (false);
		}
	
	opafterstrucchange (hmap, true);
	
	return (true);
	} /*opreorgcursor*/
	

static boolean opvalidatemovevisit (hdlheadrecord hnode, tymoveinfo *moveinfo) {
	
	/*
	5.0b15 dmb: this is confusing. the validatedrag callback makes sure that 
	the destination can accept a drag. We need only test a single source 
	headline for this test.

	the validatepastecallback looks for conflicts between the source nodes 
	and the dest. all source nodes must be tested, which it does automatcially
	*/

	if (!(*(**outlinedata).validatedragcallback) (hnode, (*moveinfo).hpre, (*moveinfo).dir))
		return (false);

	return ((*(**outlinedata).validatepastecallback) (hnode, (*moveinfo).hpre, (*moveinfo).dir));
	} /*opvalidatemovevisit*/


boolean oppromote (void) {
	
	/*
	move all the subheads of the bar cursor node out one level.
	
	5.0a25 dmb: use ophassubheads, not opnosubheads, to count dynamics

	5.0b13 dmb: added validation
	*/
	
	register hdloutlinerecord ho = outlinedata;
	register hdlheadrecord hcursor = (**ho).hbarcursor;
	register hdlheadrecord nomad, nextnomad;
	hdlscreenmap hmap;
	boolean fl = false;
	
	if (!ophassubheads (hcursor)) /*nothing to promote*/
		return (false);
	
	if (opanymarked ())
		return (false);
	
	pushundoaction (undopromotestring);

	opbeforestrucchange (&hmap, true);
	
	opnodechanged (hcursor); 
	
	(**hcursor).fldirty = true; /*leader icon will change*/
	
	opexpand (hcursor, 1, false); /*make sure subheads are visible*/
	
	/*validate the move first*/ {
		tymoveinfo moveinfo;
		
		moveinfo.hpre = hcursor;
		moveinfo.dir = down;
		
		if (!opvalidatemovevisit ((**hcursor).headlinkright, &moveinfo)) //this visits list
			goto exit;
		}

	nomad = opgetlastsubhead (hcursor); /*start at the end of the list*/
	
	while (true) {
		
		nextnomad = (**nomad).headlinkup;
		
		(**ho).hbarcursor = nomad; /*play a li'l trick*/
		
		opmoveoutlineleft ();
		
		(**ho).hbarcursor = hcursor; /*restore*/
		
		if (nextnomad == nomad) { /*we're outa here*/
			
			fl = true;

			break;
			}
			
		nomad = nextnomad;
		} /*while*/
	
	exit: {

		opafterstrucchange (hmap, true); /*update the display, set dirty bits*/

		return (fl);
		}
	} /*oppromote*/
	
	
boolean opdemote (void) {
	
	/*
	borrow Doug Baron's idea of demoting -- thanks Doug!  we move all the heads
	at the bar cursor's level, that are down from the bc, at the end of the
	bar cursor's list of subs.

	5.0b13 dmb: added validation
	*/
	
	register hdloutlinerecord ho = outlinedata;
	register hdlheadrecord hcursor = (**ho).hbarcursor;
	register hdlheadrecord nomad;
	register boolean flmovedsomething = false;
	hdlscreenmap hmap;
	
	if (opanymarked ())
		return (false);
	
	if (oplastinlist (hcursor)) //nothing to do
		return (false);

	pushundoaction (undodemotestring);
	
	opbeforestrucchange (&hmap, true);
	
	opexpand (hcursor, 1, false); /*make sure subheads are visible*/
	
	/*validate the move first*/ {
		tymoveinfo moveinfo;

		moveinfo.hpre = hcursor;
		moveinfo.dir = right;

		if (!opvalidatemovevisit ((**hcursor).headlinkdown, &moveinfo)) //this visits list
			goto exit;
		}

	while (true) {
		
		nomad = (**hcursor).headlinkdown;
		
		if (nomad == hcursor) /*no more items down from the cursor*/
			break;
		
		(**ho).hbarcursor = nomad; /*play a li'l trick*/
		
		opmoveoutlineright ();
		
		(**ho).hbarcursor = hcursor; /*restore*/
		
		opnodechanged (hcursor); /*leader icon may change*/
		
		(**hcursor).fldirty = true;
		
		flmovedsomething = true;
		} /*while*/

	exit: {
		
		opafterstrucchange (hmap, true);
			
		return (flmovedsomething);
		}
	} /*opdemote*/


static boolean opdeletesubvisit (hdlheadrecord hnode, ptrvoid refcon) {
	
	/*
	5.0.1 dmb: new routline betweeen opdeletesubs and opreleasenode,
	to avoid breaking the display

	5.0.2 dmb: perfect place to fix the next bug: maintain ctexpanded
	in case we're tossing expanded nodes
	*/

	hdlheadrecord hsafe = (hdlheadrecord) refcon;
	hdloutlinerecord ho = outlinedata;

	if ((**ho).hbarcursor == hnode)
		(**ho).hbarcursor = hsafe;
	
	if ((**ho).hline1 == hnode)
		opsetline1 (hsafe);
	
	if ((**hnode).flexpanded)
		(**ho).ctexpanded -= opgetnodelinecount (hnode);
	
	return (opreleasevisit (hnode, (ptrvoid) true));
	} /*opdeletesubvisit*/


boolean opdeletesubs (hdlheadrecord hnode) {
	
	/*
	5.0a25 dmb: use ophassubheads, not opnosubheads, to count dynamics
	
	7.0b23: Tables are dynamic. If a node is collapsed,
	headlinkright doesn't point to the first child, it points to itself.
	So call the preexpand callback to make sure headlinkright is correct.
	If it's not a table, it's a harmless call.
	*/
	
	register hdlheadrecord h = hnode;
	hdlscreenmap hmap;
	
	if (opanymarked ())
		return (false);
	
	if (!ophassubheads (h)) /*nothing to delete*/
		return (true);
	
	pushundoaction (undodeletionstring);

	opbeforestrucchange (&hmap, true);
	
	opnodechanged (h); /*leader icon will change*/
	
	if (!(*(**outlinedata).preexpandcallback) (hnode, 1, true)) /*PBS 7.0b23: fixes crashing bug in tables, harmless elsewhere.*/
		return (false);
	
	(**h).fldirty = true;
	
	opsiblingvisiter ((**h).headlinkright, true, &opdeletesubvisit, (ptrvoid) h);
	
	(**h).headlinkright = h; /*children list is now empty*/
	
	opafterstrucchange (hmap, false);
	
	return (true); 
	} /*opdeletesubs*/
	

static boolean opcopyvisit (hdlheadrecord hnode, ptrvoid refcon) {
	
	ptrcopyinfo copyinfo = (ptrcopyinfo) refcon;
	register hdlheadrecord hcopy;
	register hdlheadrecord htarget;
	register short level, lastlevel;
	register tydirection dir;
	
	hcopy = opcopyheadrecord (hnode);
	
	if (hcopy == nil) /*kill the recursion*/
		return (false);
		
	level = (**hnode).headlevel;
	
	lastlevel = (*copyinfo).level;
	
	htarget = (*copyinfo).hnode;
	
	if (level > lastlevel) 
		dir = right;
		
	else {
		htarget = oprepeatedbump (left, lastlevel - level, htarget, true);
		
		dir = down;
		}
	
	opstartinternalchange ();

	opdeposit (htarget, dir, hcopy);

	opendinternalchange ();

	(*copyinfo).level = level;
	
	(*copyinfo).hnode = hcopy;
	
	return (true);
	} /*opcopyvisit*/
	
	
hdlheadrecord opcopyoutline (hdlheadrecord horig) {
	
	register hdlheadrecord hcopy;
	tycopyinfo copyinfo;
	
	hcopy = opcopyheadrecord (horig);
	
	if (hcopy == nil)
		return (nil);

	copyinfo.hnode = hcopy;
	
	copyinfo.level = (**hcopy).headlevel;
	
	if (oprecursivelyvisit (horig, infinity, &opcopyvisit, &copyinfo))	
		return (hcopy);
	
	opdisposestructure (hcopy, true); /*release what we've allocated so far*/
	
	return (nil); /*indicate failure*/
	} /*opcopyoutline*/
	
	
boolean opcopysiblings (hdlheadrecord horig, hdlheadrecord *hcopy) {
	
	/*
	copy horig and all his siblings and return the copy in hcopy.
	
	5.0.2b15 dmb: make sure each new copy has expanded bit set, and no
	expanded subheads. also, must set headlevel of first copy to zero like 
	we do in opcopymarkedvisit; we used to assume that horig was a true summit
	
	5.0.2b16 dmb: leave the subhead expansion alone, and only set expanded bit
	if levels are changing.
	*/
	
	register hdlheadrecord hlastcopy = nil;
	register hdlheadrecord h;
	boolean fllevelschanging = (**horig).headlevel != 0;
	
	*hcopy = nil; /*default value*/
	
	while (true) { /*copy horig and each of its siblings*/
	
		h = opcopyoutline (horig);
		
		if (h == nil) { /*memory allocation failure*/
			
			opdisposestructure (*hcopy, false);
			
			*hcopy = nil; /*further indication of error*/
			
			return (false);
			}
		
		if (fllevelschanging)
			(**h).flexpanded = true;
		
		if (*hcopy == nil) /*set returned handle*/
			*hcopy = h;
		
		if (hlastcopy == nil) // first one copied
			(**h).headlevel = 0;
		
		else {
		
			opstartinternalchange ();

			opdepositdown (hlastcopy, h);

			opendinternalchange ();
			}
		
		if (fllevelschanging)
			opresetlevels (h);
		
		hlastcopy = h; /*remember for next iteration*/
		
		if (!opnavigate (down, &horig)) /*finished copying the siblings*/
			return (true);
		} /*while*/
	} /*opcopysiblings*/
	
	
boolean opcopyoutlinerecord (hdloutlinerecord horig, hdloutlinerecord *hcopy) {
	
	/*
	12/1/92 dmb: clear tophoist & hbuffer in copy. (avoids menueditor crash)
	
	5.0.2b12 dmb: set outlinedata to horig during copy, to get its callbacks.
	
	5.1.5b9 dmb: preserve barcursor, hline1
	*/
	
	register hdloutlinerecord ho;
	hdlheadrecord hnewsummit;
	Handle hnew;
	long lnumline1, lnumcursor;
	//hdloutlinerecord hsave = outlinedata;
	boolean fl;
	
	if (!copyhandle ((Handle) horig, &hnew))
		return (false);
	
	ho = *hcopy = (hdloutlinerecord) hnew; /*copy into register*/
	
	oppushoutline (horig); //opsetoutline (horig);
	
	fl = opcopysiblings ((**horig).hsummit, &hnewsummit);
	
	opgetnodeline ((**horig).hbarcursor, &lnumcursor);
	
	opgetnodeline ((**horig).hline1, &lnumline1);
	
	oppopoutline (); //opsetoutline (hsave);
	
	if (!fl) {
		
		disposehandle ((Handle) ho);
		
		return (false);
		}
	
	(**ho).hsummit = nil; /*we don't own it*/
	
	opsetsummit (ho, hnewsummit);
	
	(**ho).tophoist = 0; /*if there were hoists, they're gone! should be fixed some day*/
	
	(**ho).hbuffer = nil; /*if horig was in edit mode, this doesn't belong to us*/
	
	(**ho).hbarcursor = oprepeatedbump (flatdown, lnumcursor, hnewsummit, true);
	
	(**ho).hline1 = oprepeatedbump (flatdown, lnumline1, hnewsummit, true);
	
	return (true);
	} /*opcopyoutlinerecord*/


static boolean opclearmarkvisit (hdlheadrecord hnode, ptrvoid refcon) {
#pragma unused (refcon)

	register hdlheadrecord h = hnode;
	
	if ((**h).flmarked) {
		
		// 5.0a25 dmb: opscreenmap handles marked changes now
		// (**h).fldirty = true; /*only dirty if it was previously marked*/
		
		(**h).flmarked = false;
		
		--(**outlinedata).ctmarked;
		}
	
	return (true);
	} /*opclearmarkvisit*/


void opclearallmarks (void) {
	
	hdloutlinerecord ho = outlinedata;
	hdlscreenmap hmap;
	
	if (!opanymarked ()) { /*must check barcursor*/
		
		opsetmark ((**ho).hbarcursor, false);
		
		return;
		}
	
	if (!opgetmark ((**ho).hbarcursor)) /*odd case -- cursor wasn't in selection*/
		(**ho).flcursorneedsdisplay = true;
	
	opnewscreenmap (&hmap); /*9/11/91 dmb*/
	
	opsiblingvisiter ((**ho).hsummit, false, &opclearmarkvisit, nil);
	
	opinvalscreenmap (hmap); /*inval all the dirty lines*/
	
	//assert ((**ho).ctmarked == 0); /*really should already be zero*/
	
	(**ho).ctmarked = 0; /*make sure*/
	} /*opclearallmarks*/


void opclearmarks (hdlheadrecord hnode) {
	
	oprecursivelyvisit (hnode, infinity, &opclearmarkvisit, nil);
	} /*opclearmarks*/
	
	
boolean opgetmark (hdlheadrecord hnode) {
	
	return ((**hnode).flmarked);
	} /*opgetmark*/
	
	
void opsetmark (hdlheadrecord hnode, boolean fl) {
	
	register hdlheadrecord h = hnode;
	register hdlheadrecord hleft;
	register boolean flmarked = (**h).flmarked;
	
	if (fl == flmarked) /*no change*/
		return;
	
	(**h).flmarked = fl;
	
	// 5.0a25 dmb: opscreenmap handles marked changes now
	// (**h).fldirty = true;
	
	if (!fl)  { /*turning off the marked bit is much easier*/
		
		(**outlinedata).ctmarked--;
		
		return;
		}
	
	(**outlinedata).ctmarked++;
	
	opclearmarks (h); /*make sure nothing subordinated is marked*/
	
	while (true) { /*clear marks on all ancestors of marked node*/
		
		hleft = (**h).headlinkleft;
		
		if (hleft == h) /*we're at the summit*/
			return;
			
		h = hleft; 
		
		opclearmarkvisit (h, nil);
		} /*while*/
	} /*opsetmark*/


static boolean opsetbarcursorvisit (hdlheadrecord hnode, ptrvoid refcon) {
#pragma unused (refcon)

	(**outlinedata).hbarcursor = hnode; /*move barcursor here*/
	
	return (false); /*this is the only marked node*/
	} /*opsetbarcursorvisit*/


#if 0

static boolean debugmarkedvisit (hdlheadrecord hnode, long *intforrecursion) {
	
	if ((**hnode).flmarked)
		++*intforrecursion;
	
	return (true);
	} /*debugmarkedvisit*/

#endif

boolean opanymarked (void) {
	
	/*
	normalize the selection, and return true if anything is marked, false otherwise.
	*/
	
	register hdloutlinerecord ho = outlinedata;
	
	switch ((**ho).ctmarked) {
		
		case 0:
			return (false);
		
		case 1:
			if (!opgetmark ((**ho).hbarcursor))
				opvisitmarked (down, &opsetbarcursorvisit, nil); /*move cursor to marked headline*/
			
			return (false);
		
		default:
			return (true);
		}
	} /*opanymarked*/


static hdlheadrecord hfirstscrap, hlastscrap; /*for copying marked structures*/
	

static boolean opcopymarkedvisit (hdlheadrecord hnode, ptrvoid refcon) {
#pragma unused (refcon)

	/*
	10/3/92 dmb: set levels so that all copies are summits
	*/
	
	register hdlheadrecord hcopy;
	
	hcopy = opcopyoutline (hnode);
	
	if (hcopy == nil) /*allocation error, stop the traversal*/
		return (false);
		
	if (hfirstscrap == nil) {
	
		hfirstscrap = hcopy;
		
		hlastscrap = hcopy;
		
		(**hcopy).headlevel = 0;
		}
	else {
		
		opstartinternalchange ();
		
		opdepositdown (hlastscrap, hcopy);
		
		opendinternalchange ();
		
		hlastscrap = hcopy;
		}
	
	opresetlevels (hcopy);
	
	return (true);
	} /*opcopymarkedvisit*/
	

static boolean
optrytextcommand (
		boolean (*editroutine) (void),
		boolean flneedselection,
		boolean flchanging)
{
#pragma unused (flchanging)

	register hdloutlinerecord ho = outlinedata;
	long startsel, endsel;

	if (!(**ho).fltextmode)
		return (false);
	
	if (flneedselection) {
		
		opeditgetselection (&startsel, &endsel);
		
		if (startsel == endsel)
			return (false);
		}
	
	(*editroutine) ();
	
	return (true);
	} /*optrytextcommand*/


boolean opcopy (void) {

	/*
	5.1.3 dmb: better error handling
	*/
	
	register hdloutlinerecord ho = outlinedata;
	
	if (optrytextcommand (opeditcopy, true, false))
		return (true);
	
	if (!((**ho).validatecopycallback) (STR_copy))
		return (false);
	
	opwriteeditbuffer (); /*if a headline is being edited, update text handle*/
	
	if (!opanymarked ()) { /*just copy the bar cursor structure*/
	
		return (opsetscrap (opcopyoutline ((**ho).hbarcursor)));
		}
	
	hfirstscrap = hlastscrap = nil; /*nothing has been copied yet*/
	
	if (!opvisitmarked (nodirection, &opcopymarkedvisit, nil)) {
		
		opdisposestructure (hfirstscrap, false);
		
		return (false);
		}
	
	return (opsetscrap (hfirstscrap));
	} /*opcopy*/


static void opdisposenode (hdlheadrecord hnode) {

	opdisposestructure (hnode, true);
	} /*opdisposenode*/


static boolean opundodelete (hdlheadrecord hnode, boolean flundo); /*forward*/

static boolean opredodelete (hdlheadrecord hnode, boolean flundo) {

	if (flundo)
		oppushundo (&opundodelete, hnode);

	return (true);
	} /*opredodelete*/


static boolean opundodelete (hdlheadrecord hnode, boolean flundo) {
	
	if (flundo) {
		
		(**hnode).flmarked = true; /*just make sure it's marked; it's notin the outline now*/
		
		oppushundo (&opredodelete, hnode);
		}
	else
		opdisposenode (hnode);
	
	return (true);
	} /*opundodelete*/


boolean opdepositnewheadline (hdlheadrecord hpre, tydirection dir, Handle hstring, hdlheadrecord *hnew) {

	/*
	6.0a1 dmb: handle-based version of opaddheadline, which now calls us
	*/
	
	if (!opnewheadrecord (hstring, hnew))
		return (false);
	
	#if !fljustpacking
	
		oppushundo (&opredodelete, *hnew);
		
	#endif
	
	return (opdeposit (hpre, dir, *hnew));
	} /*opdepositnewheadline*/


boolean opaddheadline (hdlheadrecord hpre, tydirection dir, bigstring bshead, hdlheadrecord *hnew) {

	Handle hstring;
	
	if (!newtexthandle (bshead, &hstring))
		return (false);
	
	return (opdepositnewheadline (hpre, dir, hstring, hnew));
	} /*opaddheadline*/


static boolean opundonewsummit (hdlheadrecord hnode, boolean flundo) {
	
	if (flundo) {
		
		register hdloutlinerecord ho = outlinedata;
		hdlheadrecord hdelete = (**ho).hsummit;
		
		/* restore original summit */
		
		(**ho).hsummit = hnode;
		
		(**ho).hbarcursor = hnode;
		
		(**ho).hline1 = hnode;
		
		assert ((**ho).ctmarked == 0);
		
		if ((**hnode).flmarked)
			(**ho).ctmarked = 1;
		
		/* prepare reciprocal undo for what was the "new" summit */
		
		oppushundo (&opundonewsummit, hdelete);
		}
	
	return (true);
	} /*opundonewsummit*/


boolean opdeletenode (hdlheadrecord hnode) {
	
	/*
	5.0a25 dmb: don't assume that hnode itself is expanded

	5.0b7 dmb: create new summit before calling deletelinecallback
	*/

	register hdloutlinerecord ho = outlinedata;
	register hdlheadrecord hdelete = hnode;
	register hdlheadrecord hsummit;
	
	hsummit = (**ho).hsummit;
	
	if (hdelete == hsummit) { /*deleting the first summit*/
		
		register hdlheadrecord hdown = (**hsummit).headlinkdown;
		
		if (hdown == hsummit) /*deleting the only summit*/
			hdown = nil;
		
		(**ho).hsummit = hdown;
		
		opsetline1 (hdown);
		
		(**ho).hbarcursor = hdown;
		}
		
	else { /*deleting something other than the first summit*/
		
		if (!opsafebarcursor (hdelete)) { /*couldn't move up, down, or left*/
			
			popundoaction ();
			
			return (false);
			}
		}
	
	/*the cursor has been moved out of harm's way, perform the delete*/
	
	if ((**ho).hsummit == nil) { /*we deleted the only summit, make a new blank one*/
		
		opnewsummit (); /*make a new, blank summit, xxx -- add pre-flighting*/
		
		opdeletelinecallback (hdelete);
		
		opinsertlinecallback ((**ho).hsummit);
		
		oppushundo (&opredodelete, (**ho).hsummit);
		
		oppushundo (&opundonewsummit, hdelete);
		
		(**ho).ctexpanded = 1;
		
		(**ho).ctmarked = 0;
		}
	else {
		opcheckline1 (hdelete);
		
		opunlink (hdelete);
		
		if (opsubheadsexpanded (hdelete)) /*got rid of the deleted line(s)*/
			opsetctexpanded (ho);
		else if ((**hdelete).flexpanded)
			(**ho).ctexpanded -= opgetnodelinecount (hdelete);
		
		opsetscrollpositiontoline1 ();
		
		if ((**hdelete).flmarked)
			(**ho).ctmarked --;
		
		opnodechanged ((**ho).hbarcursor); 
		
		(**(**ho).hbarcursor).fldirty = true; /*be sure cursor gets displayed*/
		}
	
	oppushundo (&opundodelete, hdelete);
	
	return (true);
	} /*opdeletenode*/


static boolean opdeletenodevisit (hdlheadrecord hnode, ptrvoid refcon) {
#pragma unused (refcon)

	return (opdeletenode (hnode));
	} /*opdeletenodevisit*/


boolean opdelete (void) {
	
	/*
	delete the barcursor outline
	
	9/11/91 dmb: handle scrollbars when barcursor has expanded subheads
	
	2/12/92 dmb: must call delete/insert callbacks when replacing the 
	summit.  (failure to do so used to cause menubar crash.)
	*/
	
	hdlscreenmap hmap;
	
	opbeforestrucchange (&hmap, false);
	
	if (!opvisitmarked (down, &opdeletenodevisit, nil)) { /*an error occurred; try to unwind*/
		
		disposehandle ((Handle) hmap); /*checks for nil*/
		
		popundoaction ();

		return (false);
		}
	
	opafterstrucchange (hmap, false);
	
	return (true);
	} /*opdelete*/


boolean opclear (void) {
		
	if (optrytextcommand (opeditclear, true, true))
		return (true);
	
	pushundoaction (undoclearstring);
	
	return (opdelete ());
	} /*opclear*/
	
	
void opdeleteline (void) {
	
	/*
	support for the deleteline verb.
	*/
		
	pushundoaction (undoclearstring);
	
	opdelete ();
	} /*opdeleteline*/


boolean opcut (void) {

	if (optrytextcommand (opeditcut, true, true))
		return (true);
	
	pushundoaction (undocutstring);
	
	if (!opcopy ())
		return (false);
	
	return (opdelete ());
	} /*opcut*/


#if 0

static boolean opundopaste (hdlheadrecord hnode, boolean flundo) {

	if (flundo) {
	
		opmoveto (hnode);

		opdelete ();
		}

	return (true);
	} /*opundopaste*/

#endif


boolean isoutlinetext (Handle htext) {
	
	/*
	return true if the text represents a multiple headline 
	outline, false if it is just flat text
	
	2.1b9 dmb: broke out this snippet so it can be shared

	6.0a1 dmb: fat headlines can be > lenbigstring
	*/
	
	bigstring bsreturn;

	setstringwithchar (chreturn, bsreturn);

	return (textpatternmatch ((byte *) (*htext), gethandlesize (htext), bsreturn, false) >= 0);
	
	/*
	bigstring bs;
	
	if (gethandlesize (htext) > lenbigstring) //too big for a single head
		return (true);
	
	texthandletostring (htext, bs);
		
	return (stringfindchar (chreturn, bs));
	*/
	} /*isoutlinetext*/


boolean isoutlinescrap (void) {
	
	/*
	return true if the scrap should be pasted as an outline even when 
	in text mode
	
	12/31/96 dmb: removed check for scrap being > 0x400 bytes. simple 
	wptext scraps can be that large.
	*/
	
	boolean fltempscrap;
	tyscraptype scraptype;
	Handle hscrap;
	boolean floutline;
	
	if (!shellgetscrap (&hscrap, &scraptype)) /*try to get the scrap, ignoring type*/
		return (false);
	
	switch (scraptype) {
		
		case opscraptype: /*specifically an OP scrap*/
		case scriptscraptype:
		case menuscraptype:
		case hashscraptype:
			return (true);
		
		default:
		//	if (gethandlesize (hscrap) > 0x0400) /*too big for a headline*/
		//		return (true);
			
			if (!shellconvertscrap (textscraptype, &hscrap, &fltempscrap)) /*try converting to text*/
				return (false);
			
			floutline = isoutlinetext (hscrap);
			
			if (fltempscrap)
				disposehandle (hscrap);
			
			return (floutline);
		}
	} /*isoutlinescrap*/


static boolean equalheadstrings (hdlheadrecord h1, hdlheadrecord h2) {
	
	bigstring s1, s2;
	
	opgetheadstring (h1, s1);
	
	opgetheadstring (h2, s2);
	
	return (equalstrings (s1, s2));
	} /*equalheadstrings*/


boolean oppaste (void) {
	
	/*
	1/30/91 dmb: fixed heap bug; was passing &hmap to opafterstrucchange
	
	5.0a5 dmb: when pasting a temp scrap, must unlink each node that we don't copy
	*/
	
	register hdloutlinerecord ho = outlinedata;
	register hdlheadrecord hcursor = (**ho).hbarcursor;
	register hdlheadrecord hcopy, hfirstcopy, hlastcopy;
	hdlheadrecord nomad, nextnomad;
	hdlheadrecord houtlinescrap;
	hdlscreenmap hmap;
	boolean fltempscrap;
	boolean flmarkcopies = false;
	
	opclearallmarks ();
	
	if (isoutlinescrap ()) /*pasting outlines exits text mode*/
		opsettextmode (false);
	
	if (optrytextcommand (opeditpaste, false, true)) /*if text mode, handle & exit*/
		return (true);
	
	pushundoaction (undopastestring);
	
	if (!opgetscrap (&houtlinescrap, &fltempscrap))
		return (false);
		
	if (!((**ho).validatepastecallback) (houtlinescrap, hcursor, down))
		return (false);
	
	hcursor = (**outlinedata).hbarcursor; /*the validate routine could delete the bar cursor*/
	
	opbeforestrucchange (&hmap, false);
	
	opnodechanged (hcursor);
	
	(**hcursor).fldirty = true;
	
	nomad = houtlinescrap;
	
	hlastcopy = hcursor; /*where we deposit the next pasted structure*/
	
	hfirstcopy = nil; /*indicate nothing has been pasted yet*/
	
	while (true) { /*copy each of the summits on the scrap*/
	
		nextnomad = (**nomad).headlinkdown;
		
		if (fltempscrap) {
			
			hcopy = nomad; /*rather than disposing temp afterwards, use it directly*/
			
			if (nextnomad != nomad) { // must disconnect since we're not making a copy
				
				(**nomad).headlinkdown = nomad;
				
				(**nextnomad).headlinkup = nextnomad;
				}
			}
		else
			hcopy = opcopyoutline (nomad);
		
		oppushundo (&opredodelete, hcopy);
		
		if (hfirstcopy == nil) { /*first time*/
			
			hfirstcopy = hcopy;
			
			flmarkcopies = (nextnomad != nomad) && (nextnomad != nil); /*more to come*/
			}
		
		if (hcopy == nil) { /*memory error*/
			
			if (hfirstcopy == nil) { /*nothing to paste*/
				
				disposehandle ((Handle) hmap);
				
				popundoaction ();
				
				return (true);
				}
				
			goto L1; /*paste what we have so far*/
			}
			
		opdepositdown (hlastcopy, hcopy);
		
		hlastcopy = hcopy;
		
		opresetlevels (hcopy);
		
		if (flmarkcopies)
			opsetmark (hcopy, true);
		
		else {
			if (!equalheadstrings (hcopy, nomad)) {
				
				(**ho).flcursorneedsdisplay = true; /*make sure opmoveto does something*/
				
				(**ho).fltextmode = true;
				
				(**hcopy).flmarked = false; /*clear flag*/
				
				opeditsetselection (0, infinity);
				}
			}
		
		opfastcollapse (hcopy); /*turn off expanded bits of submaterial*/
		
		(**hcopy).flexpanded = true;
		
		if ((nomad == nextnomad) || (nextnomad == nil)) /*copied the last guy on scrap*/
			goto L1;
			
		nomad = nextnomad; /*advance to next guy on scrap*/
		} /*while*/
	
	L1:
	
	opsetctexpanded (ho); /*don't bother maintaining this, we re-compute*/
	
	(**ho).hbarcursor = hlastcopy;
	
	(*(**ho).postpastecallback) (hfirstcopy);
	
	opafterstrucchange (hmap, false);
	
	if (!opnodevisible (hfirstcopy)) {
		
		opvisinode (hfirstcopy, false);
		
		opvisinode (hlastcopy, false);
		}
	
	return (true);
	} /*oppaste*/


boolean opinsertheadline (Handle hstring, tydirection dir, boolean flcomment) {
	
	/*
	8/11/92 dmb: make sure display is enabled before drawing
	*/
	
	register hdloutlinerecord ho = outlinedata;
	register hdlheadrecord hcursor = (**ho).hbarcursor;
	hdlheadrecord hnewcursor;
	
	opunloadeditbuffer ();
	
	pushundoaction (undotypingstring);
	
	oppushundo (&opafterundo, hcursor);
	
	opdocursor (false); /*un-highlight the old bar cursor line*/
	
	if (!opdepositnewheadline (hcursor, dir, hstring, &hnewcursor)) {
		
		opdocursor (true); /*un-highlight the old bar cursor line*/
	
		popundoaction ();
		
		return (false);
		}
	
	oppushundo (&opbeforeundo, hnewcursor);
	
	opdirtyoutline ();
	
	hcursor = hnewcursor; /*copy into register*/
	
	(**hcursor).flcomment = bitboolean (flcomment);
	
	(**ho).hbarcursor = hcursor;
	
	opexpandupdate (hcursor);
	
	if (opdisplayenabled ())
		opvisibarcursor ();
	
	oploadeditbuffer ();
	
	opeditselectall (); // 5.0d18 dmb: in case text was added by callback
	
	return (true);
	} /*opinsertheadline*/
	

boolean opinsertstructure (hdlheadrecord hnode, tydirection dir) {
	
	/*
	12/13/91 dmb: created from oppaste to service insert verb.
	*/
	
	register hdloutlinerecord ho = outlinedata;
	register hdlheadrecord hcursor = (**ho).hbarcursor;
	hdlheadrecord nomad, nextnomad;
	hdlscreenmap hmap;
	
	opsettextmode (false);
	
	pushundoaction (undotypingstring);
	
	opbeforestrucchange (&hmap, false);
	
	opnodechanged (hcursor);
	
	(**hcursor).fldirty = true;
	
	nomad = hnode;
	
	while (true) { /*copy each of the summits on the scrap*/
		
		nextnomad = (**nomad).headlinkdown;
		
		oppushundo (&opredodelete, nomad);
		
		opdeposit (hcursor, dir, nomad);
		
		dir = down;
		
		hcursor = nomad;
		
		opresetlevels (hcursor);
		
		(**hcursor).flexpanded = true;
		
		if ((nomad == nextnomad) || (nextnomad == nil)) /*deposited the last guy*/
			break;
		
		nomad = nextnomad; /*advance to next guy on scrap*/
		} /*while*/
	
	// 6.0a14 dmb: done by afterstructchangenow -- opsetctexpanded (ho);
	
	(**ho).hbarcursor = hnode;
	
	opafterstrucchange (hmap, false);
	
	if (!opnodevisible (hcursor)) {
		
		opvisinode (hcursor, false);
		
		opvisinode (hnode, true);
		}
	
	return (true);
	} /*opinsertstructure*/


boolean opsettmpbitvisit (hdlheadrecord hnode, ptrvoid flset) {
	
	(**hnode).tmpbit = (boolean) ((long) flset);
	
	return (true);
	} /*opsettmpbitvisit*/


boolean opcleartmpbits (void) {
	
	return (opsiblingvisiter ((**outlinedata).hsummit, true, &opsettmpbitvisit, (ptrvoid) false));
	} /*opcleartmpbits*/




