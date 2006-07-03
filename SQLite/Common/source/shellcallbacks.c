
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

#include "memory.h"
#include "frontierconfig.h"
#include "cursor.h"
#include "font.h"
#include "kb.h"
#include "ops.h"
#include "quickdraw.h"
#include "resources.h"
#include "strings.h"
#include "frontierwindows.h"
#include "shell.h"
#include "shellprivate.h"
#include "shellbuttons.h"
#include "shellundo.h"
#include "shell.rsrc.h"
#include "oplist.h"
#include "lang.h"
#include "process.h"



tyshellglobals globalsarray [cteditors];

short topglobalsarray = 0; /*initially empty*/


tyglobalsstack globalsstack; /*for push/pop globals*/



boolean shellnewcallbacks (ptrcallbacks *callbacks) {
	
	if (topglobalsarray == cteditors) { 
		
		shellinternalerror (ideditorstackfull, STR_increase_cteditors);
		
		*callbacks = nil;
		
		return (false);
		}
	
	*callbacks = &globalsarray [topglobalsarray++];
		
	return (true);
	} /*shellnewcallbacks*/


boolean shellfindcallbacks (short configresnum, short *ixarray) {
	
	/*
	search the callbacks array for a record whose resource number matches the
	indicated resnum.  return the index into the array of the record if found,
	return false if not found.
	*/
	
	register short resnum = configresnum;
	register short i;
	
	for (i = 0; i < topglobalsarray; i++) {
		
		if (globalsarray [i].configresnum == resnum) {
		
			*ixarray = i;
			
			return (true);
			}
		} /*for*/
		
	return (false); /*loop terminated, not found*/
	} /*shellfindcallbacks*/
	

static boolean shelldefaultundo (void) {

	/*
	the default undo routine, this is installed instead of noop if nothing 
	else has been installed by the handler.  redo if available; else undo.
	*/

	short ixaction;

	if (!optionkeydown ()  &&  getredoaction (&ixaction))
		return (redolastaction (true));
	else
		return (undolastaction (true));
	} /*shelldefaultundo*/


static boolean shelldefaultsetundostatus (void) {
	
	register hdlstring hstring;
	short ixbase = 0, ixaction;
	bigstring bsundo, bs;

	hstring = (**shellwindowinfo).hundostring;

	disposehandle ((Handle) hstring);

	hstring = nil; /*indicates no undo possible*/

	/* try to get redo first, else try to get undo */

	if (!optionkeydown ()  &&  getredoaction (&ixaction))
		ixbase = redostring;
	else {
		if (getundoaction (&ixaction))
			ixbase = undostring;
		}

	if (ixaction > 0) { /*it's an undoable operation*/
	
		if (getstringlist (undolistnumber, ixbase, bsundo)  &&
			getstringlist (undolistnumber, ixaction, bs)) {
			
			hdlstring h;
			
			pushstring (bs, bsundo);

			if (newheapstring (bsundo, &h))
				hstring = h;
			}
		}
	
	(**shellwindowinfo).hundostring = hstring;
	
	return (true);
	} /*shelldefaultsetundostatus*/


static boolean shelldefaultbuttonstatusroutine (short buttonnum, tybuttonstatus *status) {
	
	(*status).fldisplay = true;
	
	(*status).flenabled = true;
	
	(*status).flbold = (buttonnum == (**shellwindowinfo).lastbuttonhit);
	
	return (true);
	} /*shelldefaultbuttonstatusroutine*/


boolean shelldefaultfontroutine (void) {
	
	/*
	if the handler doesn't define a font routine, wire it into us.  we just
	set the default font for the window and force a complete update.
	*/
	
	register hdlwindowinfo hw = shellwindowinfo;
	
	(**hw).defaultfont = (**hw).selectioninfo.fontnum;
	
	(**hw).selectioninfo.fldirty = true;
	
	eraserect ((**hw).contentrect);
	
	windowinval (shellwindow);
	
	return (true);
	} /*shelldefaultfontroutine*/
	

boolean shelldefaultsizeroutine (void) {
	
	/*
	if the handler doesn't define a size routine, wire it into us.  we just
	set the default size for the window and force a complete update.
	*/
	
	register hdlwindowinfo hw = shellwindowinfo;
	
	(**hw).defaultsize = (**hw).selectioninfo.fontsize;
	
	(**hw).selectioninfo.fldirty = true;
	
	eraserect ((**hw).contentrect);
	
	windowinval (shellwindow);
	
	return (true);
	} /*shelldefaultsizeroutine*/


#if 0

static boolean shelldefaultstyleroutine (void) {
	
	/*
	if the handler doesn't define a style routine, wire it into us.  we just
	set the default style for the window and force a complete update.
	*/
	
	register hdlwindowinfo hw = shellwindowinfo;
	
	shellsetdefaultstyle (hw); /*sets the defaultstyle field based on selectioninfo*/
	
	(**hw).selectioninfo.fldirty = true;
	
	eraserect ((**hw).contentrect);
	
	windowinval (shellwindow);
	
	return (true);
	} /*shelldefaultstyleroutine*/

#endif


boolean shelldefaultselectioninfo (void) {

	return (defaultselectioninfo (shellwindowinfo));
	} /*shelldefaultselectioninfo*/


static boolean shelldefaultgettargetdata (short id) {
	
	return (false); /*dmb 4/5/91: only EFP windows can be targets now*/
	
	return (id == -1); /*true if target type is generic -- a shell verb*/
	} /*shelldefaultgettargetdata*/


static boolean shelldefaultmsgroutine (bigstring bs, boolean flbackgroundmsg) {
#pragma unused (flbackgroundmsg)

	shellsetwindowmessage (shellwindowinfo, bs);
	
	drawwindowmessage (shellwindow);
	
	return (true);
	} /*shelldefaultmsgroutine*/


static boolean shelldefaultresetrectsroutine (hdlwindowinfo hinfo) {
	
	shellresetwindowrects (hinfo);
	
	return (true);
	} /*shelldefaultresetrectsroutine*/


static void shelldefaultadjustcursor (void) {
	
	setcursortype (cursorisarrow);
	} /*shelldefaultadjustcursor*/
	
	
static void shellchecknilroutine (callback *routine, callback defaultroutine) {
	
	if (*routine == nil)
	
		*routine = defaultroutine;
	} /*shellchecknilroutine*/


static void shelltruedefault (void *routine) {
	
	shellchecknilroutine ((callback *) routine, &truenoop);
	} /*shelltruedefault*/


static void shellfalsedefault (void *routine) {
	
	shellchecknilroutine ((callback *) routine, &falsenoop);
	} /*shellfalsedefault*/


void shellpatchnilroutines (void) {
	
	/*
	call this when the callback routines are fully installed.  we check for any
	that the application might have left unfilled, and patch them with a routine
	that does nothing (truenoop).  this allows a set of handlers to catch
	certain events and do nothing on other types.
	*/
	
	short i;
	
	for (i = 0; i < topglobalsarray; i++) {
		
		register tyshellglobals *cb = &globalsarray [i];
		
		shelltruedefault (&(*cb).initroutine);
		
		shelltruedefault (&(*cb).quitroutine);
		
		shelltruedefault (&(*cb).setglobalsroutine);
		
		shelltruedefault (&(*cb).pushroutine);
		
		shelltruedefault (&(*cb).poproutine);
		
		shelltruedefault (&(*cb).setsuperglobalsroutine);
		
		shelltruedefault (&(*cb).newrecordroutine);
		
		shelltruedefault (&(*cb).updateroutine);
		
		shelltruedefault (&(*cb).activateroutine);
		
		shelltruedefault (&(*cb).resumeroutine);
		
		shellfalsedefault (&(*cb).getcontentsizeroutine);
		
		shelltruedefault (&(*cb).resizeroutine);
		
		shellchecknilroutine ((callback *) &(*cb).resetrectsroutine, (callback) &shelldefaultresetrectsroutine);
		
		shellfalsedefault (&(*cb).scrollroutine);
		
		shelltruedefault (&(*cb).setscrollbarroutine);
		
		shelltruedefault (&(*cb).mouseroutine);

		shelltruedefault (&(*cb).rmouseroutine);

		shelltruedefault (&(*cb).cmouseroutine);

		shelltruedefault (&(*cb).wmouseroutine);
		
		shelltruedefault (&(*cb).buttonroutine);
		
		shellchecknilroutine ((callback *) &(*cb).buttonstatusroutine, (callback) &shelldefaultbuttonstatusroutine);
		
		shelltruedefault (&(*cb).presaveroutine);
		
		shelltruedefault (&(*cb).keystrokeroutine);
		
		shelltruedefault (&(*cb).cmdkeyfilterroutine);
		
		shellfalsedefault (&(*cb).titleclickroutine);
		
		shellchecknilroutine (&(*cb).undoroutine, shelldefaultundo);
		
		shellchecknilroutine (&(*cb).setundostatusroutine, &shelldefaultsetundostatus);
		
		shellfalsedefault (&(*cb).getundoglobalsroutine);
		
		shelltruedefault (&(*cb).setundoglobalsroutine);
		
		shelltruedefault (&(*cb).cutroutine);
		
		shelltruedefault (&(*cb).copyroutine);
		
		shelltruedefault (&(*cb).pasteroutine);
		
		shelltruedefault (&(*cb).clearroutine);
		
		shelltruedefault (&(*cb).selectallroutine);
		
		shellchecknilroutine (&(*cb).fontroutine, &shelldefaultfontroutine);
		
		shellchecknilroutine (&(*cb).sizeroutine, &shelldefaultsizeroutine);
		
		shellfalsedefault (&(*cb).styleroutine);
		
		shellchecknilroutine (&(*cb).setselectioninforoutine, &shelldefaultselectioninfo);
		
		shelltruedefault (&(*cb).leadingroutine);
		
		shelltruedefault (&(*cb).justifyroutine);
		
		shellfalsedefault (&(*cb).searchroutine);
		
		shellfalsedefault (&(*cb).replaceroutine);
		
		shellfalsedefault (&(*cb).executeroutine);
		
		shelltruedefault (&(*cb).disposerecordroutine);
		
		shelltruedefault (&(*cb).saveroutine);
		
		shelltruedefault (&(*cb).pagesetuproutine);
		
		shelltruedefault (&(*cb).beginprintroutine);
		
		shelltruedefault (&(*cb).endprintroutine);
		
		shelltruedefault (&(*cb).printroutine);
		
		shelltruedefault (&(*cb).setprintinfoproutine);
		
		shelltruedefault (&(*cb).setdatabaseroutine);
		
		shellfalsedefault ((callback *) &(*cb).getdatabaseroutine);
		
		shelltruedefault (&(*cb).fnumchangedroutine);
		
		shelltruedefault (&(*cb).loadroutine);
		
		shellfalsedefault (&(*cb).loadspecialroutine);
		
		shelltruedefault (&(*cb).precloseroutine);	/*4.1b5 dmb*/
		
		shelltruedefault (&(*cb).closeroutine);
		
		shelltruedefault (&(*cb).childcloseroutine);
		
		shelltruedefault (&(*cb).idleroutine);
		
		shelltruedefault (&(*cb).backgroundroutine);
		
		shellchecknilroutine ((callback *) &(*cb).gettargetdataroutine, (callback) &shelldefaultgettargetdata);
		
		shellfalsedefault (&(*cb).getvariableroutine);
		
		shelltruedefault (&(*cb).findusedblocksroutine);
		
		shellfalsedefault (&(*cb).settextmoderoutine);
		
		shellchecknilroutine ((callback *) &(*cb).zoomwindowroutine, (callback) shellzoomwindow);
		
		shellchecknilroutine ((callback *) &(*cb).msgroutine, (callback) &shelldefaultmsgroutine);
		
		shellchecknilroutine ((callback *) &(*cb).adjustcursorroutine, (callback) &shelldefaultadjustcursor);
		} /*for*/
	} /*shellpatchnilroutines*/


void shellinithandlers (void) { 
	
	/*
	call the initroutine callback for each handler.
	*/

	short i;
	
	for (i = 0; i < topglobalsarray; i++) {
		
		config = globalsarray [i].config; /*handler may want to refer to this*/
		
		(*globalsarray [i].initroutine) ();
		} /*for*/
	} /*shellinithandlers*/
	

void shellloadbuttonlists (void) {
	
	register short i;
	register short resnum;
	
	for (i = 0; i < topglobalsarray; i++) {
		
		resnum = globalsarray [i].config.idbuttonstringlist; 
		
		if (resnum != 0) { /*a button list is attached*/
			
			if (!oploadstringlist (resnum, (hdllistrecord *) &globalsarray [i].buttonlist))
				shellinternalerror (idbuttonlistmissing, STR_failed_to_load_a_button_list);
			}
		} /*for*/
	} /*shellloadbuttonlists*/
	
		
boolean shellgetconfig (short resnum, tyconfigrecord *pconfig) {
	
	short ix = 0;
	
	if (!shellfindcallbacks (resnum, &ix))
		return (false);
		
	*pconfig = globalsarray [ix].config;
	
	return (true);
	} /*shellgetconfig*/
	
	
boolean shellsetconfig (short resnum, tyconfigrecord pconfig) {
	
	short ix = 0;
	
	if (!shellfindcallbacks (resnum, &ix))
		return (false);
		
	globalsarray [ix].config = pconfig;
	
	return (true);
	} /*shellsetconfig*/
	
	
boolean shellsetglobals (WindowPtr wglobals) {
	
	/*
	set up globals for the window.
	
	8/25/90 DW: when we're closing all the windows, we pop the globals stack
	but there are no windows open to set globals to.  we put a defensive driving
	check in at the lowest level -- it can do double-duty protecting against
	similar problems that might crop up elsewhere.  the call to see if the window
	list is empty is very, very cheap.
	*/
	
	register WindowPtr w = wglobals;
	hdlwindowinfo hinfo;
	register hdlwindowinfo hw;
	short ix = 0;
	
	if (!wglobals || emptywindowlist ()) { /*defensive driving, avoid a crash, it CAN happen*/
		
		shellwindow = nil;
		
		shellwindowinfo = nil;
		
		shellundostack = nil; /*7/21/91 dmb*/
		
#ifdef WIN95VERSION
		setport (NULL); // must clear port from prev globals
#endif

		return (false);
		}
	
	if (!getwindowinfo (w, &hinfo)) /*chase the refcon field of the mac window*/
		return (false);
	
	hw = hinfo; /*copy into register*/
	
	shellwindow = w; /*a global, this is the window whose globals are current*/
	
	shellwindowinfo = hw; /*the windowinfo record whose globals are current*/
	
	shellundostack = (hdlundostack) (**hw).hundostack;
	
	shellredostack = (hdlundostack) (**hw).hredostack;
	
	shellfindcallbacks ((**hw).configresnum, &ix);
	
	shellglobals = globalsarray [ix];
	
	config = shellglobals.config;
	
	*shellglobals.windowholder = w;
	
	*shellglobals.infoholder = hw;
	
	*shellglobals.dataholder = (**hw).hdata;
	
	(*shellglobals.setglobalsroutine) ();
	
	//You must call with a GrafPtr or CGrafPtr.
	//Code change by Timothy Paustian Wednesday, August 23, 2000 9:08:56 PM
	//
	{
	#if TARGET_API_MAC_CARBON
	CGrafPtr thePort = GetWindowPort(w);
 	#else
 	GrafPtr thePort = (GrafPtr)w;
 	#endif
	if (getport () != thePort) /*make sure w is current port; a courtesy to the content handler*/
		setport (thePort);
	}

	return (true);
	} /*shellsetglobals*/
	
/*
static stacktracer (short stacktop) {
	
	bigstring bs;
	hdlwindowinfo hinfo;
	Rect r;
	WindowPtr w;
	
	if (!frontrootwindow (&hinfo)) /%no windows open%/
		return;
	
	shorttostring (stacktop, bs);
	 
	w = (**hinfo).macwindow;
	
	pushport (w);
	
	pushstyle (geneva, 9, 0);
	
	r = (*w).portRect;
	
	r.top = r.bottom - (globalfontinfo.ascent + globalfontinfo.descent); 
	
	r.left += 1; 
	
	r.right = r.left + 12;
	
	eraserect (r);
	
	movepento (r.left, r.bottom - 1);
	
	pendrawstring (bs);
	
	popstyle ();
	
	popport ();
	} /%stacktracer%/
*/
	
boolean shellpushglobals (WindowPtr wpush) {
	
	/*
	8/31/90 DW: if the caller is pushing a nil window, just save off the state
	and don't set the context.  this allows you to save the global data, mess
	with some of the globals, and then pop yourself back to the way things were.
	*/
	
	hdlwindowinfo hinfo;
	
	if (globalsstack.top >= ctglobals) {
		
		shellinternalerror (idglobalsstackfull, STR_globals_stack_overflow);
		
		return (false);
		}
	
	globalsstack.stack [globalsstack.top++] = shellwindow;
	
	/*stacktracer (globalsstack.top);*/
	
	/*
	if (wpush == nil) /%caller just wants state saved%/ {
		
		shellwindow = nil;
		
		return (true);
		}
	
	return (shellsetglobals (wpush));
	*/
	
	if (getwindowinfo (wpush, &hinfo))
		(**hinfo).ctpushes++;
	
	if (shellsetglobals (wpush))
		(*shellglobals.pushroutine) ();
	
	return (true);
	} /*shellpushglobals*/


boolean shellpopglobals (void) {
	
	WindowPtr w = shellwindow;
	hdlwindowinfo hinfo;
	
	if (globalsstack.top <= 0) {
	
		shellsetglobals (nil);
		
		return (false);
		}
	
	if (shellwindow != nil) { /*there are some globals currently set - set data to nil*/
		
		(*shellglobals.poproutine) ();
		
		*shellglobals.dataholder = nil;
		
		*shellglobals.infoholder = nil;
	
		(*shellglobals.setglobalsroutine) ();
		}
	
	shellsetglobals (globalsstack.stack [--globalsstack.top]);
	
	if (getwindowinfo (w, &hinfo)) {
		
		(**hinfo).ctpushes--;
		
		if ((**hinfo).fldisposewhenpopped)
			disposeshellwindow ((**hinfo).macwindow);
		}
	
	return (true);
	} /*shellpopglobals*/


boolean shellpushwindowglobals (hdlwindowinfo hpush) {
	
	/*
	5.0a5 dmb: we do this often enough, it deserves its own routine
	*/

	if (hpush == nil)
		return (false);

	return (shellpushglobals ((**hpush).macwindow));
	} /*shellpushwindowglobals*/


/*
void shellinvalidglobals (WindowPtr winvalid) {
	
	/%
	the indicated windowptr is no longer valid -- it was presumably 
	just disposed.  make sure that we'll never try to push its globals
	%/
	
	register WindowPtr w = winvalid;
	register short ix;
	
	for (ix = 0; ix < globalsstack.top; ++ix) {
		
		if (globalsstack.stack [ix] == w)
			globalsstack.stack [ix] = nil;
		}
	
	if (w == shellwindow)
		shellwindow = nil;
	
	processinvalidglobals (w); //every thread needs to know about the change*
	} /%shellinvalidglobals%/
*/


boolean shellpushconfigglobals (short configresnum) {
	
	short ix;
	
	if (!shellfindcallbacks (configresnum, &ix))
		return (false);
	
	if (!shellpushglobals (nil)) /*save off old state*/
		return (false);
		
	shellglobals = globalsarray [ix];
	
	config = shellglobals.config;
	
	return (true);
	} /*shellpushconfigglobals*/
	
	
boolean shellpushdefaultglobals (void) {
	
	return (shellpushconfigglobals (iddefaultconfig));
	} /*shellpushdefaultglobals*/
	
	
boolean shellpushfrontglobals (void) {
	
	WindowPtr w;
	
	if (!frontshellwindow (&w))
		return (false);
	
	return (shellpushglobals (w));
	} /*shellpushfrontglobals*/


boolean shellpushrootglobals (WindowPtr w) {
	
	/*
	set the globals with those of the root window of w 
	*/
	
	hdlwindowinfo hinfo;
	
	if (!getrootwindow (w, &hinfo)) /*save only operates on root windows*/
		return (false);
	
	return (shellpushglobals ((**hinfo).macwindow));
	} /*shellpushrootglobals*/


boolean shellpushparentglobals (void) {
	
	hdlwindowinfo hinfo, hparentinfo;
	
	if (!getwindowinfo (shellwindow, &hinfo))
		return (false);
		
	hparentinfo = (**hinfo).parentwindow;
	
	if (hparentinfo == nil)
		return (false);
		
	return (shellpushglobals ((**hparentinfo).macwindow));
	} /*shellpushparentglobals*/


boolean shellgetfrontrootinfo (hdlwindowinfo *hinfo) {
	
	/*
	11/1/91 dmb: make sure we've got a real root, not a guest root
	*/
	
	WindowPtr w;
	Handle hdata;
	
	if (!frontrootwindow (hinfo) || ((***hinfo).configresnum != iddefaultconfig)) {
		
		if (!shellfindwindow (iddefaultconfig, &w, hinfo, &hdata))
			return (false);
		
		getrootwindow (w, hinfo);
		}
	
	return (true);
	} /*shellgetfrontrootinfo*/


boolean shellpushfrontrootglobals (void) {
	
	/*
	11/1/91 dmb: make sure we've got a real root, not a guest root
	*/
	
	hdlwindowinfo hinfo;
	
	if (!shellgetfrontrootinfo (&hinfo))
		return (false);
	
	return (shellpushglobals ((**hinfo).macwindow));
	} /*shellpushfrontrootglobals*/


boolean shellsetsuperglobals (void) {
	
	/*
	the "super" globals are a handle linked into the data record of a root
	window.  we don't know how or exactly where these globals are stored,
	but we do know who to ask -- the frontmost window of type iddefaultconfig.
	
	dmb: try the root window of the front window first.  only traverse the 
	window list for a default window if the front's root isn't the right type.
	why?  because the front window's root may not be the frontmost root window.
	*/
	
	hdlwindowinfo hinfo;
	boolean fl;
	
	if (!shellgetfrontrootinfo (&hinfo))
		return (false);
	
	shellpushglobals ((**hinfo).macwindow);
	
	fl = (*shellglobals.setsuperglobalsroutine) ();
	
	shellpopglobals ();
	
	return (fl);
	} /*shellsetsuperglobals*/


boolean shellgetundoglobals (long *globaldata) {

	return ((*shellglobals.getundoglobalsroutine) (globaldata));
	} /*shellgetundoglobals*/


boolean shellsetundoglobals (long globaldata, boolean flundo) {

	return ((*shellglobals.setundoglobalsroutine) (globaldata, flundo));
	} /*shellsetundoglobals*/


void shellclearwindowdata (void) {
	
	/*
	5.0b10 dmb: why this routine hasn't existed before, I don't know.
	Under windows, we really get bitten when any of these steps is omitted.
	There's no reason that the dispose callback should have to nil the 
	globals

	5.1.5b7 dmb: don't clear the actual dataholder global. Otherwise, 
	oppopglobals called from shellpopglobals doesn't know what it's popping
	*/
	
	//*shellglobals.dataholder = nil;
	
	(**shellwindowinfo).hdata = nil;
	} /*shellclearwindowdata*/

