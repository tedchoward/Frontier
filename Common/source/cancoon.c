
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

#ifdef MACVERSION
	#include <land.h>
#endif

#include "memory.h"
#include "dialogs.h"
#include "error.h"
#include "file.h"
#include "font.h"
#include "menu.h"
#include "ops.h"
#include "resources.h"
#include "quickdraw.h"
#include "strings.h"
#include "frontierwindows.h"
#include "shell.h"
#include "shellhooks.h"
#include "shellmenu.h"
#include "shellprivate.h"
#include "shell.rsrc.h"
#include "langexternal.h"
#include "langinternal.h"
#include "langipc.h"
#include "tableinternal.h"
#include "tablestructure.h"
#include "tableverbs.h"
#include "menuverbs.h"
#include "scripts.h"
#include "process.h"
#ifdef fltrialsize
	#include "dbinternal.h"
#endif
#include "cancoon.h"
#include "cancooninternal.h"
#include "serialnumber.h"

#include "WinSockNetEvents.h" /*6.2a14 AR*/



hdlcancoonrecord cancoondata = nil;

hdlwindowinfo cancoonwindowinfo = nil;

WindowPtr cancoonwindow = nil;


hdlcancoonrecord cancoonglobals = nil;

hdlcancoonrecord supercancoonglobals = nil; /*4.1b4 dmb: only set when globals set via setsuperglobals*/


#define maxsavedccglobals 5

static hdlcancoonrecord ccglobalsstack [maxsavedccglobals];

static short cctopglobals = 0;


static short ccwindowconfigs [] = { /*table to map window info indexes to config ids*/
	
	idcancoonconfig,
	
	0, /*idmessageconfig*/
	
	idlangerrorconfig,
	
	idcommandconfig,
	
	idaboutconfig,
	
	0 /*idpaletteconfig*/
	};


static boolean fldisablesymbolcallbacks = false;



boolean ccgetwindowinfo (short windowtype, tycancoonwindowinfo *windowinfo) {
	
	*windowinfo = (**cancoondata).windowinfo [windowtype];
	
	return (true);	
	} /*ccgetwindowinfo*/


boolean ccsetwindowinfo (short windowtype, tycancoonwindowinfo windowinfo) {
	
	(**cancoondata).windowinfo [windowtype] = windowinfo;
	
	(**cancoondata).fldirty = true;
	
	return (true);
	} /*ccsetwindowinfo*/
	

boolean ccnewsubwindow (hdlwindowinfo hinfo, short windowtype) {
	
	register hdlwindowinfo hw = hinfo;
	tycancoonwindowinfo windowinfo;
	
	if (!shellpushrootglobals ((**hw).macwindow))
		return (false);
	
	ccgetwindowinfo (windowtype, &windowinfo);
		
	(**hw).defaultfont = windowinfo.fontnum;
	
	(**hw).defaultsize = windowinfo.fontsize;
	
	windowinfo.w = (**hw).macwindow;
	
	ccsetwindowinfo (windowtype, windowinfo);
	
	shellpopglobals ();
	
	return (true);
	} /*ccnewsubwindow*/
	
	
boolean cccopywindowinfo (hdlwindowinfo hinfo, short windowtype) {
	
	register hdlwindowinfo hw = hinfo;
	register hdlcancoonrecord hc;
	register boolean fldirty;
	tycancoonwindowinfo windowinfo;
	Rect r, rwindow;
	
	if (!shellpushrootglobals ((**hw).macwindow))
		return (false);
	
	hc = cancoondata; /*copy into register*/
	
	fldirty = (**hc).fldirty; /*initialize*/
	
	shellgetglobalwindowrect (hw, &r);
	
	ccgetwindowinfo (windowtype, &windowinfo);
	
	diskrecttorect (&windowinfo.windowrect, &rwindow);
	
	if (!equalrects (r, rwindow)) { /*window was moved or resized*/
		
		recttodiskrect (&r, &windowinfo.windowrect);
		
		fldirty = true;
		}
		
	if (windowinfo.fontnum != (**hw).defaultfont) {
	
		windowinfo.fontnum = (**hw).defaultfont;
		
		fldirty = true;
		}
	
	if (windowinfo.fontsize != (**hw).defaultsize) {
	
		windowinfo.fontsize = (**hw).defaultsize;
		
		fldirty = true;
		}
	
	if (fldirty) { /*dirty things up -- everywhere conceivable*/
		
		(**hc).fldirty = true;
		
		windowsetchanges (cancoonwindow, true);
		}
		
	ccsetwindowinfo (windowtype, windowinfo);
	
	shellpopglobals ();
	
	return (true);
	} /*cccopywindowinfo*/


boolean ccsubwindowclose (hdlwindowinfo hinfo, short windowtype) {
	
	tycancoonwindowinfo windowinfo;
	
	if (!cccopywindowinfo (hinfo, windowtype))
		return (false);
	
	if (!shellpushrootglobals ((**hinfo).macwindow))
		return (false);
	
	ccgetwindowinfo (windowtype, &windowinfo);
	
	windowinfo.w = nil;
	
	ccsetwindowinfo (windowtype, windowinfo);
	
	shellpopglobals ();
		
	return (true);
	} /*ccsubwindowclose*/


boolean ccgetwindowrect (short ixwindowinfo, Rect *rwindow) {
	
	/*
	5.1.5b10 dmb: use ccfindrootwindow to avoid guest databases
	*/
	
	tyconfigrecord config;	
	hdlwindowinfo hinfo;
	
//	if (shellpushfrontrootglobals ())
	if (ccfindrootwindow (&hinfo)) {
		
		tycancoonwindowinfo windowinfo;
		
		shellpushglobals ((**hinfo).macwindow);
		
		ccgetwindowinfo (ixwindowinfo, &windowinfo);
		
		diskrecttorect (&windowinfo.windowrect, rwindow);
		
		shellpopglobals ();
		
		return (true);
		}
	
	if (shellgetconfig (ccwindowconfigs [ixwindowinfo], &config)) {
	
		*rwindow = config.defaultwindowrect;
		
		return (true);
		}
	
	setrect (rwindow, -1, -1, -1, -1); /*set top, left, bottom, right*/
	
	return (false);
	} /*ccgetwindowrect*/


static void ccinitwindowinfo (hdlcancoonrecord hcancoon, short ixwindowinfo) {
	
	register hdlcancoonrecord hc = hcancoon;		
	tycancoonwindowinfo windowinfo;
	tyconfigrecord config;	
	Rect r;
	
	clearbytes (&windowinfo, sizeof (windowinfo));
	
	if (shellgetconfig (ccwindowconfigs [ixwindowinfo], &config)) {
		
		recttodiskrect (&config.defaultwindowrect, &windowinfo.windowrect);
		
		diskgetfontname (config.defaultfont, windowinfo.fontname);
		
		windowinfo.fontnum = config.defaultfont;
	
		windowinfo.fontsize = config.defaultsize;
		}
	else {
		setrect (&r, -1, -1, -1, -1);
		
		recttodiskrect (&r, &windowinfo.windowrect);
		
		windowinfo.fontnum = systemFont;
		
		windowinfo.fontsize = 12;
		
		/*
		copystring ("\x07" "Chicago", (ptrstring) windowinfo.fontname);
		*/
		
		fontgetname (systemFont, (ptrstring) windowinfo.fontname);
		}
	
	(**hc).windowinfo [ixwindowinfo] = windowinfo;
	} /*ccinitwindowinfo*/


static void ccupdatewindowinfo (short windowtype, tyversion2cancoonrecord *info) {
	
	/*
	2/8/91 dmb: subwindows now can have a save routine, which we call to 
	allow strings to be saved into cancoondata before we update the db
	
	5.0d14 dmb: clean the window's menu item
	*/
	
	tycancoonwindowinfo windowinfo;
	Rect r;
	
	ccgetwindowinfo (windowtype, &windowinfo);
	
	if (windowinfo.w != nil) {
		
		shellpushglobals (windowinfo.w);
		
		(*shellglobals.presaveroutine) ();
		
		windowinfo.flhidden = (**shellwindowinfo).flhidden;

		shellpopglobals ();
		
		getglobalwindowrect (windowinfo.w, &r);
		
		recttodiskrect (&r, &windowinfo.windowrect);
		
		windowsetchanges (windowinfo.w, false);
		}
	
	diskgetfontname (windowinfo.fontnum, windowinfo.fontname);
	
	(*info).windowinfo [windowtype] = windowinfo; /*copy into disk record*/
	
	memtodiskshort ((*info).windowinfo [windowtype].fontsize);
	memtodiskshort ((*info).windowinfo [windowtype].fontstyle);
	
	ccsetwindowinfo (windowtype, windowinfo); /*update in-memory version*/
	} /*ccupdatewindowinfo*/


static boolean ccactivatemenubar (hdlcancoonrecord hcancoon, boolean flactivate) {
	
	if (hcancoon == nil)
		return (false);
	
	return (activatemenubarlist ((**hcancoon).hmenubarlist, flactivate));
	} /*ccactivatemenubar*/


static boolean disposecancoonrecord (hdlcancoonrecord hcancoon) {
	
	/*
	1/21/91 dmb: even though disposing the root table will dispose of 
	the menu bar, we need to explicitly deactivate the menu bar here 
	in case the language postpones the table disposal until the current 
	process terminates (which may be after another menu bar has been 
	activated).
	
	1/28/91 dmb: we were forgeting to dispose the script and message strings.
	
	5.0.1 dmb: fixed leak - use tabledisposetable, not disposehashtable

	5.1 dmb: fixed leak - disposehandle of the rootvariable itself
	*/
	
	register hdlcancoonrecord hc = hcancoon;
	
	if (hc == nil)
		return (false);
	
	ccactivatemenubar (hc, false);
	
	disposeprocesslist ((**hc).hprocesslist); /*checks for nil*/
	
	//setcurrentmenubarlist (nil); /*clear menubar.c global before disposing roottable*/
	
	disposehandle ((Handle) (**hc).hmenubarlist); /*roottable disposal will do menubars*/
	
	tabledisposetable ((**hc).hroottable, false); /*yup, checks for nil*/
	
	disposehandle ((Handle) (**hc).hrootvariable); /*5.1*/

	disposehandle ((Handle) (**hc).hscriptstring); /*always has, always will*/
	
	disposehandle ((Handle) (**hc).hprimarymsg);
	
	disposehandle ((Handle) (**hc).hsecondarymsg);
	
	disposehandle ((Handle) hc);
	
	return (true);
	} /*disposecancoonrecord*/


static boolean newcancoonrecord (hdlcancoonrecord *hcancoon) {
	
	register hdlcancoonrecord hc;
	short i;
	hdlprocesslist hprocesslist;
	hdlmenubarlist hmenubarlist;
	
	if (!newclearhandle (sizeof (tycancoonrecord), (Handle *) hcancoon))
		return (false);
	
	hc = *hcancoon;
	
	if (!newprocesslist (&hprocesslist)) 
		goto error;
	
	(**hc).hprocesslist = hprocesslist;
	
	if (!newmenubarlist (&hmenubarlist))
		goto error;
	
	(**hc).hmenubarlist = hmenubarlist;
	
	for (i = 0; i < ctwindowinfo; i++) 
		ccinitwindowinfo (hc, i);
			
	return (true);
	
	error:
	
	disposecancoonrecord (hc);
	
	*hcancoon = nil;
	
	return (false);
	} /*newcancoonrecord*/


static boolean ccinstalltablestructure (boolean flhavehost) {
	
	/*
	12/31/90 dmb: since this routine can be executing in the background, 
	while the language is evaluating an expression, we now use setroothashtable, 
	replacing setcurrenthashtable
	
	1/8/91 dmb: in fact, a cleaner and more reliable solution is to leave it 
	up to the language to always push the root table before running any code, 
	so setroothashtable is gone.
	
	5.1b23 dmb: added serial number checking
	*/
	
	register hdlcancoonrecord hc = cancoondata;
	
	assert ((**hc).hroottable == roottable); /*should already be set up*/
	
	setcurrentprocesslist ((**hc).hprocesslist);
	
	if (!flhavehost && !validateserialnumber ()) {
		
		exittooperatingsystem ();

		return (false);
		}
	
	loadsystemscripts (); /*load agents, compile handlers, run startup scripts, etc.*/
	
#ifndef PIKE
	#if TARGET_API_MAC_CARBON == 0
	langipcmenustartup ();
	#endif
#endif
	
	return (true);
	} /*ccinstalltablestructure*/


static boolean ccinstallmenubar (hdlcancoonrecord hcancoon, hdlmenurecord hmenurecord) {
	
	/*
	5.0d14 dmb: with the introduction of beginner/expert modes, it no longer
	makes sense for Frontier to automatically install system.misc.menubar
	
	5.0a2 dmb: restored functionality, but now we're only called for old odbs
	*/
	
	setcurrentmenubarlist ((**hcancoon).hmenubarlist);
	
	if (hmenurecord == nil) /*we're done*/
		return (true);
	
	assert ((**hmenurecord).hmenustack == nil);
	
	return (meinstallmenubar (hmenurecord));
	} /*ccinstallmenubar*/


static boolean ccloadsystemtable (hdlcancoonrecord hcancoon, dbaddress adr, boolean flcreate) {
	
	/*
	some factored code.  try to load the root table from adr and find or 
	create the standard global tables
	*/
	
	register hdlcancoonrecord hc = hcancoon;
	register boolean fl;
	Handle hvariable;
	hdlhashtable htable;
	
	fldisablesymbolcallbacks = true; /*so table insertions wont trigger callbacks*/
	
	fl = tableloadsystemtable (adr, &hvariable, &htable, flcreate);
	
	if (fl) {
		
		(**hc).hrootvariable = hvariable;
		
		(**hc).hroottable = htable;
		
		assert (tablevalidate (htable, true));
		
		fl = settablestructureglobals (hvariable, flcreate) || !flcreate;
		}
	
	fldisablesymbolcallbacks = false;
	
	return (fl);
	} /*ccloadsystemtable*/


void setcancoonglobals (hdlcancoonrecord hcancoon) {
	
	/*
	5.0a18 dmb: added nil check, set globals to nill in that case
	*/

	register hdlcancoonrecord hc = hcancoon;
	
	if (hc != nil) {

		databasedata = (**hc).hdatabase;

		settablestructureglobals ((**hc).hrootvariable, false);

		setcurrentprocesslist ((**hc).hprocesslist);

		setcurrentmenubarlist ((**hc).hmenubarlist);
		}
	
	cancoonglobals = hc; /*this global is independent of shellpush/popglobals*/
	} /*setcancoonglobals*/


static void clearcancoonglobals (void) {
	
	databasedata = nil; /*db.c*/
	
	setcurrentprocesslist (nil); /*agents.c*/
	
	cleartablestructureglobals (); /*tablestructure.c: roottable, handlertable, etc.*/
	
	setcurrentmenubarlist (nil); /*menubar.c*/
	
	cancoonglobals = nil; /*our very own superglobal*/
	
	supercancoonglobals = nil;  /*the ones swapped in with ccsetsuperglobals*/
	} /*clearcancoonglobals*/


static boolean loadversion2cancoonfile (dbaddress adr, hdlcancoonrecord hcancoon, boolean flhavehost) {
	
	/*
	5.0d14 dmb: bumped cancoonversionnumber; hscriptstring is now a text handle, 
	not a string handle. (no 255 char limit)
	
	5.0d18 dmb: have to ccwindowsetup _after_ loading system table now
	
	5.0a2 dmb: install the menubar if the database hasn't been upgraded
	
	2002-11-11 AR: Added additional asserts to make sure the C compiler chose the
	proper byte alignment for the tycancoonwindowinfo and tyversion2cancoonrecord
	struct. If it did not, we would end up corrupting any database files we saved.
	*/
	
	register hdlcancoonrecord hc;
	tyversion2cancoonrecord info;
	short i;
	hdlmenurecord hmenurecord = nil;
	hdlstring hstring;
	boolean flguest;
	boolean frontier4root;
	
	assert (sizeof (tycancoonwindowinfo) == 62); /* one padding byte after fontname which is 33 bytes long */
	
	assert (sizeof (tyversion2cancoonrecord) == 442);
	
	assert (sizeof (tyversion2cancoonrecord) == sizeof (tyOLD42version2cancoonrecord));
	
	if (!dbreference (adr, sizeof (info), &info))
		return (false);
	
	disktomemshort (info.versionnumber);
	disktomemlong (info.adrroottable);
	disktomemlong (info.adrscriptstring);
	disktomemshort (info.ixprimaryagent);
	
	hc = hcancoon; /*copy into register*/
	
	for (i = 0; i < ctwindowinfo; i++) { /*copy info.windowinfo into hc, with filtering*/
		
		tycancoonwindowinfo windowinfo;
		
		windowinfo = info.windowinfo [i]; /*copy record out of its array*/
		
		disktomemshort (windowinfo.fontsize);
		disktomemshort (windowinfo.fontstyle);
		
		windowinfo.w = nil; /*pointers stored on disk are invalid*/
		
		diskgetfontnum (windowinfo.fontname, &windowinfo.fontnum);
		
		(**hc).windowinfo [i] = windowinfo;
		} /*for*/
	
	/*cancooninitwindowinfo (hc, ixunused2);*/ 
	
	frontier4root = info.versionnumber < 3;
	
	if (frontier4root) {
		
		bigstring bsprompt;
		
		shellgetstring (openolddatabasestring, bsprompt);
		
		if (!twowaydialog (bsprompt, "\x07" "Convert", "\x06" "Cancel"))
			return (false);
		
		dbrefheapstring (info.adrscriptstring, &hstring);
		
		pullfromhandle ((Handle) hstring, 0, 1, nil); // pluck out the length byte
		}
	else
		dbrefhandle (info.adrscriptstring, (Handle *) &hstring);
	
	(**hc).hscriptstring = (Handle) hstring;
	
	(**hc).flflagdisabled = (info.flags & flflagdisabled_mask) != 0;
	
	(**hc).flpopupdisabled = (info.flags & flpopupdisabled_mask) != 0;
	
	(**hc).flbigwindow = (info.flags & flbigwindow_mask) != 0;
	
	ccwindowsetup ((**hc).flbigwindow, frontier4root); /*use info from the windowinfo array*/
	
	if (!ccloadsystemtable (hc, info.adrroottable, !flhavehost))
		return (false);
	
	flguest = systemtable == nil;
	
	#ifdef fltrialsize

	if (flguest) {
		
		(**cancoonwindowinfo).hdata = nil; /*unlink data from window to avoid crash*/
		
		shelltrialerror (noguestdatabasesstring);

		return (false);
		}

	#endif
	
	if (!flguest) {
		
		linksystemtablestructure (roottable);
		
		if (frontier4root) {
			
			if (resourcestable != nil)
				if (menugetmenubar (resourcestable, namemenubar, false, &hmenurecord))
					ccinstallmenubar (hc, hmenurecord); /*ignore error*/
			}
		
		if (ccinstalltablestructure (flhavehost))
			ccsetprimaryagent (info.ixprimaryagent);
		else
			return (false); /*6.1b4 AR: Fix crash after cancelling serial number dialog (at least on Win32)*/
		}
	
	(**hc).flguestroot = flguest;
	
	return (true);
	} /*loadversion2cancoonfile*/


#if 0 //def MACVERSION

static boolean loadoldcancoonfile (dbaddress adr, hdlcancoonrecord hcancoon) {
	
	tyfilespec fs, fsold;
	boolean fl;
	
	if (!msgdialog ("\50" "Convert 4.x database to 5.0? (This can take a while. A backup will be retained.)"))
		return (false);
	
	if (!loadversion2cancoonfile (adr, hcancoon))
		return (false);
	
	windowgetfspec (cancoonwindow, &fs);
	
	fsold = fs;
	
	pushstring ("\x04" ".v4x", fs.name);
	
	flconvertingolddatabase = true;
	
	fl = shellsaveas (cancoonwindow, &fs);
	
	flconvertingolddatabase = false;
	
	if (!fl)
		return (false);
	
	oserror (FSpExchangeFiles (&fs, &fsold)); // ignore, but report error
	
	windowsetfspec (cancoonwindow, &fsold); // the old fspec is now the new file
	
	return (true);
	} /*loadoldcancoonfile*/

#endif

#ifdef fltrialsize
	
static boolean cctrialviolation (void) {

	long eof;
	
	dbgeteof (&eof);
	
	if (eof > 7 * 0x0100000) {
		
		shelltrialerror (dbsizelimitstring);
		
		return (true);
		}
	
	return (false);
	} /*cctrialviolation*/

#endif

boolean ccloadfile (hdlfilenum fnum, short rnum) {
	
	/*
	9/15/92 dmb: removed support for version1 format; it never shipped
  
	6.1fc2 AR: Don't link cancoondata into (**cancoonwindowinfo).hdata.
	           This would crash some installations during the initialization
			   of the serial number dialog.
			   hdata should be a hdltableformats anyway.
	*/
	
	register hdlcancoonrecord hc = nil;
	dbaddress adr;
	short versionnumber;
	hdlcancoonrecord origglobals = cancoonglobals;
	boolean flhavehost = origglobals != nil;
	boolean flhide;
	
	cancoondata = nil; /*default error return*/
	
	if (!dbopenfile (fnum, false))
		return (false);
	
	#ifdef fltrialsize
		
		if (cctrialviolation ()) {
			
			if (!flhavehost)
				shellexitmaineventloop ();
			
			goto error;
			}
	
	#endif
	
	dbgetview (cancoonview, &adr);
	
	if (!dbreference (adr, sizeof (versionnumber), &versionnumber))
		goto error;
	
	disktomemshort (versionnumber);

	if (!newcancoonrecord (&cancoondata))
		goto error;
	
	hc = cancoondata; /*copy into register*/
	
	//(**cancoonwindowinfo).hdata = (Handle) hc; /*link data to window*/
	
	(**hc).hdatabase = databasedata; /*result from dbopenfile*/
	
	switch (versionnumber) {
		
		/*
		case 1:
			if (!loadversion1cancoonfile (adr, hc))
				goto error;
			
			return (true);
		*/
		
		case 2:
		case cancoonversionnumber:
			if (!loadversion2cancoonfile (adr, hc, flhavehost))
				goto error;
			
			if ((**hc).flguestroot)
				setcancoonglobals (origglobals); /*restore databasedata, etc.*/
			
			else {

				flhide = !ccinexpertmode () || (**hc).windowinfo [ixcancooninfo].flhidden;
				
				(**cancoonwindowinfo).flhidden = flhide;
				}
			
			// ccnewfilewindow (idscriptprocessor);
			
			return (true);
		
		default:
			alertstring (baddatabaseversionstring);
			
			goto error;
		} /*switch*/
	
	error:
	
	dbdispose ();
	
	clearcancoonglobals (); /*do this now to avoid debug check in disposehashtable*/
	
	disposecancoonrecord (hc); /*checks for nil*/
	
	setcancoonglobals (origglobals); /*restore databasedata, etc.*/
	
	return (false);
	} /*ccloadfile*/


boolean ccloadspecialfile (ptrfilespec fspec, OSType filetype) {
	
	/*
	7/28/92 dmb: use new finder2frontscript to set the Frontier.findertofront 
	global, instead of having Frontier.finder2click always set it to true. also, 
	handle updates now.
	
	2.1b3 dmb: only set finder2front to true if the sender of the 'odoc' event  
	was the Finder
	*/
	
	bigstring bspath;
	bigstring bs;
	ptrbyte pbool;
	
	if (!shellsetsuperglobals ()) {
		
		if (!shellopendefaultfile () || !shellsetsuperglobals ()) {
			
			getstringlist (langerrorlist, needopendberror, bs);
	
			shellerrormessage (bs);

			return (false);
			}
		}
	
	if (!filespectopath (fspec, bspath)) {
		
		filenotfounderror ((ptrstring) fsname (fspec));
		
		return (false);
		}
	
	langdeparsestring (bspath, chclosecurlyquote); /*add needed escape sequences*/
	
	if (getsystemtablescript (idfinder2frontscript, bs)) { /*frontier.findertofront=^0*/
		
	#ifdef MACVERSION
		if ((shellevent.what == kHighLevelEvent) && ((**landgetglobals ()).maceventsender == 'MACS'))
			pbool = bstrue;
		else
	#endif
			pbool = bsfalse;
		
		parsedialogstring (bs, pbool, nil, nil, nil, bs);
		
		langrunstringnoerror (bs, bs);
		}
	
	if (!getsystemtablescript (idfinder2clickscript, bs)) /*frontier.finder2click ("^0")*/
		return (false);
	
	parsedialogstring (bs, bspath, nil, nil, nil, bs);
	
	shellpartialeventloop (updateMask); /*handle updates first*/
	
	return (processrunstring (bs));
	} /*ccloadspecialfile*/


boolean ccsavespecialfile (ptrfilespec fs, hdlfilenum fnum, short rnum, boolean flsaveas, boolean flrunnable) {
	
	/*
	6.17.97 dmb: this is called by opverbs, wpverbs, etc. to save a disk file.
	we leave it to the system.misc.saveWindow script to package the data, 
	but we take care of writing it. (our caller did the file creation.)
	
	note: if we decide to make this more open architecture, we could add 
	another flat for the config record that would prevent the shell from 
	opening the data fork for us. then, we'd need to make this callback 
	take the destination filespec as a parameter too, and the saveWindow script
	would take both the window name and the filespec (which would be the same
	for a normal save).
	
	5.0.2b6 dmb: error checking on openfile
	
	5.0.2b8 dmb: don't unregister the window until we're about to set the new path
	*/
	
	bigstring bs, bspath;
	ptrstring bsrunnable;
	tyvaluerecord val;
	Handle hscript;
	boolean fl = false;
	boolean flopenedfile = false;
	hdlexternalvariable hv;
	
	(*shellglobals.getvariableroutine) ((Handle *) &hv);
	
//	if (hv != nil)
//		langexternalunregisterwindow (shellwindowinfo);
	
	if (!windowgetpath (shellwindow, bspath))
		shellgetwindowtitle (shellwindowinfo, bspath);
	
	langdeparsestring (bspath, chclosecurlyquote); /*add needed escape sequences*/

	if (!getsystemtablescript (idsavewindowscript, bs)) /*system.misc.saveWindow ("^0","^1")*/
		return (false);
	
	bsrunnable = (flrunnable? bstrue : bsfalse);

	parsedialogstring (bs, bspath, bsrunnable, nil, nil, bs);
	
	if (!newtexthandle (bs, &hscript))
		return (false);
	
	pushprocess (nil);
	
	fl = langrun (hscript, &val);
	
	popprocess ();
	
	if (!fl)
		return (false);
	
	pushhashtable (roottable); /*need temp stack services*/
	
	pushvalueontmpstack (&val);
	
	if (coercetostring (&val)) {
		
		if (fnum == 0) {
		
			fl = openfile (fs, &fnum, false);
			
			flopenedfile = fl;
			}
		
		if (fl)
			fl = 
				fileseteof (fnum, 0) &&
				
				filesetposition (fnum, 0) &&
				
				filewritehandle (fnum, val.data.stringvalue);
		
		if (flopenedfile)
			closefile (fnum);
		}
	
	cleartmpstack ();
	
	pophashtable ();
	
	if (hv != nil)
		langexternalunregisterwindow (shellwindowinfo);
	
	if (fl)
		windowsetfspec (shellwindow, fs);
	
	if (hv != nil)
		langexternalregisterwindow (hv);
	
	return (fl);
	} /*ccsavespecialfile*/


boolean ccnewrecord (void) {
	
	/*
	5.1.5b12 dmb: save the new structure after it's created, so if we crash we don't leave junk.
	*/
	
	register hdlcancoonrecord hc;
	hdlcancoonrecord origglobals = cancoonglobals;
	hdltablevariable hvariable;
	hdlhashtable htable;
	hdlfilenum fnum;
	
	cancoondata = nil; /*default error return*/
	
	if (!newcancoonrecord (&cancoondata))
		return (false);
	
	hc = cancoondata; /*copy into register*/
	
	cancoonglobals = hc; // 5.0a2 dmb: need this for more consistent state
	
	(**cancoonwindowinfo).hdata = (Handle) hc; /*link data to window*/
	
	fnum = windowgetfnum (cancoonwindow);
	
	if (!dbnew (fnum))
		goto error;
	
	dbsetview (cancoonview, nildbaddress);
	
	(**hc).hdatabase = databasedata;
	
	ccwindowsetup (false, true); /*init info in the windowinfo array*/
	
	if (!tablenewtable (&hvariable, &htable)) /*this will be the root table*/
		goto error;
	
	(**hvariable).id = idtableprocessor;
	
	(**hc).hrootvariable = (Handle) hvariable;
	
	(**hc).hroottable = htable;
	
	(**hc).flguestroot = true;
	
	if (!ccsavefile (nil, fnum, -1, false, false)) // 5.1.5b12
		goto error;
	
	setcancoonglobals (origglobals); /*restore databasedata, etc.*/
	
	return (true);
	
	error:
	
	dbdispose ();
	
	clearcancoonglobals (); /*do this now to avoid debug check in disposehashtable*/
	
	disposecancoonrecord (hc);
	
	setcancoonglobals (origglobals); /*restore databasedata, etc.*/
	
	return (false);
	} /*ccnewrecord*/


boolean ccnewfilewindow (tyexternalid id, WindowPtr *w, boolean flhidden) {
	
	tyvaluerecord val;
	hdlwindowinfo hinfo;
	
	if (!langexternalnewvalue (id, nil, &val))
		return (false);
	
	langexternalsetdirty ((hdlexternalhandle) val.data.externalvalue, false);
	
	if (!langexternalzoomfilewindow (&val, nil, flhidden)) {
		
		disposevaluerecord (val, false);
		
		return (false);
		}
	
	if (!langexternalwindowopen (val, &hinfo)) //shouldn't ever fail
		return (false);
	
	*w = (**hinfo).macwindow;
	
	return (true);
	} /*ccnewfilewindow*/


boolean ccsavefile (ptrfilespec fs, hdlfilenum fnum, short rnum, boolean flsaveas, boolean flrunnable) {
	
	/*
	5.0b18 dmb: last-minute fix. we crash when saving while publishing a table.
	the problem is that we toss stuff out during a save, and it would be nearly 
	impossible to protect against that everywhere. so we use processwaitforquiet
	and try to be the only thread using globals
	
	5.1.5b7 dmb: disabled above "fix". we now rely on shellwindowinfo and 
	outlinedata push/pop protection, fldispoosewhenpopped flags
	*/

	register hdlcancoonrecord hc = cancoondata;
	tyversion2cancoonrecord info;
	dbaddress adr;
	short i;
	boolean fl = false;
	
	databasedata = (**hc).hdatabase;

	if (flsaveas) {
		
		if (!dbstartsaveas (fnum))
			return (false);
		
		adr = nildbaddress;
		}
	else
		dbgetview (cancoonview, &adr);
	
	if (adr == nildbaddress)
		clearbytes (&info, sizeof (info));
	else
		if (!dbreference (adr, sizeof (info), &info))
			goto exit; 
	
	disktomemlong (info.adrroottable);
	disktomemlong (info.adrscriptstring);

	info.versionnumber = conditionalshortswap (cancoonversionnumber);
	
	for (i = 0; i < ctwindowinfo; i++) 
		ccupdatewindowinfo (i, &info);
	
	/*
	if (!processwaitforquiet (120L)) { //wait up to 2 seconds 
		
		sysbeep ();

		goto exit;
		}
	*/
	
	if (!tablesavesystemtable ((**hc).hrootvariable, &info.adrroottable))
		goto exit;
	
#ifdef version42orgreater
	info.windowinfo [ixcancooninfo].flhidden = (**shellwindowinfo).flhidden;

	if (!dbassignhandle ((**hc).hscriptstring, &info.adrscriptstring))
		goto exit;
#else
	if (!dbassignheapstring (&info.adrscriptstring, (**hc).hscriptstring))
		goto exit;
#endif
	
	memtodisklong (info.adrroottable);
	memtodisklong (info.adrscriptstring);

	clearbytes (&info.waste, sizeof (info.waste));
	
	info.flags = 0;
	
	if ((**hc).flflagdisabled)
		info.flags |= flflagdisabled_mask;
	
	if ((**hc).flpopupdisabled)
		info.flags |= flpopupdisabled_mask;
	
	if ((**hc).flbigwindow)
		info.flags |= flbigwindow_mask;
	
	ccgetprimaryagent (&info.ixprimaryagent);
	
	memtodiskshort (info.ixprimaryagent);
	
	if (!dbassign (&adr, sizeof (info), &info))
		goto exit;
	
	if (!flsaveas)
		dbflushreleasestack (); /*release all the db objects that were saved up*/
	
	dbsetview (cancoonview, adr);
	
	fl = true; // success!

	#ifdef fltrialsize
		
		if (cctrialviolation ()) // just issue message
			;
	
	#endif
	
	exit:

	if (flsaveas)
		dbendsaveas ();
	
	databasedata = (**cancoonglobals).hdatabase; // may not be same as cancoondata
	
	return (fl);
	} /*ccsavefile*/


boolean ccfindusedblocks (void) {
	
	tyversion2cancoonrecord info;
	dbaddress adr;
	
	dbgetview (cancoonview, &adr);
	
	if (adr != nildbaddress) {
		
		if (!dbreference (adr, sizeof (info), &info))
			return (false);

		statsblockinuse (adr, nil);
		
		disktomemlong (info.adrscriptstring);
		
		if (!statsblockinuse (info.adrscriptstring, nil))
			return (false);
		}
	
	return (langexternalfindusedblocks ((hdlexternalvariable) (**cancoonglobals).hrootvariable, nil));
	} /*ccfindusedblocks*/


boolean ccsetdatabase (void) {

	/*
	the cancoon window should be the only one that defines this callback.  it sets 
	db.c's global data handle to the database file that stores the menubar structure.
	
	this is used for other handlers who store data in our database.

	5.0a18 dmb: this is archaic, really. we manage databasedata ourselves, we don't 
	rely on the shell. with the new odb hosting feature, we need to wire this off
	and add some logic to ccsave
	*/
	
	#ifndef version5orgreater
		databasedata = (**cancoondata).hdatabase;
	#endif
	
	return (true);
	} /*ccsetdatabase*/


boolean ccgetdatabase (hdldatabaserecord *hdatabase) {

	/*
	5.0.2b21 dmb: the new way to do it
	*/
	
	*hdatabase = (**cancoondata).hdatabase;
	
	return (true);
	} /*ccsetdatabase*/


boolean ccclose (void) {
	
	/*
	the main window of cancoon is closing.
	*/
	
#ifdef version5orgreater
//	hdlwindowinfo hw = cancoonwindowinfo;
//
//	ccsavefile ((**hw).fnum, (**hw).rnum, false, false);	//odbSaveFile (ccodb);

	if ((cancoondata != nil) && (**cancoondata).flguestroot)  //defensive driving
		return (true);
#endif

	runshutdownscripts ();
	
	langexternalcloseregisteredwindows (false);
	
	return (true);
	} /*ccclose*/


static boolean ccverifywindowclose (WindowPtr pwindow) {
	
	/*
	4.0b7 dmb: new feature, run a script before we close a window
	*/
	
	if (flinhibitclosedialogs)	/*not allowed to do verification*/
		return (true);
	
	if (flscriptrunning)		/*not a user-generated close*/
		return (true);
	
	return (shellrunwindowconfirmationscript (pwindow, idclosewindowscript));
	} /*ccverifywindowclose*/


boolean ccpreclose (WindowPtr w) {
	
	/*
	4.1b5 dmb: new callback so we can verify root close (i.e. so 
	script can save it first, to avoid dialog
	*/
	
	if (!ccverifywindowclose (w))
		return (false);
	
	if (cancoondata == nil) //5.1.5b15 dmb
		return (true);
	
	if ((**cancoondata).flguestroot)
		return (true);
	
	return (langexternalcloseregisteredwindows (true));
	} /*ccpreclose*/


boolean ccchildclose (WindowPtr w) {
	
	/*
	a child window of cancoon is closing.  
	
	for now (6/12/90) just call the close routine on the child window.
	
	9/24/91 dmb: pass through return value from child
	
	12/5/91 dmb: call the child's dispose record routine too
	*/
	
	boolean fl;
	
	if (!ccverifywindowclose (w))
		return (false);
	
	shellpushglobals (w);
	
	fl = (*shellglobals.closeroutine) ();
	
	if (fl) {

		fl = (*shellglobals.disposerecordroutine) ();
		
		shellclearwindowdata ();
		}
	
	shellpopglobals ();
	
	return (fl);
	} /*ccchildclose*/


boolean ccdisposerecord (void) {
	
	/*
	1/22/91 dmb: added scan of new ccglobalsstack
	
	2.1b5 dmb: set windowinfo's data to nil; fixes revert bug when window is 
	updated between disposal & re-opening

	5.1.5b16 dmb: kill dependent processes before swapping in our globals, or
	visitprocessthreads will make the current thread appear to be dependent on them.
	*/
	
	register hdlcancoonrecord hc = cancoondata;
	hdlcancoonrecord origglobals = cancoonglobals;
	boolean flguestroot;
	
	if (hc == NULL)
		return (true);
	
	fwsNetEventShutdownDependentListeners ((long) (**hc).hdatabase);
	
	killdependentprocesses ((long) hc);

	setcancoonglobals (hc); /*make sure superglobals are ours*/
	
	flguestroot = (**hc).flguestroot;
	
//#ifndef WIN95VERSION
	if (!flguestroot)
		processyieldtoagents (); /*let them terminate*/
//#endif
	
	dbdispose (); /*do before clearing globals -- depends on databasedata*/
	
	fldisablesymbolcallbacks = true;
	
	if (!flguestroot) {
		
#ifndef PIKE
		#if TARGET_API_MAC_CARBON == 0
		langipcmenushutdown ();
		#endif
		
#endif
		
		unlinksystemtablestructure ();
		}
	
	clearcancoonglobals (); /*do this now to avoid debug check in disposehashtable*/
	
	disposecancoonrecord (hc);
	
	cancoondata = nil;

	(**cancoonwindowinfo).hdata = nil; /*unlink data from window*/
	
	fldisablesymbolcallbacks = false;
	
	bundle { /*scan saved globals so ccrestoreglobals knows when data is valid*/
		
		register short i;
		
		for (i = 0; i < cctopglobals; i++)
			if (ccglobalsstack [i] == hc)
				ccglobalsstack [i] = nil;
		}
	
	if (origglobals != hc)
		setcancoonglobals (origglobals);
	
	return (true);
	} /*ccdisposerecord*/


boolean ccdisposefilerecord (void) {
	
	hdlwindowinfo hw = shellwindowinfo;
	Handle x;
	tyvaluerecord val;
	
	if ((**hw).parentwindow != nil) // not standalone, nothing to do
		return (true);
	
	if (!(*shellglobals.getvariableroutine) (&x))
		return (false);
	
	setexternalvalue (x, &val);
	
	langexternaldisposevalue (val, false);
	
	shellclearwindowdata (); //shouldn't be necessary, since we're called by a disposerecordroutine
	
	return (true);
	} /*ccdisposefilerecord*/

	
boolean ccsetsuperglobals (void) {
	
	/*
	the cancoon window should be the only one that defines this callback.  it sets
	the cancoon.c global "cancoonglobals" to the one linked into our table.
	
	10/2/90 dmb: this can get called from cancoonbackground on a failure condition 
	during cancoonloadfile, so we need to check cancoondata for nil
	
	10/23/91 dmb: when cancoondata is nil, make sure we clear globals
	
	4.1b4 dmb: set and respect new supercancoonglobals global so we can handle 
	the case where our globals are already set, but they weren't set "super", 
	meaning the menubar was activated.
	*/
	
	register hdlcancoonrecord hc = cancoondata;
	
	if (hc == nil) {
		
		clearcancoonglobals (); /*clears everything relevant*/
		
		return (false);
		}
	
	if (hc != supercancoonglobals && !(**hc).flguestroot) {
		
		ccactivatemenubar (cancoonglobals, false);
		
		setcancoonglobals (hc);
		
		ccactivatemenubar (hc, true);
		
		supercancoonglobals = hc;
		}
	
	return (true);
	} /*ccsetsuperglobals*/


boolean ccbackground (void) {
	
	/*
	2.1b2 dmb: added call to new langerrorflush
	*/
	
	if (!shellsetsuperglobals ())
		return (false);
	
	processscheduler (); /*give the next process a shot at the machine*/
	
	langerrorflush (); /*make sure lang error is displayed*/
	
	return (true);
	} /*ccbackground*/


boolean ccfnumchanged (hdlfilenum newfnum) {
	
	/*
	part of the implementation of Save As

	5.0a18 dmb: no, it's not. it's never called.
	*/
	
	#ifdef version5orgreater
		return (true);
	#else
		ccsetdatabase ();
	
		return (dbfnumchanged (newfnum));
	#endif
	} /*ccfnumchanged*/


static boolean ccnewobjectcommand (short ixmenu) {
	
	/*
	"Script", noIcon, "N", noMark, plain,
	"WP-Text", noIcon, noKey, noMark, plain,
	"Outline", noIcon, noKey, noMark, plain,
	"Menubar", noIcon, noKey, noMark, plain,
	"Table", noIcon, noKey, noMark, plain,
	*/
	
	tyexternalid idobject;
	WindowPtr w;
	
	switch (ixmenu) {
		case 1:
			idobject = idscriptprocessor;
			break;
		
		case 2:
			idobject = idwordprocessor;
			break;
		
		/*
		case 3:
			idobject = idpictprocessor;
			break;
		*/
		
		case 3:
			idobject = idoutlineprocessor;
			break;
		
		case 4:
			idobject = idmenuprocessor;
			break;
		
		case 5:
			idobject = idtableprocessor;
			break;
		
		case 7:
			return (shellnew ());
		
		default:
			return (false);
		}
	
	return (ccnewfilewindow (idobject, &w, false));
	} /*ccnewobjectcommand*/


boolean ccinexpertmode (void) {
	
	/*
	5.0a18 dmb: use new langgetuserflag
	*/
	
	return (langgetuserflag (idinexpertmodescript, true));
	} /*ccinexpertmode*/


#if 0

static boolean cctoggleexpertmode (void) {

	/*
	5.0a3 dmb: we now call Frontier.setExpertMode, not system.misc.toggleExpertMode
	*/
	
	bigstring bsscript;
	boolean flexpert;
	ptrstring bsexpert;
	
	flexpert = !ccinexpertmode ();
	
	getsystemtablescript (idtoggleexpertmodescript, bsscript);
	
	bsexpert = (flexpert? bstrue : bsfalse);
	
	parsedialogstring (bsscript, bsexpert, nil, nil, nil, bsscript);
	
	langrunstringnoerror (bsscript, bsscript);
	
	if (ccinexpertmode () != flexpert)
		return (false);
	
	shellwindowmenudirty ();
	
	return (true);
	} /*cctoggleexpertmode*/

#endif


static boolean ccmenuroutine (short idmenu, short ixmenu) {
	
	/*
	this is the only menu handling routine in the program.  we have no idea 
	what window is in front, it could be of any type -- so we must get our 
	own globals.
	
	return false if we successfully handle the menu item, so no further 
	processing will occur
	
	7/31/90 dmb:  must return result that is inverse of memenu result to obey 
	hook conventions.  we shouldn't assume that we're the only menu hook.
	
	8/1/90 dmb:  idmenu of zero means update all hooked menus.
	
	8/31/90 DW: preserve globals around call to memenu.
	
	9/20/90 dmb:  push frontrootglobals, rather than nil, and test the result
	
	5.0a18 dmb: disabled expert mode code (added in 5.0a?)
	*/
	
	/*
	if (idmenu == 0 && ixmenu == 0) {
		
		hdlmenu hmenu = shellmenuhandle (filemenu);
		
		enablemenuitem (hmenu, expertitem);
		
		checkmenuitem (hmenu, expertitem, ccinexpertmode ());
		}
	
	if ((idmenu == filemenu) && (ixmenu == expertitem)) {
		
		cctoggleexpertmode ();
		
		return (false);
		}
	*/
	
	/*
	if (!idmenu) { /%request to update all of our menus%/
		
		/%all our menus are enabled, dmb 8/1/90%/
		
		mecheckmenubar (); /%see if menubar needs to be redrawn%/
		
		return (true);
		}
	*/
	
	if ((idmenu == virtualmenu) && (ixmenu == helpitem))
		return (!cchelpcommand ());
	
	if (idmenu == newobjectmenu) {
		
		ccnewobjectcommand (ixmenu);
		
		return (false);
		}
	
	#ifdef fldebug
	
	if (flscriptrunning && flcanusethreads) {
		
	// *** need this check? - DebugStr ("\x28" "handling menu w/script running in thread");
		
		return (true);
		}
	
	#endif
	
	return (!memenu (idmenu, ixmenu));
	} /*ccmenuroutine*/


static boolean ccchecktablestructureglobals (hdlhashtable htable) {
	
	/*
	10/25/91 dmb: check table structure when something in the verbs table changes

	5.0a18 dmb: make sure we haven't just unloaded. maybe we're quitting
	and agents just terminated
	*/
	
	register hdlhashtable ht = htable;
	
	if (fldisablesymbolcallbacks)
		return (false);
	
	if (roottable == nil)
		return (false);

	if ((ht == roottable) || (ht == systemtable) || (ht == verbstable))
		return (settablestructureglobals (rootvariable, false));
	
	return (true);
	} /*ccchecktablestructureglobals*/


static boolean ccsymbolchanged (hdlhashtable htable, const bigstring bsname, hdlhashnode hnode, boolean flvalue) {
	
	/*
	used as a callback routine.  the value of the indicated variable has changed.
	if the cell is being displayed, update the display of it.  this is how the
	real-time changing of variables is implemented.
	
	2/28/91 dmb: also call ccchecktablestructureglobals to make sure our 
	globals are up to date
	
	11/14/01 dmb: also call new langexternalsymbolchanged. we should really create 
	a new callback so we don't have to call them explicitly here
	*/
	
	if (!ccchecktablestructureglobals (htable))
		return (false);
	
	langipcsymbolchanged (htable, bsname, flvalue);
	
	langexternalsymbolchanged (htable, bsname, hnode, flvalue);

	return (tablesymbolchanged (htable, bsname, hnode, flvalue));
	} /*ccsymbolchanged*/


static boolean ccsymbolinserted (hdlhashtable htable, const bigstring bsname, hdlhashnode hnode) {
	
	if (!ccchecktablestructureglobals (htable))
		return (false);
	
	langipcsymbolinserted (htable, bsname);
	
	langexternalsymbolinserted (htable, bsname, hnode);

	return (tablesymbolinserted (htable, bsname));
	} /*ccsymbolinserted*/


static boolean ccsymboldeleted (hdlhashtable htable, const bigstring bsname) {
	
	if (!ccchecktablestructureglobals (htable))
		return (false);
	
	langipcsymboldeleted (htable, (ptrstring) bsname);
	
	return (tablesymboldeleted (htable, bsname));
	} /*ccsymboldeleted*/
	

static short cccomparenodes (hdlhashtable htable, hdlhashnode hnode1, hdlhashnode hnode2) {
	
	return (tablecomparenodes (htable, hnode1, hnode2));
	} /*cccomparenodes*/


static boolean ccsaveglobals (void) {
	
	assert (cctopglobals <= maxsavedccglobals);
	
	ccglobalsstack [cctopglobals++] = cancoonglobals;
	
	return (true);
	} /*ccsaveglobals*/


static boolean ccrestoreglobals (void) {
	
	/*
	10/23/91 dmb: when stack entry is nil, clear globals before 
	returning false; we don't want other globals to remain in place
	*/
	
	register hdlcancoonrecord hc = ccglobalsstack [--cctopglobals];
	
	assert (cctopglobals >= 0);
	
	if (hc == nil) {
		
		clearcancoonglobals ();
		
		return (false);
		}
	
	if (hc != cancoonglobals)
		setcancoonglobals (hc);
	
	return (true);
	} /*ccrestoreglobals*/


static boolean ccpartialeventloop (short desiredevents) {
	
	/*
	3/23/93 dmb: don't use ccsave/restoreglobals, 'cause we don't want to restore
	nil globals (in case we're being called at an odd time, like from an error 
	alert during initialization
	*/
	
	boolean fl;
	hdlcancoonrecord hc;
	
	hc = cancoonglobals;
	
	fl = shellpartialeventloop (desiredevents);
	
	if (hc != cancoonglobals)
		setcancoonglobals (hc);
	
	return (fl);
	} /*ccpartialeventloop*/


boolean ccstart (void) {
	
	/*
	set up callback routines record, and link our data into the shell's 
	data structure.
	*/
	
	langcallbacks.symbolchangedcallback = &ccsymbolchanged;
	
	langcallbacks.symbolunlinkingcallback = &processsymbolunlinking;
	
	langcallbacks.symboldeletedcallback = &ccsymboldeleted;
	
	langcallbacks.symbolinsertedcallback = &ccsymbolinserted;
	
	langcallbacks.comparenodescallback = &cccomparenodes;
	
	langcallbacks.saveglobalscallback = &ccsaveglobals;
	
	langcallbacks.restoreglobalscallback = &ccrestoreglobals;
	
	langcallbacks.partialeventloopcallback = &ccpartialeventloop;
	
	shellpushmenuhook (&ccmenuroutine);
	
	return (ccwindowstart ());
	} /*ccstart*/




