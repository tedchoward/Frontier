
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

#include "file.h"
#include "kb.h"
#include "memory.h"
#include "ops.h"
#include "quickdraw.h"
#include "resources.h"
#include "strings.h"
#include "shell.h"
#include "shellprivate.h"
#include "shellundo.h"
#include "langinternal.h"
#include "langexternal.h"
#include "opinternal.h"
#include "tablestructure.h"
#include "tableinternal.h"
#include "tableverbs.h"
#include "wpengine.h"
#include "claybrowser.h"
#include "claybrowserstruc.h"
#include "claycallbacks.h"
#include "cancoon.h"




boolean tablevaltotable (tyvaluerecord val, hdlhashtable *htable, hdlhashnode hnode) {
	
	/*
	called externally -- you give us a value that holds an external value that's
	a tableprocessor variable, we'll return you a handle to the hashtable.
	*/
	
	hdltablevariable hvariable;
	short errorcode;
	
	if (!gettablevariable (val, &hvariable, &errorcode))
		return (false);
	
	if (!tableverbinmemory ((hdlexternalvariable) hvariable, hnode))
		return (false);
	
	*htable = (hdlhashtable) (**hvariable).variabledata;
	
	return (true);
	} /*tablevaltotable*/


boolean tablewindowopen (hdlexternalvariable hvariable, hdlwindowinfo *hinfo) {
	
	/*
	5.0.1 dmb: if tableformats are nil, don't call shellfinddatawindow
	*/

	register hdltablevariable hv = (hdltablevariable) hvariable;
	register hdlhashtable ht;
	hdltableformats hf;

	
	if (!(**hv).flinmemory)
		return (false);
	
	ht = (hdlhashtable) (**hv).variabledata;
	
	hf = (**ht).hashtableformats;
	
	if (hf != nil)
		shellfinddatawindow ((Handle) hf, hinfo); /*3/19/91 DW*/
	
	return ((**ht).flwindowopen);
	} /*tablewindowopen*/


boolean tableverbgetdisplaystring (hdlexternalvariable h, bigstring bs) {
	
	/*
	get string for table display.  variable is in memory.
	
	called from langexternal.c.
	*/
	
	register hdltablevariable hv = (hdltablevariable) h;
	register hdlhashtable ht;
	long ctitems;
	
	ht = (hdlhashtable) (**hv).variabledata;
	
	hashcountitems (ht, &ctitems);
	
	parsenumberstring (tablestringlist, tablesizestring, ctitems, bs);
	
	return (true);
	} /*tableverbgetdisplaystring*/


boolean tableverbgettypestring (hdlexternalvariable hvariable, bigstring bs) {
	
	register hdltablevariable hv = (hdltablevariable) hvariable;
	short id;
	
	if ((**hv).flsystemtable)
		id = systemtabletypestring;
	else {

		#ifdef xmlfeatures
			if ((**hv).flxml)
				id = xmltypestring;
			else
		#endif

			id = tabletypestring;
		}
	
	tablegetstringlist (id, bs);
	
	return (true); /*dirty status display wired off, too noisy*/
	} /*tableverbgettypestring*/


static boolean tabledisposevariable (hdlexternalvariable hvariable, boolean fldisk) {
	
	register hdltablevariable hv = (hdltablevariable) hvariable;
	
	tabledisposetable ((hdlhashtable) (**hv).variabledata, fldisk);
	
	return (true);
	} /*tabledisposevariable*/


boolean tableverbdispose (hdlexternalvariable hvariable, boolean fldisk) {
	
	/*
	1/25/91 dmb: special-case local tables; they're the only kind of 
	system table that can be thrown out.  we might be able to get rid 
	of this dirtyness after we support locked tables
	
	12/22/91 dmb: in order to release all db nodes properly, must force 
	table to be loaded into memory when fldisk is true
	*/
	
	register hdltablevariable hv = (hdltablevariable) hvariable;
	register hdlhashtable ht;
	
	if (fldisk) { /*load table into memory so that all items can release their db nodes*/
		
		if (!tableverbinmemory ((hdlexternalvariable) hv, HNoNode))
			return (false);
		}
	
	if ((**hv).flinmemory) {
		
		ht = (hdlhashtable) (**hv).variabledata;
		
		#if (MEMTRACKER == 1)
			if (internaltable != nil) // not tossing everything
		#endif

		if ((**hv).flsystemtable && !(**ht).fllocaltable)
			return (true);
		}
	
	langexternaldisposevariable ((hdlexternalvariable) hv, fldisk, &tabledisposevariable);
	
	return (true);
	} /*tableverbdispose*/


boolean tableverbnew (hdlexternalvariable *hvariable) {
	
	hdlhashtable hnewtable;
	
	return (tablenewtable ((hdltablevariable *) hvariable, &hnewtable));
	} /*tableverbnew*/


boolean tableverbinmemory (hdlexternalvariable hvariable, hdlhashnode hnode) {
	
	/*
	5.0a18 dmb: support database linking
	*/
	
	register hdltablevariable hv = (hdltablevariable) hvariable;
	Handle hpacked;
	hdlhashtable htable = nil;
	dbaddress adr;
	langerrormessagecallback savecallback;
	ptrvoid saverefcon;
	hdlhashtable hparent;
	bigstring bspath, bsunpackerror;
	boolean fl;
	
	if ((**hv).flinmemory) /*nothing to do, it's already in memory*/
		return (true);
	
	if ((hnode == nil) || (hnode == HNoNode))
		hnode = nil;

	dbpushdatabase ((**hv).hdatabase);

	adr = (dbaddress) (**hv).variabledata;
	
	if (adr == nildbaddress) { /*table has never been allocated*/
		
		shellinternalerror (idniltableaddress, "\x2b" "nil table address.  (Creating empty table.)");
		
		fl = false;
		}
	else {
		
		fl = dbrefhandle (adr, &hpacked);
		
		if (fl) {
			
			langtraperrors (bsunpackerror, &savecallback, &saverefcon);
			
			fl = tableunpacktable (hpacked, false, &htable); /*always disposes of hpackedtable*/
			
			languntraperrors (savecallback, saverefcon, !fl);
			
			if (!fl) {
				
				fllangerror = false;
				
				if (langexternalfindvariable ((hdlexternalvariable) hv, &hparent, bspath) &&
					langexternalgetfullpath (hparent, bspath, bspath, nil)) {
					
					poptrailingchars (bsunpackerror, '.');
					
					lang2paramerror (tableloadingerror, bspath, bsunpackerror);
					}
				else
					langerrormessage (bsunpackerror);
				}
			}
		}
	
	dbpopdatabase ();

	if (!fl)
		return (false);
	
	(**hv).flinmemory = true;
	
	(**hv).variabledata = (long) htable; /*link into variable structure*/
	
	(**hv).oldaddress = adr; /*last place this table was stored*/
	
	if ((**hv).flmayaffectdisplay)
		(**htable).flmayaffectdisplay = true;
	
	#ifdef xmlfeatures
		(**htable).flxml = (**hv).flxml; //5.0.1
	#endif

	(**htable).hashtablerefcon = (long) hv; /*we can get from hashtable to variable rec*/

	(**htable).thistableshashnode = hnode; /*The var rec is contained in the hashnode... RAB 1/3/00 */
	
	return (true);
	} /*tableverbinmemory*/
	

boolean tableverbgetsize (hdlexternalvariable hvariable, long *size) {
	
	register hdlexternalvariable hv = hvariable;
	long ctitems;
	
	if (!tableverbinmemory (hv, HNoNode))
		return (false);
		
	hashcountitems ((hdlhashtable) (**hv).variabledata, &ctitems);
	
	*size = (long) ctitems;
	
	return (true);
	} /*tableverbgetsize*/
	

boolean tableverbisdirty (hdlexternalvariable hvariable) {
	
	/*
	8/11/92 dmb: check windowrect now to ensure consistent results
	*/
	
	register hdltablevariable hv = (hdltablevariable) hvariable;
	register hdlhashtable ht;
	
	if (!(**hv).flinmemory)
		return (false);
	
	ht = (hdlhashtable) (**hv).variabledata;
	
	tablecheckwindowrect (ht);
	
	return ((**ht).fldirty);
	} /*tableverbisdirty*/


boolean tableverbsetdirty (hdlexternalvariable hvariable, boolean fldirty) {
	
	/*
	4/15/92 dmb: see comments in langexternalsetdirty.  also note that when the table 
	is being set as clean, its may still be dirty.
	*/
	
	register hdltablevariable hv = (hdltablevariable) hvariable;
	
	if (!tableverbinmemory ((hdlexternalvariable) hv, HNoNode))
		return (false);
	
	(**(hdlhashtable) (**hv).variabledata).fldirty = fldirty;
	
	return (true);
	} /*tableverbsetdirty*/


static boolean tableresetformatsrects () {
	
	/*
	2/14/97 dmb: rewrite for outline editor

	6.0b3 dmb: need to set display default before calcing tablerect; also
	inset based on gray3Dlook
	*/
	
	hdloutlinerecord ho = (**tableformatsdata).houtline;
	Rect r;
	
	oppushoutline (ho);
	
	opsetdisplaydefaults (ho);

	windowresetrects (tableformatswindowinfo);
	
	tablerecalccolwidths (false); /*re-distribute colwidths*/
	
	r = (**tableformatsdata).tablerect;
	
	#ifdef gray3Dlook
		insetrect (&r, -2, -2);
	#else
		insetrect (&r, 1, 1);
	#endif

	opresize (r);
	
	oppopoutline ();
	
	return (true);
	} /*tableresetformatsrects*/


boolean tableverbsetupdisplay (hdlhashtable htable, hdlwindowinfo hinfo) {
	
	/*
	6/21/92 dmb: when creating formats for a locals table, use the 
	default script font/size instead of the table config's.
	*/
	
	register hdlhashtable ht = htable;
	hdltableformats hf;
	Rect r;
	
	hf = (**ht).hashtableformats;
	
	if (hf == nil) { /*this guy has never been edited, create a formats record for him*/
		
		r = (**hinfo).contentrect;
		
		if (!tablenewformatsrecord (ht, r, &hf))
			return (false);
		
		if ((**ht).fllocaltable || ht == runtimestacktable) { /*use default script font/size for locals tables*/
			
			tyconfigrecord config;
			
			shellgetconfig (idscriptconfig, &config);
			
			(**hf).fontnum = config.defaultfont;
			
			(**hf).fontsize = config.defaultsize;
			
			(**hf).fontstyle = config.defaultstyle;
			}
		}
	
	(**hinfo).hdata = (Handle) hf; /*link to windowinfo*/
	
	if (!tableprepareoutline (hf))
		return (false);
	
	shellpushglobals ((**hinfo).macwindow);
	
	tableresetformatsrects ();
	
	shellpopglobals ();
	
	return (true); 
	} /*tableverbsetupdisplay*/


boolean tableedit (hdlexternalvariable hvariable, hdlwindowinfo hparent, ptrfilespec fs, bigstring bs, rectparam rzoom) {
	
	/*
	3/30/93 dmb: don't resort the table before zooming. not sure why this 
	was ever necessary, but it sure is slow for large tables
	*/
	
	register hdltablevariable hv = (hdltablevariable) hvariable;
	register hdlhashtable ht;
	register hdltableformats hf;
	Rect rwindow;
	hdlhashtable hparenttable;
	bigstring bsname;
	WindowPtr w;
	hdlwindowinfo hi;
	
	if (!tableverbinmemory ((hdlexternalvariable) hv, HNoNode)) /*couldn't swap it into memory*/
		return (false);
	
	ht = (hdlhashtable) (**hv).variabledata;
	
	if (tablefindvariable ((hdlexternalvariable) hv, &hparenttable, bsname))
		(**ht).parenthashtable = hparenttable;
	
	if (tablewindowopen ((hdlexternalvariable) hv, &hi)) { /*bring to front, return true*/
		
		if ((*rzoom).top > -2)
			shellbringtofront (hi);
		
		return (true);
		}
	
	hf = (**ht).hashtableformats;
	
	if (hf == nil)
		rwindow.top = -1; /*accept default*/
	else
		rwindow = (**hf).windowrect;
	
	if (!newchildwindow (idtableconfig, hparent, &rwindow, rzoom, bs, &w))
		return (false);
	
	getwindowinfo (w, &hi);
	
	if (fs != nil)
		(**hi).fspec = *fs;
	
	if (!tableverbsetupdisplay (ht, hi)) {
		
		/*xxx -- must get rid of the window!%/
		
		return (false);
		*/
		}
	
	hf = (**ht).hashtableformats;
	
	//if ((**hf).flneedresort)
	//	hashresort (ht, nil); /*sort the table before zooming*/
	
	if ((**ht).fldirty)
		shellsetwindowchanges (hi, true);
	
	(**ht).flwindowopen = true;
	
	(**(**hf).houtline).flwindowopen = true;
	
	#if TARGET_API_MAC_CARBON == 1
	
		SetWindowProxyCreatorAndType (w, 'LAND', 'FTtb', kOnSystemDisk);
	
	#endif
	
	windowzoom (w);
	
	#if TARGET_API_MAC_CARBON == 1
	
		shellupdatewindow (w);	
	
	#endif
	
	return (true);
	} /*tableedit*/


boolean tablewindowclosed (hdlexternalvariable hvariable) {
	
	/*
	the table manager needs to be informed when a window holding an external is
	closed.  this path was created (11/15/90 DW) so that the icon can be updated. in 
	the future it may be used for other good things.
	
	12/9/91 dmb: inval entire row, not just icon, so "on disk" can be updated

	5.0b18 dmb: don't call symbolchanged if a table is being closed,
	or it will cause it to be collapsed in other views
	*/
	
	hdlhashtable htable;
	bigstring bsname;
	
	if (!tablefindvariable (hvariable, &htable, bsname))
		return (false);
	
	if (htable == nil) // was the hdlrootvariable; not in another table
		return (true);
	
	if (istablevariable (hvariable))
		return (true);

	return (tablesymbolchanged (htable, bsname, nil, true));
	
	/*
	if (!tablepushglobals (htable)) // parent table isn't open, no updating needed
		return (true);
	
	fl = hashsortedsearch (htable, bsname, &row);
	
	if (fl)
		tableinvalrow (row);
	
	shellpopglobals (); // undo the call to tablepushglobals
	*/
	
	} /*tablewindowclosed*/
	
	
boolean tabledive (void) {
	
	/*
	bigstring bs;
	tyvaluerecord val;
	*/
	hdlheadrecord hcursor = (**outlinedata).hbarcursor;
	
	if (opeditingtext (hcursor)) { //((**tableformatsdata).fleditingcell)
		
		if (opsettextmode (false)) // if (tableexiteditmode ())
			opupdatenow ();
		else
			return (false);
		}
	
	tablezoomfromhead (hcursor);
	
	return (true);
	} /*tabledive*/


boolean tableclientsurface (hdlexternalvariable hvariable) {
	
	/*
	surface for one of the clients -- like the pict editor, script editor, etc.
	
	we do an exhaustive search for a table entry of external type, of no 
	specific sub-type, that points to the indicated variable.
	
	1/9/91 dmb: see comment in tablebringtofront about not setting globals 
	without a push and pop.
	
	5.0.2b21 dmb: don't surface if parent is the file windows table
	*/
	
	hdlhashtable htable;
	bigstring bsname;
	
	if (!tablefindvariable (hvariable, &htable, bsname))
		return (false);
	
	if (htable == filewindowtable)
		return (false);
	
	if (!tablezoomtoname (htable, bsname))
		return (false);
	
	if (keyboardstatus.floptionkey)
		shellclosewindow (shellwindow);
	
	return (true);
	} /*tableclientsurface*/


boolean tablesurface (void) {
	
	/*
	5/10/91 dmb: use generic tools to replace old ccbringtofront routine
	
	5.0.2b21 dmb: don't surface from a file-based window
	*/
	
	register hdlhashtable ht;
	hdlhashtable hparenttable;
	bigstring bsname;
	hdlwindowinfo hrootinfo;
	
	if (isfilewindow (tableformatswindow))
		return (false);
	
	ht = tablegetlinkedhashtable ();
	
	if ((**ht).parenthashtable == nil) { /*no parent, must be the root*/
		
		if (!getrootwindow (tableformatswindow, &hrootinfo))
			return (false);
		
		return (shellbringtofront (hrootinfo)); /*bring the home window to the front*/
		}
	
	if ((ht == roottable) || !findinparenttable (ht, &hparenttable, bsname))
		return (false);
	
	if (!(**hparenttable).flwindowopen && !tablezoomfromtable (hparenttable))
		return (false);
	
	if (!tablebringtofront (hparenttable))
		return (false);
	
	if (!shellpushfrontglobals ())
		return (false);
	
	tablemovetoname (hparenttable, bsname);
	
	shellpopglobals ();
	
	if (keyboardstatus.floptionkey)
		shellclosewindow (shellwindow);
	
	return (true);
	} /*tablesurface*/


boolean tablezoomfromtable (hdlhashtable htable) {
	
	/*
	the caller has a hashtable record that he wants displayed in a window.
	*/
	
	register hdlhashtable ht = htable;
	register hdltablevariable hv = (hdltablevariable) (**ht).hashtablerefcon;
	bigstring bspath;
	Rect rzoom;
	hdlwindowinfo hparent;

	rzoom.top = -1; /*accept default*/
	
	if (hv == nil) /*not a zoomable table*/
		return (false);
	
	if (!langexternalgetfullpath (ht, emptystring, bspath, &hparent))
		return (false);
	
	return (tableedit ((hdlexternalvariable) hv, hparent, nil, bspath, &rzoom));
	} /*tablezoomfromtable*/


boolean tablezoomfromhead (hdlheadrecord hnode) {
	
	/*
	zoom a table item into its own window.  
	*/
	
	hdlhashtable htable;
	bigstring bs;
	tyvaluerecord val;
	Rect rzoom;
	hdlhashnode hhashnode;
	
	if (!tablegetiteminfo (hnode, &htable, bs, &val, &hhashnode))
		return (false);
	
	if (val.valuetype == novaluetype) { /*not value type assigned yet*/
		
		if (!tablepopupkinddialog ())
			return (false);
		
		if (!tablegetiteminfo (hnode, &htable, bs, &val, &hhashnode))
			return (false);
		}
	
	opgetnoderect (hnode, &rzoom);
	
	localtoglobalrect (tableformatswindow, &rzoom);
	
	if (!langexternalzoomfrom (val, htable, bs, &rzoom))
		return (false);
	
	if (keyboardstatus.floptionkey)
		shellclosewindow (tableformatswindow);
	
	return (true);
	} /*tablezoomfromhead*/


typedef struct tysymbolchangedinfo {
	
	hdlhashtable htable;
	
	ptrstring bsname;
	
	boolean flvalue;
	
	boolean flfound;
	
	boolean flchanged;

	hdldatabaserecord hdatabase;
	
	opvisitcallback opvisitroutine;
	
	} tysymbolchangedinfo, *ptrsymbolchangedinfo;


typedef struct tyfinddatabaseinfo {
	
	hdldatabaserecord hdatabase;

	WindowPtr w;
	} tyfinddatabaseinfo;


static boolean finddatabasevisit (WindowPtr w, tyfinddatabaseinfo *pinfo) {
	
	if (ccwindowgetdatabase (w) == (*pinfo).hdatabase) { // got it
	
		(*pinfo).w = w;
		
		return (false);
		}
	
	return (true); // keep visiting
	} /*finddatabasevisit*/


static boolean tablefindrootwindow (hdlhashtable ht, hdlwindowinfo *hinfo) {
	
	/*
	find the root window that contains the given table
	*/

	tyfinddatabaseinfo findinfo;
	
	findinfo.hdatabase = tablegetdatabase (ht);

	*hinfo = nil;
	
	if (shellvisittypedwindows (idcancoonconfig, finddatabasevisit, &findinfo))
		return (false);
	
	return (getwindowinfo (findinfo.w, hinfo));
	} /*tablefindrootwindow*/


static boolean opnodeistable (hdlheadrecord hnode, hdlhashtable htable) {

	tybrowserspec fs;
	hdlhashtable ht;
	
	return ((**hnode).flnodeisfolder && claygetfilespec (hnode, &fs) && claygetinmemorydirid (&fs, &ht) && (ht == htable));
	} /*opnodeistable*/


static boolean tableupdatewindowtitles (hdlhashnode hnode, hdlhashtable intable) {
	
	/*
	the indicated table value used to have a different name.  if it's 
	an external value, update any window titles that depend on its path

	5.0b16 dmb: don't change the titles of file windows, or they'll become
	full paths. later, it would be better to add a fltitlelocked to 
	hdlwindowinfo, set after a window.settitle we wouldn't do anything either
	*/
	
	hdlhashtable htable;
	tyvaluerecord val;
	bigstring bspath;
	hdlwindowinfo hinfo;
	
	val = (**hnode).val;
	
	if (val.valuetype != externalvaluetype) /*can't be in a window -- unwind recursion*/
		return (true);
	
	if (intable == filewindowtable) //5.0b16
		return (true);

	if (langexternalwindowopen (val, &hinfo)) {
		
		gethashkey (hnode, bspath);
		
		langexternalgetfullpath (intable, bspath, bspath, nil);
		
		shellsetwindowtitle (hinfo, bspath);
		}
	
	if (langexternalvaltotable (val, &htable, hnode)) {
		
		hashtablevisit (htable, &tableupdatewindowtitles, htable); /*daisy chain recursion*/
		}
	
	return (true);
	} /*tableupdatewindowtitles*/


static boolean tableupdateoutlinesort (hdlheadrecord hfirst, hdlhashtable htable) {
	
	/*
	5.0d18 dmb: take hfirst instead of hparent; handle all updating here.
	*/
	
	hdlheadrecord nomad, hcursor, hmatch;
	hdlhashnode h;
	bigstring bs;
	hdlscreenmap hmap;
	
	opstartinternalchange ();
	
	opnewscreenmap (&hmap);
	
	hcursor = (**outlinedata).hbarcursor;
	
	nomad = hfirst;
	
	// loop though table nodes, mirroring order of subheads
	
	for (h = (**htable).hfirstsort; h != nil; h = (**h).sortedlink) {
		
		gethashkey (h, bs);
		
		if (opfindhead (nomad, bs, &hmatch)) { // shouldn't fail
			
			if (hmatch != nomad) {
				
				opunlink (hmatch);
				
				opdeposit (nomad, up, hmatch);
				}
			else {
				opchasedown (&nomad);
				}
			}
		#if fldebug
		else
			assert (false);
		#endif
		}
	
	(**outlinedata).hbarcursor = hcursor;
	
	opinvalscreenmap (hmap);
	
	opvisibarcursor ();
	
	opendinternalchange ();
	
	return (true);	
	} /*tableupdateoutlinesort*/


static boolean opinternaldeletenodewithupdate (hdlheadrecord hnode) {
	
	hdlscreenmap hmap;
	
	killundo (); /*must toss undos before they're potentially stranded*/
	
	opstartinternalchange ();
	
	opbeforestrucchange (&hmap, false);
	
	opdeletenode (hnode);
	
	opafterstrucchange (hmap, false);
	
	opendinternalchange ();
	
	return (true);
	} /*opinternaldeletenodewithupdate*/
	

static boolean tablesymbolzoomvisit (hdlheadrecord hnode, ptrsymbolchangedinfo symbolinfo) {
	
	/*
	5.0d18 dmb: expand tables one level to reveal the item we're looking for
	*/
	
	bigstring bs;
	tybrowserinfo info;
	tybrowserspec fs;
	hdlhashtable htable;
	
	opgetheadstring (hnode, bs);
	
	browsergetrefcon (hnode, &info);
	
	if (info.dirid == (*symbolinfo).htable) { // potentially a match
		
		if (equalidentifiers (bs, (*symbolinfo).bsname)) { // it is a match
			
			opsettextmode (false);
			
			opmoveto (hnode);
			
			shellbringtofront (shellwindowinfo);
			
			(*symbolinfo).flfound = true;
			
			return (false); // we're done
			}
		}
	
	if ((**hnode).flnodeisfolder && 	// it's a table
		!opsubheadsexpanded (hnode) &&	// ...unexpanded.
		claygetfilespec (hnode, &fs) &&	// which...
		claygetinmemorydirid (&fs, &htable) &&	// ...table?
		(htable == (*symbolinfo).htable)) {		// the one we want!
		
		if ((**shellwindowinfo).flhidden) // but the window is hidden
			return (false); // we're done, but unsuccessful
		
		opexpand (hnode, 1, false); // expand it for traversal
		}

	return (true);
	} /*tablesymbolzoomvisit*/


static boolean tablesymbolchangedvisit (hdlheadrecord hnode, ptrsymbolchangedinfo symbolinfo) {
	
	bigstring bs;
	tybrowserinfo info;
	boolean fl;
	
	if (opinternalchange ()) // for symbol changes, no further action needed
		return (false);

	opgetheadstring (hnode, bs);
	
	/*
	if (equalidentifiers (bs, (*symbolinfo).bsname)) { // potentially a match
		
		browsergetrefcon (hnode, &info);
		
		if (info.dirid == (*symbolinfo).htable) { // it is a match
	*/
	
	browsergetrefcon (hnode, &info);
	
	if (info.dirid == (*symbolinfo).htable) { // potentially a match
		
		if ((*symbolinfo).flvalue)
			fl = equalidentifiers (bs, (*symbolinfo).bsname); // a match
		else
			fl = !hashtablesymbolexists (info.dirid, bs); // best guess
		
		if (fl) { // it is a match
			
			// if we kept more info around, we'd have to reload the refcon here
			
			opinvalnode (hnode);
			
			if ((*symbolinfo).flvalue) { // value changed
			
				if ((**hnode).flnodeisfolder) //5.0b18
					opcollapse (hnode);

				browserloadnode (hnode); /*mod date changed*/
				}
			else { // name changed
			
				opsetactualheadstring (hnode, (*symbolinfo).bsname);
				
				browserinsertagain (hnode);
				}
			
			if ((**hnode).tmpbit2) {
			
				(**hnode).tmpbit2 = false; //user modified item, no longer temp
				
				langexternalpleasesave ((*symbolinfo).htable, (*symbolinfo).bsname); // 5.1.4
				}
			
			(*symbolinfo).flfound = true;
			
			return (false); // we're done
			}
		}
	
	return (true);
	} /*tablesymbolchangedvisit*/


static boolean tablesymbolinsertedvisit (hdlheadrecord hnode, ptrsymbolchangedinfo symbolinfo) {
	
	/*
	8.8.97 dmb: always check parID of summit, so we'll always catch insertions 
	to the parent table

	5.0a25 dmb: we're called for internal change now, must be more selective
	about what we do.
	
	5.1.4 dmb: detect case where we have lone "bogus" summit, not connected to 
	window's table.
	*/
	
	tybrowserspec fs;
	hdlhashtable htable;
	hdlheadrecord hnew;
	hdlheadrecord hsummit = (**outlinedata).hsummit;
	boolean flsummitisbogus = false;
	
	if ((**hnode).flnodeisfolder || hnode == hsummit) { // could be the table
		
		if (!claygetfilespec (hnode, &fs)) { // not connected to a table
			
			if (hnode == hsummit) { // all nodes were deleted, new summit was auto-created
				
				flsummitisbogus = true;
				
				browsergetparentspec (hnode, &fs); // will set right table, item is ""
				}
			}
		
		if (fs.parID == (*symbolinfo).htable) { // we must be a summit...
			
			assert ((**hnode).headlinkleft == hnode); // ...otherwise our parent would have caught it below
			
			if (!opinternalchange ()) {

				copystring ((*symbolinfo).bsname, fs.name);
				
				if (browserfileadded (nil, &fs, &hnew)) {
					
					if (flsummitisbogus)
						opinternaldeletenodewithupdate (hsummit);
					}
				}
			
			(*symbolinfo).flfound = true;
			
			return (false); // we're done
			}
		
		if ((**hnode).flnodeisfolder && claygetinmemorydirid (&fs, &htable) && (htable == (*symbolinfo).htable)) {
			
			opinvalnode (hnode); // update item count
			
			if (!opinternalchange () && opsubheadsexpanded (hnode)) {
				
				fs.parID = htable;
				
				copystring ((*symbolinfo).bsname, fs.name);
				
				browserfileadded (hnode, &fs, &hnew);
				}
			
			(*symbolinfo).flfound = true;
			
			return (false); // we're done
			}
		}
	
	return (true);
	} /*tablesymbolinsertedvisit*/


static boolean tablesymboldeletedvisit (hdlheadrecord hnode, ptrsymbolchangedinfo symbolinfo) {
	
	/*
	5.0a25 dmb: we're called for internal change now, must be more selective
	about what we do.
	*/

	bigstring bs;
	tybrowserinfo info;
	hdlheadrecord hparent;
	
	opgetheadstring (hnode, bs);
	
	browsergetrefcon (hnode, &info);
	
	if (opinternalchange ()) { // just look for parent node
		
		if (opnodeistable (hnode, (*symbolinfo).htable)) {
			
			opinvalnode (hnode); // update item count
			
			return (false);
			}
		
		return (true);
		}
	
	if (info.dirid == (*symbolinfo).htable) { // potentially a match
	
		if (equalidentifiers (bs, (*symbolinfo).bsname)) { // it is a match, or don't care
			
			// first, inval the parent table node; its item count is changing
			
			hparent = (**hnode).headlinkleft;
			
			if (hparent != hnode)
				opinvalnode (hparent);
			
			opinternaldeletenodewithupdate (hnode);
			
			(*symbolinfo).flfound = true;
			
			return (false); // we're done
			}
		}
	else {
		
		if (!opsubheadsexpanded (hnode)) { // can't rely on subhead to dirty parent
			
			if (opnodeistable (hnode, (*symbolinfo).htable)) {
				
				opinvalnode (hnode); // update item count
				
				return (false);
				}
			}
		}
	
	return (true);
	} /*tablesymboldeletedvisit*/


static boolean tablesymbolsresortedvisit (hdlheadrecord hnode, ptrsymbolchangedinfo symbolinfo) {
	
	tybrowserspec fs;
	hdlhashtable htable;
	
	if (hnode == (**outlinedata).hsummit) { // one time check to see of parent table is it
		
		claygetfilespec (hnode, &fs);
		
		if (fs.parID == (*symbolinfo).htable) {
		
			tableupdateoutlinesort (hnode, fs.parID);
			
			return (false); // we're done
			}
		}
	
	if ((**hnode).flnodeisfolder) { // could be the table
		
		claygetfilespec (hnode, &fs);
		
		if (claygetinmemorydirid (&fs, &htable) && (htable == (*symbolinfo).htable)) {
			
			if (opsubheadsexpanded (hnode))
				tableupdateoutlinesort ((**hnode).headlinkright, htable);
			
			return (false); // we're done
			}
		}
	
	return (true);
	} /*tablesymbolsresortedvisit*/



static boolean tablesymbolchangedwindowvisit (WindowPtr w, ptrsymbolchangedinfo symbolinfo) {
	
	/*
	5.0d11 dmb: check internalchange here, now that it's outline-specific
	
	5.0a2 dmb: only visit children of the symbol's root

	5.0a17 dmb: also visit standalone table windows
	
	5.0a21 dmb: visit all windows. our goal is to not visit windows that 
	aren't part of the current context, but with file-based tables and 
	guest roots, the logic is getting too messy -- especially since we 
	don't expect people to keep two full roots open at the same time.

	5.0a25 dmb: don't even check opinternalchange here; the check needs to 
	be more specific, in each visit routine

	5.1.5b15 dmb: rewrote using new ccwindowgetdatabase for specificity
	*/
	
	if (ccwindowgetdatabase (w) == (*symbolinfo).hdatabase) {
		
		shellpushglobals (w);
		
		if (tableformatsdata && outlinedata) { // && !opinternalchange ())
	
			opvisiteverything ((*symbolinfo).opvisitroutine, symbolinfo);
			
			//dirty the root (database) window, in case we don't find the change in any window
			if ((*symbolinfo).flchanged && (**shellwindowinfo).parentwindow == nil)
				shellsetwindowchanges (shellwindowinfo, true);
			}
		
		shellpopglobals ();
		}
	
	return (true); // visit all windows, even after finding an instance
	} /*tablesymbolchangedwindowvisit*/


#ifdef fldebug
//static long	ctlocaltablechecks = 0;
//static long ctglobaltablechecks = 0;
#endif

static boolean tabledrivesymbolchange (hdlhashtable htable, const bigstring bsname, boolean flchanged, boolean flvalue, opvisitcallback visit) {
	
	/*
	5.0.2b21 dmb: added flchanged parameter to make sure root get dirtied
	*/
	
	tysymbolchangedinfo symbolchangedinfo;
	hdloutlinerecord saveoutlinedata;
	hdlwindowinfo hroot;
	boolean fl;
	
	if (!(**htable).flmayaffectdisplay) {
		
		if (!(**htable).fldirty) { // first mod to this table, must dirty root window
			
			if (tablefindrootwindow (htable, &hroot))
				shellsetwindowchanges (hroot, true);
			}

		return (false);
		}
	
	symbolchangedinfo.hdatabase = tablegetdatabase (htable);
	
	symbolchangedinfo.flchanged = flchanged;

//	#ifdef fldebug
//	if ((**htable).fllocaltable)
//		++ctlocaltablechecks;
//	else
//		++ctglobaltablechecks;
//	#endif
	
	symbolchangedinfo.htable = htable;
	
	symbolchangedinfo.bsname = (ptrstring) bsname;
	
	symbolchangedinfo.flvalue = flvalue;
	
	symbolchangedinfo.opvisitroutine = visit;
	
	symbolchangedinfo.flfound = false;
	
	saveoutlinedata = outlinedata; // may not be associated with current shellglobals
	
	fl = shellvisittypedwindows (idtableconfig, &tablesymbolchangedwindowvisit, &symbolchangedinfo)
		&& shellvisittypedwindows (idcancoonconfig, &tablesymbolchangedwindowvisit, &symbolchangedinfo)
		&& symbolchangedinfo.flfound;
	
	opsetoutline (saveoutlinedata);
	
	return (fl);
	} /*tabledrivesymbolchange*/


boolean tablesymbolchanged (hdlhashtable htable, const bigstring bsname, hdlhashnode hn, boolean flvalue) {
	
	/*
	used as a callback routine.  the value of the indicated variable has changed.
	if the cell is being displayed, update the display of it.  if the name changed,
	update any window titles containing it.
	
	11/14/01 dmb: we now take hnode parameter; probably don't need to pushhashtable 
	and look it up anymore, but not having the table pushed could change the 
	behavior of tableupdatewindowtitles, so I'll leave it.  (it doesn't look like it
	would be a problem, but....)
	*/
	
	hdlhashnode hnode;
	
	if (!flvalue) { // the name changed, not the value
		
		pushhashtable (htable);
		
		if (hashlookupnode (bsname, &hnode))
			tableupdatewindowtitles (hnode, htable);
		
		pophashtable ();
		}
	
	return (tabledrivesymbolchange (htable, bsname, true, flvalue, &tablesymbolchangedvisit));
	} /*tablesymbolchanged*/


boolean tablesymbolinserted (hdlhashtable htable, const bigstring bsname) {
	
	/*
	5.0b10 dmb: need to update window titles, in case insertion is 2nd 
	half of a move operation. (even table.move can do that.) while it's 
	conceivable that we'd need to do this for local table insertions, 
	let's not and save cycles
	*/
	
	hdlhashnode hnode;
	
	if (!(**htable).fllocaltable) {

		pushhashtable (htable);
		
		if (hashlookupnode (bsname, &hnode))
			tableupdatewindowtitles (hnode, htable);
		
		pophashtable ();
		}

	return (tabledrivesymbolchange (htable, bsname, true, true, &tablesymbolinsertedvisit));
	} /*tablesymbolinserted*/


boolean tablesymboldeleted (hdlhashtable htable, const bigstring bsname) {
	
	/*
	5.0d14: it would make sense to close all windows owned by the given value 
	record here, but it's already been deleted from the table. we'd need to 
	use the symbolunlinked callback, and add a "fldisposing" parameter, so we
	could do nothing if the value is indeed being disposed. so, instead, we 
	take care of window disposing in browserlinedeleted (browsercommitchanges)
	*/
	
	return (tabledrivesymbolchange (htable, bsname, true, true, &tablesymboldeletedvisit));
	} /*tablesymboldeleted*/


boolean tablesymbolsresorted (hdlhashtable htable) {
	
	return (tabledrivesymbolchange (htable, nil, false, false, &tablesymbolsresortedvisit));
	} /*tablesymbolsresorted*/


boolean tablezoomtoname (hdlhashtable htable, bigstring bsname) {
	
	hdlwindowinfo hinfo;
	tysymbolchangedinfo symbolchangedinfo;
	
	if (tablefinddatawindow (htable, &hinfo)) {
		
		symbolchangedinfo.htable = htable;
		
		symbolchangedinfo.bsname = (ptrstring) bsname;
		
		symbolchangedinfo.flvalue = true;
		
		symbolchangedinfo.opvisitroutine = tablesymbolzoomvisit;
		
		symbolchangedinfo.flfound = false;
		
		tablesymbolchangedwindowvisit ((**hinfo).macwindow, &symbolchangedinfo);
		}
	else
		symbolchangedinfo.flfound = tabledrivesymbolchange (htable, bsname, false, true, &tablesymbolzoomvisit);
	
	if (symbolchangedinfo.flfound)
		return (true);
	
	if (!tablezoomfromtable (htable))
		return (false);
	
	if (!shellpushfrontglobals ())
		return (false);
	
	opmoveto ((**outlinedata).hsummit);

	tablemovetoname (htable, bsname);
	
	shellpopglobals ();
	
	return (true);
	} /*tablezoomtoname*/


