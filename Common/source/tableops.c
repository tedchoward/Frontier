
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

#include "quickdraw.h"
#include "cursor.h"
#include "resources.h"
#include "strings.h"

#ifndef odbengine
#include "search.h"
#endif

#include "langexternal.h"
#include "tablestructure.h"
#include "tableinternal.h"
#include "tableverbs.h"
#include "claybrowser.h"



#ifdef fldebug

hdlhashtable debugtable;
hdltableformats debugformats;
hdltablevariable debugvariable;
bigstring debugfirstsort;
hdlhashnode debugnode;
bigstring debughashkey;


boolean tablesetdebugglobals (register hdlhashtable ht, register hdlhashnode hnode) {
	
	/*
	set debug globals.  we're about to dive into the indicated node in 
	the table ht
	
	12/12/91 dmb: return true so this can be called as an assertion
	*/
	
	assert (ht && even ((long) ht) && even ((long) *ht));
	
	debugtable = ht;
	
	debugvariable = (hdltablevariable) (**ht).hashtablerefcon;
	
	#if !flruntime
	
	debugformats = (hdltableformats) (**ht).hashtableformats;
	
	#endif
	
	if ((**ht).hfirstsort)
		gethashkey ((**ht).hfirstsort, debugfirstsort);
	
	debugnode = hnode;
	
	gethashkey (hnode, debughashkey);
	
	return (true);
	} /*tablesetdebugglobals*/

#endif


#if defined (isFrontier) && !defined (flruntime)


hdlhashtable tablegetlinkedhashtable (void) {
	
	return ((hdlhashtable) (**tableformatsdata).htable);
	} /*tablegetlinkedhashtable*/


hdltablevariable tablegetlinkedtablevariable (void) {
	
	register hdlhashtable ht = tablegetlinkedhashtable ();
	
	return ((hdltablevariable) (**ht).hashtablerefcon);
	} /*tablegetlinkedtablevariable*/


boolean tablegetiteminfo (hdlheadrecord hnode, hdlhashtable *htable, bigstring bs, tyvaluerecord *val, hdlhashnode *hhashnode) {
	
	tybrowserinfo info;
	
	if (!browsergetrefcon (hnode, &info))
		return (false);
	
	*htable = info.dirid;
	
	opgetheadstring (hnode, bs);
	
	if (val == nil) // caller doesn't need value
		return (true);
	else
		return (hashtablelookup (info.dirid, bs, val, hhashnode));
	} /*tablegetiteminfo*/


boolean tablegetcursorinfo (hdlhashtable *htable, bigstring bs, tyvaluerecord *val, hdlhashnode *hhashnode) {
	
	/*
	get the name and value of the table line that the cursor points at.
	
	factors code that was appearing all over the place.
	*/
	
	if (outlinedata == nil)
		return (false);

	return (tablegetiteminfo ((**outlinedata).hbarcursor, htable, bs, val, hhashnode));
	} /*tablegetcursorinfo*/


void tablelinkformats (hdlhashtable htable, hdltableformats hformats) {
	
	/*
	establish links between hashtable and formats record.  the pointing is
	two-way.
	*/
	
	(**htable).hashtableformats = hformats;
	
	(**hformats).htable = htable;
	} /*tablelinkformats*/

#endif


boolean istablevariable (hdlexternalvariable hv) {
	
	switch ((**hv).id) {
		
		case idtableprocessor:
		#ifdef xmlfeatures
			case idxmlprocessor:
		#endif
			return true;

		default:
			return (false);
		}
	} /*istablevariable*/


boolean gettablevariable (tyvaluerecord val, hdltablevariable *hvariable, short *errcode) {
	
	/*
	extract a handle to a table variable record from the value record.
	
	return true if you got one.  this factors code which was appearing all over 
	the place.
	*/
	
	register hdlexternalvariable hv;
	
	if (val.valuetype != externalvaluetype) {
		
		*errcode = namenottableerror;
		
		return (false);
		}
		
	hv = (hdlexternalhandle) val.data.externalvalue;
	
	if (!istablevariable (hv)) {
		
		*errcode = namenottableerror;
		
		return (false);
		}
	
	*hvariable = (hdltablevariable) hv;
	
	return (true);
	} /*gettablevariable*/


boolean findnamedtable (hdlhashtable htable, bigstring bs, hdlhashtable *hnamedtable) {
	
	/*
	look for a table in the indicated table with the indicated name.  return a 
	handle to the table we found.  return false if it isn't defined.
	*/
	
	hdlhashnode hnode;
	register boolean fl;
	
	pushhashtable (htable);
	
	fl = langfindsymbol (bs, &htable, &hnode);
	
	pophashtable ();
	
	if (!fl) 
		return (false);
	
	if (!tablevaltotable ((**hnode).val, hnamedtable, hnode))
		return (false);
	
	#if !flruntime
	
	(***hnamedtable).parenthashtable = htable; /*retain parental link*/
	
	#endif
	
	return (true);
	} /*findnamedtable*/


boolean newtablevariable (boolean flinmemory, long variabledata, hdltablevariable *h, boolean flxml) {
	
	/*
	tytablevariable item;
	
	clearbytes ((ptrchar) &item, longsizeof (item));
	
	item.flinmemory = flinmemory;
	
	item.variabledata = variabledata;
	
	return (newfilledhandle ((ptrchar) &item, longsizeof (item), (Handle *) h));
	*/
	
	if (!langnewexternalvariable (flinmemory, variabledata, (hdlexternalvariable *) h))
		return (false);

	#ifdef xmlfeatures
		(**h).flxml = flxml;
	#endif

	return (true);
	} /*newtablevariable*/


boolean tablenewtablevalue (hdlhashtable *newtable, tyvaluerecord *newval) {
	
	if (!langexternalnewvalue (idtableprocessor, nil, newval))
		return (false);
	
	langexternalvaltotable (*newval, newtable, HNoNode);
	
	return (true);
	} /*tablenewtablevalue*/


#if defined (isFrontier) && !defined (flruntime)

boolean tablefinddatawindow (hdlhashtable htable, hdlwindowinfo *hinfo) {
	
	/*
	find the window that's displaying the indicated hash table.
	*/
	
	register hdlhashtable ht = htable;
	
	if (!(**ht).flwindowopen) /*can't find window for a table that isn't displayed*/
		return (false);
	
	return (shellfinddatawindow ((Handle) (**ht).hashtableformats, hinfo));
	} /*tablefinddatawindow*/


boolean tabledisposetable (hdlhashtable htable, boolean fldisk) {
	
	register hdlhashtable ht = htable;
	register hdltableformats hf;
	hdlwindowinfo hinfo;
	
	if (ht) {
		
		#ifdef fldebug

			if (tablefinddatawindow (ht, &hinfo)) {
				
				shellinternalerror (0, "\x1d" "table shouldn't have a window");
				
				shellclosewindow ((**hinfo).macwindow);
				}
		#endif
		
		hf = (hdltableformats) (**ht).hashtableformats;
		
		if (hf) {
			
			disposetableformats (hf);
			}
		
		disposehashtable (ht, fldisk);
		}
	
	return (true);
	} /*tabledisposetable*/


hdldatabaserecord tablegetdatabase (hdlhashtable ht) {
	
	if (ht == nil)
		return (nil);

	return (langexternalgetdatabase ((hdlexternalvariable) (**ht).hashtablerefcon));
	} /*tablegetdatabase*/


boolean tablesortedinversesearch (hdlhashtable htable, langsortedinversesearchcallback visit, ptrvoid refcon) {
	
	/*
	like the langhash version, but we push the table's database in case 
	a disk value must be resolved
	*/
	
	hdldatabaserecord hdb = tablegetdatabase (htable);
	hdldatabaserecord hdbsave = databasedata;
	boolean fl;
	
	if (hdb)
		databasedata = hdb;
	
	fl = hashsortedinversesearch (htable, visit, refcon);
	
	if (hdb)
		databasedata = hdbsave;
	
	return (fl);
	} /*tablesortedinversesearch*/


boolean tablecheckwindowrect (hdlhashtable htable) {
	
	/*
	some factored code -- if the indicated table is in an open
	window, we check to see if its windowrect has changed, and dirty the
	table if so.  return true if the windowrect changed.
	*/
	
	register hdlhashtable ht = htable;
	register hdltableformats hf = (hdltableformats) (**ht).hashtableformats;
	hdlwindowinfo hinfo;
	Rect r;
	
	if (!tablefinddatawindow (ht, &hinfo)) 
		return (false);
		
	shellgetglobalwindowrect (hinfo, &r);
		
	if (equalrects (r, (**hf).windowrect)) 
		return (false);
		
	(**hf).windowrect = r;
			
	(**ht).fldirty = true;
	
	return (true);
	} /*tablecheckwindowrect*/

#else

boolean tabledisposetable (hdlhashtable htable, boolean fldisk) {
	
	register hdlhashtable ht = htable;
	
	if (ht) {
		
		disposehashtable (ht, fldisk);
		}
	
	return (true);
	} /*tabledisposetable*/

#endif


boolean tableverbunload (hdlexternalvariable hvariable) {
	
	/*
	the table was loaded into memory temporarily, the caller is asking us to
	get rid of the in-memory version of the table.
	*/
	
	register hdltablevariable hv = (hdltablevariable) hvariable;
	
	if ((**hv).flinmemory) { /*if it's on disk, don't need to do anything*/
		
		tabledisposetable ((hdlhashtable) (**hv).variabledata, false);
		
		(**hv).variabledata = (**hv).oldaddress;
		
		(**hv).oldaddress = 0;
		
		(**hv).flinmemory = false;
		}
	
	return (true);
	} /*tableverbunload*/


boolean findvariablesearch (hdlhashtable intable, hdlexternalvariable forvariable, boolean flonlyinmemory, hdlhashtable *foundintable, bigstring foundname, tyfindvariablecallback ancestorcallback) {

	register hdlhashtable ht = intable;
	register hdlhashnode x;
	register short i;
	tyvaluerecord val;
	register hdlexternalvariable hv = nil;
	
	for (i = 0; i < ctbuckets; i++) {
		
		x = (**ht).hashbucket [i];
		
		while (x != nil) { /*chain through the hash list*/
			
			register boolean fltempload = false;
			
			val = (**x).val;
			
			if (val.valuetype != externalvaluetype) 
				goto nextx;
				
			hv = (hdlexternalvariable) val.data.externalvalue;
			
			if (hv == forvariable) { /*bravo!  we found it...*/
				
				*foundintable = ht;
				
				gethashkey (x, foundname);
				
				if (ancestorcallback != nil)
					(*ancestorcallback) (ht, x);
				
				return (true);
				}
			
			if (!istablevariable (hv))
				goto nextx;
			
			if (!(**hv).flinmemory) {
				
				if (flonlyinmemory)
					goto nextx;
					
				if (!tableverbinmemory (hv, x))
					return (false);
					
				fltempload = true;
				}
			
			assert (tablesetdebugglobals (ht, x)); /*set debug globals*/
			
			if (findvariablesearch ((hdlhashtable) (**hv).variabledata, forvariable, flonlyinmemory, foundintable, foundname, ancestorcallback)) {
				
				if (ancestorcallback != nil)
					(*ancestorcallback) (ht, x);
				
				//  this was an idea, but I don't think it's needed or speeds things up
				//if ((**(hdlhashtable) (**hv).variabledata).parenthashtable == nil)
				//	(**(hdlhashtable) (**hv).variabledata).parenthashtable = ht;
				
				return (true); /*unwind recursion*/
				}
				
			nextx:
			
			if (fltempload)
				tableverbunload (hv);
			
			x = (**x).hashlink; /*advance to next node in chain*/
			} /*while*/
		} /*for*/
		
	return (false);
	} /*findvariablesearch*/


boolean tablefindvariable (hdlexternalvariable hvariable, hdlhashtable *htable, bigstring bsname) {
	
	/*
	conduct an extensive but quick research project for the caller.  he has an
	external variable, and wants to know where the hashnode that points to it 
	can be located, ie what table it's in, and what the name of the variable is.
	
	this is alternate to storing back-pointers in each variable record, something
	that would cost a lot of code to maintain, and which would add complexity to
	something that's already quite complex.
	
	11/14/90 DW: if we don't find it by traversing through in-memory structures,
	try going over things that are swapped out before giving up.
	
	11/15/90 DW: undid the previous feature.  think about it -- if you have a 
	variable record handle to ask about it must live in an in-memory table.  this
	slowed down the home window's "Object DB" button, which zooms out the root table,
	which won't be found in itself -- it searches the whole object database, even
	stuff on disk before it gives up.  no, if it can't be found in memory, it 
	can't be found.
	
	4/19/91 dmb: special case for root variable
	*/
	
	if (hvariable == nil) /*defensive driving*/
		return (false);
	
	if (roottable == nil) /*most defense*/
		return (false);

	if (hvariable == (hdlexternalvariable) rootvariable) { /*special case; root variable doesn't live in a table*/
		
		*htable = nil; /*langvalue.c will deal with "root" better this way*/
		
		copystring (nameroottable, bsname);
		
		return (true);
		}
	
	if (findvariablesearch (roottable, hvariable, true, htable, bsname, nil))
		return (true);
	
	return (false); /*wire off following feature*/
	
	/*return (findvariablesearch (roottable, hvariable, false, htable, bsname, nil));*/
	} /*tablefindvariable*/


static boolean nosubsdirtyvisit (hdlhashnode hnode, ptrvoid refcon) {
	
	/*
	return true if hnode and its substructure is all clean
	*/
	
	tyvaluerecord val;
	register hdlexternalvariable hv;
	
	val = (**hnode).val;
	
	if (val.valuetype != externalvaluetype) 
		return (true);
	
	hv = (hdlexternalvariable) val.data.externalvalue;
	
	if (!istablevariable (hv))
		return (!langexternalisdirty (hv)); /*for e.g. a dirty outline or wpdoc*/
	
	if (!(**hv).flinmemory) /*can't be dirty if it isn't in memory*/
		return (true);
		
	/* RAB 97-05-21  --  taken from packstubs, should this be part of Frontier? dmb: yes!*/
	if ((**hnode).fldontsave)
		return (true);
	
	return (tablenosubsdirty ((hdlhashtable) (**hv).variabledata)); /*daisy-chain recursion*/
	} /*nosubsdirtyvisit*/


boolean tablenosubsdirty (hdlhashtable htable) {
	
	/*
	recursively branch out from htable and return false if any of its subtables
	are dirty.  also return false if the table itself is dirty.
	
	changes need to be saved for any table that has dirty subs.
	*/
	
	register hdlhashtable ht = htable;
	
	if ((**ht).fldirty) 
		return (false);
	
	return (hashtablevisit (ht, nosubsdirtyvisit, nil));
	} /*tablenosubsdirty*/


static boolean tableupdatesubsdirtyflag (hdlhashtable ht); /*forward declaration*/

static boolean updatesubsdirtyvisit (hdlhashnode hnode, ptrvoid refcon) {

	tyvaluerecord val = (**hnode).val;
	boolean *flsubsdirty = (boolean *) refcon;
	
	if (val.valuetype == externalvaluetype) {
	
		register hdlexternalvariable hv = (hdlexternalvariable) val.data.externalvalue;
		
		if (!istablevariable (hv)) /*once fldirtysub is true, we only need to visit sub-tables but none of the other externals*/
			*flsubsdirty = *flsubsdirty || langexternalisdirty (hv);
		else
			*flsubsdirty = ((**hv).flinmemory
								&& !(**hnode).fldontsave
								&& tableupdatesubsdirtyflag ((hdlhashtable) (**hv).variabledata))
							|| *flsubsdirty;
		}
	
	return (true);
	}/*updatesubsdirtyvisit*/
	
	
static boolean tableupdatesubsdirtyflag (hdlhashtable ht) {

	boolean flsubsdirty = false;
	
	hashtablevisit (ht, updatesubsdirtyvisit, &flsubsdirty);
	
	(**ht).flsubsdirty = flsubsdirty;
	
	return (flsubsdirty || (**ht).fldirty);
	}/*tableupdatesubsdirtyflag*/
	
	
boolean tablepreflightsubsdirtyflag (hdlexternalvariable hv) {
	
	 /*
	 6.2a15 AR: Call this function to update the flsubsdirty flag
	 of every loaded and saveable table in the table structure
	 */
		
	if (fldatabasesaveas)
		return (true); /*not really interested if we're saving a copy*/
	
	if (!(**hv).flinmemory)
		return (true); /*not likely, but might just as well handle it*/
		
	tableupdatesubsdirtyflag ((hdlhashtable) (**hv).variabledata);
	
	return (true);
	}/*tablepreflightsubsdirtyflag*/


#if defined (isFrontier) && !defined (flruntime)

typedef struct findtableinfo {
	
	hdlhashtable htablelookfor;
	
	hdlhashnode hnodelookfor;
	} tyfindtableinfo, *ptrfindtableinfo;


static boolean findtablevisit (hdlhashnode hnode, ptrvoid refcon) {
	
	/*
	we're looking for a node that's a table variable -- a specific one, the
	one pointed to by info.htablelookfor.
	*/
	ptrfindtableinfo info = (ptrfindtableinfo) refcon;
	hdltablevariable hvariable;
	register hdlhashtable ht;
	short errcode;
	
	if (!gettablevariable ((**hnode).val, &hvariable, &errcode))
		return (true);
		
	ht = (hdlhashtable) (**hvariable).variabledata;
		
	if (ht != (*info).htablelookfor) /*keep visiting, this ain't it*/
		return (true);
	
	(*info).hnodelookfor = hnode;
	
	return (false); /*false terminates traversal*/
	} /*findtablevisit*/


static boolean parentsearch (hdlhashtable intable, hdlhashtable fortable, boolean flonlyinmemory, hdlhashtable *hparent, bigstring bsname) {
	
	/*
	4/29/96 4.0b8 dmb: we need to convert fortable to a parenttable/name address pair
	
	root is a special case.
	
	we have no direct way of making a table-to-parent connection independent of 
	windows. someday, we should make the parenttable field always valid, but that 
	will require relatively extensive (minor but widely distributed) code change.
	
	5.0.2b13 dmb: moved this in from langvalue.c
	*/
	
	register hdlhashtable ht = intable;
	register hdlhashnode x;
	register short i;
	tyvaluerecord val;
	register hdlexternalvariable hv = nil;
	
	if (intable == fortable) {	/*special case for root*/
	
		*hparent = nil;
		
		if ((**intable).fllocaltable)
			setemptystring (bsname);
		else
			copystring (nameroottable, bsname);
		
		return (true);
		}
	
	for (i = 0; i < ctbuckets; i++) {
		
		x = (**ht).hashbucket [i];
		
		while (x != nil) { /*chain through the hash list*/
			
			register boolean fltempload = false;
			hdlhashtable htable;
			
			val = (**x).val;
			
			if (val.valuetype != externalvaluetype) 
				goto nextx;
				
			hv = (hdlexternalvariable) val.data.externalvalue;
	
			if ((**hv).id != idtableprocessor)
				goto nextx;
			
			if (!(**hv).flinmemory) {
			
				if (flonlyinmemory)	/*can't find it if it isn't in memory*/
					goto nextx;
				
				if (!tableverbinmemory (hv, x))
					return (false);
					
				fltempload = true;
				}
			
			htable = (hdlhashtable) (**hv).variabledata;
			
			if (htable == fortable) {
				
				*hparent = ht;
				
				gethashkey (x, bsname);
				
				return (true);
				}
			
			if (parentsearch (htable, fortable, flonlyinmemory, hparent, bsname)) {
				
				return (true); /*unwind recursion*/
				}
				
			nextx:
			
			if (fltempload)
				tableverbunload (hv);
			
			x = (**x).hashlink; /*advance to next node in chain*/
			} /*while*/
		} /*for*/
		
	return (false);
	} /*parentsearch*/


boolean findinparenttable (hdlhashtable htable, hdlhashtable *hparent, bigstring bs) {
	
	/*
	return enough information to identify the line in the table that the indicated
	table zoomed from.  return false if that table isn't displayed in a window, or
	if for some reason a node couldn't be found.
	
	11/14/90 DW: add flopen flag -- if true and the window isn't open, we try to 
	open it, returning false if it couldn't be opened.
	
	5.0d18 dmb: removed flopen flag, making this function public
	
	5.0.2b13 dmb: if parenthashtable is nil, search for it and set it. parentOf func
	now calls us (we stole its code).
	*/
	
	hdlhashtable h, ht = (**htable).parenthashtable;
	tyfindtableinfo info;
	
	setemptystring (bs);
	
	if (ht == nil) { // parent isn't set; search globally (& set name)
	
		if ((**htable).fllocaltable) {
			
			for (h = currenthashtable; h != nil; h = (**h).prevhashtable) {
				
				if (parentsearch (h, htable, true, &ht, bs))
					break;
				}
			}
		else
			parentsearch (roottable, htable, true, &ht, bs);
		
		(**htable).parenthashtable = ht; // retain link for future ops
		}
	else { // parent already set, just need to find name
	
		info.htablelookfor = htable; /*set global for traversal*/
		
		if (hashtablevisit (ht, &findtablevisit, &info))
			return (false);
		
		gethashkey (info.hnodelookfor, bs);
		}
	
	*hparent = ht;
	
	return (ht != nil) || (htable == roottable);
	} /*findinparenttable*/

/*
boolean tablevisicursor (void) {
	
	register hdltableformats hf = tableformatsdata;
	
	return (tablevisi ((**hf).rowcursor, (**hf).colcursor));
	} /%tablevisicursor%/
*/


boolean tableexiteditmode (void) {
	
	return (opsettextmode (false));
	} /*tableexiteditmode*/


boolean tablemovetoname (hdlhashtable htable, bigstring bsname) {
	
	/*
	2/4/91 dmb: must visi if already on cell

	5.0a25 dmb: use opfindhead, not opflatfind deriviative.
	Added htable param, but we're not using it now
	*/
	
	hdlheadrecord hfound;
	boolean fl;
	
	/*
	if (!hashsortedsearch (tablegetlinkedhashtable (), bsname, &row))
		return (false);
	*/
	
	if (!tableexiteditmode ())
		return (false);
	
	fl = opfindhead (opfirstatlevel ((**outlinedata).hbarcursor), bsname, &hfound);
	
	if (fl) {
	
		opclearallmarks ();
		
		opmoveto (hfound);
		}
	
//	tableexiteditmode ();
	
	return (fl); //tablemovecursor (row, namecolumn)
	} /*tablemovetoname*/


boolean tablemovetonode (hdlhashnode hnode) {
	
	hdlheadrecord hsummit;
	long row;
	
	if (!hashgetsortedindex (tablegetlinkedhashtable (), hnode, &row))
		return (false);
	
	if (!opnthsummit (row, &hsummit))
		return (false);
	
	opclearallmarks ();
	
	return (opmoveto (hsummit));
	} /*tablemovetonode*/


boolean tablebringtofront (hdlhashtable htable) {
	
	/*
	find the window that displays the indicated table, bring it to front.
	
	also set the table globals for this window, if we're successful.
	
	1/9/91 dmb: it's not cool to set globals unless we know for sure that 
	we'll never be called in a context where something higher in the calling 
	chain will expect its globals to remain intact.  as a utility routine, 
	we can't assume that, so we leave it to our caller to push and pop 
	globals as neccessary.  also, there's no reason to force an immediate 
	update.
	*/
	
	hdlwindowinfo hinfo;
	
	if (!tablefinddatawindow (htable, &hinfo))
		return (false);
	
	shellbringtofront (hinfo);
	
	/*
	shellsetglobals ((**hinfo).macwindow);
	
	shellupdatenow ((**hinfo).macwindow);
	*/
	
	return (true);
	} /*tablebringtofront*/
	

/*
static boolean tablesavebarcursor (hdlhashnode *hnode) {
	
	register hdltableformats hf = tableformatsdata;

	return (hashgetnthnode (tablegetlinkedhashtable (), (**hf).rowcursor, hnode));
	} /%tablesavebarcursor%/
	
	
static boolean tablerestorebarcursor (hdlhashnode hnode) {
	
	short index;
	
	hashgetsortedindex (tablegetlinkedhashtable (), hnode, &index);
	
	(**tableformatsdata).rowcursor = index; 
	
	return (true);
	} /%tablerestorebarcursor%/
*/

boolean tableresort (hdlhashtable ht, hdlhashnode hresort) {
	
	/*
	re-sort the current hashtable.
	
	3/31/93 dmb: added hresort parameter. if it's not nil, then only that 
	node needs to be resorted.
	*/
	
	register boolean fl;
	hdlheadrecord hcursor = (**outlinedata).hbarcursor;
	
//	flmustexiteditmode = true;
	
	tableexiteditmode ();
	
	opsaveeditbuffer ();
	
	fl = hashresort (ht, hresort);
	
	opmoveto (hcursor);
	
	oprestoreeditbuffer ();
	
//	invalrect ((**tableformatsdata).tablerect);
	
	tablesymbolsresorted (ht);
	
	tabledirty ();
	
	return (fl);
	} /*tableresort*/


boolean tablesetsortorder (hdlhashtable ht, short sortorder) {
	
	/*
	3/23/93 dmb: set watch cursor before sorting
	*/
	
	register hdltableformats hf = (hdltableformats) (**ht).hashtableformats;
	
	(**ht).sortorder = sortorder;
	
	if (hf != nil) {
		
		invalrect ((**hf).titlerect);
		
		setcursortype (cursoriswatch);
		
		shellforcecursoradjust (); /*make sure it reverts ASAP*/
		
		tableresort (ht, nil);
		
		opvisibarcursor ();
		}
	
	return (true);
	} /*tablesetsortorder*/


boolean tablegetsortorder (hdlhashtable ht, short *sortorder) {
	
	/*
	3/23/93 dmb: set watch cursor before sorting
	*/
	
	*sortorder = (**ht).sortorder;
	
	return (true);
	} /*tablegetsortorder*/


boolean tablegetcursorpath (bigstring bspath) {
	
	/*
	return the full path from root to the item pointed to by the table cursor.
	
	7/12/91 dmb: get the quoted path, not just the full path
	*/
	
	hdlhashtable htable;
	bigstring bs;
	tyvaluerecord val;
	hdlhashnode hnode;
	
	if (!tablegetcursorinfo (&htable, bs, &val, &hnode))
		return (false);
	
	return (langexternalgetquotedpath (htable, bs, bspath));
	} /*tablegetcursorpath*/


static boolean contextneedsobjectmodeltable (hdlhashtable htable, tyvaluetype type) {
	
	return ((type == objspecvaluetype) 
			&& (objectmodeltable != nil) 
			&& (objectmodeltable != htable)); 
			//&& !(**htable).fllocaltable);
	} /*contextneedsobjectmodeltable*/


boolean tablepushcontext (hdlhashtable ht, tyvaluetype type) {
	
	/*
	establish the appropriate context to evaluate a script as part of 
	a direct table manipulation.
	
	5.1.5b12 dmb: always put ht in the most local scope -- unless it's the root and already is
	*/
	
	if ((**ht).fllocaltable)
		return (pushhashtable (ht));
	
	langpushscopechain ();
	
	if (contextneedsobjectmodeltable (ht, type))
		chainhashtable (objectmodeltable);
	
	if (ht != roottable)
		chainhashtable (ht);
	
	return (true);
	} /*tablepushcontext*/


boolean tablepopcontext (hdlhashtable ht, tyvaluetype type) {
	
	/*
	reverse the action of tablepushcontext
	*/
	
	if ((**ht).fllocaltable)
		return (pophashtable ());
	
	if (contextneedsobjectmodeltable (ht, type))
		unchainhashtable ();
	
	if (ht != roottable)
		unchainhashtable ();
	
	langpopscopechain ();
	
	return (true);
	} /*tablepopcontext*/


#endif


boolean tablegetstringlist (short id, bigstring bs) {
	
	return (getstringlist (tablestringlist, id, bs));
	} /*tablegetstringlist*/





