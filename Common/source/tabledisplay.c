
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

#include "bitmaps.h"
#include "cursor.h"
#include "kb.h"
#include "file.h"
#include "font.h"
#include "icon.h"
#include "mouse.h"
#include "ops.h"
#include "quickdraw.h"
#include "resources.h"
#include "strings.h"
#include "shell.h"
#include "shellhooks.h"
#include "shell.rsrc.h"
#include "op.h"
#include "opdisplay.h"
#include "opicons.h"
#include "oplineheight.h"
#include "tableformats.h"
#include "tableinternal.h"
#include "wpengine.h"
#include "tabledisplay.h"
#include "claybrowserstruc.h"
#include "claycallbacks.h"
#include "claylinelayout.h"



#define fldisplaydebug false

#define mincolwidth 10
#define minrowheight 10

#define maxcolwidth 1000
#define maxrowheight 1000

#define pixelsbetweencolumns 15
#define binarycharacterstomeasure 30

#define drawinset 4


static short opnodeindentpix (hdlheadrecord hnode) {

	return ((**outlinedata).lineindent * (**hnode).headlevel);
	} /*opnodeindentpix*/


static short opiconindentpix (void) {

	return ((**outlinedata).iconwidth + (**outlinedata).pixelsaftericon);
	} /*opiconindentpix*/


/*
boolean tabledrawnodeicon (const Rect *r, boolean flhighlighted, hdlheadrecord hnode) {
	
	hdltableformats hc = tableformatsdata;
	tybrowserinfo browserinfo;	
	short transform;
	long align;
	
	transform = 0;
	
	if (flhighlighted)
		transform = 0x4000; 
			
	align = atVerticalCenter + atHorizontalCenter;
	
	browsergetrefcon (hnode, &browserinfo);
	
	ploticonresource (r, align, transform, opgetheadicon (hnode));
	
#if 0
	transform += ttLabel [browserinfo.ixlabel]; // color it according to the file's label
	
	getgenericsystemicon (&browserinfo, &iconresnum);
	
	ploticonresource (&r, align, transform, iconresnum);
#endif
	} /%tabledrawnodeicon%/
*/

static void tableprepfordraw (Rect *r) {
	
	/*
	magic incantations that prepare for a callback to draw the text of a cell
	or a column title, or perhaps someday -- something else.
	*/
	
	register short h, v;
	
	/*
	if ((**tableformatsdata).flgridlines) { // leave the gridlines out of the rect
		
		(*r).top++;
		
		(*r).left++;
		}
	*/
	
	// display looks prettier if we leave a little whitespace
	
	h = (*r).left + texthorizinset; //pen position is independent of clipping
	
	v = (*r).top + globalfontinfo.ascent;
	
	movepento (h, v); /*position the pen for the callback routine*/
	} /*tableprepfordraw*/


static void tablesetcolumnbounds (Rect *r, short col) {
	
	(*r).left += tablesumcolwidths (0, col);
	
	(*r).right = (*r).left + tablegetcolwidth (col);
	} /*tablesetcolumnbounds*/

	
static void tablegettitlerect (short col, Rect *rtitle) {
	
	/*
	return the display rectangle of the title of the indicated column.  
	*/
	
	register hdltableformats hf = tableformatsdata;
	
	*rtitle = (**hf).titlerect;
	
	tablesetcolumnbounds (rtitle, col);
	} /*tablegettitlerect*/


static void tabledisplaytitle (short col, boolean flbitmap) {
	
	hdlhashtable ht;
	bigstring bs;
	Rect r;
	short fontstyle = bold;
	
	if (col == (**tableformatsdata).sorttitlecol)
		fontstyle |= underline;
	
	pushstyle ((**outlinedata).fontnum, (**outlinedata).fontsize, fontstyle);
	
	tablegetcursorinfo (&ht, bs, nil, nil);
	
	tablegettitlerect (col, &r); /*set left and right*/
	
	tableprepfordraw (&r); /*inset the rect, move QuickDraw's pen*/
	
	// insetrect (&r, 1, 1);
	
	if (flbitmap)
		flbitmap = openbitmap (r, (WindowPtr) getport ());
	
	#if TARGET_API_MAC_CARBON == 1
	
		pushpen ();
				
		setthemepen (kThemeBrushDialogBackgroundActive, r, true);
		
		paintrect (r);
		
		poppen ();
	
	#else
	
		eraserect (r);
		
	#endif
	
	tablegetstringlist (nametitlestring+col, bs);
	
	pendrawstring (bs);
	
	if (flbitmap)
		closebitmap ((WindowPtr) getport ());
	
	popstyle ();
	} /*tabledisplaytitle*/
	
	
void tableupdatecoltitles (boolean flupdating) {
	
	hdltableformats hf = tableformatsdata;
	Rect titlerect;
	register short i;
	
	if (flupdating) {
		
		titlerect = (**hf).titlerect;
		
		if (!shellrectneedsupdate (&titlerect)) /*empty intersection, nothing to do*/
			return;
		}
	
	for (i = 0; i < (**hf).ctcols; i++) 
		tabledisplaytitle (i, false);
	} /*tableupdatecoltitles*/


boolean tablechecksortorder (void) {
	
	hdltableformats hf = tableformatsdata;
	hdlhashtable ht;
	bigstring bs;
	
	if (tablegetcursorinfo (&ht, bs, nil, nil)) {
		
		if ((**ht).sortorder != (**hf).sorttitlecol) {
			
			(**hf).sorttitlecol = (**ht).sortorder;
			
			return (true);
			}
		}
	
	return (false);
	} /*tablechecksortorder*/

/*
static void tableupdateseparator (void) {
	
	/%
	we formalized the separator rect so we could easily experiment with different
	ideas for showing the boundary between the titles of a table display and
	the content of the table.
	%/
	
	Rect seprect;
	
	seprect = (**tableformatsdata).seprect;
	
	if (!shellrectneedsupdate (&seprect)) /%no intersection, nothing to do%/
		return;
	
	if ((**tableformatsdata).flprinting)
		return;
	
	eraserect (seprect);
	} /%tableupdateseparator%/
*/


static void tableupdategridlines (void) {
	
	/*
	draw the gridlines for the table, restrict drawing to the indicated rect.
	*/
	
	register hdltableformats hf = tableformatsdata;
	register hdloutlinerecord ho = outlinedata;
	short col;
	short h;
	Rect rtable;	
	
	rtable = (**ho).outlinerect;
	
	insetrect (&rtable, -1, -1); // expand for frame

	if (!shellrectneedsupdate (&rtable)) /*empty intersection, nothing to do*/
		return;
	
	pushpen (); /*gridlines are drawn in gray*/
	
	setgraypen ();
		
	pushbackcolor (&(**outlinedata).backcolor);

	frame3sides (rtable);
	
	#if TARGET_API_MAC_CARBON == 1 /*the frame3sides routine drew over the scroll bar.*/
	
		drawscrollbar ((**shellwindowinfo).vertscrollbar);
		 
	#endif
	
	insetrect (&rtable, 1, 1); // back to original

	eraserect (rtable);
	
	popbackcolor ();
	
	h = rtable.left;
	
	for (col = namecolumn; col < kindcolumn; col++) { /*draw a vertical line*/
		
		h += (**hf).colwidths [col];
		
		movepento (h, rtable.top);
	
		pendrawline (h, rtable.bottom);		
		}
	
	poppen (); /*finished drawing the gridlines*/
	} /*tableupdategridlines*/


void tableupdate (void) {
	
	hdltableformats hf = tableformatsdata;
	
	if (hf == NULL)
		return;

	if (outlinedata == NULL)
		return;
	
	if (!isclaydisplay (hf)) {
		
		#if !TARGET_API_MAC_CARBON
		
			tableupdatecoltitles (true);
		
		#endif
		
	//	tableupdateseparator ();
		
		tableupdategridlines ();
		
		#if TARGET_API_MAC_CARBON == 1
		
			tableupdatecoltitles (true);
			
		#endif
		}
	
	opupdate ();

	//tabledrawzoombutton (false);
	
	tableupdatekindpopup ();
	
	tableupdatesortpopup ();
	
	#if TARGET_API_MAC_CARBON == 1
	
		shelldrawwindowmessage (shellwindowinfo);
	
	#endif
	} /* tableupdate */


static short tablewidth2cols (short width) {
	
	/*
	return the number of cols, starting with startcol, that are required to
	completely span width pixels
	*/
	
	register short *pwidth;
	short col;
	hdltableformats hf = tableformatsdata;
	short ctcols = (**hf).ctcols;
	
	for (col = 0, pwidth = (**hf).colwidths;  (width > 0) && (col < ctcols);  ++col)
		width -= *pwidth++;
	
	return (col);
	} /*tablewidth2cols*/


static void tablegetcellrect (hdlheadrecord hnode, const Rect *rrow, short col, Rect *rcell) {
	
	/*
	return the display rectangle of the indicated column.  
	*/
	
	short optextindent = opnodeindentpix (hnode) + opiconindentpix ();
	
	/*start with the row rect, which is really the textrect*/
	
	*rcell = *rrow;
	
	/*calculate the horizontal coordinates of the cell*/
	
	(*rcell).left -= optextindent; // get back to linerect, but with text insets
	
	tablesetcolumnbounds (rcell, col);
	
	(*rcell).right -= texthorizinset;
	
	if (col == 0)
		(*rcell).left += optextindent;
	} /*tablegetcellrect*/


static Rect titlerect;

static void tableinverttitle (boolean flinrectnow) {
#pragma unused (flinrectnow)

	invertrect (titlerect);
	} /*tableinverttitle*/


boolean tabletitleclick (Point pt) {
	
	/*
	5.0.2b20 dmb: set style when measuring title text
	*/
	
	hdltableformats hf = tableformatsdata;
	hdloutlinerecord ho = outlinedata;
	short dh;
	short col;
	bigstring bs;
	
	dh = pt.h - (**hf).titlerect.left;
	
	col = tablewidth2cols (dh + 1) - 1;
	
	tablegettitlerect (col, &titlerect); /*set left and right*/
	
	tablegettitlestring (col, bs);
	
//	tablesettitlestyle (col);
	
	pushstyle ((**ho).fontnum, (**ho).fontsize, bold);
	
	titlerect.right = titlerect.left + stringpixels (bs) + 4;
	
	popstyle ();
	
	insetrect (&titlerect, 1, 1);
	
	if (pointinrect (pt, titlerect))
		if (trackicon (titlerect, &tableinverttitle)) {
			
			hdlhashtable ht;
			bigstring lbs;
			
			tablegetcursorinfo (&ht, lbs, nil, nil);
			
			tablesetsortorder (ht,  col);
			}
	
	return (true);
	} /*tabletitleclick*/


boolean tablefindcolumnguide (Point pt, short *col) {
	
	register hdltableformats hf = tableformatsdata;
	short lastcol = (**hf).ctcols - 1;
	short h;
	short c;

	if (outlinedata == NULL)
		return (false);

	if (isclaydisplay (hf))
		return (false);
	
	h = (**outlinedata).outlinerect.left + 1;

	for (c = 0; c < lastcol; c++) {
		
		h += tablegetcolwidth (c);
		
		if (abs (pt.h - h) <= 2) {
			
			*col = c;
			
			return (true);
			}
		} /*for*/
		
	return (false); /*not pointing inside between columns*/
	} /*tablefindcell*/
	
	
boolean tableadjustcolwidth (Point ptstart, short col) {
	
	/*
	5.0.2b20 dmb: get maxwidths before resize; set editbufferrect after resize
	*/
	
	register hdltableformats hf = tableformatsdata;
	register short h;
	Rect rtable, rcell;
	Point pt;
	short linetop, linebottom, lineh;
	short lasth;
	short ctcolsafter;
	short minh, maxh;
	short diff, colwidth, newwidth, leftoverwidth;
	short ctcols = (**hf).ctcols;
	long width, height;
	
	tablegetoutlinesize (&width, &height); // recalcs max col widths
	
	rtable = (**outlinedata).outlinerect;
	
	linetop = rtable.top + 1;
	
	linebottom = rtable.bottom - 1;
	
	rcell = rtable;
	
	tablesetcolumnbounds (&rcell, col);
	
	lineh = rcell.right; /*when mouse moves, erase line at last h position*/
	
	minh = rcell.left + mincolwidth;
	
	ctcolsafter = (**hf).ctcols - col - 1;
	
	maxh = min (rcell.left + maxcolwidth, rtable.right - ctcolsafter * mincolwidth);
	
	h = lasth = ptstart.h;
	
	if (mousedoubleclick ()) { /*move column line to optimal width*/
		
		ptstart.h = rcell.right;
		
		if (col == 0)
			h = rcell.left + (**hf).maxwidths [col];
		
		else  // col == 1, but we'll size according to kind width
			h = rtable.right - (**hf).maxwidths [2];
		
		h = min (maxh, max (minh, h));
		}
	else while (mousestilldown () || shiftkeydown ()) { /*drag the column line*/
		
		getmousepoint (&pt);
		
		if (!pointinrect (pt, (**tableformatswindowinfo).contentrect))
			pt = ptstart; /*snap back*/
		
		h = pt.h; /*copy into register*/
		
		h = max (h, minh);
		
		h = min (h, maxh);
		
		if (h != lasth) {
			
			movepento (lineh, linetop); /*erase the last line*/
			
			graydrawline (lineh, linebottom);
			
			lineh += (h - lasth);
			
			movepento (lineh, linetop); /*show the new line*/
			
			graydrawline (lineh, linebottom);
			
			lasth = h;
			}
		} /*while*/
	
	diff = h - ptstart.h;
	
	if (diff == 0)
		return (false);
	
	// actually set the new width
	
	newwidth = rcell.right - rcell.left + diff;
	
	tablesetcolwidth (col, newwidth, true);
	
	for (++col; col < ctcols; ++col) {
		
		colwidth = tablegetcolwidth (col);
		
		leftoverwidth = tableavailwidth () - tablesumcolwidths (0, ctcols);
		
		tablesetcolwidth (col, colwidth + leftoverwidth, true);
		}
	
	eraserect ((**hf).titlerect);
	
	tableupdatecoltitles (false);
	
	opseteditbufferrect (); /*if in edit mode, adjust display area*/
	
	opsmashdisplay ();
	
	tabledirty ();
	
	return (true);
	} /*tableadjustcolwidth*/


boolean tablegettextrect (hdlheadrecord hnode, const Rect *linerect, Rect *textrect) {
	
	hdltableformats hf = tableformatsdata;
	
	if (isclaydisplay (hf))
		return (claygettextrect (hnode, linerect, textrect));
	
	if (!opdefaultgettextrect (hnode, linerect, textrect))
		return (false);
	
	/*
	offsetrect (textrect, 0, -1);
	
	(*textrect).bottom++;
	*/
	
	return (true);
	} /*tablegettextrect*/


boolean tablegetedittextrect (hdlheadrecord hnode, const Rect *linerect, Rect *textrect) {
	
	hdltableformats hf = tableformatsdata;
	short col;
	
	if (isclaydisplay (hf))
		return (claygetedittextrect (hnode, linerect, textrect));
	
	if ((**hf).editnode == nil) // between edits, anticipating focus
		col = (**hf).focuscol;
	else						// currently editing
		col = (**hf).editcol;
	
	tablegettextrect (hnode, linerect, textrect);
	
	tablegetcellrect (hnode, textrect, col, textrect);
	
	if (col > namecolumn)
		(*textrect).left += texthorizinset;
	
	(*textrect).top += textvertinset;
	
	return (true);
	} /*tablegetedittexrect*/


boolean tablegeticonrect (hdlheadrecord hnode, const Rect *linerect, Rect *iconrect) {
	
	Rect r;
	Rect rcontains = *linerect;
	
	if (isclaydisplay (tableformatsdata))
		return (claygeticonrect (hnode, linerect, iconrect));
	
	r.left = (*linerect).left + opnodeindent (hnode);
	
	r.right = r.left + (**outlinedata).iconwidth;
	
	r.top = (*linerect).top;
	
	r.bottom = r.top + (**outlinedata).iconheight;
	
	rcontains.left = r.left;
	
	rcontains.right = r.right;
	
	centerrect (&r, rcontains); /*center it vertically*/
	
	*iconrect = r;
	
	return (true);
	} /*tablegeticonrect*/
	

boolean tablepushnodestyle (hdlheadrecord hnode) {
	
	hdltableformats hf = tableformatsdata;
	tybrowserinfo info;
	short style;
	
	if (isclaydisplay (hf))
		return (claypushnodestyle (hnode));
	
	if (false) // (**hf).filenamebold)
		style = bold;
	else
		style = 0;
	
	browsergetrefcon (hnode, &info);

	/*
	if (info.flalias)		
		style += italic;
	*/
	
	if ((**hnode).flnodeunderlined) 
 		style += underline;
 	
	pushstyle ((**outlinedata).fontnum, (**outlinedata).fontsize, style);
	
	return (true);
	} /*tablepushnodestyle*/


boolean tablegetlineheight (hdlheadrecord hnode, short *lh) {
	
	if (isclaydisplay (tableformatsdata))
		return (claygetlineheight (hnode, lh));
	
	opdefaultgetlineheight (hnode, lh);
	
	--*lh;
	
	return (true);
	} /*tablegetlineheight*/


boolean tablegetlinewidth (hdlheadrecord hnode, short *lw) {
	
	if (isclaydisplay (tableformatsdata))
		return (claygetlinewidth (hnode, lw));
	
	*lw = tabletotalcolwidths ();

	return (true);
	} /*tablegetlinewidth*/
	

boolean tablegetcellstring (hdlheadrecord hnode, short col, bigstring bs, boolean fllimitbinaries) {
	
	/*
	1/8/91 dmb: push/pophashtable around call to hashvaltostrings, since 
	we can no longer assume that the root table is always set

	call hashgetvaluestring, _or_ hashgettype/size string; not hashvaltostrings
	
	5.0.2b3 dmb: user tablepush/popcontext; don't just push our table as a context
	*/
	
	bigstring bstype, bssize;
	tyvaluerecord val;
	tybrowserspec fs;
	hdlhashnode hhashnode;
	
//	tablegetstringlist (questionmarksstring, bs);
	
	if (col == namecolumn) {
		
		if (!opgetheadstring (hnode, bs))
			return (false);
		
		//omit xml prefix
		if ((stringlength (bs) > 5) && getstringcharacter (bs, 4) == chtab)
			deletestring (bs, 1, 5);
		else
			if ((stringlength (bs) > 9) && getstringcharacter (bs, 8) == chtab)
				deletestring (bs, 1, 9);
		
		return (true);
		}
	
	if (!claygetfilespec (hnode, &fs))
		return (false);
	
	if (!claylookupvalue (&fs, &val, &hhashnode))
		return (false);
	
	if (col == valuecolumn)  {
		
		hdlhashtable ht = fs.parID;
		
		shellpusherrorhook ((errorhookcallback) &falsenoop);
		
		tablepushcontext (ht, val.valuetype); // pushhashtable (fs.parID);
		
		hashgetvaluestring (val, bs); /*accept error values on failure*/
		
		tablepopcontext (ht, val.valuetype); // pophashtable ();
		
		shellpoperrorhook ();
		
		if (fllimitbinaries && val.valuetype == binaryvaluetype)
			setstringlength (bs, min (stringlength (bs), binarycharacterstomeasure));
		
		return (true); /*with bs set to the value of the variable*/
		}
	
	hashgettypestring (val, bstype);
	
	hashgetsizestring (&val, bssize);
	
	if (stringlength (bssize) > 0) { /*non-trivial size, show it*/
		
		pushstring ("\x02" " [", bstype);
		
		pushstring (bssize, bstype);
		
		pushchar (']', bstype);
		}
	
	copystring (bstype, bs);
	
	return (true);
	} /*tablegetcellstring*/


static boolean tablegetmaxcolwidthvisit (hdlheadrecord hnode, short col) {
	
	bigstring bs;
	short pix;
	short *pmax;
	
	if (!tablegetcellstring (hnode, col, bs, true)) /*error*/
		return (false);
	
	pix = stringpixels (bs);
	
	if (col == namecolumn)
		pix += opnodeindentpix (hnode);

	pmax = (**tableformatsdata).maxwidths + col;
	
	*pmax = max (pix, *pmax);
	
	return (true);
	} /*tablegetmaxcolwidthvisit*/
	

static boolean tablegetmaxwidthvisit (hdlheadrecord hnode, ptrvoid refcon) {
#pragma unused (refcon)

	short col;
	
	if ((**hnode).flexpanded) {
		
		for (col = namecolumn; col <= kindcolumn; ++col)
			tablegetmaxcolwidthvisit (hnode, col);
		
		return (true);
		}
	
	return (true);
	} /*tablegetmaxwidthvisit*/


boolean tablegetoutlinesize (long *width, long *height) {
	
	/*
	6.0a11 dmb: watch for nil outline
	*/

	hdloutlinerecord ho = outlinedata;
	hdltableformats hf = tableformatsdata;
	short col;
	long totalwidth = 0;
	
	if (ho == nil) {
		
		*width = 0;

		*height = 0;

		return (false);
		}

	*height = (**ho).outlinerect.top - (**hf).wholerect.top; /*to the top of the table...*/
	
	*height += opsumalllineheights ();
	
	for (col = namecolumn; col <= kindcolumn; ++col)
		(**hf).maxwidths [col] = 0;
	
	oppushstyle (ho);
	
	opsiblingvisiter ((**outlinedata).hsummit, false, &tablegetmaxwidthvisit, nil);
	
	(**hf).maxwidths [namecolumn] += opiconindentpix ();
	
	for (col = namecolumn; col <= kindcolumn; ++col)
		totalwidth += ((**hf).maxwidths [col] += pixelsbetweencolumns);
	
	popstyle ();
	
//	totalwidth += opiconindentpix ();
	
	*width = totalwidth;
	
	return (true);
	} /*tablegetoutlinesize*/


boolean tabledefaultdrawcell (hdlheadrecord hnode, short col, const Rect *rcell) {
	
	register hdltableformats hf = tableformatsdata;
	bigstring bs;
	
	if (opeditingtext (hnode) && ((**hf).editnode == hnode) && ((**hf).editcol == col)) {
		
		opeditupdate ();
		
		return (true);
		}
	
	/*
	if (col == valuecolumn)
		pushbackcolor (&lightgraycolor);
	
	eraserect (*rcell);
	
	if (col == valuecolumn)
		popbackcolor ();
	*/
	
	if (!tablegetcellstring (hnode, col, bs, false))
		return (false);
	
	ellipsize (bs, (*rcell).right - (*rcell).left);
	
	pendrawstring (bs);
	
	return (true);
	} /*tabledefaultdrawcell*/


static void tabledrawcell (hdlheadrecord hnode, short col, const Rect *rtext, boolean flselected, boolean flinverted) {
	
	Rect rcell;
	//hdloutlinerecord ho = outlinedata;
	//boolean fltextmode = (**ho).fltextmode;
	
	/*
	if (col == namecolumn) {
	
		tablepushnodestyle (hnode);
		
		if ((**tableformatsdata).editcol != namecolumn)
			(**ho).fltextmode = false;
		
		tablegetcellrect (hnode, rtext, col, &rcell);
		
		pushclip (rcell);
		
		opdefaultdrawtext (hnode, rtext, false, flinverted);
		
		popclip ();
		
		(**ho).fltextmode = fltextmode;
		
		popstyle ();
		}
	else {
	*/
		tablegetcellrect (hnode, rtext, col, &rcell);
		
		pushclip (rcell);

		rcell.top += textvertinset;
		
		if (col == namecolumn)
			rcell.left -= texthorizinset;
		
		tableprepfordraw (&rcell);

		tabledefaultdrawcell (hnode, col, &rcell);

		popclip ();
	/*
		}
	*/

	if (flselected) {
		short optextindent = opnodeindentpix (hnode) + opiconindentpix ();
		
		rcell = *rtext;
		
		rcell.left -= optextindent; // get back to linerect, but with text insets
		
		rcell.top += textvertinset;
		
		rcell.bottom -= textvertinset - 1;
		
		tablesetcolumnbounds (&rcell, col);
		
		if (col == namecolumn)
			rcell.left += opnodeindentpix (hnode);
		
		rcell.right -= 3;
		
		if (flinverted)
			invertrect (rcell);
		else
			grayframerect (rcell);
		}
	} /*tabledrawcell*/ 


boolean tablepredrawline (hdlheadrecord hnode, const Rect *linerect, boolean flselected, boolean flinverted) {
	
	short ctcols = (**tableformatsdata).ctcols;
	Rect colrect = *linerect;
	short col;
	
//	pushbackcolor (&(**outlinedata).backcolor);
	
	if (isclaydisplay (tableformatsdata))
		claypredrawline (hnode, linerect, flselected, flinverted);
	else
	
	for (col = 0; col < ctcols; col++) {
		
		colrect = *linerect;
		
		tablesetcolumnbounds (&colrect, col);
		
		insetrect (&colrect, 1, 0);
		
		if (!(**outlinedata).flprinting)
			eraserect (colrect);
		}
	
	#ifdef tablehorizlines
		movepento ((*linerect).left, (*linerect).bottom - 1);
		graydrawline ((*linerect).right - 1, (*linerect).bottom - 1);
	#endif
	
//	popbackcolor ();
	
	return (true);
	} /*tablepredrawline*/

boolean tablepostdrawline (hdlheadrecord hnode, const Rect *linerect, boolean flselected, boolean flinverted) {

	if (isclaydisplay (tableformatsdata))
		return (claypostdrawline (hnode, linerect, flselected, flinverted));
	
	return (true);
	} /*tablepostdrawline*/


boolean tabledrawline (hdlheadrecord hnode, const Rect *textrect, boolean flselected, boolean flinverted) {
	
	hdltableformats hf = tableformatsdata;
//	RGBColor rgb = (**hf).filenamecolor;
//	hdloutlinerecord ho = outlinedata;
	
//	tablepushnodestyle (hnode);
	
//	pushforecolor (&rgb);
	
	if (isclaydisplay (hf))
		claydrawline (hnode, textrect, flselected, flinverted);
	else {
		short col;
		
		for (col = 0; col < (**hf).ctcols; col++) 
			tabledrawcell (hnode, col, textrect, flselected, flinverted);
		}
	
//	popforecolor ();

//	popstyle ();
	
	return (true);
	} /*tabledrawline*/


void browserdrawnodeicon (const Rect *r, boolean flhighlighted, hdlheadrecord hnode) {
	
	tybrowserinfo browserinfo;	
	short transform = 0;
	long align = 0;
	
	if (flhighlighted)
		transform = 0x4000; 
	
#ifdef MACVERSION
	align = atVerticalCenter + atHorizontalCenter;
#endif

	browsergetrefcon (hnode, &browserinfo);
	
	ploticonresource (r, align, transform, opgetheadicon (hnode));
	
#if 0
	transform += ttLabel [browserinfo.ixlabel]; /*color it according to the file's label*/
	
	getgenericsystemicon (&browserinfo, &iconresnum);
	
	ploticonresource (&r, align, transform, iconresnum);
#endif
	} /*browserdrawnodeicon*/


boolean tabledrawnodeicon (hdlheadrecord hnode, const Rect *iconrect, boolean flselected, boolean flinverted) {
	
	if (fldisplaydebug) {
	
		if (optionkeydown ())
			return (true);
		}
	
	if (isclaydisplay (tableformatsdata))
		return (claydrawnodeicon (hnode, iconrect, flselected, flinverted));
	
	browserdrawnodeicon (iconrect, flselected, hnode);
	
	return (true);
	} /*tabledrawnodeicon*/


static short tablefindcolumn (hdlheadrecord hnode, Point pt, const Rect *textrect) {
	
	hdltableformats hf = tableformatsdata;
	short *pwidth;
	short leftedge;
	short col = 0;
	
	if (pt.h < (*textrect).left - textleftslop)
		return (-1);
	
	leftedge = (*textrect).left - opnodeindentpix (hnode) - opiconindentpix ();
	
	/*loop through range in width array, adding values*/
	
	for (col = 0, pwidth = (**hf).colwidths;  ;  ++col, ++pwidth) {
	
		leftedge += *pwidth;
		
		if (leftedge > pt.h)
			break;
		
		if (col == kindcolumn)
			break;
		}
	
	return (col);
	} /*tablefindcolumn*/


boolean tableadjustcursor (hdlheadrecord hnode, Point pt, const Rect *textrect) {
	
	short col;
	tycursortype cursortype = cursorisarrow;
	hdlhashtable htable;
	bigstring bs;
	tyvaluerecord val;
	hdlhashnode hhashnode;
	
	col = tablefindcolumn (hnode, pt, textrect);
	
	switch (col) {
		
		case namecolumn:
			cursortype = cursorisibeam;
			break;
		
		case valuecolumn:
			if (tablegetiteminfo (hnode, &htable, bs, &val, &hhashnode) && (val.valuetype != externalvaluetype))
				cursortype = cursorisibeam;
			
			break;
		
		case kindcolumn:
			break;
		}
	
	setcursortype (cursortype);
	
	return (true);
	} /*tableadjustcursor*/


boolean tablemouseinline (hdlheadrecord hnode, Point pt, const Rect *textrect, boolean *flintext) {
	
	/*
	5.0d19 dmb: zoom item on value column 2click
	*/
	
	hdltableformats hf = tableformatsdata;
	short col;
	hdlhashnode hhashnode;
	
	col = tablefindcolumn (hnode, pt, textrect);
	
	switch (col) {
		
		case namecolumn:
			break;
		
		case valuecolumn: {
			hdlhashtable htable;
			bigstring bs;
			tyvaluerecord val;
			
			if (tablegetiteminfo (hnode, &htable, bs, &val, &hhashnode) && (val.valuetype == externalvaluetype)) {
				
				*flintext = false;

				if (mousedoubleclick ()) {
					
					tablezoomfromhead (hnode);
					
					setmousedoubleclickstatus (false);
					
					return (false);
					}

				return (true);
				}
			
			break;
			}
		
		default:
			*flintext = false;

			return (true);
		}
	
	(**hf).focuscol = col;
	
	if (col != (**hf).editcol)
		(**outlinedata).flcursorneedsdisplay = true; /*make sure opmoveto does something*/
	
	*flintext = true;

	return (true);
	} /*tablemouseinline*/


/*
boolean tablereturnkey (tydirection dir) {
	
	hdloutlinerecord ho = outlinedata;
	hdlheadrecord hpre = (**ho).hbarcursor;
	tybrowserspec fspre;
	hdlhashtable htable;
	
	if (!claygetfilespec (hpre, fspre))
		return (false);
	
	if (dir == right)
		claygetdirid (&fspre, &htable);
	else
		htable = fspre.dirid;
	
	getparent
	return (opinsertheadline (emptystring, dir, flcomment));
	} /%tablereturnkey%/
*/

boolean tableinitdisplay (void) {
	
	return (true);
	} /*tableinitdisplay*/




