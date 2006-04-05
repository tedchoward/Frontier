
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
#include "cursor.h"
#include "kb.h"
#include "mouse.h"
#include "strings.h"
#include "db.h"
#include "file.h"
#include "font.h"
#include "frontierconfig.h"
#include "shell.h"
#include "shellhooks.h"
#include "shellprivate.h"
#include "lang.h"
#include "langinternal.h"
#include "langtokens.h"
#include "tablestructure.h"
#include "langexternal.h"
#include "cancoon.h"
#include "menuverbs.h"
#include "opverbs.h"
#include "pictverbs.h"
#include "tableverbs.h"
#include "wpverbs.h"
#ifdef fliowa
	#include "iowaverbs.h" /*3/18/92 dmb*/
#endif
#include "tableformats.h" /*7.0b6 PBS*/
#include "menuverbs.h" /*7.0b6 PBS*/
#include "op.h" /*7.0b6 PBS*/



/*
boolean langexternalload (hdlexternalvariable hvariable, Handle *h) {
	
	register hdlexternalvariable hv = hvariable;
	
	if ((**hv).flpathlink) {
		bigstring bspath;
		short fnum;
		boolean fl;
		
		copyheapstring ((hdlstring) (**hv).variabledata, bspath);
		
		if (!openfile (bspath, 0, &fnum))
			return (false);
		
		fl = filereadhandle (fnum, h);
		
		closefile (fnum);
		
		return (fl);
		}
	
	return (dbrefhandle ((dbaddress) (**hv).variabledata, h));
	} /%langexternalload%/


boolean langexternalsave (hdlexternalvariable hvariable, Handle h, dbaddress *adr) {
	
	register hdlexternalvariable hv = hvariable;
	
	if ((**hv).flpathlink) {
		bigstring bspath;
		short fnum;
		boolean fl;
		
		copyheapstring ((hdlstring) *adr, bspath);
		
		if (!fileopenorcreate (bspath, 0, 'LAND', '2CLK', &fnum))
			return (false);
		
		fl = filewritehandle (fnum, h);
		
		closefile (fnum);
		
		return (fl);
		}
	
	return (dbassignhandle (h, adr));
	} /%langexternalsave%/
*/

tyexternalid langexternalgettype (tyvaluerecord val) {
	
	/*
	call this when you know that val.valuetype is externalvaluetype.
	
	we hide the details of how the type of an external value is stored.
	*/
	
	if (val.valuetype != externalvaluetype) /*defensive driving*/
		return ((tyexternalid) -1);
	
	return ((tyexternalid) (**(hdlexternalhandle) val.data.externalvalue).id);
	} /*langexternalgettype*/
	
	
static boolean langexternalgetinfo (bigstring bs, hdlhashtable *htable, langvaluecallback *valueroutine) {
	
	/*
	a very central bottleneck -- we translate the name of an EFP (something like
	"shell" or "wp") into a symbol table and a routine which can be called to
	get a function value.
	
	indicate in flwindow whether the EFP requires that a window be open in order to
	evaluate a functionvalue.
	*/
	
	tyvaluerecord val;
	register hdlhashtable ht;
	hdlhashnode hnode;
	
	if (!langgetsymbolval (bs, &val, &hnode)) /*has special case for root*/
		return (false);
	
	if (!tablevaltotable (val, htable, hnode))
		return (false);
		
	ht = *htable; /*copy into register*/
	
	*valueroutine = (**ht).valueroutine;
	
	return (true);
	} /*langexternalgetinfo*/
	

boolean langexternalgettable (bigstring bs, hdlhashtable *htable) {
	
	langvaluecallback valueroutine;
	
	return (langexternalgetinfo (bs, htable, &valueroutine));
	} /*langexternalgettable*/


boolean langexternalvaltotable (tyvaluerecord val, hdlhashtable *htable, hdlhashnode hnode) {
	
	return (tablevaltotable (val, htable, hnode));
	} /*langexternalvaltotable*/


boolean langexternalfindvariable (hdlexternalvariable hv, hdlhashtable *htable, bigstring bsname) {
	
	#if langexternalfind_optimization

	if (hv != nil && (**hv).hexternaltable != nil && (**hv).hexternalnode != nil) {

		*htable = (**hv).hexternaltable;

		gethashkey ((**hv).hexternalnode, bsname);
		
		return true;
		}

	#endif

	return (tablefindvariable (hv, htable, bsname));
	} /*langexternalfindvariable*/


boolean langexternalgettablevalue (hdltreenode hfirst, short pnum, hdlhashtable *htable) {
	
	return (gettablevalue (hfirst, pnum, htable));
	} /*langexternalgettablevalue*/


hdldatabaserecord langexternalgetdatabase (hdlexternalvariable hv) {
	
	if (hv == nil)
		return (nil);
	
	return ((**hv).hdatabase);
	} /*langexternalgetdatabase*/


void langexternalsetdatabase (hdlexternalvariable hv, hdldatabaserecord hdb) {
	
	/*
	5.1.4 dmb: set the database of the external object, only if it's new (i.e.
	it wasn't loaded from a database, in which case the database is fixed)
	*/

	if (hv == nil || hdb == nil)
		return;
	
	if ((**hv).hdatabase == hdb)
		return;
	
	if ((**hv).flinmemory && (**hv).oldaddress == nildbaddress) // a new object, not from disk
		(**hv).hdatabase = hdb;
	} /*langexternalsetdatabase*/


/*
boolean langexternalarrayreference (bigstring bsarrayname, short ix, tyvaluerecord *valref) {
	
	/%
	we want to isolate the interpreter from understanding how arrays are stored.
	that's why this routine is part of the "external" interface of the language.
	
	the punchline -- an array is stored as an outline!  surprise!!!
	%/
	
	register hdlexternalhandle h;
	tyvaluerecord val;
	bigstring bs;
	Handle hnode;
	hdlexternalhandle hnew;
	
	if (!langgetsymbolval (bsarrayname, &val)) {
		
		langparamerror (unknownidentifiererror, bsarrayname);
		
		return (false);
		}
		
	if (val.valuetype != externalvaluetype) {
		
		langparamerror (notarrayerror, bsarrayname);
		
		return (false);
		}
	
	h = (hdlexternalhandle) val.data.externalvalue;
	
	if ((**h).id != idoutlineprocessor) {
		
		langparamerror (notarrayerror, bsarrayname);
		
		return (false);
		}
	
	if (!opverbarrayreference ((**h).hdata, ix, &hnode)) {
		
		langparamerror (arrayreferror, bsarrayname);
		
		return (false);
		}
	
	if (!newclearhandle (sizeof (tyexternalhandle), (Handle *) &hnew))
		return (false);
		
	h = hnew; /%copy into register%/
	
	pushtmpstack ((Handle) h); /%we just allocated a temporary value%/
	
	(**h).id = idheadrecord;
	
	(**h).hdata = hnode;
	
	setexternalvalue ((Handle) h, valref);
	
	return (true);
	} /%langexternalarrayreference%/
*/


boolean langsetexternalsymbol (hdlhashtable htable, bigstring bs, tyexternalid id, Handle hdata) {
	
	/*
	called by an External Function Processor (EFP) to link his data into the
	current symboltable.  bs is the name of the variable, id tells us who is
	responsible for the data, and hdata is the handle we move around to move
	the data, and pass back to the EFP when he needs to do something with it.
	*/
	
	register hdlexternalvariable hv = (hdlexternalvariable) hdata;
	tyvaluerecord val;
	register boolean fl;
	
	(**hv).id = id;
	
	setexternalvalue ((Handle) hv, &val);
	
	fl = hashtableassign (htable, bs, val);
	
	return (fl);
	} /*langsetexternalsymbol*/


static boolean langexternalsetsave (hdlhashtable htable, bigstring bs, boolean fldontsave) {
	
	/*
	set the dont-save bit on the variable whose name is the indicated string.
	*/
	
	hdlhashnode hnode;
	register boolean fl;
	
	pushhashtable (htable);
	
	fl = langfindsymbol (bs, &htable, &hnode);
	
	pophashtable ();
	
	if (fl)
		(**hnode).fldontsave = fldontsave;
		
	return (fl);
	} /*langexternalsetsave*/
	
	
boolean langexternaldontsave (hdlhashtable htable, bigstring bs) {
	
	/*
	set the dont-save bit on the variable whose name is the indicated string.
	*/
	
	return (langexternalsetsave (htable, bs, true));
	} /*langexternaldontsave*/
	
	
boolean langexternalpleasesave (hdlhashtable htable, bigstring bs) {
	
	/*
	clear the dont-save bit on the variable whose name is the indicated string.
	*/
	
	return (langexternalsetsave (htable, bs, false));
	} /*langexternalpleasesave*/
	
	
boolean langexternaltypestring (hdlexternalhandle h, bigstring bs) {
	
	/*
	get the type string for the external value handle.
	*/
	
	register hdlexternalvariable hv = h;
		
	switch ((**hv).id) {
		
		case idoutlineprocessor: case idscriptprocessor:
			opverbgettypestring (hv, bs);
			
			break;
		
		case idwordprocessor:
			wpverbgettypestring (hv, bs);
			
			break;
		
		case idtableprocessor:
			tableverbgettypestring (hv, bs);
			
			break;
		
		case idmenuprocessor:
			menuverbgettypestring (hv, bs);
			
			break;
			
		case idpictprocessor:
			pictverbgettypestring (hv, bs);
			
			break;
		
		#ifdef fliowa
		
		case idcardprocessor:
			cardverbgettypestring (hv, bs);
			
			break;
		
		#endif
		
		default:
			langgetmiscstring (unknownstring, bs);
			
			break;
		} /*switch*/
		
	return (true);
	} /*langexternaltypestring*/


boolean langexternalgetdisplaystring (hdlexternalhandle h, bigstring bs) {
	
	/*
	10/22/90 dmb: now that we treat the external handle's data as an 
	external variable, we can factor some code.  now we only need to 
	switch on the id if the variable is in memory
	*/
	
	register hdlexternalvariable hv = (hdlexternalvariable) h;
	
	setemptystring (bs);
	
	if (hv == nil) /*defensive driving*/
		return (false);
	
	if (!(**hv).flinmemory) {
		
		langgetmiscstring (ondiskstring, bs);
		
		/*
		copystring ("\pon disk", bs);
		*/
		
		/*
		copystring ("\pon disk at ", bs);
		
		pushlong ((**hv).variabledata, bs);
		*/
		
		return (true);
		}
	
	switch ((**h).id) {
		
		case idoutlineprocessor: case idscriptprocessor:
			opverbgetsummitstring (hv, bs);
			
			break;
		
		case idwordprocessor:
			wpverbgetdisplaystring (hv, bs);
			
			break;
		
		case idtableprocessor:
			tableverbgetdisplaystring (hv, bs);
			
			break;
			
		case idmenuprocessor:
			menuverbgetdisplaystring (hv, bs);
			
			break;
			
		case idpictprocessor:
			pictverbgetdisplaystring (hv, bs);
			
			break;
		
		#ifdef fliowa
		
		case idcardprocessor:
			cardverbgetdisplaystring (hv, bs);
			
			break;
		
		#endif
		
		default:
			langgetmiscstring (unknownstring, bs);
			
			break;
		} /*switch*/
		
	return (true);
	} /*langexternalgetdisplaystring*/
	

static boolean langexternalvariablewindowopen (hdlexternalvariable hvariable, hdlwindowinfo *hinfo) {
	
	/*
	4/15/92 dmb: extracted from langexternalwindowopen, which takes a value parameter
	*/
	
	register hdlexternalvariable hv = hvariable;
	
	*hinfo = nil; /*5.0d19 dmb*/
	
	switch ((**hv).id) {
	
		case idtableprocessor:
			return (tablewindowopen (hv, hinfo));
			
		case idoutlineprocessor: case idscriptprocessor:
			return (opwindowopen (hv, hinfo));
			
		case idwordprocessor:
			return (wpwindowopen (hv, hinfo));
			
		case idmenuprocessor:
			return (menuwindowopen (hv, hinfo));
			
		case idpictprocessor:
			return (pictwindowopen (hv, hinfo));
		
		#ifdef fliowa
		
		case idcardprocessor:
			return (cardwindowopen (hv, hinfo));
		
		#endif
		
		} /*switch*/

	return (false);
	} /*langexternalvariablewindowopen*/


boolean langexternalregisterwindow (hdlexternalvariable hv) {
	
	/*
	5.1.5b10 dmb: even if windowgetpath fails, the fspec has the (unsaved) default name
	*/
	
	hdlwindowinfo hinfo;
	WindowPtr w;
	tyfilespec fs;
	bigstring bsname;
	tyvaluerecord val;
	
	if (!langexternalvariablewindowopen (hv, &hinfo))
		return (false);
	
	if (hinfo == nil)
		return (false);

	w = (**hinfo).macwindow;
	
	if (!isfilewindow (w))
		return (false);
	
	if (!windowgetpath (w, bsname)) {
		
		windowgetfspec (w, &fs);
		
		copystring (fsname (&fs), bsname);
		
		//shellgetwindowtitle (hinfo, bsname);
		}
	
	setexternalvalue ((Handle) hv, &val);
	
	return (hashtableassign (filewindowtable, bsname, val));
	} /*langexternalregisterwindow*/


boolean langexternalunregisterwindow (hdlwindowinfo hinfo) {
	
	/*
	5.1.5b10 dmb: even if windowgetpath fails, the fspec has the (unsaved) default name
	*/
	
	tyfilespec fs;
	bigstring bsname;
	WindowPtr w;
	boolean fl;
	
	w = (**hinfo).macwindow;
	
	if (!isfilewindow (w))
		return (false);
	
	if (!windowgetpath (w, bsname)) {
		
		windowgetfspec (w, &fs);
		
		copystring (fsname (&fs), bsname);
		
		//shellgetwindowtitle (hinfo, bsname);
		}
	
	pushhashtable (filewindowtable);
	
	fl = hashdelete (bsname, false, false);
	
	pophashtable ();
	
	return (fl);
	} /*langexternalunregisterwindow*/


static boolean closeregisteredwindowvisit (hdlhashnode hnode, ptrvoid refcon) {
	
	boolean flconfirm = (refcon != nil);
	hdlwindowinfo hinfo;
	
	if (langexternalwindowopen ((**hnode).val, &hinfo))
		return (shellclose ((**hinfo).macwindow, flconfirm));
	
	return (true);
	} /*closeregisteredwindowvisit*/


boolean langexternalcloseregisteredwindows (boolean flconfirm) {
	
	/*
	5.0b12 dmb: the odb is about to close. All guest objects must
	be closed, maybe with confirmation
	*/
	
	return (hashtablevisit (filewindowtable, &closeregisteredwindowvisit, (ptrvoid) ((long) flconfirm)));
	} /*langexternalcloseregisteredwindows*/


boolean langexternalisdirty (hdlexternalhandle h) {
	
	register hdlexternalvariable hv = (hdlexternalvariable) h;
	
	switch ((**hv).id) {
		
		case idoutlineprocessor: case idscriptprocessor:
			return (opverbisdirty (hv));
		
		case idwordprocessor:
			return (wpverbisdirty (hv));
		
		case idmenuprocessor:
			return (menuverbisdirty (hv));
			
		case idpictprocessor:
			return (pictverbisdirty (hv));
		
		case idtableprocessor:
			return (tableverbisdirty (hv));
		
		#ifdef fliowa
		
		case idcardprocessor:
			return (cardverbisdirty (hv));
		
		#endif
		
		default:
			return (false);
		} /*switch*/
	} /*langexternalisdirty*/


boolean langexternalsetdirty (hdlexternalhandle h, boolean fldirty) {
	
	/*
	4/14/92 dmb: new routine.  to encapsulate as much logic as possible, we 
	take care of windowinfo here, and only call specific routine if a change 
	in state is occuring.  this prevents each external from having to avoid 
	loading an object into memory when cleaning it, for instance.
	
	9/4/92 dmb: return false if it's a new object -- in memory, with no old dbaddress
	
	5.0d3 dmb: allow cleaning of newly-created objects, in case they're file objects.
	it really shouldn't hurt if they're odb objects either; they just won't be saved.
	*/
	
	register hdlexternalvariable hv = (hdlexternalvariable) h;
	register boolean fl;
	boolean flwindowopen;
	hdlwindowinfo hinfo;
	
	if (fldirty == langexternalisdirty (hv))
		return (true);
	
	/*
	if ((**hv).flinmemory && ((**hv).oldaddress == nildbaddress)) // can't clean newly-created object
		return (false);
	*/
	
	flwindowopen = langexternalvariablewindowopen (hv, &hinfo) && (hinfo != nil);
	
	switch ((**hv).id) {
		
		case idoutlineprocessor: case idscriptprocessor:
			fl = opverbsetdirty (hv, fldirty);
			
			break;
		
		case idwordprocessor:
			fl = wpverbsetdirty (hv, fldirty);
			
			break;
		
		case idmenuprocessor:
			fl = menuverbsetdirty (hv, fldirty);
			
			break;
			
		case idpictprocessor:
			fl = pictverbsetdirty (hv, fldirty);
			
			break;
		
		case idtableprocessor:
			fl = tableverbsetdirty (hv, fldirty);
			
			break;
		
		#ifdef fliowa
		
		case idcardprocessor:
			fl = cardverbsetdirty (hv, fldirty);
			
			break;
		
		#endif
		
		default:
			return (false);
		} /*switch*/
	
	if (!fl)
		return (false);
	
	if (flwindowopen)
		shellsetwindowchanges (hinfo, fldirty);
	
	return (true);
	} /*langexternalsetdirty*/


boolean langexternalpack (hdlexternalhandle h, Handle *hpacked, boolean *flnewdbaddress) {
	
	tydiskexternalhandle rec;
	register hdlexternalvariable hv = (hdlexternalvariable) h;
	
	rollbeachball ();
	
	/*
	clearbytes (&rec, sizeof (rec));
	
	rec.flpathlink = (**h).flpathlink;
	*/
	
	rec.versionnumber = conditionalshortswap (externaldiskversionnumber);
	
	rec.id = (byte) (**hv).id;
	
	if (!newfilledhandle (&rec, sizeof (rec), hpacked))
		return (false);
		
	switch ((**hv).id) {
		
		case idoutlineprocessor: case idscriptprocessor:
			return (opverbpack (hv, hpacked, flnewdbaddress));
		
		case idwordprocessor:
			return (wpverbpack (hv, hpacked, flnewdbaddress));
		
		case idtableprocessor:
			return (tableverbpack (hv, hpacked, flnewdbaddress));
			
		case idmenuprocessor:
			return (menuverbpack (hv, hpacked, flnewdbaddress));
		
		case idpictprocessor:
			return (pictverbpack (hv, hpacked, flnewdbaddress));
		
		#ifdef fliowa
		
		case idcardprocessor:
			return (cardverbpack (hv, hpacked));
		
		#endif
		
		default:
			return (false);
		} /*switch*/
	} /*langexternalpack*/
	
	
boolean langexternalunpack (Handle hpacked, hdlexternalhandle *h) {
	
	hdlexternalvariable hdata;
	long ixload = 0;
	tydiskexternalhandle rec;
	tyexternalid id;
	
	*h = nil;
	
	if (hpacked == nil)
		return (false);
	
	assert (sizeof (tyexternalvariable) == 16L);
	
	rollbeachball ();
	
	if (!loadfromhandle (hpacked, &ixload, sizeof (rec), &rec))
		goto cantunpack;
	
	disktomemshort (rec.versionnumber);
//	disktomemshort (rec.id);

	if (rec.versionnumber != 1)
		goto cantunpack;

	id = (tyexternalid) rec.id;


	switch (id) {
		
		case idoutlineprocessor:
			if (!opverbunpack (hpacked, &ixload, &hdata))
				goto error;
				
			break;
		
		case idscriptprocessor:
			if (!opverbscriptunpack (hpacked, &ixload, &hdata))
				goto error;
				
			break;
			
		case idwordprocessor:
			if (!wpverbunpack (hpacked, &ixload, &hdata))
				goto error;
				
			break;
		
		case idtableprocessor:
			if (!tableverbunpack (hpacked, &ixload, &hdata, false))
				goto error;
				
			break;
		
		case idmenuprocessor:
			if (!menuverbunpack (hpacked, &ixload, &hdata))
				goto error;
			
			break;
			
		case idpictprocessor:
			if (!pictverbunpack (hpacked, &ixload, &hdata))
				goto error;
			
			break;
		
		#ifdef fliowa
		
		case idcardprocessor:
			if (!cardverbunpack (hpacked, &ixload, &hdata))
				goto error;
			
			break;
		
		#endif
		
		default:
			goto cantunpack;
		} /*switch*/
	
	(**hdata).id = id;
	
	/*(**hdata).flpathlink = rec.pathlink;*/
	
	*h = hdata; /*return a handle to the newly allocated external record*/
	
	return (true);
	
	cantunpack:
		
		langerror (cantunpackthisexternalerror);
	
	error:
	
		return (false);
	} /*langexternalunpack*/
	
	
boolean langexternalmemorypack (hdlexternalhandle h, Handle *hpacked, hdlhashnode hnode) {
	
	/*
	a special call for the pack and unpack verbs.  we insist that packing result in
	a handle of contiguous bytes that represents the external variable.  this is
	different from the original packing method which results in a dbaddress being
	added to the end of the packed handle.  we need the bytes!
	*/
	
	tydiskexternalhandle rec;
	register hdlexternalvariable hv = (hdlexternalvariable) h;
	
//	rollbeachball ();
	
	rec.versionnumber = conditionalshortswap (externaldiskversionnumber);
	
	rec.id = (byte) (**hv).id;
	
	if (!newfilledhandle (&rec, sizeof (rec), hpacked))
		return (false);
		
	switch ((**hv).id) {
		
		case idoutlineprocessor: case idscriptprocessor:
			return (opverbmemorypack (hv, hpacked));
		
		case idwordprocessor:
			return (wpverbmemorypack (hv, hpacked));
		
		case idtableprocessor:
			return (tableverbmemorypack (hv, hpacked, hnode));
		
		case idmenuprocessor:
			return (menuverbmemorypack (hv, hpacked));
		
		case idpictprocessor:
			return (pictverbmemorypack (hv, hpacked));
		
		#ifdef fliowa
		
		case idcardprocessor:
			return (cardverbmemorypack (hv, hpacked));
		
		#endif
		
		default:
			langerror (cantpackthisexternalerror);
			
			return (false);
		} /*switch*/
	} /*langexternalmemorypack*/


boolean langexternalmemoryunpack (Handle hpacked, hdlexternalhandle *h) {
	
	/*
	see comment at head of langexternalmemorypack.
	*/
	
	hdlexternalvariable hdata;
	long ixload = 0;
	tydiskexternalhandle rec;
	tyexternalid id;
	
	*h = nil;
	
	if (hpacked == nil)
		return (false);
	
//	rollbeachball ();
	
	if (!loadfromhandle (hpacked, &ixload, sizeof (rec), &rec))
		goto cantunpack;
		
#if 0 // def MACVERSION
	if (rec.versionnumber == 1) {
		tyOLD42diskexternalhandle oldrec;

		ixload = 0;
		if (!loadfromhandle (hpacked, &ixload, sizeof (oldrec), &oldrec))
			goto cantunpack;

		id = oldrec.id;

		goto version1ok;
		}
#endif

	disktomemshort (rec.versionnumber);
//	disktomemshort (rec.id);

	if (rec.versionnumber != externaldiskversionnumber)
		goto cantunpack;

	id = (tyexternalid) rec.id;


// version1ok:	
	
	switch (id) {
		
		case idoutlineprocessor:
			if (!opverbmemoryunpack (hpacked, &ixload, &hdata))
				goto error;
			
			break;
		
		case idscriptprocessor:
			if (!opverbscriptmemoryunpack (hpacked, &ixload, &hdata))
				goto error;
			
			break;
		
		case idwordprocessor:
			if (!wpverbmemoryunpack (hpacked, &ixload, &hdata))
				goto error;
			
			break;
		
		case idtableprocessor:
			if (!tableverbmemoryunpack (hpacked, &ixload, &hdata, false))
				goto error;
			
			break;
		
		case idmenuprocessor:
			if (!menuverbmemoryunpack (hpacked, &ixload, &hdata))
				goto error;
			
			break;
		
		case idpictprocessor:
			if (!pictverbmemoryunpack (hpacked, &ixload, &hdata))
				goto error;
			
			break;
		
		#ifdef fliowa
		
		case idcardprocessor:
			if (!cardverbmemoryunpack (hpacked, &ixload, &hdata))
				goto error;
			
			break;
		
		#endif
		
		default:
			goto cantunpack;
		} /*switch*/
	
	(**hdata).id = id;
	
	*h = hdata; /*return a handle to the newly allocated external record*/
	
	return (true);
	
	cantunpack:
	
		langerror (cantunpackthisexternalerror);
	
	error:
	
		return (false);
	} /*langexternalmemoryunpack*/


boolean langexternalcopyvalue (const tyvaluerecord *v1, tyvaluerecord *v2) {

	/*
	5.0.2b12 dmb: new routine
	*/
	
	hdlexternalhandle h = (hdlexternalhandle) (*v1).data.externalvalue;
	Handle x;
	boolean fl;
	
	switch ((**h).id) {
		
		case idoutlineprocessor: 
		case idscriptprocessor:
			if (!opverbcopyvalue (h, &h))
				return (false);
			
			setexternalvalue ((Handle) h, v2);
			
			return (true);
			
		default:
			if (!langpackvalue (*v1, &x, HNoNode))
				return (false);
			
			fl = langunpackvalue (x, v2);
			
			disposehandle (x);
			
			return (fl);
		}
	} /*langexternalcopyvalue*/


/*
boolean langexternalcoercetostring (tyvaluerecord *v) {
	
	/%
	assume the valuerecord is of type externalvaluetype.
	
	if we return true, the valuerecord is of type string.
	%/
	
	register hdlexternalhandle h = (hdlexternalhandle) (*v).data.externalvalue;
	
	switch ((**h).id) {
		
		case idheadrecord: {
			bigstring bs;
			
			opverbgetheadstring (h, bs);
			
			return (setstringvalue (bs, v));
			}
			
		default:
			langerror (cantcoercetostringerror);
			
			return (false);
		
		} /%switch%/
		
	return (false);
	} /%langexternalcoercetostring%/
*/


boolean langexternalgetowningwindow (hdlwindowinfo *hparent) {
	
	/*
	chase through the current symbol table chain until you find one that
	has a non-nil owningwindow field, and return that guy.
	
	if you reach the end of the chain, and all are nil, return false.
	
	an external guy can use this when he wants to zoom a window to edit
	something that came out of a symbol table and wants to know who to
	link the window to, child/parent-wise.
	
	10/22/90 dmb: new implementation.  it turns out that all table windows are 
	owned by the root, so there didn't need to be an owningwindow field in 
	each hash table.  to achieve the more specific zooming that was intended, 
	we traverse the hash table chain calling tablefinddatawindow, which uses the 
	hashtableformats field to find the displaying window.  if no window is found 
	(which is possible now that the root table isn't always displayed), the 
	current root window info is returned
	
	1/8/91 dmb: can no longer assume that currenthashtable isn't nil
	
	1/14/91 dmb: ignore most of above.  we actually want all windows containing 
	objects in the database to be children of the home window, not of each other 
	hierarchically.  the only exception is the menubar script window, which 
	doesn't use this routine.
	
	6.17.97 dmb: new implementation again. the root window can be invisible, and 
	the front window may be disk-based, and not owned by the root. so we must 
	find the window that owns the root. we must find cancoondata, not roottable. 
	the latter may not be open in a window
	*/
	
	/*
	register hdlhashtable nomad = currenthashtable;
	
	while (nomad != nil) {
		
		if (tablefinddatawindow (nomad, hparent))
			return (true);
		
		nomad = (**nomad).prevhashtable;
		} /%while%/
	
	/%ran out of tables to look at%/
	*/
	
	if (ccfindrootwindow (hparent))
		return (true);
	
	/*
	if (shellwindow != nil)
		return (getrootwindow (shellwindow, hparent));
	
	if (frontrootwindow (hparent))
		return (true);
	*/
	
	*hparent = nil;
	
	return (false);
	} /*langexternalgetowningwindow*/


void langexternalquotename (bigstring bsname) {
	
	/*
	5.0a16 dmb: deparse the string, and always use normal quotes
	
	5.1.3 dmb: deparse for normal quotes (which we're using), not curly quotes
	*/

	langdeparsestring (bsname, chdoublequote); /*add needed escape sequences*/

	insertchar ('"', bsname);
	
	pushchar ('"', bsname);
	} /*langexternalquotename*/


void langexternalbracketname (bigstring bsname) {
	
	/*
	5.0a16 dmb: this now takes care of deparsing, so caller shouldn't
	*/

	if (langisidentifier (bsname)) /*doesn't need brackets*/
		return;
	
	langexternalquotename (bsname);
	
	insertchar ('[', bsname);
	
	pushchar (']', bsname);
	} /*langexternalbracketname*/

long fullpathloopcount = 0;


static boolean istableintable (hdlhashtable hparent, hdlhashtable hchild, hdlhashnode * hnode) {
	hdlhashnode x;
	hdlexternalvariable hv;

	*hnode = nil;

	if (hparent == nil)
		return (false);  //defensive driving

	x = (**hparent).hfirstsort;

	while (x != nil) {
		if ((**x).val.valuetype == externalvaluetype) {
			
			hv = (hdlexternalvariable) (**x).val.data.externalvalue;

			if ((**hv).id == idtableprocessor) {

				if ((hdlhashtable)(**hv).variabledata == hchild) {
					*hnode = x;
					return (true);
					}
				}
			}

		x = (**x).sortedlink;
		}

	return (false);
	} /* istableintable */


static boolean fullpathsearch (hdlhashtable intable, hdlhashtable fortable, bigstring forname, boolean flonlyinmemory, boolean flquote, boolean flincludeself, bigstring bspath, hdlwindowinfo *hroot) {
	
	/*
	1/28/91 dmb: added flquote parameter

	5.0a18 dmb: added hroot parameter, for guest databases.
	*/
	
	register hdlhashtable ht = intable;
	register boolean fltempload;
	hdlhashnode x;
	register hdlexternalvariable hv = nil;
	bigstring bs;
	hdlwindowinfo hinfo;
	extern boolean tablesetdebugglobals (hdlhashtable, hdlhashnode);
	hdlhashtable pht;
	
	pht = nil;

	if (ht == nil) //defensive driving
		return (false);
	
	if (ht == fortable) { /* we are at the correct table */
		if (isemptystring (forname)) { /*asking for no specific name*/

			setemptystring (bspath);
			
			return (true);
			}

		if (hashtablesymbolexists (ht, forname)) { //Make sure the symbol exists

			copystring (forname, bspath);
					
			if (flquote)
				langexternalbracketname (bspath);
					
			return (true);
			}

		return (false);
		}


	if ((**fortable).parenthashtable != nil) {

		pht = (**fortable).parenthashtable;

		if ((**fortable).thistableshashnode != nil) {

			hv = (hdlexternalvariable)(**((**fortable).thistableshashnode)).val.data.externalvalue;

			if ((hdlhashtable)(**hv).variabledata != fortable) {  /*is this a bad node record*/

				(**fortable).thistableshashnode = nil; /*clear it and try to redo it below*/
				}
			}

		if ((**fortable).thistableshashnode == nil) { /*go fill this in now*/

			if (istableintable (pht, fortable, &x))
				(**fortable).thistableshashnode = x;

			else {
				/*This is bad but be defensive... The parent pointer is wrong */
				(**fortable).parenthashtable = nil;
				pht = nil;
				}
			}
		}


	if (pht != nil) { /*Then lets follow the nodes up...*/

		if (fullpathsearch (ht, pht, "", flonlyinmemory, flquote, flincludeself, bs, hroot)) {

			if ((ht != pht) || flincludeself) {

				gethashkey ((**fortable).thistableshashnode, bspath);
					
				if (flquote)
					langexternalbracketname (bspath);
					
				if (stringlength (bs) > 0) {
						
					/*bspath = bs + "." + bspath*/

					pushchar ('.', bs);
						
					pushstring (bspath, bs);

					copystring (bs, bspath);
					}
				}
			else
				setemptystring (bspath);
				


			if (! isemptystring (forname)) { /*asking for specific name*/

				if (hashtablesymbolexists (fortable, forname)) { //Make sure the symbol exists

					copystring (forname, bs);
					
					if (flquote)
						langexternalbracketname (bs);

					if (stringlength (bspath) > 0)
						pushchar ('.', bspath);

					pushstring (bs, bspath);
					}

//				return (false);
				}

			if (hroot) {
				x = (**fortable).thistableshashnode;

				hv = (hdlexternalvariable) (**x).val.data.externalvalue;

				if (langexternalvariablewindowopen (hv, &hinfo) && (hinfo != nil))
					getrootwindow ((**hinfo).macwindow, hroot);
				}

			return (true);
			}

		return (false);	
		}


	x = (**ht).hfirstsort;
	
	while (x != nil) { /*chain through the hash list*/
		
		fltempload = false;

		++fullpathloopcount;
				
		if ((**x).val.valuetype != externalvaluetype) 
			goto nextx;
			
		hv = (hdlexternalvariable) (**x).val.data.externalvalue;

		if ((**hv).id != idtableprocessor)
			goto nextx;
		
		if (!(**hv).flinmemory) {
		
			if (flonlyinmemory)	/*can't find it if it isn't in memory*/
				goto nextx;
			
			if (!tableverbinmemory (hv, x))
				return (false);
				
			fltempload = true;
			}
		
/////////			assert (tablesetdebugglobals (ht, x)); /*set debug globals*/
		
		if (fullpathsearch ((hdlhashtable) (**hv).variabledata, fortable, forname, flonlyinmemory, flquote, true, bs, hroot)) {
			hdlhashtable cht;
			hdlhashnode nn;

			cht = ((hdlhashtable) (**hv).variabledata);

			/*we do not have an if statement here since the straight assignment is faster*/
			(**cht).parenthashtable = ht;  /*set the parent pointer for next time*/

			if ((**cht).thistableshashnode == nil)
				if (istableintable (ht, cht, &nn))
					(**cht).thistableshashnode = nn;

			if (flincludeself) {
				
				gethashkey (x, bspath);
				
				if (flquote)
					langexternalbracketname (bspath);
				
				if (stringlength (bs) > 0) {
					
					pushchar ('.', bspath);
					
					pushstring (bs, bspath);
					}
				}
			else
				copystring (bs, bspath);
			
			if (hroot && (*hroot == nil))
				if (langexternalvariablewindowopen (hv, &hinfo) && (hinfo != nil))
					getrootwindow ((**hinfo).macwindow, hroot);
			
			return (true); /*unwind recursion*/
			}
			
		nextx:
		
		if (fltempload)
			tableverbunload (hv);
		
		x = (**x).sortedlink; //    hashlink; /*advance to next node in chain*/
		} /*while*/
		
	return (false);
	} /*fullpathsearch*/


long tableCount = 0;
long maxBucketCount;
long maxdepth;
long currentdepth;
long xxgoodnodelinks;
long xxbadnodelinks;

typedef struct {
	long cttotal;
	long ctloaded;
	} tyhashstatselement;

tyhashstatselement hashstats[ctvaluetypes];


boolean fullpathstats (hdlhashtable intable, boolean flfirst) {
	
	/*
	1/28/91 dmb: added flquote parameter

	5.0a18 dmb: added hroot parameter, for guest databases.
	*/
	
	register hdlhashtable ht = intable;
	register hdlhashnode x;
	//register hdlexternalhandle xval;
	//register hdlhashtable xtable;
	register long i;
	tyvaluerecord val;
	register hdlexternalvariable hv = nil;
	extern boolean tablesetdebugglobals (hdlhashtable, hdlhashnode);
	long bucketCount;
	register tyvaluetype valtype;
	boolean flloaded;

	
	if (flfirst) {
		tableCount = 0;
		maxBucketCount = 0;
		maxdepth = 0;
		currentdepth = 1;
		xxgoodnodelinks = 0;
		xxbadnodelinks = 0;
		clearbytes (hashstats, sizeof (hashstats));
		}
	else
	{
/*		x = (hdlhashnode) (**ht).hashtablerefcon;
		xval = (hdlexternalvariable) ((**x).val.data.externalvalue);
		xtable = (hdlhashtable) ((**xval).variabledata);

		if (xtable != ht)
			++xxbadnodelinks;
		else
			++xxgoodnodelinks;
*/	}
	
	++tableCount;

	if (ht == nil) //defensive driving
		return (true);
	
	for (i = 0; i < ctbuckets; i++) {
		
		x = (**ht).hashbucket [i];
		
		bucketCount = 0;

		while (x != nil) { /*chain through the hash list*/
						
			++bucketCount;

			val = (**x).val;

			valtype = val.valuetype;

			flloaded = !val.fldiskval;

			if (valtype == externalvaluetype) {

				valtype = (tyvaluetype) (outlinevaluetype + langexternalgettype (val));

				hv = (hdlexternalvariable) val.data.externalvalue;

				flloaded = flloaded && (**hv).flinmemory;
				}

			++hashstats[valtype].cttotal;

			if (flloaded)
				++hashstats[valtype].ctloaded;

			if (valtype == tablevaluetype && flloaded) {

				++currentdepth;

				if (maxdepth < currentdepth)
					maxdepth = currentdepth;

				fullpathstats ((hdlhashtable) (**hv).variabledata, false);

				--currentdepth;
				}
/*
			switch (langexternalgettypeid (val)) {

				case tablevaluetype:

					hv = (hdlexternalvariable) val.data.externalvalue;
			
					if ((**hv).flinmemory) {
					
						++currentdepth;

						if (maxdepth < currentdepth)
							maxdepth = currentdepth;

						fullpathstats ((hdlhashtable) (**hv).variabledata, false);

						--currentdepth;
						}
					
					break;
					
				case binaryvaluetype:

					++ctbinaryobjects;

					if (!val.fldiskval) { //it's actually loaded
					
						long ctbytes = gethandlesize (val.data.binaryvalue);

						++ctloadedbinaryobjects;

						sizebinaryobjects += ctbytes;

						if (sizelargestbinaryobject < ctbytes)
							sizelargestbinaryobject = ctbytes;

						if (ctbytes <= 1000L)
							++ctbinaryobjects1;
						else if (ctbytes <= 50000L)
							++ctbinaryobjects2;
						else if (ctbytes <= 100000L)
							++ctbinaryobjects3;
						else
							++ctbinaryobjects4;
						}

					break;
					
				}/%switch*/
							
			x = (**x).hashlink; /*advance to next node in chain*/
			} /*while*/

		if (bucketCount > maxBucketCount)
			maxBucketCount = bucketCount;

		} /*for*/
		
	return (true);
	} /*fullpathstats*/

#define STR_Comma_Space "\x02" ", "
#define STR_Colon_Space "\x02" ": "

boolean hashstatsverb (tyvaluerecord *v) {

	handlestream s;
	long ix;
	bigstring bs;

	fullpathstats (roottable, true);

	openhandlestream (nil, &s);

	if (!writehandlestreamlong (&s, maxBucketCount))
		goto error;

	if (!writehandlestreamstring (&s, STR_Comma_Space))
		goto error;

	if (!writehandlestreamlong (&s, tableCount))
		goto error;

	if (!writehandlestreamstring (&s, STR_Comma_Space))
		goto error;

	if (!writehandlestreamlong (&s, maxdepth))
		goto error;

	if (!writehandlestreamstring (&s, STR_Comma_Space))
		goto error;

	if (!writehandlestreamlong (&s, xxgoodnodelinks))
		goto error;

	if (!writehandlestreamstring (&s, STR_Comma_Space))
		goto error;

	if (!writehandlestreamlong (&s, xxbadnodelinks))
		goto error;

	for (ix = 1; ix < ctvaluetypes; ix++) {

		if (!writehandlestreamchar (&s, '\r'))
			goto error;

		if (!writehandlestreamlong (&s, hashstats[ix].ctloaded))
			goto error;

		if (!writehandlestreamchar (&s, '\t'))
			goto error;

		if (!writehandlestreamlong (&s, hashstats[ix].cttotal))
			goto error;

		if (!writehandlestreamchar (&s, '\t'))
			goto error;

		if (!langgettypestring (ix, bs))
			goto error;

		if (!writehandlestreamstring (&s, bs))
			goto error;
		}/*for*/

	return (setheapvalue (closehandlestream (&s), stringvaluetype, v));

error:

	disposehandlestream (&s);

	return (false);
	}/*hashstatsverb*/



static boolean getfullpath (hdlhashtable htable, bigstring bsname, boolean flquote, bigstring bspath, hdlwindowinfo *hroot) {
	
	/*
	do a traversal of the root symbol table to get the full path for the indicated
	variable.  a full path is something like "agents.secret.007".
	
	the variable is specified by [htable, bsname].  the path is returned in bspath.
	
	if bsname is empty, just return a path to the table.
	
	first we do a search restricted to only those tables that are in memory.  if 
	it's found we get off with a very cheap traversal.  otherwise we go slowly
	through the whole structure loading everything in from disk to do the search.
	
	if htable is the root table, and our initial search doesn't find bsname, 
	the only possibility is that bsname is the name of a special table.  if so, 
	it's name is it's path; otherwise, return false -- no need to look further.
	
	8/19/91 dmb: if the indicated table is a local table, and it's not found 
	in the object db (not being debugged), just return the name
	
	7.22.97 dmb: do the same for the root table (for standalones)
	
	5.0d20 dmb: search the files table first, so we get short paths for 
	items within standalone tables. shoundn't be too costly, since the table 
	is small and we aren't called that often

	5.0a18 dmb: added hroot stuff. kind of ugly. the goal: notice when a path
	is below a file-based object, and set it to that parent object, otherwise
	the active root.
	
	5.0.2b21 dmb: don't make the path to the file windows table itself empty.

	5.1.4 dmb: never call fullpathsearch with flonlyinmemory false. an in-memory table
	can never be contained by a table that's still on disk
	
	5.1.5 dmb: added flincludeself parameter to fullpathsearch. We pass false when 
	searching the file window table if we're not quoting the path, to avoid file names
	in window titles.
	*/
	
	boolean fl = true;

	if (hroot)
		*hroot = nil;

	if ((htable == roottable) && isemptystring (bsname)) {
		
		copystring (nameroottable, bspath);
		
		goto exit;
		}
	
	if (htable && !(**htable).fllocaltable) {

		if ((htable == filewindowtable) || (tablegetdatabase (htable) != tablegetdatabase (roottable)))
			if (fullpathsearch (filewindowtable, htable, bsname, true, flquote, flquote, bspath, hroot))
				goto exit;
		
		if (fullpathsearch (roottable, htable, bsname, true, flquote, true, bspath, hroot))
			goto exit;
		}
	
	if ((htable == roottable) || (htable == nil)) {
		
		/**
		1/28/91 dmb: this check seems archaic; doesn't emptystring check in
		fullpathsearch cover this?
		*/
		
		 if (langgetspecialtable (bsname, &htable)) {
			
			copystring (bsname, bspath);

			goto exit;
			}
		}
	else {
		
		if ((**htable).fllocaltable) { /*8/19/91 dmb*/
			
			hdlhashtable ht;
			
			if ((**htable).prevhashtable == nil) { // not in the local chain
			
				for (ht = currenthashtable; ht != nil; ht = (**ht).prevhashtable) {
			
					if (fullpathsearch (ht, htable, bsname, true, flquote, true, bspath, hroot))
						goto exit;
					}
				}
			
			if (!isemptystring (bsname)) {
				
				copystring (bsname, bspath);
				
				goto exit;
				}
			}
		
		// now try non-inmemory search
		//if (fullpathsearch (roottable, htable, bsname, false, flquote, bspath, hroot))
		//	goto exit;
		}
	
	fl = false;

	exit:
	
	if (!fl) {

		langparamerror (nopatherror, bsname);
		
		return (false);
		}
	
	if (hroot && (*hroot == nil))
		ccfindrootwindow (hroot);

	return (true);
	} /*getfullpath*/


boolean langexternalgetfullpath (hdlhashtable htable, bigstring bsname, bigstring bspath, hdlwindowinfo *hroot) {
	
	/*
	normal version of getfullpath, with concatenates each name in the path, 
	creating a string suitable for display purposes.

	added hroot parameter. we set it to the windowinfo of the root containing
	the node
	*/
	
	return (getfullpath (htable, bsname, false, bspath, hroot));
	} /*langexternalgetfullpath*/


boolean langexternalgetquotedpath (hdlhashtable htable, bigstring bsname, bigstring bspath) {
	
	/*
	special version of getfullpath which quotes each name in the path.
	
	this is used for interactive value and type assignment in tables, 
	where bspath is used in expressions rather than for display purposes.
	*/
	
	return (getfullpath (htable, bsname, true, bspath, nil));
	} /*langexternalgetquotedpath*/


boolean langexternalgetexternalparam (hdltreenode hfirst, short pnum, short *id, hdlexternalvariable *hv) {
	
	/*
	this code was factored from each EFP's getxxxparam routine.  it ensures that 
	the parameter specified by [hfirst, pnum] corresponds to an external value in 
	a table.  if so, the EFP id and the variable itself are returned in [id, hv].
	
	..in addition, the full path to the variable is calculated and returned in bs.
	*/
	
	tyvaluerecord val;
	register hdlexternalhandle h;
	hdlhashtable htable;
	bigstring bs;
	hdlhashnode hnode;
	
	*hv = nil; /*default*/
	
	if (!getvarparam (hfirst, pnum, &htable, bs)) /*name of variable*/
		return (false);
	
	if (!langsymbolreference (htable, bs, &val, &hnode))
		return (false);
	
	if (val.valuetype != externalvaluetype)
		return (false);
	
	//if (langexternalgetfullpath (htable, bs, bspath, nil))
	//	copystring (bspath, bs);
	
	h = (hdlexternalhandle) val.data.externalvalue;
	
	*id = (**h).id;
	
	*hv = (hdlexternalvariable) h;
	
	return (true);
	} /*langexternalgetexternalparam*/


static boolean langexternaledit (hdlexternalvariable hv, hdlwindowinfo hparent, tyfilespec *fs, bigstring bstitle, const Rect *rzoom) {
	
	/*
	6.16.97 dmb: the guts of langexternalzoom[from|filewindow]
	*/
	
	boolean fl = false;
	
	switch ((**hv).id) {
		
		case idtableprocessor:
			fl = tableedit (hv, hparent, fs, bstitle, rzoom);
			
			break;
		
		case idoutlineprocessor: 
		case idscriptprocessor:
			fl = opedit (hv, hparent, fs, bstitle, rzoom);
			
			break;
		
		case idwordprocessor:
			fl = wpedit (hv, hparent, fs, bstitle, rzoom);
			
			break;
			
		case idmenuprocessor:
			fl = menuedit (hv, hparent, fs, bstitle, rzoom);
			
			break;
			
		case idpictprocessor:
			fl = pictedit (hv, hparent, fs, bstitle, rzoom);
			
			break;
		
		#ifdef fliowa
		
		case idcardprocessor:
			fl = cardedit (hv, hparent, fs, bstitle, rzoom);
			
			break;
		
		#endif
		
		} /*switch*/
	
	return (fl);
	} /*langexternaledit*/

	
boolean langexternalsetreadonly (hdlexternalvariable hv, boolean flreadonly) {
	
	/*
	7.0b6 PBS: set the read-only flag for a window.
	Handles various outline types. If it's a wp-text window, it's an error.
	*/
	
	hdlhashtable ht;
	hdltableformats hf;
	hdloutlinerecord ho;
	hdlmenurecord hm;
	boolean fl = true;
	
	switch ((**hv).id) {
		
		case idtableprocessor:
		
			ht = (hdlhashtable) (**hv).variabledata;
			hf = (**ht).hashtableformats;
			
			(**(**hf).houtline).flreadonly = flreadonly;
			
			break;
			
		case idoutlineprocessor:
		case idscriptprocessor:
			
			ho = (hdloutlinerecord) (**hv).variabledata;
			
			(**ho).flreadonly = flreadonly;
			
			break;
			
		case idmenuprocessor:
				
			hm = (hdlmenurecord) (**hv).variabledata;
			
			(**(**hm).menuoutline).flreadonly = flreadonly;
			
			break;
		
		case idwordprocessor:
			
			if (flreadonly) {
			
				fl = false;			
			
				langerrormessage ("\x4b""Can't open as read-only because wp-text windows don't support this feature.");
				} /*if*/
				
			break;

		default:
		
			fl = false;
			
		} /*switch*/	

	return (fl);
	} /*langexternalsetreadonly*/


boolean langexternalgetreadonly (hdlexternalvariable hv) {
	
	/*
	7.0b6 PBS: Return the read-only flag for a window.
	Handles various outline types. If it's a wp-text window, it's never read-only.
	*/
	
	hdlhashtable ht;
	hdltableformats hf;
	hdloutlinerecord ho;
	hdlmenurecord hm;
	boolean flreadonly = true;
	
	switch ((**hv).id) {
		
		case idtableprocessor:
		
			ht = (hdlhashtable) (**hv).variabledata;
			hf = (**ht).hashtableformats;
			
			flreadonly = (**(**hf).houtline).flreadonly;
			
			break;
			
		case idoutlineprocessor:
		case idscriptprocessor:
			
			ho = (hdloutlinerecord) (**hv).variabledata;
			
			flreadonly = (**ho).flreadonly;
			
			break;
			
		case idmenuprocessor:
				
			hm = (hdlmenurecord) (**hv).variabledata;
			
			flreadonly = (**(**hm).menuoutline).flreadonly;
			
			break;
		
		default: /*wp-text*/
		
			flreadonly = false;
			
			break;			
		} /*switch*/	

	return (flreadonly);
	} /*langexternalgetreadonly*/


boolean langexternalzoomfrom (tyvaluerecord val, hdlhashtable htable, bigstring bsname, rectparam rzoom) {
	
	/*
	val represents an external variable, or it should.  the caller wants us to 
	zoom a window that displays the data linked into the external variable.
	
	10/3/90 DW: insert wire to editvalue.c.
	
	10/29/90 dmb: removed wire to editvalue.c (editing now done in tables)
	
	7/4/91 dmb: now take table parameter, locate the table if it nil, and get 
	the full path instead of having caller provide it.
	
	12/9/91 dmb: if variable is just being loaded into memory, force 
	table window to update row, if being displayed.
	*/
	
	register hdlexternalvariable hv;
	bigstring bstitle;
	boolean flwasinmemory;
	hdlwindowinfo hparent;
	
	if (val.valuetype != externalvaluetype)
		return (false);
	
	hv = (hdlexternalvariable) val.data.externalvalue;
	
	if (htable == nil) { /*no table provided; we'll search for it*/
		
		if (!langexternalfindvariable (hv, &htable, bsname))
			return (false);
		}
	
	if (!langexternalgetfullpath (htable, bsname, bstitle, &hparent))
		return (false);
	
	flwasinmemory = (boolean) (**hv).flinmemory;
	
	if (!langexternaledit (hv, hparent, nil, bstitle, rzoom))
		return (false);
	
	if (!flwasinmemory) /*make table update row*/
		tablewindowclosed (hv);
	
	return (true);
	} /*langexternalzoomfrom*/


boolean langexternalzoom (tyvaluerecord val, hdlhashtable htable, bigstring bsname) {
	
	/*
	7/4/91 dmb: we're now just a wrapper for langexternalzoomfrom, which 
	takes the zoom rect parameter.  a majority of callers just want the 
	default behavior
	*/
	
	Rect rzoom;
	
	rzoom.top = -1; /*use default*/
	
	return (langexternalzoomfrom (val, htable, bsname, &rzoom));
	} /*langexternalzoom*/


boolean langexternalzoomfilewindow (const tyvaluerecord *val, tyfilespec *fs, boolean flhidden) {
	
	/*
	create and open a new window containing the file-based external value in val.
	val should be in memory
	*/
	
	register hdlexternalvariable hv;
	bigstring bstitle;
	Rect rzoom;
	
	hv = (hdlexternalvariable) (*val).data.externalvalue;
	
	assert ((**hv).flinmemory);
	
	if (flhidden)
		rzoom.top = -2; //zoom hidden
	else
		rzoom.top = -1; //use default
	
	if (fs != nil)
		getfsfile (fs, bstitle); //5.0b9 dmb: new routine
	else
		getuntitledfilename (bstitle);
	
	if (!langexternaledit (hv, nil, fs, bstitle, &rzoom))
		return (false);
	
	langexternalregisterwindow (hv);
	
	return (true);
	} /*langexternalzoomfilewindow*/


boolean langexternalwindowopen (tyvaluerecord val, hdlwindowinfo *hinfo) {
	
	/*
	3/19/91 DW: add a parameter that returns a handle to the windowinfo record 
	if a window is open.  required modification to the routines in each of the
	external handlers.
	
	4/15/92 dmb: moved guts into langexternalvariablewindowopen

	5.0.1 dmb: for this higher-level funtion, don't return true if hinfo is nil
	*/
	
	if (val.valuetype != externalvaluetype) /*non-externals can't have open windows*/
		return (false);
	
	if (!langexternalvariablewindowopen ((hdlexternalvariable) val.data.externalvalue, hinfo))
		return (false);
	
	return (*hinfo != nil);
	} /*langexternalwindowopen*/


boolean langexternalwindowclosed (hdlexternalvariable hvariable) {
	
	/*
	the caller is telling us the the variable's window has just been closed
	*/
	
	return (tablewindowclosed (hvariable));
	} /*langexternalwindowclosed*/


boolean langexternaldisposevariable (hdlexternalvariable hvariable, boolean fldisk, boolean (*disposeroutine) (hdlexternalvariable, boolean)) {
	
	/*
	this code was factored from each EFP's xxxverbdispose routine.  most of the 
	routines called from langexternaldisposevalue below simple call this routine 
	with the address of a routine that knows how to dispose of the specific type.
	the entire variable is passed to the callback, rather than just the variabledata, 
	in case there are other flags or fields (like linkedcode in opverbs.c, or flpacked 
	in wpverbs.c) that are relevant to disposal
	
	6/10/92 dmb: inhibitclosedialogs when closing window during disposal
	
	6.17.97 dmb: with file windows, we can be called from their close routine. so we 
	must rely on langexternalwindowopen, not shellfinddatawindow
	
	5.0.1 dmb: test hinfo before using it to call shellclosewindow
	*/
	
	register hdlexternalvariable hv = hvariable;
	hdlwindowinfo hinfo;
	dbaddress adr;
	boolean flwindowopen;
	hdldatabaserecord savedatabasedata = databasedata;
	
	flwindowopen = langexternalvariablewindowopen (hv, &hinfo);
	
	if (hinfo != nil) {
		
		langexternalunregisterwindow (hinfo);
		
		if (flwindowopen) {
			
			flinhibitclosedialogs = true;
			
			shellclosewindow ((**hinfo).macwindow);
			
			flinhibitclosedialogs = false;
			}
		}
	
	databasedata = (**hv).hdatabase;

	if ((**hv).flinmemory) {
		
		(*disposeroutine) (hv, fldisk);

		adr = (**hv).oldaddress;
		}
	else
		adr = (dbaddress) (**hv).variabledata;
	
	if (fldisk)
		dbpushreleasestack (adr, (long) (outlinevaluetype + (**hv).id));
	
	databasedata = savedatabasedata;
	
	disposehandle ((Handle) hv);
	
	return (true);
	} /*langexternaldisposevariable*/


boolean langexternaldisposevalue (tyvaluerecord val, boolean fldisk) {
	
	register hdlexternalvariable hv;
	register boolean fl;
	
	if (val.valuetype != externalvaluetype)
		return (false);
	
	hv = (hdlexternalvariable) val.data.externalvalue; /*copy into register*/
	
	switch ((**hv).id) {
	
		case idtableprocessor:
			fl = tableverbdispose (hv, fldisk);
			
			break;
		
		case idoutlineprocessor: case idscriptprocessor:
			fl = opverbdispose (hv, fldisk);
			
			break;
		
		case idwordprocessor:
			fl = wpverbdispose (hv, fldisk);
			
			break;
		
		case idmenuprocessor:
			fl = menuverbdispose (hv, fldisk);
			
			break;
		
		case idpictprocessor:
			fl = pictverbdispose (hv, fldisk);
			
			break;
		
		#ifdef fliowa
		
		case idcardprocessor:
			fl = cardverbdispose (hv, fldisk);
			
			break;
		
		#endif
		
		default:
			fl = false;
		} /*switch*/
	
	return (fl);
	} /*langexternaldisposevalue*/


static boolean updateconfigsettings (tyvaluetype type, short configid) {
	
	/*
	5.0d14 dmb: update the speicified config record according the the 
	user's preference settings in the database
	
	5.0d17 dmb: push/pop root table for expanding dotparams
	*/
	
	bigstring bspref;
	byte bstype [16];
	bigstring bsfont;
	hdlhashtable htable;
	bigstring bs;
	tyvaluerecord val;
	short fontsize = 0;
	short ixconfig;
	hdlhashnode hnode;
	
	// build the path to the font pref
	getsystemtablescript (iduserfontprefscript, bspref);
	
	langgettypestring (type, bstype);
	
	setemptystring (bsfont);
	
	parsedialogstring (bspref, bstype, nil, nil, nil, bspref);
	
	disablelangerror ();
	
	pushhashtable (roottable);
	
	if (langexpandtodotparams (bspref, &htable, bs) && hashtablelookup (htable, bs, &val, &hnode)) {
		
		if (val.valuetype == stringvaluetype)
			pullstringvalue (&val, bsfont);
		}
	
	// build the path the the font size pref
	pushstring ("\x04Size", bspref);
	
	if (langexpandtodotparams (bspref, &htable, bs) && hashtablelookup (htable, bs, &val, &hnode)) {
		
		if (val.valuetype == longvaluetype)
			fontsize = (short) val.data.longvalue;
		else if (val.valuetype == intvaluetype)
			fontsize = val.data.intvalue;
		}
	
	pophashtable ();
	
	enablelangerror ();
	
	// modify the config according to what we found
	if (!shellfindcallbacks (configid, &ixconfig))
		return (false);
	
	if (!isemptystring (bsfont))
		fontgetnumber (bsfont, &globalsarray [ixconfig].config.defaultfont);
	
	if (fontsize > 0)
		globalsarray [ixconfig].config.defaultsize = fontsize;
	
	return (true);
	} /*updateconfigsettings*/


boolean langexternalgetconfig (tyvaluetype type, short configid, tyconfigrecord *pconfig) {
	
	/*
	5.0b9 dmb: expose functionality so user prefs hold in all contexts,
	not just creating new objects
	*/
	
	static boolean fltryingtoupdate = false; // ovoid reentrancy, which would be infinite
	
	if (!fltryingtoupdate && roottable != nil) {
		
		fltryingtoupdate = true;
		
		updateconfigsettings (type, configid);
		
		fltryingtoupdate = false;
		}

	return (shellgetconfig (configid, pconfig));
	} /*langexternalgetconfig*/


boolean langexternalnewvalue (tyexternalid id, Handle hdata, tyvaluerecord *val) {
	
	/*
	9/6/91 dmb: added hdata parameter for type-specific starter data
	
	5.0d14 dmb: use new updateconfigsettings so we always respect the user's 
	font/size preference settings.
	*/
	
	hdlexternalvariable hvariable;
	register boolean fl;
	
	switch (id) {
	
		case idtableprocessor:
			updateconfigsettings (tablevaluetype, idtableconfig);
			
			fl = tableverbnew (&hvariable);
			
			break;
		
		case idoutlineprocessor:
			updateconfigsettings (outlinevaluetype, idoutlineconfig);
			
			fl = opverbnew (idoutlineprocessor, hdata, &hvariable);
			
			break;
		
		case idscriptprocessor:
			updateconfigsettings (scriptvaluetype, idscriptconfig);
			
			fl = opverbnew (idscriptprocessor, hdata, &hvariable);
			
			break;
		
		case idwordprocessor:
			updateconfigsettings (wordvaluetype, idwpconfig);
			
			fl = wpverbnew (hdata, &hvariable);
			
			break;
		
		case idmenuprocessor:
			updateconfigsettings (menuvaluetype, idmenueditorconfig);
			
			fl = menuverbnew (hdata, &hvariable);
			
			break;
		
		case idpictprocessor:
			updateconfigsettings (pictvaluetype, idpictconfig);
			
			fl = pictverbnew (hdata, &hvariable);
			
			break;
		
		#ifdef fliowa
		
		case idcardprocessor:
			updateconfigsettings (cardvaluetype, idiowaconfig);
			
			fl = cardverbnew (hdata, &hvariable);
			
			break;
		
		#endif
		
		default:
			fl = false;
		} /*switch*/
	
	if (!fl)
		return (false);
	
	(**hvariable).id = id;
	
	setexternalvalue ((Handle) hvariable, val);
	
	return (true);
	} /*langexternalnewvalue*/


boolean langexternalvaltocode (tyvaluerecord val, hdltreenode *hcode) {
	
	register hdlexternalvariable hv;
	
	if (val.valuetype != externalvaluetype) 
		return (false);
	
	hv = (hdlexternalvariable) val.data.externalvalue;
	
	if ((**hv).id != idscriptprocessor) 
		return (false);
	
	opverbgetlinkedcode (hv, hcode);
	
	return (true); /*return true even if *hcode is nil*/
	} /*langexternalvaltocode*/


boolean langexternalgetvalsize (tyvaluerecord val, long *size) {
	
	register hdlexternalvariable hv;
	
	if (val.valuetype != externalvaluetype)
		return (false);
	
	hv = (hdlexternalvariable) val.data.externalvalue; /*copy into register*/
	
	switch ((**hv).id) {
	
		case idoutlineprocessor: case idscriptprocessor:
			return (opverbgetsize (hv, size));
		
		case idwordprocessor:
			return (wpverbgetsize (hv, size));
		
		case idtableprocessor:
			return (tableverbgetsize (hv, size));
		
		case idmenuprocessor:
			return (menuverbgetsize (hv, size));
		
		case idpictprocessor:
			return (pictverbgetsize (hv, size));
		
		#ifdef fliowa
		
		case idcardprocessor:
			return (cardverbgetsize (hv, size));
		
		#endif
		
		default:
			return (false);
		} /*switch*/
	} /*langexternalgetvalsize*/


boolean langnewexternalvariable (boolean flinmemory, long variabledata, hdlexternalvariable *h) {
	
	tyexternalvariable item;
	
	clearbytes (&item, sizeof (item));
	
	item.flinmemory = flinmemory;
	
	item.variabledata = variabledata;

	item.hdatabase = databasedata; // 5.0a18 dmb
	
	//item.hexternaltable = nil;
	//copystring (emptystring, item.bsexternalname);
	
	return (newfilledhandle (&item, sizeof (item), (Handle *) h));
	} /*langnewexternalvariable*/


static short getsortweight (tyexternalid type) {
	
	/*
	the lightest types float to the the top of a sorted list.
	*/
	
	register short w = 0;
	
	switch (type) {
	
		case idtableprocessor: 
			w = 0; break;
		
		case idscriptprocessor:
			w = 1; break;
		
		case idwordprocessor:
			w = 2; break;
		
		case idpictprocessor:
			w = 3; break;
		
		case idoutlineprocessor:
			w = 4; break;
		
		case idmenuprocessor:
			w = 5; break;
		
		case idheadrecord:
			w = 6; break;
		
		case idcardprocessor:
			w = 7; break;
		
		default:
			w = infinity; break;
		} /*switch*/
	
	return (w);
	} /*getsortweight*/


short langexternalcomparetypes (tyexternalid type1, tyexternalid type2) {
	
	/*
	support for the table editor's view-by-kind sort order.  if type1 is less than
	type2 return true.
	
	3/31/93 dmb: return signed value, not boolean
	*/
	
	register short w1, w2;
	
	w1 = getsortweight (type1);
	
	w2 = getsortweight (type2);
	
	return (sgn (w1 - w2));
	} /*langexternalcomparetypes*/


boolean langexternalsurfacekey (hdlexternalvariable hv) {
	
	/*
	5.0b18 dmb: for win, use cmd-backspace. can't get the mac combo
	*/

	if (keyboardstatus.chkb == chenter) {
		
		if (keyboardstatus.flcmdkey && keyboardstatus.flshiftkey) {
		
			tableclientsurface (hv);
			
			return (true);
			}
		}
	
	#ifdef WIN95VERSION
		if (keyboardstatus.chkb == chbackspace) {
			
			if (keyboardstatus.flshiftkey) {
			
				tableclientsurface (hv);
				
				return (true);
				}
			}
	#endif

	return (false); /*keystroke not handled*/
	} /*langexternalsurfacekey*/


boolean langexternalpacktotext (hdlexternalhandle h, Handle htext) {
	
	/*
	9/11/92 dmb: call rollbeachball
	*/
	
	register hdlexternalvariable hdata = (hdlexternalvariable) h;
	
	rollbeachball ();
	
	switch ((**hdata).id) {
		
		case idoutlineprocessor: case idscriptprocessor:
			return (opverbpacktotext (hdata, htext));
		
		case idwordprocessor:
			return (wpverbpacktotext (hdata, htext));
		
		case idtableprocessor:
			return (tableverbpacktotext (hdata, htext));
		
		case idmenuprocessor:
			return (menuverbpacktotext (hdata, htext));
		
		case idpictprocessor:
			return (pictverbpacktotext (hdata, htext));
		
		#ifdef fliowa
		
		case idcardprocessor:
			return (cardverbpacktotext (hdata, htext));
		
		#endif
		
		default:
			return (false);
		} /*switch*/
	} /*langexternalpacktotext*/


boolean langexternalsearch (tyvaluerecord val, boolean *flzoom) {
	
	/*
	search the valuerecord according to the current search parameters.  by 
	default, we also tell caller to zoom to the value if it contains a 
	match, but that can be overridden by each find routine if desired
	
	4.0.2b1 dmb: give the user a chance to abort by backgroundtasking.
	*/
	
	register hdlexternalvariable hv;
	boolean fl;
	
	if (val.valuetype != externalvaluetype) /*non-externals can't be searched*/
		return (false);
	
	if (flscriptrunning)
		fl = langbackgroundtask (false);
	else
		fl = shellbackgroundtask ();
	
	if (!fl)
		return (false);
	
	*flzoom = true; /*default*/
	
	hv = (hdlexternalvariable) val.data.externalvalue;
	
	switch ((**hv).id) {
		
		case idoutlineprocessor: case idscriptprocessor:
			return (opverbfind (hv, flzoom));
		
		case idwordprocessor:
			return (wpverbfind (hv, flzoom));
		
		case idtableprocessor:
			return (tableverbfind (hv, flzoom));
		
		case idmenuprocessor:
			return (menuverbfind (hv, flzoom));
		
		case idpictprocessor:
			return (pictverbfind (hv, flzoom));
		
		#ifdef fliowa
		
		case idcardprocessor:
			return (cardverbfind (hv, flzoom));
		
		#endif
		
		default:
			return (false);
		} /*switch*/
	} /*langexternalsearch*/


boolean langexternalcontinuesearch (hdlexternalvariable hvariable) {
	
	/*
	the caller just finished searching hvariable without finding a match.
	
	we'll call a table routine to continue the search from the variable's
	parent table
	*/
	
	return (tableverbcontinuesearch (hvariable));
	} /*langexternalcontinuesearch*/


boolean langexternalgettimes (hdlexternalhandle h, long *timecreated, long *timemodified, hdlhashnode hnode) {
	
	register hdlexternalvariable hdata = (hdlexternalvariable) h;
	
	switch ((**hdata).id) {
		
		case idoutlineprocessor: case idscriptprocessor:
			return (opverbgettimes (hdata, timecreated, timemodified));
		
		case idwordprocessor:
			return (wpverbgettimes (hdata, timecreated, timemodified));
		
		case idtableprocessor:
			return (tableverbgettimes (hdata, timecreated, timemodified, hnode));
		
		case idmenuprocessor:
			return (menuverbgettimes (hdata, timecreated, timemodified));
		
		case idpictprocessor:
			return (pictverbgettimes (hdata, timecreated, timemodified));
		
		#ifdef fliowa
		
		case idcardprocessor:
			return (cardverbgettimes (hdata, timecreated, timemodified));
		
		#endif
		
		default:
			return (false);
		} /*switch*/
	} /*langexternalgettimes*/


boolean langexternalsettimes (hdlexternalhandle h, long timecreated, long timemodified, hdlhashnode hnode) {
	
	register hdlexternalvariable hdata = (hdlexternalvariable) h;
	
	if (!langexternalsetdirty (h, true))
		return (false);
	
	switch ((**hdata).id) {
		
		case idoutlineprocessor: case idscriptprocessor:
			return (opverbsettimes (hdata, timecreated, timemodified));
		
		case idwordprocessor:
			return (wpverbsettimes (hdata, timecreated, timemodified));
		
		case idtableprocessor:
			return (tableverbsettimes (hdata, timecreated, timemodified, hnode));
		
		case idmenuprocessor:
			return (menuverbsettimes (hdata, timecreated, timemodified));
		
		case idpictprocessor:
			return (pictverbsettimes (hdata, timecreated, timemodified));
		
		#ifdef fliowa
		
		case idcardprocessor:
			return (cardverbsettimes (hdata, timecreated, timemodified));
		
		#endif
		
		default:
			return (false);
		} /*switch*/
	} /*langexternalsettimes*/


boolean langexternalfindusedblocks (hdlexternalhandle h, bigstring bspath) {
	
	/*
	this being a debuging routine, the implementation isn't as general 
	as the verbs.  see dbstats.c
	
	7/8/91 dmb: roll the beach ball
	*/
	
	register hdlexternalvariable hv = (hdlexternalvariable) h;
	boolean fl;
	
	rollbeachball ();
	
	switch ((**hv).id) {
		
		case idtableprocessor:
			return (tableverbfindusedblocks (hv, bspath));
		
		case idmenuprocessor:
			return (menuverbfindusedblocks (hv, bspath));
		
		default:
			if ((**hv).flinmemory)
				fl = statsblockinuse ((**hv).oldaddress, bspath);
			else
				fl = statsblockinuse ((dbaddress) (**hv).variabledata, bspath);
			
			return (fl);
		} /*switch*/
	} /*langexternalfindusedblocks*/


boolean langexternaltitleclick (Point pt, hdlexternalvariable hv) {
	
	/*
	return true if we consume the click
	*/
	
	if (cmdkeydown () || ismouserightclick()) {
		
		tableclienttitlepopuphit (pt, hv);
		
		return (true);
		}
	
	if (!mousedoubleclick ())
		return (false);
	
	tableclientsurface (hv);
	
	return (true); /*consumed*/
	} /*langexternaltitleclick*/


OSType langexternalgettypeid (tyvaluerecord val) {
	
	register tyvaluetype type = val.valuetype;
	
	if (type == externalvaluetype)
		type = (tyvaluetype) (outlinevaluetype + langexternalgettype (val));
	
	return (langgettypeid (type));
	} /*langexternalgettypeid*/


tyvaluetype langexternalgetvaluetype (OSType typeid) {
	
	register tyvaluetype type = langgetvaluetype (typeid);
	
	if (type >= outlinevaluetype)
		type = externalvaluetype;
	
	return (type);
	} /*langexternalgetvaluetype*/


boolean langexternalrefdata (hdlexternalvariable hv, Handle *hdata) {
	
	boolean fl;
	
	assert (!(**hv).flinmemory);
	
	dbpushdatabase ((**hv).hdatabase);
	
	fl = dbrefhandle ((dbaddress) (**hv).variabledata, hdata);
	
	dbpopdatabase ();
	
	return (fl);
	} /*langexternalrefdata*/


boolean langexternalsymbolchanged (hdlhashtable htable, const bigstring bsname, hdlhashnode hnode, boolean flvalue) {
#pragma unused(flvalue)

	return langexternalsymbolinserted (htable, bsname, hnode);
	} /*langexternalsymbolchanged*/


boolean langexternalsymbolinserted (hdlhashtable htable, const bigstring bsname, hdlhashnode hnode) {
#if !langexternalfind_optimization
#	pragma unused (htable, bsname, hnode)
#endif

#if langexternalfind_optimization
	if (	hnode != nil
		 && hnode != HNoNode
		 && (**hnode).val.valuetype == externalvaluetype) {
		
			hdlexternalvariable	hv = (hdlexternalvariable) (**hnode).val.data.externalvalue;

			(**hv).hexternaltable = htable;

			(**hv).hexternalnode = hnode;
			}
	#endif

	return true;
	} /*langexternalsymbolinserted*/


