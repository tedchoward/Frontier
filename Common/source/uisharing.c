
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

#include <AppleEvents.h>
#include <Components.h> 
#include <Gestalt.h>
#include <Processes.h>
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


static Boolean isFrontProcess () {
	
	ProcessSerialNumber currentprocess, frontprocess;
	Boolean fl;
	
	GetCurrentProcess (&currentprocess);
	
	GetFrontProcess (&frontprocess);
	
	SameProcess (&currentprocess, &frontprocess, &fl);
	
	return (fl);
	} /*isFrontProcess*/
	

#if __powerc

enum {
	uppCallComponentProcInfo = kPascalStackBased
			| RESULT_SIZE(kFourByteCode)
			| STACK_ROUTINE_PARAMETER(1, kFourByteCode)
};

extern UniversalProcPtr CallComponentUPP;

glue windoweventGlue (ComponentInstance comp, EventRecord *ev, tyWindowSharingGlobals *wsGlobals) {
	
	#define windoweventParamSize	 (sizeof (ev) + sizeof (wsGlobals))
	
	#ifdef powerc
		#pragma options align=mac68k
	#endif
	struct windoweventGluePB {
		unsigned char	componentFlags;
		unsigned char	componentParamSize;
		short componentWhat;
		tyWindowSharingGlobals *wsGlobals;
		EventRecord *ev;
		ComponentInstance	comp;
	};
	#ifdef powerc
		#pragma options align=reset
	#endif
	
	struct windoweventGluePB pb;
	
	pb.componentFlags = 0;
	pb.componentParamSize = windoweventParamSize;
	pb.componentWhat = wsEventHandlerCommand;
	pb.wsGlobals = wsGlobals;
	pb.ev = ev;
	pb.comp = comp;
	
	#if TARGET_API_MAC_CARBON == 1
	return CallComponentDispatch(&pb);
	#else	
	return CallUniversalProc(CallComponentUPP, uppCallComponentProcInfo, &pb);
	#endif
		} /*windoweventGlue*/


glue windowiscardGlue (ComponentInstance comp, WindowPtr w) {
	
	#define windowiscardParamSize	 (sizeof (w))
	
	#ifdef powerc
		#pragma options align=mac68k
	#endif
	struct windowiscardGluePB {
		unsigned char	componentFlags;
		unsigned char	componentParamSize;
		short componentWhat;
		WindowPtr w;
		ComponentInstance	comp;
	};
	#ifdef powerc
		#pragma options align=reset
	#endif
	
	struct windowiscardGluePB pb;
	
	pb.componentFlags = 0;
	pb.componentParamSize = windowiscardParamSize;
	pb.componentWhat = wsWindowIsCardCommand;
	pb.w = w;
	pb.comp = comp;
	
	#if TARGET_API_MAC_CARBON == 1
	return CallComponentDispatch(&pb);
	#else	
	return CallUniversalProc(CallComponentUPP, uppCallComponentProcInfo, &pb);
	#endif
	} /*windowiscardGlue*/
	
	
glue closewindowGlue (ComponentInstance comp, WindowPtr w) {
	
	#define closewindowParamSize	 (sizeof (w))
	
	#ifdef powerc
		#pragma options align=mac68k
	#endif
	struct closewindowGluePB {
		unsigned char	componentFlags;
		unsigned char	componentParamSize;
		short componentWhat;
		WindowPtr w;
		ComponentInstance	comp;
	};
	#ifdef powerc
		#pragma options align=reset
	#endif
	
	struct closewindowGluePB pb;
	
	pb.componentFlags = 0;
	pb.componentParamSize = closewindowParamSize;
	pb.componentWhat = wsCloseWindowCommand;
	pb.w = w;
	pb.comp = comp;
	
	#if TARGET_API_MAC_CARBON == 1
	return CallComponentDispatch(&pb);
	#else	
	return CallUniversalProc(CallComponentUPP, uppCallComponentProcInfo, &pb);
	#endif
	} /*windowiscardGlue*/
	

#ifndef isFrontier
	
	glue initsharedmenusGlue (ComponentInstance comp) {
	
		#define initsharedmenusParamSize	 (0L)
		
		#ifdef powerc
			#pragma options align=mac68k
		#endif
		struct initsharedmenusGluePB {
			unsigned char	componentFlags;
			unsigned char	componentParamSize;
			short componentWhat;
			ComponentInstance	comp;
		};
		#ifdef powerc
			#pragma options align=reset
		#endif
		
		struct initsharedmenusGluePB pb;
		
		pb.componentFlags = 0;
		pb.componentParamSize = initsharedmenusParamSize;
		pb.componentWhat = msInitSharedMenusCommand;
		pb.comp = comp;
		
		#if TARGET_API_MAC_CARBON == 1
		return CallComponentDispatch(&pb);
		#else	
		return CallUniversalProc(CallComponentUPP, uppCallComponentProcInfo, &pb);
		#endif
		} 	/*initsharedmenusGlue*/
		
	
	glue sharedmenuhitGlue (ComponentInstance comp, short idmenu, short iditem, Boolean *flshareditem) {
	
		#define sharedmenuhitParamSize	 (sizeof (idmenu) + sizeof (iditem) + sizeof (flshareditem))
		
		#ifdef powerc
			#pragma options align=mac68k
		#endif
		struct sharedmenuhitGluePB {
			unsigned char	componentFlags;
			unsigned char	componentParamSize;
			short componentWhat;
			Boolean *flshareditem;
			short iditem;
			short idmenu;
			ComponentInstance	comp;
		};
		#ifdef powerc
			#pragma options align=reset
		#endif
		
		struct sharedmenuhitGluePB pb;
		
		pb.componentFlags = 0;
		pb.componentParamSize = sharedmenuhitParamSize;
		pb.componentWhat = msSharedMenuHitCommand;
		pb.flshareditem = flshareditem;
		pb.iditem = iditem;
		pb.idmenu = idmenu;
		pb.comp = comp;
		
		#if TARGET_API_MAC_CARBON == 1
		return CallComponentDispatch(&pb);
		#else	
		return CallUniversalProc(CallComponentUPP, uppCallComponentProcInfo, &pb);
		#endif
		} 	/*sharedmenuhitGlue*/
		
	
	glue sharedscriptrunningGlue (ComponentInstance comp, Boolean *flrunning) {
	
		#define sharedscriptrunningParamSize	 (sizeof (flrunning))
		
		#ifdef powerc
			#pragma options align=mac68k
		#endif
		struct sharedscriptrunningGluePB {
			unsigned char	componentFlags;
			unsigned char	componentParamSize;
			short componentWhat;
			Boolean *flrunning;
			ComponentInstance	comp;
		};
		#ifdef powerc
			#pragma options align=reset
		#endif
		
		struct sharedscriptrunningGluePB pb;
		
		pb.componentFlags = 0;
		pb.componentParamSize = sharedscriptrunningParamSize;
		pb.componentWhat = msSharedScriptRunningCommand;
		pb.flrunning = flrunning;
		pb.comp = comp;
		
		#if TARGET_API_MAC_CARBON == 1
		return CallComponentDispatch(&pb);
		#else	
		return CallUniversalProc(CallComponentUPP, uppCallComponentProcInfo, &pb);
		#endif
		} 	/*sharedscriptrunningGlue*/
		
	
	glue cancelsharedscriptGlue (ComponentInstance comp) {
	
		#define cancelsharedscriptParamSize	 (0L)
		
		#ifdef powerc
			#pragma options align=mac68k
		#endif
		struct cancelsharedscriptGluePB {
			unsigned char	componentFlags;
			unsigned char	componentParamSize;
			short componentWhat;
			ComponentInstance	comp;
		};
		#ifdef powerc
			#pragma options align=reset
		#endif
		
		struct cancelsharedscriptGluePB pb;
		
		pb.componentFlags = 0;
		pb.componentParamSize = cancelsharedscriptParamSize;
		pb.componentWhat = msCancelSharedScriptCommand;
		pb.comp = comp;
		
		#if TARGET_API_MAC_CARBON == 1
		return CallComponentDispatch(&pb);
		#else	
		return CallUniversalProc(CallComponentUPP, uppCallComponentProcInfo, &pb);
		#endif
		} 	/*cancelsharedscriptGlue*/
		
	
	glue checksharedmenusGlue (ComponentInstance comp, short idinsertafter) {
	
		#define checksharedmenusParamSize	 (sizeof (idinsertafter))
		
		#ifdef powerc
			#pragma options align=mac68k
		#endif
		struct checksharedmenusGluePB {
			unsigned char	componentFlags;
			unsigned char	componentParamSize;
			short componentWhat;
			short idinsertafter;
			ComponentInstance	comp;
		};
		#ifdef powerc
			#pragma options align=reset
		#endif
		
		struct checksharedmenusGluePB pb;
		
		pb.componentFlags = 0;
		pb.componentParamSize = checksharedmenusParamSize;
		pb.componentWhat = msCheckSharedMenusCommand;
		pb.idinsertafter = idinsertafter;
		pb.comp = comp;
		
		#if TARGET_API_MAC_CARBON == 1
		return CallComponentDispatch(&pb);
		#else	
		return CallUniversalProc(CallComponentUPP, uppCallComponentProcInfo, &pb);
		#endif
		} 	/*checksharedmenusGlue*/
		
	
	glue disposesharedmenusGlue (ComponentInstance comp) {
	
		#define disposesharedmenusParamSize	 (0L)
		
		#ifdef powerc
			#pragma options align=mac68k
		#endif
		struct disposesharedmenusGluePB {
			unsigned char	componentFlags;
			unsigned char	componentParamSize;
			short componentWhat;
			ComponentInstance	comp;
		};
		#ifdef powerc
			#pragma options align=reset
		#endif
		
		struct disposesharedmenusGluePB pb;
		
		pb.componentFlags = 0;
		pb.componentParamSize = disposesharedmenusParamSize;
		pb.componentWhat = msDisposeSharedMenusCommand;
		pb.comp = comp;
		
		#if TARGET_API_MAC_CARBON == 1
		return CallComponentDispatch(&pb);
		#else	
		return CallUniversalProc(CallComponentUPP, uppCallComponentProcInfo, &pb);
		#endif
		} 	/*disposesharedmenusGlue*/
		
	
	glue issharedmenuGlue (ComponentInstance comp, short idmenu, Boolean *flsharedmenu) {
	
		#define issharedmenuParamSize	 (sizeof (idmenu) + sizeof (flsharedmenu))
		
		#ifdef powerc
			#pragma options align=mac68k
		#endif
		struct issharedmenuGluePB {
			unsigned char	componentFlags;
			unsigned char	componentParamSize;
			short componentWhat;
			Boolean *flsharedmenu;
			short idmenu;
			ComponentInstance	comp;
		};
		#ifdef powerc
			#pragma options align=reset
		#endif
		
		struct issharedmenuGluePB pb;
		
		pb.componentFlags = 0;
		pb.componentParamSize = issharedmenuParamSize;
		pb.componentWhat = msIsSharedMenuCommand;
		pb.flsharedmenu = flsharedmenu;
		pb.idmenu = idmenu;
		pb.comp = comp;
		
		#if TARGET_API_MAC_CARBON == 1
		return CallComponentDispatch(&pb);
		#else	
		return CallUniversalProc(CallComponentUPP, uppCallComponentProcInfo, &pb);
		#endif
		} 	/*issharedmenuGlue*/
		
	
	glue enablesharedmenusGlue (ComponentInstance comp, Boolean flenable) {
	
		#define enablesharedmenusParamSize	 (sizeof (flenable))
		
		#ifdef powerc
			#pragma options align=mac68k
		#endif
		struct enablesharedmenusGluePB {
			unsigned char	componentFlags;
			unsigned char	componentParamSize;
			short componentWhat;
			Boolean flenable;
			ComponentInstance	comp;
		};
		#ifdef powerc
			#pragma options align=reset
		#endif
		
		struct enablesharedmenusGluePB pb;
		
		pb.componentFlags = 0;
		pb.componentParamSize = enablesharedmenusParamSize;
		pb.componentWhat = msEnableSharedMenusCommand;
		pb.flenable = flenable;
		pb.comp = comp;
		
		#if TARGET_API_MAC_CARBON == 1
		return CallComponentDispatch(&pb);
		#else	
		return CallUniversalProc(CallComponentUPP, uppCallComponentProcInfo, &pb);
		#endif
		} 	/*enablesharedmenusGlue*/
		
	
	glue runsharedmenuitemGlue (ComponentInstance comp, short idmenu, short iditem) {
	
		#define runsharedmenuitemParamSize	 (sizeof (idmenu) + sizeof (iditem))
		
		#ifdef powerc
			#pragma options align=mac68k
		#endif
		struct runsharedmenuitemGluePB {
			unsigned char	componentFlags;
			unsigned char	componentParamSize;
			short componentWhat;
			short iditem;
			short idmenu;
			ComponentInstance	comp;
		};
		#ifdef powerc
			#pragma options align=reset
		#endif
		
		struct runsharedmenuitemGluePB pb;
		
		pb.componentFlags = 0;
		pb.componentParamSize = runsharedmenuitemParamSize;
		pb.componentWhat = msRunSharedMenuItemCommand;
		pb.iditem = iditem;
		pb.idmenu = idmenu;
		pb.comp = comp;
		
		#if TARGET_API_MAC_CARBON == 1
		return CallComponentDispatch(&pb);
		#else	
		return CallUniversalProc(CallComponentUPP, uppCallComponentProcInfo, &pb);
		#endif
		} 	/*runsharedmenuitemGlue*/
		
	
	glue setscripterrorcallbackGlue (ComponentInstance comp, ProcPtr scripterrorproc) {
	
		#define setscripterrorcallbackParamSize	 (sizeof (scripterrorproc))
		
		#ifdef powerc
			#pragma options align=mac68k
		#endif
		struct setscripterrorcallbackGluePB {
			unsigned char	componentFlags;
			unsigned char	componentParamSize;
			short componentWhat;
			ProcPtr scripterrorproc;
			ComponentInstance	comp;
		};
		#ifdef powerc
			#pragma options align=reset
		#endif
		
		struct setscripterrorcallbackGluePB pb;
		
		pb.componentFlags = 0;
		pb.componentParamSize = setscripterrorcallbackParamSize;
		pb.componentWhat = msSetScriptErrorCallbackCommand;
		pb.scripterrorproc = scripterrorproc;
		pb.comp = comp;
		
		#if TARGET_API_MAC_CARBON == 1
		return CallComponentDispatch(&pb);
		#else	
		return CallUniversalProc(CallComponentUPP, uppCallComponentProcInfo, &pb);
		#endif
		} 	/*setscripterrorcallbackGlue*/
	
	
	glue stubstartGlue (ComponentInstance comp) {
	
		#define stubstartParamSize	 (0L)
		
		#ifdef powerc
			#pragma options align=mac68k
		#endif
		struct stubstartGluePB {
			unsigned char	componentFlags;
			unsigned char	componentParamSize;
			short componentWhat;
			ComponentInstance	comp;
		};
		#ifdef powerc
			#pragma options align=reset
		#endif
		
		struct stubstartGluePB pb;
		
		pb.componentFlags = 0;
		pb.componentParamSize = stubstartParamSize;
		pb.componentWhat = wsStubStartCommand;
		pb.comp = comp;
		
		#if TARGET_API_MAC_CARBON == 1
		return CallComponentDispatch(&pb);
		#else	
		return CallUniversalProc(CallComponentUPP, uppCallComponentProcInfo, &pb);
		#endif
		} 	/*stubstartGlue*/

#endif


glue runhandleGlue (ComponentInstance comp, Handle h, short flscriptedcard, Str255 windowname, Point pt, uisEventCallback filter) {
	
	#define runhandleParamSize	 (sizeof (h) + sizeof (flscriptedcard) + sizeof (windowname) + sizeof (pt) + sizeof (filter))
	
	#ifdef powerc
		#pragma options align=mac68k
	#endif
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
	#ifdef powerc
		#pragma options align=reset
	#endif
	
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
	
	#if TARGET_API_MAC_CARBON == 1
	return CallComponentDispatch(&pb);
	#else	
	return CallUniversalProc(CallComponentUPP, uppCallComponentProcInfo, &pb);
	#endif
	} /*runhandleGlue*/


glue runmodalhandleGlue (ComponentInstance comp, Handle h, short flscriptedcard, Str255 windowname, Point pt, uisEventCallback filter) {
	
	#define runmodalhandleParamSize	 (sizeof (h) + sizeof (flscriptedcard) + sizeof (windowname) + sizeof (pt) + sizeof (filter))
	
	#ifdef powerc
		#pragma options align=mac68k
	#endif
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
	#ifdef powerc
		#pragma options align=reset
	#endif
	
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
	
	#if TARGET_API_MAC_CARBON == 1
	return CallComponentDispatch(&pb);
	#else	
	return CallUniversalProc(CallComponentUPP, uppCallComponentProcInfo, &pb);
	#endif
	} /*runmodalhandleGlue*/


glue setobjectvalueGlue (ComponentInstance comp, Handle hcard, Str255 name, Handle value) {
	
	#define setobjectvalueParamSize	 (sizeof (hcard) + sizeof (name) + sizeof (value))
	
	#ifdef powerc
		#pragma options align=mac68k
	#endif
	struct setobjectvalueGluePB {
		unsigned char	componentFlags;
		unsigned char	componentParamSize;
		short componentWhat;
		Handle value;
		unsigned char *name;
		Handle hcard;
		ComponentInstance	comp;
	};
	#ifdef powerc
		#pragma options align=reset
	#endif
	
	struct setobjectvalueGluePB pb;
	
	pb.componentFlags = 0;
	pb.componentParamSize = setobjectvalueParamSize;
	pb.componentWhat = wsSetObjectValueCommand;
	pb.value = value;
	pb.name = name;
	pb.hcard = hcard;
	pb.comp = comp;
	
	#if TARGET_API_MAC_CARBON == 1
	return CallComponentDispatch(&pb);
	#else	
	return CallUniversalProc(CallComponentUPP, uppCallComponentProcInfo, &pb);
	#endif
	} /*setobjectvalueGlue*/
	

glue getobjectvalueGlue (ComponentInstance comp, Handle hcard, Str255 name, Handle *value) {
	
	#define getobjectvalueParamSize	 (sizeof (hcard) + sizeof (name) + sizeof (value))
	
	#ifdef powerc
		#pragma options align=mac68k
	#endif
	struct getobjectvalueGluePB {
		unsigned char	componentFlags;
		unsigned char	componentParamSize;
		short componentWhat;
		Handle *value;
		unsigned char *name;
		Handle hcard;
		ComponentInstance	comp;
	};
	#ifdef powerc
		#pragma options align=reset
	#endif
	
	struct getobjectvalueGluePB pb;
	
	pb.componentFlags = 0;
	pb.componentParamSize = getobjectvalueParamSize;
	pb.componentWhat = wsGetObjectValueCommand;
	pb.value = value;
	pb.name = name;
	pb.hcard = hcard;
	pb.comp = comp;
	
	#if TARGET_API_MAC_CARBON == 1
	return CallComponentDispatch(&pb);
	#else	
	return CallUniversalProc(CallComponentUPP, uppCallComponentProcInfo, &pb);
	#endif
	} /*getobjectvalueGlue*/
	

glue getobjecthandleGlue (ComponentInstance comp, Handle hcard, Str255 name, Handle *h) {
	
	#define getobjecthandleParamSize	 (sizeof (hcard) + sizeof (name) + sizeof (h))
	
	#ifdef powerc
		#pragma options align=mac68k
	#endif
	struct getobjecthandleGluePB {
		unsigned char	componentFlags;
		unsigned char	componentParamSize;
		short componentWhat;
		Handle *h;
		unsigned char *name;
		Handle hcard;
		ComponentInstance	comp;
	};
	#ifdef powerc
		#pragma options align=reset
	#endif
	
	struct getobjecthandleGluePB pb;
	
	pb.componentFlags = 0;
	pb.componentParamSize = getobjecthandleParamSize;
	pb.componentWhat = wsGetObjectHandleCommand;
	pb.h = h;
	pb.name = name;
	pb.hcard = hcard;
	pb.comp = comp;
	
	#if TARGET_API_MAC_CARBON == 1
	return CallComponentDispatch(&pb);
	#else	
	return CallUniversalProc(CallComponentUPP, uppCallComponentProcInfo, &pb);
	#endif
	} /*getobjecthandleGlue*/
	

glue recalcGlue (ComponentInstance comp, Handle h) {
	
	#define recalcParamSize	 (sizeof (h))
	
	#ifdef powerc
		#pragma options align=mac68k
	#endif
	struct recalcGluePB {
		unsigned char	componentFlags;
		unsigned char	componentParamSize;
		short componentWhat;
		Handle h;
		ComponentInstance	comp;
	};
	#ifdef powerc
		#pragma options align=reset
	#endif
	
	struct recalcGluePB pb;
	
	pb.componentFlags = 0;
	pb.componentParamSize = recalcParamSize;
	pb.componentWhat = wsRecalcObjectCommand;
	pb.h = h;
	pb.comp = comp;
	
	#if TARGET_API_MAC_CARBON == 1
	return CallComponentDispatch(&pb);
	#else	
	return CallUniversalProc(CallComponentUPP, uppCallComponentProcInfo, &pb);
	#endif
	} /*recalcGlue*/


glue editGlue (ComponentInstance comp, short editcommand) {
	
	#define editParamSize	 (sizeof (editcommand))
	
	#ifdef powerc
		#pragma options align=mac68k
	#endif
	struct editGluePB {
		unsigned char	componentFlags;
		unsigned char	componentParamSize;
		short componentWhat;
		short editcommand;
		ComponentInstance	comp;
	};
	#ifdef powerc
		#pragma options align=reset
	#endif
	
	struct editGluePB pb;
	
	pb.componentFlags = 0;
	pb.componentParamSize = editParamSize;
	pb.componentWhat = wsEditCommand;
	pb.editcommand = editcommand;
	pb.comp = comp;
	
	#if TARGET_API_MAC_CARBON == 1
	return CallComponentDispatch(&pb);
	#else	
	return CallUniversalProc(CallComponentUPP, uppCallComponentProcInfo, &pb);
	#endif
	} /*editGlue*/


glue updateGlue (ComponentInstance comp, Handle h) {
	
	#define updateParamSize	 (sizeof (h))
	
	#ifdef powerc
		#pragma options align=mac68k
	#endif
	struct updateGluePB {
		unsigned char	componentFlags;
		unsigned char	componentParamSize;
		short componentWhat;
		Handle h;
		ComponentInstance	comp;
	};
	#ifdef powerc
		#pragma options align=reset
	#endif
	
	struct updateGluePB pb;
	
	pb.componentFlags = 0;
	pb.componentParamSize = updateParamSize;
	pb.componentWhat = wsUpdateCommand;
	pb.h = h;
	pb.comp = comp;
	
	#if TARGET_API_MAC_CARBON == 1
	return CallComponentDispatch(&pb);
	#else	
	return CallUniversalProc(CallComponentUPP, uppCallComponentProcInfo, &pb);
	#endif
	} /*updateGlue*/

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
	
#ifndef isFrontier

	glue initsharedmenusGlue (ComponentInstance)
		
		= ComponentCallNow (msInitSharedMenusCommand, 0L); 
		/*initsharedmenusGlue*/
		
	
	glue sharedmenuhitGlue (ComponentInstance, short, short, Boolean *)
		
		= ComponentCallNow (msSharedMenuHitCommand, sizeof (short) + sizeof (short) + sizeof (Boolean *)); 
		/*sharedmenuhitGlue*/
		
	
	glue sharedscriptrunningGlue (ComponentInstance, Boolean *)
		
		= ComponentCallNow (msSharedScriptRunningCommand, sizeof (Boolean *));
		/*sharedscriptrunningGlue*/
		
	
	glue cancelsharedscriptGlue (ComponentInstance)
		
		= ComponentCallNow (msCancelSharedScriptCommand, 0L); 
		/*cancelsharedscriptGlue*/
		
	
	glue checksharedmenusGlue (ComponentInstance, short)
		
		= ComponentCallNow (msCheckSharedMenusCommand, sizeof (short)); 
		/*checksharedmenusGlue*/
		
	
	glue disposesharedmenusGlue (ComponentInstance)
		
		= ComponentCallNow (msDisposeSharedMenusCommand, 0L); 
		/*disposesharedmenusGlue*/
		
	
	glue issharedmenuGlue (ComponentInstance, short, Boolean *)
		
		= ComponentCallNow (msIsSharedMenuCommand, sizeof (short)); 
		/*issharedmenuGlue*/
		
	
	glue enablesharedmenusGlue (ComponentInstance, Boolean)
		
		= ComponentCallNow (msEnableSharedMenusCommand, sizeof (Boolean)); 
		/*enablesharedmenusGlue*/
		
	
	glue runsharedmenuitemGlue (ComponentInstance, short, short)
		
		= ComponentCallNow (msRunSharedMenuItemCommand, sizeof (short) + sizeof (short)); 
		/*runsharedmenuitemGlue*/
		
	
	glue setscripterrorcallbackGlue (ComponentInstance, ProcPtr)
		
		= ComponentCallNow (msSetScriptErrorCallbackCommand, sizeof (ProcPtr)); 
		/*setscripterrorcallbackGlue*/
	
	
	glue stubstartGlue (ComponentInstance comp) 
	
		= ComponentCallNow (wsStubStartCommand, 0L); 
		/*stubstartGlue*/

#endif

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
	

#ifndef isFrontier
	
	glue initsharedmenusGlue (ComponentInstance)
		
		ComponentCallNow (msInitSharedMenusCommand, 0L); 
		/*initsharedmenusGlue*/
		
	
	glue sharedmenuhitGlue (ComponentInstance, short, short, Boolean *)
		
		ComponentCallNow (msSharedMenuHitCommand, sizeof (short) + sizeof (short) + sizeof (Boolean *)); 
		/*sharedmenuhitGlue*/
		
	
	glue sharedscriptrunningGlue (ComponentInstance, Boolean *)
		
		ComponentCallNow (msSharedScriptRunningCommand, sizeof (Boolean *));
		/*sharedscriptrunningGlue*/
		
	
	glue cancelsharedscriptGlue (ComponentInstance)
		
		ComponentCallNow (msCancelSharedScriptCommand, 0L); 
		/*cancelsharedscriptGlue*/
		
	
	glue checksharedmenusGlue (ComponentInstance, short)
		
		ComponentCallNow (msCheckSharedMenusCommand, sizeof (short)); 
		/*checksharedmenusGlue*/
		
	
	glue disposesharedmenusGlue (ComponentInstance)
		
		ComponentCallNow (msDisposeSharedMenusCommand, 0L); 
		/*disposesharedmenusGlue*/
		
	
	glue issharedmenuGlue (ComponentInstance, short, Boolean *)
		
		ComponentCallNow (msIsSharedMenuCommand, sizeof (short)); 
		/*issharedmenuGlue*/
		
	
	glue enablesharedmenusGlue (ComponentInstance, Boolean)
		
		ComponentCallNow (msEnableSharedMenusCommand, sizeof (Boolean)); 
		/*enablesharedmenusGlue*/
		
	
	glue runsharedmenuitemGlue (ComponentInstance, short, short)
		
		ComponentCallNow (msRunSharedMenuItemCommand, sizeof (short) + sizeof (short)); 
		/*runsharedmenuitemGlue*/
		
	
	glue setscripterrorcallbackGlue (ComponentInstance, ProcPtr)
		
		ComponentCallNow (msSetScriptErrorCallbackCommand, sizeof (ProcPtr)); 
		/*setscripterrorcallbackGlue*/
	
	
	glue stubstartGlue (ComponentInstance comp) 
	
		ComponentCallNow (wsStubStartCommand, 0L); 
		/*stubstartGlue*/

#endif


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

#ifndef isFrontier

Boolean uisDisposeSharedMenus (void) { /*dispose menu array and handles it contains*/
	
	if (msGlobals.menuserver == 0)
		return (false);
		
	return (disposesharedmenusGlue (msGlobals.menuserver) == noErr);
	} /*uisDisposeSharedMenus*/


Boolean uisIsSharedMenu (short idmenu) { /*return true if the indicated menu is one of the shared menus*/
	
	Boolean flshared;
	
	if (msGlobals.menuserver != 0) {
		
		if (issharedmenuGlue (msGlobals.menuserver, idmenu, &flshared) == noErr)
			return (flshared);
		}
		
	return (false);
	} /*uisIsSharedMenu*/


void uisEnableSharedMenus (Boolean flenable) { /*enables or disables the shared menus*/
	
	if (msGlobals.menuserver != 0)
		enablesharedmenusGlue (msGlobals.menuserver, flenable);
	} /*uisEnableSharedMenus*/


Boolean uisRunSharedMenuItem (short idmenu, short iditem) {
	 
	/*
	call the menu server to run the script linked to the indicated menu item.
	*/
	
	if (msGlobals.menuserver == 0)
		return (false);
		
	return (runsharedmenuitemGlue (msGlobals.menuserver, idmenu, iditem) == noErr);
	} /*uisRunSharedMenuItem*/
	

Boolean uisSharedMenuHit (short idmenu, short iditem) {

	Boolean flshareditem;
	
	if (msGlobals.menuserver == 0) 
		return (false);
		
	if (sharedmenuhitGlue (msGlobals.menuserver, idmenu, iditem, &flshareditem) != noErr)
		return (false);
		
	return (flshareditem); /*client handles if it wasn't a shared item*/
	} /*uisSharedMenuHit*/
	
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
	
	#ifndef isFrontier
	
	if (msGlobals.menuserver != 0) {
		
		switch ((*ev).what) {
			
			case nullEvent:
				if (isFrontProcess ())
					checksharedmenusGlue (msGlobals.menuserver, msGlobals.idinsertafter);
				
				break;
			} /*switch*/
		}
	
	#endif
	
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
		
		#if TARGET_API_MAC_CARBON == 1
		wnext = GetNextWindow(w);
		#else
		
		wnext = (WindowPtr) (*(WindowPeek) w).nextWindow;
		#endif
		
		if (uisIsSharedWindow (w))
			uisCloseSharedWindow (w);
		
		w = wnext; 
		} /*while*/
	} /*uisCloseAllSharedWindows*/
	
#ifndef isFrontier

static Boolean IACgetbinaryparam (AppleEvent *event, OSType keyword, Handle *hbinary, OSType *binarytype, OSErr *errcode) {
	
	AEDesc result;
	OSErr ec;
	
	*errcode = AEGetParamDesc (event, (AEKeyword) keyword, typeWildCard, &result);
	
	if (*errcode != noErr) 
		return (false);
	
	#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/
	
		copydatahandle (&result, hbinary);
		
	#else
	
		*hbinary = result.dataHandle;
	
	#endif
	
	*binarytype = result.descriptorType;
	
	return (true);
	} /*IACgetbinaryparam*/


static Boolean IACgetstringparam (AppleEvent *event, OSType keyword, Str255 s, OSErr *errcode) {
	
	AEDesc result;
	Handle htext;
	long lenstring;
	
	*errcode = AEGetParamDesc (event, (AEKeyword) keyword, typeChar, &result);
	
	if (*errcode != noErr) 
		return (false);
	
	#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/
	
		copydatahandle (&result, &htext);
	
	#else
	
		htext = result.dataHandle;
	
	#endif
	
	lenstring = GetHandleSize (htext);
	
	if (lenstring > 255)
		lenstring = 255;
		
	s [0] = (unsigned char) lenstring;
	
	BlockMove (*htext, &s [1], lenstring);
	
	return (true);
	} /*IACgetstringparam*/
	
	
static Boolean IACgetlongparam (AppleEvent *event, OSType keyword, long *val, OSErr *errcode) {
	
	register OSErr ec;
	DescType actualtype;
	Size actualsize;
	
	*errcode = AEGetParamPtr (
		
		event, (AEKeyword) keyword, typeLongInteger, 
		
		&actualtype, (Ptr) val, sizeof (long), &actualsize);
	
	if (*errcode != noErr) 
		return (false);
	
	return (true);
	} /*IACgetlongparam*/
	

static void IACreturnboolean (AppleEvent *event, Boolean fl) {
	
	AEPutParamPtr (event, keyDirectObject, typeBoolean, (Ptr) &fl, sizeof (Boolean));
	} /*IACreturnboolean*/


static pascal OSErr handleopencardwindow (AppleEvent *event, AppleEvent *reply, long refcon) {

	/*	
	installed as an AE handler, intended to be called by scripts that
	want to open a shared window in the application's window list.
	
	it's a tradeoff -- burn a little memory in every UI sharing-aware
	app, or make the SHUI server component perform an unnatural act. the
	problem is the component call needs a component instance. only the 
	host app knows which instance we're talking about. the server could
	manage its data according to ProcessSerialNumber, but this is not a
	natural way of doing things for Mac components. not impossible, just
	a bunch of extra code in a very hard place to debug. 3/21/93 DW
	
	here's a one-line script that calls this handler...
	
	appleEvent ('KAHL', 'SHUI', 'OPEN', '----', number ('zzzz'), 'card', scratchpad.cards.xxx, 'wtit', "Hello!")
	*/
	
	OSErr ec;
	OSType idcomponent; 
	OSType binarytype;
	Handle packedcard;
	Str255 windowtitle;
	
	if (!IACgetlongparam (event, keyDirectObject, (long *) &idcomponent, &ec))
		return (ec);

	if (!IACgetbinaryparam (event, 'card', &packedcard, &binarytype, &ec))
		return (ec);
		
	if (!IACgetstringparam (event, 'wtit', windowtitle, &ec))
		return (ec);
		
	IACreturnboolean (event, uisOpenHandle (packedcard, false, windowtitle, 0, 0, nil));
	
	return (noErr);
	} /*handleopencardwindow*/
	
	
Boolean uisStubStart (void) {
	
	if (wsGlobals.windowserver == 0)
		return (false);
		
	wsGlobals.errorcode = stubstartGlue (wsGlobals.windowserver);
	
	return (wsGlobals.errorcode == noErr);
	} /*uisStubStart*/

#endif

Boolean uisOpenHandle (Handle h, Boolean flscriptedcard, Str255 windowname, short top, short left, uisEventCallback filter) {
	
	Point pt;
	
	pt.h = left; pt.v = top;
	
	if (wsGlobals.windowserver == 0)
		return (false);
		
	wsGlobals.errorcode = runhandleGlue (wsGlobals.windowserver, h, flscriptedcard, windowname, pt, filter);
	
	return (wsGlobals.errorcode == noErr);
	} /*uisOpenHandle*/
	
	
Boolean uisOpenCardResource (short id, Boolean flscriptedcard, Str255 windowname, short top, short left, uisEventCallback filter) {
	
	Handle h;
	
	h = GetResource ('CARD', id);
	
	if (h == nil) {
		
		wsGlobals.errorcode = ResError ();
		
		return (false);
		}
		
	DetachResource (h);
	
	return (uisOpenHandle (h, flscriptedcard, windowname, top, left, filter));
	} /*uisOpenCardResource*/
	
	
Boolean uisRunModalHandle (Handle h, Boolean flscriptedcard, Str255 windowname, short top, short left, uisEventCallback filter) {
	
	Point pt;
	
	pt.h = left; pt.v = top;
	
	if (wsGlobals.windowserver == 0)
		return (false);
		
	wsGlobals.errorcode = runmodalhandleGlue (wsGlobals.windowserver, h, flscriptedcard, windowname, pt, filter);
	
	return (wsGlobals.errorcode == noErr);
	} /*uisRunModalHandle*/
	
	
Boolean uisRunModalResource (short id, Boolean flscriptedcard, Str255 windowname, short top, short left, uisEventCallback filter) {
	
	Handle h;
	
	h = GetResource ('CARD', id);
	
	if (h == nil) {
		
		wsGlobals.errorcode = ResError ();
		
		return (false);
		}
		
	DetachResource (h);
	
	return (uisRunModalHandle (h, flscriptedcard, windowname, top, left, filter));
	} /*uisRunModalResource*/
	
#ifndef isFrontier

Boolean uisSetObjectValue (Handle hcard, Str255 name, Handle hvalue) {
	
	if (wsGlobals.windowserver == 0)
		return (false);
		
	return (setobjectvalueGlue (wsGlobals.windowserver, hcard, name, hvalue));
	} /*uisSetObjectValue*/
	
	
Boolean uisGetObjectValue (Handle hcard, Str255 name, Handle *hvalue) {
	
	if (wsGlobals.windowserver == 0)
		return (false);
		
	return (getobjectvalueGlue (wsGlobals.windowserver, hcard, name, hvalue));
	} /*uisGetObjectValue*/
	
	
Boolean uisUpdate (Handle hcard) {
	
	if (wsGlobals.windowserver == 0)
		return (false);
		
	return (updateGlue (wsGlobals.windowserver, hcard));
	} /*uisUpdate*/
	
	
Boolean uisGetObjectHandle (Handle hcard, Str255 name, Handle *hvalue) {
	
	if (wsGlobals.windowserver == 0)
		return (false);
		
	return (getobjecthandleGlue (wsGlobals.windowserver, hcard, name, (Handle *) hvalue));
	} /*uisGetObjectHandle*/
	

Boolean uisRecalcObject (Handle hobject) {
	
	if (wsGlobals.windowserver == 0)
		return (false);
		
	return (recalcGlue (wsGlobals.windowserver, hobject));
	} /*uisRecalcObject*/
	
#endif

Boolean uisEdit (short editcommand) {
		
	if (wsGlobals.windowserver == 0)
		return (false);
		
	return (editGlue (wsGlobals.windowserver, editcommand));
	} /*uisEdit*/
	

Boolean uisInit (ProcPtr callback, short idinsertafter, OSType idclientapp, unsigned short bitarray) {
	
	/*
	initialize window sharing and menu sharing. 
	
	the callback is a routine that receives a Str255 as a parameter. it should
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
	
		#ifndef isFrontier
		
		AEInstallEventHandler ('SHUI', 'OPEN', (EventHandlerProcPtr) &handleopencardwindow, 0, false);
		
		#endif
		
		
		wsGlobals.windowserver = OpenDefaultComponent (wsComponentType, wsComponentSubType);
		}
	
	#ifndef isFrontier
	
	if (flmenusharing) {
	
		msGlobals.menuserver = OpenDefaultComponent (msComponentType, msComponentSubType);
	
		msGlobals.idinsertafter = idinsertafter;
	
		if (msGlobals.menuserver != 0) {
	
			if (initsharedmenusGlue (msGlobals.menuserver) != noErr)
				goto error;
		
			if (setscripterrorcallbackGlue (msGlobals.menuserver, callback) != noErr)
				goto error;
			}
		}
	
	#endif
		
	return (true);
	
	error:
	
	uisClose ();
	
	return (false);
	} /*uisInit*/
	
	
void uisClose (void) {

	if (wsGlobals.windowserver != 0)		
		CloseComponent (wsGlobals.windowserver);
	
	#ifndef isFrontier
	
	if (msGlobals.menuserver != 0) 
		CloseComponent (msGlobals.menuserver);
	
	#endif
	} /*uisClose*/
	
	
	
	