
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

 
#include <standard.h>
#include "memory.h"
#include "file.h"
#include "quickdraw.h"
#include "strings.h"
#include "kb.h"
#include "ops.h"
#include "popup.h"
#include "resources.h"
#include "scrollbar.h"
#include "search.h"
#include "windows.h"
#include "windowlayout.h"
#include "zoom.h"

#include "shell.h"
#include "shellhooks.h"
#include "shellundo.h"
#include "lang.h"
#include "langexternal.h"
#include "tableinternal.h"
#include "tablestructure.h"
#include "cancoon.h"
#include "opinternal.h"
#include "opverbs.h"
#include "claybrowser.h"
#include "claybrowserstruc.h"
//#include "browserverbs.h"
#include "kernelverbdefs.h"



#define browserstringlist 166
#define browsertypestring 1
#define browsersizestring 2


#define browsererrorlist 266
#define nobrowsererror 1
#define namenotbrowsererror 2


typedef enum tybrowsertoken { /*verbs that are processed by browserverbs.c*/
	
	zoomscriptfunc,
	
	ctbrowserverbs
	} tybrowsertoken;


WindowPtr browserwindow = nil;

hdlwindowinfo browserwindowinfo = nil;

hdlclaydata browserdata = nil;



static errornum = 0; /*error number exclusively for browser routines*/


static boolean browsersetglobals (void) {
	
	outlinedata = (**browserdata).houtline; 
	
	tableformatsdata = (**browserdata).hformats;
	
	outlinewindow = browserwindow;
	
	outlinewindowinfo = browserwindowinfo;
	
	return (opeditsetglobals ());
	} /*browsersetglobals*/


#define tabletopmargin 3


static boolean browserresetrects (hdlwindowinfo hw) {
	
	register short scrollbarwidth;
	register hdlclaydata hc = browserdata;
	Rect rcontent;
	Rect r;
	Rect tablerect, seprect, titlerect, wholerect;
	short msgheight;
	hdltableformats hformats = tableformatsdata;
	#define seprectheight 2 /*height of rectangle separating titles from table content*/
	
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
	
	/*do titlerect, seprect*/ {
		
		titlerect = tablerect; /*set top, left, right*/
		
		titlerect.bottom = titlerect.top + (**hc).computedlineinfo.filenamelineheight;
		
		seprect = tablerect; /*set left, right*/
		
		seprect.top = titlerect.bottom;
		
		seprect.bottom = seprect.top + seprectheight;
		
		tablerect.top = seprect.bottom;
		}
	
	(**hc).wholerect = wholerect;
	
	(**hc).tablerect = tablerect;
	
	(**hc).titlerect = titlerect;
	
	(**hc).seprect = seprect;
	
	/*do kindpopuprect*/ {
		
		r.left = tablerect.left + 3; /*flush with left edge of icons*/
		
		r.right = r.left + popupwidth;
		
		r.top = tablerect.bottom + ((rcontent.bottom - tablerect.bottom - msgheight) / 2);
		
		r.bottom = r.top + msgheight;
		
		(**hc).kindpopuprect = r; 
		}
		
	/*do sortpopuprect*/ {
		
		r.left = r.right + popupbetweenwidth;
		
		r.right = r.left + popupwidth;
		
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
		
		insetrect (&r, -4, 0); /*a little extra width for title*/
		
		(**hc).iconrect = r; 
		}
		
	/*do vertscrollbar*/ {
		
		r = tablerect; /*the space occupied by the grid of cells*/
		
		r.left = r.right; /*scrollbar is just to right of grid*/
		
		r.right = r.left + scrollbarwidth;
		
	//	r.bottom = r.top + (**hformats).nonblankvertpixels;
		
		setscrollbarrect ((**hw).vertscrollbar, r);
		
		showscrollbar ((**hw).vertscrollbar);
		}
	
	return (true);
	} /*browserresetrects*/
	

static boolean browsergetcontentsize (long *width, long *height) {
	
	short pixels;
	
	claygetoutlinesize (width, height);
	
	pixels = tabletopmargin + windowmargin + popupheight + 4;
	
	*height += pixels;
	
	pixels = windowmargin + windowmargin + iconrectwidth + windowmargin + getscrollbarwidth ();
	
	*width += pixels;
	
	return (true);
	} /*browsergetcontentsize*/


static void browserresize (void) {
	
	Rect r;
	
	r = (**browserdata).wholerect;
	
	eraserect (r);
	
//	tabledirty ();
	
	r = (**browserdata).tablerect;
	
	insetrect (&r, 1, 1);
	
	opresize (r);
	
	clayrecalccolwidths (true); /*re-distribute colwidths*/
	} /*browserresize*/


static boolean browsersetfont (void) {
	
	return (opsetfont ((**outlinewindowinfo).selectioninfo.fontnum));
	} /*browsersetfont*/


static boolean browsersetsize (void) {
	
	return (opsetsize ((**outlinewindowinfo).selectioninfo.fontsize));
	} /*browsersetsize*/


static boolean browsergettargetdata (short id) {
	
	/*
	a verb is about to be executed that acts on the indicated external type, or 
	on any shell window if id == -1.
	
	return true if we can handle verbs of that type and are able to set the 
	relevant globals
	*/
	
	switch (id) {
		
		case -1:
			return (true);
		
		case idoutlineprocessor:
			return (true);
		
		case idwordprocessor:
			return (opeditsetglobals ());
		
		default:
			return (false);
		}
	} /*browsergettargetdata*/


static boolean browserclose (void) {
	
	register hdloutlinerecord ho = outlinedata;
	
//	opverbcheckwindowrect (ho);
	
	killundo (); /*must toss undos before they're stranded*/
	
	if ((**ho).fldirty) { /*we have to keep the in-memory version around*/
		
		(**ho).flwindowopen = false;
		
		opcloseoutline (); /*prepare for dormancy, not in a window anymore*/
		}
	
	return (true);
	} /*browserclose*/


static void browseridle (void) {
	
	opidle ();
	
	browsercommitchanges ();
	} /*browsereventloopidle*/
	

static boolean getsystemname (bigstring systemname) { // dmb
	
	StringHandle h;
	
	h = GetString (-16413);
	
	if (ResError () != noErr)
		return (false);
		
	copyheapstring (h, systemname);
	
	if (stringlength (systemname) == 0)
		copystring ("\pUntitled System", systemname);
	
	return (true);
	} /*getsystemname*/


static boolean browsernewwindow (void) {
	
	hdlwindowinfo ha = browserwindowinfo;
	hdlclaydata hc = nil;
	hdloutlinerecord ho = nil;
	Rect rzoom, rwindow;
	hdlwindowinfo hparent;
	hdltableformats hformats = nil;
	tyfilespec fs;
	
	if (!shellgetfrontrootinfo (&hparent)) /*our parent is the frontmost root window*/
		return (false);
	
	rzoom.top = -1; /*zoom from default rect*/
	
	if (!newclearhandle (sizeof (tyclaydata), (Handle *) &hc))
		return (false);
	
	browserdata = hc;
	
	claydefaultlinelayout (&(**hc).linelayout);
	
	ccgetwindowrect (ixbrowserinfo, &rwindow);
	
	if (!newoutlinerecord (&ho))
		goto error;
	
	(**hc).houtline = ho;
	
	(**ho).outlinerefcon = (long) hc;
	
	(**ho).flusebitmaps = false; 
	
	if (!browserinitrecord (hc))
		goto error;
	
	if (!newtableformats (&hformats))
		goto error;
	
	(**browserdata).hformats = hformats;
	
	pushscratchport ();
	
//	if (!tableunpackformats (hpackedformats, hformats)) {
		
		tablesetdimension (hformats, false, 1, fixedctcols);
//		}
	
	popport ();
	
//	if (ho != nil) 	
//		getoutlinerect (&(**ho).outlinerect); /*allow callback to change statuspixels, perhaps other window measures*/
	
	shellpushfrontrootglobals ();
	
	windowgetfspec (shellwindow, &fs);
	
	shellpopglobals ();
	
	if (!newchildwindow (idbrowserconfig, hparent, rwindow, rzoom, fs.name, &browserwindow))
		goto error;
	
//	windowsetcolor (browserwindow, (**hm).forecolor, true);
	
//	windowsetcolor (browserwindow, (**hm).backcolor, false);
		
	getwindowinfo (browserwindow, &browserwindowinfo);
	
	(**browserwindowinfo).hdata = (Handle) hc;
	
	ccnewsubwindow (browserwindowinfo, ixbrowserinfo);
	
	shellpushglobals (browserwindow); 
	
	browserresize ();
	
//	browsersetfontsize ();
	
	windowzoom (browserwindow);
	
	shellpopglobals ();
	
	return (true);
	
	error:
			
		opdisposeoutline (ho, true);
		
		disposehandle ((Handle) hc);
		
		return (false);
	} /*browsernewwindow*/


static boolean browserfindvisit (WindowPtr w) {
	
	/*
	if we're called, the w is a window with the desired config, owned 
	the the current root.
	
	set up browser globals and return false to terminate the visit
	*/
	
	browserwindow = w;
	
	getwindowinfo (w, &browserwindowinfo);
	
	browserdata = (hdlclaydata) (**browserwindowinfo).hdata;
	
	return (false);
	} /*browserfindvisit*/


static boolean browserfindwindow (short idconfig) {
	
	/*
	5/22/91 dmb: use customized visit to limit search to windows belonging to the 
	current root.  this allows us to distinguish between the quickscript window 
	for each open file.
	*/
	
	hdlwindowinfo hroot;
	
	if (!frontrootwindow (&hroot))
		return (false);
	
	return (!visitownedwindows ((**hroot).macwindow, idconfig, &browserfindvisit));
	
	/*
	return (shellfindwindow (idconfig, &browserwindow, &browserwindowinfo, (Handle *) &browserdata));
	*/
	} /*browserfindwindow*/


boolean browseropenmainwindow (void) {
	
	if (browserfindwindow (idbrowserconfig)) { /*sets browserdata, browserwindow*/
		
		windowbringtofront (browserwindow);
		
		return (true);
		}
	
	return (browsernewwindow ());
	} /*browseropenmainwindow*/


boolean browserstart (void) {
	
	/*
	set up callback routines record, and link our data into the shell's 
	data structure.
	*/
	
	ptrcallbacks browsercallbacks;
	register ptrcallbacks cb;
	
//	browserinitverbs ();
	
//	meinit ();
	
	shellnewcallbacks (&browsercallbacks);
	
	cb = browsercallbacks; /*copy into register*/
	
	loadconfigresource (idbrowserconfig, &(*cb).config);
	
	(*cb).configresnum = idbrowserconfig;
		
	(*cb).windowholder = &browserwindow;
	
	(*cb).dataholder = (Handle *) &browserdata;
	
	(*cb).infoholder = &browserwindowinfo;
	
	(*cb).setglobalsroutine = &browsersetglobals;
	
	(*cb).updateroutine = &browserupdate;
	
	(*cb).activateroutine = &opactivate;
	
	(*cb).getcontentsizeroutine = &browsergetcontentsize;
	
	(*cb).resetrectsroutine = &browserresetrects;
	
	(*cb).resizeroutine = &browserresize;
	
	(*cb).scrollroutine = &opscroll;
	
	(*cb).setscrollbarroutine = &opresetscrollbars;
	
	(*cb).mouseroutine = &opmousedown;
	
	(*cb).keystrokeroutine = &opkeystroke;
	
//	(*cb).titleclickroutine = &browsertitleclick;
	
	(*cb).cutroutine = &opcut;
	
	(*cb).copyroutine = &opcopy;
	
	(*cb).pasteroutine = &oppaste;
	
	(*cb).clearroutine = &opclear;
	
	(*cb).selectallroutine = &opselectall;
	
	/*
	(*cb).disposerecordroutine = &medispose;
	*/
	
	(*cb).closeroutine = &browserclose;
	
	(*cb).getundoglobalsroutine = &opeditgetundoglobals;
	
	(*cb).setundoglobalsroutine = &opeditsetundoglobals;
	
//	(*cb).childcloseroutine = &browserchildclose;
	
	(*cb).idleroutine = &browseridle;
	
	(*cb).adjustcursorroutine = &opsetcursor;
	
	(*cb).gettargetdataroutine = &browsergettargetdata;
	
//	(*cb).getvariableroutine = &browsergetvariable;
	
	(*cb).settextmoderoutine = &opsettextmode;
	
	(*cb).fontroutine = &browsersetfont;
	
	(*cb).sizeroutine = &browsersetsize;
	
	(*cb).setselectioninforoutine = &opsetselectioninfo;
	
//	(*cb).searchroutine = &browsersearch;
	
//	(*cb).executeroutine = &browserrunscript;
	
	(*cb).setprintinfoproutine = &opsetprintinfo;
	
	(*cb).printroutine = &opprint;
	
	return (true);
	} /*browserstart*/




