
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
#include "strings.h"
#include "timedate.h"
#include "resources.h"
#include "langinternal.h"
#include "langexternal.h"
#include "tableinternal.h"
#include "tableverbs.h"
#include "tablestructure.h"


/*
the routines in this file determine the overall structure of the system
symbol tables in CanCoon.
*/



byte nameinternaltable [] = STR_compiler;

byte namemenubar [] = STR_menubar;

byte namebuiltinstable [] = STR_builtins;

byte namepathstable [] = STR_paths;

byte nameverbstable [] = STR_verbs;

byte nameiacgluetable [] = STR_apps;

byte nameiachandlertable [] = STR_traps;

byte nameagentstable [] = STR_agents;

byte nameresourcestable [] = STR_misc;

byte nameefptable [] = STR_kernel;

byte namelangtable [] = STR_language;

byte namestacktable [] = STR_stack;

byte namesemaphoretable [] = STR_semaphores; /*4.0b7 dmb*/

byte namethreadtable [] = STR_threads;	/* 4.0.1b1 dmb*/

byte namefilewindowtable [] = STR_filewindows; // 5.0d16 dmb

byte nameroottable [] = STR_root;

byte namestartuptable [] = STR_startup;

byte namesuspendtable [] = STR_suspend;

byte nameresumetable [] = STR_resume;

byte nameshutdowntable [] = STR_shutdown;

byte namesystembranch [] = STR_system;

byte namemenubartable [] = STR_menubars;

static byte namemacintoshtable [] = STR_macintosh;

static byte nameobjectmodeltable [] = STR_objectmodel;

static byte nametemptable [] = STR_temp;

byte nameenvironmenttable [] = STR_environment;



Handle rootvariable = nil;

hdlhashtable roottable = nil;

hdlhashtable systemtable = nil;

hdlhashtable internaltable = nil;

hdlhashtable efptable = nil;

hdlhashtable langtable = nil;

hdlhashtable runtimestacktable = nil;

hdlhashtable semaphoretable = nil;

hdlhashtable threadtable = nil;

hdlhashtable filewindowtable = nil;

hdlhashtable builtinstable = nil;

hdlhashtable pathstable = nil;

hdlhashtable verbstable = nil;

hdlhashtable iacgluetable = nil;

hdlhashtable iachandlertable = nil;

hdlhashtable resourcestable = nil;

hdlhashtable agentstable = nil;

/*
hdlhashtable usertable = nil;
*/

hdlhashtable menubartable = nil;

hdlhashtable objectmodeltable = nil;

hdlhashtable environmenttable = nil;



boolean getsystemtablescript (short idscript, bigstring bsscript) {
	
	return (getstringlist (idsystemtablescripts, idscript, bsscript));
	} /*getsystemtablescript*/


static boolean checktable (hdlhashtable htable, bigstring bs, boolean flcreate, hdlhashtable *hsubtable) {
	
	/*
	locate the table named bs in the htable.  return in hsubtable the table you're 
	looking for.  if flcreate is true we create the table if it doesn't exist.
	
	return false if we couldn't find or create the table.
	
	3/4/91 dmb: if we create a new table, look for a packed hash resource 
	with a matching named, and try to unpack the values
	*/
	
	#ifdef MACVERSION
		Handle hpacked;
	#endif

	register hdlhashtable *ht = hsubtable;
	
	if (findnamedtable (htable, bs, ht)) /*no problem, it exists*/
		goto exit;
	
	*ht = nil;
	
	if (!flcreate)
		return (false);
	
	if (!tablenewsubtable (htable, bs, ht))
		return (false);
	
	#if MACVERSION && !defined (odbengine)
	
	hpacked = filegetresource (filegetapplicationrnum (), 'HASH', 0, bs);
	
	if (hpacked != nil) { /*try unpacking from resource*/
		
		DetachResource (hpacked);
		
		hashunpacktable (hpacked, true, *ht); /*he always disposes of hpackedtable*/
		}
	
	#endif
	
	exit:
	
	#ifdef smartmemory
	
	(***ht).fllocked = true; /*currently only respected by purgetable code in langhash.c*/
	
	#endif
	
	return (true);
	} /*checktable*/


static boolean linksystemtable (hdlhashtable hsystem, bigstring bstable, hdlhashtable htable) {
	
	/*
	link htable into the system table using the given name
	*/

	Handle hdata = (Handle) (**htable).hashtablerefcon;
	
	if (!langsetexternalsymbol (hsystem, bstable, idtableprocessor, hdata))
		return (false);
	
	langexternaldontsave (hsystem, bstable);
	
	return (true);
	} /*linksystemtable*/


boolean linksystemtablestructure (hdlhashtable hroot) {
	
	/*
	have a look at the system table -- there's a table called compiler.
	
	it's assumed to be read-only, so it can be linked into every open cancoon
	file.  this routine links the variable that represents the compiler table
	into the system table of the root table passed in as a parameter.
	
	3/25/91 dmb: use checktable instead of findnamedtable to find the system 
	table so that one will be created if it's missing.  this allows us to 
	change the name of the system table without breaking old files.
	
	5.1 dmb: create the temp table (not shared between roots)
	
	5.1.4 dmb: do the environment table, break out linksystemtable code.
	
	the environment table is created in langstartup, as is the compiler table
	*/
	
	Handle hdata = (Handle) (**internaltable).hashtablerefcon;
	hdlhashtable hsystem, htemp;
	
	// make sure the system table exists
	if (!checktable (hroot, namesystembranch, true, &hsystem))
		return (false);
	
	// link in the compiler table, not part of saved structure
	if (!linksystemtable (hsystem, nameinternaltable, internaltable))
		return (false);
	
	// link in the environment table, not part of saved structure
	if (!linksystemtable (hsystem, nameenvironmenttable, environmenttable))
		return (false);
	
	// create the temp table, not part of saved structure
	if (checktable (hsystem, nametemptable, true, &htemp))
		langexternaldontsave (hsystem, nametemptable);
	
	return (true);
	} /*linksystemtablestructure*/


boolean unlinksystemtablestructure (void) {

	/*
	try extracting system table from root before disposal.
	*/
	
	pushhashtable (systemtable);
	
	hashdelete (nameinternaltable, false, false);
	
	pophashtable ();
	
	return (true);
	} /*unlinksystemtablestructure*/


boolean tablenewtable (hdltablevariable *hvariable, hdlhashtable *htable) {
	
	/*
	create a new hashtable variable with an empty hashtable linked into it.
	*/
	
	register hdltablevariable hv;
	register hdlhashtable ht;
	
	if (!newtablevariable (true, 0L, hvariable, false))
		return (false);
	
	hv = *hvariable; /*copy into register*/
	
	if (!newhashtable (htable)) {
		
		disposehandle ((Handle) hv);
		
		return (false);
		}
	
	ht = *htable; /*copy into register*/
	
	(**ht).timecreated = (**ht).timelastsave = timenow (); // 5.0a23 dmb
	
	/*leave the hashtableformats record unallocated, handle == nil*/
	
	(**hv).variabledata = (long) ht; /*link the table into the variable rec*/
	
	(**ht).hashtablerefcon = (long) hv; /*the pointing is mutual*/
	
	(**ht).fldirty = true; /*it's never been saved*/
	
	return (true);
	} /*tablenewtable*/


boolean tablenewsubtable (hdlhashtable htable, bigstring bsname, hdlhashtable *hnewtable) {
	
	/*
	create a new hashtable in the table indicated by htable, and return a
	handle to the hashtable record.
	
	the caller can load it full of stuff after we allocate it.  we basically
	just rely on the langhash.c to make the new table, and we link it into the 
	global table.
	
	1/23/91: don't set the dontsave bit anymore.  callers that want it set 
	should be using tablenewsystemtable.
	*/
	
	hdltablevariable hvariable;
	
	if (!tablenewtable (&hvariable, hnewtable))
		return (false);
	
	if (!langsetexternalsymbol (htable, bsname, idtableprocessor, (Handle) hvariable)) {
		
		tableverbdispose ((hdlexternalvariable) hvariable, false);
		
		return (false);
		}
	
	#if !flruntime
	
	(***hnewtable).parenthashtable = htable; /*retain parental link*/
	
	#endif
	
	return (true);
	} /*tablenewsubtable*/


boolean tablenewsystemtable (hdlhashtable htable, bigstring bs, hdlhashtable *hnewtable) {
	
	register hdlhashtable ht;
	register hdltablevariable hv;
	
	if (!tablenewsubtable (htable, bs, hnewtable))
		return (false);
	
	langexternaldontsave (htable, bs); /*set the don't-save bit in its hashnode*/
	
	ht = *hnewtable; /*copy into register*/
	
	hv = (hdltablevariable) (**ht).hashtablerefcon;
	
	(**hv).flsystemtable = true;
	
	return (true);
	} /*tablenewsystemtable*/


boolean tableloadsystemtable (dbaddress adr, Handle *hvariable, hdlhashtable *htable, boolean flcreate) {
	
	/*
	gets things started -- use this routine to load the initial hierarchic symbol
	table.
	
	create a new table variable, not in memory, with address equal to the indicated
	dbaddress.  then load it into memory and return a handle to the variable record.
	table.
	
	also return the handle to the hash table from the variable record.  we don't want
	to pollute the call with any knowledge of the format of a table variable record.
	
	6/11/90 DW: merge the contents of the root hashtable into the new table.  at this
	point it contains the handlers implemented in wpverbs.c, opverbs.c, fileverbs.c,
	etc.
	
	6/15/90 DW: if adr is 0, we just allocate an empty table and return it.
	
	10/5/90 dmb: no longer merge w/root table.  instead, call linksystemtablestructure
	
	1/21/91 dmb: create SystemLand table when starting with empty table.  also, 
	don't set roottable here.  leave that to caller
	*/
	
	register hdlexternalvariable hv;
	register hdlhashtable ht;
	hdlhashtable hsubtable;
	
	assert (sizeof (tyexternalvariable) == sizeof (tytablevariable));
	
	if (adr == nildbaddress) { /*start an empty table*/
		
		if (!tablenewtable ((hdltablevariable *) hvariable, htable)) /*this will be the root table*/
			return (false);
		
		hv = (hdlexternalvariable) *hvariable;
		
		if (flcreate && !tablenewsubtable (*htable, namesystembranch, &hsubtable)) {
			
			tableverbdispose (hv, true);
			
			return (false);
			}
		}
	else {
		
		if (!newtablevariable (false, adr, (hdltablevariable *) hvariable, false))
			return (false);
		
		hv = (hdlexternalvariable) *hvariable;
		
		if (!tableverbinmemory (hv, HNoNode)) {
			
			disposehandle ((Handle) hv);
			
			return (false);
			}
		}
	
	ht = (hdlhashtable) (**hv).variabledata;
	
	#if !odbengine && !version5orgreater
		
	if (!linksystemtablestructure (ht)) {
		
		tabledisposetable (ht, false);
		
		disposehandle ((Handle) hv);
		
		return (false);
		}

	#endif
	
	(**hv).id = idtableprocessor; /*so we can make a value out of this variable*/
	
	*htable = ht;
	
	return (true);
	} /*tableloadsystemtable*/


boolean tablesavesystemtable (Handle hvariable, dbaddress *adr) {
	
	/*
	saves out the root symbol table.  adr should be set to the address it was
	last saved at, we re-use the space if the new table will fit in it, otherwise
	adr returns with a new block, and the old one is released.
	
	dmb 10/2/90: don't rely on oldaddress; not set during Save As.  grab address 
	from handle instead.
	
	5.1.3 dmb: fancier error reporting
	*/
	
	register hdlexternalvariable hv = (hdlexternalvariable) hvariable;
	langerrormessagecallback savecallback;
	ptrvoid saverefcon;
	bigstring bspackerror;
	register boolean fl;
	Handle htmp;
	boolean fldummy;
	
	if (!newemptyhandle (&htmp)) 
		return (false);
	
	if (!flscriptrunning)
		langhookerrors ();
		
	tablepreflightsubsdirtyflag (hv); //6.2a15 AR
	
	langtraperrors (bspackerror, &savecallback, &saverefcon);
	
	fl = tableverbpack (hv, &htmp, &fldummy); /*packs table, saves to db if neccessary, pushes address on htmp*/
	
	languntraperrors (savecallback, saverefcon, !fl);
	
	if (!flscriptrunning)
		langunhookerrors ();
	
	popfromhandle (htmp, sizeof (dbaddress), adr);
	
#ifdef PACKFLIPPED
	longswap (*adr); // un-swap it; tableverbpack swapped it
#endif
	
	disposehandle (htmp); /*we can get the address from the variable record, below*/
	
	if (!fl) {
		
		fllangerror = false;
		
		setstringcharacter (bspackerror, 0, getlower (getstringcharacter (bspackerror, 0)));
		
		poptrailingchars (bspackerror, '.');
		
		if (flscriptrunning)
			langparamerror (tablesavingerror, bspackerror);
		
		else {
			bigstring bs;
			
			getstringlist (langerrorlist, tablesavingerror, bs);
			
			parsedialogstring (bs, bspackerror, nil, nil, nil, bs);
			
			shellerrormessage (bs);
			}
		}
	/*
	*adr = (**hv).oldaddress;
	*/
	
	return (fl);
	} /*tablesavesystemtable*/


static boolean checktablestructure (boolean flcreate) {

	/*
	if these tables don't exist, we create them.  return true only if everything 
	is laid out as it's supposed to.
	
	2/28/91 dmb: even if we fail, try to locate as many tables as possible

	5.0a16 dmb: last version created system.menus. This version moves paths to system.

	5.0b15 dmb: never auto-create the old "resources" table (system.misc)
	*/
	
	register boolean fl;
	hdlhashtable menustable;
	
	#ifdef MACVERSION
		hdlhashtable macintoshtable;
	#endif
	
	fl = checktable (roottable, namesystembranch, flcreate, &systemtable);
	
	if (fl) {
		
		if (!checktable (systemtable, nameverbstable, flcreate, &verbstable))
			fl = false;
		
		if (!checktable (systemtable, nameagentstable, flcreate, &agentstable))
			fl = false;
		
		if (!checktable (systemtable, nameresourcestable, false, &resourcestable))
			fl = false;
		
		if (!checktable (systemtable, namepathstable, flcreate, &pathstable))
			fl = false;

		if (checktable (systemtable, STR_menus, false, &menustable))
			checktable (menustable, namemenubartable, false, &menubartable); /*don't auto-create*/
		
		#ifdef MACVERSION
		
		if (checktable (systemtable, namemacintoshtable, false, &macintoshtable))
			checktable (macintoshtable, nameobjectmodeltable, false, &objectmodeltable);
		
		#endif
		
		}
	
	if (verbstable != nil) {
		
		if (!checktable (verbstable, namebuiltinstable, flcreate, &builtinstable))
			fl = false;
		
		if (!checktable (verbstable, nameiacgluetable, flcreate, &iacgluetable))
			fl = false;
		
		if (!checktable (verbstable, nameiachandlertable, flcreate, &iachandlertable))
			fl = false;
		}
		
	/*usertable = roottable; /*does this work?*/
	
	return (fl);
	} /*checktablestructure*/


boolean cleartablestructureglobals (void) {

	/*
	dmb 9/24/90: clear globals; root table is about to be disposed
	*/
	
	rootvariable = nil;
	
	roottable = nil;
	
	systemtable = nil;
	
	builtinstable = nil;
	
	pathstable = nil;
	
	verbstable = nil;
	
	iacgluetable = nil;
	
	iachandlertable = nil;
	
	resourcestable = nil;
	
	agentstable = nil;
	
	/*
	usertable = nil;
	*/
	
	menubartable = nil;
	
	#ifdef flnewfeatures
	
	objectmodeltable = nil;
	
	#endif
	
	/*these are never disposed; they're shared among all files
	
	internaltable = nil;
	
	efptable = nil;
	
	langtable = nil;
	
	runtimestacktable = nil;
	
	threadtable = nil;
	
	filewindowtable = nil;
	*/
	
	return (true);
	} /*cleartablestructureglobals*/


boolean settablestructureglobals (Handle hvariable, boolean flcreatesubs) {
	
	register hdltablevariable hv = (hdltablevariable) hvariable;
	register hdlhashtable ht;
	
	if (hv == nil)
		return (false);
	
	ht = (hdlhashtable) (**hv).variabledata;
	
	if (ht == nil)
		return (false);
	
	cleartablestructureglobals ();
	
	rootvariable = (Handle) hv;
	
	roottable = ht;
	
	return (checktablestructure (flcreatesubs)); /*sets agentstable, builtinstable, etc.*/
	} /*settablestructureglobals*/


