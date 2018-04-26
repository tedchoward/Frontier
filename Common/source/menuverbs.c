
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
#include "quickdraw.h"
#include "strings.h"
#include "kb.h"
#include "menu.h"
#include "ops.h"
#include "resources.h"
#include "search.h"
#include "shell.h"
#include "shellhooks.h"
#include "shellmenu.h"
#include "shellundo.h"
#include "lang.h"
#include "langexternal.h"
#include "tablestructure.h"
#include "menueditor.h"
#include "opinternal.h"
#include "opverbs.h"
#include "menuinternal.h"
#include "meprograms.h"
#include "menuverbs.h"
#include "cancoon.h"
#include "kernelverbdefs.h"
#include "file.h"



#define menustringlist 166
#define menutypestring 1
#define menusizestring 2


#define menuerrorlist 266
#define nomenuerror 1
#define namenotmenuerror 2


typedef enum tymenutoken { /*verbs that are processed by menueditor.c*/
	
	zoomscriptfunc,
	
	buildmenubarfunc,
	
	clearmenubarfunc,
	
	isinstalledfunc,
	
	installfunc,
	
	removefunc,
	
	getscriptfunc,
	
	setscriptfunc,
	
	addmenucommandfunc,
	
	deletemenucommandfunc,
	
	addsubmenufunc,
	
	deletesubmenufunc,
	
	getcommandkeyfunc,
	
	setcommandkeyfunc,
	
	ctmenuverbs
	} tymenutoken;

#if langexternalfind_optimization

	typedef tyexternalvariable tymenuvariable, *ptrmenuvariable, **hdlmenuvariable;

#else

#pragma pack(2)
typedef struct tymenuvariable { /*7.0b6 PBS: moved from menuverbs.c*/
	
	unsigned short id; /*tyexternalid: managed by langexternal.c*/
	
	unsigned short flinmemory: 1; /*if true, variabledata is an hdlmenurecord, else a dbaddress*/
	
	unsigned short flmayaffectdisplay: 1; /*not in memory, but being displayed in a table window*/
	
	long variabledata; /*either a hdlmenurecord or a dbaddress*/
	
	hdldatabaserecord hdatabase; // 5.0a18 dmb

	dbaddress oldaddress; /*last place this menubar doc was stored in db*/
	} tymenuvariable, *ptrmenuvariable, **hdlmenuvariable;
#pragma options align=reset

#endif

		
static short errornum = 0; /*error number exclusively for menu routines*/



boolean menuverbgettypestring (hdlexternalvariable hvariable, bigstring bs) {
#pragma unused (hvariable)

	getstringlist (menustringlist, menutypestring, bs);
	
	return (true);
	} /*menuverbgettypestring*/
	

static boolean newmenuvariable (boolean flinmemory, long variabledata, hdlmenuvariable *h) {

	return (langnewexternalvariable (flinmemory, variabledata, (hdlexternalvariable *) h));
	} /*newmenuvariable*/


boolean menuverbunload (hdlexternalvariable hvariable) {
	
	register hdlmenuvariable hv = (hdlmenuvariable) hvariable;
	
	if ((**hv).flinmemory) { /*if it's on disk, don't need to do anything*/
		
		medisposemenurecord ((hdlmenurecord) (**hv).variabledata, false);
		
		(**hv).variabledata = (**hv).oldaddress;
		
		(**hv).oldaddress = 0;
		
		(**hv).flinmemory = false;
		}
	
	return (true);
	} /*menuverbunload*/


static boolean menuverbinmemory (hdlmenuvariable hvariable) {
	
	/*
	5.0a18 dmb: support database linking
	*/
	
	register hdlmenuvariable hv = hvariable;
	register dbaddress adr;
	hdlmenurecord hmenurecord;
	boolean fl;
	
	if ((**hv).flinmemory)
		return (true);
	
	dbpushdatabase ((**hv).hdatabase);

	adr = (dbaddress) (**hv).variabledata;
	
	fl = meloadmenurecord (adr, &hmenurecord);

	dbpopdatabase ();

	if (!fl)
		return (false);
	
	(**hv).variabledata = (long) hmenurecord;
	
	(**hv).oldaddress = adr;
	
	(**hv).flinmemory = true;
	
	(**hmenurecord).menurefcon = (long) hv; /*we can get from menu rec to variable rec*/
	
	return (true);
	} /*menuverbinmemory*/


boolean menuverbgetsize (hdlexternalvariable hvariable, long *size) {
	
	register hdlmenuvariable hv = (hdlmenuvariable) hvariable;
	register hdlmenurecord hm;
	short ctheads;
	
	if (!menuverbinmemory (hv))
		return (false);
	
	hm = (hdlmenurecord) (**hv).variabledata;
	
	oppushoutline ((**hm).menuoutline);
	
	ctheads = opcountheads ();
	
	oppopoutline ();
	
	*size = (long) ctheads;
	
	return (true);
	} /*menuverbgetsize*/


boolean menuverbgetdisplaystring (hdlexternalvariable hvariable, bigstring bs) {
	
	/*
	get string for table display.  variable is in memory
	*/
	
	long ctheads;
	
	if (!menuverbgetsize (hvariable, &ctheads))
		return (false);
	
	parsenumberstring (menustringlist, menusizestring, ctheads, bs);

	/*
	numbertostring (ctheads, bs);
	
	if (ctheads != 1)
		pushstring ("\p items", bs);
	else
		pushstring ("\p item", bs);
	*/
	
	return (true);
	} /*menuverbgetdisplaystring*/


boolean menuverbisdirty (hdlexternalvariable hvariable) {
	
	register hdlmenuvariable hv = (hdlmenuvariable) hvariable;
	
	if (!(**hv).flinmemory) /*it's on disk, can't be dirty*/
		return (false);
	
	return (mesomethingdirty ((hdlmenurecord) (**hv).variabledata));
	} /*menuverbisdirty*/


boolean menuverbsetdirty (hdlexternalvariable hvariable, boolean fldirty) {
	
	/*
	4/15/92 dmb: see comments in langexternalsetdirty
	
	note: it may be that this routine should clean every script when 
	fldirty is false
	*/
	
	register hdlmenuvariable hv = (hdlmenuvariable) hvariable;
	
	if (!menuverbinmemory (hv))
		return (false);
	
	(**(hdlmenurecord) (**hv).variabledata).fldirty = fldirty;
	
	return (true);
	} /*menuverbsetdirty*/


boolean menuverbmemorypack (hdlexternalvariable hvariable, Handle *hpacked) {
	
	register hdlmenuvariable hv = (hdlmenuvariable) hvariable;
	register hdlmenurecord hm;
	Handle hpush;
	register boolean fl;
	boolean fltempload;
	
	fltempload = !(**hv).flinmemory;
	
	if (!menuverbinmemory (hv))
		return (false);
	
	hm = (hdlmenurecord) (**hv).variabledata;
	
	fl = mesavemenurecord (hm, false, true, nil, &hpush);
	
	if (fltempload)
		menuverbunload ((hdlexternalvariable) hv);
	
	if (fl) {
		
		fl = pushhandle (hpush, *hpacked);
		
		disposehandle (hpush);
		}
	
	return (fl);
	} /*menuverbmemorypack*/


boolean menuverbmemoryunpack (Handle hpacked, long *ixload, hdlexternalvariable *h) {
	
	/*
	create a new outline variable -- not in memory.
	
	this is a special entrypoint for the pack and unpack verbs.
	*/
	
	Handle hpackedmenu;
	hdlmenurecord hmenurecord;
	register hdlmenurecord hm;
	
	if (!loadhandleremains (*ixload, hpacked, &hpackedmenu))
		return (false);
	
	if (!meunpackmenustructure (hpackedmenu, &hmenurecord)) /*always disposes of hpackedmenu*/
		return (false);
	
	hm = hmenurecord; /*move into register*/
	
	if (!newmenuvariable (true, (long) hm, (hdlmenuvariable *) h)) {
		
		medisposemenurecord (hm, false);
		
		return (false);
		}
	
	(**hm).menurefcon = (long) *h; /*point back to variable*/
	
	(**hm).fldirty = true;
	
	return (true);
	} /*menuverbmemoryunpack*/


boolean menuverbpack (hdlexternalvariable hvariable, Handle *hpacked, boolean *flnewdbaddress) {

	/*
	6.2a15 AR: added flnewdbaddress parameter
	*/

	register hdlmenuvariable hv = (hdlmenuvariable) hvariable;
	register hdlmenurecord hm;
	dbaddress adr;
	boolean fl;
	boolean fltempload = false;
	boolean flpreservelinks;
	hdlwindowinfo hinfo;
	
	if (fldatabasesaveas) {
		
		fltempload = !(**hv).flinmemory;
		
		if (!menuverbinmemory (hv))
			return (false);
			
		*flnewdbaddress = true;
		}
	
	if (!(**hv).flinmemory) { /*simple case, menu is resident in the db*/
		
		adr = (dbaddress) (**hv).variabledata;
		
		*flnewdbaddress = false;
		
		goto pushaddress;
		}
	
	adr = (**hv).oldaddress; /*place where this menubar used to be stored*/
	
	hm = (hdlmenurecord) (**hv).variabledata;
	
	flpreservelinks = fldatabasesaveas && !fltempload;
	
	fl = mesavemenurecord (hm, flpreservelinks, false, &adr, nil);
	
	if (fltempload)
		menuverbunload ((hdlexternalvariable) hv);
	
	if (!fl)
		return (false);
	
	if (fldatabasesaveas)
		goto pushaddress;

	*flnewdbaddress = ((**hv).oldaddress != adr);
	
	(**hv).oldaddress = adr;
	
	(**hm).fldirty = false; /*we just saved off a new db version*/
	
	if (menuwindowopen ((hdlexternalvariable) hv, &hinfo) && (hinfo != nil))
		shellsetwindowchanges (hinfo, false);
	
	pushaddress:
	
	return (pushlongondiskhandle (adr, *hpacked));
	} /*menuverbpack*/


boolean menuverbunpack (Handle hpacked, long *ixload, hdlexternalvariable *h) {

	dbaddress adr;
	
	if (!loadlongfromdiskhandle (hpacked, ixload, &adr)) 
		return (false);
		
	return (newmenuvariable (false, adr, (hdlmenuvariable *) h));
	} /*menuverbunpack*/


boolean menuverbpacktotext (hdlexternalvariable hvariable, Handle htext) {
	
	/*
	5.0.2b20 dmb: unload if just loaded
	*/
	
	register hdlmenuvariable hv = (hdlmenuvariable) hvariable;
	register hdlmenurecord hm;
	boolean fl;
	boolean fltempload = !(**hv).flinmemory;
	
	if (!menuverbinmemory (hv))
		return (false);
	
	hm = (hdlmenurecord) (**hv).variabledata;
	
	fl = opoutlinetotextscrap ((**hm).menuoutline, false, htext);
	
	if (fltempload)
		menuverbunload ((hdlexternalvariable) hv);
	
	return (fl);
	} /*menuverbpacktotext*/


boolean menuverbgettimes (hdlexternalvariable h, long *timecreated, long *timemodified) {
	
	register hdlmenuvariable hv = (hdlmenuvariable) h;
	register hdlmenurecord hm;
	register hdloutlinerecord ho;
	
	if (!menuverbinmemory (hv))
		return (false);
	
	hm = menudata = (hdlmenurecord) (**hv).variabledata;
	
	ho = (**hm).menuoutline;
	
	*timecreated = (**ho).timecreated;
	
	*timemodified = (**ho).timelastsave;
	
	return (true);
	} /*menuverbgettimes*/


boolean menuverbsettimes (hdlexternalvariable h, long timecreated, long timemodified) {
	
	register hdlmenuvariable hv = (hdlmenuvariable) h;
	register hdlmenurecord hm;
	register hdloutlinerecord ho;
	
	if (!menuverbinmemory (hv))
		return (false);
	
	hm = menudata = (hdlmenurecord) (**hv).variabledata;
	
	ho = (**hm).menuoutline;
	
	(**ho).timecreated = timecreated;
	
	(**ho).timelastsave = timemodified;
	
	return (true);
	} /*menuverbsettimes*/


static boolean mefindusedblocksvisit (hdlheadrecord hnode, ptrvoid refcon) {
	
	/*
	if there's a script attached to hnode, note the database usage.
	*/
	
	ptrstring bsparent = (ptrstring) refcon;
	tymenuiteminfo item;
	bigstring bspath;
	
	if (megetmenuiteminfo (hnode, &item)) { /*something linked*/
		
		opgetheadstring (hnode, bspath);
		
		pushchar (']', bspath);

		insertstring (BIGSTRING ("\x02" " ["), bspath);

		insertstring (bsparent, bspath);

		return (statsblockinuse (item.linkedscript.adrlink, bspath));
		}
	
	return (true); /*keep visiting*/
	} /*mefindusedblocksvisit*/


boolean menuverbfindusedblocks (hdlexternalvariable hvariable, bigstring bspath) {
	
	/*
	note the database usage for the entire menubar outline.  see dbstats.c
	*/
	
	register hdlmenuvariable hv = (hdlmenuvariable) hvariable;
	register hdlmenurecord hm;
	register hdloutlinerecord ho;
	register boolean fl;
	boolean fltempload;
	hdlheadrecord hcursor;
	hdlheadrecord hsummit;
	
	fltempload = !(**hv).flinmemory;
	
	if (!menuverbinmemory (hv))
		return (false);
	
	if (!statsblockinuse ((**hv).oldaddress, bspath))
		return (false);
	
	hm = menudata = (hdlmenurecord) (**hv).variabledata; /*move into register, global*/
	
	statsblockinuse ((**hm).adroutline, bspath);
	
	ho = (**hm).menuoutline;
	
	oppushoutline (ho);
	
	hcursor = (**ho).hbarcursor;
	
	oppopallhoists (); /*pop hoists, save state to be restored after saving*/
	
	(**ho).hbarcursor = hcursor; /*scotch tape!*/
	
	opoutermostsummit (&hsummit);
	
	fl = opsiblingvisiter (hsummit, false, &mefindusedblocksvisit, bspath);
	
	oprestorehoists (); /*restore the hoist state*/
	
	oppopoutline (); /*pop outline globals*/
	
	if (fltempload)
		menuverbunload ((hdlexternalvariable) hv);
	
	return (fl);
	} /*menuverbfindusedblocks*/


boolean menuverbfind (hdlexternalvariable hvariable, boolean *flzoom) {
	
	/*
	5.1.5 dmb: fixed searching in guest databases
	*/
	
	register hdlmenuvariable hv = (hdlmenuvariable) hvariable;
	register hdlmenurecord hm, hmsave = menudata;
	register hdloutlinerecord ho;
	register boolean fl;
	boolean fltempload;
	boolean flwindowopen;
	hdlwindowinfo hinfo;
	boolean fldisplaywasenabled = true;
	
	fltempload = !(**hv).flinmemory;
	
	if (!menuverbinmemory (hv))
		return (false);
	
	hm = (hdlmenurecord) (**hv).variabledata;
	
	ho = (**hm).menuoutline;

	flwindowopen = menuwindowopen ((hdlexternalvariable) hv, &hinfo);
	
	if (flwindowopen && (hinfo != nil))
		shellpushglobals ((**hinfo).macwindow);
	
	else {
		shellpushglobals (nil);
		
		menudata = hm; //5.1.5 dmb: for search refcon callback, setting database
		
		oppushoutline (ho);
		
		fldisplaywasenabled = opdisabledisplay (); /*we haven't set up display context*/
		}
	
	fl = mesearchoutline (true, false, flzoom);
	
	if (!flwindowopen) {
		
		if (fldisplaywasenabled)
			openabledisplay ();
		
		oppopoutline ();
		
		menudata = hmsave;
		}
	
	shellpopglobals ();
	
	if (fltempload && !fl && !(**ho).fldirty)
		menuverbunload ((hdlexternalvariable) hv);
	
	return (fl);
	} /*menuverbfind*/


boolean menunewmenubar (hdlhashtable htable, bigstring bs, hdlmenurecord *hnewrecord) {

	/*
	create a new menubar variable in the indicated table 
	with the indicated name.  returns a handle to the new
	menurecord.
	*/
	
	register hdlmenurecord hm;
	hdlmenuvariable hv;
	
	if (!menewmenurecord (hnewrecord))
		return (false);
		
	hm = *hnewrecord; /*copy into register*/
		
	if (!newmenuvariable (true, (long) hm, &hv)) {
		
		medisposemenurecord (hm, false);
		
		return (false);
		}
		
	(**hv).variabledata = (long) hm; /*link the menu rec into the variable rec*/
	
	(**hm).menurefcon = (long) hv; /*the pointing is mutual*/
	
	if (!langsetexternalsymbol (htable, bs, idmenuprocessor, (Handle) hv)) {
		
		medisposemenurecord (hm, false);
		
		disposehandle ((Handle) hv);
		
		return (false);
		}
	
	return (true);
	} /*menunewmenubar*/


static boolean menugetmenuvariable (hdlhashtable htable, bigstring bs, boolean *fltypeerror, hdlmenuvariable *hvariable) {

	/*
	return a handle to the menubar variable indicated by [htable, bs].  returns false
	if there isn't a variable with the indicated name, or if itsn't of type menubar.
	
	7/7/90 DW: return a flag to indicate if an error was caused by an undefined name
	or if a variable with the indicated name exists and is the wrong type.  the caller
	may want to do something different depending on which error.
	*/
	
	hdlhashnode hnode;
	register boolean fl;
	tyvaluerecord val;
	register hdlexternalvariable hv;
	
	pushhashtable (htable);
	
	fl = langfindsymbol (bs, &htable, &hnode);
	
	pophashtable ();
	
	if (!fl) {
		
		*fltypeerror = false; /*it's an undefined variable error*/
		
		return (false);
		}
	
	*fltypeerror = true; /*if we return false below, it is a type error*/
	
	val = (**hnode).val;
	
	if (val.valuetype != externalvaluetype) /*it's defined, but it's not a menubar*/
		return (false);
		
	hv = (hdlexternalvariable) val.data.externalvalue;
	
	if ((**hv).id != idmenuprocessor) /*it's defined, it's external, but wrong type*/
		return (false);
		
	*hvariable = (hdlmenuvariable) hv;
	
	*fltypeerror = false; /*no error*/
	
	return (true);
	} /*menugetmenuvariable*/
			

boolean menugetmenubar (hdlhashtable htable, bigstring bs, boolean flcreate, hdlmenurecord *hmenubar) {

	/*
	find the indicated menubar or create an empty one.
	*/
	
	boolean fltypeerror;
	hdlmenuvariable hvariable;
	
	if (!menugetmenuvariable (htable, bs, &fltypeerror, &hvariable)) {
		
		if (!flcreate)
			return (false);
		
		if (fltypeerror) { /*don't over-write an existing variable of wrong type*/
			
			/*xxx -- error dialog goes here*/
			
			return (false);
			}
			
		return (menunewmenubar (htable, bs, hmenubar));
		}
	
	if (!menuverbinmemory (hvariable)) /*error swapping it in*/
		return (false);
		
	*hmenubar = (hdlmenurecord) (**hvariable).variabledata;
	
	return (true);
	} /*menugetmenubar*/


static boolean getmenuparam (hdltreenode hfirst, short pnum, hdlmenuvariable *hv) {
	
	short id;
	
	if (!langexternalgetexternalparam (hfirst, pnum, &id, (hdlexternalvariable *) hv)) {
		
		errornum = namenotmenuerror;
		
		return (false);
		}
	
	if (id != idmenuprocessor) {
		
		errornum = namenotmenuerror;
		
		return (false);
		}
	
	return (true);
	} /*getmenuparam*/
	

boolean menuwindowopen (hdlexternalvariable hvariable, hdlwindowinfo *hinfo) {

	/*
	5.0b9 dmb: for clipboard operations, must test hm for nil
	*/
	
	register hdlmenuvariable hv = (hdlmenuvariable) hvariable;
	hdlmenurecord hm;
	
	if (!(**hv).flinmemory)
		return (false);
	
	hm = (hdlmenurecord) (**hv).variabledata;
	
	if (hm == nil)
		return (false);

	shellfinddatawindow ((Handle) (**hv).variabledata, hinfo);
	
	return ((**hm).flwindowopen);
	} /*menuwindowopen*/


boolean menuedit (hdlexternalvariable hvariable, hdlwindowinfo hparent, ptrfilespec fs, bigstring bstitle, rectparam rzoom) {
	
	//
	// 2006-09-16 creedon: on Mac, set window proxy icon
	//
	// 5.0d19 dmb: set flwindowopen; use locals, not menu globals.
	//
	
	register hdlmenuvariable hv = (hdlmenuvariable) hvariable;
	hdlmenurecord hm;
	Rect rwindow;
	WindowPtr w;
	hdlwindowinfo hi;
	
	if ((**hv).flinmemory) {
			
		if (shellfinddatawindow ((Handle) (**hv).variabledata, &hi)) {
			
			if ((*rzoom).top > -2)
				shellbringtofront (hi);
			
			return (true);
			}
		}
		
	if (!menuverbinmemory (hv)) // error swapping menurecord into memory
		return (false);
		
	hm = (hdlmenurecord) (**hv).variabledata;
	
	rwindow = (**hm).menuwindowrect; // window comes up where it was last time
	
	if (!newchildwindow (idmenueditorconfig, hparent, &rwindow, rzoom, bstitle, &w)) 
		return (false);
	
	getwindowinfo (w, &hi);
	
	(**hi).hdata = (Handle) hm; // link data into shell's structure
	
	if ( fs != nil ) {
	
		(**hi).fspec = *fs;
		
		
			if (macfilespecisresolvable (fs))
				SetWindowProxyCreatorAndType ( w, 'LAND', 'FTmb', kOnSystemDisk );
				
		
		}
	
	shellpushglobals (w);
	
	meeditmenurecord ();
	
	shellpopglobals ();
	
	(**hm).flwindowopen = true;
	
	if ((**hm).fldirty)
		shellsetwindowchanges (hi, true);
	
	windowzoom (w); // show the window to the user
	
	return (true);
	
	} // menuedit


static boolean menudisposevariable (hdlexternalvariable hvariable, boolean fldisk) {
	
	medisposemenurecord ((hdlmenurecord) (**hvariable).variabledata, fldisk);
	
	return (true);
	} /*menudisposevariable*/


boolean menuverbdispose (hdlexternalvariable hvariable, boolean fldisk) {
	
	/*
	12/22/91 dmb: in order to release all db nodes properly, must force 
	menubar to be loaded when fldisk is true
	*/
	
	register hdlexternalvariable hv = hvariable;
	
	if (fldisk) { /*load menubar into memory so that scripts can release their db nodes*/
		
		if (!menuverbinmemory ((hdlmenuvariable) hv))
			return (false);
		}
	
	return (langexternaldisposevariable (hv, fldisk, &menudisposevariable));
	} /*menuverbdispose*/


boolean menuverbnew (Handle hdata, hdlexternalvariable *hvariable) {
	
	/*	
	12/29/91 dmb: added call to opsetctexpanded()
	*/
	
	register hdlmenurecord hm;
	register hdlmenuvariable hv;
	hdlmenurecord hnewrecord;
	
	if (!menewmenurecord (&hnewrecord))
		return (false);
	
	hm = hnewrecord; /*copy into register*/
	
	if (!newmenuvariable (true, (long) hm, (hdlmenuvariable *) hvariable)) {
		
		medisposemenurecord (hm, false);
		
		return (false);
		}
	
	hv = (hdlmenuvariable) *hvariable; /*copy into register*/
	
	if (hdata != nil) {
		
		register hdloutlinerecord ho = (**hm).menuoutline;
		register hdloutlinerecord hsource = (hdloutlinerecord) hdata;
		hdlheadrecord hsummit = nil;
		
		oppushoutline (ho);
		
		opcopysiblings ((**hsource).hsummit, &hsummit);
		
		if (hsummit == nil) {
			
			oppopoutline ();
			
			medisposemenurecord (hm, false);
			
			disposehandle ((Handle) hv);
			
			return (false);
			}
		
		opsetsummit (ho, hsummit);
		
		opcopyformatting (hsource, ho);
		
		opdirtymeasurements (); //6.0a14 dmb
		
		opsetctexpanded (ho);
		
		oppopoutline ();
		}
	
	(**hv).variabledata = (long) hm; /*link the menu rec into the variable rec*/
	
	(**hm).menurefcon = (long) hv; /*the pointing is mutual*/
	
	return (true);
	} /*menuverbnew*/


static boolean menubuildverb (void) {
	
	/*
	rebuild the menubar -- the active flag must be set in order for the changes
	to appear in the menubar.
	
	4/22/91 dmb: use new rebuildmenubarlist
	*/
	
	return (rebuildmenubarlist ());
	} /*menubuildverb*/


static boolean menuclearverb (void) {
	
	/*
	remove all of the menus from the menu bar
	*/
	
	return (meclearmenubar ());
	} /*menuclearverb*/


static boolean getmenuparaminmemory (hdltreenode hparam1, hdlmenurecord *hmenurecord) {
	
	/*
	the first parameter in hparam1 should point to a menu record.
	
	try to load it.
	*/
	
	hdlmenuvariable hv;
	
	if (!getmenuparam (hparam1, 1, &hv))
		return (false);
	
	if (!menuverbinmemory (hv))
		return (false);
	
	*hmenurecord = (hdlmenurecord) (**hv).variabledata;
	
	return (true);
	} /*getmenuparaminmemory*/


static boolean menuisinstalledverb (hdltreenode hparam1, tyvaluerecord *v) {
	
	hdlmenurecord hmenurecord;
	
	flnextparamislast = true;
	
	if (!getmenuparaminmemory (hparam1, &hmenurecord))
		return (false);
	
	(*v).data.flvalue = (**hmenurecord).flinstalled;
	
	return (true);
	} /*menuisinstalledverb*/


static boolean getmainmenuvalue (hdltreenode hparam1, hdlmenu *hmenu, short *idmenu) {
	
	tyvaluerecord val;
	bigstring bsmenu;
	
	flnextparamislast = true;
	
	if (!getparamvalue (hparam1, 1, &val))
		return (false);
	
	if (val.valuetype != stringvaluetype)
		return (false);
	
	texthandletostring ((Handle) val.data.stringvalue, bsmenu);
	
	return (shelltgetmainmenu (bsmenu, hmenu, idmenu));
	} /*getmainmenuvalue*/


static boolean menuinstallverb (hdltreenode hparam1, tyvaluerecord *v) {
	
	hdlmenurecord hmenurecord;
	hdlmenu hmainmenu;
	short idmainmenu;
	
	if (getmainmenuvalue (hparam1, &hmainmenu, &idmainmenu)) {
		
		insertmenu (hmainmenu, idmainmenu + 1);
		
		drawmenubar ();
		
		(*v).data.flvalue = true;
		
		return (true);
		}
	
	flnextparamislast = true;
	
	if (!getmenuparaminmemory (hparam1, &hmenurecord))
		return (false);
	
	(*v).data.flvalue = meinstallmenubar (hmenurecord);
	
	return (true);
	} /*menuinstallverb*/


static boolean menuremoveverb (hdltreenode hparam1, tyvaluerecord *v) {
	
	hdlmenurecord hmenurecord;
	hdlmenu hmainmenu;
	short idmainmenu;
	
	if (getmainmenuvalue (hparam1, &hmainmenu, &idmainmenu)) {
		
		removemenu (idmainmenu);
		
		drawmenubar ();
		
		(*v).data.flvalue = true;
		
		return (true);
		}
	
	flnextparamislast = true;
	
	if (!getmenuparaminmemory (hparam1, &hmenurecord))
		return (false);
	
	(*v).data.flvalue = meremovemenubar (hmenurecord);
	
	return (true);
	} /*menuremoveverb*/


static boolean menugetscriptverb (hdltreenode hparam1, tyvaluerecord *v) {
	
	/*
	menu/op globals are set.  copy the script attached to the menubar cursor 
	into the var param.  set v to true if sucessful, false if no script
	*/
	
	hdlhashtable htable;
	bigstring bsname;
	hdloutlinerecord houtline;
	register hdloutlinerecord ho;
	boolean fljustloaded;
	tyvaluerecord scriptval;
	boolean fl;
	
	flnextparamislast = true;
	
	if (!getvarparam (hparam1, 1, &htable, bsname))
		return (false);
	
	if (!meloadscriptoutline (menudata, (**outlinedata).hbarcursor, &houtline, &fljustloaded)) /*fatal error*/
		return (false);
	
	ho = houtline; /*copy into register*/
	
	if (ho == nil) /*no script attached*/
		return (true);
	
	fl = langexternalnewvalue (idscriptprocessor, (Handle) ho, &scriptval);
	
	if (fljustloaded)
		opdisposeoutline (ho, false);
	
	if (!fl)
		return (false);
	
	if (!langsetsymboltableval (htable, bsname, scriptval)) {
		
		disposevaluerecord (scriptval, true);
		
		return (false);
		}
	
	(*v).data.flvalue = true;
	
	return (true);
	} /*menugetscriptverb*/


static boolean menugetcommandkeyverb (hdltreenode hparam1, tyvaluerecord *v) {
	
	/*
	menu/op globals are set.  get the command key of the current headline
	*/
	
	tymenuiteminfo item;
	bigstring bscommandkey;
	
	if (!langcheckparamcount (hparam1, 0)) /*shouldn't have any parameters*/
		return (false);
	
	setemptystring (bscommandkey);
	
	if (megetmenuiteminfo ((**outlinedata).hbarcursor, &item))
		if (item.cmdkey != chnul)
			getcommandkeystring (item.cmdkey, keycommand, bscommandkey);

	return (setstringvalue (bscommandkey, v));
	} /*menugetscriptverb*/


static boolean menusetcommandkeyverb (hdltreenode hparam1, tyvaluerecord *v) {
	
	/*
	menu/op globals are set.  set the command key of the current headline
	*/
	
	bigstring bscommandkey;
	
	flnextparamislast = true;
	
	if (!getstringvalue (hparam1, 1, bscommandkey))
		return (false);
	
	mesetcmdkey (lastchar (bscommandkey), keycommand);
	
	return (setbooleanvalue (true, v));
	} /* menusetcommandkeyverb */


static boolean detachscript (hdlmenurecord hmenu, hdlheadrecord hnode) {
	
	/*
	menu/op globals are set.  remove the script from the specified node in hmenu
	*/
	
	register hdlmenurecord hm = hmenu;
	
	if ((**hm).scriptnode != hnode) /*simple case, not attached*/
		return (true);
	
	if ((**hm).scriptwindow == nil) /*defensive driving*/
		return (false);
	
	shellpushglobals ((**hm).scriptwindow);
	
	killundo (); /*must toss undos before they're stranded*/
	
	shellpopglobals ();
	
	// meunloadscript (); /*detach the script from the window*/
	
	(**hm).scriptnode = nil;
	
	(**hm).scriptoutline = nil;
	
	return (true);
	} /*detachscript*/


static boolean attachscript (hdlmenurecord hmenu, hdlheadrecord hnode, hdloutlinerecord hscript) {
	
	/*
	menu/op globals are set.  attach the script to the specified node in hmenu
	*/
	
	register hdlmenurecord hm = hmenu;
	tymenuiteminfo item;
	
	megetmenuiteminfo (hnode, &item); /*gets prior cmd key, or clears bytes*/
	
	mereleaserefconroutine (hnode, true); /*toss existing script, if any*/
	
	if (hscript == nil)
		opemptyrefcon (hnode);
	
	else {
		
		(**hscript).fldirty = true; /*force save on this guy's script*/
		
		item.linkedscript.houtline = hscript;
		
		item.linkedscript.adrlink = nildbaddress; /*hasn't been allocated yet*/
		
		mesetmenuiteminfo (hnode, &item); /*attach!*/
		}
	
	if ((**hm).scriptwindow != nil)
		mesmashscriptwindow ();
	
	return (true);
	} /*attachscript*/


static boolean menusetscriptverb (hdltreenode hparam1, tyvaluerecord *v) {
	
	/*
	menu/op globals are set.  attach the script to the menubar cursor 
	
	12/22/91 dmb: get previous values in refcon before setting up new script
	*/
	
	register hdlmenurecord hm = menudata;
	hdlhashtable htable;
	bigstring bsname;
	tyvaluerecord scriptval;
	hdloutlinerecord hcopy;
	hdlheadrecord hcursor;
	hdlhashnode hnode;
	
	flnextparamislast = true;
	
	if (!getvarvalue (hparam1, 1, &htable, bsname, &scriptval, &hnode))
		return (false);
	
	if (!opvaltoscript (scriptval, &hcopy)) /*not a fatal error*/
		return (true);
	
	if (!opcopyoutlinerecord (hcopy, &hcopy))
		return (false);
	
	hcursor = (**outlinedata).hbarcursor;
	
	detachscript (hm, hcursor);
	
	attachscript (hm, hcursor, hcopy);
	
	/*
	megetmenuiteminfo (hcursor, &item); /%gets prior cmd key, or clears bytes%/
	
	mereleaserefconroutine (hcursor, true);
	
	(**hcopy).fldirty = true; /%force save on this guy's script%/
	
	item.linkedscript.houtline = hcopy;
	
	item.linkedscript.adrlink = nildbaddress; /%hasn't been allocated yet%/
	
	mesetmenuiteminfo (hcursor, &item);
	
	if ((**hm).scriptwindow != nil)
		mesmashscriptwindow ();
	*/
	
	(*v).data.flvalue = true;
	
	return (true);
	} /*menusetscriptverb*/


#pragma pack(2)
typedef struct tyfindinfo {

	ptrstring pfind;

	hdlheadrecord hfound;
	} tyfindinfo, *ptrfindinfo;
#pragma options align=reset


static boolean findheadlinevisit (hdlheadrecord hnode, ptrvoid refcon) {
	
	ptrfindinfo findinfo = (ptrfindinfo) refcon;
	bigstring bs;
	
	opgetheadstring (hnode, bs);
	
	if (equalstrings ((*findinfo).pfind, bs)) {
		
		(*findinfo).hfound = hnode;
		
		return (false);
		}
	
	return (true); /*keep visiting*/
	} /*findheadlinevisit*/


static void oprefreshdisplay () {
	
	if (opdisplayenabled ()) {
		
		opdirtymeasurements (); //6.0a14 dmb
		
		opsetctexpanded (outlinedata); //6.0a14 dmb
		
		opgetscrollbarinfo (true); /*bring scroll position in range*/
		
		opresetscrollbars ();
		
		opinvaldisplay ();
		}
	} /*oprefreshdisplay*/


static hdlmenurecord hmenudatasave = nil;

static boolean mepushmenudata (hdlmenurecord hmenurecord) {
	
	assert (hmenudatasave == nil);
	
	hmenudatasave = menudata;
	
	menudata = hmenurecord;
	
	return (oppushoutline ((**hmenurecord).menuoutline));
	} /*mepushmenudata*/


static boolean mepopmenudata (void) {
	
	menudata = hmenudatasave;
	
	hmenudatasave = nil;
	
	return (oppopoutline ());
	} /*mepopmenudata*/


static boolean getsubmenuvalue (hdltreenode hfirst, short pnum, hdlheadrecord *hsubmenu) {
	
	register hdloutlinerecord ho;
	Handle hpacked;
	hdlmenuvariable hv;
	boolean fl;
	
	if (!getmenuparam (hfirst, pnum, &hv))
		return (false);
	
	if (!langexternalmemorypack ((hdlexternalvariable) hv, &hpacked, HNoNode))
		return (false);
	
	fl = langexternalmemoryunpack (hpacked, (hdlexternalvariable *) &hv);
	
	disposehandle (hpacked);
	
	if (!fl)
		return (false);
	
	ho = (**(hdlmenurecord) (**hv).variabledata).menuoutline;
	
	*hsubmenu = (**ho).hsummit;
	
	(**ho).hsummit = nil;
	
	menuverbdispose ((hdlexternalvariable) hv, false);
	
	return (true);
	} /*getsubmenuvalue*/


static void deletemenunode (hdlheadrecord hnode) {
	
	/*
	9/11/92 dmb: a bit of shared code between addmenucommand & deletemenucommand
	
	4.0b7 dmb: new implementation, handles marks, doesn't crash for unopened outlines
	*/
	
	hdlheadrecord hbarcursor;
	boolean fldisplayenabled = opdisplayenabled ();
	
	if (fldisplayenabled)
		opdisabledisplay ();
	
	hbarcursor = (**outlinedata).hbarcursor;
	
	opclearallmarks ();
	
	opmoveto (hnode);
	
	opdelete ();
	
	if (opnodeinoutline (hbarcursor))
		opmoveto (hbarcursor);
	
	if (fldisplayenabled) {
	
		openabledisplay ();
		
		oprefreshdisplay ();
		}
	/*
	
	if ((**hnode).flexpanded && opdisplayenabled ()) {
		
		opdisabledisplay ();
		
		hbarcursor = (**outlinedata).hbarcursor;
		
		opmoveto (hnode);
		
		opdelete ();
		
		if (opnodeinoutline (hbarcursor))
			opmoveto (hbarcursor);
		
		openabledisplay ();
		
		oprefreshdisplay ();
		}
	else {
		
		opunlink (hnode);
		
		opdisposestructure (hnode, true);
		
	*/
	} /*deletemenunode*/


static boolean addmenucommandverb (hdltreenode hparam1, boolean flsubmenu, tyvaluerecord *v) {
	
	/*
	menu/op globals are _not_ set.  add a new menu item or submenu to a menubar, 
	or replace the script of any existing item, depending on the parameters 
	specified.
	
	9/11/92 dmb: when adding a submenu, delete existing item with same name
	
	9/28/92 dmb: move to the newly-deposited node
	
	2.1b4 dmb: set outlinesignature of newly-created scripts to 'LAND'
	*/
	
	register hdltreenode hp1 = hparam1;
	register hdloutlinerecord ho;
	register hdlmenurecord hm;
	hdlheadrecord hnode;
	hdlheadrecord hdelete = nil;
	hdlmenurecord hmenurecord;
	bigstring bsmenu;
	bigstring bsitem;
	bigstring bsscript;
	tydirection dir;
	hdlwindowinfo hinfo;
	boolean fl;
	hdlheadrecord hsubmenu;
	boolean flmainmenu = false;
	tyfindinfo findinfo;
	
	fl = false; /*default return*/
	
	if (!getmenuparaminmemory (hp1, &hmenurecord)) /*the menu to be changed*/
		return (false);
	
	if (!getstringvalue (hp1, 2, bsmenu)) /*the title of the menu to add to*/
		return (false);
	
	if (flsubmenu) {
		
		flnextparamislast = true;
		
		if (!getsubmenuvalue (hp1, 3, &hsubmenu)) /*the submenu to be inserted*/
			return (false);
		
		flmainmenu = isemptystring (bsmenu); /*caller wants this to be a main menu*/
		
		if (flmainmenu)
			opgetheadstring (hsubmenu, bsmenu);
		else
			opgetheadstring (hsubmenu, bsitem);
		}
	else {
		
		if (!getstringvalue (hp1, 3, bsitem)) /*the item in the menu to add, or replace*/
			return (false);
		
		flnextparamislast = true;
		
		if (!getstringvalue (hp1, 4, bsscript)) /*the text of the script to be set*/
			return (false);
		}
	
	/*
	set up globals
	*/
	
	hm = hmenurecord;
	
	ho = (**hm).menuoutline;
	
	if (shellfinddatawindow ((Handle) hm, &hinfo)) {
		
		shellpushglobals ((**hinfo).macwindow);
		
		pushundoaction (0); /*no undo*/
		}
	else
		mepushmenudata (hm);
	
	/*
	move headline to menu headline, creating it if necessary
	*/
	
	findinfo.pfind = bsmenu;
	
	hnode = (**ho).hsummit;
	
	dir = down;
	
	if (!opsiblingvisiter (hnode, false, &findheadlinevisit, &findinfo)) { /*found the menu*/
		
		hnode = findinfo.hfound;
		
		if (flmainmenu) { /*delete it so it can be replaced*/
			
			hdelete = hnode;
			
			goto gotposition;
			}
		}
	else {
		
		hnode = oprepeatedbump (down, longinfinity, hnode, false);
		
		if (flmainmenu)
			goto gotposition;
		
		if (!opaddheadline (hnode, down, bsmenu, &hnode))
			goto exit;
		}
	
	/*
	move headline to item headline, creating it if necessary
	*/
	
	findinfo.pfind = bsitem;
	
	if (!oprecursivelyvisit (hnode, 1, &findheadlinevisit, &findinfo)) { /*found the item*/
		
		hnode = findinfo.hfound;
		
		if (flsubmenu) /*delete it so it can be replaced*/
			hdelete = hnode;
		}
	else {
		
		if (ophassubheads (hnode)) {
			
			hnode = oprepeatedbump (down, longinfinity, (**hnode).headlinkright, false);
			
			dir = down;
			}
		else
			dir = right;
		
		if (flsubmenu)
			goto gotposition;
		
		if (!opaddheadline (hnode, dir, bsitem, &hnode))
			goto exit;
		}
	
	gotposition:
	
	if (flsubmenu) {
		/*
		insert the submenu
		*/
		
		opdeposit (hnode, dir, hsubmenu);
		
		opresetlevels (hsubmenu);
		
		opfastcollapse (hsubmenu);
		
		if (hdelete != nil) /*delete old item*/
			deletemenunode (hdelete);
		
		opexpandto (hsubmenu);
		}
	else { 
		/*
		set up the script
		*/
	
		if (isemptystring (bsscript))
			ho = nil;
		
		else {
			hdloutlinerecord houtline;
			hdlheadrecord hsummit;
			Handle hscript;
			
			if (!newtexthandle (bsscript, &hscript))
				goto exit;
			
			if (!newoutlinerecord (&houtline)) {
				
				disposehandle (hscript);
				
				goto exit;
				}
			
			ho = houtline; /*copy script outline into register*/
			
			if (!opnewstructure (hscript, &hsummit)) {
				
				opdisposeoutline (ho, false);
				
				goto exit;
				}
			
			(**ho).outlinesignature = 'LAND';
			
			(**ho).fontnum = (**hm).defaultscriptfontnum;
			
			(**ho).fontsize = (**hm).defaultscriptfontsize;
			
			opsetsummit (ho, hsummit);
			
			oppushoutline (ho);
			
			opsetdisplaydefaults (ho);
			
			oppopoutline ();
			}
		
		/*
		connect it
		*/
		
		detachscript (hm, hnode);
		
		attachscript (hm, hnode, ho);
		
		opexpandto (hnode);
		}
	
	oprefreshdisplay ();
	
	// now done by oprefreshdisplay - opsetctexpanded (outlinedata); /*re-compute in case we deposited to expanded node*/
	
	opdirtyoutline (); /*dirty the menubar outline*/
	
	langexternalsetdirty ((hdlexternalvariable) rootvariable, true); /*make sure changes are flagged*/
	
	/*
	if (htable == menubartable)
		langsymbolchanged (htable, bs, true);
	*/
	
	fl = true;
	
	exit:
	
	if (hinfo != nil)
		shellpopglobals ();
	else
		mepopmenudata ();
	
	(*v).data.flvalue = fl;
	
	return (fl);
	} /*addmenucommandverb*/


static boolean deletemenucommandverb (hdltreenode hparam1, boolean flsubmenu, tyvaluerecord *v) {
	
	/*
	menu/op globals are _not_ set.  delete a menu item or submenu from a 
	menubar, depending on the parameters specified.
	
	5.0a4 dmb: call ophassubheads with menu's outline globals pushed
	*/
	
	register hdltreenode hp1 = hparam1;
	register hdloutlinerecord ho;
	register hdlmenurecord hm;
	hdlheadrecord hnode;
	hdlmenurecord hmenurecord;
	bigstring bsmenu;
	bigstring bsitem;
	hdlwindowinfo hinfo;
	tyfindinfo findinfo;
	boolean flchanged;
	
	if (!getmenuparaminmemory (hparam1, &hmenurecord))
		return (false);
	
	if (flsubmenu)
		flnextparamislast = true;
	
	if (!getstringvalue (hp1, 2, bsmenu))
		return (false);
	
	if (!flsubmenu) {
		
		flnextparamislast = true;
		
		if (!getstringvalue (hp1, 3, bsitem))
			return (false);
		}
	
	/*
	move headline to menu headline
	*/
	
	hm = hmenurecord;
	
	ho = (**hm).menuoutline;
	
	findinfo.pfind = bsmenu;
	
	hnode = (**ho).hsummit;
	
	if (opsiblingvisiter (hnode, false, &findheadlinevisit, &findinfo)) /*didn't find the menu*/
		return (true);
	
	hnode = findinfo.hfound;
	
	/*
	move headline to item headline
	*/
	
	if (!flsubmenu) {
		
		if (!isemptystring (bsitem)) {
			
			findinfo.pfind = bsitem;
			
			if (oprecursivelyvisit (hnode, 1, &findheadlinevisit, &findinfo)) /*didn't find the item*/
				return (true);
			
			hnode = findinfo.hfound;
			}
		}
	
	/*
	delete the menu item
	*/
	
	if (shellfinddatawindow ((Handle) hm, &hinfo)) {
		
		shellpushglobals ((**hinfo).macwindow);
		
		pushundoaction (0); /*no undo*/
		}
	else
		mepushmenudata (hm);
	
	if (ophassubheads (hnode) && !flsubmenu) /*it's not a single command*/
		flchanged = false;
	
	else {
		
		(**ho).fldirty = true; /*dirty the menubar outline*/
		
		deletemenunode (hnode);
		
		flchanged = true;
		}
	
	if (hinfo != nil)
		shellpopglobals ();
	else
		mepopmenudata ();
	
	if (flchanged)
		langexternalsetdirty ((hdlexternalvariable) rootvariable, true); /*make sure changes are flagged*/
	
	/*
	if (htable == menubartable)
		langsymbolchanged (htable, bs, true);
	*/
	
	(*v).data.flvalue = flchanged;
	
	return (true);
	} /*deletemenucommandverb*/


static boolean menufunctionvalue (short token, hdltreenode hparam1, tyvaluerecord *vreturned, bigstring bserror) {
	
	/*
	6/1/93 dmb: when vreturned is nil, return whether or not verb token must 
	be run in the Frontier process
	*/
	
	register tyvaluerecord *v = vreturned;
	register boolean fl;
	WindowPtr targetwindow;
	
	if (v == nil) { /*need Frontier process?*/
		
		switch (token) {
			
			case isinstalledfunc:
			case getscriptfunc:
			case setscriptfunc:
				return (false);
			
			default:
				return (true);
			}
		}
	
	errornum = 0;
	
	setbooleanvalue (false, v); /*by default, menu functions return false*/
	
	switch (token) {/*these verbs don't need any special globals pushed*/
		
		case buildmenubarfunc:
			if (!langcheckparamcount (hparam1, 0)) /*shouldn't have any parameters*/
				return (false);
			
			(*v).data.flvalue = menubuildverb ();
			
			return (true);
		
		case clearmenubarfunc:
			if (!langcheckparamcount (hparam1, 0))
				return (false);
			
			(*v).data.flvalue = menuclearverb ();
			
			return (true);
		
		case isinstalledfunc:
			if (!menuisinstalledverb (hparam1, v))
				goto error;
			
			return (true);
		
		case installfunc:
			if (!menuinstallverb (hparam1, v))
				goto error;
			
			return (true);
		
		case removefunc:
			if (!menuremoveverb (hparam1, v))
				goto error;
			
			return (true);
		
		case addmenucommandfunc:
			if (!addmenucommandverb (hparam1, false, v))
				goto error;
			
			return (true);
		
		case deletemenucommandfunc:
			if (!deletemenucommandverb (hparam1, false, v))
				goto error;
			
			return (true);
		
		case addsubmenufunc:
			if (!addmenucommandverb (hparam1, true, v))
				goto error;
			
			return (true);
		
		case deletesubmenufunc:
			if (!deletemenucommandverb (hparam1, true, v))
				goto error;
			
			return (true);
		} /*switch*/
	
	/*be sure there's a menu window in front -- set menueditor.c globals*/
	
	if (!langfindtargetwindow (idmenuprocessor, &targetwindow)) { /*all other verbs require an outline window in front*/
		
		errornum = nomenuerror;
		
		goto error;
		}
	
	shellpushglobals (targetwindow); /*following verbs assume that a menubar is pushed*/
	
	(*shellglobals.gettargetdataroutine) (idmenuprocessor); /*set op globals*/
	
	mecheckglobals (); /*copy handles from menudata to globals*/
	
	fl = false; /*default return value*/
	
	switch (token) { /*these verbs assume that the menueditor globals are set*/
		
		case zoomscriptfunc:
			if (!langcheckparamcount (hparam1, 0))
				return (false);
			
			(*v).data.flvalue = mezoomscriptwindow ();
			
			fl = true;
			
			break;
			
		/*
		case findscriptfunc: {
			bigstring bs;
			boolean flinscript;
			
			flnextparamislast = true;
			
			if (!getstringvalue (hparam1, 1, searchparams.bsfind))
				break;
			
			(*v).data.flvalue = mesearchoutline (false, searchparams.flwraparound, &flinscript);
			
			mepostcursormove ();
			
			fl = true;
			
			break;
			}
		
		case findfunc: {
			bigstring bs;
			
			flnextparamislast = true;
			
			if (!getstringvalue (hparam1, 1, searchparams.bsfind))
				break;
			
			(*v).data.flvalue = opflatfind (false, true);
			
			mepostcursormove ();
			
			fl = true;
			
			break;
			}
		*/
		
		case getscriptfunc:
			fl = menugetscriptverb (hparam1, v);
			
			break;
		
		case setscriptfunc:
			fl = menusetscriptverb (hparam1, v);
			
			break;
		
		case getcommandkeyfunc:
			fl = menugetcommandkeyverb (hparam1, v);
			
			break;
			
		case setcommandkeyfunc:
			fl = menusetcommandkeyverb (hparam1, v);
			
			break;
		} /*switch*/
	
	shellpopglobals ();
	
	return (fl);
	
	error:
	
	if (errornum != 0) /*get error string*/
		getstringlist (menuerrorlist, errornum, bserror);
	
	return (false);
	} /*menufunctionvalue*/


static boolean menuinitverbs (void) {
	
	return (loadfunctionprocessor (idmenuverbs, &menufunctionvalue));
	} /*menuinitverbs*/


static boolean menuverbkeystroke (void) {
	
	/*
	5.0b18 dmb: let langexternalsurfacekey decide what keys it needs
	*/

	// if ((keyboardstatus.chkb == chenter) && (keyboardstatus.flcmdkey) && (keyboardstatus.flshiftkey)) {
		
	if (langexternalsurfacekey ((hdlexternalvariable) (**menudata).menurefcon))
		return (true);
	
	return (mekeystroke ());
	} /*menuverbkeystroke*/


static boolean menuverbtitleclick (Point pt) {
	
	return (langexternaltitleclick (pt, (hdlexternalvariable) (**menudata).menurefcon));
	} /*menuverbtitleclick*/


static boolean menuverbsetfont (void) {
	
	register short fontnum = (**menuwindowinfo).selectioninfo.fontnum;
	
	(**menudata).defaultscriptfontnum = fontnum; /*side-effect*/
	
	(**menudata).defaultscriptfontsize = (**menuwindowinfo).selectioninfo.fontsize;
	
	mecheckglobals ();
	
	return (opsetfont (fontnum));
	} /*menuverbsetfont*/
	
	
static boolean menuverbsetsize (void) {
	
	register short fontsize = (**menuwindowinfo).selectioninfo.fontsize;
	
	(**menudata).defaultscriptfontsize = fontsize; /*side-effect*/
	
	(**menudata).defaultscriptfontnum = (**menuwindowinfo).selectioninfo.fontnum;
	
	mecheckglobals ();
	
	return (opsetsize (fontsize));
	} /*menuverbsetsize*/
	

static boolean menuverbsetselectioninfo (void) {
	
	mecheckglobals ();
	
	return (opsetselectioninfo ());
	} /*menuverbsetselectioninfo*/


static boolean menuverbrunscript (void) {
	
	mecheckglobals ();
	
	return (meuserselected ((**outlinedata).hbarcursor));
	} /*menuverbrunscript*/


static boolean menuverbgetvariable (hdlexternalvariable *hvariable) {
	
	if (menudata == nil)
		return (false);
	
	*hvariable = (hdlexternalvariable) (**menudata).menurefcon;
	
	return (true);
	} /*menuverbgetvariable*/


static boolean menuverbgettargetdata (short id) {
	
	/*
	a verb is about to be executed that acts on the indicated external type, or 
	on any shell window if id == -1.
	
	return true if we can handle verbs of that type and are able to set the 
	relevant globals
	*/
	
	mesetglobals ();
	
	switch (id) {
		
		case -1:
			return (true);
		
		case idoutlineprocessor:
			return (true);
		
		case idwordprocessor:
			return (opeditsetglobals ());
			
		case idmenuprocessor: /*DW 6/19/91*/
			return (true);
		
		default:
			return (false);
		}
	} /*menuverbgettargetdata*/


static boolean menuchildclose (WindowPtr w) {
	
	/*
	1/2/91 dmb: no longer call mescriptwindowclosed from here; scriptclose 
	will do it
	
	9/24/91 dmb: pass through return value from child

	5.0b11 dmb: back to the original logic; call mescriptwindowclosed
	from here, after we've popped the script's globals
	*/
	
	boolean fl;
	
	assert (w == (**menudata).scriptwindow);
	
	shellpushglobals (w);
	
	fl = (*shellglobals.closeroutine) (); /*scripts.c get a shot at closing*/
	
	shellpopglobals ();
	
	if (fl)
		mescriptwindowclosed ();
	
	return (fl);
	} /*menuchildclose*/
	
	
static boolean menucheckwindowrect (void) {
	
	/*
	return true if the menuwindowrect field of menudata changed.
	*/
	
	register hdlmenurecord hm = menudata;
	Rect r;
	
	shellgetglobalwindowrect (menuwindowinfo, &r);
	
	if (equalrects (r, (**hm).menuwindowrect))
		return (false);
		
	(**hm).menuwindowrect = r;
	
	(**hm).fldirty = true;
	
	return (true);
	} /*menucheckwindowrect*/


static boolean menuverbclose (void) {
	
	/*
	5.0d19 dmb: set hm's flwindowopen to false too, not just op's

	5.0b15 dmb: if we're not installed, remove us in case we were active
	*/
	
	register hdlmenurecord hm = menudata;
	
	if (hm == nil) //already closed & disposed?
		return (true);
	
	killundo (); /*must toss undos before they're stranded*/
	
	shellclosewindow ((**hm).scriptwindow); 
	
	menucheckwindowrect ();
	
	(**hm).flactive = false;
	
	if (!(**hm).flinstalled) { /*not an installed menu; remove from menubar & dispose*/
		
		meremovemenubar (hm); //5.0b15
		
		medisposemenubar ((**hm).hmenustack);
		
		(**hm).hmenustack = nil;
		}
	
	mecheckglobals ();
	
	(**outlinedata).flwindowopen = false;
	
	(**hm).flwindowopen = false; /*5.0d19*/
	
	opcloseoutline ();
	
	return (true);
	} /*menuverbclose*/


static boolean mebeginprint (void) {
	
	opbeginprint ();
	
	return (true);
	} /*mebeginprint*/


static boolean meendprint (void) {
	
	opsetdisplaydefaults (outlinedata); //need to reset lineheights before next call
	
	meresize ();
	
	return (true);
	} /*meendprint*/


boolean menustart (void) {
	
	/*
	set up callback routines record, and link our data into the shell's 
	data structure.
	*/
	
	ptrcallbacks menucallbacks;
	register ptrcallbacks cb;
	
	assert (sizeof (tyexternalvariable) == sizeof (tymenuvariable));
	
	menuinitverbs ();
	
	meinit ();
	
	shellpushscraphook (&mescraphook);
	
	shellnewcallbacks (&menucallbacks);
	
	cb = menucallbacks; /*copy into register*/
	
	loadconfigresource (idmenueditorconfig, &(*cb).config);
		
	(*cb).configresnum = idmenueditorconfig;
		
	(*cb).windowholder = &menuwindow;
	
	(*cb).dataholder = (Handle *) &menudata;
	
	(*cb).infoholder = &menuwindowinfo;
	
	(*cb).setglobalsroutine = &mesetglobals;
	
	(*cb).pushroutine = &oppushglobals;

	(*cb).poproutine = &oppopglobals;
	
#ifdef version42orgreater
	
	(*cb).disposerecordroutine = ccdisposefilerecord;
	
	(*cb).saveroutine = ccsavespecialfile;

#endif
	
	(*cb).updateroutine = &meupdate;
	
	(*cb).activateroutine = &meactivate;
	
	(*cb).getcontentsizeroutine = &megetcontentsize;
	
	(*cb).resetrectsroutine = &meresetwindowrects;
	
	(*cb).resizeroutine = &meresize;
	
	(*cb).scrollroutine = &mescroll;
	
	(*cb).setscrollbarroutine = &megetscrollbarinfo;
	
	(*cb).mouseroutine = &memousedown;

	(*cb).rmouseroutine = &oprmousedown; /*7.0b13 PBS: right-click in menus*/
	
	(*cb).keystrokeroutine = &menuverbkeystroke;
	
	(*cb).titleclickroutine = &menuverbtitleclick;
	
	(*cb).cmdkeyfilterroutine = &mecmdkeyfilter;
	
	(*cb).cutroutine = &mecut;
	
	(*cb).copyroutine = &mecopy;
	
	(*cb).pasteroutine = &mepaste;
	
	(*cb).clearroutine = &meclear;
	
	(*cb).selectallroutine = &meselectall;
	
	(*cb).closeroutine = &menuverbclose;
	
	(*cb).getundoglobalsroutine = &megetundoglobals;
	
	(*cb).setundoglobalsroutine = &mesetundoglobals;
	
	(*cb).childcloseroutine = &menuchildclose;
	
	(*cb).idleroutine = &meidle;
	
	(*cb).adjustcursorroutine = &meadjustcursor;
	
	(*cb).gettargetdataroutine = &menuverbgettargetdata;
	
	(*cb).getvariableroutine = (shellgetvariablecallback) &menuverbgetvariable;
	
	(*cb).settextmoderoutine = &opsettextmode;
	
	(*cb).fontroutine = &menuverbsetfont;
	
	(*cb).sizeroutine = &menuverbsetsize;
	
	(*cb).setselectioninforoutine = &menuverbsetselectioninfo;
	
	(*cb).searchroutine = &menuverbsearch;
	
	(*cb).executeroutine = &menuverbrunscript;
	
	(*cb).setprintinfoproutine = &mesetprintinfo;
	
	(*cb).beginprintroutine = &mebeginprint;
	
	(*cb).endprintroutine = &meendprint;
	
	(*cb).printroutine = &meprint;
	
	return (true);
	} /*menustart*/




