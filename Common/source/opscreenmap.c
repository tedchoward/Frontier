
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

#include "memory.h"
#include "quickdraw.h"
#include "opinternal.h"
#include "oplineheight.h"
#include "opdisplay.h"



boolean opnewscreenmap (hdlscreenmap *hmap) {
	
	/*
	take a snapshot of what headrecord is displayed on what screen line.
	*/
	
	tymapelement *pe;
	short ctelements;
	hdloutlinerecord ho = outlinedata;
	hdlheadrecord nomad = (**ho).hline1;
	hdlheadrecord hcursor = (**ho).hbarcursor;
	hdlscreenmap h;
	hdlheadrecord orignomad;
	long sizemap;
	Handle hnew;
	
	ctelements = opgetcurrentscreenlines (false) + 1; /*allow for partially visible line*/
	
	sizemap = sizeof (tyscreenmap) + (ctelements * sizeof (tymapelement));
	
	if (!newclearhandle (sizemap, &hnew)) {
		
		*hmap = nil;
		
		return (false);
		}
	
	h = *hmap = (hdlscreenmap) hnew; /*copy into returned value, register*/
	
	(**h).ctelements = ctelements;

	for (pe = (**h).map;  ctelements;  --ctelements, ++pe) {
		
		(*pe).hnode = nomad;
		
		(*pe).level = (**nomad).headlevel;
		
		(*pe).vpixels = (**nomad).vpixels;
		
		(*pe).flcursorhere = (nomad == hcursor);
		
		(*pe).flmarked = (**nomad).flmarked;
		
		orignomad = nomad;
		
		nomad = opgetnextexpanded (nomad);
	
		if (nomad == orignomad) /*rest of the items are already 0'd*/
			break;
		} /*for*/
	
	return (true);
	} /*opnewscreenmap*/
	
	
void opinvalscreenmap (hdlscreenmap hmap) {
	
	/*
	inval those lines in the current display that differ from
	the elements of the screenmap.  we're willing to do a lot
	of analysis to avoid displaying a line that does not need
	to be displayed.
	
	things that make a line dirty: 
	
	1. if its fldirty bit is set, 
	
	2. if it is now blank, but wasn't blank before, 
	
	3. if a different node is displayed on the line,
	
	4. if the level of the node changed,
	
	5. if the cursor is on the line, but wasn't before,
	
	6. if the cursor was on the line before, but isn't now.
	
	7. 5.0a25 dmb: if the node's marked status changed (and it's not the barcursor)
	
	when we're done we dispose of the map, since every caller
	seems to want to do that as the next thing.

	5.0b18 dmb: handle having more display lines than in the map
	*/
	
	hdlheadrecord nomad;
	short i;
	boolean fldirty;
	hdlscreenmap h = hmap;
	hdloutlinerecord ho = outlinedata;
	hdlheadrecord hnode;
	hdlheadrecord hcursor = (**ho).hbarcursor;
	short ctelements;
	tymapelement element;
	long ctlines;
	Rect r, rscroll;
	long scrollpixels = 0;
	
	if (!opdisplayenabled ()) {
	
		disposehandle ((Handle) h); 
		
		return;
		}
	
	if (h == nil) { /*no screenmap*/
	
		opinvaldisplay ();
		
		return;
		}
	
	nomad = (**ho).hline1;
	
	ctelements = (**h).ctelements;
	
	for (i = 0; i < ctelements; i++) {
		
		element = (**h).map [i];
		
		fldirty = false;
		
		if (nomad == nil) {
			
			if (element.hnode != nil)
				fldirty = true;
				
			goto L1;
			}
		
		opgetlineheight (nomad); //force calculation
		
		if (element.vpixels != (**nomad).vpixels) { /*level changed*/
			
			fldirty = true;
			
			if (scrollpixels == 0) { //don't already have a scroll
				
				fldirty = false; //we're going to set up special invals
				
				opgetlinerect (i, &rscroll);
				
				scrollpixels = (**nomad).vpixels - element.vpixels;
				
				if (scrollpixels < 0) //getting shorter
					rscroll.bottom = rscroll.top + element.vpixels; //original height
				}
			
			(**nomad).fldirty = false; //make sure we're clean
			
			goto L1;
			}
		
		if ((**nomad).fldirty) {
			
			fldirty = true;
			
			(**nomad).fldirty = false; /*consume it*/
			
			goto L1;
			}
			
		if (element.level != (**nomad).headlevel) { /*level changed*/
			
			fldirty = true;
			
			goto L1;
			}
		
		hnode = element.hnode; /*copy into register*/
		
		if (hnode != nomad) { /*different node on this line*/
			
			fldirty = true;
			
			goto L1;
			}
		
		if ((hnode == hcursor) != element.flcursorhere) { /*bar cursor is/was on this line*/
		
			fldirty = true;
			
			goto L1;
			}
		
		if ((**hnode).flmarked != element.flmarked) {
			
			fldirty = true;
			
			goto L1;
			}
		
		L1:
		
		if (fldirty) {
		
			if (scrollpixels) { // dirty node after a scroll node
				
				rscroll.bottom = (**ho).outlinerect.bottom;
				
				invalrect (rscroll);
				
				scrollpixels = 0; //consumed
				
				break;
				}
			
			if (nomad != nil)
				oplineinval (i);
			else {
				
				opgetlinerect (i - 1, &r);

				r.top = r.bottom;

				r.bottom = (**ho).outlinerect.bottom;

				invalrect (r);
				}
			}
			
		if (nomad != nil) {
			
			hdlheadrecord orignomad = nomad;
			
			nomad = opgetnextexpanded (nomad);
			
			if (nomad == orignomad)
				nomad = nil;
			}
		} /*for*/
	
	if (scrollpixels) { //an element changed height, no dirty lines after
		
		invalrect (rscroll);
		
		rscroll.top = rscroll.bottom;
		
		if (scrollpixels < 0)
			rscroll.top += scrollpixels;
		
		rscroll.bottom = (**ho).outlinerect.bottom;
		
		opscrollrect (rscroll, 0, scrollpixels);
		}
	
	ctlines = opgetcurrentscreenlines (false) + 1;
	
	if ((ctelements < ctlines) && (nomad != nil)) //more lines now than in the map
		opinvalafter (nomad);
	
	disposehandle ((Handle) h); /*no longer needed*/
	} /*opinvalscreenmap*/


void opinvaldirtynodes (void) {
	
	/*
	high-level guys can set the dirty bits on the nodes whose attributes they've
	changed. we take care of inval'ing all the nodes.
	
	this can save having to push and pop the port a lot. just dirty the bits and
	set the port and call us.
	*/
	
	hdlscreenmap hmap;
	
	opnewscreenmap (&hmap);
	
	opinvalscreenmap (hmap);
	} /*opinvaldirtynodes*/
	
	
	
	
