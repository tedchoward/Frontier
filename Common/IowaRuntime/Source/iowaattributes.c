
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

#include "shelltypes.h"
#include <iac.h>
#include <appletquickdraw.h>
#include "iowacore.h"
#include "iowaattributes.h"

#ifdef coderesource

	#include <SetUpA4.h>

#endif

#ifdef iowaRuntime
	
	#include <appletquickdraw.h>
	#include "iowaruntime.h"

#endif



void consumetexthandle (Handle htext, bigstring bstext) {
	
	/*
	all of the IAC routines that take a text handle as a parameter
	just copy them into a bigstring and dispose of the handle.
	
	two things are at work here:
	
	1. object names are limited to 255 characters.
		
	2. heap-allocated AE parameters belong to the caller. they
	must either be linked into a structure or disposed of.
	*/
		
	texthandletostring (htext, bstext);
	
	disposehandle (htext);
	} /*consumetexthandle*/


boolean nocarderror (void) {
	
	if (iowadata != nil) /*it's not an error, therefore return false*/
		return (false);
	
	IACreturnerror (-1, "\pThere aren’t any Card Editor windows open at this time");
	
	return (true);
	} /*nocarderror*/
	

boolean namedobjecterror (Handle objectname, hdlobject *hobject) {
	
	bigstring bsname, bserror;
	
	consumetexthandle (objectname, bsname);
	
	if (nocarderror ()) /*error*/
		return (true);
	
	if (lookupname (bsname, hobject)) /*no error*/
		return (false);
		
	copystring ("\pThere is no object named “", bserror);
	
	pushstring (bsname, bserror);
	
	pushstring ("\p” in the target Card Editor window", bserror);
	
	IACreturnerror (-1, bserror);
	
	return (true);
	} /*namedobjecterror*/


#if 0

static boolean IACpushobjectparam (hdlobject hobject, OSType keyword) {

	boolean fl;
	
	if (hobject == nil)
		fl = IACpushstringparam ("\p", keyword);
	
	else {
	
		IACglobals.dontdisposenextparam = true;
		
		fl = IACpushtextparam ((**hobject).objectname, keyword);
		}
	
	return (fl);
	} /*IACpushobjectparam*/

#endif

	
boolean IACgetobjectattributes (Handle objectname) {
	
	/*
	7/31/96 dmb: use new 	IACglobals.dontdisposenextparam flag instead of 
	duplicating all of the handles that we pass
	
	7/31/96 dmb: xxx include the name of the next object and the first child 
	object in the attributes table [decided not to enable this yet.]
	*/
	
	hdlobject hobject;
	Handle hvalue;
	tyobject obj;
	bigstring bsfont;
	
	if (nocarderror ()) /*error*/
		return (false);
	
	if (gethandlesize (objectname) == 0)
		hobject = (**iowadata).objectlist;
	else {
	
		if (namedobjecterror (objectname, &hobject)) 
			return (false);
		}
		
	obj = **hobject; /*copy for fast access*/
	
	GetFontName (obj.objectfont, bsfont);
	
	/*
	if (!copyhandle (obj.objectname, &obj.objectname))
		goto error;
	*/
	
	getobjectvalue (hobject, &hvalue); /*1.0b15 -- allow for the object being in editmode*/
	
	/*
	if (!copyhandle (hvalue, &hvalue))
		goto error;
	
	if (!copyhandle (obj.objectscript, &obj.objectscript))
		goto error;
	
	if (!copyhandle (obj.objectrecalcscript, &obj.objectrecalcscript))
		goto error;
	
	if (!copyhandle (obj.objecterrormessage, &obj.objecterrormessage))
		goto error;
	*/
	
	IACglobals.event = IACglobals.reply; /*push the params onto the reply record*/
	
	if (!IACpushlongparam (obj.objecttype, 'type'))
		goto error;
	
	if (!IACpushrectparam (&obj.objectrect, 'rect'))
		goto error;
	
	IACglobals.dontdisposenextparam = true;
		
	if (!IACpushtextparam (obj.objectname, 'name'))
		goto error;
	
	/* 7/31/96 dmb: decided not to put these in yet.
	
	if (!IACpushobjectparam (obj.nextobject, 'next'))
		goto error;
	
	if (!IACpushobjectparam (obj.childobjectlist, 'chld'))
		goto error;
	*/
	
	IACglobals.dontdisposenextparam = true;
		
	if (!IACpushtextparam (hvalue, 'text'))
		goto error;
	
	IACglobals.dontdisposenextparam = true;
		
	if (!IACpushtextparam (obj.objectscript, 'scri'))
		goto error;
	
	IACglobals.dontdisposenextparam = true;
		
	if (!IACpushtextparam (obj.objectrecalcscript, 'rcsc'))
		goto error;
	
	if (!IACpushshortparam (obj.objectrecalcstatus, 'rcst'))
		goto error;
	
	if (!IACpushshortparam (obj.objectrecalcperiod, 'rcpd'))
		goto error;
	
	if (!IACpushstring4param (obj.objectlanguage, 'lang'))
		goto error;
	
	IACglobals.dontdisposenextparam = true;
		
	if (!IACpushtextparam (obj.objecterrormessage, 'errs'))
		goto error;
	
	if (!IACpushbooleanparam (obj.objectflag, 'flag'))
		goto error;
		
	if (!IACpushbooleanparam (obj.objectvisible, 'visi'))
		goto error;
		
	if (!IACpushbooleanparam (obj.objectenabled, 'enab'))
		goto error;
		
	if (!IACpushbooleanparam (obj.objecttransparent, 'tran'))
		goto error;
		
	if (!IACpushbooleanparam (obj.objectautosize, 'auto'))
		goto error;
		
	if (!IACpushbooleanparam (obj.objectinval, 'inva'))
		goto error;
		
	if (!IACpushbooleanparam (obj.objecthasframe, 'fram'))
		goto error;
		
	if (!IACpushstringparam (bsfont, 'font'))
		goto error;
	
	if (!IACpushshortparam (obj.objectfontsize, 'size'))
		goto error;
	
	if (!IACpushshortparam (obj.objectstyle, 'styl'))
		goto error;
	
	if (!IACpushshortparam ((short) obj.objectjustification, 'just'))
		goto error;
	
	if (!IACpushRGBColorparam (&obj.objectfillcolor, 'filc'))
		goto error;
	
	if (!IACpushRGBColorparam (&obj.objecttextcolor, 'txtc'))
		goto error;
	
	if (!IACpushRGBColorparam (&obj.objectframecolor, 'frmc'))
		goto error;
	
	if (!IACpushshortparam (obj.objectdropshadowdepth, 'sdwd'))
		goto error;
	
	if (!callgetattributes (hobject, IACglobals.reply)) /*1.0b15*/
		goto error;
		
	return (true);
	
	error:
	
	return (false);
	} /*IACgetobjectattributes*/


static void invaldeferred (hdlobject h) {

	Rect r, rdeferred;

	getobjectinvalrect (h, &r);
	
	rdeferred = (**iowadata).deferredinvalrect;
	
	UnionRect (&rdeferred, &r, &rdeferred);
	
	(**iowadata).deferredinvalrect = rdeferred;
	} /*invaldeferred*/
	
	
boolean IACsetobjectattributes (Handle objectname) {
	
	hdlobject hobject;
	tyobject obj;
	bigstring bsfont;
	Handle htext;
	Boolean fl;
	
	if (namedobjecterror (objectname, &hobject)) 
		return (false);
				
	obj = **hobject; /*copy for fast access*/
	
	IACglobals.nextparamoptional = true;
	
	if (IACgetbooleanparam ('visi', &fl)) obj.objectvisible = fl; IACglobals.nextparamoptional = true;
	
	if (IACgetbooleanparam ('enab', &fl)) obj.objectenabled = fl; IACglobals.nextparamoptional = true;
	
	if (IACgetbooleanparam ('tran', &fl)) obj.objecttransparent = fl; IACglobals.nextparamoptional = true;
	
	if (IACgetbooleanparam ('auto', &fl)) obj.objectautosize = fl; IACglobals.nextparamoptional = true;
	
	if (IACgetbooleanparam ('inva', &fl)) obj.objectinval = fl; IACglobals.nextparamoptional = true;
	
	if (IACgetbooleanparam ('fram', &fl)) obj.objecthasframe = fl; IACglobals.nextparamoptional = true;
	
	IACgetshortparam ('size', &obj.objectfontsize); IACglobals.nextparamoptional = true;
	
	IACgetshortparam ('styl', &obj.objectstyle); IACglobals.nextparamoptional = true;
	
	IACgetshortparam ('rcst', &obj.objectrecalcstatus); IACglobals.nextparamoptional = true;
	
	IACgetshortparam ('rcpd', &obj.objectrecalcperiod); IACglobals.nextparamoptional = true;
	
	IACgetstring4param ('lang', &obj.objectlanguage); IACglobals.nextparamoptional = true;
	
	/*justification*/ {
		
		short x;
	
		if (IACgetshortparam ('just', &x))
			obj.objectjustification = (tyjustification) x;
		
		IACglobals.nextparamoptional = true;
		}
		
	/*the object rectangle*/ {
		
		if (IACgetrectparam ('rect', &obj.objectrect)) 
			invaldeferred (hobject);
		
		IACglobals.nextparamoptional = true;
		}

	/*the object type*/ {
	
		if (IACgetlongparam ('type', &obj.objecttype)) 
			invaldeferred (hobject);
		
		IACglobals.nextparamoptional = true;
		}
	
	/*the object flag*/ {
	
		if (IACgetbooleanparam ('flag', &fl)) {
		
			obj.objectflag = fl; 
			
			invaldeferred (hobject);
			}
	
		IACglobals.nextparamoptional = true;
		}
	
	IACgetRGBColorparam ('filc', &obj.objectfillcolor); IACglobals.nextparamoptional = true;
	
	IACgetRGBColorparam ('txtc', &obj.objecttextcolor); IACglobals.nextparamoptional = true;
	
	IACgetRGBColorparam ('frmc', &obj.objectframecolor); IACglobals.nextparamoptional = true;
	
	IACgetshortparam ('sdwd', &obj.objectdropshadowdepth); IACglobals.nextparamoptional = true;
	
	if (IACgetstringparam ('font', bsfont)) 
		GetFNum (bsfont, &obj.objectfont);
	
	**hobject = obj; /*copy it back into the heap*/
		
	if (IACgettextparam ('name', &htext)) 
		setobjectname (hobject, htext);
	
	IACglobals.nextparamoptional = true;
		
	if (IACgettextparam ('text', &htext)) 
		setobjectvalue (hobject, htext);
	
	IACglobals.nextparamoptional = true;
		
	if (IACgettextparam ('scri', &htext)) 
		setobjectscript (hobject, htext);
	
	IACglobals.nextparamoptional = true;
		
	if (IACgettextparam ('rcsc', &htext)) 
		setobjectrecalcscript (hobject, htext);
	
	IACglobals.nextparamoptional = true;
		
	if (IACgettextparam ('errs', &htext)) 
		setobjecterrormessage (hobject, htext);
		
	callsetattributes (hobject, IACglobals.event); /*1.0b15*/
	
	if (!emptyrect ((**iowadata).deferredinvalrect)) { /*1.0b15 -- redraw on every attribute change*/
		
		Rect r = (**iowadata).deferredinvalrect;
		
		iowainvalrect (&r);
		
		zerorect (&r);
		
		(**iowadata).deferredinvalrect = r;
		}
	
	invalobject (hobject);
	
	iowaupdatenow ();
		
	return (true);
	} /*IACsetobjectattributes*/


boolean IACgetcardattributes (void) {
	
	tycard card;
	
	if (nocarderror ()) 
		return (false);
	
	card = **iowadata; /*copy for fast access, smaller code*/
	
	IACglobals.event = IACglobals.reply; /*push the params onto the reply record*/
	
	/*push name of the active text object on reply*/ {
		
		bigstring bs;
		
		if (card.activetextobject == nil)
			setstringlength (bs, 0);
		else
			texthandletostring ((**card.activetextobject).objectname, bs);
			
		if (!IACpushstringparam (bs, 'atob'))
			goto error;
		}
	
	if (!IACpushRGBColorparam (&card.backcolor, 'bakc'))
		goto error;
		
	if (!IACpushbooleanparam (card.flgrid, 'grid'))
		goto error;
		
	if (!IACpushshortparam (card.gridunits, 'grun'))
		goto error;
		
	if (!IACpushbooleanparam (card.flinvisiblegrid, 'grvs'))
		goto error;
		
	if (!IACpushbooleanparam (card.recalcneeded, 'rcal'))
		goto error;
		
	if (!IACpushlongparam (card.recalctickcount, 'rctc'))
		goto error;
		
	if (!IACpushshortparam (card.rightborder, 'rbor'))
		goto error;
		
	if (!IACpushshortparam (card.bottomborder, 'bbor'))
		goto error;
		
	if (!IACpushbooleanparam (card.floater, 'floa'))
		goto error;
		
	if (!IACpushlongparam (card.idwindow, 'wind'))
		goto error;
					
	return (true);
	
	error:
	
	return (false);
	} /*IACgetcardattributes*/


boolean IACsetcardattributes (void) {
	
	tycard card;
	/*bigstring nameactivetextobject;*/
	Boolean fl;
	
	if (nocarderror ()) 		
		return (false);
	
	card = **iowadata; /*copy for fast access, smaller code*/
	
	IACglobals.nextparamoptional = true;
	
	/*IACgetstringparam ('atob', nameactivetextobject); IACglobals.nextparamoptional = true;*/

	IACgetRGBColorparam ('bakc', &card.backcolor); IACglobals.nextparamoptional = true;

	if (IACgetbooleanparam ('grid', &fl)) card.flgrid = fl; IACglobals.nextparamoptional = true;

	IACgetshortparam ('grun', &card.gridunits); IACglobals.nextparamoptional = true;
	
	if (IACgetbooleanparam ('grvs', &fl)) card.flinvisiblegrid = fl; IACglobals.nextparamoptional = true;

	if (IACgetbooleanparam ('rcal', &fl)) {card.recalcneeded = fl; card.recalcobject = nil;}; IACglobals.nextparamoptional = true;

	IACgetlongparam ('rctc', &card.recalctickcount); IACglobals.nextparamoptional = true;
	
	IACgetshortparam ('rbor', &card.rightborder); IACglobals.nextparamoptional = true;
	
	IACgetshortparam ('bbor', &card.bottomborder); IACglobals.nextparamoptional = true;
	
	if (IACgetbooleanparam ('floa', &fl)) card.floater = fl; IACglobals.nextparamoptional = true;

	IACgetlongparam ('wind', &card.idwindow); IACglobals.nextparamoptional = true;
	
	card.fullupdate = true; /*force card to redraw on idle*/
	
	**iowadata = card; /*copy it back into the heap*/
	
	return (true);
	} /*IACsetcardattributes*/
	
	
#ifndef iowaRuntime
	
	
	static void getCardAttributesVerb (void) {
		
		Boolean returnedValue;
		
		returnedValue = IACgetcardattributes ();
		
		IACreturnboolean (returnedValue);
		} /*getCardAttributesVerb*/
	
	
	static void setCardAttributesVerb (void) {
		
		Boolean returnedValue;
		
		returnedValue = IACsetcardattributes ();
		
		madechanges ();
		
		IACreturnboolean (returnedValue);
		} /*setCardAttributesVerb*/
	
	
	static void getObjectAttributesVerb (void) {
		
		Handle param1;
		Boolean returnedValue;
		
		if (!IACgettextparam ('prm1', &param1))
			return;
		
		returnedValue = IACgetobjectattributes (param1);
		
		IACreturnboolean (returnedValue);
		} /*getObjectAttributesVerb*/
	
	
	static void setObjectAttributesVerb (void) {
		
		Handle param1;
		Boolean returnedValue;
		
		if (!IACgettextparam ('prm1', &param1))
			return;
		
		returnedValue = IACsetobjectattributes (param1);
		
		madechanges ();
		
		IACreturnboolean (returnedValue);
		} /*setObjectAttributesVerb*/
	
	
	static pascal OSErr attributeshandler (AppleEvent *event, AppleEvent *reply, long refcon) {
	
		#ifdef coderesource
		
			EnterCallback ();
			
		#endif
		
		IACglobals.event = event; 
		
		IACglobals.reply = reply;
		
		IACglobals.refcon = refcon;
		
		switch (IACgetverbtoken ()) {
			
			case 'gacd':
				getCardAttributesVerb ();
				
				break;
			
			case 'sacd':
				setCardAttributesVerb ();
				
				break;
			
			case 'gaob':
				getObjectAttributesVerb ();
				
				break;
			
			case 'saob':
				setObjectAttributesVerb ();
				
				break;
			
			} /*switch*/
			
		#ifdef coderesource
		
			ExitCallback ();
			
		#endif
		
		return (noErr);
		} /*attributeshandler*/
	
	
	boolean installattributeshandler (void) {
		
		#ifdef coderesource
		
			PrepareCallback ();
			
		#endif
		
		IACinstallsystemhandler ('ceda', typeWildCard, (ProcPtr) &attributeshandler);
		
		return (true);
		} /*installattributeshandler*/
	
#endif


