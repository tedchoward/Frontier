
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

#define clayinclude


#include <standard.h>
typedef bigstring tyfoldername;
#define fastunicaseequalstrings equalidentifiers

#include "op.h"
#include "tableeditor.h"
#include "claylinelayout.h"


#define maxtablecols 3

#define outlinetopinset 3
#define outlinebottominset 0
#define outlineleftinset 3
#define outlinerightinset 0


#define browserfiletype 'FSYS'
#define outlinerfiletype 'OUTL'
#define processlistfiletype 'PROC'
#define scriptfiletype '2CLK'
#define textfiletype 'TEXT'
#define styledtextfiletype 'WPTX'
#define msgviewerfiletype 'MSGV'
#define msgeditorfiletype 'MSGE'
#define bbsoutlinefiletype 'MSGO'
#define messagereffiletype 'MREF'
#define bookmarkfiletype 'BOOK'
#define websitefiletype 'WBST'



#define ctwindowtypes 7
	

/*these macros map our names onto the app0 - app4 bits in each hdlheadrecord*/
	
	#define flnodeisfolder appbit2
	#define flnodeunderlined appbit3
	#define flnodeonscrap appbit4
	#define tmpbit2 appbit5
	#define flnodeneedsbuild appbit6
	#define flnewlyinsertednode appbit7


#define maxicons 15 /*10/28/93 DW: used to be 5*/

typedef boolean (*tygetmenucallback) (short);

typedef boolean (*tyidlecallback) (short);

typedef boolean (*tymenuselectcallback) (short, short);

typedef boolean (*tybuttonhitcallback) (short);

typedef struct tyclaystatusicon {
	
	hdlstring foldername; /*name of the icon's special folder, empty string if it's computed*/
	
	hdlstring helpstring; /*displayed in message area when icon is activated*/
	
	hdlstring buttontext; /*if it's a text button, this is the text inside the button*/
	
	Rect iconrect; /*the rectangle that the icon occupies*/
	
	short iconresnum; /*the resource id of the icon family*/
	
	MenuHandle hmenu; /*nil if icon has no popup*/
	
	boolean flpopup; /*if true, mouse click pops up a menu*/
	
	boolean flsingleclickpopup; /*if true, user must hold down mouse to get popup*/
	
	boolean fldisposemenu; /*if true, we dispose of the menu handle after running it*/
	
	boolean flbreakafter; /*start a new subgroup after this icon*/
	
	boolean fltextbutton; /*if true it's a text button, otherwise it's an icon*/
	
	boolean flnofolder; /*if true, the icon doesn't have a related sub-folder in the Prefs folder*/
	
	boolean fltoggles; /*if true, the icon toggles pressed and not pressed. effects interaction, a little*/
	
	boolean flpressed; /*if true, we display the button in its pressed state*/
	
	boolean fldisabled; /*if true, the button is disabled*/
	
	boolean flcustomrect; /*if true, we don't compute the icon's rect*/
	
	short idpopupmenu; /*the resource id of the popup menu*/
	
	tygetmenucallback getmenucallback;
	
	tymenuselectcallback menuselectcallback;
	
	tybuttonhitcallback buttonhitcallback;
	
	tyidlecallback idlecallback;
	
	long refcon; /*some icons want to keep their own data around*/
	} tyclaystatusicon;
	

typedef boolean (*tyclaycallback) (void);

typedef boolean (*tyoutlinecallback) (hdloutlinerecord);

typedef boolean (*tyshortstarcallback) (short *);

typedef boolean (*tymenuhitcallback) (short, short);


typedef struct tyclaydata { /*one of these for every window that's open*/
	
	hdlhashtable htable; // the parent table we are displaying
	
	hdloutlinerecord houtline; // the outline viewing the table
	
	hdltableformats hformats;
	
	tylinelayout linelayout; 
	
	tycomputedlineinfo computedlineinfo;
	
	short ctcols;
	
	short colwidths [maxtablecols];
	
	short maxwidths [maxtablecols];
	
	short cticons; /*the number of icons in the status bar of this window*/
	
	/*start tyextrainfo*/
	
	Rect wholerect; /*the union of all rects that make up the table display*/
	
	Rect tablerect; /*the rectangle that the table is displayed in*/
	
	Rect titlerect; /*where the column titles are stored*/
	
	Rect seprect; /*the area separating the table display from the titles*/
	
	Rect iconrect;
	
	Rect kindpopuprect;
	
	Rect sortpopuprect;
	
	short focuscol;
	
	short editcol; /*zero-based col when in text mode*/
	
	Handle editval; /*when editing a value, the original text*/
	
	hdlheadrecord editnode; /*the node being edited*/
	
	short ixicontitle; /*stringlist index of icon title*/
	
	/*end tyextrainfo*/
	
	boolean fliconenabled: 1; /*show the icon dimmed or not dimmed*/
	
	boolean flautocreated: 1; /*we're editing a auto-created value*/
	
	boolean fltexticons; /*we do different button alignment for these windows*/
	
	boolean flwindowsicons; /*the icons look like those used in Microsoft Windows apps*/
	
	boolean fluseslinelayout; /*uses claylinelayout.c routines to draw the outline*/
	
	boolean flfilebrowser; /*it's an outliner used to browse nodes which are files*/
	
	tyclaystatusicon iconarray [maxicons];
	
	hdlheadrecord lastbarcursor; /*so we can detect motion*/
	
	boolean flresetinfowindows; /*if true, force a reload of the info windows*/
	
	boolean iconhighlighted; /*draw the icon as if it were selected*/
	
	boolean flbitmap; /*true if an offscreen bitmap is open*/
	
	boolean forkedwindow; /*if true it's a fork off another window*/
	
	boolean immediatesystemidle; /*set true as the window is activated*/
	
	boolean processingscriptevent; /*don't allow user events to break the idle handler*/
	
	boolean eraseonupdate; /*true until the next update event is handled*/
	
	boolean fljumponidle;
	
	tyclaycallback keystrokecallback;
	
	tyclaycallback idlecallback;
	
	tyoutlinecallback exportscrapcallback;
	
	tyclaycallback initstatusbarcallback;
	
	tyshortstarcallback getstatusbarwidthcallback;
	
	tyclaycallback drawstatusbarcallback;
	
	tyclaycallback setcustomrectscallback;
	
	tyclaycallback statusclickcallback;
	
	tyclaycallback cursorinstatusbarcallback;
	
	tymenuhitcallback menuhitcallback;
	
	tyclaycallback beforeclosewindowcallback;
	
	tyclaycallback handleverbcallback, handlefastverbcallback;
	
	Handle hmoredata; /*non-outline data handle*/
	} tyclaydata, **hdlclaydata;
	
	
	

boolean claybrowserkeystroke (void); /*used by browser & process windows*/

//boolean claypushwindowglobals (tywindowtype, boolean);

void claypopwindowglobals (void);

//boolean clayfindwindow (tywindowtype, boolean, hdlwindowinfo *);

short claymsgclick (short, short);

void clayidleanalysis (boolean *, boolean *, boolean *);

void clayidleforall (void);

boolean claygetfrontwindow (hdlwindowinfo *);

boolean claygetfrontoutline (hdloutlinerecord *);

boolean claywindowcontainsoutline (hdlclaydata);

boolean clayfrontwindowcontainsoutline (void);

boolean clayopendoc (FSSpec *);

boolean claypushpopupitem (short, bigstring, boolean);

boolean claynewappwindow (OSType);
