
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
#include "player.h" /*7.0b4 PBS: QuickTime Player window*/
#endif

#ifdef WIN95VERSION 
#include "standard.h"
#endif

#include "about.h"
#include "file.h"
#include "resources.h"
#include "strings.h"
#include "frontierwindows.h"
#include "db.h"
#include "command.h"
#include "shell.h"
#include "shellprivate.h"
#include "lang.h"
#include "langexternal.h"
#include "langinternal.h"
#include "tablestructure.h"
#include "scripts.h"
#include "kernelverbs.h"
#include "kernelverbdefs.h"
#include "shellbuttons.h"
#include "opinternal.h"
#include "opbuttons.h"
#include "tableverbs.h"


#define windowerrorlist 268 
#define notimplementederror 1


typedef enum tywindowtoken { 
	
	isopenfunc,
	
	openfunc,
	
	isfrontfunc,
	
	bringtofrontfunc,
	
	sendtobackfunc,
	
	frontmostfunc,
	
	nextfunc,
	
	isvisiblefunc,
	
	showfunc,
	
	hidefunc,
	
	closefunc,
	
	updatefunc,
	
	ismenuscriptfunc,
	
	getposfunc,
	
	setposfunc,
	
	getsizefunc,
	
	setsizefunc,
	
	zoomfunc,
	
	runselectionfunc,
	
	scrollfunc,
	
	msgfunc,
	
	systemstatsfunc,
	
	quickscriptfunc,
	
	isdirtyfunc,
	
	setdirtyfunc,
	
	gettitlefunc,
	
	settitlefunc,
	
	aboutfunc,
	
	#ifdef MACVERSION
	
		playerfunc, /*7.0b4 PBS: QuickTime Player window*/
	
	#endif
	
	getfilefunc,
	
	isreadonlyfunc, /*7.0b6 PBS: return true if window is read-only*/
	
	ctwindowverbs
	} tywindowtoken;




static boolean getwinaddressparam (hdltreenode hfirst, short pnum, hdlwindowinfo *hinfo) {
	
	hdlhashtable htable;
	bigstring bsname;
	tyvaluerecord val;
	hdlhashnode hnode;
	
	if (!getvarvalue (hfirst, pnum, &htable, bsname, &val, &hnode))
		return (false);
	
	*hinfo = nil;
	
	langexternalwindowopen (val, hinfo);
	
	return (true);
	} /*getwinaddressparam*/


boolean getwinparam (hdltreenode hfirst, short pnum, hdlwindowinfo *hinfo) {
	
	/*
	9/19/91 dmb: we now handle strings as window specifications
	
	5.0d3 dmb: allow windows to be identified by path. also, visit 
	in front-to-back order in case of identical names (like "Untitled")
	*/
	
	bigstring bsname;
	boolean fl;
	//hdlhashtable ht;
	
	disablelangerror ();
	
	fl = getwinaddressparam (hfirst, pnum, hinfo);
	
	enablelangerror ();
	
	if (fl && *hinfo)
		return (true);
	
	if (!getstringvalue (hfirst, pnum, bsname))
		return (false);
	/*
	disablelangerror ();
	
	langexpandtodotparams  (bsname, ht, bsname); // undo effect of deparsestring, bracketname
	
	enablelangerror ();
	*/
	
	shellfindnamedwindow (bsname, hinfo); /*ignore result; hinfo is what counts*/
	
	return (true);
	} /*getwinparam*/


boolean setwinvalue (WindowPtr pwindow, tyvaluerecord *val) {
	
	/*
	set val to the address of the database object contained by pwindow
	
	9/19/91 dmb: we name return a string representation so that window 
	verbs can be applied to all windows, not just db windows.  we still 
	need to try to access a db object if possible, so that a valid 
	language path can be returned rather than the window title (in case 
	brackets are needed).
	
	1/26/93 dmb: handle case where variable can't be found (because its 
	a local). no longer cause script to fail silently
	
	7.23.97 dmb: return full path of file windows

	5.0b16 dmb: for database objects, return an address, not a string
	*/
	
	hdlexternalvariable hvariable = nil;
	hdlhashtable htable;
	bigstring bs;
	hdlwindowinfo hinfo;
	boolean fl = false;
	
	if (pwindow == nil)
		/*
		return (setaddressvalue (nil, zerostring, val));
		*/
		return (setstringvalue (zerostring, val));
	
	shellpushglobals (pwindow);
	
	hinfo = shellwindowinfo;

	fl = (*shellglobals.getvariableroutine) (&hvariable);
	
	shellpopglobals ();
	
	if (fl)
		fl = langexternalfindvariable (hvariable, &htable, bs);
	
	if (!fl) { /*window doesn't represent a global database object*/
		
		shellgetwindowtitle (hinfo, bs); // 7.24.97 dmb: was windowgettitle
		
		if (hvariable != nil) {
			
			if (windowgetpath (pwindow, bs))
				langexternalbracketname (bs);
			}
		}
	else {
		return (setaddressvalue (htable, bs, val));
		/*
		if (htable == filewindowtable)
			langexternalbracketname (bs);
		else
			langexternalgetquotedpath (htable, bs, bs);
		*/
		}
	
	return (setstringvalue (bs, val));
	} /*setwinvalue*/


static boolean isopenverb (hdltreenode hparam1, tyvaluerecord *vreturned) {
	
	/*
	returns true to the script if the window indicated by the address param is open.
	*/
	
	hdlwindowinfo hinfo;
	
	flnextparamislast = true;
	
	if (!getwinparam (hparam1, 1, &hinfo))
		return (false);
	
	setbooleanvalue (hinfo != nil, vreturned);
	
	return (true);
	} /*isopenverb*/


static boolean openverb (hdltreenode hparam1, tyvaluerecord *vreturned, bigstring bserror) {

	/*
	7.0b6 PBS: optional flReadOnly parameter allows outline-type windows
	to open in read-only mode.
	
	8.0.3b2: fixed missing scrollbar on Mac Classic.
	*/
	
	hdlhashtable htable, hscripttable;
	bigstring bsname;
	tyvaluerecord val;
	hdlhashnode hnode;
	hdlexternalvariable hv;
	hdlwindowinfo hinfo;
	short ctparams;
	boolean flreadonly = false; /*editable by default*/
	boolean flreturn = false;
	boolean flbuttontable = false;
	boolean flvisible = true;

	setbooleanvalue (false, vreturned);
	
	ctparams = langgetparamcount (hparam1);
	
	if (ctparams == 1)
	
		flnextparamislast = true;
		
	if (!getvarvalue (hparam1, 1, &htable, bsname, &val, &hnode))
		return (false);
	
	if (val.valuetype != externalvaluetype) /*not a fatal error*/
		return (true);
		
	if (ctparams > 1) { /*7.0b6 PBS: if flReadOnly is supplied*/
	
		if (ctparams == 2)
			flnextparamislast = true;
		
		if (!getbooleanvalue (hparam1, 2, &flreadonly))
		
			return (false);

		if (ctparams > 2) { /*7.1b18 PBS: get table value for buttons*/

			flnextparamislast = true;
			
			if (!gettablevalue (hparam1, 3, &hscripttable))
				return (false);

			flbuttontable = true;
			} /*if*/
		} /*if*/

	hv = (hdlexternalvariable) val.data.externalvalue;
	
	if ((flreadonly) && ((**hv).id == idwordprocessor)) { /*7.0b6 PBS: read-only wp-text is an error.*/
	
		hdlwindowinfo hinfo;
				
		langerrormessage ("\x5a""Can't open window as read-only because windows of type wp text don't support this feature.");	
		
		if (shellfinddatawindow ((Handle) (**hv).variabledata, &hinfo)) { /*window may be hidden -- if so, close*/
			
			if ((**hinfo).flhidden) { /*if hidden, close the window*/
				
				shellclose ((**hinfo).macwindow, false);
				} /*if*/
			} /*if*/
		
		return (false);
		} /*if*/	
	
	if ((flbuttontable) && ((**hv).id != idoutlineprocessor)) { /*7.1b18 PBS: buttons for non-outlines is an error.*/

		hdlwindowinfo hinfo;

		langerrormessage ("\x52""Can't attach buttons to window because windows only outlines support this feature.");

		if (shellfinddatawindow ((Handle) (**hv).variabledata, &hinfo)) { /*window may be hidden -- if so, close*/
			
			if ((**hinfo).flhidden) { /*if hidden, close the window*/
				
				shellclose ((**hinfo).macwindow, false);
				} /*if*/
			} /*if*/
		
		return (false);
		} /*if*/	
	
	if (flbuttontable) {

		flvisible = false;
		}/*if*/

	if (!langzoomvalwindow (htable, bsname, val, false))
		return (false);
		
	if (shellfinddatawindow ((Handle) (**hv).variabledata, &hinfo)) {
	
		switch ((**hv).id) {
		
			case idoutlineprocessor:
				
				if (flbuttontable) {
				
					opbuttonsattach (hinfo, hscripttable);
					} /*if*/

				else {
					
					(**hinfo).flhidebuttons = true;

					(**hinfo).flsyntheticbuttons = false;
					} /*else*/				
							
				break;
			
			case idscriptprocessor:
				
				(**hinfo).flhidebuttons = false;
				
				(**hinfo).flsyntheticbuttons = false;
				
				break;
			
			default:
			
				(**hinfo).flhidebuttons = true;
				
				(**hinfo).flsyntheticbuttons = false;
			} /*switch*/

		#if WIN95VERSION || TARGET_API_MAC_CARBON /*8.0.3b2: Windows and OS X only. Fixes missing scrollbar on Classic.*/
		
			shellpushglobals ((**hinfo).macwindow);		
		
			shelladjustaftergrow ((**hinfo).macwindow);

			shellpopglobals ();
		#endif
		} /*if*/		
	
	if (!langzoomvalwindow (htable, bsname, val, true)) /*now make it visible*/
		return (false);
	
	flreturn = langexternalsetreadonly (hv, flreadonly); /*7.0b6 PBS: set the read-only flag.*/
	
	(*vreturned).data.flvalue = flreturn;
	
	return (flreturn);
	} /*openverb*/


static boolean isreadonlyverb (hdltreenode hparam1, tyvaluerecord *vreturned) {
	
	/*
	7.0b6 PBS: return true if a window is read-only, false otherwise.
	*/
	
	hdlhashtable htable;
	tyvaluerecord val;
	hdlhashnode hnode;
	hdlexternalvariable hv;
	bigstring bsname;
	boolean flreadonly = false;

	flnextparamislast = true;
	
	if (!getvarvalue (hparam1, 1, &htable, bsname, &val, &hnode))
		return (false);
	
	if (val.valuetype == externalvaluetype) { //7.1b33 dmb

		hv = (hdlexternalvariable) val.data.externalvalue;
	
		flreadonly = langexternalgetreadonly (hv);
		}
	
	setbooleanvalue (flreadonly, vreturned);
	
	return (true);	
	} /*isreadonlyverb*/
	

static boolean isfrontverb (hdltreenode hparam1, tyvaluerecord *vreturned) {
	
	/*
	returns true to the script if the window indicated by the address param is 
	the frontmost window.
	*/
	
	register boolean fl = false; /*not frontmost*/
	hdlwindowinfo hinfo;
	WindowPtr ptarget;
	
	flnextparamislast = true;
	
	if (!getwinparam (hparam1, 1, &hinfo))
		return (false);
	
	if (hinfo != nil) {
		
		if (shellfindtargetwindow (-1, &ptarget))
			fl = ptarget == (**hinfo).macwindow;
		
		/*
		getfrontwindowinfo (&hfrontinfo);
		
		fl = hinfo == hfrontinfo;
		*/
		}
		
	setbooleanvalue (fl, vreturned);
	
	return (true);
	} /*isfrontverb*/


static boolean bringtofrontverb (hdltreenode hparam1, tyvaluerecord *vreturned) {
	
	/*
	the verb takes an address -- the name of the object database variable to be 
	brought to the front.  if it's not in a window, we open a window at the front of
	the pile of windows.  
	
	return false if there was an error.
	
	this is needed for scripts that use a single keystroke to open a window, and use
	the same keystroke to process some information in the window.  e.g. cmd-J to jump
	through the table map, or cmd-= to evaluate an expression in the favorites window.
	
	5/10/91 dmb: with the frontmost verb working, this verb should no longer do 
	anything if the window isn't already open.
	*/
	
	register boolean fl = false;
	hdlwindowinfo hinfo;
	
	flnextparamislast = true;
	
	if (!getwinparam (hparam1, 1, &hinfo))
		return (false);
	
	if (hinfo != nil) { /*a window is open*/
		
		shellbringtofront (hinfo);
		
		fl = true;
		}
	
	return (setbooleanvalue (fl, vreturned));
	} /*bringtofrontverb*/


static boolean sendtobackverb (hdltreenode hparam1, tyvaluerecord *vreturned) {
	
	/*
	2.1b12 dmb: use new windowsendtoback
	*/
	
	register boolean fl = false;
	hdlwindowinfo hinfo;
	
	flnextparamislast = true;
	
	if (!getwinparam (hparam1, 1, &hinfo))
		return (false);
	
	if (hinfo != nil) { /*a window is open*/
		
		windowsendtoback ((**hinfo).macwindow);
		
		fl = true;
		}
	
	setbooleanvalue (fl, vreturned);
	
	return (true); 
	} /*sendtobackverb*/



static boolean frontmostverb (hdltreenode hparam1, tyvaluerecord *vreturned) {
	
	/*
	3/19/91 DW:
	
	return the address of the variable that's displayed in the frontmost window.
	
	doug, you're not going to like how this is implemented -- but I needed the
	functionality to fix a bug in the cmd-= ("favorites") script.
	
	7/10/91 dmb: return address value instead of a string
	*/
	
	WindowPtr ptarget;
	
	if (!langcheckparamcount (hparam1, 0)) /*shouldn't have any parameters*/
		return (false);
	
	if (shellfindtargetwindow (-1, &ptarget))
		return (setwinvalue (ptarget, vreturned));
	
	return (setwinvalue (nil, vreturned));
	} /*frontmostverb*/


static boolean nextverb (hdltreenode hparam1, tyvaluerecord *vreturned) {
	
	/*
	return the address of the next shell window after the one indicated 
	by the parameter given
	*/
	
	register boolean fl = false;
	hdlwindowinfo hinfo;
	WindowPtr pnext;
	
	flnextparamislast = true;
	
	if (!getwinparam (hparam1, 1, &hinfo))
		return (false);
	
	if (hinfo != nil) { /*a window is open*/
		
		pnext = shellnextwindow ((**hinfo).macwindow);
		
		if (pnext != nil)
			return (setwinvalue (pnext, vreturned));
		}
	
	return (setwinvalue (nil, vreturned));
	} /*nextverb*/


static boolean isvisibleverb (hdltreenode hparam1, tyvaluerecord *vreturned) {
	
	register boolean fl = false;
	hdlwindowinfo hinfo;
	
	flnextparamislast = true;
	
	if (!getwinparam (hparam1, 1, &hinfo))
		return (false);
	
	if (hinfo != nil) { /*a window is open*/
		
		fl = windowvisible ((**hinfo).macwindow);
		}
	
	setbooleanvalue (fl, vreturned);
	
	return (true); 
	} /*isvisibleverb*/


static boolean showhideverb (hdltreenode hparam1, boolean flshow, tyvaluerecord *vreturned) {
	
	register boolean fl = false;
	hdlwindowinfo hinfo;
	
	flnextparamislast = true;
	
	if (!getwinparam (hparam1, 1, &hinfo))
		return (false);
	
	if (hinfo != nil) { /*a window is open*/
		
		if (flshow)
			shellunhidewindow (hinfo);
		else
			shellhidewindow (hinfo);
		
		fl = true;
		}
	
	setbooleanvalue (fl, vreturned);
	
	return (true); 
	} /*showhideverb*/


static boolean closeverb (hdltreenode hparam1, tyvaluerecord *vreturned) {
	
	/*
	8/26/92 dmb: don't allow the root window to be closed with this verb.
	
	(note: used to crash; need to call shellclose to handle root windows)
	*/
	
	register boolean fl = false;
	hdlwindowinfo hinfo;
	
	flnextparamislast = true;
	
	if (!getwinparam (hparam1, 1, &hinfo))
		return (false);
	
	if (hinfo != nil) { /*close it if it isn't a root window*/
		
	//	if ((**hinfo).parentwindow != nil /*|| shellgetexternaldata (hinfo, &hdata)*/)
			fl = shellclose ((**hinfo).macwindow, false);
		}
	
	setbooleanvalue (fl, vreturned);
	
	return (true); 
	} /*closeverb*/
	

static boolean updateverb (hdltreenode hparam1, tyvaluerecord *vreturned) {
	
	register boolean fl = false;
	hdlwindowinfo hinfo;
	
	flnextparamislast = true;
	
	if (!getwinparam (hparam1, 1, &hinfo))
		return (false);
	
	if (hinfo != nil) {
		
		shellwindowinval (hinfo);
		
		shellupdatenow ((**hinfo).macwindow);
		
		fl = true;
		}
	
	setbooleanvalue (fl, vreturned);
	
	return (true); 
	} /*updateverb*/
	

static boolean ismenuscriptverb (hdltreenode hparam1, tyvaluerecord *vreturned) {
	
	/*
	return true if the frontmost window contains a menubar script
	
	the implementation is really ugly, but our hope is that we'll come 
	up with a way to address menubar scripts through the language and 
	get rid of this verb.
	
	9/25/91: well, the language still can't address menubar scripts, but 
	the window verbs now can.  so we can at least take a window parameter 
	like the rest or the verbs, instead of looking at the target window.
	*/
	
	register boolean fl = false;
	hdlwindowinfo hinfo;
	
	flnextparamislast = true;
	
	if (!getwinparam (hparam1, 1, &hinfo))
		return (false);
	
	if (hinfo != nil) {
		
		shellpushglobals ((**hinfo).macwindow);
		
		if (shellglobals.configresnum == idscriptconfig)
			fl = scriptinmenubar ();
		
		shellpopglobals ();
		}
	
	/*
	WindowPtr target;
	boolean fl;
	
	if (!langcheckparamcount (hparam1, 0)) /*shouldn't have any parameters%/
		return (false);
	
	fl = false; /*default return is false%/
	
	if (shellfindtargetwindow (-1, &target)) {
		
		shellpushglobals (target);
		
		if (shellglobals.configresnum == idscriptconfig)
			fl = scriptinmenubar ();
		
		shellpopglobals ();
		}
	*/
	
	return (setbooleanvalue (fl, vreturned));
	} /*ismenuscriptverb*/


static boolean getboundsverb (hdltreenode hparam1, boolean flsize, tyvaluerecord *vreturned) {
	
	hdlwindowinfo hinfo;
	Rect r;
	hdlhashtable ht1, ht2;
	bigstring bs1, bs2;
	tyvaluerecord val;
	register boolean fl = false;
	
	if (!getwinparam (hparam1, 1, &hinfo))
		return (false);
	
	if (!getvarparam (hparam1, 2, &ht1, bs1))
		return (false);
	
	flnextparamislast = true;
	
	if (!getvarparam (hparam1, 3, &ht2, bs2))
		return (false);
	
	if (hinfo != nil) { /*a window is open*/
		register int x;
		
		shellgetglobalwindowrect (hinfo, &r);
		
		if (flsize)
			x = r.right - r.left;
		else
			x = r.left;
		
		setintvalue (x, &val);
		
		hashtableassign (ht1, bs1, val);
		
		if (flsize)
			val.data.intvalue = r.bottom - r.top;
		else
			val.data.intvalue = r.top;
		
		hashtableassign (ht2, bs2, val);
		
		fl = true;
		}
	
	setbooleanvalue (fl, vreturned);
	
	return (true); 
	} /*getboundsverb*/


static boolean setboundsverb (hdltreenode hparam1, boolean flsize, tyvaluerecord *vreturned) {
	
	/*
	10/6/91 dmb: shellmovewindow and shellsizewindow now return a result
	*/
	
	hdlwindowinfo hinfo;
	short int1, int2;
	register boolean fl = false;
	
	if (!getwinparam (hparam1, 1, &hinfo))
		return (false);
	
	if (!getintvalue (hparam1, 2, &int1))
		return (false);
	
	flnextparamislast = true;
	
	if (!getintvalue (hparam1, 3, &int2))
		return (false);
	
	if (hinfo != nil) { /*a window is open*/
		
		if (flsize)
			fl = shellsizewindow (hinfo, int1, int2);
		else
			fl = shellmovewindow (hinfo, int1, int2);
		}
	
	return (setbooleanvalue (fl, vreturned)); 
	} /*setboundsverb*/


static boolean zoomverb (hdltreenode hparam1, tyvaluerecord *vreturned) {
	
	/*
	10/6/91 dmb: shellzoomwindow now returns a result
	*/
	
	hdlwindowinfo hinfo;
	register boolean fl = false;
	
	flnextparamislast = true;
	
	if (!getwinparam (hparam1, 1, &hinfo))
		return (false);
	
	if (hinfo != nil) { /*a window is open*/
		
		shellpushglobals ((**hinfo).macwindow);
		
		fl = (*shellglobals.zoomwindowroutine) (hinfo, true);
		
		shellpopglobals ();
		}
	
	return (setbooleanvalue (fl, vreturned)); 
	} /*zoomverb*/


static boolean getdirtywinparam (hdltreenode hfirst, short pnum, hdlwindowinfo *hinfo, hdlexternalvariable *hvariable) {
	
	/*
	unlike the normal getwinparam function, this one will accept the address 
	of an object whose window isn't open.  but it will also accept non-db windows.
	
	2.1b3 dmb: don't return false unless an error is generated
	*/
	
	hdlhashtable htable;
	bigstring bs;
	tyvaluerecord val;
	boolean fl;
	hdlhashnode hnode;
	
	disablelangerror ();
	
	fl = getvarvalue (hfirst, pnum, &htable, bs, &val, &hnode);
	
	enablelangerror ();
	
	if (fl && (val.valuetype == externalvaluetype)) {
		
		*hvariable = (hdlexternalvariable) val.data.externalvalue;
		
		return (true);
		}
	
	*hvariable = nil;
	
	return (getwinparam (hfirst, pnum, hinfo));
	} /*getdirtywinparam*/


static boolean isdirtyverb (hdltreenode hparam1, tyvaluerecord *vreturned) {
	
	/*
	4/16/92 dmb: find out if the object or window indicated in hparam1 is dirty.
	if it's  not an external value or a window, always return false
	
	2.1b3 dmb: don't count on hinfo being non-nil when hvariable is nil
	*/
	
	hdlwindowinfo hinfo;
	hdlexternalvariable hvariable;
	boolean fldirty = false;
	
	flnextparamislast = true;
	
	if (!getdirtywinparam (hparam1, 1, &hinfo, &hvariable))
		return (false);
	
	if (hvariable != nil)
		fldirty = langexternalisdirty (hvariable);
	else
		if (hinfo != nil)
			fldirty = (**hinfo).flmadechanges;
	
	return (setbooleanvalue (fldirty, vreturned));
	} /*isdirtyverb*/


static boolean setdirtyverb (hdltreenode hparam1, tyvaluerecord *vreturned) {
	
	/*
	4/16/92 dmb: set the dirtyness of the object or window indicated in hparam1.
	if it's  not an external value or a window, always return false
	
	2.1b3 dmb: don't count on hinfo being non-nil when hvariable is nil
	*/
	
	hdlwindowinfo hinfo;
	hdlexternalvariable hvariable;
	boolean fldirty;
	boolean fl = false;
	
	if (!getdirtywinparam (hparam1, 1, &hinfo, &hvariable))
		return (false);
	
	flnextparamislast = true;
	
	if (!getbooleanvalue (hparam1, 2, &fldirty))
		return (false);
	
	if (hvariable != nil)
		fl = langexternalsetdirty (hvariable, fldirty);
	else
		if (hinfo != nil)
			fl = shellsetwindowchanges (hinfo, fldirty);
	
	return (setbooleanvalue (fl, vreturned));
	} /*setdirtyverb*/


static boolean gettitleverb (hdltreenode hparam1, tyvaluerecord *vreturned) {
	
	/*
	6/24/92 dmb: get the title of the object or window indicated in hparam1.
	if it's  not an external value or a window, always return the empty string
	*/
	
	hdlwindowinfo hinfo;
	bigstring bstitle;
	boolean fl = false;
	
	flnextparamislast = true;
	
	if (!getwinparam (hparam1, 1, &hinfo))
		return (false);
	
	setemptystring (bstitle);
	
	if (hinfo != nil) {
		
		shellgetwindowtitle (hinfo, bstitle); // 7.24.97 dmb: was windowgettitle
		
		fl = true;
		}
	
	return (setstringvalue (bstitle, vreturned));
	} /*gettitleverb*/


static boolean settitleverb (hdltreenode hparam1, tyvaluerecord *vreturned) {
	
	/*
	6/24/92 dmb: set the title of the object or window indicated in hparam1.
	if it's  not an external value or a window, always return false
	*/
	
	hdlwindowinfo hinfo;
	bigstring bstitle;
	boolean fl = false;
	
	if (!getwinparam (hparam1, 1, &hinfo))
		return (false);
	
	flnextparamislast = true;
	
	if (!getstringvalue (hparam1, 2, bstitle))
		return (false);
	
	if (hinfo != nil) {
		
		shellsetwindowtitle (hinfo, bstitle); // 7.24.97 dmb: was windowsettitle
		
		shellwindowmenudirty ();
		
		fl = true;
		}
	
	return (setbooleanvalue (fl, vreturned));
	} /*settitleverb*/


static boolean getfileverb (hdltreenode hparam1, tyvaluerecord *vreturned) {
	
	/*
	6/24/92 dmb: get the title of the object or window indicated in hparam1.
	if it's  not an external value or a window, always return the empty string
	*/
	
	hdlwindowinfo hinfo;
	tyfilespec fs;
	boolean fl = false;
	
	flnextparamislast = true;
	
	if (!getwinparam (hparam1, 1, &hinfo))
		return (false);
	
	if ((hinfo != nil) && windowgetfspec ((**hinfo).macwindow, &fs))
		return (setfilespecvalue (&fs, vreturned));
	
	return (setstringvalue (zerostring, vreturned));
	} /*getfileverb*/


static boolean windowfunctionvalue (short token, hdltreenode hparam1, tyvaluerecord *vreturned, bigstring bserror) {
	
	/*
	10/3/91 dmb: moved a couple of verbs in from shellverbs.c -- they're the 
	odd ones out following zoomfunc.
	
	6/24/92 dmb: added gettitle, settitle verbs
	
	6/1/93 dmb: when vreturned is nil, return whether or not verb token must 
	be run in the Frontier process

	6.2a8 AR: systemstatsfunc now works for GDBs (at least partly)
	*/
	
	register hdltreenode hp = hparam1;
	register tyvaluerecord *v = vreturned;
	register short errornum = 0;
	
	if (v == nil) /*need Frontier process?*/
		return (true);
	
	setbooleanvalue (false, v); /*by default, window functions return true*/
	
	switch (token) {
		
		case isopenfunc:
			return (isopenverb (hp, v));
		
		case openfunc:
			return (openverb (hp, v, bserror));
		
		case isfrontfunc:
			return (isfrontverb (hp, v));
		
		case bringtofrontfunc:
			return (bringtofrontverb (hp, v));
		
		case sendtobackfunc:
			return (sendtobackverb (hp, v));
		
		case frontmostfunc:
			return (frontmostverb (hp, v));
		
		case nextfunc:
			return (nextverb (hp, v));
		
		case isvisiblefunc:
			return (isvisibleverb (hp, v));
	
		case showfunc:
			return (showhideverb (hp, true, v));
	
		case hidefunc:
			return (showhideverb (hp, false, v));
	
		case closefunc:
			return (closeverb (hp, v));
		
		case updatefunc:
			return (updateverb (hp, v));
		
		case ismenuscriptfunc:
			return (ismenuscriptverb (hp, v));
		
		case getposfunc:
			return (getboundsverb (hp, false, v));
		
		case setposfunc:
			return (setboundsverb (hp, false, v));
		
		case getsizefunc:
			return (getboundsverb (hp, true, v));
		
		case setsizefunc:
			return (setboundsverb (hp, true, v));
		
		case zoomfunc:
			return (zoomverb (hp, v));
		
		case runselectionfunc:
			if (!langcheckparamcount (hparam1, 0))
				return (false);
			
			if (!shellpushtargetglobals ())
				return (false);
			
			(*v).data.flvalue = (*shellglobals.executeroutine) ();
			
			shellpopglobals ();
			
			return (true);
		
		case scrollfunc: {
			tydirection dir;
			long count;
			
			if (!getdirectionvalue (hparam1, 1, &dir))
				return (false);
			
			flnextparamislast = true;
			
			if (!getlongvalue (hparam1, 2, &count))
				return (false);
			
			if (!shellpushtargetglobals ())
				return (false);
			
			(*v).data.flvalue = (*shellglobals.scrollroutine) (dir, false, count);
			
			shellupdatescrollbars (shellwindowinfo);
			
			shellpopglobals ();
			
			return (true);
			}
		
		case msgfunc: {
			bigstring bs;
			
			flnextparamislast = true;
			
			if (!getstringvalue (hparam1, 1, bs))
				return (false);
			
			(*v).data.flvalue = shellfrontwindowmessage (bs);
			
			return (true);
			}
		
		case systemstatsfunc:
			if (langgetparamcount (hparam1) == 0)
				dbstatsmessage (nil, true);
			else {
				hdlwindowinfo hinfo;
				hdldatabaserecord hdb;

				flnextparamislast = true;
				
				if (!getwinparam (hparam1, 1, &hinfo))
					break;
				
				if (hinfo == nil || !shellgetdatabase ((**hinfo).macwindow, &hdb)) {
					
					getstringlist (langerrorlist, badwindowerror, bserror);
					
					break;
					}

				dbstatsmessage (hdb, false);
				}

			return (setbooleanvalue (true, v));
		
		case quickscriptfunc:
			if (!langcheckparamcount (hparam1, 0))
				return (false);
			
			(*v).data.flvalue = startcmddialog ();

			return (true);
		
		case isdirtyfunc:
			return (isdirtyverb (hparam1, v));
		
		case setdirtyfunc:
			return (setdirtyverb (hparam1, v));
		
		case gettitlefunc:
			return (gettitleverb (hparam1, v));
		
		case settitlefunc:
			return (settitleverb (hparam1, v));
		
		case aboutfunc:
			if (!langcheckparamcount (hparam1, 0))
				return (false);
			
			(*v).data.flvalue = aboutcommand ();
			
			return (true);
		
		#ifdef MACVERSION
			
			case playerfunc: /*7.0b4 PBS: QuickTime Player window*/
				if (!langcheckparamcount (hparam1, 0))
					return (false);
				
				(*v).data.flvalue = playeropencommand ();
				
				return (true);
			
		#endif
		
		case getfilefunc:
			return (getfileverb (hparam1, v));
		
		case isreadonlyfunc: /*7.0b6 PBS: return true if a window is read-only*/
			return (isreadonlyverb (hparam1, v));
		
		default:
			errornum = notimplementederror;
			
			goto error;
		} /*switch*/
	
	error:
	
	if (errornum != 0) /*get error string*/
		getstringlist (windowerrorlist, errornum, bserror);
	
	return (false);
	} /*windowfunctionvalue*/


boolean windowinitverbs (void) {
	
	return (loadfunctionprocessor (idwindowverbs, &windowfunctionvalue));
	} /*windowinitverbs*/




