
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
#include "cursor.h"
#include "dialogs.h"
#include "kb.h"
#include "mouse.h"
#include "frontierconfig.h"
#include "ops.h"
#include "scrap.h"
#include "strings.h"
#include "timedate.h"
#include "frontierwindows.h"
#include "shell.h"
#include "lang.h"
#include "langinternal.h"
#include "process.h"



typedef struct tydialogrecord {
	
	boolean flactive: 1; /*determines whether items are drawn as active or inactive*/
	
	boolean flwindowopen: 1;
	
	boolean flstillalive: 1;
	
	short dialogid;
	
	short defaultitem;
	
	short itemhit;
	
	hdltreenode itemhitroutine; /*itemhit handler script*/
	
	DialogPtr pdialog;
	
	hdlprocessrecord hprocess;
	
	long dialogrefcon; /*optional data for the next layer up*/
	} tydialogrecord, *ptrdialogrecord, **hdldialogrecord;
	


WindowPtr langdialogwindow = nil;

hdlwindowinfo langdialogwindowinfo = nil;

hdldialogrecord langdialogdata = nil;




static boolean langdialogselectall (void) {
	
	
	return (dialogselectall ((DialogPtr) langdialogwindow));
	} /*langdialogselectall*/


static boolean langdialogsearch (void) {
	
	/*
	we don't want to handle search commands, but don't want them to fail
	either.  instead, we'll find the window behind us and let it handle it
	*/
	
	WindowPtr nextwindow;
	boolean fl;
	
	nextwindow = shellnextwindow (langdialogwindow);
	
	if (nextwindow == nil)
		return (false);
	
	shellpushglobals (nextwindow);
	
	fl = (*shellglobals.searchroutine) ();
	
	shellpopglobals ();
	
	return (fl);
	} /*langdialogsearch*/


static boolean langdialoggettargetdata (short id) {
	
	/*
	2.1b3 dmb: use new hprocess field to allow modeless dialog to be the 
	target of another process
	*/
	
	register hdldialogrecord hd = langdialogdata;
	
	if (currentprocess == (**hd).hprocess) /*don't want our script to operate on our window*/
		return (false);
	
	return (id == -1); /*true if target type is generic -- a shell verb*/
	
	} /*langdialoggettargetdata*/


static boolean langdialogmousedown (Point pt, tyclickflags flags){
	
	register hdldialogrecord hd = langdialogdata;
	short item;
	
	#if TARGET_API_MAC_CARBON
	DialogPtr	theDial = GetDialogFromWindow(langdialogwindow);
	if(dialogevent (&shellevent, theDial, &item))
	#else
	if (dialogevent (&shellevent, langdialogwindow, &item))
	#endif
	{ /*mouse in text item*/
		
		(**hd).itemhit = item;
		
		shellevent.when = gettickcount ();
		
		mouseup (shellevent.when, shellevent.where.h, shellevent.where.v, shellevent.what); /*textedit may swallow the mouseup event when tracking click*/
		}
	
	return (true);
	} /*langdialogmousedown*/


static boolean langdialogkeystroke (void) {
	
	register hdldialogrecord hd = langdialogdata;
	short item;
	#if TARGET_API_MAC_CARBON
	DialogPtr theDial = GetDialogFromWindow(langdialogwindow);
	if (dialogevent (&shellevent, theDial, &item)) 
	#else
	if (dialogevent (&shellevent, langdialogwindow, &item))
	#endif
	{
		(**hd).itemhit = item;
	}
	
	return (true);
	} /*langdialogkeystroke*/


static boolean langdialogitemhit (hdltreenode, short, boolean); /*forward*/


static boolean langdialogclose (void) {
	
	/*
	2.1b3 dmb: special case for no threads
	
	5.0.2b13 dmb: pay attention to processyield return
	
	7.0b36 PBS: Carbon change. The dialog will be disposed while yielding during the while loop.
	So the reference to (**hd).flstillalive at the top of the loop causes a crash, as hd is no longer valid.
	The work-around is to check (in the Carbon version) that (*hd) != -1.
	Though this works, it's not very nice. A better answer requires more serious re-architecting.
	*/
	
	register hdldialogrecord hd = langdialogdata;
	
	(**hd).flwindowopen = false;
	
	if (flcanusethreads) {
		
		(**hd).itemhit = -2; /*close*/
		
		#if TARGET_API_MAC_CARBON

			while (((char)((long)*hd) != -1) && ((**hd).flstillalive)) { /*make sure it's handled before window is disposed*/
				
				processchecktimeouts ();
				
				if (!processyield ())
					return (false);
				}
			
		#else
		
			while ((**hd).flstillalive) { /*make sure it's handled before window is disposed*/
				
				processchecktimeouts ();
				
				if (!processyield ())
					return (false);
				}
			
		#endif
		
		}
	else {
		langdialogitemhit ((**hd).itemhitroutine, -2, false);
		}
	
	return (true);
	} /*langdialogclose*/


static void langdialogupdate (void) {
	
	/*
	2.1b5 dmb: make sure the event we pass is an update event; shellevent 
	may be an activate or os event
	*/
	
	EventRecord event = shellevent;
	
	event.what = updateEvt;
	#if TARGET_API_MAC_CARBON
	{
	DialogPtr	theDial = GetDialogFromWindow(langdialogwindow);
	dialogupdate (&event, theDial);
	}
	#else
	dialogupdate (&event, (DialogPtr) langdialogwindow);
	#endif
	} /*langdialogupdate*/


static void langdialogactivate (boolean flactivate) {
	
	(**langdialogdata).flactive = flactivate;
	#if TARGET_API_MAC_CARBON
	{
	DialogPtr	theDial = GetDialogFromWindow(langdialogwindow);
	dialogactivate (theDial, flactivate);
	}
	#else
	dialogactivate (langdialogwindow, flactivate);
	#endif
	} /*langdialogactivate*/


static boolean langdialogadjustcursor (Point pt){
		
	setcursortype (cursorisarrow);
	
	return (true);
	} /*langdialogadjustcursor*/
	

/*
static boolean langdialogsetfontsize (void) {
	
	register hdlwindowinfo hw = langdialogwindowinfo;
	
	dialogsetfontsize (langdialogwindow, (**hw).defaultfont, (**hw).defaultsize);
	
	return (true);
	} /%langdialogsetfontsize%/


static boolean langdialogsetfont (void) {
	
	shelldefaultfontroutine (); /%set windowinfo record defaultfont field%/
	
	return (langdialogsetfontsize ());
	} /%langdialogsetfont%/
	

static boolean langdialogsetsize (void) {
	
	shelldefaultsizeroutine (); /%set windowinfo record defaultfont field%/
	
	return (langdialogsetfontsize ());
	} /%langdialogsetsize%/
*/


static boolean langdialogsetselectioninfo (void) {
	
	clearbytes (&(**langdialogwindowinfo).selectioninfo, longsizeof (tyselectioninfo)); /*can't set font/size/style*/
	
	return (true);
	} /*langdialogsetselectioninfo*/


static void langdialogidle (void) {
	
	#if TARGET_API_MAC_CARBON
	{
	DialogPtr	theDial = GetDialogFromWindow(langdialogwindow);
	dialogidle (theDial);
	}
	#else
	dialogidle (langdialogwindow);
	#endif
	} /*langdialogidle*/


static boolean langdialogcopy (void) {
	
	Handle htext;
	
	#if TARGET_API_MAC_CARBON
	DialogPtr	theDial = GetDialogFromWindow(langdialogwindow);
	DialogCopy (theDial);
	#else
	DialogCopy (langdialogwindow);
	#endif
	
	if (!copyhandle (TEScrapHandle (), &htext))
		return (false);
	
	return (shellsetscrap (htext, textscraptype, (shelldisposescrapcallback) &disposehandle, nil));
	} /*langdialogcopy*/


static boolean langdialogpaste (void) {
	
	Handle htext;
	boolean fltempscrap;
	
	if (!shellconvertscrap (textscraptype, &htext, &fltempscrap))
		return (false);
	
	copyhandlecontents (htext, TEScrapHandle ());
	
	TESetScrapLength (gethandlesize (htext));
	
	if (fltempscrap)
		DisposeHandle (htext);
	
	shellwritescrap (textscraptype); // 5.1.6: Mac OS 8.5 fix?
	
	#if TARGET_API_MAC_CARBON
	{
	DialogPtr	theDial = GetDialogFromWindow(langdialogwindow);
	DialogPaste (theDial);
	}
	#else
	DialogPaste (langdialogwindow);
	#endif
	return (true);
	} /*langdialogpaste*/


static boolean langdialogclear (void) {
	
	#if TARGET_API_MAC_CARBON
	DialogPtr	theDial = GetDialogFromWindow(langdialogwindow);
	DialogDelete (theDial);
	#else
	DialogDelete (langdialogwindow);
	#endif
	return (true);
	} /*langdialogclear*/


static boolean langdialogcut (void) {
	
	if (!langdialogcopy ())
		return (false);
	
	return (langdialogclear ());
	} /*langdialogcut*/


static langtreenodecallback savedebugger;


static boolean langdialogdebugger (hdltreenode hnode) {
	
	/*
	show the line that's about to be executed.  then restore the 
	debugger callback so we don't get called again.
	
	return false so that the routine stops running
	
	5/20/92 dmb: debuggercallback now gets hdltreenode parameter
	*/
	
	register hdlerrorstack hs = langcallbacks.scripterrorstack;
	tyerrorrecord errorrecord = (**hs).stack [(**hs).toperror - 1];
	
	(*errorrecord.errorcallback) (errorrecord.errorrefcon, (**hnode).lnum, -1, nil, nil);
	
	langcallbacks.debuggercallback = savedebugger;
	
	return (false);
	} /*langdialogdebugger*/


static boolean langdialogitemhit (hdltreenode htree, short itemnumber, boolean flzoomscript) {
	
	/*
	we're going to call the handler code with itemnumber as a parameter.
	
	we'll form the code tree for the parameter list (itemnumber) by hand.
	*/
	
	hdltreenode hparam;
	tyvaluerecord val;
	register boolean fl;
	DialogPtr savedialogwindow;
	
	cleartmpstack ();
	
	setintvalue (itemnumber, &val);
	
	if (!newconstnode (val, &hparam))
		return (false);
	
	savedialogwindow = langmodaldialog; /*save in local stack*/
	
	if (flzoomscript) {
		
		savedebugger = langcallbacks.debuggercallback;
		
		langcallbacks.debuggercallback = &langdialogdebugger;
		}
	#if TARGET_API_MAC_CARBON
	langmodaldialog = GetDialogFromWindow(langdialogwindow); /*make current for langdialog verbs*/
	#else
	langmodaldialog = langdialogwindow; /*make current for langdialog verbs*/
	#endif
	fl = langhandlercall (htree, hparam, &val);
	
	langmodaldialog = savedialogwindow; /*restore*/
	
	if (flzoomscript) { /*make sure it's restored*/
		
		langcallbacks.debuggercallback = savedebugger;
		
		return (true);
		}
	
	langdisposetree (hparam);
	
	if (!fl)
		return (false);
	
	if (!coercetoboolean (&val))
		return (false);
	
	return (val.data.flvalue);
	} /*langdialogitemhit*/


static void langdialogdispose (hdldialogrecord hdialog) {
	
	register hdldialogrecord hd = hdialog;
	
	if ((**hd).flwindowopen)
	{	
		#if TARGET_API_MAC_CARBON
		shellclosewindow (GetDialogWindow((**hd).pdialog));
		#else
		shellclosewindow ((**hd).pdialog);
		#endif
	}
	disposehandle ((Handle) hdialog);
	} /*langdialogdispose*/


typedef struct finddialoginfo {

	short idtofind;

	hdldialogrecord hdialogfound;
	} tyfinddialoginfo, *ptrfinddialoginfo;


static boolean finddialogvisit (WindowPtr w, ptrvoid refcon) {
	
	ptrfinddialoginfo findinfo = (ptrfinddialoginfo) refcon;
	hdlwindowinfo hinfo;
	hdldialogrecord hd;
	
	getwindowinfo (w, &hinfo);
	
	hd = (hdldialogrecord) (**hinfo).hdata;
	
	if (hd == nil) /*paranoia -- if it's nil, keep visiting*/
		return (true);
	
	if ((**hd).dialogid != (*findinfo).idtofind) /*it's not the right one*/
		return (true);
	
	(*findinfo).hdialogfound = hd;
	
	return (false);
	} /*finddialogvisit*/


static boolean langdialogfindwindow (short dialogid, hdldialogrecord *hdialog) {
	
	tyfinddialoginfo findinfo;
	
	findinfo.idtofind = dialogid;
	
	if (shellvisittypedwindows (idlangdialogconfig, &finddialogvisit, &findinfo)) /*didn't find it*/
		return (false);
	
	*hdialog = findinfo.hdialogfound;
	
	return (true);
	} /*langdialogfindwindow*/


static boolean langdialognewwindow (short dialogid, short defaultitem, hdltreenode htree, hdldialogrecord *hdialog) {
	
	/*
	2.1b3 dmb: set the hprocess new field to the current process
	
	5.0.2b10 dmb: for 5.0, we need to pass in the window title
	*/
	
	register hdldialogrecord hd;
	Rect rzoom, rwindow;
	hdlwindowinfo hparent;
	tyconfigrecord config;
	DialogTHndl hdlog;
	bigstring bstitle;
	
	if (!shellgetfrontrootinfo (&hparent)) /*our parent is the frontmost root window*/
		return (false);
	
	if (!newclearhandle (longsizeof (tydialogrecord), (Handle *) hdialog))
		return (false);
	
	hd = langdialogdata = *hdialog; /*copy into register*/
	
	(**hd).dialogid = dialogid;
	
	(**hd).defaultitem = defaultitem;
	
	(**hd).itemhitroutine = htree;
	
	(**hd).hprocess = currentprocess; /*2.1b3*/
	
	rzoom.top = -1; /*zoom from default rect*/
	
	rwindow.top = -1; /*by default, use default*/
	
	hdlog = (DialogTHndl) GetResource ('DLOG', dialogid);
	
	if (hdlog != nil) {
		
		copystring ((**hdlog).title, bstitle);
		
		rwindow = (**hdlog).boundsRect;
		}
	
	shellgetconfig (idlangdialogconfig, &config);
	
	config.templateresnum = dialogid;
	
	shellsetconfig (idlangdialogconfig, config);
	
	if (!newchildwindow (idlangdialogconfig, hparent, &rwindow, &rzoom, bstitle, &langdialogwindow)) {
		
		disposehandle ((Handle) hd);
		
		return (false);
		}
	{
	#if TARGET_API_MAC_CARBON == 1
	DialogPtr	theDial = GetDialogFromWindow(langdialogwindow);
	setdefaultitem (theDial, defaultitem);
	(**hd).pdialog = theDial;
	#else
	setdefaultitem(langdialogwindow, defaultitem);
	(**hd).pdialog = langdialogwindow;
	#endif
		
	}
	(**hd).flwindowopen = true;
	
	getwindowinfo (langdialogwindow, &langdialogwindowinfo);
	
	(**langdialogwindowinfo).hdata = (Handle) hd;
	
	return (true);
	} /*langdialognewwindow*/


static boolean langdialogsleepexpired (void) {
	
	/*
	9/11/92 dmb: late-breaking feature: support clock.sleepFor in modeless 
	dialogs. unlike agents, dialogs sleep forever by default. but if they 
	establish a sleep period, they'll get send a itemhit (-3) if it expires 
	before any other itemhit
	*/
	
	unsigned long x = (**currentprocess).sleepuntil;
	
	if (x == 0)
		return (false);
	
	return (timenow () >= x);
	} /*langdialogsleepexpired*/


boolean langrunmodeless (hdltreenode hparam1, tyvaluerecord *vreturned) {
	
	/*
	run a modeless dialog.  we're expecting to be called with three arguments:
		
		rundialog (dialogid, defaultitem, itemhitcallback)
	
	after extracting these value, we construct a code tree for:
		
		itemhitcallback^
		
	and place the result in the langdialog record.  langdialogitemhit will 
	construct a parameter list for each call, and call back to the item 
	hit routine.
	
	11/5/91 dmb: added call to languserescaped
	
	12/4/91 dmb: call processnotbusy before loop, not in it.
	
	2.1b3 dmb: special case for no threads
	*/
	
	short dialogid;
	short defaultitem;
	tyvaluerecord val;
	hdltreenode htree;
	hdldialogrecord hdialog;
	register hdldialogrecord hd;
	register short itemhit;
	register boolean fl;
	boolean flzoomscript;
	
	if (!getintvalue (hparam1, 1, &dialogid))
		return (false);
	
	if (!getintvalue (hparam1, 2, &defaultitem))
		return (false);
	
	flnextparamislast = true;
	
	if (!getaddressparam (hparam1, 3, &val))
		return (false);
	
	if (langdialogfindwindow (dialogid, &hdialog)) { /*already running -- just bring to front*/
		
		#if TARGET_API_MAC_CARBON
		windowbringtofront (GetDialogWindow((**hdialog).pdialog)); /***should use windowinfo*/
		#else
		windowbringtofront ((**hdialog).pdialog); /***should use windowinfo*/
		#endif
		setbooleanvalue (true, vreturned);
		
		return (true);
		}
	
	if (!pushfunctionreference (val, &htree))
		return (false);
	
	if (!langdialognewwindow (dialogid, defaultitem, htree, &hdialog)) {
		
		langlongparamerror (cantloaddialogerror, dialogid);
		
		langdisposetree (htree);
		
		return (false);
		}
	
	hd = hdialog; /*copy into register*/
	
	if (!langdialogitemhit (htree, -1, false)) /*initialization failed*/
		goto exit;
	
	#if TARGET_API_MAC_CARBON
	shellpushglobals (GetDialogWindow((**hd).pdialog));
   	dialogselectall(GetDialogFromWindow(shellwindow));
	#else
	shellpushglobals ((**hd).pdialog);
	dialogselectall (shellwindow); /*in case there's a text item*/
	#endif
	
	windowzoom (shellwindow);
	
	shellbringtofront (shellwindowinfo); /*in case we're running under the debugger*/
	
	shellpopglobals ();
	
	(**hd).flstillalive = true;
	
	processnotbusy (); /*un-dim menus*/
	
	while (true) {
		
		if (flcanusethreads) {
			
			if (!processsleep (nil, 10)) {
				
				fllangerror = true; /*make sure we return false*/
				
				break;
				}
			}
		else {
			
			shellshorteventloop ();
			
			if (!(**hd).flwindowopen)
				break;
			}
		
		if (languserescaped (true)) /*script was killed by user*/
			break;
		
		itemhit = (**hd).itemhit;
		
		if (itemhit == 0) {
			
			if (langdialogsleepexpired ())
				itemhit = -3;
			}
		
		if (itemhit != 0) {
			
			flzoomscript = (itemhit > 0) && keyboardstatus.floptionkey;
			
			#if TARGET_API_MAC_CARBON == 1
			
				flzoomscript = false; 	/*7.1b37 PBS: wiring off a feature that: 1. no one knows about and 2. crashes anyway.*/
										/*Attempting to support this feature also causes crashes.*/
										/*That's why it's gone.*/
			
			#endif
			
			(**hd).itemhit = 0; /*clear for next time*/
			
			(**currentprocess).sleepuntil = 0; /*script must reestablish sleep each time*/
			
			if (dialogitemisbutton ((**hd).pdialog, itemhit))
				dialogselectall ((**hd).pdialog);
			
			fl = langdialogitemhit (htree, itemhit, flzoomscript);
			
			if (itemhit == -2) { /*window was closed*/
				
				(**hd).flstillalive = false;
				
				processyield (); /*let langdialogclose finish executing*/
				
				break;
				}
			
			if (!fl) /*itemhit routine returned false*/
				break;
			
			if (!flzoomscript) {
				
				if (debuggingcurrentprocess ())
				{
					#if TARGET_API_MAC_CARBON
					windowbringtofront (GetDialogWindow((**hd).pdialog)); /***should use windowinfo*/
					#else
					windowbringtofront ((**hd).pdialog); /***should use windowinfo*/
					#endif
				}
				}
			}
		}
	
	exit:
	
	(**hd).flstillalive = false;
	
	langdialogdispose (hd);
	
	langdisposetree (htree);
	
	setbooleanvalue (true, vreturned);
	
	return (!fllangerror);
	} /*langrunmodeless*/


boolean langdialogstart (void) {
	
	/*
	set up callback routines record, and link our data into the shell's 
	data structure.
	*/
	
	ptrcallbacks langdialogcallbacks;
	register ptrcallbacks cb;
	
	shellnewcallbacks (&langdialogcallbacks);
	
	cb = langdialogcallbacks; /*copy into register*/
	
	loadconfigresource (idlangdialogconfig, &(*cb).config);
	
	(*cb).configresnum = idlangdialogconfig;
	
	(*cb).windowholder = &langdialogwindow;
	
	(*cb).dataholder = (Handle *) &langdialogdata;
	
	(*cb).infoholder = &langdialogwindowinfo; 
	
	(*cb).activateroutine = &langdialogactivate;
	
	(*cb).updateroutine = &langdialogupdate;
	
	(*cb).mouseroutine = &langdialogmousedown;
	
	(*cb).keystrokeroutine = &langdialogkeystroke;
	
	(*cb).closeroutine = &langdialogclose;
	
	(*cb).adjustcursorroutine = &langdialogadjustcursor;
	
	(*cb).idleroutine = &langdialogidle;
	
	(*cb).cutroutine = &langdialogcut;
	
	(*cb).copyroutine = &langdialogcopy;
	
	(*cb).pasteroutine = &langdialogpaste;
	
	(*cb).clearroutine = &langdialogclear;
	
	/*
	(*cb).fontroutine = &langdialogsetfont;
	
	(*cb).sizeroutine = &langdialogsetsize;
	*/
	
	(*cb).setselectioninforoutine = &langdialogsetselectioninfo;
	
	(*cb).selectallroutine = &langdialogselectall;
	
	(*cb).searchroutine = &langdialogsearch;
	
	(*cb).gettargetdataroutine = &langdialoggettargetdata;
	
	return (true);
	} /*langdialogstart*/



