
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

#include "search.h"
#include "cursor.h"
#include "kb.h"
#include "langexternal.h"
#include "opinternal.h"
#include "menueditor.h"
#include "menuinternal.h"




static boolean flfoundinscript;


boolean mesearchrefconroutine (hdlheadrecord hnode) {
	
	/*
	load the script linked into the headrecord, search it for the search string, and
	return with hnodesearch (a global) pointing at the menu item where the match was
	found.
	
	9/19/90 DW: we weren't checking for a nil outlinerecord handle.  it happens when
	the menu item has no script linked to it.
	*/
	
	hdloutlinerecord houtline;
	hdloutlinerecord hmenuoutline;
	register hdloutlinerecord ho;
	boolean flfound;
	boolean fljustloaded;
	hdlwindowinfo hinfo;
	boolean fldirty;
	boolean fldisplaywasenabled;
	
	if (searchparams.flonelevel)
		return (false);
	
	if (keyboardescape ())
		return (false);
	
	if (!meloadscriptoutline (menudata, hnode, &houtline, &fljustloaded)) /*error loading script*/
		return (false);
	
	ho = houtline; /*copy into register*/
	
	if (ho == nil) /*no script linked into the menu item*/
		return (false);
	
	oppushoutline (ho);
	
	fldisplaywasenabled = opdisabledisplay (); /*we pushed outline, but not its window*/
	
	flfound = opflatfind (true, false);
	
	fldirty = (**ho).fldirty; /*in case of replace all*/
	
	if (fldisplaywasenabled)
		openabledisplay ();
	
	oppopoutline (); /*restore original*/
	
	if (flfound) { /*match found, position bar cursor and return true*/
		
		mesetscriptoutline (hnode, ho); /*make loaded state stick*/
		
		hmenuoutline = outlinedata;
		
		oppushoutline (nil); /*save on stack*/
		
		if (!mezoommenubarwindow (hmenuoutline, false, &hinfo)) { /*probably out of memory*/
			
			oppopoutline ();
			
			return (false);
			}
		
		shellpushglobals ((**hinfo).macwindow);
		
		if (searchparams.flwindowzoomed)
			(**hinfo).flopenedforfind = true;
		
		meexpandto (hnode);
		
		mezoomscriptwindow ();
		
		shellpopglobals ();
		
		oppopoutline ();
		
		flfoundinscript = true; /*so caller will know display is already done*/
		
		return (true);
		}
	
	if (fljustloaded) {
		
		if (fldirty) { /*must be doing a replace all*/
			
			mesetscriptoutline (hnode, ho); /*make loaded state stick*/
			
			(**outlinedata).fldirty = true; /*percolate up to parent outline*/
			}
		else
			opdisposeoutline (ho, false);
		}
	
	return (false);
	} /*mesearchrefconroutine*/


boolean mesearchoutline (boolean flfromtop, boolean flwrap, boolean *flzoom) {
	
	flfoundinscript = false;
	
	if (!opflatfind (flfromtop, flwrap))
		return (false);
	
	*flzoom = !flfoundinscript;
	
	/*
	if (flfoundinscript && searchparams.flwindowzoomed) {
		
		hdlwindowinfo hinfo;
		
		if (getfrontwindowinfo (&hinfo))
			(**hinfo).flopenedforfind = true;
		}
	*/
	
	return (true);
	} /*mesearchoutline*/


boolean mecontinuesearch (hdlwindowinfo hinfo, hdlheadrecord hnode) {
	
	/*
	9/12/91 dmb: new search logic checks for flat search before calling here; 
	we don't have enought info to check
	
	5.0.2b20 dmb: start search from headline _after_ hnode; don't get stuck here.
	*/
	
	boolean fl, flzoom;
	long menurefcon;
	
	shellpushglobals ((**hinfo).macwindow);
	
	mecheckglobals ();
	
	menurefcon = (**menudata).menurefcon;
	
	if (!opnavigate (flatdown, &hnode))
		fl = false;
	
	else {
		opmoveto (hnode);
		
		opsettextmode (false); /*make sure we search text of new headline*/
		
		fl = mesearchoutline (false, searchshouldwrap (menurefcon), &flzoom);
		}
	
	if (fl) {
		
		if (searchparams.flzoomfound && flzoom)
			shellbringtofront (shellwindowinfo);
		}
	else {
		
		if (searchshouldcontinue (menurefcon))
			fl = langexternalcontinuesearch ((hdlexternalvariable) menurefcon);
		}
	
	shellpopglobals ();
	
	return (fl);
	} /*mecontinuesearch*/


boolean menuverbsearch (void) {
	
	boolean flzoom;
	long menurefcon;
	
	mecheckglobals ();
	
	menurefcon = (**menudata).menurefcon;
	
	startingtosearch (menurefcon);
	
	if (mesearchoutline (false, searchshouldwrap (menurefcon), &flzoom))
		return (true);
	
	if (keyboardescape ()) {
		
		/*
		keyboardclearescape ();
		*/
		
		return (false);
		}
	
	if (!searchshouldcontinue (menurefcon))
		return (false);
	
	return (langexternalcontinuesearch ((hdlexternalvariable) menurefcon));
	} /*menuverbsearch*/




