
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

#include <standard.h>
#include "font.h"
#include "ops.h"
#include "quickdraw.h"
#include "shellprint.h"
#include "tableinternal.h"



static short tablerowsperpage (void) {
	
	register hdltableformats hf = tableformatsdata;
	register short vertsave = (**hf).vertcurrent;
	short firstrow, lastrow;
	short firstcol, lastcol;
	
	(**hf).vertcurrent = 0;
	
	tablefirstpartiallyvisible (&firstrow, &firstcol);
	
	tablelastpartiallyvisible (&lastrow, &lastcol);
	
	(**hf).vertcurrent = vertsave;
	
	return (lastrow - firstrow + 1);
	} /*tablerowsperpage*/


boolean tablesetprintinfo (void) {
	
	/*
	9/24/91 dmb: handle multiple pages
	*/
	
	register hdltableformats hf = tableformatsdata;
	Rect wholerect = (**hf).wholerect;
	
	shellprintinfo.ctpages = 1; /*pretty dumb for now*/
	
	if (!tableempty ()) {
		
		tableresetrects (shellprintinfo.paperrect);
		
		shellprintinfo.ctpages = divup ((**hf).ctrows, tablerowsperpage ());
		
		tableresetrects (wholerect);
		}
	
	return (true);
	} /*tablesetprintinfo*/


boolean tableprint (short pagenumber) {
	
	/*
	9/25/91 dmb: handle multi-page tables.  also, use new tablegettablesize to 
	more intelligently set the right margin.
	*/
	
	register hdltableformats hf = tableformatsdata;
	Rect printrect = shellprintinfo.paperrect;
	Rect wholerect = (**hf).wholerect;
	long tablewidth, tableheight;
	
	tablegettablesize (&tablewidth, &tableheight); /*get width that will show all text*/
	
	tablewidth = max (tablewidth, wholerect.right - wholerect.left); /*don't make smaller than display*/
	
	printrect.right = min (printrect.right, printrect.left + tablewidth); /*maybe bring right edge in*/
	
	tableresetrects (printrect);
	
	tablerecalccolwidths (false); /*re-distribute colwidths*/
	
	(**hf).flprinting = true;
	
	tableupdatecoltitles (printrect);
	
	if (!tableempty ()) {
		
		register RgnHandle printrgn = NewRgn ();
		register short horizsave = (**hf).horizcurrent;
		register short vertsave = (**hf).vertcurrent;
		
		RectRgn (printrgn, &printrect);
		
		(**tableformatswindowinfo).drawrgn = printrgn;
		
		(**hf).horizcurrent = 0;
		
		(**hf).vertcurrent = (pagenumber - 1) * tablerowsperpage ();
		
		tableupdateseparator (printrect);
		
		tableupdategridlines (printrect);
		
		tableupdatecells (printrect);
		
		(**hf).horizcurrent = horizsave;
		
		(**hf).vertcurrent = vertsave;
		
		DisposeRgn (printrgn);
		
		(**tableformatswindowinfo).drawrgn = nil;
		}
	
	(**hf).flprinting = false;
	
	tableresetrects (wholerect);
	
	tablerecalccolwidths (false); /*re-distribute colwidths*/
	
	return (true);
	} /*tableprint*/


