
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

#include "Paige.h"
#include "pgdeftbl.h"

#include "strings.h"
#include "font.h"
#include "quickdraw.h"


FontInfo globalfontinfo;

static bigstring cachedfontname;
static short cachedfontnum;

#ifdef WIN95VERSION
typedef struct tywindowsfontinformation {
	
	FontInfo	fi;

	LOGFONT		lfi;
	} tywindowsfontinformation;

tywindowsfontinformation ** globalWindowsFontInfo;

short globalFontCount;

static short currentFontNum;
static short currentFontSize;
static short currentFontStyle;

static HFONT globalOldFont = 0;
static HFONT globalCurrentFont = 0;

static BOOL FAR PASCAL EnumFamCallBack1(ENUMLOGFONT * lplf, LPNEWTEXTMETRIC lpntm, DWORD FontType, LPVOID counter) {
	
	short * ctr = (short *) counter;
	*ctr = *ctr + 1;
    return TRUE; 
	} /*EnumFamCallBack*/


static BOOL FAR PASCAL EnumFamCallBack2(ENUMLOGFONT * lplf, LPNEWTEXTMETRIC lpntm, DWORD FontType, LPVOID counter) {
	
	tywindowsfontinformation * wfi;
	short * ctr = (short *) counter;
	*ctr = *ctr + 1;
	wfi = *globalWindowsFontInfo + *ctr;
	wfi->lfi = lplf->elfLogFont;
	wfi->fi.ascent = (short)lpntm->tmAscent;
	wfi->fi.descent = (short)lpntm->tmDescent;
	wfi->fi.leading = (short)lpntm->tmInternalLeading;
    return TRUE; 
	} /*EnumFamCallBack2*/


short geneva = 1;

static void initgenevafont (void) {

	fontgetnumber ("\x0d" "MS Sans Serif", &geneva);
	
	if (geneva == 0)
		fontgetnumber ("\x05" "Arial", &geneva);
	} /*initgenevafont*/


void getWindowsLogFont (LOGFONT *lf) {
	
	short ctr;
	tywindowsfontinformation * wfi;

	if (globalCurrentFont != 0)
		return;

	ctr = currentFontNum;

	if (ctr == 0)
		ctr = ctr + 1;

	wfi = *globalWindowsFontInfo + ctr;

	*lf = wfi->lfi;
	lf->lfHeight = -MulDiv(currentFontSize, GetDeviceCaps(getcurrentDC(), LOGPIXELSY), 72);
	lf->lfWidth = 0;
	} /*getWindowsLogFont*/


void setWindowsFont () {
	
	short ctr;
	tywindowsfontinformation * wfi;

	if (globalCurrentFont != 0)
		return;

	ctr = currentFontNum;

	if (ctr == 0)
		ctr = ctr + 1;

	wfi = *globalWindowsFontInfo + ctr;

	wfi->lfi.lfHeight = -MulDiv(currentFontSize, GetDeviceCaps(getcurrentDC(), LOGPIXELSY), 72);
	
	wfi->lfi.lfWidth = 0;
	
	wfi->lfi.lfWeight = (currentFontStyle & bold)? FW_BOLD : FW_NORMAL;
	
	wfi->lfi.lfItalic = (currentFontStyle & italic)? true : false;
	
	wfi->lfi.lfUnderline = (currentFontStyle & underline)? true : false;
	 
	globalCurrentFont = CreateFontIndirect (&(wfi->lfi));

	globalOldFont = SelectObject (getcurrentDC(), globalCurrentFont);
	} /*setWindowsFont*/


void clearWindowsFont () {
	
	SelectObject (getcurrentDC(), globalOldFont);
	DeleteObject (globalCurrentFont);

	globalCurrentFont = 0;
	globalOldFont = 0;
	} /*clearWindowsFont*/

	
boolean findWindowsFont (ptrstring fontname, short *fontnumber) {

	short i;
	char fn[256];
	tywindowsfontinformation * wfi;

	memmove (fn, stringbaseaddress(fontname), stringlength(fontname));
	fn[stringlength(fontname)] = 0;

	for (i = 1; i <= globalFontCount; i++) {
		
		wfi = *globalWindowsFontInfo + i;
		
		if (stricmp (wfi->lfi.lfFaceName, fn)==0) {

			*fontnumber = i;
			
			return (true);
			}
		}

	return (false);
	} /*findWindowsFont*/

#endif

static boolean mapxfont (ptrstring bsfont) {
	
	/*
	2002-10-13 AR: Slight change to eliminate compiler warning about possible unwanted assignment
	*/

	register pg_char_ptr		table_ptr;
	short						alternate_index, out_index;

	if ((table_ptr = cross_font_table) == (pg_char_ptr)NULL)
		return (false);
	
	while (*table_ptr <= bsfont[1]) {
		
		alternate_index = pgCompareFontTable(&bsfont[1], table_ptr);
		
		if (alternate_index != 0) {
			
			table_ptr += alternate_index;
			pgFillBlock(bsfont, FONT_SIZE * sizeof(pg_char), 0);
			
			for (out_index = 1; out_index < FONT_SIZE; ++out_index) {
				
				bsfont[out_index] = *table_ptr++;
				bsfont[0] += 1;
				
				if (*table_ptr == ']' || *table_ptr == 0)
					break;
				}
			
			break;
			}
		
		for (;;) {
			
			if (*table_ptr++ == 0)
				break;
			}
		}
	
	return (true);
	} /*mapxfont*/


void fontgetnumber (bigstring fontname, short *fontnumber) {
	
	/*
	5.0a13 dmb: operate on a copy; mapping modifies string (64 bytes!)
	5.0a13 dmb: map on Mac platform too
	
	5.0.2b19 dmb: added cachedfont logic
	*/
	
	bigstring bsfont;
	
	copystring (fontname, bsfont);
	
	nullterminate (bsfont);
	
	if (equalstrings (cachedfontname, bsfont))
		goto exit;
	
	copystring (bsfont, cachedfontname);
	
#ifdef MACVERSION	
	GetFNum (bsfont, &cachedfontnum);

	if ((cachedfontnum == 0) && !equalstrings (bsfont, "\x07" "Chicago"))
		if (mapxfont (bsfont))
			GetFNum (bsfont, &cachedfontnum);
#endif
#ifdef WIN95VERSION
	if (findWindowsFont (bsfont, &cachedfontnum))
		goto exit;

	if (mapxfont (bsfont))
		if (findWindowsFont (bsfont, &cachedfontnum))
			goto exit;

	cachedfontnum = 0;
#endif
	
	exit:
	
	*fontnumber = cachedfontnum;
	} /*fontgetnumber*/


void fontgetname (short fontnumber, bigstring fontname) {
#ifdef MACVERSION		
	GetFontName (fontnumber, fontname);
#endif
#ifdef WIN95VERSION
	tywindowsfontinformation * wfi;

	wfi = *globalWindowsFontInfo + fontnumber;

	strcpy (stringbaseaddress(fontname), wfi->lfi.lfFaceName);
	setstringlength (fontname, strlen(wfi->lfi.lfFaceName));
#endif
	} /*fontgetname*/


void setfontsizestyle (short fontnum, short fontsize, short fontstyle) {
#ifdef MACVERSION
	TextFont (fontnum);
	
	TextSize (fontsize);
	
	TextFace (fontstyle);
#endif
#ifdef WIN95VERSION
	if (fontnum > 0)
		currentFontNum = fontnum;

	if (fontsize != 0)
		currentFontSize = fontsize;

	currentFontStyle = fontstyle;
#endif
	} /*setfontsizestyle*/
	
	
void setglobalfontsizestyle (short fontnum, short fontsize, short fontstyle) {
#ifdef MACVERSION	
	setfontsizestyle (fontnum, fontsize, fontstyle);
	
	GetFontInfo (&globalfontinfo);
#endif

#ifdef WIN95VERSION
	HFONT fnt, oldfnt;
	TEXTMETRIC tm;
	tywindowsfontinformation * wfi;
	short ctr;
	HDC hdc;
	long logpix;

	hdc = GetDC (GetDesktopWindow());

	if (fontnum > 0)
		currentFontNum = fontnum;

	if (fontsize != 0)
		currentFontSize = fontsize;

	currentFontStyle = fontstyle;

	ctr = currentFontNum;

//	ctr = ctr + 1;
	wfi = *globalWindowsFontInfo + ctr;

	wfi->lfi.lfHeight = -MulDiv(currentFontSize, GetDeviceCaps(getcurrentDC(), LOGPIXELSY), 72);
	
	wfi->lfi.lfWidth = 0;
	
	wfi->lfi.lfWeight = (currentFontStyle & bold)? FW_BOLD : FW_NORMAL;
	
	wfi->lfi.lfItalic = (currentFontStyle & italic)? true : false;
	
	wfi->lfi.lfUnderline = (currentFontStyle & underline)? true : false;

	fnt = CreateFontIndirect (&(wfi->lfi));

	oldfnt = SelectObject (hdc, fnt);

	GetTextMetrics (hdc, &tm);

	logpix = GetDeviceCaps(getcurrentDC(), LOGPIXELSY);

	globalfontinfo.ascent = (short) tm.tmAscent;
	globalfontinfo.descent = (short) tm.tmDescent;
	globalfontinfo.leading = (short) tm.tmExternalLeading;
//	globalfontinfo.leading = (short)tm.tmInternalLeading;

	SelectObject (hdc, oldfnt);
	DeleteObject (fnt);

	ReleaseDC (GetDesktopWindow(), hdc);
#endif
	} /*setglobalfontsizestyle*/

	
#if !flruntime

boolean realfont (short fontnum, short fontsize) {
#ifdef MACVERSION	
	return (RealFont (fontnum, fontsize));
#endif
#ifdef WIN95VERSION
	if ((fontnum > 0) && (fontnum <= globalFontCount))
		return (true);

	return (false);
#endif
	} /*realfont*/


short setnamedfont (bigstring bs, short fsize, short fstyle, short defaultfont) {

	/*
	give me the name of a font you like.  I'll try to set to that font,
	but if its not available, you get the default font.  
	*/	
	short fontnum;
	
	fontgetnumber (bs, &fontnum);
	
	if (fontnum == 0) 
		fontnum = defaultfont; /*use caller's second choice*/
	
	setglobalfontsizestyle (fontnum, fsize, fstyle);
	
	return (fontnum); /*return the font that we are actually using*/
	} /*setnamedfont*/

#endif


void getfontsizestyle (short *fontnum, short *fontsize, short *fontstyle) {
#ifdef MACVERSION	
	//Code change by Timothy Paustian Monday, May 1, 2000 8:36:17 PM
	//Changed to Opaque call for Carbon
	#if ACCESSOR_CALLS_ARE_FUNCTIONS == 1
	CGrafPtr	thePort = GetQDGlobalsThePort();
	*fontnum = GetPortTextFont(thePort);
	*fontsize = GetPortTextSize(thePort);
	*fontstyle = GetPortTextFace(thePort);
	
	#else
	//old code
	*fontnum = (*qd.thePort).txFont;
	*fontsize = (*qd.thePort).txSize;
	*fontstyle = (*qd.thePort).txFace;
	#endif
#endif
#ifdef WIN95VERSION
	*fontnum = currentFontNum;
	*fontsize = currentFontSize;
	*fontstyle = currentFontStyle;
#endif
	} /*getfontsizestyle*/


#if 0

void fontstring (short fontnum, short fontsize, boolean flhavefont, boolean flhavesize, bigstring bs) {
	
	bigstring bsint;
	
	setstringlength(bs,0); /*set it to the empty string*/
	
	if (flhavefont)
		fontgetname (fontnum, bs);
		
	if (flhavesize) {	
	
		NumToString (fontsize, bsint);
		
		if (flhavefont) {
			
			pushspace (bs);
			
			pushstring (bsint, bs);	
			}		
		else
			copystring ((ptrstring) "\pno consistent font", bs);
		}
	} /*fontstring*/



void getstyle (style, flplain, flbold, flitalic, flunderline, floutline, flshadow) 

	/*
	give me a style bit array, and I'll extract the booleans which are 
	slightly easier to deal with.
	
	if none of the others are true, we set flplain to true.  otherwise 
	flplain is false.
	*/

	short style; 
	boolean *flplain, *flbold, *flitalic, *flunderline, *floutline, *flshadow; 
	
	{
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

#endif


void diskgetfontname (short fontnum, diskfontstring fontname) {
	
	/*
	Apple recommends that fonts be stored on disk as strings.  we return the
	fontname, limited in length to 32, based on the indicated font number.
	*/
	
	bigstring bs;
	
	fontgetname (fontnum, bs);
	
	if (stringlength (bs) > diskfontnamelength)
		setstringlength (bs, diskfontnamelength);
	
	copystring (bs, (ptrstring) fontname);
	} /*disksavefontname*/


void diskgetfontnum (diskfontstring fontname, short *fontnum) {
	
	if (stringlength (fontname) == 0)
		*fontnum = geneva;
	else
		fontgetnumber ((ptrstring) fontname, fontnum);
	} /*diskloadfontname*/
	
		
boolean initfonts (void) {
	
	#ifdef WIN95VERSION
		HDC hdc;
		short fontnum;

		hdc = GetDC (GetDesktopWindow());

		globalFontCount = 0;

		EnumFontFamilies(hdc, (LPCTSTR) NULL, 
			(FONTENUMPROC) EnumFamCallBack1, (LPARAM) &globalFontCount); 

		globalWindowsFontInfo = (tywindowsfontinformation **) NewHandle ((globalFontCount + 1) * sizeof (tywindowsfontinformation));

		if (globalWindowsFontInfo != NULL) {
			globalFontCount = 0;

			EnumFontFamilies(hdc, (LPCTSTR) NULL, 
				(FONTENUMPROC) EnumFamCallBack2, (LPARAM) &globalFontCount); 
			}


		fontgetnumber ("\005Arial", &fontnum);
		setglobalfontsizestyle (fontnum, 12, 0);		/* set defaults*/
		ReleaseDC (GetDesktopWindow(), hdc);

		initgenevafont ();
	#endif
	
	setemptystring (cachedfontname);
	
	return (true);
	} /*initfonts*/


