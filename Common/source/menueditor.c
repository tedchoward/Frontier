
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

#include "mac.h"
#include "memory.h"
#include "strings.h"
#include "bitmaps.h"
#include "quickdraw.h"
#include "font.h"
#include "smallicon.h"
#include "scrollbar.h"
#include "icon.h"
#include "kb.h"
#include "ops.h"
#include "cursor.h"
#include "menu.h"
#include "popup.h"
#include "dialogs.h"
#include "frontierwindows.h"
#include "windowlayout.h"
#include "zoom.h"
#include "db.h"
#include "tablestructure.h"
#include "shell.h"
#include "shellprivate.h"
#include "shellundo.h"
#include "shell.rsrc.h"
#include "op.h"
#include "opinternal.h"
#include "scripts.h"
#include "menueditor.h"
#include "menuinternal.h"
#include "cancoon.h"
#include "langexternal.h"




hdlmenurecord menudata;

WindowPtr menuwindow; 

hdlwindowinfo menuwindowinfo; 


#define sizecmdkeyarray 256 /*room for 256 handles*/

#define buttonfont geneva
#define buttonsize 9
#define buttonstyle 0

#define roundrectsize 10



#ifdef fldebug

void mecheckglobals (void) {
	
	assert (menudata != nil);
	
	assert (outlinedata == (**menudata).menuoutline); 
	
	/*
	assert (outlinewindow == menuwindow);
	
	assert (outlinewindowinfo == menuwindowinfo);
	*/
	} /*mecheckglobals*/

#endif

boolean mesetglobals (void) {
	
	if (menudata == nil) {
		
		opsetoutline (nil);

		return (false);
		}

	opsetoutline ((**menudata).menuoutline);
	
	outlinewindow = menuwindow;
	
	outlinewindowinfo = menuwindowinfo;
	
	if (outlinedata != nil)
		opeditsetglobals ();
	
	return (true);
	} /*mesetglobals*/


hdldatabaserecord megetdatabase (hdlmenurecord hm) {
	
	return ((**(hdlexternalvariable) (**hm).menurefcon).hdatabase);
	} /*megetdatabase*/

	
static void megetcursornode (hdlmenurecord hm, hdlheadrecord *hcursor) {
	
	assert (menudata != nil);
	
	*hcursor = (**(**menudata).menuoutline).hbarcursor;
	} /*megetcursornode*/


boolean mescroll (tydirection dir, boolean flpage, long amount) {	
	
	mecheckglobals ();
	
	return (opscroll (dir, flpage, amount));
	} /*mescroll*/
	
	
void megetscrollbarinfo (void) {
	
	mecheckglobals ();
	
	opresetscrollbars ();
	} /*megetscrollbarinfo*/


static boolean mesetscrollbarsroutine (void) {

	register ptrwindowinfo pw = *menuwindowinfo;
	register ptroutlinerecord po = *outlinedata;
	
	(*pw).vertscrollinfo = (*po).vertscrollinfo; /*copy from outline record to window record*/
	
	(*pw).fldirtyscrollbars = true; /*force a refresh of scrollbars by the shell*/
	
	return (true);
	} /*mesetscrollbarsroutine*/


static medeactivateoutline (void) {
	
	mecheckglobals ();
	
	if ((**outlinedata).flactive)
		opactivate (false);
	
	return (true);
	} /*medeactivateoutline*/


static void meactivateoutline (void) {
	
	(**menudata).menuactiveitem = menuoutlineitem;
	

	mecheckglobals ();
	
	if (!(**outlinedata).flactive) {
		//WriteToConsole("Delta");
		opactivate (true);

	}
	} /*meactivateoutline*/


static boolean megetscriptaddress (hdlheadrecord hnode, dbaddress *address) {
	
	tymenuiteminfo item;
	
	megetmenuiteminfo (hnode, &item); /*will clear item on failure*/
	
	*address = item.linkedscript.adrlink;
	
	return (true);
	} /*megetscriptaddress*/


static boolean megetscriptoutline (hdlheadrecord hnode, hdloutlinerecord *hscript) {
	
	tymenuiteminfo item;
	
	megetmenuiteminfo (hnode, &item); /*will clear item on failure*/
	
	*hscript = item.linkedscript.houtline;
	
	return (true);
	} /*megetscriptoutline*/


boolean mesetscriptoutline (hdlheadrecord hnode, hdloutlinerecord hscript) {
	
	tymenuiteminfo item;
	
	megetmenuiteminfo (hnode, &item);
	
	item.linkedscript.houtline = hscript;
	
	return (mesetmenuiteminfo (hnode, &item));
	} /*mesetscriptoutline*/


static void medrawseparatorline (hdlheadrecord hnode, const Rect *rtext, boolean flselected, boolean flinverted) {
	
	/*
	7/10/92 dmb: don't set displaypixels to maxlevelwidth; doing so 
	ensures that all future maxlevelwidths can't decrease
	
	7.0b59 PBS: select the icon also.
	*/
	
	register hdlheadrecord hn = hnode;
	register short h, v;
	register short pixels;
	hdloutlinerecord ho = outlinedata;

	v = ((*rtext).top + (*rtext).bottom) / 2;
	
	h = (*rtext).left;
	
	pushpen ();
	
	setgraypen ();
	
	movepento (h, v);
	
	pixels = opmaxlevelwidth (hn);
	
	/*
	(**hn).displaypixels = pixels;
	*/
	
	h += pixels;
	
	pendrawline (h, v);
	
	poppen ();
	
	if (flselected) {
		
		Rect rframe = *rtext;
		
		//rframe.left -= 2; /*leave 2 extra pixels for prettier bar cursor*/
		
		rframe.right = h + 2; /*leave 2 extra pixels for prettier bar cursor*/
		
		rframe.left -= (**ho).iconwidth + (**ho).pixelsaftericon + texthorizinset; /*7.0b59 PBS: select icon.*/

		if (flinverted)
			invertrect (rframe);
		else
			grayframerect (rframe);
		}
	} /*medrawseparatorline*/
	

static boolean medrawlineroutine (hdlheadrecord hnode, const Rect *textrect, boolean flselected, boolean flinverted) {
	
	/*
	a bottleneck routine that draws a line in the outline structure.  if it is one
	of the lines we're interested in, we draw it and return.  otherwise we call the
	standard line drawing routine implemented in op.c.
	
	the design of this callback structure follows the design of the QuickDraw 
	bottleneck routines.
	*/
	
	bigstring bsnode;
	
	opgetheadstring (hnode, bsnode);
	
	if (stringlength (bsnode) >= 1) { /*length of string is at least 1*/
		
		if (getstringcharacter (bsnode, 0) == (byte) '-') { /*first character is a minus*/
			
			medrawseparatorline (hnode, textrect, flselected, flinverted);
			
			return (true);
			}
		}
	
	return (opdefaultdrawtext (hnode, textrect, flselected, flinverted));
	} /*medrawlineroutine*/


void megetoutlinerect (Rect *r) {
	
	*r = (**menudata).menuoutlinerect;
	} /*megetoutlinerect*/
	
	
static boolean meclearhandlevisit (hdlheadrecord hnode, ptrvoid refcon) {
	
	tymenuiteminfo item;
	
	if (megetmenuiteminfo (hnode, &item)) {
		
		item.linkedscript.houtline = nil;
		
		mesetmenuiteminfo (hnode, &item);
		}
		
	return (true);
	} /*meclearhandlevisit*/
	
	
static void meclearhandles (hdloutlinerecord ho) {
	
	/*
	after a structure is unpacked, we have to visit all the nodes and clear
	any handles linked in the refcon structure, since they are no longer
	valid.  we don't do this on saving because the handles are still in use.
	*/
	
	opsiblingvisiter ((**ho).hsummit, false, &meclearhandlevisit, nil);
	} /*meclearhandles*/


static boolean mesomethingdirtyvisit (hdlheadrecord hnode, ptrvoid refcon) {

	register hdlheadrecord h = hnode;
	register hdloutlinerecord ho;
	tymenuiteminfo item;
	
	if (!megetmenuiteminfo (h, &item)) /*nothing linked, keep visiting*/
		return (true);
	
	ho = item.linkedscript.houtline; /*copy into register*/
	
	if (ho == nil)
		return (true);
	
	if ((**ho).fldirty)
		return (false); /*stop traversal*/
	
	return (true);
	} /*mesomethingdirtyvisit*/


boolean mesomethingdirty (hdlmenurecord hmenurecord) {
	
	/*
	we want to know if something is dirty, either the menurecord itself or
	something linked into it.  someday, we may want to make this more efficient
	by having a system of callback routines, that percolate dirty bits up
	the ownership hierarchy.
	*/
	
	register hdlmenurecord hm = hmenurecord;
	register hdloutlinerecord ho;
	hdlheadrecord hsummit;
	boolean flsomethingdirty;
	
	if ((**hm).fldirty)
		return (true);
		
	ho = (**hm).menuoutline;
	
	if ((**ho).fldirty)
		return (true);
		
	oppushoutline (ho);
	
	opoutermostsummit (&hsummit);
	
	flsomethingdirty = !opsiblingvisiter (hsummit, false, &mesomethingdirtyvisit, nil);
	
	oppopoutline ();
	
	return (flsomethingdirty);
	} /*mesomethingdirty*/


boolean meloadoutline (dbaddress adr, hdloutlinerecord *houtline) {
	
	/*
	load the outline stored at database address adr.  if adr is nil, create a 
	new empty outline structure.  return false if something didn't work.
	
	5/21/92 dmb: fixed error handling when dbrefhandle fails.  (used to leave 
	outline pushed.)
	*/
	
	register boolean fl;
	register hdloutlinerecord ho;
	Handle hpackedoutline;
	Rect r;
	long ixload = 0;
	
	*houtline = nil; /*default return*/
	
	oppushoutline (nil); /*preserve global*/
	
	if (adr == nildbaddress) { /*new structure is called for*/
		
		megetoutlinerect (&r);
		
		fl = opnewrecord (r, houtline);
		}
	else {
		fl = dbrefhandle (adr, &hpackedoutline);
		
		if (fl) {
			
			fl = opunpack (hpackedoutline, &ixload, houtline);
			
			disposehandle (hpackedoutline);
			}
		}
	
	ho = *houtline;
	
	oppopoutline (); /*restore global*/
	
	if (!fl)
		return (false);
	
	opvalidate (ho);
	
	(**ho).setscrollbarsroutine = &mesetscrollbarsroutine;
	
	(**ho).drawlinecallback = &medrawlineroutine;
	
	meclearhandles (ho);
	
	/*
	6/21/90 DW: this can be a problem if there's no outline window open.
	none of the callers seems to depend on these two calls.
	
	megetoutlinerect (&(**ho).outlinerect);
	
	opgetdisplayinfo ();
	*/
	
	*houtline = ho;
		
	return (fl);
	} /*meloadoutline*/


boolean mesaveoutline (hdloutlinerecord ho, dbaddress *adr) {
	
	/*
	save an outline in the database, and return the address of the database
	memory that was allocated.
	*/
	
	register boolean fl;
	Handle hpackedoutline;
	
	hpackedoutline = nil; /*allocate a new handle for packing*/
	
	if (!oppackoutline (ho, &hpackedoutline))
		return (false);
	
	fl = dbsavehandle (hpackedoutline, adr);
	
	disposehandle (hpackedoutline);
	
	if (!fl)
		return (false);
	
	if (!fldatabasesaveas)
		(**ho).fldirty = false;
	
	return (true);
	} /*mesaveoutline*/


static boolean mescriptfontchangeroutine (void) {
	
	/*
	called when the font of a script window has been changed.  we implement a 
	side-effect -- we save the current outline font and size as the default
	font and size for all new script windows.
	
	12/3/91 dmb: move outlinedata into local before pushing parent globals
	
	5.0.2b16 dmb: call opseteditbufferrect
	*/
	
	register hdloutlinerecord ho = outlinedata; /*save off before pushing new globals*/
	
	shellpushparentglobals (); /*script window is in front, parent is a menu window*/
	
	assert (menudata != nil);
	
	(**menudata).defaultscriptfontnum = (**ho).fontnum;
	
	(**menudata).defaultscriptfontsize = (**ho).fontsize;
	
	shellpopglobals ();
	
	oppostfontchange ();
	
	return (true);
	} /*mescriptfontchangeroutine*/


boolean meloadscriptoutline (hdlmenurecord hm, hdlheadrecord hnode, hdloutlinerecord *houtline, boolean *fljustloaded) {
	
	/*
	load in the outline linked to the indicated headrecord, return false if there was
	an error loading the outline.
	
	if *houtline returns as nil, and the function returns true, there's no script linked
	in -- if you're zooming a window you want to create a new linked script.
	
	12/13/90 dmb: added fljustloaded parameter for safe unloading.

	7.0b26 PBS: menubar scripts get fat headlines. Horizontal scrolling is disabled.
	*/
	
	register boolean fl;
	dbaddress adr;
	Handle hpackedoutline;
	long ixload = 0;
	
	*fljustloaded = false;
	
	megetscriptoutline (hnode, houtline); /*is there a dirty copy already in memory?*/
	
	if (*houtline != nil) { /*a dirty script is already linked in*/
	
		opsetdisplaydefaults (*houtline);
		
		return (true);
		}
	
	megetscriptaddress (hnode, &adr);
	
	if (adr == nildbaddress) {
		
		*houtline = nil; /*no linked script*/
		
		return (true);
		}
	
	dbpushdatabase (megetdatabase (hm));
	
	fl = dbrefhandle (adr, &hpackedoutline);
	
	dbpopdatabase ();

	if (!fl) /*error loading from database*/
		return (false);

	
	fl = opunpack (hpackedoutline, &ixload, houtline);

	(***houtline).flfatheadlines = true; /*7.0b26 PBS: fat headlines in menubar scripts.*/

	(***houtline).flhorizscrolldisabled = true; /*7.0b26 PBS: horizontal scrolling is disabled.*/
	
	disposehandle (hpackedoutline);
	
	if (fl) {
	
		opsetdisplaydefaults (*houtline);
		
		*fljustloaded = true;
		}
	
	return (fl);
	} /*meloadscriptoutline*/


static boolean meloadscript (hdlmenurecord hm, Rect rscript, hdloutlinerecord *houtline) {
	
	/*
	1/2/91 dmb: stick the cursor's hdlheadrecord into the script outline's 
	refcon.  this allows the script debugger to use mescripterrorroutine 
	(meprograms.c) after the script has been closed and unloaded.
	
	added hm parameter; avoid globals. we assume that the script window's globals are pushed
	*/
	
	register hdloutlinerecord ho;
	hdlheadrecord hcursor;
	register boolean flnewscript = false;
	boolean flignore;
	
	megetcursornode (hm, &hcursor);
	
	if (!meloadscriptoutline (hm, hcursor, houtline, &flignore)) 
		return (false);
	
	ho = *houtline; /*copy into register*/
	
	if (ho == nil) { /*creating a new script outline*/
		
		flnewscript = true;
		
		assert (hm != nil);
		
		(**outlinewindowinfo).defaultfont = (**hm).defaultscriptfontnum;
		
		(**outlinewindowinfo).defaultsize = (**hm).defaultscriptfontsize;
		
		if (!opnewrecord (rscript, houtline))
			return (false);
		
		ho = *houtline;
		
		(**ho).outlinesignature = typeLAND;
		}
	
//	opvalidate (ho);
	
	meclearhandles (ho);
	
	(**ho).flwindowopen = true;
	
	(**ho).outlinerect = rscript;
	
	if (flnewscript)
		(**ho).fltextmode = true;
	
	/*
	(**ho).fltextmode = bitboolean (flnewscript);
	*/
	
	(**ho).horizscrollinfo.cur = (**ho).horizscrollinfo.min; /*always opens flush left*/
	
	(**ho).postfontchangecallback = &mescriptfontchangeroutine;
	
	(**ho).outlinerefcon = (long) hcursor; /*point back to headline -- for script debugger*/
	
	(**ho).outlinetype = outlineismenubarscript;
	
	(**ho).flfatheadlines = true;
	
	return (true);
	} /*meloadscript*/


static boolean meunloadscript (hdlmenurecord hm) {
	
	/*
	1/2/91: killundo.  also, use opcloseoutline, not opunloadeditbuffer
	
	5.0b13 dmb: clean up; expect menubar's, not outline's globals to be set
	but take hm parameter anyway to reduce globals usage
	*/
	
	register hdloutlinerecord ho = (**hm).scriptoutline;
	register hdlheadrecord hn = (**hm).scriptnode;
	WindowPtr w = (**hm).scriptwindow;
	
	assert (hm != nil);
	
	if (ho == nil) { /*defensive driving*/
		
		if (hn != nil)
			mesetscriptoutline (hn, nil); /*keep in synch*/
		
		return (false);
		}
	
	if (w != nil) {
		
		shellpushglobals (w);
		
		killundo (); /*must toss undos before they're stranded*/
		
		opcloseoutline (); /*prepare for dormancy, not in a window anymore*/
		}
	
	(**ho).flwindowopen = false;
	
	if (!(**ho).fldirty) { /*script isn't dirty, no reason to keep it around*/
		
		opdisposeoutline (ho, false);
		
		if (w != nil)
			shellclearwindowdata ();
		
		mesetscriptoutline (hn, nil);
		}
	
	if (w != nil)
		shellpopglobals ();
	
	return (true);
	} /*meunloadscript*/


static boolean mehaslinkedtext (hdlheadrecord hnode) {
	
	tymenuiteminfo item;
	tylinkeditem linkeditem;
	
	if (!megetmenuiteminfo (hnode, &item)) 
		return (false);
	
	linkeditem = item.linkedscript;
	
	return ((linkeditem.adrlink != nildbaddress) || (linkeditem.houtline != nil));
	} /*mehaslinkedtext*/


static void medisplayscriptbutton (boolean flpressed) {
	
	register hdlmenurecord hm = menudata;
	register boolean fl = flpressed;
	bigstring bs;
	Rect r, rfill;
	hdlheadrecord hcursor;
	
	r = (**hm).iconrect;	//dialoggetobjectrect (menuwindow, menuscriptitem, &r);
	
	if (isemptyrect (r))
		return;

	shellgetstring (scriptbuttonstring, bs);
	
	megetcursornode (hm, &hcursor);
	
	drawlabeledwindoidicon (r, bs, mehaslinkedtext (hcursor), fl);
	
	if (!fl) { /*not highlighted*/
		
		if ((**hm).scriptwindow != nil) { /*only relevant when window is open*/
	
			if ((**hm).flautosmash) { /***indicate autosmash is turned on*/
				
				rfill.left = rfill.top = 0;
				
				rfill.right = rfill.bottom = 13;
				
				centerrect (&rfill, r);
				
				offsetrect (&rfill, 0, -3);
				
				grayframerect (rfill);
				}
			}
		}
	} /*medisplayscriptbutton*/


boolean mesmashscriptwindow (void) {
	
	/*
	change the contents of the script window so that it displays the 
	script linked into the current bar cursor menu item.
	
	12/4/91 dmb: update the changes bit if the script windowinfo
	*/
	
	register hdlmenurecord hm = menudata;
	register WindowPtr w = (**hm).scriptwindow;
	hdlheadrecord hcursor; 
	hdloutlinerecord houtline;
	register boolean fl;
	
	megetcursornode (hm, &hcursor);
	
	if ((**hm).scriptnode == hcursor) /*window already displays this guy*/
		return (true);
	
	meunloadscript (hm); /*detach the script window outline from the window*/
	
	shellpushglobals (w);
	
	fl = meloadscript (hm, (**outlinewindowinfo).contentrect, &houtline);
	
	if (fl)
		shellsetwindowchanges (shellwindowinfo, (**houtline).fldirty);
	
	shellpopglobals ();
	
	if (!fl)
		return (false);
	
	scriptsetdata (w, hcursor, houtline);
	
	mesetscriptoutline (hcursor, houtline); /*link outline record into menu*/
	
	(**hm).scriptnode = hcursor;
	
	(**hm).scriptoutline = houtline;
	
	return (true);
	} /*mesmashscriptwindow*/


boolean mezoomscriptwindow (void) {
	
	/*
	5.0a10 dmb: call windowzoom here, after everything is set up. 
	for windows.
	*/

	register hdlmenurecord hm = menudata;
	WindowPtr scriptwindow = (**hm).scriptwindow;
	hdlheadrecord hcursor;
	Rect rzoom;
	WindowPtr wnew;
	
	megetcursornode (hm, &hcursor);
	
	if (scriptwindow == nil) { /*window hasn't been zoomed*/
	
		rzoom = (**hm).iconrect; // dialoggetobjectrect (menuwindow, menuscriptitem, &rzoom);
		
		localtoglobalrect (menuwindow, &rzoom);
		
		if (!scriptzoomwindow ((**hm).scriptwindowrect, rzoom, hcursor, &wnew))
			return (false);
		
		(**hm).scriptwindow = scriptwindow = wnew;
		
		medisplayscriptbutton (false); /*display may change when window is open*/
		}
	
	if (!mesmashscriptwindow ()) { /*load window contents with script*/
		
		disposeshellwindow (scriptwindow);
		
		(**hm).scriptwindow = nil;
		
		return (false);
		}
	
	windowzoom (scriptwindow);
	
	windowbringtofront (scriptwindow); 
	
	return (true);
	} /*mezoomscriptwindow*/


static boolean mefreshscriptwindow (void) {
	
	/*
	check to see if the script window is still displaying the script attached
	to the cursor in the menu structure.
	
	the cursor might have moved, the cursor might have been deleted, etc.
	
	returns true if it refreshed the window.
	
	wired-off 6/1/89 DW: user now has to hit the script button in the menu
	window to get the contents of the script window to change.
	*/
	
	register hdlmenurecord hm = menudata;
	register hdlheadrecord hcursor;
	
	if (!(**hm).flautosmash) /*turned off*/
		return (false); 
	
	if ((**hm).scriptwindow == nil) /*no script window open*/
		return (false);
	
	hcursor = (**(**hm).menuoutline).hbarcursor;
	
	if ((**hm).scriptnode == hcursor) /*cursor didn't change*/
		return (false);
	
	return (mesmashscriptwindow ());
	} /*mefreshscriptwindow*/


static byte megetcmdkey (hdlheadrecord hnode) {
	
	tymenuiteminfo item;
	
	megetmenuiteminfo (hnode, &item); /*will clear item on failure*/
	
	return (item.cmdkey);
	} /*megetcmdkey*/


static void medisplaycmdkeypopup (void) { 
	
	register hdlmenurecord hm = menudata;
	register byte ch;
	hdlheadrecord hcursor;
	bigstring bs;
	Rect r;
	
	r = (**menudata).cmdkeypopuprect; // dialoggetobjectrect (menuwindow, menucmdkeypopupitem, &r);
	
	megetcursornode (hm, &hcursor);
	
	ch = megetcmdkey (hcursor);
	
	if (ch == chnul) {
		
		shellgetstring (cmdkeypopupstring, bs);

		pushchar (':', bs);
		}
	else
		getcommandkeystring (ch, keycommand, bs);
	
	drawpopup (r, bs, true);
	} /*medisplaycmdkeypopup*/


/*
static medisplaycmdkey (void) { 
	
	/*
	10/7/91 dmb: resurrected the seperate item for the setting the command key
	%/
	
	register unsigned char ch;
	hdlheadrecord hcursor;
	Rect r;
	
	dialoggetobjectrect (menuwindow, menucmdkeyitem, &r);
	
	openbitmap (r, menuwindow);
	
	eraserect (r);
	
	framerect (r);
	
	megetcursornode (&hcursor);
	
	ch = megetcmdkey (hcursor);
	
	pushclip (r);
	
	if (ch != chnul) {
		
		bigstring bs;
		
		setstringwithchar (ch, bs);
		
		pushstyle (geneva, 9, 0);
		
		centerstring (r, bs);
		
		popstyle ();
		}
	
	if ((**menudata).menuactiveitem == menucmdkeyitem) {
		
		insetrect (&r, 1, 1);
		
		invertrect (r);
		}
	
	popclip ();
	
	closebitmap (menuwindow);
	} /*medisplaycmdkey*/


static void mezoomdownwindow (WindowPtr w) {
	
	Rect r;
	
	if (w == nil) /*defensive driving*/
		return;
	
	r = (**menudata).iconrect; // dialoggetobjectrect (menuwindow, item, &r);
	//Code change by Timothy Paustian Monday, August 21, 2000 4:31:49 PM
	//Must pass a CGrafPtr to pushport on OS X to avoid a crash
	{
	CGrafPtr	thePort;
	#if TARGET_API_MAC_CARBON == 1
	thePort = GetWindowPort(menuwindow);
	#else
	thePort = (CGrafPtr)menuwindow;
	#endif
		
	pushport (thePort);
	}
	medisplayscriptbutton (true); /*simulate pressing the button*/
	
	popport ();

	localtoglobalrect (menuwindow, &r);
	
	zoomwindowtocenter (r, w);
	
		//Code change by Timothy Paustian Monday, August 21, 2000 4:31:49 PM
	//Must pass a CGrafPtr to pushport on OS X to avoid a crash
	{
	CGrafPtr	thePort;
	#if TARGET_API_MAC_CARBON == 1
	thePort = GetWindowPort(menuwindow);
	#else
	thePort = (CGrafPtr)menuwindow;
	#endif
		
	pushport (thePort);/*who knows what zoomport did to the port?*/
	}
	
	medisplayscriptbutton (false); /*hope it looks neat!!!*/
	
	popport ();
	} /*mezoomdownwindow*/


boolean mescriptwindowclosed (void) {
	
	/*
	we need to be notified when the script window closes so we can update
	our data structures accordingly.

	5.0a10 dmb: instead of trying to fix Win crash, skip silly mezoomdownwindow
	*/
	
	register hdlmenurecord hm = menudata;
	Rect r;
	
	meunloadscript (hm); /*detach the script from the window*/
	
	getglobalwindowrect ((**hm).scriptwindow, &r);
	
	(**hm).scriptwindowrect = r; /*remember for next time we zoom a script window*/
	
	#ifdef MACVERSION
	mezoomdownwindow ((**hm).scriptwindow); /*zoom it back to its button*/
	#endif
	
	(**hm).scriptwindow = nil;
	
	(**hm).scriptnode = nil;
	
	(**hm).scriptoutline = nil;
		
	//Code change by Timothy Paustian Monday, August 21, 2000 4:31:49 PM
	//Must pass a CGrafPtr to pushport on OS X to avoid a crash
	{
	CGrafPtr	thePort;
	#if TARGET_API_MAC_CARBON == 1
	thePort = GetWindowPort(menuwindow);
	#else
	thePort = (CGrafPtr)menuwindow;
	#endif
		
	pushport (thePort);/*who knows what zoomport did to the port?*/
	} 
	
	medisplayscriptbutton (false); /*update it immediately, better visually*/
	
	popport ();
	
	/*windowsetchanges (menuwindow, true); /*the window is dirty*/	
	
	return (true);
	} /*mescriptwindowclosed*/


void mepostcursormove (void) {
	
	/*
	a bundle of functionality that gets executed after the outline cursor moves.
	*/
	
	medisplayscriptbutton (false); /*cursor might have linked text*/
	
	medisplaycmdkeypopup (); /*update dialog fields if there was a change*/
	
	mefreshscriptwindow ();
	
	(**menudata).flcursormoved = false;
	} /*mepostcursormove*/


void meexpandto (hdlheadrecord hnode) {
	
	mecheckglobals ();
	
	opexpandto (hnode);
	
	opvisibarcursor (); /*in case barcursor was already hnode, but not visible*/
	
	mepostcursormove ();
	} /*meexpandto*/


static void menubarchanged () {
	
	/*
	11/26/91 dmb: this functionality used to be implemented in menubar.c, but 
	we need to activate the callback when no menubarstack is connected, in case 
	there's one associated with a shared menu.  the callback is still managed 
	in menubar.c, but this is the only place that it's activated.
	*/
	
	(*menubarcallbacks.menubarchangedroutine) (outlinedata);
	} /*menubarchanged*/


void mesetcmdkey (byte ch, tykeyflags modifiers) { 
	
	/*
	5.0a24 dmb: made public, added modifiers parameter
	*/
	
	register hdlmenurecord hm = menudata;
	register hdlheadrecord hcursor = (**(**hm).menuoutline).hbarcursor;
	tymenuiteminfo item;
	
	megetmenuiteminfo (hcursor, &item);
	
	if (ch == chbackspace) /*same as saying it has no keystroke*/
		ch = chnul;
	
	ch = uppercasechar (ch); /*cmd-keystrokes must be uppercase*/
	
	item.cmdkey = ch;
	
	item.cmdmodifiers = modifiers;
	
	mesetmenuiteminfo (hcursor, &item);
	
	opdirtyoutline ();
	
	medisplaycmdkeypopup ();
	
	menubarchanged ();
	
	memenuitemchanged ((**hm).hmenustack, hcursor);
	} /*mesetcmdkey*/
	
	
static hdlheadrecord *globalcmdkeyarray = nil; /*for building the cmdkey popup menu*/


static boolean mebuildcmdkeypopupvisit (hdlheadrecord hnode, ptrvoid refcon) {
	
	register hdlheadrecord h = hnode;
	register unsigned char ch;
	
	if (!ophasrefcon (h)) /*no cmd-key attached to this line*/
		return (true);
	
	ch = megetcmdkey (h);
	
	if (ch == chnul) /*no cmd-key attached to this line*/
		return (true);
	
	globalcmdkeyarray [ch] = h; 
	
	return (true); /*keep traversing*/
	} /*mebuildcmdkeypopupvisit*/


static boolean mefillcmdkeypopup (hdlmenu hmenu, short *highlighteditem) {
	
	/*
	2/21/91 dmb: added "set command key" item to replace functionality lost 
	by using new-style popup menu.  we could still have a direct entry item 
	next to the popup, but I don't think it's necessary.
	
	10/21/91 dmb: check the item of the cursor cmdkey, but select the dotted line
	*/
	
	register hdloutlinerecord ho = (**menudata).menuoutline;
	register hdlheadrecord hnode;
	register short i;
	register short ixmenu = 0;
	bigstring bs;
	
	hnode = (**ho).hbarcursor;
	
	*highlighteditem = 2;
	
	if (true /*megetcmdkey (hnode) == chnul*/) { /*no cmd-key attached to this line*/
		
		shellgetstring (setcmdkeyitemstring, bs);
		
		pushmenuitem (hmenu, geneva9popupmenuid, bs, 0);
		
		pushdottedlinemenuitem (hmenu);
		
		ixmenu = 2;
		}
	
	for (i = 0; i < sizecmdkeyarray; i++) {
		
		hnode = globalcmdkeyarray [i];
		
		if (hnode != nil) {
			
			opgetheadstring (hnode, bs);
			
			pushmenuitem (hmenu, geneva9popupmenuid, bs, 0);
			
			setmenuitemcommandkey (hmenu, ++ixmenu, (char) i);
			
			if (hnode == (**ho).hbarcursor)
				checkmenuitem (hmenu, ixmenu, true);
				/*
				*highlighteditem = ixmenu;
				*/
			}
		} /*for*/
	
	return (ixmenu > 0); /*true if there are any cmd key assignments*/
	} /*mefillcmdkeypopup*/


static boolean mecmdkeypopupselect (hdlmenu hmenu, short ixmenu) {
	
	register hdlheadrecord hnode;
	short cmdkey;
	bigstring bs;
	
	if (ixmenu == 1) {
		
		cmdkey = megetcmdkey ((**(**menudata).menuoutline).hbarcursor);
		
		shellgetstring (cmdkeypromptstring, bs);
		
		if (chardialog (bs, &cmdkey)) {
		
			opsettextmode (false); /*leave edit mode*/
			
			mesetcmdkey ((byte) cmdkey, keycommand);
			}
		}
	else {
		
		getmenuitemcommandkey (hmenu, ixmenu, &cmdkey);
		
		hnode = globalcmdkeyarray [cmdkey];
		
		if (hnode != nil)
			meexpandto (hnode);
		}
	
	return (true);
	} /*mecmdkeypopupselect*/


static boolean mecmdkeypopup (void) {
	
	hdlheadrecord cmdkeyarray [sizecmdkeyarray];
	Rect r;
	hdlheadrecord hsummit;
	boolean flgeneva9 = false;
	
	#if TARGET_API_MAC_CARBON
		flgeneva9 = true;
	#endif
	
	mecheckglobals ();
	
	clearbytes (&cmdkeyarray, longsizeof (cmdkeyarray));
	
	globalcmdkeyarray = (hdlheadrecord *) &cmdkeyarray; /*callbacks & visit routine use this guy*/
	
	opoutermostsummit (&hsummit);
	
	opsiblingvisiter (hsummit, false, &mebuildcmdkeypopupvisit, nil);
	
	r = (**menudata).cmdkeypopuprect; // dialoggetobjectrect (menuwindow, menucmdkeypopupitem, &r);
	
	popupmenuhit (r, flgeneva9, &mefillcmdkeypopup, &mecmdkeypopupselect);
	
	globalcmdkeyarray = nil; /*keep it nil while it isn't being used*/
	
	return (true);
	} /*mecmdkeypopup*/


static void medisplayuseritems (void) {
	
	medisplaycmdkeypopup (); 
	
//	medisplaycmdkey ();
	
	medisplayscriptbutton (false);
	} /*medisplayuseritems*/
	

static boolean mescriptitemhit (boolean flmousedriven) {
	
	/*
	a special case for itemhit -- since we may be switching to a different window
	the display code has to be different.
	
	7/17/90 DW: we assume the script item hit was done by a human user hitting a 
	mouse button.  we track the mouse, if it's let up outside of the script item 
	we cancel the hit.
	*/
	
	assert (menudata != nil);
	
	if (flmousedriven) {
		
		Rect r;
		
		if (optionkeydown ()) { /*toggle autosmash*/
			
			(**menudata).flautosmash = !(**menudata).flautosmash;
			
			medisplayscriptbutton (false); /*show the autosmash symbol*/
			
			return (true);
			}
		
		r = (**menudata).iconrect; // dialoggetobjectrect (menuwindow, menuscriptitem, &r);
		
		if (!trackicon (r, &medisplayscriptbutton)) /*user released mouse off of button*/
			return (true);
		}
	
	mecheckglobals ();
	
	opsettextmode (false); /*leave edit mode*/
	
	mezoomscriptwindow ();
	
	/*windowsetchanges (menuwindow, true); /*the window is dirty*/
	
	return (true);
	} /*mescriptitemhit*/


boolean memousedown (Point pt, tyclickflags flags) {
	
	/*
	7.0b23 PBS: fix crashing bug on Macs when choosing a right-click menu item.
	Trap for the control key and call oprmousedown, which handles right-clicks.
	*/

	register hdlmenurecord hm;
	
	hm = menudata;

			
	#ifdef MACVERSION /*7.0b23 PBS: check for ctrl-clicking on Macs*/

		if (keyboardstatus.ctmodifiers && keyboardstatus.flcontrolkey) {
			return (oprmousedown (pt, flags)); /*Call right-click routine.*/
		}

	#endif

	if (pointinrect (pt, (**(**hm).menuoutline).outlinerect)) {
		
		mecheckglobals ();
		opmousedown (pt, flags);
		
		if (menudata == 0) return false;
		
		meactivateoutline (); /*trick -- avoid flicker -- activate after op moves cursor*/
		mepostcursormove (); 
		return (true);
		}

	setcursortype (cursorisarrow); /*erase any visual cues attached to cursor*/

	if (pointinrect (pt, (**hm).iconrect))
		return (mescriptitemhit (true));

	if (pointinrect (pt, (**hm).cmdkeypopuprect))
		return (mecmdkeypopup ());

		/*
		case menucmdkeyitem:
			medeactivateoutline ();
			
			(**menudata).menuactiveitem = item;
			
			medisplaycmdkey (); /*show it in its active form%/
			
			break;
		*/

	return (false);
	} /*memousedown*/


boolean mesetprintinfo (void) {
	
	mecheckglobals ();
	
	return (opsetprintinfo ());
	} /*mesetprintinfo*/


boolean meprint (short pagenumber) {
	
	mecheckglobals ();
	
	return (opprint (pagenumber));
	} /*meprint*/


boolean megetundoglobals (long *globals) {
	
	return (opeditgetundoglobals (globals));
	} /*megetundoglobals*/


boolean mesetundoglobals (long globals, boolean flundo) {
	
	mesetglobals ();
	
	return (opeditsetundoglobals (globals, flundo));
	} /*mesetundoglobals*/


/*
merotateactiveitem (boolean flforward) {
	
	/*
	this is the response to a tab key (flforward == true) or a backtab 
	key (flforward == false).  we rotate to the next text item in the
	dialog box.
	
	10/23/90 dmb: now that there are only two items, this is much 
	simpler (no more switches)
	%/
	
	register hdlmenurecord hm = menudata;
	register short activeitem = (**hm).menuactiveitem;
	
	if (activeitem == menuoutlineitem) { /*switch from outline to command keys%/
		
		medeactivateoutline ();
		
		activeitem = menucmdxitem;
		}
	else {
	
		activeitem = menuoutlineitem;
		
		meactivateoutline ();
		}
	
	(**hm).menuactiveitem = activeitem;
} /*merotateactiveitem%/ */

boolean mekeystroke (void) {
	
	register hdlmenurecord hm = menudata;
	register hdloutlinerecord ho = (**hm).menuoutline;
	byte chkb = keyboardstatus.chkb;
	tydirection dir = keyboardstatus.keydirection;
	boolean flcmdkey = keyboardstatus.flcmdkey;
	
	#ifdef WIN95VERSION
		if (keyboardstatus.flcontrolkey)
			flcmdkey = true;
	#endif

	
	if ((chkb == chenter) && flcmdkey) {
		
		return (mescriptitemhit (false));
		}
	
	/*
	if (chkb == chtab) {
	
		merotateactiveitem (!keyboardstatus.flshiftkey);
		
		medisplayuseritems (); /*show it in its active/deactive state%/
		
		return (true);
		}
	*/
	
	switch ((**hm).menuactiveitem) { /*send keystroke to appropriate user item*/
		
		/*
		case menucmdkeyitem:
			if (dir == nodirection) { /*not a motion keystroke%/
				
				switch (chkb) {
					
					case chtab: case chenter: case chescape:
						meactivateoutline ();
						
						medisplaycmdkey (); /*show inactive state%/
						
						break;
					
					default:
						mesetcmdkey (chkb);
					}
				
				return (true);
				}
			
			/*fall through, interpret as an op motion key, cmd-key item stays active%/
		*/
		
		default:
			mecheckglobals ();
			
			opkeystroke ();
			
			mepostcursormove ();
			
			break;
		} /*switch*/
	
	/*
	if (dialogevent (&shellevent, menuwindow, &itemnumber)) {
	
		if ((**hm).menuactiveitem == menucmdkeyitem) 
			mesetcmdkey (); /*copy cmd-key from dialog to the outline%/
		}
	*/
	
	return (true);
	} /*mekeystroke*/


boolean mecmdkeyfilter (char chkb) {
	
	/*
	return false if the cmdkey is consumed, true if not.
	*/
	
	/*
	if ((**menudata).menuactiveitem != menuoutlineitem) /*only outline snags cmdkeys%/
		return (true);
	*/
		
	mecheckglobals ();
	
	if (opcmdkeyfilter (chkb)) /*op.c didn't consume the cmd-key*/
		return (true);
	
	mepostcursormove ();
	
	return (false); /*the cmd-key was consumed*/
	} /*mecmdkeyfilter*/


static boolean metextchangedroutine (hdlheadrecord hnode, bigstring bsorig) {
	
	/*
	a callback routine, linked into the outlinerecord -- it gets called when
	the user has changed the text of one of the headlines.
	*/
	
	menubarchanged ();
	
	return (memenuitemchanged ((**menudata).hmenustack, hnode));
	} /*metextchangedroutine*/


static boolean meinsertlineroutine (hdlheadrecord hnode) {

	/*
	6/20/90 dmb:  a callback routine, called when a headline is inserted into 
	the outline.
	
	7/14/90 DW: work around double-return crash.  you have to hold down the
	option key to get it to update the menubar.
	
	7/27/90 DW: it crashed again -- saved file that crashes in doug.crash.
	
	7/31/90 dmb: bug described above was due to erronious bit calculation in 
	medeletemenu.  no longer have to hold down option key to enable this routine.
	
	10/30/91 dmb: set flcursormoved to keep button, popup up to date
	*/
	
	assert (menudata != nil);
	
	(**menudata).flcursormoved = true;
	
	menubarchanged ();
	
	return (memenuitemadded ((**menudata).hmenustack, hnode));
	} /*meinsertlineroutine*/


static boolean medeletelineroutine (hdlheadrecord hnode) {	
	
	/*
	a callback routine, it's called when the user has deleted a headline.
	
	10/30/91 dmb: call set flcursormoved to keep button, popup up to date
	
	8/17/92 dmb: if hnode is displayed in the script window, close it
	
	5.0a4 dmb: handle case where the scriptnode is subordinate to hnode
	*/
	
	register hdlmenurecord hm = menudata;
	hdlheadrecord scriptnode;
	
	assert (hm != nil);
	
	(**hm).flcursormoved = true;
	
	menubarchanged ();
	
	scriptnode = (**hm).scriptnode;
	
	if (scriptnode != nil)
		if ((hnode == scriptnode) || opsubordinateto (scriptnode, hnode))
			shellclosewindow ((**hm).scriptwindow);
	
	return (memenuitemdeleted ((**hm).hmenustack, hnode));
	} /*medeletelineroutine*/
	

static boolean meicon2click (hdlheadrecord hnode) {
	
	/*
	5.0a25 dmb: no reason not to enable this for menubars
	*/
	
	if (ophassubheads (hnode))
		return (false);
	
	mezoomscriptwindow ();
	
	return (true);
	} /*meicon2click*/


void mesetcallbacks (hdloutlinerecord houtline) {
	
	/*
	5.0a12 dmb: disable horizonal scrolling
	*/

	register hdloutlinerecord ho = houtline;
	
	(**ho).backcolor = whitecolor;
	
	(**ho).pixelsaftericon = 3; /*skip this many pixels between icon and text*/
	
	(**ho).maxlinelen = 255; /*file names can be at most 31 characters long*/
	
	(**ho).fldonterasebackground = true;

	/***(**ho).flusebitmaps = false; /*color bitmaps are slow in menubar windows; don't know why*/
	
	(**ho).setscrollbarsroutine = &mesetscrollbarsroutine;
		
	(**ho).drawlinecallback = &medrawlineroutine;
		
	(**ho).copyrefconcallback = &mecopyrefconroutine;
	
	(**ho).textualizerefconcallback = &metextualizerefconroutine;
	
	/*
	(**ho).printrefconcallback = &meprintrefconroutine;
	*/
	
	(**ho).releaserefconcallback = &mereleaserefconroutine;
	
	(**ho).searchrefconcallback = &mesearchrefconroutine;
	
	(**ho).textchangedcallback = &metextchangedroutine;
	
	(**ho).insertlinecallback = &meinsertlineroutine;
	
	(**ho).deletelinecallback = &medeletelineroutine;
	
	(**ho).haslinkedtextcallback = &mehaslinkedtext;
	
	(**ho).setscrapcallback = &mesetscraproutine;
	
	(**ho).getscrapcallback = &megetscraproutine;

	(**ho).flhorizscrolldisabled = true; /*7.0b26 PBS: was commented out -- now restored.*/
	
	(**ho).icon2clickcallback = &meicon2click;
	} /*mesetcallbacks*/


boolean meeditmenurecord (void) {
	
	/*
	assume that our globals, menudata, menuwindow and menuwindowinfo have been set up.
	
	we prepare the menudata menu record to be edited in menuwindow.
	
	2/26/91 dmb: commented-out movecursorto kluge; it shouldn't be needed any more.
	*/
	
	register hdlmenurecord hm = menudata;
	register hdloutlinerecord ho = (**hm).menuoutline;
	
	mesetglobals ();
	
	meresetwindowrects (menuwindowinfo);

	meresize (); /*set the position of all objects in menu window*/
	
	/*oppophoist (); /*for old, layered menubars*/
	
	opsetdisplaydefaults (ho);
	
	(**ho).flwindowopen = true;
	
	(**ho).flusebitmaps = false;
	
	meactivateoutline (); /*make sure we don't start w/the cmdkey item active*/
	
	return (true);
	} /*meeditmenurecord*/
	
	
boolean menewmenurecord (hdlmenurecord *hmenurecord) {
	
	/*
	4/22/91 dmb: menu outline now points to menu variable
	
	10/22/91 dmb: correctly extract the default font/size for the menubar
	and script outlines from the config resources
	
	5.0b13 dmb: use langexternalgetconfig
	*/
	
	register hdlmenurecord hm;
	hdloutlinerecord houtline = nil;
	register hdloutlinerecord ho = nil;
	tyconfigrecord config;
	
	if (!newclearhandle (longsizeof (tymenurecord), (Handle *) hmenurecord))
		return (false);
	
	hm = *hmenurecord; /*copy into register*/
	
	(**hm).menuactiveitem = menuoutlineitem;
	
	(**hm).scriptwindowrect.top = -1; /*script window uses default rect*/
	
	(**hm).menuwindowrect.top = -1; /*menu window uses default rect*/
	
	if (!newoutlinerecord (&houtline)) 
		goto error;
	
	ho = houtline; /*copy into register*/
	
	(**hm).menuoutline = ho; /*link outline into menu structure*/
	
	(**ho).outlinerefcon = (long) hm; /*pointing is mutual*/
	
	mesetcallbacks (ho); /*link in callback routines*/
	
	bundle { /*set the default font for the menubar outline*/
		
		shellgetconfig (idmenueditorconfig, &config);
		
		(**ho).fontnum = config.defaultfont;
		
		(**ho).fontsize = config.defaultsize;
		}
	
	bundle { /*set the default font for menubar scripts*/
		
		langexternalgetconfig (scriptvaluetype, idscriptconfig, &config);
		
		(**hm).defaultscriptfontnum = config.defaultfont;
		
		(**hm).defaultscriptfontsize = config.defaultsize;
		}
	
	return (true);
	
	error:
	
	disposehandle ((Handle) hm);
	
	opdisposeoutline (ho, false);
	
	return (false);
	} /*menewmenurecord*/


void medisposemenurecord (hdlmenurecord hmenurecord, boolean fldisk) {
	
	register hdlmenurecord hm = hmenurecord;
	
	medisposemenubar ((**hm).hmenustack);
	
	opdisposeoutline ((**hm).menuoutline, fldisk);
	
	if (fldisk) {
	
		dbpushdatabase (megetdatabase (hm));

		dbpushreleasestack ((**hm).adroutline, outlinevaluetype);

		dbpopdatabase ();
		}
	
	disposehandle ((Handle) hm);
	} /*medisposemenurecord*/


/*dmb 10/4/90:  shouldn't need this; handled by releaserefconroutine

boolean medisposevisit (hdlheadrecord hnode) {
	
	register hdloutlinerecord ho;
	tymenuiteminfo item;
	
	megetmenuiteminfo (hnode, &item);
	
	ho = item.linkedscript.houtline;
	
	if (ho != nil)
		opdisposeoutline (ho, false);
	
	return (true);
	} /*medisposevisit%/


boolean medispose (void) {
	
	register hdlmenurecord hm = menudata;
	hdlheadrecord hsummit;
	
	mesetglobals ();
	
	opoutermostsummit (&hsummit);
	
	opsiblingvisiter (hsummit, false, &medisposevisit);
	
	opdisposeoutline (outlinedata, false);
	
	disposehandle ((Handle) hm);
	
	return (true);
	} /*medispose%/
*/


boolean mecut (void) {
	
	mecheckglobals ();
	
	return (opcut ());
	} /*mecut*/
	
	
boolean mecopy (void) {
	
	mecheckglobals ();
	
	return (opcopy ());
	} /*mecopy*/
	
	
boolean mepaste (void) {
	
	mecheckglobals ();
	
	return (oppaste ());
	} /*mepaste*/
	
	
boolean meclear (void) {
	
	mecheckglobals ();
	
	return (opclear ());
	} /*meclear*/
	
	
boolean meselectall (void) {
	
	mecheckglobals ();
	
	return (opselectall ());
	} /*meselectall*/
	
	
boolean meadjustcursor (Point pt) {
	
//	Rect r;
	
//	megetoutlinerect (&r);
	
	if (pointinrect (pt, (**menudata).menuoutlinerect)) {
	
		mecheckglobals ();
		
		return (opsetcursor (pt));
		}
	
//	dialoggetobjectrect (menuwindow, menucmdkeypopupitem, &r);
	
	if (pointinrect (pt, (**menudata).cmdkeypopuprect)) {
		
		setcursortype (cursorispopup);
		
		return (true);
		}
	
//	dialoggetobjectrect (menuwindow, menuscriptitem, &r); /*script button rect*/
	
	if (pointinrect (pt, (**menudata).iconrect)) {
		
		setcursortype (cursorisgo);
		
		return (true);
		}
		
	setcursortype (cursorisarrow);
	
	return (true);
	} /*meadjustcursor*/
	
	
void meidle (void) {
	
	register hdlmenurecord hm = menudata;
	
	mecheckglobals ();
	
	if ((**hm).flcursormoved)
		mepostcursormove ();
	
	opidle ();
	
	/*
	if ((**hm).flzoomscriptwindow) {
		
		(**hm).flzoomscriptwindow = false;
		
		mezoomscriptwindow ();
		}
	*/
	
	pushmenubarglobals ((**hm).hmenustack);
	
	mecheckdeletedmenu (-1, true); /*6/22/90 dmb: flush deleted menu*/
	
	popmenubarglobals ();
	
	if ((**hm).flcursormoved)
		mepostcursormove ();
	
	/*
	if (optionkeydown () && shiftkeydown ()) {
	
		opvalidate ((**hm).menuoutline);
		
		if (cmdkeydown ())
			Debugger ();
		}
	*/
	} /*meidle*/


void meactivate (boolean flactivate) {
	
	register boolean fl = flactivate;
	register hdlmenurecord hm = menudata;
	register hdlmenubarstack hs = (**hm).hmenustack;
	
	/*
	if (fl)
		ccsetmenubar (menudata);
	else
		ccsetmenubar (nil); /*use default menubar%/
	*/
	
	if (!(**hm).flinstalled) { /*not an installed menu; appears only when window is active*/
		
		if (fl) { /*insert, but leave installed flag clear*/
			
			meinstallmenubar (hm);
			
			(**hm).flinstalled = false;
			}
		else {
			(**hm).flactive = false; //so remove will work

			meremovemenubar (hm);
			}
		}
	
	(**hm).flactive = bitboolean (fl);
	
	mecheckglobals ();
	
	if ((**hm).menuactiveitem == menuoutlineitem)
		opactivate (fl);
	} /*meactivate*/


void meupdate (void) {

	Rect r;
	
	if (menudata == nil) // 5.0b10 dmb
		return;

	megetoutlinerect (&r);
	
	pushbackcolor (&(**outlinedata).backcolor);
	
	frame3sides (r);
	
	insetrect (&r, 1, 1);
	
	eraserect (r);
	
	popbackcolor ();
	
	mecheckglobals (); /*get ready to call op*/
	
	opupdate ();
	
	medisplayuseritems ();
	
	#if TARGET_API_MAC_CARBON == 1
	
		shelldrawwindowmessage (shellwindowinfo);
	
	#endif

	} /*meupdate*/


boolean megetcontentsize (long *width, long *height) {
	
	/*
	6.0a13 dmb: use outlinerect, not menurect, to get pixel diff
	*/

	register DialogPtr pdialog = menuwindow;
	Rect rcontent;
	Rect routline;
	short pixels;
	
	if (!opgetoutinesize (width, height))
		return (false);
	
	rcontent = (**menuwindowinfo).contentrect;
	
	routline = (**outlinedata).outlinerect;
	
	pixels = (rcontent.right - rcontent.left) - (routline.right - routline.left);
	
	*width += pixels;
	
	pixels = (rcontent.bottom - rcontent.top) - (routline.bottom - routline.top);
	
	*height += pixels;
	
	return (true);
	} /*megetcontentsize*/


boolean meinstallmenubar (hdlmenurecord hmenurecord) {
	
	/*
	5.0b15 dmb: if we're active, we don't need to insert
	*/
	
	register hdlmenurecord hm = hmenurecord;
	hdlmenubarstack hstack;
	
	hstack = (**hm).hmenustack; /*copy into register*/
	
	if (hstack == nil) { /*no menubar stack in this record*/
		
		if (!menewmenubar ((**hm).menuoutline, &hstack))
			return (false);
		
		(**hm).hmenustack = hstack;
		
		if (!mebuildmenubar (hstack))
			return (false);
		
		(**hstack).refcon = (long) megetdatabase (hm); // 5.1.5 for auto removal on close
		}
	
	if (!(**hm).flactive) //5.0b15
		if (!meinsertmenubar (hstack))
			return (false);
	
	(**hm).flinstalled = true;
	
	return (true);
	} /*meinstallmenubar*/


boolean meremovemenubar (hdlmenurecord hmenurecord) {
	
	/*
	4/22/91 dmb: should we dispose the menubar stack at this point?
	
	5.0b15 dmb: if we're active, we don't shouldn't remove
	*/
	
	register hdlmenurecord hm = hmenurecord;
	hdlmenubarstack hstack;
	
	hstack = (**hm).hmenustack; /*copy into register*/
	
	if (hstack == nil) /*no menubar stack in this record*/
		return (false);
	
	if (!(**hm).flactive) //5.0b15
		if (!medeletemenubar (hstack))
			return (false);
	
	(**hm).flinstalled = false;
	
	return (true);
	} /*meremovemenubar*/


boolean meclearmenubar (void) {

	/*
	7.0b26 PBS: Save, clear, and restore the active menubar.
	This fixes an infinite loop -- because meremovemenubar (quite rightly)
	doesn't remove the active menu. But in this case we need to remove it
	then re-install it.*/

	
	register hdlmenubarlist hlist = menubarlist;
	register hdlmenubarstack hstack;
	register hdlmenurecord hm;
	hdlmenurecord hdlcurrmenu = nil;
	
	if (hlist == nil)
		return (true);
	
	while (true) {
		
		hstack = (**hlist).hfirst;
	
		if (hstack == nil) /*list is empty*/
			return (true);
		
		hm = (hdlmenurecord) (**(**hstack).menubaroutline).outlinerefcon;

		if ((**hm).flactive) { /*7.0b26 PBS: save active menubar*/

			hdlcurrmenu = hm;

			(**hm).flactive = false; /*allow active menubar to be removed.*/
			} /*if*/
		
		meremovemenubar (hm);
		}

	if (hdlcurrmenu != nil) /*7.0b26 PBS: restore the active menubar*/
		meinstallmenubar (hdlcurrmenu);
	} /*meclearmenubar*/


void meinit (void) {
	
	menubarinit ();
	
	menubarcallbacks.getcmdkeyroutine = &megetcmdkey;
	} /*meinit*/




