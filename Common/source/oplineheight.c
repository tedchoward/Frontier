
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

/*these routines support variable lineheight in opxxx.c*/

#ifdef MACVERSION 
#include <standard.h>
#endif

#ifdef WIN95VERSION 
#include "standard.h"
#endif

#include "ops.h"
#include "opinternal.h"
#include "oplineheight.h"
#include "opdisplay.h"
#include "wpengine.h"



short opgetline1top (void) {
	
	/*
	return the number of pixels from top of outlinerect of hline1 as a negative number
	*/
	
	register hdloutlinerecord ho = outlinedata;
	short line1linesabove = (**ho).line1linesabove;
	
	if (line1linesabove == 0)
		return (0);
	else
		return (-textvertinset - ((**ho).defaultlineheight * line1linesabove));
	} /*opgetline1top*/


hdlheadrecord opgetlastvisiblenode (void) {
	
	hdloutlinerecord ho = outlinedata;
	hdlheadrecord nomad = (**ho).hline1;
	Rect r = (**ho).outlinerect;
	short vertpixels = r.bottom - r.top;
	hdlheadrecord hlastvisible;
	short ct = 0;
	
	ct += opgetline1top ();
	
	hlastvisible = nomad;
	
	while (true) {
		
		ct += opgetlineheight (nomad);
		
		if (ct > vertpixels)
			return (hlastvisible);
			
		hlastvisible = nomad;
		
		nomad = opgetnextexpanded (nomad);
		} /*while*/
	} /*opgetlastvisiblenode*/


long opgetcurrentscreenlines (boolean flscrollwise) {
	
	/*
	return the number of lines currently showing in the window.
	
	6.0a12 dmb: added flscrollwise parameter. If true, return the number
	of scroll lines on the screen; otherwise return the number of headlines, 
	the original meaning if this function
	*/
	
	hdloutlinerecord ho = outlinedata;
	hdlheadrecord nomad = (**ho).hline1, nextnomad;
	Rect r = (**ho).outlinerect;
	long vertpixels = r.bottom - r.top;
	short ctpixels = 0, ctlines = 0;
	
	ctpixels += opgetline1top ();
	
	if (flscrollwise)
		ctlines -= (**ho).line1linesabove;
	
	while (true) {
		
		ctpixels += opgetlineheight (nomad);
		
		if (ctpixels > vertpixels) {
		
			if (flscrollwise) { //need to count lines that do fit
			
				ctpixels -= opgetlineheight (nomad); //undo the add
				
				ctlines += (vertpixels - ctpixels - textvertinset) / (**ho).defaultlineheight;
				}
			
			return (ctlines);
			}
		
		if (flscrollwise)
			ctlines += opgetnodelinecount (nomad);
		else
			ctlines++;
		
		nextnomad = opgetnextexpanded (nomad);
		
		if (nextnomad == nomad) { //ran out of lines
		
			if (flscrollwise) // account for white space
				ctlines += (vertpixels - ctpixels) / ((**ho).defaultlineheight + textvertinset);
			
			return (ctlines);
			}
			
		nomad = nextnomad;
		} /*while*/
	} /*opgetcurrentscreenlines*/
	
	
long opsumprevlineheights (long lnum, short *heightthisline) {
	
	/*
	return the sum of the lineheights of all lines above this one.
	
	lnum is 0-based.
	
	6.0b2 dmb: work with negative lnums, with the same semantics (return
	a negative number)
	*/
	
	hdloutlinerecord ho = outlinedata;
	hdlheadrecord nomad = (**ho).hline1;
	long sum = 0;
	long i;
	
	sum += opgetline1top ();
	
	if (lnum < 0) {
	
		for (i = 0; i > lnum; i--) { // at least once
			
			nomad = opbumpflatup (nomad, true);
			
			*heightthisline = opgetlineheight (nomad);
			
			sum -= *heightthisline;
			}
		}
	else {
	
		for (i = 0; i < lnum; i++) {
			
			sum += opgetlineheight (nomad);
			
			nomad = opbumpflatdown (nomad, !(**ho).flprinting); //6.0b4 dmb: can't opgetnextexpanded (nomad) when printing
			} /*while*/
		
		*heightthisline = opgetlineheight (nomad);
		}
	
	return (sum);
	} /*opsumprevlineheights*/
	
	
long opsumalllineheights (void) {
	
	hdlheadrecord nomad = (**outlinedata).hsummit, nextnomad;
	long sum = 0;
	
	while (true) {
		
		sum += opgetlineheight (nomad);
		
		nextnomad = opgetnextexpanded (nomad);
		
		if (nextnomad == nomad)
			return (sum);
			
		nomad = nextnomad;
		} /*while*/
	} /*opsumalllineheights*/
	
	
long opgetlinestoscrollupforvisi (hdlheadrecord hnode) {
	
	/*
	return the number of lines you have to scroll to make the node
	visible. assume the node lies off the bottom of the window.
	
	first we figure out how many pixels off we are (ctneeded).
	
	then we loop up from the first line until we equal or exceed
	that number.
	*/
	
	hdloutlinerecord ho = outlinedata;
	hdlheadrecord nomad = (**ho).hline1;
	Rect r = (**ho).outlinerect;
	short vertpixels = r.bottom - r.top;
	long ctpixels = 0;
	long ctneeded = 0;
	long ctlines;
	long ctscroll;
	short lh;	
	
	ctpixels = opgetline1top (); //measure  from top of line1
	
	while (true) {
		
		lh = opgetlineheight (nomad);
		
		//if ((ctpixels + lh) > vertpixels) /*line doesn't fit, or completely fit*/
		//	ctneeded += lh;
			
		ctpixels += lh;
	
		if (nomad == hnode)
			break;
		
		nomad = opgetnextexpanded (nomad);
		} /*while*/
	
	ctneeded = ctpixels - vertpixels;
	
	if (ctneeded <= 0) /*it's already visible*/
		return (0);
		
	nomad = (**ho).hline1;
	
	ctpixels = opgetline1top ();
	
	ctscroll = -(**ho).line1linesabove;
	
	while (true) {
		
		lh = opgetlineheight (nomad);
		
		ctlines = opgetnodelinecount (nomad);
		
		ctpixels += lh;
		
		ctscroll += ctlines;
		
		if (ctpixels >= ctneeded) {
		
			if (ctpixels > ctneeded && ctlines > 1)
				ctscroll -= (ctpixels - ctneeded) / (**ho).defaultlineheight;
			
			return (ctscroll);
			}
			
		nomad = opgetnextexpanded (nomad);
		}
	} /*opgetlinestoscrollupforvisi*/


long opgetlinestoscrolldownforvisi (hdlheadrecord hnode) {
	
	/*
	return the number of lines you have to scroll to make the node
	visible. assume the node lies above the top of the window.
	
	we count the number of scroll lines above hline1, up to and including
	hnode
	
	6.0b2 dmb: account for text selection in headlines taller than the screen
	*/
	
	hdloutlinerecord ho = outlinedata;
	hdlheadrecord nomad = (**ho).hline1;
	hdlheadrecord hsummit = (**ho).hsummit;
	long defaultlineheight = (**ho).defaultlineheight;
	long ctscroll = 0;
	long ctpixels = 0;
	Point ptsel;
	Rect r;
	
	if (!(nomad == hnode && opeditingtext (hnode))) {
		
		ctscroll = (**ho).line1linesabove; //first, bring hline1 to top of window
		
		ctpixels = -opgetline1top ();
		}
	
	while (true) {
		
		if (nomad == hsummit) //ran out, didn't find it
			break;
		
		if (nomad == hnode)
			break;
		
		nomad = opbumpflatup (nomad, true);
		
		ctscroll += opgetnodelinecount (nomad);
		
		ctpixels += opgetlineheight (nomad);
		}
	
	if (opeditingtext (hnode)) { // see if hnode has to scroll above the top of the window
		
		opeditgetselpoint (&ptsel);
		
		ptsel.v += ctpixels;
		
		r = (**ho).outlinerect;
		
		if (ptsel.v < r.top)
			ctscroll += divup (r.top - ptsel.v, defaultlineheight);
		
		else {
			
			ptsel.v += defaultlineheight;
			
			if (ptsel.v > r.bottom)
				ctscroll -= divup (ptsel.v - r.bottom, defaultlineheight);
			}
		}
	
	return (ctscroll);
	} /*opgetlinestoscrolldownforvisi*/
