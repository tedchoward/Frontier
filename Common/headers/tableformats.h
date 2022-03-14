
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

#ifndef tableformatsinclude
#define tableformatsinclude


#ifndef shellinclude

	#include "shell.h"

#endif


#ifndef langinclude

	#include "lang.h"

#endif

#ifndef opinclude

	#include "op.h"

#endif

#ifndef claylinelayoutinclude

	#include "claylinelayout.h"

#endif

#include "tabledisplay.h"



#define maxtablecols 3
#pragma pack(2)
typedef struct tytableformats { /*one of these for every window that's open*/
	
	hdlhashtable htable; // the parent table we are displaying
	
	hdloutlinerecord houtline; // the outline viewing the table
	
	short fontnum, fontsize, fontstyle; // only meaningful when outline was not saved and hasn't been recreated
	
	long lnumcursor; // ditto
	
	long vertcurrent; // ditto
	
	short ctcols;
	
	short defaultcolwidth;
	
	short colwidths [maxtablecols];
	
	short maxwidths [maxtablecols];
	
	Rect windowrect; /*the position & size of window last time table was packed*/
	
	Rect wholerect; /*the union of all rects that make up the table display*/
	
	Rect tablerect; /*the rectangle that the table is displayed in*/
	
	Rect titlerect; /*where the column titles are stored*/
	
	Rect iconrect;
	
	Rect kindpopuprect;
	
	Rect sortpopuprect;
	
	short sorttitlecol; /*the column that is underlined as the current sort*/
	
	short focuscol;
	
	short editcol; /*zero-based col when in text mode*/
	
	short undocol; /*column that last had it's undoglobals set*/
	
	Handle editval; /*when editing a value, the original text*/
	
	hdlheadrecord editnode; /*the node being edited*/
	
	hdlheadrecord lastbarcursor; /*so we can detect motion*/
	
	tylinelayout linelayout;
	
	tycomputedlineinfo computedlineinfo;
	
	boolean fliconenabled: 1; /*show the icon dimmed or not dimmed*/
	
	boolean fldirty; /*any changes made that aren't saved?*/
	
	boolean flactive;
	
	boolean flwindowsicons; /*the icons look like those used in Microsoft Windows apps*/
	
	boolean flneedresort; /*table needs to be resorted (after unpacking)*/
	
	boolean flprinting;
	
	long refcon;
	} tytableformats, *ptrtableformats, **hdltableformats;


/* This is an old version structure used only for conversion purposes*/

typedef struct tyversion1tablediskrecord { /*packed version of tableformats, suitable for disk storage*/

	short versionnumber; /*this record is stored on disk*/
	
	short rowcursor, colcursor; /*locates the cursor*/
	
	short ctrows, ctcols; /*dimensions of the list record*/
	
	/*in v1 fontgarbage used to store the font number -- this was a mistake!*/
	
	short fontgarbage, fontsize, fontstyle; /*the display fontsizestyle*/
	
#ifdef MACVERSION
	tylinespacing linespacing;
#endif
#ifdef WIN95VERSION
	short linespacing;
#endif
	
	short vertmin, vertmax, vertcurrent; /*values for the scrollbars*/
	
	short horizmin, horizmax, horizcurrent; /*values for the scrollbars*/
	
	long timecreated, timelastsave; /*maybe we'll use these at some later date?*/
	
	long ctsaves; /*the number of times this structure has been saved*/
	
	long sizerowarray, sizecolarray; /*sizes of two arrays stored at end of disk record*/
	
#ifdef version42orgreater
	short flags;
#else
	unsigned short fleditingcell: 1;
	
	unsigned short flgridlines: 1;
	
	unsigned short flrowsizingenabled: 1; /*if true, user is allowed to resize row heights*/
	
	unsigned short flcolsizingenabled: 1; /*if true, user is allowed to resize column widths*/
	
	unsigned short flbarcursor: 1; /*if true, cursor is the current row, not just current cell*/
	
	unsigned short flgrayoffbottom: 1; /*ditto, off the bottom of real cells*/
	
	unsigned short flautorowheight: 1; /*recalc all row heights after font/size change*/
	
	unsigned short flcoltitles: 1; /*if turned on, leave room for column titles*/
#endif

	diskfontstring fontname; /*a bit of Mac culture, font specified by a string, not a number*/

#ifdef version42orgreater
	unsigned short flags2;
#else	
	unsigned short flcenterifpossible: 1; /*center the table, otherwise flush with top & left*/
	
	unsigned short flsavedsorted: 1; /*if true, version is recent enough to retain sort*/
#endif

	diskrect windowrect; /*the position & size of window last time table was packed*/
	
	short sortorder; /*up to the application to understand what this means*/
	
	char growtharea [20];
	
	/*the variable-length intarrays are tacked on at the end of this record*/
	} tyversion1tablediskrecord, *ptrversion1tablediskrecord, **hdlversion1tablediskrecord;
	

typedef struct tyversion2tablediskrecord { /*packed version of tableformats, suitable for disk storage*/
	
	short versionnumber; /*this record is stored on disk*/
	
	short recordsize;
	
	diskfontstring fontname; // only meaningful when outline was not saved and hasn't been recreated
	
	short fontsize, fontstyle; // ditto
	
	short lnumcursor; // ditto
	
	short vertcurrent; // ditto
	
	short ctcols; /*dimensions of the list record*/
	
	short colcursor; /*locates the cursor*/
	
	short colwidths [maxtablecols];
	
	diskrect windowrect; /*the position & size of window last time table was packed*/
	
	boolean savedoutline; /*is an outline packed along with this record? (only if expanded)*/
	
	boolean savedlinelayout; //is a clay linelayout packed along with this record?
	
	short lnumcursor_hiword; //5.1.3
	
	short vertcurrent_hiword; //5.1.3
	
	short growtharea [8];
	} tyversion2tablediskrecord, *ptrversion2tablediskrecord, **hdlversion2tablediskrecord;
#pragma options align=reset	

/*globals*/
	
extern WindowPtr tableformatswindow; 

extern hdltableformats tableformatsdata; 

extern hdlwindowinfo tableformatswindowinfo; 


/*prototypes*/

extern boolean tablepushformats (hdltableformats);

extern boolean tablepopformats (void);

extern void tabledirty (void);

extern short tablegetcolwidth (short);

extern boolean tablesetcolwidth (short, short, boolean);

extern short tablesumcolwidths (short, short);

extern short tabletotalcolwidths (void);

extern short tableavailwidth (void);

extern boolean tablerecalccolwidths (boolean);

extern boolean newtableformats (hdltableformats *);

extern void disposetableformats (hdltableformats);

extern boolean tablenewformatsrecord (hdlhashtable, Rect, hdltableformats *);

extern boolean tableprepareoutline (hdltableformats);

extern boolean tablepackformats (Handle *);

extern boolean tableunpackformats (Handle, hdltableformats);

extern void tabledisposeoutline (hdltableformats);

extern boolean tableoutlineneedssaving (void);

#endif


