
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

#include "memory.h"
#include "ops.h"
#include "quickdraw.h"
#include "resources.h"
#include "scrap.h"
#include "search.h"
#include "strings.h"
#include "shell.h"
#include "shellprint.h"
#include "shellundo.h"
#include "shell.rsrc.h"
#include "lang.h"
#include "langexternal.h"
#include "tablestructure.h"
#include "cancoon.h"
#include "pict.h"
#include "pictverbs.h"
#include "kernelverbdefs.h"




#define picterrorlist 267
#define nopicterror 1
#define namenotpicterror 2



typedef enum typicttoken { /*verbs that are processed by pict.c*/
	
	scheduleupdatefunc,
	
	evalfunc,
	
	getpicturefunc,
	
	setpicturefunc,
	
	/*
	getboundsfunc,
	*/
	
	ctpictverbs
	} typicttoken;

#if langexternalfind_optimization

	typedef tyexternalvariable typictvariable, *ptrpictvariable, **hdlpictvariable;

#else
	
	typedef struct typictvariable {
		
		unsigned short id; /*tyexternalid: managed by langexternal.c*/
		
		unsigned short flinmemory: 1; /*if true, variabledata is an hdlwprecord, else a dbaddress*/
		
		unsigned short flmayaffectdisplay: 1; /*not in memory, but being displayed in a table window*/
		
		long variabledata; /*either a hdlpictrecord or a dbaddress*/
		
		hdldatabaserecord hdatabase; // 5.0a18 dmb

		dbaddress oldaddress; /*last place this pict was stored in db*/
		} typictvariable, *ptrpictvariable, **hdlpictvariable;

#endif

static short errornum = 0;



static boolean newpictvariable (boolean flinmemory, long variabledata, hdlpictvariable *h) {

	return (langnewexternalvariable (flinmemory, variabledata, (hdlexternalvariable *) h));
	} /*newpictvariable*/
	

boolean pictverbgettypestring (hdlexternalvariable hvariable, bigstring bs) {
	
	getstringlist (pictstringlist, picttypestring, bs);
	
	return (true);
	} /*pictverbgettypestring*/


static boolean pictdisposevariable (hdlexternalvariable hvariable, boolean fldisk) {
	
	return (pictdisposerecord ((hdlpictrecord) (**hvariable).variabledata));
	} /*pictdisposevariable*/


boolean pictverbdispose (hdlexternalvariable hvariable, boolean fldisk) {
	
	return (langexternaldisposevariable (hvariable, fldisk, &pictdisposevariable));
	} /*pictverbdispose*/


boolean pictverbnew (Handle hdata, hdlexternalvariable *hvariable) {
	
	/*
	9/6/91: now takes hdata parameter, but it isn't used yet.  it is 
	intended to potentially contain a picture from which to create the 
	new variable
	*/
	
	register hdlpictrecord hp;
	register hdlpictvariable hv;
	
	if (!pictnewrecord ())
		return (false);
	
	hp = pictdata; /*copy into register*/
	
	if (!newpictvariable (true, (long) hp, (hdlpictvariable *) hvariable)) {
		
		pictdisposerecord (hp);
		
		return (false);
		}
	
	hv = (hdlpictvariable) *hvariable; /*copy into register*/
	
	if (hdata != nil) { /*we've been given a picture to copy as initial data*/
		
		if (!copyhandle (hdata, &hdata)) {
			
			pictdisposerecord (hp);
			
			disposehandle ((Handle) hv);
			
			return (false);
			}
		
		(**hp).macpicture = (PicHandle) hdata;
		}
	
	(**hv).variabledata = (long) hp; /*link the pict rec into the variable rec*/
	
	(**hp).pictrefcon = (long) hv; /*the pointing is mutual*/
	
	return (true);
	} /*pictverbnew*/


#if !flruntime

static void pictverbcheckwindowrect (hdlpictrecord hpict) {
	
	register hdlpictrecord hp = hpict;
	hdlwindowinfo hinfo;
		
	if ((**hp).flwindowopen) { /*make windowrect reflect current window size & position*/
		
		if (shellfinddatawindow ((Handle) hp, &hinfo)) {
			
			Rect r;
			
			shellgetglobalwindowrect (hinfo, &r);
			
			if (!equalrects (r, (**hp).windowrect)) {
			
				(**hp).windowrect = r;
				
				(**hp).fldirty = true;
				}
			}
		}
	} /*pictverbcheckwindowrect*/
	
#else

	#define pictverbcheckwindowrect(hpict)	((void *) 0)
	
	#define fldatabasesaveas 0

#endif


static boolean pictverbinmemory (hdlpictvariable hv) {

	/*
	5.0a18 dmb: support database linking
	*/
	
	register boolean fl;
	dbaddress adr;
	Handle hpackedpict;
	long ix = 0;
	hdlpictrecord hpict;
	
	if ((**hv).flinmemory) /*nothing to do, it's already in memory*/
		return (true);
	
	dbpushdatabase ((**hv).hdatabase);

	adr = (dbaddress) (**hv).variabledata;
	
	fl = dbrefhandle (adr, &hpackedpict);
	
	if (fl) {
		
		fl = pictunpack (hpackedpict, &ix, &hpict);
		
		disposehandle (hpackedpict);
		}

	dbpopdatabase ();
	
	if (!fl) 
		return (false);
	
	(**hv).flinmemory = true;
	
	(**hv).variabledata = (long) hpict; /*link into variable structure*/
	
	(**hv).oldaddress = adr; /*last place this pict was stored*/
	
	(**hpict).pictrefcon = (long) hv; /*we can get from pict rec to variable rec*/
	
	return (true);
	} /*pictverbinmemory*/


boolean pictverbmemorypack (hdlexternalvariable h, Handle *hpacked) {
	
	/*
	a special entrypoint for the language's pack and upack verbs.
	*/
	
	register hdlpictvariable hv = (hdlpictvariable) h;
	register hdlpictrecord hp;
	register boolean fl;
	Handle hpush = nil;
	
	if (!(**hv).flinmemory) {
	
		if (!langexternalrefdata ((hdlexternalvariable) hv, &hpush))
			return (false);
		
		goto pushandexit;
		}
	
	hp = (hdlpictrecord) (**hv).variabledata;
	
	pictverbcheckwindowrect (hp);
	
	if (!pictpack (hp, &hpush))
		return (false);
	
	pushandexit:
	
	if (hpush == nil)
		return (false);
	
	fl = pushhandle (hpush, *hpacked);
	
	disposehandle (hpush);
	
	return (fl);
	} /*pictverbmemorypack*/


boolean pictverbmemoryunpack (Handle hpacked, long *ixload, hdlexternalvariable *h) {
	
	/*
	this is a special entrypoint for the pack and unpack verbs.
	
	12/14/90 dmb: must create verb in memory -- see comment in opverbmemoryunpack
	*/
	
	Handle hpackedpict;
	hdlpictrecord hpict;
	register boolean fl;
	long ix = 0;
	
	if (!loadhandleremains (*ixload, hpacked, &hpackedpict))
		return (false);
	
	/*
	fl = dballochandle (hpackedpict, &adr);
	*/
	
	fl = pictunpack (hpackedpict, &ix, &hpict);
	
	disposehandle (hpackedpict);
	
	if (!fl)
		return (false);
	
	if (!newpictvariable (true, (long) hpict, (hdlpictvariable *) h)) {
		
		pictdisposerecord (hpict);
		
		return (false);
		}
	
	(**hpict).pictrefcon = (long) *h; /*we can get from pict rec to variable rec*/
	
	(**hpict).fldirty = true;
	
	return (true);
	} /*pictverbmemoryunpack*/
	

boolean pictverbpack (hdlexternalvariable h, Handle *hpacked, boolean *flnewdbaddress) {

	/*
	6.2a15 AR: added flnewdbaddress parameter
	*/

	register hdlpictvariable hv = (hdlpictvariable) h;
	register hdlpictrecord hp;
	register boolean fl;
	Handle hpackedpict;
	dbaddress adr;
	hdlwindowinfo hinfo;
	boolean fltempload = false;
	
	if (!(**hv).flinmemory) { /*simple case, pict is resident in the db*/
		
		if (flconvertingolddatabase) {
			
			if (!pictverbinmemory (hv))
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
	
	hp = (hdlpictrecord) (**hv).variabledata;
	
	pictverbcheckwindowrect (hp);
	
	adr = (**hv).oldaddress; /*place where this pict used to be stored*/
	
	if (!fldatabasesaveas && !(**hp).fldirty) /*don't need to update the db version of the pict*/
		goto pushaddress;
		
	hpackedpict = nil; /*force a new handle to be allocated*/
	
	if (!pictpack (hp, &hpackedpict))
		return (false);
	
	fl = dbassignhandle (hpackedpict, &adr);
	
	disposehandle (hpackedpict);
	
	if (!fl)
		return (false);
	
	if (fldatabasesaveas && !fltempload)
		goto pushaddress;
	
	if (!pictwindowopen ((hdlexternalvariable) hv, &hinfo)) { /*it's been saved, we can reclaim some memory*/
		
		(**hv).flinmemory = false;
		
		(**hv).variabledata = adr;
		
		pictdisposerecord (hp); /*reclaim memory used by pict*/
		}
	else {
		
		(**hp).fldirty = false; /*we just saved off a new db version*/
		
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
	} /*pictverbpack*/


boolean pictverbunpack (Handle hpacked, long *ixload, hdlexternalvariable *hvariable) {

	dbaddress adr;
	
	if (!loadlongfromdiskhandle (hpacked, ixload, &adr)) 
		return (false);
		
	return (newpictvariable (false, adr, (hdlpictvariable *) hvariable));
	} /*pictverbunpack*/


boolean pictverbpacktotext (hdlexternalvariable h, Handle htext) {
	
	bigstring bs;
	
	getstringlist (pictstringlist, picttextstring, bs);
	
	pushchar (chreturn, bs);
	
	return (pushtexthandle (bs, htext));
	} /*pictverbpacktotext*/


#if !flruntime

boolean pictverbgetsize (hdlexternalvariable hvariable, long *size) {
	
	register hdlpictvariable hv = (hdlpictvariable) hvariable;
	register PicHandle macpicture;
	
	if (!pictverbinmemory (hv))
		return (false);
	
	macpicture = (**(hdlpictrecord) (**hv).variabledata).macpicture;
	
	*size = gethandlesize ((Handle) macpicture);
	
	return (true);
	} /*pictverbgetsize*/


boolean pictverbgetdisplaystring (hdlexternalvariable h, bigstring bs) {
	
	/*
	get string for table display.  variable is in memory
	*/
	
	long ctbytes;
	
	if (!pictverbgetsize (h, &ctbytes))
		return (false);
	
	parsenumberstring (pictstringlist, pictsizestring, ctbytes, bs);
	
	/*
	numbertostring (ctbytes, bs);
	
	pushstring ("\p bytes", bs);
	*/
	
	return (true);
	} /*pictverbgetdisplaystring*/


boolean pictverbisdirty (hdlexternalvariable hvariable) {
	
	/*
	8/11/92 dmb: check windowrect now to ensure consistent results
	*/
	
	register hdlpictvariable hv = (hdlpictvariable) hvariable;
	register hdlpictrecord hp;
	
	if (!(**hv).flinmemory)
		return (false);
	
	hp = (hdlpictrecord) (**hv).variabledata;
	
	pictverbcheckwindowrect (hp);
	
	return ((**hp).fldirty);
	} /*pictverbisdirty*/


boolean pictverbsetdirty (hdlexternalvariable hvariable, boolean fldirty) {
	
	/*
	4/15/92 dmb: see comments in langexternalsetdirty
	*/
	
	register hdlpictvariable hv = (hdlpictvariable) hvariable;
	
	if (!pictverbinmemory (hv))
		return (false);
	
	(**(hdlpictrecord) (**hv).variabledata).fldirty = fldirty;
	
	return (true);
	} /*pictverbsetdirty*/


boolean pictverbgettimes (hdlexternalvariable h, long *timecreated, long *timemodified) {
	
	register hdlpictvariable hv = (hdlpictvariable) h;
	register hdlpictrecord hp;
	
	if (!pictverbinmemory (hv)) /*couldn't swap it into memory*/
		return (false);
	
	hp = (hdlpictrecord) (**hv).variabledata; /*assume it's in memory*/
	
	*timecreated = (**hp).timecreated;
	
	*timemodified = (**hp).timelastsave;
	
	return (true);
	} /*pictverbgettimes*/


boolean pictverbsettimes (hdlexternalvariable h, long timecreated, long timemodified) {
	
	register hdlpictvariable hv = (hdlpictvariable) h;
	register hdlpictrecord hp;
	
	if (!pictverbinmemory (hv)) /*couldn't swap it into memory*/
		return (false);
	
	hp = (hdlpictrecord) (**hv).variabledata; /*assume it's in memory*/
	
	(**hp).timecreated = timecreated;
	
	(**hp).timelastsave = timemodified;
	
	return (true);
	} /*pictverbsettimes*/


boolean pictwindowopen (hdlexternalvariable hvariable, hdlwindowinfo *hinfo) {

	register hdlpictvariable hv = (hdlpictvariable) hvariable;
	register hdlpictrecord hp;
	
	if (!(**hv).flinmemory)
		return (false);
	
	hp = (hdlpictrecord) (**hv).variabledata;
	
	shellfinddatawindow ((Handle) hp, hinfo); /*3/19/91 DW*/
	
	return ((**hp).flwindowopen);
	} /*pictwindowopen*/


boolean pictedit (hdlexternalvariable hvariable, hdlwindowinfo hparent, ptrfilespec fs, bigstring bstitle, rectparam rzoom) {
	
	/*
	5.0.2b6 dmb: added flwindowopen loop to handle Windows async overlap
	*/

	register hdlpictvariable hv = (hdlpictvariable) hvariable;
	register hdlpictrecord hp;
	Rect rwindow;
	WindowPtr w;
	hdlwindowinfo hi;

	if (!pictverbinmemory (hv)) /*couldn't swap it into memory*/
		return (false);
	
	hp = (hdlpictrecord) (**hv).variabledata; /*assume it's in memory*/
	
	while ((**hp).flwindowopen) { /*bring to front, return true*/
		
		hdlwindowinfo hinfo;
		
		if (shellfinddatawindow ((Handle) hp, &hinfo)) {
			
			if ((*rzoom).top > -2)
				shellbringtofront (hinfo);
			
			return (true);
			}

		#ifdef MACVERSION
			break;
		#else
			if (!shellyield (false))
				return (false);
		#endif
		}
	
	pictgetnewwindowrect (hp, &rwindow);
	
	if (!newchildwindow (idpictconfig, hparent, &rwindow, rzoom, bstitle, &w))
		return (false);
		
	getwindowinfo (w, &hi);
	
	(**hi).hdata = (Handle) hp;
	
	if (fs != nil)
		(**hi).fspec = *fs;
	
	(**hp).windowrect = (**hi).contentrect;
	
	(**hp).flwindowopen = true;
	
	if ((**hp).fldirty)
		shellsetwindowchanges (hi, true);
	
	windowzoom (w);
	
	return (true);
	} /*pictedit*/


static boolean getpictparam (hdltreenode hfirst, short pnum, hdlpictvariable *hv) {
	
	short id;
	
	if (!langexternalgetexternalparam (hfirst, pnum, &id, (hdlexternalvariable *) hv)) {
		
		errornum = namenotpicterror;
		
		return (false);
		}
	
	if (id != idpictprocessor) {
		
		errornum = namenotpicterror;
		
		return (false);
		}
	
	return (true);
	} /*getpictparam*/


static boolean pictverbscheduleupdate (hdltreenode hparam1, tyvaluerecord *v) {
	
	short period;
	
	flnextparamislast = true;
	
	if (!getintvalue (hparam1, 1, &period))
		return (false);
	
	pictscheduleupdate (period);
	
	(*v).data.flvalue = true;
	
	return (true);
	} /*pictverbscheduleupdate*/
			

static boolean pictverbbitmapupdate (hdltreenode hparam1, tyvaluerecord *v) {
	
	boolean flbitmap;
	
	flnextparamislast = true;
	
	if (!getbooleanvalue (hparam1, 1, &flbitmap))
		return (false);
	
	pictsetbitmapupdate (flbitmap);
	
	(*v).data.flvalue = true;
	
	return (true);
	} /*pictverbbitmapupdate*/
	
			
static boolean pictverbevaluator (hdltreenode hparam1, tyvaluerecord *v) {
	
	boolean flevaluate;
	
	flnextparamislast = true;
	
	if (!getbooleanvalue (hparam1, 1, &flevaluate))
		return (false);
	
	pictsetevaluate (flevaluate);
	
	(*v).data.flvalue = true;
	
	return (true);
	} /*pictverbevaluator*/

/*
static boolean pictverbgetbounds (hdltreenode hparam1, tyvaluerecord *v) {
	
	Rect r;
	
	if (!langcheckparamcount (hparam1, 0)) /*shouldn't have any parameters%/
		return (false);
	
	if (!pictgetframerect (pictdata, &r))
		clearbytes (&r, sizeof (r));
	
	return (!newheapvalue (&r, sizeof (r), rectvaluetype, v));
 	} /*pictverbgetbounds*/


static boolean pictverbgetpicture (hdltreenode hparam1, tyvaluerecord *v) {
	
	bigstring bsname;
	Handle hpict;
	hdlhashtable htable;
	tyvaluerecord pictval;
	
	flnextparamislast = true;
	
	if (!getvarparam (hparam1, 1, &htable, bsname))
		return (false);
	
	if (!copyhandle ((Handle) (**pictdata).macpicture, &hpict))
		return (false);
	
	if (hpict == nil) /*picture was empty -- not an error, but return false to caller*/
		return (true);
	
	if (!setbinaryvalue (hpict, 'PICT', &pictval))
		return (false);
	
	if (!hashtableassign (htable, bsname, pictval))
		return (false);
	
	exemptfromtmpstack (&pictval);
	
	(*v).data.flvalue = true;
	
	return (true);
 	} /*pictverbgetpicture*/

	
static void pictresetpicture (void) {
	
	windowsetchanges (pictwindow, true);
	
	shellinvalcontent (pictwindowinfo);
	
	pictresetscrollbars ();
	
	pictdirty ();
	} /*pictresetpicture*/


static boolean pictverbsetpicture (hdltreenode hparam1, tyvaluerecord *v) {
	
	register hdlpictrecord hp = pictdata;
	Handle hpict;
	OSType typeid;
	
	flnextparamislast = true;
	
	setbooleanvalue (false, v); /*assume the worst*/
	
	if (!getbinaryvalue (hparam1, 1, false, &hpict))
		return (false);
	
	pullfromhandle (hpict, 0L, sizeof (typeid), &typeid);
	
	/***should enforce typeid == 'PICT'*/
	
	if ((**hp).macpicture == nil) {
		
		if (!copyhandle (hpict, &hpict))
			return (false);
		
		(**hp).macpicture = (PicHandle) hpict;
		}
	else {
		
		if (!copyhandlecontents (hpict, (Handle) (**hp).macpicture))
			return (false);
		}
	
	pictresetpicture ();
	
	(*v).data.flvalue = true;
	
	return (true);
 	} /*pictverbsetpicture*/


static boolean pictfunctionvalue (short token, hdltreenode hparam1, tyvaluerecord *vreturned, bigstring bserror) {
	
	/*
	bridges pict.c with the language.  the name of the verb is bs, its first parameter
	is hparam1, and we return a value in vreturned.
	
	we use a limited number of support routines from lang.c to get parameters and
	to return values. 
	
	return false only if the error is serious enough to halt the running of the script
	that called us, otherwise error values are returned through the valuerecord, which
	is available to the script.
	
	if we return false, we try to provide a descriptive error message in the 
	returned string bserror.
	
	8/25/92 dmb: commented-out getboundsfunc; undocumented
	
	6/1/93 dmb: when vreturned is nil, return whether or not verb token must 
	be run in the Frontier process
	*/
	
	register tyvaluerecord *v = vreturned;
	register boolean fl;
	WindowPtr targetwindow;
	
	if (v == nil) /*need Frontier process?*/
		return (true);
	
	errornum = 0;
	
	setbooleanvalue (false, v); /*by default, pict functions return false*/
	
	/*be sure there's a pict window in front -- set globals*/
	
	if (!langfindtargetwindow (idpictprocessor, &targetwindow)) {
		
		errornum = nopicterror;
		
		goto error;
		}
	
	shellpushglobals (targetwindow);
	
	(*shellglobals.gettargetdataroutine) (idpictprocessor); /*set pict globals*/
	
	fl = false; /*default return value*/
	
	switch (token) { /*verbs that require that a pict window be open*/
		
		case scheduleupdatefunc:
			fl = pictverbscheduleupdate (hparam1, v);
			
			break;
		
		/*
		case bitmapupdatefunc:
			fl = pictverbbitmapupdate (hparam1, v);
			
			break;
		*/
		
		case evalfunc:
			fl = pictverbevaluator (hparam1, v);
			
			break;
		
		case getpicturefunc:
			fl = pictverbgetpicture (hparam1, v);
			
			break;
		
		case setpicturefunc:
			fl = pictverbsetpicture (hparam1, v);
			
			break;
		
		/*
		case getboundsfunc:
			fl = pictverbgetbounds (hparam1, v);
			
			break;
		*/
		} /*switch*/
	
	shellupdatescrollbars (pictwindowinfo);
	
	shellpopglobals ();
	
	return (fl);
	
	error:
	
	if (errornum != 0) /*get error string, insert verb name if called for*/
		getstringlist (picterrorlist, errornum, bserror);
	
	return (false);
	} /*pictfunctionvalue*/


static boolean pictinitverbs (void) {
	
	return (loadfunctionprocessor (idpictverbs, &pictfunctionvalue));
	} /*pictinitverbs*/


static boolean pictgetcontentsize (long *width, long *height) {
	
	/*
	our ideal content size is the size of the picture with a 10 
	pixel border all around
	*/
	
	Rect r;
	
	if (!pictgetframerect (pictdata, &r))
		return (false);
	
	*width = (long) r.right - r.left + 20;
	
	*height = (long) r.bottom - r.top + 20;
	
	return (true);
	} /*pictgetcontentsize*/


static void pictresize (void) {
	
	} /*pictresize*/


static boolean pictundosetpict (Handle hpict, boolean flundo) {
	
	if (flundo) {
		
		pushundostep (pictundosetpict, (Handle) (**pictdata).macpicture);
		
		(**pictdata).macpicture = (PicHandle) hpict;
		
		pictresetpicture ();
		}
	else
		disposehandle (hpict);
	
	return (true);
	} /*pictundosetpict*/


static boolean pictclear (void) {
	
	pushundoaction (undoclearstring);
	
	pictundosetpict (nil, true);
	
	return (true);
	} /*pictclear*/


static boolean pictcopy (void) {
	
	Handle hpicture;
	
	if (!copyhandle ((Handle) (**pictdata).macpicture, &hpicture))
		return (false);
	
	if (hpicture == nil)
		return (false);
	
	return (shellsetscrap (hpicture, pictscraptype, disposehandle, nil));
	} /*pictcopy*/


static boolean pictcut (void) {
	
	pushundoaction (undocutstring);
	
	return (pictcopy () && pictundosetpict (nil, true));
	} /*pictcut*/


static boolean pictpaste (void) {
	
	Handle hpict;
	boolean fltempscrap;
	
	pushundoaction (undopastestring);
	
	if (!shellconvertscrap (pictscraptype, &hpict, &fltempscrap))
		return (false);
	
	if (!fltempscrap) { /*we need our own copy*/
		
		if (!copyhandle (hpict, &hpict))
			return (false);
		}
	
	pictundosetpict (hpict, true);
	
	return (true);
	} /*pictpaste*/


boolean pictverbfind (hdlexternalvariable hvariable, boolean *flzoom) {
	
	return (false);
	} /*pictverbfind*/


static boolean pictsearch (void) {
	
	long pictrefcon = (**pictdata).pictrefcon;
	
	startingtosearch (pictrefcon);
	
	if (!searchshouldcontinue (pictrefcon))
		return (false);
	
	return (langexternalcontinuesearch ((hdlexternalvariable) pictrefcon));
	} /*pictsearch*/


static boolean pictgetvariable (hdlexternalvariable *hvariable) {
	
	if (pictdata == nil)
		return (false);
	
	*hvariable = (hdlexternalvariable) (**pictdata).pictrefcon;
	
	return (true);
	} /*pictgetvariable*/


static boolean pictpagesetup (void) {
	
	return (true);
	} /*pictpagesetup*/


static boolean pictsetprintinfo (void) {
	
	shellprintinfo.ctpages = 1;
	
	return (true);
	} /*pictsetprintinfo*/


static boolean pictprint (short pagenumber) {
	
	register hdlpictrecord hp = pictdata;
	register PicHandle macpicture = (**hp).macpicture;
	register boolean fleval;
	Rect rframe;
	
	pictgetframerect (hp, &rframe);
	
	offsetrect (&rframe,
		shellprintinfo.paperrect.left - rframe.left,
		shellprintinfo.paperrect.top - rframe.top);
	
	fleval = (**hp).flevalexpressions;
	
	if (fleval) 
		pictupdatepatcher (); /*install our quickdraw patches*/
	
	DrawPicture (macpicture, &rframe);
	
	if (fleval) 
		pictdepatcher (); /*de-install 'em*/
	
	return (true);
	} /*pictprint*/


static boolean pictclose (void) {
	
	register hdlpictrecord hp = pictdata;
	register hdlpictvariable hv = (hdlpictvariable) (**hp).pictrefcon;
	
	assert (hv != nil); /*defensive driving*/
	
	langexternalwindowclosed ((hdlexternalvariable) hv);
	
	killundo (); /*must toss undos before they're stranded*/
	
	pictverbcheckwindowrect (hp);
	
	if ((**hp).fldirty) { /*we have to keep the in-memory version around*/
		
		(**hp).flwindowopen = false;
		}
	
	else { /*the db version is identical to memory version, save memory*/
		
		assert ((**hv).oldaddress != nildbaddress);
		
		(**hv).flinmemory = false;
		
		(**hv).variabledata = (long) (**hv).oldaddress;
		
		pictdisposerecord (hp); /*reclaim memory*/
		}
		
	return (true);
	} /*pictclose*/
	
	
static boolean pictkeystroke (void) {
	
	if (langexternalsurfacekey ((hdlexternalvariable) (**pictdata).pictrefcon))
		return (true);
	
	return (true);
	} /*pictkeystroke*/


static boolean picttitleclick (Point pt) {
	
	return (langexternaltitleclick (pt, (hdlexternalvariable) (**pictdata).pictrefcon));
	} /*picttitleclick*/


static boolean pictgettargetdata (short id) {
	
	/*
	a verb is about to be executed that acts on the indicated external type, or 
	on any shell window if id == -1.
	
	return true if we can handle verbs of that type and are able to set the 
	relevant globals
	*/
	
	switch (id) {
		
		case -1:
			return (true);
		
		case idpictprocessor: /*don't need to do anything -- our pict globals are pushed*/
			return (true);
		
		default:
			return (false);
		}
	} /*pictgettargetdata*/


boolean pictstart (void) {
	
	/*
	set up callback routines record, and link our data into the shell's 
	data structure.
	*/
	
	ptrcallbacks pictcallbacks;
	register ptrcallbacks cb;
	
	assert (sizeof (tyexternalvariable) == sizeof (typictvariable));
	
	pictinitverbs ();
	
	shellnewcallbacks (&pictcallbacks);
	
	cb = pictcallbacks; /*copy into register*/
	
	loadconfigresource (idpictconfig, &(*cb).config);
		
	(*cb).configresnum = idpictconfig;
		
	(*cb).windowholder = &pictwindow;
	
	(*cb).dataholder = (Handle *) &pictdata;
	
	(*cb).infoholder = &pictwindowinfo; 
	
#ifdef version42orgreater
	
	(*cb).disposerecordroutine = ccdisposefilerecord;
	
	(*cb).saveroutine = ccsavespecialfile;

#endif
	
	(*cb).updateroutine = &pictupdate;
	
	(*cb).getcontentsizeroutine = &pictgetcontentsize;
	
	(*cb).resizeroutine = &pictresize;
	
	(*cb).scrollroutine = &pictscroll;
	
	(*cb).setscrollbarroutine = &pictresetscrollbars;
	
	(*cb).cutroutine = &pictcut;
	
	(*cb).copyroutine = &pictcopy;
	
	(*cb).pasteroutine = &pictpaste;
	
	(*cb).clearroutine = &pictclear;
	
	(*cb).searchroutine = &pictsearch;
	
	(*cb).getvariableroutine = &pictgetvariable;
	
	(*cb).closeroutine = &pictclose;
	
	(*cb).pagesetuproutine = &pictpagesetup;
	
	(*cb).printroutine = &pictprint;
	
	(*cb).setprintinfoproutine = &pictsetprintinfo;
	
	(*cb).idleroutine = &pictidle;
	
	(*cb).keystrokeroutine = &pictkeystroke;
	
	(*cb).titleclickroutine = &picttitleclick;
	
	(*cb).gettargetdataroutine = &pictgettargetdata;
	
	return (true);
	} /*pictstart*/

#endif // !flruntime




