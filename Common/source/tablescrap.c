
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
#include "dialogs.h"
#include "kb.h"
#include "scrap.h"
#include "strings.h"
#include "shell.rsrc.h"
#include "shellundo.h"
#include "lang.h"
#include "langexternal.h"
#include "op.h"
#include "opinternal.h"
#include "menueditor.h"
#include "wpengine.h"
#include "wpverbs.h"
#include "scripts.h"
#include "shell.rsrc.h"
#include "claybrowser.h"
#include "claybrowserstruc.h"
#include "tableinternal.h"
#include "tablestructure.h"
#include "tableverbs.h"


typedef struct tytablescrap {

	Handle hpacked;
	
	bigstring bspath;
	} tytablescrap, *ptrtablescrap, **hdltablescrap;



static boolean tablegetscrapvaluetype (Handle hscrap, tyvaluetype *type) {
	
	register Handle h;
	long ix;
	OSType typeid;
	
	h = hscrap; /*copy into register*/
	
	ix = stringlength ((ptrstring) *h) + 1;
	
	if (!loadfromhandle (h, &ix, longsizeof (typeid), &typeid))
		return (false);
	
	*type = langgetvaluetype (typeid);
	
	return (true);
	} /*tablegetscrapvaluetype*/


static boolean tableexpandtodotparams (bigstring bspath, hdlhashtable *htable, bigstring bsname) {
	
	pushhashtable (roottable);
	
	if (!langexpandtodotparams (bspath, htable, bsname))
		*htable = nil;
	
	pophashtable ();
	
	return (*htable != nil);
	} /*tableexpandtodotparams*/


static boolean tablegetscrapvalue (Handle hscrap, tyvaluerecord *val, hdlhashtable *htable, bigstring bsname) {
	
	register Handle h;
	register boolean fl;
	bigstring bspath;
	
	*htable = nil;
	
	if (!copyhandle (hscrap, &hscrap))
		return (false);
	
	h = hscrap; /*copy into register*/
	
	pullfromhandle (h, 0L, stringlength ((ptrstring) *h) + 1, bspath);
	
	disablelangerror ();
	
	tableexpandtodotparams (bspath, htable, bsname);
	
	enablelangerror ();
	
	fl = langunpackvalue (h, val);
	
	disposehandle (h);
	
	return (fl);
	} /*tablegetscrapvalue*/


static boolean tableexporttextscrap (const tyvaluerecord *scrapval, Handle *hexport) {
	
	/*
	7/19/91 dmb: don't prepend value name in exported text
	
	4/29/93 dmb: don't limit scalar values to 255 chars
	*/
	
	tyvaluerecord val;
	register boolean fl;
	
	val = *scrapval;
	
	*hexport = nil;

	if (val.valuetype == externalvaluetype) {
		
		fl = newemptyhandle (hexport);
		
		if (fl)
			fl = langexternalpacktotext ((hdlexternalvariable) val.data.externalvalue, *hexport);
		}
	else {
		
		fl = copyvaluerecord (val, &val) && coercetostring (&val);
		
		if (fl) {
			
			fl = copyhandle (val.data.stringvalue, hexport);
			
			disposevaluerecord (val, false);
			}
		}
	
	if (fl)
		return (true);
	
	disposehandle (*hexport);
	
	*hexport = nil;
	
	return (false);
	} /*tableexporttextscrap*/



static tyvaluetype langgetexternalvaluetype (const tyvaluerecord *val) {
	
	/*
	5.0b9 dmb: a missing bit of langexternal functionality
	*/

	if ((*val).valuetype != externalvaluetype)
		return (novaluetype);
	
	return (tyvaluetype) (outlinevaluetype + langexternalgettype (*val));
	} /*langgetexternalvaluetype*/


static boolean tableexportoutlinescrap (const tyvaluerecord *scrapval, Handle *hexport) {
	
	/*
	convert the given table scrap value to an outline/script/menubar
	
	7/21/91 dmb: see comment in pastetooutline about poor data encapsulation 
	techniques being practiced here (or not...)
	*/
	
	tyvaluetype type;
	tyvaluerecord val;
	register hdlexternalvariable hv;
	register hdloutlinerecord ho;
	
	*hexport = nil;
	
	type = langgetexternalvaluetype (scrapval);
	
	switch (type) { /*find out if our value has an outline*/
		
		case outlinevaluetype:
		case scriptvaluetype:
		case menuvaluetype:
			break;
		
		default:
			return (false);
		}
	
	if (!copyvaluerecord (*scrapval, &val) || !copyvaluedata (&val))
		return (false);
	
	/*
	find the value's outline and steal it be nilling out the field
	*/
	
	hv = (hdlexternalvariable) val.data.externalvalue;
	
	if (type == menuvaluetype) {
		
		ho = (**(hdlmenurecord) (**hv).variabledata).menuoutline;
		
		(**(hdlmenurecord) (**hv).variabledata).menuoutline = nil; /*we've consumed it*/
		}
	else {
		
		ho = (hdloutlinerecord) (**hv).variabledata;
		
		(**hv).variabledata = 0L; /*we've consumed it*/
		}
	
	*hexport = (Handle) ho;
	
	disposevaluerecord (val, false);
	
	return (true);
	} /*tableexportoutlinescrap*/


static boolean tableexportwpscrap (const tyvaluerecord *scrapval, Handle *hexport) {
	
	/*
	convert the given table scrap value to a wp scrap
	*/
	
	tyvaluerecord val;
	register boolean fl;
	register hdlexternalvariable hv;
	register hdlwprecord hwp;
	
	*hexport = nil;
	
	if (langgetexternalvaluetype (scrapval) != wordvaluetype)
		return (false);
	
	fl = copyvaluerecord (*scrapval, &val) && copyvaluedata (&val);
	
	if (!fl)
		return (false);
	
	hv = (hdlexternalvariable) val.data.externalvalue;
	
	fl = wpverbinmemory (hv);
	
	if (fl) {
		
		hwp = (hdlwprecord) (**hv).variabledata;
		
		*hexport = (Handle) (**hwp).wpbuffer;
		
		(**hwp).wpbuffer = nil; /*we've consumed it*/
		}
	
	disposevaluerecord (val, false);
	
	return (fl);
	} /*tableexportwpscrap*/


boolean tableexportscrapvalue (const tyvaluerecord *scrapval, tyscraptype totype, Handle *hexport, boolean *fltempscrap) {
	
	*fltempscrap = true; /*usually we be the case*/
	
	switch (totype) {
		
		case hashscraptype:
			/*
			*** major problem: our refcons are not contiguous data. they 
			are pointers into the odb structure. currently, oppack doesn't 
			have a callback mechanism for packing refcon data. so we can't
			export a self-contained table scrap
			*/
			
			return (false);
		
		case textscraptype:
			return (tableexporttextscrap (scrapval, hexport));
		
		case opscraptype:
		case scriptscraptype:
		case menuscraptype:
			return (tableexportoutlinescrap (scrapval, hexport));
		
		case wpscraptype:
			return (tableexportwpscrap (scrapval, hexport));
		
		default:
			return (false);
		}
	} /*tableexportscrapvalue*/


static boolean tablegetvaluescrap (Handle hscrap, tyscraptype scraptype, tyvaluerecord *val) {
	
	Handle x;
	tyvaluetype valuetype = langgetvaluetype (scraptype);
	
	if (!copyhandle (hscrap, &x))
		return (false);
	
	if (!setbinaryvalue (x, scraptype, val))
		return (false);
	
	if (langgoodbinarytype (valuetype)) { /*unpack-able*/
		
		if (!coercevalue (val, valuetype))
			return (false);
		}
	
	return (true);
	} /*tablegetvaluescrap*/


static boolean tableexportvaluescrap (Handle hscrap, tyscraptype totype, Handle *hexport, boolean *fltempscrap) {
	
	/*
	3.0.2 dmb: new routine. if a foreign value is on the clipboard we 
	convert it to text
	*/
	
	tyscraptype scraptype;
	tyvaluerecord val;
	register boolean fl;
	
	if (totype != textscraptype)
		return (false);
	
	shellgetscrap (&hscrap, &scraptype); /*should be same value for hscrap*/
	
	disablelangerror ();
	
	fl = tablegetvaluescrap (hscrap, scraptype, &val);
	
	if (fl)
		fl = copyvaluerecord (val, &val) && coercetostring (&val);
	
	if (fl) {
		
		exemptfromtmpstack (&val);
		
		*fltempscrap = true;
			
		*hexport = val.data.stringvalue;
		}
	
	enablelangerror ();
	
	return (fl);
	} /*tableexportvaluescrap*/


static void tabledisposescrap (hdltablescrap hscrap) {
	
	disposehandle ((Handle) hscrap);
	} /*tabledisposescrap*/


boolean tablescraphook (Handle hscrap) {
	
	/*
	if our private type is on the external clipboard, set the internal 
	scrap to it.
	
	10/22/91 dmb: import any language value scrap type
	
	3.0.2 dmb: use new tableexportvaluescrap so we can paste foreign values 
	as text
	
	4.1b7 dmb: don't choose langvalue scrap types over text unless the 
	option key is down.
	*/
	
	tyvaluetype type;
	
	if (getscrap (hashscraptype, hscrap)) {
		
		hdloutlinerecord houtline;
		
		if (opunpackoutline (hscrap, &houtline))
			browsersetscrap (houtline);
		
		return (false); /*don't call any more hooks*/
		}
	
	if (!getscrap (textscraptype, nil) || keyboardstatus.floptionkey) { /*4.1b7 dmb*/
	
		for (type = charvaluetype; type < ctvaluetypes; ++type) {
			
			if (type != stringvaluetype) { /*leave TEXT for someone else*/
				
				OSType scraptype = langgettypeid (type);
				
				if (getscrap (scraptype, hscrap)) {
					
					shellsetscrap (hscrap, scraptype, &disposehandle, &tableexportvaluescrap);
					
					return (false); /*don't call any more hooks*/
					}
				}
			}
		}
	
	return (true); /*keep going*/
	} /*tablescraphook*/


#ifndef odbbrowser

boolean tablecopyroutine (void) {
	
	hdlhashtable htable;
	tyvaluerecord val;
	bigstring bsname;
	bigstring bspath;
	Handle hscrap;
	boolean fl;
	hdlhashnode hnode;
	
	if (tableeditsetglobals ())
		return (wpcopy ());
	
	if (!tablegetcursorinfo (&htable, bsname, &val, &hnode))
		return (false);
	
	langexternalgetquotedpath (htable, emptystring, bspath);
	
	pushchar ('.', bspath);
	
	langexternalbracketname (bsname);
	
	pushstring (bsname, bspath);
	
	langpusherrorcallback (nil, 0L);
	
	fl = langpackvalue (val, &hscrap, hnode);
	
	langpoperrorcallback ();
	
	if (!fl)
		return (false);
	
	if (!insertinhandle (hscrap, 0L, bspath, stringlength (bspath) + 1)) {
		
		disposehandle (hscrap);
		
		return (false);
		}
	
	shellsetscrap (hscrap, hashscraptype, &tabledisposescrap, &tableexportscrap);
	
	return (true);
	} /*tablecopyroutine*/

#endif

static boolean tableredoclear (hdlhashnode hnode, boolean flundo) {
	
	bigstring bsname;
	
	if (flundo) {
		
		hdlhashtable htable = tablegetlinkedhashtable ();
		
		tableexiteditmode ();
		
		if (!tablemovetonode (hnode)) /*error selecting node*/
			return (false);
		
		gethashkey (hnode, bsname);
		
		pushhashtable (htable);
		
		if (hashunlink (bsname, &hnode))
			pushundostep ((undocallback) tableundoclear, (Handle) hnode);
		
		pophashtable ();
		
		if (htable == agentstable)
			scriptremoveagent (hnode);
		}
	
	return (true);
	} /*tableredoclear*/


static boolean tableundoclear (hdlhashnode hnode, boolean flundo) {
	
	hdlhashtable htable = tablegetlinkedhashtable ();
	
	if (flundo) {
		
		tableexiteditmode ();
		
		tableoverridesort ((**hnode).sortedlink); /*retain original sort position*/
		
		hashinsertnode (hnode, htable);
		
		tablerestoresort ();
		
		tablemovetonode (hnode); /*select it*/
		
		if (htable == agentstable)
			scriptinstallagent (hnode);
		
		pushundostep ((undocallback) tableredoclear, (Handle) hnode);
		}
	else
		disposehashnode (htable, hnode, true, true);
	
	return (true);
	} /*tableundoclear*/


static boolean tableclosewindows (hdlhashnode hnode, ptrvoid refcon) {
	
	/*
	the indicated external value is being deleted.  close any windows 
	that contain descendants of the table
	*/
	
	hdlhashtable htable;
	tyvaluerecord val;
	hdlwindowinfo hinfo;
	
	val = (**hnode).val;
	
	if (val.valuetype != externalvaluetype) /*can't be in a window -- unwind recursion*/
		return (true);
	
	if (langexternalwindowopen (val, &hinfo))
		shellclosewindow ((**hinfo).macwindow);
	
	if (langexternalvaltotable (val, &htable, hnode))
		hashtablevisit (htable, &tableclosewindows, nil); /*daisy chain recursion*/
	
	return (true);
	} /*tableclosewindows*/
	

#ifndef odbbrowser

boolean tableclearroutine (void) {
	
	/*
	9/9/91 dmb: move barcursor up when deleting
	
	5.0a23 dmb: don't ask hashgetiteminfo for the value that we don't use
	*/
	
	register hdltableformats hf = tableformatsdata;
	bigstring bsname;
	hdlhashtable htable;
	hdlhashnode hnode;
	short row;
	
	if (tableeditsetglobals ()) {
		
		if (tableeditingemptycell ())
			tableeditleavecell ();
		else
			return (wpclear ());
		}
	
	pushundoaction (undoclearstring);
	
	htable = tablegetlinkedhashtable ();
	
	row = (**hf).rowcursor;
	
	if (!hashgetiteminfo (htable, row, bsname, nil))
		return (false);
	
	pushhashtable (htable);
	
	if (hashunlink (bsname, &hnode)) {
		
		pushundostep ((undocallback) tableundoclear, (Handle) hnode);
		
		tablebarcursoron (max (0, row - 1));
		
		tableclosewindows (hnode); /*if an external, make sure dependent windows are closed*/
		
		if (htable == agentstable) /*make sure agent stops running*/
			scriptremoveagent (hnode);
		
		tablevisicursor ();
		}
	
	pophashtable ();
	
	return (true);
	} /*tableclearroutine*/


boolean tablecutroutine (void) {
	
	if (tableeditsetglobals ()) {
		
		if (tableeditingemptycell ())
			tableeditleavecell ();
		else
			return (wpcut ());
		}
	
	pushundoaction (undocutstring);
	
	return (tablecopyroutine ()  &&  tableclearroutine ());
	} /*tablecutroutine*/

#endif

static void tablemakeuniquename (bigstring bsname, boolean *flchanged) {
	
	bigstring bscopy;
	bigstring bsunique;
	short n;
	
	for (n = 0; ; ++n) {
		
		copystring (bsname, bsunique);
		
		if (n > 0) {
			
			tablegetstringlist (copyofstring, bscopy);
			
			pushstring (bscopy, bsunique);
			
			*flchanged = true;
			}
		
		if (n > 1)
			pushint (n, bsunique);
		
		if (!hashsymbolexists (bsunique))
			break;
		}
	
	copystring (bsunique, bsname);
	} /*tablemakeuniquename*/


static boolean tableconfirmedinsert (bigstring bsname, tyvaluerecord val, boolean *flduplicate) {
	
	/*
	add the entry specified by [bsname, value] into the current hash table.
	
	if an entry with the same name already exists, confirm its impending 
	destruction with the user.
	
	5.0.2b12 dmb: use hashassign, not hashinsert. even though we know it 
	doesn't exist, we want the extra processing that it does.
	*/
	
	*flduplicate = false;
	
	if (hashsymbolexists (bsname)) {
	
		switch (replacevariabledialog (bsname)) {
			
			case 1: /*user clicked on Replace*/
				hashdelete (bsname, true, true);
				
				break;
				
			case 2: /*user clicked on Duplicate*/
				
				tablemakeuniquename (bsname, flduplicate);
				
				*flduplicate = true; /*should already be true, but let's be sure*/
				
				break;
				
			case 3: /*user clickded on Cancel*/
				return (false);
			} /*switch*/
		} /*symbol already defined*/
	
	hashassign (bsname, val);
	
	return (true);
	} /*tableconfirmedinsert*/


static boolean pastetobinary (Handle hscrap, OSType scraptype, tyvaluerecord *val) {
	
	/*
	3.0.2 dmb: if we can unpack the value, do so instead of pasting a binary
	*/
	
	if (!tablegetvaluescrap (hscrap, scraptype, val))
		return (false);

	exemptfromtmpstack (val);
	
	return (true);
	} /*pastetobinary*/


static boolean tablepastevalue (hdlhashtable htable, bigstring bsname, tyvaluerecord val, boolean flselectname) {
	
	/*
	6/13/91 dmb: seperated this code out as part of droppaste implementation
	*/
	
	register boolean fl;
	boolean flduplicate;
	
	pushhashtable (htable);
	
	fl = tableconfirmedinsert (bsname, val, &flduplicate);
	
	assert (tablevalidate (htable, true));
	
	if (!fl)
		disposevaluerecord (val, false);
	
	else {
		
		if (tablezoomtoname (htable, bsname)) { /*select newly-pasted cell*/
			
			if (shellpushfrontglobals ()) {
				
				hdlhashnode hnode;
				
				pushundoaction (undopastestring);
				
				if (hashlookupnode (bsname, &hnode))
					pushundostep ((undocallback) tableredoclear, (Handle) hnode);
				
				if (flselectname || flduplicate) { /*there was another item with the same name*/
				
					if (opsettextmode (true))
						opeditsetselection (0, infinity);
					}
				
				shellpopglobals ();
				}
			}
		}
	
	pophashtable ();
	
	return (fl);
	} /*tablepastevalue*/


#ifndef xxxodbbrowser

static boolean iseditscrap (void) {
	
	/*
	return true if the scrap could be pasted in text mode.  to save code, 
	we'll use op's test, whose logic will do just fine
	
	8/10/92 dmb: since table cells can contain more than 255 characters of 
	text, using op's logic for table values is too restrictive.  instead, 
	we'll say that anything that isn't a table element itself is eligable
	*/
	
	Handle hscrap;
	tyscraptype type;
	
	shellgetscrap (&hscrap, &type);
	
	if (type == hashscraptype)
		return (false);
	
	if ((**tableformatsdata).focuscol == valuecolumn)
		return (true);
	
	return (!isoutlinescrap ());
	} /*iseditscrap*/


boolean tablepasteroutine (void) {
	
	/*
	12/28/90 dmb: when pasting text, convert to bigstring if short enough
	
	12/7/91 dmb: now that strings can be huge, always paste text as a string
	
	12/17/91 dmb: select any made-up name
	*/
	
	tyvaluerecord val;
	hdlhashtable horigtable;
	bigstring bsname;
	hdlhashtable htable;
	tyscraptype scraptype;
	Handle hscrap;
	boolean flselectname = false;
	
	if (opeditsetglobals ()) {
		
		if (iseditscrap ())
			return (opeditpaste ());
		
		if (!tableexiteditmode ())
			return (false);
		}
	
	if (!shellgetscrap (&hscrap, &scraptype)) /*try to get the scrap, ignoring type*/
		return (false);
	
	if (!tablegetcursorinfo (&htable, bsname, nil, nil))
		return (false);
	
	setemptystring (bsname);
	
	switch (scraptype) {
		
		case hashscraptype:
			if (!tablegetscrapvalue (hscrap, &val, &horigtable, bsname))
				return (false);
			
			if (horigtable == htable) { /*value was copied from here*/
				
				pushhashtable (htable);
				
				tablemakeuniquename (bsname, &flselectname);
				
				pophashtable ();
				}
			
			break;
		
		case opscraptype:
			if (!langexternalnewvalue (idoutlineprocessor, hscrap, &val))
				return (false);
			
			break;
		
		case scriptscraptype:
			if (!langexternalnewvalue (idscriptprocessor, hscrap, &val))
				return (false);
			
			break;
		
		case menuscraptype:
			if (!langexternalnewvalue (idmenuprocessor, hscrap, &val))
				return (false);
			
			break;
		
		case wpscraptype:
			if (!langexternalnewvalue (idwordprocessor, hscrap, &val))
				return (false);
			
			break;
		
		case textscraptype: {
			Handle x;
			
			if (!copyhandle (hscrap, &x))
				return (false);
			
			if (!setheapvalue (x, stringvaluetype, &val))
				return (false);
			
			exemptfromtmpstack (&val);
			
			tablegetstringlist (pastedtextstring, bsname);
			
			flselectname = true;
			
			break;
			}
		
		case pictscraptype:
			if (!langexternalnewvalue (idpictprocessor, hscrap, &val))
				return (false);
			
			break;
		
		default:
			if (!pastetobinary (hscrap, scraptype, &val))
				return (false);
			
			break;
		}
	
	if (isemptystring (bsname)) {
		bigstring bspasted;
		
		hashgettypestring (val, bsname);
		
		tablegetstringlist (pastedstring, bspasted);
		
		insertstring (bspasted, bsname);
		
		flselectname = true;
		}
	
	return (tablepastevalue (htable, bsname, val, flselectname));
	} /*tablepasteroutine*/

#endif

boolean tabledroppasteroutine (void) {
	
	/*
	6/13/91 dmb: created this routine for pasthing into the main window
	
	pastes HASH scraps only, in their original database location
	
	12/5/91 dmb: fixed path construction on paste of new item into existing table
	*/
	
	tyvaluerecord val;
	bigstring bstable, bsname;
	hdlhashtable htable;
	tyscraptype scraptype;
	Handle hscrap;
	bigstring bsprompt;
	
	if (!shellgetscrap (&hscrap, &scraptype))
		return (false);
	
	if (scraptype != hashscraptype) {
		
		alertstring (cantpasteherestring);
		
		return (false);
		}
	
	if (!tablegetscrapvalue (hscrap, &val, &htable, bsname))
		return (false);
	
	langexternalbracketname (bsname);
	
	insertchar ('.', bsname);
	
	if (htable == nil) {
		
		insertstring (nameroottable, bsname);
		}
	else {
		
		langexternalgetquotedpath (htable, zerostring, bstable);
		
		insertstring (bstable, bsname);
		}
	
	tablegetstringlist (pasteasstring, bsprompt);
	
	if (askdialog (bsprompt, bsname)) {
		
		if (tableexpandtodotparams (bsname, &htable, bsname))
			return (tablepastevalue (htable, bsname, val, false));
		}
	
	disposevaluerecord (val, false);
	
	return (false);
	} /*tabledroppasteroutine*/



