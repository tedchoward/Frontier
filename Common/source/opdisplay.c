
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
#include "font.h"
#include "quickdraw.h"
#include "bitmaps.h"
#include "kb.h"
#include "smallicon.h"
#include "strings.h"
#include "frontierwindows.h"
#include "ops.h"
#include "textdisplay.h"
#include "op.h"
#include "opinternal.h"
#include "oplineheight.h"
#include "opicons.h"
#include "opdisplay.h"
#include "shellprint.h"
#include "wpengine.h"


#define flstructurelines true


static RGBColor invertedbackcolor, invertedtextcolor;




boolean opinitdisplayvariables (void) {
	
	invertedbackcolor = blackcolor;
	
	invertedtextcolor = whitecolor;
	
	return (true);
	} /*opinitdisplayvariables*/


short opnodeindent (hdlheadrecord hnode) {
	
	register hdloutlinerecord ho = outlinedata;
	
	return (((**ho).lineindent * (**hnode).headlevel) - (**ho).horizscrollinfo.cur);
	} /*opnodeindent*/
	
	
boolean opdefaultgettextrect (hdlheadrecord hnode, const Rect *linerect, Rect *textrect) {
	
	/*
	5.0a4 dmb: don't do the vert insetting here. For shaping the cursor, and
	deciding of a click is in text, we want to use the full headline height.
	so, we need to inset when drawing instead.

	6.0a13 dmb: account for horiz scrolling for fat healines
	*/
	
	register hdloutlinerecord ho = outlinedata;
	Rect r;
	
	r = *linerect; 
	
	/*
	r.top += textvertinset;
	
	r.bottom -= textvertinset;
	*/
	
	r.left += 
		opnodeindent (hnode) + (**ho).iconwidth + 
		
		(**ho).pixelsaftericon + 
		
		texthorizinset;
	
	r.right -= texthorizinset;
	
	if (opisfatheadlines (ho))
		r.right -= (**ho).horizscrollinfo.cur;

	*textrect = r;
	
	return (true);
	} /*opdefaultgettextrect*/


void opgettextrect (hdlheadrecord hnode, const Rect *linerect, Rect *textrect) {
	
	(*(**outlinedata).gettextrectcallback) (hnode, linerect, textrect);
	} /*opgettextrect*/


static boolean opmeasuretext (hdlheadrecord hnode) {
	
	bigstring bs;
	short pix;
	
	// thin headlines are easy to measure
	opgetheadstring (hnode, bs);
	
	oppushheadstyle (hnode);
	
	pix = stringpixels (bs) + (2 * texthorizinset);
	
	popstyle ();
	
	(**hnode).hpixels = pix;
	
	// height is even easier
	(**hnode).vpixels = (**outlinedata).defaultlineheight;
	
	return (true);
	} /*opmeauretext*/


boolean opdefaultgetlineheight (hdlheadrecord hnode, short *lh) {
	
	hdloutlinerecord ho = outlinedata;
	
	if ((**hnode).vpixels == opdirtymeasurevalue) {
		
		if (opisfatheadlines (ho))
			opeditmeasuretext (hnode);
		else
			opmeasuretext (hnode);
		}
	
	*lh = (**hnode).vpixels + (2 * textvertinset);
	
	return (true);
	} /*opdefaultgetlineheight*/


boolean opdefaultgetlinewidth (hdlheadrecord hnode, short *lw) {
	
	hdloutlinerecord ho = outlinedata;
	
	if ((**hnode).hpixels == opdirtymeasurevalue) {
		
		if (opisfatheadlines (ho))
			opeditmeasuretext (hnode);
		else
			opmeasuretext (hnode);
		}
	
	*lw = (**hnode).hpixels + (texthorizinset); // 6.0a13 dmb: not 2x horizinset
	
	return (true);
	} /*opdefaultgetlinewidth*/


short opgetlineheight (hdlheadrecord hnode) {
	
	short lh;
	
	(*(**outlinedata).getlineheightcallback) (hnode, &lh);
		
	return (lh);
	} /*opgetlineheight*/


short opgetlinewidth (hdlheadrecord hnode) {
	
	short lw;
	
	(*(**outlinedata).getlinewidthcallback) (hnode, &lw);
		
	return (lw);
	} /*opgetlinewidth*/


long opgetnodelinecount (hdlheadrecord hnode) {
	
	register hdloutlinerecord ho = outlinedata;
	
	if (opisfatheadlines (ho))
		return (opgetlineheight (hnode) / (**ho).defaultlineheight);
	else
		return (1);
	} /*opgetnodelinecount*/


boolean opdefaultgetfullrect (hdlheadrecord hnode, Rect *fullrect) {
	
	/*
	get the sum of the text rect and the icon rect for the indicated
	node. set the width of the rectangle to reflect the width of the
	text being displayed in the node.
	*/

	Rect linerect, iconrect, textrect;
	
	linerect = *fullrect;
	
	opgeticonrect (hnode, &linerect, &iconrect);
	
	opgettextrect (hnode, &linerect, &textrect);
	
	textrect.right = textrect.left + opgetlinewidth (hnode);
	
	if ((**outlinedata).flneedfulliconheight)
		unionrect (iconrect, textrect, fullrect);
	
	else {
		*fullrect = textrect;
		
		(*fullrect).left = iconrect.left;
		}
	
	return (true);
	} /*opdefaultgetfullrect*/
	

static void opgetfullrect (hdlheadrecord hnode, Rect *fullrect) {

	opgetnoderect (hnode, fullrect);

	(*(**outlinedata).getfullrectcallback) (hnode, fullrect);
	} /*opgetfullrect*/
	

void oppushheadstyle (hdlheadrecord hnode) {
	
	(*(**outlinedata).pushstylecallback) (hnode);
	} /*oppushheadstyle*/
	
	
static boolean oprectneedsupdate (Rect r) {

	#ifdef appletinclude
	
		return (apprectneedsupdate (r));
	
	#else
	
		return (shellrectneedsupdate (&r)); 
	
	#endif
	} /*oprectneedsupdate*/


boolean opdisabledisplay (void) {
	
	/*
	6.0b2 dmb: return the previous display state
	*/
	
	hdloutlinerecord ho = outlinedata;
	boolean flwasenabled = !(**ho).flinhibitdisplay;
	
	(**ho).flinhibitdisplay = true;
	
	if (opeditsetglobals ())
		(**wpdata).flinhibitdisplay = true;
	
	return (flwasenabled);
	} /*opdisabledisplay*/


boolean openabledisplay (void) {
	
	/*
	6.0b2 dmb: return true if the state changes
	*/
	
	hdloutlinerecord ho = outlinedata;
	boolean fldisabled = (**ho).flinhibitdisplay;
	
	(**ho).flinhibitdisplay = false;
	
	if (opeditsetglobals ())
		(**wpdata).flinhibitdisplay = false;
	
	return (fldisabled);
	} /*openabledisplay*/


boolean opdisplayenabled (void) {
	
	register hdloutlinerecord ho = outlinedata;
	
	if (ho == NULL)
		return (false);

	return ((**ho).flwindowopen && !(**ho).flinhibitdisplay);
	} /*opdisplayenabled*/


boolean opsetdisplayenabled (boolean fldisplay) {
	
	/*
	enable or disable display according to fl.
	
	return true if the display state changes
	
	6.0b2 dmb: don't do any redisplay
	*/
	
	hdloutlinerecord ho = outlinedata;
	
	if ((**ho).flinhibitdisplay != fldisplay) /*nothing to do*/
		return (false);
	
	if (fldisplay)
		openabledisplay ();
	else
		opdisabledisplay ();
	
	return (true);
	} /*opsetdisplayenabled*/


void opupdatenow (void) {
	
	if (!opdisplayenabled ())
		return;
		
	#ifdef appletinclude
		
		updateappwindow (app.appwindow);
		
	#else
	
		shellupdatecontent ((**outlinedata).outlinerect);
		
	#endif
	} /*opupdatenow*/
	
	
boolean opgetlinerect (long lnum, Rect *r) {

	/*
	lnum is a 0-based index into the window. lnum 0 refers to the line 
	displayed at the top of the window.
	
	6.0b2 dmb: always calculate a rect, even if it's above the screen and 
	we return false
	*/
	
	register hdloutlinerecord ho = outlinedata;
	short heightthisline;
	
	*r = (**ho).outlinerect; /*set left and right*/

	(*r).top += opsumprevlineheights (lnum, &heightthisline);
	
	(*r).bottom = (*r).top + heightthisline;
	
	return (lnum >= 0);
	} /*opgetlinerect*/
	

hdlheadrecord oppointnode (Point pt) {
	
	/*
	the point is assumed to be pointing at a node.  return that node.	
	*/
	
	hdlheadrecord nomad = (**outlinedata).hline1;
	long i, ct;
	
	ct = opgetcurrentscreenlines (false);
	
	for (i = 0; i < ct; i++) {
		
		Rect r;
		
		opgetlinerect (i, &r); 
		
		if (pointinrect (pt, r)) 
			return (nomad);
			
		nomad = opgetnextexpanded (nomad);
		} /*for*/
		
	return (nil); /*loop terminated, he's not pointing at a real node*/
	} /*oppointnode*/
	

boolean opgetscreenline (hdlheadrecord hnode, long *lnum) {

	/*
	give me a headrecord handle and I'll return the line it's displayed
	on.  it's a virtual line number, ie it can be less than 0 or greater
	than the number of lines on the screen.
	
	the line indicated by (**outlinedata).hline1 is on line 0.
	
	return false if the node isn't expanded or visible in this window.
	*/
	
	long uplnum, downlnum;
	hdlheadrecord upnomad, downnomad, lastnomad;
	hdloutlinerecord ho = outlinedata;
	hdlheadrecord hline1 = (**ho).hline1;
	
	upnomad = downnomad = hnode;
	
	uplnum = downlnum = 0;
	
	while (true) { // check up first
	
		if (upnomad == hline1) {
			
			*lnum = uplnum;
			
			return (true);
			}
		
		uplnum++;
		
		lastnomad = upnomad;
		
		upnomad = opbumpflatup (upnomad, true);
		
		if (lastnomad == upnomad) // didn't find hline1 above
			break;
		
		//uplnum += opgetnodelinecount (upnomad);
		}
	
	while (true) { // check down (we're above the screen)
			
		if (downnomad == hline1) {
			
			*lnum = downlnum;
			
			return (true);
			}
		
		downlnum--;
		
		lastnomad = downnomad;
		
		downnomad = opgetnextexpanded (downnomad);
		
		if (lastnomad == downnomad) // didn't find it below
			break;
		
		//downlnum += opgetnodelinecount (downnomad);
		}
		
	*lnum = 0;
	
	return (false);
	} /*opgetscreenline*/


boolean opgetnoderect (hdlheadrecord hnode, Rect *r) {
	
	/*
	6.0b2 dmb: only return false if the node isn't displayed at all, not if
	it's above the display -- the rect is now meaningfull.
	*/
	
	long lnum;
	
	if (!opgetscreenline (hnode, &lnum)) 
		return (false);
	
	opgetlinerect (lnum, r);
	
	return (true);
	} /*opgetnoderect*/


void opsetdisplaydefaults (hdloutlinerecord ho) {
	
	short lh;
	
	oppushstyle (ho);
	
	lh = globalfontinfo.ascent + globalfontinfo.descent + globalfontinfo.leading;
	
	if ((**ho).flneedfulliconheight)
		lh = max (lh, (**ho).iconheight); 

	(**ho).defaultlineheight = lh;
	
	if ((**ho).flprinting && iscurrentportprintport()) {
		long scaleMult, scaleDiv;

		getprintscale (&scaleMult, &scaleDiv);

		(**ho).lineindent = (15L * scaleMult) / scaleDiv;
		
		#ifdef WIN95VERSION // can't print icons
			(**ho).iconheight = 0;
			(**ho).iconwidth = 0;
		#else
			(**ho).iconheight = (16L * scaleMult) / scaleDiv;
			(**ho).iconwidth = (16L * scaleMult) / scaleDiv;
		#endif
		}
	else {
		(**ho).lineindent = 15;
		(**ho).iconheight = 16;
		(**ho).iconwidth = 16;
		}

	popstyle ();
	} /*opsetdisplaydefaults*/
	

static boolean oppushclip (Rect *rclip) {
	
	/*
	we make sure that the rect doesn't have any territory outside the 
	display rectangle, if it does, then we cut it back down to size.
	
	returns false if the intersection of the two rects is empty.
	
	to pop, call popclip -- if we return false, don't -- we didn't
	push anything.
	*/
	
	if (!intersectrect (*rclip, (**outlinedata).outlinerect, rclip))
		return (false);
		
	return (pushclip (*rclip));
	} /*oppushclip*/
	

void oplineinval (long lnum) {
	
	Rect r;
	
	if (!opdisplayenabled ())
		return;
	
	if (opgetlinerect (lnum, &r))
		invalrect (r);
	} /*oplineinval*/
	

static void oprangeinval (long firstlnum, long lastlnum) {
	
	Rect r1, r2;
	
	opgetlinerect (firstlnum, &r1);
	
	opgetlinerect (lastlnum, &r2);
	
	r1.bottom = r2.bottom;
	
	invalrect (r1);
	} /*oprangeinval*/
	

boolean opinvalnode (hdlheadrecord hnode) {
	
	/*
	1/6/97 dmb: don't inval if getting line returns false
	
	2/19/97 dmb: don't even opgetscreenline if display is disabled
	*/
	
	long lnum;
	
	if (opdisplayenabled ()) {
		
		if (opgetscreenline (hnode, &lnum))
			oplineinval (lnum);
		}
	
	return (true);
	} /*opinvalnode*/
	

static boolean opinvalnodevisit (hdlheadrecord hnode, ptrvoid refcon) {
	
	/*
	1/6/97 dmb: don't inval if getting line returns false
	*/
	
	opinvalnode (hnode);
	
	return (true);
	} /*opinvalnodevisit*/
	
	
void opinvalstructure (hdlheadrecord hnode) {
	
	/*
	inval all the nodes subordinate to hnode, and hnode itself.
	*/
	
	opinvalnode (hnode);
	
	oprecursivelyvisit (hnode, infinity, &opinvalnodevisit, nil);
	} /*opinvalstructure*/


void opinvalafter (hdlheadrecord hnode) {
	
	long lnum;
	
	opgetscreenline (hnode, &lnum);
	
	oprangeinval (lnum, opgetcurrentscreenlines (false));
	} /*opinvalafter*/
	
	
void opinvalbarcursor (void) {
	
	opinvalnode ((**outlinedata).hbarcursor);
	} /*opinvalbarcursor*/
	

void opinvaldisplay (void) {
	
	if (!opdisplayenabled ())
		return;
	
	invalrect ((**outlinedata).outlinerect);
	} /*opinvaldisplay*/
	
	
void opsmashdisplay (void) {
	
	opinvaldisplay ();
	
	opupdatenow ();
	} /*opsmashdisplay*/
	
	
void operaserect (Rect r) {
	
	pushbackcolor (&(**outlinedata).backcolor);
	
	eraserect (r);
	
	popbackcolor ();
	} /*operaserect*/


void operasedisplay (void) {
	
	pushbackcolor (&(**outlinedata).backcolor);
	
	smashrect ((**outlinedata).outlinerect); /*erase and inval*/
	
	popbackcolor ();
	} /*operasedisplay*/


static boolean opdirtymeasurementsvisit (hdlheadrecord hnode, ptrvoid refcon) {
	
	(**hnode).hpixels = (**hnode).vpixels = opdirtymeasurevalue;
	
	return (true);
	} /*opdirtymeasurementsvisit*/


boolean opdirtymeasurements (void) {
	
	return (opvisiteverything (opdirtymeasurementsvisit, nil));
	} /*opdirtymeasurements*/


static void opupdateafterresize (void) {
	
	/*
	7.0b16 PBS: fix the display glitch when in edit mode, you resize a window,
	and the display is incorrect. The outline view should get updated.

	Before calling oppostedit, make sure there is a global outline,
	it's in text mode, there's an edit cursor, and there are wp globals.
	*/

	if (outlinedata == nil) /*Must have an outline global*/
		return;

	if (!(**outlinedata).fltextmode) /*Must be in edit mode*/
		return;

	if ((**outlinedata).heditcursor == nil) /*Must be an edit cursor*/
		return;

	if (wpdata == nil) /*Must have wp globals*/
		return;

	oppostedit (); /*Update the outline display*/
	} /*opupdateafterresize*/


boolean oppostfontchange (void) {
	
	/*
	6.0b3 dmb: must opsetscrollpositiontoline1

	7.0b16: Fix display glitch after resizing an outline when in edit mode.
	*/
	
	boolean fl = false;

	opdirtymeasurements ();
	
	opsetctexpanded (outlinedata); //6.0a12 dmb
	
	opsetscrollpositiontoline1 ();
	
	fl = opseteditbufferrect ();

	opupdateafterresize (); /*7.0b16 PBS*/

	return (fl);
	} /*oppostfontchange*/


short opmaxlevelwidth (hdlheadrecord hnode) {
	
	/*
	visit each of the siblings of hnode, and compute the maximum displaypixels
	value for all the nodes.  do not include the node itself in the computation.
	
	if he's the only node at his level, return 0.
	*/
	
	register hdlheadrecord nomad, nextnomad;
	register short maxpixels = 0;
	Rect fullrect;
	short pixels;
		
	nomad = opfirstatlevel (hnode);
	
	while (true) {
		
		if (nomad != hnode) { /*don't include the node itself in the analysis*/
			
			opgetfullrect (hnode, &fullrect);	
			
			pixels = (fullrect.right - fullrect.left);
			
			if (pixels > maxpixels) /*found a new max*/
				maxpixels = pixels;
			}
		
		nextnomad = (**nomad).headlinkdown;
		
		if (nextnomad == nomad) /*reached the last one*/
			return (maxpixels);
		
		nomad = nextnomad; /*advance to next node at level*/
		} /*while*/
	} /*opmaxlevelwidth*/


boolean opdefaultpredrawline (hdlheadrecord hnode, const Rect *rline, boolean flselected, boolean flinverted) {
	
	pushbackcolor (&(**outlinedata).backcolor);
	
	eraserect (*rline); 
	
	popbackcolor ();
	
	return (true);
	} /*opdefaultpredrawline*/


boolean opdefaultdrawtext (hdlheadrecord hnode, const Rect *rtext, boolean flselected, boolean flinverted) {
	
	/*
	the normal text drawing routine, used for script editor and
	menu editor in Frontier. 
	*/
	
	hdloutlinerecord ho = outlinedata;
	bigstring headstring;
	Rect r = *rtext;
	
	if (opeditingtext (hnode)) {
	
		opeditupdate ();
		
		return (true);
		}
	
	r.top += textvertinset;
	
	r.bottom -= textvertinset;
	
	if (opisfatheadlines (ho)) {
		
		opeditdrawtext (hnode, &r);
		}
	
	else {
		getheadstring (hnode, headstring);
		
		if ((**hnode).fldrawelipsizes) /*DW 10/9/95 -- hack -- see claylinelayout.c*/
			ellipsize (headstring, r.right - r.left);
		
		movepento (r.left, r.top + globalfontinfo.ascent);
		
		pendrawstring (headstring);
		}
	
	if (flselected) {
		
	//	Point pt;
		Rect rframe = r;
		
		rframe.right = rframe.left + (**hnode).hpixels + texthorizinset;
		
	//	rframe.left -= 2; /*leave 2 extra pixels for prettier bar cursor*/
		rframe.left -= (**ho).iconwidth + (**ho).pixelsaftericon + texthorizinset;
		
		/*
		rframe.top += textvertinset;
		
		rframe.bottom -= textvertinset;
		*/
		
		/*
		getpenpoint (&pt);
		
		rframe.right = pt.h + 2; /*leave 2 extra pixels for prettier bar cursor*/
		
		if (flinverted)
			invertrect (rframe);
		else
			grayframerect (rframe);
		}
	

	return (true);
	} /*opdefaultdrawtext*/
	

boolean opdefaultpostdrawline (hdlheadrecord hnode, const Rect * linerect, boolean flselected, boolean flinverted) {

	return (true);
	} /*opdefaultpostdrawline*/
	

void opgetlineselected (hdlheadrecord hnode, boolean *flinverted, boolean *flselected) {

	/*
	analyze the line to determine if it should be drawn selected and/or
	inverted.
	*/

	register hdloutlinerecord ho = outlinedata;

	*flinverted = (**ho).flactive; /*text is inverted when we're the active app & window*/

	if ((**ho).fltextmode) {
		
		*flselected = false; /*when editing text nothing is selected*/
		
		return;
		}
		
	if ((**ho).flprinting) { 
		
		*flselected = false; /*no barcusor in printout*/
		
		return;
		}
		
	if (!(**ho).flbarcursoron) {
		
		*flselected = (**hnode).flmarked; /*someone turned the cursor display off*/
		
		return;
		}
		
	if (opanymarked ()) {
	
		*flselected = (**hnode).flmarked;
		
		return;
		}
	
	/*not in textmode, printing, barcursor is on, nothing is marked*/
	
	*flselected = (**ho).hbarcursor == hnode;
	} /*opgetlineselected*/


void opdrawicon (hdlheadrecord hnode, Rect linerect) {

	/*
	6.0a1 dmb: draw just the headline's icon, accounting for selectedness, 
	invertedness
	*/

	register hdloutlinerecord ho = outlinedata;
	Rect rclip = linerect;
	Rect iconrect;
	boolean flinverted, flselected;
	
	if (hnode == nil) /*defensive driving, old versions would erase the rect*/
		return;
	
	opgetlineselected (hnode, &flinverted, &flselected);
	
	opgeticonrect (hnode, &linerect, &iconrect); /*iconrect is derived from the line rect*/

	opgettextrect (hnode, &linerect, &rclip);

	rclip.top += textvertinset;
	
	rclip.bottom -= textvertinset;

	rclip.right = rclip.left;
		
	rclip.left -= (**ho).iconwidth + (**ho).pixelsaftericon; // + texthorizinset;
	
	//intersectrect (rclip, iconrect, &rclip);
	
	if (!oppushclip (&rclip))
		return;
	
	oppushcolors (ho);
	
	(*(**ho).predrawlinecallback) (hnode, &linerect, flselected, flinverted);
	
	(*(**ho).drawiconcallback) (hnode, &iconrect, flselected, flinverted);
	
	if (flselected && flinverted)
		invertrect (rclip);
	
	(*(**ho).postdrawlinecallback) (hnode, &linerect, flselected, flinverted);

	popcolors ();
	
	popclip ();
	} /*opdrawicon*/


void opdrawline (hdlheadrecord hnode, Rect linerect) {
	
	/*
	linerect frames the line which displays the node. left and right
	are outlinerect's left and right. top and bottom reflect the 
	scrolled position of the line, and the line's height.
	
	we compute the icon rectangle and text rectangle and whether things
	are highlighted or not, and call the callback routines to draw
	the icon and text.
	
	12/11/96 dmb: no bitmaps. it's just a waste of bitcopying when 
	responding to an update, and it requires more elaborate update 
	calls for paige when text is active
	*/
	
	register hdloutlinerecord ho = outlinedata;
	Rect iconrect, textrect;
	Rect rclip = linerect;
	boolean flbitmap = false;
	boolean flinverted, flselected;
	
	if (hnode == nil) /*defensive driving, old versions would erase the rect*/
		return;
	
	(**hnode).fldirty = false; /*doesn't survive a refresh*/
	
	if (!oppushclip (&rclip))
		return;
	
	/*
	#ifdef fldebug
		if (!optionkeydown ())
			flbitmap = openbitmap (rclip, outlinewindow);
	#else
		flbitmap = openbitmap (rclip, outlinewindow);
	#endif
	*/
	
	opgeticonrect (hnode, &linerect, &iconrect); /*iconrect is derived from the line rect*/
		
	opgettextrect (hnode, &linerect, &textrect); /*textrect is derived from the line rect*/
		
	opgetlineselected (hnode, &flinverted, &flselected);
	
	(*(**ho).predrawlinecallback) (hnode, &linerect, flselected, flinverted);
	
//	if (oprectneedsupdate (iconrect))
		(*(**ho).drawiconcallback) (hnode, &iconrect, flselected, flinverted);
	
//	if (oprectneedsupdate (textrect))
		(*(**ho).drawlinecallback) (hnode, &textrect, flselected, flinverted);
	
	(*(**ho).postdrawlinecallback) (hnode, &linerect, flselected, flinverted);
	
	/*
	if (flbitmap)
		closebitmap (outlinewindow);
	*/
	
	popclip ();
	} /*opdrawline*/
	
	
void opupdate (void) {

	register hdloutlinerecord ho = outlinedata;
	hdlheadrecord nomad;
	Rect outlinerect;
	Rect r; 
	hdlheadrecord nextnomad;
	long i;
	
	if (ho == NULL)
		return;

	nomad = (**ho).hline1;
	
	r = outlinerect = (**ho).outlinerect;
	
	i = 0;

 	oppushstyle (ho);
 	
	oppushcolors (ho);
	
	#ifdef gray3Dlook
		if (!(**ho).fldonterasebackground) {
			
			insetrect (&outlinerect, -3, -3);
			
			framerect (outlinerect);
			
			insetrect (&outlinerect, 1, 1);

			eraserect (outlinerect);

			insetrect (&outlinerect, 2, 2);
			}
	#endif
	
	r.top += opgetline1top ();
	
 	while (true) {
 		
 		++i;
 		
 		r.bottom = r.top + opgetlineheight (nomad);
 		
 		if (r.top >= outlinerect.bottom)
 			break;
 		
 		if (oprectneedsupdate (r))		
 			opdrawline (nomad, r);
 		
 		r.top = r.bottom;
 		
  		//if (nomad == (**ho).hbarcursor)
 		//	(**ho).lnumbarcursor = i;
 			
 		nextnomad = opgetnextexpanded (nomad);
 		
 		if (nextnomad == nomad) 
 			break;
 			
 		nomad = nextnomad;
 		} /*while*/
	
	#ifndef gray3Dlook
		if (!(**ho).fldonterasebackground) {

 			r.bottom = outlinerect.bottom;
 			
			if (oprectneedsupdate (r))
				operaserect (r);
			}
	#endif
	
	popcolors ();
	
 	popstyle ();
   } /*opupdate*/


void opdocursor (boolean flon) {
	
	register hdloutlinerecord ho = outlinedata;
	long lnumcursor;
	Rect r;
	
	if (!opdisplayenabled ())
		return;
	
	oppushstyle (ho);
	
	oppushcolors (ho);

	(**ho).flbarcursoron = flon;
	
	opgetscreenline ((**ho).hbarcursor, &lnumcursor);
	
	//(**ho).lnumbarcursor = lnumcursor;
	
	if (opgetlinerect (lnumcursor, &r))
		opdrawline ((**ho).hbarcursor, r);
	
	popcolors ();

	popstyle ();
	
	(**ho).flbarcursoron = true; 
	} /*opdocursor*/
	
	
void opscrollrect (Rect r, long dh, long dv) {
	
	pushbackcolor (&(**outlinedata).backcolor); /*so that erasures use the right color*/
	
	scrollrect (r, dh, dv);
	
	popbackcolor ();
	} /*opscrollrect*/
	
	
void opmakegap (long lnum, short lineheight) {
	
	/*
	create a gap in the display after the indicated line number.
	
	5.0d18 dmb: handle lnum < 0 (-1) so opexpandupdate will work when 
	new node is first in list
	*/
	
	register hdloutlinerecord ho = outlinedata;
	Rect r;
	
	if (lnum < 0)
		opgetlinerect (0, &r);
	
	else {
		opgetlinerect (lnum, &r);
		
		r.top = r.bottom;
		}
	
	r.bottom = (**ho).outlinerect.bottom;
	
	opscrollrect (r, 0, lineheight);
	} /*opmakegap*/
	

static boolean opvertscrollrecord (long *ctscroll, long *ctpixels) {
	
	/*
	only scrolls the record, the data structures, not the pixels on the screen.
	
	if ctscroll is negative, the text scrolls up; positive ==> down.
	
	returns true if any scrolling is necessary.
	
	7/24/91 dmb: protect against integer overflow
	
	6.0a12 dmb: scrolling is line-wise, not headline-wise; also count pixels here
	*/
	
	register hdloutlinerecord ho = outlinedata;
	long ct = *ctscroll;
	long vc;
	hdlheadrecord nomad;
	
	if (ctpixels != nil)
		*ctpixels = 0;
	
	vc = (**ho).vertscrollinfo.cur - (long) ct;
	
	vc = min (vc, (**ho).vertscrollinfo.max);
	
	vc = max (vc, (**ho).vertscrollinfo.min);
	
	ct = (**ho).vertscrollinfo.cur - (long) vc;
	
	if (ct == 0) /*nothing to do*/
		return (false);
		
	(**ho).vertscrollinfo.cur = (long) vc;
	
	*ctscroll = ct;
	
	ct -= (**ho).line1linesabove; //bring line1 to top before counting
	
	if (ctpixels != nil)
		*ctpixels -= opgetline1top ();
	
	nomad = (**ho).hline1;
	
	if (ct > 0) {
	
		while (ct > 0) {
			
			nomad = opbumpflatup (nomad, true); 
			
			ct -= opgetnodelinecount (nomad);
			
			if (ctpixels != nil)
				*ctpixels += opgetlineheight (nomad);
			}
		
		(**ho).line1linesabove = -ct;
		
		if (ctpixels != nil)
			*ctpixels += opgetline1top ();
		}
	else {
		
		ct = -ct;
		
		while (opgetnodelinecount (nomad) <= ct) {
			
			ct -= opgetnodelinecount (nomad);
			
			if (ctpixels != nil)
				*ctpixels -= opgetlineheight (nomad);
			
			nomad = opbumpflatdown (nomad, true);
			}
		
		(**ho).line1linesabove = ct;
		
		if (ctpixels != nil)
			*ctpixels += opgetline1top ();
		}
	
	(**ho).hline1 = nomad;
	
	//(**ho).lnumbarcursor -= ct;
	
	return (true);
	} /*opvertscrollrecord*/
	

static boolean ophorizscrollrecord (long *ctscroll) {

	/*
	if ctscroll is negative, the text scrolls left; positive ==> right.
	
	7/24/91 dmb: protect against integer overflow
	*/
	
	register hdloutlinerecord ho = outlinedata;
	register long ct = *ctscroll;
	register long horizcurrent = (**ho).horizscrollinfo.cur;
	register long hc;
	
	hc = horizcurrent - (long) ct;
	
	hc = min (hc, (**ho).horizscrollinfo.max);
	
	hc = max (hc, (**ho).horizscrollinfo.min);
	
	ct = horizcurrent - (long) hc;
	
	*ctscroll = ct;
	
	if (ct == 0) /*nothing to do*/
		return (false);
		
	(**ho).horizscrollinfo.cur = (long) hc;
	
	return (true);
	} /*ophorizscrollrecord*/
	

static boolean ophorizscroll (long ctpixels) {
	
	opupdatenow ();
	
	if (!ophorizscrollrecord (&ctpixels)) /*only deals with fields of outline record*/
		return (false);
	
	opseteditbufferrect ();
	
	opscrollrect ((**outlinedata).outlinerect, ctpixels, 0);
	
	opresetscrollbars ();
	
	opupdatenow ();
	
	return (true);
	} /*ophorizscroll*/
	

static long opvisiupcountpixels (hdlheadrecord nomad, long vscroll) {
	
	long i, ct = 0;
	
	for (i = 1; i <= vscroll; i++) {
		
		nomad = opbumpflatup (nomad, true);
		
		ct += opgetlineheight (nomad);	
		} /*for*/
		
	return (ct);
	} /*opvisiupcountpixels*/
	
	
static long opvisidowncountpixels (hdlheadrecord nomad, long vscroll) {
	
	long i, ct = 0;
	
	for (i = 1; i <= vscroll; i++) {
		
		ct += opgetlineheight (nomad);
		
		nomad = opgetnextexpanded (nomad);
		} /*for*/
		
	return (ct);
	} /*opvisidowncountpixels*/


static boolean opvertscroll (long ctlines) {
	
	//hdlheadrecord oldline1 = (**outlinedata).hline1;
	long ctpixels;
	
	opupdatenow ();
	
	if (!opvertscrollrecord (&ctlines, &ctpixels)) /*only deals with fields of outline record*/
		return (false);
	
	opseteditbufferrect ();
	
	if (opdisplayenabled ()) {
		
		/*
		if (ctlines > 0) 
			ctpixels = opvisiupcountpixels (oldline1, ctlines);
		else 
			ctpixels = -opvisidowncountpixels (oldline1, -ctlines);
		*/
		
		opscrollrect ((**outlinedata).outlinerect, 0, ctpixels);
		}
	
	opresetscrollbars ();
	
	opupdatenow ();
	
	return (true);
	} /*opvertscroll*/


void opjumpdisplayto (hdlheadrecord holdcursor, hdlheadrecord hnewcursor) {
	
	/*
	a special way to move the cursor when you know it's going a long 
	distance. no point erasing the old cursor. no point trying to scroll
	or erase the display. everything is going to be updated. do it all
	in one shot for nicer staging.
	
	DW 8/15/93: try to make it sexier if there is no scrolling required.
	
	DW 10/17/93: erase the display in addition to inval'ing it. cb can
	take a few seconds to redraw the window, this makes for better 
	staging.
	
	6.0a12 dmb: need resetscrollbars here
	*/
	
	long hscroll, vscroll;
	
	opinvalnode (holdcursor);
	
	if (opneedvisiscroll (hnewcursor, &hscroll, &vscroll, true)) {
		
		/*DW 10/27/93 -- make sure the head is on the first line in the window*/ {
			
			if (vscroll < 0)
				vscroll -= opgetcurrentscreenlines (true) - 1;
			}
			
		opvertscrollrecord (&vscroll, nil); 

		ophorizscrollrecord (&hscroll);
		
		opinvaldisplay (); /*everything will get redrawn*/
		
		operaserect ((**outlinedata).outlinerect);
		}
	else
		opinvalnode (hnewcursor);
	
	opresetscrollbars ();
	} /*opjumpdisplayto*/


static boolean opscrollto (long h, long v) {
	
	/*
	DW 10/27/93: new version bends over backwards to avoid 
	jerkyness.
	*/
	
	long horizpixels, vertlines;
	boolean flhoriz, flvert;
	
	horizpixels = (**outlinedata).horizscrollinfo.cur - h;
	
	flhoriz = ophorizscrollrecord (&horizpixels);
	
	vertlines = (**outlinedata).vertscrollinfo.cur - v;
	
	flvert = opvertscrollrecord (&vertlines, nil);
	
	if ((!flvert) && (!flhoriz)) 
		return (false);
	
	opseteditbufferrect ();

	opresetscrollbars ();
	
	operasedisplay ();
	
	return (true);
	} /*opscrollto*/


static long scrollquantum (register long hscroll) {
	
	register long hquantum = (**outlinedata).lineindent;
	
	return  (quantumize (hscroll, hquantum));
	} /*scrollquantum*/


static long getuppagescrolllines (void) {
	
	/*
	6.0a12 dmb: this routine was in error. the correct number of lines
	to scroll up for a page depends on what is on the screen now, not what's
	below it. the number has already been calculated in the vertscrollinfo.
	*/
	
	register hdloutlinerecord ho = outlinedata;
	
	return ((**ho).vertscrollinfo.pag);
	
	/*
	Rect r = (**ho).outlinerect;
	long vertpixels = r.bottom - r.top;
	long ctpixels = 0, ctlines = 0;
	hdlheadrecord h, hnext;
	
	h = opgetnextexpanded (opgetlastvisiblenode ());
	
	while (true) {
		
		ctpixels += opgetlineheight (h);
		
		if (ctpixels > vertpixels)
			return (ctlines);
		
		ctlines += opgetnodelinecount (h);
			
		hnext = opgetnextexpanded (h);
		
		if (hnext == h)
			return (ctlines);
			
		h = hnext;
		}
	*/
	} /*getuppagescrolllines*/


static long getdownpagescrolllines (void) {
	
	/*
	6.0a12 dmb: subtract one from the number calcuated to create a one-line
	overlap between pages. also, handle partial lines and don't count hline1
	*/
	
	register hdloutlinerecord ho = outlinedata;
	Rect r = (**ho).outlinerect;
	long vertpixels = r.bottom - r.top;
	long ctpixels = 0, ctlines = 0;
	hdlheadrecord h, hnext;
	
	h = (**ho).hline1;
	
	ctpixels = -opgetline1top ();
	
	ctlines = (**ho).line1linesabove;
	
	while (true) {
			
		hnext = opbumpflatup (h, true);
		
		if (hnext == h)
			return (ctlines);
		
		h = hnext;
		
		ctpixels += opgetlineheight (h);
		
		if (ctpixels > vertpixels) {
			
			ctlines += (ctpixels - vertpixels) / (**ho).defaultlineheight;
			
			return (ctlines - 1);
			}
		
		ctlines += opgetnodelinecount (h);
		} /*while*/
	} /*getdownpagescrolllines*/


boolean opscroll (tydirection dir, boolean flpage, long ctscroll) {
	
	/*
	5.1.5b12 dmb: check for nil outline
	*/
	
	register hdloutlinerecord ho = outlinedata;
//	hdlheadrecord hline1;
	boolean fl = false;
	Rect r;
	
	if (ho == nil)
		return (false);
	
//	hline1 = (**ho).hline1;
	
	switch (dir) {
		
		case up: { /*text scrolls up, cursor moves down*/
			if (flpage) {
			
				ctscroll = getuppagescrolllines ();
				
				fl = opvertscroll (-ctscroll);
				}
			else {
				fl = opvertscroll (-ctscroll);
				}
				
			break;
			}
			
		case down: { /*text scrolls down, cursor moves up*/
			if (flpage) {
			
				ctscroll = getdownpagescrolllines ();
			
				fl = opvertscroll (ctscroll);
				}
			else {
				fl = opvertscroll (ctscroll);
				}
				
			break;
			}
		
		case left: case right:
			if (flpage) {
				
				r = (**ho).outlinerect;
				
				ctscroll *= scrollquantum ((r.right - r.left) / 2);
				}
			else
				if (ctscroll == 1)
					ctscroll *= (**ho).lineindent;
					
			if (dir == left)
				fl = ophorizscroll (-ctscroll);
			else
				fl = ophorizscroll (ctscroll);
					
			break;
		} /*switch*/
				
	opdirtyview ();
	
	return (fl);
	} /*opscroll*/


#if 0

static void supersmarthorizscrollsmarts (long *hscroll) {
	
	/*
	new idea, DW 10/17/93. after certain very jarring operations, like
	jumping a long distance, or switching in a different style in clay
	basket, it's really ugly to leave the screen less than optimally
	scrolled.
	
	we travel over all the lines in the window and figure out how much
	horizontal scrolling would be necessary to make them all visible.
	
	we prefer to make everything off to the left visible, if there's 
	stuff scrolled off both edges.
	
	we never scroll enough to make the cursor headline not visible. that
	makes it safe to call this after making the cursor horizontally
	visible.
	
	DW 4/10/95: allow for blocking this visi routine.
	*/
	
	hdlheadrecord nomad = (**outlinedata).hline1;
	Rect outlinerect = (**outlinedata).outlinerect;
	long maxoffleft = 0, maxoffright = 0;
	long i, ct;
	Rect rcursor;
	
	return; /*10/9/95 -- feature wired off -- it's not optimal*/
	
	if ((**outlinedata).blocksupersmartvisi) {
		
		(**outlinedata).blocksupersmartvisi = false;
		
		return;
		}
	
	ct = opgetcurrentscreenlines ();
	
	for (i = 0; i < ct; i++) {
		
		Rect r;
		long offleft, offright;
		
		opgetfullrect (nomad, &r);
		
		offleft = outlinerect.left - r.left;
		
		if (offleft > maxoffleft)
			maxoffleft = offleft;
		
		offright = r.right - outlinerect.right;
		
		if (offright > maxoffright)
			maxoffright = offright;
			
		nomad = opgetnextexpanded (nomad);
		} /*for*/
		
	maxoffleft = scrollquantum (maxoffleft);
		
	maxoffright = scrollquantum (maxoffright);
		
	if ((maxoffleft == 0) && (maxoffright == 0)) /*no scrolling needed, everything's visible*/
		return;
		
	opgetfullrect ((**outlinedata).hbarcursor, &rcursor);
		
	if (maxoffleft > 0) {
		
		if ((rcursor.right + maxoffleft) <= outlinerect.right) { 

			*hscroll = maxoffleft;
		
			return;
			}
		}
		
	if (maxoffright > 0) {
		
		if ((rcursor.left - maxoffright) >= outlinerect.left)
			*hscroll = -maxoffright;
		}
	} /*supersmarthorizscrollsmarts*/

#endif

boolean opneedvisiscroll (hdlheadrecord hnode, long *hscroll, long *vscroll, boolean flcheckhoriz) {
	
	/*
	return true if one of hscroll or vscroll is non-zero.
	
	6.0b2 dmb: return false when window isn't open instead of when display is disabled.
	this allows script-generated visiing to work, but protects opverbfind from 
	screwing the display.
	*/
	
	register hdloutlinerecord ho = outlinedata;
	register long leftdiff, rightdiff;
	Rect r;
	long lnum;
	
	*hscroll = *vscroll = 0;
	
	if (!ho || !(**ho).flwindowopen) //opdisplayenabled ()
		return (false);
	
	if (!opgetscreenline (hnode, &lnum))
		return (false);
	
	if (lnum <= 0)/*line is scrolled off the top of the window*/
		*vscroll = opgetlinestoscrolldownforvisi (hnode);
	
	else {
		long ct = opgetlinestoscrollupforvisi (hnode); 
		
		if (ct > 0)
			*vscroll = -ct;		
		}
	
	if (!flcheckhoriz)
		return (*vscroll != 0);
	
	if (opeditingtext (hnode))
		opeditgetselrect (&r);
	else 
		opgetfullrect (hnode, &r);
	
	rightdiff = scrollquantum ((**ho).outlinerect.left - r.left);
	
	leftdiff = scrollquantum (r.right - (**ho).outlinerect.right);
	
	if (rightdiff > 0) 
		*hscroll = rightdiff; /*scroll text to the right*/
	else {
		leftdiff = minint (leftdiff, -rightdiff);
		
		if (leftdiff > 0)
			*hscroll = -leftdiff; /*scroll text to the left*/
		}
		
//	supersmarthorizscrollsmarts (hscroll);
	
	return ((*hscroll != 0) || (*vscroll != 0));
	} /*opneedvisiscroll*/


void opdovisiscroll (long hscroll, long vscroll) {
	
//	hdlheadrecord oldline1 = (**outlinedata).hline1;
	long vpixels;
	
	if (!(**outlinedata).blockvisiupdate)
		opupdatenow ();
	
	opvertscrollrecord (&vscroll, &vpixels); 
	
	ophorizscrollrecord (&hscroll);
	
	opseteditbufferrect ();
	
	if (opdisplayenabled ()) {
		
		/*
		if (vscroll > 0) 
			vscroll = opvisiupcountpixels (oldline1, vscroll);
		else 
			vscroll = -opvisidowncountpixels (oldline1, -vscroll);
		*/
		
		opscrollrect ((**outlinedata).outlinerect, hscroll, vpixels);
		}
	
	opresetscrollbars ();
	
	if (!(**outlinedata).blockvisiupdate)
		opupdatenow ();
	} /*opdovisiscroll*/


boolean opnodevisible (hdlheadrecord hnode) {
	
	long hscroll, vscroll;
	
	return (!opneedvisiscroll (hnode, &hscroll, &vscroll, false));
	} /*opnodevisible*/


boolean opvisinode (hdlheadrecord hnode, boolean flhoriz) {

	/*
	make the node vertically visible in the window.  
	
	return true if scrolling was necessary.
	
	dmb 9/21/90: take flhoriz parameter to permit vertical-only visiing
	*/
	
	long hscroll, vscroll;
	
	if (!opneedvisiscroll (hnode, &hscroll, &vscroll, flhoriz))
		return (false);
	
	opdovisiscroll (hscroll, vscroll);
	
	return (true);
	} /*opvisinode*/


void opvisisubheads (hdlheadrecord hnode) {

	if (ophassubheads (hnode) && opsubheadsexpanded (hnode))
		opvisinode (oplastexpanded (hnode), false);
	
	opvisinode (hnode, true);
	} /*opvisisubheads*/


static boolean opgetmaxwidthvisit (hdlheadrecord hnode, long *maxheadwidth) {
	
	short width;
	hdloutlinerecord ho = outlinedata;
		
	if ((**hnode).flexpanded) {
		
		(*(**ho).getlinewidthcallback) (hnode, &width);
		
		width += (**ho).lineindent * (**hnode).headlevel + (**ho).iconwidth + (**ho).pixelsaftericon;
		
		width = scrollquantum (width);
		
		if (width > *maxheadwidth)
			*maxheadwidth = width;
		}
	
	return (true);
	} /*opgetwidthvisit*/


boolean opgetoutinesize (long *width, long *height) {
	
	/*
	6.0a13 dmb: with wrapping headlines, this gets must more complicated.
	we need to see how things would wrap if we were the size of the full 
	screen, not the size when wrapped to the current window.
	
	7.0b17 PBS: On Macs, add a little to the width to prevent wrapping of a headline.
	*/
	
	register hdloutlinerecord ho = outlinedata;
	Rect rwindow, rscreen;
	Rect routline;
	long hextra, maxheadwidth;
	
	//grow outlinerect temporarily as if the window grew to fill the screen

	getglobalwindowrect (shellwindow, &rwindow);

	getwindowscreenbounds (&rwindow, &rscreen);
	
	insetrect (&rscreen, 3, 3);
	
	hextra = (rscreen.right - rscreen.left) - (rwindow.right - rwindow.left);
	
	routline = (**ho).outlinerect;
	
	(**ho).outlinerect.right += hextra;

	opvisiteverything (opdirtymeasurementsvisit, nil);
	
	//measure everything
	
	*height = opsumalllineheights ();
	
	maxheadwidth = 0; /*set for visit routine*/
	
	opsiblingvisiter ((**outlinedata).hsummit, false, &opgetmaxwidthvisit, &maxheadwidth);
	
	*width = maxheadwidth;
	
	#ifdef MACVERSION
	
		*width = *width + 5; /*7.0b17 PBS: a little extra needed on Macs to prevent wrapping.*/
	
	#endif

	//restore outlinerect
	
	(**ho).outlinerect = routline;
	
	opvisiteverything (opdirtymeasurementsvisit, nil);

	return (true);
	} /*opgetoutinesize*/



