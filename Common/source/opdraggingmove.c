
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
#include "kb.h"
#include "mouse.h"
#include "quickdraw.h"
#include "ops.h"
#include "smallicon.h"
#include "frontier_strings.h"
#include "shell.rsrc.h"
#include "opdraggingmove.h"
#include "opdisplay.h"
#include "oplineheight.h"
#include "opicons.h"
#include "shellundo.h"




#define draggingpix 3 /*distance to initiate drag*/

#define draggingticks 30 /*time before auto-drag initiation*/

#define draggingsloppix 10 /*any further outsize displayrect cancels drag*/

#define dragginghotspot (iconstart + 5)

#define hotspotturnedon true /*set to true and hotspot is displayed while dragging*/

static hdlheadrecord hfirstmoved;

static boolean opredocopy (hdlheadrecord hnode, boolean flundo); /*forward*/

#pragma pack(2)
typedef struct tydraginfo {
	
	hdlheadrecord hnode;
	
	tydirection dir;
	} tydraginfo, *ptrdraginfo;
#pragma options align=reset


#if false

void opgetwindowhandle (Point pt, Handle *windowhandle) {
	
	opgetwindowhandlecallback cb = (**outlinedata).getwindowhandlecallback;
	
	if (cb == nil) {
		
		localtoglobalpoint (nil, &pt);
		
		findappwindow (pt, (hdlappwindow *) windowhandle);
		}
	else
		(*cb) (pt, windowhandle);
	} /*opgetwindowhandle*/


void opsetwindowhandlecontext (Handle windowhandle) {
	
	opsetwindowhandlecontextcallback cb = (**outlinedata).setwindowhandlecontextcallback;
	
	if (cb == nil)
		setappwindow ((hdlappwindow) windowhandle);
	else
		(*cb) (windowhandle);
	} /*opsetwindowhandlecontext*/

#endif

static short oppointlevel (Point pt) {
	
	/*
	give me a point in the display rectangle, and I'll tell you what 
	level it refers to.
	
	to visualize the process, draw vertical lines on the screen at 
	(**outlinedata).lineindent increments.  each line corresponds to a
	level in the outline.  we match the pt up with the levels.
	*/
	
	register hdloutlinerecord ho = outlinedata;
	hdlheadrecord hsummit = (**ho).hsummit;
	Rect r = (**ho).outlinerect;
	short indent;
	
	insetrect (&r, -draggingsloppix, -draggingsloppix);
	
	if (!pointinrect (pt, r)) /*too sloppy -- no move*/
		return (-1);
	
	indent = (pt.h - (**ho).outlinerect.left) - (opnodeindent (hsummit) + dragginghotspot);
	
	return ((**hsummit).headlevel + divround (indent, (**ho).lineindent));
	} /*oppointlevel*/


static boolean oppointaboveline1 (Point pt) {
	
	Rect r = (**outlinedata).outlinerect;
	
	r.top += 3; /*the top 3 pixels of the outline display cause scrolling*/
	
	return (pt.v <= r.top);
	} /*oppointaboveline1*/
	

void operasehotspot (tyhotspot *hotspot) {
	
	register tyhotspot *hs = hotspot;
	
	if ((*hs).fldisplayed) {
		
		opdrawarrowicon ((*hs).htarget, (*hs).lnum, nodirection);
		
		(*hotspot).fldisplayed = false;
		}
	} /*operasehotspot*/
	

static void opdrawhotspot (tyhotspot *hotspot) {
	
	register tyhotspot *hs = hotspot;
	
	opdrawarrowicon ((*hs).htarget, (*hs).lnum, (*hs).dir);
	
	(*hs).fldisplayed = true;
	} /*opdrawhotspot*/
	

static boolean isundermark (hdlheadrecord htarget, tydirection dir) {
#pragma unused (dir)

	while (true) {
		
		if (opgetmark (htarget))
			return (true);
		
		if (!opnavigate (left, &htarget))
			return (false);
		} /*while*/
	} /*isundermark*/


static void opupdatehotspot (Point ptstart, Point pt, tyhotspot *hotspot) {
	
	/*
	the hot spot is displayed as the user scrolls around during the first
	half of the dragging move operation.  it gives him a pretty good idea
	of where the moving headline is likely to land.
	
	we maintain a record that fully describes the hotspot, a pointer to
	it is passed to us.
	
	8/27/92 dmb: added multiple selection support, and fixed dragging above line1
	
	5.0.2b14 dmb: added ptstart, so we can avoid unintentional drags
	*/
	
	register tyhotspot *hs = hotspot;
	register hdloutlinerecord ho = outlinedata;
	register hdlheadrecord htarget;
	register hdlheadrecord hline1 = (**ho).hline1;
	register hdlheadrecord hsummit = (**ho).hsummit;
	register hdlheadrecord hsource = (*hs).hsource;
	register tydirection dir;
	register short mouselevel;
	long lnum;
	boolean fldraw;
	
	if ((equalpoints (pt, (*hs).pt)) && (*hs).fldisplayed) /*no change*/
		return;
	
	mouselevel = oppointlevel (pt);
	
	if (mouselevel < 0) /*pointing outside the content region of window*/
		goto noway;
	
	if (pointdist (ptstart, pt) < draggingpix)
		goto noway;
	
	if (oppointaboveline1 (pt)) { /*try to move up from the summit*/
		
		if (hline1 == hsource) /*can't move summit up from itself*/
			goto noway;
		
		htarget = (**ho).hline1;
		
		if (htarget == hsummit) {
			
			dir = up;
			
			goto gottarget;
			}
		
		htarget = opbumpflatup (htarget, true);
		}
	else
		htarget = oppointnode (pt); /*find out which node he's pointing to*/
	
	if (htarget == nil) /*he's not pointing at a real line*/
		goto noway;
	
	if (opsubordinateto (htarget, hsource)) /*can't move under self*/
		goto noway;
		
	if (htarget == hsource) { /*pointing at the same guy we're moving*/
		
		register short level = (**htarget).headlevel;
		
		if (mouselevel == level) /*no move required*/
			goto noway;
			
		if (mouselevel > level) { /*trying to move to the right of node up from here*/
			
			while (true) { /*seek upwards*/
				
				if (opfirstinlist (htarget)) /*no move possible*/
					goto noway;
				
				htarget = (**htarget).headlinkup;
				
				if (!(**htarget).flmarked) /*not among group being dragged*/
					break;
				}
			
			if (opnosubheads (htarget)) { /*becomes only subhead of up sibling*/
				
				dir = right;
				
				goto gottarget;
				}
			
			htarget = oplastexpandedatlevel (htarget, mouselevel);  
			
			if ((**htarget).headlevel < mouselevel)
				dir = right;
			else
				dir = down;
			
			goto gottarget;
			}
			
		/*mouselevel < level*/
		
		htarget = opgetancestor (htarget, mouselevel);
		
		dir = down; 
		
		goto gottarget; 
		} /*htarget == hsource*/
	
	if (mouselevel > (**htarget).headlevel) {
		
		if (hsource == (**htarget).headlinkright) /*already there*/
			goto noway;
		
		dir = right; 
		
		goto gottarget;
		}
	
	dir = down; /*not pointing at moving node, not moving to right of something*/
	
	while (mouselevel < (**htarget).headlevel)
		htarget = (**htarget).headlinkleft;
	
	if (hsource == (**htarget).headlinkdown) /*already there*/
		goto noway;
	
	gottarget: /*display the hotspot, save values in the structure and return*/
	
	if (isundermark (htarget, dir))
		goto noway;
	
	if (!(*(**ho).validatedragcallback) (hsource, htarget, dir))
		goto noway;
	
	opgetscreenline (htarget, &lnum);
	
	if ((lnum < 0) && (dir == down)) {
		
		hdlheadrecord hdown = (**htarget).headlinkdown;
		
		if ((hdown != htarget) && opnodevisible (hdown)) {
			
			htarget = hdown;
			
			dir = up;
			
			opgetscreenline (hdown, &lnum);
			}
		}
	
	fldraw = true;
	
	if ((*hs).fldisplayed) {
		
		if (((*hs).htarget == htarget) && ((*hs).lnum == lnum) && ((*hs).dir == dir))
			fldraw = false;
		}
	
	if (fldraw)
		operasehotspot (hs);
	
	(*hs).fldisplayed = true;
	
	(*hs).lnum = lnum;
	
	(*hs).pt = pt;
	
	(*hs).dir = dir;
	
	(*hs).htarget = htarget;
	
	if (fldraw)
		opdrawhotspot (hs);
	
	return;
	
	noway: /*goto here if the mouse points to a place we can't move to*/
	
	operasehotspot (hs);
	
	(*hs).htarget = nil; 
	} /*opupdatehotspot*/


static boolean opundocopy (hdlheadrecord hnode, boolean flundo) {
	
	if (flundo)
		oppushundo (&opredocopy, hnode);
	
	return (true);
	} /*opundocopy*/


static boolean opredocopy (hdlheadrecord hnode, boolean flundo) {
	
	if (flundo)
		oppushundo (&opundocopy, hnode);
	else
		opdisposestructure (hnode, true);
	
	return (true);
	} /*opredocopy*/


static boolean movetohotspotvisit (hdlheadrecord hnode, ptrvoid refcon) {
	
	register hdlheadrecord h = hnode;
	ptrdraginfo draginfo = (ptrdraginfo) refcon;
	register hdlheadrecord hpre = (*draginfo).hnode;
	tydirection dir = (*draginfo).dir;
	
	if (!(**h).tmpbit) {
		
		oppushunmarkundo (hnode);
		
		if ((*(**outlinedata).dragcopycallback) (h, hpre)) {
			
			h = opcopyoutline (h); /*caution: wipes out recursion globals*/
			
			if (h == nil)
				return (false);
			
			oppushundo (&opundocopy, h);
			}
		else
			opunlink (h); 
		
		opdeposit (hpre, dir, h);
		
		opresetlevels (h);
		
		(*draginfo).hnode = h;
		
		(*draginfo).dir = down;
		
		(**h).tmpbit = true; /*don't visit again*/
		
		if (hfirstmoved == nil)
			hfirstmoved = h;
		}
	
	return (true);
	} /*movetohotspotvisit*/


boolean opmovetohotspot (tyhotspot *hotspot) {
	
	/*
	9/3/93 DW: allow predragcallback to alert us that he's deleted the target
	node. we can safely redefine the move operation, and we do.
	
	9/22/93 dmb: allow postpastecallback to post-process the drag -- perhaps 
	to resort the level. also, turned off dropboxdrag, which conflicts with 
	some applets' use of the option key (Clay Basket, for one). We could 
	add an op preference or flag for this, but it's an obscure feature and 
	I don't think it's worth the mess.
	
	DW 4/10/95: I don't see what the conflict is! I'm re-enabling it, it was
	requested by a tester.

	5.0b9 dmb: improved the predrag callback so it can modify the target to be
	safe instead of returning -1 to say it's bad
	*/
	
	register tyhotspot *hs = hotspot;
	//hdlheadrecord hsource = (*hs).hsource;
	hdlheadrecord htarget = (*hs).htarget;
	tydirection dir = (*hs).dir;
	hdlheadrecord horigcursor = (**outlinedata).hbarcursor;
	hdlscreenmap hmap;
	tydraginfo draginfo;
	
	boolean fldropboxdrag = false;
	
	if (htarget == nil) /*wasn't pointing at anything we could interpret*/
		return (false);
	
	if ((dir == right) && !opsubheadsexpanded (htarget)) {
		
		fldropboxdrag = optionkeydown (); /*deposit nodes below target w/out expanding*/
		
		if (ophassubheads (htarget)) {
			
			hdlheadrecord hlastsub;
			
			if (!fldropboxdrag) 
				opexpand (htarget, 1, false);
			
			hlastsub = opgetlastsubhead (htarget);
			
			if (hlastsub != htarget) { /*DW 9/1/93: allow for clay basket*/
				
				htarget = hlastsub;
			
				dir = down;
				}
			}
		}
	
	#if false /*DW 4/10/95 disabled*/
	
		if (fldropboxdrag) { /*flash the icon for feedback*/
			
			register short ctflashes = 2;
			
			while (--ctflashes >= 0) {
				
				opdrawhotspot (hs);
				
				delayticks (8);
				
				operasehotspot (hs);
				
				delayticks (4);
				}
			}
	#endif
	
	pushundoaction (undomovestring);
	
	opbeforestrucchange (&hmap, false);
	
	/*DW 9/3/93 -- defend against the target getting deleted in predragcallback*/ {
	
		//hdlheadrecord htargetparent = (**htarget).headlinkleft;
		
		switch ((*(**outlinedata).predragcallback) (&htarget, &dir)) {
			
			case false: /*user declined to do the move after hearing the consequences*/
				goto exit;
			
		//	case (boolean) -1: /*special signal that the target has been deleted*/
		//		htarget = htargetparent;
		//		
		//		dir = right;
		//		
		//		break;
			} /*switch*/
		}
	
	draginfo.hnode = htarget;
	
	draginfo.dir = dir;
	
	hfirstmoved = nil;
	
	opvisitmarked (nodirection, &movetohotspotvisit, &draginfo);
	
	opcleartmpbits (); /*set by movetohotspotvisit*/
	
	(*(**outlinedata).postpastecallback) (hfirstmoved);
	
	if (fldropboxdrag) {
		
		htarget = (*hs).htarget; /*back to parent, in case it already had kids*/
		
		opfastcollapse (htarget);
		
		opclearmarks (htarget);
		
		opmoveto (htarget); /*move to "drop box" headline*/
		
		opresetscrollbars ();
		}
	else {
		
		opvisinode (draginfo.hnode, false); /*visi the last node deposited*/
		
		(**outlinedata).hbarcursor = horigcursor;
		
		opmoveto (hfirstmoved); /*move to first node deposited*/
		}
	
	exit:
	
	opafterstrucchange (hmap, false);
	
	opsetctexpanded (outlinedata); /*DW 4/10/95*/
	
	return (true);
	} /*opmovetohotspot*/


void opscrollfordrag (tyhotspot *hotspot, tydirection scrolldir) {
	
	register hdloutlinerecord ho = outlinedata;
	register long vertcurrent = (**ho).vertscrollinfo.cur;
	register tydirection dir = scrolldir;
	
	if ((vertcurrent <= (**ho).vertscrollinfo.min) && (dir == down)) /*no scrolling possible*/
		return;
		
	if ((vertcurrent >= (**ho).vertscrollinfo.max) && (dir == up))
		return;
	
	operasehotspot (hotspot);
	
	opscroll (dir, false, 1);	
	} /*opscrollfordrag*/
	
	
boolean opisdraggingmove (Point ptorig, unsigned long origticks) {

	Point ptnew;
	
	getmousepoint (&ptnew);
		
	return ((pointdist (ptorig, ptnew) > draggingpix) || ((gettickcount () - origticks) > draggingticks));
	} /*opisdraggingmove*/
	
	
void opdraggingmove (Point ptstart, hdlheadrecord hsource) {
	
	/*
	9/22/93 dmb: need to validate a drag just like a copy. also need to 
	provide for client sorting, like a paste
	*/
	
	long tc = 0;
	tyhotspot hotspot;
	Point pt;
	tydirection dir;
	
	if (!((**outlinedata).validatecopycallback) (STR_move))
		return;
	
	clearbytes ((ptrchar) &hotspot, longsizeof (hotspot));
	
	hotspot.pt = ptstart;
	
	hotspot.hsource = hsource;
	
	/*
	opgetwindowhandle (pt, &hotspot.sourcewindowhandle);
	
	hotspot.destwindowhandle = hotspot.sourcewindowhandle;
	*/
	
	if (true /*claystartdrag(ptstart, hsource) != noErr*/) {
		
		while (mousestilldown ()) {
			
			getmousepoint (&pt);
			
			#if false
			
				Handle currentwindow;
			
				if (!pointinrect (pt, (*FrontWindow ()).portRect))
					Debugger ();
			
				opgetwindowhandle (pt, &currentwindow);
				
				if (currentwindow != hotspot.destwindowhandle) { /*pointing into another window*/
					
					opsetwindowhandlecontext (hotspot.destwindowhandle);
			
					operasehotspot (&hotspot);
					
					hotspot.destwindowhandle = currentwindow;
					
					opsetwindowhandlecontext (hotspot.destwindowhandle);
			
					opupdatehotspot (ptstart, pt, &hotspot);
					}
			#endif
			
			if ((gettickcount () - tc) > draggingscrollrate) {
				
				if (mousecheckautoscroll (pt, (**outlinedata).outlinerect, false, &dir)) {
				
					opscrollfordrag (&hotspot, dir);
					
					tc = gettickcount ();
					}
				}
			
			opupdatehotspot (ptstart, pt, &hotspot);
			} /*while*/
		
		operasehotspot (&hotspot); /*user let up on the button*/
	
		opmovetohotspot (&hotspot);
		}  // if noErr
	} /*opdraggingmove*/




