
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
#include "quickdraw.h"
#include "kb.h"
#include "textdisplay.h"
#include "shell.h"
#include "shellprint.h"
#include "op.h"
#include "opinternal.h"
#include "wpengine.h"




/*
static void opgetprintdisplayinfo (tytextdisplayinfo *info) {
	
	register hdloutlinerecord ho = outlinedata;
	
	gettextdisplayinfo (
		shellprintinfo.paperrect, (**ho).fontnum, (**ho).fontsize, (**ho).fontstyle, 
		
		(**ho).linespacing, (**ho).lineindent, info);
	} /%opgetprintdisplayinfo%/
*/

static short opgetpagecount (void) {
	
	/*
	5.0.2b20 dmb: we're called after beginprint, so we need to account 
	for scaling.

	6.0b4 dmb: now, beginprint doesn't resize, so we must not use scaling
	*/

	hdlheadrecord nomad = (**outlinedata).hsummit, nextnomad;
	short vertpixels;
	short ctpages = 1;
	short sum = 0;
	short lh; 
	typrintinfo *lpi = &shellprintinfo;
	
	vertpixels = (lpi->paperrect.bottom - lpi->paperrect.top); //6.0b4, was: * lpi->scaleMult / lpi->scaleDiv;
	
	while (true) {
		
		lh = opgetlineheight (nomad);
		
		sum += lh;
		
		if (sum > vertpixels) {
			
			ctpages++;
			
			sum = lh;
			}
		
		nextnomad = opbumpflatdown (nomad, false);
		
		if (nextnomad == nomad)
			return (ctpages);
			
		nomad = nextnomad;
		} /*while*/
	} /*opgetpagecount*/
	
	
void opgetprintrect (Rect *r) {
	
	long scaleMult, scaleDiv;
	
	getprintscale (&scaleMult, &scaleDiv);
	
	(*r).top = (long) (shellprintinfo.paperrect.top * scaleMult) / scaleDiv;
	(*r).bottom = (long) (shellprintinfo.paperrect.bottom * scaleMult) / scaleDiv;
	(*r).left = (long) (shellprintinfo.paperrect.left * scaleMult) / scaleDiv;
	(*r).right = (long) (shellprintinfo.paperrect.right * scaleMult) / scaleDiv;
	} /*opgetprintrect*/


boolean opsetprintinfo (void) {
	
	shellprintinfo.ctpages = opgetpagecount ();
	
	return (true);
	} /*opsetprintinfo*/


boolean opbeginprint (void) {
	
	if (opisfatheadlines (outlinedata))
		wpbeginprint ();
	
	/*
	Rect r;
	long scaleMult, scaleDiv;
	
	getprintscale (&scaleMult, &scaleDiv);
	
	r.top = (long) (shellprintinfo.paperrect.top * scaleMult) / scaleDiv;
	r.bottom = (long) (shellprintinfo.paperrect.bottom * scaleMult) / scaleDiv;
	r.left = (long) (shellprintinfo.paperrect.left * scaleMult) / scaleDiv;
	r.right = (long) (shellprintinfo.paperrect.right * scaleMult) / scaleDiv;
	
	opresize (r);
	*/

	return (true);
	} /*opbeginprint*/


boolean opendprint (void) {
	
	/*
	opresize ((**outlinewindowinfo).contentrect); 
	*/
	
	if (opisfatheadlines (outlinedata))
		wpendprint ();
	
	oppostfontchange (); //cleanup measurements that couldn't be made while printing

	return (true);
	} /*opendprint*/


boolean opprint (short pagenumber) {
	
	/*
	6.0b3 dmb: added before/afterprintpage callbacks, opgetprintrect;
			   various display maintenance fixes
	*/

	register hdloutlinerecord ho = outlinedata;
	short vertpixels;
	short lnum = 0;
	short sum = 0;
	hdlheadrecord nomad;
	tyscrollinfo oldvertscrollinfo, oldhorizscrollinfo;
	hdlheadrecord oldline1;
	short oldline1linesabove;
	//typrintinfo *pi = &shellprintinfo;
	Rect printrect;
//	Rect oldoutlinerect;
	short olddefaultlineheight;
	boolean flmore;
	
	opgetprintrect (&printrect);

	//vertpixels = (pi->paperrect.bottom - pi->paperrect.top) * pi->scaleMult / pi->scaleDiv;
	vertpixels = printrect.bottom - printrect.top;

	if (pagenumber == 1) /*first page?*/
		nomad = (**ho).hsummit;
	else
		nomad = (**ho).hprintcursor;
		
//	oldoutlinerect = (**ho).outlinerect;
	
//	(**ho).outlinerect = shellprintinfo.paperrect;
	
	olddefaultlineheight = (**ho).defaultlineheight;
	
	oldline1 = (**ho).hline1;
	
	oldline1linesabove = (**ho).line1linesabove;
	
	oldhorizscrollinfo = (**ho).horizscrollinfo;
	
	oldvertscrollinfo = (**ho).vertscrollinfo;
	
	(**ho).horizscrollinfo.cur = 0;
	
	(**ho).flprinting = true; /*for display special-casing*/
	
	opresize (printrect);
	
	(*(**ho).beforeprintpagecallback) ();
	
 	oppushstyle (ho);
	
	(**ho).hline1 = nomad;
	
	(**ho).line1linesabove = 0;
 	
	while (true) {
 		
 		Rect r;
		
		opgetlinerect (lnum++, &r);
 			
 		sum += r.bottom - r.top;
 		
 		if (sum > vertpixels) { /*line doesn't fit on page*/
			
			flmore = true;

 			break;
			}
		
		opdrawline (nomad, r);
		
		if (!opnavigate (flatdown, &nomad)) {
			
			flmore = false;
			
 			break;
			}
 		} /*for*/
	
 	popstyle ();
	
	(**ho).hprintcursor = nomad;
	
	(**ho).flprinting = false;
	
	opresize ((**outlinewindowinfo).contentrect); 
	
	(*(**ho).afterprintpagecallback) ();
	
	(**ho).horizscrollinfo = oldhorizscrollinfo;
	
	(**ho).vertscrollinfo = oldvertscrollinfo;

//	(**ho).outlinerect = oldoutlinerect;
	
	(**ho).hline1 = oldline1;
	
	(**ho).line1linesabove = oldline1linesabove;
	
	(**ho).defaultlineheight = olddefaultlineheight;
	
	opdirtymeasurements ();
	
	opredrawscrollbars ();
	
	return (flmore);
	} /*opprint*/



