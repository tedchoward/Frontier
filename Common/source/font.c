
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

#include "PAIGE.H"
#include "PGDEFTBL.H"

#include "strings.h"
#include "font.h"
#include "quickdraw.h"


FontInfo globalfontinfo;

static bigstring cachedfontname;
static short cachedfontnum;


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
	
	GetFNum (bsfont, &cachedfontnum);

	if ((cachedfontnum == 0) && !equalstrings (bsfont, BIGSTRING ("\x07" "Chicago")))
		if (mapxfont (bsfont))
			GetFNum (bsfont, &cachedfontnum);
	
	exit:
	
	*fontnumber = cachedfontnum;
	} /*fontgetnumber*/


void fontgetname (short fontnumber, bigstring fontname) {
	GetFontName (fontnumber, fontname);
	} /*fontgetname*/


void setfontsizestyle (short fontnum, short fontsize, short fontstyle) {
	TextFont (fontnum);
	
	TextSize (fontsize);
	
	TextFace (fontstyle);
	} /*setfontsizestyle*/
	
	
void setglobalfontsizestyle (short fontnum, short fontsize, short fontstyle) {
	setfontsizestyle (fontnum, fontsize, fontstyle);
	
	GetFontInfo (&globalfontinfo);

	} /*setglobalfontsizestyle*/

	

boolean realfont (short fontnum, short fontsize) {
	return (RealFont (fontnum, fontsize));
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



void getfontsizestyle (short *fontnum, short *fontsize, short *fontstyle) {
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
	
	
	setemptystring (cachedfontname);
	
	return (true);
	} /*initfonts*/


