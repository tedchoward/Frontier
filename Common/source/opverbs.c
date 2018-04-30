
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

#include "frontierconfig.h"
#include "memory.h"
#include "strings.h"
#include "quickdraw.h"
#include "font.h"
#include "cursor.h"
#include "ops.h"
#include "resources.h"
#include "search.h"
#include "threads.h"
#include "shell.h"
#include "shellbuttons.h"
#include "shellhooks.h"
#include "shellmenu.h"
#include "shellundo.h"
#include "lang.h"
#include "langexternal.h"
#include "langinternal.h"
#include "langxml.h" //7.1b44 dmb
#include "tablestructure.h"
#include "tableverbs.h"
#include "process.h"
#include "op.h"
#include "opinternal.h"
#include "opverbs.h"
#include "oplist.h" //6.2a6 AR
#include "langsystem7.h" //6.2a6 AR
#include "scripts.h"
#include "cancoon.h"
#include "kernelverbdefs.h"
#include "opxml.h" // 7.0b21 PBS
#include "wpengine.h"
#include "opbuttons.h"
#include "file.h" // 2006-09-17 creedon



	#include "osacomponent.h"
	


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
#define rejecttablenum 6 /*7.0b23 PBS*/


typedef enum tyoptoken { /*verbs that are processed by op.c*/
	
	linetextfunc,
	
	levelfunc,
	
	countsubsfunc,
	
	countsummitsfunc,
	
	gofunc,
	
	firstsummitfunc,
	
	expandfunc,
	
	collapsefunc,
	
	getexpandedfunc,

	insertfunc,
	
	findfunc,
	
	sortfunc,
	
	setlinetextfunc,
	
	reorgfunc,
	
	promotefunc,
	
	demotefunc,
	
	hoistfunc,
	
	dehoistfunc,
	
	/*
	getmarkedfunc,
	
	setmarkedfunc,
	
	clearmarkedfunc,
	*/
	
	deletesubsfunc,
	
	deletelinefunc,
	
	tabkeyreorgfunc,
	
	flatcursorkeysfunc,
	
	getdisplayfunc,
	
	setdisplayfunc,
	
	getcursorfunc,
	
	setcursorfunc,
	
	getrefconfunc,
	
	setrefconfunc,
	
	getexpansionstatefunc,
	
	setexpansionstatefunc,
	
	getscrollstatefunc,
	
	setscrollstatefunc,
	
	getsuboutlinefunc,
	
	insertoutlinefunc,
	
	setmodifiedfunc, /*7.0b5 PBS: new verb for setting dirty bit of outline*/
	
	getselectfunc, /*7.0b15 PBS: verb for getting selection as list of cursors*/
	
	getheadnumberfunc, /*7.0b17: one-based index of the current node*/
	
	visitallfunc, /*7.0b17: visit every headline in an outline*/
	
	getselectedsuboutlinesfunc, /*7.0b18: get selected headlines as an outline*/
	
	xmltooutlinefunc, /*7.0b21: XML outlineDocument to outline*/
	
	outlinetoxmlfunc, /*7.0b21: outline to XML outlineDocument*/

	sethtmlformattingfunc, /*7.0b28: turn on and off HTML semi-wizzy formatting*/

	gethtmlformattingfunc, /*7.0b28: return true if HTML semi-wizzy formatting is on*/

	setdynamicfunc, /*7.0b30: set the dynamic flag of a headline record*/

	getdynamicfunc, /*7.0b30: get the dynamic flag of a headline record*/
	
	/*opattributes verbs start here*/ /*7.0b16 PBS*/
	
	addgroupfunc,
	
	getallfunc,
	
	getonefunc,
	
	makeemptyfunc,
	
	setonefunc,

	/*script verbs start here*/
	
	compilefunc,
	
	uncompilefunc,
	
	getcodefunc,
			
	gettypefunc,
	
	settypefunc,

	makecommentfunc,
	
	uncommentfunc,
	
	iscommentfunc,
	
	getbreakpointfunc,
	
	setbreakpointfunc,
	
	clearbreakpointfunc,
	
	startprofilefunc,
	
	stopprofilefunc,
	
	getosacodefunc,
	
	getosasourcefunc,
	
	ctscriptverbs
	
	} tyoptoken;




#pragma pack(2)
	typedef struct tyoutlinevariable {
		
		unsigned short id; /*tyexternalid: managed by langexternal.c*/
		
		unsigned short flinmemory: 1; /*if true, variabledata is an hdloutlinerecord, else a dbaddress*/
		
		unsigned short flmayaffectdisplay: 1; /*not in memory, but being displayed in a table window*/
		
		unsigned short flscript: 1; /*outlines and scripts are identical, except for this bit*/
		
		long variabledata; /*either a hdloutlinerecord or a dbaddress*/
		
		hdldatabaserecord hdatabase; // 5.0a18 dmb

		dbaddress oldaddress; /*last place this outline was stored in db*/
		
		Handle linkedcode; /*you can link code into any outline, mostly for scripts though*/
		} tyoutlinevariable, *ptroutlinevariable, **hdloutlinevariable;
#pragma options align=reset




static short errornum = 0;


static hdloutlinerecord outlinedataholder = nil;

static void opverbresize (void); /*forward*/


static boolean opverbsetglobals (void) {
	
	/*
	5.1.5b7 dmb: added this layer so we can trap the setting of outlinedata
	*/
	
	opsetoutline (outlinedataholder);
	
	return (opeditsetglobals ());
	} /*opverbsetglobals*/




boolean opverbgettypestring (hdlexternalvariable hvariable, bigstring bs) {
	
	register hdloutlinevariable hv = (hdloutlinevariable) hvariable;
	short id;
	
	if ((**hv).flscript)
		id = scripttypestring;
	else
		id = optypestring;
	
	getstringlist (opstringlist, id, bs);
	
	return (true);
	} /*opverbgettypestring*/


static boolean opverbdisposecode (hdloutlinevariable hvariable) {
	
	/*
	5/19/92 dmb: attempt to fix 1.0 crashing bug: if we dispose code that is 
	running, but isn't the main routine (i.e. it's been called by another script), 
	we crash.  we needed a call to processcodedisposed here, but didn't have 
	a pointer back to the node that produced the code, which is the refcon that 
	processcodedisposed expects.  scriptgetcode now stashes this link in the nodeval 
	at the top of the code tree, just as langaddhandler does to maintain the source 
	position when executing local procedures.  this ought to work.
	*/
	
	register hdloutlinevariable hv = hvariable;
	register hdltreenode hcode = (hdltreenode) (**hv).linkedcode;
	
	if (hcode != nil) {
		
		if (!processdisposecode (hcode)) { /*process manager didn't handle it*/
			
			processcodedisposed ((**hcode).nodeval.data.longvalue); /*5/19/92 dmb*/
			
			langdisposetree (hcode);
			}
		
		(**hv).linkedcode = nil;
		}
	
	return (true);
	} /*opverbdisposecode*/


static boolean opdisposevariable (hdlexternalvariable hvariable, boolean fldisk) {
	
	register hdloutlinevariable hv = (hdloutlinevariable) hvariable;
	
	opdisposeoutline ((hdloutlinerecord) (**hv).variabledata, fldisk);
	
	return (true);
	} /*opdisposevariable*/


boolean opverbdispose (hdlexternalvariable hvariable, boolean fldisk) {
	
	register hdlexternalvariable hv = hvariable;
	
	opverbdisposecode ((hdloutlinevariable) hv); /*must check for code even if outline isn't in memory*/
	
	return (langexternaldisposevariable (hv, fldisk, &opdisposevariable));
	} /*opverbdispose*/


static boolean newoutlinevariable (boolean flinmemory, long variabledata, hdloutlinevariable *h) {
	
	tyoutlinevariable item;
	
	clearbytes (&item, sizeof (item));
	
	item.flinmemory = flinmemory;
	
	item.variabledata = variabledata;
	
	item.hdatabase = databasedata; // 5.0a18 dmb

	return (newfilledhandle (&item, sizeof (item), (Handle *) h));
	} /*newoutlinevariable*/


void opverbunload (hdlexternalvariable hvariable, dbaddress adr) {
	
	register hdloutlinevariable hv = (hdloutlinevariable) hvariable;
	register hdloutlinerecord ho = (hdloutlinerecord) (**hv).variabledata;
	
	assert ((**hv).flinmemory);
	
	// assert (adr != nildbaddress);
	
	opdisposeoutline (ho, false); /*reclaim memory used by outline*/
	
	if (adr == nildbaddress) { // 5.0d3 dmb: a file object, we're done with the variable
	
		disposehandle ((Handle) hv);
		}
	else {
		
		(**hv).flinmemory = false;
		
		(**hv).variabledata = adr;
		}
	} /*opverbunload*/


boolean opverblinkcode (hdlexternalvariable hvariable, Handle hcode) {
	
	/*
	we have access to the declaration of an outlinevariable, so we have to
	bottleneck any setting of the linked code field.  it's a good idea anyway
	because we may have to do surgery on the code before linking it.
	*/
	
	register hdloutlinevariable hv = (hdloutlinevariable) hvariable;
	
	(**hv).linkedcode = hcode;
	
	if ((**hv).flinmemory) {
	
		register hdloutlinerecord ho = (hdloutlinerecord) (**hv).variabledata;
		hdlwindowinfo hinfo;
		
		(**ho).flrecentlychanged = false;
		
		if ((**ho).flwindowopen && shellfinddatawindow ((Handle) ho, &hinfo)) {
			
			shellpushglobals ((**hinfo).macwindow);
			
			shellinvalbuttons ();
			
			shellpopglobals ();
			}
		}
	
	return (true);
	} /*opverblinkcode*/


boolean opverbgetlinkedcode (hdlexternalvariable hvariable, hdltreenode *hcode) {
	
	register hdltreenode h;
	
	*hcode = h = (hdltreenode) (**(hdloutlinevariable) hvariable).linkedcode;
	
	return (h != nil) ;
	} /*opverbgetlinkedcode*/


static void opverbsetcallbacks (hdloutlinevariable hvariable, hdloutlinerecord houtline) {
	
	/*
	register hdloutlinerecord ho = houtline;
	
	(**ho).setscrollbarsroutine = &opverbsetscrollbarsroutine;	// 1/31/97 dmb: default is ok now
	*/
	
	if ((**hvariable).flscript)
		scriptsetcallbacks (houtline);
	} /*opverbsetcallbacks*/


static void opverbcheckwindowrect (hdloutlinerecord houtline) {
	
	register hdloutlinerecord ho = houtline;
	hdlwindowinfo hinfo;
		
	if ((**ho).flwindowopen) { /*make windowrect reflect current window size & position*/
		
		if (shellfinddatawindow ((Handle) ho, &hinfo)) {
			
			Rect r;
			
			shellgetglobalwindowrect (hinfo, &r);
			
			if (!equalrects (r, (**ho).windowrect)) {
			
				(**ho).windowrect = r;
				
				(**ho).fldirty = true;
				}
			}
		}
	} /*opverbcheckwindowrect*/


boolean opwindowopen (hdlexternalvariable hvariable, hdlwindowinfo *hinfo) {
	
	/*
	5.0b9 dmb: for clipboard operations, must test ho for nil
	*/

	register hdloutlinevariable hv = (hdloutlinevariable) hvariable;
	register hdloutlinerecord ho;
	
	if (!(**hv).flinmemory)
		return (false);
	
	ho = (hdloutlinerecord) (**hv).variabledata;
	
	if (ho == nil)
		return (false);

	shellfinddatawindow ((Handle) ho, hinfo); /*3/19/91 DW*/
	
	
	return ((**ho).flwindowopen);
	} /*opwindowopen*/



static void opverbsetupoutline (hdloutlinerecord ho, hdloutlinevariable hv) {

	/*
	7.0d8 PBS: set structured text keys for outlines, not just odb outlines.
	When an outline is created, it's in text mode, not bar cursor mode,
	so you can actually start typing right away.

	7.0b33 PBS: For non-outline outlines, explicitly set flhtml to false.
	Fixes a bug where scripts may get HTML formatting.
	*/

	(**ho).outlinerefcon = (long) hv; /*point back to variable*/
	
	if ((**hv).flscript)
		(**ho).outlinetype = outlineisdatabasescript;
	else
		(**ho).outlinetype = outlineisoutline;
	
	(**ho).flfatheadlines = true;

	(**ho).flhorizscrolldisabled = true; /*7.0b26 PBS: no horizontal scrolling.*/

#ifdef PIKExxx /*7.1b29 PBS: this "feature" has been rolled back.*/

	/*7.0d8 PBS: type characters to find headline.*/

	(**ho).flstructuredtextkeys = true; 

#endif
	
	if ((**ho).outlinetype == outlineisoutline)
		(**ho).flhtml = true;
	else
		(**ho).flhtml = false; /*7.0b33 PBS: make sure HTML formatting is off for scripts.*/
	
	opverbsetcallbacks (hv, ho);
	} /*opverbsetupoutline*/


static boolean opverbpackoutline (hdloutlinerecord houtline, Handle *hpacked) {
	
	*hpacked = nil;
	
	return (oppackoutline (houtline, hpacked));
	} /*opverbpackoutline*/


static boolean opverbinmemory (hdloutlinevariable hv) {

	/*
	5.0a18 dmb: support database linking
	*/
	
	hdloutlinerecord ho;
	register boolean fl;
	dbaddress adr;
	Handle hpackedoutline;
	long ix = 0;
	
	if ((**hv).flinmemory) /*nothing to do, it's already in memory*/
		return (true);
	
	dbpushdatabase ((**hv).hdatabase);

	adr = (dbaddress) (**hv).variabledata;
	
	fl = dbrefhandle (adr, &hpackedoutline);
	
	if (fl) {

		fl = opunpack (hpackedoutline, &ix, &ho);
		
		disposehandle (hpackedoutline);
		}

	dbpopdatabase ();
	
	if (!fl) 
		return (false);
	
	(**hv).flinmemory = true;
	
	(**hv).variabledata = (long) ho; /*link into variable structure*/
	
	(**hv).oldaddress = adr; /*last place this outline was stored*/
	
	opverbsetupoutline (ho, hv);
	
	return (true);
	} /*opverbinmemory*/


boolean opverbmemorypack (hdlexternalvariable h, Handle *hpacked) {
	
	/*
	a special entrypoint for the language's pack and upack verbs.
	*/
	
	register hdloutlinevariable hv = (hdloutlinevariable) h;
	register hdloutlinerecord ho;
	boolean fl;
	Handle hpush = nil;
	
	if (!(**hv).flinmemory) {
	
		if (!langexternalrefdata ((hdlexternalvariable) hv, &hpush))
			return (false);
		
		goto pushandexit;
		}
	
	ho = (hdloutlinerecord) (**hv).variabledata;
	
	opverbcheckwindowrect (ho);
	
	if (!opverbpackoutline (ho, &hpush))
		return (false);
	
	pushandexit:
	
	if (hpush == nil)
		return (false);
	
	fl = pushhandle (hpush, *hpacked);
	
	disposehandle (hpush);
	
	return (fl);
	} /*opverbmemorypack*/


boolean opverbmemoryunpack (Handle hpacked, long *ixload, hdlexternalvariable *hvariable) {
	
	/*
	xxx [create a new outline variable -- not in memory.]
	
	12/14/90 dmb: not good...
	can't create the variable on disk, because the database doesn't have 
	provisions for managing unsaved data on disk.  that is, there's no 
	mechanism to free the bytes allocated if the user doesn't save 
	changes (or the unpack was for temporary copy).  such a mechanism 
	could be added, but in the meanwhile...
	
	create a new outline variable, in memory.
	
	this is a special entrypoint for the pack and unpack verbs.
	
	5.0.2b12 dmb: if ixload is nil, we just unpack hpacked directly, and consume it.
	*/
	
	hdloutlinevariable *h = (hdloutlinevariable *) hvariable;
	Handle hpackedop;
	hdloutlinerecord ho;
	boolean fl;
	long ix = 0;
	
	if (ixload == nil)
		hpackedop = hpacked;
	else
		if (!loadhandleremains (*ixload, hpacked, &hpackedop))
			return (false);
	
	/*
	fl = dballochandle (hpackedop, &adr);
	*/
	
	fl = opunpack (hpackedop, &ix, &ho);
	
	disposehandle (hpackedop);
	
	if (!fl)
		return (false);
	
	if (!newoutlinevariable (true, (long) ho, h)) {
		
		opdisposeoutline (ho, false);
		
		return (false);
		}
	
	opverbsetupoutline (ho, *h);
	
	(**ho).fldirty = true; /*never been saved, can't be clean*/
	
	return (true);
	} /*opverbmemoryunpack*/


boolean opverbscriptmemoryunpack (Handle hpacked, long *ixload, hdlexternalvariable *hvariable) {
	
	/*
	4/24/92 dmb: must call opverbsetcallbacks here, after setting flscript to 
	true.  this should explain while cmd-click wouldn't always toggle breakpoints.
	*/
	
	register hdloutlinevariable hv;
	hdloutlinerecord ho;
	
	if (!opverbmemoryunpack (hpacked, ixload, hvariable))
		return (false);
	
	hv = (hdloutlinevariable) *hvariable;
	
	ho = (hdloutlinerecord) (**hv).variabledata;
	
	(**hv).flscript = true;
	
	opverbsetupoutline (ho, hv);
	
	/*
	(**ho).outlinetype = outlineisdatabasescript;
	
	(**hv).flscript = true;
	
	opverbsetcallbacks (hv, ho);
	*/
	
	return (true);
	} /*opverbscriptmemoryunpack*/


boolean opverbpack (hdlexternalvariable h, Handle *hpacked, boolean *flnewdbaddress) {

	/*
	6.2a15 AR: added flnewdbaddress parameter
	*/
	
	register hdloutlinevariable hv = (hdloutlinevariable) h;
	register hdloutlinerecord ho;
	register boolean fl;
	Handle hpackedoutline;
	dbaddress adr;
	hdlwindowinfo hinfo;
	boolean fltempload = false;
	
	if (!(**hv).flinmemory) { /*simple case, outline is resident in the db*/
		
		if (flconvertingolddatabase) {
			
			if (!opverbinmemory (hv))
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
	
	ho = (hdloutlinerecord) (**hv).variabledata;
	
	opverbcheckwindowrect (ho);
	
	adr = (**hv).oldaddress; /*place where this outline used to be stored*/
	
	if (!fldatabasesaveas && !(**ho).fldirty && !(**ho).fldirtyview) /*don't need to update the db version of the outline*/
		goto pushaddress;
	
	if (!opverbpackoutline (ho, &hpackedoutline))
		return (false);
	
	fl = dbassignhandle (hpackedoutline, &adr);
	
	disposehandle (hpackedoutline);
	
	if (!fl)
		return (false);
	
	if (fldatabasesaveas && !fltempload)
		goto pushaddress;
	
	if (!opwindowopen ((hdlexternalvariable) hv, &hinfo) /* (**ho).flwindowopen*/ ) /*it's been saved, we can reclaim some memory*/
		opverbunload ((hdlexternalvariable) hv, adr);
	
	else {
		
		assert (!fltempload);
		
		(**ho).fldirty = false; /*we just saved off a new db version*/
		
		(**ho).fldirtyview = false;
		
		if (hinfo != nil)
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
	} /*opverbpack*/
	
	
boolean opverbunpack (Handle hpacked, long *ixload, hdlexternalvariable *hvariable) {

	dbaddress adr;
	
	if (!loadlongfromdiskhandle (hpacked, ixload, &adr)) 
		return (false);
		
	return (newoutlinevariable (false, adr, (hdloutlinevariable *) hvariable));
	} /*opverbunpack*/


boolean opverbscriptunpack (Handle hpacked, long *ixload, hdlexternalvariable *hvariable) {
	
	register hdloutlinevariable hv;
	
	if (!opverbunpack (hpacked, ixload, hvariable))
		return (false);
	
	hv = (hdloutlinevariable) *hvariable;
	
	(**hv).flscript = true;
	
	return (true);
	} /*opverbscriptunpack*/


boolean opverbgetlangtext (hdlexternalvariable hvariable, boolean flpretty, Handle *htext, long *signature) {
	
	/*
	10/7/91 dmb: if we have to load the outline to get it's text, release 
	the outline structure when we're done.  our caller is going to compile 
	the code and link it into the variable; there typically won't be any 
	further need for the outline itself.
	
	2.1b12 dmb: if htext is nil, just return the signature
	*/
	
	register hdloutlinevariable hv = (hdloutlinevariable) hvariable;
	register hdloutlinerecord ho;
	register boolean fltempload;
	boolean fl;
	
	fltempload = !(**hv).flinmemory;
	
	if (!opverbinmemory (hv))
		return (false);
	
	ho = (hdloutlinerecord) (**hv).variabledata;
	
	*signature = (**ho).outlinesignature;
	
	if (htext == nil)
		fl = true;
	else
		fl = opgetlangtext (ho, flpretty, htext);
	
	if (fltempload)
		opverbunload ((hdlexternalvariable) hv, (**hv).oldaddress);
	
	return (fl);
	} /*opverbgetlangtext*/



boolean opverbgetsize (hdlexternalvariable hvariable, long *size) {
	
	register hdloutlinevariable hv = (hdloutlinevariable) hvariable;
	register hdloutlinerecord ho;
	register long ctheads;
	
	if (!opverbinmemory (hv))
		return (false);
	
	ho = (hdloutlinerecord) (**hv).variabledata;
	
	oppushoutline (ho);
	
	ctheads = opcountheads ();
	
	oppopoutline ();
	
	*size = (long) ctheads;
	
	return (true);
	} /*opverbgetsize*/


boolean opverbgetsummitstring (hdlexternalvariable h, bigstring bs) {
	
	/*
	get string for table display.  variable is in memory
	*/
	
	long ctheads;
	
	if (!opverbgetsize (h, &ctheads))
		return (false);
	
	parsenumberstring (opstringlist, opsizestring, ctheads, bs);
	
	/*
	numbertostring (ctheads, bs);
	
	if (ctheads != 1)
		pushstring ("\p lines", bs);
	else
		pushstring ("\p line", bs);
	*/
	
	return (true);
	} /*opverbgetsummitstring*/
	

boolean opverbisdirty (hdlexternalvariable hvariable) {
	
	/*
	8/11/92 dmb: check windowrect now to ensure consistent results
	*/
	
	register hdloutlinevariable hv = (hdloutlinevariable) hvariable;
	register hdloutlinerecord ho;
	
	if (!(**hv).flinmemory)
		return (false);
	
	ho = (hdloutlinerecord) (**hv).variabledata;
	
	opverbcheckwindowrect (ho);
	
	return ((**ho).fldirty || (**ho).fldirtyview);
	} /*opverbisdirty*/


boolean opverbsetdirty (hdlexternalvariable hvariable, boolean fldirty) {
	
	/*
	4/15/92 dmb: see comments in langexternalsetdirty
	*/
	
	register hdloutlinevariable hv = (hdloutlinevariable) hvariable;
	register hdloutlinerecord ho;
	
	if (!opverbinmemory (hv))
		return (false);
	
	ho = (hdloutlinerecord) (**hv).variabledata;
	
	(**ho).fldirty = (**ho).fldirtyview = (**ho).flrecentlychanged = fldirty;
	
	return (true);
	} /*opverbsetdirty*/


boolean opverbpacktotext (hdlexternalvariable h, Handle htext) {
	
	/*
	12/13/91 dmb: now that opgetlangtext can make it pretty, use it 
	when exporting scripts
	
	5.0.2b20 dmb: unload if just loaded
	*/
	
	register hdloutlinevariable hv = (hdloutlinevariable) h;
	register hdloutlinerecord ho;
	register boolean fl;
	Handle hprogram;
	boolean fltempload = !(**hv).flinmemory;
	
	if (!opverbinmemory (hv))
		return (false);
	
	ho = (hdloutlinerecord) (**hv).variabledata;
	
	if ((**hv).flscript) {
		
		fl = opgetlangtext (ho, true, &hprogram);
		
		if (fl) {
			
			fl = pushhandle (hprogram, htext);
			
			disposehandle (hprogram);
			}
		}
	else
		fl = opoutlinetotextscrap (ho, false, htext);
	
	if (fltempload)
		opverbunload ((hdlexternalvariable) hv, (**hv).oldaddress);
	
	return (fl);
	} /*opverbpacktotext*/


boolean opverbgettimes (hdlexternalvariable h, long *timecreated, long *timemodified) {
	
	register hdloutlinevariable hv = (hdloutlinevariable) h;
	register hdloutlinerecord ho;
	
	if (!opverbinmemory (hv))
		return (false);
	
	ho = (hdloutlinerecord) (**hv).variabledata;
	
	*timecreated = (**ho).timecreated;
	
	*timemodified = (**ho).timelastsave;
	
	return (true);
	} /*opverbgettimes*/


boolean opverbsettimes (hdlexternalvariable h, long timecreated, long timemodified) {
	
	register hdloutlinevariable hv = (hdloutlinevariable) h;
	register hdloutlinerecord ho;
	
	if (!opverbinmemory (hv))
		return (false);
	
	ho = (hdloutlinerecord) (**hv).variabledata;
	
	(**ho).timecreated = timecreated;
	
	(**ho).timelastsave = timemodified;
	
	return (true);
	} /*opverbsettimes*/


boolean opverbnew (short id, Handle hdata, hdlexternalvariable *hvariable) {
	
	/*
	9/6/91 dmb: now accept hdata parameter as an outline from which to copy 
	headline and formatting data.
	
	12/29/91 dmb: added call to opsetctexpanded()
	
	2.1b11 dmb: if source outline is given in hdata, grab its outlinesigature
	
	5.0.2b13 dmb: write the edit buffer of the source outline
	
	5.1.5b9 dmb: preserve barcursor in copied outline
	
	2006-03-03 aradke: if the source outline is hoisted, temporarily unhoist it
		in order to copy all lines. this fixes data loss when making an in-memory
		copy of an outline and it restores pre-5.0.2b12 behavior. until then,
		obverbcopyvalue used pack/unpack to make a copy which preserves hoisted lines.
		http://sourceforge.net/tracker/index.php?func=detail&aid=1259245&group_id=120666&atid=687798
	*/
	
	register hdloutlinevariable hv;
	register hdloutlinerecord ho;
	hdlheadrecord hsummit = nil;
	hdloutlinerecord houtline;
	
	if (!newoutlinevariable (true, 0L, (hdloutlinevariable *) hvariable))
		return (false);
	
	hv = (hdloutlinevariable) *hvariable; /*copy into register*/
	
	if (!newoutlinerecord (&houtline)) {
		
		disposehandle ((Handle) hv);
		
		return (false);
		}
	
	ho = houtline; /*copy into register*/
	
	if (hdata != nil) {
		
		register hdloutlinerecord hsource = (hdloutlinerecord) hdata;
		long lnumcursor;
		boolean flpoppedhoists;
		
		oppushoutline (hsource);
		
			opwriteeditbuffer ();
			
			flpoppedhoists = oppopallhoists (); /*2006-03-03 aradke: temporarily unhoist before getting cursor position*/
		
			opgetnodeline ((**hsource).hbarcursor, &lnumcursor);
		
		oppopoutline ();
		
		oppushoutline (ho);
		
			opcopysiblings ((**hsource).hsummit, &hsummit);
			
			if (hsummit == nil) {
				
				oppopoutline ();
				
				opdisposeoutline (ho, false);
				
				disposehandle ((Handle) hv);
				
				return (false);
				}
			
			opsetsummit (ho, hsummit);
			
			opcopyformatting (hsource, ho);
			
			(**ho).outlinesignature = (**hsource).outlinesignature;
			
			opsetctexpanded (ho);
				
			(**ho).hbarcursor = oprepeatedbump (flatdown, lnumcursor, hsummit, true);
	
		oppopoutline ();
		
		if (flpoppedhoists) {	/*2006-03-03 aradke*/
			
			oppushoutline (hsource);
			
			oprestorehoists ();
			
			oppopoutline ();
			}
		}
	
	(**hv).variabledata = (long) ho; /*link the outline rec into the variable rec*/
	
	if (id == idscriptprocessor) {
	
		(**hv).flscript = true;
		
		if (hdata == nil) { //set the default font for new scripts
			
			shellgetconfig (idscriptconfig, &config);
			
			(**ho).fontnum = config.defaultfont;
			
			(**ho).fontsize = config.defaultsize;
			}
		}
	
	opverbsetupoutline (ho, hv); /*must be called after variable.flscript is set*/
	
	(**ho).fldirty = true; /*needs to be saved*/
	
	return (true);
	} /*opverbnew*/


boolean opverbcopyvalue (hdlexternalvariable hsource, hdlexternalvariable *hcopy) {
	
	/*
	5.0.2b12 dmb: new routine, so we don't have to pack/unpack
	*/
	
	register hdloutlinevariable hv = (hdloutlinevariable) hsource;
	dbaddress adr;
	Handle hpackedoutline;
	boolean fl;
	
	if ((**hv).flinmemory) { //in memory, make a new variable from the outline
		
		fl = opverbnew ((**hv).id, (Handle) (**hv).variabledata, hcopy);
		}
	else { //not in memory, unpack into new variable
		
		dbpushdatabase ((**hv).hdatabase);
		
		adr = (dbaddress) (**hv).variabledata;
		
		fl = dbrefhandle (adr, &hpackedoutline);
		
		dbpopdatabase ();
		
		if (!fl) 
			return (false);
		
		if ((**hv).flscript)
			fl = opverbscriptmemoryunpack (hpackedoutline, nil, hcopy);
		else
			fl = opverbmemoryunpack (hpackedoutline, nil, hcopy);
		}
	
	if (fl)
		(***hcopy).id = (**hv).id;
	
	return (fl);
	} /*opverbcopyvalue*/




static boolean getscriptparam (hdltreenode hfirst, short pnum, hdloutlinevariable *hv) {
	
	/*
	the caller wants an outline parameter.  we only accept them as "var"
	parameters, we just want the name of a variable that holds an outline.
	
	this keeps lang.c from having to know too much about outlines, but 
	more importantly, keeps them from being automatically copied!  could
	be very wasteful of memory because these things can get very large.
	
	we return a handle to the outline record and the identifier we 
	referenced to get the outline.  the string may used to name a window,
	for example.
	
	2.1b12 dmb: we now get scripts only, not outlines
	*/
	
	short id;
	
	if (!langexternalgetexternalparam (hfirst, pnum, &id, (hdlexternalvariable *) hv)) {
		
		errornum = namenotscripterror;
		
		return (false);
		}
	
	if (id != idscriptprocessor) {
		
		errornum = namenotscripterror;
		
		return (false);
		}
	
	return (true);
	} /*getscriptparam*/


boolean getoutlinevalue (hdltreenode hfirst, short pnum, hdloutlinerecord *houtline) {
	
	/*
	5.0.2b15 dmb: public routline - get an outline or script parameter, passed by reference
	*/
	
	hdloutlinevariable hv;
	bigstring bserror;
	short id;
	
	if (!langexternalgetexternalparam (hfirst, pnum, &id, (hdlexternalvariable *) &hv) ||
		 ((id != idoutlineprocessor) && (id != idscriptprocessor))) {
		
		getstringlist (operrorlist, namenotoutlineerror, bserror);
		
		langerrormessage (bserror);
		
		return (false);
		}
	
	if (!opverbinmemory (hv))
		return (false);
	
	*houtline = (hdloutlinerecord) (**hv).variabledata;
	
	return (true);
	} /*getoutlinevalue*/



boolean opverbarrayreference (hdlexternalvariable hvariable, long ix, hdlheadrecord *hnode) {
	
	register hdloutlinevariable hv = (hdloutlinevariable) hvariable;
	register boolean fl;
	
	*hnode = nil;
	
	if (!opverbinmemory (hv)) /*couldn't swap into memory*/
		return (false);
		
	oppushoutline ((hdloutlinerecord) (**hv).variabledata); /*assume it's in memory*/
	
	fl = oparrayreference (ix, hnode);
	
	oppopoutline ();
	
	return (fl);
	} /*opverbarrayreference*/


boolean opedit (hdlexternalvariable hvariable, hdlwindowinfo hparent, ptrfilespec fs, bigstring bstitle, rectparam rzoom) {
	
	//
	// 2006-09-16 creedon: on Mac, set window proxy icon
	//
	// 5.0.2b6 dmb: added flwindowopen loop to handle Windows async overlap
	//
	// 5.0b18 dmb: further protection: set flwindowopen bit before newchildwindow, which will yield
	//
	// 1997-04-17 dmb: protect against globals smashing under Windows
	//
	
	register hdloutlinevariable hv = (hdloutlinevariable) hvariable;
	register hdloutlinerecord ho;
	Rect rwindow;
	WindowPtr w;
	hdlwindowinfo hi;
	short id;

	if (!opverbinmemory (hv)) // couldn't swap it into memory
		return (false);
	
	ho = (hdloutlinerecord) (**hv).variabledata; // assume it's in memory
	
	while ((**ho).flwindowopen) { // bring to front, return true
		
		if (shellfinddatawindow ((Handle) ho, &hi)) {
			
			if ((*rzoom).top > -2)
				shellbringtofront (hi);
			
			return (true);
			}
		
			break;
		}
	
	rwindow = (**ho).windowrect; // window comes up where it was last time
	
	if ((**hv).flscript)
		id = idscriptconfig;
	else
		id = idoutlineconfig;
	
	(**ho).flwindowopen = true;

	if (!newchildwindow (id, hparent, &rwindow, rzoom, bstitle, &w)) {
		
		(**ho).flwindowopen = false;

		return (false);
		}
	
	getwindowinfo (w, &hi);

	if ((**hv).flscript)	
		(**hi).flhidebuttons = false;	
	else
		(**hi).flhidebuttons = true;
	
	(**hi).flsyntheticbuttons = false;

	(**hi).hdata = (Handle) ho;
	
	if ( fs != nil ) {
	
		(**hi).fspec = *fs;
		
		
			FSRef fsref;
		
			if ( macgetfsref ( fs, &fsref ) == noErr ) {
			
				OSType type = 0;
			
				if ( ( **hv ).flscript ) {
				
					bigstring bs;
					LSItemInfoRecord iteminfo;
					OSStatus status;
					
					status = LSCopyItemInfoForRef ( &fsref, kLSRequestTypeCreator || kLSRequestExtension, &iteminfo );
						
					cfstringreftobigstring ( iteminfo.extension, bs );
					
					alllower ( bs );
					
					if ( equalstrings ( bs, BIGSTRING("\x04" "ftds") ) )
						type = 'FTds';
					
					if ( iteminfo.filetype == 'FTds' )
						type = 'FTds';
					
					if ( type == 0 )
						type = 'FTsc';
						
					}
				else
					type = 'FTop';
				
				SetWindowProxyCreatorAndType ( w, 'LAND', type, kOnSystemDisk );
				
				}
		
		}
	
	if (hparent == nil)
		(**ho).outlinetype = outlineisstandalonescript;
	
	(**ho).outlinerect = (**hi).contentrect;
	
	shellpushglobals (w);

	shelladjustaftergrow ((**hi).macwindow);
	
	opsetdisplaydefaults (ho);
	
	opdirtymeasurements (); // 6.0a14 dmb: need this too; may have bogus pre-display values
	
	opsetctexpanded (ho); // 6.0a12 dmb: now that we set fatheadlines and outlinerect
	
	oprestorescrollposition ();
	
	opresetscrollbars ();

	shellpopglobals ();
	
	if ((**ho).fldirty)
		shellsetwindowchanges (hi, true);
	
	if ((**ho).outlinetype == outlineisoutline) { // 7.1b18 PBS: hide synthetic buttons
		
		(**hi).flhidebuttons = true;

		(**hi).buttonlist = nil;
		} /*if*/

	windowzoom (w);

	return (true);
	
	} // opedit
	
	
boolean opvaltoscript (tyvaluerecord val, hdloutlinerecord *houtline) {
	
	/*
	called externally -- you give us a value that holds a script that's
	a tableprocessor variable, we'll return you a handle to the outline.
	*/
	
	register hdloutlinevariable hv;
	
	if (val.valuetype != externalvaluetype)
		return (false);
	
	hv = (hdloutlinevariable) val.data.externalvalue;
	
	if ((**hv).id != idscriptprocessor)
		return (false);
	
	if (!opverbinmemory (hv))
		return (false);
	
	*houtline = (hdloutlinerecord) (**hv).variabledata;
	
	return (true);
	} /*opvaltoscript*/

/*
static boolean oprunverb (hdltreenode hparam1, tyvaluerecord *v) {
	
	hdloutlinevariable hv;
	Handle htext;
	tyvaluerecord val;
	bigstring bs;
	
	setbooleanvalue (false, v); /%assume the worst%/
	
	flnextparamislast = true;
	
	if (!getscriptparam (hparam1, 1, bs, &hv))
		return (false);
	
	if (!opverbgetlangtext ((hdlexternalvariable) hv, &htext))
		return (false);
	
	return (langrun (htext, v));
	} /%oprunverb%/
*/


boolean opinserthandle (Handle htext, tydirection dir) {
	
	/*
	12/13/91 dmb: take text handle instead of string, and potentially 
	deposit an entire structure.
	
	2.1b9 dmb: use new isoutlinetext routine to save code
	*/
	
	register hdloutlinerecord ho = outlinedata;
	register hdlheadrecord hbarcursor;
	register boolean fl;
	hdlheadrecord hnode;
	boolean floutline;
	
	hbarcursor = (**ho).hbarcursor; /*copy into register*/
	
	if (dir == left) { /*special case: convert left to down from parent*/
		
		hbarcursor = (**hbarcursor).headlinkleft;
		
		if (hbarcursor == (**ho).hbarcursor) /*couln't move left*/
			return (false);
		
		opmoveto (hbarcursor);
		
		dir = down;
		}
	
	floutline = isoutlinetext (htext);
	
	if (floutline)
		if (!optexttooutline (ho, htext, &hnode))
			return (false);
	
	if (dir == right) { /*maybe we need to expand?*/
	
		if (!opsubheadsexpanded (hbarcursor))
			opexpand (hbarcursor, 1, false);
		}
	
	if (floutline) {
		
		fl = opinsertstructure (hnode, dir);
		
		if (!fl)
			opdisposestructure (hnode, false);
		}
	else {
		
		fl = copyhandle (htext, &htext);
		
		if (fl)
			fl = opinsertheadline (htext, dir, false);
		}
	
	if (fl)
		shellcheckdirtyscrollbars ();
	
	return (fl); 
	} /*opinserthandle*/


static boolean opsetlinetextverb (Handle htext) {
	
	/*
	6.0b4 dmb: for wrapping headline display, this is a struc change
	*/
	
	boolean fl;
	hdlheadrecord hcursor;
	hdlscreenmap hmap;
	
	opsettextmode (false);
	
	hcursor = (**outlinedata).hbarcursor;
	
	opbeforestrucchange (&hmap, false);
	
	fl = opsetheadtext (hcursor, htext); //consumes htext
	
	opgetlineheight (hcursor); //force recalc
	
	opafterstrucchange (hmap, false);
	
	if (fl)	
		opupdatenow ();
	
	return (fl);	
	} /*opsetlinetextverb*/

#pragma pack(2)
typedef struct tybitinfo {
	
	boolean flset;
	
	boolean flanychanged;
	} tybitinfo, *ptrbitinfo;
#pragma options align=reset


static boolean opsetbitverb (boolean fl, opvisitcallback visit) {
	
	/*
	9/22/92 dmb: shared code for opcommentverb and opbreakpoint verb
	*/
	
	tybitinfo bitinfo;
	
	bitinfo.flset = bitboolean (fl); /*make sure it's a 1 or a 0*/
	
	bitinfo.flanychanged = false;
	
	opvisitmarked (down, visit, &bitinfo);
	
	if (!bitinfo.flanychanged)
		return (false);
	
	opdirtyoutline ();
	
	opupdatenow ();
	
	return (true);
	} /*opsetbitverb*/


static boolean opcommentvisit (hdlheadrecord hnode, ptrvoid refcon) {
	
	hdlheadrecord h = hnode;
	ptrbitinfo bitinfo = (ptrbitinfo) refcon;
	
	if ((**h).flcomment != (*bitinfo).flset) { /*changing*/
		
		(**h).flcomment = (*bitinfo).flset;
		
		opinvalstructure (h);
		
		(*bitinfo).flanychanged = true;
		}
	
	return (true);
	} /*opcommentvisit*/


static boolean opcommentverb (boolean fl) {
	
	/*
	change the comment state of the cursor line.  only update if it 
	actually changed.  fl == true means make it a comment.
	
	9/22/92 dmb: visit all marked headlines
	*/
	
	return (opsetbitverb (fl, &opcommentvisit));
	} /*opcommentverb*/


static boolean opbreakpointvisit (hdlheadrecord hnode, ptrvoid refcon) {
	
	ptrbitinfo bitinfo = (ptrbitinfo) refcon;
	hdlheadrecord h = hnode;
	
	if ((**h).flbreakpoint != (*bitinfo).flset) { /*changing*/
		
		(**h).flbreakpoint = (*bitinfo).flset;
		
		opinvalnode (h);
		
		(*bitinfo).flanychanged = true;
		}
	
	return (true);
	} /*opbreakpointvisit*/


static boolean opbreakpointverb (boolean fl) {
	
	/*
	change the breakpoint state of the cursor line.  only update if it 
	actually changed.  fl == true means set a breakpoint.
	
	9/22/92 dmb: use new opsetbitverb to visit all marked headlines
	*/
	
	return (opsetbitverb (fl, &opbreakpointvisit));
	} /*opbreakpointverb*/


/*
static boolean opmarkedverb (boolean fl) {
	
	/%
	change the marked state of the cursor line.  only update if it 
	actually changed.  fl == true means set a mark.
	%/
	
	hdlheadrecord hcursor;
	
	fl = bitboolean (fl); /%make sure it's a 1 or a 0%/
	
	hcursor = (**outlinedata).hbarcursor;
	
	if ((**hcursor).flmarked == fl) /%not changing%/
		return (false);
	
	(**hcursor).flmarked = fl;
	
	opinvalstructure (hcursor);
	
	opdirtyoutline ();
	
	opupdatenow ();
	
	return (true);
	} /%opmarkedverb%/
*/

static boolean opsetdisplayverb (boolean fldisplay) {
	
	/*
	enable or disable display according to fl.  if display is becoming 
	enabled, force an full refresh
	
	return true if the display state changes
	
	6.0a14 dmb: need to do more stuff now when enabling
	*/
	
	register hdloutlinerecord ho = outlinedata;
	
	if ((**ho).flinhibitdisplay != fldisplay) /*nothing to do*/
		return (false);
	
	(**ho).flinhibitdisplay = !fldisplay;
	
	if (opdisplayenabled ()) { /*waking up display*/
		
		opdirtymeasurements (); //6.0a14 dmb: need this too; may have bogus pre-display values
		
		opsetctexpanded (ho); //6.0a12 dmb: now that we set fatheadlines and outlinerect
		
		opgetscrollbarinfo (true); /*bring scroll position in range*/
		
		opresetscrollbars ();
		
		opvisibarcursor ();
		
		opsmashdisplay ();
		}
	
	return (true);
	} /*opsetdisplayverb*/


static boolean opsetcursorverb (long cursor) {
	
	/*
	make the saved cursor the current bar cursor.  make sure it's still value 
	by visiting the entire outline.
	
	return true if the cursor was valid
	*/
	
	register hdlheadrecord hcursor = (hdlheadrecord) cursor;
	
	if (!opnodeinoutline (hcursor)) /*it's been deleted, or something*/
		return (false);
	
	opexpandto (hcursor);
	
	return (true);
	} /*opsetcursorverb*/


boolean opverbgetheadstring (hdlheadrecord hnode, bigstring bs) {
	
	getheadstring (hnode, bs);
	
	return (true);
	} /*opverbgetheadstring*/



static boolean opcodeisrunning (hdlhashnode hnode) {
	
	/*
	return true if the hnode's code is running.  lacking a formal mechanism, 
	we determine this by walking through the error stack and looking for a 
	refcon that matched hnode.  see the last line of scriptpushsourcecode.
	*/
	
	langerrorcallback errorcallback;
	
	return (langfinderrorrefcon ((long) hnode, &errorcallback));
	} /*opcodeisrunning*/


static boolean opcompileverb (hdltreenode hparam1, tyvaluerecord *v) {
	
	hdlhashtable htable;
	bigstring bsname;
	hdlhashnode hnode;
	hdltreenode hcode;
	boolean fl;
	
	flnextparamislast = true;
	
	if (!getvarparam (hparam1, 1, &htable, bsname))
		return (false);
	
	pushhashtable (htable);
	
	fl = hashlookupnode (bsname, &hnode);
	
	pophashtable ();
	
	if (!processyieldtoagents ()) /*file closed while agents were finishing up*/
		return (false);
	
	if (fl) {
		
		if (opcodeisrunning (hnode)) /*can't uncompile running code; return false to caller*/
			fl = false;
		else
			fl = langcompilescript (hnode, &hcode);
		}
	
	return (setbooleanvalue (fl, v));
	} /*opcompileverb*/


static boolean opuncompileverb (hdltreenode hparam1, tyvaluerecord *v) {
	
	hdlhashtable htable;
	bigstring bsname;
	tyvaluerecord val;
	hdltreenode hcode;
	hdlhashnode hnode;
	
	flnextparamislast = true;
	
	if (!getvarvalue (hparam1, 1, &htable, bsname, &val, &hnode))
		return (false);
	
	if (!langexternalvaltocode (val, &hcode)) { /*easy way to check for a script*/
		
		errornum = namenotscripterror;
		
		return (false);
		}
	
	pushhashtable (htable);
	
	hashlookupnode (bsname, &hnode);
	
	pophashtable ();
	
	if (opcodeisrunning (hnode)) /*can't uncompile running code; return false to caller*/
		return (true);
	
	if (!processyieldtoagents ()) /*file closed while agents were finishing up*/
		return (false);
	
	opverbdisposecode ((hdloutlinevariable) val.data.externalvalue);
	
	return (setbooleanvalue (true, v));
	} /*opuncompileverb*/


static boolean opgettypeverb (hdltreenode hparam1, tyvaluerecord *v) {
	
	/*
	2.1b12 dmb: new verb
	*/
	
	register hdltreenode hp1 = hparam1;
	bigstring bsname;
	hdloutlinevariable hv;
	long signature;
	
	flnextparamislast = true;
	
	if (!getscriptparam (hp1, 1, &hv))
		return (false);
	
	if (!opverbgetlangtext ((hdlexternalvariable) hv, false, nil, &signature))
		return (false);
	
	scriptgettypename (signature, bsname);
	
	return (setstringvalue (bsname, v));
	} /*opgettypeverb*/


static boolean opsettypeverb (hdltreenode hparam1, tyvaluerecord *v) {
	
	/*
	2.1b12 dmb: new verb
	*/
	
	register hdltreenode hp1 = hparam1;
	bigstring bsname;
	hdloutlinevariable hv;
	hdloutlinerecord ho;
	long signature;
	hdlwindowinfo hinfo;
	
	if (!getscriptparam (hp1, 1, &hv))
		return (false);
	
	flnextparamislast = true;
	
	if (!getstringvalue (hp1, 2, bsname))
		return (false);
	
	if (!scriptgetnametype (bsname, &signature)) /*unknown, let verb return false*/
		return (true);
	
	if (!opverbinmemory (hv))
		return (false);
	
	ho = (hdloutlinerecord) (**hv).variabledata;
	
	(**ho).outlinesignature = signature;
	
	opverbsetdirty ((hdlexternalvariable) hv, true);
	
	if (opwindowopen ((hdlexternalvariable) hv, &hinfo)) /*crude, but effective update*/
		if (hinfo != nil)
			shellwindowinval (hinfo);
	
	return (setbooleanvalue (true, v));
	} /*opsettypeverb*/


static boolean opgetcodeverb (hdltreenode hparam1, boolean flosacode, tyvaluerecord *v) {
	
	/*
	2.1b15 dmb: fixed memory leak; osagetcode does _not_ consume htext
	*/
	
	register hdltreenode hp1 = hparam1;
	short pnum;
	hdlhashtable htable;
	bigstring bsname;
	tyvaluerecord val;
	hdlhashnode hnode;
	tyvaluerecord vcode;
	boolean fl;
	
	initvalue (&vcode, codevaluetype);
	
	if (flosacode) {
		
			hdloutlinevariable hv;
			Handle htext;
			long signature;
			boolean flexecuteonly;
			
			if (!havecomponentmanager ()) {
				
				langerror (nocomponentmanagererror);
				
				return (false);
				}
			
			if (!getscriptparam (hp1, 1, &hv))
				return (false);
			
			if (!getbooleanvalue (hp1, 2, &flexecuteonly))
				return (false);
			
			if (!opverbgetlangtext ((hdlexternalvariable) hv, !flexecuteonly, &htext, &signature))
				return (false);
			
			fl = osagetcode (htext, signature, flexecuteonly, &vcode);
			
			disposehandle (htext);
			
			if (!fl)
				return (false);
			
			pnum = 3;
		}
	else {
		
		if (!getvarvalue (hp1, 1, &htable, bsname, &val, &hnode))
			return (false);
		
		if (!langexternalvaltocode (val, &vcode.data.codevalue)) {
			
			errornum = namenotscripterror;
			
			return (false);
			}
		
		if (vcode.data.codevalue == nil) {
			
			pushhashtable (htable);
			
			fl = hashlookupnode (bsname, &hnode);
			
			pophashtable ();
			
			if (!langcompilescript (hnode, &vcode.data.codevalue))
				return (false);
			}
		
		if (!copyvaluerecord (vcode, &vcode))
			return (false);
		
		pnum = 2;
		}
	
	flnextparamislast = true;
	
	if (!getvarparam (hp1, pnum, &htable, bsname))
		return (false);
	
	if (!hashtableassign (htable, bsname, vcode))
		return (false);
	
	exemptfromtmpstack (&vcode);
	
	return (setbooleanvalue (true, v));
	} /*opgetcodeverb*/


static boolean opgetsourceverb (hdltreenode hparam1, boolean flosacode, tyvaluerecord *v) {
#pragma unused(flosacode)

	register hdltreenode hp1 = hparam1;
	hdlhashtable htable;
	bigstring bsname;
	tyvaluerecord vcode;
	tyvaluerecord vsource;
	tyvaluerecord vscript;
	hdlheadrecord hsummit;
	hdloutlinerecord houtline;
	register hdloutlinerecord ho;
	OSType signature;
	
	initvalue (&vcode, binaryvaluetype);
	
	if (!getbinaryvalue (hp1, 1, true, &vcode.data.binaryvalue))
		return (false);
	
	flnextparamislast = true;
	
	if (!getvarparam (hp1, 2, &htable, bsname))
		return (false);
	
		if (!osagetsource (&vcode, &signature, &vsource))
			return (false);
	
	if (!langexternalnewvalue (idscriptprocessor, nil, &vscript))
		return (false);
	
	opvaltoscript (vscript, &houtline);
	
	ho = houtline; /*copy into register*/
	
	(**ho).outlinesignature = signature;
	
	if (!optexttooutline (ho, vsource.data.stringvalue, &hsummit))
		goto error;
	
	//oppushoutline (ho);
	
	opsetsummit (ho, hsummit);
	
	opsetctexpanded (ho);
	
	//oppopoutline ();
	
	if (!hashtableassign (htable, bsname, vscript))
		goto error;
	
	return (setbooleanvalue (true, v));
	
	error: {
		
		disposevaluerecord (vscript, true);
		
		return (false);
		}
	} /*opgetsourceverb*/


static boolean opstartprofileverb (hdltreenode hparam1, tyvaluerecord *v) {

	/*
	5.0.2b15 dmb: new feature: turn on profiling. this will create a new 
	profile data table internally if it's the first time for this process
	*/
	
	boolean fltimesliced;
	
	flnextparamislast = true;
	
	if (!getbooleanvalue (hparam1, 1, &fltimesliced))
		return (false);
	
	return (setbooleanvalue (processstartprofiling (fltimesliced), v));
	} /*opstartprofileverb*/


static boolean convertprofiletoticksvisit (hdlhashnode hnode, ptrvoid refcon) {
#pragma unused (refcon)

	tyvaluerecord * val = &(**hnode).val;
	
	(*val).data.longvalue = ( ( (*val).data.longvalue * 3 ) / 50 );
	
	return (true);
}

static boolean opstopprofileverb (hdltreenode hp1, tyvaluerecord *v) {
	
	/*
	5.0.2b15 dmb: turn off profiling. If the user passes an address (not nil),
	we assign the profiledata table and remove it from the process.
	
	2005-01-14 SMD: results are stored in milliseconds, but the return values
	   have been in ticks for a long time. So, added an optional param,
	   flUseMilliseconds. Default is false. If true, we convert the milliseconds
	   to ticks.
	*/
	
	hdlprocessrecord hp = currentprocess;
	hdlhashtable ht;
	bigstring bs;
	tyvaluerecord val;
	short ctconsumed = 1;
	short ctpositional = 1;
	tyvaluerecord vflusemilliseconds; /* for optional flUseMilliseconds param */
	
	if (!getvarparam (hp1, 1, &ht, bs))
		return (false);
	
	setbooleanvalue (false, &vflusemilliseconds);
	
	flnextparamislast = true;
	
	if (!getoptionalparamvalue (hp1, &ctconsumed, &ctpositional, BIGSTRING ("\x11" "flUseMilliseconds"), &vflusemilliseconds))
		return (false);
	
	if (!processstopprofiling ())
		return (setbooleanvalue (false, v));
	
	if (!isemptystring (bs)) { // move our table to caller's address
		
		if ((**hp).hprofiledata == nil)
			setnilvalue (&val);
		else
		{
			if (! vflusemilliseconds.data.flvalue) /* convert values to ticks */
			{
				if (!hashtablevisit ((hdlhashtable) (**(**hp).hprofiledata).variabledata, &convertprofiletoticksvisit, (ptrvoid) nil))
					return (false);
			}
			
			setexternalvalue ((Handle) (**hp).hprofiledata, &val);
		}
		
		hashtableassign (ht, bs, val);
		
		(**hp).hprofiledata = nil;
	}
	
	return (setbooleanvalue (true, v));
} /*opstopprofileverb*/


static boolean opverbrejectmenubar (bigstring bserror) {
	
	if ((**shellwindowinfo).configresnum == idmenueditorconfig) {
		
		getstringlist (operrorlist, rejectmenubarnum, bserror);
		
		return (true);
		}
	
	return (false);
	} /*opverbrejectmenubar*/


static boolean opverbrejecttable (bigstring bserror) {
	
	/*
	PBS 7.0b21: for some op verbs, tables should be rejected.
	For instance, you can't set the refcon of an item in a table.
	*/
	
	if ((**shellwindowinfo).configresnum == idtableconfig) {
		
		getstringlist (operrorlist, rejecttablenum, bserror);
		
		return (true);
		}
	
	return (false);
	} /*opverbrejecttable*/


static boolean opgetrefconverb (hdltreenode hparam1, hdlheadrecord hnode, tyvaluerecord *v) {
	
	/*
	4/16/92 dmb: new verb, value = op.setRefcon ()
	
	7/20/92 dmb: don't try to coerce external types -- no can do!
	
	11/6/92 dmb: use new langgoodbinarytype
	
	5.0.2b17 dmb: use langpack/unpack, not binary coercion
	*/
	
	Handle hrefcon = (**hnode).hrefcon;
	tyvaluerecord linkedval;
	//tyvaluetype valuetype;
	
	if (!langcheckparamcount (hparam1, 0))
		return (false);
	
	/*
	flnextparamislast = true;
	
	if (!getvarparam (hparam1, 1, &htable, bs)) /%returned handle holder%/
		return (false);
	*/
	
	if (hrefcon == nil)
		setlongvalue (0, &linkedval);
	
	else {
			
			if (!langunpackvalue (hrefcon, &linkedval))
				return (false);
			
			pushvalueontmpstack (&linkedval);
			
		}
	
	*v = linkedval;
	
	/*
	if (!langsetsymboltableval (htable, bs, linkedval)) /%probably a memory error%/
		return (false);
	
	exemptfromtmpstack (linkedval);
	
	(*v).data.flvalue = true;
	*/
	
	return (true);
	} /*opgetrefconverb*/


static boolean opsetrefconverb (hdltreenode hparam1, hdlheadrecord hnode, tyvaluerecord *v) {
	
	/*
	4/16/92 dmb: new verb, op.setrefcon (value)
	
	4.1b8 dmb: dirty the outline
	
	5.0.2b17 dmb: use langpack/unpack, not binary coercion
	*/
	
	tyvaluerecord val;
	Handle hbinary;
	boolean fl;
	
	flnextparamislast = true;
	
	if (!getparamvalue (hparam1, 1, &val))
		return (false);
	
	if ((val.valuetype == longvaluetype) && (val.data.longvalue == 0)) {
		
		opemptyrefcon (hnode);
		
		fl = true;
		}
	else {
		
			
			if (!langpackvalue (val, &hbinary, HNoNode))
				return (false);
			
			disposehandle ((**hnode).hrefcon);
			
			(**hnode).hrefcon = hbinary;
			
			fl = true;
			
		}
	
	if (fl)
		opdirtyoutline ();
	
	return (setbooleanvalue (fl, v));
	} /*opsetrefconverb*/


#define str_searchfor		(BIGSTRING ("\x09" "searchfor"))
#define str_replacewith		(BIGSTRING ("\x0b" "replacewith"))
#define str_casesensitive	(BIGSTRING ("\x0d" "casesensitive"))
#define str_wholewords		(BIGSTRING ("\x0a" "wholewords"))
#define str_wraparound		(BIGSTRING ("\x0a" "wraparound"))
#define str_flatsearch		(BIGSTRING ("\x0a" "flatsearch"))
#define str_frontwindowonly	(BIGSTRING ("\x0f" "frontwindowonly"))
#define str_closewindows	(BIGSTRING ("\x0c" "closewindows"))

static boolean opfindverb (hdltreenode hp1, tyvaluerecord *vreturned) {

	/*
	5.1.5b15 dmb: on find (bs, wholewords=user.prefs.search.wholewords,
			casesensitive=user.prefs.search.casesensitive)
	
	6.0b2 dmb: restore original searchparams when we're done.
	
	2003-05-17 AR: Updated to reflect changes opflatfind related to the
	new Find & Replace dialog with regexp support
	
	2003-05-24 AR: avoid errors if there is currently an invalid regexp
	specified in the Find & Replace dialog
	*/
	
	bigstring bs;
	short ctconsumed = 0;
	short ctpositional;
	tyvaluerecord vwholewords, vcasesensitive;
	tysearchparameters savesearchparams;
	boolean fl;
	
	disablelangerror (); /*2003-05-24 AR*/
	
	getsearchparams (); //side effect: protects search strings
	
	enablelangerror ();
	
	savesearchparams = searchparams;
	
	setsearchparams (); //restore search strings
	
	if (!getstringvalue (hp1, ++ctconsumed, bs))
		return (false);
	
	ctpositional = ctconsumed;
	
	setbooleanvalue (searchparams.flwholewords, &vwholewords);
	
	if (!getoptionalparamvalue (hp1, &ctconsumed, &ctpositional, str_wholewords, &vwholewords))
		return (false);
	
	setbooleanvalue (!searchparams.flunicase, &vcasesensitive);
	
	flnextparamislast = true;
	
	if (!getoptionalparamvalue (hp1, &ctconsumed, &ctpositional, str_casesensitive, &vcasesensitive))
		return (false);
	
	endcurrentsearch ();
	
	searchparams.flonelevel = true; /*don't dive into scripts, if in a menubar*/
	
	searchparams.floneobject = true;
	
	searchparams.flwraparound = false;
	
	searchparams.flregexp = false;
	
	searchparams.flunicase = !vcasesensitive.data.flvalue;
	
	searchparams.flwholewords = vwholewords.data.flvalue;
	
	copystring (bs, searchparams.bsorigfind);
	
	copystring (bs, searchparams.bsfind);
	
	if (searchparams.flunicase)
		alllower (searchparams.bsfind);
	
	startnewsearch (false, false);
	
	startingtosearch (0);
	
	fl = opflatfind (false, searchparams.flwraparound);
	
	searchparams = savesearchparams; //don't leave anything lying around
	
	return (setbooleanvalue (fl, vreturned));
	} /*opfindverb*/


static boolean opgetexpansionstatevisit (hdlheadrecord hnode, long *lp, hdllistrecord hlist) {

	register hdlheadrecord nomad;

	while (true) {

		(*lp)++;

		nomad = hnode;

		if (opsubheadsexpanded (nomad))			
			if (!langpushlistlong (hlist, *lp))
				return (false);

		hnode = opbumpflatdown (nomad, true);

		if (nomad == hnode)
			break;
		}

	return (true);
	}/*opgetexpansionstatevisit*/


boolean opgetexpansionstateverb (tyvaluerecord *v) {

	/*
	6.2a6 AR: Returns a list of numbers; each number is the
				line number (one-based) of an expanded headline
	
	7.0b21 PBS: no longer static -- needed by opxml.c to get the
	expansion state of an outline.
	*/
	register hdloutlinerecord ho = outlinedata;
	hdlheadrecord hnode;
	hdllistrecord hlist;
	long ct = 0;

	assert ((ho != nil) && (*ho != nil));

	assert ((**ho).hsummit != nil);

	hnode = (**ho).hsummit;

	opnewlist (&hlist, false);

	if (!opgetexpansionstatevisit (hnode, &ct, hlist)) {

		opdisposelist (hlist);

		return (false);
		}

	return (setheapvalue ((Handle) hlist, listvaluetype, v));
	}/*opgetexpansionstateverb*/


boolean opsetexpansionstateverb (tyvaluerecord *vlist, tyvaluerecord *v) {
#pragma unused(v)

	/*
	6.2a6 AR: Takes the output of op.getExpansionState and sets the outline state accordingly.
				Returns true if successful, otherwise false, e.g. if a line number
				given in explist doesn't exist in the outline. 
				
	7.0b21 PBS: no longer static -- needed by opxml.c to set the expansion state of an XML
	outlineDocument.
	*/
	
	hdloutlinerecord ho = outlinedata;
	hdlheadrecord nomad;
	hdlheadrecord hnode;
	tyvaluerecord vitem;
	long ct;
	long ix = 1;
	long ixlist = 0;
	long ixoutline = 0;

	assert ((ho != nil) && (*ho != nil));

	if (!langgetlistsize (vlist, &ct))
		return (false);

	if (ct > 0) {

		if (!langgetlistitem (vlist, ix++, nil, &vitem))
			return (false);

		if (!coercevalue (&vitem, longvaluetype))
			return (false);

		ixlist = vitem.data.longvalue;
		}

	opsetdisplayverb (false);

	opsettextmode (false);

	hnode = (**ho).hsummit;

	while (true) {

		++ixoutline;

		nomad = hnode;

		if (ixoutline == ixlist) {

			if (!opsubheadsexpanded (nomad) && (**nomad).flexpanded)
				opexpand (nomad, 1, true);

			if (ix <= ct) {
				if (!langgetlistitem (vlist, ix++, nil, &vitem))
					goto error;

				if (!coercevalue (&vitem, longvaluetype))
					goto error;

				ixlist = vitem.data.longvalue;
				}
			}
		else {
			if (opsubheadsexpanded (nomad))
				opcollapse (nomad);
			}

		hnode = opbumpflatdown (nomad, true);

		if (hnode == nomad)
			break;
		}

	hnode = (**ho).hbarcursor;

	while (!(**hnode).flexpanded) {

		if (hnode == (**hnode).headlinkleft)
			break;

		hnode = (**hnode).headlinkleft;
		}

	opmoveto (hnode);
		
	opsetdisplayverb (true);

	return (true);

error:
	opsetdisplayverb (true);

	return (false);
	}/*opsetexpansionstateverb*/
				

boolean opgetscrollstateverb (tyvaluerecord *v) {

	/*
	6.2a6 AR: Returns the line number (one-based) of the first headline
				displayed in the outline window.
				
	7.0b21 PBS: no longer static -- needed in opxml.c to get the scroll state of an outline.
	*/

	register hdloutlinerecord ho = outlinedata;
	register hdlheadrecord nomad, hsummit;
	register long ctheadlines = 1;

	assert ((ho != nil) && (*ho != nil));

	assert ((**ho).hline1 != nil);

	assert ((**ho).hsummit != nil);

	hsummit = (**ho).hsummit;

	nomad = (**ho).hline1;

	while (nomad != hsummit) {

		nomad = opbumpflatup (nomad, true);

		++ctheadlines;
		}

	return (setlongvalue (ctheadlines, v));
	}/*opgetscrollstateverb*/


boolean opsetscrollstateverb (long line1, tyvaluerecord *v) {

	/*
	6.2a6 AR: Scrolls the outline so that lnum is the first line displayed in the window.
				If lnum is out of range, it scrolls to the last line.
				
	7.0b21 PBS: no longer static -- needed by opxml.c to set scroll state of converted
	outlineDocument.
	*/

	register hdloutlinerecord ho = outlinedata;
	register hdlheadrecord nomad;

	assert ((ho != nil) && (*ho != nil));

	assert ((**ho).hsummit != nil);

	nomad = (**ho).hsummit;

	while (--line1 > 0 && nomad != nil)
		nomad = opbumpflatdown (nomad, true);

	opsetdisplayverb (false); //hide display updates

	opsettextmode (false); //switch to select mode
	
	opmotionkey (flatdown, longinfinity, false); //move to the last headline

	opmoveto (nomad); //move to our headline -- BUG: only works if expanded

	opsetdisplayverb (true); //reenable display updates

	return (setbooleanvalue (true, v));
	}/*opsetscrollstateverb*/


static boolean opgetselvisit (hdlheadrecord hnode, ptrvoid refcon) {
	
	/*
	7.0b15 PBS: for each marked item, store the cursor in a list.
	
	Used by opgetselectverb, which is called by op.getSelect ().
	
	Code based on tablegetselvisit.
	*/
	
	hdllistrecord hlist = (hdllistrecord) refcon;
	tyvaluerecord val;
	
	if (!setlongvalue ((long) hnode, &val)) /*cursors are hdlheadrecords, coerced to a long*/
		return (false);
	
	if (!langpushlistval (hlist, nil, &val)) /*add value to list*/
		return (false);
	
	disposevaluerecord (val, true); /*don't let them accumulate*/
	
	return (true);
	} /*opgetselvisit*/


static boolean opgetselectverb (hdltreenode hp1, tyvaluerecord *v) {
	
	/*
	7.0b15 PBS: get a list of cursors, the current selection in an outline.
	
	Based on tablegetselectionverb, which returns a list of addresses,
	the selections in a table.
	*/
	
	hdllistrecord hlist;
	
	if (!langcheckparamcount (hp1, 0)) /*too many parameters were passed*/
		return (false);
	
	if (!opnewlist (&hlist, false)) /*create a list*/
		return (false);
	
	if (!opvisitmarked (down, &opgetselvisit, (ptrvoid) hlist)) {
		
		opdisposelist (hlist);
		
		return (false);
		}
	
	return (setheapvalue ((Handle) hlist, listvaluetype, v));
	} /*opgetselectverb*/


static boolean opgetsuboutlinevisit (hdlheadrecord hnode, boolean flindent, long level, handlestream *str) {

	/*
	6.2a6 AR: Recursively get the current headline and all its subheads as a string
	*/

	hdlheadrecord nomad = hnode;
	long indent = (flindent ? level : 0);

	if (!writehandlestreamhandleindent (str, (**hnode).headstring, indent))
		return (false);

	if (opnavigate (right, &nomad)) {

		level++;

		while (true) {

			if (!opgetsuboutlinevisit (nomad, flindent, level, str)) //recurse
				return (false);

			if (!opnavigate (down, &nomad))
				break;
			}
		}

	return (true);
	}/*opgetsuboutlinevisit*/


static boolean opgetsuboutlineverb (hdlheadrecord hnode, boolean flindent, tyvaluerecord *v) {

	/*
	6.2a6 AR: See opgetsuboutlinevisit
	*/

	handlestream s;

	assert ((hnode != nil) && (*hnode != nil));

	openhandlestream (nil, &s);

	if (!opgetsuboutlinevisit (hnode, flindent, 0L, &s)) {

		disposehandlestream (&s);

		return (false);
		}

	return (setheapvalue (closehandlestream (&s), stringvaluetype, v));
	}/*opgetsuboutlineverb*/


static boolean opinsertoutlineverb (hdloutlinerecord hosource, tydirection dir, tyvaluerecord *v) {
	
	/*
	7.0d2 AR: Insert hosource into current outline, similar to op.insert but better at preserving outline state
	
	7.0b13 PBS: Set cursor back to original cursor.
	*/

	hdlheadrecord hbarcursor = (**outlinedata).hbarcursor;
	hdlheadrecord horigcursor = hbarcursor; /*7.0b13 PBS*/
	hdlheadrecord hcopy;

	if (dir == left) { /*special case: convert left to down from parent*/
		
		hbarcursor = (**hbarcursor).headlinkleft;
		
		if (hbarcursor == (**outlinedata).hbarcursor) /*couln't move left*/
			return (setbooleanvalue (false, v));
		
		opmoveto (hbarcursor);
		
		dir = down;
		}
	
	if (!opcopysiblings ((**hosource).hsummit, &hcopy))
		return (false);
		
	if (dir == right) { /*maybe we need to expand?*/
	
		if (!opsubheadsexpanded (hbarcursor))
			opexpand (hbarcursor, 1, false);
		}

	if (!opinsertstructure (hcopy, dir)) {
	
		opdisposestructure (hcopy, false);
	
		return (false);
		}
	
	opmoveto (horigcursor);

	return (setbooleanvalue (true, v)); 
	}/*opinsertoutlineverb*/


static long opgetheadnumberverb (hdlheadrecord hnode) {
	
	/*
	7.0b17 PBS: return the index, one-based, of the current node.
	If hnode is the third head at this level, return 3.
	*/
	
	long ixnode;
	
	opgetsiblingnumber (hnode, &ixnode); /*zero-based*/
	
	ixnode++; /*add 1 -- scripters expect one-based indexes*/
	
	return (ixnode);
	} /*opgetheadnumberverb*/


extern boolean langsettarget (hdlhashtable htable, bigstring bsname, tyvaluerecord *prevtarget);

extern boolean langcleartarget (tyvaluerecord *prevtarget);

static boolean opvisitallvisit (hdlheadrecord hnode, ptrvoid bsscriptname) {

	/*
	7.0b17 PBS: visit every headline in an outline, calling a callback script for each.
	
	It's a wrapper for opvisiteverything.

	2004-11-03 aradke: Take advantage of langrunscript now accepting
	a nil pointer if the script doesn't take any parameters.
	We don't need to create an empty list anymore.
	*/
	
	tyvaluerecord vreturned;
	
	/*
	tyvaluerecord vparams;
	hdllistrecord hlist;
		
	if (!opnewlist (&hlist, false)) /%create a list%/
		return (false);

	if (!setheapvalue ((Handle) hlist, listvaluetype, &vparams))
		return (false);
	*/

	oppushoutline (outlinedata);
		
	(**outlinedata).hbarcursor = hnode;
		
	(**outlinedata).flwindowopen = true;
	
	langrunscript (bsscriptname, nil, nil, &vreturned);
	
	oppopoutline ();
	
	return (true);
	} /*opvisitallvisit*/


static boolean opvisitallverb (hdltreenode hparam1, tyvaluerecord *v, bigstring bserror) {
#pragma unused (bserror)

	/*
	7.0b17 PBS: visit every headline, running the specified callback script.
	*/
	
	bigstring bsscriptname;
	hdloutlinerecord ho;
	hdlhashtable htable, htableoldtarget;
	bigstring bsname, bsnameoldtarget;
	tyvaluerecord val, prevtarget;
	hdlhashnode hnode;
	hdlheadrecord horigcursor;
	boolean fl = false;
	boolean flhadtarget = false;

	/*Get outline parameter.*/
	
	if (!getoutlinevalue (hparam1, 1, &ho))
		goto exit1;
	
	/*Set the target to the outline.*/
	
	if (!getvarvalue (hparam1, 1, &htable, bsname, &val, &hnode))
		goto exit1;
	
	if (!langsettarget (htable, bsname, &prevtarget))
		goto exit1;

	if (getaddressvalue (prevtarget, &htableoldtarget, bsnameoldtarget)) /*get info about previous target*/
		flhadtarget = true;

	if (htableoldtarget == nil)
		flhadtarget = false;
	
	/*Open window as hidden.*/

	if (!langzoomvalwindow  (htable, bsname, val, false))
		goto exit2;
		
	/*Get script parameter.*/
	
	flnextparamislast = true;
	
	if (!getstringvalue (hparam1, 2, bsscriptname))
		goto exit2;

//	if (!getvarvalue (hparam1, 2, &htable, bsname, &val, &hnode))
//		goto exit2;
	
	oppushoutline (ho);
	
	horigcursor = (**ho).hbarcursor; /*get cursor*/

	opvisiteverything (&opvisitallvisit, bsscriptname); /*call the visit routine*/

//	opvisiteverything (&opvisitallvisit, val); /*call the visit routine*/

	(**ho).hbarcursor = horigcursor; /*restore cursor*/

	oppopoutline ();
	
	fl = true;
	
	exit2:
	
	/*Restore the previous target.*/
	
	if (flhadtarget)
		langsettarget (htableoldtarget, bsnameoldtarget, &prevtarget);
	else
		langcleartarget (nil);
	
	exit1:
	
	setbooleanvalue (fl, v);

	return (fl);
	} /*opvisitallverb*/


static boolean opgetselectedsuboutlinesvisit (hdlheadrecord hnode, ptrvoid refcon) {
	
	/*
	7.0b18 PBS: callback routine for op.getSelectesSubOutlines.
	*/
	
	hdloutlinerecord ho = (hdloutlinerecord) refcon;
	hdlheadrecord hcopy;
	hdlheadrecord hbarcursor = (**ho).hbarcursor;

	hcopy = opcopyoutline (hnode);
	
	if (hcopy == nil)
		return (false);
	
	oppushoutline (ho);
	
	opstartinternalchange ();
	
	opdeposit (hbarcursor, down, hcopy);
		
	opresetlevels ((**hbarcursor).headlinkdown);
	
	opendinternalchange ();
	
	oppopoutline ();	

	return (true);
	} /*opgetselectedsuboutlinesvisit*/


static boolean opgetselectedsuboutlinesverb (hdloutlinerecord ho) {
	
	/*
	7.0b18 PBS: get selected headlines and their subheads as an outline.
	*/
	
	bigstring bsheadstring;
	
	/*Copy selected headlines*/
	
	if (!opvisitmarked (up, &opgetselectedsuboutlinesvisit, (ptrvoid) ho))
		return (false);
	
	/*Delete the top, empty summit*/
	
	oppushoutline (ho);
	
	opmoveto ((**ho).hsummit); /*go back to first summit*/
	
	getheadstring ((**ho).hsummit, bsheadstring);
	
	if (equalstrings (bsheadstring, emptystring)) /*if it's "", delete it*/

		opdelete ();
		
	oppopoutline ();
	
	return (true);
	} /*opgetselectedsuboutlinesverb*/


static boolean getoptionaltablevalue (hdltreenode hp1, short pnum, hdlhashtable *ht) {

	tyvaluerecord v;

	if (!getparamvalue (hp1, pnum, &v))
		return (false);

	if (v.valuetype == novaluetype) {

		*ht = nil;

		return (true);
		}

	return (gettablevalue (hp1, pnum, ht));
	} /*getoptionaltablevalue*/


static boolean getoptionaltablereference (hdltreenode hp1, short pnum, hdlhashtable *ht) {

	/*
	7.1b45 dmb: new function to get an optional table reference that may be nil
	*/

	tyvaluerecord v;
	xmladdress adr;

	if (!getparamvalue (hp1, pnum, &v))
		return (false);

	if (v.valuetype == novaluetype) {

		*ht = nil;

		return (true);
		}

	if (!getvarparam (hp1, pnum, &adr.ht, adr.bs))
		return (false);

	return (langassignnewtablevalue (adr.ht, adr.bs, ht));
	} /*getoptionaltablereference*/


static boolean opxmltooutlineverb (hdltreenode hparam1, tyvaluerecord *v) {

	/*
	7.0b21 PBS -- convert an XML outlineDocument to an outline.
	
	Two parameters: xml text and the address of an outline.
	
	opxmltooutline is in opxml.c.

	7.1b43 dmb: added optional cloudSpec parameter. table can be null.
	*/
	
	hdloutlinerecord ho;
	hdlhashtable htable;
	bigstring bsname;
	tyvaluerecord val;
	hdlhashnode hnode;
	boolean fl = false;
	Handle htext;
	boolean flnewoutline;
	hdlhashtable hcloud = nil;

	if (!getreadonlytextvalue (hparam1, 1, &htext))
		goto exit;
		
	if (!getoutlinevalue (hparam1, 2, &ho))
		goto exit;
	
	if (!getvarvalue (hparam1, 2, &htable, bsname, &val, &hnode))
		goto exit;
		
	if (!getbooleanvalue (hparam1, 3, &flnewoutline))
		goto exit;

	if (langgetparamcount (hparam1) > 3) {
		
		flnextparamislast = true;

		if (!getoptionaltablereference (hparam1, 4, &hcloud))
			return (false);
		}
	
	oppushoutline (ho);
	
	fl = opxmltooutline (htext, ho, flnewoutline, htable, bsname, val, hcloud);
	
	oppopoutline ();
		
	exit:
	
	setbooleanvalue (fl, v);
	
	return (fl);
	} /*opxmltooutlineverb*/


static boolean opoutlinetoxmlverb (hdltreenode hparam1, tyvaluerecord *v) {
	
	/*
	7.0b21 PBS: convert an outline to outlineDocument XML.

	7.1b43 dmb; handle optional cloud parameter. also, fixed handle disposal
	in error condition
	*/
	
	boolean fl = false;
	Handle htext = nil;
	Handle hname = nil;
	Handle hemail = nil;
	hdloutlinerecord ho;
	hdlhashtable ht;
	tyvaluerecord val;
	hdlhashnode hnode;
	bigstring bsname;
	hdlhashtable hcloud = nil;
		
	if (!getoutlinevalue (hparam1, 1, &ho)) /*Get the outline pointed to in first parameter*/
		goto exit;
	
	if (!getvarvalue (hparam1, 1, &ht, bsname, &val, &hnode)) /*We'll be opening the window.*/
		goto exit;
	
	if (!getexempttextvalue (hparam1, 2, &hname)) /*ownerName*/
		goto exit;
	
	if (!getexempttextvalue (hparam1, 3, &hemail)) /*ownerEmail*/
		goto exit;

	if (langgetparamcount (hparam1) > 3) {
		
		flnextparamislast = true;

		if (!getoptionaltablevalue (hparam1, 4, &hcloud))
			return (false);
		}

	if (!opoutlinetoxml (ho, hname, hemail, &htext, ht, bsname, val, hcloud)) /*Convert from outline to XML text*/
		goto exit;
	
	fl = setheapvalue (htext, stringvaluetype, v); /*htext is returned XML text*/
	
	exit:
	
	disposehandle (hname);
	
	disposehandle (hemail);
	
	return (fl);
	} /*opoutlinetoxmlverb*/


static boolean opfunctionvalue (short token, hdltreenode hparam1, tyvaluerecord *vreturned, bigstring bserror) {
	
	/*
	bridges op.c with the language.  the name of the verb is bs, its first parameter
	is hparam1, and we return a value in vreturned.
	
	we use a limited number of support routines from lang.c to get parameters and
	to return values. 
	
	return false only if the error is serious enough to halt the running of the script
	that called us, otherwise error values are returned through the valuerecord, which
	is available to the script.
	
	if we return false, we try to provide a descriptive error message in the 
	returned string bserror.
	
	2/18/91 dmb: use new opverbfindtarget instead of opsetverbglobals -- we no 
	longer require that the outline window be in the front.
	
	12/16/91 dmb: on getlinetext, writeeditbuffer instead of forcing textmode false
	
	4/3/92 dmb: findfunc now calls newly-public get/setsearchparams
	
	8/25/92 dmb: added firstsummitfunc; levelfunc now returns 1-based value so glue 
	isn't needed; removed mark functions -- unused & undocumented
	
	9/4/92 dmb: disable hoist when there's a multiple selection
	
	6/1/93 dmb: when vreturned is nil, return whether or not verb token must 
	be run in the Frontier process
	
	2.1b12 dmb: added getosacodefunc & getosasourcefunc to list not needing process swap

	5.1.4 dmb: don't call setsearchparams
	
	5.1.5 dmb: findfunc take optional params
	*/
	
	register tyvaluerecord *v = vreturned;
	register boolean fl;
	register hdloutlinerecord ho;
	register hdlheadrecord hbarcursor;
	WindowPtr targetwindow;
	
	if (v == nil) { /*need Frontier process?*/
		
		switch (token) {
			
			case tabkeyreorgfunc:
			case compilefunc:
			case uncompilefunc:
			case getcodefunc:
			case gettypefunc:
			case getosacodefunc:
			case getosasourcefunc:
			case startprofilefunc:
			case stopprofilefunc:
				return (false);
			
			default:
				return (true);
			}
		}
	
	errornum = 0;
	
	setbooleanvalue (false, v); /*by default, op functions return false*/
	
	switch (token) { /*these verbs don't require an open outline window*/
		
		
		/*
		case runfunc:
			if (!oprunverb (hparam1, v))
				goto error;
			
			return (true);
		*/
		
		case tabkeyreorgfunc:
			
			flnextparamislast = true;
			
			if (!getbooleanvalue (hparam1, 1, &opprefs.fltabkeyreorg))
				goto error;
			
			(*v).data.flvalue = true;
			
			return (true);
			
		case flatcursorkeysfunc:
			
			flnextparamislast = true;
			
			if (!getbooleanvalue (hparam1, 1, &opprefs.flflatcursorkeys))
				goto error;
			
			(*v).data.flvalue = true;
			
			return (true);
		
		
		case compilefunc:
			
			if (!opcompileverb (hparam1, v))
				goto error;
			
			return (true);
		
		case uncompilefunc:
			
			if (!opuncompileverb (hparam1, v))
				goto error;
			
			return (true);
		
		case getcodefunc:
			
			if (!opgetcodeverb (hparam1, false, v))
				goto error;
			
			return (true);
		
		case gettypefunc:
			
			if (!opgettypeverb (hparam1, v))
				goto error;
			
			return (true);
		
		case getosacodefunc:
			
			if (!opgetcodeverb (hparam1, true, v))
				goto error;
			
			return (true);
		
		case settypefunc:
			
			if (!opsettypeverb (hparam1, v))
				goto error;
			
			return (true);
		
		case getosasourcefunc:
			
			if (!opgetsourceverb (hparam1, true, v))
				goto error;
			
			return (true);
			
		case startprofilefunc:
			if (!opstartprofileverb (hparam1, v))
				goto error;
			
			return (true);
		
		case stopprofilefunc:
			if (!opstopprofileverb (hparam1, v))
				goto error;
			
			return (true);
		
		case visitallfunc: { /*7.0b17 PBS: visit all nodes in an outline*/
			
			if (!opvisitallverb (hparam1, v, bserror))
				goto error;
			
			return (true);
			}
		
		case xmltooutlinefunc: { /*7.0b21 PBS: convert from XML to an outline*/
			
			if (!opxmltooutlineverb (hparam1, v))
				goto error;
			
			return (true);
			}
		
		case outlinetoxmlfunc: { /*7.0b21 PBS: convert outline to XML*/
			
			if (!opoutlinetoxmlverb (hparam1, v))
				goto error;
			
			return (true);
			}
			
		} /*switch*/
	
	
	if (!langfindtargetwindow (idoutlineprocessor, &targetwindow)) { /*all other verbs require an outline window in front*/
		
		errornum = noooutlineerror;
		
		goto error;
		}
	
	shellpushglobals (targetwindow); /*following verbs assume that an outline is in front*/
	
	(*shellglobals.gettargetdataroutine) (idoutlineprocessor); /*set op globals*/
	
	ho = outlinedata; /*copy into register*/
	
	hbarcursor = (**ho).hbarcursor;
	
	fl = false; /*default return value*/
	
	switch (token) {
			
		case linetextfunc: {
			Handle htext;
			
			if (!langcheckparamcount (hparam1, 0))
				break;
			
			opwriteeditbuffer (); /*if headline is being edited, update text handle*/
			
			if (!copyhandle ((**hbarcursor).headstring, &htext))
				break;
			
			fl = setheapvalue (htext, stringvaluetype, v);
			
			break;
			}
			
		case levelfunc:
			if (!langcheckparamcount (hparam1, 0))
				break;
			
			fl = setlongvalue ((**hbarcursor).headlevel + 1, v);
			
			break;
			
		case countsubsfunc: {
			short level;
			
			flnextparamislast = true;
			
			if (!getintvalue (hparam1, 1, &level))
				break;
			
			fl = setlongvalue (opcountsubheads (hbarcursor, level), v);
			
			break;
			}
			
		case countsummitsfunc:
			if (!langcheckparamcount (hparam1, 0))
				break;
			
			fl = setlongvalue (opcountatlevel ((**ho).hsummit), v);
			
			break;
			
		case gofunc: {
			tydirection dir;
			long units;
			
			if (!getdirectionvalue (hparam1, 1, &dir))
				break;
			
			flnextparamislast = true;
			
			if (!getlongvalue (hparam1, 2, &units))
				break;
			
			opsettextmode (false);
			
			(*v).data.flvalue = opmotionkey (dir, units, false);
			
			fl = true;
			
			break;
			}
		
		case firstsummitfunc:
			if (!langcheckparamcount (hparam1, 0))
				break;
			
			opsettextmode (false);
			
			(*v).data.flvalue = opmotionkey (flatup, longinfinity, false);
			
			fl = true;
			
			break;
			
		case expandfunc: {
			short level;
			
			flnextparamislast = true;
			
			if (!getintvalue (hparam1, 1, &level))
				break;

//			if (langopruncallbackscripts (idopexpandscript)) /*7.0b30: call callback script*/
				
//				(*v).data.flvalue = true;  /*fuction consumed the click*/

//			else
			
				(*v).data.flvalue = opexpand (hbarcursor, level, true);
			
			fl = true;
			
			break;
			}
		
		case collapsefunc:
			if (!langcheckparamcount (hparam1, 0))
				break;

//			if (langopruncallbackscripts (idopcollapsescript)) /*7.0b30: call callback script*/
			
//				(*v).data.flvalue = true;

//			else

				(*v).data.flvalue = opcollapse (hbarcursor);
			
			fl = true;
			
			break;
		
		case getexpandedfunc:
			if (!langcheckparamcount (hparam1, 0))
				break;
			
			(*v).data.flvalue = opsubheadsexpanded (hbarcursor);
			
			fl = true;
			
			break;

		case insertfunc: {
			Handle htext;
			tydirection dir;
			
			if (!getexempttextvalue (hparam1, 1, &htext)) /*headline string*/
				break;
			
			flnextparamislast = true;
			
			if (!getdirectionvalue (hparam1, 2, &dir)) /*direction*/
				break;
				
			(*v).data.flvalue = opinserthandle (htext, dir);
			
			disposehandle (htext);
			
			fl = true;
			
			break;
			}
		
		case findfunc:
			fl = opfindverb (hparam1, v);
			
			break;
		
		case sortfunc:
			if (!langcheckparamcount (hparam1, 0))
				break;
			
			(*v).data.flvalue = opsortlevel (hbarcursor);
			
			fl = true;
			
			break;
		
		case setlinetextfunc: {
			Handle htext;
			
			flnextparamislast = true;
			
			if (!getexempttextvalue (hparam1, 1, &htext))
				break;
			
			(*v).data.flvalue = opsetlinetextverb (htext);
			
			fl = true;
			
			break;
			}
		
		case reorgfunc: {
			tydirection dir;
			long units;
			
			if (!getdirectionvalue (hparam1, 1, &dir))
				break;
			
			flnextparamislast = true;
			
			if (!getlongvalue (hparam1, 2, &units))
				break;
			
			(*v).data.flvalue = opreorgcursor (dir, units);
			
			fl = true;
			
			break;
			}
			
		case deletesubsfunc:
			if (!langcheckparamcount (hparam1, 0))
				break;
			
			(*v).data.flvalue = opdeletesubs (hbarcursor);
			
			fl = true;
			
			break;
			
		case deletelinefunc:
			if (!langcheckparamcount (hparam1, 0))
				break;
			
			opdeleteline ();
			
			(*v).data.flvalue = true;
			
			fl = true;
			
			break;
		
		case promotefunc:
			if (!langcheckparamcount (hparam1, 0))
				break;
			
			(*v).data.flvalue = oppromote ();
			
			fl = true;
			
			break;
		
		case demotefunc:
			if (!langcheckparamcount (hparam1, 0))
				break;
			
			(*v).data.flvalue = opdemote ();
			
			fl = true;
			
			break;
			
		case hoistfunc:
			if (!langcheckparamcount (hparam1, 0))
				break;
			
			if (!opanymarked ())
				(*v).data.flvalue = oppushhoist (hbarcursor);
			
			fl = true;
			
			break;
			
		case dehoistfunc:
			if (!langcheckparamcount (hparam1, 0))
				break;
			
			(*v).data.flvalue = oppophoist ();
			
			fl = true;
			
			break;
		
		case makecommentfunc: case uncommentfunc:
			if (!langcheckparamcount (hparam1, 0))
				break;
			
			(*v).data.flvalue = opcommentverb (token == makecommentfunc);
			
			fl = true;
			
			break;
			
		case iscommentfunc:
			if (!langcheckparamcount (hparam1, 0))
				break;
			
			(*v).data.flvalue = (**hbarcursor).flcomment;
			
			fl = true;
			
			break;
		
		case getbreakpointfunc:
			if (!langcheckparamcount (hparam1, 0))
				break;
			
			(*v).data.flvalue = (**hbarcursor).flbreakpoint;
			
			fl = true;
			
			break;
			
		case setbreakpointfunc: case clearbreakpointfunc:
			if (!langcheckparamcount (hparam1, 0))
				break;
			
			(*v).data.flvalue = opbreakpointverb (token == setbreakpointfunc);
			
			fl = true;
			
			break;
		
		/*
		case getmarkedfunc:
			if (!langcheckparamcount (hparam1, 0))
				break;
			
			(*v).data.flvalue = (**hbarcursor).flmarked;
			
			fl = true;
			
			break;
		
		case setmarkedfunc: case clearmarkedfunc:
			if (!langcheckparamcount (hparam1, 0))
				break;
			
			(*v).data.flvalue = opmarkedverb (token == setmarkedfunc);
			
			fl = true;
			
			break;
		*/
		
		case getdisplayfunc:
			if (!langcheckparamcount (hparam1, 0))
				break;
			
			(*v).data.flvalue = !(**ho).flinhibitdisplay;
			
			fl = true;
			
			break;
		
		case setdisplayfunc: {
			boolean fldisplay;
			
			flnextparamislast = true;
			
			if (!getbooleanvalue (hparam1, 1, &fldisplay))
				break;
			
			(*v).data.flvalue = opsetdisplayverb (fldisplay);
			
			fl = true;
			
			break;
			}
		
		case getcursorfunc:
			if (!langcheckparamcount (hparam1, 0))
				break;
			
			fl = setlongvalue ((long) hbarcursor, v);
			
			break;
		
		case setcursorfunc: {
			long cursor;
			
			flnextparamislast = true;
			
			if (!getlongvalue (hparam1, 1, &cursor))
				break;
			
			(*v).data.flvalue = opsetcursorverb (cursor);
			
			fl = true;
			
			break;
			}
		
		case getrefconfunc:
			if (opverbrejectmenubar (bserror))
				break;
			
			if (opverbrejecttable (bserror)) /*7.0b23 PBS*/
				break;
			
			fl = opgetrefconverb (hparam1, hbarcursor, v);
			
			break;
			
		case setrefconfunc:
			if (opverbrejectmenubar (bserror))
				break;
			
			if (opverbrejecttable (bserror)) /*7.0b23 PBS*/
				break;
			
			fl = opsetrefconverb (hparam1, hbarcursor, v);
			
			break;
		
		case getexpansionstatefunc:
			if (!langcheckparamcount (hparam1, 0))
				break;

			fl = opgetexpansionstateverb (v);
			
			break;
			
		case setexpansionstatefunc: {
			tyvaluerecord vlist;
			
			flnextparamislast = true;
			
			if (!getparamvalue (hparam1, 1, &vlist))
				break;

			if (!coercevalue (&vlist, listvaluetype))
				return (false);

			fl = opsetexpansionstateverb (&vlist, v);
			
			break;
			}

		case getscrollstatefunc:
			if (!langcheckparamcount (hparam1, 0))
				break;

			fl = opgetscrollstateverb (v);
			
			break;

		case setscrollstatefunc: {
			long firstline;
			
			flnextparamislast = true;
			
			if (!getlongvalue (hparam1, 1, &firstline))
				break;

			fl = opsetscrollstateverb (firstline, v);
			
			break;
			}

		case getsuboutlinefunc: {
			tyvaluerecord vindent;
			short ctconsumed = 0;
			short ctpositional = 0;
			
			setbooleanvalue (true, &vindent);

			flnextparamislast = true;

			if (!getoptionalparamvalue (hparam1, &ctconsumed, &ctpositional, BIGSTRING ("\x08" "flindent"), &vindent))
				return (false);

			fl = opgetsuboutlineverb (hbarcursor, vindent.data.flvalue, v);
			
			break;
			}
		
		case insertoutlinefunc: {
			
			/*
			7.0b13 PBS: menus can be inserted into menus now.
			*/

			hdloutlinevariable hv;
			short id;
			tydirection dir;
			boolean floutline = true;

			if (!langexternalgetexternalparam (hparam1, 1, &id, (hdlexternalvariable *) &hv))
				floutline = false;

			if (floutline) {

				switch (id) {

					case idoutlineprocessor:
					case idmenuprocessor:
					case idscriptprocessor:
					
						floutline = true;

						break;

					default:

						floutline = false;
					} /*switch*/
				} /*if*/
			
			//if (!langexternalgetexternalparam (hparam1, 1, &id, (hdlexternalvariable *) &hv) || (id != idoutlineprocessor)) {
			if (!floutline) {
					
				bigstring lbserror;
				
				getstringlist (operrorlist, namenotoutlineerror, lbserror);
				
				langerrormessage (lbserror);
				
				return (false);
				}
			
			if (!opverbinmemory (hv))
				return (false);
			
			flnextparamislast = true;
			
			if (!getdirectionvalue (hparam1, 2, &dir)) /*direction*/
				break;

			if (id == idmenuprocessor) { /*menubar*/

				hdlmenurecord hm;
			
				hm = (hdlmenurecord) (**hv).variabledata;

				fl = opinsertoutlineverb ((hdloutlinerecord)(**hm).menuoutline, dir, v);
				} /*if*/
			
			else /*outline*/

				fl = opinsertoutlineverb ((hdloutlinerecord)(**hv).variabledata, dir, v);
			
			break;
			}		
		
		case setmodifiedfunc: { /*7.0b5 PBS: set or un-set dirty bits*/
			
			boolean fldirty;
			
			flnextparamislast = true;
			
			if (!getbooleanvalue (hparam1, 1, &fldirty))
				break;
			
			if (fldirty)
			
				opdirtyoutline ();
						
			else {
			
				register hdloutlinerecord lho = outlinedata;
					
				(**lho).fldirty = false; /*the outline structure is not dirty*/
	
				(**lho).flrecentlychanged = false; /*in case someone is maintaining a parallel structure*/
				} /*else*/
						
			fl = true;
			
			break;			
			}
		
		case getselectfunc: { /*7.0b15 PBS: get list of cursors, the current selection*/
			
			fl = opgetselectverb (hparam1, v);
			
			break;
			}
		
		case getheadnumberfunc: { /*7.0b17 PBS: get the index of the current headline*/

			/*7.0b21: changed name to op.getHeadNumber -- because op.index suggests the search engine*/
			
			if (!langcheckparamcount (hparam1, 0))
				break;

			fl = setlongvalue (opgetheadnumberverb (hbarcursor), vreturned);

			break;
			}
		
		case getselectedsuboutlinesfunc: { /*7.0b18 PBS: get selected headlines as an outline*/
			
			hdloutlinerecord lho;
			
			flnextparamislast = true;
			
			/*Get outline parameter.*/
	
			if (getoutlinevalue (hparam1, 1, &lho)) /*First param is an outline to add to*/
				
				fl = opgetselectedsuboutlinesverb (lho);
			
			setbooleanvalue (fl, vreturned);

			break;
			}
		
		case sethtmlformattingfunc: { /*7.0b28: turn on and off HTML formatting.*/

			boolean lfl = false;
			boolean flchanged = false;

			flnextparamislast = true;

			if (!getbooleanvalue (hparam1, 1, &lfl))
				break;
			
			if ((**outlinedata).outlinetype == outlineisoutline) { /*outlines only*/
				
				if ((**outlinedata).flhtml != lfl) { /*Don't bother if it's already set as requested.*/
					
					long startsel, endsel;

					boolean fltextmode = (**outlinedata).fltextmode;

					if (fltextmode) {

						wpgetselection (&startsel, &endsel);

						opunloadeditbuffer ();
						}

					(**outlinedata).flhtml = lfl;

					opdirtyview ();

					opdirtymeasurements ();

					opinvaldisplay ();
					
					if (fltextmode) {

						oploadeditbuffer ();

						wpsetselection (startsel, endsel); /*restore selection*/
						}

					flchanged = true;
					} /*if*/
				} /*if*/

			lfl = setbooleanvalue (flchanged, vreturned);

			break;
			}

		case gethtmlformattingfunc: { /*7.0b28 PBS: return true if HTML formatting is on.*/

			boolean flhtml;

			flhtml = (**outlinedata).flhtml;

			if ((**outlinedata).outlinetype != outlineisoutline) /*outlines only*/
				flhtml = false;

			fl = setbooleanvalue (flhtml, vreturned);

			break;
			}
		
		case setdynamicfunc: { /*7.0b30 PBS: set the dynamic flag for a headline record.*/
			
			boolean fldynamic;

			flnextparamislast = true;

			if (!getbooleanvalue (hparam1, 1, &fldynamic))
				break;

			(**hbarcursor).fldynamic = fldynamic;

			fl = setbooleanvalue (true, vreturned);

			break;
			}

		case getdynamicfunc: { /*7.0b30 PBS: get the dynamic flag for a headline record.*/

			boolean fldynamic = (**hbarcursor).fldynamic;

			fl = setbooleanvalue (fldynamic, vreturned);

			break;
			}

		/*7.0b16 PBS: opattributes verbs*/
		
		case addgroupfunc: { /*7.0b16 PBS: op.attributes.addGroup*/
			
			
			
			break;
			}
		
		case getallfunc: { /*7.0b16 PBS: op.attributes.getAll*/
			
			hdlhashtable ht;
			bigstring bsvarname;
			tyvaluerecord val;

			flnextparamislast = true;

			if (!getvarparam (hparam1, 1, &ht, bsvarname))
				break;
			
			if (!opattributesgetpackedtablevalue (hbarcursor, &val)) {
			
				break;
				}
			
			if (!langsetsymboltableval (ht, bsvarname, val)) {
		
				disposevaluerecord (val, false);
		
				break;
				}
			
			fl = true;
			
			break;
			}
		
		case getonefunc: { /*7.0b16 PBS: op.attributes.getOne*/
		
			bigstring bsattname, bsvarname;
			tyvaluerecord val;
			hdlhashtable ht;
			
			fl = true;
						
			if (!getstringvalue (hparam1, 1, bsattname))
				break;
		
			flnextparamislast = true;
			
			if (!getvarparam (hparam1, 2, &ht, bsvarname))
				break;
			
			(*v).data.flvalue = opattributesgetoneattribute (hbarcursor, bsattname, &val);
			
			if ((*v).data.flvalue) {
				
				if (!langsetsymboltableval (ht, bsvarname, val)) {
		
					disposevaluerecord (val, false);

					break;
					} /*if*/
				} /*if*/

			break;
			}
		
		case makeemptyfunc: { /*7.0b16 PBS: op.attributes.makeEmpty*/
			
			
			break;
			}
		
		case setonefunc: { /*7.0b16 PBS: op.attributes.setOne*/
			
			
			break;
			}
	
		} /*switch*/
	
	shellupdatescrollbars (outlinewindowinfo);
			
	shellpopglobals ();
			
	return (fl);
	
	
	error:
	
	if (errornum != 0) /*get error string*/
		getstringlist (operrorlist, errornum, bserror);
	
	return (false);
	} /*opfunctionvalue*/


static boolean opinitverbs (void) {
	
	return (loadfunctionprocessor (idopverbs, &opfunctionvalue));
	} /*opinitverbs*/



static void opverbresize (void) {
	
	opresize ((**outlinewindowinfo).contentrect);
	
	} /*opverbresize*/


boolean opverbclose (void) {
	
	register hdloutlinerecord ho = outlinedata;
	register hdloutlinevariable hv;
	
	if (ho == nil) //already closed & disposed?
		return (true);
	
	hv = (hdloutlinevariable) (**ho).outlinerefcon;
	
	langexternalwindowclosed ((hdlexternalvariable) hv);
	
	opverbcheckwindowrect (ho);
	
	killundo (); /*must toss undos before they're stranded*/
	
	if ((**ho).fldirty || isfilewindow (shellwindow)) { /*we have to keep the in-memory version around*/
		
		(**ho).flwindowopen = false;
		
		opcloseoutline (); /*prepare for dormancy, not in a window anymore*/
		}
	
	else { /*the db version is identical to memory version, save memory*/
		
		if (hv == nil) /*defensive driving*/
			return (true);
		
		opverbunload ((hdlexternalvariable) hv, (**hv).oldaddress);
		
		//opsetoutline (nil); // 4.15.97 dmb
		}
	
	return (true);
	} /*opverbclose*/


boolean opverbfind (hdlexternalvariable hvariable, boolean *flzoom) {
#pragma unused(flzoom)

	register hdloutlinevariable hv = (hdloutlinevariable) hvariable;
	register hdloutlinerecord ho;
	register boolean fl;
	boolean fltempload;
	boolean flwindowopen;
	hdlwindowinfo hinfo;
	boolean fldisplaywasenabled = true;
	
	fltempload = !(**hv).flinmemory;
	
	if (!opverbinmemory (hv))
		return (false);
	
	ho = (hdloutlinerecord) (**hv).variabledata;
	
	flwindowopen = opwindowopen ((hdlexternalvariable) hv, &hinfo) && hinfo;
	
	if (flwindowopen)
		shellpushglobals ((**hinfo).macwindow);
	
	else {
		shellpushglobals (nil);
		
		oppushoutline (ho);
		
		fldisplaywasenabled = opdisabledisplay ();
		}
	
	fl = opflatfind (true, false);
	
	if (!flwindowopen) {
		
		if (fldisplaywasenabled)
			openabledisplay ();
		
		oppopoutline ();
		}
	
	shellpopglobals ();
	
	if (fltempload && !fl && !(**ho).fldirty)
		opverbunload ((hdlexternalvariable) hv, (**hv).oldaddress);
	
	return (fl);
	} /*opverbfind*/


static boolean opverbsearch (void) {
	
	register long oprefcon = (**outlinedata).outlinerefcon;
	
	startingtosearch (oprefcon); /*will be accepted if not part of larger search*/
	
	if (opflatfind (false, searchshouldwrap (oprefcon)))
		return (true);
	
	if (!searchshouldcontinue (oprefcon))
		return (false);
	
	return (langexternalcontinuesearch ((hdlexternalvariable) oprefcon));
	} /*opverbsearch*/


boolean opverbruncursor (void) {
	
	bigstring bs, bsresult;
	
	opgetheadstring ((**outlinedata).hbarcursor, bs);
	
	return (langrunstring (bs, bsresult));
	} /*opverbruncursor*/


boolean opverbgetvariable (hdlexternalvariable *hvariable) {
	
	if (outlinedata == nil)
		return (false);
	
	*hvariable = (hdlexternalvariable) (**outlinedata).outlinerefcon;
	
	return (true);
	} /*opverbgetvariable*/


static boolean opverbtitleclick (Point pt) {
	
	return (langexternaltitleclick (pt, (hdlexternalvariable) (**outlinedata).outlinerefcon));
	} /*opverbtitleclick*/


static void opverbidle (void) {
	
	opidle ();
	} /*opverbidle*/


static boolean opverbsetfont (void) {
	
	return (opsetfont ((**outlinewindowinfo).selectioninfo.fontnum));
	} /*opverbsetfont*/


static boolean opverbsetsize (void) {
	
	return (opsetsize ((**outlinewindowinfo).selectioninfo.fontsize));
	} /*opverbsetsize*/


boolean opverbgettargetdata (short id) {
	
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
	} /*opverbgettargetdata*/


static boolean opverbkeystroke (void) {

	if (langexternalsurfacekey ((hdlexternalvariable) (**outlinedata).outlinerefcon))
		return (true);
	
	return (opkeystroke ());
	} /*opverbkeystroke*/



static boolean opmenuroutine (short idmenu, short ixmenu) {
	
	/*
	carefull - we have no idea what window is in front, we must 
	only take action if our globals are set
	
	return false if we successfully handle the menu item, so no further 
	action is taken
	*/
	
	tydirection dir = nodirection;
	
	if ((idmenu == virtualmenu) &&
		(outlinewindow == shellwindow) && 
		(outlinedata != NULL)) {

		switch (ixmenu) {
			case moveupitem:
				dir = up;
				break;

			case movedownitem:
				dir = down;
				break;

			case moveleftitem:
				dir = left;
				break;

			case moverightitem:
				dir = right;
				break;
			}
		}
	
	if (dir == nodirection)
		return (true);
	
	opreorgcursor (dir, 1);

	return (false);
	} /*opmenuroutine*/


boolean opstart (void) {
	
	/*
	set up callback routines record, and link our data into the shell's 
	data structure.
	*/
	
	ptrcallbacks callbacks;
	register ptrcallbacks cb;
	
	assert (sizeof (tyexternalvariable) == sizeof (tyoutlinevariable) - sizeof (Handle));
	
	if (!opinitverbs ())
		return (false);
	
	opinitdisplayvariables ();
	
	
	shellpushscraphook (&opscraphook);
	
	shellpushmenuhook (&opmenuroutine);

	shellnewcallbacks (&callbacks);
	
	cb = callbacks; /*copy into register*/
	
	loadconfigresource (idoutlineconfig, &(*cb).config);
	
	(*cb).configresnum = idoutlineconfig;
		
	(*cb).windowholder = &outlinewindow;
	
	(*cb).dataholder = (Handle *) &outlinedataholder;
	
	(*cb).infoholder = &outlinewindowinfo;
	
	(*cb).setglobalsroutine = &opverbsetglobals;
	
	(*cb).pushroutine = &oppushglobals;

	(*cb).poproutine = &oppopglobals;
	
	
	(*cb).disposerecordroutine = ccdisposefilerecord;
	
	(*cb).saveroutine = ccsavespecialfile;

	
	(*cb).updateroutine = &opupdate;
	
	(*cb).activateroutine = &opactivate;
	
	(*cb).getcontentsizeroutine = &opgetoutinesize;
	
	(*cb).resizeroutine = &opverbresize;
	
	(*cb).scrollroutine = &opscroll;
	
	(*cb).setscrollbarroutine = &opresetscrollbars;
	
	(*cb).mouseroutine = &opmousedown;

	(*cb).rmouseroutine = &oprmousedown; /*7.0b10 PBS: right-click menus*/
	
	(*cb).keystrokeroutine = &opverbkeystroke;
	
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
	
	(*cb).idleroutine = &opverbidle;
	
	(*cb).adjustcursorroutine = &opsetcursor;
	
	(*cb).gettargetdataroutine = &opverbgettargetdata;
	
	(*cb).setprintinfoproutine = &opsetprintinfo;
	
	(*cb).beginprintroutine = &opbeginprint;
	
	(*cb).endprintroutine = &opendprint;
	
	(*cb).printroutine = &opprint;
	
	(*cb).titleclickroutine = &opverbtitleclick;
	
	(*cb).getvariableroutine = (shellgetvariablecallback) &opverbgetvariable;
	
	(*cb).settextmoderoutine = &opsettextmode;
	
	(*cb).searchroutine = &opverbsearch;
	
	(*cb).executeroutine = &opverbruncursor;
	
	(*cb).cmdkeyfilterroutine = &opcmdkeyfilter;
	
	(*cb).closeroutine = &opverbclose;

	(*cb).buttonstatusroutine = &opbuttonstatus; /*7.1b18 PBS*/

	(*cb).buttonroutine = &opbutton; /*7.1b18 PBS*/
	
	
	return (true);
	} /*opstart*/






