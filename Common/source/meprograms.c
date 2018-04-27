
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

#include "kb.h"
#include "strings.h"
#include "frontierwindows.h"
#include "lang.h"
#include "langexternal.h"
#include "shellmenu.h"
#include "process.h"
#include "scripts.h"
#include "menueditor.h"
#include "menuinternal.h"
#include "meprograms.h"
#include "menuverbs.h"
#include "cancoon.h"




typedef struct findnodeinfo{
	
	hdlheadrecord hnode;
	
	hdlwindowinfo hinfo;
	} tyfindnodeinfo;


static boolean mefindnodevisit (WindowPtr w, ptrvoid refcon) {
	
	tyfindnodeinfo *findinfo = (tyfindnodeinfo *) refcon;
	boolean fl;
	
	shellpushglobals (w);
	
	fl = opnodeinoutline ((*findinfo).hnode);
	
	(*findinfo).hinfo = shellwindowinfo; /*do now while globals are set*/
	
	shellpopglobals ();
	
	if (fl) /*found the outline we're looking for; stop visiting*/
		return (false);
	
	return (true); /*keep visiting*/
	} /*mefindnodevisit*/


boolean mezoommenubarwindow (hdloutlinerecord houtline, boolean flbringtofront, hdlwindowinfo *hinfo) {
	
	/*
	trace the indicated outline's refcon back to the menubar variable 
	that contains it, and zoom that variable into its window.
	
	7/15/91 dmb: get the fullpath of the menubar for the title
	
	9/12/91 dmb: added flbringtofront, hinfo parameters to avoid window noise 
	during searches.
	*/
	
	hdlmenurecord hm = (hdlmenurecord) (**houtline).outlinerefcon;
	hdlexternalvariable hv = (hdlexternalvariable) (**hm).menurefcon;
	hdlhashtable htable;
	bigstring bs, bstitle;
	Rect rzoom;
	hdlwindowinfo hparent;
	
	if (!langexternalfindvariable (hv, &htable, bs))
		return (false);
	
	if (!langexternalgetfullpath (htable, bs, bstitle, &hparent))
		return (false);
	
	if (flbringtofront)
		rzoom.top = -1;
	else
		rzoom.top = -2;
	
	if (!menuedit (hv, hparent, nil, bstitle, &rzoom))
		return (false);
	
	if (hinfo != nil)
		return (shellfinddatawindow ((Handle) (**hv).variabledata, hinfo));
	
	return (true);
	} /*mezoommenubarwindow*/



boolean mescripterrorroutine (long scripterrorrefcon, long lnum, short charnum, hdlhashtable *htable, bigstring bsname) {
	
	/*
	refcon points to a node with a linked script that generated a language
	error.  lnum, charnum indicates where in the script the error occurred.
	
	4.1b3 dmb: support getting htable, bsname instead of zooming
	*/
	
	
	register hdlheadrecord h = (hdlheadrecord) scripterrorrefcon;
	register boolean fl = false;
	hdloutlinerecord houtline;
	hdlwindowinfo hinfo;
	tyfindnodeinfo findinfo;
	
	if (h == nil) /*defensive driving*/
		return (false);
	
	if (htable != nil) { /*4.1b3 dmb: caller wants table, name*/
		
		*htable = nil;
		
		getheadstring (h, bsname);
		
		return (true);
		}
	
	findinfo.hnode = h;
	
	if (!shellvisittypedwindows (idmenueditorconfig, &mefindnodevisit, &findinfo)) { /*found in open window*/
		
		hinfo = findinfo.hinfo;
		}
	else {
		
		if (!melocatemenubarnode (h, &houtline)) /*couldn't find the node in menu bar*/
			return (false);
		
		if (!mezoommenubarwindow (houtline, false, &hinfo)) /*probably out of memory*/
			return (false);
		}
	
	shellpushglobals ((**hinfo).macwindow);
	
	mesetglobals ();
	
	meexpandto (h); 
	
	if (mezoomscriptwindow ()) {
		
		shellpushglobals ((**menudata).scriptwindow);
		
		fl = opshowerror (lnum, charnum);
		
		shellpopglobals (); /*scriptwindow*/
		}
	
	shellpopglobals (); /*menuwindow*/
	
	return (fl);
	
	} /*mescripterrorroutine*/


boolean megetnodelangtext (hdlheadrecord hnode, Handle *htext, long *signature) {
	
	/*
	12/14/90 dmb: use meloadscriptoutline instead of taking care of loading 
	manually
	*/
	
	register hdlheadrecord h = hnode;
	register hdloutlinerecord ho;
	hdloutlinerecord houtline;
	register boolean fl;
	boolean fldispose = false;
	
	
	mesetglobals (); /*be sure op, db and lang have globals set up properly*/
	
	
	if (!meloadscriptoutline (menudata, h, &houtline, &fldispose)) 
		return (false);
	
	ho = houtline; /*copy into register*/
	
	if (ho == nil) /*nothing to run*/
		return (false);
	
	*signature = (**ho).outlinesignature;
	
	fl = opgetlangtext (ho, false, htext);
	
	if (fldispose) 
		opdisposeoutline (ho, false);
	
	return (fl);
	} /*megetnodelangtext*/



boolean meshownode (hdlheadrecord hnode) {
	
	/*
	1/7/90 dmb: the option key case, must push/popglobals around mexpandto
	
	can assume that outlinedata is current for hnode
	*/
	
	boolean flshowscript = keyboardstatus.flshiftkey || shiftkeydown ();
	hdlwindowinfo hinfo;
	
	if (!mezoommenubarwindow (outlinedata, !flshowscript, &hinfo)) /*make sure menubar window is open, in front*/
		return (false);
	
	shellpushglobals ((**hinfo).macwindow);
	
	meexpandto (hnode);
	
	if (flshowscript)
		mezoomscriptwindow ();
	
	shellpopglobals ();
	
	return (true);
	} /*meshownode*/


static boolean meprocesscallback (void) {
	
	shellforcemenuadjust (); /*for menu dimming*/
	
	return (true);
	} /*meprocesscallback*/



boolean meuserselected (hdlheadrecord hnode) {
	
	/*
	1/7/90 dmb: in option key case, must push/popglobals around mexpandto
	
	3/29/91 dmb: moved option key case into it's own routine; caller checks.
	
	2.1b2 dmb: support osa scripts by using scriptbuildtree instead of langbuildtree
	
	3.0a dmb: we're now allowed to add the new process here for runtime too.
	
	4.1b3 dmb: don't set newlyaddedprocess here anymore; addprocess does it.
	*/
	
	Handle htext;
	long signature;
	hdltreenode hcode;
	hdlprocessrecord hprocess;
	register hdlprocessrecord hp;
	
	boolean fl;
	
		
		menudata = (hdlmenurecord) (**outlinedata).outlinerefcon;
	
	
	if (!megetnodelangtext (hnode, &htext, &signature)) 
		return (false);
	
	if (!langpusherrorcallback (&mescripterrorroutine, (long) hnode))
		return (false);
	
	fl = scriptbuildtree (htext, signature, &hcode);
	
	langpoperrorcallback ();
	
	if (!fl) /*syntax error*/
		return (false);
	
	langerrorclear (); /*compilation produced no error, be sure error window is empty*/
	
	if (!newprocess (hcode, true, &mescripterrorroutine, (long) hnode, &hprocess)) {
		
		langdisposetree (hcode);
		
		return (false);
		}
	
	hp = hprocess; /*copy into register*/
	
		
		(**hp).processkilledroutine = &meprocesscallback;
		
		shellforcemenuadjust (); /*for menu dimming*/
		
	
	addprocess (hp);
	
	return (true);
	} /*meuserselected*/




