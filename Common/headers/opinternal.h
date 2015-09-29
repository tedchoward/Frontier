
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

#ifndef opinternalinclude 
#define opinternalinclude 



#ifndef opinclude

	#include "op.h"
	
#endif

#ifndef opscreenmapinclude

	#include "opscreenmap.h"
	
#endif

#ifndef opdisplayinclude

	#include "opdisplay.h"
	
#endif


#define outlineistable 'otbl'
#define outlineisoutline 'oout'
#define outlineismenubarscript 'ombs'
#define outlineisdatabasescript 'odbs'
#define outlineisstandalonescript 'osas'


#define undocutstring 4
#define undocopystring 5
#define undopastestring 6
#define undoclearstring 7
#define undotypingstring 8
#define undomovestring 9
#define undosortstring 10
#define undopromotestring 11
#define undodemotestring 12
#define undodeletionstring 13
#define undoformatstring 14


#define textvertinset 2 /*leaves room for a nice frame around the bar cursor*/
#define texthorizinset 2

#define textleftslop 5
#define textrightslop 8

#define iconstart 5 /*this has got to go*/

#pragma pack(2)
typedef struct tyscraprecord {
	
	boolean fldirty; /*needs to be put into the desktop scrap*/
	
	Handle htext; /*if not nil, points to a chunk of text*/
	
	hdlheadrecord hnode; /*if not nil, points to an outline structure*/
	} tyscraprecord;


typedef struct tyopuserprefs {
	
	boolean fltabkeyreorg; /*tab, shift/tab to reorg right, left?*/
	
	boolean flflatcursorkeys; /*up/down are flat*/
	} tyopuserprefs;
#pragma options align=reset


typedef boolean (*opundocallback) (hdlheadrecord, boolean);


#define ctoutlinestack 10 /*we can remember outline contexts up to 10 levels deep*/

extern short topoutlinestack;

extern hdloutlinerecord outlinestack [ctoutlinestack];


/*globals*/

extern boolean flforrelease;

extern boolean flinternalchange;

extern tyopuserprefs opprefs; /*user preferences*/


/*prototypes*/

extern void opinitcallbacks (hdloutlinerecord houtline); /*opinit.c*/

extern boolean opdefaultreleaserefconroutine (hdlheadrecord hnode, boolean fldisk);

extern boolean opdefaultsetscraproutine (hdloutlinerecord houtline);


extern boolean opdefaultadjustcursor (hdlheadrecord, Point, const Rect *); /*op.c*/

extern boolean opdefaultreturnkey (tydirection);

extern void opdirtyoutline (void);

extern void opdirtyview (void);

extern void oppoststylechange (void);

extern boolean opsetfont (short);

extern boolean opsetsize (short);

extern boolean opsetselectioninfo (void);

extern boolean opscraphook (Handle);

extern void opresize (Rect);

extern void opupdate (void);

extern boolean opsetcursor (Point);

extern boolean opmousedown (Point, tyclickflags);

extern boolean opkeystroke (void);

extern boolean opcmdkeyfilter (char);

extern boolean opselectall (void);

extern void opidle (void);

extern void opactivate (boolean);


extern tydirection opdowndir (void); /*opops.c*/

extern tydirection opupdir (void);

extern boolean opfirstinlist (hdlheadrecord);

extern boolean oplastinlist (hdlheadrecord);

extern boolean opsubordinateto (hdlheadrecord, hdlheadrecord);

extern boolean opnestedincomment (hdlheadrecord);

extern hdlheadrecord opgetancestor (hdlheadrecord, short);

extern hdlheadrecord opgetlastsubhead (hdlheadrecord);

extern boolean opnthsummit (long, hdlheadrecord *);

extern boolean opgetnthnode (long, hdlheadrecord *);

extern boolean oparrayreference (long, hdlheadrecord *);

extern hdlheadrecord oplastexpanded (hdlheadrecord);

extern hdlheadrecord oplastexpandedatlevel (hdlheadrecord, short);

extern boolean opchaseup (hdlheadrecord *);

extern boolean opchasedown (hdlheadrecord *);

extern boolean opchaseleft (hdlheadrecord *);

extern boolean opchaseright (hdlheadrecord *);

extern void opsummitof (hdlheadrecord, hdlheadrecord *);

extern void opfirstsibling (hdlheadrecord, hdlheadrecord *);

extern void opgetsiblingnumber (hdlheadrecord, long *);

extern hdlheadrecord opgetnextexpanded (hdlheadrecord hnode);

extern hdlheadrecord opbumpflatdown (hdlheadrecord, boolean);

extern hdlheadrecord opbumpflatup (hdlheadrecord, boolean);

extern hdlheadrecord oprepeatedbump (tydirection, long, hdlheadrecord, boolean);

extern boolean opnavigate (tydirection, hdlheadrecord *);

extern boolean opfindhead (hdlheadrecord, bigstring, hdlheadrecord *);

extern boolean opbestfind (hdlheadrecord hfirst, bigstring bs, hdlheadrecord *hnode);

extern long opcountatlevel (hdlheadrecord);

extern hdlheadrecord op1stsibling (hdlheadrecord hnode);

extern long opcountsubs (hdlheadrecord);

extern long opsetctexpanded (hdloutlinerecord);

extern void opsetexpandedbits (hdlheadrecord, boolean);

extern boolean opcontainsnode (hdlheadrecord, hdlheadrecord);

extern void opgetnodeline (hdlheadrecord, long *);

extern boolean opnewheadrecord (Handle, hdlheadrecord *);

extern boolean opnewstructure (Handle, hdlheadrecord *);

extern boolean opnewsummit (void);

extern boolean newoutlinerecord (hdloutlinerecord *);

extern boolean opnewrecord (Rect, hdloutlinerecord *);

extern boolean opreleasevisit (hdlheadrecord, ptrvoid);

extern void opreleasenode (hdlheadrecord, boolean);

extern void opdisposestructure (hdlheadrecord, boolean);

extern void opdisposeoutline (hdloutlinerecord, boolean);

extern boolean optexttooutline (hdloutlinerecord, Handle, hdlheadrecord *);

extern boolean opemptysummit (hdlheadrecord);

extern boolean opemptyoutlinerecord (hdloutlinerecord);

extern hdlheadrecord opfirstatlevel (hdlheadrecord);

extern long opcountsubheads (hdlheadrecord, short);

extern boolean opfindnode (hdlheadrecord, bigstring, short, hdlheadrecord *);

extern void opresetlevels (hdlheadrecord);

extern long opgetlnum (hdlheadrecord);

extern short opgetdirectionicon (tydirection);

extern void oppushstyle (hdloutlinerecord);

extern void oppushcolors (hdloutlinerecord);

extern boolean opbitmaps (boolean, boolean *, boolean *);

extern boolean opsetactualheadstring (hdlheadrecord, bigstring);

extern boolean opsetheadtext (hdlheadrecord, Handle);

extern boolean opsetheadstring (hdlheadrecord, bigstring);

extern boolean opgetheadstring (hdlheadrecord, bigstring);

extern void opgetsortstring (hdlheadrecord, bigstring);

extern void opcopyformatting (hdloutlinerecord, hdloutlinerecord);


extern boolean opsetprintinfo (void); /*opprint.c*/

extern void opgetprintrect (Rect *);

extern boolean opbeginprint (void);

extern boolean opendprint (void);

extern boolean opprint (short);


extern boolean opgetscrollbarinfo (boolean); /*opscrollbar.c*/

extern boolean opsetscrollpositiontoline1 (void);

extern boolean oprestorescrollposition (void);

extern void opredrawscrollbars (void);

extern void opresetscrollbars (void);


extern void opstartinternalchange (void); /*opstructure.c*/

extern void opendinternalchange (void);

extern boolean opinternalchange (void);

extern boolean oppushundo (opundocallback, hdlheadrecord);

extern void opunlink (hdlheadrecord);

extern boolean opdeposit (hdlheadrecord, tydirection, hdlheadrecord);

extern boolean opcopyrefconroutine (hdlheadrecord, hdlheadrecord);

extern boolean oppushunmarkundo (hdlheadrecord);

extern boolean opbeforestrucchange (hdlscreenmap *, boolean);

extern boolean opafterstrucchange (hdlscreenmap, boolean);

extern boolean opsortlevel (hdlheadrecord);

extern boolean opreorgcursor (tydirection, long);

extern void opsetline1 (hdlheadrecord);

extern boolean opdeletesubs (hdlheadrecord);

extern hdlheadrecord opcopyoutline (hdlheadrecord);

extern boolean opcopysiblings (hdlheadrecord, hdlheadrecord *);

extern boolean opcopyoutlinerecord (hdloutlinerecord, hdloutlinerecord *);

extern void opclearallmarks (void);

extern void opclearmarks (hdlheadrecord);

extern boolean opgetmark (hdlheadrecord);

extern void opsetmark (hdlheadrecord, boolean);

extern boolean opanymarked (void);

extern void opdeleteline (void);

extern boolean opsettmpbitvisit (hdlheadrecord, ptrvoid);

extern boolean opcleartmpbits (void);


extern void opdraggingmove (Point, hdlheadrecord); /*opdraggingmove.c*/


extern boolean opeditsetglobals (void); /*opedit.c*/

extern boolean opdefaultgetedittextrect (hdlheadrecord, const Rect *, Rect *);

extern boolean opdefaultsetwpedittext (hdlheadrecord);

extern boolean opdefaultgetwpedittext (hdlheadrecord, boolean);

extern boolean opseteditbufferrect (void);

extern boolean oploadeditbuffer (void);

extern boolean opwriteeditbuffer (void);

extern boolean opunloadeditbuffer (void);

extern boolean opsaveeditbuffer (void);

extern boolean oprestoreeditbuffer (void);

extern boolean opeditmeasuretext (hdlheadrecord);

extern boolean opeditdrawtext (hdlheadrecord, const Rect *);

extern void opeditgetmaxpos (long *);

extern void opeditgetselection (long *, long *);

extern void opeditsetselection (long, long);

extern void opeditgetseltext (bigstring);

extern void opeditgetselrect (Rect *);

extern void opeditgetselpoint (Point *);

extern void opeditresetselpoint (void);

extern void opeditsetselpoint (Point);

extern boolean opeditcango (tydirection);

extern boolean opeditkey (void);

extern boolean opeditcopy (void);

extern boolean opeditcut (void);

extern boolean opeditpaste (void);

extern boolean opeditclear (void);

extern boolean opeditinsert (bigstring);

extern boolean opeditclick (Point, tyclickflags);

extern void opeditselectall (void);

extern void opeditactivate (boolean);

extern void opeditupdate (void);

extern void opeditidle (void);

extern void opeditdispose (void);

extern boolean opeditgetundoglobals (long *);

extern boolean opeditsetundoglobals (long, boolean);

#endif


