
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

/*This file has been forked to protect the shipping version. The Carbon version is at the top, the "good"
version at the bottom. This file should be reconciled later.*/
/*
	2004-10-27 aradke: Reconciled the Carbon and Classic versions.
*/

//#pragma options (pack_enums) /* 2002-10-13 AR: pragma not supported by CodeWarrior */

#include <land.h>
#include <UserTalk.h>
#include "dialogs.h"
#include "error.h"
#include "file.h"
#include "font.h"
#include "kb.h"
#include "launch.h"
#include "memory.h"
#include "ops.h"
#include "resources.h"
#include "strings.h"
#include "timedate.h"
#include "lang.h"
#include "langexternal.h"
#include "langinternal.h"
#include "langipc.h"
#include "langsystem7.h"
#include "shellhooks.h"
#include "scripts.h"
#include "process.h"
#include "processinternal.h"
#include "tablestructure.h"
#include "osacomponent.h"
#include "osadroplet.h"
#include "osainternal.h"
#include "osamenus.h"
#include "osaparseaete.h"
#include "osawindows.h"
#include <SetUpA5.h>
#include "byteorder.h"
#include "aeutils.h"


/* forward declarations for static functions */

static boolean osabackgroundtask (boolean);

static boolean osadebugger (hdltreenode);

static boolean osapartialeventloop (short);

static pascal OSErr osaclientactive (long refcon);

static pascal OSErr osaclientsend (const AppleEvent *, AppleEvent *, AESendMode, AESendPriority, long, AEIdleUPP, AEFilterUPP, long);

static pascal Boolean osaclientidleproc (EventRecord *ev, long *sleep, RgnHandle *mousergn);

static pascal ComponentResult cmpclose (Handle storage, ComponentInstance self);

static pascal OSAError osaLoad (hdlcomponentglobals	hglobals,
			const AEDesc*		scriptData,
			long				modeFlags,
			OSAID*				resultingCompiledScriptID);

static pascal ComponentResult cmpcando (short selector);

static pascal ComponentResult cmpversion (void);

static pascal OSAError osaStore (
			hdlcomponentglobals	hglobals,
			OSAID				compiledScriptID, 
			DescType			desiredType,
			long				modeFlags,
			AEDesc*				resultingScriptData);

static pascal OSAError osaExecute (
			hdlcomponentglobals	hglobals,
			OSAID				compiledScriptID,
			OSAID				contextID,
			long				modeFlags,
			OSAID*				resultingScriptValueID);

static pascal OSAError osaSetScriptInfo (
			hdlcomponentglobals	hglobals,
			OSAID				scriptID,
			OSType				selector,
			long				value);
			
static pascal OSAError osaGetScriptInfo (
			hdlcomponentglobals	hglobals,
			OSAID				scriptID,
			OSType				selector,
			long*				result);
			
static pascal OSAError osaCompile (
			hdlcomponentglobals	hglobals,
			const AEDesc*		sourceData,
			long				modeFlags,
			OSAID*				scriptID);
			
static pascal OSAError osaGetSource (
			hdlcomponentglobals	hglobals,
			OSAID				scriptID,
			DescType			desiredType,
			AEDesc*				resultingSourceData);
			
static pascal OSAError osaCoerceFromDesc (
			hdlcomponentglobals	hglobals,
			const AEDesc*		scriptData,
			long				modeFlags,
			OSAID*				resultingScriptID);


static pascal OSAError osaCoerceToDesc (
			hdlcomponentglobals	hglobals,
			OSAID				scriptID,
			DescType			desiredType,
			long				modeFlags,
			AEDesc*				result);
			
static pascal OSAError osaStartRecording (
			hdlcomponentglobals	hglobals,
			OSAID				*compiledScriptToModifyID);

static pascal OSAError osaStopRecording (
			hdlcomponentglobals	hglobals,
			OSAID				compiledScriptID);

static pascal OSAError osaScriptingComponentName (
			hdlcomponentglobals	hglobals,
			AEDesc*				resultingScriptingComponentName);

static pascal OSAError osaLoadExecute (
			hdlcomponentglobals	hglobals,
			const AEDesc*		scriptData,
			OSAID				contextID,
			long				modeFlags,
			OSAID*				resultingScriptValueID);
static pascal OSAError osaMakeContext (
			hdlcomponentglobals	hglobals,
			const AEDesc*		contextName,
			OSAID				parentContext,
			OSAID*				resultingContextID);
			
static pascal OSAError osaDisplay (
			hdlcomponentglobals	hglobals,
			OSAID				scriptValueID,
			DescType			desiredType,
			long				modeFlags,
			AEDesc*				resultingText);
	

static pascal OSAError osaSetResumeDispatchProc (
				hdlcomponentglobals	hglobals,
				AEEventHandlerUPP	resumeDispatchProc,
				long				refCon);
				
static pascal OSAError osaGetResumeDispatchProc (
				hdlcomponentglobals	hglobals,
				AEEventHandlerUPP*	resumeDispatchProc,
				long*				refCon);
				
static pascal OSAError osaExecuteEvent (
				hdlcomponentglobals	hglobals,
				AppleEvent*			event,
				OSAID				contextID,
				long				modeFlags,
				OSAID*				resultingScriptValueID);

static pascal OSAError osaDoEvent (
			hdlcomponentglobals	hglobals,
			AppleEvent*			event,
			OSAID				contextID,
			long				modeFlags,
			AppleEvent*			reply);
			
static pascal OSAError osaSetDebugProc (
			hdlcomponentglobals	hglobals,
			OSADebugUPP			debugProc,
			long				refCon);
			
static pascal OSAError osaDebug (
			hdlcomponentglobals	hglobals,
			OSType				selector,
			const AEDesc*		selectorData,
			DescType			desiredType,
			AEDesc*				resultingDebugInfoOrDataToSet);

static pascal OSAError osaGetSendProc (
			hdlcomponentglobals	hglobals,
			OSASendUPP*			sendProc,
			long*				refCon);

static pascal OSAError osaSetCreateProc (
			hdlcomponentglobals			hglobals,
			OSACreateAppleEventUPP		createProc,
			long						refCon);
			
static pascal OSAError osaGetCreateProc (
			hdlcomponentglobals			hglobals,
			OSACreateAppleEventUPP*		createProc,
			long*						refCon);
			


	/* proc infos for building routine descriptors and universal procedure pointers */
	
	enum {
			cmpcloseProcInfo = kPascalStackBased
				 | RESULT_SIZE(SIZE_CODE(sizeof(OSAError)))
				 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(Handle)))
				 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(ComponentInstance)))
		};
		
		enum {
			cmpcandoProcInfo = kPascalStackBased
				 | RESULT_SIZE(SIZE_CODE(sizeof(OSAError)))
				 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(short)))
		};
		
		enum {
			cmpversionProcInfo = kPascalStackBased
				 | RESULT_SIZE(SIZE_CODE(sizeof(OSAError)))
		};
		
		enum {
			osaLoadProcInfo = kPascalStackBased
				 | RESULT_SIZE(SIZE_CODE(sizeof(OSAError)))
				 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(hdlcomponentglobals)))
				 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(AEDesc *)))
				 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(long)))
				 | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(OSAID *)))
		};
		
		enum {
			osaStoreProcInfo = kPascalStackBased
				 | RESULT_SIZE(SIZE_CODE(sizeof(OSAError)))
				 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(hdlcomponentglobals)))
				 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(OSAID)))
				 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(DescType)))
				 | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(long)))
				 | STACK_ROUTINE_PARAMETER(5, SIZE_CODE(sizeof(AEDesc *)))
		};
		
		enum {
			osaExecuteProcInfo = kPascalStackBased
				 | RESULT_SIZE(SIZE_CODE(sizeof(OSAError)))
				 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(hdlcomponentglobals)))
				 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(OSAID)))
				 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(OSAID)))
				 | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(long)))
				 | STACK_ROUTINE_PARAMETER(5, SIZE_CODE(sizeof(OSAID*)))
		};
		
		enum {
			osaDisplayProcInfo = kPascalStackBased
				 | RESULT_SIZE(SIZE_CODE(sizeof(OSAError)))
				 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(hdlcomponentglobals)))
				 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(OSAID)))
				 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(DescType)))
				 | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(long)))
				 | STACK_ROUTINE_PARAMETER(5, SIZE_CODE(sizeof(AEDesc *)))
		};
		
		enum {
			osaScriptErrorProcInfo = kPascalStackBased
				 | RESULT_SIZE(SIZE_CODE(sizeof(OSAError)))
				 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(hdlcomponentglobals)))
				 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(OSType)))
				 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(DescType)))
				 | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(AEDesc *)))
		};
		
		enum {
			osaDisposeProcInfo = kPascalStackBased
				 | RESULT_SIZE(SIZE_CODE(sizeof(OSAError)))
				 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(hdlcomponentglobals)))
				 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(OSAID)))
		};
		
		enum {
			osaSetScriptInfoProcInfo = kPascalStackBased
				 | RESULT_SIZE(SIZE_CODE(sizeof(OSAError)))
				 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(hdlcomponentglobals)))
				 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(OSAID)))
				 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(OSType)))
				 | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(long)))
		};
		
		enum {
			osaGetScriptInfoProcInfo = kPascalStackBased
				 | RESULT_SIZE(SIZE_CODE(sizeof(OSAError)))
				 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(hdlcomponentglobals)))
				 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(OSAID)))
				 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(OSType)))
				 | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(long *)))
		};
		
		enum {
			osaCompileProcInfo = kPascalStackBased
				 | RESULT_SIZE(SIZE_CODE(sizeof(OSAError)))
				 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(hdlcomponentglobals)))
				 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(AEDesc *)))
				 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(long)))
				 | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(OSAID*)))
		};
		
		enum {
			osaGetSourceProcInfo = kPascalStackBased
				 | RESULT_SIZE(SIZE_CODE(sizeof(OSAError)))
				 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(hdlcomponentglobals)))
				 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(OSAID)))
				 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(DescType)))
				 | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(AEDesc *)))
		};
		
		enum {
			osaCoerceFromDescProcInfo = kPascalStackBased
				 | RESULT_SIZE(SIZE_CODE(sizeof(OSAError)))
				 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(hdlcomponentglobals)))
				 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(AEDesc*)))
				 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(long)))
				 | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(OSAID*)))
		};
		
		enum {
			osaCoerceToDescProcInfo = kPascalStackBased
				 | RESULT_SIZE(SIZE_CODE(sizeof(OSAError)))
				 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(hdlcomponentglobals)))
				 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(OSAID)))
				 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(DescType)))
				 | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(long)))
				 | STACK_ROUTINE_PARAMETER(5, SIZE_CODE(sizeof(AEDesc*)))
		};
		
		enum {
			osaStartRecordingProcInfo = kPascalStackBased
				 | RESULT_SIZE(SIZE_CODE(sizeof(OSAError)))
				 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(hdlcomponentglobals)))
				 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(OSAID *)))
		};
		
		enum {
			osaStopRecordingProcInfo = kPascalStackBased
				 | RESULT_SIZE(SIZE_CODE(sizeof(OSAError)))
				 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(hdlcomponentglobals)))
				 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(OSAID)))
		};
		
		enum {
			osaScriptingComponentNameProcInfo = kPascalStackBased
				 | RESULT_SIZE(SIZE_CODE(sizeof(OSAError)))
				 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(hdlcomponentglobals)))
				 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(AEDesc *)))
		};
		
		enum {
			osaLoadExecuteProcInfo = kPascalStackBased
				 | RESULT_SIZE(SIZE_CODE(sizeof(OSAError)))
				 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(hdlcomponentglobals)))
				 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(AEDesc*)))
				 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(OSAID)))
				 | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(long)))
				 | STACK_ROUTINE_PARAMETER(5, SIZE_CODE(sizeof(OSAID*)))
		};
		
		enum {
			osaCompileExecuteProcInfo = kPascalStackBased
				 | RESULT_SIZE(SIZE_CODE(sizeof(OSAError)))
				 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(hdlcomponentglobals)))
				 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(AEDesc*)))
				 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(OSAID)))
				 | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(long)))
				 | STACK_ROUTINE_PARAMETER(5, SIZE_CODE(sizeof(OSAID*)))
		};
		
		enum {
			osaDoScriptProcInfo = kPascalStackBased
				 | RESULT_SIZE(SIZE_CODE(sizeof(OSAError)))
				 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(hdlcomponentglobals)))
				 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(AEDesc*)))
				 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(OSAID)))
				 | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(DescType)))
				 | STACK_ROUTINE_PARAMETER(5, SIZE_CODE(sizeof(long)))
				 | STACK_ROUTINE_PARAMETER(6, SIZE_CODE(sizeof(AEDesc*)))
		};
		
		enum {
			osaMakeContextProcInfo = kPascalStackBased
				 | RESULT_SIZE(SIZE_CODE(sizeof(OSAError)))
				 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(hdlcomponentglobals)))
				 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(AEDesc*)))
				 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(OSAID)))
				 | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(OSAID*)))
		};
		
		enum {
			osaSetResumeDispatchProcProcInfo = kPascalStackBased
				 | RESULT_SIZE(SIZE_CODE(sizeof(OSAError)))
				 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(hdlcomponentglobals)))
				 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(AEEventHandlerUPP)))
				 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(long)))
		};
		
		enum {
			osaGetResumeDispatchProcProcInfo = kPascalStackBased
				 | RESULT_SIZE(SIZE_CODE(sizeof(OSAError)))
				 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(hdlcomponentglobals)))
				 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(AEEventHandlerUPP*)))
				 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(long*)))
		};
		
		enum {
			osaExecuteEventProcInfo = kPascalStackBased
				 | RESULT_SIZE(SIZE_CODE(sizeof(OSAError)))
				 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(hdlcomponentglobals)))
				 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(AppleEvent*)))
				 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(OSAID)))
				 | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(long)))
				 | STACK_ROUTINE_PARAMETER(5, SIZE_CODE(sizeof(OSAID*)))
		};
		
		enum {
			osaDoEventProcInfo = kPascalStackBased
				 | RESULT_SIZE(SIZE_CODE(sizeof(OSAError)))
				 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(hdlcomponentglobals)))
				 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(AppleEvent*)))
				 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(OSAID)))
				 | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(long)))
				 | STACK_ROUTINE_PARAMETER(5, SIZE_CODE(sizeof(AppleEvent*)))
		};
		
		enum {
			osaSetActiveProcProcInfo = kPascalStackBased
				 | RESULT_SIZE(SIZE_CODE(sizeof(OSAError)))
				 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(hdlcomponentglobals)))
				 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(OSAActiveUPP)))
				 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(long)))
		};
		
		enum {
			osaSetDebugProcProcInfo = kPascalStackBased
				 | RESULT_SIZE(SIZE_CODE(sizeof(OSAError)))
				 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(hdlcomponentglobals)))
				 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(OSADebugUPP)))
				 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(long)))
		};
		
		enum {
			osaDebugProcInfo = kPascalStackBased
				 | RESULT_SIZE(SIZE_CODE(sizeof(OSAError)))
				 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(hdlcomponentglobals)))
				 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(OSType)))
				 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(AEDesc*)))
				 | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(DescType)))
				 | STACK_ROUTINE_PARAMETER(5, SIZE_CODE(sizeof(AEDesc*)))
		};
		
		enum {
			osaSetSendProcProcInfo = kPascalStackBased
				 | RESULT_SIZE(SIZE_CODE(sizeof(OSAError)))
				 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(hdlcomponentglobals)))
				 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(OSASendUPP)))
				 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(long)))
		};
		
		enum {
			osaGetSendProcProcInfo = kPascalStackBased
				 | RESULT_SIZE(SIZE_CODE(sizeof(OSAError)))
				 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(hdlcomponentglobals)))
				 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(OSASendUPP*)))
				 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(long*)))
		};
		
		enum {
			osaSetCreateProcProcInfo = kPascalStackBased
				 | RESULT_SIZE(SIZE_CODE(sizeof(OSAError)))
				 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(hdlcomponentglobals)))
				 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(OSACreateAppleEventUPP)))
				 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(long)))
		};
		enum {
			osaGetCreateProcProcInfo = kPascalStackBased
				 | RESULT_SIZE(SIZE_CODE(sizeof(OSAError)))
				 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(hdlcomponentglobals)))
				 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(OSACreateAppleEventUPP*)))
				 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(long*)))
		};
		enum {
			osaSetDefaultTargetProcInfo = kPascalStackBased
				 | RESULT_SIZE(SIZE_CODE(sizeof(OSAError)))
				 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(hdlcomponentglobals)))
				 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(AEAddressDesc*)))
		};


		
		/*
			For Carbon we have to build univeral procedure pointers at runtime.
			So we just declare the UPPs here.
		*/
		
		ComponentRoutineUPP	cmpcloseDesc;
		ComponentRoutineUPP	cmpcandoDesc;
		ComponentRoutineUPP	cmpversionDesc;
		ComponentRoutineUPP	osaLoadDesc;
		ComponentRoutineUPP	osaStoreDesc;
		ComponentRoutineUPP	osaExecuteDesc;
		ComponentRoutineUPP	osaDisplayDesc;
		ComponentRoutineUPP	osaScriptErrorDesc;
		ComponentRoutineUPP	osaDisposeDesc;
		ComponentRoutineUPP	osaSetScriptInfoDesc;
		ComponentRoutineUPP	osaGetScriptInfoDesc;
		ComponentRoutineUPP	osaCompileDesc;
		ComponentRoutineUPP	osaGetSourceDesc;
		ComponentRoutineUPP	osaCoerceFromDescDesc;
		ComponentRoutineUPP	osaCoerceToDescDesc;
		ComponentRoutineUPP	osaStartRecordingDesc;
		ComponentRoutineUPP	osaStopRecordingDesc;
		ComponentRoutineUPP	osaScriptingComponentNameDesc;
		ComponentRoutineUPP	osaLoadExecuteDesc;
		ComponentRoutineUPP	osaCompileExecuteDesc;
		ComponentRoutineUPP	osaDoScriptDesc;
		ComponentRoutineUPP	osaMakeContextDesc;
		ComponentRoutineUPP	osaSetResumeDispatchProcDesc;
		ComponentRoutineUPP	osaGetResumeDispatchProcDesc;
		ComponentRoutineUPP	osaExecuteEventDesc;
		ComponentRoutineUPP	osaDoEventDesc;
		ComponentRoutineUPP	osaSetActiveProcDesc;
		ComponentRoutineUPP	osaSetDebugProcDesc;
		ComponentRoutineUPP	osaDebugDesc;
		ComponentRoutineUPP	osaSetSendProcDesc;
		ComponentRoutineUPP	osaGetSendProcDesc;
		ComponentRoutineUPP	osaSetCreateProcDesc;
		ComponentRoutineUPP	osaGetCreateProcDesc;
	
		OSAActiveUPP	osaclientactiveDesc;
		AEIdleUPP	osaclientidleDesc;
		OSASendUPP	osaclientsendDesc;
		
		#define osaclientactiveUPP (osaclientactiveDesc)
		#define osaclientidleUPP (osaclientidleDesc)
		#define osaclientsendUPP (osaclientsendDesc)

	/*
	static RoutineDescriptor osaSetDefaultTargetDesc = BUILD_ROUTINE_DESCRIPTOR (osaSetDefaultTargetProcInfo, osaSetDefaultTarget);
	*/

#define kOSAScriptIsBeingEdited			'edit'
	// Selector returns boolean.

#define kOSAScriptIsBeingRecorded		'recd'
	// Selector returns boolean.

#pragma pack(2)
typedef struct tyservercomponent {
	
	struct tyservercomponent **hnext;
	
	ComponentInstance instance;
	
	ProcessSerialNumber clientpsn; /*3.015*/
	
	OSType type;
	} tyservercomponent, *ptrservercomponent, **hdlservercomponent;


typedef struct tyclientlist {
	
	hdlcomponentglobals hfirst;
	
	} tyclientlist, **hdlclientlist;
#pragma options align=reset


enum { /*recording strings*/
	
	nullstring = 1,
	
	afterstring,
	
	beforestring,
	
	beginningofstring,
	
	endofstring,
	
	replacingstring,
	
	insertionlocstring,
	
	withobjectmodelstring,
	
	bringtofrontstring,
	
	sysbringapptofrontstring,
	
	idstring,
	
	appleeventstring,
	
	noverbtablestring,
	
	multipleclientsstring,
	
	specificclientstring
	};


Component osacomponent = nil;

boolean flosashutdown = false;


static hdlservercomponent hserverlist = nil;

static hdlclientlist hclientlist = nil;


static byte bssource [] = "\p_source";

static byte bscode [] = "\p_code";


/*
static byte bscontext [] = "\p_context";

static byte bsmodeflags [] = "\p_flags";
*/



/*
typedef struct tystubrecord {
	
	unsigned short jmp;
	
	ProcPtr adr;
	} tystubrecord, **hdlstubrecord;
*/

#pragma pack(2)
typedef struct tystylerecord {
	
	short ctstyles;
	
	ScrpSTElement styles [1];
	} tystylerecord;
#pragma options align=reset


static THz homezone;


static short homeresfile;

static ProcessSerialNumber homepsn;


hdlcomponentglobals osaglobals = nil;

static long osacoercionhandlerinstalled = 0;

static long osabackgroundtime = 0;



void disposecomponentglobals (hdlcomponentglobals hglobals) {
	
	/*
	dispose globals entirely.
	
	need to swap in our tablestack while disposing storage table in case 
	global environment isn't hospitable
	*/
	
	register hdlcomponentglobals hcg = hglobals;
	register hdltablestack hs = hashtablestack; /*save*/
	long ctbytes = longinfinity;
	
	if (hcg == nil)
		return;
	
	listunlink ((hdllinkedlist) hclientlist, (hdllinkedlist) hcg);
	
	hashtablestack = (**(**hcg).clientthreadglobals).htablestack;
		
  DisposeOSAActiveUPP((**hcg).activeproc);
  DisposeOSASendUPP((**hcg).sendproc);
  DisposeOSACreateAppleEventUPP((**hcg).createproc);
  
  DisposeComponentFunctionUPP((**hcg).cmpcloseUPP);
  DisposeComponentFunctionUPP((**hcg).cmpcandoUPP);
  DisposeComponentFunctionUPP((**hcg).cmpversionUPP);
  DisposeComponentFunctionUPP((**hcg).osaLoadUPP);
  DisposeComponentFunctionUPP((**hcg).osaStoreUPP);
  DisposeComponentFunctionUPP((**hcg).osaExecuteUPP);
  DisposeComponentFunctionUPP((**hcg).osaDisplayUPP);
  DisposeComponentFunctionUPP((**hcg).osaScriptErrorUPP);
  DisposeComponentFunctionUPP((**hcg).osaDisposeUPP);
  DisposeComponentFunctionUPP((**hcg).osaSetScriptInfoUPP);
  DisposeComponentFunctionUPP((**hcg).osaGetScriptInfoUPP);
  DisposeComponentFunctionUPP((**hcg).osaCompileUPP);
  DisposeComponentFunctionUPP((**hcg).osaGetSourceUPP);
  DisposeComponentFunctionUPP((**hcg).osaCoerceFromDescUPP);
  DisposeComponentFunctionUPP((**hcg).osaCoerceToDescUPP);
  DisposeComponentFunctionUPP((**hcg).osaStartRecordingUPP);
  DisposeComponentFunctionUPP((**hcg).osaStopRecordingUPP);
  DisposeComponentFunctionUPP((**hcg).osaScriptingComponentNameUPP);
  DisposeComponentFunctionUPP((**hcg).osaLoadExecuteUPP);
  DisposeComponentFunctionUPP((**hcg).osaCompileExecuteUPP);
  DisposeComponentFunctionUPP((**hcg).osaDoScriptUPP);
  DisposeComponentFunctionUPP((**hcg).osaMakeContextUPP);
  DisposeComponentFunctionUPP((**hcg).osaSetResumeDispatchProcUPP);
  DisposeComponentFunctionUPP((**hcg).osaGetResumeDispatchProcUPP);
  DisposeComponentFunctionUPP((**hcg).osaExecuteEventUPP);
  DisposeComponentFunctionUPP((**hcg).osaDoEventUPP);
  DisposeComponentFunctionUPP((**hcg).osaSetActiveProcUPP);
  DisposeComponentFunctionUPP((**hcg).osaSetDebugProcUPP);
  DisposeComponentFunctionUPP((**hcg).osaDebugUPP);
  DisposeComponentFunctionUPP((**hcg).osaSetSendProcUPP);
  DisposeComponentFunctionUPP((**hcg).osaGetSendProcUPP);
  DisposeComponentFunctionUPP((**hcg).osaSetCreateProcUPP);
  DisposeComponentFunctionUPP((**hcg).osaGetCreateProcUPP);
		
	disposehashtable ((**hcg).storagetable, false);
	
	hashtablestack = hs; /*restore*/
	
	// assert ((**hcg).clientthreadglobals != getcurrentthreadglobals ());
	
	disposethreadglobals ((**hcg).clientthreadglobals);
	
	/*
	disposehandle ((Handle) (**hcg).hMSglobals);
	*/
	
	disposehandle ((Handle) hcg);
	
	hashflushcache (&ctbytes); // 5.1b23 dmb: don't reuse any handles allocated
	} /*disposecomponentglobals*/


pascal OSErr osadefaultactiveproc (long refcon) {
#pragma unused (refcon)
	/*
	see if user pressed cmd-period.
	
	this gets called in client's context, so don't call any of our routines. in 
	any case, keyboardescape has its own timing mechanism that we don't want.
	
	4.1b13 dmb: use new iscmdperiodevent
	*/
	
	EventRecord ev;
  
	//Code change by Timothy Paustian Friday, June 16, 2000 1:35:10 PM
	//Changed to Opaque call for Carbon
	//updated to new call
  EventTypeSpec eventType;
  eventType.eventClass = kEventClassKeyboard;
  eventType.eventKind = kEventRawKeyDown;
  EventRef ref = AcquireFirstMatchingEventInQueue(GetCurrentEventQueue(), 1, &eventType, kEventQueueOptionsNone);
  if (ref) {
    ConvertEventRefToEventRecord(ref, &ev);
    
    if (iscmdperiodevent (ev.message, ev.what, ev.modifiers)) {
      return (userCanceledErr);
    }
  }
	return (noErr);
	} /*osadefaultactiveproc*/


static pascal OSErr
osadefaultcreate (
		AEEventClass		 class,
		AEEventID			 id,
		const AEAddressDesc	*target,
		short				 returnID,
		long				 transactionID,
		AppleEvent			*result,
		long				 refcon)
{
#pragma unused (refcon)

	return (AECreateAppleEvent(class, id, target, returnID, transactionID, result));
	} /*osadefaultcreate*/


static pascal OSErr
osadefaultsend (
		const AppleEvent	*event,
		AppleEvent			*reply,
		AESendMode			 sendmode,
		AESendPriority		 priority,
		long				 timeout,
		AEIdleUPP			 idleproc,
		AEFilterUPP			 filterproc,
		long				 refcon)
{
#pragma unused (refcon)

	return (AESend (event, reply, sendmode, priority, timeout, idleproc, filterproc));
	} /*osadefaultsend*/



	//Code change by Timothy Paustian Friday, July 21, 2000 10:52:57 PM
	//I think I can get away with this because only frontier code calls it.


OSAActiveUPP osadefaultactiveDesc = nil;
OSACreateAppleEventUPP osadefaultcreateDesc = nil;
OSASendUPP osadefaultsendDesc = nil;

#define osadefaultactiveUPP (osadefaultactiveDesc)
#define osadefaultcreateUPP (osadefaultcreateDesc)
#define osadefaultsendUPP (osadefaultsendDesc)

boolean newcomponentglobals (Component self, long clienta5, hdlcomponentglobals *hglobals) {
	
	/*
	encapsulate as much state information as possible for this component instance.
	
	we include an entire set of thread globals to avoid ad-hoc saving & restoring of 
	shell and lang globals & callbacks.
	
	5.0b7 dmb: allocate storage table in our own heap; langhash caches them
	*/
	
	register hdlcomponentglobals hcg;
	register hdlthreadglobals htg;
	hdlhashtable storagetable;
	hdlthreadglobals hthreadglobals;
	ProcessSerialNumber psn;
	OSType appid;
	Boolean flsame;
	boolean fl;
	
	if (!newclearhandle (sizeof (tycomponentglobals), (Handle *) hglobals))
		return  (false);
	
	hcg = *hglobals;
	
	listlink ((hdllinkedlist) hclientlist, (hdllinkedlist) hcg);
	
	GetCurrentProcess (&psn);
	
	appid = getprocesscreator ();
	
	if ((SameProcess (&psn, &homepsn, &flsame) == noErr) && flsame)
		(**hcg).isHomeProcess = true;
	
	(**hcg).self = self;
	
	(**hcg).clienta5 = clienta5;
	
	//Code change by Timothy Paustian Friday, June 16, 2000 1:39:07 PM
	//Changed to Opaque call for Carbon - you can't use this in carbon anyway
	
		
	(**hcg).clientid = appid;
	
	(**hcg).clientpsn = psn;
	
		
	fl = newhashtable (&storagetable);
	
	
	if (!fl) {
	
		disposehandle ((Handle) hcg);
	
		return (false);
		}
	
	(**hcg).storagetable = storagetable;
	
	
  (**hcg).activeproc = NewOSAActiveUPP(osadefaultactiveproc);
  (**hcg).createproc = NewOSACreateAppleEventUPP(osadefaultcreate);
  (**hcg).sendproc = NewOSASendUPP(osadefaultsend);

  //Code change by Timothy Paustian Sunday, September 3, 2000 9:57:20 PM
  //We have to create theses all and store them in the globals variables.

  (**hcg).cmpcloseUPP = NewComponentFunctionUPP(cmpclose, cmpcloseProcInfo);
  (**hcg).cmpcandoUPP = NewComponentFunctionUPP((ProcPtr) cmpcando, cmpcandoProcInfo);
  (**hcg).cmpversionUPP = NewComponentFunctionUPP(cmpversion, cmpversionProcInfo);
  (**hcg).osaLoadUPP = NewComponentFunctionUPP(osaLoad, osaLoadProcInfo);
  (**hcg).osaStoreUPP = NewComponentFunctionUPP(osaStore, osaStoreProcInfo);
  (**hcg).osaExecuteUPP = NewComponentFunctionUPP(osaExecute, osaExecuteProcInfo);
  (**hcg).osaDisplayUPP = NewComponentFunctionUPP(osaDisplay, osaDisplayProcInfo);
  (**hcg).osaScriptErrorUPP = NewComponentFunctionUPP(osaScriptError, osaScriptErrorProcInfo);
  (**hcg).osaDisposeUPP = NewComponentFunctionUPP(osaDispose, osaDisposeProcInfo);
  (**hcg).osaSetScriptInfoUPP = NewComponentFunctionUPP(osaSetScriptInfo, osaSetScriptInfoProcInfo);
  (**hcg).osaGetScriptInfoUPP = NewComponentFunctionUPP(osaGetScriptInfo, osaGetScriptInfoProcInfo);
  (**hcg).osaCompileUPP = NewComponentFunctionUPP(osaCompile, osaCompileProcInfo);
  (**hcg).osaGetSourceUPP = NewComponentFunctionUPP(osaGetSource, osaGetSourceProcInfo);
  (**hcg).osaCoerceFromDescUPP = NewComponentFunctionUPP(osaCoerceFromDesc, osaCoerceFromDescProcInfo);
  (**hcg).osaCoerceToDescUPP = NewComponentFunctionUPP(osaCoerceToDesc, osaCoerceToDescProcInfo);
  (**hcg).osaStartRecordingUPP = NewComponentFunctionUPP(osaStartRecording, osaStartRecordingProcInfo);
  (**hcg).osaStopRecordingUPP = NewComponentFunctionUPP(osaStopRecording, osaStopRecordingProcInfo);
  (**hcg).osaScriptingComponentNameUPP = NewComponentFunctionUPP(osaScriptingComponentName, osaScriptingComponentNameProcInfo);
  (**hcg).osaLoadExecuteUPP = NewComponentFunctionUPP(osaLoadExecute, osaLoadExecuteProcInfo);
  (**hcg).osaCompileExecuteUPP = NewComponentFunctionUPP(osaCompileExecute, osaCompileExecuteProcInfo);
  (**hcg).osaDoScriptUPP = NewComponentFunctionUPP(osaDoScript, osaDoScriptProcInfo);
  (**hcg).osaMakeContextUPP = NewComponentFunctionUPP(osaMakeContext, osaMakeContextProcInfo);
  (**hcg).osaSetResumeDispatchProcUPP = NewComponentFunctionUPP(osaSetResumeDispatchProc, osaSetResumeDispatchProcProcInfo);
  (**hcg).osaGetResumeDispatchProcUPP = NewComponentFunctionUPP(osaGetResumeDispatchProc, osaGetResumeDispatchProcProcInfo);
  (**hcg).osaExecuteEventUPP = NewComponentFunctionUPP(osaExecuteEvent, osaExecuteEventProcInfo);
  (**hcg).osaDoEventUPP = NewComponentFunctionUPP(osaDoEvent, osaDoEventProcInfo);
  (**hcg).osaSetActiveProcUPP = NewComponentFunctionUPP(osaSetActiveProc, osaSetActiveProcProcInfo);
  (**hcg).osaSetDebugProcUPP = NewComponentFunctionUPP(osaSetDebugProc, osaSetDebugProcProcInfo);
  (**hcg).osaDebugUPP = NewComponentFunctionUPP(osaDebug, osaDebugProcInfo);
  (**hcg).osaSetSendProcUPP = NewComponentFunctionUPP(osaSetSendProc, osaSetSendProcProcInfo);
  (**hcg).osaGetSendProcUPP = NewComponentFunctionUPP(osaGetSendProc, osaGetSendProcProcInfo);
  (**hcg).osaSetCreateProcUPP = NewComponentFunctionUPP(osaSetCreateProc, osaSetCreateProcProcInfo);
  (**hcg).osaGetCreateProcUPP = NewComponentFunctionUPP(osaGetCreateProc, osaGetCreateProcProcInfo);
  
	if (!newthreadglobals (&hthreadglobals)) {
		
		disposecomponentglobals (hcg);
		
		return (false);
		}
	
	htg = hthreadglobals;
	
	(**hcg).clientthreadglobals = htg;	// 4.1b3 dmb: use local
	
	(**htg).htable = storagetable;
	
	(**htg).applicationid = appid;
	
	#if version42orgreater
		// 2/28/97 dmb: set up langcallbacks here so they'll always be in effect
		
		(**htg).langcallbacks.backgroundtaskcallback = &osabackgroundtask;
		
		(**htg).langcallbacks.debuggercallback = &osadebugger;
		
		(**htg).langcallbacks.pushsourcecodecallback = &scriptpushsourcecode;
		
		(**htg).langcallbacks.popsourcecodecallback = &scriptpopsourcecode;
		
		(**htg).langcallbacks.partialeventloopcallback = &osapartialeventloop;
		
		(**htg).fldisableyield = true;
	#endif
	
	return (true);
	} /*newcomponentglobals*/


OSAError osageterror (void) {
	
	long n;
	
	n = getoserror ();
	
	if (n == noErr)
		n = errOSAScriptError;
	
	return (n);
	} /*osageterror*/


static boolean inosasource (void) {
	
	/*
	very carefully call back to the client's debugging proc.
	*/
	
	register hdlerrorstack hs = langcallbacks.scripterrorstack;
	
	if ((hs != nil) && ((**hs).toperror > 1))
		if ((**hs).stack [(**hs).toperror - 1].errorrefcon != -1L) /*we're in a call to another script*/
			return (false);
	
	return (true);
	} /*inosasource*/


static boolean osaerrormessage (bigstring bs, ptrvoid refcon) {
#pragma unused (refcon)

	AEDesc
		list,
		rec,
		desc;
	long n;
	OSErr err;
	
	list = (**osaglobals).errordesc;
	
	AEDisposeDesc (&list);
	
	n = osageterror ();
	
	err = AECreateList (nil, 0, true, &list);
	
	if (err != noErr) {
		
			newdescnull (&list, typeNull);
		
		}
	else {
		
		err = AEPutKeyPtr (&list, kOSAErrorNumber, typeLongInteger, (Ptr) &n, sizeof (n));
		
		err = AEPutKeyPtr (&list, kOSAErrorMessage, typeChar, (Ptr) bs + 1, stringlength (bs));
		
		err = AEPutKeyPtr (&list, kOSAErrorBriefMessage, typeChar, (Ptr) bs + 1, stringlength (bs));
		
		err = AECreateList (nil, 0, true, &rec);
		
		if (err == noErr) {
			
			long ix;
			
			if (inosasource ())
				ix = langgetsourceoffset (ctscanlines, ctscanchars);
			else
				ix = 0;
			
			err = AEPutKeyPtr (&rec, keyOSASourceStart, typeLongInteger, (Ptr) &ix, sizeof (ix));
			
			err = AEPutKeyPtr (&rec, keyOSASourceEnd, typeLongInteger, (Ptr) &ix, sizeof (ix));
			
			err = AECoerceDesc (&rec, typeOSAErrorRange, &desc);
			
			AEDisposeDesc (&rec);
			
			if (err == noErr) {
				
				err = AEPutKeyDesc (&list, kOSAErrorRange, &desc);
				
				AEDisposeDesc (&desc);
				}
			}
		}
	
	(**osaglobals).errordesc = list;
	
	return (false); /*consume the error*/
	} /*osaerrormessage*/


static pascal OSErr
coerceTEXTtoSTXT (
		DescType	 fromtype,
		Ptr			 pdata,
		long		 size,
		DescType	 totype,
		long		 refcon,
		AEDesc		*result) {
#pragma unused(fromtype, refcon)
	/*
	2.1b2 dmb: don't use clearbytes so we don't have to set up a5
	*/
	
	tystylerecord stylerecord;
	OSErr err;
	AEDesc list;
	register ScrpSTElement *pstyle;
	
	#ifdef fldebug	// 2006-04-04 - kw --- this was fldegug
	
	if (totype != typeStyledText)
		DebugStr ("\punexpected coercion");
	
	#endif
	
	stylerecord.ctstyles = 1;
	
	pstyle = &stylerecord.styles [0];
	
	(*pstyle).scrpStartChar = 0;
	
	(*pstyle).scrpHeight = 14;
	
	(*pstyle).scrpAscent = 12;
	
	(*pstyle).scrpFont = geneva;
	
	(*pstyle).scrpSize = 9;
	
	(*pstyle).scrpFace = 0;
	
	(*pstyle).scrpColor.red = 0;
	
	(*pstyle).scrpColor.green = 0;
	
	(*pstyle).scrpColor.blue = 0;
	
	err = AECreateList (nil, 0, true, &list);
	
	if (err == noErr) {
		
		err = AEPutKeyPtr (&list, 'ksty', 'styl', (Ptr) &stylerecord, sizeof (stylerecord));
		
		if (err == noErr)
			err = AEPutKeyPtr (&list, 'ktxt', typeChar, pdata, size);
		
		if (err == noErr)
			err = AECoerceDesc (&list, typeStyledText, result);
		
		AEDisposeDesc (&list);
		}
	
	return (err);
	} /*coerceTEXTtoSTXT*/


static pascal OSErr
coerceTypetoObj (
		AEDesc		*desc,
		DescType	 totype,
		long		 refcon,
		AEDesc		*result) {
#pragma unused(totype, refcon)

	/*
	2.1b1 dmb: if Frontier passes a string4 identifier where an object specifier 
	is required, this coercion handler will take care of it
	*/
	
	AEDesc containerdesc;
	OSErr err;
	
	
	#ifdef fldebug
		if ((*desc).descriptorType != typeType)
			DebugStr ("\punexpected coercion");
	#endif
	
  /*PBS 03/14/02: AE OS X fix.*/
  newdescnull (&containerdesc, typeNull);
	
	err = CreateObjSpecifier (cProperty, &containerdesc, formPropertyID, desc, false, result);
	
	return (err);
	} /*coerceTypetoObj*/


//Code change by Timothy Paustian Friday, June 16, 2000 1:38:13 PM
//Changed to Opaque call for Carbon

AECoercionHandlerUPP	coerceTEXTtoSTXTDesc;
AECoercionHandlerUPP	coerceTypetoObjDesc;

#define coerceTEXTtoSTXTUPP (coerceTEXTtoSTXTDesc)
#define coerceTypetoObjUPP (coerceTypetoObjDesc)

static void osapushfastcontext (hdlcomponentglobals hglobals) {
	
	/*
	4/20/93 dmb: always set client's ccglobals to nil. if root has been closed, 
	we want nil globals to persist, or new globals to be picked up
	
	3.0b14 dmb: handling nested osa contexts
	
	4.1b3 dmb: nesting wasn't thoroughly handled by the hserverosaglobals field.
	added ctpushes field that handles nesting better, assuming it's simple nesting 
	of the same globals. to handle arbitrary, daisy chain globals nesting, we'd 
	need to keep a list or a stack of serverthreadglobals.
	*/
	
	register hdlcomponentglobals hcg = hglobals;
	register hdlthreadglobals htg;
	
	/*
	short rnum = OpenComponentResFile ((**hcg).self);
	*/
	
	if ((**hcg).ctpushes++ > 0) { // 4.1b3 dmb: nesting of same globals
	
		assert (osaglobals == hcg);
		
		assert ((**hcg).serverthreadglobals);
		
		assert ((**hcg).clientthreadglobals == getcurrentthreadglobals ());
		}
	else {
	
		(**hcg).hserverosaglobals = osaglobals; /*in case we're nested*/
		
		osaglobals = hcg; /*make visible to callbacks*/
		
		htg = getcurrentthreadglobals ();
		
		(**hcg).serverthreadglobals = htg;
		
		copythreadglobals (htg); /*save*/
		
		htg = (**hcg).clientthreadglobals;
		
		
		(**htg).hccglobals = nil; /*want to leave them untouched*/
		
		
		swapinthreadglobals (htg);
		}
	
	if (++osacoercionhandlerinstalled == 1) {
    if(coerceTEXTtoSTXTUPP == nil)
      coerceTEXTtoSTXTUPP = NewAECoerceDescUPP((AECoerceDescProcPtr)coerceTEXTtoSTXT);

		AEInstallCoercionHandler (typeChar, typeStyledText, coerceTEXTtoSTXTUPP, 0, false, false);
		}
	
	if (!(**hcg).isHomeProcess) {
	
		(**hcg).clientresfile = CurResFile ();
		
		//Code change by Timothy Paustian Friday, June 16, 2000 2:08:20 PM
		//Changed to Opaque call for Carbon
		//This is not supported in carbon. You cannot use res files in OS X
		//We may have some serious rewriting to do for this.
		
		UseResFile (homeresfile);
		}
	} /*osapushfastcontext*/


static void osapopfastcontext (hdlcomponentglobals hglobals) {
	
	/*
	3.0b14 dmb: handling nested osa contexts
	*/
	
	register hdlcomponentglobals hcg = hglobals;
	
	/*
	CloseComponentResFile (rnum);
	*/
	
	if (--(**hcg).ctpushes == 0) { // 4.1b3 dmb: no nesting of same globals
	
		copythreadglobals ((**hcg).clientthreadglobals);
		
		swapinthreadglobals ((**hcg).serverthreadglobals);
		
		osaglobals = (**hcg).hserverosaglobals; /*in case we were nested*/
		}
	
	if (--osacoercionhandlerinstalled == 0)
	{
		AERemoveCoercionHandler (typeChar, typeStyledText, coerceTEXTtoSTXTUPP, false);

		//Code change by Timothy Paustian Friday, July 21, 2000 11:02:21 PM
		//added dispose of coercion handler
    if(coerceTEXTtoSTXTUPP != nil) {
      DisposeAECoerceDescUPP(coerceTEXTtoSTXTUPP);
      coerceTEXTtoSTXTUPP = nil;
      }
	}
	if (!(**hcg).isHomeProcess) {
		UseResFile ((**hcg).clientresfile);
		}
	} /*osapopfastcontext*/


long osapreclientcallback (hdlcomponentglobals hglobals) {
	
	/*
	before we put up standard file, or the ppc browser, we need to put the 
	clients environment in order, and make sure that the Frontier environment 
	is restored, since it may get swapped in by the process manager.
	
	case in point: Norton Directory Assistance trashes both our heap & the 
	client's if we don't do this.
	
	since we're split into a pre- and post- routine, our caller has to manage 
	A5 for us. (we could probably get around this, but I think it would be 
	even messier.)
	*/
	
	register hdlcomponentglobals hcg = hglobals;
	
	osapopfastcontext (hcg);
	
	return ((**hcg).clienta5);
	} /*osapreclientcallback*/


void osapostclientcallback (hdlcomponentglobals hglobals) {
	
	osapushfastcontext (hglobals);
	} /*osapostclientcallback*/



GNEUPP osainstallpatch (hdlcomponentglobals hglobals) {
#pragma unused (hglobals)

	return (nil);
	} /*osainstallpatch*/


void osaremovepatch (hdlcomponentglobals hglobals, GNEUPP origproc) {
#pragma unused (hglobals, origproc)

	} /*osaremovepatch*/



static boolean osapartialeventloop (short desiredevents) {
	
	/*
	2.1b8 dmb: we can't call waitnextevent on behalf of the client, because 
	the OSA doesn't provide an API for safely doing so. so we return false.
	
	in case we're being called to handle a process switch, we'd better 
	un-hilite any menu. otherwise, the menu manager gets confused, and 
	the menubar can be trashed when the no-longer-current app calls 
	HiliteMenu (0)
	
	2.1b11 dmb: one way we can call WNE for our client is to send an 
	apple event. so we'll send one to Frontier.
	
	3.0b15 dmb: if we're our own client, sending the noop event won't do 
	use any good -- the AE will be short-circuited and return EventNotHandled, 
	and the idleproc won't be called. so we want to call our client thread's 
	partialeventloop. this should be retrievable through the clientthreadglobals 
	of our osaglobals, but we're going to take a little shortcut and just 
	call shellpartialeventloop directly. not as good as ccpartialeventloop (the 
	normal langpartialeventloop), but it'll do for now. time to ship!
	
	3.0a dmb: must do HiliteMenu (0) even if we are our own client; the 
	problem shows up in Runtime's own shared menus.
	*/
	
	HiliteMenu (0);
	
	if (iscurrentapplication (homepsn))
		return (shellpartialeventloop (desiredevents));
	
	return (langipcnoop ());
	} /*osapartialeventloop*/

static pascal OSErr osacreateevent (AEEventClass class, AEEventID id,
                    const AEAddressDesc *target, short returnID,
                    long transactionID, AppleEvent *result) {

	/*
	2.1b4 dmb: as part of "event sending" support, we need to do this. it 
	should be reasonable to assume that a client's "create" proc won't 
	call WNE, so we don't do quite as much futzing around as when sending 
	events
	*/
	
	register hdlcomponentglobals hcg = osaglobals;
	OSErr err;
	
	osapopfastcontext (hcg);
	
  err = InvokeOSACreateAppleEventUPP (class, id, target, returnID, transactionID, result, (**hcg).createprocrefcon, (**hcg).createproc);
		
	osapushfastcontext (hcg);
	
	return (err);
	} /*osacreateevent*/


static pascal OSErr
osasendevent (
		const AppleEvent	*event,
		AppleEvent			*reply,
		AESendMode			 mode,
		AESendPriority		 priority,
		long				 timeout,
		AEIdleUPP			 idleproc,
		AEFilterUPP			 filterproc )
{
#pragma unused (idleproc, filterproc)
	
	/*
	2/16/93 dmb: in case the event is to be direct dispatched correctly, we need 
	to set up the client's A5 world
	
	2.1b5 dmb: watch out for nil sendProc -- the AS script editor sets this.
	*/
	
	register hdlcomponentglobals hcg = osaglobals;
	OSErr err;
	register GNEUPP getnexteventproc;
	OSASendUPP sendproc;
	
	sendproc = (**hcg).sendproc;
	
	getnexteventproc = (**hcg).getnexteventproc;
	
	osapopfastcontext (hcg);
	
	if (getnexteventproc != nil)
		osaremovepatch (hcg, nil); /*unpatch*/
  err = InvokeOSASendUPP (event, reply, mode, priority, timeout, nil, nil, (**hcg).sendprocrefcon, sendproc);
	
	if (getnexteventproc != nil)
		osainstallpatch (hcg); /*repatch*/
	osapushfastcontext (hcg);
	
	return (err);
	} /*osasendevent*/


static boolean osabackgroundtask (boolean flresting) {
	
	/*
	very carefully call back to the client.
	*/
	
	register hdlcomponentglobals hcg = osaglobals;
	OSErr err;
	
	if ((**hcg).activeproc == nil) /*no callback provided*/
		return (true);
	
	if (langdialogrunning ()) /*no can do*/
		return (true);
	
	if (langerrorenabled ()) // 5.0b7 dmb: another time not to background
		return (true);

	if ((gettickcount () - osabackgroundtime < 20) && (!flresting)) /*not time*/
		return (true);
	
	flscriptresting = flresting;
	
	osapopfastcontext (hcg);
	
	osaremovepatch (hcg, nil); /*unpatch*/
	
  err = InvokeOSAActiveUPP ((**hcg).activeprocrefcon, (**hcg).activeproc);
	
	osainstallpatch (hcg); /*repatch*/
		
	osapushfastcontext (hcg);
	
	flscriptresting = false;
	
	osabackgroundtime = gettickcount ();
	
	return (!oserror (err));
	} /*osabackgroundtask*/


static boolean osadebugger (hdltreenode hnode) {
	
	/*
	very carefully call back to the client's debugging proc.
	*/
	
	register hdlcomponentglobals hcg = osaglobals;
	register hdltreenode hn = hnode;
	tytreetype op;
	OSErr err;
	
	if (languserescaped (false)) /*stop running the script immediately*/
		return (false);
	
	if ((**hcg).debugproc == nil) /*no callback provided*/
		return (true);
	
	if (!debuggingcurrentprocess ())
		return (true);
	
	if (!inosasource ())
		return (true);
	
	op = (**hn).nodetype; /*test for "meaty" op*/
	
	if ((op == moduleop) || (op == noop) || (op == bundleop) || (op == localop)) /*never stop on these*/
		return (true);
	
	(**hcg).debugnode = hn;
	
	osapopfastcontext (hcg);
	
	osaremovepatch (hcg, nil); /*unpatch*/
		
	err = CallOSADebugProc ((**hcg).debugproc, (**hcg).debugprocrefcon);
	
	osainstallpatch (hcg); /*repatch*/
		
	osapushfastcontext (hcg);
	
	return (!oserror (err));
	} /*osadebugger*/



static boolean osaprocessstarted (void) {
	
	/*
	we don't want Frontier's menus to dim when serving osa scripts
	*/
	
	processnotbusy ();
	
	return (true);
	} /*osaprocessstarted*/



static boolean osahandlerunscript (hdlcomponentglobals hglobals, hdltreenode hcode, 
	hdlhashtable hcontext, long modeflags, tyvaluerecord *resultval) {
	
	/*
	12/31/92 dmb: cloned from langipc.c, this will need to deal with 
	lang/threadglobals more carefully in the future
	
	1/21/93 dmb: it now deals with lang/threadglobal very carefully indeed! final 
	touch: we path getnextevent so we can deal with it like a background callback. 
	note that we may need to do this with eventavail, waitnextevent as well.
	
	6/2/93 dmb: no longer override msg verb, since it now works OK along with 
	other frontier process-based verbs
	
	2.1b5 dmb: pass -1 for errorrefcon instead of zero to prevent top level 
	lexical scope from being transparent (just like QuickScript does). for 
	isosasource to work, we also need to hook up push/popsourcecode callbacks.
	
	2.1b12 dmb: restore all langcallbacks that we set
	
	2.1b13 dmb: added processstarted routine to prevent Frontier's menus from
	dimming when running OSA scripts.
	
	3.0b15 dmb: test heap space for 2K before trying to run a process.
	
	3.0.1b1 dmb: generate error message when no file is open
	*/
	
	register hdlcomponentglobals hcg = hglobals;
	hdlprocessrecord hprocess;
	register hdlprocessrecord hp;
	register boolean fl = false;
	GNEUPP origproc;
	long errorrefcon = -1;
	
	if (roottable == nil) {
		
		langerror (nofileopenerror); /*3.0.1b1*/
		
		return (false);
		}
	
	if ((modeflags & kOSAModeTransparentScope) != 0)
		errorrefcon = 0;
		
	if (!newprocess (hcode, true, nil, errorrefcon, &hprocess))
		goto exit;
	
	hp = hprocess; /*copy into register*/
	
	(**hp).processrefcon = (long) hglobals;
	
	(**hp).errormessagecallback = &osaerrormessage;
	
	(**hp).hcontext = hcontext;
	
		
		(**hp).fldebugging = bitboolean (modeflags & kOSAModeDebug);
		
		(**hp).processstartedroutine = &osaprocessstarted;
		
	
	#if !version42orgreater
		langcallbacks.backgroundtaskcallback = &osabackgroundtask;
		
		langcallbacks.debuggercallback = &osadebugger;
		
		langcallbacks.pushsourcecodecallback = &scriptpushsourcecode;
		
		langcallbacks.popsourcecodecallback = &scriptpopsourcecode;
		
		langcallbacks.partialeventloopcallback = &osapartialeventloop;
		
		fldisableyield = true;
	#endif
	
	osabackgroundtime = gettickcount () + 30;
	
	origproc = osainstallpatch (hcg);
		
	if (testheapspace (2 * 1024)) /*enough memory to run a process*/
		fl = processruncode (hp, resultval);
	else
		fl = false;
	
	osaremovepatch (hcg, origproc);
	
	#if !version42orgreater
		langcallbacks.backgroundtaskcallback = (langbooleancallback) &truenoop;
		
		langcallbacks.debuggercallback = (langtreenodecallback) &truenoop;
		
		langcallbacks.pushsourcecodecallback = (langsourcecodecallback) &truenoop;
		
		langcallbacks.popsourcecodecallback = (langvoidcallback) &truenoop;
		
		langcallbacks.partialeventloopcallback = (langshortcallback) &falsenoop;
	#endif
	
	(**hp).hcode = nil; /*we don't own it*/
	
	disposeprocess (hp);
	
	exit:
	
	return (fl);
	} /*osahandlerunscript*/


static pascal boolean osacreatemodulecontext (hdltreenode htree, hdlhashtable *hcontext) {
	
	/*
	create a new table containing any module definitions in htree
	*/
	
	register hdltreenode h = (**htree).param1;
	tyvaluerecord val;
	
	if (!newhashtable (hcontext))
		return (false);
	
	pushhashtable (*hcontext);
	
	for (h = (**htree).param1; h != nil; h = (**h).link) {
		
		if ((**h).nodetype == moduleop)
			evaluatetree (h, &val);
		}
	
	pophashtable ();
	
	return (true);
	} /*osacreatemodulecontext*/


static boolean lookupeventname (hdlcomponentglobals hglobals, AEEventClass class, AEEventID id, bigstring bsname) {
	
	/*
	try to find the name of the specified event in the 'aete' or 'aeut' resource
	
	12/7/93, 3.x dmb: use client's context and check its aete.
	*/
	
	register hdlcomponentglobals hcg = hglobals;
	Handle haete;
	boolean flgotname;
	long paramoffset;
	
	osapopfastcontext (hcg);
	
	haete = GetIndResource ('aete', 1);
	
	flgotname = osaparseaete (haete, class, id, bsname, &paramoffset);
	
	osapushfastcontext (hcg);
	
	if (!flgotname) {
		
		haete = GetIndResource ('aeut', 1);
		
		flgotname = osaparseaete (haete, class, id, bsname, &paramoffset);
		}
	
	return (flgotname);
	
	/*
	h = GetIndResource ('aeut', 1);
	
	if (osaparseaete (h, class, id, bsname, &paramoffset))
		return (true);
	
	return (false);
	*/
	} /*lookupeventname*/


static boolean osahandletrapverb (hdlcomponentglobals hglobals, hdlverbrecord hverb, 
	hdltreenode hmodule, long modeflags, tyvaluerecord *vreturned) {
	
	/*
	2.1b3 dmb: added support for subroutine event
	
	12/7/93 3.x dmb: lookupeventname now takes hglobals
	*/
	
	register boolean fl = false;
	register hdlverbrecord hv = hverb;
	bigstring bsname;
	tyvaluerecord val;
	hdltreenode hname;
	hdltreenode hparams;
	hdltreenode hcode;
	hdlhashtable hcontext;
	AEEventClass class;
	AEEventID id;
	
	if (!osacreatemodulecontext (hmodule, &hcontext))
		return (false);
	
	class = (**hv).verbclass;
	
	id = (**hv).verbtoken;
	
	if ((class == kOSASuite) && (id == kASSubroutineEvent)) {
		
		if (!landgetstringparam (hv, keyASSubroutineName, bsname)) 
			return (false);
		}
	else {
		
		ostypetostring (id, bsname);
		
		poptrailingwhitespace (bsname);
		}
	
	if (!hashtablesymbolexists (hcontext, bsname)) {
		
		// 2/14/97 dmb: what if it's an event intended to be handled by the odb, 
		// not by this script itself?
		// 2/27/97 dmb: answer - osahandleevent will now dispatch the event to Frontier's 
		// main ae handler.
		
		if (!lookupeventname (hglobals, class, id, bsname) || !hashtablesymbolexists (hcontext, bsname)) {
			
			if ((class == 'aevt') && (id == 'oapp')) /*no run handler, exec main body*/
				fl = osahandlerunscript (hglobals, hmodule, hcontext, modeflags, vreturned);
			else
				oserror (errAEEventNotHandled);
			
			goto exit;
			}
		}
	
	initvalue (&val, stringvaluetype);
	
	if (!newtexthandle (bsname, &val.data.stringvalue))
		goto exit;
	
	if (!newidnode (val, &hname))
		goto exit;
	
	if (!langipcbuildparamlist (nil, hv, &hparams)) {
		
		langdisposetree (hname);
		
		goto exit;
		}
	
	if (!pushbinaryoperation (functionop, hname, hparams, &hcode)) /*consumes input parameters*/
		goto exit;
	
	if (!pushbinaryoperation (moduleop, hcode, nil, &hcode)) /*needs this level*/
		goto exit;
	
	fl = osahandlerunscript (hglobals, hcode, hcontext, modeflags, vreturned);
	
	langdisposetree (hcode);
	
	exit:
	
	disposehashtable (hcontext, true);
	
	return (fl);
	} /*osahandletrapverb*/


static boolean osabuildsubroutineevent (bigstring bsname, hdltreenode hparam1, AppleEvent *event) {
	
	/*
	2.1b3 dmb: build an OSA subroutine event given the name and parameter list
	
	2.1b12 dmb: the guts have been moved into langipcbuildsubroutineevent
	*/
	
	AEDesc desc = {typeNull, nil};
	register OSErr err;
	
	err = AECreateAppleEvent (kOSASuite, kASSubroutineEvent, &desc, kAutoGenerateReturnID, kAnyTransactionID, event);
	
	if (oserror (err))
		return (false);
	
	return (langipcbuildsubroutineevent (event, bsname, hparam1)); /*disposes event on error*/
	} /*osabuildsubroutineevent*/


static boolean setstorageval (hdlcomponentglobals hglobals, tyvaluerecord *val, OSAID id) {
#pragma unused (hglobals)

	/*
	add val to the id table using the indicated id. on error, dispose of the value
	*/
	
	bigstring bs;
	
	numbertostring (id, bs);
	
	if (hashassign (bs, *val)) {
		
		exemptfromtmpstack (val);
		
		return (true);
		}
	
	disposevaluerecord (*val, false);
	
	return (false);
	} /*setstorageval*/


static boolean addstorageval (hdlcomponentglobals hglobals, tyvaluerecord *val, OSAID *id) {
	
	*id = (OSAID) ++(**hglobals).idcounter;
	
	return (setstorageval (hglobals, val, *id));
	} /*addstorageval*/


static boolean getstorageval (hdlcomponentglobals hglobals, OSAID id, tyvaluerecord *val, hdlhashnode * hnode) {
#pragma unused (hglobals)

	bigstring bs;
	
	numbertostring (id, bs);
	
	if (!hashlookup (bs, val, hnode)) {
		
		oserror (errOSAInvalidID);
		
		return (false);
		}
	
	return (true);
	} /*getstorageval*/


static boolean deletestorageval (hdlcomponentglobals hglobals, OSAID id) {
#pragma unused (hglobals)

	bigstring bs;
	
	numbertostring (id, bs);
	
	return (hashdelete (bs, true, false));
	} /*deletestorageval*/


static boolean storagevaltodesc (tyvaluerecord *val, OSType desctype, AEDesc *result) {
	
	/*
	create a descriptor containing a copy of val's data, coercing 
	to the requested type
	*/
	
	register tyvaluerecord *v;
	AEDesc desc;
	tyvaluerecord vtemp;
	OSErr err;
	tyvaluetype valtype;
	
	if (!copyvaluerecord (*val, &vtemp))
		return (false);
	
	v = &vtemp; /*copy into register*/
	
	if (desctype == typeStyledText)
		valtype = stringvaluetype;
	else
		valtype = langgetvaluetype (desctype);
	
	if (langgoodbinarytype (valtype)) { /*desired type is a valid Frontier type*/
		
		if (!coercevalue (v, valtype)) { /*apply UserTalk coercion*/
			
			disposevaluerecord (*v, true);
			
			return (false);
			}
		}
	
	if (!valuetodescriptor (v, &desc))
		return (false);
	
	if ((desctype != typeWildCard) && (desctype != desc.descriptorType)) { /*AE coercion needed*/
		
		err = AECoerceDesc (&desc, desctype, result);
		
		AEDisposeDesc (&desc);
		
		return (!oserror (err));
		}
	
	*result = desc;
	
	return (true);
	} /*storagevaltodesc*/


static boolean osagetcontext (hdlcomponentglobals hglobals, OSAID id, hdlhashtable *hcontext) {
	
	tyvaluerecord vcontext;
	hdlhashnode hnode;
	
	if (id == kOSANullScript)
		*hcontext = nil;
	
	else {
		
		if (!getstorageval (hglobals, id, &vcontext, &hnode))
			return (false);
		
		if (!langexternalvaltotable (vcontext, hcontext, hnode)) {
			
			oserror (errOSAInvalidID);
			
			return (false);
			}
		}
	
	return (true);
	} /*osagetcontext*/


static boolean osanewvalue (tyexternalid id, Handle hdata, tyvaluerecord *val) {
	
	/*
	5.1b23 dmb: langexternalnewvalue does stuff with prefs that may load an odb table 
	into memory. we need our own zone.
	*/
	
	boolean fl;

	fl = langexternalnewvalue (id, hdata, val);

  return (fl);
	} /*osanewvalue*/


static pascal OSAError
osaLoad (
		hdlcomponentglobals	 hglobals,
		const AEDesc		*scriptData,
		long				 modeFlags,
		OSAID				*resultingCompiledScriptID)
{
#pragma unused (modeFlags)
	
	/*
	2.1b1 dmb: don't insist that the loaded value is a context (i.e. a table)
	*/
	Handle		descData = nil;
	tyvaluerecord vscript, vsource, vcode;
	hdlhashtable hcontext;
	Handle hsource;
	hdltreenode hcode;
	Handle hdata;
	DescType subtype;
	boolean fl = false;
	OSAError err = noErr;
	hdlhashnode hnodesource;
	hdlhashnode hnodecode;
	
  if (!copydatahandle ((AEDesc*)scriptData, &descData))	/* AE OS X fix */
    return (memFullErr);
	
	switch (scriptData->descriptorType) {
		
		case typeLAND:
			
			fl = langunpackvalue (descData, &vscript);
			
			break;
		
		case typeOSAGenericStorage:
	
			err = OSAGetStorageType ((AEDataStorage) descData, &subtype);
			
			if (err != noErr)
				break;
			
			if (subtype != typeLAND) {
				err = errOSABadStorageType;
				break;
				}
			
			if (!copyhandle (descData, &hdata)) {
				err = memFullErr;
				break;
				}
			
			#ifdef SWAP_BYTE_ORDER
			{
				/*
				2006-04-17 aradke: This is a MAJOR HACK (FIXME)!
				
					Compiled osa scripts of the generic storage type have a 12-byte trailer:
					
					Bytes 0-3 contain the signature of the OSA component that knows
					how to execute the script, e.g. 'LAND' for the UserTalk component.
					The meaning of bytes 4-5 is unknown.
					Bytes 6-7 seem to contain the length of the trailer as a 16bit integer.
					Bytes 8-11 contain 0xFADEDEAD as a magic signature for the trailer.
								
					Example: 4C41 4E44 0001 000C FADE DEAD (see 'scpt' resource #1024 in iowaruntime.r)
					
					OSARemoveStorageType is supposed to remove this trailer if it is present.
					However, it doesn't perform byte-order swapping on the length bytes in the trailer.
					The call below ends up removing 3072 bytes instead of 12 bytes from hdata.
					
					Until we figure out how to properly deal with this bug(?), we use our own
					implementation of OSARemoveStorageType on Intel Macs.
				*/
				
				long hlen = gethandlesize (hdata);
				char * p = *hdata;
				long marker;
				
				p += (hlen - 4);
				
				marker = *(long *) p;
				
				disktomemlong (marker);
				
				if (marker == 0xFADEDEAD) {
					sethandlesize (hdata, hlen - 12);	//remove storage type trailer
					}
			}
			#else
				err = OSARemoveStorageType ((AEDataStorage) hdata);
			#endif
			
			if (err == noErr)
				fl = langunpackvalue (hdata, &vscript);

			disposehandle (hdata);
			
			break;
		
		default:
			err = errOSABadStorageType;
		}

  if(descData != nil) {

    disposehandle(descData);	/* AE OS X fix */

    descData = nil;
    }
	
	if (err != noErr)
		return (err);
	
	if (fl) {

		if (langexternalvaltotable (vscript, &hcontext, HNoNode)) {
			
			pushhashtable (hcontext);
			
			if (!hashlookup (bscode, &vcode, &hnodecode) && hashlookup (bssource, &vsource, &hnodesource)) { /*no code, but have source*/
				
				fl = copyhandle (vsource.data.stringvalue, &hsource);
				
				if (fl)
					fl = langbuildtree (hsource, false, &hcode); /*syntax error*/
				
				if (fl) {
					
					initvalue (&vcode, codevaluetype);
					
					vcode.data.codevalue = hcode;
					
					fl = hashinsert (bscode, vcode);
					}
				}
			
			(**hcontext).fldirty = false;
			
			pophashtable ();
			}
		}

	if (fl)
		fl = addstorageval (hglobals, &vscript, resultingCompiledScriptID);
	
	if (!fl)
		return (osageterror ());
	
	return (noErr);
	} /*osaLoad*/


static pascal OSAError osaStore (
			hdlcomponentglobals	hglobals,
			OSAID				compiledScriptID, 
			DescType			desiredType,
			long				modeFlags,
			AEDesc*				resultingScriptData) {
	
	/*
	4/26/93 dmb: support kOSAModePreventGetSource
	*/
	
	tyvaluerecord val;
	AEDesc desc;
	DescType descType;
	hdlhashtable hcontext;
	hdlhashnode hnode;
	boolean flunlinkedsource = false;
	boolean fl;
	OSAError err;
	Handle hpacked = nil;
	
	
	if (compiledScriptID == kOSANullScript)
		return (errOSAInvalidID);
	
	if ((desiredType != typeOSAGenericStorage) && (desiredType != typeLAND))
		return (errOSABadStorageType);
	
	if (!getstorageval (hglobals, compiledScriptID, &val, &hnode))
		return (errOSAInvalidID);
	
	if (langexternalvaltotable (val, &hcontext, hnode)) {
		
		if (modeFlags & kOSAModePreventGetSource) {
			
			pushhashtable (hcontext);
			
			flunlinkedsource = hashunlink (bssource, &hnode);
			
			pophashtable ();
			}
		}

	fl = langpackvalue (val, &hpacked, HNoNode);
	
	if (flunlinkedsource)
		hashinsertnode (hnode, hcontext);
	
	if (!fl)
		return (memFullErr);

	if (desiredType == typeOSAGenericStorage) {
		
		err = OSAAddStorageType ((AEDataStorage) hpacked, typeLAND);

		if (err != noErr) {
			disposehandle (hpacked);
			return (err);
			}
		
		descType = typeOSAGenericStorage;
		}
	else {
		descType = typeLAND;
		}

  fl = newdescwithhandle (&desc, descType, hpacked);
	
	*resultingScriptData = desc;
	
	return (noErr);
	} /*osaStore*/


pascal OSAError osaDispose (
			hdlcomponentglobals	hglobals,
			OSAID				scriptID) {
	
	boolean fl;
	
	fl = deletestorageval (hglobals, scriptID);
	
	if (!fl)
		return (osageterror ());
	
	return (noErr);
	} /*osaDispose*/


static pascal OSAError osaSetScriptInfo (
			hdlcomponentglobals	hglobals,
			OSAID				scriptID,
			OSType				selector,
			long				value) {
	
	tyvaluerecord vscript;
	hdlhashtable hcontext;
	hdlhashnode hnode;
	
	if (scriptID == kOSANullScript)
		return (errOSAInvalidID);
	
	if (!getstorageval (hglobals, scriptID, &vscript, &hnode))
		return (errOSAInvalidID);
	
	switch (selector) {
		
		case kOSAScriptIsModified:
			if (!langexternalvaltotable (vscript, &hcontext, hnode))
				return (errOSABadSelector);
			
			(**hcontext).fldirty = value != 0;
			
			break;
		
		default:
			return (errOSABadSelector);
		}
	
	return (noErr);
	} /*osaSetScriptInfo*/


static pascal OSAError osaGetScriptInfo (
			hdlcomponentglobals	hglobals,
			OSAID				scriptID,
			OSType				selector,
			long*				result) {
	
	/*
	2.1b4 dmb: added code for kOSACanGetSource and kASHasOpenHandler selectors
	*/
	
	register hdlcomponentglobals hcg = hglobals;
	tyvaluerecord vscript;
	tyvaluerecord vcode;
	hdlhashtable hcontext;
	hdlhashnode hnode;
	
	*result = 0;
	
	if (scriptID == kOSANullScript)
		return (errOSAInvalidID);
	
	if (!getstorageval (hcg, scriptID, &vscript, &hnode))
		return (errOSAInvalidID);
	
	if (!langexternalvaltotable (vscript, &hcontext, hnode))
		hcontext = nil;
	
	switch (selector) {
		
		case kOSAScriptIsModified:
			if (hcontext == nil)
				return (errOSABadSelector);
			
			*result = (**hcontext).fldirty;
			
			break;
		
		case kOSAScriptIsBeingEdited:
			break;
		
		case kOSAScriptIsBeingRecorded:
			*result = (**hcg).recordingstate.flrecording;
			
			break;
		
		case kOSAScriptIsTypeCompiledScript:
			break;
		
		case kOSAScriptIsTypeScriptValue:
			*result = hcontext == nil;
			
			break;
		
		case kOSAScriptIsTypeScriptContext:
			*result = hcontext != nil;
			
			break;
		
		case kOSAScriptBestType:
			*result = typeChar;
			
			break;
		
		case kOSACanGetSource:
			if (hcontext == nil)
				return (errOSABadSelector);
			
			*result = hashtablesymbolexists (hcontext, bssource);
			
			break;
		
		case kASHasOpenHandler:
			if (hcontext == nil)
				return (errOSABadSelector);
			
			if (!hashtablelookup (hcontext, bscode, &vcode, &hnode))
				break;
			
			if (!osacreatemodulecontext (vcode.data.codevalue, &hcontext))
				return (false);
			
			*result = hashtablesymbolexists (hcontext, "\podoc") || hashtablesymbolexists (hcontext, "\popen");
			
			disposehashtable (hcontext, true);
			
			break;
		
		default:
			return (errOSABadSelector);
		}
	
	return (noErr);
	} /*osaGetScriptInfo*/


static pascal OSAError osaSetResumeDispatchProc (
				hdlcomponentglobals	hglobals,
				AEEventHandlerUPP	resumeDispatchProc,
				long				refCon) {
	
	/*
	2.1b4 dmb: we're not currently using the dispatch proc. UserTalk doesn't 
	have an equivalent to AS's "continue" statement, but a script can do 
	a scripterror (-1708) to get exectueevent to return that error, in which 
	case the client should invoke its own dispatch proc
	
	in any case, we're keeping track of what the caller sets up, just for 
	the hell of it.
	*/
	
	register hdlcomponentglobals hcg = hglobals;
	
	(**hcg).resumedispatchproc = resumeDispatchProc;
	
	(**hcg).resumedispatchprocrefcon = refCon;
	
	return (noErr);
	} /*osaSetResumeDispatchProc*/


static pascal OSAError
osaGetResumeDispatchProc (
		hdlcomponentglobals	 hglobals,
		AEEventHandlerUPP	*resumeDispatchProc,
		long				*refCon)
{
#pragma unused (hglobals)
	/*
	we're never invoking the resumedispatch proc, so be honest and 
	return these constants
	*/
	
	*resumeDispatchProc = (AEEventHandlerUPP) kOSANoDispatch;
	
	*refCon = 0;
	
	return (noErr);
	} /*osaGetResumeDispatchProc*/


pascal OSAError osaSetActiveProc (
			hdlcomponentglobals	hglobals,
			OSAActiveUPP		activeProc,
			long				refCon) {
	
	register hdlcomponentglobals hcg = hglobals;
	
	(**hcg).activeproc = activeProc;
	
	(**hcg).activeprocrefcon = refCon;
	
	return (noErr);
	} /*osaSetActiveProc*/


pascal OSAError osaSetSendProc (
			hdlcomponentglobals	hglobals,
			OSASendUPP			sendProc,
			long				refCon) {
	
	register hdlcomponentglobals hcg = hglobals;
	
	if (sendProc == nil)
		sendProc = osadefaultsendUPP;
	
	(**hcg).sendproc = sendProc;
	
	(**hcg).sendprocrefcon = refCon;
	
	return (noErr);
	} /*osaSetSendProc*/


static pascal OSAError osaGetSendProc (
			hdlcomponentglobals	hglobals,
			OSASendUPP*			sendProc,
			long*				refCon) {
	
	register hdlcomponentglobals hcg = hglobals;
	
	*sendProc = (**hcg).sendproc;
	
	*refCon = (**hcg).sendprocrefcon;
	
	return (noErr);
	} /*osaGetSendProc*/


static pascal OSAError osaSetCreateProc (
			hdlcomponentglobals			hglobals,
			OSACreateAppleEventUPP		createProc,
			long						refCon) {
	
	register hdlcomponentglobals hcg = hglobals;
	
	if (createProc == nil)
		createProc = osadefaultcreateUPP;
	
	(**hcg).createproc = createProc;
	
	(**hcg).createprocrefcon = refCon;
	
	return (noErr);
	} /*osaSetCreateProc*/


static pascal OSAError osaGetCreateProc (
			hdlcomponentglobals			hglobals,
			OSACreateAppleEventUPP*		createProc,
			long*						refCon) {
	
	register hdlcomponentglobals hcg = hglobals;
	
	*createProc = (**hcg).createproc;
	
	*refCon = (**hcg).createprocrefcon;
	
	return (noErr);
	} /*osaGetCreateProc*/


#if 0

	static pascal OSAError osaSetDefaultTarget (
				hdlcomponentglobals	hglobals,
				const AEAddressDesc*	target) {
		
		return (noErr);
		} /*osaSetDefaultTarget*/

#endif


static pascal OSAError osacompiledesc (
			const AEDesc*		sourceData,
			tyvaluerecord*		vcode) {
	
	Handle htext;
	hdltreenode hcode;
	
	if ((*sourceData).descriptorType != typeChar)
		return (errAECoercionFail);
	
	/*PBS 03/14/02: AE OS X fix.*/
	
  if (!copydatahandle ((AEDesc*) sourceData, &htext)) /*don't let langbuildtree consume caller's text*/
    return (memFullErr);
	
	if (!langbuildtree (htext, false, &hcode)) /*syntax error*/
		return (osageterror ());
	
	initvalue (vcode, codevaluetype);
	
	(*vcode).data.codevalue = hcode;
	
	return (noErr);
	} /*osacompiledesc*/


static pascal OSAError osaCompile (
			hdlcomponentglobals	hglobals,
			const AEDesc*		sourceData,
			long				modeFlags,
			OSAID*				scriptID) {
	
	/*
	first cut: handle contexts, but not augmenting contexts.
	*/
	
	OSAError err = noErr;
	hdlhashtable hcontext;
	tyvaluerecord vcode, vcontext, vsource;
	OSAID id = *scriptID;
	Handle htext;
	boolean flkeepsource = false;
	hdlhashnode hnode;
	
	err = osacompiledesc (sourceData, &vcode);
	
	if (err != noErr)
		return (err);
	
	if ((modeFlags & kOSAModePreventGetSource) == 0) { /*bit not set*/
		
		flkeepsource = true;
		
		/*PBS 03/14/02: AE OS X fix.*/
    
    if (!copydatahandle ((AEDesc*) sourceData, &htext)) /*make copy for eventual getsource*/
      return (memFullErr);
		
		if (!setheapvalue (htext, stringvaluetype, &vsource))
			return (osageterror ());
		}
	
	if (id == kOSANullScript) {
		
		if (!osanewvalue (idtableprocessor, nil, &vcontext))
			return (memFullErr);
		
		langexternalvaltotable (vcontext, &hcontext, HNoNode);
		}
	else {
		
		if (!getstorageval (hglobals, id, &vcontext, &hnode))
			return (osageterror ());
		
		if (!langexternalvaltotable (vcontext, &hcontext, hnode))
			return (errOSAInvalidID);
		}
	
	pushhashtable (hcontext);
	
	hashassign (bscode, vcode);
	
	if (flkeepsource)
		flkeepsource = hashassign (bssource, vsource);
	
	pophashtable ();
	
	if (flkeepsource)
		exemptfromtmpstack (&vsource);
	
	if (id == kOSANullScript) {
		
		if (!addstorageval (hglobals, &vcontext, scriptID))
			return (osageterror ());
		}
	
	return (noErr);
	} /*osaCompile*/


static pascal OSAError osaGetSource (
			hdlcomponentglobals	hglobals,
			OSAID				scriptID,
			DescType			desiredType,
			AEDesc*				resultingSourceData) {
	
	tyvaluerecord val;
	hdlhashtable hcontext;
	hdlhashnode hnode;
	
	if (scriptID == kOSANullScript)
		return (errOSAInvalidID);
	
	if (!osagetcontext (hglobals, scriptID, &hcontext))
		goto error;
	
	if (!hashtablelookup (hcontext, bssource, &val, &hnode))
		return (errOSASourceNotAvailable);
	
	if (val.valuetype == novaluetype)
		return (errOSASourceNotAvailable);
	
	if (!storagevaltodesc (&val, desiredType, resultingSourceData))
		goto error;
	
	return (noErr);
	
	error:
		return (osageterror ());
	
	} /*osaGetSource*/


static pascal OSAError
osaCoerceFromDesc (
		hdlcomponentglobals	 hglobals,
		const AEDesc		*scriptData,
		long				 modeFlags,
		OSAID				*resultingScriptID)
{
#pragma unused (modeFlags)
	
	/*
	3.0a dmb: fixed leak when val is an externalvaluetype.
	*/
	
	AEDesc desc;
	tyvaluerecord val;
	register boolean fl;
	
	if (oserror (AEDuplicateDesc (scriptData, &desc)))
		goto error;
	
	if (langgetvaluetype (desc.descriptorType) >= outlinevaluetype) {
		
		val.valuetype = externalvaluetype;
		
		/*PBS 03/14/02: AE OS X fix.*/
    {
    Handle h;
    
    copydatahandle (&desc, &h);
    
    fl = langexternalmemoryunpack (h, (hdlexternalhandle *) &val.data.externalvalue);
    
    disposehandle (h);
    }
		
		AEDisposeDesc (&desc);
		
		if (!fl)
			goto error;
		}
	else {
		
		if (!setdescriptorvalue (desc, &val))
			goto error;
		}
	
	if (!addstorageval (hglobals, &val, resultingScriptID))
		goto error;
	
	return (noErr);
	
	error:
		return (osageterror ());
	} /*osaCoerceFromDesc*/


static pascal OSAError
osaCoerceToDesc (
		hdlcomponentglobals	 hglobals,
		OSAID				 scriptID,
		DescType			 desiredType,
		long				 modeFlags,
		AEDesc				*result)
{
#pragma unused (modeFlags)
	
	tyvaluerecord val;
	hdlhashnode hnode;
	
	if (scriptID == kOSANullScript)
		return (errOSAInvalidID);
	
	if (!getstorageval (hglobals, scriptID, &val, &hnode))
		goto error;
	
	if (!storagevaltodesc (&val, desiredType, result))
		goto error;
	
	return (noErr);
	
	error:
		return (osageterror ());
	
	} /*osaCoerceToDesc*/


static Handle getcomponentstringhandle (short id) {
	
	/*
	2.1b11 dmb: return a handle to the specified string in the component 
	string list
	*/
	
	bigstring bs;
	
	if (!getstringlist (componentlistnumber, id, bs))
		return (nil);
	
	return ((Handle) NewString (bs));
	} /*getcomponentstringhandle*/


static boolean getrecordingstring (short id, bigstring bs) {
	
	return (getstringlist (recordinglistnumber, id, bs));
	} /*getrecordingstring*/


static pascal OSErr
coerceInsltoTEXT (
		const AEDesc	*desc,
		DescType		 totype,
		long			 refcon,
		AEDesc			*result)
{
#pragma unused(totype, refcon)

	/*
	2.1b2 dmb: this is installed as a typeInsertionLoc -> typeObjectSpecifier 
	coercer, but it's actually generating source text for the recorder.  it's 
	not installed as a -> typeChar coercer to prevent getobjectmodeldisplaystring 
	from putting quotes around the result
	
	2.1b3 dmb: if position isn't one of the standard 5, call insertionLoc
	*/
	
	AEDesc rec;
	AEDesc obj;
	byte bsverb [16];
	byte bspos [64];
	bigstring bsobj;
	bigstring bs;
	tyvaluerecord val;
	short ix;
	OSErr err;
	DescType type;
	long size;
	OSType pos;
		
	#ifdef fldebug // 2006-04-04 - kw --- this was fldegug
	
	if ((*desc).descriptorType != typeInsertionLoc)
		DebugStr ("\punexpected coercion");
	
	#endif
	
	err = AECoerceDesc (desc, typeAERecord, &rec);
	
	if (err != noErr)
		goto exit;
	
	err = AEGetKeyPtr (&rec, keyAEPosition, typeEnumeration, &type, &pos, sizeof (pos), &size);
	
	if (err == noErr)
		err = AEGetKeyDesc (&rec, keyAEObject, typeWildCard, &obj);
	
	AEDisposeDesc (&rec);
	
	if (err != noErr)
		goto exit;
	
	setemptystring (bspos);
	
	switch (pos) {
		
		case kAEBefore:
			ix = beforestring; break;
		
		case kAEBeginning:
			ix = beginningofstring; break;
		
		case kAEAfter:
			ix = afterstring; break;
		
		case kAEEnd:
			ix = endofstring; break;
		
		case kAEReplace:
			ix = replacingstring; break;
		
		default:
			ix = insertionlocstring;
			
			setostypevalue (pos, &val);
			
			getobjectmodeldisplaystring (&val, bspos);
			
			pushstring ("\p, ", bspos);
			
			break;
		}
	
	getrecordingstring (ix, bsverb);
	
	setdescriptorvalue (obj, &val);
	
	getobjectmodeldisplaystring (&val, bsobj);
	
	disposevaluerecord (val, true);
	
	if (isemptystring (bsobj))
		copystring ("\p\"\"", bsobj);
	
	parsedialogstring ("\p^0 (^1^2)", bsverb, bspos, bsobj, nil, bs);
	
	err = AECreateDesc (typeChar, bs + 1, stringlength (bs), result);
	
exit:
	
	return (err);
	} /*coerceInsltoTEXT*/


//Code change by Timothy Paustian Friday, June 16, 2000 1:38:13 PM
//Changed to Opaque call for Carbon
AECoercionHandlerUPP	coerceInsltoTEXTDesc;

#define coerceInsltoTEXTUPP (coerceInsltoTEXTDesc);

static pascal OSErr
sendrecordingevent (
		hdlcomponentglobals		 hglobals,
		AEEventID				 id)
{
#pragma unused (hglobals)
	
	/*
	2.1b5 dmb: don't use sendproc for these events; they're not part of 
	script execution
	*/
	
	AEDesc desc;
	OSErr err;
	ProcessSerialNumber psn;
	AppleEvent event, reply;
	
	GetCurrentProcess (&psn);
	
	err = AECreateDesc (typeProcessSerialNumber, (Ptr) &psn, sizeof (psn), &desc);
	
	if (err == noErr) {
		
		err = AECreateAppleEvent (kCoreEventClass, id, &desc, kAutoGenerateReturnID, kAnyTransactionID, &event);
		
		AEDisposeDesc (&desc);
		
		if (err == noErr) {
			
			err = AESend (&event, &reply, 
				
				(AESendMode) kAEDontRecord + kAECanSwitchLayer + kAECanInteract + kAENoReply, 
				
				(AESendPriority) kAENormalPriority, (long) kAEDefaultTimeout, nil, nil);
			
			AEDisposeDesc (&event);
			
			AEDisposeDesc (&reply);
			}
		}
	
	return (err);
	} /*sendrecordingevent*/


static pascal OSErr sendrecordedtextevent (hdlcomponentglobals hcg, bigstring bs) {
	
	/* 
	2.1b5 dmb: maintain our own version of the recorded text
	
	2.1b8 dmb: set up currentA5 and client context for AESend call.
	it will be short-circuited to client app, so it's like a callback.
	*/
	
	AppleEvent event, reply;
	AEDesc desc;
	ProcessSerialNumber psn;
	OSErr err;
	Handle htext;
	bigstring bssend;
	
	copystring (bs, bssend);
	
	pushchar (chreturn, bssend);
	
	htext = (**hcg).recordingstate.hrecordedtext;
	
	if (htext != nil)
		pushtexthandle (bssend, htext); /*if this fails, so will subsequent calls*/
	
	psn.highLongOfPSN = 0;
	
	psn.lowLongOfPSN = kCurrentProcess;
	
	err = AECreateDesc (typeProcessSerialNumber, (Ptr) &psn, sizeof (psn), &desc);
	
	if (err == noErr) {
		
		err = AECreateAppleEvent (kOSASuite, kOSARecordedText, &desc, kAutoGenerateReturnID, kAnyTransactionID, &event);
		
		AEDisposeDesc (&desc);
		
		if (err == noErr) {
			
			err = AEPutParamPtr (&event, keyDirectObject, typeChar, (Ptr) bssend + 1, stringlength (bssend));
			
			osapreclientcallback (hcg);
			
			err = AESend (&event, &reply, 
				
				(AESendMode) kAENoReply + kAEDontRecord, 
				
				(AESendPriority) kAENormalPriority, (long) kNoTimeOut, nil, nil);
					
			osapostclientcallback (hcg);
			
			AEDisposeDesc (&event);
			
			AEDisposeDesc (&reply);
			}
		}
	
	return (err);
	} /*sendrecordedtextevent*/


static pascal OSErr pusheventparameter (const AppleEvent *event, AEKeyword key, boolean flpushkey, bigstring bsparam, bigstring bsevent) {
	
	AEDesc desc;
	tyvaluerecord val;
	bigstring bsval;
	byte bskey [6];
	byte bscoerce [16];
	OSErr err;
	
	err = AEGetParamDesc (event, key, typeWildCard, &desc);
	
	if (err != noErr)
		return (err);
	
	if (!setdescriptorvalue (desc, &val))
		return (getoserror ());
	
	getobjectmodeldisplaystring (&val, bsval);
	
	switch (val.valuetype) { /*see if coercion is needed*/
		
		case filespecvaluetype:
		case aliasvaluetype:
			langgettypestring (val.valuetype, bscoerce);
			
			pushstring ("\p (", bscoerce);
			
			insertstring (bscoerce, bsval);
			
			pushchar (')', bsval);
			
			break;
		
		default:
			break;
		}
	
	disposevaluerecord (val, false);
	
	if (flpushkey) {
		
		pushstring ("\p, '", bsevent);
		
		ostypetostring (key, bskey);
		
		pushstring (bskey, bsevent);
		
		pushchar ('\'', bsevent);
		}
	
	if (bsevent [stringlength (bsevent)] != '(') /*not 1st item in param list*/
		pushstring ("\p, ", bsevent);
	
	if (!isemptystring (bsparam)) {
		
		pushstring (bsparam, bsevent);
		
		pushstring ("\p: ", bsevent);
		}
	
	pushstring (bsval, bsevent);
	
	return (noErr);
} /*pusheventparameter*/


static pascal OSErr
handlerecordableevent (
		const AppleEvent	*event,
		AppleEvent			*reply,
		SInt32				 refcon)
{
#pragma unused (reply)

	/*
	map the event to a line of source code, and send the text in a Recorded Text event
	
	2.1b2: added bringToFront calls
	
	2.1b3: include braces & semicolons; script.c will strip them out
	
	2.1b4: added ugly special case for comment events. if there's no glue for an 
	event, use appleevent verb. allocate 8 bytes for ostype strings to leave room 
	for single quotes and length.
	
	2.1b5: do everything in our heap. finding the app table is especially important; 
	we don't want our tables loaded in another heap. the same probably applies to any 
	resource strings that might be used, though we can probably preload what we need 
	if we want to keep the clients heap set for same reason.
	
	2.1b7: ignore errors from sendrecordedtextevent. we want to continue 
	accumulating our version of the text no matter what.
	
	3.0b15 dmb: call SetResLoad (false) before opening recorded app's resource 
	fork, or all of its preload resources will be loaded into our heap
	
	3.0a dmb: if no app table is found, add comment to that effect to "with" 
	statement.
	*/
	
	register hdlcomponentglobals hcg = (hdlcomponentglobals) refcon;
	AEDesc desc;
	OSErr err;
	AEEventClass class;
	AEEventID id;
	DescType type;
	long size;
	boolean flgotname;
	long paramoffset;
	bigstring bs;
	bigstring bsname;
	bigstring bsparam;
	byte bssignature [8];
	byte bsclass [8];
	byte bsid [8];
	hdlhashtable happtable = nil;
	FSSpec fs;
	OSType signature;
	ProcessSerialNumber psn;
	boolean flisfront;
	boolean flpushkeys;
	boolean flignore;
	short rnum = 0;
	Handle haete = nil;
	
	
	err = landsystem7getsenderinfo (event, &psn, &fs, &signature);
	
	if (err != noErr) {
		
		return (err);
		}
	
	osapushfastcontext (hcg);

  coerceInsltoTEXTDesc = NewAECoerceDescUPP(coerceInsltoTEXT);
  AEInstallCoercionHandler (typeInsertionLoc, typeObjectSpecifier, coerceInsltoTEXTDesc, 0, true, false);
	
	if (signature == (**hcg).recordingstate.lastappid)
		happtable = (**hcg).recordingstate.lastapptable;
	
	else {
		
		if ((**hcg).recordingstate.lastappid != 0) {
			
			sendrecordedtextevent (hcg, "\p\t};");
			
			/*
			if (err != noErr)
				goto exit;
			*/
			}
		
		flisfront = isfrontapplication (psn);
		
		getrecordingstring (withobjectmodelstring, bs);
		
		if (langipcfindapptable (signature, false, &happtable, bsname)) {
			
			pushstring ("\p, ", bs);
			
			pushstring (bsname, bs);
			
			pushstring ("\p {", bs);
			
			sendrecordedtextevent (hcg, bs);
			
			/*
			if (err != noErr)
				goto exit;
			*/
			
			if (flisfront) {
				
				getrecordingstring (bringtofrontstring, bs);
				
				sendrecordedtextevent (hcg, bs);
				
				/*
				if (err != noErr)
					goto exit;
				*/
				}
			}
		else {
			
			pushstring ("\p { ", bs);
			
			getrecordingstring (noverbtablestring, bsparam); /*3.0a*/
			
			getprocessname (psn, bsname, &flignore);
			
			parsedialogstring (bsparam, bsname, nil, nil, nil, bsparam);
			
			pushstring (bsparam, bs);
			
			sendrecordedtextevent (hcg, bs);
			
			/*
			if (err != noErr)
				goto exit;
			*/
			
			if (flisfront) {
				
				ostypetostring (signature, bssignature);
				
				getrecordingstring (sysbringapptofrontstring, bs);
				
				parsedialogstring (bs, bssignature, nil, nil, nil, bs);
				
				sendrecordedtextevent (hcg, bs);
				}
			}
		
		(**hcg).recordingstate.lastappid = signature;
		
		(**hcg).recordingstate.lastapptable = happtable;
		}
	
	err = AEGetAttributePtr (event, keyEventClassAttr, typeType, &type, &class, sizeof (class), &size);
	
	if (err == noErr)
		err = AEGetAttributePtr (event, keyEventIDAttr, typeType, &type, &id, sizeof (id), &size);
	
	if (err != noErr)
		goto exit;
	
	setemptystring (bsparam);
	
	flgotname = false;
	
	if ((class == kASAppleScriptSuite) && (id == kASCommentEvent)) { /*special case*/
		
		setemptystring (bs);
		
		err = AEGetParamDesc (event, keyDirectObject, typeChar, &desc);
		
		if (err == noErr) {
			
			/*PBS 03/14/02: AE OS X fix.*/
      datahandletostring (&desc, bs);
			
			insertstring ("\p\t«", bs);
			
			AEDisposeDesc (&desc);
			}
		
		goto exit;
		}
	
	if (happtable != nil) {
		
		pushhashtable (happtable);
		
		SetResLoad (false);
		
		rnum = FSpOpenResFile (&fs, fsRdPerm);
		
		SetResLoad (true);
		
		if (rnum != -1) {
			
			haete = Get1IndResource ('aete', 1);
			
			flgotname = osaparseaete (haete, class, id, bsname, &paramoffset);
			}
		
		if (!flgotname) {
			
			haete = GetIndResource ('aeut', 1);
			
			flgotname = osaparseaete (haete, class, id, bsname, &paramoffset);
			}
		
		if (flgotname && !hashsymbolexists (bsname)) /*event doesn't have verb glue*/
			flgotname = false;
		}
	
	if (flgotname) {
		
		parsedialogstring ("\p\t^0 (", bsname, nil, nil, nil, bs);
		
		flpushkeys = false;
		}
	else {
		
		if (happtable == nil) {
			
			ostypetostring (signature, bssignature);
			
			insertchar ('\'', bssignature);
			
			pushchar ('\'', bssignature);
			}
		else
			getrecordingstring (idstring, bssignature);
		
		ostypetostring (class, bsclass);
		
		ostypetostring (id, bsid);
		
		getrecordingstring (appleeventstring, bs);
		
		parsedialogstring (bs, bssignature, bsclass, bsid, nil, bs);
		
		flpushkeys = true;
		}
	
	pusheventparameter (event, keyDirectObject, flpushkeys, bsparam, bs);
	
	while (AEGetAttributePtr (event, keyMissedKeywordAttr, typeKeyword, &type, &id, sizeof (id), &size) == noErr) {
		
		if (flgotname) {
			
			if (osaparseaeteparamlist (haete, paramoffset, id, bsparam)) {
				
				if (hashtablesymbolexists (hkeywordtable, bsparam)) { /*it's a keyword*/
					
					bsparam [1] = toupper (bsparam [1]);
					
					insertstring (bsname, bsparam); /*prepend verb name using innerCase*/
					}
				}
			}
		
		err = pusheventparameter (event, id, flpushkeys, bsparam, bs);
		
		if (err != noErr)
			break;
		}
	
	/*
	while (AEGetAttributePtr (event, keyOptionalKeywordAttr, typeAEList, ...) == noErr)
		pusheventparameter (event, id, flpushkeys, bs);
	*/
	
	if (happtable != nil) {
		
		if (rnum != -1)
			CloseResFile (rnum);
		
		pophashtable ();
		}
	
	pushstring ("\p);", bs);
	
exit:
	
	if (err == noErr)
		err = sendrecordedtextevent (hcg, bs);
	
  //Code change by Timothy Paustian Friday, July 21, 2000 11:03:49 PM
	//Get rid of the UPP wer are done.
  AERemoveCoercionHandler (typeInsertionLoc, typeChar, coerceInsltoTEXTDesc, false);
  DisposeAECoerceDescUPP(coerceInsltoTEXTDesc);
	
	osapopfastcontext (hcg);
	
	return (err);
	} /*handlerecordableevent*/


AEEventHandlerUPP handlerecordableeventDesc = nil;

#define handlerecordableeventUPP (handlerecordableeventDesc)

static pascal OSAError osaStartRecording (
			hdlcomponentglobals	hglobals,
			OSAID				*compiledScriptToModifyID) {
	
	/*
	2.1b2 dmb: check gestaltAppleEventsAttr before attempting recording
	*/
	
	register hdlcomponentglobals hcg = hglobals;
	tyvaluerecord vcontext;
	hdlhashtable hcontext;
	long result;
	Handle htext;
	OSAError err;
	
	if (!gestalt (gestaltAppleEventsAttr, &result) || (result <= 1))
		return (errAENewerVersion);
	
	if ((**hcg).recordingstate.flrecording)
		return (errOSARecordingIsAlreadyOn);
	
	if (*compiledScriptToModifyID == kOSANullScript) {
		
		if (!osanewvalue (idtableprocessor, nil, &vcontext))
			return (memFullErr);
		
		langexternalvaltotable (vcontext, &hcontext, HNoNode);
		
		if (!addstorageval (hglobals, &vcontext, compiledScriptToModifyID))
			return (osageterror ());
		
		newemptyhandle (&htext);
		}
	else
		htext = nil;
	
	(**hcg).recordingstate.hrecordedtext = htext;
	
	err = sendrecordingevent (hglobals, kAEStartRecording);
	
	if (err == noErr) {
		
		(**hcg).recordingstate.flrecording = true;
		
		(**hcg).recordingstate.lastappid = 0;

		//Code change by Timothy Paustian Friday, July 28, 2000 2:40:45 PM
		//we need to create the UPP now that we want to use it.
		if (handlerecordableeventDesc == nil)
			handlerecordableeventDesc = NewAEEventHandlerUPP(handlerecordableevent);
		
		err = AEInstallEventHandler (kCoreEventClass, kAENotifyRecording, handlerecordableeventUPP, (long) hglobals, false);
		}
	
	return (err);
	} /*osaStartRecording*/


static pascal OSAError osaStopRecording (
			hdlcomponentglobals	hglobals,
			OSAID				compiledScriptID) {
	
	/*
	2.1b5 dmb: compile the recorded text if necessary
	*/
	
	register hdlcomponentglobals hcg = hglobals;
	OSAError err;
	AEDesc sourcedesc;
	
	if (!(**hcg).recordingstate.flrecording)
		return (noErr);
	
	if ((**hcg).recordingstate.lastappid != 0) /*need to terminate "with" statement*/
		err = sendrecordedtextevent (hcg, "\p\t}");
	
	err = sendrecordingevent (hglobals, kAEStopRecording);
	
	if ((err == noErr) && (compiledScriptID != kOSANullScript)) {
	
		/*PBS 03/14/02: AE OS X fix.*/
    newdescwithhandle (&sourcedesc, typeChar, (**hcg).recordingstate.hrecordedtext);

		if ((**hcg).recordingstate.hrecordedtext != nil) {	/*script was created by recording process*/

			sourcedesc.descriptorType = typeChar;
			
			err = osaCompile (hcg, &sourcedesc, kOSAModeNull, &compiledScriptID);
			}
		}
	
	(**hcg).recordingstate.flrecording = false;
	
	disposehandle ((**hcg).recordingstate.hrecordedtext);

	//Code change by Timothy Paustian Friday, July 28, 2000 2:43:14 PM
	//I am assuming the osaStopRecording is called every time osaStartRecording is called.

  DisposeAEEventHandlerUPP(handlerecordableeventDesc);
  handlerecordableeventDesc = nil;

	return (err);
	} /*osaStopRecording*/


static pascal OSAError
osaScriptingComponentName (
		hdlcomponentglobals	 hglobals,
		AEDesc				*resultingScriptingComponentName)
{
#pragma unused (hglobals)

	bigstring bs;
	
	getstringlist (componentlistnumber, usertalkstring, bs);
	
	return (AECreateDesc (typeChar, (Ptr) bs + 1, stringlength (bs), resultingScriptingComponentName));
	} /*osaScriptingComponentName*/


static pascal OSAError osaExecute (
			hdlcomponentglobals	hglobals,
			OSAID				compiledScriptID,
			OSAID				contextID,
			long				modeFlags,
			OSAID*				resultingScriptValueID) {
	
	tyvaluerecord vcode;
	tyvaluerecord vresult;
	hdlhashtable hscript;
	hdlhashtable hcontext = nil;
	hdlhashnode hnode;
	
	if (compiledScriptID == kOSANullScript)
		return (errOSAInvalidID);
	
	if (!osagetcontext (hglobals, compiledScriptID, &hscript))
		goto error;
	
	if (!hashtablelookup (hscript, bscode, &vcode, &hnode))
		return (errOSACorruptData);
	
	if (!osagetcontext (hglobals, contextID, &hcontext))
		return (errOSAInvalidID);
	
	if (!osahandlerunscript (hglobals, vcode.data.codevalue, hcontext, modeFlags, &vresult))
		goto error;
	
	if (!addstorageval (hglobals, &vresult, resultingScriptValueID))
		goto error;
	
	return (noErr);
	
	error:
		return (osageterror ());
	
	} /*osaExecute*/


static pascal OSAError
osaDisplay (
		hdlcomponentglobals	 hglobals,
		OSAID				 scriptValueID,
		DescType			 desiredType,
		long				 modeFlags,
		AEDesc				*resultingText)
{
#pragma unused (desiredType, modeFlags)
	
	/*
	****should handle desiredType other than typechar
	*/
	
	tyvaluerecord val;
	bigstring bs;
	OSAError err;
	hdlhashnode hnode;
	
	if (scriptValueID == kOSANullScript)
		return (errOSAInvalidID);
	
	if (!getstorageval (hglobals, scriptValueID, &val, &hnode))
		goto error;
	
	if (!hashgetvaluestring (val, bs))
		goto error;
	
	err = AECreateDesc (typeChar, (Ptr) bs + 1, stringlength (bs), resultingText);
	
	/*
	if ((err == noErr) && (desiredType != typeChar) && (desiredType != typeWildCard)) {
		
		AEDesc tempdesc = *resultingText;
		
		err = AECoerceDesc (&tempdesc, desiredType, resultingText);
		
		AEDisposeDesc (&tempdesc);
		}
	*/
	
	return (err);
	
	error:
		return (osageterror ());
	
	} /*osaDisplay*/


pascal OSAError osaScriptError (
			hdlcomponentglobals	hglobals,
			OSType				selector,
			DescType			desiredType,
			AEDesc*				resultingErrorDescription) {
	
	AEDesc errordesc;
	
	errordesc = (**hglobals).errordesc;
	
	return (AEGetKeyDesc (&errordesc, selector, desiredType, resultingErrorDescription));
	} /*osaScriptError*/


static pascal OSAError osaLoadExecute (
			hdlcomponentglobals	hglobals,
			const AEDesc*		scriptData,
			OSAID				contextID,
			long				modeFlags,
			OSAID*				resultingScriptValueID) {
	
	OSAID scriptid;
	OSAError err;
	
	err = osaLoad (hglobals, scriptData, modeFlags, &scriptid);
	
	if (err == noErr) {
		
		err = osaExecute (hglobals, scriptid, contextID, modeFlags, resultingScriptValueID);
		
		osaDispose (hglobals, scriptid);
		}
	
	return (err);
	} /*osaLoadExecute*/


pascal OSAError osaCompileExecute (
			hdlcomponentglobals	hglobals,
			const AEDesc*		sourceData,
			OSAID				contextID,
			long				modeFlags,
			OSAID*				resultingScriptValueID) {
	
	OSAID scriptid;
	OSAError err;
	
	scriptid = kOSANullScript;
	
	err = osaCompile (hglobals, sourceData, modeFlags | kOSAModePreventGetSource, &scriptid);
	
	if (err == noErr) {
		
		err = osaExecute (hglobals, scriptid, contextID, modeFlags, resultingScriptValueID);
		
		osaDispose (hglobals, scriptid);
		}
	
	return (err);
	} /*osaCompileExecute*/


pascal OSAError osaDoScript (
			hdlcomponentglobals	hglobals,
			const AEDesc*		sourceData,
			OSAID				contextID,
			DescType			desiredType,
			long				modeFlags,
			AEDesc*				resultingText) {
	
	/*
	3/29/93 dmb: set menusharingglobals.idscript so that script can be cancelled
	*/
	
	register hdlcomponentglobals hcg = hglobals;
	tyvaluerecord vcode;
	tyvaluerecord vresult;
	hdlhashtable hcontext;
	boolean fl;
	OSAError err;
	
	if (!osagetcontext (hglobals, contextID, &hcontext))
		return (errOSAInvalidID);
	
	err = osacompiledesc (sourceData, &vcode);
	
	if (err != noErr)
		return (err);
	
	(**hcg).menusharingglobals.idscript = (long) vcode.data.codevalue;
	
	fl = osahandlerunscript (hcg, vcode.data.codevalue, hcontext, modeFlags, &vresult);
	
	langdisposetree (vcode.data.codevalue);
	
	if (!fl) {
		
		osaScriptError (hcg, kOSAErrorMessage, desiredType, resultingText);
		
		if (osageterror () == userCanceledErr)
			return (userCanceledErr);
		else
			return (errOSAScriptError);
		}
	
	fl = storagevaltodesc (&vresult, desiredType, resultingText);
	
	disposetmpvalue (&vresult);
	
	if (!fl)
		return (osageterror ());
	
	return (noErr);
	} /*osaDoScript*/


static pascal OSAError
osaMakeContext (
		hdlcomponentglobals	 hglobals,
		const AEDesc		*contextName,
		OSAID				 parentContext,
		OSAID				*resultingContextID)
{
#pragma unused (contextName, parentContext)
	
	tyvaluerecord vcontext;
	
	*resultingContextID = kOSANullScript;
	
	if (!osanewvalue (idtableprocessor, nil, &vcontext))
		return (memFullErr);
	
	addstorageval (hglobals, &vcontext, resultingContextID);
	
	return (noErr);
	} /*osaMakeContext*/


static boolean getverbresult (hdlverbrecord hv, tyvaluerecord *vresult) {

	typaramrecord result;
	AppleEvent event, reply;
	
	landsystem7geteventrecords (hv, &event, &reply);
	
	if (landsystem7getparam (&reply, returnedvaluekey, notype, &result))
		setdescriptorvalue (result.desc, vresult);
	else
		initvalue (vresult, novaluetype);
	
	return (true);
	} /*getverbresult*/


static pascal Boolean osahandleevent (
				hdlcomponentglobals	hglobals,
				hdlverbrecord		hverb,
				OSAID				contextID,
				long				modeFlags,
				tyvaluerecord*		vresult) {
	
	/*
	2/27/97 dmb: major improvement. first, fix bug where null context id
	would lead to crash in hashtablelookup with a nil table. next, if the 
	verb isn't handled by the given context, pass if off to the global 
	handleverbroutine for default processing by Frontier
	*/
	
	tyvaluerecord vcode;
	hdlhashtable hcontext;
	hdlhashnode hnode;
	
	if (contextID == kOSANullScript) {
	
		oserror (errAEEventNotHandled);
		
		goto error;
		}
	
	if (!osagetcontext (hglobals, contextID, &hcontext))
		goto error;
	
	if (!hashtablelookup (hcontext, bscode, &vcode, &hnode)) {
		
		oserror (errOSACorruptData);
		
		goto error;
		}
	
	if (!osahandletrapverb (hglobals, hverb, vcode.data.codevalue, modeFlags, vresult))
		goto error;
	
	return (true);
	
	error:
	
	#if version42orgreater
	
		if (osageterror () == errAEEventNotHandled) { // verb not handled by context
		
			/***this is where the ResumeDispatchProc should be invoked*/
	
			register hdlcomponentglobals hcg = osaglobals;
			hdllandglobals hg = landgetglobals (); // get global verb handler
			GNEUPP origproc;
			boolean fl;
			
			origproc = osainstallpatch (hcg);
      
			fl = (*(**hg).handleverbroutine) (hverb);
			
			getverbresult (hverb, vresult);

      osaremovepatch (hcg, origproc);
			
			return (fl);
			}
	#endif
		
		return (false);
	} /*osahandleevent*/


static pascal OSAError osaExecuteEvent (
				hdlcomponentglobals	hglobals,
				AppleEvent*			event,
				OSAID				contextID,
				long				modeFlags,
				OSAID*				resultingScriptValueID) {
	
	hdlverbrecord hverb = nil;
	tyvaluerecord vresult;
	boolean fl = false;
	
	if (contextID == kOSANullScript)
		return (errOSAInvalidID);
	
	if (!landsystem7unpackverb (event, nil, &hverb))
		goto exit;
	
	if (!osahandleevent (hglobals, hverb, contextID, modeFlags, &vresult))
		goto exit;
	
	if (!addstorageval (hglobals, &vresult, resultingScriptValueID))
		goto exit;
	
	fl = true;
	
	exit: {
		
		landdisposeverb (hverb);
		
		if (!fl)
			return (osageterror ());
		
		return (noErr);
		}
	} /*osaExecuteEvent*/


static pascal OSAError osaDoEvent (
			hdlcomponentglobals	hglobals,
			AppleEvent*			event,
			OSAID				contextID,
			long				modeFlags,
			AppleEvent*			reply) {
	
	/*
	2.1b4 dmb: if an error occurs running the script, try filling out error reply
	
	4.0.2b1 dmb: create reply is caller hasn't already
	*/
	
	register hdlcomponentglobals hcg = hglobals;
	hdlverbrecord hverb = nil;
	tyvaluerecord vresult;
	boolean fl = false;
	
	if (contextID == kOSANullScript)
		return (errOSAInvalidID);
	
	if ((*reply).descriptorType == typeNull)	/*dmb 6.17.96*/
		if (oserror (AECreateList (nil, 0, true, reply)))
			goto exit;
	
	if (!landsystem7unpackverb (event, reply, &hverb))
		goto exit;
	
	if (!osahandleevent (hcg, hverb, contextID, modeFlags, &vresult))
		goto exit;
	
	fl = true;
	
	landstartreturn (hverb);
	
	langipcpushparam (&vresult, returnedvaluekey, hverb);
	
	exit: {
		
		landdisposeverb (hverb);
		
		if (!fl) {
			
			AEDesc desc;
			OSAError err = osageterror ();
			
			if (err == errOSAScriptError) {
				
				err = errOSAGeneralError;
				
				if (osaScriptError (hcg, kOSAErrorMessage, typeChar, &desc) == noErr) {
					
					AEPutKeyDesc (reply, keyErrorString, &desc);
					
					AEDisposeDesc (&desc);
					}
				
				if (osaScriptError (hcg, kOSAErrorNumber, typeShortInteger, &desc) == noErr) {
					
					/*PBS 03/14/02: AE OS X fix.*/
					
          {
          Handle h;
          
          copydatahandle (&desc, &h);
          
          err = numberfromhandle (h);
          
          disposehandle (h);
          } 
					
					
					AEPutKeyDesc (reply, keyErrorNumber, &desc);
					
					AEDisposeDesc (&desc);
					}
				}
			
			return (err);
			}
		
		return (noErr);
		}
	} /*osaDoEvent*/


static pascal OSAError osaSetDebugProc (
			hdlcomponentglobals	hglobals,
			OSADebugUPP			debugProc,
			long				refCon) {
	
	register hdlcomponentglobals hcg = hglobals;
	
	(**hcg).debugproc = debugProc;
	
	(**hcg).debugprocrefcon = refCon;
	
	return (noErr);
	} /*osaSetDebugProc*/


static pascal OSAError osaDebug (
			hdlcomponentglobals	hglobals,
		OSType				 selector,
		const AEDesc		*selectorData,
		DescType			 desiredType,
		AEDesc				*resultingDebugInfoOrDataToSet) {
#pragma unused (hglobals, desiredType)

	OSAError err = noErr;
	hdlhashnode hnode;

	switch (selector) {

		case kOSADebugRange: {
			AEDesc rec;
			
			err = AECreateList (nil, 0, true, &rec);
			
			if (err == noErr) {
				
				long ix = langgetsourceoffset (ctscanlines, ctscanchars);
				
				err = AEPutKeyPtr (&rec, keyOSADebugRangeStart, typeLongInteger, (Ptr) &ix, sizeof (ix));
				
				err = AEPutKeyPtr (&rec, keyOSADebugRangeEnd, typeLongInteger, (Ptr) &ix, sizeof (ix));
				
				if (err == noErr)
					err = AECoerceDesc (&rec, typeOSADebugRange, resultingDebugInfoOrDataToSet);
				
				AEDisposeDesc (&rec);
				}
			
			break;
			}
		
		case kOSADebugSetData: {
			bigstring bs;
			tyvaluerecord val;
			
			if ((*selectorData).descriptorType != typeChar) {
				
				err = errAEWrongDataType;
				
				break;
				}
			
			/*PBS 03/14/02: AE OS X fix.*/
			
      datahandletostring ((AEDesc*) selectorData, bs);
			
			
			if (!langgetsymbolval (bs, &val, &hnode)) {
				
				err = errOSAScriptError;
				
				break;
				}
			
			if (!copyvaluerecord (val, &val)) {
				
				err = memFullErr;
				
				break;
				}
			
			/*
			if (!coercetobinary (&val)) {
				
				disposevaluerecord (val, true);
				
				break;
				}
			
			binarytodesc (val.data.binaryvalue, resultingDebugInfoOrDataToSet);
			*/
			
			if (!valuetodescriptor (&val, resultingDebugInfoOrDataToSet))
				err = memFullErr;
			
			break;
			}
		
		default:
			return (errOSABadSelector);
		}
	
	return (err);
	} /*osaDebug*/


static pascal ComponentResult cmpclose (Handle storage, ComponentInstance self) {
#pragma unused(self)

	register hdlcomponentglobals hglobals = (hdlcomponentglobals) storage;
	
	if (hglobals != nil) { /*defensive driving -- can be necessary during shutdown*/
		
		osaStopRecording (hglobals, kOSANullScript); /*make sure we don't leave recoding in progress*/
		
		disposecomponentglobals (hglobals);
		}
	
	return (noErr);
	} /*cmpclose*/


static pascal ComponentResult cmpcando (short selector) {
	
	switch (selector) {
		
		case kComponentOpenSelect:
		case kComponentCloseSelect:
		case kComponentCanDoSelect:
		case kComponentVersionSelect:
		case kOSASelectLoad:
		case kOSASelectStore:
		case kOSASelectExecute:
		case kOSASelectDisplay:
		case kOSASelectScriptError:
		case kOSASelectDispose:
		case kOSASelectSetScriptInfo:
		case kOSASelectGetScriptInfo:
		case kOSASelectCompile:
		case kOSASelectGetSource:
		case kOSASelectCoerceFromDesc:
		case kOSASelectCoerceToDesc:
		case kOSASelectStartRecording:
		case kOSASelectStopRecording:
		case kOSASelectScriptingComponentName:
		case kOSASelectLoadExecute:
		case kOSASelectCompileExecute:
		case kOSASelectDoScript:
		case kOSASelectMakeContext:
		case kOSASelectSetResumeDispatchProc:
		case kOSASelectGetResumeDispatchProc:
		case kOSASelectExecuteEvent:
		case kOSASelectDoEvent:
		case kOSASelectSetActiveProc:
		case kOSASelectSetDebugProc:
		case kOSASelectDebug:
		
		case kOSASelectSetSendProc:
		case kOSASelectGetSendProc:
		case kOSASelectSetCreateProc:
		case kOSASelectGetCreateProc:
		/*
		case kOSASelectSetDefaultTarget:
		*/
			return (true);
		}
	
	return (false);
	} /*cmpcando*/


static pascal ComponentResult cmpversion (void) {

	return (0x04100100);
	} /*cmpversion*/


pascal ComponentResult callosafunction (Handle storage, register ComponentParameters *params, ComponentFunctionUPP func) {
	
	/*
	4/13/93 dmb: account for re-entrancy, though I don't think it's handled everywhere
	
	2.1b5 dmb: re-entrancy is pretty good at this point (better be!), but we do 
	need to clear fllangerror before exiting. (we might do this at the beginning 
	instead; shouldn't really matter.)
	
	3.0a dmb: that was the wrong place to clear fllangerror, since it was affecting 
	the background context, not the osa context. the cleanest place to clear it is 
	before we call the osa function, but we really need to clear it when we're done. 
	that way, this is a nested call, an error here doesn't force the outer call to 
	die.
	*/
	
	hdlcomponentglobals hcg = (hdlcomponentglobals) storage;
	hdllandglobals hlg;
	hdlhashtable ht;
	ComponentResult result;

	//Code change by Timothy Paustian Monday, June 26, 2000 9:38:15 PM
	//
  CGrafPtr saveport;
  saveport = GetQDGlobalsThePort();
		
	osapushfastcontext (hcg);
	
	ht = (**hcg).storagetable;
	
	if (ht == currenthashtable) /*avoid redundant push -- save stack space*/
		ht = nil;
	
	if (ht != nil)
		pushhashtable (ht);
	
	shellpusherrorhook ((errorhookcallback) &osaerrormessage);
	
	flthreadkilled = false; /*reset*/
	
	langcallbacks.errormessagecallback = &osaerrormessage;
	
	hlg = landgetglobals ();
	
	(**hlg).eventcreatecallback = &osacreateevent;
	
	(**hlg).eventsendcallback = &osasendevent;

	result = CallComponentFunctionWithStorage ((Handle) hcg, params, func);
	
	shellpoperrorhook ();
	
	if (ht != nil)
		pophashtable ();
	
	fllangerror = false; /*clear in case we're nested*/
	
	osapopfastcontext (hcg);
	
	oserror (noErr); /*clear it out to avoid conflicts with current thread*/

	//Code change by Timothy Paustian Monday, June 26, 2000 9:37:17 PM
	//
    {
    CGrafPtr thePort;
    thePort = GetQDGlobalsThePort();
	
		if (thePort != saveport)
			SetPort (saveport);
		}

	return (result);
	} /*callosafunction*/


static pascal ComponentResult osaDispatch (ComponentParameters *params, Handle storage) {
	
	ComponentResult result = noErr;
	short what = (*params).what;

  hdlcomponentglobals	theGlobals = nil;

  if(what != kComponentOpenSelect) //else we create the globals
    theGlobals = (hdlcomponentglobals)storage;

	
	if (what < 0) { /*negative selectors are component manager calls*/
		
		switch (what) {
			
			case kComponentOpenSelect: { /*can't call subroutine because a5 isn't ours*/
				
				hdlcomponentglobals hglobals;
				Component self = (Component) (*params).params [0];
				long selfa5;
				
				selfa5 = GetComponentRefcon (self);
				
				//Code change by Timothy Paustian Monday, June 26, 2000 9:39:54 PM
				//We don't need this
				
				if (newcomponentglobals (self, 0, &hglobals))
					SetComponentInstanceStorage ((ComponentInstance) self, (Handle) hglobals);
				else
					result = memFullErr;
				
				break;
				}
			
			case kComponentCloseSelect:
				result = CallComponentFunctionWithStorage (storage, params, (**theGlobals).cmpcloseUPP);
				break;
			
			case kComponentCanDoSelect:
				result = CallComponentFunction (params, (**theGlobals).cmpcandoUPP);
				break;
			
			case kComponentVersionSelect:
				result = CallComponentFunction(params, (**theGlobals).cmpversionUPP);
				break;
			
			default:
				result = badComponentSelector;
				break;
			}
		}
	else {	/*positive selectors are OSA calls*/
		
		ComponentFunctionUPP func = nil;

		switch (what) {
			
			case kOSASelectLoad:
				func = (**theGlobals).osaLoadUPP;
				break;
			
			case kOSASelectStore:
				func = (**theGlobals).osaStoreUPP;
				break;
			
			case kOSASelectExecute:
				func = (**theGlobals).osaExecuteUPP;
				break;
			
			case kOSASelectDisplay:
				func = (**theGlobals).osaDisplayUPP;
				break;
			
			case kOSASelectScriptError:
				func = (**theGlobals).osaScriptErrorUPP;
				break;
			
			case kOSASelectDispose:
				func = (**theGlobals).osaDisposeUPP;
				break;
			
			case kOSASelectSetScriptInfo:
				func = (**theGlobals).osaSetScriptInfoUPP;
				break;
			
			case kOSASelectGetScriptInfo:
				func = (**theGlobals).osaGetScriptInfoUPP;
				break;
			
			case kOSASelectCompile:
				func = (**theGlobals).osaCompileUPP;
				break;
			
			case kOSASelectGetSource:
				func = (**theGlobals).osaGetSourceUPP;
				break;
			
			case kOSASelectCoerceFromDesc:
				func = (**theGlobals).osaCoerceFromDescUPP;
				break;
			
			case kOSASelectCoerceToDesc:
				func = (**theGlobals).osaCoerceToDescUPP;
				break;
			
			case kOSASelectStartRecording:
				func = (**theGlobals).osaStartRecordingUPP;
				break;
			
			case kOSASelectStopRecording:
				func = (**theGlobals).osaStopRecordingUPP;
				break;
			
			case kOSASelectScriptingComponentName:
				func = (**theGlobals).osaScriptingComponentNameUPP;
				break;
			
			case kOSASelectLoadExecute:
				func = (**theGlobals).osaLoadExecuteUPP;
				break;
			
			case kOSASelectCompileExecute:
				func = (**theGlobals).osaCompileExecuteUPP;
				break;
			
			case kOSASelectDoScript:
				func = (**theGlobals).osaDoScriptUPP;
				break;
			
			case kOSASelectMakeContext:
				func = (**theGlobals).osaMakeContextUPP;
				break;
			
			case kOSASelectSetResumeDispatchProc:
				func = (**theGlobals).osaSetResumeDispatchProcUPP;
				break;
			
			case kOSASelectGetResumeDispatchProc:
				func = (**theGlobals).osaGetResumeDispatchProcUPP;
				break;
			
			case kOSASelectExecuteEvent:
				func = (**theGlobals).osaExecuteEventUPP;
				break;
			
			case kOSASelectDoEvent:
				func = (**theGlobals).osaDoEventUPP;
				break;
			
			case kOSASelectSetActiveProc:
				func = (**theGlobals).osaSetActiveProcUPP;
				break;
			
			case kOSASelectSetDebugProc:
				func = (**theGlobals).osaSetDebugProcUPP;
				break;
			
			case kOSASelectDebug:
				func = (**theGlobals).osaDebugUPP;
				break;
			
			case kOSASelectSetSendProc:
				func = (**theGlobals).osaSetSendProcUPP;
				break;
			
			case kOSASelectGetSendProc:
				func = (**theGlobals).osaGetSendProcUPP;
				break;
			
			case kOSASelectSetCreateProc:
				func = (**theGlobals).osaSetCreateProcUPP;
				break;
			
			case kOSASelectGetCreateProc:
				func = (**theGlobals).osaGetCreateProcUPP;
				break;
			
			} /*switch*/
		
		if (func == nil)
			result = badComponentSelector;
		else
			result = callosafunction (storage, params, func);
		}
	
	return (result);
	} /*osaDispatch*/


static boolean osacomponenterror (ComponentInstance comp, OSAID idscript, OSAError err) {
	
	/*
	2.1b4 dmb: get error number and check for user canceled
	
	2.1b11 dmb: talked to William Cook at Apple. it turns out that AS can 
	only return a valid error range if you first ask for the source. 
	otherwise, it doesn't know what dialect the script is in.
	*/
	
	AEDesc errordesc;
	AEDesc scriptsource;
	
	if (err != errOSAScriptError)
		return (oserror (err));
	
	if (OSAScriptError (comp, kOSAErrorNumber, typeLongInteger, &errordesc) == noErr) {
		
		/*PBS 03/14/02: AE OS X fix.*/
    {
    Handle h;
    
    copydatahandle (&errordesc, &h);
    
    err = numberfromhandle (h);
    
    disposehandle (h);
    }
		
		AEDisposeDesc (&errordesc);
		
		if (err == userCanceledErr)
			return (true);
		}
	
	if (idscript != kOSANullScript) { /*ask for source so AS knows dialect for range info*/
		
		if (OSAGetSource (comp, idscript, typeWildCard, &scriptsource) == noErr)
			AEDisposeDesc (&scriptsource);
		}
	
	if (OSAScriptError (comp, kOSAErrorRange, typeOSAErrorRange, &errordesc) == noErr) {
		
		AERecord rec;
		unsigned long offset, type;
		long size;
		
		if (AECoerceDesc (&errordesc, typeAERecord, &rec) == noErr) {
			
			if (AEGetKeyPtr (&rec, keyOSASourceEnd, typeLongInteger, &type, (Ptr) &offset, sizeof (offset), &size) == noErr)
				langsetsourceoffset (offset);
			
			AEDisposeDesc (&rec);
			}
		
		AEDisposeDesc (&errordesc);
		}
	
	if (OSAScriptError (comp, kOSAErrorMessage, typeChar, &errordesc) == noErr) {
		
		bigstring bserror;
		
		/*PBS 03/14/02: AE OS X fix.*/
    datahandletostring (&errordesc, bserror);
		
		AEDisposeDesc (&errordesc);
		
		langerrormessage (bserror);
		}
	else
		oserror (err);
	
	return (true);
	} /*osacomponenterror*/


static boolean addosaserver (ComponentInstance instance, OSType type) {
	
	/*
	3.0b16 dmb: the client's psn is now part of the server record; server 
	instances cannot be safely shared between clients
	*/
	
	tyservercomponent server;
	hdlservercomponent hserver;
	
	server.instance = instance;
	
	server.type = type;
	
	GetCurrentProcess (&server.clientpsn); /*3.015*/
	
	if (!newfilledhandle (&server, sizeof (server), (Handle *) &hserver))
		return (false);
	
	listlink ((hdllinkedlist) hserverlist, (hdllinkedlist) hserver);
	
	return (true);
	} /*addosaserver*/


ComponentInstance getosaserver (OSType type) {
	
	/*
	3.0b16 dmb: instances are no longer shared between client processes
	
	3.0a dmb: set A5 to CurA5 when opening a component
	*/
	
	register hdlservercomponent hserver;
	register ComponentInstance instance;
	
	for (hserver = hserverlist; hserver != nil; hserver = (**hserver).hnext) {
		
		if ((**hserver).type == type) {
			
			if (!iscurrentapplication ((**hserver).clientpsn)) /*3.0b16*/
				continue;
			
			instance = (**hserver).instance;
			
			if (GetComponentVersion (instance) == badComponentInstance) { /*no longer valid*/
				
				listunlink ((hdllinkedlist) hserverlist, (hdllinkedlist) hserver);
				
				disposehandle ((Handle) hserver);
				
				break;
				}
			
			return (instance);
			}
		}

  instance = OpenDefaultComponent (kOSAComponentType, type);
	
	if (instance != nil)
		addosaserver (instance, type);
	
	return (instance);
	} /*getosaserver*/


static boolean initosaservers (void) {
	
	/*
	initialize the list of open servers. once open, we never close a server 
	until quit time.   the first record that we allocate here is just  the 
	list header.
	*/
	
	return (newclearhandle (longsizeof (tyservercomponent), (Handle *) &hserverlist));
	} /*initosaservers*/


void closeosaservers (void) {
	
	/*
	cleanly close each open server and remove from the server list.
	
	3.0b16 dmb: only close servers that were opened in this process
	*/
	
	register hdlservercomponent hserver;
	register hdlservercomponent hnext;
	
	for (hserver = hserverlist; hserver != nil; hserver = hnext) {
		
		hnext = (**hserver).hnext;
		
		if (!iscurrentapplication ((**hserver).clientpsn)) /*3.0b16*/
			continue;
		
		if ((**hserver).instance != nil) {
			
			CloseComponent ((**hserver).instance);
			
			listunlink ((hdllinkedlist) hserverlist, (hdllinkedlist) hserver);
			
			disposehandle ((Handle) hserver);
			}
		}
	} /*closeosaservers*/


static boolean openscriptcomponent (const tyvaluerecord *osaval, AEDesc *scriptdata, OSType *subtype, ComponentInstance *comp) {
	
	/*
	common code: htree is an osascript node. create an AEDesc that contains 
	the compiled script, and open the component for the script.
	
	2.1b4 dmb: try to get storage type directly instead of relying on the 
	generic component to translate
	*/
	
	Handle hdata;
	OSType idserver;
	
	assert ((*osaval).valuetype == binaryvaluetype);
	
	if ((*osaval).valuetype != binaryvaluetype) /*not a script*/
		return (false);
	
	if (!copyhandle ((*osaval).data.binaryvalue, &hdata))
		return (false);
	
	binarytodesc (hdata, scriptdata);
	
	/*PBS 03/14/02: AE OS X fix.*/
  {
  Handle h;
  
  copydatahandle (scriptdata, &h);
  
  if (OSAGetStorageType ((AEDataStorage) h, &idserver) != noErr)
    idserver = (*scriptdata).descriptorType;
    
  disposehandle (h);
  }
	
	*comp = getosaserver (idserver);
	
	if (*comp == nil) {
		
		AEDisposeDesc (scriptdata);
		
		langostypeparamerror (cantopencomponenterror, idserver);
		
		return (false);
		}
	
	*subtype = idserver;
	
	return (true);
	} /*openscriptcomponent*/


boolean osagetcode (Handle htext, OSType idserver, boolean fljustexecutable, tyvaluerecord *vcode) {
	
	/*
	caller owns htext; we don't dispose it
	
	3.0b15 dmb: fixed memory leak -- must dispose script id after 
	it's been stored.
	*/
	
	ComponentInstance comp;
	AEDesc scriptsource, scriptdata;
	OSAID id;
	long mode;
	boolean fl = false;
	OSAError err;
	
	comp = getosaserver (idserver);
	
	if (comp == nil) {
		
		langostypeparamerror (cantopencomponenterror, idserver);
		
		return (false);
		}
	
	/*PBS 03/14/02: AE OS X fix.*/
  newdescwithhandle (&scriptsource, typeChar, htext);
	
	id = kOSANullScript;
	
	err = OSACompile (comp, &scriptsource, kOSAModeCompileIntoContext, &id);

  AEDisposeDesc (&scriptsource);	/* 2004-10-27 aradke: we're done with it */
	
	if (!osacomponenterror (comp, kOSANullScript, err)) {
		
		if (fljustexecutable)
			mode = kOSAModePreventGetSource;
		else
			mode = kOSANullMode;
		
		err = OSAStore (comp, id, typeOSAGenericStorage, mode, &scriptdata);
		
		fl = !oserror (err);
		
		OSADispose (comp, id); /*we're done with the id*/
		}
	
	if (!fl)
		return (false);
	
	/*PBS 03/14/02: AE OS X fix.*/
  {
  Handle h;
  
  copydatahandle (&scriptdata, &h);
  
  fl = setbinaryvalue (h, scriptdata.descriptorType, vcode);
  
  AEDisposeDesc (&scriptdata);	/* 2004-10-27 aradke: we're done with it */

  return (fl);
  }
	} /*osagetcode*/


boolean osagetsource (const tyvaluerecord *osaval, OSType *idserver, tyvaluerecord *vsource) {
	
	ComponentInstance comp;
	AEDesc scriptsource, scriptdata;
	OSAID idscript;
	OSAError err;
	
	if (!openscriptcomponent (osaval, &scriptdata, idserver, &comp))
		return (false);
	
	err = OSALoad (comp, &scriptdata, kOSANullMode, &idscript);
	
	AEDisposeDesc (&scriptdata);
	
	if (err == noErr) {
		
		err = OSAGetSource (comp, idscript, typeChar, &scriptsource);
		
		OSADispose (comp, idscript);
		}
	
	if (osacomponenterror (comp, kOSANullScript, err))
		return (false);
	
	/*PBS 03/14/02: AE OS X fix.*/
  {
  Handle h;
  
  copydatahandle (&scriptsource, &h);
  
  AEDisposeDesc (&scriptsource);	/* 2004-10-28 aradke: we're done with it */
  
  return (setheapvalue (h, stringvaluetype, vsource));
  }
	} /*osagetsource*/


boolean isosascriptnode (hdltreenode htree, tyvaluerecord *osaval) {
	
	register hdltreenode h = htree;
	
	if ((**h).nodetype == moduleop)
		h = (**h).param1;
	
	if ((**h).nodetype != osascriptop)
		return (false);
	
	*osaval = (**h).nodeval;
	
	return (true);
	} /*isosascriptnode*/


#if 0 //not used

boolean isosascriptvalue (const tyvaluerecord *val) {
	
	/*
	enforce a strong definition of an osascriptvalue: a binary whose type
	is 'scpt', that might be confused with a packed Frontier script
	*/
	
	Handle x;
	OSErr err;
	OSType subtype;
	
	if ((*val).valuetype != binaryvaluetype)
		return (false);
	
	x = (*val).data.binaryvalue;
	
	if (getbinarytypeid (x) != typeOSAGenericStorage) /*it might be; can't tell*/
		return (false);
	
	err = OSAGetStorageType (x, &subtype);
	
	return (err == noErr);
	} /*isosascriptvalue*/

#endif


static pascal OSErr osaclientactive (long refcon) {
#pragma unused (refcon)

	/*
	while executing an osa script, we want to check for cmd-period, 
	and allow other threads to run. this does part of the job.
	*/
	
	OSErr err = noErr;
	
	if (!langbackgroundtask (false) || languserescaped (false))
		err = userCanceledErr;
	
	return (err);
	} /*osaclientactive*/


static pascal Boolean osaclientidleproc (EventRecord *ev, long *sleep, RgnHandle *mousergn) {
	
	/*
	2.1b11 dmb: we don't want to lose events when the server sends an 
	apple event.
	
	2.1b14 dmb: don't process null events, or scriptbackgroundtask will be 
	thrown off, and we won't be able to cancel
	*/
	
	#pragma unused (sleep, mousergn)
	
	if ((*ev).what != nullEvent)
		shellprocessevent (ev);
	
	return (false); /*keep waiting*/
	} /*osaclientidleproc*/


static pascal OSErr
osaclientsend (
		const AppleEvent	*event,
		AppleEvent			*reply,
		AESendMode			 sendmode,
		AESendPriority		 priority,
		long				 timeout,
		AEIdleUPP			 idleproc,
		AEFilterUPP			 filterproc,
		long				 refcon)
{
#pragma unused (refcon, idleproc)

	/*
	with AppleScript, at least, the activeproc doesn't get called enough 
	for decent thread cooperation. ideally, we'd send this event using 
	the same logic as landsystem7send, sleeping this thread if possible. 
	but that would take too much new coding and testing to do now.  instead, 
	we'll just do what the active proc does before sending the event.
	
	3.0b14 dmb: must go through landglobals send callback; we're running as 
	part of a normal script.
	*/
	
	OSErr err;
	register hdllandglobals hlg;

	
	hlg = landgetglobals ();
	
	err = (*(**hlg).eventsendcallback) (event, reply, sendmode, priority, timeout, osaclientidleUPP, filterproc);

	/*
	err = AESend (event, reply, sendmode, priority, timeout, osaclientidleUPP, filterproc);
	*/
	
	if (err == noErr) {
		
		if (!langbackgroundtask (false) || languserescaped (false))
			err = userCanceledErr;
		}
	
	return (err);
	} /*osaclientsend*/


boolean evaluateosascript (const tyvaluerecord *osaval, hdltreenode hparam1, bigstring bsname, tyvaluerecord *vreturned) {
	
	/*
	2.1b5 dmb: set up an activeproc
	
	2.1b6 dmb: work around AS requirement that the subroutine name be 
	"canonified". I had a back-and-forth about this with Warren of the 
	AS team, and he's having their internationalization people "look 
	into" this problem.
	
	2.1b11 dmb: set up a send proc along with the activeproc.  also, don't 
	dispose of the script until we call osacomponenterror, so it can 
	be decompiled
	
	3.0b16 dmb: added call to dispose the script result id
	
	5.0d8 dmb: make sure to return false if an error is encountered.
	*/
	
	ComponentInstance comp;
	OSType subtype;
	AEDesc scriptdata, scriptresult;
	OSAID idscript, idresult;
	AppleEvent event;
	OSAError err;
	boolean fl = false;
	
	if (!openscriptcomponent (osaval, &scriptdata, &subtype, &comp))
		return (false);
	
	err = OSALoad (comp, &scriptdata, kOSANullMode, &idscript);
	
	AEDisposeDesc (&scriptdata);
	
	if (err == errOSABadStorageType) {
		
		langparamerror (notfunctionerror, bsname);
		
		return (false);
		}
	
	if (oserror (err))
		return (false);
	
	//Code change by Timothy Paustian Sunday, September 3, 2000 10:45:59 PM
	//are theses globasl a problem?
	OSASetActiveProc (comp, osaclientactiveUPP, 0L); /*make sure we can cancel*/
	
	OSASetSendProc (comp, osaclientsendUPP, 0L); /*allows for better thread cooperation*/
			
	if (hparam1 == nil) { /*no parameters, just execute the code*/
		
		fl = true;
		
		err = OSAExecute (comp, idscript, kOSANullScript, kOSANullMode, &idresult);
		}
	else {
		
		if (subtype == 'ascr') /*work around AS bug; it requires "canonified" name as of 1.0.1*/
			alllower (bsname);
		
		fl = osabuildsubroutineevent (bsname, hparam1, &event);
		
		if (fl) {
			
			err = OSAExecuteEvent (comp, &event, idscript, kOSANullMode, &idresult);
			
			AEDisposeDesc (&event);
			}
		}
	
	OSASetActiveProc (comp, nil, 0L); /*clear it out -- AS does this too*/
	
	OSASetSendProc (comp, nil, 0L); /* ditto */
	
	if (fl)
		fl = !osacomponenterror (comp, idscript, err);
	
	if (fl) {
		
		if (idresult == kOSANullScript)
			fl = setbooleanvalue (true, vreturned);
		
		else {
			
			err = OSACoerceToDesc (comp, idresult, typeWildCard, kOSANullMode, &scriptresult);
			
			OSADispose (comp, idresult); /*3.0b16*/
			
			fl = !oserror (err) && setdescriptorvalue (scriptresult, vreturned);
			}
		}
	
	OSADispose (comp, idscript);
	
	return (fl);
	} /*evaluateosascript*/


boolean evaluateosascriptevent (const tyvaluerecord *osaval, const AppleEvent *event, AppleEvent *reply) {
	
	/*
	2.1b11 dmb: pass a valid script id to osacomponent error for proper 
	reporting
	*/
	
	ComponentInstance comp;
	AEDesc scriptdata;
	OSAID idscript;
	OSType tosstype;
	OSAError err;
	boolean fl = false;
	
	if (!openscriptcomponent (osaval, &scriptdata, &tosstype, &comp))
		return (false);
	
	err = OSALoad (comp, &scriptdata, kOSANullMode, &idscript);
	
	AEDisposeDesc (&scriptdata);
	
	if (!osacomponenterror (comp, kOSANullScript, err)) {
		
		err = OSADoEvent (comp, event, idscript, kOSANullMode, reply);
		
		fl = !osacomponenterror (comp, idscript, err);
		
		OSADispose (comp, idscript);
		}
	
	return (fl);
	} /*evaluateosascriptevent*/


boolean osafindclienteventfilter (long clienta5, long *eventfilter) {
	
	/*
	3.0b15 dmb: look to see if any menu sharing client has the specified 
	a5 value and a non-nil event filter. if such a client exists, set  
	eventfilter and return true.
	
	we should really create a generalize client visit routine, but this 
	is a very last-minute change, so I'm doing as little as possible.
	*/
	
	register hdlcomponentglobals hclient;
	
	for (hclient = (**hclientlist).hfirst; hclient != nil; hclient = (**hclient).hnext) {
		
		if ((**hclient).clienta5 == clienta5) { /*maybe the one*/
			
			*eventfilter = (long) (**hclient).menusharingglobals.eventfiltercallback;
			
			if (*eventfilter != 0)
				return (true);
			}
		}
	
	return (false);
	} /*osafindclienteventfilter*/


static boolean osagethomeresfile (void) {
	
	homeresfile = CurResFile ();
	
	return (true);
	} /*osagethomeresfile*/


Component osaregistercomponent (OSType type, long flags, ComponentRoutine dispatch, short idname, short iddescription) {
	
	/*
	2.1b11 dmb: new routine consolodates code for registering all 
	Frontier components.  strings are now in a single STR# resource
	
	7.0b17 AR: For Radio UserLand (Pike), register all OSA components as local.
	It looks like this will allow us to run Frontier and Radio UserLand simultaneously.
	*/
	
	ComponentDescription desc;
	Handle hname, hdescription, hicon;
	Component comp;
	#ifdef PIKE
		short global = 0;
	#else
		short global = registerComponentGlobal;
	#endif
	
	desc.componentType = type;
	
	desc.componentSubType = 'LAND';
	
	desc.componentManufacturer = 'LAND';
	
	desc.componentFlags = flags;
	
	desc.componentFlagsMask = 0;
	
	hname = getcomponentstringhandle (idname);
	
	hdescription = getcomponentstringhandle (iddescription);
	
	hicon = GetIcon (129);
	
  comp = RegisterComponent (&desc, NewComponentRoutineUPP (dispatch),
                  global, hname, hdescription, hicon);
	
	disposehandle (hname);
	
	disposehandle (hdescription);
	
	return (comp);
	} /*osaregistercomponent*/


static boolean initosacomponent (void) {
	
	/*
	register the scripting component.
	
	all attempts to use a resource-based thng failed miserably. this might be 
	a bug in the system, but the only reason to was attempted was to get use 
	of the OpenComponentResFile call, which we do manually.
	
	7.0b17 AR: After we have registered the component successfully,
	open it and create an instance for our own use. This is to avoid
	cross-talk between Frontier and Radio UserLand when running MacBird cards.
	*/
	
	long flags = 0;
	
	osagethomeresfile();
	
	//Code change by Timothy Paustian Friday, July 21, 2000 11:18:39 PM
	//create all the osa UPPs
  osaclientactiveDesc = NewOSAActiveUPP(osaclientactive);
  osaclientsendDesc = NewOSASendUPP(osaclientsend);
  osaclientidleDesc = NewAEIdleUPP(osaclientidleproc);
  osadefaultactiveDesc = NewOSAActiveUPP(osadefaultactiveproc);
  osadefaultcreateDesc = NewOSACreateAppleEventUPP(osadefaultcreate);
  osadefaultsendDesc = NewOSASendUPP(osadefaultsend);
		
	shellpushfilehook (&osagethomeresfile);
	
	flags = kOSASupportsCompiling |
			kOSASupportsGetSource |
			kOSASupportsConvenience |
			kOSASupportsRecording |
			kOSASupportsAECoercion |
			kOSASupportsAESending |
			kOSASupportsEventHandling;
	
			
	osacomponent = osaregistercomponent (kOSAComponentType, flags, &osaDispatch, usertalkstring, scriptingcomponentstring);
	
	if (osacomponent != nil) { /*7.0b17 AR*/

		ComponentInstance instance;
		
		instance = OpenComponent (osacomponent);
		
		if (instance != nil)
			addosaserver (instance, 'LAND');
		}

	
	return (osacomponent != nil);
	} /*initosacomponent*/


boolean havecomponentmanager (void) {

	long result;
	
	if (!gestalt (gestaltComponentMgr, &result))
		return (false);
	
	return (result != 0);
	
	} /*havecomponentmanager*/


boolean getprocessname (ProcessSerialNumber psn, bigstring bsname, boolean *flbackgroundonly) {
	
	ProcessInfoRec info;
	
	info.processInfoLength = (long) sizeof (info);
	
	info.processName = bsname;
	
	info.processAppSpec = nil;
	
	if (GetProcessInformation (&psn, &info) != noErr)
		return (false);
	
	*flbackgroundonly = (info.processMode & modeOnlyBackground) != 0;
	
	return (true);
	} /*getprocessname*/


boolean osacomponentverifyshutdown (void) {
	
	/*
	see if we have any active clients; if so, warn user before and confirm 
	shutdown.
	
	note: need to closeosaservers first since we may be our own client
	
	2.1b5 dmb: check clientlist for nil in case it was never created
	
	2.1b11 dmb: don't alert users about menusharing clients
	*/
	
	register hdlcomponentglobals hclient;
	short ctclients;
	short lidstring;
	bigstring bs;
	bigstring bsprompt;
	typrocessid psn, lastpsn;
	boolean flbackgroundonly;
	Boolean flsame;
	
	if (hclientlist == nil) /*list never initted, probably no component manager*/
		return (true);
	
	closeosaservers (); /*this is ok to do, event if we don't end up quitting*/
	
	ctclients = 0;
	
	lastpsn.highLongOfPSN = lastpsn.lowLongOfPSN = kNoProcess;
	
	for (hclient = (**hclientlist).hfirst; hclient != nil; hclient = (**hclient).hnext) {
		
		psn = (**hclient).clientpsn;
		
		if ((**hclient).menusharingglobals.clientid != 0) /*ignore menusharing clients*/
			return (true);
		
		if (!getprocessname (psn, bs, &flbackgroundonly)) /*must be gone*/
			continue;
		
		if (flbackgroundonly)
			continue;
		
		if ((SameProcess (&lastpsn, &psn, &flsame) == noErr) && flsame)
			continue;
		
		if (iscurrentapplication (psn)) /*frontier is it's own client -- maybe ran a card?*/
			continue;
		
		lastpsn = psn;
		
		++ctclients;
		}
	
	if (ctclients > 0) {
		
		if (ctclients == 1)
			lidstring = specificclientstring;
		
		else {
			numbertostring (ctclients, bs);
			
			lidstring = multipleclientsstring;
			}
		
		if (getrecordingstring (lidstring, bsprompt)) {
			
			parsedialogstring (bsprompt, bs, nil, nil, nil, bsprompt);
			
			if (!msgdialog (bsprompt))
				return (false);
			}
		}
	
	return (true);
	} /*osacomponentverifyshutdown*/


static boolean servingsharedmenus (hdlcomponentglobals *hclient) {
	
	/*
	look to see if any component client has a set of shared menus 
	in place. if such a client exists and is a valid process, set 
	bs to its name and return true. otherwise, return false.
	
	2.1b13 dmb: changed the criteria for deciding whether we're serving 
	shared menus. if the menusharingglobals clientid is non-zero, we 
	need to wait for a disconnect event when hsharedmenus is nil. 
	otherwise, we won't have a chance to remove our event handler from 
	the client
	
	2.1b13 dmb: for runtime, don't wait for own disconnect, because it's 
	already happened.
	
	3.0.4b6 dmb: return component globals of found client, not its process
	name. that way, caller can send it an apple event.
	*/
	
	register hdlcomponentglobals hcg;
	typrocessid psn;
	bigstring bs;
	boolean flbackgroundonly;
	
	for (hcg = (**hclientlist).hfirst; hcg != nil; hcg = (**hcg).hnext) {
		
		psn = (**hcg).clientpsn;
		
		if (!getprocessname (psn, bs, &flbackgroundonly)) /*must be gone*/
			continue;
		
		
		if ((**hcg).menusharingglobals.clientid != 0) {
		
			*hclient = hcg;
			
			return (true);
			}
		}
	
	return (false);
	} /*servingsharedmenus*/


static pascal OSErr sendmenusharingshutdownevent (hdlcomponentglobals hcg) {
	
	/*
	3.0.4b6 dmb: send event to menu sharing client that will be handled by 
	the handler we installed during initialization. the handler will 
	shutdown cleanly within the client process.  If the client had menus 
	installed, it should have already receive one of these as we disposed 
	the menubar.
	
	5.0d3 dmb: send event with kAENoReply, not kAEWaitReply. Otherwise we can 
	deallock. Our caller is looping and will keep sending the event until it
	is handled.
	*/
	
	AEDesc desc;
	OSErr err;
	ProcessSerialNumber psn = (**hcg).clientpsn;
	AppleEvent event, reply;
	
	err = AECreateDesc (typeProcessSerialNumber, (Ptr) &psn, sizeof (psn), &desc);
	
	if (err == noErr) {
		
		err = AECreateAppleEvent ((**hcg).clientid, idupdatemenus, &desc, kAutoGenerateReturnID, kAnyTransactionID, &event);
		
		AEDisposeDesc (&desc);
		
		if (err == noErr) {
			
			err = AESend (&event, &reply, 
				
				(AESendMode) kAEDontRecord + kAECanSwitchLayer + kAECanInteract + kAENoReply, 
				
				(AESendPriority) kAEHighPriority, (long) kAEDefaultTimeout, nil, nil);
			
			AEDisposeDesc (&event);
			
			AEDisposeDesc (&reply);
			}
		}
	
	return (err);
	} /*sendmenusharingshutdownevent*/


#define shutdowntimeout 10 /*seconds we'll wait for clients to receive update menus event*/


void osacomponentshutdown (void) {
	
	/*
	2.1b8 dmb: wait for up to 4 seconds for menu sharing clients to 
	receive langipcmenu.c's 'done' message and dipose their shared 
	menus. after that, it's safe for us to go away
	
	2.1b13 dmb: oops, timenow is seconds, not ticks; we were waiting 
	600 seconds instead of 10.
	
	3.0.1b2 dmb: check clientlist for nil in case it was never created
	
	3.0.4b6 dmb: force client to shutdown menus explicitly by sending 
	it a dirty menus event. If it never had menus, it wouldn't otherwise
	receive this event.
	*/
	
	unsigned long startloop;
	hdlcomponentglobals hclient;
	
	flosashutdown = true; /*make sure we remove AE handlers*/
	
	if (hclientlist == nil) /*list never initted, probably no component manager*/
		return;
	
	closeosaservers ();
	
	startloop = timenow ();
	
	while (servingsharedmenus (&hclient)) {
		
		sendmenusharingshutdownevent (hclient);	// 3.0.4b6 dmb
		
		shellpartialeventloop (osMask);
		
		if (timenow () - startloop > shutdowntimeout)
			break;
		}
	
	AERemoveCoercionHandler (typeType, typeObjectSpecifier, coerceTypetoObjUPP, true);

  DisposeAECoerceDescUPP(coerceTypetoObjDesc);
  
  //the next three are used to send apple events
  DisposeOSAActiveUPP(osaclientactiveDesc);
  DisposeOSASendUPP(osaclientsendDesc);
  DisposeAEIdleUPP(osaclientidleDesc);
  
  DisposeOSAActiveUPP(osadefaultactiveDesc);
  DisposeOSACreateAppleEventUPP(osadefaultcreateDesc);
  DisposeOSASendUPP(osadefaultsendDesc);
	} /*osacomponentshutdown*/


boolean osacomponentstart (void) {
	
	/*
	2.1b4 dmb: register any thng's that might have been added to Frontier's 
	resource fork.  (we may want to bake in an app runner at some point, or 
	perhaps something else.)
	
	3.0b15 dmb: grab our own psn now.  used in osapartialeventloop.
	*/
	
	if (!havecomponentmanager ())
		return (false);
	
	GetCurrentProcess (&homepsn);

	if (!initosaservers ())
		return (false);
	
	initosacomponent ();
	
		initwindowsharingcomponent ();
	
	#ifdef dropletcomponent
		initdropletcomponent ();
	#endif
	
	RegisterComponentResourceFile (filegetapplicationrnum (), true); /*2.1b4*/
	
  coerceTypetoObjDesc = NewAECoerceDescUPP((AECoerceDescProcPtr)coerceTypetoObj);

	AEInstallCoercionHandler (typeType, typeObjectSpecifier, coerceTypetoObjUPP, 0, true, true);
	
	return (newclearhandle (longsizeof (tyclientlist), (Handle *) &hclientlist));
	} /*osacomponentstart*/

