
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

#include "font.h"
#include "icon.h"
#include "ops.h"
#include "quickdraw.h"
#include "strings.h"
#include "timedate.h"
#include "op.h"
#include "opdisplay.h"
#include "opicons.h"
#include "opinternal.h"
#include "langinternal.h"
#include "tableformats.h"
#include "tableinternal.h"
#include "tablestructure.h"
#include "claybrowser.h"
//#include "clayicons.h"


typedef struct tydisklinelayout {
	
	short recordsize;
	
	byte claydisplay; //if true, we're actually displaying in clay mode
	
	byte realicons; /*if true, we go into the desktop db for icons, otherwise use generic ones*/
	
	byte filenamebold; /*if true, the file name is drawn in bold*/
	
	byte includeline2; /*if true we include the 2nd line of the display*/
	
	byte includedate; /*if true, we include the modification date*/
	
	byte includeframe; /*if true, each line has a frame, if false it's just like an outliner*/
	
	byte iconsize; /*full-size, small or very small icons*/
	
	RGBColor fillcolor, framecolor, cursorcolor, filenamecolor, othertextcolor; 
	
	RGBColor backcolor, statuscolor;
	
	diskfontstring filenamefont, othertextfont;
	
	short filenamefontsize, othertextfontsize;
	} tydisklinelayout;


#define fldisplaydebug false

#define drawinset 4

#define spacebetweenlines 1

#define vertspacebetweenframes 5

#ifdef claydialoginclude
	static hdlappwindow targetwindow = nil;
		
	static hdlcard layoutcard = nil;

	static void resetfilefontobjects (tylinelayout *); /*forward declaration*/
#endif


#define disktomemcolor(rgb) do {disktomemshort (rgb.red); \
	disktomemshort (rgb.green); \
	disktomemshort (rgb.blue); } while (0)

#define memtodiskcolor(rgb) do {memtodiskshort (rgb.red); \
	memtodiskshort (rgb.green); \
	memtodiskshort (rgb.blue); } while (0)

#if 0

static boolean midcolorof (Rect r, RGBColor *rgb1, RGBColor *rgb2, RGBColor *rgbmid) {
	
	#ifdef MACVERSION
		GDHandle hgd;
		
		*rgbmid = *rgb2;
		
		hgd = GetMaxDevice (&r);
		
		return (GetGray (hgd, rgb1, rgbmid));
	#endif

	#ifdef WIN95VERSION
		(*rgbmid).red = ((long) (*rgb1).red + (*rgb2).red) / 2;
		(*rgbmid).green = ((long) (*rgb1).green + (*rgb2).green) / 2;
		(*rgbmid).blue = ((long) (*rgb1).blue + (*rgb2).blue) / 2;

		return (true);
	#endif
	} /*midcolorof*/


static void lightcolorof (RGBColor *rgb) {
	
	(*rgb).red += (65535 - (*rgb).red) / 3;
	
	(*rgb).green += (65535 - (*rgb).green) / 3;
	
	(*rgb).blue += (65535 - (*rgb).blue) / 3;
	} /*lightcolorof*/

#endif


static void darkcolorof (RGBColor *rgb) {
	
	(*rgb).red -= (*rgb).red / 4;
	
	(*rgb).green -= (*rgb).green / 4;
	
	(*rgb).blue -= (*rgb).blue / 4;
	} /*darkcolorof*/


/*static boolean claypushtextstyle (diskfontstring fontname, short fontsize) {
	
	bigstring bs;
	short fontnum;
	
	copystring (fontname, bs);
	
	fontgetnumber (bs, &fontnum);
	
	return (pushstyle (fontnum, fontsize, 0));
	} /%claypushtextstyle%/
*/


void claysmashoutlinefields (hdlwindowinfo appwindow, hdltableformats hf) {

	/*
	bind the outline's font and size to the font/size from the
	layout record.
	
	10/14/93 DW: set outline backcolor too.
	*/ 
	
	hdloutlinerecord ho = (**hf).houtline;	
	
	if (ho != nil) { /*has an outline linked in*/ 
	
		//bigstring bs;
		//short fontnum;
		//copystring ((**hf).linelayout.filenamefont, bs);
		//fontgetnumber (bs, &fontnum);
		//(**ho).fontnum = fontnum;
		
		(**ho).fontnum = (**hf).linelayout.filenamefont;
		
		(**ho).fontsize = (**hf).linelayout.filenamefontsize;
		
		(**ho).backcolor = (**hf).linelayout.backcolor;
		
		oppoststylechange ();
		
		//(**appwindow).backcolor = (**hf).linelayout.backcolor;
		}
	} /*claysmashoutlinefields*/

	
static void smashlayoutfields (hdltableformats hf) {

	/*
	bind the layout's font and size to the font/size from the
	outline record.
	
	10/14/93 DW: set outline backcolor too.
	
	10/9/5 DW: Yuck! Made a mistake -- instead of "ho" it was
	"outlinedata". Skanky hard to trace in debugger. Gotcha!
	*/ 
		
	hdloutlinerecord ho = (**hf).houtline;	
	
	/*
	bigstring bs;
	
	fontgetname ((**ho).fontnum, bs);
	
	copystring (bs, (**hf).linelayout.filenamefont);
	*/
	
	(**hf).linelayout.filenamefont = (**ho).fontnum;
	
	(**hf).linelayout.filenamefontsize = (**ho).fontsize;
	
	(**hf).linelayout.backcolor = (**ho).backcolor;
	} /*smashlayoutfields*/

	
static void setcomputedfields (hdltableformats hf) {
	
	hdloutlinerecord ho = (**hf).houtline;
	
	smashlayoutfields (hf); /*make the layout agree with the outline*/
	
	#ifdef claydialoginclude
		if (layoutcard != nil) { /*the card is open*/
			
			tylinelayout layout = (**hf).linelayout;
			
			#ifdef claydialoginclude
				resetfilefontobjects (&layout);
			#endif
			}
	#endif

	switch ((**hf).linelayout.iconsize) {
		
		case fullsizeicon:
			(**ho).iconwidth = 32; 
	
			(**ho).iconheight = 32; 
			
			break;
		
		case smallsizeicon: 	
			(**ho).iconwidth = 16; 
	
			(**ho).iconheight = 16; 
			
			break;
			
		case verysmallsizeicon:
			(**ho).iconwidth = 12;
			
			(**ho).iconheight = 12;
			
			break;			
		} /*switch*/
	
	(**ho).lineindent = (**ho).iconwidth + drawinset;
	
	/*compute display info for file names*/ {
		
		short style = 0;
		
		if ((**hf).linelayout.filenamebold)
			style = bold;
			
		pushstyle ((**outlinedata).fontnum, (**outlinedata).fontsize, style);
	
		(**hf).computedlineinfo.filenamelineheight = globalfontinfo.ascent + globalfontinfo.descent;
		
		#ifdef MACVERSION
			(**hf).computedlineinfo.filenamewidth = (globalfontinfo.widMax * 31) / 2;
		#endif
		#ifdef WIN95VERSION
			(**hf).computedlineinfo.filenamewidth = stringpixels ("\x02" "Wi") * 16;
		#endif

		popstyle ();
		}
	
	/*compute display info for other text*/ {
	
		pushstyle ((**hf).linelayout.othertextfont, (**hf).linelayout.othertextfontsize, normal);
	
		(**hf).computedlineinfo.othertextlineheight = globalfontinfo.ascent + globalfontinfo.descent;
	
		(**hf).computedlineinfo.datewidth = stringpixels ("\x08" "11:59 AM");
	
		(**hf).computedlineinfo.datewidth += (**hf).computedlineinfo.datewidth / 3;
	
		popstyle ();
		}
	} /*setcomputedfields*/


void claybrowserinitdraw (void) {
	
	setcomputedfields (tableformatsdata);
	} /*claybrowserinitdraw*/


static boolean hasframe (void) {
	
	hdltableformats hf = tableformatsdata;
	
	return ((**hf).linelayout.includeframe);
	} /*hasframe*/


static short getframeheight (void) {
	
	hdltableformats hf = tableformatsdata;
	short height, minheight;

	height = (**outlinedata).iconheight;
	
	if (hasframe ())
		height += (2 * drawinset);
	else
		height += drawinset; /*a little extra space between lines*/
	
	minheight = (**hf).computedlineinfo.filenamelineheight;
	
	if ((**hf).linelayout.includeline2) {
	
		minheight += (**hf).computedlineinfo.othertextlineheight + spacebetweenlines;
		
		minheight += (2 * drawinset);
		}
	
	if (height < minheight)
		height = minheight;
		
	return (height);
	} /*getframeheight*/
	

static short getframewidth (void) {
	
	hdltableformats hf = tableformatsdata;
	short width;
	
	width = drawinset + (**outlinedata).iconwidth + (2 * drawinset) + 
	
		(**hf).computedlineinfo.filenamewidth;
	
	if ((**hf).linelayout.includedate)
		width += (**hf).computedlineinfo.datewidth;
		
	return (width);
	} /*getframewidth*/
	
	
static boolean simpleoutlinerdisplay (void) { /*DW 4/7/95*/

	hdltableformats hf = tableformatsdata;
	tylinelayout lo = (**hf).linelayout;
		
	if ((!lo.includeline2) && (!lo.includedate) && (!lo.includeframe))
		return (true);
			
	return (false);
	} /*simpleoutlinerdisplay*/
	

boolean claygettextrect (hdlheadrecord hnode, const Rect *linerect, Rect *textrect) {
	
	hdltableformats hf = tableformatsdata;
	Rect r;
	
	r.left = (*linerect).left + opnodeindent (hnode) + drawinset + (**outlinedata).iconwidth + drawinset;
	
	if (simpleoutlinerdisplay ()) 
		r.right = (*linerect).right - texthorizinset;
	else {
		if (hasframe ()) /*it looks better with a little more space separating the icon and the name*/
			r.left += drawinset;
	
		r.right = r.left + (**hf).computedlineinfo.filenamewidth;
		}
	
	r.top = (*linerect).top;
	
	r.bottom = r.top + (**hf).computedlineinfo.filenamelineheight;
	
	/*center the text vertically*/ {
	
		short dv;
		
		if ((**hf).linelayout.includeline2) {
			
			short combinedlineheight = (**hf).computedlineinfo.filenamelineheight + (**hf).computedlineinfo.othertextlineheight;
			
			dv = (getframeheight () - combinedlineheight - spacebetweenlines);
			}
		else 
			dv = (getframeheight () - (**hf).computedlineinfo.filenamelineheight);
			
		dv /= 2;
		
		if ((dv % 2) == 1) /*prefer to add extra pixel at top, not bottom*/
			dv++;
		
		if (dv > 0) 		
			offsetrect (&r, 0, dv);
		}
		
	*textrect = r;
	
	return (true);
	} /*claygettextrect*/
	

boolean claygetedittextrect (hdlheadrecord hnode, const Rect *linerect, Rect *textrect) {
	
	//claypushnodestyle (hnode);
	
	claygettextrect (hnode, linerect, textrect);
	
	//popstyle ();
	
	return (true);
	} /*claygetedittextrect*/


boolean claygeticonrect (hdlheadrecord hnode, const Rect *linerect, Rect *iconrect) {
	
	Rect r;
	
	r.left = (*linerect).left + opnodeindent (hnode);
	
	r.right = r.left + (**outlinedata).iconwidth;
	
	r.top = (*linerect).top;
	
	r.bottom = r.top + (**outlinedata).iconheight;
	
	if (hasframe ()) 
		offsetrect (&r, drawinset, drawinset);
	else {
		Rect rcontains = *linerect;
		
		rcontains.left = r.left;
		
		rcontains.right = r.right;
		
		centerrect (&r, rcontains); /*center it vertically*/
		}
	
	*iconrect = r;
	
	return (true);
	} /*claygeticonrect*/
	

boolean claypushnodestyle (hdlheadrecord hnode) {
	
	hdltableformats hf = tableformatsdata;
	tybrowserinfo info;
	
	short style;
	
	if ((**hf).linelayout.filenamebold)
		style = bold;
	else
		style = 0;
		
	browsergetrefcon (hnode, &info);

	#if filebrowser
		if (info.flalias)		
			style += italic;
	#endif
	
	if ((**hnode).flnodeunderlined) 
 		style += underline;
 	
	pushstyle ((**outlinedata).fontnum, (**outlinedata).fontsize, style);
	
	return (true);
	} /*claypushnodestyle*/


boolean claygetlineheight (hdlheadrecord hnode, short *lh) {
	
	*lh = getframeheight ();
	
	if (hasframe ())
		*lh += vertspacebetweenframes;
	
	return (true);
	} /*claygetlineheight*/


boolean claygetlinewidth (hdlheadrecord hnode, short *lw) {
	
	//claypushnodestyle (hnode);

	boolean fl = opdefaultgetlinewidth (hnode, lw);

	//popstyle ();
	
	return (fl);
	} /*claygetlinewidth*/
	
	
boolean claygetnodeframe (hdlheadrecord hnode, Rect *r) {
	
	(*r).left += opnodeindent (hnode);
	
	if (simpleoutlinerdisplay ()) {
		
		short lw;
		
		claygetlinewidth (hnode, &lw);
		
		(*r).right = (*r).left + drawinset + (**outlinedata).iconwidth + drawinset + lw;
		}
	else
		(*r).right = (*r).left + getframewidth ();
	
	(*r).bottom = (*r).top + getframeheight ();
	
	return (true);
	} /*claygetnodeframe*/
	
	
static void getfatdatestring (long modtime, bigstring bs) {
	
	/*
	actually we get a very slim date string.
	*/
	
	timetodatestring (modtime, bs, false); /*9/8/93*/
	} /*getfatdatestring*/
	
	
static void getfattimestring (long modtime, bigstring bs) {
	
	/*
	actually we get a very slim time string.
	*/
	
	timetotimestring (modtime, bs, false); /*9/8/93*/
	} /*getfattimestring*/
	
	
#if 0

static void getfatsizestring (tybrowserinfo *browserinfo, bigstring bs) {
	
	setemptystring (bs);
		
	if ((*browserinfo).flfolder) {
		
		short ctfiles = (*browserinfo).filesize;
		
		shorttostring (ctfiles, bs);
		
		pushstring ("\x05" " item", bs);
		
		if (ctfiles != 1)
			pushchar ('s', bs);
	
		return;
		}
	
	getsizestring ((*browserinfo).filesize, bs);
	} /*getfatsizestring*/

#endif
	

static Rect rfilename, rframe, rdate;


boolean claypredrawline (hdlheadrecord hnode, const Rect *linerect, boolean flselected, boolean flinverted) {
	
	hdltableformats hf = tableformatsdata;
	Rect r;
	
	rframe = *linerect;
	
	claygetnodeframe (hnode, &rframe); /*set global*/
	
	if (hasframe ()) {
		
		/*erase the rect in the fill color or cursor color*/ {
			
			if (flselected) 	
				pushbackcolor (&(**hf).linelayout.cursorcolor);
			else 
				pushbackcolor (&(**hf).linelayout.fillcolor);	
			
			r = rframe;
			
			eraserect (r);
			
			popbackcolor ();
			}
		
		/*
		/%an experiment -- try to give the files "depth" or 3D indentation%/ {
			
			/%top and left edges are framed in dark color%/ {
				
				RGBColor rgb = (**hf).linelayout.backcolor;
				
				darkcolorof (&rgb);
				
				pushforecolor (&rgb);
				
				r = rframe;
			
				MoveTo (r.left, r.top);
				
				LineTo (r.right, r.top);
				
				MoveTo (r.left, r.top + 1);
				
				LineTo (r.right - 1, r.top + 1);
				
				MoveTo (r.left, r.top);
				
				LineTo (r.left, r.bottom);
				
				MoveTo (r.left + 1, r.top);
				
				LineTo (r.left + 1, r.bottom - 1);
				
				popforecolor ();
				}
				
			/%bottom and right edges are framed in light color%/ {
				
				RGBColor rgb = (**hf).linelayout.backcolor;
				
				lightcolorof (&rgb);
				
				pushforecolor (&rgb);
				
				r = rframe;
			
				MoveTo (r.right, r.bottom);
				
				LineTo (r.right, r.top + 1);
				
				MoveTo (r.right - 1, r.bottom);
				
				LineTo (r.right - 1, r.top + 2);
				
				MoveTo (r.right, r.bottom);
				
				LineTo (r.left + 1, r.bottom);
				
				MoveTo (r.right, r.bottom - 1);
				
				LineTo (r.left + 2, r.bottom - 1);
				
				popforecolor ();
				}
			}
			*/
			
		/*draw the node's frame*/ {
		
			r = rframe;
			
			pushforecolor (&(**hf).linelayout.framecolor);
	
			framerect (r);
		
			popforecolor ();
			}
		
		/*drop shadow*/ {
			
			RGBColor rgb = (**hf).linelayout.backcolor;
				
			r = rframe;
			
			darkcolorof (&rgb);
			
			pushforecolor (&rgb);
			
			dropshadowrect (r, 2, false);
			
			popforecolor ();
			}
		
		/*anti-aliasing%/ {
		
			RGBColor rgb;
				
			r = rframe;
			
			InsetRect (&r, 1, 1);
			
			midcolorof (r, &(**hf).linelayout.fillcolor, &(**hf).linelayout.framecolor, &rgb);
			
			pushforecolor (&rgb);
			
			FrameRect (&r);
			
			popforecolor ();
			}
		*/
		}
	
	return (true);
	} /*claypredrawline*/
	

static boolean claydrawtext (hdlheadrecord hnode, const Rect *rtext, boolean flselected, boolean flinverted) {
	
	/*
	the normal text drawing routine, used for script editor and
	menu editor in Frontier. 
	*/
	
	bigstring bshead;
	
	if (opeditingtext (hnode)) {
	
		opeditupdate ();
		
		return (true);
		}
	
	getheadstring (hnode, bshead);
	
	if (!hasframe ())
		ellipsize (bshead, (*rtext).right - (*rtext).left);
	
	movepento ((*rtext).left, (*rtext).top + globalfontinfo.ascent);
	
	if (!flselected) {

		pendrawstring (bshead);
		
		return (true);
		}
	
	/*special tweaked-up code for drawing the selected line*/ {
	
		Rect rframe = *rtext;
		
		rframe.right = rframe.left + stringpixels (bshead);
		
		if (rframe.right > (*rtext).right) 
			rframe.right = (*rtext).right;
		
		rframe.bottom = rframe.top + globalfontinfo.ascent + globalfontinfo.descent;
			
		if (flinverted) {
			
			Rect r = rframe;
			
			insetrect (&r, -1, -1);
			
			pushbackcolor (&blackcolor);
			
			eraserect (r);
			
			popbackcolor ();
			
			pushforecolor (&whitecolor);
			
			pendrawstring (bshead);
		
			popforecolor ();
			}
		else
			pendrawstring (bshead);
			
		/*frame the cursor*/ {
		
			insetrect (&rframe, -texthorizinset, -textvertinset);
	
			grayframerect (rframe);
			}
		}

	return (true);
	} /*claydrawtext*/


boolean claydrawline (hdlheadrecord hnode, const Rect *textrect, boolean flselected, boolean flinverted) {
	
	hdltableformats hf = tableformatsdata;
	RGBColor rgb = (**hf).linelayout.filenamecolor;
	
	claypushnodestyle (hnode);
	
	pushforecolor (&rgb);
	
	claydrawtext (hnode, textrect, flselected, flinverted);
	
	popforecolor ();

	popstyle ();
	
	return (true);
	} /*claydrawline*/
	
	
boolean claypostdrawline (hdlheadrecord hnode, const Rect *linerect, boolean flselected, boolean flinverted) {
	
	/*
	we draw everything but the icon and the filename, if we're in fatview mode.
	
	9/26/94 DW: hack it up to display messages in the Trade Secrets BBS.
	*/
	
	hdltableformats hf = tableformatsdata;
	bigstring bsname;
	tybrowserinfo browserinfo;
	unsigned long nodetime = 0;
	hdlhashnode hhashnode;
	
	pushforecolor (&(**hf).linelayout.othertextcolor);
	
	pushstyle ((**hf).linelayout.othertextfont, (**hf).linelayout.othertextfontsize, normal);
	
	browsergetrefcon (hnode, &browserinfo);
	
	opgetheadstring (hnode, bsname);
	
	claygettextrect (hnode, linerect, &rfilename);
		
	if ((**hf).linelayout.includeline2) { /*draw the file kind/size string*/ 
		
		Rect r = rfilename;
		bigstring bs, bstype;
		tyvaluerecord val;
		
		r.top = r.bottom + spacebetweenlines;
		
		r.bottom = r.top + (**hf).computedlineinfo.othertextlineheight;
		
		if (hashtablelookup (browserinfo.dirid, bsname, &val, &hhashnode)) {
			
			tablegetcellstring (hnode, valuecolumn, bs, true);
			
			if (val.valuetype == externalvaluetype) {
				
				hashgettypestring (val, bstype);
				
				pushstring ("\x02" ", ", bstype);
				
				insertstring (bstype, bs);
				}
			
			ellipsize (bs, r.right - r.left);
			
			movepento (r.left, r.top + globalfontinfo.ascent);
			
			pendrawstring (bs);
			}
		}
	
	nodetime = browserinfo.timemodified;
	
	if ((**hf).linelayout.includedate && (nodetime != 0)) { /*draw the modification date string*/
	
		Rect r = rfilename;
		bigstring bs;
		
		r.right = rframe.right - drawinset;
		
		r.left = r.right - (**hf).computedlineinfo.datewidth;
		
		getfatdatestring (nodetime, bs);
		
		ellipsize (bs, r.right - r.left);
		
		movepento (r.right - stringpixels (bs), r.top + globalfontinfo.ascent);
		
		pendrawstring (bs);
		
		rdate = r;
		}
				
	if ((**hf).linelayout.includeline2 && (**hf).linelayout.includedate) { /*draw the mod time string*/
	
		Rect r = rdate;
		bigstring bs;
		
		r.top = r.bottom + spacebetweenlines;
		
		r.bottom = r.top + (**hf).computedlineinfo.othertextlineheight;
		
		getfattimestring (nodetime, bs);
		
		ellipsize (bs, r.right - r.left);
		
		movepento (r.right - stringpixels (bs), r.top + globalfontinfo.ascent);
		
		pendrawstring (bs);
		}
	
	popstyle ();
	
	popforecolor ();
	
	return (true);
	} /*claypostdrawline*/
	

boolean claydrawnodeicon (hdlheadrecord hnode, const Rect *iconrect, boolean flselected, boolean flinverted) {
	
	short iconnum;
	short transform = 0;
	
	iconnum = opgetheadicon (hnode);
	
	if (flselected)
		transform = 0x4000; 
	
#ifdef MACVERSION
	ploticonresource ((Rect *) iconrect, atVerticalCenter + atHorizontalCenter, transform, iconnum);
#endif

#ifdef WIN95VERSION
	ploticonresource (iconrect, 0, transform, iconnum);
#endif
//	opdrawheadicon (iconnum, iconrect, false);
	
	/*
	if (flselected) {
		
		Rect r = *iconrect;
		
		insetrect (&r, 1, 1);
		
		invertrect (r);
		}
	*/
	
	return (true);
	} /*claydrawnodeicon*/


void claysetlinelayout (hdlwindowinfo targetwindow, tylinelayout *newlayout) {
					
	hdltableformats hf = (hdltableformats) (**targetwindow).hdata;
	
	(**hf).linelayout = *newlayout;
	
	claysmashoutlinefields (targetwindow, hf);
	
	setcomputedfields (hf); 
	
	shellwindowinval (targetwindow);
	
	(**targetwindow).selectioninfo.fldirty = true; /*force update of font/size submenus of the Edit menu*/
	
	#ifdef claydialoginclude
	if ((layoutcard != nil) && ((**hf).houtline != nil)) {
	
		currentcard = layoutcard;
		
		copytodialog ();
		}
	#endif
	} /*claysetlinelayout*/


#ifdef claydialoginclude

static void initdialog (void) {
	
	setcardpopupfrommenu ("\pfilefont", hdlfontmenu, 1);
	
	setcardpopupfrommenu ("\potherfont", hdlfontmenu, 1);
	} /*initdialog*/
	

static void resetfilefontobjects (tylinelayout *layout) {
	
	currentcard = layoutcard;
	
	setfontsizepopup ("\pfilesize", (*layout).filenamefontsize);
	
	setfontpopup ("\pfilefont", (*layout).filenamefont);
	} /*resetfilefontobjects*/
	
	
boolean claywindowuseslayout (hdlappwindow appwindow) {

	hdltableformats hf;
	
	if (appwindow == nil) 
		return (false);
	
	hf = (hdltableformats) (**appwindow).appdata;
	
	return ((**hf).fluseslinelayout);
	} /*claywindowuseslayout*/


static void copytodialog (void) {
	
	tylinelayout layout;
	
	if (!claywindowuseslayout (targetwindow)) {

		enableallcardobjects (false);
		
		return;
		}
	
	layout = (**(hdltableformats) (**targetwindow).appdata).linelayout;
		
	enableallcardobjects (true); /*also invals all objects*/
	
	setcardenable ("\preal", false); /*DW 10/19/95 -- wired off*/
		
	setcardenable ("\pgeneric", false); /*DW 10/19/95 -- wired off*/
		
	setcardflag ("\preal", layout.realicons);
	
	setcardflag ("\pgeneric", !layout.realicons);
	
	setcardflag ("\pfullsize", layout.iconsize == fullsizeicon);
	
	setcardflag ("\psmall", layout.iconsize == smallsizeicon);
	
	setcardflag ("\pverysmall", layout.iconsize == verysmallsizeicon);
	
	setcardflag ("\pline2", layout.includeline2);
	
	setcardflag ("\pdate", layout.includedate);
	
	setcardflag ("\pbold", layout.filenamebold);
	
	setcardflag ("\phasframe", layout.includeframe);
	
	setcardcolorpopup ("\pfill", &layout.fillcolor);
	
	setcardcolorpopup ("\pframe", &layout.framecolor);
	
	setcardcolorpopup ("\pcursor", &layout.cursorcolor);	
	
	setcardcolorpopup ("\pfilecolor", &layout.filenamecolor);	
	
	setcardcolorpopup ("\pothercolor", &layout.othertextcolor);	
	
	setcardcolorpopup ("\pbackcolor", &layout.backcolor);	
	
	setcardcolorpopup ("\pstatuscolor", &layout.statuscolor);	
	
	resetfilefontobjects (&layout);
	
	setfontsizepopup ("\pothersize", layout.othertextfontsize);
	
	setfontpopup ("\potherfont", layout.othertextfont);
	} /*copytodialog*/
	
	
static void copyfromdialog (tylinelayout *layout) {
	
	clearbytes (layout, longsizeof (tylinelayout));
	
	(*layout).realicons = getcardbit ("\preal");
	
	if (getcardbit ("\pfullsize"))
		(*layout).iconsize = fullsizeicon;
		
	if (getcardbit ("\psmall"))
		(*layout).iconsize = smallsizeicon;
		
	if (getcardbit ("\pverysmall"))
		(*layout).iconsize = verysmallsizeicon;
		
	(*layout).includeline2 = getcardbit ("\pline2");
	
	(*layout).includedate = getcardbit ("\pdate");
	
	(*layout).filenamebold = getcardbit ("\pbold");
	
	(*layout).includeframe = getcardbit ("\phasframe");
	
	getcardcolorpopup ("\pfill", &(*layout).fillcolor);
	
	getcardcolorpopup ("\pframe", &(*layout).framecolor);
	
	getcardcolorpopup ("\pcursor", &(*layout).cursorcolor);
	
	getcardcolorpopup ("\pfilecolor", &(*layout).filenamecolor);
	
	getcardcolorpopup ("\pothercolor", &(*layout).othertextcolor);
	
	getcardcolorpopup ("\pbackcolor", &(*layout).backcolor);
	
	getcardcolorpopup ("\pstatuscolor", &(*layout).statuscolor);
	
	(*layout).filenamefontsize = getfontsizepopup ("\pfilesize");
	
	(*layout).othertextfontsize = getfontsizepopup ("\pothersize");
	
	getfontpopup ("\pfilefont", (*layout).filenamefont);
	
	getfontpopup ("\potherfont", (*layout).othertextfont);
	} /*copyfromdialog*/
	
	
static boolean gettargetwindow (void) {
	
	/*
	return true if the target window was changed
	*/

	hdlappwindow newtarget;
	
	claygetfrontwindow (&newtarget);
	
	if (newtarget != targetwindow) {
		
		targetwindow = newtarget;
		
		copytodialog ();
		
		return (true); /*changed*/
		}
		
	return (false); /*didn't change*/
	} /*gettargetwindow*/


static void linelayoutprefsfilter (EventRecord *ev) {
	
	switch ((*ev).what) {
		
		case iowaInitEvent: 
			currentcard = (hdlcard) (*ev).message;
			
			layoutcard = currentcard;
			
			claygetfrontwindow (&targetwindow);
			
			initdialog ();
			
			copytodialog ();
			
			break;
			
		case activateEvt: 
			if ((*ev).modifiers & activeFlag) {
				
				currentcard = layoutcard; /*DW 4/11/95: added*/
				
				gettargetwindow ();
				}
				
			break;
			
		case iowaCloseEvent:
			currentcard = (hdlcard) (*ev).message; 
			
			appsavedialoginfo (137); /*remember window position*/
			
			layoutcard = nil; /*we always forget to do this!!! DW 9/13/93*/
			
			break;
				
		case nullEvent: 
			currentcard = layoutcard;
			
			iowavalidate (true); /*DW 4/11/95: added*/

			if (gettargetwindow ()) /*target window changed, card already updated*/			
				break;
			
			if (!claywindowuseslayout (targetwindow)) /*it's an outline or process window, for e.g.*/
				break;
				
			/*check for changes to card objects*/ {
			
				if ((**currentcard).runmodechanges) {
					
					tylinelayout newlayout;
					
					copyfromdialog (&newlayout);

					setappwindowmadechanges (targetwindow); 
				
					claysetlinelayout (targetwindow, &newlayout);
					
					(**currentcard).runmodechanges = false;
					}
				
				/*
				hdltableformats hf = (hdltableformats) (**targetwindow).appdata;
				tylinelayout oldlayout, newlayout;
			
				oldlayout = (**hf).linelayout;
				
				copyfromdialog (&newlayout);
				
				if (!equalmemory (&newlayout, &oldlayout, longsizeof (oldlayout))) {
					
					setappwindowmadechanges (targetwindow); 
				
					claysetlinelayout (&newlayout);
					}
				*/
				}
				
			break;
		} /*switch*/
	} /*linelayoutprefsfilter*/
	
	
void linelayoutprefsdialog (void) {
	
	if (layoutcard == nil)		
		appopencardresource (137, &linelayoutprefsfilter);
	else 
		appcardtofront (layoutcard); 
	} /*linelayoutprefsdialog*/


void linelayoutbeforeclosewindow (void) {
	
	if (targetwindow == app.appwindow)	
		targetwindow = nil;
	} /*linelayoutbeforeclosewindow*/

#endif

static boolean hashlookupboolean (bigstring bslookup) {
	
	/*
	look up a boolean value, defaulting to false if any errors occur
	*/
	
	tyvaluerecord val;
	hdlhashnode hnode;
	
	if (!hashlookup (bslookup, &val, &hnode))
		return (false);
	
	if (!copyvaluerecord (val, &val) || !coercetoboolean (&val))
		return (false);
	
	return (val.data.flvalue);
	} /*hashlookupboolean*/


static short hashlookupshort (bigstring bslookup, short defaultval) {
	
	/*
	look up a short value, defaulting to defaultval if any errors occur
	*/
	
	tyvaluerecord val;
	hdlhashnode hnode;
	
	if (!hashlookup (bslookup, &val, &hnode))
		return (defaultval);
	
	if (!copyvaluerecord (val, &val) || !coercetoint (&val))
		return (defaultval);
	
	return (val.data.intvalue);
	} /*hashlookupshort*/


static RGBColor hashlookupcolor (bigstring bslookup, const RGBColor *defaultrgb) {
	
	/*
	look up a rgb color value, defaulting to defaultrgb if any errors occur
	*/
	
	tyvaluerecord val;
	hdlhashnode hnode;
	
	if (!hashlookup (bslookup, &val, &hnode))
		return (*defaultrgb);
	
	if (!copyvaluerecord (val, &val) || !coercetorgb (&val))
		return (*defaultrgb);
	
	return (**val.data.rgbvalue);
	} /*hashlookupcolor*/


static boolean hashlookupstring (bigstring bslookup, bigstring bs) {
	
	/*
	look up a font value, returning false if any errors occur
	*/
	
	tyvaluerecord val;
	hdlhashnode hnode;
	
	if (!hashlookup (bslookup, &val, &hnode))
		return (false);
	
	if (!copyvaluerecord (val, &val) || !coercetostring (&val))
		return (false);
	
	pullstringvalue (&val, bs);
	
	return (true);
	} /*hashlookupstring*/


static boolean langassigncolorvalue (hdlhashtable ht, const bigstring bs, const RGBColor *color) {
	
	/*
	could be useful...
	*/
	
	tyvaluerecord val;
	
	if (!newheapvalue ((void *) color, sizeof (*color), rgbvaluetype, &val))
		return (false);
	
	if (!hashtableassign (ht, bs, val)) {
		
		disposevaluerecord (val, false);
		
		return (false);
		}
	
	exemptfromtmpstack (&val);
	
	return (true);
	} /*langassignbooleanvalue*/


boolean claytabletolayout (hdlhashtable ht, tylinelayout *layout) {
	
	bigstring bsfont;
	
	clearbytes (layout, sizeof (tylinelayout)); // clears flinitted, everything else
	
	pushhashtable (ht);
	
	(*layout).claydisplay = hashlookupboolean (str_claydisplay);
	(*layout).realicons	 = hashlookupboolean (str_realicons);
	(*layout).filenamebold = hashlookupboolean (str_filenamebold);
	(*layout).includeline2 = hashlookupboolean (str_includeline2);
	(*layout).includedate = hashlookupboolean (str_includedate);
	(*layout).includeframe = hashlookupboolean (str_includeframe);
	
	(*layout).fillcolor = hashlookupcolor (str_fillcolor, &whitecolor);
	(*layout).framecolor = hashlookupcolor (str_framecolor, &blackcolor);
	(*layout).cursorcolor = hashlookupcolor (str_cursorcolor, &blackcolor);
	(*layout).filenamecolor = hashlookupcolor (str_filenamecolor, &blackcolor);
	(*layout).othertextcolor = hashlookupcolor (str_othertextcolor, &blackcolor);
	(*layout).backcolor = hashlookupcolor (str_backcolor, &whitecolor);
	(*layout).statuscolor = hashlookupcolor (str_statuscolor, &blackcolor);
	
	if (hashlookupstring (str_filenamefont, bsfont))
		fontgetnumber (bsfont, &(*layout).filenamefont);
	else
		(*layout).filenamefont = config.defaultfont;
	
	if (hashlookupstring (str_othertextfont, bsfont))
		fontgetnumber (bsfont, &(*layout).othertextfont);
	else
		(*layout).othertextfont = config.defaultfont;
	
	(*layout).iconsize = (tyiconsize) hashlookupshort (str_iconsize, 1);
	(*layout).filenamefontsize = hashlookupshort (str_filenamefontsize, 12);
	(*layout).othertextfontsize = hashlookupshort (str_othertextfontsize, 9);
	
	pophashtable ();
	
	(*layout).flinitted = true;
	
	return (true);
	} /*claytabletolayout*/


boolean claylayouttotable (const tylinelayout *layout, hdlhashtable ht) {
	
	bigstring bs;
	
	if (!(*layout).flinitted)
		return (false);
	
	langassignbooleanvalue (ht, str_claydisplay, (*layout).claydisplay);
	langassignbooleanvalue (ht, str_realicons, (*layout).realicons);
	langassignbooleanvalue (ht, str_filenamebold, (*layout).filenamebold);
	langassignbooleanvalue (ht, str_includeline2, (*layout).includeline2);
	langassignbooleanvalue (ht, str_includedate, (*layout).includedate);
	langassignbooleanvalue (ht, str_includeframe, (*layout).includeframe);
	
	langassigncolorvalue (ht, str_fillcolor, &(*layout).fillcolor);
	langassigncolorvalue (ht, str_framecolor, &(*layout).framecolor);
	langassigncolorvalue (ht, str_cursorcolor, &(*layout).cursorcolor);
	langassigncolorvalue (ht, str_filenamecolor, &(*layout).filenamecolor);
	langassigncolorvalue (ht, str_othertextcolor, &(*layout).othertextcolor);
	langassigncolorvalue (ht, str_backcolor, &(*layout).backcolor);
	langassigncolorvalue (ht, str_statuscolor, &(*layout).statuscolor);
	
	fontgetname ((*layout).filenamefont, bs);
	langassignstringvalue (ht, str_filenamefont, bs);
	
	fontgetname ((*layout).othertextfont, bs);
	langassignstringvalue (ht, str_othertextfont, bs);
	
	langassignlongvalue (ht, str_iconsize, (*layout).iconsize);
	langassignlongvalue (ht, str_filenamefontsize, (*layout).filenamefontsize);
	langassignlongvalue (ht, str_othertextfontsize, (*layout).othertextfontsize);
	
	return (true);
	} /*claylayouttotable*/


boolean clayinitlinelayout (tylinelayout *layout) {
	
	hdlhashtable hprefs;
	
	if (!langfastaddresstotable (roottable, "\x17" "user.prefs.tableDisplay", &hprefs))
		return (false);
	
	return (claytabletolayout (hprefs, layout));
	} /*clayinitlinelayout*/


boolean claypacklinelayout (Handle hpacked) {

	tylinelayout layout = (**tableformatsdata).linelayout;
	tydisklinelayout info;
	
	info.recordsize = sizeof (info);
	
	info.claydisplay = (byte) layout.claydisplay;
	
	info.realicons = (byte) layout.realicons;
	
	info.filenamebold = (byte) layout.filenamebold;
	
	info.includeline2 = (byte) layout.includeline2;
	
	info.includedate = (byte) layout.includedate;
	
	info.includeframe = (byte) layout.includeframe;
	
	info.iconsize = (byte) layout.iconsize;
	
	info.fillcolor = layout.fillcolor;
	
	info.framecolor = layout.framecolor;
	
	info.cursorcolor = layout.cursorcolor;
	
	info.filenamecolor = layout.filenamecolor;
	
	info.othertextcolor = layout.othertextcolor; 
	
	info.backcolor = layout.backcolor;
	
	info.statuscolor = layout.statuscolor;
	
	fontgetname (layout.filenamefont, info.filenamefont);
	
	fontgetname (layout.othertextfont, info.othertextfont);
	
	info.filenamefontsize = layout.filenamefontsize;
	
	info.othertextfontsize = layout.othertextfontsize;
	
	memtodiskshort (info.recordsize);
	
	memtodiskcolor (info.fillcolor);
	memtodiskcolor (info.framecolor);
	memtodiskcolor (info.cursorcolor);
	memtodiskcolor (info.filenamecolor);
	memtodiskcolor (info.othertextcolor);
	memtodiskcolor (info.backcolor);
	memtodiskcolor (info.statuscolor);
	
	memtodiskshort (info.filenamefontsize);
	memtodiskshort (info.othertextfontsize);
	
	return (enlargehandle (hpacked, sizeof (info), &info));
	} /*claypacklinelayout*/


boolean clayunpacklinelayout (Handle hpacked, long *ixhandle, hdltableformats hf) {

	/*
	2002-11-11 AR: Added assert to make sure the C compiler chose the
	proper byte alignment for the tydisklinelayout struct. If it did not,
	we would end up corrupting any database files we saved.
	*/
	
	tylinelayout layout;
	tydisklinelayout info;
	
	assert (sizeof(tydisklinelayout) == 124); /* one padding byte each after iconsize, filenamefont, and othertextfont */
	
	clearbytes (&info, sizeof (info));
	
	if (!loadfromhandle (hpacked, ixhandle, sizeof (info.recordsize), &info))
		return (false);
	
	disktomemshort (info.recordsize);
	
	if (!loadfromhandle (hpacked, ixhandle, min (sizeof (info), info.recordsize) - sizeof (info.recordsize), &info.claydisplay))
		return (false);
	
	disktomemcolor (info.fillcolor);
	disktomemcolor (info.framecolor);
	disktomemcolor (info.cursorcolor);
	disktomemcolor (info.filenamecolor);
	disktomemcolor (info.othertextcolor);
	disktomemcolor (info.backcolor);
	disktomemcolor (info.statuscolor);
	
	disktomemshort (info.filenamefontsize);
	disktomemshort (info.othertextfontsize);
	
	layout.flinitted = true;
	
	layout.claydisplay = (boolean) info.claydisplay;
	
	layout.realicons = (boolean) info.realicons;
	
	layout.filenamebold = (boolean) info.filenamebold;
	
	layout.includeline2 = (boolean) info.includeline2;
	
	layout.includedate = (boolean) info.includedate;
	
	layout.includeframe = (boolean) info.includeframe;
	
	layout.iconsize = (tyiconsize) info.iconsize;
	
	layout.fillcolor = info.fillcolor;
	
	layout.framecolor = info.framecolor;
	
	layout.cursorcolor = info.cursorcolor;
	
	layout.filenamecolor = info.filenamecolor;
	
	layout.othertextcolor = info.othertextcolor; 
	
	layout.backcolor = info.backcolor;
	
	layout.statuscolor = info.statuscolor;
	
	fontgetnumber (info.filenamefont, &layout.filenamefont);
	
	fontgetnumber (info.othertextfont, &layout.othertextfont);
	
	layout.filenamefontsize = info.filenamefontsize;
	
	layout.othertextfontsize = info.othertextfontsize;
	
	(**hf).linelayout = layout;
	
	return (true);
	} /*clayunpacklineinfo*/



