
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

#ifndef osainternalinclude
#define osainternalinclude


#ifndef __COMPONENTS__
	#include <Components.h>
#endif

#ifndef __OSA__
	#include <OSA.h>
#endif

#ifndef usertalkinclude
	#include <UserTalk.h>
#endif

#ifndef processinternalinclude
	#include "processinternal.h"
#endif

/*constants*/

#define typeLAND 'LAND'

#define componentlistnumber 512
#define recordinglistnumber 513

enum { /*recording strings*/
	
	frontierstring = 1,
	
	usertalkstring,
	
	scriptingcomponentstring,
	
	menusharingcomponentstring,
	
	windowsharingcomponentstring
	};


/*types*/

typedef pascal Boolean (*GNEProcPtr) (short, EventRecord *);

typedef pascal void (*tyMSstringcallback) (Str255);

typedef pascal void (*tyMSeventcallback) (EventRecord *);

typedef pascal void (*tyMSmenuscallback) (hdlmenuarray);


enum {
	uppGNEProcInfo = kPascalStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(Boolean)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(short)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(EventRecord*)))
};

enum {
	uppMSstringcallbackProcInfo = kPascalStackBased
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(char *)))
};

enum {
	uppMSeventcallbackProcInfo = kPascalStackBased
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(EventRecord*)))
};

enum {
	uppMSmenuscallbackProcInfo = kPascalStackBased
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(hdlmenuarray)))
};

//Code change by Timothy Paustian Friday, June 16, 2000 1:32:41 PM
//Changed to Opaque call for Carbon
//Had to mask out the GENERATINGCFM directive. 
#if TARGET_API_MAC_CARBON
	typedef GNEProcPtr GNEUPP;
	
	#define CallGNEProc(userRoutine, mask, theEvent)		\
			(*(userRoutine))((mask), (theEvent))
	#define NewGNEProc(userRoutine)		\
			(GNEUPP)(userRoutine)
	
	typedef tyMSstringcallback MSstringcallbackUPP;
	
	#define CallMSstringcallbackProc(userRoutine, theString)		\
			(*(userRoutine))((theString))
	#define NewMSstringcallbackProc(userRoutine)		\
			(MSstringcallbackUPP)(userRoutine)
	
	typedef tyMSeventcallback MSeventcallbackUPP;
	
	#define CallMSeventcallbackProc(userRoutine, theEvent)		\
			(*(userRoutine))((theEvent))
	#define NewMSeventcallbackProc(userRoutine)		\
			(MSeventcallbackUPP)(userRoutine)
	
	typedef tyMSmenuscallback MSmenuscallbackUPP;
	
	#define CallMSmenuscallbackProc(userRoutine, theMenus)		\
			(*(userRoutine))((theMenus))
	#define NewMSmenuscallbackProc(userRoutine)		\
			(MSmenuscallbackUPP)(userRoutine)
#else
#if GENERATINGCFM

	typedef UniversalProcPtr GNEUPP;
	
	#define CallGNEProc(userRoutine, mask, theEvent)		\
			CallUniversalProc((UniversalProcPtr)(userRoutine), uppGNEProcInfo, (mask), (theEvent))
	#define NewGNEProc(userRoutine)		\
			(GNEUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppGNEProcInfo, GetCurrentISA())
	
	typedef UniversalProcPtr MSstringcallbackUPP;
	
	#define CallMSstringcallbackProc(userRoutine, theString)		\
			CallUniversalProc((UniversalProcPtr)(userRoutine), uppMSstringcallbackProcInfo, (theString))
	#define NewMSstringcallbackProc(userRoutine)		\
			(MSstringcallbackUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppMSstringcallbackProcInfo, GetCurrentISA())
	
	typedef UniversalProcPtr MSeventcallbackUPP;
	
	#define CallMSeventcallbackProc(userRoutine, theEvent)		\
			CallUniversalProc((UniversalProcPtr)(userRoutine), uppMSeventcallbackProcInfo, (theEvent))
	#define NewMSeventcallbackProc(userRoutine)		\
			(MSeventcallbackUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppMSeventcallbackProcInfo, GetCurrentISA())

	typedef UniversalProcPtr MSmenuscallbackUPP;
	
	#define CallMSmenuscallbackProc(userRoutine, theMenus)		\
			CallUniversalProc((UniversalProcPtr)(userRoutine), uppMSmenuscallbackProcInfo, (theMenus))
	#define NewMSmenuscallbackProc(userRoutine)		\
			(MSmenuscallbackUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppMSmenuscallbackProcInfo, GetCurrentISA())
#else

	typedef GNEProcPtr GNEUPP;
	
	#define CallGNEProc(userRoutine, mask, theEvent)		\
			(*(userRoutine))((mask), (theEvent))
	#define NewGNEProc(userRoutine)		\
			(GNEUPP)(userRoutine)
	
	typedef tyMSstringcallback MSstringcallbackUPP;
	
	#define CallMSstringcallbackProc(userRoutine, theString)		\
			(*(userRoutine))((theString))
	#define NewMSstringcallbackProc(userRoutine)		\
			(MSstringcallbackUPP)(userRoutine)
	
	typedef tyMSeventcallback MSeventcallbackUPP;
	
	#define CallMSeventcallbackProc(userRoutine, theEvent)		\
			(*(userRoutine))((theEvent))
	#define NewMSeventcallbackProc(userRoutine)		\
			(MSeventcallbackUPP)(userRoutine)
	
	typedef tyMSmenuscallback MSmenuscallbackUPP;
	
	#define CallMSmenuscallbackProc(userRoutine, theMenus)		\
			(*(userRoutine))((theMenus))
	#define NewMSmenuscallbackProc(userRoutine)		\
			(MSmenuscallbackUPP)(userRoutine)
#endif	//GENERATINGCFM
#endif //TARGET_API_MAC_CARBON


#pragma pack(2)
typedef struct tyMSglobals { /*Menu Sharing globals, all in one struct*/

	OSType serverid; /*identifier for shared menu server*/

	OSType clientid; /*id of this application*/
	
	hdlmenuarray hsharedmenus; /*data structure that holds shared menus*/
	
	Boolean fldirtysharedmenus; /*if true, menus are reloaded next time app comes to front*/
	
	Boolean flscriptcancelled; /*set true by calling CancelSharedScript*/
	
	Boolean flscriptrunning; /*true if a script is currently running*/
	
	Boolean flhavecomponentmanager; /*true if the Component Manager is present*/
	
	ComponentInstance serverinstance; /*if we have a Component Manager connection, this is it*/
	
	unsigned long timefirstrequest; /*5.0d19 dmb: keeps track of time of first requeste of new menus.*/
	
	unsigned long timelastformulacheck; /*5.0b6 dmb: so we don't checkformulas too often*/
	
	MSstringcallbackUPP scripterrorcallback; /*if non-nil, it's called when a script error occurs*/
	
	MSeventcallbackUPP eventfiltercallback; /*if non-nil, it's called w/an event while script runs*/
	
	MSmenuscallbackUPP installmenuscallback; /*4.1b4 dmb: if non-nil, it's called to install the shared menus*/
	
	MSmenuscallbackUPP removemenuscallback; /*4.1b4 dmb: if non-nil, it's called to remove the shared menus*/
	
	long idscript; /*the server's id for the currently running script, makes it easy to kill it*/
	} tyMSglobals, *ptrMSglobals, **hdlMSglobals;


typedef struct tyrecordingstate { /*maintain info about recording in progress*/
	
	boolean flrecording;
	
	OSType lastappid;
	
	hdlhashtable lastapptable;
	
	Handle hrecordedtext;
	} tyrecordingstate;

#ifdef dropletcomponent

typedef struct tydropletglobals { /*droplet application globals*/
	
	DialogPtr pmainwindow;
	
	MenuHandle happlemenu, hfilemenu;
	
	boolean flexitmainloop;
	
	AppleEvent initialevent;
	} tydropletglobals;

#endif

typedef struct tycomponentglobals { /*a place to store your globals safely*/
	
	struct tycomponentglobals **hnext; /*for linked list*/
	
	struct tycomponentglobals **hserverosaglobals; /*for nesting*/
	
	short ctpushes; /*also for nesting*/
	
	Component self;
	
	Boolean isHomeProcess;
	
	long clienta5;
	
	THz clientzone;
	
	short clientresfile;
	
	Handle clientresmaphandle;
	
	OSType clientid;
	
	ProcessSerialNumber clientpsn;
	
	long idcounter;
	
	hdlhashtable storagetable;
	
	AEDesc errordesc;

	GNEUPP getnexteventproc;
	
	OSAActiveUPP activeproc;
	
	OSACreateAppleEventUPP createproc;
	
	OSASendUPP sendproc;
	
	AEEventHandlerUPP resumedispatchproc;
	
	OSADebugUPP debugproc;
	
	long activeprocrefcon;
	
	long createprocrefcon;
	
	long sendprocrefcon;
	
	long resumedispatchprocrefcon;
	
	long debugprocrefcon;
	
	hdltreenode debugnode;
	
	hdlthreadglobals clientthreadglobals;
	
	hdlthreadglobals serverthreadglobals;
	
	tyMSglobals menusharingglobals;
	
	#ifdef dropletcomponent
	
	tydropletglobals dropletglobals;
	
	#endif
	
	tyrecordingstate recordingstate;
	//Code change by Timothy Paustian Sunday, September 3, 2000 9:56:14 PM
	//These are dynamic and each component needs its own set of UPPs that cannot
	//be shared. So put them here.
	#if TARGET_API_MAC_CARBON
	ComponentRoutineUPP	cmpcloseUPP;
	ComponentRoutineUPP	cmpcandoUPP;
	ComponentRoutineUPP	cmpversionUPP;
	ComponentRoutineUPP	osaLoadUPP;
	ComponentRoutineUPP	osaStoreUPP;
	ComponentRoutineUPP	osaExecuteUPP;
	ComponentRoutineUPP	osaDisplayUPP;
	ComponentRoutineUPP	osaScriptErrorUPP;
	ComponentRoutineUPP	osaDisposeUPP;
	ComponentRoutineUPP	osaSetScriptInfoUPP;
	ComponentRoutineUPP	osaGetScriptInfoUPP;
	ComponentRoutineUPP	osaCompileUPP;
	ComponentRoutineUPP	osaGetSourceUPP;
	ComponentRoutineUPP	osaCoerceFromDescUPP;
	ComponentRoutineUPP	osaCoerceToDescUPP;
	ComponentRoutineUPP	osaStartRecordingUPP;
	ComponentRoutineUPP	osaStopRecordingUPP;
	ComponentRoutineUPP	osaScriptingComponentNameUPP;
	ComponentRoutineUPP	osaLoadExecuteUPP;
	ComponentRoutineUPP	osaCompileExecuteUPP;
	ComponentRoutineUPP	osaDoScriptUPP;
	ComponentRoutineUPP	osaMakeContextUPP;
	ComponentRoutineUPP	osaSetResumeDispatchProcUPP;
	ComponentRoutineUPP	osaGetResumeDispatchProcUPP;
	ComponentRoutineUPP	osaExecuteEventUPP;
	ComponentRoutineUPP	osaDoEventUPP;
	ComponentRoutineUPP	osaSetActiveProcUPP;
	ComponentRoutineUPP	osaSetDebugProcUPP;
	ComponentRoutineUPP	osaDebugUPP;
	ComponentRoutineUPP	osaSetSendProcUPP;
	ComponentRoutineUPP	osaGetSendProcUPP;
	ComponentRoutineUPP	osaSetCreateProcUPP;
	ComponentRoutineUPP	osaGetCreateProcUPP;
	#endif
	} tycomponentglobals, **hdlcomponentglobals;
#pragma options align=reset

typedef pascal ComponentResult (*ComponentRoutine) (ComponentParameters *, Handle);


/*globals*/

extern hdlcomponentglobals osaglobals;

extern boolean flosashutdown;


/*prototypes*/

extern void disposecomponentglobals (hdlcomponentglobals);

extern boolean newcomponentglobals (Component, long, hdlcomponentglobals *);

extern pascal OSAError osaScriptError (hdlcomponentglobals, OSType, DescType, AEDesc*);

extern pascal ComponentResult callosafunction (Handle, ComponentParameters *, ComponentFunctionUPP);

extern long osapreclientcallback (hdlcomponentglobals); /*osacomponent.c*/

extern void osapostclientcallback (hdlcomponentglobals);

extern GNEUPP osainstallpatch (hdlcomponentglobals);

extern void osaremovepatch (hdlcomponentglobals, GNEUPP);

extern boolean getprocessname (ProcessSerialNumber, bigstring, boolean *);

extern void closeosaservers (void);

extern Component osaregistercomponent (OSType, long, ComponentRoutine, short, short);

#endif






