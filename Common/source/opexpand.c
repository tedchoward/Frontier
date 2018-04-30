
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
#include "mouse.h"
#include "op.h"
#include "opinternal.h"
#include "oplineheight.h"
#include "opdisplay.h"
#include "opicons.h"

#include "tablestructure.h" //7.0d5 AR


static boolean flnothingcollapsed;

static long pixelscollapsed;

static long pixelsexpanded;

static long pixelsalreadyexpanded;

static long ctalreadyexpanded;



static boolean opcollapsevisit (hdlheadrecord hnode, ptrvoid refcon) {
#pragma unused (refcon)

	register hdlheadrecord h = hnode;
	
	if ((**h).flexpanded) {
		
		flnothingcollapsed = false;	
		
		(**outlinedata).ctexpanded -= opgetnodelinecount (h);
		
		(**h).flexpanded = false;
		
		pixelscollapsed += opgetlineheight (h);
		}
	
	return (true);
	} /*opcollapsevisit*/
	

void opfastcollapse (hdlheadrecord h) {
	
	/*
	clear the expanded bits of all subordinate material, don't do anything
	to the display.
	*/

	oprecursivelyvisit (h, infinity, &opcollapsevisit, nil); 
	} /*opfastcollapse*/
	

boolean opcollapse (hdlheadrecord hnode) {
	
	register hdloutlinerecord ho = outlinedata;
	long origct = (**ho).ctexpanded;
	long ctscroll;
	long lnum;
	Rect linerect;
	
	pixelscollapsed = 0;
	
	flnothingcollapsed = true;
	
	oprecursivelyvisit (hnode, infinity, &opcollapsevisit, nil); /*clear expanded bits*/
	
	if (flnothingcollapsed)
		return (false);
	
	opdirtyview ();
	
	if (!opdisplayenabled ())
		goto exit;
	
	opgetscreenline (hnode, &lnum);
	
	opgetlinerect (lnum, &linerect);
	
//	opgeticonrect (hnode, &linerect, &iconrect);
	
//	invalrect (iconrect); /*the icon changes, avoid flash, don't inval the whole line*/
	
	opdrawicon (hnode, linerect);

	if (opgetnextexpanded (hnode) == hnode) { /*last expanded node, nothing to scroll up*/
		
		Rect r = linerect;
		
		r.top = linerect.bottom; 
		
		r.bottom = (**ho).outlinerect.bottom;
		
		smashrect (r); 
		
		goto exit;
		}
	
	if (linerect.bottom >= (**ho).outlinerect.bottom)
		goto exit;
	
	ctscroll = origct - (**ho).ctexpanded;
	
	if (ctscroll > 0) {
		
		Rect r = (**ho).outlinerect;
		
		if (linerect.bottom > r.top) { /*headline isn't above display*/
			
			r.top = linerect.bottom;
			
			opscrollrect (r, 0, -pixelscollapsed);
			}
		}

	exit:
	
	opresetscrollbars (); /*number of expanded lines changed*/
	
	(*(**ho).postcollapsecallback) (hnode);
	
	opupdatenow (); /*fill in newly revealed stuff immediately*/
	
	return (true); /*something was collapsed*/
	} /*opcollapse*/


static boolean opexpandvisit (hdlheadrecord hnode, ptrvoid refcon) {
#pragma unused (refcon)

	short lh = opgetlineheight (hnode);
	
	if ((**hnode).flexpanded) {
	
		++ctalreadyexpanded;
		
		pixelsalreadyexpanded += lh;
		}
	else {
		(**outlinedata).ctexpanded += opgetnodelinecount (hnode);
		
		(**hnode).flexpanded = true;
		
		(**hnode).fldirty = true;
		
		pixelsexpanded += lh;
		}
	
	return (true);
	} /*opexpandvisit*/
	

boolean opexpand (hdlheadrecord hnode, short level, boolean flmaycreatesubs) {
	
	/*
	expand the given node to the indicated level.
	
	scroll the screen vertically to make the last sub-node visible, if
	possible.
	
	return true if something was expanded, false otherwise.
	
	10/18/91 dmb: account for already-expanded subheads when no scrolling is required
	
	6.0b2 dmb: fixed scrollbar handling for large expansions
	*/
	
	register hdloutlinerecord ho = outlinedata;
	Rect outlinerect = (**ho).outlinerect;
	long origct = (**ho).ctexpanded;
	long lnum;
	Rect linerect, iconrect, r;
	long hscroll, vscroll;
	
	if (!(*(**ho).preexpandcallback) (hnode, level, flmaycreatesubs))
		return (false);
	
	ctalreadyexpanded = 0;
	
	pixelsexpanded = 0;
	
	pixelsalreadyexpanded = 0;
	
	oprecursivelyvisit (hnode, level, &opexpandvisit, nil);
	
	origct = (**ho).ctexpanded - origct; 
	
	if (origct == 0) /*nothing expanded*/
		return (false); 
	
	opdirtyview ();
	
	if (!opdisplayenabled ())
		return (true);
	
	opgetscreenline (hnode, &lnum);
	
	opgetlinerect (lnum, &linerect); 
	
	/*handle case where height of the head + subs is > height of win*/ {
		
		long heightheads = pixelsexpanded + pixelsalreadyexpanded + opgetlineheight (hnode);
		long heightwin = outlinerect.bottom - outlinerect.top;
		
		if (heightheads > heightwin) { /*smash the display, no optimization possible*/
		
			if (hnode != (**ho).hline1) {
				
				opsetline1 (hnode);
				
				opsetscrollpositiontoline1 ();
				}
			
			/*
			opgetscrollbarinfo (false); 
			
			(**ho).vertscrollinfo.cur += lnum; //text scrolls up
			*/
			
			opresetscrollbars (); 
			
			opseteditbufferrect (); //in case we're in text mode
			
			opinvaldisplay ();
			
			//operaserect ((**ho).outlinerect);
			
			opupdatenow ();
			
			return (true);
			}
		}
	
	//else
	//	opinvalnode (hnode);
	
	opresetscrollbars (); 
	
	if (opneedvisiscroll (oplastexpanded (hnode), &hscroll, &vscroll, false)) {
	
		(**ho).blockvisiupdate = true;
		
		opdovisiscroll (hscroll, vscroll);
		
		(**ho).blockvisiupdate = false;
		
		//opinvalafter ((**hnode).headlinkright);
	 	opinvalafter (hnode);
	 	}
	else {
		r = linerect; /*get ready to scroll to make room for newly visible text*/
		
		r.top = r.bottom;
		
		if (ctalreadyexpanded > 0) { /*account for already-expanded lines*/
			
			r.bottom = r.top + pixelsalreadyexpanded;
			
			invalrect (r);
			
			r.top = r.bottom;
			}
		
		r.bottom = outlinerect.bottom;
		
		opscrollrect (r, 0, pixelsexpanded); /*scroll text down to make room for new heads*/
		
		opinvalnode (hnode);
		}
	
	opupdatenow (); 
	
	return (true);
	} /*opexpand*/
	

static void oprecursivelyexpandto (hdlheadrecord hnode) {
	
	/*
	work your way left until you find a node which is expanded.  expand it.
   
	then retrace your steps, expanding as you go.
	*/
	
	hdlheadrecord hparent = (**hnode).headlinkleft;
	
	if (hparent == hnode) /*at a summit, all summits are expanded*/
		return;
	
	if ((**hnode).flexpanded) 
		return;
		
	oprecursivelyexpandto (hparent); /*recurse*/
	
	oprecursivelyvisit (hparent, 1, &opexpandvisit, nil); /*set expanded bits on sibs*/
	} /*oprecursivelyexpandto*/


void opexpandto (hdlheadrecord hnode) {
	
	/*
	make sure that the indicated node is expanded. also moves 
	the cursor to the node.
	
	8/10/92 dmb: pophoists as necessary to make the node accessable
	*/
	
	register hdloutlinerecord ho = outlinedata;
	
	if ((**ho).hbarcursor == hnode) /*cursor is already on the node*/
		return;
	
	while (((**ho).tophoist > 0) && !opcontainsnode ((**ho).hsummit, hnode))
		oppophoist ();
	
	if ((**hnode).flexpanded) { /*just move the cursor to it and return*/
		
		opmoveto (hnode);
	
		opdirtyview ();
		
		return;
		}
	
	/*the node we're looking for isn't expanded*/
	
	oprecursivelyexpandto (hnode);
	
	opsetscrollpositiontoline1 ();
	
	opresetscrollbars ();
	
	opmoveto (hnode);
	
	opsmashdisplay ();
	
	opdirtyview ();
	} /*opexpandto*/


void opexpandtoggle (void) {
	
	/*
	if expanded, collapse.  if not expanded, expand.
	
	11/4/92 dmb: always try one or the other, in case subheads appear dynamically
	
	7.0d5 AR: Run op callback scripts 
	*/
	
	register hdlheadrecord hcursor = (**outlinedata).hbarcursor;
	register hdlheadrecord hright = (**hcursor).headlinkright;
	
	if ((hright != hcursor) && (**hright).flexpanded) { /*first subhead is showing, collapse is called for*/

		if (langopruncallbackscripts (idopcollapsescript))
			return;  /*fuction consumed the click*/

		opcollapse (hcursor);
		}		
	else {

		if (langopruncallbackscripts (idopexpandscript))
			return;  /*fuction consumed the click*/

		opexpand (hcursor, 1, true);
		}
	} /*opexpandtoggle*/


void opexpandupdate (hdlheadrecord hnewnode) {
	
	/*
	5.0b6 dmb: allow for partially-visible lines
	*/
	
	register hdloutlinerecord ho = outlinedata;
	long ctlines;
	long lnum;
	
	(**hnewnode).flexpanded = true; 
	
	ctlines = opgetnodelinecount (hnewnode);
	
	(**ho).ctexpanded += ctlines;
	
	if (opdisplayenabled ()) {
		
		opresetscrollbars ();
		
		opgetscreenline (hnewnode, &lnum);
		
		if (lnum < 0)
			(**ho).vertscrollinfo.cur += ctlines;
		
		else if (lnum <= opgetcurrentscreenlines (false)) {/*node is visible*/
		
			opmakegap (lnum - 1, opgetlineheight (hnewnode));
			
			opupdatenow ();
			}
		}
	} /*opexpandupdate*/


void opcollapseall (void) {
	
	hdlheadrecord nomad = (**outlinedata).hsummit;
	boolean fldisplaywasenabled;
	
	fldisplaywasenabled = opdisabledisplay ();
	
	while (true) {
		
		opjumpto (nomad);
	
		if (opsubheadsexpanded (nomad)) 
			opcollapse (nomad);
			
		if (!opchasedown (&nomad)) {
			
			opjumpto ((**outlinedata).hsummit);
			
			if (fldisplaywasenabled) {
				
				openabledisplay ();
				
				opinvaldisplay ();
				}
			
			return;
			}
		} /*while*/
	} /*opcollapseall*/
	
	
boolean opsetlongcursor (long cursor) { 

	hdlheadrecord hcursor = (hdlheadrecord) cursor;

	if (!opnodeinoutline (hcursor)) /*it's been deleted, or something*/
		return (false);
	
	opexpandto (hcursor);
	
	return (true);
	} /*opsetlongcursor*/
	





