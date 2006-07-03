
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

#ifndef claylinelayoutinclude
#define claylinelayoutinclude


typedef enum tyiconsize {
	
	fullsizeicon = 1,
	
	smallsizeicon = 2,
	
	verysmallsizeicon = 3
	} tyiconsize;
	
	
typedef struct tylinelayout {
	
	boolean flinitted; //if true, the other fields in this record been set
	
	boolean claydisplay; // if true, we're using the clay line layout
	
	boolean realicons; /*if true, we go into the desktop db for icons, otherwise use generic ones*/
	
	boolean filenamebold; /*if true, the file name is drawn in bold*/
	
	boolean includeline2; /*if true we include the 2nd line of the display*/
	
	boolean includedate; /*if true, we include the modification date*/
	
	boolean includeframe; /*if true, each line has a frame, if false it's just like an outliner*/
	
	tyiconsize iconsize; /*full-size, small or very small icons*/
	
	RGBColor fillcolor, framecolor, cursorcolor, filenamecolor, othertextcolor; 
	
	RGBColor backcolor, statuscolor;
	
	short filenamefont, othertextfont;
	
	short filenamefontsize, othertextfontsize;
	} tylinelayout;


typedef struct tycomputedlineinfo { /*computed fields that depend on a linelayout record*/

	short filenamelineheight;
	
	short othertextlineheight;
	
	short filenamewidth; /*the number of pixels reserved for the file name*/
	
	short datewidth; /*number of pixels reserved for the date*/
	} tycomputedlineinfo;


#define str_claydisplay		("\x0b" "claydisplay")
#define str_realicons		("\x09" "realicons")
#define str_filenamebold	("\x0c" "filenamebold")
#define str_includeline2	("\x0c" "includeline2")
#define str_includedate		("\x0b" "includedate")
#define str_includeframe	("\x0c" "includeframe")
#define str_iconsize		("\x08" "iconsize")
#define str_fillcolor		("\x09" "fillcolor")
#define str_framecolor		("\x0a" "framecolor")
#define str_cursorcolor		("\x0b" "cursorcolor")
#define str_filenamecolor	("\x0d" "filenamecolor")
#define str_othertextcolor	("\x0e" "othertextcolor")
#define str_backcolor		("\x09" "backcolor")
#define str_statuscolor		("\x0b" "statuscolor")
#define str_filenamefont	("\x0c" "filenamefont")
#define str_othertextfont	("\x0d" "othertextfont")
#define str_filenamefontsize	("\x10" "filenamefontsize")
#define str_othertextfontsize	("\x11" "othertextfontsize")

extern void claybrowserinitdraw (void);

extern boolean claypushnodestyle (hdlheadrecord);

extern boolean claygetlineheight (hdlheadrecord, short *);

extern boolean claygetlinewidth (hdlheadrecord, short *);

extern boolean claydrawline (hdlheadrecord, const Rect *, boolean, boolean);

extern boolean claygettextrect (hdlheadrecord, const Rect *, Rect *);

extern boolean claygetedittextrect (hdlheadrecord, const Rect *, Rect *);

extern boolean claygeticonrect (hdlheadrecord, const Rect *, Rect *);

extern boolean claypredrawline (hdlheadrecord, const Rect *, boolean, boolean);

extern boolean claypostdrawline (hdlheadrecord, const Rect *, boolean, boolean);

extern boolean claydrawnodeicon (hdlheadrecord, const Rect *, boolean, boolean);

extern boolean claygetnodeframe (hdlheadrecord, Rect *);

extern void claysmashoutlinefields (hdlwindowinfo, struct tytableformats **);

extern void claysetlinelayout (hdlwindowinfo, tylinelayout *);

extern boolean clayinitlinelayout (tylinelayout *);

extern boolean claylayouttotable (const tylinelayout *, hdlhashtable);

extern boolean claytabletolayout (hdlhashtable, tylinelayout *);

extern boolean claypacklinelayout (Handle);

extern boolean clayunpacklinelayout (Handle, long *, struct tytableformats **);

#ifdef claydialoginclude

extern boolean claywindowuseslayout (hdlappwindow);

extern void linelayoutprefsdialog (void);

extern void linelayoutbeforeclosewindow (void);

#endif

#endif