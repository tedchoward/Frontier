
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

#include "mac.h"
#include "quickdraw.h"
#include "cursor.h"
#include "scrollbar.h"
#include "menu.h"
#include "mouse.h"
#include "frontierwindows.h"
#include "kb.h"
#include "shell.h"
#include "shellbuttons.h"
#include "shellhooks.h"
#include "shellmenu.h"
#include "shellprivate.h"



#ifdef MACVERSION
static void shellhandlecontent (Point pt) {
	
	register WindowPtr w = shellwindow;
	register hdlwindowinfo h = shellwindowinfo;
	hdlscrollbar ctrl;
	short part;
	tyclickflags flags = clicknormal;
	
	if (pointinrect (pt, (**h).buttonsrect)) {
		
		shellbuttonhit (pt);
		
		return;
		}
	
	if (findscrollbar (pt, w, &ctrl, &part)) { /*mouse hit in a scrollbar*/
	
		register hdlscrollbar scrollbar = (**h).vertscrollbar;
		
		if (ctrl == scrollbar) {
			
			shellscroll (true, scrollbar, part, pt);
			
			return;
			}
			
		scrollbar = (**h).horizscrollbar;
		
		if (ctrl == scrollbar) {
			
			shellscroll (false, scrollbar, part, pt);
			
			return;
			}
		}
		
	setkeyboardstatus (shellevent.message, shellevent.what, shellevent.modifiers); /*allow callback to work with global keyboard record*/
	
	if (keyboardstatus.flshiftkey)
		flags |= clickextend;

	if (keyboardstatus.flcontrolkey)
		flags |= clickdiscontiguous;
	
	if (keyboardstatus.floptionkey)
		flags |= clickoption;
	
	if (keyboardstatus.flcmdkey)
		flags |= clickcommand;
	
	if (mousestatus.fldoubleclick)
		flags |= clickwords;
	
	if (ismouseleftclick())
		(*shellglobals.mouseroutine) (pt, flags);

	else if (ismouserightclick())
		(*shellglobals.rmouseroutine) (pt, flags);
	
	else if (ismousecenterclick())
		(*shellglobals.cmouseroutine) (pt, flags);
	
	else if (ismousewheelclick())
		(*shellglobals.wmouseroutine) (pt, flags);
	} /*shellhandlecontent*/
	

static void shellhandlegrow (Point pt, WindowPtr wgrow) {
	
	register WindowPtr w = wgrow;
	register long x;
	Rect boundsrect;
	Rect r;
	
	getdesktopbounds (&r);
	
	boundsrect.left = config.rmin.right - config.rmin.left; /*minimum window width*/
	
	boundsrect.right = r.right - r.left - 5; /*maximum window width*/
	
	boundsrect.top = config.rmin.bottom - config.rmin.top; /*minimum window height*/
	
	boundsrect.bottom = r.bottom - r.top - 5; /*maximum window height*/
	
	x = GrowWindow (w, pt, &boundsrect);
	
	if (x) {
		
		sizewindow (w, LoWord (x), HiWord (x));
		
		shelladjustaftergrow (w);
		}
	} /*shellhandlegrow*/


static void shellhandlezoom (EventRecord ev, WindowPtr wzoom, short part) {
	
	register WindowPtr w = wzoom;
	
	assert (w == shellwindow);
	
	if (TrackBox (w, ev.where, part))
		(*shellglobals.zoomwindowroutine) (shellwindowinfo, part == inZoomIn);
	} /*shellhandlezoom*/


void shellhandledrag (EventRecord ev, WindowPtr w) {
	
	/*
	7/25/91 dmb: if the mouse isn't still down, make sure that the 
	window is brought to the front, unless the command key is down.
	
	note that mouse up event is created before calling DragWindow to 
	avoid accidental double-clicks when trying to move windows around
	*/
	
	Rect r;
	
	ev.when = gettickcount ();
	
	mouseup (ev.when, ev.where.h, ev.where.v, ev.what); /*DragWindow will consume mouseup; create one to enable double-click*/
	
	getdesktopbounds (&r);
	
	insetrect (&r, dragscreenmargin, dragscreenmargin);
	
	if (mousestilldown ())
		DragWindow (w, ev.where, &r);
	
	else {
		
		if (!keyboardstatus.flcmdkey)
			windowbringtofront (w);
		}
	} /*shellhandledrag*/

#endif

void shellhandlemouseup (void) {
	mouseup (shellevent.when, shellevent.where.h, shellevent.where.v, shellevent.what); /*call mouse.c routine to record info*/
	} /*shellhandlemouseup*/

void shellhandlemouse (void) {
	
	/*
	1/30/91 dmb: moved shellcalleventhooks calls and test for nil macwindow 
	into new cases in first switch statement.  much simpler; hopefully correct.
	
	5/27/92 dmb: only call the titleclickcallback when the window is frontmost
	*/
	
	Point mousept;
	short windowpart;
	WindowPtr macwindow;
	register WindowPtr w;
#ifdef WIN95VERSION		
	tyclickflags flags = clicknormal;
#endif

	#ifdef MACVERSION
		Point localpt;
	#endif

	mousedown (shellevent.when, shellevent.where.h, shellevent.where.v, shellevent.what); /*call mouse.c routine to record info*/
	
	mousept = shellevent.where;

	findmousewindow (mousept, &macwindow, &windowpart);

#ifdef WIN95VERSION		
	macwindow = (WindowPtr) shellevent.hwnd;

	if (!isshellwindow (macwindow))	// 4.0b6, 2/23/96 dmb
		return;
	
	if (!shellcalleventhooks (&shellevent, macwindow))
		return;

	w = macwindow;

	shellpushglobals (w); /*following mouse operations assume globals are pushed*/
	
	if ((macwindow != getfrontwindow ())) {
		
		windowbringtofront (macwindow);
		
		if (!config.fldontconsumefrontclicks) {
			
			mousedoubleclickdisable (); /*click can't be part of a doubleclick*/
			
			goto exit; /*the mouse click is consumed by the windowbringtofront operation*/
			}
		}

	setkeyboardstatus (shellevent.message, shellevent.what, shellevent.modifiers); /*allow callback to work with global keyboard record*/
	
	if (keyboardstatus.flshiftkey)
		flags |= clickextend;

	if (keyboardstatus.flcontrolkey)
		flags |= clickdiscontiguous;
	
	if (keyboardstatus.floptionkey)
		flags |= clickoption;
	
	if (keyboardstatus.flcmdkey)
		flags |= clickcommand;
	
	if (mousestatus.fldoubleclick)
		flags |= clickwords;
	
	if (pointinrect (mousept, (**shellwindowinfo).buttonsrect)) {
		
		shellbuttonhit (mousept);
		
		goto exit;
		}

	switch (shellevent.part) {
		case HTCLIENT:
			if (ismouseleftclick())
				(*shellglobals.mouseroutine) (mousept, flags);

			else if (ismouserightclick())
				(*shellglobals.rmouseroutine) (mousept, flags);
			
			else if (ismousecenterclick())
				(*shellglobals.cmouseroutine) (mousept, flags);
			
			else if (ismousewheelclick())
				(*shellglobals.wmouseroutine) (mousept, flags);
			break;

		case HTCAPTION:
			if (macwindow == getfrontwindow ())
				if ((*shellglobals.titleclickroutine) (mousept)) /*click was consumed*/
					break;
			break;

		case HTBORDER:
		case HTBOTTOM:
		case HTBOTTOMLEFT:
		case HTBOTTOMRIGHT:
		case HTGROWBOX:
		case HTHSCROLL:
		case HTLEFT:
		case HTMENU:
		case HTREDUCE:
		case HTRIGHT:
		case HTSYSMENU:
		case HTTOP:
		case HTTOPLEFT:
		case HTTOPRIGHT:
		case HTVSCROLL:
		case HTZOOM:
			break;

		default:
			break;
		}

#endif

#ifdef MACVERSION	
	switch (windowpart) { /*operations that don't require globals pushed*/
		
		case inDesk:
			return;
		
		case inSysWindow:
			//Code change by Timothy Paustian Friday, June 16, 2000 3:07:31 PM
			//Changed to Opaque call for Carbon
			//not used in carbon
			#if !TARGET_API_MAC_CARBON
			SystemClick (&shellevent, macwindow); 
			#endif

			return;
		
		case inMenuBar:
			assert (macwindow == nil);
			
			frontshellwindow (&macwindow);
			
			break;
		
		default:
			assert (macwindow != nil);
			
			if (!isshellwindow (macwindow))	// 4.0b6, 2/23/96 dmb
				return;
			
			if (!shellcalleventhooks (&shellevent, macwindow))
				return;
			
			break;
		} /*switch*/
	
	w = macwindow; /*copy into register*/
	
	shellpushglobals (w); /*following mouse operations assume globals are pushed*/
	
	#if TARGET_API_MAC_CARBON == 1
	
		/*On OS X, some operations don't require the window to come to front. PBS 7.0b53.*/

		if ((macwindow != getfrontwindow ()) && (windowpart != inDrag) && (windowpart != inMenuBar)
			&& (windowpart != inGoAway) && (windowpart != inZoomOut) && (windowpart != inZoomIn)
			&& (windowpart != inProxyIcon)) {
	
	#else
	
		if ((macwindow != getfrontwindow ()) && (windowpart != inDrag) && (windowpart != inMenuBar)) {
	
	#endif
		
		windowbringtofront (macwindow);
		
		if (!config.fldontconsumefrontclicks) {
			
			mousedoubleclickdisable (); /*click can't be part of a doubleclick*/
			
			goto exit; /*the mouse click is consumed by the windowbringtofront operation*/
			}
		}
	
	setkeyboardstatus (shellevent.message, shellevent.what, shellevent.modifiers); /*fill keyboardstatus record with modifier key info*/

	localpt = mousept;
	
	globaltolocalpoint (w, &localpt);
	
	switch (windowpart) {
	
		case inGrow:
			/*
			if (!shellcalleventhooks (&shellevent, w))
				break;
			*/
			
			if (config.flgrowable) {
				
				shellhandlegrow (mousept, w);
				
				break;
				}
			
			/*window not growable -- fall through to content-handling code*/
			
		case inContent: 
			/*
			if (!shellcalleventhooks (&shellevent, w))
				break;
			*/
			shellhandlecontent (localpt); 
			
			break;
		
		case inMenuBar: 
			shellupdatemenus (); /*be sure the menus are properly checked and highlighted*/
			
			setcursortype (cursorisarrow); /*arrow cursor is more appropriate here*/
			
			shellhandlemenu (trackmenu (mousept)); 
			
			break;
		
		case inDrag:
			/*
			if (!shellcalleventhooks (&shellevent, w))
				break;
			*/
			
			if (macwindow == getfrontwindow ())
				if ((*shellglobals.titleclickroutine) (localpt)) /*click was consumed*/
					break;
			
			shellhandledrag (shellevent, w);
			
			break;
			
		case inGoAway:
			/*
			if (!shellcalleventhooks (&shellevent, w))
				break;
			*/
			
			if (TrackGoAway (w, mousept)) 
				if (keyboardstatus.floptionkey)
					shellcloseall (w, true);
				else
					shellclose (w, true);
					
			break;
		
		case inZoomOut: case inZoomIn:
			/*
			if (!shellcalleventhooks (&shellevent, w))
				break;
			*/
			
			shellhandlezoom (shellevent, w, windowpart);
			
			break;
			
		#if TARGET_API_MAC_CARBON == 1
		
			case inProxyIcon: /*7.0b50: proxy icons are draggable in OS X*/
			
				TrackWindowProxyDrag (w, mousept);
				
				break;
		#endif
		} /*switch*/
#endif
	
	exit:
	
	shellpopglobals ();
	} /*shellhandlemouse*/


boolean shellcheckautoscroll (Point pt, boolean flhoriz, tydirection *dir) {
	
	return (mousecheckautoscroll (pt, (**shellwindowinfo).contentrect, flhoriz, dir));
	} /*shellcheckautoscroll*/


#if TARGET_API_MAC_CARBON == 1

static pascal OSStatus mousewheelhandler (EventHandlerCallRef nextHandler, EventRef theEvent, void* userData) {
	
	/*
	7.1b24 PBS: called from the system when the mouse wheel is moved.
	*/
	
	#pragma unused(nextHandler) /*happy compiler*/
	#pragma unused(userData)
	
	SInt32 delta;
	OSErr ec = eventNotHandledErr;
	WindowPtr w;
	tydirection dir = down;
		
	if (GetEventParameter (theEvent, kEventParamMouseWheelDelta, typeLongInteger, NULL, sizeof (delta), NULL, &delta) != noErr)
		return (ec);

	w = getfrontwindow ();
	
	shellpushglobals (w);
	
	if (delta < 0)
		dir = up;
		
	(*shellglobals.scrollroutine) (dir, false, abs (delta) * 2);
	
	shellpopglobals ();

	return (noErr);
	} /*dockmenuhandler*/


static void shellinstallmousewheelhandler (void) {
	
	EventTypeSpec myevents = {kEventClassMouse, kEventMouseWheelMoved};
	
	InstallApplicationEventHandler (NewEventHandlerUPP (mousewheelhandler), 1, &myevents, 0, NULL);	
	} /*dockmenuinstallhandler*/

#endif

void initmouse (void) {
	
	#if TARGET_API_MAC_CARBON == 1
	
		shellinstallmousewheelhandler (); /*7.1b24 PBS: install mouse wheel support on OS X.*/
		
	#endif
	} /*initmouse*/