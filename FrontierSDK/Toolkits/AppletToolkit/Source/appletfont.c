
/*	$Id$    */

/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include "appletdefs.h"
#include "appletstrings.h"
#include "appletfont.h"


#define styleseparator "\p+"

FontInfo globalfontinfo;

#define ctstyles 7 

short stylesarray [ctstyles] = 
	
	{0, bold, italic, underline, 
	
	bold + italic, bold + underline, 
	
	italic + underline};




short sumcharwidths (void *pstring) {
	
	char *bs = (char *) pstring;
	short sum = 0;
	short i;
	
	for (i = 1; i <= stringlength (bs); i++)
		sum += CharWidth (bs [i]);
		
	return (sum);
	} /*sumcharwidths*/
	

void setfontsizestyle (short fontnum, short fontsize, short fontstyle) {

	TextFont (fontnum);
	
	TextSize (fontsize);
	
	TextFace (fontstyle);
	} /*setfontsizestyle*/
	
	
void setglobalfontsizestyle (short fontnum, short fontsize, short fontstyle) {

	setfontsizestyle (fontnum, fontsize, fontstyle);
	
	GetFontInfo (&globalfontinfo);
	} /*setglobalfontsizestyle*/
	
	
short setnamedfont (bigstring bs, short fsize, short fstyle, short defaultfont) {

	/*
	give me the name of a font you like.  I'll try to set to that font,
	but if its not available, you get the default font.  be sure to select
	a default font that is mandatory -- like geneva or newYork.
	*/
	
	short fontnum;
	
	GetFNum (bs, &fontnum);
	
	if (fontnum == 0) 
		fontnum = defaultfont; /*use caller's second choice*/
		
	setglobalfontsizestyle (fontnum, fsize, fstyle);
	
	return (fontnum); /*return the font that we are actually using*/
	} /*setnamedfont*/
		

void getfontsizestyle (short *fontnum, short *fontsize, short *fontstyle) {

	*fontnum = (*quickdrawglobal (thePort)).txFont;
	
	*fontsize = (*quickdrawglobal (thePort)).txSize;
	
	*fontstyle = (*quickdrawglobal (thePort)).txFace;
	} /*getfontsizestyle*/
	
	
void getstyle (short style, short *flplain, short *flbold, short *flitalic, short *flunderline, short *floutline, short *flshadow) {

	/*
	give me a style bit array, and I'll extract the booleans which are 
	slightly easier to deal with.
	
	if none of the others are true, we set flplain to true.  otherwise 
	flplain is false.
	*/

	*flplain = true;  /*default values*/
	
	*flbold = false;
	
	*flitalic = false;
	
	*flunderline = false;
	
	*floutline = false;
	
	*flshadow = false;
	
	if (style >= shadow) {
		
		style -= shadow;
		
		*flshadow = true;
		
		*flplain = false;
		}
		
	if (style >= outline) {
		
		style -= outline;
		
		*floutline = true;
		
		*flplain = false;
		}
		
	if (style >= underline) {
		
		style -= underline;
		
		*flunderline = true;
		
		*flplain = false;
		}
		
	if (style >= italic) {
		
		style -= italic;
		
		*flitalic = true;
		
		*flplain = false;
		}
		
	if (style >= bold) {
		
		style -= bold;
		
		*flbold = true;
		
		*flplain = false;
		}
	} /*getstyle*/
	

void checkstyle (boolean fl, bigstring bsin, bigstring bsout) {

	if (fl) {
		if (bsout [0]) { /*already something on the output string*/
		
			pushstring (styleseparator, bsout);
			
			pushstring (bsin, bsout);
			}
		else
			copystring (bsin, bsout);
		}
	} /*checkstyle*/
		

short getfontheight (void) {
	
	FontInfo info;
	
	GetFontInfo (&info);
	
	return (info.ascent + info.descent);
	} /*getfontheight*/
	
	
void fontgetname (short fontnum, bigstring fontname) {
	
	GetFontName (fontnum, fontname);
	} /*fontgetname*/


void fontgetnumber (bigstring fontname, short *fontnum) {
	
	if (stringlength (fontname) == 0)
		*fontnum = geneva;
	else
		GetFNum ((ptrstring) fontname, fontnum);
	} /*fontgetnumber*/
	

void diskgetfontname (short fontnum, diskfontstring fontname) {
	
	/*
	Apple recommends that fonts be stored on disk as strings.  we return the
	fontname, limited in length to 32, based on the indicated font number.
	*/
	
	bigstring bs;
	
	GetFontName (fontnum, bs);
	
	if (stringlength (bs) > diskfontnamelength)
		setstringlength (bs, diskfontnamelength);
	
	copystring (bs, (ptrstring) fontname);
	} /*diskgetfontname*/


void diskgetfontnum (diskfontstring fontname, short *fontnum) {
	
	if (stringlength (fontname) == 0)
		*fontnum = geneva;
	else
		GetFNum ((ptrstring) fontname, fontnum);
	} /*diskgetfontnum*/
	

void getnextfont (short *fontnum) {
	
	/*
	MenuHandle hmenu = hdlfontmenu;
	short i, ctitems;
	bigstring fontname, bs;
	
	GetFontName (*fontnum, fontname);
	
	ctitems = CountMItems (hmenu);
	
	for (i = 1; i <= ctitems; i++) {
		
		GetItem (hmenu, i, bs);
		
		if (unicaseequalstrings (bs, fontname)) {
			
			if (i == ctitems)
				GetItem (hmenu, 1, fontname);
			else
				GetItem (hmenu, i + 1, fontname);
				
			GetFNum (fontname, fontnum);
				
			return;
			}
		} /%for%/
		
	*fontnum = applFont; /%the input font isn't on this system%/
	*/
	} /*getnextfont*/
	
	
void getprevfont (short *fontnum) {
	
	/*
	MenuHandle hmenu = hdlfontmenu;
	short i, ctitems;
	bigstring fontname, bs;
	
	GetFontName (*fontnum, fontname);
	
	ctitems = CountMItems (hmenu);
	
	for (i = 1; i <= ctitems; i++) {
		
		GetItem (hmenu, i, bs);
		
		if (unicaseequalstrings (bs, fontname)) {
			
			if (i == 1)
				GetItem (hmenu, ctitems, fontname);
			else
				GetItem (hmenu, i - 1, fontname);
				
			GetFNum (fontname, fontnum);
				
			return;
			}
		} /%for%/
		
	*fontnum = applFont; /%the input font isn't on this system%/
	*/
	} /*getprevfont*/
	
	
void getnextstyle (short *style) { 
	
	short i;
	
	for (i = 0; i < ctstyles; i++) {
		
		if (*style == stylesarray [i]) {
			
			if (i == (ctstyles - 1)) 
				*style = stylesarray [0];
			else
				*style = stylesarray [i + 1];
				
			return;
			}
		} /*for*/
		
	*style = stylesarray [0];
	} /*getnextstyle*/
	
	
void getprevstyle (short *style) { 
	
	short i;
	
	for (i = 0; i < ctstyles; i++) {
		
		if (*style == stylesarray [i]) {
			
			if (i == 0) 
				*style = stylesarray [ctstyles - 1];
			else
				*style = stylesarray [i - 1];
			
			return;
			}
		} /*for*/
		
	*style = stylesarray [0];
	} /*getprevstyle*/
	
	
