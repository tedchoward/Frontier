
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

#include "cursor.h"
#include "memory.h"
#include "file.h"
#include "quickdraw.h"
#include "strings.h"
#include "kb.h"
#include "mouse.h"
#include "ops.h"
#include "popup.h"
#include "resources.h"
#include "scrap.h"

#include "scrollbar.h"
#include "search.h"
#include "frontierwindows.h"
#include "windowlayout.h"
#include "zoom.h"

#include "shell.h"
#include "shellhooks.h"
#include "shellundo.h"
#include "lang.h"
#include "langexternal.h"
#include "tableinternal.h"
#include "tabledisplay.h"
#include "tablestructure.h"
#include "tableverbs.h"
#include "cancoon.h"
#include "opinternal.h"
#include "opverbs.h"
#include "claybrowser.h"
#include "claybrowserstruc.h"
//#include "browserverbs.h"
#include "kernelverbdefs.h"


// 5.0a25 dmb: wire off Zoom button
#undef iconrectheight
#undef iconrectwidth
#define iconrectheight 0
#define iconrectwidth 0


WindowPtr tableformatswindow = nil;

hdlwindowinfo tableformatswindowinfo = nil;

hdltableformats tableformatsdata = nil;



boolean tableverbsetglobals (void) {
	
	/*
	5.0b15 dmb: handle nil tableformatsdata
	*/

	if (tableformatsdata == nil) {
		
		opsetoutline (nil);

		return (false);
		}

	opsetoutline ((**tableformatsdata).houtline); 
	
	outlinewindow = tableformatswindow;
	
	outlinewindowinfo = tableformatswindowinfo;
	
	if (outlinedata != nil)
		opeditsetglobals ();

	return (true);
	} /*tableverbsetglobals*/


#define tabletopmargin 3


static boolean tableverbresetrects (hdlwindowinfo hw) {
	
	register short scrollbarwidth;
	register hdltableformats hc = (hdltableformats) (**hw).hdata;
	hdloutlinerecord ho;
	Rect rcontent;
	Rect r;
	Rect tablerect, titlerect, wholerect;
	short msgheight;
	short titleheight = 12;
	#define seprectheight 4 /*height of rectangle separating titles from table content*/
	
	if (hc != nil) { // don't check outlinedata if tableformats aren't even attached yet
		
		ho = (**hc).houtline;
		
		if (ho == nil) // this emulates original return when outlinedata was nil
			return (false);
		
		if (isclaydisplay (hc))
			titleheight = 0;
		else
			titleheight = (**ho).defaultlineheight;
		}
	
	rcontent = (**hw).contentrect;
	
	/*set up some values for all rect computations*/ {
	
		scrollbarwidth = getscrollbarwidth ();
		
		msgheight = popupheight; 
		}
	
	/*do info.growiconrect*/ {
		
		shellcalcgrowiconrect (rcontent, hw);
		}
	
	/*do wholerect*/ {
		
		r = rcontent;
		
		r.top += tabletopmargin; /*three pixels between titles and top of window*/
		
		r.bottom -= msgheight + windowmargin;
		
		r.left += windowmargin;
		
		r.right -= windowmargin + iconrectwidth + windowmargin + scrollbarwidth;
		
		wholerect = r;
		}
	
	tablerect = wholerect;
	
	/*do titlerect*/ {
		
		titlerect = tablerect; /*set top, left, right*/
		
		titlerect.bottom = titlerect.top + titleheight;
		
		tablerect.top = titlerect.bottom + seprectheight;
		}
	
	if (hc != nil) {
		
		(**hc).wholerect = wholerect;
		
		(**hc).tablerect = tablerect;
		
		(**hc).titlerect = titlerect;
		}
	
	/*do kindpopuprect*/ {
		
		r.left = tablerect.left + 3; /*flush with left edge of icons*/
		
		r.right = r.left + popupwidth;
		
		r.top = tablerect.bottom + ((rcontent.bottom - tablerect.bottom - msgheight) / 2);
		
		r.bottom = r.top + msgheight;
		
		if (hc != nil)
			(**hc).kindpopuprect = r; 
		}
		
	/*do sortpopuprect*/ {
		
		r.left = r.right + popupbetweenwidth;
		
		r.right = r.left + popupwidth;
		
		if (hc != nil)
			(**hc).sortpopuprect = r;
		}
		
	/*do messagerect*/ {
		
		r.left = r.right + popupbetweenwidth;
		
		r.right = tablerect.right + scrollbarwidth;
		
		(**hw).messagerect = r; 
		}
		
	/*do iconrect*/ {
	
		r.top = tablerect.top;
		
		r.bottom = r.top + iconrectheight;
		
		r.right = rcontent.right - windowmargin;
		
		r.left = r.right - iconrectwidth;
		
		if (iconrectwidth > 0)
			insetrect (&r, -4, 0); /*a little extra width for title*/
		
		if (hc != nil)
			(**hc).iconrect = r; 
		}
		
	/*do vertscrollbar*/ {
		
		r = tablerect; /*the space occupied by the grid of cells*/
		
		r.left = r.right; /*scrollbar is just to right of grid*/
		
		r.right = r.left + scrollbarwidth;
		
		setscrollbarrect ((**hw).vertscrollbar, r);
		
		showscrollbar ((**hw).vertscrollbar);
		}
	
	return (true);
	} /*tableverbresetrects*/
	

static boolean tableverbgetcontentsize (long *width, long *height) {
	
	short pixels;
	
	tablegetoutlinesize (width, height);
	
	pixels = tabletopmargin + windowmargin + popupheight + 4;
	
	*height += pixels;
	
	pixels = windowmargin + windowmargin + iconrectwidth + windowmargin + getscrollbarwidth ();
	
	*width += pixels;
	
	return (true);
	} /*tableverbgetcontentsize*/


static void tableverbresize (void) {
	
	hdltableformats hf = tableformatsdata;
	Rect r;
	
	if (hf != nil) {

		r = (**hf).wholerect;
		
	//	eraserect (r);
		
	//	tabledirty ();
		
		r = (**hf).tablerect;
		
		#ifdef gray3Dlook
			insetrect (&r, -2, -2);
		#else
			insetrect (&r, 1, 1);
		#endif
				
		opresize (r);
		
		tablerecalccolwidths (false); // 5.0a5 dmb: don't insist on recalc
				
		#ifdef WIN95VERSION
			opupdatenow ();
		#endif
		}
	} /*tableverbresize*/


static boolean tableverbsetfont (void) {
	
	opsetfont ((**outlinewindowinfo).selectioninfo.fontnum);
	
	shelladjustaftergrow (tableformatswindow); /*PBS 7.0b52: Get col. titles caught up.*/
	
	return (true);
	} /*tableverbsetfont*/


static boolean tableverbsetsize (void) {
	
	opsetsize ((**outlinewindowinfo).selectioninfo.fontsize);

	shelladjustaftergrow (tableformatswindow); /*PBS 7.0b52: Get col. titles caught up.*/
	
	return (true);
	} /*tableverbsetsize*/


static boolean tablesetselectioninfo (void) {
	
	if (outlinedata == nil)
		return (false);
		
	return (opsetselectioninfo ());
	} /*tablesetselectioninfo*/


static boolean tableverbgettargetdata (short id) {
	
	/*
	a verb is about to be executed that acts on the indicated external type, or 
	on any shell window if id == -1.
	
	return true if we can handle verbs of that type and are able to set the 
	relevant globals
	*/
	
	switch (id) {
		
		case -1:
			return (!debuggingcurrentprocess () || !(**tablegetlinkedhashtable ()).fllocaltable);
		
		case idtableprocessor:
		case idoutlineprocessor:
		#ifdef xmlfeatures
			case idxmlprocessor:
		#endif
			return (true);
		
		case idwordprocessor:
			return (opeditsetglobals ());
		
		default:
			return (false);
		}
	} /*tableverbgettargetdata*/


static void tableverbidle (void) {
	
	if (outlinedata == nil)
		return;

	if (tablechecksortorder ())
		tableupdatecoltitles (false);
	
	if (tablecheckzoombutton ())
		tabledrawzoombutton (false);
	
	browsercommitchanges ();
	
	opidle ();
	} /*tableverbeventloopidle*/


static boolean tablegetvariableroutine (hdlexternalvariable *hvariable) {
	
	hdlhashtable ht;
	
	if (tableformatsdata == nil)
		return (false);
	
	ht = (hdlhashtable) (**tableformatsdata).htable;
	
	*hvariable = (hdlexternalvariable) (**ht).hashtablerefcon;
	
	return (true);
	} /*tablegetvariableroutine*/


boolean tableverbclose (void) {
	
	hdltableformats hf = tableformatsdata;
	hdlhashtable ht;
	hdloutlinerecord ho = outlinedata;
	
	if (hf == nil) //already closed & disposed?
		return (true);
	
	ht = (hdlhashtable) (**hf).htable;
	
//	tableverbcheckwindowrect (ho);
	
	killundo (); /*must toss undos before they're stranded*/
	
//	flmustexiteditmode = true;
	
	tableexiteditmode ();
	
	assert ((**hf).editval == nil);
	
	if (tableoutlineneedssaving ()) { /*we have to keep the in-memory version around*/
		
		(**ho).flwindowopen = false;
		
		opcloseoutline (); /*prepare for dormancy, not in a window anymore*/
		}
	else {
		tabledisposeoutline (hf);
		}
	
	killundo (); /*toss any newly-generated undos*/
	
	tablewindowclosed ((hdlexternalvariable) (**ht).hashtablerefcon);
	
	shellclosechildwindows (tableformatswindowinfo); /*close our sub-windows first*/
	
	tablecheckwindowrect (ht);
	
	(**ht).flwindowopen = false;
	
	return (true);	
	} /*tableverbclose*/


static boolean tableverbchildclose (WindowPtr w) {
	
	/*
	a child window of the table is closing.  
	
	for now (6/12/90) just call the close routine on the child window.
	
	9/24/91 dmb: pass through return value from child
	*/
	
	boolean fl;
	
	shellpushglobals (w);
	
	fl = (*shellglobals.closeroutine) ();
	
	shellpopglobals ();
	
	return (fl);
	} /*tableverbchildclose*/


static void tableverbupdate (void) {
	
	tableupdate ();
	
	} /*tableverbupdate*/


static void tableverbactivate (boolean flactivate) {
	
	if (outlinedata == NULL) // we're not completely set up yet
		return;

	opactivate (flactivate);
	
	(**tableformatsdata).flactive = flactivate;
	
	tabledrawzoombutton (false); /*its appearance might depend on activation state*/
	} /*tableverbactivate*/


static boolean tableverbtitleclick (Point pt) {
	
	if (cmdkeydown () || ismouserightclick()) {
		
		tableclienttitlepopuphit (pt, (hdlexternalvariable) tablegetlinkedtablevariable ());
		
		return (true);
		}
	
	if (!mousedoubleclick ())
		return (false);
	
	tablesurface ();
	
	return (true); /*consumed*/
	} /*tableverbtitleclick*/


static boolean tableverbadjustcursor (Point pt) {
	
	register hdltableformats hf = tableformatsdata;
	short col;
	
	if (pointinrect (pt, (**hf).tablerect)) {
		
		if (tablefindcolumnguide (pt, &col)) {
			
			setcursortype (cursorisverticalrails);
			
			return (true);
			}
				
		return (opsetcursor (pt));
		}
	
	if (adjustpopupcursor (pt, (**hf).kindpopuprect))
		return (true);
	
	if (adjustpopupcursor (pt, (**hf).sortpopuprect))
		return (true);
	
	setcursortype (cursorisarrow);
	
	return (true);
	} /*tableverbadjustcursor*/


static boolean tableverbmousedown (Point pt, tyclickflags flags) {
	
	register hdltableformats hf = tableformatsdata;
	short col;
	
	if (pointinrect (pt, (**hf).titlerect))
		return (tabletitleclick (pt));
		
	if (pointinrect (pt, (**hf).iconrect)) {
		
		if (!(**hf).fliconenabled)
			return (true);
		
		return (tablezoombuttonhit ());
		}
	
	if (pointinrect (pt, (**hf).kindpopuprect)) 
		return (tablekindpopuphit (pt));
	
	if (pointinrect (pt, (**hf).sortpopuprect)) 
		return (tablesortpopuphit (pt));
	
	if (pointinrect (pt, (**hf).tablerect))  {
	
		if (tablefindcolumnguide (pt, &col))
			return (tableadjustcolwidth (pt, col));
		
		return (opmousedown (pt, flags));
		}
	
	/*
	ouch ();
	*/
	
	return (true);
	} /*tableverbmousedown*/


static void tableedittabkey () {
	
	short col = (**tableformatsdata).editcol;
	
	if (keyboardstatus.flshiftkey)
		--col;
	else
		++col;
	
	if (tablecelliseditable ((**outlinedata).hbarcursor, col)) {
		
		(**tableformatsdata).focuscol = col;
		
		opunloadeditbuffer ();
		
		oploadeditbuffer ();
		
		opeditselectall ();
		}
	else
		sysbeep ();
	} /*tableedittabkey*/


static void tableeditreturnkey () {
	} /*tableeditreturnkey*/


static boolean tableverbkeystroke (void) {
	
	/*
	5.0a17 dmb: for Win, Backspace surfaces

	5.0b18 dmb: for Win, control key always does window surface
	*/

	boolean flcmdkey = keyboardstatus.flcmdkey;
	
	#ifdef WIN95VERSION
		if ((keyboardstatus.chkb == chbackspace) && (keyboardstatus.flshiftkey || !opistextmode ())) {
			
			if (keyboardstatus.flshiftkey || !opmotionkey (left, 1, false))	//can't move to parent*/
				if (!tablesurface ())			//can't surface to parent
					shellouch ();
			
			return (true);
			}
	#endif
	
	if ((keyboardstatus.chkb == chreturn) && opistextmode ()) {
		
		opsettextmode (false);
		
		return (true);
		}

	if (keyboardstatus.chkb == chtab) { // cycle through columns
		
		if ((**outlinedata).fltextmode) {
			
			tableedittabkey ();
			
			return (true);
			}
		}
	
	if ((keyboardstatus.chkb == chenter) && flcmdkey) {
		
		if (keyboardstatus.flshiftkey)
			return (tablesurface ());
		else
			return (tabledive ());
		}
	
	return (opkeystroke ());
	} /*tableverbkeystroke*/


static boolean tableverbpaste (void) {
	
	Handle hscrap;
	tyscraptype type;
	
	if (!shellgetscrap (&hscrap, &type))
		return (false);
	
	if (type == hashscraptype)
		return (oppaste ());
	else
		return (tablepasteroutine ());
	} /*tableverbpaste*/


static boolean tableverbruncursor (void) {
	
	if (!tablecursorisrunnable ())
		return (false);
	
	return (tableruncursor ());
	} /*tableverbruncursor*/


static boolean tableverbcmdkeyfilter (char chkb) {

	if (outlinedata == nil)
		return (true);
	
	return (opcmdkeyfilter (chkb));
	} /*tableverbcmdkeyfilter*/


static short beforeprintcolwidths [maxtablecols];


static boolean tablebeginprint (void) {
	
	hdltableformats hf = tableformatsdata;
	
	moveleft ((**hf).colwidths, beforeprintcolwidths, sizeof (short) * maxtablecols);
	
	opbeginprint ();
	
	/*
	(**hf).tablerect = (**outlinedata).outlinerect;
	
	clearbytes ((**hf).colwidths,  sizeof (short) * maxtablecols);
	
	tablerecalccolwidths (true);
	*/

	return (true);
	} /*tablebeginprint*/


static boolean tableendprint (void) {
	
	opendprint ();
	
	opsetdisplaydefaults (outlinedata); //need to reset lineheights before next call
	
	tableverbresetrects (tableformatswindowinfo);

	tableverbresize ();
	
	moveleft (beforeprintcolwidths, (**tableformatsdata).colwidths, sizeof (short) * maxtablecols);
	
	return (true);
	} /*tableendprint*/


boolean tablebeforeprintpage (void) {
	
	hdltableformats hf = tableformatsdata;
	
	(**hf).tablerect = (**outlinedata).outlinerect;
	
	clearbytes ((**hf).colwidths,  sizeof (short) * maxtablecols);
	
	tablerecalccolwidths (true);
	
	return (true);
	} /*tablebeforeprintpage*/


boolean tableafterprintpage (void) {
	
	moveleft (beforeprintcolwidths, (**tableformatsdata).colwidths, sizeof (short) * maxtablecols);
	
	return (true);
	} /*tableafterprintpage*/


boolean tablestart (void) {
	
	/*
	set up callback routines record, and link our data into the shell's 
	data structure.
	*/
	
	ptrcallbacks tablecallbacks;
	register ptrcallbacks cb;
	
	assert (sizeof (tytablevariable) == 16L);
	
	tableinitverbs ();
	
	shellpushscraphook (&tablescraphook);

	shellnewcallbacks (&tablecallbacks);
	
	cb = tablecallbacks; /*copy into register*/
	
	loadconfigresource (idtableconfig, &(*cb).config);
	
	(*cb).configresnum = idtableconfig;
		
	(*cb).windowholder = &tableformatswindow;
	
	(*cb).dataholder = (Handle *) &tableformatsdata;
	
	(*cb).infoholder = &tableformatswindowinfo;
	
	(*cb).setglobalsroutine = &tableverbsetglobals;
	
	(*cb).pushroutine = &oppushglobals;

	(*cb).poproutine = &oppopglobals;
	
#ifdef version42orgreater
	
	(*cb).disposerecordroutine = ccdisposefilerecord;
	
	(*cb).saveroutine = ccsavespecialfile;

#endif
	
	(*cb).updateroutine = &tableverbupdate;
	
	(*cb).activateroutine = &tableverbactivate;
	
	(*cb).getcontentsizeroutine = &tableverbgetcontentsize;
	
	(*cb).resetrectsroutine = &tableverbresetrects;
	
	(*cb).resizeroutine = &tableverbresize;
	
	(*cb).scrollroutine = &opscroll;
	
	(*cb).setscrollbarroutine = &opresetscrollbars;
	
	(*cb).mouseroutine = &tableverbmousedown;

	(*cb).rmouseroutine = &oprmousedown; /*7.0b13 PBS: right-click in tables*/

	(*cb).keystrokeroutine = &tableverbkeystroke;
	
	(*cb).titleclickroutine = &tableverbtitleclick;
	
	(*cb).cutroutine = &opcut;
	
	(*cb).copyroutine = &opcopy;
	
	(*cb).pasteroutine = &tableverbpaste;
	
	(*cb).clearroutine = &opclear;
	
	(*cb).selectallroutine = &opselectall;
	
	(*cb).closeroutine = &tableverbclose;
	
	(*cb).childcloseroutine = &tableverbchildclose;
	
	(*cb).getundoglobalsroutine = &tableeditgetundoglobals;
	
	(*cb).setundoglobalsroutine = &tableeditsetundoglobals;
	
	(*cb).idleroutine = &tableverbidle;
	
	(*cb).adjustcursorroutine = &tableverbadjustcursor;
	
	(*cb).gettargetdataroutine = &tableverbgettargetdata;
	
	(*cb).getvariableroutine = &tablegetvariableroutine;
	
	(*cb).settextmoderoutine = &opsettextmode;
	
	(*cb).fontroutine = &tableverbsetfont;
	
	(*cb).sizeroutine = &tableverbsetsize;
	
	(*cb).setselectioninforoutine = &tablesetselectioninfo;
	
	(*cb).searchroutine = &tableverbsearch;
	
	(*cb).executeroutine = &tableverbruncursor;
	
	(*cb).setprintinfoproutine = &opsetprintinfo;
	
	(*cb).beginprintroutine = &tablebeginprint;
	
	(*cb).endprintroutine = &tableendprint;
	
	(*cb).printroutine = &opprint;
	
	(*cb).cmdkeyfilterroutine = &tableverbcmdkeyfilter;
	
	return (true);
	} /*tablestart*/




