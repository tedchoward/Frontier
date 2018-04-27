
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

#include "langxcmd.h"


#include "memory.h"
#include "frontierconfig.h"
#include "cursor.h"
#include "dialogs.h"
#include "error.h"
#include "font.h"
#include "kb.h"
#include "mouse.h"
#include "ops.h"
#include "quickdraw.h"
#include "resources.h"
#include "sounds.h"
#include "strings.h"
#include "frontierwindows.h"
#include "shell.h"
#include "shellhooks.h"
#include "oplist.h"
#include "lang.h"
#include "langinternal.h"
#include "langexternal.h"
#include "langipc.h"
#include "langwinipc.h"
#include "langdll.h"
#include "langsystem7.h"
#include "langtokens.h"
#include "BASE64.H"
#include "tablestructure.h"
#include "tableverbs.h"
#include "process.h"
#include "processinternal.h"
#include "kernelverbdefs.h"
#include "WinSockNetEvents.h"
#include "notify.h"
#include "timedate.h"
#include "langpython.h"


static byte nametargetval [] = "\x08" "_target_";

static byte semaphorewhen [] = "\x04" "when";

static byte semaphorewho [] = "\x03" "who";



typedef enum tylangtoken { /*verbs that are processed by langverbs.c*/
	
	/*lang*/
	
		killscriptfunc,
		
		newfunc,
		
		disposefunc,
		
		
		editfunc,
		
		closefunc,
		
		
		timecreatedfunc,
		
		timemodifiedfunc,
		
		settimecreatedfunc,
		
		settimemodifiedfunc,
		
		booleanfunc, 
		
		charfunc,
		
		intfunc,
		
		longfunc,
		
		datefunc,
		
		directionfunc,
		
		ostypefunc,
		
		stringfunc,
		
		displaystringfunc,
		
		addressfunc,
		
		binaryfunc,
		
		getbinarytypefunc,
		
		setbinarytypefunc,
		
		pointfunc,
		
		rectfunc,
		
		rgbfunc,
		
		patternfunc,
		
		fixedfunc,
		
		singlefunc,
		
		doublefunc,
		
		filespecfunc,
		
		aliasfunc,
		
		listfunc,
		
		recordfunc,
		
		enumfunc,
		
		memavailfunc,
		
		flushmemfunc,
		
		randomfunc,
		
		evaluatefunc,
		
		evaluatethreadfunc,
		
		beachballfunc,
		
		absfunc,
		
		seteventtimeoutfunc,
		
		seteventtransactionidfunc,
		
		seteventinteractionlevelfunc,
		
		geteventattrfunc,
		
		coerceappleitemfunc,
		
		getlistitemfunc,
		
		putlistitemfunc,
		
		countlistitemsfunc,
		
		systemeventfunc,
		
		microsofteventfunc,
		
		transactioneventfunc,
		
		msgfunc,
		
		callxcmdfunc,
		
		calldllfunc,
		
		packwindowfunc,
		
		unpackwindowfunc,

		callscriptfunc,
		
	/*clock*/
	
		timefunc,
		
		settimefunc,
		
		sleepfunc,
		
		tickcountfunc,
		
		millisecondcountfunc,
		
		delayfunc,
		
		delaysixtiethsfunc,
		
	
	/*date*/
		
		getdatefunc,
		
		setdatefunc,

		abbrevstringfunc,

		dayofweekfunc,

		daysinmonthfunc,

		daystringfunc,

		firstofmonthfunc,

		lastofmonthfunc,

		longstringfunc,

		nextmonthfunc,

		nextweekfunc,

		nextyearfunc,

		prevmonthfunc,

		prevweekfunc,

		prevyearfunc,

		shortstringfunc,

		tomorrowfunc,

		weeksinmonthfunc,

		yesterdayfunc,
	
		getcurrenttimezonefunc,
		
		netstandardstringfunc, //AR 07/07/1999
		
		monthtostringfunc, //AR 07/07/1999
		
		dayofweektostringfunc, //AR 07/07/1999
		
		dateversionlessthanfunc, //AR 07/07/1999
		
		datedayfunc,	// SMD 2005-04-07
		
		datemonthfunc,	// SMD 2005-04-07
		
		dateyearfunc,	// SMD 2005-04-07
		
		datehourfunc,	// SMD 2005-04-07
		
		dateminutefunc,	// SMD 2005-04-07
		
		datesecondsfunc,	// SMD 2005-04-07
	
	/*dialog*/
		
		alertdialogfunc,
		
		rundialogfunc,
		
		
		runmodelessfunc,
		
		
		runcardfunc,
		
		runmodalcardfunc,
		
		ismodalcardfunc,
		
		setmodalcardtimeoutfunc,
		
		getdialogvaluefunc,
		
		setdialogvaluefunc,
		
		setdialogitemenablefunc,
		
		showdialogitemfunc,
		
		hidedialogitemfunc,
	
		twowaydialogfunc,
		
		threewaydialogfunc,
		
		askdialogfunc,
		
		getintdialogfunc,

		notifytdialogfunc,

		getuserinfodialogfunc,
		
		askpassworddialogfunc,

	/*kb*/
	
		optionkeyfunc,
		
		cmdkeyfunc,
		
		shiftkeyfunc,
		
		controlkeyfunc,
	
	/*mouse*/
		
		
		mousebuttonfunc,
		
		mouselocationfunc,
		
	
	
	/*point*/
		
		getpointfunc,
		
		setpointfunc,
		
	
	/*rectangle*/
		
		getrectfunc,
		
		setrectfunc,
		
	
	/*rgb*/
		
		getrgbfunc,
		
		setrgbfunc,
		
	
	/*speaker*/
	
		sysbeepfunc,
		
		soundfunc,
		
		playsoundfunc,
	
	
	/*target*/
		
		
		gettargetfunc,
		
		settargetfunc,
		
		cleartargetfunc,
		
		
	
	/*bit*/
		
		getbitfunc,
		
		setbitfunc,
		
		clearbitfunc,

		bitandfunc,

		bitorfunc,

		bitxorfunc,

		bitshiftleftfunc,

		bitshiftrightfunc,

	/*semaphore*/
	
		lockfunc,
		
		unlockfunc,
	
	/*base64*/
		
		base64encodefunc,
		
		base64decodefunc,

	/*winsock - netevents - tcp*/
	
		netaddressdecode,
		netaddressencode,
		netaddresstoname,
		netnametoaddress,
		netmyaddress,
		netabortstream,
		netclosestream,
		netcloselisten,
		netopenaddrstream,
		netopennamestream,
		netreadstream,
		netwritestream,
		netlistenstream,
		netstatusstream,
		netgetpeeraddress,
		netgetpeerport,
		netwritestringtostream,
		netwritefiletostream,
		netreadstreamuntil,
		netreadstreambytes,
		netreadstreamuntilclosed,
		netstatus,
		netcountconnections, /*7.0b37 PBS*/

	/*dll*/

		dllcallfunc,

		dllloadfunc,

		dllunloadfunc,

		dllisloadedfunc,

	/*python*/
		
		pythondoscriptfunc,

	/*htmlcontrol - 08/26/00 by RAB*/

		htmlcontrolbackfunc,

		htmlcontrolforwardfunc,

		htmlcontrolrefreshfunc,

		htmlcontrolhomefunc,

		htmlcontrolstopfunc,

		htmlcontrolnavigatefunc,

		htmlcontrolisofflinefunc,

		htmlcontrolsetofflinefunc,

	/*statusbar -- 7.0b21 PBS -- Bob's verbs*/
		
		statusbarmsgfunc,

		statusbarsetsectionsfunc,

		statusbargetsectionsfunc,

		statusbargetsectiononefunc,

		statusbargetmessagefunc,

	/*winRegistry -- Radio 7.0.2b1 PBS*/
		
		winregistrydeletefunc,

		winregistryreadfunc,

		winregistrygettypefunc,

		winregistrywritefunc,

	numberoflangtokens
	} tylangtoken;



static boolean gettimesverb (hdltreenode hparam1, long *timecreated, long *timemodified) {
	
	/*
	get the creation date and modification date of the object indicated in hparam1
	*/
	
	hdlhashtable htable;
	bigstring bs;
	tyvaluerecord v;
	hdlhashnode hnode;
	
	flnextparamislast = true;
	
	if (!getvarvalue (hparam1, 1, &htable, bs, &v, &hnode))
		return (false);
	
	*timecreated = 0;
	
	*timemodified = 0;
	
	if (v.valuetype == externalvaluetype)
		return (langexternalgettimes ((hdlexternalvariable) v.data.externalvalue, timecreated, timemodified, hnode));
	
	return (false);
	} /*gettimesverb*/


static boolean settimesverb (tylangtoken token, hdltreenode hparam1, tyvaluerecord *vreturned) {
	
	/*
	get the creation date and modification date of the object indicated in hparam1
	*/
	
	hdlhashtable htable;
	bigstring bs;
	tyvaluerecord v;
	hdlexternalvariable hv;
	unsigned long timecreated = 0;
	unsigned long timemodified = 0;
	unsigned long newtime;
	hdlhashnode hnode;
	
	if (!getvarvalue (hparam1, 1, &htable, bs, &v, &hnode))
		return (false);
	
	flnextparamislast = true;
	
	if (!getdatevalue (hparam1, 2, &newtime))
		return (false);
	
	if (v.valuetype != externalvaluetype)
		return (setbooleanvalue (false, vreturned));
	
	hv = (hdlexternalvariable) v.data.externalvalue;
	
	if (!langexternalgettimes (hv, (long *)(&timecreated), (long *)(&timemodified), hnode))
		return (false);
	
	if (token == settimecreatedfunc)
		timecreated = newtime;
	else
		timemodified = newtime;
	
	return (setbooleanvalue (langexternalsettimes (hv, timecreated, timemodified, hnode), vreturned));
	} /*settimesverb*/


static boolean keyboardmodifierverb (tylangtoken token) {
	
	/*
	return true iff the indicated modifier key is down.
	*/
	
	tykeystrokerecord kb;
	
	keyboardpeek (&kb);
	
	switch (token) {
		
		case optionkeyfunc: 
			return (kb.floptionkey);
		
		case cmdkeyfunc: 
			return (kb.flcmdkey);
		
		case shiftkeyfunc: 
			return (kb.flshiftkey);
		
		case controlkeyfunc:
			return (kb.flcontrolkey);
		
		default:
			return (false);
		} /*switch*/
	
	} /*keyboardmodifierverb*/
	
/*
static boolean presskeyverb (char ch) {
	
	register OSErr err;
	long size;
	EvQEl ev;
	short fl;
	short keycode;
	short keychar;
	
	keycode = 0x7b;
	
	keychar = chleftarrow;
	
	err = PPostEvent (keyDown, (keycode << 8) + keychar, &ev);
	
	//ev.evtQModifiers = btnState; /%the mouse is up%/
	
	return (err == noErr);
	} /%presskeyverb%/
*/



static boolean langclosehiddenwindow (tyvaluerecord val) {
	
	/*
	2.1b3 dmb: don't close hidden window if its globals are current

	5.0a22 dmb: don't generate runtime errors trying to clear the target

	5.1.4 dmb: note: we return true is val is a val is a valid 
	address of an existing item.
	*/
	
	hdlhashtable htable;
	bigstring bsname;
	hdlwindowinfo hinfo;
	boolean fl;
	hdlhashnode hnode;
	
	assert (val.valuetype == addressvaluetype); /*08/04/2000 AR*/

	disablelangerror ();

	fl = getaddressvalue (val, &htable, bsname);
	
	if (fl)
		fl = langsymbolreference (htable, bsname, &val, &hnode);
	
	enablelangerror ();
	
	if (!fl)
		return (false);
	
	if (langexternalwindowopen (val, &hinfo)) {
		
		if (((**hinfo).flhidden) && (hinfo != shellwindowinfo))
			shellclosewindow ((**hinfo).macwindow);
		}
	
	return (true);
	} /*langclosehiddenwindow*/


static boolean copyexemptvalue (const tyvaluerecord *v, tyvaluerecord *vcopy) {
	
	/*
	5.0a22 dmb: this was more compilcated before, but it's still shared code
	*/

	boolean fl;
	
	fl = copyvaluerecord (*v, vcopy);
	
	if (fl)
		exemptfromtmpstack (vcopy);
	
	return (fl);
	} /*copyexemptvalue*/


boolean langcleartarget (tyvaluerecord *prevtarget) {
	
	/*
	5.0a22 dmb: added prevtarget parameter
	*/
	
	tyvaluerecord val;
	boolean fl = true;
	hdlhashnode hnode;
	
	if (prevtarget != nil)
		setnilvalue (prevtarget);

	pushouterlocaltable ();
	
	if (hashlookup (nametargetval, &val, &hnode)) {
		
		if (langclosehiddenwindow (val))
			if (prevtarget != nil)
				copyexemptvalue (&val, prevtarget);
		
		if (fl)
			fl = hashdelete (nametargetval, true, true);
		}
	else
		fl = false;
	
	pophashtable ();
	
	return (fl);
	} /*langcleartarget*/


boolean langsettarget (hdlhashtable htable, bigstring bsname, tyvaluerecord *prevtarget) {
	
	/*
	6/22/91 dmb: if existing target is open but hidden, close the window 
	so the script doesn't accumulate a jillion hidden window (or so).

	5.0a22 dmb: added prevtarget parameter

	5.1.4 dmb: if the old target isn't valid (langclosehiddenwindow returns false), 
	set prevtarget to nil
	*/
	
	tyvaluerecord val, oldval;
	boolean flhadtarget;
	boolean fl;
	hdlhashnode hnode;
	
	if (!setaddressvalue (htable, bsname, &val))
		return (false);
	
	pushouterlocaltable ();
	
	flhadtarget = langgetsymbolval (nametargetval, &oldval, &hnode);
	
	if (flhadtarget) { /*we're smashing existing target*/
		
		if (!equalhandles ((Handle) val.data.addressvalue, (Handle) oldval.data.addressvalue)) /*changing*/
			if (!langclosehiddenwindow (oldval))
				setnilvalue (&oldval);
		
		if (prevtarget != nil)
			copyexemptvalue (&oldval, prevtarget);
		}
	else {
		if (prevtarget != nil)
			setnilvalue (prevtarget);
		}
	
	fl = hashassign (nametargetval, val);
	
	pophashtable ();
	
	if (!fl)
		return (false);
	
	exemptfromtmpstack (&val);
	
	return (true);
	} /*langsettarget*/


static boolean langgettarget (hdlhashtable *htable, bigstring bsname) {
	
	tyvaluerecord val;
	boolean fl;
	hdlhashnode hnode;
	
	pushouterlocaltable ();
	
	fl = langgetsymbolval (nametargetval, &val, &hnode);
	
	pophashtable ();
	
	if (!fl)
		return (false);
	
	if (val.valuetype != addressvaluetype) /*not an address*/
		return (false);
	
	return (getaddressvalue (val, htable, bsname));
	} /*langgettarget*/


static boolean langunsettarget (hdlhashtable htable, bigstring bsname) {
	
	/*
	if table, name is the current target, clear the target and return true.
	
	otherwise, return false
	*/
	
	hdlhashtable htargettable;
	bigstring bstargetname;
	
	if (langgettarget (&htargettable, bstargetname)) { /*a target is set*/
		
		if ((htable == htargettable) && equalidentifiers (bsname, bstargetname))
			return (langcleartarget (nil));
		}
	
	return (false);
	} /*langunsettarget*/



static boolean newvaluefunc (hdltreenode hparam1, tyvaluerecord *vreturned) {
	
	/*
	10/4/91 dmb: create heap-based values properly
	
	4.1b2 dmb: don't use getvarparam for the address param; it's too soft
	and can mistakenly wipe out a value that can't be coerced to an address 
	as expected. use getaddressparam instead

	5.0a22 dmb: new tables are local if creatdd in a local table
	*/
	
	hdlhashtable htable;
	bigstring bs;
	OSType typeid;
	tyvaluetype type;
	tyvaluerecord val;
	boolean fl;
	hdlhashtable newtable;
	
	if (!getostypevalue (hparam1, 1, &typeid))
		return (false);
	
	flnextparamislast = true;
	
//	if (!getvarparam (hparam1, 2, &htable, bs))
	if (!getaddressparam (hparam1, 2, &val) || !getaddressvalue (val, &htable, bs))
		return (false);
	
	if (htable == nil) { /*4.1b7 dmb*/
		
		if (isemptystring (bs)) {
			
			langerror (niladdresserror);
			
			return (false);
			}
		
		langsearchpathlookup (bs, &htable);
		}
	
	type = langgetvaluetype (typeid);
	
	if ((type >= outlinevaluetype) && (type <= pictvaluetype)) {
		
		if (!langexternalnewvalue ((tyexternalid) (type - outlinevaluetype), nil, &val))
			return (false);

		if ((type == tablevaluetype) && langexternalvaltotable (val, &newtable, HNoNode))
			(**newtable).fllocaltable = (**htable).fllocaltable;
		}
	else {
		initvalue (&val, novaluetype); /*nil all data*/
		
		if (!coercevalue (&val, type)) /*should only fail on low-mem*/
			return (false);
		}
	
	fl = langsetsymboltableval (htable, bs, val);
	
	if (fl)
		exemptfromtmpstack (&val);
	
	else {
		
		disposevaluerecord (val, true);
		
		return (false);
		}
	
	(*vreturned).data.flvalue = true;
	
	return (true);
	} /*newvaluefunc*/


static boolean disposevaluefunc (hdltreenode hparam1, tyvaluerecord *vreturned) {
	
	/*
	dispose (address): boolean; completely dispose the indicated table value
	
	12/17/91 dmb: make sure item isn't the target
	
	2.1b2 dmb: use new deletevalue for arrays
	*/
	
	register hdltreenode hp1 = hparam1;
	hdlhashtable htable;
	bigstring bs;
	
	if (!langcheckparamcount (hp1, 1))
		return (false);
	
	if ((**hp1).nodetype == addressofop)
		hp1 = (**hp1).param1;
	
	if ((**hp1).nodetype == arrayop)
		return (deletevalue (hp1, vreturned));
	
	if (!getvarparam (hparam1, 1, &htable, bs)) /*use original hparam1, not hp1*/
		return (false);
	
	
	langunsettarget (htable, bs); /*make sure it's not still the target*/
	
	
	(*vreturned).data.flvalue = hashtabledelete (htable, bs);
	
	return (true);
	} /*disposevaluefunc*/



boolean langzoomvalwindow (hdlhashtable htable, bigstring bs, tyvaluerecord val, boolean flmakevisible) {
	
	/*
	7/4/91 dmb: langexternalzoom now takes table/name pair instead 
	of full path.
	*/
	
	Rect rzoom;
	
	if (flmakevisible)
		rzoom.top = -1; /*visible window; accept default rect*/
	else
		rzoom.top = -2; /*invisible window; don't zoom yet*/
	
	return (langexternalzoomfrom (val, htable, bs, &rzoom));
	} /*langzoomvalwindow*/


boolean langfindtargetwindow (short id, WindowPtr *targetwindow) {
	
	/*
	find the target window for a verb; id is the external id for the EFP 
	that supports the verb (-1 for a shell verb)
	
	if we find a target address in the local stack, we return the corresponding 
	window if it supports the verb type.  for now, we require that the window 
	be open.
	
	otherwise, we call the shell to locate the best target
	
	10/3/91 dmb: tightened default logic.  we're no longer willing to 
	look byond the first eligable window for a window that is willing 
	to set up globals for id.  so if no target is set, we pass -1 to 
	shellfindtargetwindow instead of id; then we see if the window returned 
	will actually support this kind of verb.
	*/
	
	hdlhashtable htable;
	bigstring bsname;
	tyvaluerecord val;
	hdlwindowinfo hinfo;
	register WindowPtr w;
	hdlhashnode hnode;
	
	if (!langgettarget (&htable, bsname)) { /*no target set; try default window*/
		
		if (!shellfindtargetwindow (-1, targetwindow)) /*should never fail*/
			return (false);
		
		return (shellgettargetdata (id, *targetwindow));
		}
	
	if (!langsymbolreference (htable, bsname, &val, &hnode))
		return (false);
	
	if (!langexternalwindowopen (val, &hinfo)) {
		
		if (!langzoomvalwindow (htable, bsname, val, false))
			return (false);
		
		if (!langexternalwindowopen (val, &hinfo)) {
			
			shellinternalerror (idwindowzoombug, BIGSTRING ("\x23" "windowzoom/windowopen inconsistency"));
			
			return (false);
			}
		}
	
	w = (**hinfo).macwindow; /*move into register*/
	
	if (!shellgettargetdata (id, w))
		return (false);
	
	*targetwindow = w;
	
	return (true);
	} /*langfindtargetwindow*/


static boolean editvalue (hdltreenode hparam1, tyvaluerecord *vreturned) {
	
	hdlhashtable htable;
	bigstring bsname;
	tyvaluerecord val;
	hdlhashnode hnode;
	
	flnextparamislast = true;
	
	if (!getvarvalue (hparam1, 1, &htable, bsname, &val, &hnode))
		return (false);
	
	if (val.valuetype == externalvaluetype) {
		
		if (!langzoomvalwindow (htable, bsname, val, true))
			return (false);
		
		langsettarget (htable, bsname, nil); /*for future editing verbs*/
		}
	else {
		if (!tablezoomtoname (htable, bsname))
			return (false);
		}
	
	setbooleanvalue (true, vreturned);
	
	return (true);
	} /*editvalue*/


static boolean closevalue (hdltreenode hparam1, tyvaluerecord *vreturned) {
	
	/*
	if the indicated value has it's window open, close it.  also, if it is 
	the current target, clear the target.
	
	set the return value to true if either change occurs.
	
	7.23.97 dmb: use shellclose, not shellclosewindow, to handle file
	objects.
	*/
	
	hdlhashtable htable;
	bigstring bsname;
	tyvaluerecord val;
	hdlwindowinfo hinfo;
	Handle hdata;
	boolean fl = false;
	hdlhashnode hnode;
	
	flnextparamislast = true;
	
	if (!getvarvalue (hparam1, 1, &htable, bsname, &val, &hnode))
		return (false);
	
	if (langexternalwindowopen (val, &hinfo))
		if ((**hinfo).parentwindow != nil || shellgetexternaldata (hinfo, &hdata))
			fl = shellclose ((**hinfo).macwindow, false);
	
	if (langunsettarget (htable, bsname))
		fl = true;
	
	return (setbooleanvalue (fl, vreturned));
	} /*closevalue*/


static boolean langgettargetfunc (hdltreenode hparam1, tyvaluerecord *vreturned) {
	
	/*
	5.0a22 dmb: if there's no explicit or implicit target, return nil
	*/

	hdlhashtable htable;
	bigstring bsname;
	boolean fl;
	
	if (!langcheckparamcount (hparam1, 0))
		return (false);
	
	fl = langgettarget (&htable, bsname);

	if (!fl) {
		
		WindowPtr target;
		hdlexternalvariable hvariable;
		
		htable = nil;
		
		setemptystring (bsname);
		
		if (langfindtargetwindow (-1, &target)) {
			
			shellpushglobals (target);
			
			if ((*shellglobals.getvariableroutine) (&hvariable))
				fl = langexternalfindvariable (hvariable, &htable, bsname);
			
			shellpopglobals ();
			}
		}
	
	if (fl)
		return (setaddressvalue (htable, bsname, vreturned));
	else
		return (setnilvalue (vreturned));
	} /*langgettargetfunc*/


static boolean langsettargetfunc (hdltreenode hparam1, tyvaluerecord *vreturned) {
	
	/*
	8/21/91 dmb: don't generate error if type isn't external; just return false
	
	6/23/92 dmb: open target window now, instead of waiting for an editing 
	verb to trigger it via findtargetwindow.  this allows window verbs to 
	operator on it.

	5.0a22 dmb: if there's an error zooming, don't leave target value set.
				if called with nil, clear the target
	*/
	
	hdlhashtable htable;
	bigstring bsname;
	tyvaluerecord val;
	hdlhashnode hnode;
	
	setbooleanvalue (false, vreturned); /*default return*/
	
	flnextparamislast = true;
	
	if (!getvarvalue (hparam1, 1, &htable, bsname, &val, &hnode))
		return (false);
	
	if (val.valuetype != externalvaluetype) {
		
		if (val.valuetype == novaluetype) { // 5.0a22
			
			if (langcleartarget (vreturned))
				pushtmpstackvalue (vreturned);

			return (true);
			}

		langparamerror (externalvaluerequirederror, bsname);
		
		return (false);
		}
	
	if (!langsettarget (htable, bsname, vreturned))
		return (false);
	
	if (!langzoomvalwindow (htable, bsname, val, false)) {
		
		disposevaluerecord (*vreturned, false);
		
		disablelangerror ();
		
		langcleartarget (nil);
		
		enablelangerror ();
		
		return (false);
		}
	
//	5.0a22 dmb: vretured is already set now. was - setbooleanvalue (true, vreturned);
	
	pushtmpstackvalue (vreturned);
	
	return (true);
	} /*langsettargetfunc*/


static boolean getuserinfofunc (hdltreenode hparam1, tyvaluerecord *vreturned) {
	
	bigstring bsvarname, bsvarinitials, bsvarorg, bsvaremail;
	bigstring bsname, bsinitials, bsorg, bsemail;
	hdlhashtable htable;
	register boolean fl;
	
	if (!getvarparam (hparam1, 1, &htable, bsvarname)) 
		return (false);

	if (!getvarparam (hparam1, 2, &htable, bsvarinitials)) 
		return (false);

	if (!getvarparam (hparam1, 3, &htable, bsvarorg)) 
		return (false);
	
	flnextparamislast = true;
	
	if (!getvarparam (hparam1, 4, &htable, bsvaremail)) 
		return (false);
	
	idstringvalue (htable, bsvarname, bsname); /*not an error if string is undefined*/
	
	if (fllangerror) /*probably a bad array reference*/
		return (false);
	
	idstringvalue (htable, bsvarinitials, bsinitials); /*not an error if string is undefined*/
	
	if (fllangerror) /*probably a bad array reference*/
		return (false);
	
	idstringvalue (htable, bsvarorg, bsorg); /*not an error if string is undefined*/
	
	if (fllangerror) /*probably a bad array reference*/
		return (false);
	
	idstringvalue (htable, bsvaremail, bsemail); /*not an error if string is undefined*/
	
	if (fllangerror) /*probably a bad array reference*/
		return (false);
	
		
		fl = false;  //not supported on this platform

	setbooleanvalue (fl, vreturned);
	
	if (fl) { /*user pressed OK*/
		
		pushhashtable (htable);
		
		langsetstringval (bsvarname, bsname);

		langsetstringval (bsvarinitials, bsinitials);

		langsetstringval (bsvarorg, bsorg);
		
		langsetstringval (bsvaremail, bsemail);
		
		pophashtable ();
		
		return (true);
		}
		
	return (true);
	} /*getuserinfofunc*/


static boolean askfunc (hdltreenode hparam1, tyvaluerecord *vreturned, boolean flPassword) {
	
	bigstring bsprompt, bsvarname, bsanswer;
	hdlhashtable htable;
	register boolean fl;
	
	if (!getstringvalue (hparam1, 1, bsprompt))
		return (false);
	
	flnextparamislast = true;
	
	if (!getvarparam (hparam1, 2, &htable, bsvarname)) 
		return (false);
	
	idstringvalue (htable, bsvarname, bsanswer); /*not an error if string is undefined*/
	
	if (fllangerror) /*probably a bad array reference*/
		return (false);
	
	if (flPassword)
		fl = askpassword (bsprompt, bsanswer);
	else
		fl = askdialog (bsprompt, bsanswer);
	
	setbooleanvalue (fl, vreturned);
	
	if (fl) { /*user pressed OK*/
		
		pushhashtable (htable);
		
		langsetstringval (bsvarname, bsanswer);
		
		pophashtable ();
		
		return (true);
		}
		
	return (true);
	} /*askfunc*/


static boolean getintfunc (hdltreenode hparam1, tyvaluerecord *vreturned) {
	
	bigstring bsprompt, bsvarname, bsanswer;
	hdlhashtable htable;
	register boolean fl;
	short n;
	long longn;
	tyvaluerecord val;
	
	if (!getstringvalue (hparam1, 1, bsprompt))
		return (false);
	
	flnextparamislast = true;
	
	if (!getvarparam (hparam1, 2, &htable, bsvarname)) 
		return (false);
	
	idstringvalue (htable, bsvarname, bsanswer); /*not an error if string is undefined*/
	
	if (fllangerror) /*probably a bad array reference*/
		return (false);
	
	stringtonumber (bsanswer, &longn);

	n = (short) longn;

	fl = intdialog (bsprompt, &n);
	
	setbooleanvalue (fl, vreturned);
	
	if (fl) { /*user pressed OK*/
		
		setintvalue (n, &val);
		
		langsetsymboltableval (htable, bsvarname, val);
		
		return (true);
		}
		
	return (true);
	} /*getintfunc*/


static boolean twowayfunc (hdltreenode hparam1, tyvaluerecord *v) {

	bigstring bsprompt, bs1, bs2;
	
	if (!getstringvalue (hparam1, 1, bsprompt))
		return (false);
	
	if (!getstringvalue (hparam1, 2, bs1))
		return (false);
	
	flnextparamislast = true;
	
	if (!getstringvalue (hparam1, 3, bs2))
		return (false);
	
	(*v).data.flvalue = twowaydialog (bsprompt, bs1, bs2);
	
	return (true);
	} /*twowayfunc*/


static boolean threewayfunc (hdltreenode hparam1, tyvaluerecord *v) {

	bigstring bsprompt, bs1, bs2, bs3;
	
	if (!getstringvalue (hparam1, 1, bsprompt))
		return (false);
	
	if (!getstringvalue (hparam1, 2, bs1))
		return (false);
	
	if (!getstringvalue (hparam1, 3, bs2))
		return (false);
	
	flnextparamislast = true;
	
	if (!getstringvalue (hparam1, 4, bs3))
		return (false);
	
	return (setintvalue (threewaydialog (bsprompt, bs1, bs2, bs3), v));
	} /*threewayfunc*/




static boolean callscriptverb (hdltreenode hparam1, tyvaluerecord *vreturned) {
	
	/*
	3/29/98 rab: created.
	*/
	
	register hdltreenode hp1 = hparam1;
	bigstring bsscriptname;
	tyvaluerecord vparams;
	hdlhashtable hcontext = nil;
	boolean fl;
	
	if (!getstringvalue (hp1, 1, bsscriptname))
		return (false);
	
	if (!getparamvalue (hp1, 2, &vparams))
		return (false);
	
	if (vparams.valuetype != recordvaluetype)
		if (!coercetolist (&vparams, listvaluetype))
			return (false);
	
	if (langgetparamcount (hparam1) > 2) {
		
		flnextparamislast = true;

		if (!gettablevalue (hparam1, 3, &hcontext))
			return (false);
		}
	
	fl = langrunscript (bsscriptname, &vparams, hcontext, vreturned);
	
	return (fl);
	} /*callscriptverb*/


static boolean getbitparams (hdltreenode hparam1, unsigned long *bits, unsigned short *bitnum, short ixerror) {
	
	if (!getlongvalue (hparam1, 1, (long *) bits))
		return (false);
	
	flnextparamislast = true;
	
	if (!getintvalue (hparam1, 2, (short *) bitnum))
		return (false);
	
	if (*bitnum > 31) {
		
		langerror (ixerror);
		
		return (false);
		}
	
	return (true);
	} /*getbitparams*/
	
	
static boolean getbitnumparams (hdltreenode hparam1, unsigned long *bits1, unsigned long *bits2) {
	
	if (!getlongvalue (hparam1, 1, (long *) bits1))
		return (false);
	
	flnextparamislast = true;
	
	if (!getlongvalue (hparam1, 2, (long *) bits2))
		return (false);
	
	return (true);
	} /*getbitnumparams*/
	
	
static boolean bitgetverb (hdltreenode hparam1, tyvaluerecord *vreturned) {
	
	unsigned long bits;
	unsigned short bitnum;
	
	if (!getbitparams (hparam1, &bits, &bitnum, bitindexerror))
		return (false);
	
	return (setbooleanvalue ((bits >> bitnum) & 1, vreturned));
	} /*bitgetverb*/
	

static boolean bitsetverb (hdltreenode hparam1, tyvaluerecord *vreturned) {
	
	unsigned long bits;
	unsigned short bitnum;
	
	if (!getbitparams (hparam1, &bits, &bitnum, bitindexerror))
		return (false);
	
	return (setlongvalue (bits | ((unsigned long) 1 << bitnum), vreturned));
	} /*bitsetverb*/


static boolean bitclearverb (hdltreenode hparam1, tyvaluerecord *vreturned) {
	
	unsigned long bits;
	unsigned short bitnum;
	
	if (!getbitparams (hparam1, &bits, &bitnum, bitindexerror))
		return (false);
	
	return (setlongvalue (bits & (~((unsigned long) 1 << bitnum)), vreturned));
	} /*bitclearverb*/


static boolean bitandverb (hdltreenode hparam1, tyvaluerecord *vreturned) {
	
	unsigned long bits1;
	unsigned long bits2;
	
	if (!getbitnumparams (hparam1, &bits1, &bits2))
		return (false);
	
	return (setlongvalue (bits1 & bits2, vreturned));
	} /*bitandverb*/


static boolean bitorverb (hdltreenode hparam1, tyvaluerecord *vreturned) {
	
	unsigned long bits1;
	unsigned long bits2;
	
	if (!getbitnumparams (hparam1, &bits1, &bits2))
		return (false);
	
	return (setlongvalue (bits1 | bits2, vreturned));
	} /*bitorverb*/


static boolean bitxorverb (hdltreenode hparam1, tyvaluerecord *vreturned) {
	
	unsigned long bits1;
	unsigned long bits2;
	
	if (!getbitnumparams (hparam1, &bits1, &bits2))
		return (false);
	
	return (setlongvalue (bits1 ^ bits2, vreturned));
	} /*bitxorverb*/


static boolean bitshiftleftverb (hdltreenode hparam1, tyvaluerecord *vreturned) {
	
	unsigned long bits;
	unsigned short bitdist;
	
	if (!getbitparams (hparam1, &bits, &bitdist, bitshiftdisterror))
		return (false);
	
	return (setlongvalue (bits << bitdist, vreturned));
	} /*bitshiftleftverb*/


static boolean bitshiftrightverb (hdltreenode hparam1, tyvaluerecord *vreturned) {
	
	unsigned long bits;
	unsigned short bitdist;
	
	if (!getbitparams (hparam1, &bits, &bitdist, bitshiftdisterror))
		return (false);
	
	return (setlongvalue (bits >> bitdist, vreturned));
	} /*bitshiftrightverb*/


static boolean locksemaphoreverb (hdltreenode hparam1, tyvaluerecord *vreturned) {
	
	/*
		on lock (semaphorename, timeoutticks)
			local (adr = @semaphores.values [semaphorename])
			local (startticks = clock.ticks ())
			while defined (adr^)
				sys.systemtask () Çgive up the processor
				if (clock.ticks () - startticks) > timeoutticks  Çwaited more than timeoutticks, get out
					scriptError ("Semaphore timer expired after " + timeoutticks + " sixtieths of a second.")
			new (booleantype, adr) Çclaim the semaphore
			return (true)
	*/
	
	bigstring bssemaphorename;
	long timeoutticks;
	long startticks = gettickcount ();
	bigstring bsticks;
	tyvaluerecord val;
		hdllistrecord hlist;
	
	if (!getstringvalue (hparam1, 1, bssemaphorename))
		return (false);
	
	flnextparamislast = true;
	
	if (!getlongvalue (hparam1, 2, &timeoutticks))
		return (false);
	
	while (hashtablesymbolexists (semaphoretable, bssemaphorename)) {
		
		if (!langbackgroundtask (true))
			return (false);
		
		if (gettickcount () - startticks >= (unsigned long) timeoutticks) {
		
			numbertostring (timeoutticks, bsticks);
			
			langparamerror (semaphoretimeouterror, bsticks);
			
			return (false);
			}
		}
	
		if (!opnewlist (&hlist, true))
			return (false);
		
		setdatevalue (timenow(), &val);
		
		if (!langpushlistval (hlist, semaphorewhen, &val))
			goto error;
		
		setlongvalue ((long) (**getcurrentthreadglobals()).idthread, &val);
		
		if (!langpushlistval (hlist, semaphorewho, &val))
			goto error;
		
		if (!setheapvalue ((Handle) hlist, recordvaluetype, &val))
			return (false);
	
	if (!hashtableassign (semaphoretable, bssemaphorename, val))
		return (false);
	
	exemptfromtmpstack (&val);
	
	return (setbooleanvalue (true, vreturned));
	
		error:
			opdisposelist (hlist);
			
			return (false);
	} /*locksemaphoreverb*/


static boolean unlocksemaphoreverb (hdltreenode hparam1, tyvaluerecord *vreturned) {
	
	/*
		on unlock (semaphorename)
			local (adr = @semaphores.values [semaphorename])
			if defined (adr^) << added 9/10/95 MAH - checks for semaphore before deleting
				delete (adr) <<release the semaphore
			return (true)
	*/

	bigstring bssemaphorename;
	boolean fl;
	
	flnextparamislast = true;
	
	if (!getstringvalue (hparam1, 1, bssemaphorename))
		return (false);
	
	if (isemptystring (bssemaphorename))
		fl = langreleasesemaphores (nil);

	else {

		pushhashtable (semaphoretable);
		
		fl = hashsymbolexists (bssemaphorename) && hashdelete (bssemaphorename, true, true);
		
		pophashtable ();
		}
	
	return (setbooleanvalue (fl, vreturned));
	} /*unlocksemaphoreverb*/


static boolean releasesemaphorevisit (hdlhashnode hnode, ptrvoid idthread) {
	
	tyvaluerecord val = (**hnode).val;
	tyvaluerecord vwho;
	
	if (val.valuetype == recordvaluetype) {
	
		if (langgetlistitem (&val, -1, semaphorewho, &vwho)) {
			
			if (vwho.data.longvalue == (long) idthread)
				hashdelete ((**hnode).hashkey, true, false);
			
			disposevaluerecord (vwho, false);
			}
		}
	
	return (true); // continue traversal
} /*releasesemaphorevisit*/


boolean
langreleasesemaphores (hdlprocessrecord xxxhp)
{
#pragma unused(xxxhp)

	pushhashtable (semaphoretable); // for visit's hashdelete

	hashtablevisit (semaphoretable, &releasesemaphorevisit, (ptrvoid) (**getcurrentthreadglobals()).idthread);
	
	pophashtable ();
	
	return (true);
	} /*langreleasesemaphores*/


static boolean langfunctionvalue (short token, hdltreenode hparam1, tyvaluerecord *vreturned, bigstring bserror) {
	
	/*
	9/26/91 dmb: use getdatevalue for time/date verbs so that string coercion 
	can be performed.  reimplemented absfunc so that it handles (and maintains) 
	arbitrary types.  also, set default return value for all verbs to false.
	
	2/14/92 dmb: evaluate & runstring no longer use bigstrings
	
	5/29/92 dmb: scripterror (killscriptfunc) must always return false to ensure 
	proper termination
	
	6/24/92 dmb: added countlistitemsfunc
	
	8/17/92 dmb: setbinarytypefunc -- set table's dirty flag
	
	9/4/92 dmb: generate oserror for numeric scriptError
	
	9/11/92 dmb: sleepfunc: accept verb when a dialog is running
	
	10/3/92 dmb: comment out runstringfunc
	
	6/1/93 dmb: when vreturned is nil, return whether or not verb token must 
	be run in the Frontier process
	
	2.1b12 dmb: runcardfunc and runmodalcardfunc must run in frontier process
	
	3.0.4b8 dmb: msgfunc requires a window

	6.1d13 AR: Added netwritestringtostream, netreadstreamuntil, netreadstreambytes.
	
	6.2b10 AR: Added netwritefiletostream.
	*/
	
	register tyvaluerecord *v = vreturned;
	register boolean fl;
	bigstring bs;
	
	if (vreturned == nil) { /*need Frontier process?*/
		
		switch (token) {
			
			
			case editfunc:
			case gettargetfunc:
			case settargetfunc:
			case cleartargetfunc:
			
			
			case msgfunc:			// 3.0.4b8 dmb
			case runmodelessfunc:	// 4.1b3 dmb
			/*
			case runcardfunc:
			case runmodalcardfunc:
			*/
				return (true);
			
			default:
				return (false);
			}
	}

	setbooleanvalue (false, v); /*by default, lang functions return false*/

	switch (token)
	{
		case killscriptfunc:
		{
			tyvaluerecord val;
			boolean lfl;

			flnextparamislast = true;

			if (!getparamvalue (hparam1, 1, &val))
				return (false);

			if (val.valuetype == longvaluetype)
			{
				langgetmiscstring (unknownstring, bs);

				setoserrorparam (bs);

				lfl = oserror (val.data.longvalue);
			}
			else
			{
				if (!coercetostring (&val))
					return (false);

				pullstringvalue (&val, bs);

				lfl = langerrormessage (bs);
			}

			/*
			if (!getstringvalue (hparam1, 1, bs))
				return (false);
			*/

			(*v).data.flvalue = lfl;

			return (false);
		}

		case newfunc:
			return (newvaluefunc (hparam1, v));
		
		case disposefunc:
			return (disposevaluefunc (hparam1, v));
		
		
		case editfunc:
			return (editvalue (hparam1, v));
		
		case closefunc:
			return (closevalue (hparam1, v));
		
		case gettargetfunc:
			return (langgettargetfunc (hparam1, v));
		
		case settargetfunc:
			return (langsettargetfunc (hparam1, v));
		
		case cleartargetfunc:
			if (!langcheckparamcount (hparam1, 0))
				return (false);
			
			setbooleanvalue (langcleartarget (nil), v);
			
			return (true);
		
		
		case booleanfunc:
			flnextparamislast = true;
			
			return (getbooleanparam (hparam1, 1, v));
		
		case charfunc:
			flnextparamislast = true;
			
			return (getcharparam (hparam1, 1, v));
		
		case intfunc:
			flnextparamislast = true;
			
			return (getintparam (hparam1, 1, v));
		
		case longfunc:
			flnextparamislast = true;
			
			return (getlongparam (hparam1, 1, v));
		
		case timefunc: {
			
			if (!langcheckparamcount (hparam1, 0))
				return (false);
			
			return (setdatevalue (timenow (), v));
			}
		
		
		case datefunc:
			flnextparamislast = true;
			
			return (getdateparam (hparam1, 1, v));
		
		case setdatefunc: {
			short day, month, year, hour, minute, second;
			unsigned long date;
			
			if (!getintvalue (hparam1, 1, &day))
				return (false);
			
			if (!getintvalue (hparam1, 2, &month))
				return (false);
			
			if (!getintvalue (hparam1, 3, &year))
				return (false);
			
			if (!getintvalue (hparam1, 4, &hour))
				return (false);
			
			if (!getintvalue (hparam1, 5, &minute))
				return (false);
			
			flnextparamislast = true;
			
			if (!getintvalue (hparam1, 6, &second))
				return (false);
			
			date = datetimetoseconds (day, month, year, hour, minute, second);
			
			return (setdatevalue (date, v));
			}
		
		case getdatefunc: {
			unsigned long secs;
			short day, month, year, hour, minute, second;
			
			if (!langcheckparamcount (hparam1, 7)) /*preflight before changing values*/
				return (false);
			
			if (!getdatevalue (hparam1, 1, &secs))
				return (false);
			
			secondstodatetime (secs, &day, &month, &year, &hour, &minute, &second);
			
			if (!setintvarparam (hparam1, 2, day))
				return (false);
			
			if (!setintvarparam (hparam1, 3, month))
				return (false);
			
			if (!setintvarparam (hparam1, 4, year))
				return (false);
			
			if (!setintvarparam (hparam1, 5, hour))
				return (false);
			
			if (!setintvarparam (hparam1, 6, minute))
				return (false);
			
			if (!setintvarparam (hparam1, 7, second))
				return (false);
			
			(*v).data.flvalue = true;
			
			return (true);
			}
		
		case datedayfunc:		/* SMD 2005-04-07 */
		case datemonthfunc:
		case dateyearfunc:
		case datehourfunc:
		case dateminutefunc:
		case datesecondsfunc: {
			tyvaluerecord vsecs;
			short ctconsumed = 0;
			short ctpositional = 0;
			short day, month, year, hour, minute, second;
			
			initvalue (&vsecs, datevaluetype);

			flnextparamislast = true;
			
			if (!getoptionalparamvalue (hparam1, &ctconsumed, &ctpositional, BIGSTRING ("\x07" "theDate"), &vsecs)) 
				return (false);
			
			if (!vsecs.data.datevalue) {
				if (!setdatevalue (timenow(), &vsecs))
					return (false);
				}
			
			secondstodatetime (vsecs.data.datevalue, &day, &month, &year, &hour, &minute, &second);
			
			switch (token) {
				case datedayfunc:
					return (setintvalue (day, v));
				case datemonthfunc:
					return (setintvalue (month, v));
				case dateyearfunc:
					return (setintvalue (year, v));
				case datehourfunc:
					return (setintvalue (hour, v));
				case dateminutefunc:
					return (setintvalue (minute, v));
				case datesecondsfunc:
					return (setintvalue (second, v));
				}
			}

		case abbrevstringfunc: {
			unsigned long date;

			flnextparamislast = true;
			
			if (!getdatevalue (hparam1, 1, &date))
				return (false);

			abbrevdatestring (date, bs);

			return (setstringvalue (bs, v));
			}

		case dayofweekfunc: {
			unsigned long date;
			short day;

			flnextparamislast = true;
			
			if (!getdatevalue (hparam1, 1, &date))
				return (false);

			secondstodayofweek (date, &day);

			return (setintvalue (day, v));
			}

		case daysinmonthfunc: {
			unsigned long date;
			short day, month, year, hour, minute, second;

			flnextparamislast = true;
			
			if (!getdatevalue (hparam1, 1, &date))
				return (false);

			secondstodatetime (date, &day, &month, &year, &hour, &minute, &second);

			day = daysInMonth (month, year);

			return (setintvalue (day, v));
			}

		case daystringfunc: {
			unsigned long date;
			short dayofweek;

			flnextparamislast = true;
			
			if (!getdatevalue (hparam1, 1, &date))
				return (false);

			secondstodayofweek (date, &dayofweek);

			getdaystring (dayofweek, bs, true);

			return (setstringvalue (bs, v));
			}

		case firstofmonthfunc: {
			unsigned long date;

			flnextparamislast = true;
			
			if (!getdatevalue (hparam1, 1, &date))
				return (false);

			date = firstofmonth(date);

			return (setdatevalue (date, v));
			}

		case lastofmonthfunc: {
			unsigned long date;

			flnextparamislast = true;
			
			if (!getdatevalue (hparam1, 1, &date))
				return (false);

			date = lastofmonth(date);

			return (setdatevalue (date, v));
			}

		case longstringfunc: {
			unsigned long date;

			flnextparamislast = true;
			
			if (!getdatevalue (hparam1, 1, &date))
				return (false);

			longdatestring (date, bs);

			return (setstringvalue (bs, v));
			}

		case nextmonthfunc: {
			unsigned long date;

			flnextparamislast = true;
			
			if (!getdatevalue (hparam1, 1, &date))
				return (false);

			date = nextmonth(date);

			return (setdatevalue (date, v));
			}

		case nextweekfunc: {
			unsigned long date;

			flnextparamislast = true;
			
			if (!getdatevalue (hparam1, 1, &date))
				return (false);

			date = date + (7L*24L*60L*60L);

			return (setdatevalue (date, v));
			}

		case nextyearfunc: {
			unsigned long date;

			flnextparamislast = true;
			
			if (!getdatevalue (hparam1, 1, &date))
				return (false);

			date = nextyear(date);

			return (setdatevalue (date, v));
			}

		case prevmonthfunc: {
			unsigned long date;

			flnextparamislast = true;
			
			if (!getdatevalue (hparam1, 1, &date))
				return (false);

			date = prevmonth(date);

			return (setdatevalue (date, v));
			}

		case prevweekfunc: {
			unsigned long date;

			flnextparamislast = true;
			
			if (!getdatevalue (hparam1, 1, &date))
				return (false);

			date = date - (7L*24L*60L*60L);

			return (setdatevalue (date, v));
			}

		case prevyearfunc: {
			unsigned long date;

			flnextparamislast = true;
			
			if (!getdatevalue (hparam1, 1, &date))
				return (false);

			date = prevyear(date);

			return (setdatevalue (date, v));
			}

		case shortstringfunc: {
			unsigned long date;

			flnextparamislast = true;
			
			if (!getdatevalue (hparam1, 1, &date))
				return (false);

			shortdatestring (date, bs);

			return (setstringvalue (bs, v));
			}

		case tomorrowfunc: {
			unsigned long date;

			flnextparamislast = true;
			
			if (!getdatevalue (hparam1, 1, &date))
				return (false);

			date = date + (24L*60L*60L);

			return (setdatevalue (date, v));
			}

		case weeksinmonthfunc: {
			unsigned long date;
			short day, month, year, hour, minute, second, dayoffset;

			flnextparamislast = true;
			
			if (!getdatevalue (hparam1, 1, &date))
				return (false);

			date = firstofmonth (date);

			secondstodatetime (date, &day, &month, &year, &hour, &minute, &second);

			day = daysInMonth (month, year);

			secondstodayofweek (date, &dayoffset);
			--dayoffset; /* set ot 0 to 6 */

			return (setintvalue ((short)((day + 6 + dayoffset) / 7), v));
			}

		case yesterdayfunc: {
			unsigned long date;

			flnextparamislast = true;
			
			if (!getdatevalue (hparam1, 1, &date))
				return (false);

			date = date - (24L*60L*60L);

			return (setdatevalue (date, v));
			}

		case getcurrenttimezonefunc: {
			if (!langcheckparamcount (hparam1, 0))
				return (false);

			return (setlongvalue (getcurrenttimezonebias(), v));
			}

		case netstandardstringfunc: { //AR 07/07/1999
			unsigned long date;

			flnextparamislast = true;
			
			if (!getdatevalue (hparam1, 1, &date))
				return (false);
						
			return (datenetstandardstring (date, v));
			}
		
		case monthtostringfunc: { //AR 07/07/1999
			long x;

			flnextparamislast = true;
			
			if (!getlongvalue (hparam1, 1, &x))
				return (false);
			
			return (datemonthtostring (x, v));
			}

		case dayofweektostringfunc: { //AR 07/07/1999
			long x;

			flnextparamislast = true;
			
			if (!getlongvalue (hparam1, 1, &x))
				return (false);
			
			return (datedayofweektostring (x, v));
			}

		case dateversionlessthanfunc: { //AR 07/07/1999
			bigstring bs1, bs2;
			Handle h1, h2;
			
			if (!getreadonlytextvalue (hparam1, 1, &h1))
				return (false);

			flnextparamislast = true;

			if (!getreadonlytextvalue (hparam1, 2, &h2))
				return (false);
			
			texthandletostring (h1, bs1);

			texthandletostring (h2, bs2);
			
			return (dateversionlessthan (bs1, bs2, v));
			}

		case directionfunc:
			flnextparamislast = true;
			
			return (getdirectionparam (hparam1, 1, v));
		
		case ostypefunc:
			flnextparamislast = true;
			
			return (getostypeparam (hparam1, 1, v));
		
		case stringfunc:
			flnextparamislast = true;
			
			flcoerceexternaltostring = true; /*special case -- enable for this verb*/
			
			fl = getstringparam (hparam1, 1, v);
			
			flcoerceexternaltostring = false;
			
			return (fl);
		
		case displaystringfunc:
			flnextparamislast = true;
			
			if (!getparamvalue (hparam1, 1, v))
				break;
			
			flcoerceexternaltostring = true; /*special case -- enable for this verb*/
			
			fl = getobjectmodeldisplaystring (v, bs);
			
			flcoerceexternaltostring = false;
			
			if (!fl)
				break;
			
			return (setstringvalue (bs, v));
			
		case addressfunc:
			flnextparamislast = true;
			
			return (getaddressparam (hparam1, 1, v));
		
		case binaryfunc:
			flnextparamislast = true;
			
			return (getbinaryparam (hparam1, 1, v));
		
		case getbinarytypefunc: {
			Handle x;
			tyvaluerecord val;
			
			flnextparamislast = true;
			
				if (!getparamvalue (hparam1, 1, &val))
					break;
				
				if (!coercetobinary (&val))
					break;
				
				x = val.data.binaryvalue;
			
			setostypevalue (getbinarytypeid (x), v);
			
			return (true);
			}
		
		case setbinarytypefunc: {
			hdlhashtable htable;
			tyvaluerecord val;
			OSType type;
			hdlhashnode hnode;
			
			if (!getvarvalue (hparam1, 1, &htable, bs, &val, &hnode))
				break;
			
			if (val.valuetype != binaryvaluetype) {
				
				langerror (binaryrequirederror);
				
				break;
				}
			
			flnextparamislast = true;
			
			if (!getostypevalue (hparam1, 2, &type))
				break;
			
			setbinarytypeid (val.data.binaryvalue, type);
			
			langsymbolchanged (htable, bs, hnode, true);
			
			(*v).data.flvalue = true;
			
			return (true);
			}
		
		case pointfunc:
			flnextparamislast = true;
			
			return (getpointparam (hparam1, 1, v));
		
		case setpointfunc: {
			Point pt;
			
			if (!getintvalue (hparam1, 1, &pt.h))
				break;
			
			flnextparamislast = true;
			
			if (!getintvalue (hparam1, 2, &pt.v))
				break;
			
			setpointvalue (pt, v);
			
			return (true);
			}
		
		case getpointfunc: {
			Point pt;
			
			if (!getpointvalue (hparam1, 1, &pt))
				break;
			
			if (!setintvarparam (hparam1, 2, pt.h))
				break;
			
			flnextparamislast = true;
			
			if (!setintvarparam (hparam1, 3, pt.v))
				break;
			
			(*v).data.flvalue = true;
			
			return (true);
			}
		
		case rectfunc:
			flnextparamislast = true;
			
			return (getrectparam (hparam1, 1, v));
		
		case setrectfunc: {
			Rect r;
			diskrect dr;
			
			if (!getintvalue (hparam1, 1, &dr.top))
				break;
			
			if (!getintvalue (hparam1, 2, &dr.left))
				break;
			
			if (!getintvalue (hparam1, 3, &dr.bottom))
				break;
			
			flnextparamislast = true;
			
			if (!getintvalue (hparam1, 4, &dr.right))
				break;

			/*can not use diskrecttorect because that swaps the byte order */
			/*		diskrecttorect (&dr, &r);		*/
			r.top = dr.top;
			r.bottom = dr.bottom;
			r.left = dr.left;
			r.right = dr.right;
			
			return (newheapvalue (&r, longsizeof (r), rectvaluetype, v));
			}
		
		case getrectfunc: {
			Rect r;
			
			if (!langcheckparamcount (hparam1, 5)) /*preflight*/
				break;
			
			if (!getrectvalue (hparam1, 1, &r))
				break;
			
			if (!setintvarparam (hparam1, 2, (short)r.top))
				break;
			
			if (!setintvarparam (hparam1, 3, (short)r.left))
				break;
			
			if (!setintvarparam (hparam1, 4, (short)r.bottom))
				break;
			
			if (!setintvarparam (hparam1, 5, (short)r.right))
				break;
			
			(*v).data.flvalue = true;
			
			return (true);
			}
		
		case rgbfunc:
			flnextparamislast = true;
			
			return (getrgbparam (hparam1, 1, v));
		
		case setrgbfunc: {
			RGBColor rgb;
			
			if (!getintvalue (hparam1, 1, (short *) &rgb.red))
				break;
			
			if (!getintvalue (hparam1, 2, (short *) &rgb.green))
				break;
			
			flnextparamislast = true;
			
			if (!getintvalue (hparam1, 3, (short *) &rgb.blue))
				break;
			
			return (newheapvalue (&rgb, longsizeof (rgb), rgbvaluetype, v));
			}
		
		case getrgbfunc: {
			RGBColor rgb;
			
			if (!langcheckparamcount (hparam1, 4)) /*preflight*/
				break;
			
			if (!getrgbvalue (hparam1, 1, &rgb))
				break;
			
			if (!setintvarparam (hparam1, 2, rgb.red))
				break;
			
			if (!setintvarparam (hparam1, 3, rgb.green))
				break;
			
			if (!setintvarparam (hparam1, 4, rgb.blue))
				break;
			
			(*v).data.flvalue = true;
			
			return (true);
			}
		
		case patternfunc:
			flnextparamislast = true;
			
			return (getpatternparam (hparam1, 1, v));
		
		case fixedfunc:
			flnextparamislast = true;
			
			return (getfixedparam (hparam1, 1, v));
		
		case singlefunc:
			flnextparamislast = true;
			
			return (getsingleparam (hparam1, 1, v));
		
		case doublefunc:
			flnextparamislast = true;
			
			return (getdoubleparam (hparam1, 1, v));
		
		case filespecfunc:
			flnextparamislast = true;
			
			return (getfilespecparam (hparam1, 1, v));
		
		case aliasfunc:
			flnextparamislast = true;
			
			return (getaliasparam (hparam1, 1, v));
		
		case listfunc:
			flnextparamislast = true;
			
			if (!getparamvalue (hparam1, 1, v))
				break;
			
			return (coercevalue (v, listvaluetype));
		
		case recordfunc:
			flnextparamislast = true;
			
			if (!getparamvalue (hparam1, 1, v))
				break;
			
			return (coercevalue (v, recordvaluetype));
		
		case enumfunc:
			flnextparamislast = true;
			
			if (!getparamvalue (hparam1, 1, v))
				break;
			
			return (coercevalue (v, enumvaluetype));
		
		case memavailfunc:
			if (!langcheckparamcount (hparam1, 0))
				break;
			
			return (setlongvalue (FreeMem (), v));
		
		case flushmemfunc: {
			long ctbytes = longinfinity;
			
			if (!langcheckparamcount (hparam1, 0))
				break;
			
			shellcallmemoryhooks (&ctbytes);
			
			#if MACVERSION && fldebug
			
				CompactMem (longinfinity); /*makes it easier to debug heap leakage*/
			
			#endif
			
			return (setbooleanvalue ((boolean)(ctbytes < longinfinity), v));
			}
		
		case randomfunc: {
			long lower, upper, n;
			
			if (!getlongvalue (hparam1, 1, &lower))
				break;
			
			flnextparamislast = true;
			
			if (!getlongvalue (hparam1, 2, &upper))
				break;
			
			if (lower > upper) {
				
				langerror (badrandomboundserror);
				
				break;
				}
			
			n = rand ();	// 3/10/97 dmb - was: Random ();
			
			n = lower + (abs (n) % (upper - lower + 1));
			
			return (setlongvalue (n, v));
			
			/*
			if (!langcheckparamcount (hparam1, 0))
				break;
			
			return (setlongvalue (Random (), v));
			*/
			}
		
		
		case mousebuttonfunc:
			if (!langcheckparamcount (hparam1, 0))
				break;
			
			return (setbooleanvalue (mousebuttondown (), v));
		
		case mouselocationfunc: {
			Point pt;
			WindowPtr w = getfrontwindow ();
			
			if (!langcheckparamcount (hparam1, 0))
				break;
			
			if (w == nil)
				pt.h = pt.v = 0;
			
			else {
				//Code change by Timothy Paustian Monday, August 21, 2000 4:29:53 PM
				//pushport must receive a CGrafPtr, no implicit converstion on OS X
				CGrafPtr	thePort;
				thePort = GetWindowPort(w);
				pushport (thePort);
				
				getmousepoint (&pt);
				
				popport ();
				}
			
			return (setpointvalue (pt, v));
			}
		
		
		case optionkeyfunc: case cmdkeyfunc: case shiftkeyfunc: case controlkeyfunc:
			if (!langcheckparamcount (hparam1, 0))
				break;
			
			return (setbooleanvalue (keyboardmodifierverb ((tylangtoken) token), v));
		
		/*	
		case presskeyfunc: {
			char ch;
			
			flnextparamislast = true;
			
			if (!getcharvalue (hparam1, 1, &ch))
				break;
			
			return (setbooleanvalue (presskeyverb (ch), v));
			}
		*/
		
		case absfunc: { /*absolute value*/
			tyvaluerecord val;
			tyvaluerecord zeroval;
			tyvaluerecord tempval;
			
			flnextparamislast = true;
			
			if (!getparamvalue (hparam1, 1, &val))
				break;
			
			setlongvalue (0, &zeroval); /*our comparison value*/
			
			if (!copyvaluerecord (val, &tempval)) /*work on a copy*/
				break;
			
			if (!LTvalue (tempval, zeroval, &tempval)) /*couldn't compare to zero*/
				break;
			
			if (tempval.data.flvalue) /*val was less than zero -- return negative*/
				return (unaryminusvalue (val, v));
			
			return (copyvaluerecord (val, v)); /*return original value*/
			}
		
		case evaluatefunc: {
			Handle htext;
			
			flnextparamislast = true;
			
			if (!getexempttextvalue (hparam1, 1, &htext))
				break;
			
			/*
			if (!getstringvalue (hparam1, 1, bs))
				break;
			
			if (!newtexthandle (bs, &htext))
				break;
			*/
			
			if (!langrun (htext, v))
				setbooleanvalue (false, v);
			
			return (true);
			}
		
		case evaluatethreadfunc: {
			Handle htext;
			
			flnextparamislast = true;
			
			if (!getexempttextvalue (hparam1, 1, &htext))
				break;
			
			if (!processruntext (htext))
				break;
			
			(*v).data.flvalue = true;
			
			return (true);
			}
		
		case alertdialogfunc:
			flnextparamislast = true;
			
			if (!getstringvalue (hparam1, 1, bs))
				break;
			
			(*v).data.flvalue = alertdialog (bs);
			
			return (true);
		
		case rundialogfunc:
			return (langrundialog (hparam1, v));
		
		
		case runmodelessfunc:
			return (langrunmodeless (hparam1, v));
		

	
		case getdialogvaluefunc:
			return (langgetdialogvalue (hparam1, v));
		
		case setdialogvaluefunc:
			return (langsetdialogvalue (hparam1, v));
		
		case setdialogitemenablefunc:
			return (langsetdialogitemenable (hparam1, v));
		
		case showdialogitemfunc:
			return (langsetdialogitemvis (hparam1, true, v));
		
		case hidedialogitemfunc:
			return (langsetdialogitemvis (hparam1, false, v));
	
		case twowaydialogfunc:
			return (twowayfunc (hparam1, v));		
		
		case threewaydialogfunc:
			return (threewayfunc (hparam1, v));
		
		case askdialogfunc:
			return (askfunc (hparam1, v, false));

		case askpassworddialogfunc:
			return (askfunc (hparam1, v, true));
		
		case getintdialogfunc:
			return (getintfunc (hparam1, v));

		case notifytdialogfunc:
			flnextparamislast = true;
			
			if (!getstringvalue (hparam1, 1, bs))
				break;
			
			(*v).data.flvalue = notifyuser (bs);
			
			return (true);

		case getuserinfodialogfunc:
			return (getuserinfofunc (hparam1, v));

		case soundfunc: {
			short duration, amplitude, frequency;
			
			if (!getintvalue (hparam1, 1, &duration))
				break;
				
			if (!getintvalue (hparam1, 2, &amplitude))
				break;
			
			flnextparamislast = true;
				
			if (!getintvalue (hparam1, 3, &frequency))
				break;
								
			dosound (duration, amplitude, frequency);
			
			(*v).data.flvalue = true;
			
			return (true);
			}
		
		case playsoundfunc:
			flnextparamislast = true;
			
			if (!getstringvalue (hparam1, 1, bs))
				break;
			
			return (setbooleanvalue (playnamedsound (bs), v));
		
		case sysbeepfunc:
			if (!langcheckparamcount (hparam1, 0))
				break;
			
			sysbeep ();
			
			(*v).data.flvalue = true;
			
			return (true);
		
		case tickcountfunc:
			if (!langcheckparamcount (hparam1, 0))
				break;
			
			return (setlongvalue (gettickcount (), v));
		
		case millisecondcountfunc: {  /* 2005-01-08 SMD */
			if (!langcheckparamcount (hparam1, 0))
				break;
			
			return (setlongvalue (getmilliseconds (), v));
			}
		
		case delayfunc: {
			long ctseconds;
			
			flnextparamislast = true;
			
			if (!getlongvalue (hparam1, 1, &ctseconds))
				break;
			
			delayseconds (ctseconds);
			
			(*v).data.flvalue = true;
			
			return (true);
			}
		
		case sleepfunc: {
			long ctseconds;
			
			if (processisoneshot (true) && !langdialogrunning ()) {
				
				langparamerror (foragentsonlyerror, bsfunctionname);
				
				break;
				}
			
			flnextparamislast = true;
			
			if (!getlongvalue (hparam1, 1, &ctseconds))
				break;
				
			(*v).data.flvalue = processagentsleep (ctseconds);
			
			return (true);
			}
					
		case delaysixtiethsfunc: {
			long ctsixtieths;
			
			flnextparamislast = true;
			
			if (!getlongvalue (hparam1, 1, &ctsixtieths))
				break;
			
			delayticks (ctsixtieths);
			
			(*v).data.flvalue = true;
			
			return (true);
			}
		
		case beachballfunc: {
			if (!langcheckparamcount (hparam1, 0))
				break;
			
			if (!beachballcursor ())
				initbeachball (right);
			
			rollbeachball ();
			
			(*v).data.flvalue = true;
			
			return (true);
			}
		
	#if isFrontier && MACVERSION
	
		case seteventtimeoutfunc:
			return (langipcsettimeout (hparam1, v));
		
		case seteventtransactionidfunc:
			return (langipcsettransactionid (hparam1, v));
		
		case seteventinteractionlevelfunc:
			return (langipcsetinteractionlevel (hparam1, v));
		
		case geteventattrfunc:
			return (langipcgeteventattr (hparam1, v));
		
		case coerceappleitemfunc:
			return (langipccoerceappleitem (hparam1, v));
		
		case putlistitemfunc:
			return (langipcputlistitem (hparam1, v));
		
		case getlistitemfunc:
			return (langipcgetlistitem (hparam1, v));
		
		case countlistitemsfunc:
			return (langipccountlistitems (hparam1, v));
	
		/*
		case ddeinitfunc:
			return (langipcddeinit (hparam1, v));
		*/
		
		case systemeventfunc:
			return (langipcmessage (hparam1, systemmsg, v));
		
		case microsofteventfunc:
			return (langipcmessage (hparam1, noreplymsg + transactionmsg + microsoftmsg, v));
		
		case transactioneventfunc:
			return (langipcmessage (hparam1, transactionmsg, v));
	
	#endif
		
		case timecreatedfunc: {
			long timecreated, timemodified;
			
			if (!gettimesverb (hparam1, &timecreated, &timemodified))
				return (setbooleanvalue (false, v));
			
			return (setdatevalue (timecreated, v));
			}
		
		case timemodifiedfunc: {
			long timecreated, timemodified;
			
			if (!gettimesverb (hparam1, &timecreated, &timemodified))
				return (setbooleanvalue (false, v));
			
			return (setdatevalue (timemodified, v));
			}
		
		case settimecreatedfunc:
			return (settimesverb ((tylangtoken) settimecreatedfunc, hparam1, v));
		
		case settimemodifiedfunc:
			return (settimesverb ((tylangtoken) settimemodifiedfunc, hparam1, v));
		
		case msgfunc:
			return ((*langcallbacks.msgverbcallback) (hparam1, v));
		

		case callscriptfunc:
			return (callscriptverb (hparam1, v));

		case dllcallfunc:
		case calldllfunc:		/* this is remaining for historical usage per Dave. rab: 5.0b4 1/6/98 */
			return (dllcallverb (hparam1, v));

		case dllloadfunc:
			return (dllloadverb (hparam1, v));
		
		case dllunloadfunc:
			return (dllunloadverb (hparam1, v));
		
		case dllisloadedfunc:
			return (dllisloadedverb (hparam1, v));

		case packwindowfunc:
			return (langpackwindowverb (hparam1, v));
		
		case unpackwindowfunc:
			return (langunpackwindowverb (hparam1, v));
		
		case getbitfunc:
			return (bitgetverb (hparam1, v));
		
		case setbitfunc:
			return (bitsetverb (hparam1, v));
	
		case clearbitfunc:
			return (bitclearverb (hparam1, v));
		
		case bitandfunc:
			return (bitandverb (hparam1, v));

		case bitorfunc:
			return (bitorverb (hparam1, v));

		case bitxorfunc:
			return (bitxorverb (hparam1, v));

		case bitshiftleftfunc:
			return (bitshiftleftverb (hparam1, v));

		case bitshiftrightfunc:
			return (bitshiftrightverb (hparam1, v));

		case lockfunc:
			return (locksemaphoreverb (hparam1, v));
		
		case unlockfunc:
			return (unlocksemaphoreverb (hparam1, v));
		
		case base64encodefunc:
			return (base64encodeverb (hparam1, v));
		
		case base64decodefunc:
			return (base64decodeverb (hparam1, v));


/* This is the only place where these verbs are ifdef'd out for the Macintosh.  
   This is done so that an app written to these verbs will have a graceful unimplemented
   error message on the mac. */

//#ifdef WIN95VERSION
		case netaddressdecode:
			{
			long addr;

			flnextparamislast = true;

			if (!getlongvalue (hparam1, 1, &addr))
				return (false);

			if (fwsNetEventAddressDecode (addr, bs))
				return (setstringvalue (bs, v));

			return (false);
			}

		case netaddressencode:
			{
			long addr;

			flnextparamislast = true;

			if (!getstringvalue (hparam1, 1, bs))
				return (false);

			if (fwsNetEventAddressEncode(bs, (unsigned long *)(&addr)))
				return (setlongvalue (addr, v));

			return (false);
			}

		case netaddresstoname:
			{
			long addr;

			flnextparamislast = true;

			if (!getlongvalue (hparam1, 1, &addr))
				return (false);

			if (fwsNetEventAddressToName (addr, bs))
				return (setstringvalue (bs, v));

			return (false);
			}

		case netnametoaddress:
			{
			long addr;

			flnextparamislast = true;

			if (!getstringvalue (hparam1, 1, bs))
				return (false);

			if (fwsNetEventNameToAddress(bs, (unsigned long *)(&addr)))
				return (setlongvalue (addr, v));

			return (false);
			}

		case netmyaddress:
			{
			long addr;

			if (fwsNetEventMyAddress((unsigned long *)(&addr)))
				return (setlongvalue (addr, v));

			return (false);
			}

		case netabortstream:
			{
			long stream;

			flnextparamislast = true;

			if (!getlongvalue (hparam1, 1, &stream))
				return (false);

			if (fwsNetEventAbortStream (stream))
				return (setbooleanvalue (true, v));

			return (false);
			}

		case netclosestream:
			{
			long stream;

			flnextparamislast = true;

			if (!getlongvalue (hparam1, 1, &stream))
				return (false);

			if (fwsNetEventCloseStream (stream))
				return (setbooleanvalue (true, v));

			return (false);
			}

		case netcloselisten:
			{
			long stream;

			flnextparamislast = true;

			if (!getlongvalue (hparam1, 1, &stream))
				return (false);

			if (fwsNetEventCloseListen (stream))
				return (setbooleanvalue (true, v));

			return (false);
			}

		case netopenaddrstream:
			{
			long stream;
			long port;
			long addr;

			if (!getlongvalue (hparam1, 1, &addr))
				return (false);

			flnextparamislast = true;

			if (!getlongvalue (hparam1, 2, &port))
				return (false);

			if (fwsNetEventOpenAddrStream (addr, port, (unsigned long *)(&stream)))
				return (setlongvalue (stream, v));

			return (false);
			}

		case netopennamestream:
			{
			long stream;
			long port;

			if (!getstringvalue (hparam1, 1, bs))
				return (false);

			flnextparamislast = true;

			if (!getlongvalue (hparam1, 2, &port))
				return (false);

			if (fwsNetEventOpenNameStream (bs, port, (unsigned long *)(&stream)))
				return (setlongvalue (stream, v));

			return (false);
			}

		case netreadstream:
			{
			long stream;
			long len;
			char * charbuffer;
			Handle buf;

			if (!getlongvalue (hparam1, 1, &stream))
				return (false);

			flnextparamislast = true;

			if (!getlongvalue (hparam1, 2, &len))
				return (false);

			if (!newhandle (len, &buf))
				return (false);

			lockhandle (buf);

			charbuffer = *buf;

			if (fwsNetEventReadStream (stream, (unsigned long *)(&len), charbuffer)) {
				unlockhandle (buf);
				SetHandleSize (buf, len);
				return (setbinaryvalue (buf, '\?\?\?\?', v));
				}

			unlockhandle (buf);		/*cleanup*/
			disposehandle (buf);
			return (false);
			}

		case netwritestream:
			{
			long stream;
			long len;
			char * charbuffer;
			Handle buf;

			if (!getlongvalue (hparam1, 1, &stream))
				return (false);

			flnextparamislast = true;
	
			if (!gettextvalue (hparam1, 2, &buf))
				return (false);

			len = gethandlesize (buf);

			lockhandle (buf);

			charbuffer = *buf;

			if (fwsNetEventWriteStream (stream, len, charbuffer)) {
				unlockhandle (buf);
				return (setbooleanvalue (true, v));
				}

			unlockhandle (buf);
			return (false);
			}

		case netlistenstream:
			{
			long stream;
			long port;
			long refcon;
			long depth;
			unsigned long ipaddr;
			tyvaluerecord callbackval;
			hdlhashtable ht;	
			bigstring bsFullName;

			if (!getlongvalue (hparam1, 1, &port))
				return (false);

			if (!getlongvalue (hparam1, 2, &depth))
				return (false);

			if (!getaddressparam (hparam1, 3, &callbackval))
				return (false);
			
			if (!getaddressvalue (callbackval, &ht, bs))
				return (false);

			if (! langexternalgetquotedpath (ht, bs, bsFullName))
				return (false);

			if (!getlongvalue (hparam1, 4, &refcon))
				return (false);

			flnextparamislast = true;
	
			if (!getlongvalue (hparam1, 5, (long *)(&ipaddr)))
				return (false);
			
			if (fwsNetEventListenStream (port, depth, bsFullName, refcon, (unsigned long *)(&stream), ipaddr, (long)(**((hdlexternalvariable)(**ht).hashtablerefcon)).hdatabase))
				return (setlongvalue (stream, v));

			return (false);
			}


		case netstatusstream:
			{
			long stream;
			unsigned long bytesPending;

			if (!langcheckparamcount (hparam1, 2))
				return (false);

			if (!getlongvalue (hparam1, 1, &stream))
				return (false);

			if (fwsNetEventStatusStream (stream, bs, &bytesPending)) {
				langsetlongvarparam (hparam1, 2, bytesPending);
				return (setstringvalue (bs, v));
				}

			return (false);
			}

		case netgetpeeraddress: {
			long stream;
			unsigned long adr, port;

			flnextparamislast = true;

			if (!getlongvalue (hparam1, 1, &stream))
				return (false);

			if (fwsNetEventGetPeerAddress (stream, &adr, &port))
				return (setlongvalue (adr, v));

			return (false);
			}

		case netgetpeerport: {
			long stream;
			unsigned long adr, port;

			flnextparamislast = true;

			if (!getlongvalue (hparam1, 1, &stream))
				return (false);

			if (fwsNetEventGetPeerAddress (stream, &adr, &port))
				return (setlongvalue (port, v));

			return (false);
			}

		case netwritestringtostream: {
			long stream, chunksize, timeout;
			Handle htext;

			if (!getlongvalue (hparam1, 1, &stream))
				return (false);

			if (!getreadonlytextvalue (hparam1, 2, &htext))
				return (false);

			if (!getlongvalue (hparam1, 3, &chunksize))
				return (false);
			
			flnextparamislast = true;

			if (!getlongvalue (hparam1, 4, &timeout))
				return (false);

			if (!fwsNetEventWriteHandleToStream (stream, htext, chunksize, timeout))
				return (false);

			return (setbooleanvalue (true, v));
			}
		
		case netwritefiletostream: {
			long stream;
			tyvaluerecord vprefix;
			tyvaluerecord vsuffix;
			short ctconsumed = 2;
			short ctpositional = 2;
			tyfilespec fs;

			if (!getlongvalue (hparam1, 1, &stream))
				return (false);
				
			if (!getfilespecvalue (hparam1, 2, &fs))
				return (false);
				
			initvalue (&vprefix, stringvaluetype);

			if (!getoptionalparamvalue (hparam1, &ctconsumed, &ctpositional, BIGSTRING ("\x06" "prefix"), &vprefix)) 
				return (false);
			
			initvalue (&vsuffix, stringvaluetype);

			flnextparamislast = true;

			if (!getoptionalparamvalue (hparam1, &ctconsumed, &ctpositional, BIGSTRING ("\x06" "suffix"), &vsuffix)) 
				return (false);

			if (!fwsNetEventWriteFileToStream (stream, vprefix.data.stringvalue, vsuffix.data.stringvalue, &fs))
				return (false);

			return (setbooleanvalue (true, v));
			}


		case netreadstreamuntil: {
			long stream, timeout;
			Handle hpattern;
			tyvaluerecord vadrbuffer, vbuffer;
			tyaddress adrbuffer;
			hdlhashnode hnode;
			
			if (!getlongvalue (hparam1, 1, &stream))
				return (false);

			if (!getreadonlytextvalue (hparam1, 2, &hpattern))
				return (false);

			if (!getlongvalue (hparam1, 3, &timeout))
				return (false);
			
			flnextparamislast = true;

			if (!getaddressparam (hparam1, 4, &vadrbuffer))
				return (false);

			if (!getaddressvalue (vadrbuffer, &adrbuffer.ht, adrbuffer.bs))
				return (false);

			if (!langhashtablelookup (adrbuffer.ht, adrbuffer.bs, &vbuffer, &hnode))
				return (false);

			if (!coercetostring (&vbuffer))
				return (false);

			if (!fwsNetEventReadStreamUntil (stream, vbuffer.data.stringvalue, hpattern, timeout))
				return (false);

			// new hdlhashnode parameter set to nil, should be OK because we only care if it's externl
			langsymbolchanged (adrbuffer.ht, adrbuffer.bs, nil, true);

			return (setbooleanvalue (true, v));
			}

		case netreadstreambytes: {
			long stream, ctbytes, timeout;
			tyvaluerecord vadrbuffer, vbuffer;
			tyaddress adrbuffer;
			hdlhashnode hnode;

			if (!getlongvalue (hparam1, 1, &stream))
				return (false);

			if (!getlongvalue (hparam1, 2, &ctbytes))
				return (false);
			

			if (!getlongvalue (hparam1, 3, &timeout))
				return (false);
			
			flnextparamislast = true;

			if (!getaddressparam (hparam1, 4, &vadrbuffer))
				return (false);

			if (!getaddressvalue (vadrbuffer, &adrbuffer.ht, adrbuffer.bs))
				return (false);

			if (!langhashtablelookup (adrbuffer.ht, adrbuffer.bs, &vbuffer, &hnode))
				return (false);

			if (!coercetostring (&vbuffer))
				return (false);

			if (!fwsNetEventReadStreamBytes (stream, vbuffer.data.stringvalue, ctbytes, timeout))
				return (false);

			langsymbolchanged (adrbuffer.ht, adrbuffer.bs, nil, true);

			return (setbooleanvalue (true, v));
			}

		case netreadstreamuntilclosed: {
			long stream, timeout;
			tyvaluerecord vadrbuffer, vbuffer;
			tyaddress adrbuffer;
			hdlhashnode hnode;
			
			if (!getlongvalue (hparam1, 1, &stream))
				return (false);

			if (!getlongvalue (hparam1, 2, &timeout))
				return (false);
			
			flnextparamislast = true;

			if (!getaddressparam (hparam1, 3, &vadrbuffer))
				return (false);

			if (!getaddressvalue (vadrbuffer, &adrbuffer.ht, adrbuffer.bs))
				return (false);

			if (!langhashtablelookup (adrbuffer.ht, adrbuffer.bs, &vbuffer, &hnode))
				return (false);

			if (!coercetostring (&vbuffer))
				return (false);

			if (!fwsNetEventReadStreamUntilClosed (stream, vbuffer.data.stringvalue, timeout))
				return (false);

			langsymbolchanged (adrbuffer.ht, adrbuffer.bs, nil, true);

			return (setbooleanvalue (true, v));
			}
		
		case netstatus: {
		
				langerror (unimplementedverberror);

				return (false);
			}

		case netcountconnections: {

			/*7.0b37 PBS: return the current TCP connections count.*/

			return (setlongvalue (fwsNetEventGetConnectionCount (), v));
			}

		//case pythondoscriptfunc:
		//	return (langrunpythonscript (hparam1, v));


		/* These functions are  for Windows - the code to ifdef 
		    is in htmlcontrol.c  8/26/00 by RAB */
		
		
//#endif

		default:
			getstringlist (langerrorlist, unimplementedverberror, bserror);
			
			break;
		} /*switch*/
	
	return (false);
	} /*langfunctionvalue*/


/*
DW 6/18/19

obsolete verbs that we won't have glue for, won't document
	
	checksyntax
	closeprogressdialog
	continueprogressdialog
	getfiletext
	openpalette
	pattern
	runprogram
	runstring
	startprogressdialog
	syntaxtrace
	username

remaining very basic verbs, implemented in lang.c

	appleevent
	complexevent
	defined
	sizeof
	typeof
	pack
	unpack
	EFP
	finderevent
	gestalt
	myMoof
	objspec						   
	syscrash
	tmp
	setobj

*/



boolean langinitbuiltins (void) {
	
	/*
	9/15/92 dmb: initialize randSeed with current date/time so we don't get 
	the same sequence each time
	
	2.1b5 dmb: use new loadfunctionprocessor for resource-based initialization
	*/

	srand (timenow ());	 // 3/10/97 dmb - was: qd.randSeed = timenow ();

	return (loadfunctionprocessor (idlangverbs, &langfunctionvalue));
	} /*langinitbuiltins*/




