
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

#include "shell.h"
#include "ops.h"
#include "quickdraw.h"
#include "scrap.h"
#include "op.h"
#include "opinternal.h"
#include "opicons.h"



static void opdisposescrap (void *hnode) {
	
	/*
	5/23/91 dmb: preserve edit globals in case a refcon disposal disturbs 
	them.
	
	7/20/91 dmb: edit globals now preserved within opeditdispose
	*/
	
	opdisposeoutline ((hdloutlinerecord) hnode, false);
	} /*opdisposescrap*/


static boolean opexportscrap (void *houtline, tyscraptype totype, Handle *htext, boolean *fltempscrap) {

	/*
	if the requested type isn't text, return false.  otherwise, try to 
	convert the given scrap to text
	*/
	
	*fltempscrap = true;
	
	if (totype == opscraptype) /*make contiguous version for system scrap*/
		return (oppackoutline ((hdloutlinerecord) houtline, htext));
	
	if (totype != 'TEXT')
		return (false);
	
	return (opoutlinetonewtextscrap (houtline, htext));
	} /*opexportscrap*/


boolean opdefaultsetscraproutine (hdloutlinerecord houtline) {
	
	return (shellsetscrap ((Handle) houtline, opscraptype, &opdisposescrap, &opexportscrap));
	} /*opdefaultsetscraproutine*/


static boolean opgetscraproutine (hdloutlinerecord *houtline, boolean *fltempscrap) {
	
	return (shellconvertscrap (opscraptype, (Handle *) houtline, fltempscrap));
	} /*opgetscraproutine*/


boolean opscraphook (Handle hscrap) {
	
	/*
	if our private type is on the external clipboard, set the internal 
	scrap to it.
	*/
	
	if (getscrap (opscraptype, hscrap)) {
		
		hdloutlinerecord houtline;
		
		if (opunpackoutline (hscrap, &houtline))
			opdefaultsetscraproutine (houtline);
		
		return (false); /*don't call any more hooks*/
		}
	
	return (true); /*keep going*/
	} /*opscraphook*/


boolean opdefaultreleaserefconroutine (hdlheadrecord hnode, boolean fldisk) {
	
	/*
	default callback for releasing the refcon, does nothing.  note that the 
	callback is responsible for releasing the data linked into the refcon
	field, not for releasing the refcon handle itself.
	*/
	
	return (true);
	} /*opdefaultreleaserefconroutine*/
	
	
static boolean opdefaultpushstyle (hdlheadrecord hnode) {
	
	oppushstyle (outlinedata);
	
	return (true);
	} /*opdefaultpushstyle*/
	
	
static boolean opdefaultmouseinline (hdlheadrecord hnode, Point pt, const Rect *textrect, boolean *flintext) {
	
	/*
	2/7/97 dmb: chaning the meaning of this callback somewhat, we return
	true if the mouse is over text
	
	5.0a4 dmb: apparantly, the Mac's PtInRect is stricture than we want to be.
	also, use textslop pixels like 4.x did

	5.0b7 dmb: added flintext parameter; we now return false if normal 
	processing should stop
	*/
	
	Rect r = *textrect;
	
	r.left -= textleftslop;
	
	if (!(**outlinedata).fltextmode)
		r.right = r.left + opgetlinewidth (hnode) + textrightslop;
	
	// return (pointinrect (pt, r));
	
	*flintext = (pt.h >= r.left && pt.h <= r.right) &&
				(pt.v >= r.top && pt.v <= r.bottom);
	
	return (true);
	} /*opdefaultmouseinline*/


static boolean opdefaulticon2click (hdlheadrecord hnode) {
	
	return (false); /*don't consume the double-click*/
	} /*opdefaulticon2click*/


static boolean opdefaultsetscrollbars (void) {

	register ptrwindowinfo pw = *outlinewindowinfo;
	register ptroutlinerecord po = *outlinedata;
	
	(*pw).vertscrollinfo = (*po).vertscrollinfo;
	
	(*pw).horizscrollinfo = (*po).horizscrollinfo;
	
	(*pw).fldirtyscrollbars = true; /*force a refresh of scrollbars by the shell*/
	
	return (true);
	} /*opdefaultsetscrollbars*/


void opinitcallbacks (hdloutlinerecord houtline) {
	
	/*
	5.0a25 dmb: default cmdclickcallback needs to return false now
	
	5.0.2b16 dmb: default postfontchangecallback is opseteditbufferrect
	
	5.1.5 dmb: ...now, it's oppostfontchange
	*/
	
	#if !fljustpacking
	
		register hdloutlinerecord ho = houtline;
		
		(**ho).setscrollbarsroutine = &opdefaultsetscrollbars;
		
		// (**ho).getlinedisplayinfocallback = (opgetlineinfocallback) &truenoop;
		
		(**ho).drawlinecallback = &opdefaultdrawtext; /*the normal text-drawing routine*/
		
		(**ho).gettextrectcallback = &opdefaultgettextrect;
		
		(**ho).getedittextrectcallback = &opdefaultgetedittextrect;
		
		(**ho).setwpedittextcallback = &opdefaultsetwpedittext; /*edits the node's headstring*/
		
		(**ho).getwpedittextcallback = &opdefaultgetwpedittext; /*updates the node's headstring*/
		
		(**ho).drawiconcallback = &opdefaultdrawicon; /*the normal icon-drawing routine*/
		
		(**ho).geticonrectcallback = &opdefaultgeticonrect; /*the normal icon-positioning routine*/
		
		(**ho).predrawlinecallback = &opdefaultpredrawline; /*erases to background*/
		
		(**ho).postdrawlinecallback = &opdefaultpostdrawline; /*no-op*/
		
		(**ho).copyrefconcallback = &opcopyrefconroutine; /*just copies the handle*/
		
		(**ho).textualizerefconcallback = (optextualizerefconcallback) &truenoop; /*very much like a noop*/
		
		(**ho).printrefconcallback = (opnodecallback) &truenoop; /*indistinguishable from a noop*/
		
		(**ho).releaserefconcallback = &opdefaultreleaserefconroutine; /*basically a no-op*/
		
		(**ho).searchrefconcallback = (opnodecallback) &falsenoop; /*just returns false*/
		
		(**ho).deletelinecallback = (opnodecallback) &truenoop; /*called before a line is deleted*/
		
		(**ho).insertlinecallback = (opnodecallback) &truenoop; /*called after a line is inserted*/
		
		(**ho).textchangedcallback = (optextchangedcallback) &truenoop; /*called when the text of a line has changed*/
		
		(**ho).mouseinlinecallback = &opdefaultmouseinline;
		
		(**ho).postfontchangecallback = &oppostfontchange;
		
		(**ho).hasdynamicsubscallback = (opnodecallback) &falsenoop;
		
		(**ho).haslinkedtextcallback = (opnodecallback) &falsenoop;
		
		(**ho).cmdclickcallback = (opnodecallback) &falsenoop;
		
		(**ho).doubleclickcallback = &truenoop;
		
		(**ho).getscrapcallback = (opgetscrapcallback) &opgetscraproutine;
		
		(**ho).setscrapcallback = (opsetscrapcallback) &opdefaultsetscraproutine;
		
		(**ho).texttooutlinecallback = &optextscraptooutline;
		
		(**ho).preexpandcallback = (oppreexpandcallback) &truenoop;
		
		(**ho).postcollapsecallback = (opnodecallback) &truenoop;
		
		(**ho).validatedragcallback = (opvalidatecallback) &truenoop;
		
		(**ho).predragcallback = (opdragtargetcallback) &truenoop;
		
		(**ho).dragcopycallback = (op2nodescallback) &falsenoop;
		
		(**ho).getlineheightcallback = &opdefaultgetlineheight;
		
		(**ho).getlinewidthcallback = &opdefaultgetlinewidth;
		
		(**ho).adjustcursorcallback = opdefaultadjustcursor;
		
		(**ho).pushstylecallback = &opdefaultpushstyle;
		
		(**ho).icon2clickcallback = &opdefaulticon2click;
		
		(**ho).validatepastecallback = (opvalidatecallback) &truenoop;
		
		(**ho).postpastecallback = (opnodecallback) &truenoop;
		
		(**ho).validatecopycallback = (opstringcallback) &truenoop;
		
		(**ho).caneditcallback = (opnodecallback) &truenoop;
		
		(**ho).getfullrectcallback = &opdefaultgetfullrect;
		
		(**ho).nodechangedcallback = (opnodecallback) &truenoop;
		
	//	(**ho).returnkeycallback = opdefaultreturnkey;

		(**ho).beforeprintpagecallback = &truenoop;

		(**ho).afterprintpagecallback = &truenoop;

	#endif
	} /*opinitcallbacks*/


