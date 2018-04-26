
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

/*Forked. Carbon version at the top, "good" version at the bottom. This file should be
reconciled later.*/

/*
	2004-10-28 aradke: Menusharing won't work on Carbon.
	This file is only relevant for the classic Mac OS version.
*/


 
/*© Copyright 1992-1993 UserLand Software, Inc.  All Rights Reserved.*/

#include <land.h>
#include "dialogs.h"
#include "ops.h"
#include "kb.h"
#include "memory.h"
#include "langinternal.h"
#include "langipc.h"
#include "osacomponent.h"
#include "osainternal.h"
#include "osamenus.h"
#include "resources.h"
#include <SetUpA5.h>

	#include "aeutils.h"


#define MSglobals ((**hcg).menusharingglobals)

// 3/7/97 dmb: bumped this constant from 3 to 5
// 9.24/97 dmb: changed this constant to be ticks, not requests
#define tickstoignorerequests 15

#define tickstocheckformulas 15



static pascal Boolean ProcessInForeground () {
	
	/*
	return true if we're running in the foreground, false if we're in the
	background.
	*/
	
	ProcessSerialNumber currentprocess, frontprocess;
	Boolean fl;
	
	GetCurrentProcess (&currentprocess);
	
	GetFrontProcess (&frontprocess);
	
	SameProcess (&currentprocess, &frontprocess, &fl);
	
	return (fl);
	} /*ProcessInForeground*/


static pascal short CountMenuArray (hdlcomponentglobals hcg) {
	
	/*
	return the number of menus in the menu array.
	*/
	
	register hdlmenuarray hm = MSglobals.hsharedmenus;
	
	if (hm == nil)
		return (0);
	
	return ((short) (GetHandleSize ((Handle) hm) / sizeof (tyruntimemenurecord)));
	} /*CountMenuArray*/


static pascal Boolean InstallSharedMenus (hdlcomponentglobals hcg, short idmenuafter) {
	
	/*
	insert all of the menus in the menuarray into the menu bar.  main 
	menus are inserted in front of idmenuafter. if idmenuafter is zero, 
	main (non-hierarchic) menus will be added to the right of all others.
	*/
	
	register hdlmenuarray hm = MSglobals.hsharedmenus;
	register short i, ct;
	tyruntimemenurecord item;
	
	if (MSglobals.installmenuscallback != nil) {
		
		long appA5 = SetUpCurA5 ();
		
		CallMSmenuscallbackProc (MSglobals.installmenuscallback, hm);
		
		RestoreA5 (appA5);
		}
	else {
	
		ct = CountMenuArray (hcg);
		
		for (i = 0; i < ct; i++) {
			
			item = (**hm) [i];
			
			if (item.flhierarchic)
				InsertMenu (item.hmenu, -1);
			else
				InsertMenu (item.hmenu, idmenuafter);
			
			(**hm) [i].flinserted = true; /*so we'll know it needs to be removed*/
			} /*for*/
		}
	
	return (true);
	} /*InstallSharedMenus*/


static pascal Boolean GetSharedMenus (hdlcomponentglobals hglobals, short firstresource) {
	
	/*
	call the menu server to get a menuarray, keyed off of our application id.
	
	firstresource is the starting id to be used for the menus; if there are 
	n menus, their ids will range from firstresource to firstresource + n - 1.
	
	2.1b13 dmb: use new osasethome/clientzone
	*/
	
	register hdlcomponentglobals hcg = hglobals;
	boolean fl;
	hdlmenuarray hsharedmenus = (hdlmenuarray) NewHandle (sizeof (tyruntimemenurecord));
	
	osasethomezone (hcg);
	
	fl = langipcgetmenuarray (MSglobals.clientid, firstresource, true, (Handle *) &hsharedmenus);
	
	osasetclientzone (hcg);
	
	if (!fl)
		return (false);
	
	MSglobals.hsharedmenus = hsharedmenus;
	
	return (true);
	} /*GetSharedMenus*/


static pascal Boolean CheckFormulas (hdlcomponentglobals hglobals) {
	
	/*
	probably should maintain msglobal and do this less often
	
	2.1b13 dmb: need to use our heap when calcualting formulas. use new 
	osasethome/clientzone to do so.
	*/
	
	register hdlcomponentglobals hcg = hglobals;
	boolean fl;
	
	++fldisableyield;	/*dmb 4.1b2 - in case forulas send AEs*/
			
	osasethomezone (hcg);
	
	pushhashtable (nil); /*override storage table set up by callosafunction so langrunhandle will set up root*/
	
	fl = langipccheckformulas (MSglobals.clientid);
	
	pophashtable ();
	
	osasetclientzone (hcg);
	
	--fldisableyield;	/*dmb 4.1b2*/
	
	return (fl);
	} /*CheckFormulas*/


static pascal ComponentResult osaSharedScriptRunning (hdlcomponentglobals hcg, Boolean *flrunning) {
	
	/*
	returns true if a shared script is currently running, false otherwise.
	
	it's provided so that an application can intelligently handle cmd-period
	script termination in its keystroke handling routine.
	*/
	
	*flrunning = flscriptrunning; /*MSglobals.flscriptrunning*/
	
	return (noErr);
	} /*osaSharedScriptRunning*/


static pascal ComponentResult osaCancelSharedScript (hdlcomponentglobals hcg) {
	
	/*
	call this when the user presses cmd-period or otherwise indicates to you that
	he or she wants the currently running script to be halted. 
		
	8/13/92 DW: if we're talking to post-2.0 Frontier or Runtime 1.0, we send a
	message to the server telling it to kill the script. otherwise we do it the
	old less elegant way, by setting a flag that gets monitored in calls to 
	SharedScriptCancelled.
	*/
	
	processdisposecode ((hdltreenode) MSglobals.idscript);
	
	return (noErr);
	} /*osaCancelSharedScript*/


static pascal ComponentResult osaDisposeSharedMenus (hdlcomponentglobals hglobals) {
	
	/*
	completely dispose of the menuarray and the menu handles it contains.
	
	10/10/91 DW: check for no shared menus before disposing, save code if 
	its ever called from more than one place. also set the global handle to
	nil after disposing and redraw the menu bar.
	
	2.1b13 dmb: we're now called by closesharedmenus, as part of our client's 
	ExitToShell procedure. to prevent Menu Manager crashes, we have to make 
	sure MenuList isn't nil before deleting a menu. in fact, event disposing a 
	menu crashes in this context, so if menulist is nil we'll assume the app 
	is shutting down, and just skip that part entirely.
	
	4.0b7 dmb: set fldirtysharedmenus to true;
	*/
	
	register hdlcomponentglobals hcg = hglobals;
	register hdlmenuarray hm = MSglobals.hsharedmenus;
	register short i;
	register short ctmenus;
	tyruntimemenurecord item;
	//9/1/00 Timothy Paustian
	//got rid of nasty use of direct memory access.
	//not allowed in carbon
	MenuBarHandle MenuList = nil;
	MenuList = GetMenuBar();
			
	MSglobals.fldirtysharedmenus = true;	// 4.0b7 dmb: need this for client to have dynamic menubar
	
	if (hm == nil) /*no shared menus to toss*/
		return (noErr);
	
	if (MenuList != nil) {
		
		if (MSglobals.removemenuscallback != nil) {
		
			long appA5 = SetUpCurA5 ();
			
			CallMSmenuscallbackProc (MSglobals.removemenuscallback, hm);
			
			RestoreA5 (appA5);
			}
		
		ctmenus = CountMenuArray (hcg);
		
		for (i = 0; i < ctmenus; i++) {
			
			item = (**hm) [i];
			
			if (item.flinserted && (MSglobals.removemenuscallback == nil))
				DeleteMenu (item.idmenu);
			
			DisposeMenu (item.hmenu);
			} /*for*/
		
		DrawMenuBar ();
		}
	
	langipcdisposemenuarray (MSglobals.clientid, (Handle) hm);
	
	/*
	DisposeHandle ((Handle) hm);
	*/
	
	MSglobals.hsharedmenus = nil;
	//Code change by Timothy Paustian Tuesday, September 5, 2000 9:26:37 PM
	//only dispose of menubars in carbon. In OS 9 the menubar handle is 
	//not a copy
	DisposeMenuBar(MenuList);
		
	return (noErr);
	} /*osaDisposeSharedMenus*/


static pascal ComponentResult osaCheckSharedMenus (hdlcomponentglobals hglobals, short idinsertafter) {
	
	/*
	call this from your main event loop after receiving and processing every
	event. if the menus need updating, we send a message to the server asking
	for our shared menus.
	
	if we load menus, they are assigned resource ids starting with idinsertafter.
	this number must be less than 255 to allow for hierarchic menus, and must be
	small enough so that no menu has an id of greater than 255. 
	
	9/28/91 DW: only update menus if we're the front process. this catches the
	delay on re-loading a changed menu structure on the Multifinder switch. No
	extra burden on the script writer editing the menu bar.
	
	4.1b6 dmb: decided that the problem where Netscape Navigator, CodeWarrior and 
	maybe other apps pick up Frontier's icon in the system Application menu must 
	be timing related. Added ctmenurequests, numrequeststoignore logic to make 
	sure that the client has called WNE a few times before we serve any menus.
	
	5.0d19 dmb: chanced ctmenurequests to timefirstrequest, and numrequeststoignore
	to tickstoignorerequests, so processor speed would stop breaking the fix
	
	5.0b6 dmb: added timelastformulacheck logic
	*/
	
	register hdlcomponentglobals hcg = hglobals;
	unsigned long ticksnow;
	
	if (!ProcessInForeground ()) /*only update menus if we're the front process*/
		return (noErr);
	
	if (flscriptrunning) /*probably re-intering from eventfilter*/
		return (noErr);
	
	ticksnow = gettickcount ();
	
	if (!MSglobals.fldirtysharedmenus) { /*no need for an update, return quickly*/
		
		if (ticksnow - MSglobals.timelastformulacheck > tickstocheckformulas) {
			
			CheckFormulas (hcg);
			
			MSglobals.timelastformulacheck = ticksnow;
			}
		
		return (noErr);
		}
	
	if (ticksnow - MSglobals.timefirstrequest <= tickstoignorerequests)
		return (noErr);
	
	osaDisposeSharedMenus (hcg);
	
	if (GetSharedMenus (hcg, idinsertafter)) {
		
		InstallSharedMenus (hcg, 0); /*install to the right of all other menus*/
		
		DrawMenuBar ();
		}
	
	MSglobals.fldirtysharedmenus = false;
	
	MSglobals.timelastformulacheck = ticksnow;
	
	return (noErr);
	} /*osaCheckSharedMenus*/


static pascal ComponentResult osaIsSharedMenu (hdlcomponentglobals hcg, short idmenu, Boolean *flshared) {
	
	/*
	return true if the indicated menu is one of the shared menus.
	*/
	
	register hdlmenuarray hm = MSglobals.hsharedmenus;
	register short ct, i;
	tyruntimemenurecord item;
	
	ct = CountMenuArray (hcg);
	
	*flshared = false;
	
	for (i = 0; i < ct; i++) {
		
		item = (**hm) [i];
		
		if (item.idmenu == idmenu) {
			
			*flshared = true;
			
			break;
			}
		} /*for*/
	
	return (noErr);
	} /*osaIsSharedMenu*/


static pascal ComponentResult osaEnableSharedMenus (hdlcomponentglobals	hcg, Boolean flenable) {
	
	/*
	Enables or disables the the menus in the specified menu array.
	
	Always returns true.
	*/
	
	register hdlmenuarray hm = MSglobals.hsharedmenus;
	register short i;
	register short ctmenus;
	register MenuHandle hmenu;
	
	ctmenus = CountMenuArray (hcg);
	
	for (i = 0; i < ctmenus; i++) {
		
		hmenu = (**hm) [i].hmenu;
		//Code change by Timothy Paustian Monday, June 26, 2000 4:16:27 PM
		//Update to modern call

		if (flenable)
			#if ACCESSOR_CALLS_ARE_FUNCTIONS == 1
			EnableMenuItem (hmenu, 0);
			#else
			EnableItem (hmenu, 0);
			#endif
		else
			#if ACCESSOR_CALLS_ARE_FUNCTIONS == 1
			DisableMenuItem (hmenu, 0);
			#else
			DisableItem (hmenu, 0);
			#endif
		} /*for*/
	
	DrawMenuBar ();
	
	return (noErr);
	} /*osaEnableSharedMenus*/


static pascal OSErr sharedmenueventfilter (EventRecord *event, long callback) {
	
	/*
	2.1b11 dmb: handle an event for the activeproc or the sendproc
	
	3.0a dmb: HiliteMenu (0) any time we see an os event so menu 
	manager won't get confused when client does it later and it's 
	not in front.
	
	4.1b13 dmb: use new iscmdperiodevent
	*/
	
	register EventRecord *ev = event;
	long appA5;
	
	switch ((*ev).what) {
		
		case keyDown:
			if (iscmdperiodevent ((*ev).message, keyDown, (*ev).modifiers))
				return (userCanceledErr);
			
			break;
		
		case mouseDown: /*don't let client see these -- can't do menu selection*/
			break;
		
		default:
			if ((*ev).what == osEvt) /*3.0a*/
				HiliteMenu (0);
			
			appA5 = SetUpCurA5 ();
			
			CallMSeventcallbackProc ((MSeventcallbackUPP) callback, ev);
			
			RestoreA5 (appA5);
			
			if ((*ev).what == userCanceledErr)
				return (userCanceledErr);
			
			break;
		}
	
	return (noErr);
	} /*sharedmenueventfilter*/


static pascal OSErr sharedmenuactiveproc (long refcon) {
	
	/*
	2.1b10 dmb: the client provided an event filter, which we've stashed 
	in the refon.
	
	on a keypress, see if user pressed cmd-period.
	
	on system & null events, pass event to caller. if the event comes back 
	with the ev.what field set to userCanceledErr, abort the script.
	
	the client's context is set when we get here, so we need to set up 
	our A5 to call anything else
	
	2.1b13: also call the default activeproc so that we'll see cmd-. 
	event there are other events preempting (updates & activates)
	*/
	
	EventRecord ev;
	OSErr err;
	
	long curA5 = SetUpAppA5 ();
	
	WaitNextEvent (keyDownMask | mDownMask | osMask | activMask | updateMask, &ev, 1L, nil);
	
	err = sharedmenueventfilter (&ev, refcon);
	
	if (err == noErr)
		err = osadefaultactiveproc (0L);
	
	RestoreA5 (curA5);
	
	return (err);
	} /*sharedmenuactiveproc*/


static pascal Boolean sharedmenuidleproc (EventRecord *ev, long *sleep, RgnHandle *mousergn) {
	
	/*
	2.1b10: pass the event through to the client application so it can 
	respond to it.
	
	3.0b15 dmb: see comment in sharedmenusendproc below. the only way to get 
	the event filter is to find our globals.
	*/
	
	#pragma unused (sleep, mousergn)
	
	OSErr err = noErr;
	long eventfilter;
	
	long curA5 = SetUpAppA5 ();
	
	if (osafindclienteventfilter ((long) nil, &eventfilter))
		err = sharedmenueventfilter (ev, eventfilter);
		
	RestoreA5 (curA5);
	
	return (err != noErr); /*false means keep waiting*/
	} /*sharedmenuidleproc*/


static pascal OSErr sharedmenusendproc (const AppleEvent *, AppleEvent *, AESendMode, AESendPriority, long, AEIdleUPP, AEFilterUPP, long);


#if !TARGET_RT_MAC_CFM

	#define sharedmenuactiveprocUPP sharedmenuactiveproc
	
	#define sharedmenuidleprocUPP sharedmenuidleproc
	
	#define sharedmenusendprocUPP sharedmenusendproc

#else

	
	OSAActiveUPP	sharedmenuactiveprocDesc;
	
	AEIdleUPP	sharedmenuidleprocDesc;
	
	OSASendUPP	sharedmenusendprocDesc;
	
	#define sharedmenuactiveprocUPP (sharedmenuactiveprocDesc)
	
	#define sharedmenuidleprocUPP (sharedmenuidleprocDesc)
	
	#define sharedmenusendprocUPP (sharedmenusendprocDesc)

	
	
#endif


static pascal OSErr sharedmenusendproc (const AppleEvent *event, AppleEvent *reply,
		AESendMode sendmode, AESendPriority priority, long timeout,
        AEIdleUPP idleproc, AEFilterUPP filterproc, long refcon) {
	
	/*
	2.1b11 dmb: the client provided an event filter. we do the same event 
	filtering as we do for the activeproc.
	
	the client's context is set when we get here, so we need to set up 
	our A5 to call anything else
	
	3.0b14 dmb: call osadefaultactiveproc here for cmd-period detection. 
	not needed for Frontier scripts, but essential for AppleScript scripts.
	
	3.0b15 dmb: since the AESend idleProc doesn't get a refcon, we have no way 
	to directly pass along our refcon, the eventfilter. we were using a static 
	global here, but if one client sends an event while another is waiting for 
	a reply, that doesn't work. the best solution I can think of right now is 
	for the idleproc to the use the CurrentA5 value to look up the globals for 
	this client, and get the eventfilter routine from there.
	*/
	
	OSErr err;
	
	long curA5 = SetUpAppA5 ();
	
	err = osadefaultactiveproc (0L);
	
	/*
	sharedmenufilterproc = refcon;
	*/
	
	idleproc = sharedmenuidleprocUPP;
	
	RestoreA5 (curA5);
	
	if (err == noErr)
		err = AESend (event, reply, sendmode, priority, timeout, idleproc, filterproc);
	
	return (err);
	} /*sharedmenusendproc*/


static boolean sharedmenuprocessevent (EventRecord *ev) {
	
	/*
	4.1b13 dmb: new routine, for new processevent langcallback.
	we're only installed if the filterproc is non-nil
	
	4.1b14 dmb: we must pre/post client callback here.
	*/
	
	register hdlcomponentglobals hcg = osaglobals;
	long appA5;
	
	osapreclientcallback (hcg);
	
	appA5 = SetUpCurA5 ();
	//Code change by Timothy Paustian Friday, July 28, 2000 2:58:43 PM
	//This stuff is macros and is probably going to give me trouble.
	CallMSeventcallbackProc (MSglobals.eventfiltercallback, ev);
	
	RestoreA5 (appA5);
	
	osapostclientcallback (hcg);
	
	return (true);
	} /*sharedmenuprocessevent*/


static pascal ComponentResult osaRunSharedMenuItem (hdlcomponentglobals hglobals, short idmenu, short iditem) {
	 
	/*
	call the menu server to run the script linked to the indicated menu item.
	
	the script will execute asynchonously, after this call returns.
	
	SDK 2.0: if the server isn't running, remove the shared menus and return
	false. this will only happen if the server has crashed without letting us
	know that our menus are dirty.
	
	2.1b4 dmb: handle langtext signature (i.e. use specific component)
	
	2.1b6 dmb: if the script is UserTalk, don't go through the generalized 
	procedure of opening a component instance. not only is it slow, but the 
	initial context for the instance would be all wrong, so operations 
	that restore the client context, like osagetnextevent, would break.
	
	2.1b10 dmb: added support for the new eventfilter callback
	
	2.1b11 dmb: extended eventfilter support to flow through a sendproc 
	as well. also, realized that when the script is not in usertalk, 
	the client context needs to be restored in both the activeproc and 
	the sendproc.
	
	2.1b12 dmb: leave fldisableyield in it's original state, just to be safe.
	
	2.1b13 dmb: use new osasethome/clientzone. also, try setting our zone 
	while getting langtext; we seem to be having spurious heap problems, 
	and maybe this is it.
	
	3.0b15 dmb: when OSADoScript returns a script error, it seems that we still 
	need to call OSAScriptError to get the error text -- at least for AppleScript 
	we do.  (This is counter to my recollection of the DoScript spec, but that's 
	how it is.)  Also, initialize result to null, just in case.
	
	3.0b16 dmb: when the script is non-UserTalk, must set flscriptrunning true 
	and false manually.
	*/
	
	register hdlcomponentglobals hcg = hglobals;
	Handle htext;
	long signature;
	ComponentInstance comp;
	MSeventcallbackUPP eventfilter;
	AEDesc desc;
	AEDesc result = {typeNull, nil};
	long clienta5, appA5;
	boolean fl;
	bigstring bserror;
	OSAError err;
	
	osasethomezone (hcg);
	
	fl = langipcgetitemlangtext ((**hcg).clientid, idmenu, iditem, &htext, &signature);
	
	osasetclientzone (hcg);
	
	if (htext == nil) /*we must have just displayed script node*/
		return (noErr);
	
	if (!fl)
		return (osageterror ());
	
	desc.descriptorType = typeChar;
	
	
		newdescwithhandle (&desc, typeChar, htext);
	
	
	eventfilter = MSglobals.eventfiltercallback;
	
	if (signature == typeLAND) {
		
		if (eventfilter != nil) {
			
			osaSetActiveProc (hcg, sharedmenuactiveprocUPP, (long) eventfilter);
			
			osaSetSendProc (hcg, sharedmenusendprocUPP, (long) eventfilter);
			
			langcallbacks.processeventcallback = sharedmenuprocessevent;
			}
		
		err = osaDoScript (hcg, &desc, kOSANullScript, typeChar, kOSANullMode, &result);
		
		if (eventfilter != nil) {
			
			osaSetActiveProc (hcg, nil, 0L); /*don't leave this set for when we calc formulas*/
			
			osaSetSendProc (hcg, nil, 0L);
			}
		}
	else {
		
		osasethomezone (hcg);
		
		comp = getosaserver (signature);
		
		osasetclientzone (hcg);
		
		if (comp == nil) {
			
			langostypeparamerror (cantopencomponenterror, signature); /*set the error*/
			
			osaScriptError (hcg, kOSAErrorMessage, typeChar, &result); /*get the text*/
			
			err = osageterror (); /*always returns non-zero*/
			}
		else {
			
			if (eventfilter != nil) {
				
				OSASetActiveProc (comp, sharedmenuactiveprocUPP, (long) eventfilter);
				
				OSASetSendProc (comp, sharedmenusendprocUPP, (long) eventfilter);
				}
			
			flscriptrunning = true; /*need to set to block checksharedmenus*/
			
			clienta5 = osapreclientcallback (hcg);
			
			#ifdef THINK_C
				
				asm {
					move.l	a5,-(a7)
					move.l	clienta5,a5
					}
			
			#else
			
				appA5 = SetUpCurA5 ();	// SetUpThisA5 (clienta5);	// 4.0b5: was: SetUpCurA5 ();
			
			#endif
			
			err = OSADoScript (comp, &desc, kOSANullScript, typeChar, kOSANullMode, &result);
			
			if ((err == errOSAScriptError) && (result.descriptorType == typeNull)) { /*need error text*/
				
				if (OSAScriptError (comp, kOSAErrorNumber, typeLongInteger, &result) == noErr) {
					
						
						{
						Handle hcopy;
						
						copydatahandle (&result, &hcopy);
						
						err = **(long **) hcopy;
						
						disposehandle (hcopy);
						}
									
					
					AEDisposeDesc (&result);
					}
				
				if (err != userCanceledErr)
					OSAScriptError (comp, kOSAErrorMessage, typeChar, &result);
				}
			
			RestoreA5 (appA5);
			
			osapostclientcallback (hcg);
			
			flscriptrunning = false; /*unblock checksharedmenus*/
			
			if (eventfilter != nil) {
				
				OSASetActiveProc (comp, nil, 0L); /*clear it out*/
				
				OSASetSendProc (comp, nil, 0L);
				}
			}
		}
	
	osasethomezone (hcg);
	
	AEDisposeDesc (&desc);
	
	osasetclientzone (hcg);
	
	if ((err != noErr) && (err != userCanceledErr) && (result.descriptorType == typeChar)) {
		
		
			datahandletostring (&result, bserror);
		
		
		if (MSglobals.scripterrorcallback == nil) {
			
			osainstallpatch (hcg);
			
			osasethomezone (hcg);
			
			++fldisableyield; /*shellactivate, at least, needs this in this context*/
			
			alertdialog (bserror);
			
			--fldisableyield; /*restore*/
			
			osasetclientzone (hcg);
			
			osaremovepatch (hcg, nil);
			}
		else {
			clienta5 = osapreclientcallback (hcg);
			
			#ifdef THINK_C
			
			asm {
				move.l	a5,-(a7)
				move.l	clienta5,a5
				}
				
				(*(tyMSstringcallback)MSglobals.scripterrorcallback) (bserror);
			
			#else
			
				appA5 = SetUpCurA5 ();	// SetUpThisA5 (clienta5);	// 4.0b5: was: SetUpCurA5 ();
				
				CallMSstringcallbackProc (MSglobals.scripterrorcallback, bserror);
				
			#endif
			
			RestoreA5 (appA5);
			
			osapostclientcallback (hcg);
			}
		}
	
	AEDisposeDesc (&result);
	
	return (err);
	} /*osaRunSharedMenuItem*/


static pascal ComponentResult osaSetScriptErrorCallback (hdlcomponentglobals hglobals, MSstringcallbackUPP errorcallback) {
	
	register hdlcomponentglobals hcg = hglobals;
	
	MSglobals.scripterrorcallback = errorcallback;
	
	return (noErr);
	} /*osaSetScriptErrorCallback*/


static pascal ComponentResult osaSetEventFilterCallback (hdlcomponentglobals hglobals, MSeventcallbackUPP eventcallback) {
	
	/*
	2.1b10 dmb: new routine to allow some background operations to go on 
	while a shared menu script is running
	*/
	
	register hdlcomponentglobals hcg = hglobals;
	
	MSglobals.eventfiltercallback = eventcallback;
	
	return (noErr);
	} /*osaSetEventFilterCallback*/


static pascal ComponentResult osaSetMenusInserterCallback (hdlcomponentglobals hglobals, MSmenuscallbackUPP callback) {
	
	/*
	2.1b10 dmb: new routine to allow some background operations to go on 
	while a shared menu script is running
	*/
	
	register hdlcomponentglobals hcg = hglobals;
	
	MSglobals.installmenuscallback = callback;
	
	return (noErr);
	} /*osaSetMenusInserterCallback*/


static pascal ComponentResult osaSetMenusRemoverCallback (hdlcomponentglobals hglobals, MSmenuscallbackUPP callback) {
	
	/*
	2.1b10 dmb: new routine to allow some background operations to go on 
	while a shared menu script is running
	*/
	
	register hdlcomponentglobals hcg = hglobals;
	
	MSglobals.removemenuscallback = callback;
	
	return (noErr);
	} /*osaSetMenusRemoverCallback*/


static pascal ComponentResult osaSharedMenuHit (hdlcomponentglobals hglobals, short idmenu, short iditem, Boolean *flhit) {

	register hdlcomponentglobals hcg = hglobals;
	OSErr err;
	
	err = osaIsSharedMenu (hcg, idmenu, flhit);
	
	if (err == noErr) {
		
		if (*flhit)
			err = osaRunSharedMenuItem (hcg, idmenu, iditem);
		}
	
	return (err);
	} /*osaSharedMenuHit*/


static boolean removemenusharinghandlers (hdlcomponentglobals); /*forward*/


static pascal ComponentResult osaDirtySharedMenus (hdlcomponentglobals hglobals) {
	
	/*
	2.1b8 dmb: toss dirty menus now; the component may be gone by the time 
	we're frontmost & try to refresh them. in order to do this, we now need 
	to set up a5.
	
	3.0b16 dmb: added call to closeosaservers. since Frontier may be going 
	away while the client sticks around, this is the only reliable place to 
	get rid of those instances -- 36K of memory for AppleScript
	
	4.1b2 dmb: broke this code out of HandleMenuDirty so that it can be called
	directly by a (4.1-aware) menusharing client
	
	4.1b12 dmb: reset MSglobals.ctmenurequests when we dirty them, so client 
	will have to ask a few times after switching in.
	*/
	
	register hdlcomponentglobals hcg = hglobals;
	
// 4.0b7 dmb: now done by osaDisposeSharedMenus --	MSglobals.fldirtysharedmenus = true;
	
	long curA5 = SetUpAppA5 ();
	
	osaDisposeSharedMenus (hcg); /*2.1b8: toss them now*/
	
	if (flosashutdown) {
		
		removemenusharinghandlers (hcg);
		
		closeosaservers ();
		
		MSglobals.clientid = 0; /*so servingsharedmenus can break out of loop*/
		}
	
	MSglobals.timefirstrequest = gettickcount (); /*4.1b12*/
	
	RestoreA5 (curA5);
	
	return (noErr);
	} /*osaDirtySharedMenus*/


static pascal OSErr HandleMenuDirty (const AppleEvent *event, AppleEvent *reply, UInt32 refcon) {
	
	/*
	this Apple event handler is called when the application's menu bar has been 
	edited by the script writer in the menu server's menu editor.
	
	4.1b2 dmb: call osaDirtySharedMenus, which does what we used to do directly.
	*/
	
	#pragma unused (event, reply)

	osaDirtySharedMenus ((hdlcomponentglobals) refcon);
	
	return (noErr);
	} /*HandleMenuDirty*/


#if !TARGET_RT_MAC_CFM

	#define HandleMenuDirtyUPP HandleMenuDirty

#else

	AEEventHandlerUPP  HandleMenuDirtyDesc;

	#define HandleMenuDirtyUPP (&HandleMenuDirtyDesc)

#endif


static boolean installmenusharinghandlers (hdlcomponentglobals hglobals) {
	
	register hdlcomponentglobals hcg = hglobals;
	OSErr err;
	
	#if TARGET_API_MAC_CARBON && TARGET_RT_MAC_CFM
	HandleMenuDirtyDesc = NewAEEventHandlerUPP(HandleMenuDirty);
	#endif
	err = AEInstallEventHandler (MSglobals.clientid, 'updm', HandleMenuDirtyUPP, (long) hcg, false);
	
	/*
	err = AEInstallEventHandler (MSglobals.clientid, 'done', (EventHandlerProcPtr) &HandleScriptComplete, (long) hglobals, false);
	*/
	
	return (err == noErr);
	} /*installmenusharinghandlers*/


static boolean removemenusharinghandlers (hdlcomponentglobals hglobals) {
	
	register hdlcomponentglobals hcg = hglobals;
	OSErr err;
	
	err = AERemoveEventHandler (MSglobals.clientid, 'updm', HandleMenuDirtyUPP, false);
	
	/*
	err = AERemoveEventHandler (MSglobals.clientid, 'done', (EventHandlerProcPtr) &HandleScriptComplete, false);
	*/
	
	return (err == noErr);
	} /*removemenusharinghandlers*/




static pascal ComponentResult menucandofunction (short selector) {
	
	switch (selector) {
		
		case kComponentOpenSelect:
		case kComponentCloseSelect:
		case kComponentCanDoSelect:
		case kComponentVersionSelect:
		
		case msInitSharedMenusCommand:
		case msSharedMenuHitCommand:
		case msSharedScriptRunningCommand:
		case msCancelSharedScriptCommand:
		case msCheckSharedMenusCommand:
		case msDisposeSharedMenusCommand:
		case msIsSharedMenuCommand:
		case msEnableSharedMenusCommand:
		case msRunSharedMenuItemCommand:
		case msSetScriptErrorCallbackCommand:
		case msSetEventFilterCallbackCommand:
		case msSetMenusInserterCallbackCommand:
		case msSetMenusRemoverCallbackCommand:
		case msDirtySharedMenusCommand:
			return (true);
		}
	
	return (false);
	} /*menucandofunction*/
//Code change by Timothy Paustian Wednesday, July 26, 2000 9:52:26 PM
//A forward declaration to satify the compiler. I had to move things
//around a bit to get the carbon version to work.
static pascal ComponentResult osaInitSharedMenus (hdlcomponentglobals hglobals);

#if !TARGET_RT_MAC_CFM
	
	#define menucandofunctionUPP	((ComponentFunctionUPP) menucandofunction)
	#define osaInitSharedMenusUPP	((ComponentFunctionUPP) osaInitSharedMenus)
	#define osaSharedMenuHitUPP	((ComponentFunctionUPP) osaSharedMenuHit)
	#define osaSharedScriptRunningUPP	((ComponentFunctionUPP) osaSharedScriptRunning)
	#define osaCancelSharedScriptUPP	((ComponentFunctionUPP) osaCancelSharedScript)
	#define osaCheckSharedMenusUPP	((ComponentFunctionUPP) osaCheckSharedMenus)
	#define osaDisposeSharedMenusUPP	((ComponentFunctionUPP) osaDisposeSharedMenus)
	#define osaIsSharedMenuUPP	((ComponentFunctionUPP) osaIsSharedMenu)
	#define osaEnableSharedMenusUPP	((ComponentFunctionUPP) osaEnableSharedMenus)
	#define osaRunSharedMenuItemUPP	((ComponentFunctionUPP) osaRunSharedMenuItem)
	#define osaSetScriptErrorCallbackUPP	((ComponentFunctionUPP) osaSetScriptErrorCallback)
	#define osaSetEventFilterCallbackUPP	((ComponentFunctionUPP) osaSetEventFilterCallback)
	#define osaSetMenusInserterCallbackUPP	((ComponentFunctionUPP) osaSetMenusInserterCallback)
	#define osaSetMenusRemoverCallbackUPP	((ComponentFunctionUPP) osaSetMenusRemoverCallback)
	#define osaDirtySharedMenusUPP	((ComponentFunctionUPP) osaDirtySharedMenus)

#else

	enum {
		menucandofunctionProcInfo = kPascalStackBased
			 | RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
			 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(hdlcomponentglobals)))
			 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(short)))
	};
	
	enum {
		osaInitSharedMenusProcInfo = kPascalStackBased
			 | RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
			 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(hdlcomponentglobals)))
	};
	
	enum {
		osaSharedMenuHitProcInfo = kPascalStackBased
			 | RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
			 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(hdlcomponentglobals)))
			 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(short)))
			 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(short)))
			 | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(Boolean *)))
	};
	
	enum {
		osaSharedScriptRunningProcInfo = kPascalStackBased
			 | RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
			 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(hdlcomponentglobals)))
			 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(short)))
			 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(short)))
	};
	
	enum {
		osaCancelSharedScriptProcInfo = kPascalStackBased
			 | RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
			 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(hdlcomponentglobals)))
	};
	
	enum {
		osaCheckSharedMenusProcInfo = kPascalStackBased
			 | RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
			 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(hdlcomponentglobals)))
			 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(short)))
	};
	
	enum {
		osaDisposeSharedMenusProcInfo = kPascalStackBased
			 | RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
			 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(hdlcomponentglobals)))
	};
	
	enum {
		osaIsSharedMenuProcInfo = kPascalStackBased
			 | RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
			 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(hdlcomponentglobals)))
			 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(short)))
			 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(Boolean *)))
	};
	
	enum {
		osaEnableSharedMenusProcInfo = kPascalStackBased
			 | RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
			 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(hdlcomponentglobals)))
			 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(Boolean)))
	};
	
	enum {
		osaRunSharedMenuItemProcInfo = kPascalStackBased
			 | RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
			 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(hdlcomponentglobals)))
			 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(short)))
			 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(short)))
	};
	
	enum {
		osaSetScriptErrorCallbackProcInfo = kPascalStackBased
			 | RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
			 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(hdlcomponentglobals)))
			 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(MSstringcallbackUPP)))
	};
	
	enum {
		osaSetEventFilterCallbackProcInfo = kPascalStackBased
			 | RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
			 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(hdlcomponentglobals)))
			 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(MSeventcallbackUPP)))
	};
	
	enum {
		osaSetMenusInserterCallbackProcInfo = kPascalStackBased
			 | RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
			 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(hdlcomponentglobals)))
			 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(MSmenuscallbackUPP)))
	};
	
	enum {
		osaSetMenusRemoverCallbackProcInfo = kPascalStackBased
			 | RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
			 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(hdlcomponentglobals)))
			 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(MSmenuscallbackUPP)))
	};
	
	enum {
		osaDirtySharedMenusProcInfo = kPascalStackBased
			 | RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
			 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(hdlcomponentglobals)))
	};
	
	ComponentRoutineUPP		menucandofunctionDesc;
	ComponentRoutineUPP		osaInitSharedMenusDesc;
	ComponentRoutineUPP		osaSharedMenuHitDesc;
	ComponentRoutineUPP		osaSharedScriptRunningDesc;
	ComponentRoutineUPP		osaCancelSharedScriptDesc;
	ComponentRoutineUPP		osaCheckSharedMenusDesc;
	ComponentRoutineUPP		osaDisposeSharedMenusDesc;
	ComponentRoutineUPP		osaIsSharedMenuDesc;
	ComponentRoutineUPP		osaEnableSharedMenusDesc;
	ComponentRoutineUPP		osaRunSharedMenuItemDesc;
	ComponentRoutineUPP		osaSetScriptErrorCallbackDesc;
	ComponentRoutineUPP		osaSetEventFilterCallbackDesc;
	ComponentRoutineUPP		osaSetMenusInserterCallbackDesc;
	ComponentRoutineUPP		osaSetMenusRemoverCallbackDesc;
	ComponentRoutineUPP		osaDirtySharedMenusDesc;
	
	#define menucandofunctionUPP (menucandofunctionDesc)
	#define osaInitSharedMenusUPP (osaInitSharedMenusDesc)
	#define osaSharedMenuHitUPP (osaSharedMenuHitDesc)
	#define osaSharedScriptRunningUPP (osaSharedScriptRunningDesc)
	#define osaCancelSharedScriptUPP (osaCancelSharedScriptDesc)
	#define osaCheckSharedMenusUPP (osaCheckSharedMenusDesc)
	#define osaDisposeSharedMenusUPP (osaDisposeSharedMenusDesc)
	#define osaIsSharedMenuUPP (osaIsSharedMenuDesc)
	#define osaEnableSharedMenusUPP (osaEnableSharedMenusDesc)
	#define osaRunSharedMenuItemUPP (osaRunSharedMenuItemDesc)
	#define osaSetScriptErrorCallbackUPP (osaSetScriptErrorCallbackDesc)
	#define osaSetEventFilterCallbackUPP (osaSetEventFilterCallbackDesc)
	#define osaSetMenusInserterCallbackUPP (osaSetMenusInserterCallbackDesc)
	#define osaSetMenusRemoverCallbackUPP (osaSetMenusRemoverCallbackDesc)
	#define osaDirtySharedMenusUPP (osaDirtySharedMenusDesc)

#endif

static pascal ComponentResult closesharedmenus (hdlcomponentglobals hglobals) {
	
	/*
	2.1b8 dmb: new routine to be called by cmpclose for cleanup
	
	2.1b13 dmb: dispose shared menus here so that langipcmenus can 
	release the associated data structures, which would otherwise 
	be linked to menus in the heap that's about to disappear.
	
	3.0b16 dmb: added call to closeosaservers. osaservers are now 
	managed on a per-process basic, so this cleanup helps.
	*/
	
	register hdlcomponentglobals hcg = hglobals;
	
	//Code change by Timothy Paustian Friday, July 21, 2000 11:32:00 PM
	//get rid of the UPPs
	#if TARGET_API_MAC_CARBON && TARGET_RT_MAC_CFM
	DisposeComponentFunctionUPP(menucandofunctionDesc);
	DisposeComponentFunctionUPP(osaInitSharedMenusDesc);
	DisposeComponentFunctionUPP(osaSharedMenuHitDesc);
	DisposeComponentFunctionUPP(osaSharedScriptRunningDesc);
	DisposeComponentFunctionUPP(osaCancelSharedScriptDesc);
	DisposeComponentFunctionUPP(osaCheckSharedMenusDesc);
	DisposeComponentFunctionUPP(osaDisposeSharedMenusDesc);
	DisposeComponentFunctionUPP(osaIsSharedMenuDesc);
	DisposeComponentFunctionUPP(osaEnableSharedMenusDesc);
	DisposeComponentFunctionUPP(osaRunSharedMenuItemDesc);
	DisposeComponentFunctionUPP(osaSetScriptErrorCallbackDesc);
	DisposeComponentFunctionUPP(osaSetEventFilterCallbackDesc);
	DisposeComponentFunctionUPP(osaSetMenusInserterCallbackDesc);
	DisposeComponentFunctionUPP(osaSetMenusRemoverCallbackDesc);
	DisposeComponentFunctionUPP(osaDirtySharedMenusDesc);
	
	DisposeOSAActiveUPP(sharedmenuactiveprocDesc);
	DisposeAEIdleUPP(sharedmenuidleprocDesc);
	DisposeOSASendUPP(sharedmenusendprocDesc);
	#endif
		if (hcg == nil)
		return (invalidComponentID);
	
	osaDisposeSharedMenus (hcg);
	
	removemenusharinghandlers (hcg);
	
	disposecomponentglobals (hcg);
	
	closeosaservers (); /*3.0b16*/
	
	return (noErr);
	} /*closesharedmenus*/


static pascal ComponentResult osaInitSharedMenus (hdlcomponentglobals hglobals) {
	
	register hdlcomponentglobals hcg = hglobals;
	long id;
	
	#if TARGET_API_MAC_CARBON && TARGET_RT_MAC_CFM
	//Code change by Timothy Paustian Friday, July 21, 2000 11:28:35 PM
	//create the upps
	//I have to dispose of these at shutdown.
	menucandofunctionDesc = NewComponentFunctionUPP(menucandofunction, menucandofunctionProcInfo);
	osaInitSharedMenusDesc = NewComponentFunctionUPP(osaInitSharedMenus, osaInitSharedMenusProcInfo);
	osaSharedMenuHitDesc = NewComponentFunctionUPP(osaSharedMenuHit, osaSharedMenuHitProcInfo);
	osaSharedScriptRunningDesc = NewComponentFunctionUPP(osaSharedScriptRunning, osaSharedScriptRunningProcInfo);
	osaCancelSharedScriptDesc = NewComponentFunctionUPP(osaCancelSharedScript, osaCancelSharedScriptProcInfo);
	osaCheckSharedMenusDesc = NewComponentFunctionUPP(osaCheckSharedMenus, osaCheckSharedMenusProcInfo);
	osaDisposeSharedMenusDesc = NewComponentFunctionUPP(osaDisposeSharedMenus, osaDisposeSharedMenusProcInfo);
	osaIsSharedMenuDesc = NewComponentFunctionUPP(osaIsSharedMenu, osaIsSharedMenuProcInfo);
	osaEnableSharedMenusDesc = NewComponentFunctionUPP(osaEnableSharedMenus, osaEnableSharedMenusProcInfo);
	osaRunSharedMenuItemDesc = NewComponentFunctionUPP(osaRunSharedMenuItem, osaRunSharedMenuItemProcInfo);
	osaSetScriptErrorCallbackDesc = NewComponentFunctionUPP(osaSetScriptErrorCallback, osaSetScriptErrorCallbackProcInfo);
	osaSetEventFilterCallbackDesc = NewComponentFunctionUPP(osaSetEventFilterCallback, osaSetEventFilterCallbackProcInfo);
	osaSetMenusInserterCallbackDesc = NewComponentFunctionUPP(osaSetMenusInserterCallback, osaSetMenusInserterCallbackProcInfo);
	osaSetMenusRemoverCallbackDesc = NewComponentFunctionUPP(osaSetMenusRemoverCallback, osaSetMenusRemoverCallbackProcInfo);
	osaDirtySharedMenusDesc = NewComponentFunctionUPP(osaDirtySharedMenus, osaDirtySharedMenusProcInfo);
				
	sharedmenuactiveprocDesc = NewOSAActiveUPP(sharedmenuactiveproc);
	
	sharedmenuidleprocDesc = NewAEIdleUPP(sharedmenuidleproc);
	
	sharedmenusendprocDesc = NewOSASendUPP(sharedmenusendproc);
	#endif
		
	if (hcg == nil)
		return (invalidComponentID);
	
	id = (**hcg).clientid;
	
	MSglobals.serverid = typeLAND; /*Frontier's creator id*/
	
	MSglobals.clientid = id;
	
	MSglobals.hsharedmenus = nil; /*haven't loaded shared menus yet*/
	
	MSglobals.fldirtysharedmenus = true; /*force update 1st time thru event loop*/
	
	MSglobals.flhavecomponentmanager = true;
	
	MSglobals.timefirstrequest = gettickcount (); /*5.0d19*/
	
	installmenusharinghandlers (hcg);
	
	return (noErr);
	} /*osaInitSharedMenus*/

static pascal ComponentResult menusharingdispatch (register ComponentParameters *params, Handle storage) {
	
	/*
	2.1b9 dmb: new dispatcher; menu sharing component now completely 
	seperate from OSA
	*/
	
	ComponentResult result = badComponentSelector;
	short what = (*params).what;
	
	if (what < 0) { /*negative selectors are component manager calls*/
		
		switch (what) {
			
			case kComponentOpenSelect: { /*can't call subroutine because a5 isn't ours*/
				
				hdlcomponentglobals hglobals;
				Component self = (Component) (*params).params [0];
				long selfa5;
				long clienta5;
				
				selfa5 = GetComponentRefcon (self);
				
				#ifdef THINK_C
					
					asm {
						move.l	a5,clienta5
						move.l	a5,-(a7)
						move.l	selfa5,a5
						}
					
				#else
				
					clienta5 = SetUpAppA5 ();
					

				#endif
				
				
				
				if (newcomponentglobals (self, clienta5, &hglobals)) {
					
					SetComponentInstanceStorage ((ComponentInstance) self, (Handle) hglobals);
					
					result = noErr;
					}
				else
					result = memFullErr;
				
				RestoreA5 (clienta5);
				
				break;
				}
			
			case kComponentCloseSelect:
				
				result = closesharedmenus ((hdlcomponentglobals) storage);
				
				break;
			
			case kComponentCanDoSelect:
				result = CallComponentFunction (params, menucandofunctionUPP);
				break;
			
			case kComponentVersionSelect:
				result = 0x04100100;
				
				break;
			}
		}
	else  {	/*positive selectors are menu sharing calls*/
		
		ComponentFunctionUPP func = nil;
		
		switch (what) {
			
			case msInitSharedMenusCommand:
				func = osaInitSharedMenusUPP;
				
				break;
			
			case msSharedMenuHitCommand:
				func = osaSharedMenuHitUPP;
				
				break;
			
			case msSharedScriptRunningCommand:
				func = osaSharedScriptRunningUPP;
				
				break;
			
			case msCancelSharedScriptCommand:
				func = osaCancelSharedScriptUPP;
				
				break;
			
			case msCheckSharedMenusCommand:
				func = osaCheckSharedMenusUPP;
				
				break;
			
			case msDisposeSharedMenusCommand:
				func = osaDisposeSharedMenusUPP;
				
				break;
			
			case msIsSharedMenuCommand:
				func = osaIsSharedMenuUPP;
				
				break;
			
			case msEnableSharedMenusCommand:
				func = osaEnableSharedMenusUPP;
				
				break;
			
			case msRunSharedMenuItemCommand:
				func = osaRunSharedMenuItemUPP;
				
				break;
			
			case msSetScriptErrorCallbackCommand:
				func = osaSetScriptErrorCallbackUPP;
				
				break;
			
			case msSetEventFilterCallbackCommand:
				func = osaSetEventFilterCallbackUPP;
				
				break;
			
			case msSetMenusInserterCallbackCommand:
				func = osaSetMenusInserterCallbackUPP;
				
				break;
			
			case msSetMenusRemoverCallbackCommand:
				func = osaSetMenusRemoverCallbackUPP;
				
				break;
			
			case msDirtySharedMenusCommand:
				func = osaDirtySharedMenusUPP;
				
				break;
			
			} /*switch*/
		
		if (func != nil)
			result = callosafunction (storage, params, func);
		}
	
	return (result);
	} /*menusharingdispatch*/


boolean initmenusharingcomponent (void) {
	
	/*
	register the menu sharing component.
	*/
	
	Component comp;
	
	
	comp = osaregistercomponent (msComponentType, 0, &menusharingdispatch, frontierstring, menusharingcomponentstring);
	
	return (comp != nil);
	} /*initmenusharingcomponent*/



