
/*	$Id$    */

/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include <GestaltEqu.h>

#include <iac.h>
#include <iowacore.h>
#include "appletinternal.h"
#include "appletmsg.h"
#include "appletmain.h"
#include "appletmenu.h"
#include "appletscrollbar.h"
#include "appletmenuops.h"
#include "appletresource.h"
#include "appletidle.h"
#include "appletwires.h"
#include "appletfilemodified.h"
#include "appletsyserror.h"



#define fldoublestatusline false /*set true if you want a double line separating status bar from window content*/

#define saveasprompt "\pSave As…"

#define openprompt "\pOpen…"


/*
7/25/94 dmb: updated old-style function declarations
*/

/*
for communication with the applet body code -- it has copies of all the globals,
and pointers to the callback routines that define the applet.
*/
	tyappletrecord app;

/*
globals used by the window visiter.
*/
	ptrfilespec pfsvisit;
	AEDesc *prefconvisit;
	bigstring bsvisit;
	short x1, x2; /*two short registers for visit routines*/
	hdlappwindow lastwindowvisited;
	
	
/*
set true if the last event was a keystroke and it was cmd-period.
*/
	boolean flcmdperiod = false;
	

/*
for fast events, we need to store a pointer to our application heap, so any
allocations we do happen in the right heap.
*/
	static THz appletheapzone;


typedef struct tydiskwindowinfo { /*saved on disk after the applet's data handle*/
	
	short versionnumber;
	
	Rect windowrect;
	
	short vertmin, vertmax, vertcurrent;
	
	short horizmin, horizmax, horizcurrent;
	
	diskfontstring defaultfontstring;
	
	short defaultsize;
	
	tyjustification defaultjustification;
	
	char waste [64]; /*room to grow*/
	} tydiskwindowinfo;
	
EventRecord appletevent; /*the last event received by appletmain.c*/
	
boolean flexitmainloop = false;


#define nowindowerror 7 /*index into the verb error STR# list*/
#define windowopenerror 8
#define nopictcallbackerror 9
#define notextcallbackerror 10
#define noputtextcallbackerror 11
#define noputpictcallbackerror 12


static boolean flcurrentlyactive = true; /*we start active, then get juggled*/


#define jugglerEvt 15 /*a suspend/resume event from the OS to us*/

typedef struct tyjugglermessage {
	
	long eventtype: 8; /*bits 24 -- 31*/
	
	long reservedbits: 22; /*bits 2 -- 23*/
	
	long flconvertclipboard: 1; /*bit 1*/
	
	long flresume: 1; /*bit 0*/
	} tyjugglermessage;


enum { /*key codes*/
	
	clearkey = 0x47,
	
	helpkey = 0x72,
	
	homekey = 0x73,
	
	endkey = 0x77,
	
	pgupkey = 0x74,
	
	pgdnkey = 0x79
	};




static boolean filterfunctionkey (long *codeword) {
	
	short item;
	short menu = editmenu;
	
	switch (keyboardstatus.keycode) {
		
		case 'z':
			item = undoitem;
			
			break;
		
		case 'x':
			item = cutitem;
			
			break;
		
		case 'c':
			item = copyitem;
			
			break;
		
		case 'v':
			item = pasteitem;
			
			break;
		
		case clearkey:
			item = clearitem;
			
			break;
		
		case helpkey:
			item = helpitem;
			
			menu = extendedmenu;
			
			break;
			
		case homekey:
			item = homeitem;
			
			menu = extendedmenu;
			
			break;
			
		case endkey:
			item = enditem;
			
			menu = extendedmenu;
			
			break;
			
		case pgupkey:
			item = pageupitem;
			
			menu = extendedmenu;
			
			break;
			
		case pgdnkey:
			item = pagedownitem;
			
			menu = extendedmenu;
			
			break;
		
		default:
			return (false);
		} /*switch*/
	
	*codeword = ((long) menu << 16) + item;
	
	return (true);
	} /*filterfunctionkey*/


void apperaserect (Rect r) {
	
	appprecallback ();
	
	pushclip (r);
	
	(*app.eraserectcallback) (r);
	
	popclip ();
	
	apppostcallback ();
	} /*apperaserect*/
	

void setselectionstyleinfo (tyselectioninfo *pselinfo, boolean flpack) {
	
	union {
		
		short fontstyle;
		
		struct {
			
			int extrabits: 7; /*highest-order 7 bits are unused*/
			
			boolean flsubscript: 1;
			
			boolean flsuperscript: 1;
			
			boolean flextended: 1; /*not currently used*/
			
			boolean flcondensed: 1;
			
			boolean flshadow: 1;
			
			boolean floutline: 1;
			
			boolean flunderline: 1;
			
			boolean flitalic: 1;
			
			boolean flbold: 1;
			} stylebits;
		} u;
	
	tyselectioninfo x = *pselinfo;
	
	if (flpack) {
		
		u.fontstyle = 0; /*clear all bits*/
		
		u.stylebits.flsubscript = x.flsubscript;
		
		u.stylebits.flsuperscript = x.flsuperscript;
		
		u.stylebits.flextended = x.flextended;
		
		u.stylebits.flcondensed = x.flcondensed;
		
		u.stylebits.flshadow = x.flshadow;
		
		u.stylebits.floutline = x.floutline;
		
		u.stylebits.flunderline = x.flunderline;
		
		u.stylebits.flitalic = x.flitalic;
		
		u.stylebits.flbold = x.flbold;
		
		x.fontstyle = u.fontstyle;
		}
	else {
		u.fontstyle = x.fontstyle;
		
		x.flsubscript = bitboolean (u.stylebits.flsubscript);
		
		x.flsuperscript = bitboolean (u.stylebits.flsuperscript);
		
		x.flextended = bitboolean (u.stylebits.flextended);
		
		x.flcondensed = bitboolean (u.stylebits.flcondensed);
		
		x.flshadow = bitboolean (u.stylebits.flshadow);
		
		x.floutline = bitboolean (u.stylebits.floutline);
		
		x.flunderline = bitboolean (u.stylebits.flunderline);
		
		x.flitalic = bitboolean (u.stylebits.flitalic);
		
		x.flbold = bitboolean (u.stylebits.flbold);		
		}
	
	*pselinfo = x;
	} /*setselectionstyleinfo*/
	
		
static void getgrowiconrect (hdlappwindow appwindow, Rect *rgrowicon) {
	
	WindowPtr w = (**appwindow).macwindow;
	Rect r;
	
	r = (*w).portRect;
	
	r.left = r.right - 15;
	
	r.top = r.bottom - 15;
	
	*rgrowicon = r;
	} /*getgrowiconrect*/
	

void drawappgrowicon (hdlappwindow appwindow) {
	
	/*
	DW 6/24/93: made it so that it always just draws the grow
	icon, it never draws the lines for the scroll bars. this
	overwrote the status bar in clay basket. the scroll bars
	take care of updating these lines anyway.
	*/
	
	WindowPtr w = (**appwindow).macwindow;
	Rect rclip;
		
	if ((**appwindow).flprinting)
		return;
		
	getgrowiconrect (appwindow, &rclip);
	
	pushclip (rclip);
	
	DrawGrowIcon (w);
	
	popclip ();
	} /*drawappgrowicon*/
	

boolean apppushwindow (hdlappwindow appwindow) {
	
	return (pushmacport ((**appwindow).macwindow));
	} /*apppushwindow*/
	

boolean apppopwindow (void) {
	
	return (popmacport ());
	} /*apppopwindow*/
	
	
boolean apppushclip (Rect r) {

	/*
	this front-end for pushclip respects the current settings of the origin. applets
	should call this routine instead of calling the quickdraw.c routine.
	*/
	
	hdlappwindow ha = app.appwindow;
	
	if (!pushclip (r))
		return (false);
	
	if ((**ha).originpushdepth > 0) 
		OffsetRgn ((*(**ha).macwindow).clipRgn, -(**ha).scrollorigin.h, -(**ha).scrollorigin.v);
	
	return (true);
	} /*apppushclip*/
	
	
boolean apppopclip (void) {

	/*
	this front-end for popclip respects the current settings of the origin. applets
	should call this routine instead of calling the quickdraw.c routine.
	*/
	
	hdlappwindow ha = app.appwindow;
	
	if (!popclip ())
		return (false);
	
	if ((**ha).originpushdepth > 0) 
		OffsetRgn ((*(**ha).macwindow).clipRgn, -(**ha).scrollorigin.h, -(**ha).scrollorigin.v);

	return (true);
	} /*apppopclip*/
	

void appprecallback (void) {
	
	hdlappwindow ha = app.appwindow;
	
	if (ha != nil) {
		
		WindowPtr w = (**ha).macwindow;
		
		pushmacport (w);
		
		pushclip ((**ha).contentrect);
			
		(**ha).originpushdepth++;
		
		if ((**ha).originpushdepth == 1) { 
			
			SetOrigin ((**ha).scrollorigin.h, (**ha).scrollorigin.v);
		
			OffsetRgn ((*w).clipRgn, (**ha).scrollorigin.h, (**ha).scrollorigin.v);
			}
		}
	
	(*app.setglobalscallback) (); 
	} /*appprecallback*/
	
	
void apppostcallback (void) {
	
	hdlappwindow ha = app.appwindow;
	
	if (app.dontpostcallback) { /*applet already did the post-callback call*/
		
		app.dontpostcallback = false; /*must be reset every time*/
		
		return;
		}
	
	if (ha != nil) {
		
		(**ha).originpushdepth--;
	
		if ((**ha).originpushdepth == 0) { 
			
			WindowPtr w = (**ha).macwindow;
			
			pushmacport (w);
			
			OffsetRgn ((*w).clipRgn, -(**ha).scrollorigin.h, -(**ha).scrollorigin.v);
		
			SetOrigin (0, 0);
			
			popmacport ();
			}
		
		popclip ();
		
		popmacport ();
		}
	} /*apppostcallback*/
	
	
boolean appopenbitmap (Rect r, hdlappwindow appwindow) {
	
	return (openbitmap (r, (**appwindow).macwindow));
	} /*appopenbitmap*/
	

boolean appclosebitmap (hdlappwindow appwindow) {
	
	closebitmap ((**appwindow).macwindow);
	
	return (true);
	} /*appclosebitmap*/
	

boolean getappwindow (WindowPtr w, hdlappwindow *appwindow) {
	
	hdlappwindow ha;
	
	if (w == nil) /*it can happen: when using FrontWindow, and no window is open*/
		return (false);
	
	#ifdef appRunsCards
	
		if (isCardWindow (w))
			return (false);
	
	#endif
	
	ha = (hdlappwindow) GetWRefCon (w);
	
	*appwindow = ha;
	
	return (ha != nil);
	} /*getappwindow*/
	
	
boolean findappwindow (Point pt, hdlappwindow *appwindow) {
	
	short part;
	WindowPtr w;
	
	*appwindow = nil; /*default*/
		
	part = FindWindow (pt, &w);
	
	if (w == nil) /*not pointing at any window*/
		return (false);
		
	return (getappwindow (w, appwindow));
	} /*findappwindow*/


void getappdata (WindowPtr w, Handle *appdata) {
	
	hdlappwindow appwindow;
	
	if (getappwindow (w, &appwindow))
		*appdata = (**appwindow).appdata;
	else
		*appdata = nil;
	} /*getappdata*/
	

void getappwindowtitle (hdlappwindow appwindow, bigstring bs) {
	
	if (appwindow == nil)
		setstringlength (bs, 0);
	else
		GetWTitle ((**appwindow).macwindow, bs);
	} /*getappwindowtitle*/
	
	
void setappwindowtitle (hdlappwindow appwindow, bigstring bs) {
	
	SetWTitle ((**appwindow).macwindow, bs);
	} /*setappwindowtitle*/
	
	
void hideappwindow (hdlappwindow appwindow) {
	
	HideWindow ((**appwindow).macwindow);
	} /*hideappwindow*/
	

void setappwindowmadechanges (hdlappwindow appwindow) {
	
	assert (appwindow != nil);
	
	(**appwindow).flmadechanges = true;
	} /*setappwindowmadechanges*/
	
	
boolean frontappwindow (hdlappwindow appwindow) {
	
	assert (appwindow != nil);
	
	return ((**appwindow).macwindow == FrontWindow ());
	} /*frontappwindow*/
	
	
static void getappfilename (hdlappwindow appwindow, bigstring bs) {
	
	filegetfilename (&(**appwindow).filespec, bs);
	} /*getappfilename*/
	

void getappwindowfilespec (hdlappwindow appwindow, ptrfilespec pfs) {

	hdlappwindow ha = appwindow;
	
	if ((**appwindow).fnum == 0) 
		clearbytes (pfs, longsizeof (tyfilespec)); /*Iowa 1.0b15*/
	else
		*pfs = (**ha).filespec;
	} /*getappwindowfilespec*/
	
		
void showappwindow (hdlappwindow appwindow) {
	
	ShowWindow ((**appwindow).macwindow);
	} /*showappwindow*/
	
	
void selectappwindow (hdlappwindow appwindow) {

	SelectWindow ((**appwindow).macwindow);
	} /*selectappwindow*/
	
	
void invalappwindow (hdlappwindow appwindow, boolean flerase) {
	
	Rect r;
	
	pushmacport ((**appwindow).macwindow); 
	
	r = (**appwindow).windowrect;
	
	invalrect (r);
	
	if (flerase) 
		apperaserect (r);
		
	popmacport ();
	} /*invalappwindow*/
	
	
void setappwindow (hdlappwindow appwindow) {
	
	app.appwindow = appwindow;
		
	if (appwindow == nil) {
		
		app.appdata = nil;
		
		undostack = nil;
		
		redostack = nil;
		}
	else {
		SetPort ((**appwindow).macwindow);
		
		ClipRect (&(**appwindow).contentrect);
		
		undostack = (hdlundostack) (**appwindow).undostack;
		
		redostack = (hdlundostack) (**appwindow).redostack;
		
		app.appdata = (**appwindow).appdata;
		
		(*app.setglobalscallback) (); /*app can copy from app.appdata to its own globals*/
		}
	} /*setappwindow*/
	
	
boolean apprectneedsupdate (Rect r) {
	
	WindowPtr w = (**app.appwindow).macwindow;
	
	return (RectInRgn (&r, (*w).visRgn));
	} /*apprectneedsupdate*/
	
	
void appinvalstatusseparator (void) {
	
	if (fldoublestatusline) {
	
		Rect r = (**app.appwindow).statusrect;
	
		r.top = r.bottom;
	
		r.bottom += 2;
	
		InvalRect (&r);
		}
	} /*appinvalstatusseparator*/
	
	
void updateappwindow (hdlappwindow appwindow) {
	
	/*
	DW 4/24/92: reordered things for IOWA.
	
	DW 4/25/92: added updating of palette.
	
	DW 4/30/92: added pre-update callback to allow IOWA to inval all dirty objects.
	
	DW 3/18/93: reordered things again to make IOWA look nice. just staging.
	*/
	
	hdlappwindow ha = appwindow;
	WindowPtr w = (**ha).macwindow;
	Rect rwindow = (*w).portRect;
	Rect rstatus = (**ha).statusrect;
	
	BeginUpdate (w);
	
	setappwindow (ha);
	
	/*update the status area*/ {
		
		if ((**ha).statuspixels > 0) { /*has a status area*/
			
			Rect r = rstatus;
				
			if (apprectneedsupdate (r)) {
			
				pushclip (r);
				
				(*app.updatestatuscallback) ();
				
				MoveTo (r.left, r.bottom - 1);
				
				LineTo (r.right, r.bottom - 1);
			
				popclip ();
				}
			
			if (fldoublestatusline) {
			
				r.top = r.bottom;
				
				r.bottom += 4;
				
				if (apprectneedsupdate (r)) { /*draw the double separator line*/
					
					r.bottom -= 2;
					
					pushclip (rwindow);
					
					apperaserect (r);
					
					MoveTo (r.left, r.bottom);
			
					LineTo (r.right, r.bottom);
					
					popclip ();
					}
				}
			}
		}
		
	/*update the palette*/ {
		
		if (app.haspalette) {
		
			if (apprectneedsupdate ((**(**ha).hpalette).r)) {
			
				pushclip (rwindow);
			
				paletteupdate ((**ha).hpalette);
		
				popclip ();
				}
			}
		}
		
	/*update the grow icon, scrollbars*/ {
		
		Rect r;
		
		pushclip (rwindow);
		
		getgrowiconrect (ha, &r);
		
		if (apprectneedsupdate (r)) 
			drawappgrowicon (ha);
			
		appdrawmessage (ha);

		updateappscrollbars (ha);
		
		popclip ();
		}
		
	/*let the applet update his stuff*/ {
		
		Rect rcontent = (**ha).contentrect;
		
		if (apprectneedsupdate (rcontent)) {
			
			appprecallback ();
			
			(**ha).updaterect = (**(*w).visRgn).rgnBBox;
			
			(*app.setglobalscallback) (); /*app can copy from app.appdata to its own globals*/
			
			(*app.updatecallback) ();
			
			apppostcallback ();
			}
		}
		
	/*for BarChart and Card Editor, no scroll bars, may need redraw of grow icon*/ {
		
		Rect r;
		
		getgrowiconrect (ha, &r);
		
		if (apprectneedsupdate (r)) 
			drawappgrowicon (ha);
		}
		
	EndUpdate (w);
	} /*updateappwindow*/
	
	
void computewindowinfo (WindowPtr macwindow, hdlappwindow appwindow) {
	
	WindowPtr w = macwindow;
	hdlappwindow ha = appwindow;
	hdlpaletterecord hpal = (hdlpaletterecord) (**appwindow).hpalette;
	Rect r, rcontent, rstatus, rpalette;
	
	(**ha).windowrect = r = (*w).portRect;
	
	rcontent = r;
	
	/*take out for scrollbars*/ {
		
		short scrollbarwidth = getscrollbarwidth ();
		
		if (app.vertscroll)
			rcontent.right -= scrollbarwidth - 1;
		
		if (app.horizscroll)
			rcontent.bottom -= scrollbarwidth - 1;
		}
	
	if ((**ha).statuspixels > 0) { /*has a status area*/
	
		rcontent.top += (**ha).statuspixels;
		
		if (fldoublestatusline)
			rcontent.top += 3; /*a 3-pixel tall area separates status & content*/
		}
		
	if (app.haspalette) { /*has an icon palette*/
		
		rpalette = rcontent;
		
		rcontent.left += (**hpal).palettewidth;
		
		rpalette.right = rcontent.left;
		}
	else
		zerorect (&rpalette);	
	
	(**ha).oldcontentrect = (**ha).contentrect;
		
	(**ha).contentrect = rcontent;
	
	if (app.haspalette) 
		(**(**ha).hpalette).r = rpalette;
	
	rstatus = r; /*copy left, right, top*/
	
	rstatus.bottom = rstatus.top + (**ha).statuspixels;
	
	(**ha).statusrect = rstatus;
	
	(**ha).windowvertpixels = r.bottom - r.top;
	
	(**ha).windowhorizpixels = r.right - r.left;
	
	pushmacport (w);
	
	pushstyle ((**ha).defaultfont, (**ha).defaultsize, (**ha).defaultstyle);
	
	(**ha).fontheight = globalfontinfo.ascent + globalfontinfo.descent;
	
	popstyle ();
	
	popmacport ();
	
	(**ha).macwindow = w;
	} /*computewindowinfo*/
	

void appgetdesktoprect (hdlappwindow appwindow, Rect *rwindow) {
	
	WindowPtr w = (**appwindow).macwindow;
	
	*rwindow = (*w).portRect;
	
	pushmacport (w);
			
	localtoglobalrect (rwindow);
			
	popmacport ();
	} /*appgetdesktoprect*/
	
	
void disposeappwindow (hdlappwindow appwindow) {
	
	hdlappwindow ha = app.appwindow;
	WindowPtr w;
	
	if (ha == nil) /*4.1b1 dmb: allow for nil*/
		return;
	
	w = (**ha).macwindow;
	
	appprecallback ();
	
	(*app.disposerecordcallback) ();
	
	apppostcallback ();
	
	if (app.haspalette) 
		disposepalette ((hdlpaletterecord) (**ha).hpalette);
	
	disposescrollbar ((**ha).vertbar);
	
	disposescrollbar ((**ha).horizbar);
	
	disposeundostack ((hdlundostack) (**ha).undostack);
	
	disposeundostack ((hdlundostack) (**ha).redostack);
	
	DisposeWindow (w);
	
	AEDisposeDesc (&(**ha).iacdatarefcon); /*4.1b1 dmb*/
	
	disposehandle ((Handle) ha);
	
	setfrontglobals ();
	} /*disposeappwindow*/
	

typedef boolean (*tyvisitapproutine) (hdlappwindow);


boolean visitappwindows (tyvisitapproutine visitproc) {
	
	/*
	visit all the windows from front to back.  call the visitproc for each window, 
	if it returns false we stop the traversal and return false.
	
	the visitproc takes one parameter -- a hdlappwindow, indicating the window
	to be visited.
	
	return true if all the visits returned true.
	
	9/4/91 DW: add lastwindowvisited -- returns nil if no match found, returns
	pointing to the appwindow record for the window that matched the visitproc's
	criteria.
	*/
	
	WindowPtr w = FrontWindow (), wnext;
	hdlappwindow appwindow;
	
	lastwindowvisited = nil;
	
	while (w != nil) {
		
		wnext = (WindowPtr) (*(WindowPeek) w).nextWindow; /*allow the visit callback to close the window*/
		
		if (!getappwindow (w, &appwindow))
			goto next;
		
		if (!(*visitproc) (appwindow)) { /*stop the traversal*/
		
			lastwindowvisited = appwindow; /*set global*/
			
			return (false);
			}
		
		next:
		
		w = wnext; 
		} /*while*/
		
	return (true); /*completed the traversal*/
	} /*visitappwindows*/
	

boolean selectvisit (hdlappwindow appwindow) {
	
	bigstring bs;
	
	getappwindowtitle (appwindow, bs);
	
	if (!equalstrings (bsvisit, bs)) /*no match -- keep traversing*/
		return (true);
	
	/*found a match, select the window, stop the traversal*/
	
	selectappwindow (appwindow);
		
	return (false); /*stop the traversal*/
	} /*selectvisit*/
	
	
boolean selectwindowbytitle (bigstring bs) {

	/*
	visit all the windows from front to back.  when we encounter one of the 
	app's windows check the window title.  if it equals bs, then select the 
	window and set the globals accordingly.
	
	return false if there is no app window with that name.
	*/
	
	copystring (bs, bsvisit); /*copy into global so visit routine can access*/
	
	return (!visitappwindows (&selectvisit));
	} /*selectwindowbytitle*/	
	
	
boolean findbytitlevisit (hdlappwindow appwindow) {
	
	bigstring bs;
	
	getappwindowtitle (appwindow, bs);
	
	if (!equalstrings (bsvisit, bs)) /*no match -- keep traversing*/
		return (true);
	
	return (false); /*stop the traversal*/
	} /*findbytitlevisit*/
	
	
boolean findbywindowtitle (bigstring bstitle, hdlappwindow *appwindow) {

	copystring (bstitle, bsvisit); /*copy into global so visit routine can access*/
	
	visitappwindows (&findbytitlevisit);
	
	if (lastwindowvisited == nil)
		return (false);
		
	*appwindow = lastwindowvisited;
	
	return (true);
	} /*findbywindowtitle*/
	
	
boolean findnthvisit (hdlappwindow appwindow) {
	
	return (++x2 < x1); /*visit until they're equal*/
	} /*findnthvisit*/
	
	
boolean findnthwindow (short n, hdlappwindow *appwindow) {
	
	x1 = n; /*copy into a "register" for visit routine*/
	
	x2 = 0; /*another register*/
	
	visitappwindows (&findnthvisit);
	
	*appwindow = lastwindowvisited;
	
	return (lastwindowvisited != nil);
	} /*findnthwindow*/
	
	
boolean countwindowsvisit (hdlappwindow appwindow) {
	
	x1++;
	
	return (true); /*keep going*/
	} /*countwindowsvisit*/
	
	
short countwindows (void) {
	
	x1 = 0; /*copy into a "register" for visit routine*/
	
	visitappwindows (&countwindowsvisit);
	
	return (x1);
	} /*countwindows*/
	
	
boolean resetdirtyscrollbarsvisit (hdlappwindow appwindow) {
	
	register hdlappwindow ha = appwindow;
	
	setappwindow (ha);
	
	if ((**ha).flresetscrollbars) {
		
		resetappscrollbars (ha);
		
		(**ha).flresetscrollbars = false; /*consume it*/
		}
	
	return (true); /*visit all open windows*/
	} /*resetdirtyscrollbarsvisit*/
	
	
boolean resetdirtyscrollbars (void) {

	visitappwindows (&resetdirtyscrollbarsvisit);
	
	return (true);
	} /*resetdirtyscrollbars*/
	
	
boolean getuntitledtitle (bigstring bs) {
	
	long ct = 1;
	hdlappwindow appwindow;
	
	while (true) {
		
		copystring ("\pUntitled ", bs);
		
		pushlong (ct++, bs);
		
		if (!findbywindowtitle (bs, &appwindow))
			return (true);
		} /*while*/
	} /*getuntitledtitle*/
	

static boolean findbyfilevisit (hdlappwindow appwindow) {
	
	return (!equalfilespecs (&(**appwindow).filespec, pfsvisit));
	} /*findbyfilevisit*/
	
	
boolean findbyfile (ptrfilespec pfs, hdlappwindow *appwindow) {
	
	pfsvisit = pfs;
	
	visitappwindows (&findbyfilevisit);
	
	if (lastwindowvisited == nil)
		return (false);
		
	*appwindow = lastwindowvisited;
	
	return (true);
	} /*findbyfile*/


static boolean findbyrefconvisit (hdlappwindow appwindow) {
	
	if ((**appwindow).iacdatarefcon.descriptorType != (*prefconvisit).descriptorType)
		return (true);
	
	return (!equalhandles ((**appwindow).iacdatarefcon.dataHandle, (*prefconvisit).dataHandle));
	} /*findbyrefconvisit*/


static boolean findbyrefcon (AEDesc *refon, hdlappwindow *appwindow) {
	
	prefconvisit = refon;
	
	visitappwindows (&findbyrefconvisit);
	
	if (lastwindowvisited == nil)
		return (false);
	
	*appwindow = lastwindowvisited;
	
	return (true);
	} /*findbyrefcon*/


boolean setfrontglobalsvisit (hdlappwindow appwindow) {
	
	setappwindow (appwindow);
	
	return (false); /*stop the traversal*/
	} /*setfrontglobalsvisit*/
	
	
boolean setfrontglobals (void) {
	
	setappwindow (nil);
	
	visitappwindows (&setfrontglobalsvisit);
	
	return (app.appwindow != nil);
	} /*setfrontglobals*/


void moveappwindow (hdlappwindow appwindow, Rect r) {
	
	WindowPtr w = (**appwindow).macwindow;
		
	constraintodesktop (&r);
	
	SizeWindow (w, r.right - r.left, r.bottom - r.top, true);
	
	MoveWindow (w, r.left, r.top, false);
	
	computewindowinfo (w, appwindow);
	
	resizeappscrollbars (appwindow);
	
	resetappscrollbars (appwindow);
	} /*moveappwindow*/
	

static void saveappwindowinfo (hdlappwindow appwindow, tydiskwindowinfo *appwindowinfo) {
	
	hdlappwindow ha = appwindow;
	tydiskwindowinfo x;
	
	clearbytes (&x, longsizeof (x));
	
	x.versionnumber = 1;
	
	appgetdesktoprect (ha, &x.windowrect);
	
	getscrollbarinfo ((**ha).vertbar, &x.vertmin, &x.vertmax, &x.vertcurrent);
	
	getscrollbarinfo ((**ha).horizbar, &x.horizmin, &x.horizmax, &x.horizcurrent);
	
	x.defaultsize = (**ha).defaultsize;
	
	x.defaultjustification = (**ha).defaultjustification;
	
	diskgetfontname ((**ha).defaultfont, x.defaultfontstring);
	
	*appwindowinfo = x;
	} /*saveappwindowinfo*/
	

static void loadappwindowinfo (hdlappwindow appwindow, tydiskwindowinfo appwindowinfo) {
	
	hdlappwindow ha = appwindow;
	tydiskwindowinfo x = appwindowinfo;
	short fontnum;
	
	moveappwindow (ha, x.windowrect);
	
	setscrollbarinfo ((**ha).vertbar, x.vertmin, x.vertmax, x.vertcurrent);
	
	setscrollbarinfo ((**ha).horizbar, x.horizmin, x.horizmax, x.horizcurrent);
	
	/*
	(**ha).scrollorigin.v = x.vertcurrent;
	
	(**ha).scrollorigin.h = x.horizcurrent;
	*/
	
	(**ha).defaultsize = x.defaultsize;
	
	(**ha).defaultjustification = x.defaultjustification;
	
	diskgetfontnum (x.defaultfontstring, &fontnum);
	
	(**ha).defaultfont = fontnum;
	} /*loadappwindowinfo*/
	

boolean saveappwindow (ptrfilespec pfs) {
	
	hdlappwindow ha = app.appwindow;
	Handle h = nil;
	short rnum;
	boolean fl;
	
	watchcursor ();
	
	setstringlength (bsalertstring, 0);
	
	if ((**ha).fnum == 0) { /*file isn't open*/
	
		short fnum;
		
		if (!filenew (pfs, (**ha).creator, (**ha).filetype, &fnum)) {
			
			alertdialog ("\pError saving the front window.");
			
			return (false);
			}
		
		(**ha).fnum = fnum;
		}
	else { /*file is already open*/
		
		if (!fileseteof ((**ha).fnum, (long) 0)) {
			
			alertdialog ("\pError truncating the file.");
			
			return (false);
			}
		}
		
	appprecallback ();
	
	fl = (*app.packcallback) (&h); /*From Mayson Lancaster -- move this up in the routine, DW 12/9/93*/
	
	apppostcallback ();
	
	if (!fl) {
		
		alertdialog ("\pOut of memory.");
		
		goto error;
		}
			
	if (!filewritehandle ((**ha).fnum, h)) {
		
		alertdialog ("\pError writing to file.");
		
		goto error;
		}
	
	if (openresourcefile (pfs, &rnum)) {
	
		tydiskwindowinfo x; 
		Handle hoptions = nil;
		
		saveappwindowinfo (ha, &x);
		
		putresource (rnum, 'aptk', 128, longsizeof (x), &x);
		
		(*app.getoptionscallback) (&hoptions);
		
		if (hoptions != nil) {
		
			putresourcehandle (rnum, 'aptk', 129, hoptions);
		
			disposehandle (hoptions);
			}
		
		closeresourcefile (rnum);
		}
	
	disposehandle (h);
	
	(**ha).flmadechanges = false;
	
	if ((**ha).touchparentsonsave)
		filetouchallparentfolders (pfs);
	
	FlushVol (nil, (*pfs).vRefNum); /*12/12/93 DW*/
	
	appprecallback ();
	
	(*app.aftersavewindowcallback) ();
	
	apppostcallback ();
	
	return (true);
		
	error:
	
	disposehandle (h);
	
	return (false);
	} /*saveappwindow*/
	

static void checkzoombeforeopen (hdlappwindow appwindow) {

	if ((**appwindow).flzoombeforeopen) {
		
		zoomappwindow (appwindow);
		
		(**appwindow).flzoombeforeopen = false;
		}
	} /*checkzoombeforeopen*/
	
	
static void appinitfontdefaults (hdlappwindow ha) {

	(**ha).defaultfont = app.defaultfont;
	
	(**ha).defaultsize = app.defaultsize;
	
	(**ha).defaultstyle = app.defaultstyle;
	
	(**ha).defaultjustification = leftjustified;
	} /*appinitfontdefaults*/
	

boolean newappwindow (bigstring bstitle, boolean flshowwindow) {
	
	hdlappwindow ha = app.appwindow;
	WindowPtr macwindow;
	boolean flrelative = false;
	short hwindow, vwindow;
	short widthwindow, heightwindow;
	Rect rwindow;
	bigstring bslastword;
	hdlscrollbar vertbar = nil, horizbar = nil;
	hdlundostack undostack = nil, redostack = nil;
	boolean fl;
	
	if (ha != nil) { /*seed window position from frontmost window*/
		
		appgetdesktoprect (ha, &rwindow);
		
		hwindow = rwindow.left + 17;
		
		vwindow = rwindow.top + 17;
		
		widthwindow = rwindow.right - rwindow.left;
				
		heightwindow = rwindow.bottom - rwindow.top;
		
		flrelative = true;
		}
		
	if (!newclearhandle (longsizeof (tyappwindow), (Handle *) &app.appwindow))
		goto error;
		
	ha = app.appwindow; /*copy into register*/	
	
	(**ha).filetype = app.filetype; /*default*/

	(**ha).creator = app.creator; /*default*/
	
	(**ha).notsaveable = app.notsaveable; /*default*/
	
	(**ha).systemidleticks = app.systemidleticks; /*default*/
	
	(**ha).statuspixels = app.statuspixels; /*default*/
	
	(**ha).iacdatarefcon.descriptorType = typeNull; /*4.1b1 dmb*/
	
	if (app.usecolor && systemhascolor ()) {
	
		macwindow = GetNewCWindow (128, nil, (WindowPtr) -1);
		
		if (app.exactcolors)
			getcolorpalette (macwindow);
		
		(**ha).flcolorwindow = true;
		
		(**ha).forecolor = blackcolor;
		
		(**ha).backcolor = whitecolor;
		}
	else {
		macwindow = GetNewWindow (128, nil, (WindowPtr) -1);
		
		(**ha).flcolorwindow = false;
		}
		
	if (flrelative)
		SizeWindow (macwindow, widthwindow, heightwindow, false);

	if (macwindow == nil)
		goto error;
	
	if (app.vertscroll) {
	
		if (!newscrollbar (macwindow, true, &vertbar))
			goto error;
		}
		
	if (app.horizscroll) {
	
		if (!newscrollbar (macwindow, false, &horizbar))
			goto error;
		}
	
	(**ha).vertbar = vertbar;
	
	(**ha).horizbar = horizbar;
	
	if (!newundostack (&undostack))
		goto error;
	
	if (!newundostack (&redostack))
		goto error;
	
	(**ha).undostack = (Handle) undostack;
	
	(**ha).redostack = (Handle) redostack;
	
	if (flrelative) /*position window relative to previous window*/
		MoveWindow (macwindow, hwindow, vwindow, false);
	else
		centerwindow (macwindow, quickdrawglobal (screenBits).bounds);
	
	/*copystring (bstitle, (**ha).fname);*/ /*DW 6/14/92*/
	
	if (app.haspalette) {
		
		hdlpaletterecord hpal;
		
		hpal = newpalette (macwindow, app.ctpaletteicons);
		
		if (hpal == nil)
			goto error;
			
		(**ha).hpalette = hpal;
		}
	
	(**ha).macwindow = macwindow;
	
	computewindowinfo (macwindow, ha);
	
	appinitfontdefaults (ha);
	
	SetWRefCon (macwindow, (long) ha);
	
	pushmacport (macwindow);
	
	fl = (*app.newrecordcallback) (); /*this callback is allowed to change window size and pos*/
		
	popmacport ();
	
	if (!fl)
		goto error;
		
	computewindowinfo (macwindow, ha); /*window might have been changed by callback*/
	
	(**ha).appdata = app.appdata; /*copy handle alloc'd by newrecord*/
	
	lastword (bstitle, ':', bslastword); /*avoid displaying long paths*/
	
	setappwindowtitle (ha, bslastword);
	
	(**ha).selectioninfo.fldirty = true;
	
	resizeappscrollbars (ha);
	
	resetappscrollbars (ha);
	
	if (flshowwindow) {
		
		checkzoombeforeopen (ha);
		
		showappscrollbars (ha); /*4.1b1 dmb: do before showing window*/
		
		showappwindow (ha);
		}
			
	return (true);
	
	error:
	
	if (app.haspalette) 
		disposepalette ((hdlpaletterecord) (**ha).hpalette);
	
	disposeundostack (undostack);
	
	disposeundostack (redostack);
	
	disposescrollbar (vertbar);
	
	disposescrollbar (horizbar);
	
	macwindow = (**ha).macwindow;
	
	if (macwindow != nil)
		DisposeWindow (macwindow);
		
	disposehandle ((Handle) ha);
	
	setappwindow (nil);
	
	return (false);
	} /*newappwindow*/
	
	
static void prepareandshowwindow (hdlappwindow ha) {
	
	/*
	4.1b1 dmb: new routine shared by openappwindow and editappwindow
	*/
	
	resetappscrollbars (ha);
	
	showappscrollbars (ha);
	
	/*invalappwindow (ha, false); /*dmb 4.1b1 -- commented out*/
	
	appprecallback ();
	
	(*app.afteropenwindowcallback) ();
	
	apppostcallback ();
	
	checkzoombeforeopen (ha);
	
	showappwindow (ha);
	} /*prepareandshowwindow*/


boolean newuntitledappwindow (boolean flshowwindow) {
		
	bigstring bstitle;
	Boolean	result;
	
	getuntitledtitle (bstitle);
	
	result = newappwindow (bstitle, flshowwindow);
	
	// LDR: added 9/18/95; we need to make this callback to get the dragMgr stuff in place
	if (result) {
	
		appprecallback ();
		
		(*app.afteropenwindowcallback) ();
		
		apppostcallback ();
	}
	
	return (result);
	} /*newuntitledappwindow*/
	

boolean openappwindow (ptrfilespec pfs) {
	
	OSType origfiletype = app.filetype, origfilecreator = app.creator;
	Handle hoptions = nil;
	hdlappwindow appwindow;
	hdlappwindow ha = nil;
	short fnum;
	Handle h;
	short rnum;
	boolean fl;
	bigstring fname;
	
	setstringlength (bsalertstring, 0);
	
	if ((*app.interpretfilecallback) (pfs)) /*the file has been processed, don't open a window*/
		return (true);
	
	if (findbyfile (pfs, &appwindow)) { /*the file is already open*/
		
		selectappwindow (appwindow);
		
		/*appsetmessage (appwindow, "\pThe file is already open.");*/

		return (true);
		}
	
	if (!fileopen (pfs, &fnum)) {
		
		alertdialog ("\pError opening the file.");
		
		return (false);
		}
		
	if (!filereadwholefile (fnum, &h)) {
		
		alertdialog ("\pError reading from file.");
		
		goto error;
		}
	
	/*the data fork of the file stays open while the user is editing it*/
	
	/*set up app.filetype and app.creator so that newrecord routine has these available*/ {
		
		tyfileinfo info;
		
		filegetinfo (pfs, &info);
		
		app.filetype = info.filetype;
		
		app.creator = info.filecreator;
		}
	
	app.openingfile = true; /*so newrecord routine can tell that a file is being opened -- hack!*/
	
	filegetfilename (pfs, fname);
	
	fl = newappwindow (fname, false);
	
	app.openingfile = false; 
	
	app.filetype = origfiletype;
	
	app.creator = origfilecreator;
	
	if (!fl) 
		goto error;
	
	ha = app.appwindow; /*copy into register*/
	
	if (openresourcefile (pfs, &rnum)) {
		
		tydiskwindowinfo x;
		
		if (getresource (rnum, 'aptk', 128, longsizeof (x), &x))
			loadappwindowinfo (ha, x); /*resize window, set fields of ha*/
		
		getresourcehandle (rnum, 'aptk', 129, &hoptions);
		
		closeresourcefile (rnum);
		}
		
	appprecallback ();
	
	fl = (*app.unpackcallback) (h);
	
	apppostcallback ();
		
	if (!fl) {
		
		/*alertdialog ("\pError reading from file.");*/ /*DW 12/31/95 -- commented*/
		
		goto error;
		}
		
	if (hoptions != nil) {
		
		appprecallback ();
		
		(*app.putoptionscallback) (hoptions);
		
		apppostcallback ();
		}
	
	disposehandle (h);
	
	disposehandle (hoptions);
	
	(**ha).appdata = app.appdata; /*copy from the app to our record*/
	
	(**ha).fnum = fnum;
	
	(**ha).filespec = *pfs;
	
	prepareandshowwindow (ha);
	
	return (true);
	
	error:
	
	disposeappwindow (ha); /*4.1b1 dmb: need to do this in case error is during unpack*/
	
	disposehandle (h);
	
	disposehandle (hoptions);
	
	fileclose (fnum);
	
	return (false);
	} /*openappwindow*/


boolean editappwindow (Handle hpackeddata, OSType sender, AEDesc *refcon, bigstring bstitle) {
	
	/*
	4.1b1 dmb: new call to open a window and edit data that isn't from a file
	
	we consume hpackeddata iff we succeed. on error, caller must dispose it.
	*/
	
	boolean fl = false;
	hdlappwindow appwindow;
	Handle h = hpackeddata;
	hdlappwindow ha;
	
	if (findbyrefcon (refcon, &appwindow)) {
		
		disposehandle (h);
		
		AEDisposeDesc (refcon);
		
		selectappwindow (appwindow);
		
		return (true);
		}
	
	if (stringlength (bstitle) == 0)
		getuntitledtitle (bstitle);
	
	app.openingfile = true; /*so newrecord routine can tell that a file is being opened -- hack!*/
	
	fl = newappwindow (bstitle, false);
	
	app.openingfile = false; 
	
	if (!fl)
		return (false);
	
	ha = app.appwindow; /*copy into register*/
	
	appprecallback ();
	
	fl = (*app.unpackcallback) (h);
	
	apppostcallback ();
	
	if (!fl)
		goto error;
	
	disposehandle (h);
	
	(**ha).appdata = app.appdata; /*copy from the app to our record*/
	
	(**ha).fliacdata = true;
	
	(**ha).iacdataowner = sender;
	
	(**ha).iacdatarefcon = *refcon;
	
	(**ha).flzoombeforeopen = true;
	
	prepareandshowwindow (ha);
	
	return (true);
	
	error:
	
		disposeappwindow (ha); /*need to do this in case error is during unpack*/
		
		return (false);
	} /*editverb*/


static short savedialog (bigstring fname) {
	
	bigstring prompt;

	if (!fldialogsenabled) /*don't save changes*/
		return (2);
		
	sysbeep; /*call attention to the user*/
			
	copystring ("\pSave “", prompt);
	
	pushstring (fname, prompt);
	
	pushstring ("\p” before closing?", prompt);
	
	return (threewaydialog (prompt, "\pCancel", "\pNo", "\pYes"));
	} /*savedialog*/
	
	
boolean closeappwindow (hdlappwindow appwindow, boolean fldialog) { 
	
	hdlappwindow ha = appwindow;
	
	appprecallback ();
	
	(*app.beforeclosewindowcallback) ();
	
	apppostcallback ();
	
	if ((**ha).flmadechanges && fldialog && (!(**ha).notsaveable)) {
		
		bigstring bs;
		
		getappwindowtitle (ha, bs);
		
		switch (savedialog (bs)) { /*Save “bs” before closing?*/
			
			case 1: { /*yes*/
				hdlappwindow oldappwindow = app.appwindow;
				boolean fl;
				
				setappwindow (ha);
				
				fl = savefrontwindow ();
				
				setappwindow (oldappwindow);
				
				if (!fl) /*user cancelled save*/
					return (false);
				
				break;
				}
				
			case 2: /*no*/
				break;
				
			case 3: /*cancel*/
				return (false);
				
				break;
			} /*switch*/
		}
	
	setappwindow (ha);
	
	fileclose ((**ha).fnum); 

	disposeappwindow (ha);
	
	return (true);
	} /*closeappwindow*/
	

boolean closewindowvisit (hdlappwindow appwindow) {
	
	return (closeappwindow (appwindow, true));
	} /*closewindowvisit*/
	

boolean closeallwindows (void) {
	
	#ifdef appRunsCards
	
		closeAllCardWindows (); /*close all shared windows*/
	
	#endif
	
	return (visitappwindows (&closewindowvisit));
	} /*closeallwindows*/

	
static boolean invalallwindowsvisit (hdlappwindow appwindow) {
	
	invalappwindow (appwindow, false);
	
	return (true);
	} /*invalallwindowsvisit*/
	

boolean invalallwindows (void) {
	
	return (visitappwindows (&invalallwindowsvisit));
	} /*invalallwindows*/

	
static boolean eraseallwindowsvisit (hdlappwindow appwindow) {
	
	invalappwindow (appwindow, true);
	
	return (true);
	} /*eraseallwindowsvisit*/
	

boolean eraseallwindows (void) {
	
	return (visitappwindows (&eraseallwindowsvisit));
	} /*eraseallwindows*/


boolean saveaswindow (ptrfilespec pfs) {
	
	hdlappwindow ha = app.appwindow;
	bigstring oldname;
	bigstring fname;
		
	getappwindowtitle (ha, oldname);

	fileclose ((**ha).fnum); /*close the file if it's open*/
	
	(**ha).fnum = 0;
		
	if (!saveappwindow (pfs))
		return (false);
	
	filegetfilename (pfs, fname);
	
	/*
	if (app.embeddedtables) 
		renameEmbeddedTable (oldname, fname, &alertdialog);
	*/
	
	(**ha).filespec = *pfs;
	
	setappwindowtitle (ha, fname);
	
	copystring (fname, bstargetwindowname);
	
	return (true);
	} /*saveaswindow*/
	
	
boolean saveasfrontwindow (void) {
	
	hdlappwindow ha = app.appwindow;
	tyfilespec fs;
	
	getappwindowfilespec (ha, &fs);
		
	if (!sfdialog (true, saveasprompt, &fs, app.filefiltercallback, app.filetype))
		return (false);
		
	return (saveaswindow (&fs));
	} /*saveasfrontwindow*/


static boolean saveiacdata (void) {
	
	/*
	1.0b19 dmb: odbEditor protocol -- send owner new data.
	*/
	
	hdlappwindow ha = app.appwindow;
	AppleEvent event = {typeNull, nil};
	AppleEvent reply = {typeNull, nil};
	AEDesc desc;
	Handle h = nil;
	boolean fl;
	
	appprecallback ();
	
	fl = (*app.packcallback) (&h); /*From Mayson Lancaster -- move this up in the routine, DW 12/9/93*/
	
	apppostcallback ();
	
	if (!fl) {
		
		alertdialog ("\pOut of memory.");
		
		return (false);
		}
	
	if (!IACnewverb ((**ha).iacdataowner, (**ha).creator, 'swin', &event)) {
		
		disposehandle (h);
		
		return (false);
		}
	
	IACglobals.event = &event;
	
	if (!IACpushbinaryparam (h, (**ha).filetype, '----'))
		goto exit;
	
	desc = (**ha).iacdatarefcon;
	
	IACglobals.dontdisposenextparam = true;
	
	if (!IACpushbinaryparam (desc.dataHandle, desc.descriptorType, 'refc'))
		goto exit;
	
	if (!IACsendverb (&event, &reply))
		goto exit;
	
	(**ha).flmadechanges = false;
	
	appprecallback ();
	
	(*app.aftersavewindowcallback) ();
	
	apppostcallback ();
	
	fl = true;
	
	exit:
	
	AEDisposeDesc (&event);	
	
	AEDisposeDesc (&reply);
	
	return (fl);
	} /*saveiacdata*/


boolean savefrontwindow (void) {
	
	hdlappwindow ha = app.appwindow;
	tyfilespec fs;
	
	if (ha == nil) /*defensive driving*/
		return (false);
	
	if ((**ha).fliacdata) /*4.1b1 dmb*/
		return (saveiacdata ());
	
	if ((**ha).fnum == 0)
		return (saveasfrontwindow ());
	
	fs = (**ha).filespec;
	
	return (saveappwindow (&fs));
	} /*savefrontwindow*/
	
	
boolean closefrontwindow (void) {

	WindowPtr w = FrontWindow ();
	hdlappwindow appwindow;
	
	#ifdef appRunsCards
	
		if (closeCardWindow (w))
			return (true);
	
	#endif
				
	getappwindow (w, &appwindow);
	
	return (closeappwindow (appwindow, true));
	} /*closefrontwindow*/
	

boolean openfrontwindow (void) {
	
	tyfilespec fs;
	
	if (!sfdialog (false, openprompt, &fs, app.filefiltercallback, app.filetype))
		return (false);
		
	return (openappwindow (&fs));
	} /*openfrontwindow*/
	
	
boolean revertfrontwindow (void) {
	
	hdlappwindow ha = app.appwindow;
	tyfilespec fs = (**ha).filespec;
	
	if ((**ha).flmadechanges && fldialogsenabled) {
		
		bigstring bs, bstitle;
		
		sysbeep; /*call attention to the user*/
			
		getappwindowtitle (ha, bstitle);
		
		copystring ("\pDiscard changes to “", bs);
		
		pushstring (bstitle, bs);
		
		pushstring ("\p”?", bs);
		
		if (!yesnodialog (bs))
			return (false);
			
		(**ha).flmadechanges = false; /*avoid dialog in closeappwindow call*/
		}
			
	closeappwindow (ha, fldialogsenabled);
		
	setfrontglobals (); /*seed new window from frontmost window*/

	return (openappwindow (&fs));
	} /*revertfrontwindow*/
	
	
boolean exitmainloop (void) {
	
	boolean flquitcancelled;
	
	appprecallback ();
	
	flquitcancelled = (*app.beforequitcallback) (); /*returns true if no quit*/
	
	apppostcallback ();
	
	if (flquitcancelled) /*we've been told not to exit the program*/
		return (true);
	
	flexitmainloop = closeallwindows ();
	
	setfrontglobals (); /*some or all of the windows might now be closed*/
	
	return (true);
	} /*exitmainloop*/


static void handleupdate (EventRecord *ev) {
	
	WindowPtr w = (WindowPtr) (*ev).message;
	hdlappwindow appwindow;
	
	if (!getappwindow (w, &appwindow))
		return;
	
	updateappwindow (appwindow);
	} /*handleupdate*/
	
	
static void handlediskinserted (EventRecord *ev) {
	
	/*
	see Think Reference page for DIBadMount for more info
	*/
	
	long message = (*ev).message;
	
	if ((message & 0xFFFF0000) != 0) { /*error mounting*/
		
		Point pt;
		OSErr ec;
		
		SetPt (&pt, 100, 100);
		
		ec = DIBadMount (pt, message);
		
		if (ec != noErr)
			return;
		}
	
	appprecallback ();
	
	(*app.diskinsertedcallback) (message & 0x0000FFFF);
	
	apppostcallback ();
	} /*handlediskinserted*/
	

static void handleactivate (EventRecord *ev) {

	WindowPtr w = (WindowPtr) (*ev).message;
	boolean flactivate = (*ev).modifiers & activeFlag;
	hdlappwindow appwindow;
		
	if (!getappwindow (w, &appwindow))
		return;
	
	(**appwindow).flactive = flactivate;
	
	/*
	if (app.haspalette) 
		invalpalette ((hdlpaletterecord) (**appwindow).hpalette);
	*/
	
	flipcustommenu (appwindow, flactivate);
	
	setappwindow (appwindow);

	activateappscrollbars (appwindow, flactivate);
	
	updateappwindow (appwindow); /*DW 9/16/93*/
	
	drawappgrowicon (appwindow);
	
	appprecallback (); 

	(*app.activatecallback) (flactivate);
	
	apppostcallback ();
	} /*handleactivate*/
	
	
static void handledrag (EventRecord *ev, WindowPtr w) {
	
	Rect r;

	r = quickdrawglobal (screenBits).bounds; 
   
	r.top = r.top + getmenubarheight (); 
               
	r.left = r.left + dragscreenmargin;  
               
	r.right = r.right - dragscreenmargin;
               
	r.bottom = r.bottom - dragscreenmargin;
             
	DragWindow (w, (*ev).where, &r);   
	} /*handledrag*/
	
	
static void setappwindowfrommacwindow (WindowPtr macwindow) {

	hdlappwindow appwindow;
	
	if (!getappwindow (macwindow, &appwindow))
		return;
	
	setappwindow (appwindow);
	} /*setappwindowfrommacwindow*/
	
	
static void invalforgrow (Rect r) {
	
	pushclip (r);
	
	invalrect (r);
	
	/*
	FillRect (&r, gray); 
	*/
	
	if (!app.eraseonresize)
		eraserect (r);
	
	/*
	pushbackcolor (&(**app.appwindow).backcolor);
	
	eraserect (r);
	
	popbackcolor ();
	*/
	
	popclip ();
	} /*invalforgrow*/
	
	
void adjustaftergrow (WindowPtr w, Rect oldportrect) {
	
	/*
	8/26/92 DW: optimize. we now ask for the old portrect for the window
	and only inval any parts of the window that are newly exposed.
	
	11/4/92 DW: erase the grow box first. with all the optimization we now
	have, it can take an annoying amount of time for it to get erased.
	*/
	
	hdlappwindow ha;
	short scrollbarwidth = getscrollbarwidth ();
	
	/*erase the grow box, avoid annoying delay*/ {
		
		Rect r = oldportrect;
		
		r.left = r.right - scrollbarwidth;
		
		r.top = r.bottom - scrollbarwidth;
		
		invalforgrow (r);
		}
	
	setappwindowfrommacwindow (w);
		
	/*smart inval*/ {
		
		Rect portrect = (*w).portRect;
		Rect r;
		
		r.left = oldportrect.right - scrollbarwidth;
		
		r.right = portrect.right;
		
		r.top = oldportrect.top;
		
		r.bottom = portrect.bottom;
		
		invalforgrow (r);
		
		r.top = oldportrect.bottom - scrollbarwidth;
		
		r.bottom = portrect.bottom;
		
		r.left = oldportrect.left;
		
		r.right = portrect.right;
		
		invalforgrow (r);
		}

	ha = app.appwindow;
	
	computewindowinfo (w, ha);
	
	appprecallback (); 
	
	(*app.windowresizecallback) ();
	
	apppostcallback ();
	
	resizeappscrollbars (ha);
	
	resetappscrollbars (ha);
	
	invalscrollbar ((**ha).vertbar); 
	
	invalscrollbar ((**ha).horizbar);
	
	/*always inval the grow icon*/ {
		
		Rect r;
		
		getgrowiconrect (ha, &r);
	
		invalforgrow (r);
		}
	
	setappwindowmadechanges (ha); /*we save window size and position*/
	} /*adjustaftergrow*/
	
	
static void erasewindow (WindowPtr w) {
		
	Rect r = (*w).portRect;
	
	pushmacport (w);
	
	pushclip (r);
	
	/*EraseRect (&r);*/
	
	apperaserect (r);
	
	InvalRect (&r);
	
	popclip ();
	
	popmacport ();
	} /*erasewindow*/


static void handlegrow (Point pt, WindowPtr w) {
	
	long x;
	Rect boundsrect;
	Rect r;
	Rect oldportrect = (*w).portRect;
	
	boundsrect.left = app.minwindowwidth;
	
	boundsrect.top = app.minwindowheight; 
	
	r = quickdrawglobal (screenBits).bounds;
	
	boundsrect.right = r.right - r.left; /*maximum window width*/
	
	boundsrect.bottom = r.bottom - r.top; /*maximum window height*/
	
	x = GrowWindow (w, pt, &boundsrect);
	
	/*DW 12/10/93*/ {
		
		hdlappwindow appwindow;
		Rect r;
		
		getappwindow (w, &appwindow);
		
		getgrowiconrect (appwindow, &r);
		
		EraseRect (&r);
		}
	
	SizeWindow (w, LoWord (x), HiWord (x), false);
	
	if (app.eraseonresize) 
		erasewindow (w);
	
	adjustaftergrow (w, oldportrect);
	} /*handlegrow*/
	

void zoomappwindow (hdlappwindow appwindow) {
	
	hdlappwindow ha = appwindow;
	WindowPtr w = (**ha).macwindow;
	WStateData **hstatedata = (WStateData **) (*(WindowPeek) w).dataHandle;	
	short scrollbarwidth = getscrollbarwidth ();
	Rect rzoomed, rcurrent;
	Rect oldportrect = (*w).portRect;
	short part;
	
	if (app.eraseonresize) /*DW 7/16/93*/
		erasewindow (w);
	
	appgetdesktoprect (ha, &rcurrent);
	
	rzoomed = (**hstatedata).stdState; /*default*/
	
	if (!optionkeydown ()) { /*allow option key to over-ride "smart" zoom*/
		
		boolean fl;
		
		appprecallback ();
		
		fl = (*app.getcontentsizecallback) ();
		
		apppostcallback ();
		
		if (fl) {
			
			short height = (**ha).zoomheight;
			short width = (**ha).zoomwidth;
			Rect r;
			
			if ((**ha).statuspixels > 0) 
				height += (**ha).statuspixels + 3; /*a 3-pixel line separates status & content*/
			
			width += (**ha).contentrect.left;
			
			/*allow for scrollbars and/or grow icon*/ {
				
				if (app.horizscroll)
					height += scrollbarwidth;
				
				if (app.vertscroll)
					width += scrollbarwidth;
				}
			
			/*allow for the window's title*/ {
				
				bigstring bs;
				short titlewidth;
				
				pushstyle (systemFont, 12, normal);
				
				getappwindowtitle (ha, bs);
				
				titlewidth = StringWidth (bs) + 64;
				
				if (width < titlewidth)
					width = titlewidth;
				
				popstyle ();
				}
			
			if (app.haspalette) {
				
				short palheight, palwidth;
				
				getpalettesize ((**ha).hpalette, &palheight, &palwidth);
				
				if (height < palheight)
					height = palheight;
				}
		
			if (width < app.minwindowwidth)
				width = app.minwindowwidth;
				
			if (height < app.minwindowheight)
				height = app.minwindowheight;
				
			r = rcurrent;
			
			r.right = r.left + width;
			
			r.bottom = r.top + height;
			
			constraintodesktop (&r);
			
			rzoomed = r;
			}
		}

	if (equalrects (rzoomed, rcurrent))
		part = inZoomIn;
	else {
		(**hstatedata).stdState = rzoomed;
		
		part = inZoomOut;
		}
	
	ZoomWindow (w, part, true);
	
	if (part == inZoomOut) { /*quickly fill ugly white space*/
		
		Rect r = (*w).portRect;
		
		if (app.vertscroll) /*avoid flicker in the scrollbar areas*/
			r.right -= scrollbarwidth;
			
		if (app.horizscroll) /*avoid flicker in the scrollbar areas*/
			r.bottom -= scrollbarwidth;
		
		apperaserect (r);
		}
		
	adjustaftergrow (w, oldportrect);
	
	/*scroll the contents if there's room to show everything*/ {
		
		if (part == inZoomOut) {
		
			Rect r = (**ha).contentrect;
			short minscroll, maxscroll, currentscroll;
					
			if ((r.bottom - r.top) >= (**ha).zoomheight) {
			
				getscrollbarinfo ((**ha).vertbar, &minscroll, &maxscroll, &currentscroll);
				
				setscrollbarcurrent ((**ha).vertbar, minscroll);
				
				/*note #1 -- in this case the vertical scrollbar is not inval'd anymore*/
				}
			
			getscrollbarinfo ((**ha).horizbar, &minscroll, &maxscroll, &currentscroll);
			
			setscrollbarcurrent ((**ha).horizbar, minscroll);
				
			appprecallback ();
			
			(*app.scrolltocallback) ();
			
			apppostcallback ();
			}
		}
				
	invalscrollbar ((**ha).vertbar); /*smash a bug above us, see note #1*/
	
	updateappwindow (ha);
	} /*zoomappwindow*/


boolean selectallcommand (void) {
	
	boolean fl;
	
	appprecallback ();
	
	fl = (*app.selectallcallback) ();
	
	apppostcallback ();
	
	return (fl);
	} /*selectallcommand*/
	

void appsetfont (bigstring fontname) {

	hdlappwindow ha = app.appwindow;
	short fontnumber;
	
	GetFNum (fontname, &fontnumber);
	
	(**ha).selectioninfo.fontnum = fontnumber;
	
	appprecallback (); 
	
	(*app.setfontcallback) ();
	
	apppostcallback ();
	
	(**ha).selectioninfo.fldirty = true;
	
	(**ha).defaultfont = fontnumber; /*side-effect, 7/6/92 DW*/
	} /*appsetfont*/
			
	
void appsetfontsize (short size) {
				
	hdlappwindow ha = app.appwindow;

	(**ha).selectioninfo.fontsize = size;
	
	appprecallback (); 
	
	(*app.setsizecallback) ();
	
	apppostcallback ();
	
	(**ha).selectioninfo.fldirty = true;
	
	(**ha).defaultsize = size; /*side-effect, 7/6/92 DW*/
	} /*appsetfontsize*/
	
	
static void handlekeystroke (EventRecord *ev) {
	
	/*
	11/12/92 dmb: added functionkey handling; also, allow cmd-arrows to be 
	processed by the app's keystroke callback
	*/
	
	char ch = (*ev).message & charCodeMask;
	boolean flcmdkey = (*ev).modifiers & cmdKey;
	long codeword;
	
	flcmdperiod = flcmdkey && (ch == '.'); /*set global*/
	
	#if false /*not needed, per Frontier 3.0 readme file*/
	
		if (SharedScriptRunning ()) { /*cmd-period terminates the script*/
		
			if (flcmdperiod) { 
			
				CancelSharedScript (); /*cancel the shared menu script*/
				
				return;
				}
			}
	#endif
	
	setkeyboardstatus (*ev); 
	
	if (filterfunctionkey (&codeword)) {
		
		HiliteMenu (HiWord (codeword));
		
		handlemenu (codeword);
		
		return;
		}
	
	if (flcmdkey && !arrowkey (ch)) {
	
		handlemenu (MenuKey (ch));
		
		return;
		}
	
	if (keyboardstatus.keydirection == nodirection) /*DW 9/6/93*/
		ObscureCursor ();
		
	if (app.appwindow != nil) {
		
		appprecallback (); 
		
		(*app.keystrokecallback) ();
		
		apppostcallback ();
		}
	} /*handlekeystroke*/
	
	
static void handlecontent (EventRecord *ev, WindowPtr w) {
	
	hdlappwindow ha = app.appwindow;
	Point pt = (*ev).where;
	hdlscrollbar scrollbar;
	short part;
		
	globaltolocalpoint (w, &pt);

	if (findscrollbar (pt, w, &scrollbar, &part)) { 
		
		/*DW 12/16/93 -- it could be one of Iowa's scrollbars*/
		
		if (scrollbar == (**ha).vertbar) {
		
			handlescrollbar (true, scrollbar, part, pt);
		
			return;
			}
		
		if (scrollbar == (**ha).horizbar) {
		
			handlescrollbar (false, scrollbar, part, pt);
		
			return;
			}
		}
		
	/*send mouse hit to the applet*/
	
	mousestatus.localpt = pt;
	
	mousestatus.fldoubleclick = mousedoubleclick ((*ev).where);
	
	if (PtInRect (pt, &(**ha).contentrect)) {
		
		mousestatus.localpt.h += (**ha).scrollorigin.h;
	
		mousestatus.localpt.v += (**ha).scrollorigin.v;
		
		setkeyboardstatus (*ev); /*applet can tell if shift key is down, other modifiers*/
	
		appprecallback (); 
		
		(*app.mousecallback) ();
		
		apppostcallback ();
		}

	if (PtInRect (pt, &(**ha).statusrect)) {
		
		setkeyboardstatus (*ev); /*applet can tell if shift key is down, other modifiers*/
	
		pushclip ((**ha).statusrect);
		
		(*app.mouseinstatuscallback) ();
		
		popclip ();
		}
		
	if (app.haspalette) {

		if (PtInRect (pt, &(**(**ha).hpalette).r))
			palettemousedown ((**ha).hpalette);
		}
		
	if (PtInRect (pt, &(**ha).messagerect))
		appmessageclick (ha);
	} /*handlecontent*/
	

static void handlemouseup (EventRecord *ev) {
	
	/*
	call this when you receive an mouse up event.  if the last mouse down was
	a double click, we set things up so that the next single click will not
	be interpreted as a double click.
	*/
	
	if (!mousestatus.fldoubleclickdisabled) {
		
		mousestatus.mouseuptime = (*ev).when;
		
		mousestatus.mouseuppoint = (*ev).where;
		
		mousestatus.mousedowntime = 0L; /*hasn't happened yet*/
		}
	
	mousestatus.fldoubleclickdisabled = false; /*next mouse up is important*/
	} /*handlemouseup*/


static void handlemousedown (EventRecord *ev) {

	short part;
	WindowPtr w;
	
	mousestatus.mousedowntime = (*ev).when; /*set globals so we can detect a 2click*/
	
	mousestatus.mousedownpoint = (*ev).where;
	
  	part = FindWindow ((*ev).where, &w);
	
	switch (part) {
	
		case inMenuBar: 
			handlemenu (MenuSelect ((*ev).where)); 
			
			break;
		
		case inContent:
			if (w != nil) {
			
				if (w != FrontWindow ()) { /*just like all other Mac programs*/
					
					SelectWindow (w);
					
					mousestatus.fldoubleclickdisabled = true; /*DW 9/1/93 -- avoid annoying launches in clay basket*/
									
					return; /*the mouse click is consumed by the bringtofront operation*/
					}	
				}
			
			setfrontglobals ();
			
			handlecontent (ev, w);
			
			break;
		
		case inSysWindow:
			SystemClick (ev, w); 
			
			break;
		
		case inDrag:
			handledrag (ev, w);
			
			break;
			
		case inGrow:
			handlegrow ((*ev).where, w);
			
			break;
			
		case inGoAway:
			if (TrackGoAway (w, (*ev).where)) {
			
				if (optionkeydown ())
					closeallwindows ();
				else
					closefrontwindow ();
				}
				
			break;
		
		case inZoomOut: case inZoomIn:
			if (TrackBox (w, (*ev).where, part))
				zoomappwindow (app.appwindow);
			
			break;
      	
		} /*switch*/
	} /*handlemousedown*/
	
	
static boolean jugglervisit (hdlappwindow appwindow) {
	
	hdlappwindow ha = appwindow;
	boolean fl = flcurrentlyactive && ((**ha).macwindow == FrontWindow ());
	
	setappwindow (ha);
	
	activateappscrollbars (ha, fl);
	
	appprecallback ();
	
	(*app.switchcallback) (fl); /*DW 8/28/93*/
	
	apppostcallback ();
	
	drawappgrowicon (ha); 
	
	return (true);
	} /*jugglervisit*/
	
	
static void handlejuggler (EventRecord *ev) {
	
	boolean flresume;
	tyjugglermessage jmsg;
	
	moveleft (&(*ev).message, &jmsg, longsizeof (jmsg));
	
	if (jmsg.eventtype == 1) { /*suspend or resume subevent*/
		
		flresume = jmsg.flresume; /*copy into register*/
		
		if (jmsg.flconvertclipboard) { /*11/3/91 DW: get to this later*/
			}
		
		appprecallback (); 
		
		(*app.resumecallback) (flresume); 
		
		apppostcallback ();
		
		flcurrentlyactive = flresume; /*set global before visit*/
		
		visitappwindows (&jugglervisit); /*send message to all open windows*/
		}
	} /*handlejuggler*/


static boolean handledialogevent (EventRecord *ev) {
	
	/*
	if the event belongs to a dialog, allow it to be processed in 
	the standard way. the applet toolkit doesn't currently support the 
	creation of modeless dialogs, but we must be a host to them anyway.
	*/
	
	DialogPtr dlog;
	short itemnumber;
	
	if (!IsDialogEvent (ev)) 
		return (false);
	
	DialogSelect (ev, &dlog, &itemnumber);
	
	return (true);
	} /*handledialogevent*/


boolean flappopenfileonidle; /*can be set externally, for iowawires.c*/

tyfilespec fsidleopen;


static void appidle (void) {
	
	if (flappopenfileonidle) {
		
		flappopenfileonidle = false;
		
		openappwindow (&fsidleopen);
		}

	appprecallback ();
	
	(*app.idlecallback) ();
	
	(*app.preupdatecallback) ();
	
	apppostcallback ();		
	} /*appidle*/
	
	
static boolean appletmainvalidatevisit (hdlappwindow appwindow) {
	
	/*
	DW 10/9/94 -- it appears that the flactive bit in the appwindow 
	record for the Reader window is incorrect when a card is in front.
	
	the problem was in bbsAppleTalkAPI.c -- we were throwing away events
	while waiting for a reply to a message. oops.
	*/
	
		/*
		if (((**appwindow).macwindow == FrontWindow ()) && flcurrentlyactive)
			assert ((**appwindow).flactive);
		else
			assert (!(**appwindow).flactive);
		*/
	
	return (true); /*keep visiting*/
	} /*appletmainvalidatevisit*/
	
	
static void appletmainvalidate (void) {
	
	visitappwindows (&appletmainvalidatevisit);
	} /*appletmainvalidate*/
	
	
static boolean nulleventvisit (hdlappwindow appwindow) {
	
	setappwindow (appwindow);
	
	appidle ();
			
	return (true); /*keep traversing*/
	} /*nulleventvisit*/
	
	
void appleteventhandler (EventRecord *ev) {
	
	/*
	if you have an event, and have determined that it's not for you, pass it
	thru this routine for processing. 
	
	this routine is ideal for card routines to pass to apprunmodalresource 
	as its event filter or to call it from their event filter.
	*/
	
	WindowPtr oldfrontwindow = FrontWindow ();

	if (countwindows () == 0) { /*DW 10/27/95, updated 11/17/95*/
	
		setcursortype (cursorisarrow);
		
		disableeditmenuitems ();
		}
	
	app.commanderror = false; /*9/3/93 DW: app can raise an error flag which some functions watch*/
	
	appresetsystemidle (ev); /*based on event type, might start another idle watch*/
		
	switch ((*ev).what) {
	
		case keyDown: case autoKey: 
			handlekeystroke (ev);
			
			break;
			
		case mouseDown:
			handlemousedown (ev);
		   
			break;
			
		case mouseUp:
			handlemouseup (ev);
			
			break;
		   
		case activateEvt:
			handleactivate (ev); 
		   
			break;
		
		case osEvt:
			handlejuggler (ev);
			
			break;

		case updateEvt:
			handleupdate (ev);
		   
			break;
			
		case diskEvt:
			handlediskinserted (ev);
			
			break;
			
		case nullEvent:
			visitappwindows (&nulleventvisit);
			
			resetdirtyscrollbars ();
			
			setfrontglobals (); /*the menus reflect the status of the frontmost window*/
			
			adjustmenus ();
			
			appidle ();
			
			appsystemidle ();
			
			if (flcurrentlyactive)
				CheckSharedMenus (firstsharedmenu);
			
			#ifdef fldebug
				appletmainvalidate ();
			#endif
	
			break;
			
		case kHighLevelEvent:
			AEProcessAppleEvent (ev);
			
			break;
		} /*switch*/
		
	if (FrontWindow () != oldfrontwindow) { /*a different window is in front*/
		
		if (setfrontglobals ())
			(**app.appwindow).selectioninfo.fldirty = true; /*force update of selection info*/
		else
			flipcustommenu (nil, false); /*DW 8/19/93: last window just closed, get rid of any custom menus*/
		}
	} /*appleteventhandler*/
	

void appleteventdispatcher (EventRecord *ev) {
	
	/*
	the event might be for the Dialog Manager (handling a modeless dialog)
	or a window sharing component, or for the applet itself.
	
	we pass the event thru each entity, if it returns false, we go to the
	next one. finally we pass it thru the applet hierarchy.
	*/
	
	if (!handledialogevent (ev)) {
		
		#ifdef appRunsCards
		
			boolean flcloseallwindows;
		
			if (isModelessCardEvent (ev, &flcloseallwindows)) {
				
				if (flcloseallwindows) /*user option-clicked in close box of a card window*/
					closeallwindows ();
					
				return;
				}
		#endif
	
		IACglobals.waitroutine = &appwaitroutine; /*for waiting after sending an Apple Event*/

		setfrontglobals (); /*event applies to frontmost window*/
		
		GetMouse (&mousestatus.localpt);

		appleteventhandler (ev); 
		}
	} /*appleteventdispatcher*/
	
	
static boolean idleclosevisit (hdlappwindow appwindow) {
	
	if ((**appwindow).flcloseonidle) 		
		closeappwindow (appwindow, true);
		
	return (true); /*keep traversing*/
	} /*idleclosevisit*/
	
	
static void checkcloseonidle (void) {
	
	/*8/31/94 DW
	the clay basket message editor wants to close a message 
	editing window after it has been successfully uploaded to
	the server. it does so by setting a flag in the appwindow
	record, we watch for it here and close the window in a 
	safe place.
	*/
	
	visitappwindows (&idleclosevisit);
	} /*checkcloseonidle*/


static long getwaitticks (void) {
		
	long ticks = 10;
	
	if (!flcurrentlyactive)
		ticks = 20;

	return (ticks);
	} /*getwaitticks*/
	
	
void maineventloop (void) {
	
	EventRecord ev;
	
	while (!flexitmainloop) {
		
		keyboardclearescape ();
		
		checkcloseonidle ();
		
		WaitNextEvent (everyEvent, &ev, getwaitticks (), nil);
		
		appletevent = ev; /*set global, can be accessed by anyone with an interest*/
		
		appleteventdispatcher (&ev);
		} /*while*/
	} /*maineventloop*/


boolean appserviceeventqueue (void) {
	
	/*
	pop and process all waiting events, return true when we get to a null 
	event. return false if the user selected the quit command, or otherwise
	wants the program to terminate.
	
	8/8/92 DW: return false also if the user presses cmd-period.
	*/
	
	EventRecord ev;
	
	flcmdperiod = false;
	
	while (!flexitmainloop) {
		
		WaitNextEvent (everyEvent, &ev, getwaitticks (), nil);
		
		if (ev.what == nullEvent) /*got all the meaningful events*/
			return (true);
		
		appleteventdispatcher (&ev);
		
		if (flcmdperiod) {
			
			/*sysbeep;*/
			
			return (false);
			}
		} /*while*/
		
	return (false); /*user must have selected the quit command*/
	} /*appserviceeventqueue*/


static void initscrap (void) {
	
	PScrapStuff x;
	
	x = InfoScrap ();
	
	if ((*x).scrapState < 0) /*following advice of THINK Reference*/
		ZeroScrap ();
	} /*initscrap*/
	

static boolean noopcallback (void) {
	
	/*sysbeep;*/
	
	return (false);
	} /*noopcallback*/
	
	
static void checkcallback (tyappcallback *cb) {
	
	if (*cb == nil) /*the applet doesn't define this callback*/
		*cb = &noopcallback;
	} /*checkcallback*/
	
	
static void normaleraserectcallback (Rect r) {
	
	EraseRect (&r);
	} /*normaleraserectcallback*/
	
	
static boolean normalsetfontcallback (void) {
	
	hdlappwindow ha = app.appwindow;
	tyselectioninfo x = (**ha).selectioninfo;
	
	if (x.fontnum != -1)
		(**ha).defaultfont = x.fontnum;
	
	if (x.fontsize != -1)
		(**ha).defaultsize = x.fontsize;
		
	invalappwindow (ha, true);
	
	return (true);
	} /*normalsetfontcallback*/
	
	
static void checknilcallbacks (void) {
	
	installscroll (); /*install default handlers if appropriate*/
	
	checkcallback (&app.newrecordcallback); 
	
	checkcallback (&app.disposerecordcallback); 
	
	checkcallback (&app.idlecallback); 
	
	checkcallback ((tyappcallback *) &app.activatecallback);
	
	checkcallback ((tyappcallback *) &app.switchcallback);
	
	checkcallback (&app.updatecallback);
	
	checkcallback (&app.preupdatecallback);
	
	checkcallback (&app.windowresizecallback);
	
	checkcallback (&app.iacmessagecallback);
	
	checkcallback (&app.iacfastmessagecallback);
	
	checkcallback ((tyappcallback *) &app.packcallback);
	
	checkcallback ((tyappcallback *) &app.unpackcallback);
	
	checkcallback ((tyappcallback *) &app.gettextcallback); 
	
	checkcallback ((tyappcallback *) &app.getpictcallback); 
	
	checkcallback ((tyappcallback *) &app.scrollcallback); 
	
	checkcallback ((tyappcallback *) &app.resetscrollcallback); 
	
	checkcallback (&app.scrolltocallback); 
	
	checkcallback (&app.pagesetupcallback);
	
	checkcallback (&app.openprintcallback);
	
	checkcallback ((tyappcallback *) &app.printpagecallback);
	
	checkcallback (&app.closeprintcallback);
	
	checkcallback ((tyappcallback *) &app.puttextcallback);
	
	checkcallback ((tyappcallback *) &app.putpictcallback);
	
	checkcallback (&app.haveselectioncallback);
	
	checkcallback (&app.selectallcallback);
	
	checkcallback (&app.keystrokecallback);
	
	checkcallback (&app.mousecallback);
	
	checkcallback (&app.mouseinstatuscallback);
	
	checkcallback (&app.updatestatuscallback);
	
	checkcallback ((tyappcallback *) &app.menucallback);
	
	checkcallback (&app.insertmenucallback);
	
	checkcallback (&app.setselectioninfocallback);
		
	if (app.setfontcallback == nil)
		app.setfontcallback = &normalsetfontcallback;
	
	if (app.setsizecallback == nil)
		app.setsizecallback = &normalsetfontcallback;
			
	checkcallback (&app.setstylecallback);
	
	checkcallback (&app.setjustifycallback);
	
	checkcallback ((tyappcallback *) &app.copycallback);
		
	checkcallback (&app.clearcallback);
	
	checkcallback ((tyappcallback *) &app.pastecallback);
	
	checkcallback (&app.initmacintoshcallback);
	
	checkcallback ((tyappcallback *) &app.resumecallback);
	
	checkcallback (&app.openappcallback);
	
	checkcallback ((tyappcallback *) &app.opendoccallback);
	
	if (app.eraserectcallback == nil)
		app.eraserectcallback = &normaleraserectcallback;
		
	checkcallback ((tyappcallback *) &app.getcontentsizecallback);
	
	checkcallback ((tyappcallback *) &app.setglobalscallback);
	
	checkcallback ((tyappcallback *) &app.getoptionscallback);
	
	checkcallback ((tyappcallback *) &app.putoptionscallback);
	
	checkcallback ((tyappcallback *) &app.getundoglobalscallback);
	
	checkcallback ((tyappcallback *) &app.setundoglobalscallback);
	
	checkcallback ((tyappcallback *) &app.afteropenwindowcallback);
	
	checkcallback ((tyappcallback *) &app.beforeclosewindowcallback);
	
	checkcallback ((tyappcallback *) &app.beforequitcallback);
	
	checkcallback ((tyappcallback *) &app.diskinsertedcallback);
	
	checkcallback ((tyappcallback *) &app.interpretfilecallback);
	
	checkcallback ((tyappcallback *) &app.aftersavewindowcallback);
	
	if (app.alertcallback == nil)
		app.alertcallback = &alertdialog;
	} /*checknilcallbacks*/
	
	
static pascal void pascalalertdialog (Str255 s) {
	
	if (stringlength (s) > 0)
		alertdialog (s);
	} /*pascalalertdialog*/
	
	
static pascal void pascaleventfilter (EventRecord *ev) {
	
	appleteventdispatcher (ev);
	} /*pascaleventfilter*/
	
	
static boolean managersinited = false;

	
boolean appletinitmanagers (void) { /*4/13/93 DW: returns a boolean, for Card Editor*/

	initmacintosh ();
	
	checknilcallbacks (); /*initmenus uses a callback routine*/
	
	if (app.minwindowwidth == 0)
		app.minwindowwidth = 125;
		
	if (app.minwindowheight == 0)
		app.minwindowheight = 100;
		
	app.dontpostcallback = false; 
	
	appletheapzone = GetZone ();
	
	initmenus ();
	
	initscrap ();
	
	initsyserror ();

	IACinit ();
	
	clearbytes (&mousestatus, longsizeof (mousestatus));
	
	#ifdef appRunsCards
	
		initIowaRuntime ();
	
	#endif
		
	InitSharedMenus (&pascalalertdialog, &pascaleventfilter);
		
	initappletwires (); 
	
	(*app.initmacintoshcallback) ();
	
	managersinited = true;
	
	return (true);
	} /*appletinitmanagers*/
	
	
void runapplet (void) {
	
	if (!managersinited) 
		appletinitmanagers ();
	
	maineventloop ();
	
	#if false
	
		IACremovesystemhandlers ();
		
	#endif
	} /*runapplet*/
	
	
	