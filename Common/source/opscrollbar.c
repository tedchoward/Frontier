
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

#ifdef MACVERSION 
#include <standard.h>
#endif

#ifdef WIN95VERSION 
#include "standard.h"
#endif

#include "ops.h"
#include "shellprint.h"
#include "op.h"
#include "opinternal.h"
#include "oplineheight.h"




static long xxxgetcurrentscreenlines (void) {
	
	/*
	return the number of lines currently showing in the window.
	*/
	
	hdlheadrecord nomad = (**outlinedata).hline1, nextnomad;
	Rect r = (**outlinedata).outlinerect;
	short vertpixels = r.bottom - r.top;
	short ctpixels = 0;
	long ctlines = 0;
	
	while (true) {
		
		ctpixels += opgetlineheight (nomad);
		
		if (ctpixels > vertpixels)
			return (ctlines);
			
		ctlines++;
		
		nextnomad = opgetnextexpanded (nomad);
		
		if (nextnomad == nomad)
			return (ctlines);
			
		nomad = nextnomad;
		} /*while*/
	} /*getcurrentscreenlines*/


boolean oprestorescrollposition (void) {
	
	register hdloutlinerecord ho = outlinedata;
	hdlheadrecord hline1 = (**ho).hsummit;
	long ctscrolllines = (**ho).vertscrollinfo.cur;
	
	while (ctscrolllines > 0) {
	
		ctscrolllines -= opgetnodelinecount (hline1);
		
		if (ctscrolllines < 0) { //this is the top line, partially above the window
			
			ctscrolllines += opgetnodelinecount (hline1);
			
			break;
			}
		
		hline1 = oprepeatedbump (flatdown, 1, hline1, true); //if ctscrolllines is zero, this is line1
		}
	
	(**ho).hline1 = hline1;
	
	(**ho).line1linesabove = ctscrolllines;
	
	return (true);
	} /*oprestorescrollposition*/


boolean opsetscrollpositiontoline1 (void) {
	
	/*
	6.0b3 dmb: make the vertical scroll position agree with hline1/line1linesabove
	*/
	
	register hdloutlinerecord ho = outlinedata;
	hdlheadrecord nomad = (**ho).hsummit;
	hdlheadrecord hline1 = (**ho).hline1;
	long ctscrolllines = (**ho).line1linesabove;
	
	if (!(**hline1).flexpanded) //6.0b4 dmb: should never be true, but when printing..
		return (false);
	
	while (nomad != hline1) {
		
		ctscrolllines += opgetnodelinecount (nomad);
		
		nomad = opgetnextexpanded (nomad);
		}
	
	(**ho).vertscrollinfo.cur = ctscrolllines;
	
	return (true);
	} /*opsetscrollpositiontoline1*/


boolean opgetscrollbarinfo (boolean flpin) {
	
	/*
	9/11/91 dmb: added flpin parameter.  if true, limit the vertical 
	scroll position to the calculated vert max, instead of enlarging 
	vert to maintain the current position.  also, when not pinning, 
	make sure the vertinfo.cur isn't as large as ctexpanded
	
	9/22/92 dmb: horizinfo.max must be in scroll quanta, not pixels
	
	8/21/93 DW: add flvertscrolldisabled and flhorizscrolldisabled to 
	allow clay basket to have windows that don't enable their
	horizontal scrollbars.

	7.0b17 PBS: Don't subtract one from current screen lines --
	fixes an off-by-one bug.
	*/
	
	register hdloutlinerecord ho = outlinedata;
	//register hdlheadrecord hline1;
	long ctexpanded;
	short vscrollquantum;
	tyscrollinfo vertinfo, horizinfo;
	Rect r;
	
	if (ho == NULL)
		return (false);

	ctexpanded = (**ho).ctexpanded;

	if ((**ho).flvertscrolldisabled) 	
		vertinfo.min = vertinfo.cur = vertinfo.max = vertinfo.pag = 0;
	else {
		vertinfo.min = 0;
		
		vertinfo.cur = (**ho).vertscrollinfo.cur;
		
		vertinfo.cur = min (vertinfo.cur, ctexpanded - 1);
		
		r = (**ho).outlinerect;
		
		vscrollquantum = (**ho).defaultlineheight;
		
		vertinfo.pag = opgetcurrentscreenlines (true);// - 1; /*7.0b17 PBS: Don't subtract one. Off-by-one fix.*/

		vertinfo.max = max (vertinfo.min, ctexpanded - vertinfo.pag); /*DW 7/9/93*/
		
		if (flpin)
			vertinfo.cur = min (vertinfo.cur, vertinfo.max); /*bring it in range*/
		else
			vertinfo.max = max (vertinfo.max, vertinfo.cur); /*make it accomodate current pos*/
		
		/*
		hline1 = oprepeatedbump (flatdown, vertinfo.cur, (**ho).hsummit, true);
		
		if (hline1 != (**ho).hline1) {
		
			(**ho).hline1 = hline1;
			
			opinvaldisplay (); /*serious updating needed
			}
		*/
		}
	
	if ((**ho).flhorizscrolldisabled) 	
		horizinfo.min = horizinfo.cur = horizinfo.max = horizinfo.pag = 0;
	else {
		horizinfo.min = 0;
		
		horizinfo.cur = (**ho).horizscrollinfo.cur;
		
		r = shellprintinfo.paperrect; /*(**ho).outlinerect*/
		
		horizinfo.pag = (r.right - r.left);
		
		horizinfo.max = 3 * horizinfo.pag; /*three paper-widths*/
		
		horizinfo.cur = max (horizinfo.min, horizinfo.cur);
		
		horizinfo.cur = min (horizinfo.max, horizinfo.cur);
		}
	
	(**ho).vertscrollinfo = vertinfo;
	
	(**ho).horizscrollinfo = horizinfo;
	
	return (true);
	} /*opgetscrollbarinfo*/


void opredrawscrollbars (void) {
	
	(*(**outlinedata).setscrollbarsroutine) ();
	} /*opredrawscrollbars*/


void opresetscrollbars (void) {
	
	opgetscrollbarinfo (false);
	
	if (opdisplayenabled ())
		opredrawscrollbars ();
	} /*opresetscrollbars*/
	

