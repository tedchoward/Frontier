
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
#include "scrap.h"
#include "strings.h"
#include "smallicon.h"
#include "cursor.h"
#include "kb.h"
#include "ops.h"
#include "mouse.h"
#include "sounds.h"
#include "timedate.h"
#include "shell.h"
#include "shellundo.h"
#include "shell.rsrc.h"
#include "lang.h"
#include "langinternal.h"
#include "op.h"
#include "opinternal.h"
#include "opicons.h"
#include "oplineheight.h"
#include "opdraggingmove.h"
#include "scripts.h"
#include "tablestructure.h" //7.0d5 AR
#include "frontierwindows.h" /*7.0b10 PBS*/


#ifdef appletinclude
	
	#define mousedoubleclick() 		(mousestatus.fldoubleclick);
	#define mousestilldown()		(StillDown())
	#define pointinrect(pt, r)		(PtInRect (pt, &r)
	#define gettickcount()			(TickCount ())
	#define langzoomobject(bs)		((void *) 0)
	#define uppercasechar(ch)		(toupper(ch))

#endif


WindowPtr outlinewindow; 

hdloutlinerecord outlinedata; 

hdlwindowinfo outlinewindowinfo; 



tyopuserprefs opprefs = { true, false };

static boolean flvisiforiconclick = false; /*makes it possible for 2clicking on icon 2 not be interrupted by horiz visi'ing*/




static boolean opcantedittext (hdlheadrecord hnode) {
	
	register hdloutlinerecord ho = outlinedata;
	
	if ((**ho).flreadonly)
		return (true);
	
	return (!(*(**ho).caneditcallback) (hnode));
	} /*opcantedittext*/
	

static boolean opcanteditcursor (void) {	
	
	return (opcantedittext ((**outlinedata).hbarcursor));
	} /*opcanteditcursor*/
	

void opdirtyoutline (void) {
	
	/*
	2.1b4 dmb: update timelastsave on each change
	
	7.0b6 PBS: scripts can dirty a read-only outline.
	*/
	
	register hdloutlinerecord ho = outlinedata;
	if (outlinedata==0) return;
	
	/*assert (!(**ho).flreadonly);*/ /*can't dirty a read-only outline*/
	/*7.0b6 PBS: scripts can dirty a read-only outline.*/
	
	if ((**ho).flwindowopen && (outlinewindowinfo != nil)) /*the window is dirty*/
		windowsetchanges (outlinewindow, true);
	
	(**ho).fldirty = true; /*the outline structure is dirty*/
	
	(**ho).flrecentlychanged = true; /*in case someone is maintaining a parallel structure*/
	
	(**ho).timelastsave = timenow (); /*modification time until saved*/
	} /*opdirtyoutline*/


void opdirtyview (void) {
	
	/*
	the user has made a view-only change: cursor move, scroll, window resize, 
	expand, collapse, etc.  we want to record this fact, but will let opverbs 
	decide whether or not it's important.
	*/
	
	(**outlinedata).fldirtyview = true; /*the outline structure is dirty*/
	} /*opdirtyview*/


boolean opistextmode (void) {
	
	return ((**outlinedata).fltextmode);
	} /*opistextmode*/


static boolean ophaslinkedtext (hdlheadrecord hnode) {
	
	return ((*(**outlinedata).haslinkedtextcallback) (hnode));
	} /*ophaslinkedtext*/
	

void opvisibarcursor (void) {

	boolean flhoriz = true;

	if ((**outlinedata).flhorizscrolldisabled)
		flhoriz = false;
	
	opvisinode ((**outlinedata).hbarcursor, flhoriz); 
	} /*opvisibarcursor*/


void opschedulevisi (void) {
	
	(**outlinedata).flcheckvisi = true;
	
	(**outlinedata).timevisi = gettickcount ();
	
	if (flvisiforiconclick) {
		
		flvisiforiconclick = false; /*consume it*/
		
		(**outlinedata).timevisi += getmousedoubleclicktime ();
		}		
	} /*opschedulevisi*/


static void opcheckvisi (void) {
	
	register hdloutlinerecord ho = outlinedata;
	
	if ((**ho).flcheckvisi && (**ho).flactive) {
		
		if (gettickcount () > (**ho).timevisi) {
		
			opvisibarcursor ();
		
			(**ho).flcheckvisi = false;
			}
		}
	} /*opcheckvisi*/


void oppoststylechange (void) {
	
	/*
	5.0a13 dmb: preserve recentlychanged so style change won't force recompile
	*/
	
	hdloutlinerecord ho = outlinedata;
	boolean flrecentlychanged = (**ho).flrecentlychanged;

	opsaveeditbuffer ();
	
	operasedisplay ();
	
	/*opsmashdisplay ();*/
	
	if (!(**ho).flreadonly) {
		
		opdirtyoutline ();

		(**ho).flrecentlychanged = flrecentlychanged; //don't dirty this flag on our account
		}
	
	(**outlinewindowinfo).selectioninfo.fldirty = true;
	
	opsetdisplaydefaults (ho);
	
	(*(**ho).postfontchangecallback) ();
	
	opgetscrollbarinfo (true);
	
	oprestorescrollposition ();
	
	opredrawscrollbars ();

	oprestoreeditbuffer ();

	opupdatenow ();
	} /*oppoststylechange*/


static boolean undosetfont (Handle hdata, boolean flundo) {
	
	if (flundo)
		opsetfont ((short) hdata);
	
	return (true);
	} /*undosetfont*/


boolean opsetfont (short fontnum) { 
	
	/*
	DW 8/21/93: allow font change for read-only outlines.
	*/
	
	hdloutlinerecord ho = outlinedata;
	
	if ((**ho).fontnum == fontnum)
		return (false);
	
	pushundostep (&undosetfont, (Handle) (**ho).fontnum);
	
	(**ho).fontnum = fontnum;
	
	oppoststylechange ();
	
	return (true);
	} /*opsetfont*/


static boolean undosetsize (Handle hdata, boolean flundo) {
	
	if (flundo)
		opsetsize ((short) hdata);
	
	return (true);
	} /*undosetsize*/


boolean opsetsize (short fontsize) {
	
	hdloutlinerecord ho = outlinedata;
	
	if ((**ho).fontsize == fontsize)
		return (false);
	
	pushundostep (&undosetsize, (Handle) (**ho).fontsize);
	
	(**ho).fontsize = fontsize;
	
	oppoststylechange ();
	
	return (true);
	} /*opsetsize*/
	

boolean opsetselectioninfo (void) {

	tyselectioninfo x;
	
	clearbytes (&x, sizeof (x)); /*initialize all fields to zero*/
	
	x.flcansetfont = true;
	
	x.flcansetsize = true;
	
	x.fontnum = (**outlinedata).fontnum; 
	
	x.fontsize = (**outlinedata).fontsize; 
	
	(**outlinewindowinfo).selectioninfo = x;
	
	return (true);
	} /*opsetselectioninfo*/


boolean opsettextmode (boolean fltextmode) {
	
	/*
	switches between textmode and structure mode and vice versa.
	
	just hacks its way into opmoveto, which is equipped to deal with
	a mode change.
	*/
	
	register hdloutlinerecord ho = outlinedata;
	
	if (opcanteditcursor ())
		return (true);
	
	if (fltextmode == (**ho).fltextmode) /*we're already in requested mode*/
		return (true);
	
	(**ho).flcursorneedsdisplay = true; /*make sure opmoveto does something*/
	
	(**ho).fltextmode = fltextmode;
	
	if (fltextmode)
		opclearallmarks ();
	
	opmoveto ((**ho).hbarcursor);
	
	opeditresetselpoint (); /*cursoring up & down should stick to new horiz position*/
	
	opschedulevisi ();
	
	return (true);
	} /*opsettextmode*/


static void optoggletextmode (void) {
	
	opsettextmode (!opistextmode ());
	} /*optoggletextmode*/


static struct {
	
	long offset;
	
	hdlheadrecord hnode;
	} scanstate;


static boolean opfindoffsetvisit (hdlheadrecord hnode, ptrvoid refcon) {
	
	bigstring bs;
	short len;
	
	scanstate.hnode = hnode;
	
	scanstate.offset -= (**hnode).headlevel; /*account for leading tabs*/
	
	if (opnestedincomment (hnode))
		setemptystring (bs);
	else
		getheadstring (hnode, bs);
	
	len = stringlength (bs) + 1; /*leading tabs, text, trailing cr*/
	
	if (scanstate.offset <= len)
		return (false); /*terminate the traversal*/
	
	scanstate.offset -= len; 
	
	return (true); /*continue the traversal*/
	} /*opfindoffsetvisit*/


#if isFrontier

boolean opshowerror (long lnum, short charnum) {
	
	/*
	map a line number and character position onto a spot in the current
	outline structure.  we traverse the outline, counting nodes until we
	hit the line numbered "lnum".  then we expand to that node.
	
	if charnum is greater than zero, we position the text mode cursor on
	the indicated character.
	
	10/23/91 dmb: if charnum is less than zero, make sure we're not in 
	edit mode.
	
	2.1b5 dmb: handle encoded absolute offset for non-UserTalk outlines
	*/
	
	hdlheadrecord hnodeerror;
	
	if ((**outlinedata).outlinesignature != typeLAND) {
		
		scanstate.offset = langgetsourceoffset (lnum, charnum);
		
		opsiblingvisiter ((**outlinedata).hsummit, false, &opfindoffsetvisit, nil);
		
		hnodeerror = scanstate.hnode;
		
		charnum = (short) scanstate.offset;
		}
	else {
		
		if (!opgetnthnode (lnum, &hnodeerror))
			return (false);
		}
	
	opclearallmarks ();
	
	opexpandto (hnodeerror);
	
	if (charnum >= 0) {
		
		opsettextmode (true);
		
		opeditsetselection (charnum, charnum);
		}
	else
		opsettextmode (false);
	
	return (true);
	} /*opshowerror*/

#endif


boolean opsetscrap (hdlheadrecord hnode) {
	
	/*
	7/9/91 dmb: in order to reliably dispose and export menubar scraps, 
	outline scraps are now hdloutlinerecords, not hdlheadrecords.  this 
	could use further cleanup; ultimately, we'd like to put packed outlines 
	onto the clipboard along with text
	
	9/9/91 dmb: we've been putting packed outlines onto the clipboard for 
	export for a while now.  now we want to maintain formatting info in the 
	structured scrap so it doesn't get lost when pasted into a table cell.
	
	2.1b5 dmb: grab the source outline's signature (can affect text scrap 
	conversion)
	
	5.0.2b17 dmb: set ctexpanded
	*/
	
	register hdloutlinerecord ho;
	hdloutlinerecord houtline;
	
	if (hnode == nil) /*defensive driving*/
		return (false);
	
	if (!newoutlinerecord (&houtline))
		return (false);
	
	ho = houtline; /*copy into register*/
	
	(**ho).copyrefconcallback = (**outlinedata).copyrefconcallback;
	
	(**ho).textualizerefconcallback = (**outlinedata).textualizerefconcallback;
	
	(**ho).releaserefconcallback = (**outlinedata).releaserefconcallback;
	
	(**ho).outlinesignature = (**outlinedata).outlinesignature;
	
	opsetsummit (ho, hnode);
	
	opcopyformatting (outlinedata, ho); /*9/9/91*/
	
	opsetctexpanded (ho);
	
	(**ho).flbuildundo = false;
	
	return ((*(**outlinedata).setscrapcallback) (ho));
	} /*opsetscrap*/


boolean opgetscrap (hdlheadrecord *hnode, boolean *fltempscrap) {
	
	/*
	7/9/91 dmb: outline scraps are now hdloutlinerecords, not hdlheadrecords
	*/
	
	boolean fl;
	Handle hscrap;
	boolean fltemptext;
	hdloutlinerecord houtline;
	
	/*first see if scrap is, or can be converted to, an outline*/
	
	if ((*(**outlinedata).getscrapcallback) (&houtline, fltempscrap)) {
		
		*hnode = (**houtline).hsummit;
		
		if (*fltempscrap) {
			
			(**houtline).hsummit = nil; /*we've stolen it*/
			
			opdisposeoutline (houtline, false);
			}
		
		return (true); /*scrap manager was able to handle it*/
		}
	
	/*if actual scrap is (or can be converted to) text, convert to an outline*/
	
	if (!shellconvertscrap (textscraptype, &hscrap, &fltemptext))
		return (false);
	
	*fltempscrap = true;
	
	fl = optexttooutline (outlinedata, hscrap, hnode); /*memory leak?*/

	if (fltemptext)
		disposehandle (hscrap);
	
	return (fl);
	} /*opgetscrap*/


static void opmarklevel (hdlheadrecord hnode) {
	
	hnode = op1stsibling (hnode);
	
	while (true) {
		
		opsetmark (hnode, true); /*set the marked bit*/
		
		if (!opchasedown (&hnode))
			break;
		}
	} /*opmarklevel*/


static boolean opfindclickvisit (hdlheadrecord hnode, ptrvoid hclicked) {
	
	/*
	5.0a25 dmb: part of the shift-click operation, just find the 
	clicked node so we know what direction to go in
	*/
	
	return (hnode != (hdlheadrecord) hclicked);
	} /*opfindclickvisit*/


static boolean opancestormarked (hdlheadrecord hnode) {
	
	/*
	return true iff one of my ancestors is marked
	*/
	
	while (opchaseleft (&hnode)) {
		
		if ((**hnode).flmarked)
			return (true);
		}
	
	return (false);
	} /*opancestormarked*/


static boolean opshiftclickvisit (hdlheadrecord hnode, ptrvoid hclicked) {
	
	/*
	5.0a25 dmb: as part of the shift-click operation, we know that
	there aren't already any marked nodes except those we just visited
	
	5.0b1 dmb: we must keep maintain the rule: no children of marked 
	nodes can be marked
	*/
	
	if (!opancestormarked (hnode)) {
	
		opclearmarks (hnode); /*make sure nothing subordinated is marked*/
		
		(**hnode).flmarked = true;
		
		if (++(**outlinedata).ctmarked == 1) // only thing marked now; barcursor was subordinate
			(**outlinedata).hbarcursor = hnode;
		}
	
	return (hnode != (hdlheadrecord) hclicked);
	} /*opshiftclickvisit*/


static void opshiftclick (hdlheadrecord hnode, boolean flmarklevel) {
	
	hdlscreenmap hmap;
	hdlheadrecord hcursor = (**outlinedata).hbarcursor;
	tydirection dir;
	
	opnewscreenmap (&hmap);
	
	if (flmarklevel)
		opmarklevel (hnode);
	else {
		opdisabledisplay ();
				
		opclearallmarks ();
		
		openabledisplay ();
		
		dir = flatdown;
		
		if (opbumpvisit (hcursor, dir, &opfindclickvisit, (ptrvoid) hnode))
			dir = flatup;
		
		opbumpvisit (hcursor, dir, opshiftclickvisit, (ptrvoid) hnode);
		}
	
	opanymarked ();
	
	opinvalscreenmap (hmap); /*inval all the dirty lines*/
	} /*opshiftclick*/


static boolean opcmdclick (hdlheadrecord hnode) {
	
	/*
	5.0.2b8 dmb: try to make 2clicks always leave the node selected, and
	allow zooming
	
	7.0b15 PBS: if cmd-clicking, and the previous headline was in edit mode, ignore the click.
	This fixes a crashing bug, when hbarcursor != heditcursor.
	*/
	
	hdlscreenmap hmap;
	hdlheadrecord hcursor = (**outlinedata).hbarcursor;
	
	opnewscreenmap (&hmap);

	if (((**outlinedata).hbuffer) && (hcursor != hnode)) /*7.0b15 PBS: was edit mode on? Then ignore the cmd key.*/
		goto noclick;
	
	if (!opanymarked ()) {
		
		if (hcursor != hnode)
			opsetmark (hcursor, true); // set the marked bit
		else
			goto noclick; // no change
		}
	
	if (mousedoubleclick ()) { //be sure to leave it selected
		
		(**outlinedata).hbarcursor = hnode;
		
		opsetmark (hnode, true);

		opinvalscreenmap (hmap);
		
		return (false); //  not consumed
		}
	
	opsetmark (hnode, !opgetmark (hnode)); /*toggle the marked bit*/
	
	if (opgetmark (hnode))
		(**outlinedata).hbarcursor = hnode;
	

/*
	if (opgetmark (hnode))
		(**outlinedata).hbarcursor = hnode;
	else {
		if (mousedoubleclick ()) { //leave it selected
			
			opsetmark (hnode, true);

			goto noclick;
			}
		}
*/
	
	opinvalscreenmap (hmap); /*inval all the dirty lines*/
	
	return (true); // made a change

	noclick:
	
		disposehandle ((Handle) hmap);

		return (false);
	} /*opcmdclick*/


void opresize (Rect r) {
	
	if (outlinedata != NULL) {

		#ifdef gray3Dlook
			insetrect (&r, 3, 3);
		#endif
		
		(**outlinedata).outlinerect = r;
		
		opsetdisplaydefaults (outlinedata);
		
		oppostfontchange (); //if in edit mode, adjust display area
		
		#ifndef appletinclude /*applet toolkit will call the reset scroll bar routine after resizing*/
		
			opresetscrollbars (); /*need this since scroll range depends on screenlines*/
		
		#endif
		
		#ifdef fldebug
			opvalidate (outlinedata);
		#endif
		}
	} /*opresize*/
	

/*void opupdate (void) {
	
	opindenteddisplay ();
	} /*opupdate*/
	
	
boolean opdefaultadjustcursor (hdlheadrecord hnode, Point pt, const Rect *textrect) {
	
	/*
	5.0a4 dmb: recoded with slop, more consitency with opdefaultmouseinline
	
	also, we must always shape cursor if we return true, which we want to do
	since we're taking responsibility
	*/
	
	Rect r = *textrect;
	
	r.left -= textleftslop;
	
	if (!opistextmode ()) /*more liberal with ibeams when in text mode*/
		r.right = r.left + opgetlinewidth (hnode) + textrightslop;
	
	if (pointinrect (pt, r))
		setcursortype (cursorisibeam);
	else
		setcursortype (cursorisarrow);
	
	return (true);
	} /*opdefaultadjustcursor*/
	
	
boolean opsetcursor (Point pt) {
	
	/*
	set the mouse cursor according to the structure.  for example, if it is
	over text, use the I-beam.  if it isn't use the standard arrow.
	
	returns true if we set the cursor, false otherwise.
	*/
	
	Rect linerect, textrect;
	hdlheadrecord hnode;
	
	if (!opdisplayenabled ())
		return (true);
	
	hnode = oppointnode (pt);
	
	if (hnode == nil) /*not pointing at a node at all*/
		goto arrow;
		
	if (opcantedittext (hnode))
		goto arrow;
		
	opgetnoderect (hnode, &linerect);
	
	opgettextrect (hnode, &linerect, &textrect);
	
	if ((*(**outlinedata).adjustcursorcallback) (hnode, pt, &textrect))
		return (true);
	
	arrow:
	
	setcursortype (cursorisarrow);
		
	return (true);
	} /*opsetcursor*/


boolean opmousedown (Point pt, tyclickflags flags) {
	
	/*
	8/7/92 dmb: don't exit after expand/collapse; still allow drag
	
	8/28/92 dmb: added multiple selection code
	
	DW 8/30/93: if the cursor moved on this mouse click, don't allow 
	a double-click.
	
	5.0a2 dmb: exit after icon2clickcallback returns true; it may have 
	destroyed our context. anyway, we're done.
	*/
	
	register hdloutlinerecord ho = outlinedata;
	hdlheadrecord origbarcursor = (**ho).hbarcursor;
	boolean fltextmode = (**ho).fltextmode;
	long initialticks = gettickcount (); 
	Rect iconrect, textrect;
	hdlheadrecord hcursor;
	long i;
	Rect r;

#ifdef MACVERSION /*7.0b11 PBS: check for ctrl-clicking on Macs*/

	if (keyboardstatus.ctmodifiers && keyboardstatus.flcontrolkey)

		return (oprmousedown (pt, flags)); /*Call right-click routine.*/

#endif
	
	if (!pointinrect (pt, (**ho).outlinerect))
		return (false);
	
	opdirtyview ();
	
	openabledisplay (); /*make sure no scripts have left us crippled*/
	
	/*determine the line number and rectangle for the mouse click*/ {
	
		long screenlines = opgetcurrentscreenlines (false) + 1;
		
		for (i = 0; i < screenlines; i++) {
			
			opgetlinerect (i, &r);
			
			if (pt.v <= r.bottom) /*we've reached the right line*/
				goto L2;
			} /*for*/
					
		opclearallmarks (); /*not pointing at any line, unmark everything and return*/
		
		return (true);;
		}
	
	L2:
	
	hcursor = oprepeatedbump (flatdown, i, (**ho).hline1, true);
	
	opgeticonrect (hcursor, &r, &iconrect);
	
	if (pointinrect (pt, iconrect)) {

		fltextmode = false; /*switch into structure mode*/
		
		flvisiforiconclick = true; 
		
		goto L1;
		}
	
	if (opcantedittext (hcursor))
		fltextmode = false;
	
	else {
		if (fltextmode || (!keyboardstatus.flshiftkey)) {
			
			opgettextrect (hcursor, &r, &textrect);
			
			if (!(*(**ho).mouseinlinecallback) (hcursor, pt, &textrect, &fltextmode))
				return (true);
			}
		}
		
	L1:
	
	if (fltextmode != (**ho).fltextmode) { /*changed!*/
		
		(**ho).flcursorneedsdisplay = true; /*make sure opmoveto does something*/
		
		(**ho).fltextmode = fltextmode;
		
		opwriteeditbuffer ();
		}
	
	if (fltextmode) {
		
		opclearallmarks ();
		
		opmoveto (hcursor); /*potentially re-position the bar cursor*/ 
		
		opupdatenow (); /*be sure any changes are visible now*/
		
		opeditclick (pt, flags);
		
		if (keyboardstatus.ctmodifiers && mousestatus.fldoubleclick) {
		
			bigstring bs;
			
			if ((*(**ho).doubleclickcallback) ()) {
				
				opeditgetseltext (bs);
				
				langzoomobject (bs);
				
				opsetoutline (ho); /*zoomobject script may have changed it*/
				}
			}
		
		return (true);
		}
	
	if (keyboardstatus.flcmdkey) {
		
		if ((*(**outlinedata).cmdclickcallback) (hcursor))
			return (true);
		
		if (opcmdclick (hcursor))
			goto drag;
		}
	
	if (keyboardstatus.flshiftkey) {
		
		opshiftclick (hcursor, mousestatus.fldoubleclick);
		
		goto drag;
		}
	
	if (keyboardstatus.ctmodifiers == 0)
		if (!opgetmark (hcursor))
			opclearallmarks ();
	
	opmoveto (hcursor); /*potentially re-position the bar cursor*/ 
	
	opupdatenow (); /*be sure any changes are visible now*/
	
	if (mousestatus.fldoubleclick) {
		if (langopruncallbackscripts (idopstruct2clickscript))
			return (true);  /*fuction consumed the click*/

		if ((*(**ho).icon2clickcallback) (hcursor)) /*callback consumed double-click*/
			return (true);
		
		if ((**outlinedata).hbarcursor == origbarcursor) /*cursor didn't move on this mouse click*/
			opexpandtoggle ();
		}
	
	drag:
	
	if ((**ho).flreadonly)
		return (true);
	
	while (mousestilldown ()) { /*wait until it qualifies as a dragging move*/
		
		opupdatenow ();
		
		if (opisdraggingmove (pt, initialticks)) {
			
			setcursortype (cursorfordraggingmove);
			
			opdraggingmove (pt, (**ho).hbarcursor);
			
			break;
			}
		} /*while*/
	
	return (true);
	} /*opmousedown*/


static boolean opcmdmove (tydirection dir) {
	
	if (!opreorgcursor (dir, 1)) {
		
		shellouch ();
		
		return (false);
		}
	
	return (true);
	} /*opcmdmove*/
	
	
boolean opmotionkey (tydirection dir, long units, boolean flextendselection) {
	
	register hdloutlinerecord ho = outlinedata;
	register hdlheadrecord hbarcursor = (**ho).hbarcursor;
	register boolean fltextmode = (**ho).fltextmode;
	hdlheadrecord hnewcursor;
	Point selpt;
	register boolean flmoved = false;
	
	opdirtyview ();
	
	while (--units >= 0) {
			
		if (!opmovecursor (hbarcursor, dir, 1, &hnewcursor)) 
			break;
		
		flmoved = true;
		
		if (flextendselection) {
			
			if (opgetmark (hnewcursor))
				opsetmark (hbarcursor, false);			
			else {			
				opsetmark (hbarcursor, true);
				
				opsetmark (hnewcursor, true);
				}
			
			opinvalnode (hbarcursor); /*leave trail of invals*/
			}
		else
			opclearallmarks ();
		
		hbarcursor = hnewcursor;
		}
	
	if (!flmoved)
		return (false);
	
	if (fltextmode)
		opeditgetselpoint (&selpt);
	
	opmoveto (hnewcursor);
	
	if (fltextmode)
		
		switch (keyboardstatus.keydirection) {
			
			case left:
				opeditsetselection (infinity, infinity);
				
				break;
			
			case right:
				opeditsetselection (0, 0);
				
				break;
			
			case down: case flatdown:
				opeditsetselection (0, 0); //start at first line...
				
				opeditsetselpoint (selpt); //... maintaining horizontal cursor position*/
				
				break;
			
			case up: case flatup:
				opeditsetselection (infinity, infinity); //start at last line
				
				opeditsetselpoint (selpt); //... maintaining horizontal cursor position
				
				break;
			} 
		
	return (true);
	} /*opmotionkey*/


/*
boolean opdefaultreturnkey (tydirection dir) {
	
	boolean flcomment = keyboardstatus.flshiftkey;
	
	return (opinsertheadline (emptystring, dir, flcomment));
	} /*opdefaultreturnkey*/


static void opreturnkey (void) {

	/*
	7.1b7 PBS: run a callback after a headline is created.
	*/
	
	register tydirection dir;
	register hdloutlinerecord ho = outlinedata;
	Handle hstring;
	boolean flcomment = keyboardstatus.flshiftkey;
/*
	boolean flsplit = keyboardstatus.flcmdkey && (**ho).fltextmode;
*/	
	(**ho).fltextmode = true;
	
	dir = right;
	
	if (!opsubheadsexpanded ((**ho).hbarcursor))
		dir = down;
		
	langopruncallbackscripts (idopreturnkeyscript); /* DW 1/19/00, let the opReturnKey callbacks have a chance to hook in */
	
/*	if (flsplit) { /* AR 2/16/00, split the current headline at the cursor position %/
	
		long maxpos, startsel, endsel;
		
		opeditgetmaxpos (&maxpos);
		
		opeditgetselection (&startsel, &endsel);
		
		//set current headline to 0..startsel
				
		if (!loadfromhandletohandle ((**ho).hbuffer, &startsel, 4, false, &hstring)) {
			ouch ();
			return;
			}
		
		opinsertheadline (hstring, dir, flcomment);
		}
	else
*/
		if (!newemptyhandle (&hstring))
			ouch ();
		else
			opinsertheadline (hstring, dir, flcomment);

	langopruncallbackscripts (idopinsertscript); /*7.1b7: callback after a headline is created.*/

	opdocursor (true);
	} /*opreturnkey*/


static boolean openterkey (void) {
	
	optoggletextmode ();
	
	return (true);
	} /*openterkey*/


static boolean opmovetovisit (hdlheadrecord hnode, ptrvoid refcon) {
	
	(**outlinedata).hbarcursor = hnode; /*move cursor to first headline encountered*/
	
	return (false); /*stop visiting*/
	} /*opmovetovisit*/


#ifdef version42orgreater

static boolean oppartialsortedsearch (hdlheadrecord hfirst, bigstring bsname, short seek, hdlheadrecord *hnode) {
	
	/*
	2/5/97 dmb: logic cloned from tableeditsortedsearch
	
	just like opfindhead except that instead of looking for an exact match, 
	we find the first item that begins with the substring bsname.  if nothing 
	matches, return the first item that sorts aphabetically after bsname, or the 
	last item in the list.
	*/
	
	register hdlheadrecord nomad, nextnomad;
	bigstring bskey;
	hdlheadrecord hbest = nil; /*no candidate found*/
	bigstring bsbest;
	short comp;
	boolean flwantexact = false;
	
	if (seek == 0) {
		
		flwantexact = true;
		
		seek = 1; /*next best thing*/
		}
	
	alllower (bsname);
	
	nomad = op1stsibling (hfirst);
	
	while (true) {
		
		opgetheadstring (nomad, bskey);
		
		alllower (bskey);
		
		comp = comparestrings (bskey, bsname);
		
		if (comp == 0) { /*an exact match*/
			
			if (flwantexact) {
				
				*hnode = nomad;
				
				return (true);
				}
			}
		
		else if (comp != -seek) { /*a possible candidate*/
			
			if ((hbest == nil) || (comparestrings (bskey, bsbest) == -seek)) {
				
				copystring (bskey, bsbest);
				
				hbest = nomad;
				}
			}
		
		nextnomad = (**nomad).headlinkdown;
		
		if (nextnomad == nomad)
			break;
		
		nomad = nextnomad;
		} /*while*/
	
	if (hbest == nil) { /*didn't find any item that would follow bsname*/
		
		*hnode = nomad; /*select last name*/
		
		return (false);
		}
	
	*hnode = hbest;
	
	return (true);
	} /*oppartialsortedsearch*/
	

static bigstring bsopselection;

static long timelastkey = 0L;


static boolean opstructuretextkey (byte chkey) {
	
	/*
	add chkey to the typing buffer bstableselect, resetting the buffer 
	depending on elapsed time.  select the table entry that whose name 
	starts with a string equal to or greater than the resulting string.
	the net effect should be much like typing in standard file.
	
	note that statics  are used to retain information between calls.
	
	11/17/92 dmb: use time of event, not time we get here
	*/
	
	register byte c = chkey;
	register long timethiskey;
	hdlheadrecord hnode;
	
	timethiskey = shellevent.when; /*gettickcount ()*/
	
	if ((timethiskey - timelastkey) > (2 * getkeyboardstartrepeattime ()))
		setemptystring (bsopselection);
	
	timelastkey = timethiskey; /*set up for next time*/
	
	pushchar (chkey, bsopselection);
	
	oppartialsortedsearch ((**outlinedata).hbarcursor, bsopselection, 0, &hnode);
	
	opmoveto (hnode);
	
	return (true);
	} /*opstructuretextkey*/


static boolean opstructuretabkey (tydirection dir) {
	
	/*
	4/27/93 dmb: new feature -- tab/shift-tab to go next, prev alphabetically
	*/
	
	hdlheadrecord hcursor = (**outlinedata).hbarcursor;
	bigstring bsname;
	short seek;
	hdlheadrecord hnode;
	
	setemptystring (bsopselection); /*reset*/
	
	opgetheadstring (hcursor, bsname); /*start with selected name*/
	
	if (dir == left) /*go prev, or next*/
		seek = -1;
	else
		seek = +1;
	
	if (oppartialsortedsearch (hcursor, bsname, seek, &hnode))
		opmoveto (hnode);
	else
		shellouch ();
	
	return (true);
	} /*opstructuretabkey*/


#endif

boolean opkeystroke (void) {
	
	/*
	12/30/91 dmb: the backspace key (a.k.a. "Delete") now does an opclear 
	when in structure mode.
	
	6/25/92 dmb: let shift-arrow in text mode always be processed as text key
	
	8/28/92 dmb: added multiple selection code
	
	9/11/92 dmb: when cursoring off of multiselection, start from matching end
	
	7/26/93 DW: support for read-only outlines.
	*/
	
	register hdloutlinerecord ho = outlinedata;
	char chkb = keyboardstatus.chkb;
	boolean fltextmode = (**ho).fltextmode;
	register tydirection dir = keyboardstatus.keydirection;
	long units;
	boolean flextendselection = keyboardstatus.flshiftkey;
	boolean floption = keyboardstatus.floptionkey;
	boolean flpreexpand = false;
	boolean flpostcollapse = false;
	boolean fldidsomething = false;
	
	#ifdef WIN95VERSION
		if (keyboardstatus.flcmdkey) {
			
			if ((keyboardstatus.ctmodifiers == 1) && (dir == up || dir == down))
				return (opscroll (oppositdirection (dir), false, 1));

			floption = true;
			}
	#endif

	opdirtyview ();
	
	openabledisplay (); /*make sure no scripts have left us crippled*/
	
	if ((chkb == chtab) && opprefs.fltabkeyreorg) {
		
		if (keyboardstatus.flshiftkey)
			dir = left;
		else
			dir = right;
		
		#ifdef version42orgreater
		
			#ifdef PIKE
				
				/*PBS 7.0d9: structured tab keys for tables only. All other
				types use the tab key to reorganize headlines.
				What's bad is that sometimes for tables the outlinetype is 0.
				It should be outlineistable.
				This issue needs to be revisited.*/

				if ((**ho).flstructuredtextkeys && 
					((**ho).outlinetype == outlineistable) || ((**ho).outlinetype == 0)) {
	
					return (opstructuretabkey (dir));
					} /*if*/

			#endif

		#endif
		
		if ((**ho).flreadonly)
			return (true);
		
		return (opcmdmove (dir));
		}
	
	if (!fltextmode) {
		
		if (chkb == chbackspace || chkb == chdelete) {
			
			if (opcanteditcursor ())
				return (true);
				
			return (opclear ());
			}
		
		if (dir != nodirection) {
			
			if (keyboardstatus.flshiftkey) /*extending selection*/
				goto L1;
			
			else { /*moving off of selection -- make sure we move from right end*/
				
				if ((dir == up) || (dir == right))
					dir = down;
				else
					dir = up;
				
				opvisitmarked (dir, &opmovetovisit, nil);
				}
			}
		}
	
	if (opanymarked ()) {
		
		opclearallmarks ();
		
		fldidsomething = true;
		}
	
	if (fldidsomething)
		opupdatenow ();
	
	if (chkb == chreturn) {
		
		if ((**ho).flreadonly)
			return (true);
		
		opreturnkey ();
		
		return (true);
		}
	
	if (chkb == chenter) {
		
		if (keyboardstatus.ctmodifiers > 0) { /*at least one modifier key is down*/
			
			shellouch (); /*it wasn't caught by our owner*/
			
			return (true);
			}
		
		if (opcanteditcursor ())
			return (true);
			
		return (openterkey ());
		}
		
	if (keyboardstatus.flkeypad) {
		
		register hdlheadrecord hcursor = (**ho).hbarcursor;
		
		if (chkb == '+') {
			if (langopruncallbackscripts (idopexpandscript))
				return (true);  /*fuction consumed the click*/
			
			return (opexpand (hcursor, 1, true));
			}
			
		if (chkb == '*') {
			if (langopruncallbackscripts (idopexpandscript))
				return (true);  /*fuction consumed the click*/
			
			return (opexpand (hcursor, infinity, false));
			}
			
		if (chkb == '-') {
			if (langopruncallbackscripts (idopcollapsescript))
				return (true);  /*fuction consumed the click*/
			
			return (opcollapse (hcursor));
			}
		} /*keypad expand/collapse keys*/
	
	if (!fltextmode) {
		
		if (isprint (chkb)) { /*switch into text mode before processing*/
			
			#ifdef version42orgreater
			
			if ((**ho).flstructuredtextkeys) {
				
				opstructuretextkey (chkb);
				
				return (true);
				}
			
			#endif
			
			if (opcanteditcursor ())
				return (true);
			
			opsettextmode (true); /*switch into textmode*/
			
			opeditselectall ();
			
			fltextmode = true; /*set local copy*/
			}
		}
	
	if (fltextmode) {
		
		if (arrowkey (chkb) && !(**ho).fllimittextarrows && !flextendselection && !opeditcango (dir)) {
		
			switch (chkb) {
				
				case chuparrow:
					keyboardstatus.keydirection = flatup;
					
					break;
				
				case chdownarrow:
					keyboardstatus.keydirection = flatdown;
					
					break;
				}
			
			goto L1;
			}
		
		return (opeditkey ()); /*pass keystroke to TextEdit*/
		}
	
	L1: /*process keystroke as a structure command*/
	
	if (shellfilterscrollkey (chkb))
		return (true);
	
	dir = keyboardstatus.keydirection;
	
	if (keyboardstatus.flcmdkey)
		units = longinfinity;
	else
		units = 1;
	
	if (keyboardstatus.flshiftkey) { /*selection overrides modes & optionkey*/
		
		switch (dir) {
			
			case up:
			case left:
				dir = up;
				
				break;
			
			case down:
			case right:
				dir = down;
				
				break;
			}
		
		goto motion;
		}
	
	if (floption && keyboardstatus.flcmdkey) { /*check for surface & dive keys*/
		
		switch (dir) {
			
			case down:
			case right:
				flpreexpand = true;
				
				units = 1;
				
				break;
			
			case up:
			case left:
				flpostcollapse = true;
				
				units = 1;
				
				break;
			}
		
		goto motion;
		}
	
	if (!floption) {
	
		if (opprefs.flflatcursorkeys) { /*flat motion is default*/
			
			switch (dir) {
				
				case up:
				case left:
					dir = flatup;
					
					break;
				
				case down:
				case right:
					dir = flatdown;
					
					break;
				}
			}
		else { /*default arrow keys, structure mode*/
			
			if (dir == left)
				dir = flatup;
			
			if (dir == right)
				dir = flatdown;
			}
		}
	
	motion:
	
	if (flpreexpand) {
	
		if (langopruncallbackscripts (idopexpandscript))
			return (true);
			
		opexpand ((**ho).hbarcursor, 1, true);
		}
	
	if (opmotionkey (dir, units, flextendselection)) { /*we were able to move*/
		
		if (flpostcollapse) {
			
			if (langopruncallbackscripts (idopcollapsescript))
				return (true);
			
			opcollapse ((**ho).hbarcursor);
			}
		}
	else {
		
		if (!fldidsomething) /*kinda anti-climatic*/
			shellouch ();
		}
	
	return (true);
	} /*opkeystroke*/


boolean opcmdkeyfilter (char chkb) {
	
	/*
	return false if we consume the cmd-keystroke, true if we want it passed
	on to the menubar.
	*/
	
	if ((**outlinedata).flreadonly)
		return (true);
		
	switch (uppercasechar (chkb)) {
		
		case 'U':
			opcmdmove (up);
			
			return (false); /*cmdkey has been consumed*/
			
		case 'D':
			opcmdmove (down);
			
			return (false); 
			
		case 'L': case chtab:
			opcmdmove (left);
			
			return (false); 
			
		case 'R': case chbacktab:
			opcmdmove (right);
			
			return (false); 
		
		case ',':
			opexpandtoggle ();
			
			return (false);
			
		} /*switch*/
		
	return (true); /*we don't consume the keystroke*/
	} /*opcmdkeyfilter*/


boolean opselectall (void) {
	
	if (opistextmode ()) { 
		
		opsettextmode (true);
		
		opeditselectall ();
		}
	else {
		hdlscreenmap hmap;
		
		opnewscreenmap (&hmap);
		
		opmarklevel ((**outlinedata).hbarcursor);
		
		opinvalscreenmap (hmap);
		}
	
	return (true);
	} /*opselectall*/


void opgetcursorinfo (long *row, short *col) {
	
	/*
	translate the cursor position into a set of numbers.
	*/
	
	opgetscreenline ((**outlinedata).hbarcursor, row);
	
	*col = 0; /*no info for this yet*/
	} /*opgetcursorinfo*/
	
	
void opsetcursorinfo (long row, short col) {
	
	/*
	translate a row and column position into the outline's internal data
	structure.
	*/
	
	(**outlinedata).hbarcursor = 
		
		oprepeatedbump (flatdown, row, (**outlinedata).hsummit, true);
	} /*opsetcursorinfo*/
 	
 
boolean opcloseoutline (void) {
	
	/*
	the outline in outlinedata is going into dormancy -- its window is closing,
	but we have to hold on to it, probably because it's dirty.

	save off all window-dependent structures.
	*/

	opsaveeditbuffer ();
	
	return (true);
	} /*opcloseoutline*/
	
	
boolean opopenoutline (void) {
	
	/*
	the outline in outlinedata is coming out of dormancy.  undoes what 
	opcloseoutline does.
	*/
	
	oprestoreeditbuffer ();
	
	return (true);
	} /*opopenoutline*/
	

static void opcheckreopen (void) {
	
	/*
	check to see if outline has been closed and re-opened.  now that its window
	is allocated, it's been displayed, it's safe to show the window-dependent
	stuff.
	*/
	
	if ((**outlinedata).flreopenpending) {
		
		(**outlinedata).flreopenpending = false; /*must be reset every time*/
		
		opopenoutline ();
		}
	} /*opcheckreopen*/


void opidle (void) {
		
	opinvaldirtynodes ();
	
	opcheckvisi ();
	
	if ((**outlinedata).fltextmode) 
		opeditidle ();
	} /*opidle*/


void opactivate (boolean flactivate) {
	
	opcheckreopen ();
	
	(**outlinedata).flactive = flactivate;
	
	opresetscrollbars ();
	
	if ((**outlinedata).fltextmode)
		opeditactivate (flactivate);
	else {
		if (opanymarked ())
			opsmashdisplay ();
		else
			opdocursor (true);
		}
	
	if (flactivate)
		opcheckvisi ();
	} /*opactivate*/


