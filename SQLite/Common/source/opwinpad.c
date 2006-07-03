
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

#ifdef MACVERSION 
	#include <standard.h>
#endif

#ifdef WIN95VERSION 
	#include "standard.h"
#endif

#include "config.h"
#include "dialogs.h"
#include "memory.h"
#include "strings.h"
#include "quickdraw.h"
#include "font.h"
#include "cursor.h"
#include "file.h"
#include "ops.h"
#include "resources.h"
#include "search.h"
#include "shell.h"
#include "shellhooks.h"
#include "shellundo.h"
#include "lang.h"
#include "langexternal.h"
#include "langinternal.h"
#include "tablestructure.h"
#include "process.h"
#include "op.h"
#include "opinternal.h"
#include "opverbs.h"
#include "wpengine.h"
#include "scripts.h"
#include "kernelverbdefs.h"
#if MACVERSION
	#include "osacomponent.h"
#else
	#define havecomponentmanager() (false)
#endif



#define opstringlist 159
#define optypestring 1
#define scripttypestring 2
#define opsizestring 3

#define operrorlist 259
#define noooutlineerror 1
#define internalerror 2
#define namenotoutlineerror 3
#define namenotscripterror 4
#define rejectmenubarnum 5



enum {
	runbutton = 1,
	
	debugbutton,
	
	recordbutton,
	
	stepbutton,
	
	inbutton,
	
	outbutton,
	
	followbutton,
	
	gobutton,
	
	stopbutton,
	
	killbutton,
	
	localsbutton,
	
	installbutton
	};



#define maxchainedlocals 80 /*limit for local table nesting in debugged process*/

#define maxnestedsources 40 /*local handlers count as nested sources*/



typedef struct tysourcerecord {
	
	WindowPtr pwindow; /*window containing running script; nil if closed or unknown*/
	
	hdlhashnode hnode; /*hash node containing script variable*/
	
	hdloutlinerecord houtline; /*script outline itself*/
	} tysourcerecord;


typedef struct tydebuggerrecord {
	
	hdlprocessrecord scriptprocess;
	
	tysourcerecord scriptsourcestack [maxnestedsources]; /*for stepping other scripts*/
	
	short topscriptsource; /*source stack pointer*/
	
	short sourcesteplevel; /*level in source stack at which we suspend after step/in/out*/
	
	tydirection stepdir; /*left,right,down & flatdown for out,in,step & trace*/
	
	short lastlnum; /*the last line number stepped onto*/
	
	hdlhashtable hlocaltable; /*the most local table in our process*/
	
	hdlhashnode localtablestack [maxchainedlocals]; /*nodes of tables in the runtimestack*/
	
	/***Handle localtableformatsstack [maxchainedlocals]; /*nodes of tables in the runtimestack*/
	
	short toplocaltable; /*table stack pointer*/
	
	/*
	short deferredbuttonnum; /*see scriptdeferbutton%/
	
	WindowPtr deferredwindow; /*ditto%/
	*/
	
	hdlheadrecord hbarcursor; /*the headline we've most recently shown*/
	
#ifdef flcomponent
	ComponentInstance servercomp; /*component that we have open for this script*/

	OSType servertype; /*the component's type*/
	
	OSAID idscript; /*while recording, this is the id of the script*/
#endif
	
	short lastindent;
	
	boolean flscriptsuspended: 1; /*user is debugging a script*/
	
	boolean flstepping: 1; /*single-stepping thru scripts*/
	
	boolean flfollowing: 1; /*does the bar cursor follow the interpreter?*/
	
	boolean flscriptkilled: 1; /*true if the script has been killed*/
	
	boolean flscriptrunning: 1; /*true when the script is running*/
	
	boolean flwindowclosed: 1; /*false if script's window is open*/
	
	boolean fllangerror: 1; /*are we stopped with an error?*/
	
	boolean flrecording: 1; /*are we recording into this script?*/
	
	boolean flcontextlocked: 1; /*is out context being operated on externally?*/
	
	long scriptrefcon; /*copied from outline refcon*/
	} tydebuggerrecord, *ptrdebuggerrecord, **hdldebuggerrecord;


static hdldebuggerrecord debuggerdata = nil;


static boolean scriptinruntimestack (void) {
	
	/*
	is the current script part of the current process?
	
	6/24/92 dmb: compare outlines in loop, not windows; windows may not yet 
	be assigned into stack (if opened manaually or by Error Info)
	*/
	
	register hdldebuggerrecord hd = debuggerdata;
	register short ix;
	
	for (ix = 0; ix < (**hd).topscriptsource; ix++) { /*visit every window in stack*/
		
		/*
		if (outlinewindow == (**hd).scriptsourcestack [ix].pwindow) {
		*/
		
		if (outlinedata == (**hd).scriptsourcestack [ix].houtline) {
			
			if (ix == 0) /*main source window.  make sure same script is displayed*/
			
				return ((**hd).scriptrefcon == (**outlinedata).outlinerefcon);
			
			return (true);
			}
		}
	
	return (false);
	} /*scriptinruntimestack*/


static void scriptkillbutton (void) {
	
	/*
	11/5/91 dmb: use new scriptprocess field to make kill more explicit
	*/
	
	register hdldebuggerrecord hd = debuggerdata;
	
//	processkill ((**hd).scriptprocess);
	
	(**hd).flscriptkilled = true;
	
	(**hd).flscriptsuspended = false; /*allow script to resume, so it can die*/
	} /*scriptkillbutton*/


static boolean scriptnewprocess (short buttonnum) {

	Handle hlangtext = nil;
	bigstring bsresult;

	if (!opgetlangtext (outlinedata, false, &hlangtext))
		return (false);
	
	langrunhandle (hlangtext, bsresult);
	
	alertdialog (bsresult);
	
	return (true);
	} /*scriptnewprocess*/


static boolean scriptbutton (short buttonnum) {
	
	register hdldebuggerrecord hd = debuggerdata;
	
	if (buttonnum != localsbutton) /*exit edit mode on any button hit except locals*/
		opsettextmode (false);
	
	switch (buttonnum) {
		
		case runbutton:
			if (scriptnewprocess (runbutton))
				shellupdatenow (outlinewindow);
			
			return (true);
			
		case killbutton:
			scriptkillbutton ();
			
			return (true);
		} /*switch*/
	
	return (true);
	} /*scriptbutton*/


static boolean scriptbuttonenabled (short buttonnum) {
	
	register short x = buttonnum;
	register hdldebuggerrecord hd = debuggerdata;
	register boolean flscriptrunning = (**hd).flscriptrunning;
	register boolean flscriptsuspended = (**hd).flscriptsuspended;
	register boolean flrunningthisscript;
	
	if (outlinedata == NULL)
		return (false);

	flrunningthisscript = flscriptrunning && scriptinruntimestack ();
	
	if (flscriptrunning) {
		
		if ((x != installbutton) && (x != killbutton)) /*rule 3*/
			if (!flrunningthisscript)
				return (false);
		}
	
	if ((**hd).flrecording)
		return (x == stopbutton);
	
	if ((**hd).flcontextlocked)
		return (false);
	
	/*
	else {
		
		if ((x != runbutton) && (x != debugbutton) && (x != installbutton)) /*rule 1%/
			return (false);
		}
	*/
	
	switch (x) {
		
		case runbutton:
			return (!flscriptrunning);
		
		case debugbutton:
			return (!flscriptrunning && ((**outlinedata).outlinesignature == typeLAND));
		
		case stopbutton:
			return (!flscriptsuspended);
		
		case gobutton:
			return ((flscriptsuspended || (**hd).flfollowing) && !(**hd).fllangerror);
		
		case followbutton:
		case stepbutton:
		case inbutton:
		case outbutton:
			return (flscriptrunning && flscriptsuspended && !(**hd).fllangerror);
		
		case localsbutton:
			return (flscriptrunning && flscriptsuspended); /*(**hd).flstepping && (!(**hd).flfollowing))*/
		
		case killbutton:
			return (flscriptrunning);
		} /*switch*/
	
	return (true);
	} /*scriptbuttonenabled*/


static boolean scriptbuttondisplayed (short buttonnum) {
	
	register hdldebuggerrecord hd = debuggerdata;
	register hdlprocessrecord hp = (**hd).scriptprocess;
	register boolean flscriptrunning = (**hd).flscriptrunning;
	register boolean flrunningthisscript;
	register boolean fldebuggingthisscript;
	register short x = buttonnum;
	
	flrunningthisscript = flscriptrunning && scriptinruntimestack (); /*1/2/91*/
	
	fldebuggingthisscript = flrunningthisscript && (**hp).fldebugging;
	
	if ((**hd).flrecording)
		return ((x == recordbutton) || (x == stopbutton));
	
	switch (x) {
		
		case recordbutton:
		case runbutton:
		case debugbutton:
			return (!flscriptrunning);
		
		case stopbutton:
			return (fldebuggingthisscript && !(**hd).flscriptsuspended);
		
		case gobutton:
			return (fldebuggingthisscript && (**hd).flscriptsuspended);
		
		case followbutton:
		case stepbutton:
		case inbutton:
		case outbutton:
		case localsbutton:
			return (fldebuggingthisscript);
		
		case killbutton:
			return (flscriptrunning);
		} /*switch*/
	
	return (true);
	} /*scriptbuttondisplayed*/


static boolean scriptbuttonstatus (short buttonnum, tybuttonstatus *status) {
	
	(*status).flenabled = scriptbuttonenabled (buttonnum);
	
	(*status).fldisplay = scriptbuttondisplayed (buttonnum);
	
	(*status).flbold = false; /*our buttons are never bold*/
	
	return (true);
	} /*scriptbuttonstatus*/


static boolean opverbsetscrollbarsroutine (void) {

	register ptrwindowinfo pw = *outlinewindowinfo;
	register ptroutlinerecord po = *outlinedata;
	
	(*pw).vertscrollinfo = (*po).vertscrollinfo;
	
	(*pw).horizscrollinfo = (*po).horizscrollinfo;
	
	(*pw).fldirtyscrollbars = true; /*force a refresh of scrollbars by the shell*/
	
	return (true);
	} /*opverbsetscrollbarsroutine*/


static void opverbsetcallbacks (hdloutlinerecord houtline) {
	
	register hdloutlinerecord ho = houtline;
	
	(**ho).setscrollbarsroutine = &opverbsetscrollbarsroutine;
	
	/*link in the line layout callbacks*/ /*{
	
		(**ho).drawlinecallback = &claydrawline;
		
		(**ho).postdrawlinecallback = &claypostdrawline;
		
		(**ho).predrawlinecallback = &claypredrawline;
		
		(**ho).drawiconcallback = &claydrawnodeicon;
		
		(**ho).gettextrectcallback = &claygettextrect;
		
		(**ho).geticonrectcallback = &claygeticonrect;
		
		(**ho).getlineheightcallback = &claygetlineheight;
		
		(**ho).getlinewidthcallback = &claygetlinewidth;
		
		(**ho).pushstylecallback = &claypushnodestyle;
		
		(**ho).postfontchangecallback = (opvoidcallback) &claybrowserinitdraw;
		
		(**ho).getfullrectcallback = (opgefullrectcallback) &claygetnodeframe;
		}*/
	
	} /*opverbsetcallbacks*/


static void opverbcheckwindowrect (hdloutlinerecord houtline) {
	
	register hdloutlinerecord ho = houtline;
	hdlwindowinfo hinfo = outlinewindowinfo;
		
	if ((**ho).flwindowopen) { /*make windowrect reflect current window size & position*/
	
		Rect r;
		
		shellgetglobalwindowrect (hinfo, &r);
		
		if (!equalrects (r, (**ho).windowrect)) {
		
			(**ho).windowrect = r;
			
			(**ho).fldirty = true;
			}
		}
	} /*opverbcheckwindowrect*/


static void opverbresize (void) {
	
	opresize ((**outlinewindowinfo).contentrect);
	} /*opverbresize*/


boolean opverbclose (void) {
	
	register hdloutlinerecord ho = outlinedata;
	
	opverbcheckwindowrect (ho);
	
	killundo (); /*must toss undos before they're stranded*/
	
	if ((**ho).fldirty) { /*we have to keep the in-memory version around*/
		
		(**ho).flwindowopen = false;
		
		opcloseoutline (); /*prepare for dormancy, not in a window anymore*/
		}
	
	return (true);
	} /*opverbclose*/


static boolean opverbsetfont (void) {
	
	return (opsetfont ((**outlinewindowinfo).selectioninfo.fontnum));
	} /*opverbsetfont*/


static boolean opverbsetsize (void) {
	
	return (opsetsize ((**outlinewindowinfo).selectioninfo.fontsize));
	} /*opverbsetsize*/


static boolean opwinnewrecord (void) {
	
	register hdloutlinerecord ho;
	
	if (!opnewrecord ((**outlinewindowinfo).contentrect))
		return (false);
	
	ho = outlinedata; /*copy into register*/
	
	(**outlinewindowinfo).hdata = (Handle) ho;
	
	(**ho).flwindowopen = true;
	
	(**ho).fldirty = true; /*needs to be saved*/
	
	opverbsetcallbacks (ho);
	
	return (true);
	} /*opwinnewrecord*/


static boolean opwindisposerecord (void) {

	opdisposeoutline (outlinedata, true);
	
	outlinedata = nil;
	
	(**outlinewindowinfo).hdata = nil;

	return true;
	} /*opwindisposerecord*/

	
static boolean opwinloadfile (hdlfilenum fnum, short rnum) {
	
	Handle hpackedop;
	register hdloutlinerecord ho;
	boolean fl;
	long ixload = 0;
	
	if (!filereadhandle (fnum, &hpackedop))
		return (false);
	
	fl = opunpack (hpackedop, &ixload);
	
	disposehandle (hpackedop);
	
	if (!fl)
		return (false);
	
	ho = outlinedata; /*remember for linking into variable structure*/
	
	(**outlinewindowinfo).hdata = (Handle) ho;
	
	(**ho).flwindowopen = true;
	
	(**ho).fldirty = true; /*never been saved, can't be clean*/
	
	opverbsetcallbacks (ho);
	
	return (true);
	} /*opwinloadfile*/


static boolean opwinsavefile (hdlfilenum fnum, short rnum, boolean flsaveas) {
	
	Handle hpackedop = nil;
	boolean fl;
	
	if (!oppack (&hpackedop))
		return (false);
	
	fl = 
		fileseteof (fnum, 0) &&
		
		filesetposition (fnum, 0) &&
		
		filewritehandle (fnum, hpackedop);
	
	disposehandle (hpackedop);
	
	return (fl);
	} /*opwinsavefile*/


boolean opstart (void) {
	
	/*
	set up callback routines record, and link our data into the shell's 
	data structure.
	*/
	
	ptrcallbacks callbacks;
	register ptrcallbacks cb;
	
	opinitdisplayvariables ();
	
	shellnewcallbacks (&callbacks);
	
	cb = callbacks; /*copy into register*/
	
	loadconfigresource (idscriptconfig, &(*cb).config);
	
	(*cb).config.flnewonlaunch = true; // *** need to update resource

	(*cb).configresnum = idscriptconfig;
		
	(*cb).windowholder = &outlinewindow;
	
	(*cb).dataholder = (Handle *) &outlinedata;
	
	(*cb).infoholder = &outlinewindowinfo;
		
	(*cb).initroutine = &wpinit;
	
	(*cb).quitroutine = &wpshutdown;
	
	(*cb).setglobalsroutine = &opeditsetglobals;
	
	(*cb).newrecordroutine = opwinnewrecord;
	
	(*cb).disposerecordroutine = opwindisposerecord;
	
	(*cb).loadroutine = opwinloadfile;
	
	(*cb).saveroutine = opwinsavefile;
	
	(*cb).updateroutine = &opupdate;
	
	(*cb).activateroutine = &opactivate;
	
	(*cb).getcontentsizeroutine = &opgetoutinesize;
	
	(*cb).resizeroutine = &opverbresize;
	
	(*cb).scrollroutine = &opscroll;
	
	(*cb).setscrollbarroutine = &opresetscrollbars;
	
	(*cb).mouseroutine = &opmousedown;
	
	(*cb).keystrokeroutine = &opkeystroke;
	
	(*cb).getundoglobalsroutine = &opeditgetundoglobals;
	
	(*cb).setundoglobalsroutine = &opeditsetundoglobals;
	
	(*cb).cutroutine = &opcut;
	
	(*cb).copyroutine = &opcopy;
	
	(*cb).pasteroutine = &oppaste;
	
	(*cb).clearroutine = &opclear;
	
	(*cb).selectallroutine = &opselectall;
	
	(*cb).fontroutine = &opverbsetfont;
	
	(*cb).sizeroutine = &opverbsetsize;
	
	(*cb).setselectioninforoutine = &opsetselectioninfo;
	
	(*cb).idleroutine = &opidle;
	
	(*cb).adjustcursorroutine = &opsetcursor;
	
	(*cb).setprintinfoproutine = &opsetprintinfo;
	
	(*cb).printroutine = &opprint;
	
	(*cb).settextmoderoutine = &opsettextmode;
	
	(*cb).cmdkeyfilterroutine = &opcmdkeyfilter;
	
	(*cb).closeroutine = &opverbclose;
	
	(*cb).buttonroutine = &scriptbutton;
	
	(*cb).buttonstatusroutine = &scriptbuttonstatus;
	
	if (!newclearhandle (longsizeof (tydebuggerrecord), (Handle *) &debuggerdata)) /*all fields are cool at 0*/
		return (false);
	
	return (true);
	} /*opstart*/






