
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

#include "Paige.h"

#include "frontierconfig.h"
#include "cursor.h"
#include "file.h"
#include "kb.h"
#include "memory.h"
#include "mouse.h"
#include "ops.h"
#include "quickdraw.h"
#include "resources.h"
#include "strings.h"
#include "search.h"
#include "zoom.h"
#include "shell.h"
#include "shellhooks.h"
#include "shellprint.h"
#include "shellundo.h"
#include "lang.h"
#include "langexternal.h"
#include "tablestructure.h"
#include "cancoon.h"
#include "wpengine.h"
#include "wpinternal.h"
#include "wpverbs.h"
#include "kernelverbdefs.h"



#define wperrorlist 264 
#define nowperror 1
#define namenotwperror 2

#define wpstringlist 164
#define wptypestring 1
#define wpsizestring 2


typedef enum tywptoken { /*verbs that are processed by wp.c*/
	
	intextmodefunc,
	
	settextmodefunc,
	
	gettextfunc,
	
	settextfunc,
	
	getseltextfunc,
	
	getdisplayfunc,
	
	setdisplayfunc,
	
	getrulerfunc,
	
	setrulerfunc,
	
	getindentfunc,
	
	setindentfunc,
	
	getleftmarginfunc,
	
	setleftmarginfunc,
	
	getrightmarginfunc,
	
	setrightmarginfunc,
	
	setspacingfunc,
	
	setjustificationfunc,
	
	settabfunc,
	
	cleartabsfunc,
	
	getselectfunc,
	
	setselectfunc,
	
	insertfunc,
	
	#ifdef flvariables
	
	newvariablefunc,
	
	deletevariablefunc,
	
	getnthvariablefunc,
	
	setvariablevaluefunc,
	
	getvariablevaluefunc,
	
	insertvariablefunc,
	
	getevalfunc,
	
	setevalfunc,
	
	hilitevariablesfunc,
	
	#endif
	
	rulerlengthfunc,
	
	gofunc,
	
	selectwordfunc,
	
	selectlinefunc,
	
	selectparafunc,
	
	ctwpverbs
	
	} tywptoken;



#if langexternalfind_optimization

	typedef tyexternalvariable tywpvariable, *ptrwpvariable, **hdlwpvariable;

#else
	
typedef struct tywpvariable {
	
	unsigned short id; /*tyexternalid: managed by langexternal.c*/
	
	unsigned short flinmemory: 1; /*if true, variabledata is an hdlwprecord, else a dbaddress*/
	
	unsigned short flmayaffectdisplay: 1; /*not in memory, but being displayed in a table window*/
	
	unsigned short flpacked: 1; /*the wp doc isn't being edited, so we store it packed*/
	
	long variabledata; /*either a hdlwprecord or a dbaddress*/
	
	hdldatabaserecord hdatabase; // 5.0a18 dmb

	dbaddress oldaddress; /*last place this wp doc was stored in db*/
	} tywpvariable, *ptrwpvariable, **hdlwpvariable;

#endif

static errornum = 0; /*error number exclusively for wp routines*/


#if !flruntime

static void wpverbsetscrollbarsroutine (void) {

	register ptrwindowinfo pw = *wpwindowinfo;
	register ptrwprecord pwp = *wpdata;
	
	(*pw).vertscrollinfo = (*pwp).vertscrollinfo; /*copy from wp record to window record*/
	
	(*pw).horizscrollinfo = (*pwp).horizscrollinfo;
	
	(*pw).fldirtyscrollbars = true; /*force a refresh of scrollbars by the shell*/
	} /*wpverbsetscrollbarsroutine*/


static void wplinkwindowinfo (void) {
	
	/*
	link wpdata to wpwindowinfo, and make sure that the content rect of 
	the WS handle is up to date.
	
	5.0a2 dmb: use paperrect, not contentrect, as bounds
	*/
	
	register hdlwindowinfo hw = wpwindowinfo;
	Rect rbounds;
	
	wpsetupwindow ();
	
	rbounds = (**hw).contentrect;
	
	insetrect (&rbounds, 3, 3);
	
	rbounds.right = rbounds.left + shellprintinfo.paperrect.right - shellprintinfo.paperrect.left;
	
	rbounds.bottom = rbounds.top + shellprintinfo.paperrect.bottom - shellprintinfo.paperrect.top;
	
	wpsetbufferrect ((**hw).contentrect, rbounds);
	
	(**wpdata).setscrollbarsroutine = &wpverbsetscrollbarsroutine;
	} /*wplinkwindowinfo*/


boolean wpverbgettypestring (hdlexternalvariable hvariable, bigstring bs) {
	
	getstringlist (wpstringlist, wptypestring, bs);
	
	return (true);
	} /*wpverbgettypestring*/


#endif // !flruntime

static boolean newwpvariable (boolean flinmemory, boolean flpacked, long variabledata, hdlwpvariable *h) {

	if (!langnewexternalvariable (flinmemory, variabledata, (hdlexternalvariable *) h))
		return (false);
	
	(***h).flpacked = flpacked;

	return (true);
	} /*newwpvariable*/


static boolean wpdisposevariable (hdlexternalvariable hvariable, boolean fldisk) {
	
	register hdlwpvariable hv = (hdlwpvariable) hvariable;
	register long vdata = (**hv).variabledata;
	
	if ((**hv).flpacked) 
		disposehandle ((Handle) vdata);
	else
		wpdisposerecord ((hdlwprecord) vdata);
		
	return (true);
	} /*wpdisposevariable*/


boolean wpverbdispose (hdlexternalvariable hvariable, boolean fldisk) {
	
	return (langexternaldisposevariable (hvariable, fldisk, &wpdisposevariable));
	} /*wpverbdispose*/


#if !flruntime

static void wpverbcheckwindowrect (register hdlwprecord hwp) {
	
	hdlwindowinfo hinfo;
	
	if ((**hwp).flwindowopen) { /*make windowrect reflect current window size & position*/
		
		if (shellfinddatawindow ((Handle) hwp, &hinfo)) {
			
			Rect r;
			
			shellgetglobalwindowrect (hinfo, &r);
			
			if (!equalrects (r, (**hwp).windowrect)) {
			
				(**hwp).windowrect = r;
				
				(**hwp).fldirty = true;
				}
			}
		}
	} /*wpverbcheckwindowrect*/

#else
	
	#define wpverbcheckwindowrect(hwp)	((void *) 0)

#endif

#if !flruntime

boolean wpverbisdirty (hdlexternalvariable hvariable) {
	
	/*
	8/11/92 dmb: check windowrect now to ensure consistent results
	*/
	
	register hdlwpvariable hv = (hdlwpvariable) hvariable;
	register hdlwprecord hwp;
	
	if (!(**hv).flinmemory)
		return (false);
	
	if ((**hv).flpacked) /*if it's packed and in memory, it must be dirty*/
		return (true);
	
	hwp = (hdlwprecord) (**hv).variabledata;
	
	wpverbcheckwindowrect (hwp);
	
	return ((**hwp).fldirty || (**hwp).fldirtyview);
	} /*wpverbisdirty*/


boolean wpverbsetdirty (hdlexternalvariable hvariable, boolean fldirty) {
	
	/*
	4/15/92 dmb: see comments in langexternalsetdirty
	*/
	
	register hdlwpvariable hv = (hdlwpvariable) hvariable;
	register hdlwprecord hwp;
	
	if (!wpverbinmemory ((hdlexternalvariable) hv))
		return (false);
	
	hwp = (hdlwprecord) (**hv).variabledata;
	
	(**hwp).fldirty = (**hwp).fldirtyview = fldirty;
	
	return (true);
	} /*wpverbsetdirty*/

#endif // !flruntime


static void wpverblinkvariable (hdlwprecord hwp, hdlwpvariable hv) {
	
	(**hv).variabledata = (long) hwp; /*link the wp rec into the variable rec*/
	
	(**hwp).wprefcon = (long) hv; /*the pointing is mutual*/
	} /*wpverblinkvariable*/


boolean wpverbnew (Handle hdata, hdlexternalvariable *hvariable) {
	
	/*
	9/11/91 dmb: now take hdata parameter.  note: for now, we're assuming 
	that only tablescrap calls us with non-nil hdata.  to allow it to be 
	either WS text or plain text, we actually ignore hdata and let 
	wppaste to the interpretation.
	*/
	
	register hdlwpvariable hv;
	hdlwprecord hwp;
	Rect r;
	
	if (!newwpvariable (true, false, 0L, (hdlwpvariable *) hvariable))
		return (false);
	
	hv = (hdlwpvariable) *hvariable; /*copy into register*/
	
	setrect (&r, 0, 0, infinity, infinity); /*very large clip & bounds area*/
	
	if (!wpnewrecord (r, &hwp)) {
		
		disposehandle ((Handle) hv);
		
		return (false);
		}
	
	if (hdata != nil) { /*we've been given intial text*/
		
		shellpushglobals (nil); /*preserve our globals*/
		
		wppushdata (hwp);
		
		/*
		wpsettexthandle (hdata);
		*/
		
		wppaste (true); /*we're assuming that only tablescrap calls us with data*/
		
		wppopdata ();
		
		shellpopglobals ();
		}
	
	wpverblinkvariable (hwp, hv); /*set up pointers to each other*/
	
	return (true);
	} /*wpverbnew*/


static boolean wpverbpackrecord (hdlwprecord hwp, Handle *hpacked) {
	
	register boolean fl;
	register hdlwprecord hwporig = wpdata;
	
	wpdata = hwp;
	
	fl = wppack (hpacked);
	
	wpdata = hwporig;
	
	return (fl);
	} /*wpverbpackrecord*/


boolean wpverbmemorypack (hdlexternalvariable h, Handle *hpacked) {
	
	/*
	a special entrypoint for the language's pack and upack verbs.
	*/
	
	register hdlwpvariable hv = (hdlwpvariable) h;
	register hdlwprecord hwp;
	register boolean fl;
	Handle hpush = nil;
	
	if (!(**hv).flinmemory) {
	
		if (!langexternalrefdata ((hdlexternalvariable) hv, &hpush))
			return (false);
			
		goto pushandexit;
		}
		
	if ((**hv).flpacked) {
		
		if (!copyhandle ((Handle) (**hv).variabledata, &hpush))
			return (false);
		
		goto pushandexit;
		}
	
	/*it's in memory and not packed*/
	
	hwp = (hdlwprecord) (**hv).variabledata;
	
	wpverbcheckwindowrect (hwp);
	
	if (!wpverbpackrecord (hwp, &hpush))
		return (false);
	
	pushandexit:
	
	if (hpush == nil)
		return (false);
	
	fl = pushhandle (hpush, *hpacked);
	
	disposehandle (hpush);
	
	return (fl);
	} /*wpverbmemorypack*/
	
	
boolean wpverbmemoryunpack (Handle hpacked, long *ixload, hdlexternalvariable *h) {
	
	/*
	create a new variable -- in memory, packed, with hpacked as the data handle.
	
	this is a special entrypoint for the pack and unpack verbs.
	*/
	
	Handle hpackedwp;
	
	if (!loadhandleremains (*ixload, hpacked, &hpackedwp))
		return (false);
	
	return (newwpvariable (true, true, (long) hpackedwp, (hdlwpvariable *) h));	
	} /*wpverbmemoryunpack*/


static void wpverbondisk (hdlwpvariable hv, dbaddress adr) {

	(**hv).flinmemory = false;
	
	(**hv).flpacked = false;
	
	(**hv).variabledata = (long) adr;
	} /*wpverbondisk*/


static void wpverbunload (hdlwpvariable hv) {
	
	/*
	5.0.2b20 dmb: corrected disposal of loaded variable
	*/
	
	long variabledata = (**hv).variabledata;
	
	assert ((**hv).flinmemory);
	
	if ((**hv).flpacked)
		disposehandle ((Handle) variabledata);
	else
		wpdisposerecord ((hdlwprecord) variabledata);
	
	if ((**hv).oldaddress == nil) // 5.0d3 dmb
		disposehandle ((Handle) hv);
	else
		wpverbondisk (hv, (**hv).oldaddress);
	} /*wpverbunload*/


boolean wpverbpack (hdlexternalvariable h, Handle *hpacked, boolean *flnewdbaddress) {

	/*
	6.2a15 AR: added flnewdbaddress parameter
	*/

	register hdlwpvariable hv = (hdlwpvariable) h;
	register boolean flpacked = (**hv).flpacked;
	register hdlwprecord hwp;
	register boolean fl;
	Handle hpackedwp;
	dbaddress adr;
	hdlwindowinfo hinfo;
	boolean fltempload = false;
	
	if (!(**hv).flinmemory) { /*simple case, wp doc is resident in the db*/
		
		if (flconvertingolddatabase) {
			
			if (!wpverbinmemory (h))
				return (false);
			
			fltempload = true;
			}
		else {
		
			adr = (dbaddress) (**hv).variabledata;
			
			if (fldatabasesaveas)
				if (!dbcopy (adr, &adr))
					return (false);
			
			goto pushaddress;
			}
		}
	
	/*the doc is in memory, either as a WP-Engine structure or a packed handle*/
	
	adr = (**hv).oldaddress; /*place where this wp doc used to be stored*/
	
	if ((**hv).flpacked) { /*no window open, but changes were made*/
		
		hpackedwp = (Handle) (**hv).variabledata;
		
		if (!dbassignhandle (hpackedwp, &adr))
			return (false);
		
		if (fldatabasesaveas)
			goto pushaddress;
		
		wpverbondisk (hv, adr);
		
		disposehandle (hpackedwp); /*reclaim memory used by packed doc*/
		
		goto pushaddress;
		}
	
	/*the wpdoc is in memory and it's not packed*/
	
	hwp = (hdlwprecord) (**hv).variabledata;
	
	wpverbcheckwindowrect (hwp);
	
	if (!fldatabasesaveas && !(**hwp).fldirty && !(**hwp).fldirtyview) /*don't need to update the db version of the wpdoc*/
		goto pushaddress;
	
	if (!wpverbpackrecord (hwp, &hpackedwp))
		return (false);
	
	fl = dbassignhandle (hpackedwp, &adr);
	
	disposehandle (hpackedwp);
	
	if (!fl)
		return (false);
	
	if (fldatabasesaveas)
		goto pushaddress;
	
	if (!wpwindowopen ((hdlexternalvariable) hv, &hinfo)) { /*it's been saved, we can reclaim some memory*/
		
		wpverbondisk (hv, adr);
		
		wpdisposerecord (hwp); /*reclaim memory used by doc*/
		}
	else {
		
		assert (!fltempload);
		
		(**hwp).fldirty = false; /*we just saved off a new db version*/
		
		(**hwp).fldirtyview = false;
		
		shellsetwindowchanges (hinfo, false);
		}
	
	pushaddress:
	
	if (!fldatabasesaveas) {
	
		*flnewdbaddress = ((**hv).oldaddress != adr);
			
		(**hv).oldaddress = adr;
		}
	else
		*flnewdbaddress = true;	
	
	return (pushlongondiskhandle (adr, *hpacked));
	} /*wpverbpack*/


boolean wpverbunpack (Handle hpacked, long *ixload, hdlexternalvariable *h) {

	dbaddress adr;
	
	if (!loadlongfromdiskhandle (hpacked, ixload, &adr)) 
		return (false);

	return (newwpvariable (false, false, adr, (hdlwpvariable *) h));
	} /*wpverbunpack*/


boolean wpverbinmemory (hdlexternalvariable h) {
	
	/*
	6/6/91 dmb: after loading, set dirty bit if value has never been 
	saved to disk
	
	11/14/01 dmb: always marked as dirty when expanding from in-memory packed state
	
	5.0a18 dmb: support database linking
	*/
	
	register hdlwpvariable hv = (hdlwpvariable) h;
	hdlwprecord hwp;
	boolean flinmemory = (**hv).flinmemory;
	boolean fl;
	dbaddress adr;
	Handle hpackedwp;
	boolean fldirty;
	
	if (flinmemory && !(**hv).flpacked) /*nothing to do, it's already unpacked & in memory*/
		return (true);
	
	if (flinmemory) { /*don't need to read packed wp from database*/
		
		adr = (**hv).oldaddress;
		
		hpackedwp = (Handle) (**hv).variabledata;
		
		fldirty = true; /*otherwise, wouldn't have been kept in memory*/
		}
	else {
		adr = (dbaddress) (**hv).variabledata;
		
		if (!langexternalrefdata ((hdlexternalvariable) hv, &hpackedwp))
			return (false);
		
		fldirty = false; /*clean version from disk*/
		}

	fl = wpunpack (hpackedwp, &hwp);
	
	if (fl || !flinmemory) /*on failure, don't dispose packed version if in memory*/
		disposehandle (hpackedwp);
	
	if (!fl)
		return (false);
	
	(**hv).flinmemory = true;
	
	(**hv).flpacked = false;
	
	(**hv).oldaddress = adr; /*last place this wp was stored*/
	
	(**hwp).fldirty = fldirty;
	
	wpverblinkvariable (hwp, hv); /*set up pointers to each other*/
	
	return (true);
	} /*wpverbinmemory*/


#if !flruntime

boolean wpverbgetsize (hdlexternalvariable hvariable, long *size) {
	
	register hdlwpvariable hv = (hdlwpvariable) hvariable;
	
	if ((**hv).flpacked)
		wpgetpackedmaxpos ((Handle) (**hv).variabledata, size);
		
	else {
		
		if (!wpverbinmemory ((hdlexternalvariable) hv))
			return (false);
		
		wppushdata ((hdlwprecord) (**hv).variabledata);
		
		wpgetmaxpos (size);
		
		wppopdata ();
		}
	
	return (true);
	} /*wpverbgetsize*/


boolean wpverbgetdisplaystring (hdlexternalvariable h, bigstring bs) {
	
	/*
	get string for table display.  variable is in memory
	*/
	
	long ctchars;
	
	if (!wpverbgetsize (h, &ctchars))
		return (false);
	
	parsenumberstring (wpstringlist, wpsizestring, ctchars, bs);
	
	return (true);
	} /*wpverbgetdisplaystring*/


boolean wpverbpacktotext (hdlexternalvariable h, Handle htext) {
	
	/*
	5.0.2b20 dmb: unload if just loaded
	*/
	
	register hdlwpvariable hv = (hdlwpvariable) h;
	register hdlwprecord hwp;
	register boolean fl;
	register hdlwprecord hwporig = wpdata;
	Handle hwptext;
	boolean fltempload = !(**hv).flinmemory;
	
	if (!wpverbinmemory ((hdlexternalvariable) hv))
		return (false);
	
	hwp = (hdlwprecord) (**hv).variabledata;
	
	wpdata = hwp;
	
	fl = wpgettexthandle (&hwptext);
	
	wpdata = hwporig;
	
	if (fl)
		fl = pushhandle (hwptext, htext);
	
	disposehandle (hwptext);
	
	if (fltempload)
		wpverbunload (hv);
	
	return (fl);
	} /*wpverbpacktotext*/


boolean wpverbgettimes (hdlexternalvariable h, long *timecreated, long *timemodified) {
	
	register hdlexternalvariable hv = h;
	register hdlwprecord hwp;
	
	if (!wpverbinmemory (hv))
		return (false);
	
	hwp = (hdlwprecord) (**hv).variabledata;
	
	*timecreated = (**hwp).timecreated;
	
	*timemodified = (**hwp).timelastsave;
	
	return (true);
	} /*wpverbgettimes*/


boolean wpverbsettimes (hdlexternalvariable h, long timecreated, long timemodified) {
	
	register hdlexternalvariable hv = h;
	register hdlwprecord hwp;
	
	if (!wpverbinmemory (hv))
		return (false);
	
	hwp = (hdlwprecord) (**hv).variabledata;
	
	(**hwp).timecreated = timecreated;
	
	(**hwp).timelastsave = timemodified;
	
	return (true);
	} /*wpverbsettimes*/


static boolean getwpparam (hdltreenode hfirst, short pnum, hdlwpvariable *hv) {
	
	/*
	the caller wants an wp doc parameter.  we only accept them as "var"
	parameters, we just want the name of a variable that holds an wpdoc.
	
	this keeps lang.c from having to know too much about wpdocs, but 
	more importantly, keeps them from being automatically copied!  could
	be very wasteful of memory because these things can get very large.
	
	we return a handle to the wp record and the identifier we 
	referenced to get the wp doc.  the string may used to name a window,
	for example.
	*/
	
	short id;
	
	if (!langexternalgetexternalparam (hfirst, pnum, &id, (hdlexternalvariable *) hv)) {
		
		errornum = namenotwperror;
		
		return (false);
		}
	
	if (id != idwordprocessor) {
		
		errornum = namenotwperror;
		
		return (false);
		}
	
	return (true);
	} /*getwpparam*/


boolean wpwindowopen (hdlexternalvariable hvariable, hdlwindowinfo *hinfo) {
	
	/*
	1/1/92 dmb: check flpacked!
	
	5.0b9 dmb: for clipboard operations, must test hwp for nil
	*/
	
	register hdlwpvariable hv = (hdlwpvariable) hvariable;
	register hdlwprecord hwp;
	
	if (!(**hv).flinmemory)
		return (false);
	
	if ((**hv).flpacked) /*if it's packed, window can't be open*/
		return (false);
	
	hwp = (hdlwprecord) (**hv).variabledata;
	
	if (hwp == nil)
		return (false);
	
	shellfinddatawindow ((Handle) hwp, hinfo); /*3/19/91 DW*/
	
	return ((**hwp).flwindowopen);
	} /*wpwindowopen*/


boolean wpedit (hdlexternalvariable hvariable, hdlwindowinfo hparent, ptrfilespec fs, bigstring bstitle, rectparam rzoom) {

	/*
	4.17.97 dmb: protect against globals smashing under Windows

	5.0b18 dmb: further protection: set flwindowopen bit before 
	newchildwindow, which will yield

	5.0.2b6 dmb: added flwindowopen loop to handle Windows async overlap
	*/
	
	register hdlwpvariable hv = (hdlwpvariable) hvariable;
	register hdlwprecord hwp;
	Rect rwindow;
	WindowPtr w;
	hdlwindowinfo hi;
	
	if (!wpverbinmemory ((hdlexternalvariable) hv)) /*couldn't swap it into memory*/
		return (false);
	
	hwp = (hdlwprecord) (**hv).variabledata; /*it's in memory*/
	
	while ((**hwp).flwindowopen) { /*bring to front, return true*/
		
		if (shellfinddatawindow ((Handle) hwp, &hi)) {
			
			if ((*rzoom).top > -2)
				shellbringtofront (hi);
			
			return (true);
			}

		#ifdef MACVERSION
			break;
		#else
			if (!shellyield (false))
				return (false);
		#endif
		}
	
	rwindow = (**hwp).windowrect; /*window comes up where it was last time*/
	
	(**hwp).flwindowopen = true;

	if (!newchildwindow (idwpconfig, hparent, &rwindow, rzoom, bstitle, &w)) {
		
		(**hwp).flwindowopen = false;

		return (false);
		}
	
	wpdata = hwp; /*copy into global*/
	
	getwindowinfo (w, &hi);
	
	(**hi).hdata = (Handle) hwp;
	
	if (fs != nil)
		(**hi).fspec = *fs;
	
	shellpushglobals (w);

	wplinkwindowinfo ();
	
	shellpopglobals ();
	
	//(**hwp).flwindowopen = true;
	
	if ((**hwp).fldirty)
		shellsetwindowchanges (hi, true);

	windowzoom (w); /*show the window to the user. do last, or we'd have to push globals*/
	
	return (true);
	} /*wpedit*/


static boolean wpfunctionvalue (short token, hdltreenode hparam1, tyvaluerecord *vreturned, bigstring bserror) {
	
	/*
	bridges wp.c with the language.  the name of the verb is bs, its first parameter
	is hparam1, and we return a value in vreturned.
	
	we use a limited number of support routines from lang.c to get parameters and
	to return values. 
	
	return false only if the error is serious enough to halt the running of the script
	that called us, otherwise error values are returned through the valuerecord, which
	is available to the script.
	
	if we return false, we try to provide a descriptive error message in the 
	returned string bserror.
	
	12/30/91 dmb: use wpinserthandle for insertfunc
	
	5/17/92 dmb: setselectfunc now uses longs, as it should always have
	
	6/1/93 dmb: when vreturned is nil, return whether or not verb token must 
	be run in the Frontier process
	
	5.0.2b17 dmb: insert, settext get readonly handles
	*/
	
	register tyvaluerecord *v = vreturned;
	register boolean fl;
	WindowPtr targetwindow;
	short idprocessor = idwordprocessor;
	
	if (v == nil)
		return (true);
	
	errornum = 0;
	
	setbooleanvalue (false, v); /*by default, wp functions return true*/
	
	switch (token) {
		
		/*
		case packtextfunc:
			if (!wppacktextverb (hparam1, v))
				goto error;
			
			return (true);
		
		case unpacktextfunc:
			if (!wpunpacktextverb (hparam1, v))
				goto error;
			
			return (true);
		*/
		
		case intextmodefunc: /*verb that don't require wp-specific target*/
		case settextmodefunc:
			idprocessor = -1;
			
			break;
		}
	
	/*all other verbs require a wp window in front -- set wpengine.c globals*/
	
	if (!langfindtargetwindow (idprocessor, &targetwindow)) {
		
		errornum = nowperror;
		
		goto error;
		}
	
	shellpushglobals (targetwindow);
	
	(*shellglobals.gettargetdataroutine) (idprocessor); /*set wp globals*/
	
	fl = false; /*default return value*/
	
	switch (token) {
		
		case intextmodefunc: {
			WindowPtr textwindow;
			
			if (!langcheckparamcount (hparam1, 0))
				break;
			
			fl = setbooleanvalue (langfindtargetwindow (idwordprocessor, &textwindow), v);
			
			break;
			}
		
		case settextmodefunc: {
			boolean flenter;
			
			flnextparamislast = true;
			
			if (!getbooleanvalue (hparam1, 1, &flenter))
				break;
			
			fl = setbooleanvalue ((*shellglobals.settextmoderoutine) (flenter), v);
			
			break;
			}
		
		case gettextfunc: {
			Handle htext;
			
			if (!langcheckparamcount (hparam1, 0))
				break;
			
			if (!wpgettexthandle (&htext))
				break;
			
			fl = setheapvalue (htext, stringvaluetype, v);
			
			break;
			}
		
		case settextfunc: {
			Handle htext;
			
			flnextparamislast = true;
			
			if (!getreadonlytextvalue (hparam1, 1, &htext))
				break;
			
			if (!wpsettexthandle (htext))
				break;
			
			fl = setbooleanvalue (true, v);
			
			break;
			}
		
		case getseltextfunc: {
			/*
			bigstring bs;
			*/
			Handle htext;
			
			if (!langcheckparamcount (hparam1, 0))
				break;
			
			if (!wpgetseltexthandle (&htext))
				break;
			
			fl = setheapvalue (htext, stringvaluetype, v);
			
			/*
			wpgetseltext (bs);
			
			fl = setstringvalue (bs, v);
			*/
			
			break;
			}
		
		case getdisplayfunc:
			
			if (!langcheckparamcount (hparam1, 0)) /*too many parameters were passed*/
				break;
			
			(*v).data.flvalue = !(**wpdata).flinhibitdisplay;
			
			fl = true;
			
			break;
		
		case setdisplayfunc: {
			boolean fldisplay;
			
			flnextparamislast = true;
			
			if (!getbooleanvalue (hparam1, 1, &fldisplay))
				break;
			
			(*v).data.flvalue = wpsetdisplay (fldisplay);
			
			fl = true;
			
			break;
			}
		
		case getrulerfunc:
			
			if (!langcheckparamcount (hparam1, 0))
				break;
			
			(*v).data.flvalue = wpgetruler ();
			
			fl = true;
			
			break;
		
		case setrulerfunc: {
			boolean flruler;
			
			flnextparamislast = true;
			
			if (!getbooleanvalue (hparam1, 1, &flruler))
				break;
			
			(*v).data.flvalue = wpsetruler (flruler);
			
			fl = true;
			
			break;
			}
		
		case getindentfunc: {
			short indent;
			
			if (!langcheckparamcount (hparam1, 0))
				break;
			
			wpgetindent (&indent);
			
			fl = setlongvalue (indent, v);
			
			break;
			}
			
		case setindentfunc: {
			short indent;
			
			flnextparamislast = true;
			
			if (!getintvalue (hparam1, 1, &indent))
				break;
			
			(*v).data.flvalue = wpsetindent (indent);
			
			fl = true;
			
			break;
			}
			
		case getleftmarginfunc: {
			short leftmargin;
			
			if (!langcheckparamcount (hparam1, 0))
				break;
			
			wpgetleftmargin (&leftmargin);
			
			fl = setlongvalue (leftmargin, v);
			
			break;
			}
			
		case setleftmarginfunc: {
			short leftmargin;
			
			flnextparamislast = true;
			
			if (!getintvalue (hparam1, 1, &leftmargin))
				break;
			
			(*v).data.flvalue = wpsetleftmargin (leftmargin);
			
			fl = true;
			
			break;
			}
			
		case getrightmarginfunc: {
			short rightmargin;
			
			if (!langcheckparamcount (hparam1, 0))
				break;
			
			wpgetrightmargin (&rightmargin);
			
			fl = setlongvalue (rightmargin, v);
			
			break;
			}
			
		case setrightmarginfunc: {
			short rightmargin;
			
			flnextparamislast = true;
			
			if (!getintvalue (hparam1, 1, &rightmargin))
				break;
			
			(*v).data.flvalue = wpsetrightmargin (rightmargin);
			
			fl = true;
			
			break;
			}
		
		case setspacingfunc: {
			short spacing;
			
			flnextparamislast = true;
			
			if (!getintvalue (hparam1, 1, &spacing))
				break;
			
			(**wpwindowinfo).selectioninfo.leading = spacing;
			
			fl = setbooleanvalue (wpsetleading (), v);
			
			break;
			}
		
		case setjustificationfunc: {
			short justification;
			
			flnextparamislast = true;
			
			if (!getintvalue (hparam1, 1, &justification))
				break;
			
			(**wpwindowinfo).selectioninfo.justification = (tyjustification) (justification - 1);
			
			fl = setbooleanvalue (wpsetjustify (), v);
			
			break;
			}
		
		case settabfunc: {
			short pos;
			short type;
			char fill;
			
			if (!getintvalue (hparam1, 1, &pos))
				break;
			
			if (!getintvalue (hparam1, 2, &type)) /*1 for left tab; 2 center; etc.*/
				break;
			
			flnextparamislast = true;
			
			if (!getcharvalue (hparam1, 3, &fill))
				break;
			
			fl = setbooleanvalue (wpsettab (pos, type - 1, fill), v);
			
			break;
			}
		
		case cleartabsfunc: {
			if (!langcheckparamcount (hparam1, 0))
				break;
			
			fl = setbooleanvalue (wpcleartabs (), v);
			
			break;
			}
		
		case getselectfunc: {
			long startsel, endsel;
			
			wpgetselection (&startsel, &endsel);
			
			if (!langsetlongvarparam (hparam1, 1, startsel))
				return (false);
			
			flnextparamislast = true;
			
			if (!langsetlongvarparam (hparam1, 2, endsel))
				return (false);
			
			(*v).data.flvalue= true;
			
			fl = true;
			
			break;
			}
		
		case setselectfunc: {
			long startsel, endsel;
			
			if (!getlongvalue (hparam1, 1, &startsel))
				break;
			
			flnextparamislast = true;
			
			if (!getlongvalue (hparam1, 2, &endsel))
				break;
			
			wpsetselection (startsel, endsel);
			
			(*v).data.flvalue= true;
			
			fl = true;
			
			break;
			}
		
		case insertfunc: {
			Handle x;

			flnextparamislast = true;
			
			if (!getreadonlytextvalue (hparam1, 1, &x))
				break;
			
			wptrimhiddentext ();
			
			if (!wpinserthandle (x))
				break;
			
			(*v).data.flvalue = true;
			
			fl = true;
			
			break;
			}
		
		/*
		case findfunc: {
			bigstring bs;
			
			flnextparamislast = true;
			
			if (!getstringvalue (hparam1, 1, bs))
				break;
			
			(*v).data.flvalue = wpsearch (bs, false, false, false, false);
			
			fl = true;
			
			break;
			}
		
		case replacefunc:
			if (!langcheckparamcount (hparam1, 0))
				break;
			
			(*v).data.flvalue = wpinsert (searchparams.bsreplace);
			
			fl = true;
			
			break;
		*/
		
		#if false
		
		case writetofilefunc: {
			bigstring bs;
			
			flnextparamislast = true;
			
			if (!getstringvalue (hparam1, 1, bs))
				break;
			
			(*v).data.flvalue = wpwritetofile (bs);
			
			fl = true;
			
			break;
			}
			
		case readfromfilefunc: {
			bigstring bs;
			
			flnextparamislast = true;
			
			if (!getstringvalue (hparam1, 1, bs))
				break;
			
			(*v).data.flvalue = wpreadfromfile (bs);
			
			fl = true;
			
			break;
			}
		
		case readfromtextfilefunc: {
			bigstring bs;
			
			flnextparamislast = true;
			
			if (!getstringvalue (hparam1, 1, bs))
				break;
			
			(*v).data.flvalue = wpreadfromtextfile (bs);
			
			fl = true;
			
			break;
			}
		
		case writetotextfilefunc: {
			bigstring bs;
			
			flnextparamislast = true;
			
			if (!getstringvalue (hparam1, 1, bs))
				break;
			
			(*v).data.flvalue = wpwritetotextfile (bs);
			
			fl = true;
			
			break;
			}
		
		#endif
		
		#ifdef flvariables
		
		case newvariablefunc: {
			bigstring bs, bsvalue;
			
			if (!getstringvalue (hparam1, 1, bs))
				break;
			
			flnextparamislast = true;
			
			if (!getstringvalue (hparam1, 2, bsvalue))
				break;
			
			(*v).data.flvalue = wpnewvariable (bs, bsvalue);
			
			fl = true;
			
			break;
			}
		
		case deletevariablefunc: {
			bigstring bs;
			
			flnextparamislast = true;
			
			if (!getstringvalue (hparam1, 1, bs))
				break;
			
			(*v).data.flvalue = wpdeletevariable (bs);
			
			fl = true;
			
			break;
			}
		
		case getnthvariablefunc: {
			short ix;
			bigstring bs;
			
			flnextparamislast = true;
			
			if (!getintvalue (hparam1, 1, &ix))
				break;
			
			if (!wpgetnthvariable (ix, bs))
				setemptystring (bs);
			
			fl = setstringvalue (bs, v);
			
			break;
			}
		
		case setvariablevaluefunc: {
			bigstring bs, bsvalue;
			
			if (!getstringvalue (hparam1, 1, bs))
				break;
			
			flnextparamislast = true;
			
			if (!getstringvalue (hparam1, 2, bsvalue))
				break;
			
			(*v).data.flvalue = wpsetvariablevalue (bs, bsvalue);
			
			fl = true;
			
			break;
			}
		
		case getvariablevaluefunc: {
			bigstring bs, bsvalue;
			
			flnextparamislast = true;
			
			if (!getstringvalue (hparam1, 1, bs))
				break;
			
			if (!wpgetvariablevalue (bs, bsvalue))
				setemptystring (bsvalue);
			
			fl = setstringvalue (bsvalue, v);
			
			break;
			}
		
		case insertvariablefunc: {
			bigstring bs;
			
			flnextparamislast = true;
			
			if (!getstringvalue (hparam1, 1, bs))
				break;
			
			(*v).data.flvalue = wpinsertvariable (bs);
			
			fl = true;
			
			break;
			}
		
		case getevalfunc:
			if (!langcheckparamcount (hparam1, 0))
				break;
			
			(*v).data.flvalue = wpgetevaluate ();
			
			fl = true;
			
			break;
		
		case setevalfunc: {
			boolean flexpand;
			
			flnextparamislast = true;
			
			if (!getbooleanvalue (hparam1, 1, &flexpand))
				break;
			
			(*v).data.flvalue = wpsetevaluate (flexpand);
			
			fl = true;
			
			break;
			}
		
		case hilitevariablesfunc: {
			boolean flhilite;
			
			flnextparamislast = true;
			
			if (!getbooleanvalue (hparam1, 1, &flhilite))
				break;
			
			(*v).data.flvalue = wphilitevariables (flhilite);
			
			fl = true;
			
			break;
			}
		
		#endif
		
		case rulerlengthfunc:
			if (!langcheckparamcount (hparam1, 0))
				break;
			
			fl = setlongvalue (wpavailwidth (), v);
			
			break;
		
		case gofunc: {
			tydirection dir;
			long dist;
			
			if (!getdirectionvalue (hparam1, 1, &dir))
				break;
			
			flnextparamislast = true;
			
			if (!getlongvalue (hparam1, 2, &dist))
				break;
			
			fl = setbooleanvalue (wpgo (dir, dist), v);
			
			break;
			}
		
		case selectwordfunc:
			if (!langcheckparamcount (hparam1, 0))
				break;
			
			fl = setbooleanvalue (wpselectword (), v);
			
			break;
		
		case selectlinefunc:
			if (!langcheckparamcount (hparam1, 0))
				break;
			
			fl = setbooleanvalue (wpselectline (), v);
			
			break;
		
		case selectparafunc:
			if (!langcheckparamcount (hparam1, 0))
				break;
			
			fl = setbooleanvalue (wpselectpara (), v);
			
			break;
		
		} /*switch*/
	
	shellupdatescrollbars (shellwindowinfo);
	
	shellsetselectioninfo (); /*force ruler update*/
	
	shellpopglobals ();
	
	return (fl);
	
	error:
	
	if (errornum != 0) /*get error string*/
		getstringlist (wperrorlist, errornum, bserror);
	
	return (false);
	} /*wpfunctionvalue*/


static boolean wpclose (void) {
	
	/*
	6.18.97 dmb: no more in-memory packed state. doesn't work well 
	with file based objects.
	*/
	
	register hdlwprecord hwp = wpdata;
	register hdlwpvariable hv;
	// Handle hpackedwp;
	
	if (hwp == nil) //already closed & disposed?
		return (true);
	
	hv = (hdlwpvariable) (**hwp).wprefcon;
	
	langexternalwindowclosed ((hdlexternalvariable) hv);
	
	killundo (); /*must toss undos before they're stranded*/
	
	wpverbcheckwindowrect (hwp);
	
	if ((**hwp).fldirty || isfilewindow (shellwindow)) {
		
		/*
		if (wppack (&hpackedwp)) { // pack wp doc. the window is going away.
			
			wpdispose (); // no longer need the unpacked version
			
			(**hv).flpacked = true;
			
			(**hv).variabledata = (long) hpackedwp;
			}
		else
		*/
		(**hwp).flwindowopen = false; /*not in a window anymore*/
		}
	else {
		// assert ((**hv).oldaddress != nildbaddress);
		
		wpverbunload (hv);
		
		wpdata = nil;
		
		return (true);
		}
	
	return (true);
	} /*wpclose*/


static boolean wpinitverbs (void) {
	
	return (loadfunctionprocessor (idwpverbs, &wpfunctionvalue));
	} /*wpinitverbs*/


static boolean dosearch (boolean flfromtop, boolean flwrap, boolean *fltempload) {
	
	/*
	4.1b12 dmb: handle replaceall wraparound when replacement string contains 
	search string. also, check for cmd-period
	
	5.0.2b18 dmb: fixed flwrap case when matches were close together.
	*/
	
	register boolean fl;
	long startsel, endsel;
	long origendsel, searchstart;
	short diff;
	boolean flwrapped = false;
	
	if (searchparams.flreplaceall && flwrap) {/*4.1b12*/
	
		wpgetselection (&startsel, &endsel);
		
		origendsel = endsel;
		
		if (searchparams.ctreplaced > 0) /*there was a selection, already replaced*/
			origendsel -= stringlength (searchparams.bsreplace);
		}
	
	while (true) {
		
		searchstart = endsel; /*4.1b12*/
		
		fl = wpsearch (flfromtop, !searchparams.flunicase, searchparams.flwholewords, flwrap);
		
		if (!fl)
			break;
		
		*fltempload = false;
		
		if (!searchparams.flreplaceall)
			break;
		
		if (flwrap) { /*4.1b12*/
		
			wpgetselection (&startsel, &endsel);
			
			if (startsel < searchstart) { /*wrapped*/
				
				if (flwrapped) /*wrapped before; we must be done*/
					goto overlapped;
				
				flwrapped = true;
				}
			
			if (flwrapped) {
				
				if (endsel > origendsel) /*wrapped beyond original search start*/
					goto overlapped;
				
				diff = stringlength (searchparams.bsreplace) - (endsel - startsel);
				
				endsel += diff; /*adjust current sel end for next searchstart*/
				
				origendsel += diff; /*adjust orig start*/
				}
			}
		
		if (!wpinsert (searchparams.bsreplace))
			break;
		
		endsel = startsel + stringlength (searchparams.bsreplace); //keep in synch
		
		++searchparams.ctreplaced;
		
		if (keyboardescape ()) /*user pressed cmd-period -- stop visiting*/
			return (false);
	
		flfromtop = false;
		}
	
	return (fl);
	
	overlapped: {
		
		wpsetselection (searchstart, searchstart); /*go back to last replacement*/
		
		return (false);
		}
	} /*dosearch*/


boolean wpverbfind (hdlexternalvariable hvariable, boolean *flzoom) {
	
	register hdlwpvariable hv = (hdlwpvariable) hvariable;
	register hdlwprecord hwp;
	register boolean fl;
	boolean fltempload;
	boolean flwindowopen;
	hdlwindowinfo hinfo;
	boolean flfromtop = true;
	
	fltempload = !(**hv).flinmemory;
	
	if (!wpverbinmemory ((hdlexternalvariable) hv))
		return (false);
	
	hwp = (hdlwprecord) (**hv).variabledata;
	
	flwindowopen = wpwindowopen ((hdlexternalvariable) hv, &hinfo);
	
	if (flwindowopen)
		shellpushglobals ((**hinfo).macwindow);
	
	else {
		shellpushglobals (nil);
		
		wppushdata (hwp);
		}
	
	fl = dosearch (true, false, &fltempload);
	
	if (fltempload && !fl && !(**hwp).fldirty)
		wpverbunload (hv);
	
	if (!flwindowopen)
		wppopdata ();
	
	shellpopglobals ();
	
	return (fl);
	} /*wpverbfind*/


static boolean wpverbsearch (void) {
	
	/*
	2/13/92 dmb: use new dosearch to handle replace all
	*/
	
	long wprefcon = (**wpdata).wprefcon;
	boolean fltempload = false;
	
	startingtosearch (wprefcon);
	
	/*
	if (wpsearch (searchparams.bsfind, false, !searchparams.flunicase, searchparams.flwholewords, searchshouldwrap (wprefcon)))
		return (true);
	*/
	
	if (dosearch (false, searchshouldwrap (wprefcon), &fltempload))
		return (true);
	
	if (!searchshouldcontinue (wprefcon))
		return (false);
	
	return (langexternalcontinuesearch ((hdlexternalvariable) wprefcon));
	} /*wpverbsearch*/


static boolean wpverbruncursor (void) {
	
	/*
	9/26/91 dmb: work with handles, not strings
	*/
	
	bigstring bsresult;
	long startsel, endsel;
	Handle htext;
	
	wpgetselection (&startsel, &endsel);
	
	if (startsel == endsel)
		wpselectline ();
	
	if (!wpgetseltexthandle (&htext))
		return (false);
	
	return (langrunhandle (htext, bsresult)); /*consumes htext*/
	
	/*
	if (startsel == endsel)
		wpsetselection (0, 255);
	
	wpgetseltext (bs);
	
	return (langrunstring (bs, bsresult));
	*/
	
	} /*wpverbruncursor*/


static boolean wpverbtitleclick (Point pt) {
	
	return (langexternaltitleclick (pt, (hdlexternalvariable) (**wpdata).wprefcon));
	} /*wpverbtitleclick*/


static boolean wpverbgetvariable (hdlexternalvariable *hvariable) {
	
	if (wpdata == nil)
		return (false);
	
	*hvariable = (hdlexternalvariable) (**wpdata).wprefcon;
	
	return (true);
	} /*wpverbgetvariable*/


static boolean wpverbgetwpdata (void) {
	
	return (true);
	} /*wpverbgetwpdata*/


static boolean wpverbgettargetdata (short id) {
	
	/*
	a verb is about to be executed that acts on the indicated external type, or 
	on any shell window if id == -1.
	
	return true if we can handle verbs of that type and are able to set the 
	relevant globals
	*/
	
	switch (id) {
		
		case -1:
			return (true);
		
		case idwordprocessor: /*don't need to do anything -- our wp globals are pushed*/
			return (true);
		
		default:
			return (false);
		}
	} /*wpverbgettargetdata*/


static boolean wpverbclick (Point pt, tyclickflags flags) {
	
	wpclick (pt, flags);
	
	if (keyboardstatus.ctmodifiers && mousestatus.fldoubleclick) {
		
		bigstring bs;
		
		wpgetseltext (bs);
		
		langzoomobject (bs);
		}
	
	return (true);
	} /*wpverbclick*/


static boolean wpverbkeystroke (void) {
	
	if (langexternalsurfacekey ((hdlexternalvariable) (**wpdata).wprefcon))
		return (true);
	
	return (wpkeystroke ());
	} /*wpverbkeystroke*/


static boolean wpverbpaste (void) {
	
	/*
	6.0b3 dmb: had to add this layer as wppaste gained the flstyles parameter
	*/
	
	return (wppaste (true));
	} /*wpverbpaste*/


boolean wpstart (void) {
	
	/*
	set up callback routines record, and link our data into the shell's 
	data structure.
	*/
	
	ptrcallbacks wpcallbacks;
	register ptrcallbacks cb;
	
	assert (sizeof (tyexternalvariable) == sizeof (tywpvariable));
	
	wpinitverbs ();
	
	shellpushscraphook (&wpscraphook);
	
	shellnewcallbacks (&wpcallbacks);
	
	cb = wpcallbacks; /*copy into register*/
	
	loadconfigresource (idwpconfig, &(*cb).config);
		
	(*cb).configresnum = idwpconfig;
		
	(*cb).windowholder = &wpwindow;
	
	(*cb).dataholder = (Handle *) &wpdata;
	
	(*cb).infoholder = &wpwindowinfo; 
	
	(*cb).initroutine = &wpinit;
	
	(*cb).quitroutine = &wpshutdown;
	
#ifdef version42orgreater
	
	(*cb).disposerecordroutine = ccdisposefilerecord;
	
	(*cb).saveroutine = ccsavespecialfile;

#endif
	
	(*cb).closeroutine = &wpclose;
	
	(*cb).titleclickroutine = &wpverbtitleclick;
	
	(*cb).searchroutine = &wpverbsearch;
	
	(*cb).executeroutine = &wpverbruncursor;
	
	(*cb).getvariableroutine = &wpverbgetvariable;
	
	(*cb).updateroutine = &wpupdate;
	
	(*cb).activateroutine = &wpactivate;
	
	(*cb).getcontentsizeroutine = &wpgetcontentsize;
	
	(*cb).resizeroutine = &wpresize;
	
	(*cb).scrollroutine = &wpscroll;
	
	(*cb).setscrollbarroutine = &wpresetscrollbars;
	
	(*cb).mouseroutine = &wpverbclick;
	
	(*cb).keystrokeroutine = &wpverbkeystroke;
	
	/*
	(*cb).undoroutine = &wpundo;
	*/
	
	(*cb).cutroutine = &wpcut;
	
	(*cb).copyroutine = &wpcopy;
	
	(*cb).pasteroutine = &wpverbpaste;
	
	(*cb).clearroutine = &wpclear;
	
	(*cb).selectallroutine = &wpselectall;
	
	(*cb).fontroutine = &wpsetfont;
	
	(*cb).sizeroutine = &wpsetsize;
	
	(*cb).styleroutine = &wpsetstyle;
	
	(*cb).leadingroutine = &wpsetleading;
	
	(*cb).justifyroutine = &wpsetjustify;
	
	(*cb).setselectioninforoutine = &wpsetselectioninfo;
	
	(*cb).pagesetuproutine = &wppagesetup;
	
	(*cb).beginprintroutine = &wpbeginprint;
	
	(*cb).endprintroutine = &wpendprint;
	
	(*cb).printroutine = &wpprint;
	
	(*cb).setprintinfoproutine = &wpsetprintinfo;
	
	(*cb).idleroutine = &wpidle;
	
	(*cb).adjustcursorroutine = &wpadjustcursor;
	
	(*cb).gettargetdataroutine = &wpverbgettargetdata;
	
	return (true);
	} /*wpstart*/

#endif // !flruntime



