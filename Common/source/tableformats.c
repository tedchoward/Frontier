
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

#include "bitmaps.h"
#include "cursor.h"
#include "font.h"
#include "kb.h"
#include "memory.h"
#include "ops.h"
#include "quickdraw.h"
#include "resources.h"
#include "smallicon.h"
#include "opinternal.h"
#include "tabledisplay.h"
#include "tableformats.h"
#include "tableinternal.h"
#include "claybrowserstruc.h"
#include "claybrowserexpand.h"
#include "claycallbacks.h"
#include "claybrowservalidate.h"
#include "timedate.h"


#define mincolwidth 50
#define maxcolwidth 1000

#define ctformatstack 5 /*we can remember format contexts up to 5 levels deep*/

short topformatstack = 0;

hdltableformats formatstack [ctformatstack];



void tablegettitlestring (short col, bigstring bstitle) {
	
	if ((col >= namecolumn) && (col <= kindcolumn))
		tablegetstringlist (nametitlestring + col, bstitle);
	else
		tablegetstringlist (questionmarksstring, bstitle);
	} /*tablegettitlestring*/


boolean tablepushformats (hdltableformats hformats) {
	
	/*
	when you want to temporarily work with a different formats record, call this
	routine, do your stuff and then call tablepopformats.
	*/
	
	if (topformatstack >= ctformatstack) {
		
		shellinternalerror (idformatstackfull, "\16" "format stack overflow!");
		
		return (false);
		}
	
	formatstack [topformatstack++] = tableformatsdata;
	
	tableformatsdata = hformats;
	
	oppushoutline ((**hformats).houtline);
	
	return (true);
	} /*tablepushformats*/
		

boolean tablepopformats (void) {
	
	if (topformatstack <= 0) {
		
		shellinternalerror (idformatstackempty, "\x13" "too many popformats");
		
		return (false);
		}
	
	oppopoutline ();
	
	tableformatsdata = formatstack [--topformatstack];
	
	return (true);
	} /*tablepopformats*/


void tabledirty (void) {
	
	/*
	2.1b4 dmb: update timelastsave on each change
	*/
	
	register hdltableformats hf = tableformatsdata;
	register hdlhashtable ht = (hdlhashtable) (**hf).htable;
	hdloutlinerecord ho = (**hf).houtline;
	
	// (**ht).fldirty = true; // 2/14/97 dmb: langhash takes care of dirtyness itself
	
	(**ho).timelastsave = timenow (); /*modification time until saved*/
	
	windowsetchanges (tableformatswindow, true);
	} /*tabledirty*/


short tablegetcolwidth (short col) {
	
	return ((**tableformatsdata).colwidths [col]);
	} /*tablegetcolwidth*/


static short tablegetmaxwidth (short col) {
	
	return ((**tableformatsdata).maxwidths [col]);
	} /*tablegetmaxwidth*/


static short tablelimitcolwidth (short width) {
	
	register short w = width;
	
	/*
	if odd (w)
		w++;
	*/
	
	w = max (mincolwidth, w);
	
	w = min (maxcolwidth, w);
	
	return (w);
	} /*tablelimitcolwidth*/


boolean tablesetcolwidth (short col, short width, boolean fllimit) {

	if ((col < 0) || (col >= (**tableformatsdata).ctcols)) /*out of range*/
		return (false);
	
	if (fllimit)
		width = tablelimitcolwidth (width);
	
	(**tableformatsdata).colwidths [col] = width;
	
	return (true);
	} /*tablesetcolwidth*/
	

short tablesumcolwidths (short startcol, short ctcols) {
	
	register short *pwidth;
	short sum = 0;
	hdltableformats hf = tableformatsdata;
	
	/*loop through range in width array, adding values*/
	
	for (pwidth = (**hf).colwidths + startcol;  --ctcols >= 0;  ++pwidth)
		sum += *pwidth;
	
	return (sum);
	} /*tablesumcolwidths*/


short tableavailwidth (void) {
	
	register hdltableformats hc = tableformatsdata;
	register Rect *r = &(**hc).tablerect;
	register short availwidth;
	
	availwidth = (*r).right - (*r).left;
	
	return (availwidth);
	} /*tableavailwidth*/


short tabletotalcolwidths (void) {
	
	return (tablesumcolwidths (0, (**tableformatsdata).ctcols));
	} /*tabletotalcolwidths*/


boolean tablerecalccolwidths (boolean flmustrecalc) {
	
	/*
	the width of the table rectangle has changed.  reallocate the column widths
	so that they are optimal for our 3-column database tables.
	
	9/26/90 dmb: always set the width of the last column to the remaining table width
	
	6/21/92 dmb: new, object database-specific (3-column) algorithm:
	
	1.	when widening the table, never narrow a column; when narrowing a table, 
		never widen a column
	
	2.	expand columns 1 & 3 enough to accommodate their contents; column 2 gets the rest
	
	3.	if there's extra width, divide it among the three evenly
	
	4.	if there's too little width, column 2 is truncated, down to the minimum limit; 
		if there's still too little, columns 3 & 1 absorb it (in that order).
	
	10/3/92 dmb: if the colwidths already fit the display, do nothing. (currently, no 
	caller would insist on the recalc, and this preserves hand-tuning.)
	
	10/6/92 dmb: added flmustrecalc parameter. (tableresize really wants to ensure recalc; 
	it's used after a font/size change as well as resizing.)
	
	5.0d19 dmb: change rule #3 above. If there's extra width, give it to the name column.

	5.0a5 dmb: another tweak
	*/
	
	register hdltableformats hc = tableformatsdata;
	hdloutlinerecord ho = outlinedata;
	long curwidth, newwidth;
	short i;
	short lastcol = (**hc).ctcols - 1;
	short remaining;
	short diff;
	short extra;
	short norm;
	long maxwidth, height;
	
	if (isclaydisplay (hc))
		claybrowserinitdraw ();
	
	assert (lastcol == 2);
	
	newwidth = tableavailwidth ();
	
	curwidth = tabletotalcolwidths ();
	
	diff = newwidth - curwidth; // is table bigger than last time? smaller?*/
	
	if ((diff == 0) && !flmustrecalc) // nothing to do
		return (true);
	
	tablegetoutlinesize (&maxwidth, &height); // recalcs max col widths
	
	extra = newwidth - maxwidth;
	
	remaining = newwidth;
	
	for (i = 0; i < lastcol; i++) {
		
		register long w; /*do math in longs, avoid overflow*/
		register long x;
		
		w = tablegetcolwidth (i);
		
		x = tablegetmaxwidth (i);
		
		extra = remaining - maxwidth;
		
		if (curwidth == 0)
			norm = x + extra / (lastcol - i);
		else
			norm = x + extra;
		
		switch (sgn (diff)) {
			
			case 1: /*growing the table*/
				w = max (w, min (w + diff, max (x, norm)));
				
				break;
			
			case -1: /*shrinking the table*/
				w = min (w, max (w + diff, min (x, norm)));
				
				break;
			
			default:
				w = norm;
			}
		
		w = min (w, remaining - ((lastcol - i) * mincolwidth));
		
		w = tablelimitcolwidth ((short) w);
		
		tablesetcolwidth (i, (short) w, false);
		
		remaining -= w;
		
		maxwidth -= x;
		}
	
	tablesetcolwidth (lastcol, remaining, false);
	
	opseteditbufferrect ();
	
	return (true);
	} /*tablerecalccolwidths*/


static boolean tablepostfontchange (void) {
	
	/*
	5.0a10 dmb: must dirty table to make sure changes stick
	*/

	hdltableformats hf = tableformatsdata;
	hdlhashtable ht = tablegetlinkedhashtable ();
	
	if (hf == nil)
		return (false);
	
	oppostfontchange ();
	
	if (ht != nil)
		(**ht).fldirty = true;
	
	//shelladjustaftergrow (tableformatswindow); /*recalc and redraw the entire display*/
	
	if (isclaydisplay (hf))
		claybrowserinitdraw ();
	
	return (true);
	} /*claypostfontchange*/


static boolean tableicon2click (hdlheadrecord hnode) {

	if ((**hnode).flnodeisfolder && !keyboardstatus.flcmdkey)
		return (false);
	
	return (tablezoomfromhead (hnode));
	} /*tableicon2click*/


static void setuptableoutlinecallbacks (hdloutlinerecord ho) {
	
	(**ho).fldonterasebackground = true;
	
	(**ho).flusebitmaps = false; 
	
	(**ho).pixelsaftericon = 3; /*skip this many pixels between icon and text*/
	
	(**ho).maxlinelen = 31; /*file names can be at most 31 characters long*/
	
	(**ho).fllimittextarrows = true;
	
	(**ho).flstructuredtextkeys = true;
	
	(**ho).flhorizscrolldisabled = true;
	
	/*link in the line layout callbacks*/ {
	
		(**ho).drawlinecallback = tabledrawline;
		
		(**ho).postdrawlinecallback = tablepostdrawline;
		
		(**ho).predrawlinecallback = tablepredrawline;
		
		(**ho).drawiconcallback = tabledrawnodeicon;
		
		(**ho).gettextrectcallback = tablegettextrect;
		
		(**ho).getedittextrectcallback = tablegetedittextrect;
		
		(**ho).geticonrectcallback = tablegeticonrect;
		
		(**ho).getlineheightcallback = tablegetlineheight;
		
		(**ho).getlinewidthcallback = tablegetlinewidth;
		
	//	(**ho).getmaxlinewidthcallback = tablegetmaxlinewidth;
		
		(**ho).pushstylecallback = tablepushnodestyle;
		
		(**ho).postfontchangecallback = tablepostfontchange;
		
		(**ho).getfullrectcallback = (opgefullrectcallback) truenoop;
	
		}
	
		/*link in the line layout callbacks/ {
	
		(**ho).drawlinecallback = &claydrawline;
		
		(**ho).postdrawlinecallback = &claypostdrawline;
		
		(**ho).predrawlinecallback = &claypredrawline;
		
		(**ho).drawiconcallback = &claydrawnodeicon;
		
		(**ho).gettextrectcallback = &claygettextrect;
		
		(**ho).geticonrectcallback = &claygeticonrect;
		
		(**ho).getlineheightcallback = &claygetlineheight;
		
		(**ho).getlinewidthcallback = &claygetlinewidth;
		
		(**ho).pushstylecallback = &claypushnodestyle;
		
		(**ho).postfontchangecallback = (opvoidcallback) &claybrowserinitdraw;
		
		(**ho).getfullrectcallback = (opgefullrectcallback) &claygetnodeframe;
		}*/

	(**ho).preexpandcallback = browserpreexpand;
	
	(**ho).postcollapsecallback = browserpostcollapse;
	
	(**ho).hasdynamicsubscallback = browsercanexpand;
	
	(**ho).textchangedcallback = browsertextchanged;
	
	(**ho).insertlinecallback = browserlineinserted;
	
	(**ho).deletelinecallback = browserlinedeleted;
	
	(**ho).copyrefconcallback = browsercopyrefcon;
	
	(**ho).textualizerefconcallback = browsertextualizerefcon;
	
	(**ho).releaserefconcallback = browserreleaserefcon;
	
	(**ho).setwpedittextcallback = tablesetwpedittext; /*edits the node's headstring*/
	
	(**ho).getwpedittextcallback = tablegetwpedittext; /*updates the node's headstring*/
	
	(**ho).validatedragcallback = browservalidatedrag;
	
	(**ho).predragcallback = browserpredrag;
	
	(**ho).dragcopycallback = browserdragcopy;
	
	(**ho).mouseinlinecallback = tablemouseinline;
	
	(**ho).adjustcursorcallback = tableadjustcursor;
	
	(**ho).icon2clickcallback = tableicon2click;
	
	(**ho).validatepastecallback = browservalidatepaste;
	
//	(**ho).postpastecallback = browserpostpaste;
	
	(**ho).validatecopycallback = browservalidatecopy;
	
//	(**ho).caneditcallback = browsernodeislocked;
	
//	(**ho).returnkeycallback = tablereturnkey;
	
	(**ho).setscrapcallback = browsersetscrap;

	(**ho).getscrapcallback = browsergetscrap;
	
	(**ho).beforeprintpagecallback = tablebeforeprintpage;

	(**ho).afterprintpagecallback = tableafterprintpage;
	} /*setuptableoutlinecallbacks*/


static boolean duplicateexpansion (hdlheadrecord, hdlheadrecord); // forward

static boolean duplicateexpansionvisit (hdlheadrecord hnode, hdlheadrecord hfirstnew) {
	
	if (opsubheadsexpanded (hnode)) {
		
		bigstring bs;
		hdlheadrecord hmatch;
		
		opgetheadstring (hnode, bs);
		
		if (opfindhead (hfirstnew, bs, &hmatch))
			duplicateexpansion (hnode, hmatch);
		}
	
	return (true);
	} /*duplicateexpansionvisit*/


static boolean duplicateexpansion (hdlheadrecord horig, hdlheadrecord hnew) {
	
	opexpand (hnew, 1, true);
	
	return (oplistvisit ((**horig).headlinkright, duplicateexpansionvisit, (**hnew).headlinkright));
	} /*duplicateexpansion*/


static boolean initializetableoutline (hdloutlinerecord ho, hdlhashtable ht) {
	
	/*
	5.0.2b21 dmb: use shellgetdatabase, not databasedata global
	
	5.1.3 dmb: roll beachball; don't ignore clayfolderloop return value
	*/

	hdlheadrecord hsummit = (**ho).hsummit;
	hdlheadrecord hnewsummit, hlastsummit;
	hdlheadrecord nomad, hnext;
	tybrowserspec fsroot;
	tyexpandinfo expandinfo;
	hdlwindowinfo hinfo;
	long lnumcursor;
	boolean fl = false;
	
	oppushoutline (ho);
	
	opgetnodeline ((**ho).hbarcursor, &lnumcursor);
	
	hlastsummit = hsummit;
	
	while (opchasedown (&hlastsummit))
		;
	
	opdisabledisplay ();
	
	opsetdisplaydefaults (ho);
	
	expandinfo.hparent = nil; /*special case, see browserfindinsertionpoint*/
	
	expandinfo.ctlevels = 1;
	
	expandinfo.flsortnodes = false;
	
	expandinfo.flsettmpbits = false;
	
	if (shellfinddatawindow ((Handle) (**ho).outlinerefcon, &hinfo))
		shellgetdatabase ((**hinfo).macwindow, &fsroot.vRefNum);
	else
		fsroot.vRefNum = databasedata;
	
	fsroot.parID = ht;
	
	setemptystring (fsroot.name);
	
	initbeachball (right);
	
	if (!clayfolderloop (&fsroot, false, browserexpandvisit, (long) &expandinfo))
		goto exit;
	
	hnewsummit = (**hlastsummit).headlinkdown;
	
	if (hnewsummit == hlastsummit) { // table was empty
		
		browserlineinserted (hnewsummit);
		
		(**ho).fltextmode = true;
		
		opeditsetselection (0, infinity);
		}
	else { // table was not empty
	
		opstartinternalchange (); 
		
		nomad = hsummit;
		
		hnewsummit = (**hlastsummit).headlinkdown;
		
		while (true) {
			
			hnext = (**nomad).headlinkdown;
			
			duplicateexpansionvisit (nomad, hnewsummit);
			
			opunlink (nomad);
			
			opdisposestructure (nomad, false);
			
			if (hnext == hnewsummit)
				break;
			
			nomad = hnext;
			}
		
		(**ho).hbarcursor = oprepeatedbump (flatdown, lnumcursor, hnewsummit, true);
		
		(**ho).hline1 = oprepeatedbump (flatdown, (**ho).vertscrollinfo.cur, hnewsummit, true);
		
		opendinternalchange ();
		
		opdirtymeasurements (); //6.0a14 dmb
		
		opsetctexpanded (ho);
		}
	
	#ifdef fldebug
		opvalidate (outlinedata);
	#endif
	
	fl = true;
	
exit:
	openabledisplay ();
	
	oppopoutline ();
	
//	(**(**ho).hsummit).flnodeisfolder = true;
	
	return (fl);
	} /*initializetableoutline*/


static boolean validatetableoutline (hdloutlinerecord ho, hdltableformats hf, hdlhashtable ht) {
	
	// 5.0d18 dmb: must link table & outline now, so globals can be
	// maintained as outline is initialized

	(**hf).houtline = ho;	// point to outline
	
	(**ho).outlinerefcon = (long) hf; // pointing is mutual
	
	return (initializetableoutline (ho, ht));
	} /*validatetableoutline*/


boolean newtableformats (hdltableformats *hformats) {
	
	/*
	allocate a new tableformats record, and set the fields to 
	their default values.
	
	4.0b7 dmb: initialize timelastsave to creation time; it's really a modification date.
	
	5.0d1 dmb: rewrote. table windows are outlines now. don't allocate
	the outline here, however.
	*/
	
	register hdltableformats hf;
	//Rect r;
	
	if (!newclearhandle (sizeof (tytableformats), (Handle *) hformats))
		return (false);
	
	hf = *hformats; /*copy into register*/
	
	(**hf).ctcols = 3;
	
	//(**hf).defaultcolwidth = (r.right - r.left) / fixedctcols; /*arrgh: slight bias to 3-col tables*/
	
	return (true);
	} /*newtableformats*/


void disposetableformats (hdltableformats hformats) {
	
	if (hformats != nil)
		opdisposeoutline ((**hformats).houtline, false);
	
	disposehandle ((Handle) hformats);
	} /*disposetableformats*/


static boolean tablenewoutlinerecord (hdltableformats hf, hdlhashtable ht) {
	
	/*
	5.1.3 dmb: handle errors
	*/
	
	hdloutlinerecord ho;
	
	if (!newoutlinerecord (&ho)) {
		
		disposehandle ((Handle) hf);
		
		return (false);
		}
	
	(**ho).outlinetype = outlineistable;

	(**ho).fontnum = (**hf).fontnum;
	
	(**ho).fontsize = (**hf).fontsize;
	
	(**ho).fontstyle = (**hf).fontstyle;
	
	// 6.11.97 dmb: must link table & outline now, so globals can be
	// maintained as outline is initialized

	(**hf).houtline = ho;	// xxx - point to outline now that's it's been initialized
	
	(**ho).outlinerefcon = (long) hf; // pointing is mutual
	
	if (!initializetableoutline (ho, ht)) {
		
		opdisposeoutline (ho, false);
		
		disposehandle ((Handle) hf);
		
		return (false);
		}
	
	setuptableoutlinecallbacks (ho);
	
	return (true);
	} /*tablenewoutlinerecord*/


boolean tableprepareoutline (hdltableformats hf) {
	
	hdlhashtable ht = (**hf).htable;
	hdloutlinerecord ho = (**hf).houtline;

	if (ho == nil)
		return (tablenewoutlinerecord (hf, ht));
	else
		return (validatetableoutline (ho, hf, ht));
	} /*tableprepareoutline*/


void tabledisposeoutline (hdltableformats hf) {

	hdlhashtable ht = (**hf).htable;
	hdloutlinerecord ho = (**hf).houtline;

	if (ho == nil)
		return;
	
	// grab display setting from outline
	(**hf).fontnum = (**ho).fontnum;
	
	(**hf).fontsize = (**ho).fontsize;
	
	(**hf).fontstyle = (**ho).fontstyle;
	
	(**hf).vertcurrent = (**ho).vertscrollinfo.cur;
	
	opgetnodeline ((**ho).hbarcursor, &(**hf).vertcurrent);

	// if we have an auto-generated node, delete the table entry
	
	/*
	hdlheadrecord nomad;
	long ctitems;
	nomad = (**ho).hsummit;
	
	if ((**nomad).tmpbit2) {
	
		hashcountitems (ht, &ctitems);
		
		if ((ctitems == 1) && (**(**ht).hfirstsort).val.valuetype == novaluetype)
			emptyhashtable (ht, true);
		}
	*/
	
	(**ho).flinternalchange++; // we shouldn't be connected to a window, but just in case...
	
	browserdeletedummyvalues ((**ho).hsummit);
	
	// unlink & dispose
//	if (ho == outlinedata)
//		outlinedata = nil;
	
	(**hf).houtline = nil;
	
	opdisposeoutline (ho, false);
	} /*tabledisposeoutline*/


boolean tablenewformatsrecord (hdlhashtable ht, Rect tablerect, hdltableformats *hformats) {
	
	/*
	allocate a new tableformats and set default and/or initial values of fields of
	the record.
	*/
	 
	register hdltableformats hf = nil;
	tyconfigrecord config;
	
	if (!newtableformats (hformats))
		return (false);
	
	hf = *hformats; /*copy into register*/
	
	langexternalgetconfig (tablevaluetype, idtableconfig, &config);
	
	(**hf).windowrect = config.defaultwindowrect;
	
	(**hf).fontnum = config.defaultfont;
	
	(**hf).fontsize = config.defaultsize;
	
	(**hf).fontstyle = config.defaultstyle;
	
	tableformatsdata = hf; /*set global for displayinfo routine*/
	
	tablelinkformats (ht, hf); /*link formats record and hashtable*/
	
	/*
	if (!tablenewoutlinerecord (hf, ht)) {
		
		disposetableformats (hf);
		
		return (false);
		}
	
	/*
	tableresetrects (tablerect);
	
	x = (tablerect.right - tablerect.left) / ctcols; /*initial column width*/
	
	/*
	x = (**hf).defaultcolwidth = tablelimitcolwidth (x);
	
	for (i = 0; i < ctcols; i++) 
		(**hf).colwidths [i] = x;
	*/
	
	//if (isclaydisplay (hf))
	//	claybrowserinitdraw (); //sets the computed fields of (**hf).linelayout
	
	return (true);
	} /*tablenewformatsrecord*/


static boolean notexpandedvisit (hdlheadrecord hnode, ptrvoid refcon)  {
	
	return (!opsubheadsexpanded (hnode));
	} /*notexpandedvisit*/


boolean tableoutlineneedssaving (void) {
	
	if (outlinedata == nil)
		return (false);
	
	return (!opsummitvisit (notexpandedvisit, nil)); // something expanded
	} /*tableoutlineneedssaving*/


boolean tablepackformats (Handle *hpacked) {
	
	/*
	pack the data from the current tableformatsdata record into a compact form, suitable
	for saving to disk.  we only record the formatting options, it is up to the 
	other guy to save the data that's being displayed in the table.

	save font/size/style settings when we're not saving an outline
	*/
	
	register hdltableformats hf = tableformatsdata;
	hdloutlinerecord ho = outlinedata;
	tyversion2tablediskrecord info;
	int i;
	long vertcurrent, lnumcursor;
	
	clearbytes (&info, sizeof (info));
	
	info.versionnumber = conditionalshortswap (0x0010);
	
	info.recordsize = conditionalshortswap (sizeof (tyversion2tablediskrecord));
	
	info.ctcols = conditionalshortswap ((**hf).ctcols);
	
	info.colcursor = conditionalshortswap ((**hf).editcol);
	
	info.savedoutline = tableoutlineneedssaving ();
	
	info.savedlinelayout = (**hf).linelayout.flinitted;
	
/*	moveleft ((**hf).colwidths, info.colwidths, maxtablecols * sizeof (short)); */

	for (i = 0; i < maxtablecols; i++) {
		info.colwidths[i] = conditionalshortswap ((**hf).colwidths[i]);
		}
	
	recttodiskrect (&(**hf).windowrect, &info.windowrect);
	
	if ((ho != nil) && !info.savedoutline) {
		
		diskgetfontname ((**ho).fontnum, info.fontname);
		
		info.fontsize = conditionalshortswap ((**ho).fontsize);
		
		info.fontstyle = conditionalshortswap ((**ho).fontstyle);
		
		vertcurrent = (**ho).vertscrollinfo.cur;
		
		memlongtodiskwords (vertcurrent, info.vertcurrent, info.vertcurrent_hiword);
		
		opgetnodeline ((**ho).hbarcursor, &lnumcursor);
		
		memlongtodiskwords (lnumcursor, info.lnumcursor, info.lnumcursor_hiword);
		}
	else {
		
		diskgetfontname ((**hf).fontnum, info.fontname);
		
		info.fontsize = conditionalshortswap ((**hf).fontsize);
		
		info.fontstyle = conditionalshortswap ((**hf).fontstyle);
		}

	if (!newfilledhandle (&info, sizeof (info), hpacked))
		return (false);
	
	if (info.savedoutline) {
		
		if (!oppack (hpacked))
			goto error;
		}
	
	if (info.savedlinelayout) {
		
		if (!claypacklinelayout (*hpacked))
			goto error;
		}
	
	return (true);
	
	error:
		disposehandle (*hpacked);
		
		*hpacked = nil;
	
		return (false);
	} /*tablepackformats*/


static boolean tableunpackversion1formats (Handle hpacked, hdltableformats hformats) {
	
	/*
	5/7/93: added flsavedsorted and introduce new version logic: anything from 
	1 to 10 is cool. in the future, to _prevent_ backward compatibility, bump it 
	to 11 or higher
	*/
	
	register hdltableformats hf = hformats;
	long ixhandle = 0;
	tyversion1tablediskrecord info;
	short fontnum;
	hdlhashtable ht = (**hf).htable;
	
	if (!loadfromhandle (hpacked, &ixhandle, sizeof (info), &info)) {
		
		shellinternalerror (iderrorloadingformats, "\x2b" "error loading table formatting information.");
		
		return (false);
		}
	
	(**hf).editcol = conditionalshortswap (info.colcursor);
	
	(**hf).ctcols = max (conditionalshortswap (info.ctcols), (**hf).ctcols);
	
	(**ht).sortorder = conditionalshortswap (info.sortorder);
	
	(**ht).timecreated = conditionallongswap (info.timecreated);
	
	(**ht).timelastsave = conditionallongswap (info.timelastsave);
	
//	if (!info.flsavedsorted) {
//		
//		(**hf).fldirty = true; /*old table, wasn't saved sorted*/
		
//		if (info.sortorder != sortbyname)
//			(**hf).flneedresort = true;
//		}
	
	diskrecttorect (&info.windowrect, &(**hf).windowrect);
	
	disktomemlong (info.sizerowarray);
	disktomemlong (info.sizecolarray);

	ixhandle += info.sizerowarray;
	
	if (info.sizecolarray <= maxtablecols * sizeof (short))
		loadfromhandle (hpacked, &ixhandle, info.sizecolarray, (**hf).colwidths);
	
	diskgetfontnum (info.fontname, &fontnum);
	
	(**hf).fontnum = fontnum;
	
	(**hf).fontsize = conditionalshortswap (info.fontsize);
	
	(**hf).fontstyle = conditionalshortswap (info.fontstyle);
	
	(**hf).vertcurrent = conditionalshortswap (info.vertcurrent);
	
	(**hf).lnumcursor = conditionalshortswap (info.rowcursor);
	
	/*
	(**hf).linespacing = info.linespacing;
	
	(**hf).ctsaves = info.ctsaves;
	*/
	
//	tablepushformats (hf);
//	
//	tablelimitformats (); /*check the colwidths and rowheights arrays*/
//	
//	tablepopformats ();
	
	return (true);
	} /*tableunpackversion1formats*/


boolean tableunpackformats (Handle hpacked, hdltableformats hformats) {
	
	/*
	unpack the table formats from the handle.  we take care not to reduce the
	size of the table using these formats, rather add the information to the 
	set of info we have about the table.
	
	for example, if we only have column widths for two of four columns, we apply
	the information to the two we have, and leave the other two alone.  earlier
	versions would have reduced the table size, effectively throwing away 
	columns.  this makes it easy for tables to grow, either according to the
	wishes of a user, or a using program.
	
	10/16/91 dmb: added paranoia check to ensure that row and column arrays 
	are big enough for the current table size.  an old database bug would 
	lead to this not being the case; better safe than sorry!
	
	5/7/93: added flsavedsorted and introduce new version logic: anything from 
	1 to 10 is cool. in the future, to _prevent_ backward compatibility, bump it 
	to 11 or higher
	*/
	
	register hdltableformats hf = hformats;
	hdloutlinerecord ho;
	long ixhandle = 0;
	tyversion2tablediskrecord info;
	short fontnum;
	boolean fl;
	int i;
	#define sizepreload (sizeof (info.versionnumber) + sizeof (info.recordsize))
	
	if (hpacked == nil) /*defensive driving, return true, default formats*/
		return (true);
	
	// load first field - the record size
	if (!loadfromhandle (hpacked, &ixhandle, sizepreload, &info))
		goto loaderror;
	
	disktomemshort (info.versionnumber);

	disktomemshort (info.recordsize);
	
	//validate version
	if (info.versionnumber == 1)
		return (tableunpackversion1formats (hpacked, hf));
	
	if ((info.versionnumber < 1) || (info.versionnumber > 0x001f)) { /*not in handleable range*/
		
		shellinternalerror (idbadtableformatsversion, "\x21" "bad table formats version number.");
		
		return (false);
		}
	
	// load remaining data in record
	if (!loadfromhandle (hpacked, &ixhandle, min (sizeof (info), info.recordsize) - sizepreload, &info.fontname))
		goto loaderror;
	
	//load table outline
	if (info.savedoutline) {
		
		tyconfigrecord config;
		
		langexternalgetconfig (tablevaluetype, idtableconfig, &config); // force loading of prefs
	
		fl = opunpack (hpacked, &ixhandle, &ho);
		
		if (!fl)
			return (false);
		
		setuptableoutlinecallbacks (ho);
		
	// can't do this here:	validatetableoutline (ho, (**hf).htable);
		
		(**hf).houtline = ho;
		}
	else {
	
		diskgetfontnum (info.fontname, &fontnum);
		
		if (isemptystring (info.fontname)) //it was munged in beta
			(**hf).fontnum = config.defaultfont;
		else
			(**hf).fontnum = fontnum;
		
		if (info.fontsize == 0) //it was munged in beta
			(**hf).fontsize = config.defaultsize;
		else
			(**hf).fontsize = conditionalshortswap (info.fontsize);
		
		(**hf).fontstyle = conditionalshortswap (info.fontstyle);
		
		(**hf).vertcurrent = diskwordstomemlong (info.vertcurrent, info.vertcurrent_hiword);
		
		(**hf).lnumcursor = diskwordstomemlong (info.lnumcursor, info.lnumcursor_hiword);
		}
	
	(**hf).editcol = conditionalshortswap (info.colcursor);
	
	(**hf).ctcols = conditionalshortswap (info.ctcols);
	
	#ifdef fldebug
		if ((**hf).ctcols != 3) {
		//	assert ((**hf).ctcols == 3); // report it
			(**hf).ctcols = 3;			 // fix it
			}
	#endif
	
	/* moveleft (info.colwidths, (**hf).colwidths, maxtablecols * sizeof (short));  */

	for (i = 0; i < maxtablecols; i++) {
		(**hf).colwidths[i] = conditionalshortswap (info.colwidths[i]);
		}
	
	diskrecttorect (&info.windowrect, &(**hf).windowrect);
	
	if (info.savedlinelayout) {
		
		if (!clayunpacklinelayout (hpacked, &ixhandle, hf))
			return (false);
		}
	
//	tablepushformats (hf);
//	
//	tablelimitformats (); /*check the colwidths and rowheights arrays*/
//	
//	tablepopformats ();
	
	return (true);
	
	loaderror:
		
		shellinternalerror (iderrorloadingformats, "\x2b" "error loading table formatting information.");
		
		return (false);
	} /*tableunpackformats*/


