
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
 
#include <uisharing.h>
#include <uisinternal.h>


#define glue static pascal ComponentResult


tyWindowSharingGlobals wsGlobals = {0, 0, false};

tyMenuSharingGlobals msGlobals = {0, 0};
	

Boolean uisDisposeSharedMenus (void); /*for compatibility with earlier menu sharing toolkits*/

Boolean uisIsSharedMenu (short);

void uisEnableSharedMenus (Boolean);

Boolean uisRunSharedMenuItem (short, short);



static Boolean HaveComponentManager (void) {

	long result;
	
	if (Gestalt (gestaltComponentMgr, &result) != noErr)
		return (false);
	
	return (result != 0);
	} /*HaveComponentManager*/


	

/* 2004-10-22 aradke: Not sure if this is the right thing to do for the Mach-O build,
		but at least it makes the link errors for _editGlue etc. go away
*/
#if TARGET_RT_MAC_CFM || TARGET_RT_MAC_MACHO

enum {
	uppCallComponentProcInfo = kPascalStackBased
			| RESULT_SIZE(kFourByteCode)
			| STACK_ROUTINE_PARAMETER(1, kFourByteCode)
};

extern UniversalProcPtr CallComponentUPP;

#pragma options align=mac68k

glue windoweventGlue (ComponentInstance comp, EventRecord *ev, tyWindowSharingGlobals *wsGlobals) {
	
	#define windoweventParamSize	 (sizeof (ev) + sizeof (wsGlobals))
	
	struct windoweventGluePB {
		unsigned char	componentFlags;
		unsigned char	componentParamSize;
		short componentWhat;
		tyWindowSharingGlobals *wsGlobals;
		EventRecord *ev;
		ComponentInstance	comp;
	};
	
	struct windoweventGluePB pb;
	
	pb.componentFlags = 0;
	pb.componentParamSize = windoweventParamSize;
	pb.componentWhat = wsEventHandlerCommand;
	pb.wsGlobals = wsGlobals;
	pb.ev = ev;
	pb.comp = comp;
	
	return CallComponentDispatch((ComponentParameters *)&pb);
		} /*windoweventGlue*/


glue windowiscardGlue (ComponentInstance comp, WindowPtr w) {
	
	#define windowiscardParamSize	 (sizeof (w))
	
	struct windowiscardGluePB {
		unsigned char	componentFlags;
		unsigned char	componentParamSize;
		short componentWhat;
		WindowPtr w;
		ComponentInstance	comp;
	};
	
	struct windowiscardGluePB pb;
	
	pb.componentFlags = 0;
	pb.componentParamSize = windowiscardParamSize;
	pb.componentWhat = wsWindowIsCardCommand;
	pb.w = w;
	pb.comp = comp;
	
	return CallComponentDispatch((ComponentParameters *)&pb);
	} /*windowiscardGlue*/
	
	
glue closewindowGlue (ComponentInstance comp, WindowPtr w) {
	
	#define closewindowParamSize	 (sizeof (w))
	
	struct closewindowGluePB {
		unsigned char	componentFlags;
		unsigned char	componentParamSize;
		short componentWhat;
		WindowPtr w;
		ComponentInstance	comp;
	};
	
	struct closewindowGluePB pb;
	
	pb.componentFlags = 0;
	pb.componentParamSize = closewindowParamSize;
	pb.componentWhat = wsCloseWindowCommand;
	pb.w = w;
	pb.comp = comp;
	
	return CallComponentDispatch((ComponentParameters *)&pb);
	} /*windowiscardGlue*/
	



glue runhandleGlue (ComponentInstance comp, Handle h, short flscriptedcard, Str255 windowname, Point pt, uisEventCallback filter) {
	
	#define runhandleParamSize	 (sizeof (h) + sizeof (flscriptedcard) + sizeof (windowname) + sizeof (pt) + sizeof (filter))
	
	struct runhandleGluePB {
		unsigned char	componentFlags;
		unsigned char	componentParamSize;
		short componentWhat;
		uisEventCallback filter;
		Point pt;
		unsigned char *windowname;
		short flscriptedcard;
		Handle h;
		ComponentInstance	comp;
	};
	
	struct runhandleGluePB pb;
	
	pb.componentFlags = 0;
	pb.componentParamSize = runhandleParamSize;
	pb.componentWhat = wsRunFromHandleCommand;
	pb.filter = filter;
	pb.pt = pt;
	pb.windowname = windowname;
	pb.flscriptedcard = flscriptedcard;
	pb.h = h;
	pb.comp = comp;
	
	return CallComponentDispatch((ComponentParameters *)&pb);
	} /*runhandleGlue*/


glue runmodalhandleGlue (ComponentInstance comp, Handle h, short flscriptedcard, Str255 windowname, Point pt, uisEventCallback filter) {
	
	#define runmodalhandleParamSize	 (sizeof (h) + sizeof (flscriptedcard) + sizeof (windowname) + sizeof (pt) + sizeof (filter))
	
	struct runmodalhandleGluePB {
		unsigned char	componentFlags;
		unsigned char	componentParamSize;
		short componentWhat;
		uisEventCallback filter;
		Point pt;
		unsigned char *windowname;
		short flscriptedcard;
		Handle h;
		ComponentInstance	comp;
	};
	
	struct runmodalhandleGluePB pb;
	
	pb.componentFlags = 0;
	pb.componentParamSize = runmodalhandleParamSize;
	pb.componentWhat = wsRunModalHandleCommand;
	pb.filter = filter;
	pb.pt = pt;
	pb.windowname = windowname;
	pb.flscriptedcard = flscriptedcard;
	pb.h = h;
	pb.comp = comp;
	
	return CallComponentDispatch((ComponentParameters *)&pb);
	} /*runmodalhandleGlue*/




glue editGlue (ComponentInstance comp, short editcommand) {
	
	#define editParamSize	 (sizeof (editcommand))
	
	struct editGluePB {
		unsigned char	componentFlags;
		unsigned char	componentParamSize;
		short componentWhat;
		short editcommand;
		ComponentInstance	comp;
	};
	
	struct editGluePB pb;
	
	pb.componentFlags = 0;
	pb.componentParamSize = editParamSize;
	pb.componentWhat = wsEditCommand;
	pb.editcommand = editcommand;
	pb.comp = comp;
	
	return CallComponentDispatch((ComponentParameters *)&pb);
	} /*editGlue*/




#pragma options align=reset

#elif 0 // defined (THINK_C)

glue windoweventGlue (ComponentInstance comp, EventRecord *ev, tyWindowSharingGlobals *wsGlobals) 

	= ComponentCallNow (wsEventHandlerCommand, sizeof (EventRecord *) + sizeof (tyWindowSharingGlobals *)); 
	/*windoweventGlue*/


glue windowiscardGlue (ComponentInstance comp, WindowPtr w) 
	
	= ComponentCallNow (wsWindowIsCardCommand, sizeof (WindowPtr)); 
	/*windowiscardGlue*/
	
	
glue closewindowGlue (ComponentInstance comp, WindowPtr w) 
	
	= ComponentCallNow (wsCloseWindowCommand, sizeof (WindowPtr)); 
	/*windowiscardGlue*/
	

glue runhandleGlue (ComponentInstance comp, Handle h, short flscriptedcard, Str255 windowname, Point pt, uisEventCallback filter) 
	
	= ComponentCallNow (wsRunFromHandleCommand, sizeof (Handle) + sizeof (short) + sizeof (char *) + sizeof (Point) + sizeof (uisEventCallback)); 
	/*runhandleGlue*/


glue runmodalhandleGlue (ComponentInstance comp, Handle h, short flscriptedcard, Str255 windowname, Point pt, uisEventCallback filter) 
	
	= ComponentCallNow (wsRunModalHandleCommand, sizeof (Handle) + sizeof (short) + sizeof (char *) + sizeof (Point) + sizeof (uisEventCallback)); 
	/*runmodalhandleGlue*/


glue setobjectvalueGlue (ComponentInstance comp, Handle hcard, Str255 name, Handle value) 
	
	= ComponentCallNow (wsSetObjectValueCommand, sizeof (Handle) + sizeof (char *) + sizeof (Handle)); 
	/*setobjectvalueGlue*/
	

glue getobjectvalueGlue (ComponentInstance comp, Handle hcard, Str255 name, Handle *value) 
	
	= ComponentCallNow (wsGetObjectValueCommand, sizeof (Handle) + sizeof (char *) + sizeof (Handle *)); 
	/*getobjectvalueGlue*/
	

glue getobjecthandleGlue (ComponentInstance comp, Handle hcard, Str255 name, Handle *h) 
	
	= ComponentCallNow (wsGetObjectHandleCommand, sizeof (Handle) + sizeof (char *) + sizeof (Handle *)); 
	/*getobjecthandleGlue*/
	

glue recalcGlue (ComponentInstance comp, Handle h) 
	
	= ComponentCallNow (wsRecalcObjectCommand, sizeof (Handle)); 
	/*recalcGlue*/


glue editGlue (ComponentInstance comp, short editcommand) 
	
	= ComponentCallNow (wsEditCommand, sizeof (short)); 
	/*editGlue*/


glue updateGlue (ComponentInstance comp, Handle h) 
	
	= ComponentCallNow (wsUpdateCommand, sizeof (Handle)); 
	/*updateGlue*/

#else

glue windoweventGlue (ComponentInstance comp, EventRecord *ev, tyWindowSharingGlobals *wsGlobals) 
	
	ComponentCallNow (wsEventHandlerCommand, sizeof (EventRecord *) + sizeof (tyWindowSharingGlobals *)); 
	/*windoweventGlue*/


glue windowiscardGlue (ComponentInstance comp, WindowPtr w) 
	
	ComponentCallNow (wsWindowIsCardCommand, sizeof (WindowPtr)); 
	/*windowiscardGlue*/
	
	
glue closewindowGlue (ComponentInstance comp, WindowPtr w) 
	
	ComponentCallNow (wsCloseWindowCommand, sizeof (WindowPtr)); 
	/*windowiscardGlue*/
	



glue runhandleGlue (ComponentInstance comp, Handle h, short flscriptedcard, Str255 windowname, Point pt, uisEventCallback filter) 
	
	ComponentCallNow (wsRunFromHandleCommand, sizeof (Handle) + sizeof (short) + sizeof (char *) + sizeof (Point) + sizeof (uisEventCallback)); 
	/*runhandleGlue*/


glue runmodalhandleGlue (ComponentInstance comp, Handle h, short flscriptedcard, Str255 windowname, Point pt, uisEventCallback filter) 
	
	ComponentCallNow (wsRunModalHandleCommand, sizeof (Handle) + sizeof (short) + sizeof (char *) + sizeof (Point) + sizeof (uisEventCallback)); 
	/*runmodalhandleGlue*/


glue setobjectvalueGlue (ComponentInstance comp, Handle hcard, Str255 name, Handle value) 
	
	ComponentCallNow (wsSetObjectValueCommand, sizeof (Handle) + sizeof (char *) + sizeof (Handle)); 
	/*setobjectvalueGlue*/
	

glue getobjectvalueGlue (ComponentInstance comp, Handle hcard, Str255 name, Handle *value) 
	
	ComponentCallNow (wsGetObjectValueCommand, sizeof (Handle) + sizeof (char *) + sizeof (Handle *)); 
	/*getobjectvalueGlue*/
	

glue getobjecthandleGlue (ComponentInstance comp, Handle hcard, Str255 name, Handle *h) 
	
	ComponentCallNow (wsGetObjectHandleCommand, sizeof (Handle) + sizeof (char *) + sizeof (Handle *)); 
	/*getobjecthandleGlue*/
	

glue recalcGlue (ComponentInstance comp, Handle h) 
	
	ComponentCallNow (wsRecalcObjectCommand, sizeof (Handle)); 
	/*recalcGlue*/


glue editGlue (ComponentInstance comp, short editcommand) 
	
	ComponentCallNow (wsEditCommand, sizeof (short)); 
	/*editGlue*/


glue updateGlue (ComponentInstance comp, Handle h) 
	
	ComponentCallNow (wsUpdateCommand, sizeof (Handle)); 
	/*updateGlue*/

#endif


Boolean uisHandleEvent (EventRecord *ev, Boolean *flcloseallwindows) {
	
	if (wsGlobals.windowserver != 0) {
		
		Boolean fl;
	
		wsGlobals.flcloseallwindows = false;
	
		fl = windoweventGlue (wsGlobals.windowserver, ev, &wsGlobals);
	
		*flcloseallwindows = wsGlobals.flcloseallwindows;
		
		if (fl)
			return (true); /*we handled the event*/
		}
	
	
	return (false); /*client app should handle the event*/
	} /*uisHandleEvent*/
	

Boolean uisIsSharedWindow (WindowPtr w) {
	
	if (wsGlobals.windowserver == 0)
		return (false);
		
	if (w == nil)
		return (false);
		
	return (windowiscardGlue (wsGlobals.windowserver, w));
	} /*uisIsSharedWindow*/
	
	
Boolean uisCloseSharedWindow (WindowPtr w) {
	
	if (wsGlobals.windowserver == 0)
		return (false);
		
	if (!uisIsSharedWindow (w))
		return (false); /*we didn't close it*/
		
	return (closewindowGlue (wsGlobals.windowserver, w)); 
	} /*uisCloseSharedWindow*/
	
	
void uisCloseAllSharedWindows (void) {
	
	WindowPtr w = FrontWindow ();
	WindowPtr wnext;
	
	while (w != nil) {
		
		wnext = GetNextWindow(w);
		
		if (uisIsSharedWindow (w))
			uisCloseSharedWindow (w);
		
		w = wnext; 
		} /*while*/
	} /*uisCloseAllSharedWindows*/
	

Boolean uisOpenHandle (Handle h, Boolean flscriptedcard, Str255 windowname, short top, short pleft, uisEventCallback filter) {
	
	Point pt;
	
	pt.h = pleft; pt.v = top;
	
	if (wsGlobals.windowserver == 0)
		return (false);
		
	wsGlobals.errorcode = runhandleGlue (wsGlobals.windowserver, h, flscriptedcard, windowname, pt, filter);
	
	return (wsGlobals.errorcode == noErr);
	} /*uisOpenHandle*/
	
	
Boolean uisOpenCardResource (short id, Boolean flscriptedcard, Str255 windowname, short top, short pleft, uisEventCallback filter) {
	
	Handle h;
	
	h = GetResource ('CARD', id);
	
	if (h == nil) {
		
		wsGlobals.errorcode = ResError ();
		
		return (false);
		}
		
	DetachResource (h);
	
	return (uisOpenHandle (h, flscriptedcard, windowname, top, pleft, filter));
	} /*uisOpenCardResource*/
	
	
Boolean uisRunModalHandle (Handle h, Boolean flscriptedcard, Str255 windowname, short top, short pleft, uisEventCallback filter) {
	
	Point pt;
	
	pt.h = pleft; pt.v = top;
	
	if (wsGlobals.windowserver == 0)
		return (false);
		
	wsGlobals.errorcode = runmodalhandleGlue (wsGlobals.windowserver, h, flscriptedcard, windowname, pt, filter);
	
	return (wsGlobals.errorcode == noErr);
	} /*uisRunModalHandle*/
	
	
Boolean uisRunModalResource (short id, Boolean flscriptedcard, Str255 windowname, short top, short pleft, uisEventCallback filter) {
	
	Handle h;
	
	h = GetResource ('CARD', id);
	
	if (h == nil) {
		
		wsGlobals.errorcode = ResError ();
		
		return (false);
		}
		
	DetachResource (h);
	
	return (uisRunModalHandle (h, flscriptedcard, windowname, top, pleft, filter));
	} /*uisRunModalResource*/
	

Boolean uisEdit (short editcommand) {
		
	if (wsGlobals.windowserver == 0)
		return (false);
		
	return (editGlue (wsGlobals.windowserver, editcommand));
	} /*uisEdit*/
	

Boolean uisInit (ProcPtr pcallback, short idinsertafter, OSType idclientapp, unsigned short bitarray) {
	
	/*
	initialize window sharing and menu sharing. 
	
	the pcallback is a routine that receives a Str255 as a parameter. it should
	display the string in a modal dialog box and wait for the user to click 
	on OK. it's used for reporting script compiling or runtime errors. pass in
	nil if you want errors to not be reported to the user (not a great thing
	to do to your users!).
	
	idinsertafter tells menu sharing where to begin allocating new menu ids.
	it's the same parameter that was passed to CheckSharedMenus in previous
	incarnations of menu sharing.
	
	idclientapp is the 4-character identifier of the client app. if it's
	'wxyz', then we'll get the shared menus at system.menubars.wxyz. If you
	pass in 0, the menu sharing server uses the Process Manager to get your
	id, as the creator id of the running application. This feature is supported
	in the UI sharing API only, as of June 12, 1993. A modification needs to
	be made to Frontier to support this fully.
	
	DW 8/18/93: added a 16-bit bit-array that allows the caller to modify
	the behavior of UI Sharing. if it's set to 0 (the usual case) you get 
	all the features.
	*/
	
	Boolean flmenusharing = true, flwindowsharing = true;
	
	if (bitarray & noMenuSharing)
		flmenusharing = false;
		
	if (bitarray & noWindowSharing)
		flwindowsharing = false;
	
	wsGlobals.windowserver = 0;
	
	msGlobals.menuserver = 0;
	
	if ((!flmenusharing) && (!flwindowsharing))
		return (true);
	
	if (!HaveComponentManager ())
		return (false);
	
	if (flwindowsharing) {
	
		
		
		wsGlobals.windowserver = OpenDefaultComponent (wsComponentType, wsComponentSubType);
		}
	
		
	return (true);
	
	} /*uisInit*/
	
	
void uisClose (void) {

	if (wsGlobals.windowserver != 0)		
		CloseComponent (wsGlobals.windowserver);
	
	} /*uisClose*/
	
	
	
	