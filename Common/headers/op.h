
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

#ifndef opinclude
#define opinclude /*so other includes can tell if we've been loaded*/

#include "lang.h" /*7.0b16 PBS*/


/*
if you're building a full structure editor, then set this to false, 
otherwise, if you're building a program that just packs and unpacks
structures, set it true.
*/

#ifndef fljustpacking

	#if !flruntime

		#define fljustpacking false

	#else
		
		#define fljustpacking true

	#endif

#endif


#if !fljustpacking /*packing doesn't need to include any of these*/

	#ifndef shellinclude
	
		#include "shell.h"
	
	#endif

#endif


#define cthoists 5 /*depth of hoist stack*/

typedef struct tyoutlinerecord *ptroutlinerecord, **hdloutlinerecord; /*forward declaration*/

typedef struct tyheadrecord *ptrheadrecord, **hdlheadrecord;/*forward declaration*/

typedef struct tyheadrecord {
	
	struct tyheadrecord **headlinkdown, **headlinkup, **headlinkleft, **headlinkright;
	
	short headlevel;
	
	boolean flexpanded: 1;
	
	boolean fldirty: 1; /*set this true to force an update of the line*/
	
	boolean flmarked: 1; /*did the user shift-click on the headline?*/
	
	boolean tmpbit: 1; /*can be used as a temporary mark for a variety of tasks*/
	
	boolean fllocked: 1; /*is this line locked?*/
	
	boolean flcomment: 1; /*is this line a language comment?*/
	
	boolean flbreakpoint: 1; /*in a script, is a breakpoint set here?*/
	
	boolean fldrawelipsizes : 1; /*hack for clay basket drawing routines*/

	boolean fldynamic: 1; /*7.0b30: if a headline is dynamic, children aren't saved when serialized as OPML.*/
	
	boolean appbit0: 1; /*a bit that the application can use -- saved in packed format*/
	
	boolean appbit1: 1; /*a bit that the application can use -- saved in packed format*/
	
	boolean appbit2: 1; /*a bit that the application can use -- not saved*/
	
	boolean appbit3: 1; /*a bit that the application can use -- not saved*/
	
	boolean appbit4: 1; /*a bit that the application can use -- not saved*/
	
	boolean appbit5: 1; /*a bit that the application can use -- not saved*/
	
	boolean appbit6: 1; /*a bit that the application can use -- not saved*/
	
	boolean appbit7: 1; /*a bit that the application can use -- not saved*/

	short hpixels; //width of headline text; -1 is dirty value
	
	short vpixels; //height of headline text; -1 is dirty value
	
	Handle hrefcon; /*for use by application, see opgetrefcon*/
	
	Handle headstring; /*text of headline lives in its own block*/
	} tyheadrecord;


typedef struct tyhoistelement {
	
	hdlheadrecord hhoisted;
	
	hdlheadrecord hbarcursor;
	
	hdlheadrecord hsummit;
	
	long lnumbarcursor;
	
	hdlheadrecord hline1;
	} tyhoistelement;
	

typedef struct tytextinfo { /*selected fields from a TERec*/
	
	boolean flvalid; /*true if there is valid stuff in the rest of the record*/
	
	long selStart, selEnd; 
	} tytextinfo;


typedef enum tydraggingop { 
	
	draginit = 1, 
	
	dragtrack = 2, 
	
	dragdrop = 3, 
	
	dragclose = 4
	} tydraggingop;

#define opdirtymeasurevalue (-1)

typedef boolean (*opvisitcallback) (hdlheadrecord, ptrvoid);

typedef boolean (*opnodecallback) (hdlheadrecord);

typedef boolean (*opvoidcallback) (void);

typedef boolean (*opstringcallback) (bigstring);

typedef boolean (*oppreexpandcallback) (hdlheadrecord, short, boolean);

typedef boolean (*op2nodescallback) (hdlheadrecord, hdlheadrecord);

//typedef boolean (*opgeticoncallback) (hdlheadrecord, short *, short *);

typedef boolean (*opdirectioncallback) (tydirection);

typedef boolean (*opdragtargetcallback) (hdlheadrecord *, tydirection *);

typedef boolean (*opvalidatecallback) (hdlheadrecord, hdlheadrecord, tydirection);

typedef boolean (*opdrawiconcallback) (hdlheadrecord, boolean, Rect *);

typedef boolean (*opgetrectcallback) (hdlheadrecord, const Rect *, Rect *);

//typedef boolean (*opgetlineinfocallback) (hdlheadrecord, Rect *, short *);

typedef boolean (*opgefullrectcallback) (hdlheadrecord, Rect *);

typedef boolean (*opsetscrapcallback) (hdloutlinerecord);

typedef boolean (*opgetscrapcallback) (hdloutlinerecord *, boolean *);

typedef boolean (*optextchangedcallback) (hdlheadrecord, bigstring);

typedef boolean (*opcopyrefconcallback) (hdlheadrecord, hdlheadrecord);

typedef boolean (*opnodebooleancallback) (hdlheadrecord, boolean);

typedef boolean (*optextualizerefconcallback)(hdlheadrecord, Handle);

typedef boolean (*opgetlineshortcallback) (hdlheadrecord, short *);

typedef boolean (*opdrawcallback) (hdlheadrecord, const Rect *, boolean, boolean);

typedef boolean (*opmousecallback) (hdlheadrecord, Point, const Rect *);

typedef boolean (*opmousebooleancallback) (hdlheadrecord, Point, const Rect *, boolean *);

typedef boolean (*opgetwindowhandlecallback) (Point, Handle *);

typedef boolean (*opsetwindowhandlecontextcallback) (Handle);

typedef	boolean (*optexttooutlinecallback) (hdloutlinerecord, Handle, hdlheadrecord *);


typedef struct tyoutlinerecord {
	
	hdlheadrecord hsummit; /*the first summit of the structure*/
	
	hdlheadrecord hbarcursor; /*the line the bar cursor is on*/
	
	hdlheadrecord hline1; /*this node is displayed on the 0th line in the window*/
	
	long lnumbarcursor; /*line number that the bar cursor is on, may be off screen*/
	
	short line1linesabove; /*text lines of hline1 that are scrolled above the display*/
	
	long ctexpanded; /*number of nodes expanded*/
	
	long ctmarked; /*number of nodes selected, or marked*/
	
	short fontnum, fontsize, fontstyle; /*the default display font*/
	
	short lineindent; /*how many pixels to indent for each level*/
	
	short defaultlineheight; /*if the caller doesn't override, this is the height of each line*/
	
	tylinespacing linespacing; /*a vestige, it's preserved, but ignored*/
	
	Rect outlinerect; /*the rectangle that the outline is displayed in*/
	
	Rect windowrect; /*the size and position of window that last displayed outline*/
	
	tyscrollinfo vertscrollinfo; /*vertical scrollbar info*/
	
	tyscrollinfo horizscrollinfo; /*horiz scrollbar info*/
	
	unsigned long timecreated, timelastsave; /*number of seconds since 1/1/04*/
	
	long ctsaves; /*the number of times this structure has been saved*/
	
	RGBColor forecolor; /*5.0b11 dmb: you can't really have a backcolor without a forecolor*/
	
	RGBColor backcolor; /*DW 7/13/93 -- allow applet to set the display background color*/
	
	#if !fljustpacking /*these fields allow structure to be edited, not needed for packing*/
		
		tyhoistelement hoiststack [cthoists]; /*return addresses for hoists*/
	
		short tophoist; /*if 0, nothing is hoisted*/
		
		short editcolumn; /*for multiple column outlines, maintained by callbacks*/
		
		Handle hbuffer; /*holds the text currently being edited by wp*/
		
		hdlheadrecord heditcursor; /*the headline who's text hbuffer holds*/
		
		Point selpoint; /*for maintaining horizontal position when cursoring up & down*/
		
		tytextinfo textinfo; /*saved information about edit buffer selection*/
		
		short caretoffset; /*for showing error locations*/
		
		hdlheadrecord hprintcursor; /*when printing, this maintains position between pages*/
		
		short pixelsaftericon; /*skip this many pixels between icon and text*/
		
		short iconheight, iconwidth; /*number of pixels used to display the icon*/
		
		opvoidcallback setscrollbarsroutine; /*we call this routine to set the op's scrollbars*/
		
		opvoidcallback measurelinecallback; /*we call this routine to measure the length of a line*/
		
		opnodecallback setcursorcallback; /*we call this routine to shape the cursor in a line*/
		
		opmousebooleancallback mouseinlinecallback; /*we call this routine when the user clicks in a line*/
		
		opvoidcallback postfontchangecallback; /*called after font or size has been changed*/
		
		// unused - opgetlineinfocallback getlinedisplayinfocallback; /*called to get display info about a line*/
		
		opcopyrefconcallback copyrefconcallback; /*copies the refcon handle linked into the headrecord*/
		
		optextualizerefconcallback textualizerefconcallback; /*pack contents of refcon handle to text*/
		
		opnodecallback printrefconcallback; /*print contents of refcon handle*/
		
		opvoidcallback beforeprintpagecallback; /*after resizing, before printing a page*/
		
		opvoidcallback afterprintpagecallback; /*after printing a page and restoring rects*/
		 
		opnodecallback searchrefconcallback; /*searches contents of refcon per searchparams*/
		
		opnodecallback deletelinecallback; /*called before a line is deleted*/
		
		opnodecallback insertlinecallback; /*called after a line is inserted*/
		
		optextchangedcallback textchangedcallback; /*called when the text of a line has changed*/
		
		opnodecallback hasdynamicsubscallback; /*called to find out if a node can generate subheads if expanded*/
		
		opnodecallback haslinkedtextcallback; /*called to find out if a node has anything linked to it*/
		
		opnodecallback cmdclickcallback; /*called when the user cmd-clicks on a headline*/
		
		opvoidcallback doubleclickcallback; /*called after the user double-clicks in text*/
		
		opgetscrapcallback getscrapcallback; /*called to get a headline scrap from the clipboard*/
		
		opsetscrapcallback setscrapcallback; /*called to put a headline scrap onto the clipboard*/
	
		optexttooutlinecallback texttooutlinecallback; /*called to convert a text scrap to an outline=*/
		
		oppreexpandcallback preexpandcallback; /*called before expanding a node*/
		
		opnodecallback postcollapsecallback; /*called after collapsing a node*/
		
		op2nodescallback comparnodescallback; /*called after collapsing a node*/
		
		// unused - opgeticoncallback getleadericoncallback;
		
		opvalidatecallback validatedragcallback;
		
		opdragtargetcallback predragcallback;
		
		op2nodescallback dragcopycallback;
		
		opdrawcallback drawiconcallback; /*we call this routine to draw a line's icon*/
		
		opgetrectcallback geticonrectcallback; 
		
		opdrawcallback drawlinecallback; /*we call this routine to display a line*/
		
		opgetrectcallback gettextrectcallback;
		
		opgetrectcallback getedittextrectcallback;
		
		opnodebooleancallback getwpedittextcallback;
		
		opnodecallback setwpedittextcallback;
		
		opdrawcallback predrawlinecallback;
		
		opdrawcallback postdrawlinecallback; 
		
		opgetlineshortcallback getlineheightcallback;
		
		opgetlineshortcallback getlinewidthcallback;
		
		opmousecallback adjustcursorcallback;
		
		opnodecallback pushstylecallback;
		
		opnodecallback icon2clickcallback; /*user 2clicked on an icon*/
		
		opgetwindowhandlecallback getwindowhandlecallback; /*for drag/drop*/
		
		opsetwindowhandlecontextcallback setwindowhandlecontextcallback; /*for drag/drop*/
		
		opvalidatecallback validatepastecallback; /*verifies that it's ok to paste*/
		
		opnodecallback postpastecallback; /*does additional cleanup, if necessary*/
		
		opstringcallback validatecopycallback; /*verifies that it's ok to copy*/
		
		opnodecallback caneditcallback; /*returns true if the node's text can be edited*/
		
		opgefullrectcallback getfullrectcallback; /*determines the rectangle that the node occupies*/
		
		opnodecallback nodechangedcallback; /*something about the node changed, requiring a rebuild in CB, for example*/ 
		
	#endif
		
	opnodebooleancallback releaserefconcallback; /*releases storage held by refcon data*/
	
	long ctpushes;
	
	boolean fldirty: 1; /*any changes made that aren't saved?*/
	
	boolean fldirtyview: 1; /*did user change view: scroll, cursor, or window position?*/
	
	boolean fllocked: 1; /*are changes allowed to be made?*/
	
	boolean fltextmode: 1;
	
	boolean flactive: 1;
	
	boolean flbarcursoron: 1; /*internal flag, controls updating cursor line*/
	
	boolean flcursorneedsdisplay: 1; /*draw the cursor even if it hasn't moved*/
	
	boolean flusebitmaps: 1; /*are we allowed use of offscreen bitmaps?*/
	
	boolean flcheckvisi: 1; /*try to visi things on the next idle event*/
	
	boolean flwindowopen: 1; /*is it being displayed in a window?*/
	
	boolean flreopenpending: 1; /*true if outline has been closed, then re-opened*/
	
	boolean flbuildundo: 1; /*should we build an undo trail for changes in this outline?*/
	
	boolean flprinting: 1; /*some display routines may special-case when printing*/
	
	boolean flrecentlychanged: 1; /*for use by application -- scripts use it to keep track of code*/
	
	boolean flinhibitdisplay: 1; /*inhibit display update?*/
	
	boolean flreadonly: 1; /*if true, no editing changes are allowed*/
	
	boolean flvertscrolldisabled: 1, flhorizscrolldisabled: 1; /*some clay basket windows don't do horiz scrolling*/
	
	boolean flalwaysshowtextselection: 1; /*for clay basket find & replace, if true, text selection is shown even if window isn't active*/
	
	boolean blockvisiupdate: 1; /*a bit of baling wire, for expanding and visi'ing*/
	
	boolean blocksupersmartvisi: 1; /*some operations want to turn this feature off*/
	
	boolean flneedfulliconheight: 1; /*to support frontier and clay lineheight rules*/
	
	boolean fllimittextarrows: 1; /*for browsing, don't let text arrows move out of edit field*/
	
	boolean flstructuredtextkeys: 1; /*for browsing, typing in structure mode moves cursor*/
	
	boolean fldonterasebackground: 1; /*for clients that draw their own outline background*/
	
	boolean flinternalchange; /*set to inhibit structure callbacks, undo*/
	
	boolean flfatheadlines: 1; //does text of headlines wrap?
	
	boolean fldisposewhenpopped: 1; //true if an attempt was made to dispose outline while pushed

	boolean flhtml; /*7.0b28 PBS: true if outline is in WYSIWYG HTML mode.*/
	
	unsigned long timevisi; /*the time for the next scheduled visi check*/
	
	unsigned long maxlinelen; /*often 255, but sometimes less, and someday more*/
	
	unsigned long outlinescraptype;
	
	unsigned long outlinesignature; /*to identify client*/
	
	long outlinetype; /*for use by application. types in opinternal.h*/
	
	long outlinerefcon; /*for use by application*/
	} tyoutlinerecord;



#define getheadstring(h,bs)	texthandletostring ((**(h)).headstring, bs)

#define opisfatheadlines(ho) ((**(ho)).flfatheadlines)

/*globals*/

extern hdloutlinerecord outlinedata; /*global, points to "current" structure*/

#if !fljustpacking /*allow structure to be edited, not needed for packing*/

	extern WindowPtr outlinewindow; 
	
	extern hdlwindowinfo outlinewindowinfo; 

#endif


/*prototypes*/

extern boolean opistextmode (void); /*op.c*/

extern boolean ophaslinkedtext (hdlheadrecord);

extern void opsmashmouse (void);

extern boolean opeditingtext (hdlheadrecord);

extern boolean opsettextmode (boolean);

extern void optoggletextmode (void);

extern boolean opshowerror (long, short);

extern boolean opsetscrap (hdlheadrecord);

extern boolean opgetscrap (hdlheadrecord *, boolean *);

extern boolean opmotionkey (tydirection, long, boolean);

extern void opgetcursorinfo (long *, short *);

extern void opsetcursorinfo (long, short);

extern boolean opcloseoutline (void);

extern boolean opopenoutline (void);

extern void opvisibarcursor (void);

extern void opschedulevisi (void);

extern void opschedulevisi (void);


extern boolean opdisabledisplay (void); /*opdisplay.c*/

extern boolean openabledisplay (void);

extern boolean opdisplayenabled (void);

extern boolean opgetoutinesize (long *, long *);


extern void oppostedit (void); /*opedit.c*/ /*7.0b16 PBS: no longer static*/


extern void opfastcollapse (hdlheadrecord); /*opexpand.c*/

extern boolean opcollapse (hdlheadrecord);

extern void opcollapseall (void);

extern boolean opexpand (hdlheadrecord, short, boolean);

extern void opexpandto (hdlheadrecord);

extern void opexpandtoggle (void);


extern void ophoistdisplay (void); /*ophoist.c*/

extern boolean oppushhoist (hdlheadrecord);

extern boolean oppophoist (void);

extern boolean oppopallhoists (void);

extern void oprestorehoists (void);

extern void opoutermostsummit (hdlheadrecord *);


extern boolean opgetlangtext (hdloutlinerecord, boolean, Handle *); /*oplangtext.c*/


extern boolean oppushoutline (hdloutlinerecord); /*opops.c*/

extern boolean oppopoutline (void);

extern hdloutlinerecord opsetoutline (hdloutlinerecord);

extern boolean oppushglobals (void);

extern boolean oppopglobals (void);

extern boolean opsetsummit (hdloutlinerecord, hdlheadrecord);

extern void opdisposeoutline (hdloutlinerecord, boolean);

extern boolean opnosubheads (hdlheadrecord);

extern boolean ophassubheads (hdlheadrecord);

extern boolean opislastsubhead (hdlheadrecord);

extern long opcountheads (void);

extern boolean opsubheadsexpanded (hdlheadrecord);

extern hdlheadrecord opnthsubhead (hdlheadrecord, long);

extern boolean opnodeinoutline (hdlheadrecord);


extern boolean oppack (Handle *); /*oppack.c*/

//extern boolean opoutlinetotable (hdlheadrecord, Handle, long *);

extern boolean oppackoutline (hdloutlinerecord, Handle *);

extern boolean opunpack (Handle, long *, hdloutlinerecord *);

extern boolean opunpackoutline (Handle, hdloutlinerecord *);

extern boolean optextscraptooutline (hdloutlinerecord, Handle, hdlheadrecord *);

extern boolean opoutlinetotextstream (hdloutlinerecord, boolean, struct handlestream *);

extern boolean opoutlinetotextscrap (hdloutlinerecord, boolean, Handle);

extern boolean opoutlinetonewtextscrap (hdloutlinerecord, Handle *);


extern boolean opsetrefcon (hdlheadrecord, ptrvoid, long); /*oprefcon.c*/

extern boolean opgetrefcon (hdlheadrecord, ptrvoid, long);

extern void opemptyrefcon (hdlheadrecord);

extern boolean ophasrefcon (hdlheadrecord);

extern boolean opattributesgettypestring (hdlheadrecord hnode, bigstring bstype); /*7.0b8 PBS*/

boolean opattributesgetpackedtablevalue (hdlheadrecord hnode, tyvaluerecord *val); /*7.0b16 PBS*/

boolean opattributesgetoneattribute (hdlheadrecord hnode, bigstring bsattname, tyvaluerecord *val); /*7.0b16 PBS*/


extern boolean opaddheadline (hdlheadrecord, tydirection, bigstring, hdlheadrecord *); /*opstructure.c*/

extern boolean opdepositnewheadline (hdlheadrecord, tydirection, Handle, hdlheadrecord *);

extern boolean opmoveto (hdlheadrecord);

extern boolean opjumpto (hdlheadrecord);

extern boolean opgetsafenode (hdlheadrecord *hnode);

extern boolean opmovecursor (hdlheadrecord, tydirection, long, hdlheadrecord *);

extern boolean opflatfind (boolean, boolean);

extern boolean oppromote (void);

extern boolean opdemote (void);

extern boolean isoutlinetext (Handle);

extern boolean isoutlinescrap (void);

extern boolean opcopy (void);

extern boolean opdeletenode (hdlheadrecord);

extern boolean opdelete (void);

extern boolean opclear (void);

extern boolean opcut (void);

extern boolean oppaste (void);

extern boolean opinsertheadline (Handle, tydirection, boolean);

extern boolean opinsertstructure (hdlheadrecord, tydirection);


extern boolean opvalidate (hdloutlinerecord); /*opvalidate.c*/


extern hdlheadrecord opcopyoutline (hdlheadrecord);

extern boolean opinserthandle (Handle, tydirection); /*opverbs.c*/

extern boolean opsetexpansionstateverb (tyvaluerecord *vlist, tyvaluerecord *v);

extern boolean opsetscrollstateverb (long line1, tyvaluerecord *v);

extern boolean opgetexpansionstateverb (tyvaluerecord *v);

extern boolean opgetscrollstateverb (tyvaluerecord *v);



extern boolean oprecursivelyvisit (hdlheadrecord, short, opvisitcallback, ptrvoid); /*opvisit.c*/

extern boolean oplistvisit (hdlheadrecord, opvisitcallback, ptrvoid);

extern boolean opsummitvisit (opvisitcallback, ptrvoid);

extern boolean opparentvisit (hdlheadrecord, boolean, opvisitcallback, ptrvoid);

extern boolean oprecursivelyvisit (hdlheadrecord, short, opvisitcallback, ptrvoid);

extern boolean opvisiteverything (opvisitcallback, ptrvoid);

extern boolean oprecursivelyvisitkidsfirst (hdlheadrecord, short, opvisitcallback, ptrvoid);

extern boolean opsiblingvisiter (hdlheadrecord, boolean, opvisitcallback, ptrvoid);

extern boolean opvisitmarked (tydirection, opvisitcallback, ptrvoid);

extern boolean opbumpvisit (hdlheadrecord, tydirection, opvisitcallback, ptrvoid);


extern boolean opscroll (tydirection, boolean, long);

extern boolean opscrollto (long, long);

extern void opsetdisplaydefaults (hdloutlinerecord);

extern boolean opsetlongcursor (long cursor);

extern boolean opsetdisplayenabled (boolean);

extern boolean opsetcursorlinetext (bigstring);

extern boolean opnodechanged (hdlheadrecord);


extern boolean oprmousedown (Point pt, tyclickflags flags); /*oppopup.c*/

#endif



