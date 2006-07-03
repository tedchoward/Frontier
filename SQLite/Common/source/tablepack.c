
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
#include "db.h"
#include "langexternal.h"
#include "tablestructure.h"
#include "tableinternal.h"
#include "tableverbs.h"



boolean tablepacktable (hdlhashtable htable, boolean flmemory, Handle *hpacked, boolean *flmustsave) {
	
	/*
	10/6/91 dmb: mergehandles now consumes both source handles.
	
	6.2a15 AR: added flmustsave parameter.
	*/
	
	register hdlhashtable ht = htable;
	register hdltableformats hf;
	Handle hpackedtable, hpackedformats;
	register boolean fl;
	
	if (!hashpacktable (ht, flmemory, &hpackedtable, flmustsave))
		return (false);
	
	hf = (hdltableformats) (**ht).hashtableformats; /*copy into register*/
	
	if (hf == nil) /*no formats linked in*/
		hpackedformats = nil;
	
	else {
		tablepushformats (hf); /*set table.c global*/
		
		fl = tablepackformats (&hpackedformats);
		
		tablepopformats ();
		
		if (!fl) {
			
			disposehandle (hpackedtable);
			
			return (false);
			}
		}
	
	fl = mergehandles (hpackedtable, hpackedformats, hpacked);
	
	/*
	disposehandle (hpackedtable);
	
	disposehandle (hpackedformats);
	*/
	
	return (fl);
	} /*tablepacktable*/


boolean tableunpacktable (Handle hpacked, boolean flmemory, hdlhashtable *htable) {
	
	/*
	9/24/91 dmb: don't treat format unpacking failure as a fatal error.
	
	10/16/91 dmb: clear table's dirty bit after unpacking.
	
	3.0.4b8 dmb: use scratchport for the duration; port is unknown
	
	4.0b7 dmb: don't set scratchport while unpacking the table; too many
	nested pushports result. Just set it when we care, and leave each table
	entry to fend for itself.
	
	5.0a23 dmb: don't create table formats if none are packed
	
	5.0a25 dmb: don't clear table's fldirty flag anymore.
	*/
	
	Handle hpackedtable = nil;
	Handle hpackedformats = nil;
	hdlhashtable ht = nil;
	hdltableformats hformats = nil;
	
	if (!unmergehandles (hpacked, &hpackedtable, &hpackedformats)) /*comsumes hpacked*/
		return (false);
	
	if (!newhashtable (htable)) {
		
		disposehandle (hpackedtable);
		
		goto error; /*will dispose of everything but hpackedtable*/
		}
	
	ht = *htable; /*move into register*/
	
	if (!hashunpacktable (hpackedtable, flmemory, ht)) /*always disposes of hpackedtable*/
		goto error;
	
	if (hpackedformats != nil) {
	
		if (!newtableformats (&hformats))
			goto error;
		
		tablelinkformats (ht, hformats);
		
		pushscratchport ();
		
		if (!tableunpackformats (hpackedformats, hformats)) {
			
		//	tablesetdimension (hformats, false, 0, fixedctcols);
			}
		
		popport ();
		
		disposehandle (hpackedformats);
		
		if ((**hformats).fldirty) /*formats were out of date*/
			(**ht).fldirty = true;
		}
	
	return (true);
	
	error: {
		
		disposehandle (hpackedformats);
		
		disposetableformats (hformats);
		
		disposehashtable (ht, false);
		
		return (false);
		}
	} /*tableunpacktable*/


boolean tableverbmemorypack (hdlexternalvariable h, Handle *hpacked, hdlhashnode hnode) {
	
	register hdlexternalvariable hv = h;
	register hdlhashtable ht;
	Handle hpush;
	register boolean fl;
	boolean fltempload;
	boolean fldummy;
	
	fltempload = !(**hv).flinmemory;
	
	if (!tableverbinmemory (hv, hnode))
		return (false);
	
	ht = (hdlhashtable) (**hv).variabledata; 
	
	tablecheckwindowrect (ht); 
	
	fl = tablepacktable (ht, true, &hpush, &fldummy);
	
	if (fltempload)
		tableverbunload (hv);
	
	if (fl) {
		
		fl = pushhandle (hpush, *hpacked);
		
		disposehandle (hpush);
		}
	
	return (fl);
	} /*tableverbmemorypack*/


boolean tableverbmemoryunpack (Handle hpacked, long *ixload, hdlexternalvariable *h, boolean flxml) {
	
	/*
	create a new outline variable -- not in memory.
	
	this is a special entrypoint for the pack and unpack verbs.
	*/
	
	Handle hpackedtable;
	hdlhashtable htable;
	register hdlhashtable ht;
	
	if (!loadhandleremains (*ixload, hpacked, &hpackedtable))
		return (false);
	
	if (!tableunpacktable (hpackedtable, true, &htable)) /*always disposes of hpackedtable*/
		return (false);
	
	ht = htable; /*move into register*/
	
	if (!newtablevariable (true, (long) ht, (hdltablevariable *) h, flxml)) {
		
		tabledisposetable (ht, false);
		
		return (false);
		}
	
	(**ht).hashtablerefcon = (long) *h; /*we can get from hashtable to variable rec*/
	
	(**ht).fldirty = true;
	
	return (true); /*in memory*/
	} /*tableverbmemoryunpack*/


boolean tableverbpack (hdlexternalvariable h, Handle *hpacked, boolean *flnewdbaddress) {
	
	/*
	12/4/91 dmb: set windowinfo's dirty bit to false after save
	
	6.2a15 AR: Rely on flsubsdirty flag in hashtable instead of calling tablenosubsdirty.
	Set new flnewdbaddress parameter appropriately -- we only guarantee it to be accurate
	if the function returns true.
	*/
	
	register hdlexternalvariable hv = h;
	register hdlhashtable ht;
	dbaddress adr;
	Handle hpackedtable;
	register boolean fl = true;
	boolean fltempload = false;
	boolean flmustsave = false;
	hdlwindowinfo hinfo;
	
	if (fldatabasesaveas) {
		
		fltempload = !(**hv).flinmemory;
		
		if (!tableverbinmemory (hv, HNoNode))
			return (false);
		
		*flnewdbaddress = true; /*it's in another database even*/
		}
	
	if (!(**hv).flinmemory) { /*not in memory, just push the old db address*/
		
		adr = (dbaddress) (**hv).variabledata;
		
		*flnewdbaddress = false;

		goto pushaddress;
		}
		
	adr = (**hv).oldaddress;
	
	ht = (hdlhashtable) (**hv).variabledata;
	
	tablecheckwindowrect (ht);
	
	assert (fldatabasesaveas || (((**ht).fldirty || (**ht).flsubsdirty) == !tablenosubsdirty (ht)));
	
	if (!fldatabasesaveas && !(**ht).flsubsdirty && !(**ht).fldirty) { /*none of our subs are dirty, old address still good*/

		*flnewdbaddress = false;

		goto pushaddress;
		}
	
	/*it's in memory and either the table itself or one of its subs are dirty, so pack the table*/
	
	fl = tablepacktable (ht, false, &hpackedtable, &flmustsave);
	
	if (!fl)
		goto pushaddress;
	
	/*only save if we're saving a copy, if the table itself is dirty (i.e. a scalar or the name of an object changed),
		or if one of its subs changed in  a way so that the table itself actually needs saving now*/
	
	if (fldatabasesaveas || (**ht).fldirty || flmustsave)
		fl = dbsavehandle (hpackedtable, &adr);
	
#if 0
	else {
		/*
		Unfortunately, this debugging code is bogus. The problem is that in hashpackexternal
		we previously assigned a random value to the unused byte of the tyexternaldiskrecord.
		*/
		
		Handle htemp;
		boolean fl;
		long ctbytes;
		
		fl = dbrefhandle ((**hv).oldaddress, &htemp);
		
		assert (fl);
		
		pullfromhandle (htemp, 0L, sizeof (long), &ctbytes);
		
		assert (ctbytes <= gethandlesize (htemp));
		
		sethandlesize (htemp, ctbytes);
		
		pullfromhandle (hpackedtable, 0L, sizeof (long), &ctbytes);
		
		assert (ctbytes <= gethandlesize (hpackedtable));
		
		sethandlesize (hpackedtable, ctbytes);
		
		assert (equalhandles (htemp, hpackedtable));
		
		disposehandle (htemp);
		}	
#endif
	
	disposehandle (hpackedtable);
	
	if (!fl)
		goto pushaddress;
	
	if (fldatabasesaveas)
		goto pushaddress;
	
	*flnewdbaddress = ((**hv).oldaddress != adr);
	
	(**hv).oldaddress = adr;
	
	(**ht).fldirty = false; /*it's been saved to the db*/
	
	(**ht).flsubsdirty = false;
	
	if (tablewindowopen (hv, &hinfo))
		shellsetwindowchanges (hinfo, false);
	
	pushaddress:
	
	if (fltempload)
		tableverbunload (hv);
	
	if (!fl)
		return (false);
	
#ifdef PACKFLIPPED
	longswap (adr);
#endif
	
	return (enlargehandle (*hpacked, sizeof (adr), (ptrchar) &adr));
	} /*tableverbpack*/


boolean tableverbunpack (Handle hpacked, long *ixload, hdlexternalvariable *h, boolean flxml) {

	dbaddress adr;
	
	if (!loadlongfromdiskhandle (hpacked, ixload, &adr)) 
		return (false);
		
	return (newtablevariable (false, adr, (hdltablevariable *) h, flxml));
	} /*tableverbunpack*/


static boolean tablepacktotextvisit (bigstring bsname, hdlhashnode hnode, tyvaluerecord val, ptrvoid refcon) {
#pragma unused (hnode)

	/*
	4.0.2b1 dmb: handle fldiskvals. see comment in hashsortedinversesearch
	*/
	
	Handle htextscrap = (Handle) refcon;
	boolean fl;
	
	pushchar (chtab, bsname);
	
	if (!pushtexthandle (bsname, htextscrap))
		return (true); /*abort traversal*/
	
	if (val.fldiskval)
		if (!copyvaluerecord (val, &val))
			return (true);
	
	fl = langvaluetotextscrap (val, htextscrap);
	
	if (exemptfromtmpstack (&val))
		disposevaluerecord (val, false);
	
	if (!fl)
		return (true);
	
	return (false); /*keep going*/
	} /*tablepacktotextvisit*/


boolean tableverbpacktotext (hdlexternalvariable h, Handle htext) {
	
	/*
	12/23/92 dmb: hashinversesearch now takes table as param; don't push/pop
	
	12/31/92 dmb: use hashsortedinversesearch so text is in correct order
	
	5.0.2b20 dmb: unload if just loaded
	
	5.1.5 dmb: use tablesortedinversesearch for guest databases
	*/
	
	register hdlexternalvariable hv = h;
	register hdlhashtable ht;
	register boolean fl;
	boolean fltempload = !(**hv).flinmemory;
	
	if (!tableverbinmemory (hv, HNoNode))
		return (false);
	
	ht = (hdlhashtable) (**hv).variabledata;
	
	//htextscrap = htext; /*set for visit routine*/
	
	//fl = !hashinversesearch (ht, &tablepacktotextvisit, bsname); /*false means complete traversal%/
	
	fl = !tablesortedinversesearch (ht, &tablepacktotextvisit, htext);
	
	if (fltempload)
		tableverbunload (hv);
	
	return (fl);
	} /*tableverbpacktotext*/


boolean tableverbgettimes (hdlexternalvariable h, long *timecreated, long *timemodified, hdlhashnode hnode) {
	
	register hdlexternalvariable hv = h;
	register hdlhashtable ht;
	
	if (!tableverbinmemory (hv, hnode))
		return (false);
	
	ht = (hdlhashtable) (**hv).variabledata;
	
	*timecreated = (**ht).timecreated;
	
	*timemodified = (**ht).timelastsave;
	
	return (true);
	} /*tableverbgettimes*/


boolean tableverbsettimes (hdlexternalvariable h, long timecreated, long timemodified, hdlhashnode hnode) {
	
	register hdlexternalvariable hv = h;
	register hdlhashtable ht;
	
	if (!tableverbinmemory (hv, hnode))
		return (false);
	
	ht = (hdlhashtable) (**hv).variabledata;
	
	(**ht).timecreated = timecreated;
	
	(**ht).timelastsave = timemodified;
	
	return (true);
	} /*tableverbsettimes*/


static boolean findusedblocksvisit (hdlhashnode hnode, ptrvoid refcon) {
	
	/*
	5.1.5b16: check fldontsave flag to avoid diving into guest databases, etc.
	*/
	
	ptrstring bsparent = (ptrstring) refcon;
	tyvaluerecord val = (**hnode).val;
	bigstring bspath;
	
//	if ((**hnode).fldontsave) //never gets saved in the database
//		return (true);
	
	gethashkey (hnode, bspath);
	
	if (bsparent != nil) {

		insertchar ('.', bspath);

		insertstring (bsparent, bspath);
		}

	if (val.valuetype == externalvaluetype)
		return (langexternalfindusedblocks ((hdlexternalvariable) val.data.externalvalue, bspath));
	
	if (val.fldiskval)
		return (statsblockinuse (val.data.diskvalue, bspath));
	
	return (true); /*continue traversal*/
	} /*findusedblocksvisit*/


boolean tableverbfindusedblocks (hdlexternalvariable h, bigstring bspath) {
	
	register hdlexternalvariable hv = h;
	register hdlhashtable ht;
	register boolean fl;
	boolean fltempload;
	
	fltempload = !(**hv).flinmemory;
	
	if (!tableverbinmemory (hv, HNoNode))
		return (false);
	
	if (!statsblockinuse ((**hv).oldaddress, bspath))
		return (false);
	
	ht = (hdlhashtable) (**hv).variabledata; 
	
	if (ht == filewindowtable)
		fl = true;
	else
		fl = hashtablevisit (ht, findusedblocksvisit, bspath);
	
	if (fltempload)
		tableverbunload (hv);
	
	return (fl);
	} /*tableverbfindusedblocks*/




