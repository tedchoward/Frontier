
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
#include "dialogs.h"
#include "file.h"
#include "font.h"
#include "resources.h"
#include "ops.h"
#include "quickdraw.h"
#include "strings.h"
#include "langexternal.h"
#include "langinternal.h"
#include "langipc.h"
#include "tablestructure.h"
#include "tableinternal.h"
#include "shell.rsrc.h"
#include "odbinternal.h"
#ifdef isFrontier
	#include "shellprivate.h"
#endif
#include "timedate.h"
#include "byteorder.h"	/* 2006-04-08 aradke: endianness conversion macros */

#pragma pack(2)
typedef struct tycancoonrecord { /*one of these for every cancoon file that's open*/

	hdldatabaserecord hdatabase; /*db.c's record*/
	
	hdlhashtable hroottable; /*the root symbol table for this file*/
	
	Handle hrootvariable; /*the variable record for the root symbol table*/
	
	hdltablestack htablestack;
	
	boolean accesssing;
	
	#ifdef isFrontier
	
	WindowPtr shellwindow;
	
	#endif
	} tycancoonrecord, *ptrcancoonrecord, **hdlcancoonrecord;
#pragma options align=reset

#define cancoonversionnumber 0x03


#ifndef isFrontier

boolean fldatabasesaveas;

static byte canthandlethistypeerror [] = "\x43" "This version of ODB Engine can not get or set objects of this type.";

#endif


#define ctwindowinfo 6 /*number of windowinfo records saved in each cancoon record*/
#pragma pack(2)
typedef struct tycancoonwindowinfo { /*lives both in memory and on disk*/	
	
	Rect windowrect;
	
	diskfontstring fontname; /*only maintained on disk*/
	
	short fontnum; /*only valid when it's in memory*/
	
	short fontsize, fontstyle;
	
	WindowPtr w; /*only valid when it's in memory*/
	
	char waste [10];
	} tycancoonwindowinfo;
#pragma options align=reset

#pragma pack(2)
typedef struct tyversion2cancoonrecord {
	
	short versionnumber;
	
	dbaddress adrroottable;
	
	tycancoonwindowinfo windowinfo [ctwindowinfo];
	
	dbaddress adrscriptstring; /*the string that appears in the quickscript window*/
	
	unsigned short flags;
	
	short ixprimaryagent;
	
	short waste [28]; /*room to grow*/
	} tyversion2cancoonrecord;
#pragma options align=reset

	#define flflagdisabled_mask 0x8000 /*hide the flag?*/
	#define flpopupdisabled_mask 0x4000 /*hide the agents popup menu?*/
	#define flbigwindow_mask 0x2000 /*is the flag toggled to the big window state?*/


static bigstring bserror = "\0";

static hdlcancoonrecord cancoonglobals = nil;

#ifndef isFrontier

boolean alertdialog (bigstring bs) {
	
	copystring (bs, bserror);
	
	return (true);
	} /*alertdialog*/

#endif

static boolean disposecancoonrecord (hdlcancoonrecord hcancoon) {
	
	/*
	5.0.1 dmb: fixed leak - use tabledisposetable, not disposehashtable
	*/
	
	hdlcancoonrecord hc = hcancoon;
	
	if (hc == nil)
		return (false);
	
	if (!(**hc).accesssing) {
		
		cleartablestructureglobals ();	/*do this now to avoid debug check in disposehashtable*/
		
		tabledisposetable ((**hc).hroottable, false); /*yup, checks for nil*/
		
		disposehandle ((Handle) (**hc).hrootvariable); /*1.0b2 dmb: site of memory leak*/
		}
	
	disposehandle ((Handle) (**hc).htablestack);
	
	disposehandle ((Handle) hc);
	
	return (true);
	} /*disposecancoonrecord*/


static boolean newcancoonrecord (hdlcancoonrecord *hcancoon) {
	
	hdlcancoonrecord hc;
	hdltablestack htablestack;
	
	if (!newclearhandle (sizeof (tycancoonrecord), (Handle *) hcancoon))
		return (false);
	
	hc = *hcancoon;
	
	if (!newclearhandle (sizeof (tytablestack), (Handle *) &htablestack)) {
		
		disposehandle ((Handle) hc);
		
		return (false);
		}
	
	(**hc).htablestack = htablestack;

	return (true);
	} /*newcancoonrecord*/


static boolean ccloadsystemtable (hdlcancoonrecord hcancoon, dbaddress adr) {
	
	/*
	1.0b2 dmb: we don't rely on the system table, so don't force 
	its creation by calling settablestructureglobals.
	*/
	
	hdlcancoonrecord hc = hcancoon;
	boolean fl;
	Handle hvariable;
	hdlhashtable htable;
	
	hashtablestack = (**hc).htablestack;
	
	fl = tableloadsystemtable (adr, &hvariable, &htable, false);
	
	if (fl) {
		
		cleartablestructureglobals ();
		
		(**hc).hrootvariable = rootvariable = hvariable;
		
		(**hc).hroottable = roottable = htable;
		
		assert (tablevalidate (htable, true));
		
		/*
		fl = settablestructureglobals (hvariable, true);
		*/
		
		currenthashtable = roottable;
		}
	
	return (fl);
	} /*ccloadsystemtable*/


static boolean odberrorroutine (bigstring bs, ptrvoid refcon) {
#pragma unused (refcon)

	copystring (bs, bserror);
	
	return (false); /*consume the error*/
	} /*odberrorroutine*/


static void setcancoonglobals (hdlcancoonrecord hcancoon) {
	
	hdlcancoonrecord hc = hcancoon;
	
#ifndef isFrontier

	if (hc != cancoonglobals) 
	
#endif
		{
		databasedata = (**hc).hdatabase;
		
		hashtablestack = (**hc).htablestack;
		
		settablestructureglobals ((**hc).hrootvariable, false);
		
		currenthashtable = roottable;
		
		cancoonglobals = hc; /*this global is independent of shellpush/popglobals*/
		
		langcallbacks.errormessagecallback = &odberrorroutine;
		}
	} /*setcancoonglobals*/


static void clearcancoonglobals (void) {
	
	databasedata = nil; /*db.c*/
	
	cleartablestructureglobals (); /*tablestructure.c: roottable, handlertable, etc.*/
	
	currenthashtable = nil;
	
	cancoonglobals = nil; /*our very own superglobal*/
	
	hashtablestack = nil;
	} /*clearcancoonglobals*/


static boolean loadversion2cancoonfile (dbaddress adr, hdlcancoonrecord hcancoon) {
	
	tyversion2cancoonrecord info;
	
	if (!dbreference (adr, sizeof (info), &info))
		return (false);
	
	/* only variable used here! */
	disktomemlong (info.adrroottable);

	if (!ccloadsystemtable (hcancoon, info.adrroottable))
		return (false);
	
	assert ((**cancoonglobals).hroottable == roottable); /*should already be set up*/
	
	return (true);
	} /*loadversion2cancoonfile*/


#ifndef isFrontier

	static short tablecomparenames (hdlhashtable ht, hdlhashnode hnode1, hdlhashnode hnode2) {

		bigstring bs1, bs2;
		
		gethashkey (hnode1, bs1);

		gethashkey (hnode2, bs2);
		
		alllower (bs1); /*comparison is unicase*/
		
		alllower (bs2);
		
		return (comparestrings (bs1, bs2));
		} /*tablecomparenames*/


	static boolean odbtabledirty (hdlhashtable htable, const bigstring bsname) {
		
	//	hdltablediskrecord hf = (hdltablediskrecord) (**htable).hashtableformats;
		
	//	if (hf != nil)
		(**htable).timelastsave = timenow (); /*modification time until saved*/
		
		return (true);
		} /*tabledirty*/

	static boolean odbsymbolchanged (hdlhashtable htable, const bigstring bsname, hdlhashnode hnode, boolean flvalue) {
		
		return (odbtabledirty (htable, bsname));
		} /*odbsymbolchanged*/

	static boolean odbsymbolunlinking (hdlhashtable ht, hdlhashnode hn) {

		return (true);
		} /*odbsymbolunlinking*/


	static void initlangcallbacks (void) {

		langcallbacks.symbolchangedcallback = &odbsymbolchanged; 
		
		langcallbacks.symboldeletedcallback = &odbtabledirty;
		
		langcallbacks.symbolinsertedcallback = &odbtabledirty;
		
		langcallbacks.symbolunlinkingcallback = &odbsymbolunlinking;
		
		langcallbacks.comparenodescallback = &tablecomparenames;
		}  /*initlangcallbacks*/

	#define odbexpandtodotparams(bs, htable, bsname) langexpandtodotparams(bs, htable, bsname)

#else

	static boolean odbexpandtodotparams (bigstring bs, hdlhashtable *htable, bigstring bsname) {
		
		/*
		the odbengine version of langexpandtodotparams gaurantees that htable 
		will be non-nil. the frontier version doesn't. if we have just a name, 
		we interpret it as a root-level item
		*/
		
		boolean fl;
		
		disablelangerror ();
		
		fl = langexpandtodotparams (bs, htable, bsname);
		
		enablelangerror ();
		
		if (!fl) {
			
			langparamerror (addresscoerceerror, bs);
			
			return (false);
			}
		
		if (*htable == nil)
			langsearchpathlookup (bsname, htable); /*always sets htable*/
		
		return (true);
		} /*odbexpandtodotparams*/

#endif


static boolean odbvaltotable (tyvaluerecord val, hdlhashtable *htable, hdlhashnode hnode) {
	
	if (!langexternalvaltotable (val, htable, hnode)) {
		
		bigstring bs;
		
		getstringlist (tableerrorlist, namenottableerror, bs);
		
		langerrormessage (bs);
		
		return (false);
		}
	
	return (true);		
	} /*odbvaltotable*/

#ifdef isFrontier
pascal boolean odbUpdateOdbref (WindowPtr w, odbref odb) {
	hdlcancoonrecord hc;
	
	setemptystring (bserror);
	
	hc = (hdlcancoonrecord) odb;
	
	if (w != NULL) {
		shellpushglobals (w);
	
		(*shellglobals.setsuperglobalsroutine) ();
		}
	
	(**hc).hdatabase = databasedata; /*current database*/
	
	(**hc).hroottable = roottable; /*current root table*/
	
	(**hc).hrootvariable = rootvariable; /*current root variable*/
	
	(**hc).shellwindow = shellwindow;
	
	(**hc).accesssing = true;
		
	if (w != NULL) {
		shellpopglobals ();
	
		(*shellglobals.setsuperglobalsroutine) ();	
		}
	
	return (true);
	} /*odbUpdateOdbref*/

pascal boolean odbAccessWindow (WindowPtr w, odbref *odb) {
	
	/*
	4.1b5 dmb: new entrypoint
	
	for the db verbs withing frontier, we need to be able to "open" a 
	database that is already open. We keep our own hashtablestack; that's
	our context. But the root table and variable and the database are just 
	borrowed from the given root window.
	*/
		
	setemptystring (bserror);
	
	if (!newcancoonrecord (&cancoonglobals))
		return (false);
	
	*odb = (odbref) cancoonglobals;
	
	odbUpdateOdbref (w, *odb);
	return (true);
	} /*odbAccess*/

#endif


pascal boolean odbNewFile (hdlfilenum fnum) {
	
	/*
	4.1b5 dmb: new routine. minimal db creation. does not leave it open
	*/
	
	tyversion2cancoonrecord info;
	dbaddress adr = nildbaddress;
	boolean fl;
	
	setemptystring (bserror);
	
	if (!dbnew (fnum))
		return (false);
	
	clearbytes (&info, sizeof (info));
	
	info.versionnumber = conditionalshortswap (cancoonversionnumber);
	
	fl = dbassign (&adr, sizeof (info), &info);
	
	if (fl) {
		
		dbsetview (cancoonview, adr);
		
		dbclose ();
		}
	
	cancoonglobals = nil;	/*if they've been set, they're out of date*/
	
	dbdispose ();
	
	return (fl);
	} /*odbNewFile*/


pascal boolean odbOpenFile (hdlfilenum fnum, odbref *odb, boolean flreadonly) {
	
	hdlcancoonrecord hc = nil;
	dbaddress adr;
	short versionnumber;
	
	setemptystring (bserror);
	
	#ifndef isFrontier
	
		initlangcallbacks ();
	
	#endif
	
	if (!dbopenfile (fnum, flreadonly))
		return (false);
	
	dbgetview (cancoonview, &adr);
	
	if (!dbreference (adr, sizeof (versionnumber), &versionnumber))
		goto error;
	
	disktomemshort (versionnumber);
	
	if (!newcancoonrecord (&cancoonglobals))
		goto error;
	
	hc = cancoonglobals;
	
	(**hc).hdatabase = databasedata; /*result from dbopenfile*/
	
	switch (versionnumber) {
		
		case 2:
		case cancoonversionnumber:
			if (!loadversion2cancoonfile (adr, hc))
				goto error;
			
			*odb = (odbref) hc;
			
			return (true);
			
		default:
			alertdialog ((ptrstring) "\x59" "The version number of this database file is not recognized by this version of Frontier.");
			
			goto error;
		} /*switch*/
	
	error:
	
	dbdispose ();
	
	disposecancoonrecord (hc); /*checks for nil*/
	
	clearcancoonglobals ();
	
	return (false);
	} /*odbOpenFile*/


pascal boolean odbSaveFile (odbref odb) {
	
	hdlcancoonrecord hc = (hdlcancoonrecord) odb;
	tyversion2cancoonrecord info;
	dbaddress adr;
	
	setemptystring (bserror);
	
	setcancoonglobals (hc);
	
	#ifdef isFrontier
	
	if ((**hc).accesssing) {
		
		return (shellsave ((**hc).shellwindow));
		}
	
	#endif
	
	dbgetview (cancoonview, &adr);
	
	if (adr == nildbaddress)
		return (false);
	
	if (!dbreference (adr, sizeof (info), &info))
		return (false); 
	
	info.versionnumber = conditionalshortswap (cancoonversionnumber);
	
	if (!tablesavesystemtable ((**hc).hrootvariable, &info.adrroottable))
		return (false);
	
	memtodisklong (info.adrroottable);

	clearbytes (&info.waste, sizeof (info.waste));
	
	if (!dbassign (&adr, sizeof (info), &info))
		return (false);
	
	dbflushreleasestack (); /*release all the db objects that were saved up*/
	
	dbsetview (cancoonview, adr);
	
	return (true);
	} /*odbSaveFile*/


pascal boolean odbCloseFile (odbref odb) {
	
	/*
	1/22/91 dmb: added scan of new ccglobalsstack
	
	2/26/93 dmb: support shutdown scripts
	*/
	
	hdlcancoonrecord hc = (hdlcancoonrecord) odb;
	
	setemptystring (bserror);
	
	if (hc == nil) /*nothing to do*/
		return (true);
	
	setcancoonglobals (hc);
	
	if (!(**hc).accesssing)
		dbdispose (); /*do before clearing globals -- depends on databasedata*/
	
	disposecancoonrecord (hc);
	
	clearcancoonglobals ();
	
	return (true);
	} /*odbCloseFile*/


pascal boolean odbDefined (odbref odb, bigstring bspath) {

	/*
	4.1b5 dmb: new routine
	*/
	
	hdlhashtable htable;
	bigstring bsname;
	boolean fl;
	
	setemptystring (bserror);
	
	setcancoonglobals ((hdlcancoonrecord) odb);
	
	disablelangerror ();
	
	fl = odbexpandtodotparams (bspath, &htable, bsname);
	
	enablelangerror ();
	
	if (fl) {
		
		pushhashtable (htable);
		
		fl = hashsymbolexists (bsname);
		
		pophashtable ();
		}
	
	return (fl);
	} /*odbDefined*/


pascal boolean odbDelete (odbref odb, bigstring bspath) {

	hdlhashtable htable;
	bigstring bsname;
	
	setemptystring (bserror);
	
	setcancoonglobals ((hdlcancoonrecord) odb);
	
	if (!odbexpandtodotparams (bspath, &htable, bsname))
		return (false);
	
	return (hashtabledelete (htable, bsname));
	} /*odbDelete*/


pascal boolean odbGetType (odbref odb, bigstring bspath, OSType *odbType) {
	
	hdlhashtable htable;
	bigstring bsname;
	tyvaluerecord val;
	hdlhashnode hnode;
	
	setemptystring (bserror);
	
	setcancoonglobals ((hdlcancoonrecord) odb);
	
	if (!odbexpandtodotparams (bspath, &htable, bsname))
		return (false);
	
	if (!langsymbolreference (htable, bsname, &val, &hnode))
		return (false);
	
	if (val.valuetype == binaryvaluetype)
		*odbType = getbinarytypeid (val.data.binaryvalue);
	else
		*odbType = langexternalgettypeid (val);
	
	return (true);
	} /*odbGetType*/


pascal boolean odbGetValue (odbref odb, bigstring bspath, odbValueRecord *value) {

	hdlhashtable htable;
	bigstring bsname;
	tyvaluerecord val;
	hdlhashnode hnode;
	
	setemptystring (bserror);
	
	setcancoonglobals ((hdlcancoonrecord) odb);
	
	if (!odbexpandtodotparams (bspath, &htable, bsname))
		return (false);
	
	if (!langsymbolreference (htable, bsname, &val, &hnode))
		return (false);
	
	#ifndef isFrontier

	if (val.valuetype == externalvaluetype) {
		
		langerrormessage (canthandlethistypeerror);
		
		return (false);
		}
	
	#endif
	
	if (!copyvaluerecord (val, &val))
		return (false);
	
	if (!copyvaluedata (&val))
		return (false);
	
	exemptfromtmpstack (&val);
	
	(*value).valuetype = (odbValueType) langexternalgettypeid (val);
	
	/*
	if (val.valuetype == binaryvaluetype)
		pullfromhandle (val.data.binaryvalue, 0L, sizeof (typeid), &(*value).valuetype);
	*/
	
	(*value).data.binaryvalue = val.data.binaryvalue; /*largest field covers everything*/
	
	return (true);
	} /*odbGetValue*/


pascal boolean odbSetValue (odbref odb, bigstring bspath, odbValueRecord *value) {

	hdlhashtable htable;
	bigstring bsname;
	tyvaluerecord val;
	tyvaluetype type = langexternalgetvaluetype (value->valuetype);
	
	setemptystring (bserror);
	
	#ifndef isFrontier
	
	if (type == externalvaluetype) {
		
		langerrormessage (canthandlethistypeerror);
		
		return (false);
		}
	
	#endif
	
	setcancoonglobals ((hdlcancoonrecord) odb);
	
	if (!odbexpandtodotparams (bspath, &htable, bsname))
		return (false);
	
	if (type == (tyvaluetype) -1) {
	
		if (!setbinaryvalue (value->data.binaryvalue, value->valuetype, &val))
			return (false);
		}
	else {
	
		initvalue (&val, type);
		
		val.data.binaryvalue = value->data.binaryvalue;
		}
	
	if (!copyvaluerecord (val, &val))
		return (false);
	
	if (!hashtableassign (htable, bsname, val)) {
	
		disposevaluerecord (val, true);
		
		return (false);
		}
	
	exemptfromtmpstack (&val);
	
	return (true);	
	} /*odbSetValue*/


pascal boolean odbNewTable (odbref odb, bigstring bspath) {

	hdlhashtable htable;
	bigstring bsname;
	tyvaluerecord val;
	
	setemptystring (bserror);
	
	setcancoonglobals ((hdlcancoonrecord) odb);
	
	if (!odbexpandtodotparams (bspath, &htable, bsname))
		return (false);
	
	if (!langexternalnewvalue (idtableprocessor, nil, &val))
		return (false);
	
	if (!hashtableassign (htable, bsname, val)) {
	
		disposevaluerecord (val, true);
		
		return (false);
		}
	
	exemptfromtmpstack (&val);
	
	return (true);	
	} /*odbNewTable*/


pascal boolean odbCountItems (odbref odb, bigstring bspath, long *count) {

	hdlhashtable htable;
	bigstring bsname;
	tyvaluerecord val;
	long ctitems;
	hdlhashnode hnode;
	
	setemptystring (bserror);
	
	setcancoonglobals ((hdlcancoonrecord) odb);
	
	if (!odbexpandtodotparams (bspath, &htable, bsname))
		return (false);
	
	if (!langsymbolreference (htable, bsname, &val, &hnode))
		return (false);
	
	if (!odbvaltotable (val, &htable, hnode))
		return (false);
	
	if (!hashcountitems (htable, &ctitems))
		return (false);
	
	*count = ctitems;
	
	return (true);
	} /*odbCountItems*/


pascal boolean odbGetNthItem (odbref odb, bigstring bspath, long n, bigstring bsname) {

	hdlhashtable htable;
	tyvaluerecord val;
	hdlhashnode hnode;
	
	setemptystring (bserror);
	
	setcancoonglobals ((hdlcancoonrecord) odb);
	
	if (!odbexpandtodotparams (bspath, &htable, bsname))
		return (false);
	
	if (!langsymbolreference (htable, bsname, &val, &hnode))
		return (false);
	
	if (!odbvaltotable (val, &htable, hnode))
		return (false);
	
	if (!hashgetiteminfo (htable, (short) (n - 1), bsname, nil))
		return (false);
	
	return (true);
	} /*odbGetNthItem*/



pascal boolean odbGetModDate (odbref odb, bigstring bspath, unsigned long *date) {

	hdlhashtable htable;
	bigstring bsname;
	tyvaluerecord val;
	hdlhashnode hnode;
	
	setemptystring (bserror);
	
	setcancoonglobals ((hdlcancoonrecord) odb);
	
	if (!odbexpandtodotparams (bspath, &htable, bsname))
		return (false);
	
	if (!langsymbolreference (htable, bsname, &val, &hnode))
		return (false);
	
	if (!odbvaltotable (val, &htable, hnode))
		return (false);
	
	*date = (**htable).timelastsave;
	
	return (true);
	} /*odbGetModDate*/


pascal void odbDisposeValue (odbref odb, odbValueRecord *value) {
	
	tyvaluetype type;
	tyvaluerecord val;
	
	setemptystring (bserror);
	
	setcancoonglobals ((hdlcancoonrecord) odb);
	
	type = langexternalgetvaluetype ((OSType) (*value).valuetype);
	
	if (type == -1)	/*no match; must have been a binary value*/
		type = binaryvaluetype;
	
	initvalue (&val, type);
	
	val.data.binaryvalue = (*value).data.binaryvalue;
	
	disposevaluerecord (val, false);
	} /*odbDisposeValue*/


pascal void odbGetError (bigstring bs) {

	copystring (bserror, bs);
	} /*odbGetError*/



