
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

/*Forked file. Carbon at top, "good" version at bottom. This file should be reconciled later.*/

#include "frontier.h"
#include "standard.h"

#if TARGET_API_MAC_CARBON == 1

#include <uisharing.h>
#include <uisinternal.h>
#include <land.h>
#include "ops.h"
#include "kb.h"
#include "memory.h"
#include "quickdraw.h"
#include "resources.h"
#include "shell.h"
#include "shellmenu.h"
#include "shellprivate.h"
#include "tablestructure.h"
#include "osainternal.h"
#include "osawindows.h"
#include <SetUpA5.h>



static WindowPtr osageteventwindow (EventRecord *ev) {
	
	WindowPtr w;
	WindowPtr wfront;
	
	wfront = FrontWindow ();
	
	w = nil; /*default*/
	
	switch ((*ev).what) { /*some events apply to windows other than the frontmost*/
		
		case nullEvent: 
			w = wfront;
			
			break;
		
		case keyDown: case autoKey: case keyUp:
			w = wfront;
			
			break;
		
		case mouseDown:	case mouseUp:
			FindWindow ((*ev).where, &w);
			
			break;
		
		case updateEvt: case activateEvt:
			w = (WindowPtr) (*ev).message;
			
			break;
		
		case jugglerEvt:
			w = wfront;
			
			break;
		
		default: /*we don't handle other events*/
			break;
		} /*switch*/
	
	return (w);
	} /*osageteventwindow*/

static boolean removewindowsharinghandlers (void);

static pascal ComponentResult handlewindoweventcommand (Handle hglobals, EventRecord *ev, tyWindowSharingGlobals *wsGlobals) {
	
	boolean flcloseallwindows = false;
	WindowPtr w;
	boolean fl = false;
	
	if (flosashutdown)
		removewindowsharinghandlers ();
	
	w = osageteventwindow (ev);
	
	if (!isshellwindow (w))
		return (false);
	
	if (!shellpushglobals (w))
		return (false);
	
	switch ((*ev).what) { /*some events apply to windows other than the frontmost*/
		
		case nullEvent: 
			shellidle ();
			
			shelladjustcursor ();
			
			break;
		
		case jugglerEvt: {
			tyjugglermessage *jmsg = (tyjugglermessage *) &(*ev).message;
			
			if ((*jmsg).eventtype == 1) /*suspend or resume subevent*/
				shellactivatewindow (w, (*jmsg).flresume);
			
			break;
			}
		
		case keyDown: case autoKey: case keyUp:
			setkeyboardstatus ((*ev).message, (*ev).what, (*ev).modifiers); 
			
			if (keyboardstatus.flcmdkey) {
				
				if (!(*shellglobals.cmdkeyfilterroutine) (keyboardstatus.chkb)) {
					
					fl = true;
					
					break;
					}
				
				if (textchar (keyboardstatus.chkb)) /*pass it up to menu mgr*/
					break;
				}
			
			/*fall through into default case*/
		
		default: /*we don't handle other events*/
			
			langcallbacks.errormessagecallback = &langerrordialog;
			
			fl = shellprocessevent (ev);
			
			break;
		} /*switch*/
	
	shellpopglobals ();
	
	(*wsGlobals).flcloseallwindows = flcloseallwindows; /****/
	
	/*
	processyield ();
	*/
	
	return (fl); /*handled the event -- or not*/
	} /*eventhandlercommand*/


static pascal ComponentResult windowiscardcommand (Handle hglobals, WindowPtr w) {
	
	return (isshellwindow (w));
	} /*windowiscardcommand*/


static pascal ComponentResult closewindowcommand (Handle hglobals, WindowPtr w) {
	
	return (shellclosewindow (w));
	} /*closewindowcommand*/


static pascal ComponentResult windoweditcommand (Handle hglobals, short editcmd) {
	
	register WindowPtr w;
	register boolean fl;
	
	w = FrontWindow ();
	
	if (!isshellwindow (w))
		return (false);
	
	if (!shellpushglobals (w))
		return (false);
	
	fl = shelleditcommand ((tyeditcommand) editcmd);
	
	shellpopglobals ();
	
	return (fl);
	} /*windoweditcommand*/


static pascal ComponentResult windowopencommand (Handle hglobals, Handle h, short flscripted, Str255 windowname, Point pt, uisEventCallback filter) {
	
	#pragma unused (flscripted, windowname, pt, filter)
	
	/*
	2.1b8 dmb: new routine so client can open window at will
	*/
	
	bigstring bsname;
	hdlhashtable htable; 
	tyvaluerecord val;
	boolean fl = false;
	hdlhashnode hnode;
	
	texthandletostring (h, bsname);
	
	pushhashtable (roottable); /*override storage table set up by callosafunction so langrunhandle will set up root*/
	
	if (langexpandtodotparams (bsname, &htable, bsname)) {
		
		if (htable == nil)
			langsearchpathlookup (bsname, &htable);
		
		if (hashtablelookup (htable, bsname, &val, &hnode)) {
			
			if (val.valuetype == externalvaluetype)
				fl = langzoomvalwindow (htable, bsname, val, true);
			}
		}
	
	pophashtable ();
	
	return (fl);
	} /*windowopencommand*/


static pascal ComponentResult windowcandofunction (short selector) {
	
	switch (selector) {
		
		case kComponentOpenSelect:
		case kComponentCloseSelect:
		case kComponentCanDoSelect:
		case kComponentVersionSelect:
		
		case wsEventHandlerCommand:
		case wsWindowIsCardCommand:
		case wsCloseWindowCommand:
		case wsEditCommand:
		case wsRunFromHandleCommand:
			return (true);
		}
	
	return (false);
	} /*windowcandofunction*/


static pascal OSErr handleselectwindow (const AppleEvent *event, AppleEvent *reply, UInt32 refcon) {
	
	/*
	handle the window select event in the current application context.
	
	in case we're in Frontier, dirty the window menu
	*/
	
	OSErr err;
	OSType type;
	long size;
	WindowPtr w;
	long curA5;
	
	err = AEGetParamPtr (event, '----', typeLongInteger, &type, (Ptr) &w, sizeof (w), &size);
	
	if (err == noErr) {
		
		SelectWindow (w);
		
		curA5 = SetUpAppA5 ();
		
		shellwindowmenudirty ();
		
		RestoreA5 (curA5);
		}
	
	return (err);
	} /*handleselectwindow*/


#if THINK_C || !TARGET_RT_MAC_CFM

	#define windowcandofunctionUPP	((ComponentFunctionUPP) windowcandofunction)
	#define handlewindoweventcommandUPP	((ComponentFunctionUPP) handlewindoweventcommand)
	#define windowiscardcommandUPP	((ComponentFunctionUPP) windowiscardcommand)
	#define closewindowcommandUPP	((ComponentFunctionUPP) closewindowcommand)
	#define windoweditcommandUPP	((ComponentFunctionUPP) windoweditcommand)
	#define windowopencommandUPP	((ComponentFunctionUPP) windowopencommand)

#else

	enum {
		windowcandofunctionProcInfo = kPascalStackBased
			 | RESULT_SIZE(SIZE_CODE(sizeof(OSAError)))
			 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(hdlcomponentglobals)))
			 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(short)))
			 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(short)))
	};
	enum {
		handlewindoweventcommandProcInfo = kPascalStackBased
			 | RESULT_SIZE(SIZE_CODE(sizeof(OSAError)))
			 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(hdlcomponentglobals)))
			 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(short)))
			 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(short)))
	};
	enum {
		windowiscardcommandProcInfo = kPascalStackBased
			 | RESULT_SIZE(SIZE_CODE(sizeof(OSAError)))
			 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(hdlcomponentglobals)))
			 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(short)))
			 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(short)))
	};
	enum {
		closewindowcommandProcInfo = kPascalStackBased
			 | RESULT_SIZE(SIZE_CODE(sizeof(OSAError)))
			 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(hdlcomponentglobals)))
			 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(short)))
			 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(short)))
	};
	enum {
		windoweditcommandProcInfo = kPascalStackBased
			 | RESULT_SIZE(SIZE_CODE(sizeof(OSAError)))
			 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(hdlcomponentglobals)))
			 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(short)))
			 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(short)))
	};
	enum {
		windowopencommandProcInfo = kPascalStackBased
			 | RESULT_SIZE(SIZE_CODE(sizeof(OSAError)))
			 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(hdlcomponentglobals)))
			 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(short)))
			 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(short)))
	};
	
	#if TARGET_API_MAC_CARBON == 1
	ComponentFunctionUPP		windowcandofunctionDesc;
	ComponentFunctionUPP		handlewindoweventcommandDesc;
	ComponentFunctionUPP		windowiscardcommandDesc;
	ComponentFunctionUPP		closewindowcommandDesc;
	ComponentFunctionUPP		windoweditcommandDesc;
	ComponentFunctionUPP		windowopencommandDesc;
	
	#define windowcandofunctionUPP (windowcandofunctionDesc)
	#define handlewindoweventcommandUPP (handlewindoweventcommandDesc)
	#define windowiscardcommandUPP (windowiscardcommandDesc)
	#define closewindowcommandUPP (closewindowcommandDesc)
	#define windoweditcommandUPP (windoweditcommandDesc)
	#define windowopencommandUPP (windowopencommandDesc)
	#else
		
	static RoutineDescriptor windowcandofunctionDesc = BUILD_ROUTINE_DESCRIPTOR (windowcandofunctionProcInfo, windowcandofunction);
	static RoutineDescriptor handlewindoweventcommandDesc = BUILD_ROUTINE_DESCRIPTOR (handlewindoweventcommandProcInfo, handlewindoweventcommand);
	static RoutineDescriptor windowiscardcommandDesc = BUILD_ROUTINE_DESCRIPTOR (windowiscardcommandProcInfo, windowiscardcommand);
	static RoutineDescriptor closewindowcommandDesc = BUILD_ROUTINE_DESCRIPTOR (closewindowcommandProcInfo, closewindowcommand);
	static RoutineDescriptor windoweditcommandDesc = BUILD_ROUTINE_DESCRIPTOR (windoweditcommandProcInfo, windoweditcommand);
	static RoutineDescriptor windowopencommandDesc = BUILD_ROUTINE_DESCRIPTOR (windowopencommandProcInfo, windowopencommand);
	
	#define windowcandofunctionUPP (&windowcandofunctionDesc)
	#define handlewindoweventcommandUPP (&handlewindoweventcommandDesc)
	#define windowiscardcommandUPP (&windowiscardcommandDesc)
	#define closewindowcommandUPP (&closewindowcommandDesc)
	#define windoweditcommandUPP (&windoweditcommandDesc)
	#define windowopencommandUPP (&windowopencommandDesc)
	#endif
		
#endif

static pascal OSErr handleselectwindow (const AppleEvent *, AppleEvent *, UInt32); /*forward*/

//Code change by Timothy Paustian Friday, July 21, 2000 11:41:07 PM
//Let's see if we can get away with just installing this.
#if !TARGET_RT_MAC_CFM

	#define handleselectwindowUPP handleselectwindow

#else

	#if TARGET_API_MAC_CARBON == 1
	AEEventHandlerUPP	handleselectwindowDesc = nil;
	#define handleselectwindowUPP (handleselectwindowDesc)

	#else
		
	static RoutineDescriptor handleselectwindowDesc = BUILD_ROUTINE_DESCRIPTOR (uppAEEventHandlerProcInfo, handleselectwindow);

	#define handleselectwindowUPP (&handleselectwindowDesc)

	#endif
#endif


static boolean installwindowsharinghandlers (void) {
	
	OSErr err;
	
	#if TARGET_API_MAC_CARBON && TARGET_RT_MAC_CFM	
	if (handleselectwindowDesc == nil)
		handleselectwindowDesc = NewAEEventHandlerUPP(handleselectwindow);
	#endif
		
	err = AEInstallEventHandler ('SHUI', 'selw', handleselectwindowUPP, 0, false);
	
	return (err == noErr);
	} /*installwindowsharinghandlers*/


static boolean removewindowsharinghandlers (void) {
	
	OSErr err;
	
	err = AERemoveEventHandler ('SHUI', 'selw', handleselectwindowUPP, false);
	#if TARGET_API_MAC_CARBON && TARGET_RT_MAC_CFM	
	if (handleselectwindowDesc != nil)
		DisposeAEEventHandlerUPP(handleselectwindowDesc);
	
	DisposeComponentFunctionUPP(windowcandofunctionDesc);
	DisposeComponentFunctionUPP(handlewindoweventcommandDesc);
	DisposeComponentFunctionUPP(windowiscardcommandDesc);
	DisposeComponentFunctionUPP(closewindowcommandDesc);
	DisposeComponentFunctionUPP(windoweditcommandDesc);
	DisposeComponentFunctionUPP(windowopencommandDesc);
	#endif

	
	return (err == noErr);
	} /*removewindowsharinghandlers*/
	
static pascal ComponentResult windowsharingdispatch (register ComponentParameters *params, Handle storage) {
	
	/*
	2.1b8 dmb: added case for wsRunFromHandleCommand
	*/
	
	ComponentResult result = noErr;
	short what = (*params).what;
	
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
			
			#if !TARGET_API_MAC_CARBON
			SetComponentInstanceA5 ((ComponentInstance) self, selfa5);
			#endif

			if (newcomponentglobals (self, clienta5, &hglobals))
				SetComponentInstanceStorage ((ComponentInstance) self, (Handle) hglobals);
			else
				result = memFullErr;
			
			installwindowsharinghandlers ();
			
			/*
			initprocess ();
			*/
			
			RestoreA5 (clienta5);
			
			break;
			}
		
		case kComponentCloseSelect:
			disposecomponentglobals ((hdlcomponentglobals) storage);
			
			removewindowsharinghandlers ();
			
			/*
			processclose ();
			*/
			
			result = noErr; /***maybe look for shared windows & close them*/
			
			break;
		
		case kComponentCanDoSelect:
			result = CallComponentFunction (params, windowcandofunctionUPP);
			break;
		
		case kComponentVersionSelect:
			result = 0x04100100;
			
			break;
		
		case wsEventHandlerCommand:
			result = callosafunction (storage, params, handlewindoweventcommandUPP);
			
			break;
		
		case wsWindowIsCardCommand:
			result = CallComponentFunctionWithStorage (storage, params, windowiscardcommandUPP);
			
			break;
		
		case wsCloseWindowCommand:
			result = callosafunction (storage, params, closewindowcommandUPP);
			
			break;
		
		case wsEditCommand:
			result = callosafunction (storage, params, windoweditcommandUPP);
			
			break;
		
		case wsRunFromHandleCommand:
			result = callosafunction (storage, params, windowopencommandUPP);
			
			break;
		
		default:
			result = badComponentSelector;
			
			break;
		}
	
	return (result);
	} /*windowsharingdispatch*/


boolean initwindowsharingcomponent (void) {
	
	/*
	register the window sharing component.
	*/
	
	Component comp;
	
	#if !TARGET_API_MAC_CARBON
	RememberA5 ();
	#endif
	
	#if TARGET_API_MAC_CARBON && TARGET_RT_MAC_CFM
	windowcandofunctionDesc = NewComponentFunctionUPP(windowcandofunction, windowcandofunctionProcInfo);
	handlewindoweventcommandDesc = NewComponentFunctionUPP(handlewindoweventcommand, handlewindoweventcommandProcInfo);
	windowiscardcommandDesc = NewComponentFunctionUPP(windowiscardcommand, windowiscardcommandProcInfo);
	closewindowcommandDesc = NewComponentFunctionUPP(closewindowcommand, closewindowcommandProcInfo);
	windoweditcommandDesc = NewComponentFunctionUPP(windoweditcommand, windoweditcommandProcInfo);
	windowopencommandDesc = NewComponentFunctionUPP(windowopencommand, windowopencommandProcInfo);
	#endif
		
	
	comp = osaregistercomponent (wsComponentType, 0, &windowsharingdispatch, frontierstring, windowsharingcomponentstring);
	
	if (comp == nil)
		return (false);
	
	installwindowsharinghandlers (); /*we need to support them too*/
	
	return (true);
	} /*initwindowsharingcomponent*/


#else

#include <uisharing.h>
#include <uisinternal.h>
#include <land.h>
#include "ops.h"
#include "kb.h"
#include "memory.h"
#include "quickdraw.h"
#include "resources.h"
#include "shell.h"
#include "shellmenu.h"
#include "shellprivate.h"
#include "tablestructure.h"
#include "osainternal.h"
#include "osawindows.h"
#include <SetUpA5.h>



static WindowPtr osageteventwindow (EventRecord *ev) {
	
	WindowPtr w;
	WindowPtr wfront;
	
	wfront = FrontWindow ();
	
	w = nil; /*default*/
	
	switch ((*ev).what) { /*some events apply to windows other than the frontmost*/
		
		case nullEvent: 
			w = wfront;
			
			break;
		
		case keyDown: case autoKey: case keyUp:
			w = wfront;
			
			break;
		
		case mouseDown:	case mouseUp:
			FindWindow ((*ev).where, &w);
			
			break;
		
		case updateEvt: case activateEvt:
			w = (WindowPtr) (*ev).message;
			
			break;
		
		case jugglerEvt:
			w = wfront;
			
			break;
		
		default: /*we don't handle other events*/
			break;
		} /*switch*/
	
	return (w);
	} /*osageteventwindow*/


static pascal OSErr handleselectwindow (const AppleEvent *, AppleEvent *, UInt32); /*forward*/


#if !TARGET_RT_MAC_CFM || TARGET_API_MAC_CARBON

	#define handleselectwindowUPP handleselectwindow

#else

	static RoutineDescriptor handleselectwindowDesc = BUILD_ROUTINE_DESCRIPTOR (uppAEEventHandlerProcInfo, handleselectwindow);

	#define handleselectwindowUPP (&handleselectwindowDesc)

#endif


static boolean installwindowsharinghandlers (void) {
	
	OSErr err;
	
	err = AEInstallEventHandler ('SHUI', 'selw', handleselectwindowUPP, 0, false);
	
	return (err == noErr);
	} /*installwindowsharinghandlers*/


static boolean removewindowsharinghandlers (void) {
	
	OSErr err;
	
	err = AERemoveEventHandler ('SHUI', 'selw', handleselectwindowUPP, false);
	
	return (err == noErr);
	} /*removewindowsharinghandlers*/


static pascal ComponentResult handlewindoweventcommand (Handle hglobals, EventRecord *ev, tyWindowSharingGlobals *wsGlobals) {
	
	boolean flcloseallwindows = false;
	WindowPtr w;
	boolean fl = false;
	
	if (flosashutdown)
		removewindowsharinghandlers ();
	
	w = osageteventwindow (ev);
	
	if (!isshellwindow (w))
		return (false);
	
	if (!shellpushglobals (w))
		return (false);
	
	switch ((*ev).what) { /*some events apply to windows other than the frontmost*/
		
		case nullEvent: 
			shellidle ();
			
			shelladjustcursor ();
			
			break;
		
		case jugglerEvt: {
			tyjugglermessage *jmsg = (tyjugglermessage *) &(*ev).message;
			
			if ((*jmsg).eventtype == 1) /*suspend or resume subevent*/
				shellactivatewindow (w, (*jmsg).flresume);
			
			break;
			}
		
		case keyDown: case autoKey: case keyUp:
			setkeyboardstatus ((*ev).message, (*ev).what, (*ev).modifiers); 
			
			if (keyboardstatus.flcmdkey) {
				
				if (!(*shellglobals.cmdkeyfilterroutine) (keyboardstatus.chkb)) {
					
					fl = true;
					
					break;
					}
				
				if (textchar (keyboardstatus.chkb)) /*pass it up to menu mgr*/
					break;
				}
			
			/*fall through into default case*/
		
		default: /*we don't handle other events*/
			
			langcallbacks.errormessagecallback = &langerrordialog;
			
			fl = shellprocessevent (ev);
			
			break;
		} /*switch*/
	
	shellpopglobals ();
	
	(*wsGlobals).flcloseallwindows = flcloseallwindows; /****/
	
	/*
	processyield ();
	*/
	
	return (fl); /*handled the event -- or not*/
	} /*eventhandlercommand*/


static pascal ComponentResult windowiscardcommand (Handle hglobals, WindowPtr w) {
	
	return (isshellwindow (w));
	} /*windowiscardcommand*/


static pascal ComponentResult closewindowcommand (Handle hglobals, WindowPtr w) {
	
	return (shellclosewindow (w));
	} /*closewindowcommand*/


static pascal ComponentResult windoweditcommand (Handle hglobals, short editcmd) {
	
	register WindowPtr w;
	register boolean fl;
	
	w = FrontWindow ();
	
	if (!isshellwindow (w))
		return (false);
	
	if (!shellpushglobals (w))
		return (false);
	
	fl = shelleditcommand ((tyeditcommand) editcmd);
	
	shellpopglobals ();
	
	return (fl);
	} /*windoweditcommand*/


static pascal ComponentResult windowopencommand (Handle hglobals, Handle h, short flscripted, Str255 windowname, Point pt, uisEventCallback filter) {
	
	#pragma unused (flscripted, windowname, pt, filter)
	
	/*
	2.1b8 dmb: new routine so client can open window at will
	*/
	
	bigstring bsname;
	hdlhashtable htable; 
	tyvaluerecord val;
	boolean fl = false;
	hdlhashnode hnode;
	
	texthandletostring (h, bsname);
	
	pushhashtable (roottable); /*override storage table set up by callosafunction so langrunhandle will set up root*/
	
	if (langexpandtodotparams (bsname, &htable, bsname)) {
		
		if (htable == nil)
			langsearchpathlookup (bsname, &htable);
		
		if (hashtablelookup (htable, bsname, &val, &hnode)) {
			
			if (val.valuetype == externalvaluetype)
				fl = langzoomvalwindow (htable, bsname, val, true);
			}
		}
	
	pophashtable ();
	
	return (fl);
	} /*windowopencommand*/


static pascal ComponentResult windowcandofunction (short selector) {
	
	switch (selector) {
		
		case kComponentOpenSelect:
		case kComponentCloseSelect:
		case kComponentCanDoSelect:
		case kComponentVersionSelect:
		
		case wsEventHandlerCommand:
		case wsWindowIsCardCommand:
		case wsCloseWindowCommand:
		case wsEditCommand:
		case wsRunFromHandleCommand:
			return (true);
		}
	
	return (false);
	} /*windowcandofunction*/


static pascal OSErr handleselectwindow (const AppleEvent *event, AppleEvent *reply, UInt32 refcon) {
	
	/*
	handle the window select event in the current application context.
	
	in case we're in Frontier, dirty the window menu
	*/
	
	OSErr err;
	OSType type;
	long size;
	WindowPtr w;
	long curA5;
	
	err = AEGetParamPtr (event, '----', typeLongInteger, &type, (Ptr) &w, sizeof (w), &size);
	
	if (err == noErr) {
		
		SelectWindow (w);
		
		curA5 = SetUpAppA5 ();
		
		shellwindowmenudirty ();
		
		RestoreA5 (curA5);
		}
	
	return (err);
	} /*handleselectwindow*/


#if THINK_C || !TARGET_RT_MAC_CFM || TARGET_API_MAC_CARBON

	#define windowcandofunctionUPP	((ComponentFunctionUPP) windowcandofunction)
	#define handlewindoweventcommandUPP	((ComponentFunctionUPP) handlewindoweventcommand)
	#define windowiscardcommandUPP	((ComponentFunctionUPP) windowiscardcommand)
	#define closewindowcommandUPP	((ComponentFunctionUPP) closewindowcommand)
	#define windoweditcommandUPP	((ComponentFunctionUPP) windoweditcommand)
	#define windowopencommandUPP	((ComponentFunctionUPP) windowopencommand)

#else

	enum {
		windowcandofunctionProcInfo = kPascalStackBased
			 | RESULT_SIZE(SIZE_CODE(sizeof(OSAError)))
			 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(hdlcomponentglobals)))
			 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(short)))
			 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(short)))
	};
	enum {
		handlewindoweventcommandProcInfo = kPascalStackBased
			 | RESULT_SIZE(SIZE_CODE(sizeof(OSAError)))
			 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(hdlcomponentglobals)))
			 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(short)))
			 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(short)))
	};
	enum {
		windowiscardcommandProcInfo = kPascalStackBased
			 | RESULT_SIZE(SIZE_CODE(sizeof(OSAError)))
			 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(hdlcomponentglobals)))
			 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(short)))
			 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(short)))
	};
	enum {
		closewindowcommandProcInfo = kPascalStackBased
			 | RESULT_SIZE(SIZE_CODE(sizeof(OSAError)))
			 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(hdlcomponentglobals)))
			 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(short)))
			 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(short)))
	};
	enum {
		windoweditcommandProcInfo = kPascalStackBased
			 | RESULT_SIZE(SIZE_CODE(sizeof(OSAError)))
			 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(hdlcomponentglobals)))
			 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(short)))
			 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(short)))
	};
	enum {
		windowopencommandProcInfo = kPascalStackBased
			 | RESULT_SIZE(SIZE_CODE(sizeof(OSAError)))
			 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(hdlcomponentglobals)))
			 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(short)))
			 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(short)))
	};
	
	static RoutineDescriptor windowcandofunctionDesc = BUILD_ROUTINE_DESCRIPTOR (windowcandofunctionProcInfo, windowcandofunction);
	static RoutineDescriptor handlewindoweventcommandDesc = BUILD_ROUTINE_DESCRIPTOR (handlewindoweventcommandProcInfo, handlewindoweventcommand);
	static RoutineDescriptor windowiscardcommandDesc = BUILD_ROUTINE_DESCRIPTOR (windowiscardcommandProcInfo, windowiscardcommand);
	static RoutineDescriptor closewindowcommandDesc = BUILD_ROUTINE_DESCRIPTOR (closewindowcommandProcInfo, closewindowcommand);
	static RoutineDescriptor windoweditcommandDesc = BUILD_ROUTINE_DESCRIPTOR (windoweditcommandProcInfo, windoweditcommand);
	static RoutineDescriptor windowopencommandDesc = BUILD_ROUTINE_DESCRIPTOR (windowopencommandProcInfo, windowopencommand);
	
	#define windowcandofunctionUPP (&windowcandofunctionDesc)
	#define handlewindoweventcommandUPP (&handlewindoweventcommandDesc)
	#define windowiscardcommandUPP (&windowiscardcommandDesc)
	#define closewindowcommandUPP (&closewindowcommandDesc)
	#define windoweditcommandUPP (&windoweditcommandDesc)
	#define windowopencommandUPP (&windowopencommandDesc)

#endif

static pascal ComponentResult windowsharingdispatch (register ComponentParameters *params, Handle storage) {
	
	/*
	2.1b8 dmb: added case for wsRunFromHandleCommand
	*/
	
	ComponentResult result = noErr;
	short what = (*params).what;
	
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
			
			#if !TARGET_API_MAC_CARBON
			SetComponentInstanceA5 ((ComponentInstance) self, selfa5);
			#endif

			if (newcomponentglobals (self, clienta5, &hglobals))
				SetComponentInstanceStorage ((ComponentInstance) self, (Handle) hglobals);
			else
				result = memFullErr;
			
			installwindowsharinghandlers ();
			
			/*
			initprocess ();
			*/
			
			RestoreA5 (clienta5);
			
			break;
			}
		
		case kComponentCloseSelect:
			disposecomponentglobals ((hdlcomponentglobals) storage);
			
			removewindowsharinghandlers ();
			
			/*
			processclose ();
			*/
			
			result = noErr; /***maybe look for shared windows & close them*/
			
			break;
		
		case kComponentCanDoSelect:
			result = CallComponentFunction (params, windowcandofunctionUPP);
			
			break;
		
		case kComponentVersionSelect:
			result = 0x04100100;
			
			break;
		
		case wsEventHandlerCommand:
			result = callosafunction (storage, params, handlewindoweventcommandUPP);
			
			break;
		
		case wsWindowIsCardCommand:
			result = CallComponentFunctionWithStorage (storage, params, windowiscardcommandUPP);
			
			break;
		
		case wsCloseWindowCommand:
			result = callosafunction (storage, params, closewindowcommandUPP);
			
			break;
		
		case wsEditCommand:
			result = callosafunction (storage, params, windoweditcommandUPP);
			
			break;
		
		case wsRunFromHandleCommand:
			result = callosafunction (storage, params, windowopencommandUPP);
			
			break;
		
		default:
			result = badComponentSelector;
			
			break;
		}
	
	return (result);
	} /*windowsharingdispatch*/


boolean initwindowsharingcomponent (void) {
	
	/*
	register the window sharing component.
	*/
	
	Component comp;
	
	#if !TARGET_API_MAC_CARBON
	RememberA5 ();
	#endif
	
	comp = osaregistercomponent (wsComponentType, 0, &windowsharingdispatch, frontierstring, windowsharingcomponentstring);
	
	if (comp == nil)
		return (false);
	
	installwindowsharinghandlers (); /*we need to support them too*/
	
	return (true);
	} /*initwindowsharingcomponent*/


#endif
