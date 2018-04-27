
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
#include "ops.h"
#include "strings.h"
#include "shell.h"
#include "shellprivate.h"
#include "shellundo.h"
#include "langexternal.h"
#include "opinternal.h"
#include "tablestructure.h"
#include "tableinternal.h"
#include "wpengine.h"
#include "claybrowser.h"
#include "claycallbacks.h"



boolean tablecelliseditable (hdlheadrecord hnode, short col) {
	
	/*
	return true if there should be an ibeam cursor in this cell.
	*/
	
	hdlhashtable htable;
	bigstring bs;
	tyvaluerecord val;
	hdlhashnode hhashnode;
	
	if (!tablegetiteminfo (hnode, &htable, bs, &val, &hhashnode))
		return (false);
		
	switch (col) {
		
		case namecolumn: /*names are always editable*/				
			return (true);
		
		case valuecolumn: /*only scalar values are editable*/
			return ((val.valuetype != externalvaluetype) && (val.valuetype != codevaluetype));
		
		default: /*kind is not directly editable*/
			return (false);
		} /*switch*/
	} /*tablecelliseditable*/


static boolean getvalueedittext (const tyvaluerecord *val, Handle *htext) {
	
	/*
	return the editable text corresponding to the value.
	*/
	
	tyvaluerecord valcopy;
	bigstring bsvalue;
	
	switch ((*val).valuetype) {
	
		case stringvaluetype:
		case listvaluetype:
		case recordvaluetype:
			if (!copyvaluerecord (*val, &valcopy) || !coercetostring (&valcopy))
				return (false);
		
			exemptfromtmpstack (&valcopy);
			
			*htext = valcopy.data.stringvalue;
			
			break;
		
		case binaryvaluetype:
			if (!bytestohex ((*val).data.binaryvalue, htext))
				return (false);
			
			pullfromhandle (*htext, 2, 8, nil); /*get rid of the binary type*/
			
			break;
		
		default:
			hashgetvaluestring (*val, bsvalue);
			
			if (!newtexthandle (bsvalue, htext))
				return (false);
			
			break;
		}
	
	return (true);
	} /*getvalueedittext*/


boolean tablesetwpedittext (hdlheadrecord hnode) {
	
	/*
	2/7/97 dmb: adapted from tableeditentercell, but our role is 
	more focused: we just need to put the right text into the wp.
	*/
	
	hdltableformats hc = tableformatsdata;
	short col = (**hc).focuscol;
	hdlhashtable htable;
	tyvaluerecord val;
	bigstring bs;
	boolean fl = false;
	Handle htext = nil;
	hdlhashnode hhashnode;
	
	(**hc).focuscol = 0; // reset
	
	switch (col) {
	
		case namecolumn:
			getheadstring (hnode, bs);
			
			fl = newtexthandle (bs, &htext);
			
			break;
		
		case valuecolumn:
			if (!tablegetiteminfo (hnode, &htable, bs, &val, &hhashnode))
				break;
			
			if (val.valuetype == externalvaluetype) /*can't edit external in its cell*/			
				break;
			
			pushhashtable (htable);
			
			fl = getvalueedittext (&val, &htext);
			
			pophashtable ();
			
			break;
		
		default:
			break;
		}
	
	if (fl) {
		
		fl = wpsettexthandle (htext);

		wpscroll (right, false, longinfinity);
		}
	
	if (!fl) { /*out of memory*/
		
		disposehandle (htext);
		
		return (false);
		}
	
	(**wpdata).flneverscroll = false; /*we'll handle all scrolling, visiing ourselves*/
	
	(**wpdata).flalwaysmeasuretext = false; //we don't need the size info
	
	wpupdate ();
	
	(**hc).editcol = col;
	
	(**hc).editnode = hnode;
	
	(**hc).editval = htext;
	
	return (true);
	} /*tablesetwpedittext*/


boolean tablegetwpedittext (hdlheadrecord hnode, boolean flunload) {
	
	/*
	2/7/97 dmb: adapted from tableeditleavecell
	
	called when some action implies that the user is finished editing the cell.
	
	we attempt to save off the new value, or new name, potentially causing an 
	error if there was a duplicate name, or something bad about the value.
	*/
	
	hdltableformats hc = tableformatsdata;
	short col = (**hc).editcol;
	tybrowserspec fs;
	tyvaluerecord val;
	bigstring bstext;
	boolean fl = false;
	Handle htext;
	tyvaluerecord newval;
	Handle x;
	hdlhashtable ht;
	boolean flmustexit;
	hdlhashnode hhashnode;
	
	//assert (hnode == (**hc).editnode);
	
	flmustexit = false; // ***flmustexiteditmode;
	
	if (!wpgettexthandle (&htext))
		return (false);
	
	texthandletostring (htext, bstext);
	
	if (!claygetfilespec (hnode, &fs)) { // a new node
		
		assert (col == namecolumn);
		}
	else {

		ht = fs.parID;
		
		if (!hashtablesymbolexists (ht, fs.name)) { /*node is gone!*/
			
			fl = true;
			
			goto exit;
			}
		}
	
	if (col == namecolumn) {
		
		if (isemptystring (bstext)) /*name wasn't changed; undo will be bogus*/
			killundo ();
		else
			fl = opsetheadstring (hnode, bstext); // must do first so we can 
		
		goto exit;
		}
	
	/*we were editing a value - run a script to do the assignment*/
	
	assert (col == valuecolumn);
	
	flmustexit = true; /*don't want to stick in edit mode w/out error dialogs*/
	
	if (equalhandles (htext, (**hc).editval)) { /*no change*/
	
		fl = true;
		
		goto exit;
		}
	
	if (!claylookupvalue (&fs, &val, &hhashnode))
		goto exit;
	
	if (isemptystring (bstext) && (val.valuetype != novaluetype) && (val.valuetype != stringvaluetype)) {
		
		fl = true;
		
		goto exit;
		}
	
	if (val.valuetype == binaryvaluetype) {
	
		OSType binarytype = getbinarytypeid (val.data.binaryvalue);
		Handle hbytes;
		
		if (!hextobytes (htext, &hbytes))
			goto exit;
		
		if (!setbinaryvalue (hbytes, binarytype, &newval))
			goto exit;
		
		if (hashtableassign (ht, fs.name, newval)) {
			
			exemptfromtmpstack (&newval);
			
			fl = true;
			}
		
		goto exit;
		}
	
	if (!copyhandle (htext, &x))
		goto exit;
	
	tablepushcontext (ht, val.valuetype);
	
	disablelangerror ();
	
	shellblockevents (); /*don't want backgrounding during assignment*/
	
	
		if (langrun (x, &newval) && (newval.valuetype != externalvaluetype) && !isemptystring (bstext))
			disposehandle (htext);
		
		else {
			if (val.valuetype != stringvaluetype && !isemptystring (bstext)) {
			
				byte ch = chdoublequote;
				
				if (copyhandle (htext, &x)) { /*2.1b2: try again, with quotes*/
					
					insertinhandle (x, 0, &ch, 1);
					
					enlargehandle (x, 1, &ch);
					
					if (langrun (x, &newval) && (newval.valuetype != externalvaluetype)) {
						
						disposehandle (htext);
						
						htext = nil;
						}
					}
				}
			
			if (htext != nil)
				setheapvalue (htext, stringvaluetype, &newval);
			}
	
	htext = nil; /*it's been consumed, one way or another*/
	
	shellpopevents ();
	
	if ((val.valuetype == novaluetype) || coercevalue (&newval, val.valuetype)) {
		
		if (hashtableassign (ht, fs.name, newval)) {
			
			exemptfromtmpstack (&newval);
			
			fl = true;
			}
		}
	
	enablelangerror ();
	
	if (!fl) {
		
		sysbeep ();
		
		/*disposevaluerecord (newval, false);*/
		
		cleartmpstack ();
		}
	
	tablepopcontext (ht, val.valuetype);
	
	exit:
	
	disposehandle (htext);
	
	if (flunload) { /*exiting edit mode*/
		
		disposehandle ((**hc).editval);
		
		(**hc).editval = nil;
		
		(**hc).editnode = nil;
		}
	
	return (fl);
	} /*tablegetwpedittext*/


boolean tableeditgetundoglobals (long *globals) {
	
	/*
	callback for undo mechanism.  this routine should stuff into the 
	globals parameter enough information to restore the undo context in 
	tableeditsetundoglobals below.  in the current undo implementation, there 
	is no "disposeundoglobals" callback, so nothing should be allocated by 
	this routine.
	
	for tables, the row & column of the cell being edited is all the 
	information we need
	
	4.1b2 dmb: need more info in globals now that leaving a cell can move it.
	*/
	
	(**tableformatsdata).undocol = (**tableformatsdata).editcol;
	
	return (opeditgetundoglobals (globals));
	} /*tableeditgetundoglobals*/


boolean tableeditsetundoglobals (long globals, boolean flundo) {
	
	if (!globals || !flundo)
		return (true);
	
	(**tableformatsdata).focuscol = (**tableformatsdata).undocol;
	
	return (opeditsetundoglobals (globals, flundo));
	} /*tableeditsetundoglobals*/


