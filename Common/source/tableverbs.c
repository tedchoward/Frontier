
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

#include "file.h"
#include "memory.h"
#include "resources.h"
#include "frontier_strings.h"
#include "shell.h"
#include "opinternal.h"
#include "oplist.h"
#include "langinternal.h"
#include "langsystem7.h"
#include "tablestructure.h"
#include "tableinternal.h"
#include "tableverbs.h"
#include "kernelverbdefs.h"




typedef enum tytabletoken { /*verbs that are processed by table.c*/
	
	movefunc,
	
	copyfunc,
	
	renamefunc,
	
	moveandrenamefunc,
	
	/*
	lockfunc,
	
	islockedfunc,
	*/
	
	assignfunc,
	
	validatefunc,
	
	sortbyfunc,
	
	getcursorfunc,
	
	getselectionfunc,
	
	gofunc,
	
	gotofunc,
	
	gotonamefunc,
	
	jettisonfunc,
	
	packtablefunc,
	
	emptytablefunc,
	
	getdisplaysettings,
	
	setdisplaysettings,

	sortorderfunc,
	
	cttableverbs
	} tytabletoken;



static boolean gettableparam (hdltreenode hfirst, short pnum, hdlhashtable *htable, bigstring bs, hdltablevariable *hv, hdlhashnode *hnode) {
	
	/*
	5.0.2b13 dmb: on failure, generate an error; don't set global tableverberrornum
	*/
	
	tyvaluerecord val;
	short berrornum = 0;
	bigstring bserror;
	
	*hv = nil; /*default*/
	
	if (!getvarparam (hfirst, pnum, htable, bs)) /*name of table*/
		return (false);
	
	if (!langsymbolreference (*htable, bs, &val, hnode))
		return (false);
	
	if (!gettablevariable (val, hv, &berrornum)) {
		
		getstringlist (tableerrorlist, berrornum, bserror);
		
		langerrormessage (bserror);
		
		return (false);
		}
	
	return (true);
	} /*gettableparam*/


boolean gettablevalue (hdltreenode hfirst, short pnum, hdlhashtable *htable) {
	
	hdltablevariable hv;
	bigstring bsname;
	hdlhashnode hnode;
	
	if (!gettableparam (hfirst, pnum, htable, bsname, &hv, &hnode))
		return (false);
	
	if (!tableverbinmemory ((hdlexternalvariable) hv, hnode)) /*couldn't swap it into memory*/
		return (false);
	
	*htable = (hdlhashtable) (**hv).variabledata;
	
	return (true);
	} /*gettablevalue*/


static boolean tablevalidateverb (hdltreenode hparam1, tyvaluerecord *v) {
	
	/*
	11/14/90 DW: validating a table is now something you can do from a script.
	*/
	
	hdlhashtable htable;
	
	flnextparamislast = true;
	
	if (!gettablevalue (hparam1, 1, &htable))
		return (false);
		
	(*v).data.flvalue = tablevalidate (htable, false);
	
	return (true);
	} /*tablevalidateverb*/


static boolean tablemoveverb (hdltreenode hparam1, tyvaluerecord *v) {
	
	/*
	table.move (address, tableaddress): boolean; move the indicated table 
	entry to the given table
	
	9/30/91 dmb: use hashassign, not hashinsert, so existing item is overwritten
	
	10/3/91 dmb: use new fllanghashassignprotect flag to override protection

	5.0a15 dmb: on success, return address of moved value
	
	5.1.4 dmb: generate errors if item doesn't exist
	*/
	
	hdlhashtable ht1, ht2;
	bigstring bs;
	tyvaluerecord val;
	boolean fl;
	hdlhashnode hnode;
	
	if (!getvarparam (hparam1, 1, &ht1, bs))
		return (false);
	
	flnextparamislast = true;
	
	if (!gettablevalue (hparam1, 2, &ht2))
		return (false);
	
	pushhashtable (ht1);
	
	fl = hashlookup (bs, &val, &hnode);
	
	if (fl)
		hashdelete (bs, false, false); /*don't toss the value*/
	
	pophashtable ();
	
	if (!fl) {
	
		langparamerror (unknownidentifiererror, bs);
		
		return (false);
		}
		
	if (!hashtableassign (ht2, bs, val))
		return (false);
	
	return (setaddressvalue (ht2, bs, v));
	} /*tablemoveverb*/


static boolean tablecopyverb (hdltreenode hparam1, tyvaluerecord *v) {
	
	/*
	table.copy (address, tableaddress): boolean; copy the indicated table 
	entry to the given table
	
	9/30/91 dmb: use hashassign, not hashinsert, so existing item is overwritten
	
	10/3/91 dmb: use new fllanghashassignprotect flag to override protection

	5.0a15 dmb: on success, return address of moved value
	
	5.1.4 dmb: generate errors if item doesn't exist
	*/
	
	hdlhashtable ht1, ht2;
	bigstring bs;
	tyvaluerecord val;
	boolean fl;
	Handle hpacked;
	hdlhashnode hnode;
	
	if (!getvarparam (hparam1, 1, &ht1, bs))
		return (false);
	
	flnextparamislast = true;
	
	if (!gettablevalue (hparam1, 2, &ht2))
		return (false);
	
	fl = hashtablelookup (ht1, bs, &val, &hnode);
	
	if (!fl) {
		
		langparamerror (unknownidentifiererror, bs);
		
		return (false);
		}
		//return (true); /*not fatal error; false is returned to caller*/
	
	if (!langpackvalue (val, &hpacked, hnode)) /*error packing -- probably out of memory*/
		return (false);
	
	fl = langunpackvalue (hpacked, &val);
	
	disposehandle (hpacked);
	
	if (fl) {
		
		/*
		fllanghashassignprotect = false;
		*/
		
		fl = hashtableassign (ht2, bs, val);
		
		/*
		fllanghashassignprotect = true;
		*/
		
		if (!fl)
			disposevaluerecord (val, true);
		}
	
	if (fl)
		fl = setaddressvalue (ht2, bs, v);
	
	return (fl);
	} /*tablecopyverb*/


static boolean tablerenameverb (hdltreenode hparam1, tyvaluerecord *v) {
	
	/*
	table.rename (address, string): boolean; rename the indicated table 
	entry to the given name
	
	12/28/91 dmb: resort the table after changing its name
	
	8/26/92 dmb: if the table is displayed in a window, use tableresort 
	to ensure clean update

	5.0a15 dmb: on success, return address of renamed value
	
	5.1.4 dmb: disallow rename if new name is already in use; generate 
	errors if item doesn't exist
	*/
	
	hdlhashtable htable;
	bigstring bs, bsname;
	hdlhashnode hnode;
	
	if (!getvarparam (hparam1, 1, &htable, bs))
		return (false);
	
	flnextparamislast = true;
	
	if (!getstringvalue (hparam1, 2, bsname))
		return (false);
	
	if (!hashtablelookupnode (htable, bs, &hnode)) {
		
		langparamerror (unknownidentifiererror, bs);
		
		return (false);
		}
	
	if (!equalidentifiers (bs, bsname)) {
		
		if (hashtablesymbolexists (htable, bsname)) {
		
			lang2paramerror (badrenameerror, bs, bsname);
			
			return (false);
			}
		
		if (!hashsetnodekey (htable, hnode, bsname))
			return (false);
		}
	
	hashresort (htable, hnode);
	
	return (setaddressvalue (htable, bsname, v));
	} /*tablerenameverb*/


static boolean tablemoveandrenameverb (hdltreenode hparam1, tyvaluerecord *v) {
	
	/*
	5.0a15 dmb: on success, return address of moved value
	
	5.1.4 dmb: generate errors if item doesn't exist
	*/
	
	hdlhashtable ht1, ht2;
	bigstring bs1, bs2;
	tyvaluerecord val;
	boolean fl;
	hdlhashnode hnode;
	
	if (!getvarparam (hparam1, 1, &ht1, bs1))
		return (false);
	
	flnextparamislast = true;
	
	if (!getvarparam (hparam1, 2, &ht2, bs2))
		return (false);
	
	pushhashtable (ht1);
	
	fl = hashlookup (bs1, &val, &hnode);
	
	if (fl)
		hashdelete (bs1, false, false); /*don't toss the value*/
	
	pophashtable ();
	
	if (!fl) {
		
		langparamerror (unknownidentifiererror, bs1);
		
		return (false);
		}
	
	if (!hashtableassign (ht2, bs2, val))
		return (false);
	
	return (setaddressvalue (ht2, bs2, v));
	} /*tablemoveandrenameverb*/


/*
static boolean tablefindverb (hdltreenode hparam1, tyvaluerecord *v) {
	
	/%
	table.find (adr, bs): boolean; search the (external) value at adr 
	for the string bs.  if adr is a table, do *not* search subordinate 
	tables
	%/
	
	hdlhashtable htable;
	bigstring bsname, bs;
	tyvaluerecord val;
	boolean flzoom;
	boolean flsave;
	
	if (!getvarparam (hparam1, 1, &htable, bsname))
		return (false);
	
	flnextparamislast = true;
	
	if (!getstringvalue (hparam1, 2, searchparams.bsfind))
		return (false);
	
	startnewsearch (false, false);
	
	pushhashtable (htable);
	
	if (hashlookup (bsname, &val)) {
		
		flsave = searchparams.flonelevel;
		
		searchparams.flonelevel = true;
		
		(*v).data.flvalue = langexternalsearch (val, &flzoom);
		
		searchparams.flonelevel = flsave;
		}
	
	pophashtable ();
	
	return (true);
	} /%tablefindverb%/
*/


static boolean tableassignverb (hdltreenode hparam1, tyvaluerecord *v) {
	
	/*
	10/3/91 dmb: use new fllanghashassignprotect flag to override protection
	*/
	
	hdlhashtable htable;
	bigstring bsname;
	tyvaluerecord val;
	boolean fl;
	
	if (!getvarparam (hparam1, 1, &htable, bsname))
		return (false);
	
	flnextparamislast = true;
	
	if (!getparamvalue (hparam1, 2, &val))
		return (false);
	
	if (!copyvaluerecord (val, &val))
		return (false);
	
	/*
	fllanghashassignprotect = false;
	*/
	
	fl = hashtableassign (htable, bsname, val);
	
	/*
	fllanghashassignprotect = true;
	*/
	
	if (!fl)
		return (false);
	
	exemptfromtmpstack (&val);
	
	(*v).data.flvalue = true;
	
	return (true);
	} /*tableassignverb*/


#ifdef MACVERSION

static boolean tablepacktableverb (hdltreenode hparam1, tyvaluerecord *v) {
	
	/*
	this is really for internal use only; packes a table and creates a 
	named HASH resource that can be loaded in tablestructure
	*/
	
	hdlhashtable htable;
	bigstring bs;
	register boolean fl;
	Handle hpacked;
	boolean fldummy;
	
	if (!gettablevalue (hparam1, 1, &htable))
		return (false);
	
	flnextparamislast = true;
	
	if (!getstringvalue (hparam1, 2, bs))
		return (false);
	
	fl = hashpacktable (htable, true, &hpacked, &fldummy);
	
	if (fl) {
		
		lockhandle (hpacked);
		
		fl = filewriteresource (filegetapplicationrnum (), 'HASH', -1, bs, gethandlesize (hpacked), *hpacked);
		
		disposehandle (hpacked);
		}
	
	(*v).data.flvalue = fl;
	
	return (true);
	} /*tablepacktableverb*/

#endif

static boolean tableemptytableverb (hdltreenode hparam1, tyvaluerecord *v) {
	
	/*
	4/25/96 4.0b7 dmb: kernel implementation for speed
	*/
	
	hdlhashtable htable;
	
	flnextparamislast = true;
	
	if (!gettablevalue (hparam1, 1, &htable))
		return (false);
	
	return (setintvalue (emptyhashtable (htable, true), v));
	} /*tableemptytableverb*/


static boolean tablegetselvisit (hdlheadrecord hnode, ptrvoid refcon) {
	
	hdllistrecord hlist = (hdllistrecord) refcon;
	hdlhashtable ht;
	bigstring bs;
	tyvaluerecord val;
	
	if (!tablegetiteminfo (hnode, &ht, bs, nil, nil))
		return (false);
	
	if (!setaddressvalue (ht, bs, &val))
		return (false);
	
	if (!langpushlistval (hlist, nil, &val))
		return (false);
	
	disposevaluerecord (val, true); // don't let them accumulate
	
	return (true);
	} /*tablegetselvisit*/


static boolean tablegetselectionverb (hdltreenode hp1, tyvaluerecord *v) {
	
	hdllistrecord hlist;
	
	if (!langcheckparamcount (hp1, 0)) /*too many parameters were passed*/
		return (false);
	
	if (!opnewlist (&hlist, false))
		return (false);
	
	if (!opvisitmarked (down, &tablegetselvisit, (ptrvoid) hlist)) {
		
		opdisposelist (hlist);
		
		return (false);
		}
	
	return (setheapvalue ((Handle) hlist, listvaluetype, v));
	} /*tablegetselectionverb*/


static boolean tablegetdisplaysettingsverb (hdltreenode hp1, tyvaluerecord *v) {
	
	hdltableformats hf = tableformatsdata;
	hdlhashtable hsettings;
	tylinelayout layout;
	
	flnextparamislast = true;
	
	if (!gettablevalue (hp1, 1, &hsettings))
		return (false);
	
	layout = (**hf).linelayout;
	
	if (!layout.flinitted) // no layout assigned, get deault
		clayinitlinelayout (&layout);
	
	if (!claylayouttotable (&layout, hsettings))
		return (false);
	
	return (setbooleanvalue (true, v));
	} /*tablegetdisplaysettingsverb*/


static boolean tablesetdisplaysettingsverb (hdltreenode hp1, tyvaluerecord *v) {
	
	hdlhashtable hsettings;
	tylinelayout layout;
	
	flnextparamislast = true;
	
	if (!gettablevalue (hp1, 1, &hsettings))
		return (false);
	
	if (!claytabletolayout (hsettings, &layout))
		return (false);
	
	claysetlinelayout (tableformatswindowinfo, &layout);
	
	return (setbooleanvalue (true, v));
	} /*tablesetdisplaysettingsverb*/


static boolean tablefunctionvalue (short token, hdltreenode hparam1, tyvaluerecord *vreturned, bigstring bserror) {
	
	/*
	bridges table.c with the language.  the name of the verb is bs, its first parameter
	is hparam1, and we return a value in vreturned.
	
	we use a limited number of support routines from lang.c to get parameters and
	to return values. 
	
	return false only if the error is serious enough to halt the running of the script
	that called us, otherwise error values are returned through the valuerecord, which
	is available to the script.
	
	if we return false, we try to provide a descriptive error message in the 
	returned string bserror.
	
	11/14/91 dmb: getcursorfunc returns address, not full path
	
	10/3/92 dmb: commented out setcolwidthfunc. (this verb still isn't "offical")
	
	4/2/93 dmb: added jettisonfunc
	
	6/1/93 dmb: when vreturned is nil, return whether or not verb token must 
	be run in the Frontier process
	
	5.1.5b11 dmb: fixed gotofunc silent failure
	*/
	
	register tyvaluerecord *v = vreturned;
	register boolean fl = false;
	WindowPtr targetwindow;
	
	if (v == nil) { /*need Frontier process?*/
		
		switch (token) {
			
			case sortbyfunc:
			case getcursorfunc:
			case getselectionfunc:
			case gotofunc:
			case gotonamefunc:
			case gofunc:
			case getdisplaysettings:
			case setdisplaysettings:
				return (true);
			
			default:
				return (false);
			}
		}
	
	setbooleanvalue (false, v); /*by default, table functions return false*/
	
	switch (token) { /*these verbs don't require an open table window*/
		
		/*
		case findfunc:
			if (!tablefindverb (hparam1, v))
				goto error;
			
			return (true);
		*/
		
		case validatefunc:
			return (tablevalidateverb (hparam1, v));
		
		case movefunc:
			return (tablemoveverb (hparam1, v));
		
		case copyfunc:
			return (tablecopyverb (hparam1, v));
		
		case renamefunc:
			return (tablerenameverb (hparam1, v));
		
		case moveandrenamefunc:
			return (tablemoveandrenameverb (hparam1, v));
		
		/*
		case lockfunc:
		case islockedfunc:
		*/
		
		case assignfunc:
			return (tableassignverb (hparam1, v));
			
		#ifdef MACVERSION
		case packtablefunc:
			return (tablepacktableverb (hparam1, v));
		#endif

		case emptytablefunc:
			return (tableemptytableverb (hparam1, v));
		
		case jettisonfunc: { /*toss an object w/out forcing it into memory. for database recovery.*/
			hdlhashtable htable;
			bigstring bs;
			
			if (!getvarparam (hparam1, 1, &htable, bs)) /*name of table*/
				return (false);
			
			pushhashtable (htable);
			
			(*v).data.flvalue = hashdelete (bs, true, false);
			
			pophashtable ();
			
			return (true);
			}
		} /*switch*/
	
	/*all other verbs require a table window in front*/
	
	if (!langfindtargetwindow (idtableprocessor, &targetwindow)) {
		
		getstringlist (tableerrorlist, notableerror, bserror);
		
		return (false);
		}
	
	shellpushglobals (targetwindow); /*following verbs assume that an table is pushed*/
	
	(*shellglobals.gettargetdataroutine) (idtableprocessor); /*set table globals*/
	
	switch (token) {
		
		/*
		case setcolwidthfunc: {
			short colnum, colwidth;
			
			if (!getintvalue (hparam1, 1, &colnum))
				break;
			
			flnextparamislast = true;
			
			if (!getintvalue (hparam1, 2, &colwidth))
				break;
			
			(*v).data.flvalue = (*(**tableformatsdata).adjustcolwidthroutine) (colnum - 1, colwidth);
			
			tablesmashdisplay ();
			
			fl = true;
			
			break;
			}
		*/
		
		case sortbyfunc: {
			bigstring bssort, bstitle;
			short ixcol;
			
			flnextparamislast = true;
			
			if (!getstringvalue (hparam1, 1, bssort))
				break;
			
			alllower (bssort);
			
			for (ixcol = namecolumn; ixcol <= kindcolumn; ++ixcol) {
				
				tablegettitlestring (ixcol, bstitle);
				
				alllower (bstitle);
				
				if (equalstrings (bssort, bstitle)) {
					
					hdlhashtable ht;
					bigstring bs;
					
					tablegetcursorinfo (&ht, bs, nil, nil);
	
					(*v).data.flvalue = tablesetsortorder (ht, ixcol);
					
					break;
					}
				}
			
			fl = true;
			
			break;
			}
		
		/*
		case centertablefunc: {
			boolean flcenter;
			
			flnextparamislast = true;
			
			if (!getbooleanvalue (hparam1, 1, &flcenter))
				break;
				
			(*v).data.flvalue = tablesetcenter (flcenter);
			
			fl = true;
			
			break;
			}
		*/
			
		case getcursorfunc: {
			hdlhashtable htable;
			bigstring bs;
			tyvaluerecord val;
			hdlhashnode hhashnode;
			
			if (!langcheckparamcount (hparam1, 0)) /*too many parameters were passed*/
				break;
			
			/*
			tablegetcursorpath (bspath);
			
			fl = setstringvalue (bspath, v);
			*/
			
			if (!tablegetcursorinfo (&htable, bs, &val, &hhashnode))
				fl = setstringvalue (zerostring, v);
			else
				fl = setaddressvalue (htable, bs, v);
			
			break;
			}
		
		case getselectionfunc:
			fl = tablegetselectionverb (hparam1, v);
			
			break;
		
		case gotofunc: {
			short row;
			hdlheadrecord hsummit;
			
			flnextparamislast = true;
			
			if (!getintvalue (hparam1, 1, &row))
				break;
			
			if (opnthsummit (row, &hsummit)) {
				
				opclearallmarks ();
				
				opmoveto (hsummit);
				
				(*v).data.flvalue = true;
				}
			
			fl = true;
			
			break;
			}
		
		case gotonamefunc: {
			hdlhashtable ht;
			bigstring bs;
			
			if (!tablegetcursorinfo (&ht, bs, nil, nil))
				ht = nil;
			
			flnextparamislast = true;
			
			if (!getstringvalue (hparam1, 1, bs))
				break;
			
			(*v).data.flvalue = tablemovetoname (ht, bs);
			
			fl = true;
			
			break;
			}
		
		case gofunc: {
			tydirection dir;
			short count;
			
			if (!getdirectionvalue (hparam1, 1, &dir))
				break;
			
			flnextparamislast = true;
			
			if (!getintvalue (hparam1, 2, &count))
				break;
			
			opsettextmode (false);
			
			if (dir == down)
				dir = flatdown;
			
			if (dir == up)
				dir = flatup;
			
			(*v).data.flvalue = opmotionkey (dir, count, false);
			
			fl = true;
			
			break;
			}
		
		case getdisplaysettings:
			fl = tablegetdisplaysettingsverb (hparam1, v);
			
			break;
		
		case setdisplaysettings:
			fl = tablesetdisplaysettingsverb (hparam1, v);
			
			break;

		case sortorderfunc: {
			hdlhashtable ht;
			bigstring bs;
			short ixcol;

			tablegetcursorinfo (&ht, bs, nil, nil);

			tablegetsortorder (ht, &ixcol);

			tablegettitlestring (ixcol, bs);

			fl = setstringvalue (bs, v);

			break;
			}
		} /*switch*/
	
	shellupdatescrollbars (shellwindowinfo);
	
	shellpopglobals ();
	
	return (fl);
	} /*tablefunctionvalue*/


boolean tableinitverbs (void) {
	
	return (loadfunctionprocessor (idtableverbs, &tablefunctionvalue));
	} /*tableinitverbs*/






