
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
#include "kb.h"
#include "icon.h"
#include "mouse.h"
#include "ops.h"
#include "quickdraw.h"
#include "smallicon.h"
#include "sounds.h"
#include "strings.h"
#include "shellhooks.h"
#include "langexternal.h"
#include "tableinternal.h"
#include "tableverbs.h"



void tablegettitlestring (short col, bigstring bstitle) {
	
	if ((col >= namecolumn) && (col <= kindcolumn))
		tablegetstringlist (nametitlestring + col, bstitle);
	else
		tablegetstringlist (questionmarksstring, bstitle);
	} /*tablegettitlestring*/


static void tablesettitlestyle (short col) {
	
	register hdltableformats hf = tableformatsdata;
	short fontstyle;
	
	if (col == (**hf).sortorder)
		fontstyle = bold + underline;
	else
		fontstyle = bold;
	
	setfontsizestyle ((**hf).fontnum, (**hf).fontsize, fontstyle);
	} /*tablesettitlestyle*/


static boolean tableadjustcolwidthcallback (short col, short cellwidth) {
	
	/*
	called after the user resized the indicated column.  after setting the
	colwith, we adjust the widths of all columns to the right.  they maintain
	their previous proportions to each other, but fill all the space to
	the right of the column that was resized.
	
	this rule makes sense for tables that don't have a horizontal scrollbar,
	but you still want to allow for column resizing.
	
	6/22/92 dmb: canned the proportional auto-adjustment logic.  with the new, 
	improved auto-column sizing, dragging column widths should take user's  
	actions as literally as possible.
	*/
	
	register hdltableformats hf = tableformatsdata;
	register long leftoverwidth;
	register short i;
	short colwidth;
	short ctcols = (**hf).ctcols;
	
	tablesetcolwidth (col, cellwidth, true);
	
	for (i = col + 1; i < ctcols; i++) {
		
		tablegetcolwidth (i, &colwidth);
		
		leftoverwidth = tableavailwidth () - tablesumcolwidths (0, ctcols);
		
		tablesetcolwidth (i, colwidth + leftoverwidth, true);
		}
	
	/*
	if (col == lastcol)
		return (true);
	
	col += 1; /*adjustment begin with next column%/
	
	sum = tablesumcolwidths (col, (**hf).ctcols - col);
	
	leftoverwidth = tableavailwidth () - tablesumcolwidths (0, col);
	
	for (i = col; i < lastcol; i++) {
		
		register long x; /*do math in longs, avoid overflow%/
		
		tablegetcolwidth (i, &colwidth);
		
		x = colwidth;
		
		x *= leftoverwidth;
		
		if (sum > 0)
			x /= sum;
		
		tablesetcolwidth (i, (short) x, true);
		} /*for%/
	
	tablesetcolwidth (lastcol, tableavailwidth () - tablesumcolwidths (0, lastcol), false);
	*/
	
	return (true);
	} /*tableadjustcolwidthcallback*/
	

static boolean tablegetcellstring (short row, short col, bigstring bs) {
	
	/*
	1/8/91 dmb: push/pophashtable around call to hashvaltostrings, since 
	we can no longer assume that the root table is always set
	*/
	
	bigstring bstype, bssize;
	tyvaluerecord val;
	hdlhashtable htable;
	
	tablegetstringlist (questionmarksstring, bs);
	
	htable = tablegetlinkedhashtable ();
	
	if (!hashgetiteminfo (htable, row, bs, &val))
		return (false);
	
	if (col == namecolumn) 
		return (true); /*with bs set to the name of the variable*/
	
	shellpusherrorhook ((errorhookcallback) &falsenoop);
	
	pushhashtable (htable);
	
	hashvaltostrings (val, bstype, bs, bssize); /*accept error values on failure*/
	
	pophashtable ();
	
	shellpoperrorhook ();
	
	if (col == valuecolumn) 
		return (true); /*with bs set to the value of the variable*/
		
	if (stringlength (bssize) > 0) { /*non-trivial size, show it*/
		
		pushstring ("\p [", bstype);
		
		pushstring (bssize, bstype);
		
		pushchar (']', bstype);
		}
	
	copystring (bstype, bs);
	
	return (true);
	} /*tablegetcellstring*/
	
	
static boolean tabledrawcellcallback (short row, short col, Rect rcell) {
	
	bigstring bs;
	
	if ((**tableformatsdata).fleditingcell)
		if (tableeditdrawcell (row, col))
			return (true);
	
	if (!tablegetcellstring (row, col, bs))
		return (false);
	
	pendrawstring (bs);
	
	return (true);
	} /*tabledrawcellcallback*/


static boolean tablecellcontentwidth (short row, short col, short *width) {
	
	bigstring bs;
	
	if (!tablegetcellstring (row, col, bs)) /*error*/
		return (false);
	
	*width = stringpixels (bs);
	
	return (true);
	} /*tablecellcontentwidth*/


static boolean tablekeystrokecallback (void) {
	
	/*
	return true to consume keystroke, false to pass it thru to the table
	displayer.  we let up and down arrow move us thru the table in bar
	cursor mode.
	
	2.1b2 dmb: only capture modified returns -- don't ouch unconditionally
	*/
	
	if ((keyboardstatus.chkb == chenter) && (keyboardstatus.ctmodifiers > 0)) {
		
		if (keyboardstatus.flcmdkey) {
			
			if (keyboardstatus.flshiftkey)
				tablesurface ();
			else
				tabledive ();
				
			return (true);
			}
		
		ouch (); /*we're reserving the rest of the modified enter keystrokes*/
			
		return (true); /*consume the keystroke*/
		}
	
	if (keyboardstatus.chkb == chreturn) {
		
		if (keyboardstatus.ctmodifiers > 0) {
			
			tablemakenewvalue ();
			
			return (true);
			}
		}
	
	return (tableeditkeystroke ());
	} /*tablekeystrokecallback*/
	
	
static boolean tableadjustcursorcallback (short row, short col) {
	
	return (tableeditadjustcursor (row, col));
	} /*tableadjustcursorcallback*/


static boolean tablecellclickcallback (Point pt, short row, short col) {
	
	if (tableeditmousedown (pt, row, col))
		return (true);
	
	if ((col == valuecolumn) && mousedoubleclick ())
		return (tablezoomfromcell (row, valuecolumn));
	
	return (false);
	
	/*
	return (tableeditmousedown (pt, row, col));
	*/
	} /*tablecellclickcallback*/


static Rect titlerect;

static void tableinverttitle (boolean flinrectnow) {
	
	invertrect (titlerect);
	} /*tableinverttitle*/


static boolean tabletitleclickcallback (Point pt, short col) {
	
	bigstring bs;
	
	tablegettitlerect (col, &titlerect);
	
	tablegettitlestring (col, bs);
	
	tablesettitlestyle (col);
	
	titlerect.right = titlerect.left + stringpixels (bs) + 4;
	
	insetrect (&titlerect, 1, 1);
	
	if (pointinrect (pt, titlerect))
		if (trackicon (titlerect, &tableinverttitle))
			tablesetsortorder (col);
	
	return (true);
	} /*tabletitleclickcallback*/


static void tabledirtycallback (void) {
	
	register hdlhashtable ht = (hdlhashtable) (**tableformatsdata).hdata;
	
	(**ht).fldirty = true;
	} /*tabledirtycallback*/
	

static boolean tabledrawtitlecallback (short col, Rect r) {
	
	/*
	12/12/90 dmb: only underline title that corresponds to sortorder.  note
	that we don't need to preserve the font/size/style in this routine
	*/
	
	bigstring bs;
	
	tablegettitlestring (col, bs);
	
	tablesettitlestyle (col);
	
	pendrawstring (bs);
	
	return (true);
	} /*tabledrawtitlecallback*/
	

static boolean tableverbislinked (short row) {
	
	/*
	return true if the table row has an item linked into it -- governs the display
	of smallicons in the table displayer, and the dimming or not dimming of one of the
	buttons.
	*/
	
	bigstring bsname;
	tyvaluerecord val;
	
	if (!tablegetiteminfo (row, bsname, &val)) 
		return (false);
	
	return (val.valuetype == externalvaluetype);
	} /*tableverbislinked*/


/*
boolean tablegetwedgeicon (row, iconlist, iconnum) short *iconlist, *iconnum; {
	
	/*
	11/11/90 DW: tried to get it to work just like the outline processor, but...
	
	when a window closes it's very hard to know which icon needs updating, going
	from black to gray.  
	
	rationalization: our icons are black even when a window is open because
	double clicking on them does something meaningful.  you can put that in 
	the brochure for the goddam product!
	
	11/15/90 DW: we're going to try again.  it requires that each external window
	manager call a table routine when it closes its window to inval the icon
	in front of the line we zoomed from.
	%/

	*iconlist = leadersmalliconlist;
	
	*iconnum = tablegrayicon;
	
	if (tableverbislinked (row)) {
		
		if (!tableverbwindowopen (row)) {
		
			*iconnum = tableblackicon;
			}
		}
		
	return (true);
	} /*tablegetwedgeicon%/
*/

	
static boolean tablegetcircleicon (short row, short *iconlist, short *iconnum) {
	
	register hdltableformats hf = tableformatsdata;
	register short x;
	
	if (tableverbislinked (row)) 
		x = tablefullicon;
	else
		x = tableemptyicon;
	
	if (!(**hf).flprinting)
		if (row == (**hf).rowcursor)
			x += 2;
	
	*iconnum = x;
	
	return (true);
	} /*tablegetcircleicon*/
	
/*
static boolean tablegetfileicon (short row, short *iconlist, short *iconnum) {
	
	bigstring bs;
	tyvaluerecord val;
	register short x;
	
	if (!tablegetiteminfo (row, bs, &val))
		return (false);
	
	*iconlist = tablesmalliconlist;
	
	if (val.valuetype != externalvaluetype) {
	
		x = scalaricon;
		
		goto exit;
		}
		
	switch ((**(hdlexternalhandle) val.data.externalvalue).id) {
	
		case idtableprocessor:
			x = tableicon; 
			
			break;
		
		case idwordprocessor:
			x = wordicon; 
			
			break;
		
		case idpictprocessor:
			x = picticon; 
			
			break;
		
		case idoutlineprocessor:
			x = outlineicon; 
			
			break;
		
		case idscriptprocessor:
			x = scripticon; 
			
			break;
		
		case idmenuprocessor:
			x = menuicon; 
			
			break;
			
		default:
			x = unknownicon;
			
			break;
		} /*switch%/
	
	exit:
	
	*iconnum = x;
	
	return (true);
	} /*tablegetfileicon*/
	

static boolean tablegeticoncallback (short row, short *iconlist, short *iconnum) {
	
	/*
	11/15/90 DW: experimenting with different kinds of icons -- this front-end
	makes it easy for us to switch back and forth depending on the mood.
	*/
	
	return (tablegetcircleicon (row, iconlist, iconnum));
	} /*tablegeticoncallback*/


static boolean tableiconhitcallback (short row, Rect ricon) {
	
	if (!mousedoubleclick ())
		return (true);
	
	tableupdatenow (); /*show new state*/
	
	invalrect (ricon); /*state of icon probably changing*/
	
	if (!tablezoomfromcell (row, -1))
		return (false);
	
	return (true);
	} /*tableiconhitcallback*/


/*
boolean tableiconhitcallback (short row, boolean flzoom, Rect ricon) {
	
	tableeditleavecell (); /*if editing, save off%/
	
	if (flzoom) {
		
		tableupdatenow (); /*show new state%/
		
		invalrect (ricon); /*state of icon probably changed%/
		
		if (!tablezoomfromcell (row, 0))
			return (false);
		
		return (true);
		}
	
	return (true);
	} /*tableiconhitcallback*/
	
	
static boolean tableleavecellcallback (void) {

	return (tableeditleavecell ()); 
	} /*tableleavecellcallback*/
	

static boolean tablesetscrollbarcallback (void) {
	
	register ptrtableformats pf = *tableformatsdata;
	register ptrwindowinfo pi = *tableformatswindowinfo;
	
	(*pi).vertmin = (*pf).vertmin;
	
	(*pi).vertmax = (*pf).vertmax;
	
	(*pi).vertcurrent = (*pf).vertcurrent;
	
	(*pi).fldirtyscrollbars = true; /*force a refresh of scrollbars by the shell*/
	
	return (true);
	} /*tablesetscrollbarcallback*/


static boolean tablepostmovecallback (void) {
	
	register hdltableformats hf = tableformatsdata;
	register hdlextrainfo hi = (hdlextrainfo) (**hf).hextrainfo;
			
	shellforcecursoradjust (); /*context change may require new cursor shape*/
	
	if (!tableeditleavecell ()) /*if in edit mode, accept new entry*/
		return (false);
		
	/***tablecheckzoombutton (); /*maybe inval it if state changed*/
	
	/*
	invalrect ((**hi).kindpopuprect);
	*/
	
	tableinvalicon ((**hf).rowcursor);
	
	return (true);
	} /*tablepostmovecallback*/


static boolean tablepremovecallback (void) {
	
	tableinvalicon ((**tableformatsdata).rowcursor);
	
	return (tableeditleavecell ());
	} /*tablepremovecallback*/


static boolean tableresetcellrectscallback (void) {
	
	/*
	after a scroll, resize, font change, etc., the rect for any given cell 
	may change.  this callback allows the table client to adjust anything 
	that depends on a cell's rect
	*/
	
	tableeditsetbufferrect ();
	
	return (true);
	} /*tableresetcellrectscallback*/


void tableinstallcallbacks (hdltableformats hformats) {
	
	register hdltableformats hf = hformats;
	
	(**hf).cellcontentwidthroutine = (callback) &tablecellcontentwidth;
	
	(**hf).drawcellroutine = (callback) &tabledrawcellcallback;
	
	(**hf).drawtitleroutine = (callback) &tabledrawtitlecallback;
	
	(**hf).adjustcolwidthroutine = (callback) &tableadjustcolwidthcallback;
	
	(**hf).precursormoveroutine = &tablepremovecallback;
	
	(**hf).postcursormoveroutine = &tablepostmovecallback;
	
	(**hf).cellclickroutine = (callback) &tablecellclickcallback;
	
	(**hf).titleclickroutine = (callback) &tabletitleclickcallback;
	
	(**hf).poststylechangeroutine = &tableeditpoststylechange;
	
	(**hf).dirtyroutine = &tabledirtycallback;
	
	(**hf).geticonroutine = (callback) &tablegeticoncallback;
	
	(**hf).iconhitroutine = (callback) &tableiconhitcallback;
	
	(**hf).islinkedroutine = (callback) &tableverbislinked;
	
	(**hf).setscrollbarsroutine = &tablesetscrollbarcallback;
	
	(**hf).adjustcursorroutine = (callback) &tableadjustcursorcallback;
	
	(**hf).keystrokeroutine = &tablekeystrokecallback;
	
	(**hf).leavecellroutine = &tableleavecellcallback;
	
	(**hf).resetcellrectsroutine = &tableresetcellrectscallback;
	} /*tableinstallcallbacks*/



