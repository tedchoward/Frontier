
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

#include "cursor.h"
#include "dialogs.h"
#include "file.h"
#include "kb.h"
#include "launch.h"
#include "memory.h"
#include "resources.h"
#include "strings.h"
#include "frontierwindows.h"
#include "zoom.h"
#include "cancoon.h"
#include "shell.rsrc.h"
#include "shell.h"
#include "shellhooks.h"
#include "shellprivate.h"
#include "shellundo.h"
#include "lang.h"
#include "langinternal.h"
#include "tablestructure.h"


	#include "mac.h"
	#include <uisharing.h>
	


#define str_desktopscript	BIGSTRING ("\x04" "ftds")
#define str_normalscript	BIGSTRING ("\x04" "ftsc")
#define type_desktopscript	'FTds'
#define type_normalscript	'FTsc'
#define type_wptext			'FTwp'
#define type_plaintext		'TEXT'


boolean flinhibitclosedialogs = false; /*global for close routines*/

boolean flconvertingolddatabase = false; /*global for saveas operation*/

boolean flshellclosingall = false; /*global for ccwindowpreclose*/


typedef struct findvisitinfo {
	
	ptrfilespec fsfind;

	WindowPtr *wfound;

	boolean flhidden;
	} tyfindvisitinfo, *ptrfindvisitinfo;


static boolean findfilevisit (WindowPtr w, ptrvoid refcon) {
	
	/*
	if the filespec for the given window is the one we're looking for, 
	select that window and stop visiting
	
	5.1b22 dmb: make sure the window is visible by calling shellbringtofront
	*/
	
	ptrfindvisitinfo info = (ptrfindvisitinfo) refcon;
	tyfilespec fs;
	hdlwindowinfo hinfo;
	
	windowgetfspec (w, &fs);
	
	if (equalfilespecs ((*info).fsfind, &fs)) {
		
		getwindowinfo (w, &hinfo);
		
		if (!(*info).flhidden)
			shellbringtofront (hinfo);
		
		if ((*info).wfound != nil)
			*(*info).wfound = w;
		
		return (false);
		}
	
	return (true);
	} /*findfilevisit*/


boolean shellopenfile (ptrfilespec fs, boolean flhidden, WindowPtr *wnew) {
	
	//
	// open the specified file in a new window.
	//
	// 2006-08-26 creedon:	for Mac, FSRef-ized
	//
	//				for Mac, set the proxy icon to LAND/TABL
	//
	// 6.0a11 dmb: fixed wnew handling for already-open windows
	//
	// 3.0.2 dmb: if asked to open a file that's already open in a root window, just bring it to the front and return true
	//
	// 2.1b3 dmb: resolve alias files
	//
	// 1992-08-20 dmb:	since handling of update events can push/pop globals, we need to re-establish default globals
	//				after calling partialeventloop
	//
	// 1992-07-29 dmb:	moved handling of update events after loadspecial handler, so that is can see what kind of event
	//				triggered the open
	//
	// 1990-11-08 dmb: push default globals so that caller doesn't have to
	//
	// 1990-08-31 DW:	replace setglobals calls with shellpush/popglobals.  also commented out the call to
	//				shellfrontglobals.
	//
	// 1990-08-20 dmb:	added resfile support.
	//
	//				show window after success (window now initially invisible).
	//
	
	WindowPtr w;
	hdlfilenum fnum;
	short rnum;
	boolean fl;
	OSType filetype;
	tyfindvisitinfo info;
	

		bigstring bsext, bs;
		

	if (wnew != nil)
		*wnew = nil;
	
	info.fsfind = fs;
	info.wfound = wnew;
	info.flhidden = flhidden;
	
	if (!shellvisittypedwindows (iddefaultconfig, &findfilevisit, &info))
		return (true);
	
	initbeachball (right);
	
	if (!fileresolvealias (fs))
		return (false);
	
	if (!getfiletype (fs, &filetype))
		return (false);
	

		getfsfile ( fs, bs );

		lastword ( bs, '.', bsext);

		if (equalidentifiers (bsext, "\proot"))
			filetype = 'TABL';
			
	
	if (!shellpushdefaultglobals ())
		return (false);
	
	if (filetype != config.filetype) {
		
		fl = (*shellglobals.loadspecialroutine) (fs, filetype);
		
		shellpopglobals ();
		
		return (fl);
		}
	
	shellpopglobals (); /*next statement may smash 'em*/
	
	shellpartialeventloop (updateMask); /*handle updates*/
	
	shellpushdefaultglobals (); /*set 'em back up*/
	
	w = nil;
	
	rnum = -1;
	
	if (!openfile (fs, &fnum, false))
		goto error;
	
	
		if (config.flopenresfile)
			if (!openresourcefile (fs, &rnum, resourcefork)) // 2005-09-02 creedon - added support for fork parameter,
										// see resources.c: openresourcefile and pushresourcefile
				goto error;
				
	
	if (!newfilewindow (fs, fnum, rnum, flhidden, &w))
		goto error;
	
	
		SetWindowProxyCreatorAndType ( w, 'LAND', 'TABL', kOnSystemDisk );
	
	
	if (wnew != nil)
		*wnew = w;
	
	shellpushglobals (w);
	
	fl = (*shellglobals.loadroutine) (fnum, rnum);
	
	setcursortype (cursorisarrow); /*don't leave stopped beach ball lingering*/
	
	shellpopglobals ();
	
	if (!fl)
		goto error;
	
	zoomfilewindow (w);
	
	if (!flhidden)
		windowinval (w); /*be sure absolutely everything gets drawn*/
	
	shellpopglobals ();
	
	shellcallfilehooks (); /*a file was just opened or closed*/
	
	return (true);
	
	error:

	shellpopglobals ();
	
	disposeshellwindow (w);
	
	closefile (fnum);

	
		closeresourcefile (rnum);
		
		
	return (false);
	
	} // shellopenfile
	

static boolean clickersvisit (hdlhashnode hnode, ptrvoid refcon) {
	
	ptrsftypelist psftypes = (ptrsftypelist) refcon;
	bigstring bs;
	byte bstype [8];
	
	gethashkey (hnode, bs);
	
	if (patternmatch (BIGSTRING ("\x04type"), bs) == 1) {
		
		midstring (bs, 5, 4, bstype);
		
		stringtoostype (bstype, &(*psftypes).types [(*psftypes).cttypes++]);
		}
		
	return ((*psftypes).cttypes < maxsftypelist);
	} /*clickersvisit*/


boolean shellopen (void) {
	
	//
	// 2006-10-18 creedon: for Mac, FSRef-ized
	//
	// 5.0d3 dmb: look in clickers table for types we can actually open
	//
	// 2.1b4 dmb: option-open to show all types
	// 
	
	tyconfigrecord lconfig;
	tyfilespec fspec;
	tysftypelist filetypes;
	ptrsftypelist ptypes = nil;
	hdlhashtable htable;
	bigstring bs;
	boolean fl;
	hdlwindowinfo hdummy;
	
	if (!keyboardstatus.floptionkey) { // 2.1b4
		
		shellgetconfig (iddefaultconfig, &lconfig);
		
		filetypes.cttypes = 1;
		
		filetypes.types [0] = lconfig.filetype;
		
		// filetypes.types [1] = '2CLK'; // application glue files
		
		ptypes = &filetypes;
		}
	
	getsystemtablescript (idfrontierclickers, bs); // "Frontier.clickers.typeXXXX"
	
	disablelangerror ();

	fl = langexpandtodotparams (bs, &htable, bs);
	
	enablelangerror ();
	
	if (fl)
		hashtablevisit (htable, &clickersvisit, &filetypes);
	
	clearbytes (&fspec, sizeof (fspec));
	
	// RAB: 1/27/98  - make this look better when opening a file after the root is open
	
	if (ccfindrootwindow (&hdummy) == false)
		getstringlist (defaultlistnumber, startupfileprompt, bs);
	else
		getstringlist (defaultlistnumber, nonstartupfileprompt, bs);
	
	if (!sfdialog (sfgetfileverb, bs, ptypes, &fspec, 'LAND')) // 2005-10-06 creedon - added 'LAND'
		return (false);
	
	return ( shellopenfile ( &fspec, false, nil ) );
	
	} // shellopen


static void prepuserforwait (WindowPtr w) {
#	pragma unused(w)

	/*
	4.1.1b1 dmb: call langpartialeventloop when appropriate

	7.0b16 PBS: Fix for Windows display glitch -- don't gray windows for save
	if this is Windows, it leads to the display getting messed up for the frontmost
	outline.
	
	2009-09-18 aradke: Fix for Carbon OS X display glitch -- don't gray windows while saving
	because it would sometimes gray out the menubar and then let it stay grayed out indefinitely.
	*/
	
	initbeachball (left);
	
	if (flscriptrunning)
		langpartialeventloop (updateMask);
	else
		shellpartialeventloop (updateMask);

	} /*prepuserforwait*/


static boolean killundovisit (WindowPtr w, ptrvoid refcon) {
#pragma unused (refcon)

	if (shellpushglobals (w)) {
		
		killundo ();
		
		shellpopglobals ();
		}
	
	return (true);
	} /*killundovisit*/


void killownedundo (WindowPtr w) {
	
	/*
	3/8/91 dmb: added this routine to make sure that all undos are tossed 
	before saving.  we'd be able to get rid of this if we made the undo 
	stacks part of the superglobals, and added support for file-specific 
	instead of window-specific undo
	*/
	
	killundo ();
	
	visitownedwindows (w, -1, &killundovisit, nil);
	} /*killownedundo*/


static boolean shellsavefile (WindowPtr w, ptrfilespec fspec, hdlfilenum fnum, short rnum, boolean flsaveas, boolean flrunnable) {
	
	/*
	this just factors out some code common to all save routines in this file
	*/
	
	killownedundo (w);
	
	prepuserforwait (w);
	
	return ((*shellglobals.saveroutine) (fspec, fnum, rnum, flsaveas, flrunnable));
	} /*shellsavefile*/


static boolean shelldatabasesaveas (WindowPtr wsave, ptrfilespec fspec) {
	
	//
	// a save-as designed for applications that work with a file that it reads and writes while the user is working.
	//
	// this routine now actually implements a "save a copy"; the newly-saved file does not become the current file.
	//
	// 2006-09-18 creedon: for Mac, set window proxy icon
	//
	// 1991-07-05 dmb: use new copyallresources routine to handle the resource fork.
	//
	// 1990-10-03 dmb: rewrote
	//
	
	register WindowPtr w = wsave;
	hdlfilenum fnum;
	short rnum;
	hdlwindowinfo hinfo;
	
	if (!opennewfile (fspec, config.filecreator, config.filetype, &fnum))
		return (false);
	
	rnum = -1;


	if (config.flopenresfile)
		if (!openresourcefile (fspec, &rnum, resourcefork)) { // 2005-09-02 creedon - added support for fork parameter, see
									       // resources.c: openresourcefile and pushresourcefile
			
			closefile (fnum);
			
			return (false);
			}
	
	getwindowinfo (w, &hinfo);
	

	if (macfilespecisresolvable (fspec))
		SetWindowProxyCreatorAndType ( w, 'LAND', config.filetype, kOnSystemDisk );
		
		
	/*
	shellbringtofront (hinfo);
	
	shellclosechildwindows (hinfo); // Save As can't maintain these, so close them now
	*/
	
	if (!shellsavefile (w, fspec, fnum, rnum, true, false)) {
		
		closefile (fnum);

		closeresourcefile (rnum);
		
		deletefile (fspec);
		
		return (false);
		}
	
	if (config.flopenresfile)
		copyallresources ((**hinfo).rnum, rnum);

	
	if (!flconvertingolddatabase) { // close new file; we're really doing a "Save a Copy"
	
		closefile (fnum);
		
		closeresourcefile (rnum);
		
		}
	else { // close old file and re-open newly-converted file
		
		(*shellglobals.closeroutine) ();
		
		(*shellglobals.disposerecordroutine) ();
		
		shellclearwindowdata ();

		closewindowfile (w); // close old file
		
		(**hinfo).flneversaved = false; // must do after closewindowfile
		
		windowsetfspec (w, fspec);
		
		windowsetfnum (w, fnum);
		
		windowsetrnum (w, rnum);
		
		if (!(*shellglobals.loadroutine) (fnum, rnum)) {
			
			closewindowfile (w); // close new file
			
			disposeshellwindow (w);
			
			alertdialog ((ptrstring) "\x47" "A copy of the file was successfully converted, but could not be opened.");
			
			return (false);
			}
		
		windowsetchanges (w, false);
		}
	
	flushvolumechanges (fspec, fnum); // write changes out to disk
	
	return (true);
	
	} // shelldatabasesaveas


static boolean shellnormalsaveas (WindowPtr wsave, ptrfilespec fspec, boolean flrunnable) {
	
	//
	// 2006-09-18 creedon: for Mac, set window proxy icon
	//
	// 5.0.2b19 dmb: special case file type for runnable
	//
	// 8/20/90 dmb: added resfile support
	//
	
	register WindowPtr w = wsave;
	hdlfilenum fnum;
	short rnum = -1;
	tyfilespec fs;
	OSType filetype = config.filetype;
	OSType type;
	
	switch (filetype) { // handle special cases
	
		case type_normalscript:
			if (flrunnable)
				filetype = type_desktopscript;
			
			break;
		
		case type_wptext:
			if (windowgetfspec (w, &fs) && getfiletype (&fs, &type) && (type == type_plaintext))
				filetype = type_plaintext;
			
			break;
		}
	
	if (!opennewfile (fspec, config.filecreator, filetype, &fnum))
		return (false);
	
	if (config.flopenresfile) {
	
		if (!openresourcefile (fspec, &rnum, resourcefork)) { // 2005-09-02 creedon - added support for fork parameter, see
									       // resources.c: openresourcefile and pushresourcefile
			
			closefile (fnum);
			
			return (false);
			}
		}
	
	if (shellsavefile (w, fspec, fnum, rnum, true, flrunnable)) { 
		
		(**shellwindowinfo).flneversaved = false;
		
		flushvolumechanges (fspec, fnum); // write changes out to disk
		
		closefile (windowgetfnum (w));
		
	
		closeresourcefile (windowgetrnum (w));
		
		SetWindowProxyCreatorAndType ( w, 'LAND', filetype, kOnSystemDisk );
		
	
		windowsetchanges (w, false);
		
		windowsetfnum (w, fnum);
		
		windowsetrnum (w, rnum);
		
		windowsetfspec (w, fspec);
		}
	
	return (true);
	
	} // shellnormalsaveas


boolean shellsaveas (WindowPtr wsave, ptrfilespec fspec, boolean flrunnable) {
	
	//
	// if we're given a path, save the current file to that location with no dialog.
	//
	// if bspath is nil, use standard file to determine the new file location
	//
	// 2006-08-26 creedon:	for Mac, FSRef-ized
	//
	//				for Mac, add object type extension to file name
	//
	// 5.0d8 dmb: pass file type to sfdialog for Win
	//
	// 1991-12-12 dmb: make sure we don't use a full path as the default file name
	//
	
//	#if TARGET_API_MAC_CARBON == 1 /*7.0b50 PBS: set window title icon*/
//		OSStatus err = noErr;
//	#endif
	
	register boolean fl;
	register WindowPtr w;
	tyfilespec fs;
	tysftypelist filetypes;
	
	if (!shellpushrootglobals (wsave)) // save as only operates on root windows
		return (false);
	
	w = shellwindow; // move into register
	
	if (fspec != nil) {
		
		fs = *fspec;
		
		fl = true;
		}
	else {
		
		if ( ! windowgetfspec ( w, &fs ) ) {
		
			bigstring bsname;
			
			getdefaultfilename ( bsname );

				
				if ( fsnamelength ( &fs.name ) <= 0 ) {
					
					bigstringtofsname ( bsname, &fs.name );
					}
			


			}
		
		filetypes.cttypes = 1;

		filetypes.types [0] = config.filetype;

		if (flrunnable) { // special case for desktop scripts.
			bigstring bsfile, bssuffix;
			
			getfsfile (&fs, bsfile);
			
			lastword (bsfile, '.', bssuffix);
			
			if (equalidentifiers (bssuffix, str_normalscript)) {
				
				replacestring (bsfile, stringlength (bsfile) - 3, 4, str_desktopscript);
				
				setfsfile (&fs, bsfile);
				}
			
			filetypes.types [0] = type_desktopscript;
			}
		
		fl = sfdialog (sfputfileverb, nil, &filetypes, &fs, 'LAND'); // 2005-10-06 creedon - added 'LAND'
		}
	
	if (fl) {
		
		if (config.flcreateonnew)
			fl = shelldatabasesaveas (w, &fs);
		else
			fl = shellnormalsaveas (w, &fs, flrunnable);
			
//		#if TARGET_API_MAC_CARBON == 1 /*7.0b50 PBS: set window title icon*/
			
//			err = SetWindowProxyFSSpec (w, &fs);

//		#endif
		}

	shellpopglobals ();
	
	return (fl);
	
	} // shellsaveas


boolean shellnewfile (ptrfilespec fspec, boolean flhidden, WindowPtr *wnew) {
	
	/*
	create a new file with the default config
	
	7/27/90 dmb: clean up after error.
	
	8/20/90 dmb: show window after success.  added resfile support.
	
	8/31/90 DW: eliminate setglobals calls.
	
	11/8/90 dmb: fixed globals push/pop imbalance
	
	2/6/91 dmb: take fname, vnum as parameters so verbs can call us
	*/
	
	hdlfilenum fnum = 0;
	short rnum = -1;
	WindowPtr w = nil;
	boolean fl;
	
	shellpushdefaultglobals (); // so that config is correct
	
	if (config.flcreateonnew) {
		
		if (!opennewfile (fspec, config.filecreator, config.filetype, &fnum))
			goto error;

	
		if (config.flopenresfile)
			if (!openresourcefile (fspec, &rnum, resourcefork)) // 2005-09-02 creedon - added support for fork parameter,
										     // see resources.c: openresourcefile and pushresourcefile
				goto error;
	
		}
	
	if (!newfilewindow (fspec, fnum, rnum, flhidden, wnew)) 
		goto error;
	
	w = *wnew;
	
	
		SetWindowProxyCreatorAndType ( w, 'LAND', 'TABL', kOnSystemDisk );
	
	
	shellpushglobals (w);
	
	(**shellwindowinfo).flneversaved = (boolean) config.flcreateonnew;
	
	fl = (*shellglobals.newrecordroutine) ();
	
	shellpopglobals ();
	
	if (!fl)
		goto error;
	
	zoomfilewindow (w);
	
	shellpopglobals ();
	
	return (true);
	
	error:
	
	disposeshellwindow (w); // checks for nil
	
	if (fnum != 0) { // we actually opened a file
		
		closefile (fnum);
		
			closeresourcefile (rnum);
		
		deletefile (fspec);
		}
	
	shellpopglobals ();
	
	return (false);
	
	} // shellnewfile
	

boolean shellnew (void) {
	
	//
	// new front end to shellnewfile, isolates user interface
	//
	// 2006-08-26 creedon:	for Mac, FSRef-ized
	//				for Mac, add ".root" to bsname
	//
	// 2005-10-06 creedon - added 'LAND' to sfdialog call
	//
	// 5.0d8 dmb: pass file type to sfdialog for Win
	//
	
	tyconfigrecord lconfig;
	tyfilespec fspec;
	tysftypelist filetypes;
	WindowPtr w;
	bigstring bsname;
	
	shellgetconfig (iddefaultconfig, &lconfig);
	
	clearbytes (&fspec, sizeof (fspec));
	
	getuntitledfilename ( bsname );
	
	
		pushstring ( BIGSTRING("\x05"".root"), bsname );
				
		bigstringtofsname ( bsname, &fspec.name );



	if (lconfig.flcreateonnew) {
		
		filetypes.cttypes = 1;

		filetypes.types [0] = lconfig.filetype;
		
		if (!sfdialog (sfputfileverb, nil, &filetypes, &fspec, 'LAND')) // user canceled
			return (false);
		}
	
	return (shellnewfile (&fspec, false, &w));
	
	} // shellnew
	
	
boolean shellsave (WindowPtr wsave) {
	
	/*
	6/14/91 dmb: dirty window menu after successful save
	*/
	
	register WindowPtr w;
	tyfilespec fspec;
	boolean fl;
	boolean flrunnable;
	OSType ostype;
	bigstring osstring;

	if (!shellpushrootglobals (wsave)) /*save only operates on root windows*/
		return (false);
	
	w = shellwindow; /*move into register*/
	
	if (config.flstoredindatabase) {
		
		windowgetfspec (w, &fspec);
		
		(*shellglobals.setdatabaseroutine) (); /*make sure db.c is ready to be called*/
		}
	
	else {
		if (!windowgetfspec (w, &fspec)) { /*no file open to save to, prompt the user for name*/
			
			shellpopglobals ();
			
			return ( shellsaveas ( w, nil, false ) );
			}
		}

	//RAB: 1/22/98 - set runnable if the file extension is ftds
	flrunnable = false;

	if (getfiletype (&fspec, &ostype)) {
		ostypetostring (ostype, osstring);

		//use string since the windows type can be any case
		if (equalidentifiers (osstring, str_desktopscript))
			flrunnable = true;
		}

	fl = shellsavefile (w, &fspec, windowgetfnum (w), windowgetrnum (w), false, flrunnable);
	
	if (fl) { /*no error saving*/
		
		(**shellwindowinfo).flneversaved = false;
		

		windowgetfspec (w, &fspec);
		
		flushvolumechanges (&fspec, windowgetfnum (w));
		
		windowsetchanges (w, false);
		
		shellwindowmenudirty ();
		}
	
	shellpopglobals ();
	
	return (fl);
	} /*shellsave*/


boolean shellclosechildwindows (hdlwindowinfo hparent) {
	
	/*
	2.1b1 don't ignore childcloseroutine's return value, and pass it 
	along to caller

	5.0b12 dmb: I don't know if we can ever end up reentering a child 
	window's close routine, but not what the flag is in the hdlwindowinfo,
	let's make sure. we can't selectively skip children as coded, so we 
	just bail
	
	5.0.2b13 dmb: set flbeingclosed while we call childcloseroutine
	*/
	
	while (true) {
		
		hdlwindowinfo hchild;
		register WindowPtr w;
		boolean flclosed;
		
		if (!shellfirstchildwindow (hparent, &hchild))
			return (true);
		
		if ((**hchild).flbeingclosed) //don't reenter
			return (true);
		
		(**hchild).flbeingclosed = true;
		
		shellpushglobals ((**hparent).macwindow);
		
		w = (**hchild).macwindow;
		
		flclosed = (*shellglobals.childcloseroutine) (w);
		
		shellpopglobals ();
		
		(**hchild).flbeingclosed = false;
		
		if (!flclosed)
			return (false);
		
		disposeshellwindow (w);
		} /*while*/
	
	return (true);
	} /*shellclosechildwindows*/
	
	
boolean shellclosewindow (WindowPtr wclose) {
	
	/*
	close the indicated child window
	*/
	
	register WindowPtr w = wclose;
	hdlwindowinfo hinfo, hparentinfo;
	boolean flclosed;
	
	if (w == nil) /*closing a nil window is very easy*/
		return (true);
	
	getwindowinfo (w, &hinfo);
	
	hparentinfo = (**hinfo).parentwindow;
	
		if (hparentinfo == nil) // don't close -- hide
			return (shellhidewindow (hinfo));
	
	shellpushglobals ((**hparentinfo).macwindow); /*close message goes to parent window*/
	
	flclosed = (*shellglobals.childcloseroutine) (w);
	
	shellpopglobals ();
	
	if (!flclosed)
		return (false);
	
	disposeshellwindow (w); 
	
	return (true);
	} /*shellclosewindow*/


boolean shellclose (WindowPtr wclose, boolean fldialog) {
	
	/*
	return false if the user made changes and chooses not to discard them.
	
	dmb 9/14/90:  added killundo to release memory before close
	
	3/26/93 dmb: check result of getwindowinfo
	
	4.1b5 dmb: added shellglobals.precloseroutine for ccpreclose
	
	4.1b6 dmb: added flclosingmainshellwindow to prevent reentracy

	5.0b7 dmb: recently-added autosave flag is now checked right

	5.0b12 dmb: replaced static flclosingmainshellwindow with windowinfo's
	flbeingclosed

	5.1.5b15 dmb: wait for activates before putting up save dialog; closeall
	doesn't do it for us anymore.
	*/
	
	register WindowPtr w = wclose;
	hdlwindowinfo hinfo;
	long ctbytes;
	
	if (w == nil) /*closing a nil window is very easy*/
		return (true);

	
	if (!isshellwindow (w)) {
		
		if (uisIsSharedWindow (w))
			return (uisCloseSharedWindow (w));
		
		return (false);
		}
	
	
	if (!getwindowinfo (w, &hinfo))
		return (false);
	
	if ((**hinfo).parentwindow != nil) /*part of another window's data structure*/
		return (shellclosewindow (w));
	
	if ((**hinfo).flbeingclosed) /*don't reenter*/
		return (true);
	
	(**hinfo).flbeingclosed = true;
	
	shellpushglobals (w);
	
	if (fldialog && !(*shellglobals.precloseroutine) (w))
		goto cancelled;
	
	if (fldialog && (**hinfo).flmadechanges) {
		
		bigstring bs;
		
		if (config.flstoredindatabase && langgetuserflag (idautosave, true)) { //5.0a23 dmb: databases save w/out confirmation
				
				if (!keyboardstatus.flshiftkey) // shift disables autosave
					if (!shellsave (w))
						goto cancelled;		
			}
		else {
			shellgetwindowtitle (hinfo, bs); // 7.24.97 dmb: was windowgettitle
			
			filefrompath (bs, bs);
			
			if (flscriptrunning)
				langpartialeventloop (activMask);
			else
				shellpartialeventloop (activMask); /*wait for activates*/

			switch (savedialog (bs)) {
				
				case 1: /*OK*/
					if (!shellsave (w))
						goto cancelled;
					
					break;
				
				case 2: /*discard changes*/
					break;
					
				case 3: /*cancel, continue editing*/
					goto cancelled;
				} /*switch*/
			}
		}
	else { /*changes were not made*/
		
		/*
		if ((**hinfo).flchangedposition)
			shellsave (w);
		*/
		}
	
	killownedundo (w);
	
	flinhibitclosedialogs = true; /*it's too late to stop the process now*/
	
	(*shellglobals.closeroutine) ();
	
	shellclosechildwindows (hinfo); /*close any windows that parent didn't close*/
	
	flinhibitclosedialogs = false; /*keep it cleared*/
	
	(*shellglobals.disposerecordroutine) ();
	
	shellclearwindowdata ();
	
	shellpopglobals ();
	
	zoomtoorigin (w);
	
	closewindowfile (w);
	
	(**hinfo).flbeingclosed = false; //it's about to be disposed, but might as well
	
	disposeshellwindow (w);
	
	shellcallfilehooks (); /*a file was just opened or closed*/
	
	ctbytes = longinfinity;

	hashflushcache (&ctbytes); //rab 6.0b6 free the memory

	return (true);
	
	cancelled:
	
		shellpopglobals ();
		
		(**hinfo).flbeingclosed = false;
		
		return (false);
	} /*shellclose*/


static boolean shellclosefilewindowvisit (WindowPtr wclose, ptrvoid fldialog) {
	
	/*
	5.0.2b13 dmb: bring to front before shellclose if fldialog
	*/
	
	hdlwindowinfo hinfo;
	
	if (!getwindowinfo (wclose, &hinfo))
		return (false);
	
	if ((**hinfo).parentwindow != nil)
		//return (shellclosewindow (wclose));
		return (true);
	
	if ((**hinfo).configresnum != iddefaultconfig) {
	
		if (fldialog) {
			
			shellbringtofront (hinfo);
			
			shellupdatenow ((**hinfo).macwindow);
			}
		
		return (shellclose (wclose, (boolean) ((long) fldialog)));
		}
	
	return (true);
	} /*shellclosefilewindowvisit*/


boolean shellcloseall (WindowPtr w, boolean fldialog) {
	
	/*
	close all "root" windows, or ones that have a parent field equal to nil.
	
	it is the responsibility of the root window handler to close all of his
	subordinate windows.
	
	return false if some windows are still open on exit.
	
	10/15/90 dmb: use frontrootwindow instead of firstrootwindow, so windows peel
	off front to back.  also, wait for activates so globals are set cleanly
	
	3/6/91 dmb: now take window parameter.  if it's nil or is a root window, 
	close all as before.  however, if w is non-nil, and it's a child window, 
	close all the children of its root.
	
	4/20/93 dmb: wait for activates _after_ determining that there's a root window left
	
	4.16 dmb: call langpartialeventloop if a script is running.
	
	7.31.97 dmb: if a non-child window is specified, and it's not a root, 
	close all windows non-root windows.

	5.0b10 dmb: don't bring root to the front after closing all children
	
	5.0.2b12 dmb: don't ignore the result of shellvisittypedwindows

	5.1.5b15 dmb: don't wait for activates between closes, leave that to
	shellclose when we need to put up a dialog
	*/
	
	hdlwindowinfo hinfo;
	boolean fl;
	
		uisCloseAllSharedWindows ();
	
	if (w != nil) {
		
		if (!getwindowinfo (w, &hinfo))
			return (false);
		
		if ((**hinfo).parentwindow != nil) { /*it's a child window; close all children*/
			
			getrootwindow (w, &hinfo);
			
			if (!shellclosechildwindows (hinfo))
				return (false);
			
			
			return (true);
			}
		
		if (!shellclose (w, fldialog))
			return (false);
		
		if ((**hinfo).configresnum != iddefaultconfig)
			return (shellvisittypedwindows (-1, &shellclosefilewindowvisit, (ptrvoid) ((long) fldialog)));
		}
	
	if (!shellvisittypedwindows (-1, &shellclosefilewindowvisit, (ptrvoid) ((long) fldialog)))
		return (false);
	
	while (true) { /*close all windows*/
		
		if (!frontrootwindow (&hinfo)) /*no more root windows*/
			return (true);
		
		flshellclosingall = true;
		
		fl = shellclose ((**hinfo).macwindow, fldialog);
		
		flshellclosingall = false;
		
		if (!fl)
			return (false);
		
		//shellsetglobals (nil); /*everything is disposed; must clear for nested push/popglobals*/
		} /*while*/
	} /*shellcloseall*/


boolean shellrevert (WindowPtr wrevert, boolean fldialog) {
	
	/*
	dmb 10/3/90:  1st implementation - pretty rough
	*/
	
	register WindowPtr w;
	
	if (!shellpushrootglobals (wrevert)) /*revert only operates on root windows*/
		return (false);
	
	w = shellwindow; /*move into register*/
	
	if (fldialog) {
		
		bigstring bs;
		
		shellgetwindowtitle (shellwindowinfo, bs); // 7.24.97 dmb: was windowgettitle
		
		filefrompath (bs, bs);
		
		if (!revertdialog (bs)) {
			
			shellpopglobals ();
			
			return (false);
			}
		}
	
	killownedundo (w);
	
	shellbringtofront (shellwindowinfo);
	
	flinhibitclosedialogs = true;
	
	(*shellglobals.closeroutine) ();
	
	shellclosechildwindows (shellwindowinfo); /*close any windows that parent didn't close*/
	
	flinhibitclosedialogs = false;
	
	(*shellglobals.disposerecordroutine) ();
	
	shellclearwindowdata ();
	
	prepuserforwait (w); /*gray the window, set watch cursor while reloading*/
	
	if (!(*shellglobals.loadroutine) (windowgetfnum (w), windowgetrnum (w))) {
		
		closewindowfile (w);
		
		disposeshellwindow (w);
		
		shellpopglobals ();
		
		alertstring (reopenerrorstring);
		
		shellcallfilehooks (); /*a file was just opened or closed*/
		
		return (false);
		}
	
	windowsetchanges (w, false);
	
	shellpopglobals ();
	
	return (true);
	} /*shellrevert*/


boolean shellopendefaultfile (void) {
	
	//
	// no files were selected to be open from the finder.  we either open a new file, or we try to open the default startup file --
	//	similar to autoexec.bat from MS-DOS and System.Startup from UCSD Pascal.
	//
	// 2006-10-18 creedon: for Mac, FSRef-ized
	//
	// 7.30.97 dmb: use defaultpath instead of application filespec, and use new setfsfile to plug in default file name
	//
	// 5/19/92 dmb: always return a value; explicitly return boolean
	//
	// 4/24/92 dmb: use filegetapplicationvnum rather than zero (the default volume) when opening the default file.
	//		       otherwise, if the default volume changes later, the file info won't be adequate to locate the file.
	//
	// 5/20/91 dmb: if the option key is down, just do a normal open file
	//
	
	bigstring bsdefault;
	boolean fl = false, flfolder;
	tyfilespec fs;
	
	if (optionkeydown ())
		return (shellopen ());
	
	shellpushdefaultglobals (); // so config is correct
	
	if (config.flnewonlaunch)
		fl = shellnew ();
	else {
	
		getapplicationfilespec (nil, &fs); // get our own app filespec
		
		if (getstringlist (defaultlistnumber, startupfilename, bsdefault)) {
			
			setfsfile (&fs, bsdefault);
			
			if (fileexists (&fs, &flfolder))
				fl = shellopenfile (&fs, false, nil);
			else {
				
				filegetdefaultpath (&fs);
				
				setfsfile ( &fs, bsdefault );
				
				if (fileexists (&fs, &flfolder))
					fl = shellopenfile (&fs, false, nil);
				else
					fl = shellopen ();
				}
			}
		}
	
	shellpopglobals ();
	
	return (fl);
	} /*shellopendefaultfile*/




