
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

#if TARGET_API_MAC_CARBON == 1


/*copyright 1992-1993, UserLand Software, Inc.*/

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

#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/
	#include "aeutils.h"
#endif

static pascal OSErr osaclientactive (long refcon);
static pascal OSErr osaclientsend (const AppleEvent *, AppleEvent *, AESendMode, AESendPriority, long, AEIdleUPP, AEFilterUPP, long);
static pascal Boolean osaclientidleproc (EventRecord *ev, long *sleep, RgnHandle *mousergn);
//Code change by Timothy Paustian Sunday, September 3, 2000 10:02:12 PM
//more forward declarations
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
			
#if !TARGET_RT_MAC_CFM

	#define cmpcloseUPP	((ComponentFunctionUPP) cmpclose)
	#define cmpcandoUPP	((ComponentFunctionUPP) cmpcando)
	#define cmpversionUPP	((ComponentFunctionUPP) cmpversion)
	#define osaLoadUPP	((ComponentFunctionUPP) osaLoad)
	#define osaStoreUPP	((ComponentFunctionUPP) osaStore)
	#define osaExecuteUPP	((ComponentFunctionUPP) osaExecute)
	#define osaDisplayUPP	((ComponentFunctionUPP) osaDisplay)
	#define osaScriptErrorUPP	((ComponentFunctionUPP) osaScriptError)
	#define osaDisposeUPP	((ComponentFunctionUPP) osaDispose)
	#define osaSetScriptInfoUPP	((ComponentFunctionUPP) osaSetScriptInfo)
	#define osaGetScriptInfoUPP	((ComponentFunctionUPP) osaGetScriptInfo)
	#define osaCompileUPP	((ComponentFunctionUPP) osaCompile)
	#define osaGetSourceUPP	((ComponentFunctionUPP) osaGetSource)
	#define osaCoerceFromDescUPP	((ComponentFunctionUPP) osaCoerceFromDesc)
	#define osaCoerceToDescUPP	((ComponentFunctionUPP) osaCoerceToDesc)
	#define osaStartRecordingUPP	((ComponentFunctionUPP) osaStartRecording)
	#define osaStopRecordingUPP	((ComponentFunctionUPP) osaStopRecording)
	#define osaScriptingComponentNameUPP	((ComponentFunctionUPP) osaScriptingComponentName)
	#define osaLoadExecuteUPP	((ComponentFunctionUPP) osaLoadExecute)
	#define osaCompileExecuteUPP	((ComponentFunctionUPP) osaCompileExecute)
	#define osaDoScriptUPP	((ComponentFunctionUPP) osaDoScript)
	#define osaMakeContextUPP	((ComponentFunctionUPP) osaMakeContext)
	#define osaSetResumeDispatchProcUPP	((ComponentFunctionUPP) osaSetResumeDispatchProc)
	#define osaGetResumeDispatchProcUPP	((ComponentFunctionUPP) osaGetResumeDispatchProc)
	#define osaExecuteEventUPP	((ComponentFunctionUPP) osaExecuteEvent)
	#define osaDoEventUPP	((ComponentFunctionUPP) osaDoEvent)
	#define osaSetActiveProcUPP	((ComponentFunctionUPP) osaSetActiveProc)
	#define osaSetDebugProcUPP	((ComponentFunctionUPP) osaSetDebugProc)
	#define osaDebugUPP	((ComponentFunctionUPP) osaDebug)
	#define osaSetSendProcUPP	((ComponentFunctionUPP) osaSetSendProc)
	#define osaGetSendProcUPP	((ComponentFunctionUPP) osaGetSendProc)
	#define osaSetCreateProcUPP	((ComponentFunctionUPP) osaSetCreateProc)
	#define osaGetCreateProcUPP	((ComponentFunctionUPP) osaGetCreateProc)
	#define osaSetDefaultTargetUPP	((ComponentFunctionUPP) osaSetDefaultTarget)

#else
	
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

	#if !TARGET_API_MAC_CARBON
	static RoutineDescriptor cmpcloseDesc = BUILD_ROUTINE_DESCRIPTOR (cmpcloseProcInfo, cmpclose);
	static RoutineDescriptor cmpcandoDesc = BUILD_ROUTINE_DESCRIPTOR (cmpcandoProcInfo, cmpcando);
	static RoutineDescriptor cmpversionDesc = BUILD_ROUTINE_DESCRIPTOR (cmpversionProcInfo, cmpversion);
	static RoutineDescriptor osaLoadDesc = BUILD_ROUTINE_DESCRIPTOR (osaLoadProcInfo, osaLoad);
	static RoutineDescriptor osaStoreDesc = BUILD_ROUTINE_DESCRIPTOR (osaStoreProcInfo, osaStore);
	static RoutineDescriptor osaExecuteDesc = BUILD_ROUTINE_DESCRIPTOR (osaExecuteProcInfo, osaExecute);
	static RoutineDescriptor osaDisplayDesc = BUILD_ROUTINE_DESCRIPTOR (osaDisplayProcInfo, osaDisplay);
	static RoutineDescriptor osaScriptErrorDesc = BUILD_ROUTINE_DESCRIPTOR (osaScriptErrorProcInfo, osaScriptError);
	static RoutineDescriptor osaDisposeDesc = BUILD_ROUTINE_DESCRIPTOR (osaDisposeProcInfo, osaDispose);
	static RoutineDescriptor osaSetScriptInfoDesc = BUILD_ROUTINE_DESCRIPTOR (osaSetScriptInfoProcInfo, osaSetScriptInfo);
	static RoutineDescriptor osaGetScriptInfoDesc = BUILD_ROUTINE_DESCRIPTOR (osaGetScriptInfoProcInfo, osaGetScriptInfo);
	static RoutineDescriptor osaCompileDesc = BUILD_ROUTINE_DESCRIPTOR (osaCompileProcInfo, osaCompile);
	static RoutineDescriptor osaGetSourceDesc = BUILD_ROUTINE_DESCRIPTOR (osaGetSourceProcInfo, osaGetSource);
	static RoutineDescriptor osaCoerceFromDescDesc = BUILD_ROUTINE_DESCRIPTOR (osaCoerceFromDescProcInfo, osaCoerceFromDesc);
	static RoutineDescriptor osaCoerceToDescDesc = BUILD_ROUTINE_DESCRIPTOR (osaCoerceToDescProcInfo, osaCoerceToDesc);
	static RoutineDescriptor osaStartRecordingDesc = BUILD_ROUTINE_DESCRIPTOR (osaStartRecordingProcInfo, osaStartRecording);
	static RoutineDescriptor osaStopRecordingDesc = BUILD_ROUTINE_DESCRIPTOR (osaStopRecordingProcInfo, osaStopRecording);
	static RoutineDescriptor osaScriptingComponentNameDesc = BUILD_ROUTINE_DESCRIPTOR (osaScriptingComponentNameProcInfo, osaScriptingComponentName);
	static RoutineDescriptor osaLoadExecuteDesc = BUILD_ROUTINE_DESCRIPTOR (osaLoadExecuteProcInfo, osaLoadExecute);
	static RoutineDescriptor osaCompileExecuteDesc = BUILD_ROUTINE_DESCRIPTOR (osaCompileExecuteProcInfo, osaCompileExecute);
	static RoutineDescriptor osaDoScriptDesc = BUILD_ROUTINE_DESCRIPTOR (osaDoScriptProcInfo, osaDoScript);
	static RoutineDescriptor osaMakeContextDesc = BUILD_ROUTINE_DESCRIPTOR (osaMakeContextProcInfo, osaMakeContext);
	static RoutineDescriptor osaSetResumeDispatchProcDesc = BUILD_ROUTINE_DESCRIPTOR (osaSetResumeDispatchProcProcInfo, osaSetResumeDispatchProc);
	static RoutineDescriptor osaGetResumeDispatchProcDesc = BUILD_ROUTINE_DESCRIPTOR (osaGetResumeDispatchProcProcInfo, osaGetResumeDispatchProc);
	static RoutineDescriptor osaExecuteEventDesc = BUILD_ROUTINE_DESCRIPTOR (osaExecuteEventProcInfo, osaExecuteEvent);
	static RoutineDescriptor osaDoEventDesc = BUILD_ROUTINE_DESCRIPTOR (osaDoEventProcInfo, osaDoEvent);
	static RoutineDescriptor osaSetActiveProcDesc = BUILD_ROUTINE_DESCRIPTOR (osaSetActiveProcProcInfo, osaSetActiveProc);
	static RoutineDescriptor osaSetDebugProcDesc = BUILD_ROUTINE_DESCRIPTOR (osaSetDebugProcProcInfo, osaSetDebugProc);
	static RoutineDescriptor osaDebugDesc = BUILD_ROUTINE_DESCRIPTOR (osaDebugProcInfo, osaDebug);
	static RoutineDescriptor osaSetSendProcDesc = BUILD_ROUTINE_DESCRIPTOR (osaSetSendProcProcInfo, osaSetSendProc);
	static RoutineDescriptor osaGetSendProcDesc = BUILD_ROUTINE_DESCRIPTOR (osaGetSendProcProcInfo, osaGetSendProc);
	static RoutineDescriptor osaSetCreateProcDesc = BUILD_ROUTINE_DESCRIPTOR (osaSetCreateProcProcInfo, osaSetCreateProc);
	static RoutineDescriptor osaGetCreateProcDesc = BUILD_ROUTINE_DESCRIPTOR (osaGetCreateProcProcInfo, osaGetCreateProc);
	
	#define cmpcloseUPP (&cmpcloseDesc)
	#define cmpcandoUPP (&cmpcandoDesc)
	#define cmpversionUPP (&cmpversionDesc)
	#define osaLoadUPP (&osaLoadDesc)
	#define osaStoreUPP (&osaStoreDesc)
	#define osaExecuteUPP (&osaExecuteDesc)
	#define osaDisplayUPP (&osaDisplayDesc)
	#define osaScriptErrorUPP (&osaScriptErrorDesc)
	#define osaDisposeUPP (&osaDisposeDesc)
	#define osaSetScriptInfoUPP (&osaSetScriptInfoDesc)
	#define osaGetScriptInfoUPP (&osaGetScriptInfoDesc)
	#define osaCompileUPP (&osaCompileDesc)
	#define osaGetSourceUPP (&osaGetSourceDesc)
	#define osaCoerceFromDescUPP (&osaCoerceFromDescDesc)
	#define osaCoerceToDescUPP (&osaCoerceToDescDesc)
	#define osaStartRecordingUPP (&osaStartRecordingDesc)
	#define osaStopRecordingUPP (&osaStopRecordingDesc)
	#define osaScriptingComponentNameUPP (&osaScriptingComponentNameDesc)
	#define osaLoadExecuteUPP (&osaLoadExecuteDesc)
	#define osaCompileExecuteUPP (&osaCompileExecuteDesc)
	#define osaDoScriptUPP (&osaDoScriptDesc)
	#define osaMakeContextUPP (&osaMakeContextDesc)
	#define osaSetResumeDispatchProcUPP (&osaSetResumeDispatchProcDesc)
	#define osaGetResumeDispatchProcUPP (&osaGetResumeDispatchProcDesc)
	#define osaExecuteEventUPP (&osaExecuteEventDesc)
	#define osaDoEventUPP (&osaDoEventDesc)
	#define osaSetActiveProcUPP (&osaSetActiveProcDesc)
	#define osaSetDebugProcUPP (&osaSetDebugProcDesc)
	#define osaDebugUPP (&osaDebugDesc)
	#define osaSetSendProcUPP (&osaSetSendProcDesc)
	#define osaGetSendProcUPP (&osaGetSendProcDesc)
	#define osaSetCreateProcUPP (&osaSetCreateProcDesc)
	#define osaGetCreateProcUPP (&osaGetCreateProcDesc)
	#define osaSetDefaultTargetUPP (&osaSetDefaultTargetDesc)
	#else
	
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
	
	/*#define cmpcloseUPP (cmpcloseDesc)
	#define cmpcandoUPP (cmpcandoDesc)
	#define cmpversionUPP (cmpversionDesc)
	//#define osaLoadUPP (osaLoadDesc)
	#define osaStoreUPP (osaStoreDesc)
	#define osaExecuteUPP (osaExecuteDesc)
	#define osaDisplayUPP (osaDisplayDesc)
	#define osaScriptErrorUPP (osaScriptErrorDesc)
	#define osaDisposeUPP (osaDisposeDesc)
	#define osaSetScriptInfoUPP (osaSetScriptInfoDesc)
	#define osaGetScriptInfoUPP (osaGetScriptInfoDesc)
	#define osaCompileUPP (osaCompileDesc)
	#define osaGetSourceUPP (osaGetSourceDesc)
	#define osaCoerceFromDescUPP (osaCoerceFromDescDesc)
	#define osaCoerceToDescUPP (osaCoerceToDescDesc)
	#define osaStartRecordingUPP (osaStartRecordingDesc)
	#define osaStopRecordingUPP (osaStopRecordingDesc)
	#define osaScriptingComponentNameUPP (osaScriptingComponentNameDesc)
	#define osaLoadExecuteUPP (osaLoadExecuteDesc)
	#define osaCompileExecuteUPP (osaCompileExecuteDesc)
	#define osaDoScriptUPP (osaDoScriptDesc)
	#define osaMakeContextUPP (osaMakeContextDesc)
	#define osaSetResumeDispatchProcUPP (osaSetResumeDispatchProcDesc)
	#define osaGetResumeDispatchProcUPP (osaGetResumeDispatchProcDesc)
	#define osaExecuteEventUPP (osaExecuteEventDesc)
	#define osaDoEventUPP (osaDoEventDesc)
	#define osaSetActiveProcUPP (osaSetActiveProcDesc)
	#define osaSetDebugProcUPP (osaSetDebugProcDesc)
	#define osaDebugUPP (osaDebugDesc)
	#define osaSetSendProcUPP (osaSetSendProcDesc)
	#define osaGetSendProcUPP (osaGetSendProcDesc)
	#define osaSetCreateProcUPP (osaSetCreateProcDesc)
	#define osaGetCreateProcUPP (osaGetCreateProcDesc)
	#define osaSetDefaultTargetUPP (osaSetDefaultTargetDesc)*/
	#endif
	
	/*
	static RoutineDescriptor osaSetDefaultTargetDesc = BUILD_ROUTINE_DESCRIPTOR (osaSetDefaultTargetProcInfo, osaSetDefaultTarget);
	*/

	

#endif

#if !TARGET_RT_MAC_CFM
	
	#define osaclientactiveUPP (&osaclientactive)
	
	#define osaclientidleUPP (&osaclientidleproc)
	
	#define osaclientsendUPP (&osaclientsend)
	
#else

	#if !TARGET_API_MAC_CARBON
	static RoutineDescriptor osaclientactiveDesc = BUILD_ROUTINE_DESCRIPTOR (uppOSAActiveProcInfo, osaclientactive);
	
	static RoutineDescriptor osaclientidleDesc = BUILD_ROUTINE_DESCRIPTOR (uppAEIdleProcInfo, osaclientidleproc);
	
	static RoutineDescriptor osaclientsendDesc = BUILD_ROUTINE_DESCRIPTOR (uppOSASendProcInfo, osaclientsend);
	
	#define osaclientactiveUPP (&osaclientactiveDesc)
	
	#define osaclientidleUPP (&osaclientidleDesc)
	
	#define osaclientsendUPP (&osaclientsendDesc)
	
	#else
	OSAActiveUPP	osaclientactiveDesc;
	
	AEIdleUPP	osaclientidleDesc;
	
	OSASendUPP	osaclientsendDesc;
	
	#define osaclientactiveUPP (osaclientactiveDesc)
	
	#define osaclientidleUPP (osaclientidleDesc)
	
	#define osaclientsendUPP (osaclientsendDesc)
		
	
	#endif
#endif


#define kOSAScriptIsBeingEdited			'edit'
	// Selector returns boolean.

#define kOSAScriptIsBeingRecorded		'recd'
	// Selector returns boolean.


typedef struct tyservercomponent {
	
	struct tyservercomponent **hnext;
	
	ComponentInstance instance;
	
	ProcessSerialNumber clientpsn; /*3.015*/
	
	OSType type;
	} tyservercomponent, *ptrservercomponent, **hdlservercomponent;


typedef struct tyclientlist {
	
	hdlcomponentglobals hfirst;
	
	} tyclientlist, **hdlclientlist;


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


typedef struct tystylerecord {
	
	short ctstyles;
	
	ScrpSTElement styles [1];
	} tystylerecord;


static THz homezone;

static Handle homeresmap;

static short homeresfile;

static ProcessSerialNumber homepsn;


hdlcomponentglobals osaglobals = nil;

static osacoercionhandlerinstalled = 0;

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
	
	#if 0 /*def fldebug*/
	
	if (systemtable != nil) {
		
		hashtabledelete (systemtable, (ptrstring) 0x910);
		}
	else
	
	#endif
	
	#if TARGET_API_MAC_CARBON && TARGET_RT_MAC_CFM
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
	#endif
		
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
	if (EventAvail (keyDownMask, &ev)) {
		
		if (iscmdperiodevent (ev.message, ev.what, ev.modifiers))
			return (userCanceledErr);
		}
	
	return (noErr);
	} /*osadefaultactiveproc*/


static pascal OSErr osadefaultcreate (AEEventClass class, AEEventID id,
                    const AEAddressDesc *target, short returnID,
                    long transactionID, AppleEvent *result, long refcon) {
	
	return (AECreateAppleEvent(class, id, target, returnID, transactionID, result));
	} /*osadefaultcreate*/


static pascal OSErr osadefaultsend (const AppleEvent *event, AppleEvent *reply,
		AESendMode sendmode, AESendPriority priority, long timeout,
        AEIdleUPP idleproc, AEFilterUPP filterproc, UInt32 refcon) {
	
	return (AESend (event, reply, sendmode, priority, timeout, idleproc, filterproc));
	} /*osadefaultsend*/

//Code change by Timothy Paustian Friday, July 21, 2000 10:52:57 PM
//I think I can get away with this because only frontier code calls it.
#if !TARGET_RT_MAC_CFM

	#define osadefaultactiveUPP ((OSAActiveProcPtr) &osadefaultactiveproc)
	
	#define osadefaultcreateUPP ((OSACreateAppleEventProcPtr) &osadefaultcreate)
	
	#define osadefaultsendUPP ((OSASendProcPtr) &osadefaultsend)
	
#else

	#if TARGET_API_MAC_CARBON
	OSAActiveUPP osadefaultactiveDesc = nil;
	
	OSACreateAppleEventUPP osadefaultcreateDesc = nil;
	
	OSASendUPP osadefaultsendDesc = nil;

	#define osadefaultactiveUPP (osadefaultactiveDesc)
	
	#define osadefaultcreateUPP (osadefaultcreateDesc)
	
	#define osadefaultsendUPP (osadefaultsendDesc)
	
	#else
	static RoutineDescriptor osadefaultactiveDesc = BUILD_ROUTINE_DESCRIPTOR (uppOSAActiveProcInfo, osadefaultactiveproc);
	
	static RoutineDescriptor osadefaultcreateDesc = BUILD_ROUTINE_DESCRIPTOR (uppOSACreateAppleEventProcInfo, osadefaultcreate);
	
	static RoutineDescriptor osadefaultsendDesc = BUILD_ROUTINE_DESCRIPTOR (uppOSASendProcInfo, osadefaultsend);

	#define osadefaultactiveUPP (&osadefaultactiveDesc)
	
	#define osadefaultcreateUPP (&osadefaultcreateDesc)
	
	#define osadefaultsendUPP (&osadefaultsendDesc)
	#endif


#endif 

// forward declarations
static boolean osabackgroundtask (boolean);
static boolean osadebugger (hdltreenode);
static boolean osapartialeventloop (short);


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
	
	#if TARGET_API_MAC_CARBON == 1
	(**hcg).clientzone = LMGetApplZone();
	#else	
	(**hcg).clientzone = GetZone ();
	#endif
		
	(**hcg).clientid = appid;
	
	(**hcg).clientpsn = psn;
	
	#if 0 /*def fldebug*/
	
	if (!tablenewsystemtable (systemtable, (ptrstring) 0x910, &storagetable))
	
	#else
	
	#if TARGET_API_MAC_CARBON == 1
	LMSetApplZone(homezone);
	#else	
	SetZone (homezone);
	#endif
		
	fl = newhashtable (&storagetable);
	
	#if TARGET_API_MAC_CARBON == 1
	LMSetApplZone((**hcg).clientzone);
	#else	
	SetZone ((**hcg).clientzone);
	#endif
	
	if (!fl)
	
	#endif
		{
		disposehandle ((Handle) hcg);
		
		return (false);
		}
	
	(**hcg).storagetable = storagetable;
	
	#if TARGET_API_MAC_CARBON && TARGET_RT_MAC_CFM
	(**hcg).activeproc = NewOSAActiveUPP(osadefaultactiveproc);
	
	(**hcg).createproc = NewOSACreateAppleEventUPP(osadefaultcreate);
	
	(**hcg).sendproc = NewOSASendUPP(osadefaultsend);
	//Code change by Timothy Paustian Sunday, September 3, 2000 9:57:20 PM
	//We have to create theses all and store them in the globals variables.
	(**hcg).cmpcloseUPP = NewComponentFunctionUPP(cmpclose, cmpcloseProcInfo);
	(**hcg).cmpcandoUPP = NewComponentFunctionUPP(cmpcando, cmpcandoProcInfo);
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
	#else
		
	(**hcg).activeproc = osadefaultactiveUPP;
	
	(**hcg).createproc = osadefaultcreateUPP;
	
	(**hcg).sendproc = osadefaultsendUPP;
	#endif
			
	/*
	if (!newclearhandle (longsizeof (tyMSglobals), (Handle *) &hMSglobals)) {
		
		disposecomponentglobals (hcg);
		
		return (false);
		}
	
	(**hcg).hMSglobals = hMSglobals;
	*/
	
	if (!newthreadglobals (&hthreadglobals)) {
		
		disposecomponentglobals (hcg);
		
		return (false);
		}
	
	htg = hthreadglobals;
	
	(**hcg).clientthreadglobals = htg;	// 4.1b3 dmb: use local
	
	(**htg).htable = storagetable;
	
	(**htg).applicationid = appid;
	
	#if version42orgreater
		// 2/28/97 dmb: see up langcallbacks here so they'll always be in effect
		
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
	
	AEDesc list, rec, desc;
	long n;
	OSErr err;
	
	list = (**osaglobals).errordesc;
	
	AEDisposeDesc (&list);
	
	n = osageterror ();
	
	err = AECreateList (nil, 0, true, &list);
	
	if (err != noErr) {
		
		#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/
		
			newdescnull (&list, typeNull);
		
		#else
		
			list.descriptorType = typeNull;
		
			list.dataHandle = nil;
			
		#endif
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
	
	exit:
	
	(**osaglobals).errordesc = list;
	
	return (false); /*consume the error*/
	} /*osaerrormessage*/

#if 1

static pascal OSErr coerceTEXTtoSTXT (
		DescType fromtype,
		Ptr pdata,
		long size,
		DescType totype,
		long refcon,
		AEDesc *result) {
	
	/*
	2.1b2 dmb: don't use clearbytes so we don't have to set up a5
	*/
	
	tystylerecord stylerecord;
	OSErr err;
	AEDesc list;
	register ScrpSTElement *pstyle;
	
	#ifdef fldegug
	
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

#else

static pascal OSErr coerceTEXTtoSTXT (
		AEDesc *fromdesc,
		AEDesc *result) {
	
	/*
	2.1b2 dmb: no longer installed as a coercion handler, we call it directly
	*/
	
	tystylerecord stylerecord;
	OSErr err;
	AEDesc list;
	register ScrpSTElement *pstyle;
	
	if ((*fromdesc).descriptorType != typeChar)
		return (errAECoercionFail);
	
	stylerecord.ctstyles = 1;
	
	pstyle = &stylerecord.styles [0];
	
	(*pstyle).scrpStartChar = 0;
	
	(*pstyle).scrpHeight = 16;
	
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
			err = AEPutKeyDesc (&list, 'ktxt', fromdesc);
		
		if (err == noErr)
			err = AECoerceDesc (&list, typeStyledText, result);
		
		AEDisposeDesc (&list);
		}
	
	return (err);
	} /*coerceTEXTtoSTXT*/

#endif


static pascal OSErr coerceTypetoObj (
		AEDesc *desc,
		DescType totype,
		long refcon,
		AEDesc *result) {
	
	/*
	2.1b1 dmb: if Frontier passes a string4 identifier where an object specifier 
	is required, this coercion handler will take care of it
	*/
	
	AEDesc containerdesc;
	OSErr err;
	
	long curA5 = SetUpAppA5 ();
	
	#ifdef fldebug
	
	if ((*desc).descriptorType != typeType)
		DebugStr ("\punexpected coercion");
	
	#endif
	
	#if TARGET_API_MAC_CARBON == 1
	
		newdescnull (&containerdesc, typeNull);
	
	#else
	
		containerdesc.descriptorType = typeNull;
	
		containerdesc.dataHandle = nil;
	
	#endif
	
	err = CreateObjSpecifier (cProperty, &containerdesc, formPropertyID, desc, false, result);
	
	RestoreA5 (curA5);
	
	return (err);
	} /*coerceTypetoObj*/

//Code change by Timothy Paustian Friday, June 16, 2000 1:38:13 PM
//Changed to Opaque call for Carbon
#if !TARGET_RT_MAC_CFM
	
	#define coerceTEXTtoSTXTUPP ((AECoercionHandlerUPP) &coerceTEXTtoSTXT)
	
	#define coerceTypetoObjUPP ((AECoercionHandlerUPP) &coerceTypetoObj)
	
#else

	#if !TARGET_API_MAC_CARBON
	static RoutineDescriptor coerceTEXTtoSTXTDesc = BUILD_ROUTINE_DESCRIPTOR (uppAECoercePtrProcInfo, coerceTEXTtoSTXT);
	
	static RoutineDescriptor coerceTypetoObjDesc = BUILD_ROUTINE_DESCRIPTOR (uppAECoerceDescProcInfo, coerceTypetoObj);
	#define coerceTEXTtoSTXTUPP ((AECoercionHandlerUPP) &coerceTEXTtoSTXTDesc)
	
	#define coerceTypetoObjUPP ((AECoercionHandlerUPP) &coerceTypetoObjDesc)
	
	#else
	AECoercionHandlerUPP	coerceTEXTtoSTXTDesc;
	AECoercionHandlerUPP	coerceTypetoObjDesc;
	
	#define coerceTEXTtoSTXTUPP (coerceTEXTtoSTXTDesc)
	#define coerceTypetoObjUPP (coerceTypetoObjDesc)
	#endif

	
#endif

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
		
		#if !flruntime
		
		(**htg).hccglobals = nil; /*want to leave them untouched*/
		
		#endif
		
		swapinthreadglobals (htg);
		}
	
	if (++osacoercionhandlerinstalled == 1)
	{
		#if TARGET_API_MAC_CARBON && TARGET_RT_MAC_CFM
		if(coerceTEXTtoSTXTUPP == nil)
			coerceTEXTtoSTXTUPP = NewAECoerceDescUPP(coerceTEXTtoSTXT);
		#endif
		AEInstallCoercionHandler (typeChar, typeStyledText, coerceTEXTtoSTXTUPP, 0, false, false);
	}
	if (!(**hcg).isHomeProcess) {
	
		(**hcg).clientresfile = CurResFile ();
		
		//Code change by Timothy Paustian Friday, June 16, 2000 2:08:20 PM
		//Changed to Opaque call for Carbon
		//This is not supported in carbon. You cannot use res files in OS X
		//We may have some serious rewriting to do for this.
		#if !TARGET_API_MAC_CARBON
		(**hcg).clientresmaphandle = LMGetTopMapHndl ();
		
		LMSetTopMapHndl (homeresmap);
		#endif

		
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
		#if TARGET_API_MAC_CARBON  && TARGET_RT_MAC_CFM
		if(coerceTEXTtoSTXTUPP != nil)
		{
			//DisposeAECoerceDescUPP(coerceTEXTtoSTXT);
			DisposeAECoerceDescUPP(coerceTEXTtoSTXTUPP);
			coerceTEXTtoSTXTUPP = nil;
		}
		#endif
		
	}
	if (!(**hcg).isHomeProcess) {
		#if !TARGET_API_MAC_CARBON
		LMSetTopMapHndl ((**hcg).clientresmaphandle);
		#endif

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


static UniversalProcPtr installpatch (short trapnum, GNEUPP patch) {
	
	/*
	2002-10-13 AR: Initialiize origtrap to nil just in case some Carbon code
	actually relies on us to return a meaningful result.
	*/
	
	UniversalProcPtr origtrap = nil;
	
	#if !TARGET_API_MAC_CARBON
	origtrap = NGetTrapAddress (trapnum, ToolTrap);
	
	NSetTrapAddress ((UniversalProcPtr) patch, trapnum, ToolTrap);
	#endif
	
	return (origtrap);
	} /*installpatch*/


static void removepatch (short trapnum, GNEUPP origtrap) {
	
	#if !TARGET_API_MAC_CARBON
	NSetTrapAddress ((UniversalProcPtr) origtrap, trapnum, ToolTrap);
	#endif

	} /*removepatch*/


static pascal Boolean osagetnextevent (short, EventRecord *);

//Code change by Timothy Paustian Monday, June 26, 2000 4:11:25 PM
//
#if !TARGET_RT_MAC_CFM || TARGET_API_MAC_CARBON

	#define osagetnexteventUPP (&osagetnextevent)

#else

	static RoutineDescriptor osagetnexteventDesc = BUILD_ROUTINE_DESCRIPTOR (uppGNEProcInfo, osagetnextevent);
	
	#define osagetnexteventUPP (&osagetnexteventDesc)
	
#endif


GNEUPP osainstallpatch (hdlcomponentglobals hglobals) {
	
	/*
	3.0a dmb: return the original value of getnexteventproc so it can 
	be restored in osaremovepath. this allows patching pairs to be 
	nested, so calls to handlerunscript can be nested.
	*/
	
	register hdlcomponentglobals hcg = hglobals;
	GNEUPP origproc;
	UniversalProcPtr origtrap;
	
	if ((**hcg).isHomeProcess)
		origproc = nil;
		
	else {
	
		#if !TARGET_API_MAC_CARBON
		origtrap = installpatch (_GetNextEvent, osagetnexteventUPP);
		#else
		origtrap = nil;
		#endif
		
		origproc = (**hcg).getnexteventproc;
		
		(**hcg).getnexteventproc = (GNEUPP) origtrap;
		}
	
	return (origproc);
	} /*osainstallpatch*/


void osaremovepatch (hdlcomponentglobals hglobals, GNEUPP origproc) {

	register hdlcomponentglobals hcg = hglobals;
	
	if ((**hcg).isHomeProcess)
		;	
	else {
	
		assert ((**hcg).getnexteventproc != nil);
		
		#if !TARGET_API_MAC_CARBON
		removepatch (_GetNextEvent, (**hcg).getnexteventproc);
		#endif
				
		(**hcg).getnexteventproc = origproc;
		}
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


THz osasethomezone (hdlcomponentglobals hglobals) {
	
	/*
	2.1b13 dmb: added this to balance the setclientzone call, and to 
	allow us to keep the homezone global private.
	
	in the future, this routine might reset the clientzone in hgobals. 
	now's not the time to fiddle with this, though.
	*/
	
	/*
	register hdlcomponentglobals hcg = hglobals;
	*/
	register THz origzone;
	//Code change by Timothy Paustian Monday, June 26, 2000 9:37:30 PM
	//
	#if TARGET_API_MAC_CARBON == 1
	origzone = LMGetApplZone();
	LMSetApplZone(homezone);
	#else
		
	origzone = GetZone ();
	
	SetZone (homezone);
	#endif
		
	return (origzone);
	} /*osasethomezone*/


THz osasetclientzone (hdlcomponentglobals hglobals) {
	
	/*
	2.1b11 dmb: snippet for setting the clients zone.  we return 
	the current zone because most callers will want to restore it
	*/
	
	register hdlcomponentglobals hcg = hglobals;
	register THz origzone;
	#if TARGET_API_MAC_CARBON == 1
	origzone = LMGetApplZone();
	LMSetApplZone((**hcg).clientzone);
	#else
		
 	origzone = GetZone ();
	
	SetZone ((**hcg).clientzone);
	#endif
		
	return (origzone);
	} /*osasetclientzone*/


static pascal Boolean osagetnextevent (short mask, EventRecord *event) {
	
	/*
	if any toolbox call that we use calls GetNextEvent or EventAvail, we 
	need to take many of the same precautions as we do for background 
	tasking. in particular, we need to make sure that the Frontier environment 
	is restored, since it may get swapped in by the process manager
	*/
	
	register hdlcomponentglobals hcg;
	register GNEUPP getnexteventproc;
	register THz origzone;
	Boolean fl;
	
	long curA5 = SetUpAppA5 ();
	
	hcg = osaglobals;
	
	getnexteventproc = (**hcg).getnexteventproc;
	
	osapopfastcontext (hcg);
	
	origzone = osasetclientzone (hcg);
	
	osaremovepatch (hcg, nil); /*unpatch*/
	
	RestoreA5 (curA5);
	
	fl = CallGNEProc (getnexteventproc, mask, event);

	curA5 = SetUpAppA5 ();
	
	osainstallpatch (hcg); /*repatch*/
	#if TARGET_API_MAC_CARBON == 1
	LMSetApplZone(origzone);
	#else
	SetZone (origzone);
	#endif
		
	osapushfastcontext (hcg);
	
	RestoreA5 (curA5);
	
	return (fl);
	} /*osagetnextevent*/


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
	register THz origzone;
	OSErr err;
	long clienta5, appA5;
	
	osapopfastcontext (hcg);
	
	origzone = osasetclientzone (hcg);
	
	clienta5 = (**hcg).clienta5;
	
	#ifdef THINK_C
	
		asm {
				move.l	a5,-(a7)
				move.l	clienta5,a5
			}
		
		err = (*(**hcg).createproc) (class, id, target, returnID, transactionID, result, (**hcg).createprocrefcon);
	
	#else
	
		appA5 = SetUpCurA5 ();	// SetUpThisA5 (clienta5);	// 4.0b5: was: SetUpCurA5 ();
		
		#if TARGET_API_MAC_CARBON == 1

			err = InvokeOSACreateAppleEventUPP (class, id, target, returnID, transactionID, result, (**hcg).createprocrefcon, (**hcg).createproc);
		
		#else
		
			err = CallOSACreateAppleEventProc ((**hcg).createproc, class, id, target, returnID, transactionID, result, (**hcg).createprocrefcon);
		
		#endif
		
	#endif
	
	RestoreA5 (appA5);
	#if TARGET_API_MAC_CARBON == 1
	LMSetApplZone(origzone);
	#else
	SetZone (origzone);
	#endif
		
	osapushfastcontext (hcg);
	
	return (err);
	} /*osacreateevent*/


static pascal OSErr osasendevent (const AppleEvent *event, AppleEvent *reply,
		AESendMode mode, AESendPriority priority, long timeout,
        AEIdleUPP idleproc, AEFilterUPP filterproc ) {
	
	/*
	2/16/93 dmb: in case the event is to be direct dispatched correctly, we need 
	to set up the client's A5 world
	
	2.1b5 dmb: watch out for nil sendProc -- the AS script editor sets this.
	*/
	
	register hdlcomponentglobals hcg = osaglobals;
	OSErr err;
	long clienta5, appA5;
	register GNEUPP getnexteventproc;
	register THz origzone;
	OSASendUPP sendproc;
	
	sendproc = (**hcg).sendproc;
	
	getnexteventproc = (**hcg).getnexteventproc;
	
	osapopfastcontext (hcg);
	
	origzone = osasetclientzone (hcg);
	
	if (getnexteventproc != nil)
		osaremovepatch (hcg, nil); /*unpatch*/
	
	clienta5 = (**hcg).clienta5;
	
	#ifdef THINK_C
	
		asm {
				move.l	a5,-(a7)
				move.l	clienta5,a5
			}
		
		err = (*sendproc) (event, reply, mode, priority, timeout, nil, nil, (**hcg).sendprocrefcon);
	
	#else
	
		appA5 = SetUpCurA5 ();	// SetUpThisA5 (clienta5);	// 4.0b5: was: SetUpCurA5 ();
		
		#if TARGET_API_MAC_CARBON == 1

			err = InvokeOSASendUPP (event, reply, mode, priority, timeout, nil, nil, (**hcg).sendprocrefcon, sendproc);
		
		#else
		
			err = CallOSASendProc (sendproc, event, reply, mode, priority, timeout, nil, nil, (**hcg).sendprocrefcon);
	
		#endif
		
	#endif
	
	RestoreA5 (appA5);
	
	if (getnexteventproc != nil)
		osainstallpatch (hcg); /*repatch*/
	
	#if TARGET_API_MAC_CARBON == 1
	LMSetApplZone(origzone);
	#else	
	SetZone (origzone);
	#endif
		
	osapushfastcontext (hcg);
	
	return (err);
	} /*osasendevent*/


static boolean osabackgroundtask (boolean flresting) {
	
	/*
	very carefully call back to the client.
	*/
	
	register hdlcomponentglobals hcg = osaglobals;
	THz origzone;
	OSErr err;
	long clienta5, appA5;
	
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
	
	origzone = osasetclientzone (hcg);
	
	osaremovepatch (hcg, nil); /*unpatch*/
	
	clienta5 = (**hcg).clienta5;
	
	#ifdef THINK_C
	
		asm {
				move.l	a5,-(a7)
				move.l	clienta5,a5
			}
		
		err = (*(**hcg).activeproc) ((**hcg).activeprocrefcon);
	
	#else
	
		appA5 = SetUpThisA5 (clienta5);	// 4.0b5: was: SetUpCurA5 ();
		
		#if TARGET_API_MAC_CARBON == 1

			err = InvokeOSAActiveUPP ((**hcg).activeprocrefcon, (**hcg).activeproc);
		
		#else
		
			err = CallOSAActiveProc ((**hcg).activeproc, (**hcg).activeprocrefcon);
		
		#endif
		
	#endif
	
	RestoreA5 (appA5);
	
	osainstallpatch (hcg); /*repatch*/
	
	#if TARGET_API_MAC_CARBON == 1
	LMSetApplZone(origzone);
	#else	
	SetZone (origzone);
	#endif
		
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
	THz origzone;
	OSErr err;
	long clienta5, appA5;
	
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
	
	origzone = osasetclientzone (hcg);
	
	osaremovepatch (hcg, nil); /*unpatch*/
	
	clienta5 = (**hcg).clienta5;
	
	#ifdef THINK_C
	
		asm {
				move.l	a5,-(a7)
				move.l	clienta5,a5
			}
		
		err = (*(**hcg).debugproc) ((**hcg).debugprocrefcon);
	
	#else
	
		appA5 = SetUpCurA5 ();	// SetUpThisA5 (clienta5);	// 4.0b5: was: SetUpCurA5 ();
		
		err = CallOSADebugProc ((**hcg).debugproc, (**hcg).debugprocrefcon);
	
	#endif
	
	RestoreA5 (appA5);
	
	osainstallpatch (hcg); /*repatch*/
	
	#if TARGET_API_MAC_CARBON == 1
	LMSetApplZone(origzone);
	#else
	SetZone (origzone);
	#endif
		
	osapushfastcontext (hcg);
	
	return (!oserror (err));
	} /*osadebugger*/


#if !flruntime

static boolean osaprocessstarted (void) {
	
	/*
	we don't want Frontier's menus to dim when serving osa scripts
	*/
	
	processnotbusy ();
	
	return (true);
	} /*osaprocessstarted*/

#endif


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
	
	#if !flruntime
		
		(**hp).fldebugging = bitboolean (modeflags & kOSAModeDebug);
		
		(**hp).processstartedroutine = &osaprocessstarted;
		
	#endif
	
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
	
	#if TARGET_API_MAC_CARBON == 1
	LMSetApplZone(homezone);
	#else	
	SetZone (homezone);
	#endif
		
	if (testheapspace (2 * 1024)) /*enough memory to run a process*/
		fl = processruncode (hp, resultval);
	else
		fl = false;
	
	osasetclientzone (hcg);
	
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
	
	HNoPurge (haete);
	
	flgotname = osaparseaete (haete, class, id, bsname, &paramoffset);
	
	HPurge (haete);
	
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

#if 1

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

#else

static boolean osabuildsubroutineevent (bigstring bsname, hdltreenode hparam1, AppleEvent *event) {
	
	/*
	2.1b3 dmb: build an OSA subroutine event given the name and parameter list
	
	2.1b4 dmb: handle named parameters. if a name is defined, try to use its value 
	as the key. otherwise, 
	*/
	
	AEDesc params = {typeNull, nil};
	AEDesc desc;
	tyvaluerecord vparam;
	tyvaluerecord vkey;
	bigstring bskey;
	OSType key;
	register hdltreenode hp;
	register OSErr err;
	register boolean fl = false;
	hdlhashnode hnode;
	
	err = AECreateAppleEvent (kOSASuite, kASSubroutineEvent, &params, kAutoGenerateReturnID, kAnyTransactionID, event);
	
	if (oserror (err))
		return (false);
	
	err = AEPutParamPtr (event, keyASSubroutineName, typeChar, (Ptr) bsname + 1, stringlength (bsname));
	
	if (err != noErr)
		goto exit;
	
	err = AECreateList (nil, 0, false, &params);
	
	if (err != noErr)
		goto exit;
	
	for (hp = hparam1; hp != nil; hp = (**hp).link) {
		
		if ((**hp).nodetype == fieldop) {
		
			if (!langgetidentifier ((**hp).param1, bsname))
				goto exit;
			
			if (langgetsymbolval (bsname, &vkey, &hnode) && (vkey.valuetype == ostypevaluetype))
				key = vkey.data.ostypevalue;
			else
				stringtoostype (bsname, &key);
			
			if (!evaluatetree ((**hp).param2, &vparam))
				goto exit;
			
			if (!valuetodescriptor (&vparam, &desc))
				goto exit;
			
			err = AEPutKeyDesc (event, key, &desc);
			}
		else {
			
			if (!evaluatetree (hp, &vparam))
				goto exit;
			
			if (!valuetodescriptor (&vparam, &desc))
				goto exit;
			
			err = AEPutDesc (&params, 0, &desc);
			}
		
		AEDisposeDesc (&desc);
		
		if (err != noErr)
			goto exit;
		}
	
	err = AEPutParamDesc (event, keyDirectObject, &params);
	
	if (err != noErr)
		goto exit;
	
	fl = true;
	
	exit:
	
	AEDisposeDesc (&params);
	
	if (!fl) {
		
		oserror (err); /*set error code, if available*/
		
		AEDisposeDesc (event);
		}
	
	return (fl);
	} /*osabuildsubroutineevent*/

#endif

static boolean setstorageval (hdlcomponentglobals hglobals, tyvaluerecord *val, OSAID id) {
	
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
	
	bigstring bs;
	
	numbertostring (id, bs);
	
	if (!hashlookup (bs, val, &hnode)) {
		
		oserror (errOSAInvalidID);
		
		return (false);
		}
	
	return (true);
	} /*getstorageval*/


static boolean deletestorageval (hdlcomponentglobals hglobals, OSAID id) {
	
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
	
	/*
	if (!coercetobinary (v)) { /*get it into a handle-based form%/
		
		disposevaluerecord (*v, true);
		
		return (false);
		}
	
	binarytodesc ((*v).data.binaryvalue, &desc);
	
	exemptfromtmpstack (*v); /*descriptor owns handle now%/
	*/
	
	if (!valuetodescriptor (v, &desc))
		return (false);
	
	if ((desctype != typeWildCard) && (desctype != desc.descriptorType)) { /*AE coercion needed*/
		
		err = AECoerceDesc (&desc, desctype, result);
		
		/*
		if ((err != noErr) && (desctype == typeStyledText))
			err = coerceTEXTtoSTXT (&desc, result);
		*/
		
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
	
	register THz origzone;
	boolean fl;
	//Code change by Timothy Paustian Monday, June 26, 2000 9:37:45 PM
	//
	#if TARGET_API_MAC_CARBON == 1
	origzone = LMGetApplZone();
	LMSetApplZone(homezone);
	#else
	origzone = GetZone ();
	SetZone (homezone);
	#endif
	fl = langexternalnewvalue (id, hdata, val);
	
	#if TARGET_API_MAC_CARBON == 1
	LMSetApplZone(origzone);
	#else	
	SetZone (origzone);
	#endif
		
	return (fl);
	} /*osanewvalue*/


static pascal OSAError osaLoad (
			hdlcomponentglobals	hglobals,
			const AEDesc*		scriptData,
			long				modeFlags,
			OSAID*				resultingCompiledScriptID) {
	
	/*
	2.1b1 dmb: don't insist that the loaded value is a context (i.e. a table)
	*/
	Handle		descData = nil;
	DescType	descType;
	tyvaluerecord vscript, vsource, vcode;
	hdlhashtable hcontext;
	Handle hsource;
	hdltreenode hcode;
	Handle hdata;
	DescType subtype;
	boolean fl;
	OSAError err;
	hdlhashnode hnodesource;
	hdlhashnode hnodecode;
	
	#if TARGET_API_MAC_CARBON == 1
	SInt32	descDataSize = AEGetDescDataSize(scriptData);
	if(descDataSize != 0)
	{
		descData = NewHandle(descDataSize);
		HLock(descData);
		AEGetDescData(scriptData, *descData, descDataSize);
		HUnlock(descData);
	}
	#else
	descData = scriptData->dataHandle;
	#endif
	descType = scriptData->descriptorType;
	
	
	switch (descType) {
		
		case typeLAND:
			err = noErr;
			
			fl = langunpackvalue (descData, &vscript);
			
			break;
		
		case typeOSAGenericStorage:
			err = OSAGetStorageType (descData, &subtype);
			
			if (err != noErr)
				break;
			
			if (subtype != typeLAND)
				return (errOSABadStorageType);
			
			if (!copyhandle (descData, &hdata))
				return (memFullErr);
			
			err = OSARemoveStorageType (hdata);
			
			if (err == noErr)
				fl = langunpackvalue (hdata, &vscript);

			disposehandle (hdata);
			
			break;
		
		default:
			err = errOSABadStorageType;
		}
	
	if (err != noErr)
		goto error;
	
	if (!fl)
	{
		goto error;
	}
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
	
	if (fl)
		fl = addstorageval (hglobals, &vscript, resultingCompiledScriptID);
	
	error:
	//clean up memory
	#if TARGET_API_MAC_CARBON
	if(descData != nil)
	{
		DisposeHandle(descData);
		descData = nil;
	}
	#endif

	if (!fl)
		return (osageterror ());
	else if (err!= noErr)
		return err;
	
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
	hdlhashtable hcontext;
	hdlhashnode hnode;
	boolean flunlinkedsource = false;
	boolean fl;
	OSAError err;
	
	
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
	
	#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/
	
		{
		Handle hcopy;
		
		fl = langpackvalue (val, &hcopy, HNoNode);
		
		newdescwithhandle (&desc, typeLAND, hcopy);
		
		disposehandle (hcopy);
		}
	
	#else
	
		fl = langpackvalue (val, &desc.dataHandle, HNoNode);
	
	#endif
	
	if (flunlinkedsource)
		hashinsertnode (hnode, hcontext);
	
	if (!fl)
		return (memFullErr);
	
	desc.descriptorType = typeLAND;
	
	if (desiredType == typeOSAGenericStorage) {
		
		#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/
		
			{
			Handle hcopy;
			
			copydatahandle (&desc, &hcopy);
			
			err = OSAAddStorageType (hcopy, typeLAND);
			
			//disposehandle (hcopy);

			if (err != noErr)				
				return (err);
			}
		
		#else
		
			err = OSAAddStorageType (desc.dataHandle, typeLAND);
		
			if (err != noErr) {
				
				DisposeHandle (desc.dataHandle);
				
				return (err);
				}
				
		#endif
		
		
		desc.descriptorType = typeOSAGenericStorage;
		}
	
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


static pascal OSAError osaGetResumeDispatchProc (
				hdlcomponentglobals	hglobals,
				AEEventHandlerUPP*	resumeDispatchProc,
				long*				refCon) {
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
	
	#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/
	
		if (!copydatahandle ((AEDesc*) sourceData, &htext)) /*don't let langbuildtree consume caller's text*/
			return (memFullErr);
	
	#else
	
		if (!copyhandle ((*sourceData).dataHandle, &htext)) /*don't let langbuildtree consume caller's text*/
			return (memFullErr);
		
	#endif
	
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
	boolean fl = false;
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
		
		#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/
			
			if (!copydatahandle ((AEDesc*) sourceData, &htext)) /*make copy for eventual getsource*/
				return (memFullErr);		
		
		#else
		
			if (!copyhandle ((*sourceData).dataHandle, &htext)) /*make copy for eventual getsource*/
				return (memFullErr);
		
		#endif
		
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


static pascal OSAError osaCoerceFromDesc (
			hdlcomponentglobals	hglobals,
			const AEDesc*		scriptData,
			long				modeFlags,
			OSAID*				resultingScriptID) {
	
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
		
		#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/
		
			{
			Handle h;
			
			copydatahandle (&desc, &h);
			
			fl = langexternalmemoryunpack (h, (hdlexternalhandle *) &val.data.externalvalue);
			
			//disposehandle (h);
			}
		#else
		
			fl = langexternalmemoryunpack (desc.dataHandle, (hdlexternalhandle *) &val.data.externalvalue);
		
		#endif
		
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


static pascal OSAError osaCoerceToDesc (
			hdlcomponentglobals	hglobals,
			OSAID				scriptID,
			DescType			desiredType,
			long				modeFlags,
			AEDesc*				result) {
	
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


static pascal OSErr coerceInsltoTEXT (
		AEDesc *desc,
		DescType totype,
		long refcon,
		AEDesc *result) {
	
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
	
	long curA5 = SetUpAppA5 ();
	
	#ifdef fldegug
	
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
	
	RestoreA5 (curA5);
	
	return (err);
	} /*coerceInsltoTEXT*/


#if !TARGET_RT_MAC_CFM
	
	#define coerceInsltoTEXTUPP ((AECoercionHandlerUPP) &coerceInsltoTEXT)
	
#else

	#if TARGET_API_MAC_CARBON == 1
	AECoercionHandlerUPP	coerceInsltoTEXTDesc;
	
	#define coerceInsltoTEXTUPP (coerceInsltoTEXTDesc);
	#else
		
	static RoutineDescriptor coerceInsltoTEXTDesc = BUILD_ROUTINE_DESCRIPTOR (uppAECoerceDescProcInfo, coerceInsltoTEXT);

	#define coerceInsltoTEXTUPP ((AECoercionHandlerUPP) &coerceInsltoTEXTDesc)

	#endif
#endif


static pascal OSErr sendrecordingevent (hdlcomponentglobals	hglobals, AEEventID id) {
	
	/*
	2.1b5 dmb: don't use sendproc for these events; they're not part of 
	script execution
	*/
	
	register hdlcomponentglobals hcg = hglobals;
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
	THz origzone;
	
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
			
			long appA5;
			
			err = AEPutParamPtr (&event, keyDirectObject, typeChar, (Ptr) bssend + 1, stringlength (bssend));
			
			osapreclientcallback (hcg);
			
			origzone = osasetclientzone (hcg);
			
			appA5 = SetUpCurA5 ();
			
			err = AESend (&event, &reply, 
				
				(AESendMode) kAENoReply + kAEDontRecord, 
				
				(AESendPriority) kAENormalPriority, (long) kNoTimeOut, nil, nil);
			
			RestoreA5 (appA5);
			
			#if TARGET_API_MAC_CARBON == 1
			LMSetApplZone(origzone);
			#else	
			SetZone (origzone);
			#endif		
			osapostclientcallback (hcg);
			
			AEDisposeDesc (&event);
			
			AEDisposeDesc (&reply);
			}
		}
	
	return (err);
	} /*sendrecordedtextevent*/


static pascal OSErr targettoprocessinfo (const TargetID *target, ProcessSerialNumber *psn, FSSpec *fs, OSType *signature) {
	#if !TARGET_API_MAC_CARBON
	ProcessInfoRec info;
	OSErr ec;
	
	
	if ((*target).location.locationKindSelector == ppcNoLocation) { /*local program*/
		
		ec = GetProcessSerialNumberFromPortName ((PPCPortPtr) &(*target).name, psn);
		
		if (ec != noErr)
			return (ec);
		
		info.processInfoLength = (long) sizeof (info);
		
		info.processName = nil;
		
		info.processAppSpec = fs;
		
		ec = GetProcessInformation (psn, &info);
		
		if (ec != noErr)
			return (ec);
		
		*signature = info.processSignature;
		}
		
	else { /*not a local program*/
	#endif

		
		clearbytes (psn, sizeof (ProcessSerialNumber));
		
		clearbytes (fs, sizeof (FSSpec)); /*null*/
		
		if ((*target).name.portKindSelector == ppcByCreatorAndType)
			*signature = (*target).name.u.port.portCreator;
		else
			BlockMove ((Ptr) (*target).name.u.portTypeStr + 1, (Ptr) signature, 4L); /*kloooge*/
	#if !TARGET_API_MAC_CARBON
	}
	#endif

	
	return (noErr);
	} /*targettoprocessinfo*/


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


static pascal OSErr handlerecordableevent (const AppleEvent *event, AppleEvent *reply, UInt32 refcon) {
	
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
	short rnum;
	Handle haete;
	
	long curA5 = SetUpAppA5 ();
	
	err = landsystem7getsenderinfo (event, &psn, &fs, &signature);
	
	if (err != noErr) {
		
		RestoreA5 (curA5);
		
		return (err);
		}
	
	osapushfastcontext (hcg);
	
	#if TARGET_API_MAC_CARBON  && TARGET_RT_MAC_CFM	
	coerceInsltoTEXTDesc = NewAECoerceDescUPP(coerceInsltoTEXT);
	AEInstallCoercionHandler (typeInsertionLoc, typeObjectSpecifier, coerceInsltoTEXTDesc, 0, true, false);
	
	#else
	AEInstallCoercionHandler (typeInsertionLoc, typeObjectSpecifier, coerceInsltoTEXTUPP, 0, true, false);
	
	#endif
	
	#if TARGET_API_MAC_CARBON == 1
	LMSetApplZone(homezone);
	#else	
	SetZone (homezone);
	#endif
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
			
			#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/
			
				datahandletostring (&desc, bs);
				
			#else
			
				texthandletostring (desc.dataHandle, bs);
			
			#endif
			
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
	
	osasetclientzone (hcg);
	
	//Code change by Timothy Paustian Friday, July 21, 2000 11:03:49 PM
	//Get rid of the UPP wer are done.
	#if TARGET_API_MAC_CARBON && TARGET_RT_MAC_CFM
	AERemoveCoercionHandler (typeInsertionLoc, typeChar, coerceInsltoTEXTDesc, false);
	DisposeAECoerceDescUPP(coerceInsltoTEXTDesc);
	#else
	AERemoveCoercionHandler (typeInsertionLoc, typeChar, coerceInsltoTEXTUPP, false);
	
	#endif
	osapopfastcontext (hcg);
	
	RestoreA5 (curA5);
	
	return (err);
	} /*handlerecordableevent*/

//Code change by Timothy Paustian Monday, June 26, 2000 9:37:56 PM
//
#if !TARGET_RT_MAC_CFM
	#define handlerecordableeventUPP handlerecordableevent

#else

	#if TARGET_API_MAC_CARBON == 1
	
	AEEventHandlerUPP handlerecordableeventDesc = nil;
	#define handlerecordableeventUPP (handlerecordableeventDesc)
	#else
		
	static RoutineDescriptor handlerecordableeventDesc = BUILD_ROUTINE_DESCRIPTOR (uppAEEventHandlerProcInfo, handlerecordableevent);

	#define handlerecordableeventUPP (&handlerecordableeventDesc)
	#endif
		
#endif

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
		#if TARGET_API_MAC_CARBON && TARGET_RT_MAC_CFM
		if (handlerecordableeventDesc == nil)
			handlerecordableeventDesc = NewAEEventHandlerUPP(handlerecordableevent);
		#endif
		
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
	
		#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/
		
			newdescwithhandle (&sourcedesc, typeChar, (**hcg).recordingstate.hrecordedtext);
			
			if (AEGetDescDataSize (&sourcedesc) > 0) {
		
		#else
		
			sourcedesc.dataHandle = (**hcg).recordingstate.hrecordedtext;
		
			if (sourcedesc.dataHandle != nil) { /*script was created by recording process*/

		#endif
		
			
			sourcedesc.descriptorType = typeChar;
			
			err = osaCompile (hcg, &sourcedesc, kOSAModeNull, &compiledScriptID);
			}
		}
	
	(**hcg).recordingstate.flrecording = false;
	
	disposehandle ((**hcg).recordingstate.hrecordedtext);
	//Code change by Timothy Paustian Friday, July 28, 2000 2:43:14 PM
	//I am assuming the osaStopRecording is called every time osaStartRecording is called.
	#if TARGET_API_MAC_CARBON && TARGET_RT_MAC_CFM
	DisposeAEEventHandlerUPP(handlerecordableeventDesc);
	handlerecordableeventDesc = nil;
	#endif
		return (err);
	} /*osaStopRecording*/


static pascal OSAError osaScriptingComponentName (
			hdlcomponentglobals	hglobals,
			AEDesc*				resultingScriptingComponentName) {
	
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


static pascal OSAError osaDisplay (
			hdlcomponentglobals	hglobals,
			OSAID				scriptValueID,
			DescType			desiredType,
			long				modeFlags,
			AEDesc*				resultingText) {
	
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


static pascal OSAError osaMakeContext (
			hdlcomponentglobals	hglobals,
			const AEDesc*		contextName,
			OSAID				parentContext,
			OSAID*				resultingContextID) {
	
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
			
			#if TARGET_API_MAC_CARBON == 1
			LMSetApplZone(homezone);
			#else	
			SetZone (homezone);
			#endif		
			fl = (*(**hg).handleverbroutine) (hverb);
			
			getverbresult (hverb, vresult);
			
			osasetclientzone (hcg);
			
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
					
					#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/
					
						{
						Handle h;
						
						copydatahandle (&desc, &h);
						
						err = numberfromhandle (h);
						
						disposehandle (h);
						} 
					
					#else
					
						err = numberfromhandle (desc.dataHandle);
					
					#endif
					
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
			OSType				selector,
			const AEDesc*		selectorData,
			DescType			desiredType,
			AEDesc*				resultingDebugInfoOrDataToSet) {
	
	OSAError err;
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
			
			#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/
			
				datahandletostring ((AEDesc*) selectorData, bs);
			
			#else
			
				texthandletostring ((*selectorData).dataHandle, bs);
			
			#endif
			
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
	#if TARGET_API_MAC_CARBON == 1
	CGrafPtr saveport;
	saveport = GetQDGlobalsThePort();
	#else
	GrafPtr saveport;
	saveport = quickdrawglobal (thePort);
	#endif
		
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
	#if TARGET_API_MAC_CARBON == 1
	thePort = GetQDGlobalsThePort();
	#else
	thePort = quickdrawglobal (thePort);
	#endif
	if (thePort != saveport)
		SetPort (saveport);
	}
	return (result);
	} /*callosafunction*/


static pascal ComponentResult osaDispatch (ComponentParameters *params, Handle storage) {
	
	ComponentResult result = noErr;
	short what = (*params).what;
	
	#if TARGET_API_MAC_CARBON
	hdlcomponentglobals	theGlobals = nil;
	//if kComponentOpenSelect then we create the globals not use them.
	if(what != kComponentOpenSelect)
		theGlobals = (hdlcomponentglobals)storage;
	#endif
	
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
					
					// 5.0.1: might not be - assert (clienta5 == (long) LMGetCurrentA5 ());
					
				#endif
				//Code change by Timothy Paustian Monday, June 26, 2000 9:39:54 PM
				//We don't need this
				#if !TARGET_API_MAC_CARBON
				SetComponentInstanceA5 ((ComponentInstance) self, selfa5);
				#endif
				
				if (newcomponentglobals (self, clienta5, &hglobals))
					SetComponentInstanceStorage ((ComponentInstance) self, (Handle) hglobals);
				else
					result = memFullErr;
				#if !TARGET_API_MAC_CARBON
				RestoreA5 (clienta5);
				#endif
				
				
				break;
				}
			
			case kComponentCloseSelect:
				#if TARGET_API_MAC_CARBON && TARGET_RT_MAC_CFM
				result = CallComponentFunctionWithStorage (storage, params, (**theGlobals).cmpcloseUPP);								
				#else
				result = CallComponentFunctionWithStorage (storage, params, cmpcloseUPP);
				#endif
				break;
			
			case kComponentCanDoSelect:
				#if TARGET_API_MAC_CARBON && TARGET_RT_MAC_CFM
				result = CallComponentFunction (params, (**theGlobals).cmpcandoUPP);			
				#else
				result = CallComponentFunction(params, cmpcandoUPP);
				#endif
				break;
			
			case kComponentVersionSelect:
				#if TARGET_API_MAC_CARBON && TARGET_RT_MAC_CFM
				result = CallComponentFunction(params, (**theGlobals).cmpversionUPP);			
				#else
				result = CallComponentFunction(params, cmpversionUPP);
				#endif
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
				#if TARGET_API_MAC_CARBON && TARGET_RT_MAC_CFM
				func = (**theGlobals).osaLoadUPP;
				#else
				func = osaLoadUPP;
				#endif
				break;
			
			case kOSASelectStore:
				#if TARGET_API_MAC_CARBON && TARGET_RT_MAC_CFM
				func = (**theGlobals).osaStoreUPP;
				#else
				func = osaStoreUPP;
				#endif
				break;
			
			case kOSASelectExecute:
				#if TARGET_API_MAC_CARBON && TARGET_RT_MAC_CFM
				func = (**theGlobals).osaExecuteUPP;
				#else
				func = osaExecuteUPP;
				#endif
				break;
			
			case kOSASelectDisplay:
				#if TARGET_API_MAC_CARBON && TARGET_RT_MAC_CFM
				func = (**theGlobals).osaDisplayUPP;
				#else
				func = osaDisplayUPP;
				#endif
				break;
			
			case kOSASelectScriptError:
				#if TARGET_API_MAC_CARBON && TARGET_RT_MAC_CFM
				func = (**theGlobals).osaScriptErrorUPP;
				#else
				func = osaScriptErrorUPP;
				#endif
				break;
			
			case kOSASelectDispose:
				#if TARGET_API_MAC_CARBON && TARGET_RT_MAC_CFM
				func = (**theGlobals).osaDisposeUPP;
				#else
				func = osaDisposeUPP;
				#endif
				break;
			
			case kOSASelectSetScriptInfo:
				#if TARGET_API_MAC_CARBON && TARGET_RT_MAC_CFM
				func = (**theGlobals).osaSetScriptInfoUPP;
				#else
				func = osaSetScriptInfoUPP;
				#endif
				break;
			
			case kOSASelectGetScriptInfo:
				#if TARGET_API_MAC_CARBON && TARGET_RT_MAC_CFM
				func = (**theGlobals).osaGetScriptInfoUPP;
				#else
				func = osaGetScriptInfoUPP;
				#endif
				break;
			
			case kOSASelectCompile:
				#if TARGET_API_MAC_CARBON && TARGET_RT_MAC_CFM
				func = (**theGlobals).osaCompileUPP;
				#else
				func = osaCompileUPP;
				#endif
				break;
			
			case kOSASelectGetSource:
				#if TARGET_API_MAC_CARBON && TARGET_RT_MAC_CFM
				func = (**theGlobals).osaGetSourceUPP;
				#else
				func = osaGetSourceUPP;
				#endif
				break;
			
			case kOSASelectCoerceFromDesc:
				#if TARGET_API_MAC_CARBON && TARGET_RT_MAC_CFM
				func = (**theGlobals).osaCoerceFromDescUPP;
				#else
				func = osaCoerceFromDescUPP;
				#endif
				break;
			
			case kOSASelectCoerceToDesc:
				#if TARGET_API_MAC_CARBON && TARGET_RT_MAC_CFM
				func = (**theGlobals).osaCoerceToDescUPP;
				#else
				func = osaCoerceToDescUPP;
				#endif
				break;
			
			case kOSASelectStartRecording:
				#if TARGET_API_MAC_CARBON && TARGET_RT_MAC_CFM
				func = (**theGlobals).osaStartRecordingUPP;
				#else
				func = osaStartRecordingUPP;
				#endif
				break;
			
			case kOSASelectStopRecording:
				#if TARGET_API_MAC_CARBON && TARGET_RT_MAC_CFM
				func = (**theGlobals).osaStopRecordingUPP;
				#else
				func = osaStopRecordingUPP;
				#endif
				break;
			
			case kOSASelectScriptingComponentName:
				#if TARGET_API_MAC_CARBON && TARGET_RT_MAC_CFM
				func = (**theGlobals).osaScriptingComponentNameUPP;
				#else
				func = osaScriptingComponentNameUPP;
				#endif
				break;
			
			case kOSASelectLoadExecute:
				#if TARGET_API_MAC_CARBON && TARGET_RT_MAC_CFM
				func = (**theGlobals).osaLoadExecuteUPP;
				#else
				func = osaLoadExecuteUPP;
				#endif
				break;
			
			case kOSASelectCompileExecute:
				#if TARGET_API_MAC_CARBON && TARGET_RT_MAC_CFM
				func = (**theGlobals).osaCompileExecuteUPP;
				#else
				func = osaCompileExecuteUPP;
				#endif
				break;
			
			case kOSASelectDoScript:
				#if TARGET_API_MAC_CARBON && TARGET_RT_MAC_CFM
				func = (**theGlobals).osaDoScriptUPP;
				#else
				func = osaDoScriptUPP;
				#endif
				break;
			
			case kOSASelectMakeContext:
				#if TARGET_API_MAC_CARBON && TARGET_RT_MAC_CFM
				func = (**theGlobals).osaMakeContextUPP;
				#else
				func = osaMakeContextUPP;
				#endif
				break;
			
			case kOSASelectSetResumeDispatchProc:
				#if TARGET_API_MAC_CARBON && TARGET_RT_MAC_CFM
				func = (**theGlobals).osaSetResumeDispatchProcUPP;
				#else
				func = osaSetResumeDispatchProcUPP;
				#endif
				break;
			
			case kOSASelectGetResumeDispatchProc:
				#if TARGET_API_MAC_CARBON && TARGET_RT_MAC_CFM
				func = (**theGlobals).osaGetResumeDispatchProcUPP;
				#else
				func = osaGetResumeDispatchProcUPP;
				#endif
				break;
			
			case kOSASelectExecuteEvent:
				#if TARGET_API_MAC_CARBON && TARGET_RT_MAC_CFM
				func = (**theGlobals).osaExecuteEventUPP;
				#else
				func = osaExecuteEventUPP;
				#endif
				break;
			
			case kOSASelectDoEvent:
				#if TARGET_API_MAC_CARBON && TARGET_RT_MAC_CFM
				func = (**theGlobals).osaDoEventUPP;
				#else
				func = osaDoEventUPP;
				#endif
				break;
			
			case kOSASelectSetActiveProc:
				#if TARGET_API_MAC_CARBON && TARGET_RT_MAC_CFM
				func = (**theGlobals).osaSetActiveProcUPP;
				#else
				func = osaSetActiveProcUPP;
				#endif
				break;
			
			case kOSASelectSetDebugProc:
				#if TARGET_API_MAC_CARBON && TARGET_RT_MAC_CFM
				func = (**theGlobals).osaSetDebugProcUPP;
				#else
				func = osaSetDebugProcUPP;
				#endif
				break;
			
			case kOSASelectDebug:
				#if TARGET_API_MAC_CARBON && TARGET_RT_MAC_CFM
				func = (**theGlobals).osaDebugUPP;
				#else
				func = osaDebugUPP;
				#endif
				break;
			
			case kOSASelectSetSendProc:
				#if TARGET_API_MAC_CARBON && TARGET_RT_MAC_CFM
				func = (**theGlobals).osaSetSendProcUPP;
				#else
				func = osaSetSendProcUPP;
				#endif
				break;
			
			case kOSASelectGetSendProc:
				#if TARGET_API_MAC_CARBON && TARGET_RT_MAC_CFM
				func = (**theGlobals).osaGetSendProcUPP;
				#else
				func = osaGetSendProcUPP;
				#endif
				break;
			
			case kOSASelectSetCreateProc:
				#if TARGET_API_MAC_CARBON && TARGET_RT_MAC_CFM
				func = (**theGlobals).osaSetCreateProcUPP;
				#else
				func = osaSetCreateProcUPP;
				#endif
				break;
			
			case kOSASelectGetCreateProc:
				#if TARGET_API_MAC_CARBON && TARGET_RT_MAC_CFM
				func = (**theGlobals).osaGetCreateProcUPP;
				#else
				func = osaGetCreateProcUPP;
				#endif
				break;
			
			#if 0
			
			case kOSASelectSetDefaultTarget:
				#if TARGET_API_MAC_CARBON && TARGET_RT_MAC_CFM
				func = (**theGlobals).osaSetDefaultTargetUPP;
				#else
				func = osaSetDefaultTargetUPP;
				#endif
				break;
			
			#endif
			
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
		
		#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/
		
			{
			Handle h;
			
			copydatahandle (&errordesc, &h);
			
			err = numberfromhandle (h);
			
			disposehandle (h);
			}
		
		#else
		
			err = numberfromhandle (errordesc.dataHandle);
		
		#endif
		
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
		
		#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/
		
			datahandletostring (&errordesc, bserror);
		
		#else
		
			texthandletostring (errordesc.dataHandle, bserror);
		
		#endif
		
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
	long appA5;
	
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
	
	appA5 = SetUpCurA5 (); /*3.0a*/
	
	instance = OpenDefaultComponent (kOSAComponentType, type);
	
	RestoreA5 (appA5);
	
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
	
	#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/
	
		{
		Handle h;
		
		copydatahandle (scriptdata, &h);
		
		if (OSAGetStorageType (h, &idserver) != noErr)
			idserver = (*scriptdata).descriptorType;
			
		//disposehandle (h);
		}
	
	#else
	
		if (OSAGetStorageType ((*scriptdata).dataHandle, &idserver) != noErr)
			idserver = (*scriptdata).descriptorType;
	
	#endif
	
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
	
	scriptsource.descriptorType = typeChar;
	
	#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/
	
		newdescwithhandle (&scriptsource, typeChar, htext);
	
	#else
	
		scriptsource.dataHandle = htext;
	
	#endif
	
	id = kOSANullScript;
	
	err = OSACompile (comp, &scriptsource, kOSAModeCompileIntoContext, &id);
	
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
	
	#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/
	
		{
		Handle h;
		
		copydatahandle (&scriptdata, &h);
		
		return (setbinaryvalue (h, scriptdata.descriptorType, vcode));
		}
	
	#else
	
		return (setbinaryvalue (scriptdata.dataHandle, scriptdata.descriptorType, vcode));
	#endif
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
	
	#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/
		
		{
		Handle h;
		
		copydatahandle (&scriptsource, &h);
		
		return (setheapvalue (h, stringvaluetype, vsource));
		}
	
	#else
	
		return (setheapvalue (scriptsource.dataHandle, stringvaluetype, vsource));
		
	#endif
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


/*
boolean isosascriptvalue (const tyvaluerecord *val) {
	
	/*
	enforce a strong definition of an osascriptvalue: a binary whose type
	is 'scpt', that might be confused with a packed Frontier script
	%/
	
	Handle x;
	OSErr err;
	OSType subtype;
	
	if ((*val).valuetype != binaryvaluetype)
		return (false);
	
	x = (*val).data.binaryvalue;
	
	if (getbinarytypeid (x) != typeOSAGenericStorage) /*it might be; can't tell%/
		return (false);
	
	err = OSAGetStorageType (x, &subtype);
	
	return (err == noErr);
	} /*isosascriptvalue*/


static pascal OSErr osaclientactive (long refcon) {
	
	/*
	while executing an osa script, we want to check for cmd-period, 
	and allow other threads to run. this does part of the job.
	*/
	
	OSErr err = noErr;
	
	long curA5 = SetUpAppA5 ();
	
	if (!langbackgroundtask (false) || languserescaped (false))
		err = userCanceledErr;
	
	RestoreA5 (curA5);
	
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



//Code change by Timothy Paustian Thursday, July 20, 2000 2:13:15 PM
//Moved UPPs and prototypes to above osadispatch with the rest.


static pascal OSErr osaclientsend (const AppleEvent *event, AppleEvent *reply,
		AESendMode sendmode, AESendPriority priority, long timeout,
        AEIdleUPP idleproc, AEFilterUPP filterproc, long refcon) {
			
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
	
	long curA5 = SetUpAppA5 ();
	
	hlg = landgetglobals ();
	
	err = (*(**hlg).eventsendcallback) (event, reply, sendmode, priority, timeout, osaclientidleUPP, filterproc);
	/*
	err = AESend (event, reply, sendmode, priority, timeout, osaclientidleUPP, filterproc);
	*/
	
	if (err == noErr) {
		
		if (!langbackgroundtask (false) || languserescaped (false))
			err = userCanceledErr;
		}
	
	RestoreA5 (curA5);
	
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
	
	#if !TARGET_API_MAC_CARBON
	homeresmap = LMGetTopMapHndl ();
	#endif

	return (true);
	} /*osagethomeresfile*/


Component osaregistercomponent (OSType type, long flags, ComponentRoutine dispatch, short idname, short iddescription) {
	
	/*
	2.1b11 dmb: new routine consolodates code for registering all 
	Frontier components.  strings are now in a single STR# resource
	*/
	
	ComponentDescription desc;
	Handle hname, hdescription, hicon;
	Component comp;
	short global = registerComponentGlobal;
	bigstring bsname;
	
	desc.componentType = type;
	
	desc.componentSubType = 'LAND';
	
	desc.componentManufacturer = 'LAND';
	
	desc.componentFlags = flags;
	
	desc.componentFlagsMask = 0;
	
	hname = getcomponentstringhandle (idname);
	
	texthandletostring (hname, bsname);
	
	hdescription = getcomponentstringhandle (iddescription);
	
	hicon = GetIcon (129);
	
	#if TARGET_API_MAC_CARBON == 1

		comp = RegisterComponent (&desc, NewComponentRoutineUPP (dispatch), global, hname, hdescription, hicon);
	
	#else
	
		comp = RegisterComponent (&desc, NewComponentRoutineProc (dispatch), global, hname, hdescription, hicon);
	
	#endif
	
	disposehandle (hname);
	
	disposehandle (hdescription);
	
	#if !TARGET_API_MAC_CARBON
	if (comp != nil)
		SetComponentRefcon (comp, (long) LMGetCurrentA5 ());
	#endif

	return (comp);
	} /*osaregistercomponent*/


static boolean initosacomponent (void) {
	
	/*
	register the scripting component.
	
	all attempts to use a resource-based thng failed miserably. this might be 
	a bug in the system, but the only reason to was attempted was to get use 
	of the OpenComponentResFile call, which we do manually.
	*/
	
	long flags = 0;
	
	#if !TARGET_API_MAC_CARBON
	RememberA5 ();
	#endif
	
	#if TARGET_API_MAC_CARBON == 1
	homezone = LMGetApplZone();
	#else	
	homezone = GetZone ();
	#endif

	osagethomeresfile();
		//Code change by Timothy Paustian Friday, July 21, 2000 11:18:39 PM
	//create all the osa UPPs
	#if TARGET_API_MAC_CARBON && TARGET_RT_MAC_CFM
	
	osaclientactiveDesc = NewOSAActiveUPP(osaclientactive);
	osaclientsendDesc = NewOSASendUPP(osaclientsend);
	osaclientidleDesc = NewAEIdleUPP(osaclientidleproc);
	osadefaultactiveDesc = NewOSAActiveUPP(osadefaultactiveproc);
	osadefaultcreateDesc = NewOSACreateAppleEventUPP(osadefaultcreate);
	osadefaultsendDesc = NewOSASendUPP(osadefaultsend);
	
	#endif
		
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
	short idstring;
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
			idstring = specificclientstring;
		
		else {
			numbertostring (ctclients, bs);
			
			idstring = multipleclientsstring;
			}
		
		if (getrecordingstring (idstring, bsprompt)) {
			
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
		
		#if flruntime
		
		if (iscurrentapplication (psn)) /*runtime is it's own menusharing client*/
			continue;
		
		#endif
		
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
	#if TARGET_API_MAC_CARBON && TARGET_RT_MAC_CFM
	DisposeAECoerceDescUPP(coerceTypetoObjDesc);
	
	//the next three are used to send apple events

	DisposeOSAActiveUPP(osaclientactiveDesc);
	DisposeOSASendUPP(osaclientsendDesc);
	DisposeAEIdleUPP(osaclientidleDesc);
	
	DisposeOSAActiveUPP(osadefaultactiveDesc);
	DisposeOSACreateAppleEventUPP(osadefaultcreateDesc);
	DisposeOSASendUPP(osadefaultsendDesc);
	#endif
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

	initosacomponent ();
	
	#ifndef PIKE
	#if !TARGET_API_MAC_CARBON
	//no menu sharing in carbon. It won't work with the memory
	//model of OS X.
	initmenusharingcomponent ();
	#endif
	#endif
	
	#if !flruntime
	
		initwindowsharingcomponent ();
	
	#endif
	
	#ifdef dropletcomponent
	
		initdropletcomponent ();
	
	#endif
	
	RegisterComponentResourceFile (filegetapplicationrnum (), true); /*2.1b4*/
	
	#if TARGET_API_MAC_CARBON && TARGET_RT_MAC_CFM
	coerceTypetoObjDesc = NewAECoerceDescUPP(coerceTypetoObj);
	#endif
	AEInstallCoercionHandler (typeType, typeObjectSpecifier, coerceTypetoObjUPP, 0, true, true);
	
	if (!initosaservers ())
		return (false);
	
	return (newclearhandle (longsizeof (tyclientlist), (Handle *) &hclientlist));
	} /*osacomponentstart*/




#else

/*copyright 1992-1993, UserLand Software, Inc.*/

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



#define kOSAScriptIsBeingEdited			'edit'
	// Selector returns boolean.

#define kOSAScriptIsBeingRecorded		'recd'
	// Selector returns boolean.


typedef struct tyservercomponent {
	
	struct tyservercomponent **hnext;
	
	ComponentInstance instance;
	
	ProcessSerialNumber clientpsn; /*3.015*/
	
	OSType type;
	} tyservercomponent, *ptrservercomponent, **hdlservercomponent;


typedef struct tyclientlist {
	
	hdlcomponentglobals hfirst;
	
	} tyclientlist, **hdlclientlist;


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


typedef struct tystylerecord {
	
	short ctstyles;
	
	ScrpSTElement styles [1];
	} tystylerecord;


static THz homezone;

static Handle homeresmap;

static short homeresfile;

static ProcessSerialNumber homepsn;


hdlcomponentglobals osaglobals = nil;

static osacoercionhandlerinstalled = 0;

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
	
	#if 0 /*def fldebug*/
	
	if (systemtable != nil) {
		
		hashtabledelete (systemtable, (ptrstring) 0x910);
		}
	else
	
	#endif
	
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
	if (EventAvail (keyDownMask, &ev)) {
		
		if (iscmdperiodevent (ev.message, ev.what, ev.modifiers))
			return (userCanceledErr);
		}
	
	return (noErr);
	} /*osadefaultactiveproc*/


static pascal OSErr osadefaultcreate (AEEventClass class, AEEventID id,
                    const AEAddressDesc *target, short returnID,
                    long transactionID, AppleEvent *result, long refcon) {
	
	return (AECreateAppleEvent(class, id, target, returnID, transactionID, result));
	} /*osadefaultcreate*/


static pascal OSErr osadefaultsend (const AppleEvent *event, AppleEvent *reply,
		AESendMode sendmode, AESendPriority priority, long timeout,
        AEIdleUPP idleproc, AEFilterUPP filterproc, UInt32 refcon) {
	
	return (AESend (event, reply, sendmode, priority, timeout, idleproc, filterproc));
	} /*osadefaultsend*/

#if !TARGET_RT_MAC_CFM || TARGET_API_MAC_CARBON

	#define osadefaultactiveUPP ((OSAActiveProcPtr) &osadefaultactiveproc)
	
	#define osadefaultcreateUPP ((OSACreateAppleEventProcPtr) &osadefaultcreate)
	
	#define osadefaultsendUPP ((OSASendProcPtr) &osadefaultsend)
	
#else

	static RoutineDescriptor osadefaultactiveDesc = BUILD_ROUTINE_DESCRIPTOR (uppOSAActiveProcInfo, osadefaultactiveproc);
	
	static RoutineDescriptor osadefaultcreateDesc = BUILD_ROUTINE_DESCRIPTOR (uppOSACreateAppleEventProcInfo, osadefaultcreate);
	
	static RoutineDescriptor osadefaultsendDesc = BUILD_ROUTINE_DESCRIPTOR (uppOSASendProcInfo, osadefaultsend);

	#define osadefaultactiveUPP (&osadefaultactiveDesc)
	
	#define osadefaultcreateUPP (&osadefaultcreateDesc)
	
	#define osadefaultsendUPP (&osadefaultsendDesc)

#endif 

// forward declarations
static boolean osabackgroundtask (boolean);
static boolean osadebugger (hdltreenode);
static boolean osapartialeventloop (short);


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
	
	#if TARGET_API_MAC_CARBON == 1
	(**hcg).clientzone = LMGetApplZone();
	#else	
	(**hcg).clientzone = GetZone ();
	#endif
		
	(**hcg).clientid = appid;
	
	(**hcg).clientpsn = psn;
	
	#if 0 /*def fldebug*/
	
	if (!tablenewsystemtable (systemtable, (ptrstring) 0x910, &storagetable))
	
	#else
	
	#if TARGET_API_MAC_CARBON == 1
	LMSetApplZone(homezone);
	#else	
	SetZone (homezone);
	#endif
		
	fl = newhashtable (&storagetable);
	
	#if TARGET_API_MAC_CARBON == 1
	LMSetApplZone((**hcg).clientzone);
	#else	
	SetZone ((**hcg).clientzone);
	#endif
	
	if (!fl)
	
	#endif
		{
		disposehandle ((Handle) hcg);
		
		return (false);
		}
	
	(**hcg).storagetable = storagetable;
	
	(**hcg).activeproc = osadefaultactiveUPP;
	
	(**hcg).createproc = osadefaultcreateUPP;
	
	(**hcg).sendproc = osadefaultsendUPP;
		
	/*
	if (!newclearhandle (longsizeof (tyMSglobals), (Handle *) &hMSglobals)) {
		
		disposecomponentglobals (hcg);
		
		return (false);
		}
	
	(**hcg).hMSglobals = hMSglobals;
	*/
	
	if (!newthreadglobals (&hthreadglobals)) {
		
		disposecomponentglobals (hcg);
		
		return (false);
		}
	
	htg = hthreadglobals;
	
	(**hcg).clientthreadglobals = htg;	// 4.1b3 dmb: use local
	
	(**htg).htable = storagetable;
	
	(**htg).applicationid = appid;
	
	#if version42orgreater
		// 2/28/97 dmb: see up langcallbacks here so they'll always be in effect
		
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
	
	AEDesc list, rec, desc;
	long n;
	OSErr err;
	
	list = (**osaglobals).errordesc;
	
	AEDisposeDesc (&list);
	
	n = osageterror ();
	
	err = AECreateList (nil, 0, true, &list);
	
	if (err != noErr) {
		
		#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/
		
			newdescnull (&list, typeNull);
		
		#else
		
			list.descriptorType = typeNull;
		
			list.dataHandle = nil;
			
		#endif
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
	
	exit:
	
	(**osaglobals).errordesc = list;
	
	return (false); /*consume the error*/
	} /*osaerrormessage*/

#if 1

static pascal OSErr coerceTEXTtoSTXT (
		DescType fromtype,
		Ptr pdata,
		long size,
		DescType totype,
		long refcon,
		AEDesc *result) {
	
	/*
	2.1b2 dmb: don't use clearbytes so we don't have to set up a5
	*/
	
	tystylerecord stylerecord;
	OSErr err;
	AEDesc list;
	register ScrpSTElement *pstyle;
	
	#ifdef fldegug
	
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

#else

static pascal OSErr coerceTEXTtoSTXT (
		AEDesc *fromdesc,
		AEDesc *result) {
	
	/*
	2.1b2 dmb: no longer installed as a coercion handler, we call it directly
	*/
	
	tystylerecord stylerecord;
	OSErr err;
	AEDesc list;
	register ScrpSTElement *pstyle;
	
	if ((*fromdesc).descriptorType != typeChar)
		return (errAECoercionFail);
	
	stylerecord.ctstyles = 1;
	
	pstyle = &stylerecord.styles [0];
	
	(*pstyle).scrpStartChar = 0;
	
	(*pstyle).scrpHeight = 16;
	
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
			err = AEPutKeyDesc (&list, 'ktxt', fromdesc);
		
		if (err == noErr)
			err = AECoerceDesc (&list, typeStyledText, result);
		
		AEDisposeDesc (&list);
		}
	
	return (err);
	} /*coerceTEXTtoSTXT*/

#endif


static pascal OSErr coerceTypetoObj (
		AEDesc *desc,
		DescType totype,
		long refcon,
		AEDesc *result) {
	
	/*
	2.1b1 dmb: if Frontier passes a string4 identifier where an object specifier 
	is required, this coercion handler will take care of it
	*/
	
	AEDesc containerdesc;
	OSErr err;
	
	long curA5 = SetUpAppA5 ();
	
	#ifdef fldebug
	
	if ((*desc).descriptorType != typeType)
		DebugStr ("\punexpected coercion");
	
	#endif
	
	#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/
		
		newdescnull (&containerdesc, typeNull);
	
	#else
	
		containerdesc.descriptorType = typeNull;
	
		containerdesc.dataHandle = nil;
		
	#endif
	
	err = CreateObjSpecifier (cProperty, &containerdesc, formPropertyID, desc, false, result);
	
	RestoreA5 (curA5);
	
	return (err);
	} /*coerceTypetoObj*/

//Code change by Timothy Paustian Friday, June 16, 2000 1:38:13 PM
//Changed to Opaque call for Carbon
#if TARGET_API_MAC_CARBON == 1
	#define coerceTEXTtoSTXTUPP ((AECoercionHandlerUPP) &coerceTEXTtoSTXT)
	
	#define coerceTypetoObjUPP ((AECoercionHandlerUPP) &coerceTypetoObj)
#else

#if !GENERATINGCFM
	
	#define coerceTEXTtoSTXTUPP ((AECoercionHandlerUPP) &coerceTEXTtoSTXT)
	
	#define coerceTypetoObjUPP ((AECoercionHandlerUPP) &coerceTypetoObj)
	
#else

	static RoutineDescriptor coerceTEXTtoSTXTDesc = BUILD_ROUTINE_DESCRIPTOR (uppAECoercePtrProcInfo, coerceTEXTtoSTXT);
	
	static RoutineDescriptor coerceTypetoObjDesc = BUILD_ROUTINE_DESCRIPTOR (uppAECoerceDescProcInfo, coerceTypetoObj);

	#define coerceTEXTtoSTXTUPP ((AECoercionHandlerUPP) &coerceTEXTtoSTXTDesc)
	
	#define coerceTypetoObjUPP ((AECoercionHandlerUPP) &coerceTypetoObjDesc)

#endif
#endif

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
		
		#if !flruntime
		
		(**htg).hccglobals = nil; /*want to leave them untouched*/
		
		#endif
		
		swapinthreadglobals (htg);
		}
	
	if (++osacoercionhandlerinstalled == 1)
		AEInstallCoercionHandler (typeChar, typeStyledText, coerceTEXTtoSTXTUPP, 0, false, false);
	
	if (!(**hcg).isHomeProcess) {
	
		(**hcg).clientresfile = CurResFile ();
		
		//Code change by Timothy Paustian Friday, June 16, 2000 2:08:20 PM
		//Changed to Opaque call for Carbon
		//This is not supported in carbon. You cannot use res files in OS X
		//We may have some serious rewriting to do for this.
		#if !TARGET_API_MAC_CARBON
		(**hcg).clientresmaphandle = LMGetTopMapHndl ();
		
		LMSetTopMapHndl (homeresmap);
		#endif

		
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
		AERemoveCoercionHandler (typeChar, typeStyledText, coerceTEXTtoSTXTUPP, false);
	
	if (!(**hcg).isHomeProcess) {
		#if !TARGET_API_MAC_CARBON
		LMSetTopMapHndl ((**hcg).clientresmaphandle);
		#endif

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


#if !TARGET_API_MAC_CARBON

static UniversalProcPtr installpatch (short trapnum, GNEUPP patch) {
	
	UniversalProcPtr origtrap;
	
	origtrap = NGetTrapAddress (trapnum, ToolTrap);
	
	NSetTrapAddress ((UniversalProcPtr) patch, trapnum, ToolTrap);

	return (origtrap);
	} /*installpatch*/


static void removepatch (short trapnum, GNEUPP origtrap) {
	
	NSetTrapAddress ((UniversalProcPtr) origtrap, trapnum, ToolTrap);

	} /*removepatch*/

#endif /*!TARGET_API_MAC_CARBON*/


static pascal Boolean osagetnextevent (short, EventRecord *);

//Code change by Timothy Paustian Monday, June 26, 2000 4:11:25 PM
//
#if !TARGET_RT_MAC_CFM || TARGET_API_MAC_CARBON

	#define osagetnexteventUPP (&osagetnextevent)

#else

	static RoutineDescriptor osagetnexteventDesc = BUILD_ROUTINE_DESCRIPTOR (uppGNEProcInfo, osagetnextevent);
	
	#define osagetnexteventUPP (&osagetnexteventDesc)
	
#endif


GNEUPP osainstallpatch (hdlcomponentglobals hglobals) {
	
	/*
	3.0a dmb: return the original value of getnexteventproc so it can 
	be restored in osaremovepath. this allows patching pairs to be 
	nested, so calls to handlerunscript can be nested.
	*/
	
	register hdlcomponentglobals hcg = hglobals;
	GNEUPP origproc;
	UniversalProcPtr origtrap;
	
	if ((**hcg).isHomeProcess)
		origproc = nil;
		
	else {
	
		origtrap = installpatch (_GetNextEvent, osagetnexteventUPP);
		
		origproc = (**hcg).getnexteventproc;
		
		(**hcg).getnexteventproc = (GNEUPP) origtrap;
		}
	
	return (origproc);
	} /*osainstallpatch*/


void osaremovepatch (hdlcomponentglobals hglobals, GNEUPP origproc) {

	register hdlcomponentglobals hcg = hglobals;
	
	if ((**hcg).isHomeProcess)
		;	
	else {
	
		assert ((**hcg).getnexteventproc != nil);
		
		removepatch (_GetNextEvent, (**hcg).getnexteventproc);
		
		(**hcg).getnexteventproc = origproc;
		}
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


THz osasethomezone (hdlcomponentglobals hglobals) {
	
	/*
	2.1b13 dmb: added this to balance the setclientzone call, and to 
	allow us to keep the homezone global private.
	
	in the future, this routine might reset the clientzone in hgobals. 
	now's not the time to fiddle with this, though.
	*/
	
	/*
	register hdlcomponentglobals hcg = hglobals;
	*/
	register THz origzone;
	//Code change by Timothy Paustian Monday, June 26, 2000 9:37:30 PM
	//
	#if TARGET_API_MAC_CARBON == 1
	origzone = LMGetApplZone();
	LMSetApplZone(homezone);
	#else
		
	origzone = GetZone ();
	
	SetZone (homezone);
	#endif
		
	return (origzone);
	} /*osasethomezone*/


THz osasetclientzone (hdlcomponentglobals hglobals) {
	
	/*
	2.1b11 dmb: snippet for setting the clients zone.  we return 
	the current zone because most callers will want to restore it
	*/
	
	register hdlcomponentglobals hcg = hglobals;
	register THz origzone;
	#if TARGET_API_MAC_CARBON == 1
	origzone = LMGetApplZone();
	LMSetApplZone((**hcg).clientzone);
	#else
		
 	origzone = GetZone ();
	
	SetZone ((**hcg).clientzone);
	#endif
		
	return (origzone);
	} /*osasetclientzone*/


static pascal Boolean osagetnextevent (short mask, EventRecord *event) {
	
	/*
	if any toolbox call that we use calls GetNextEvent or EventAvail, we 
	need to take many of the same precautions as we do for background 
	tasking. in particular, we need to make sure that the Frontier environment 
	is restored, since it may get swapped in by the process manager
	*/
	
	register hdlcomponentglobals hcg;
	register GNEUPP getnexteventproc;
	register THz origzone;
	Boolean fl;
	
	long curA5 = SetUpAppA5 ();
	
	hcg = osaglobals;
	
	getnexteventproc = (**hcg).getnexteventproc;
	
	osapopfastcontext (hcg);
	
	origzone = osasetclientzone (hcg);
	
	osaremovepatch (hcg, nil); /*unpatch*/
	
	RestoreA5 (curA5);
	
	fl = CallGNEProc (getnexteventproc, mask, event);

	curA5 = SetUpAppA5 ();
	
	osainstallpatch (hcg); /*repatch*/
	#if TARGET_API_MAC_CARBON == 1
	LMSetApplZone(origzone);
	#else
	SetZone (origzone);
	#endif
		
	osapushfastcontext (hcg);
	
	RestoreA5 (curA5);
	
	return (fl);
	} /*osagetnextevent*/


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
	register THz origzone;
	OSErr err;
	long clienta5, appA5;
	
	osapopfastcontext (hcg);
	
	origzone = osasetclientzone (hcg);
	
	clienta5 = (**hcg).clienta5;
	
	#ifdef THINK_C
	
		asm {
				move.l	a5,-(a7)
				move.l	clienta5,a5
			}
		
		err = (*(**hcg).createproc) (class, id, target, returnID, transactionID, result, (**hcg).createprocrefcon);
	
	#else
	
		appA5 = SetUpCurA5 ();	// SetUpThisA5 (clienta5);	// 4.0b5: was: SetUpCurA5 ();
		
		err = CallOSACreateAppleEventProc ((**hcg).createproc, class, id, target, returnID, transactionID, result, (**hcg).createprocrefcon);
	
	#endif
	
	RestoreA5 (appA5);
	#if TARGET_API_MAC_CARBON == 1
	LMSetApplZone(origzone);
	#else
	SetZone (origzone);
	#endif
		
	osapushfastcontext (hcg);
	
	return (err);
	} /*osacreateevent*/


static pascal OSErr osasendevent (const AppleEvent *event, AppleEvent *reply,
		AESendMode mode, AESendPriority priority, long timeout,
        AEIdleUPP idleproc, AEFilterUPP filterproc ) {
	
	/*
	2/16/93 dmb: in case the event is to be direct dispatched correctly, we need 
	to set up the client's A5 world
	
	2.1b5 dmb: watch out for nil sendProc -- the AS script editor sets this.
	*/
	
	register hdlcomponentglobals hcg = osaglobals;
	OSErr err;
	long clienta5, appA5;
	register GNEUPP getnexteventproc;
	register THz origzone;
	OSASendUPP sendproc;
	
	sendproc = (**hcg).sendproc;
	
	getnexteventproc = (**hcg).getnexteventproc;
	
	osapopfastcontext (hcg);
	
	origzone = osasetclientzone (hcg);
	
	if (getnexteventproc != nil)
		osaremovepatch (hcg, nil); /*unpatch*/
	
	clienta5 = (**hcg).clienta5;
	
	#ifdef THINK_C
	
		asm {
				move.l	a5,-(a7)
				move.l	clienta5,a5
			}
		
		err = (*sendproc) (event, reply, mode, priority, timeout, nil, nil, (**hcg).sendprocrefcon);
	
	#else
	
		appA5 = SetUpCurA5 ();	// SetUpThisA5 (clienta5);	// 4.0b5: was: SetUpCurA5 ();
		
		err = CallOSASendProc (sendproc, event, reply, mode, priority, timeout, nil, nil, (**hcg).sendprocrefcon);
	
	#endif
	
	RestoreA5 (appA5);
	
	if (getnexteventproc != nil)
		osainstallpatch (hcg); /*repatch*/
	
	#if TARGET_API_MAC_CARBON == 1
	LMSetApplZone(origzone);
	#else	
	SetZone (origzone);
	#endif
		
	osapushfastcontext (hcg);
	
	return (err);
	} /*osasendevent*/


static boolean osabackgroundtask (boolean flresting) {
	
	/*
	very carefully call back to the client.
	*/
	
	register hdlcomponentglobals hcg = osaglobals;
	THz origzone;
	OSErr err;
	long clienta5, appA5;
	
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
	
	origzone = osasetclientzone (hcg);
	
	osaremovepatch (hcg, nil); /*unpatch*/
	
	clienta5 = (**hcg).clienta5;
	
	#ifdef THINK_C
	
		asm {
				move.l	a5,-(a7)
				move.l	clienta5,a5
			}
		
		err = (*(**hcg).activeproc) ((**hcg).activeprocrefcon);
	
	#else
	
		appA5 = SetUpThisA5 (clienta5);	// 4.0b5: was: SetUpCurA5 ();
		
		err = CallOSAActiveProc ((**hcg).activeproc, (**hcg).activeprocrefcon);
	
	#endif
	
	RestoreA5 (appA5);
	
	osainstallpatch (hcg); /*repatch*/
	
	#if TARGET_API_MAC_CARBON == 1
	LMSetApplZone(origzone);
	#else	
	SetZone (origzone);
	#endif
		
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
	THz origzone;
	OSErr err;
	long clienta5, appA5;
	
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
	
	origzone = osasetclientzone (hcg);
	
	osaremovepatch (hcg, nil); /*unpatch*/
	
	clienta5 = (**hcg).clienta5;
	
	#ifdef THINK_C
	
		asm {
				move.l	a5,-(a7)
				move.l	clienta5,a5
			}
		
		err = (*(**hcg).debugproc) ((**hcg).debugprocrefcon);
	
	#else
	
		appA5 = SetUpCurA5 ();	// SetUpThisA5 (clienta5);	// 4.0b5: was: SetUpCurA5 ();
		
		err = CallOSADebugProc ((**hcg).debugproc, (**hcg).debugprocrefcon);
	
	#endif
	
	RestoreA5 (appA5);
	
	osainstallpatch (hcg); /*repatch*/
	
	#if TARGET_API_MAC_CARBON == 1
	LMSetApplZone(origzone);
	#else
	SetZone (origzone);
	#endif
		
	osapushfastcontext (hcg);
	
	return (!oserror (err));
	} /*osadebugger*/


#if !flruntime

static boolean osaprocessstarted (void) {
	
	/*
	we don't want Frontier's menus to dim when serving osa scripts
	*/
	
	processnotbusy ();
	
	return (true);
	} /*osaprocessstarted*/

#endif


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
	
	#if !flruntime
		
		(**hp).fldebugging = bitboolean (modeflags & kOSAModeDebug);
		
		(**hp).processstartedroutine = &osaprocessstarted;
		
	#endif
	
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
	
	#if TARGET_API_MAC_CARBON == 1
	LMSetApplZone(homezone);
	#else	
	SetZone (homezone);
	#endif
		
	if (testheapspace (2 * 1024)) /*enough memory to run a process*/
		fl = processruncode (hp, resultval);
	else
		fl = false;
	
	osasetclientzone (hcg);
	
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
	
	HNoPurge (haete);
	
	flgotname = osaparseaete (haete, class, id, bsname, &paramoffset);
	
	HPurge (haete);
	
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

#if 1

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

#else

static boolean osabuildsubroutineevent (bigstring bsname, hdltreenode hparam1, AppleEvent *event) {
	
	/*
	2.1b3 dmb: build an OSA subroutine event given the name and parameter list
	
	2.1b4 dmb: handle named parameters. if a name is defined, try to use its value 
	as the key. otherwise, 
	*/
	
	AEDesc params = {typeNull, nil};
	AEDesc desc;
	tyvaluerecord vparam;
	tyvaluerecord vkey;
	bigstring bskey;
	OSType key;
	register hdltreenode hp;
	register OSErr err;
	register boolean fl = false;
	hdlhashnode hnode;
	
	err = AECreateAppleEvent (kOSASuite, kASSubroutineEvent, &params, kAutoGenerateReturnID, kAnyTransactionID, event);
	
	if (oserror (err))
		return (false);
	
	err = AEPutParamPtr (event, keyASSubroutineName, typeChar, (Ptr) bsname + 1, stringlength (bsname));
	
	if (err != noErr)
		goto exit;
	
	err = AECreateList (nil, 0, false, &params);
	
	if (err != noErr)
		goto exit;
	
	for (hp = hparam1; hp != nil; hp = (**hp).link) {
		
		if ((**hp).nodetype == fieldop) {
		
			if (!langgetidentifier ((**hp).param1, bsname))
				goto exit;
			
			if (langgetsymbolval (bsname, &vkey, &hnode) && (vkey.valuetype == ostypevaluetype))
				key = vkey.data.ostypevalue;
			else
				stringtoostype (bsname, &key);
			
			if (!evaluatetree ((**hp).param2, &vparam))
				goto exit;
			
			if (!valuetodescriptor (&vparam, &desc))
				goto exit;
			
			err = AEPutKeyDesc (event, key, &desc);
			}
		else {
			
			if (!evaluatetree (hp, &vparam))
				goto exit;
			
			if (!valuetodescriptor (&vparam, &desc))
				goto exit;
			
			err = AEPutDesc (&params, 0, &desc);
			}
		
		AEDisposeDesc (&desc);
		
		if (err != noErr)
			goto exit;
		}
	
	err = AEPutParamDesc (event, keyDirectObject, &params);
	
	if (err != noErr)
		goto exit;
	
	fl = true;
	
	exit:
	
	AEDisposeDesc (&params);
	
	if (!fl) {
		
		oserror (err); /*set error code, if available*/
		
		AEDisposeDesc (event);
		}
	
	return (fl);
	} /*osabuildsubroutineevent*/

#endif

static boolean setstorageval (hdlcomponentglobals hglobals, tyvaluerecord *val, OSAID id) {
	
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
	
	bigstring bs;
	
	numbertostring (id, bs);
	
	if (!hashlookup (bs, val, &hnode)) {
		
		oserror (errOSAInvalidID);
		
		return (false);
		}
	
	return (true);
	} /*getstorageval*/


static boolean deletestorageval (hdlcomponentglobals hglobals, OSAID id) {
	
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
	
	/*
	if (!coercetobinary (v)) { /*get it into a handle-based form%/
		
		disposevaluerecord (*v, true);
		
		return (false);
		}
	
	binarytodesc ((*v).data.binaryvalue, &desc);
	
	exemptfromtmpstack (*v); /*descriptor owns handle now%/
	*/
	
	if (!valuetodescriptor (v, &desc))
		return (false);
	
	if ((desctype != typeWildCard) && (desctype != desc.descriptorType)) { /*AE coercion needed*/
		
		err = AECoerceDesc (&desc, desctype, result);
		
		/*
		if ((err != noErr) && (desctype == typeStyledText))
			err = coerceTEXTtoSTXT (&desc, result);
		*/
		
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
	
	register THz origzone;
	boolean fl;
	//Code change by Timothy Paustian Monday, June 26, 2000 9:37:45 PM
	//
	#if TARGET_API_MAC_CARBON == 1
	origzone = LMGetApplZone();
	LMSetApplZone(homezone);
	#else
	origzone = GetZone ();
	SetZone (homezone);
	#endif
	fl = langexternalnewvalue (id, hdata, val);
	
	#if TARGET_API_MAC_CARBON == 1
	LMSetApplZone(origzone);
	#else	
	SetZone (origzone);
	#endif
		
	return (fl);
	} /*osanewvalue*/


static pascal OSAError osaLoad (
			hdlcomponentglobals	hglobals,
			const AEDesc*		scriptData,
			long				modeFlags,
			OSAID*				resultingCompiledScriptID) {
	
	/*
	2.1b1 dmb: don't insist that the loaded value is a context (i.e. a table)
	*/
	
	const AEDesc *d = scriptData;
	tyvaluerecord vscript, vsource, vcode;
	hdlhashtable hcontext;
	Handle hsource;
	hdltreenode hcode;
	Handle hdata;
	DescType subtype;
	boolean fl;
	OSAError err;
	hdlhashnode hnodesource;
	hdlhashnode hnodecode;
	
	switch ((*d).descriptorType) {
		
		case typeLAND:
			err = noErr;
			
			#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/
			
				{
				Handle h;
				
				copydatahandle (d, &h);
				
				fl = langunpackvalue (h, &vscript);
				
				disposehandle (h);
				}
			
			#else
			
				fl = langunpackvalue ((*d).dataHandle, &vscript);
			
			#endif
			
			break;
		
		case typeOSAGenericStorage:
		
			#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/
			
				{
				Handle hcopy;
				
				copydatahandle (d, &hcopy);
				
				err = OSAGetStorageType (hcopy, &subtype);
				
				//disposehandle (h);
				}
			
			#else
			
				err = OSAGetStorageType ((*d).dataHandle, &subtype);
			
			#endif
			
			if (err != noErr)
				break;
			
			if (subtype != typeLAND)
				return (errOSABadStorageType);
			
			#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/
			
				if (!copydatahandle (d, &hdata))
					return (memFullErr);
			
			#else
			
				if (!copyhandle ((*d).dataHandle, &hdata))
					return (memFullErr);
			
			#endif
			
			err = OSARemoveStorageType (hdata);
			
			if (err == noErr)
				fl = langunpackvalue (hdata, &vscript);
			
			disposehandle (hdata);
			
			break;
		
		default:
			err = errOSABadStorageType;
		}
	
	if (err != noErr)
		return (err);
	
	if (!fl)
		return (osageterror ());
	
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
	hdlhashtable hcontext;
	hdlhashnode hnode;
	boolean flunlinkedsource = false;
	boolean fl;
	OSAError err;
	
	
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
	
	
	#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/
	
		{
		Handle hcopy;
		
		copydatahandle (&desc, hcopy);
		
		fl = langpackvalue (val, &hcopy, HNoNode);
		
		//disposehandle (hcopy);
		}
	
	#else
	
		fl = langpackvalue (val, &desc.dataHandle, HNoNode);
	
	#endif
	
	if (flunlinkedsource)
		hashinsertnode (hnode, hcontext);
	
	if (!fl)
		return (memFullErr);
	
	desc.descriptorType = typeLAND;
	
	if (desiredType == typeOSAGenericStorage) {
		
		#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/
		
			{
			Handle hcopy;
			
			copydatahandle (&desc, &hcopy);
			
			err = OSAAddStorageType (hcopy, typeLAND);
			
			if (err != noErr) {
			
				disposehandle (hcopy);
				
				return (err);
				}
			}
		
		#else
		
			err = OSAAddStorageType (desc.dataHandle, typeLAND);
				
			if (err != noErr) {
				
				DisposeHandle (desc.dataHandle);
				
				return (err);
				}

		#endif
		
		desc.descriptorType = typeOSAGenericStorage;
		}
	
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


static pascal OSAError osaGetResumeDispatchProc (
				hdlcomponentglobals	hglobals,
				AEEventHandlerUPP*	resumeDispatchProc,
				long*				refCon) {
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
	
	#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/
	
		if (!copydatahandle (sourceData, &htext)
			return (memFullErr);
	
	#else
	
		if (!copyhandle ((*sourceData).dataHandle, &htext)) /*don't let langbuildtree consume caller's text*/
			return (memFullErr);
	
	#endif
	
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
	boolean fl = false;
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
		
		#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/
		
			if (!copydatahandle (sourceData, &htext))
				return (memFullErr);
				
		#else
		
			if (!copyhandle ((*sourceData).dataHandle, &htext)) /*make copy for eventual getsource*/
				return (memFullErr);
		
		#endif
		
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


static pascal OSAError osaCoerceFromDesc (
			hdlcomponentglobals	hglobals,
			const AEDesc*		scriptData,
			long				modeFlags,
			OSAID*				resultingScriptID) {
	
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
		
		#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/
		
			{
			Handle h;
			
			copydatahandle (&desc, &h);
			
			fl = langexternalmemoryunpack (h, (hdlexternalhandle *) &val.data.externalvalue);
			
			disposehandle (h);
			}
		
		#else
		
			fl = langexternalmemoryunpack (desc.dataHandle, (hdlexternalhandle *) &val.data.externalvalue);
		
		#endif
		
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


static pascal OSAError osaCoerceToDesc (
			hdlcomponentglobals	hglobals,
			OSAID				scriptID,
			DescType			desiredType,
			long				modeFlags,
			AEDesc*				result) {
	
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


static pascal OSErr coerceInsltoTEXT (
		AEDesc *desc,
		DescType totype,
		long refcon,
		AEDesc *result) {
	
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
	
	long curA5 = SetUpAppA5 ();
	
	#ifdef fldegug
	
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
	
	RestoreA5 (curA5);
	
	return (err);
	} /*coerceInsltoTEXT*/


#if !TARGET_RT_MAC_CFM || TARGET_API_MAC_CARBON
	
	#define coerceInsltoTEXTUPP ((AECoercionHandlerUPP) &coerceInsltoTEXT)
	
#else

	static RoutineDescriptor coerceInsltoTEXTDesc = BUILD_ROUTINE_DESCRIPTOR (uppAECoerceDescProcInfo, coerceInsltoTEXT);

	#define coerceInsltoTEXTUPP ((AECoercionHandlerUPP) &coerceInsltoTEXTDesc)

#endif


static pascal OSErr sendrecordingevent (hdlcomponentglobals	hglobals, AEEventID id) {
	
	/*
	2.1b5 dmb: don't use sendproc for these events; they're not part of 
	script execution
	*/
	
	register hdlcomponentglobals hcg = hglobals;
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
	THz origzone;
	
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
			
			long appA5;
			
			err = AEPutParamPtr (&event, keyDirectObject, typeChar, (Ptr) bssend + 1, stringlength (bssend));
			
			osapreclientcallback (hcg);
			
			origzone = osasetclientzone (hcg);
			
			appA5 = SetUpCurA5 ();
			
			err = AESend (&event, &reply, 
				
				(AESendMode) kAENoReply + kAEDontRecord, 
				
				(AESendPriority) kAENormalPriority, (long) kNoTimeOut, nil, nil);
			
			RestoreA5 (appA5);
			
			#if TARGET_API_MAC_CARBON == 1
			LMSetApplZone(origzone);
			#else	
			SetZone (origzone);
			#endif		
			osapostclientcallback (hcg);
			
			AEDisposeDesc (&event);
			
			AEDisposeDesc (&reply);
			}
		}
	
	return (err);
	} /*sendrecordedtextevent*/


static pascal OSErr targettoprocessinfo (const TargetID *target, ProcessSerialNumber *psn, FSSpec *fs, OSType *signature) {
	#if !TARGET_API_MAC_CARBON
	ProcessInfoRec info;
	OSErr ec;
	
	
	if ((*target).location.locationKindSelector == ppcNoLocation) { /*local program*/
		
		ec = GetProcessSerialNumberFromPortName ((PPCPortPtr) &(*target).name, psn);
		
		if (ec != noErr)
			return (ec);
		
		info.processInfoLength = (long) sizeof (info);
		
		info.processName = nil;
		
		info.processAppSpec = fs;
		
		ec = GetProcessInformation (psn, &info);
		
		if (ec != noErr)
			return (ec);
		
		*signature = info.processSignature;
		}
		
	else { /*not a local program*/
	#endif

		
		clearbytes (psn, sizeof (ProcessSerialNumber));
		
		clearbytes (fs, sizeof (FSSpec)); /*null*/
		
		if ((*target).name.portKindSelector == ppcByCreatorAndType)
			*signature = (*target).name.u.port.portCreator;
		else
			BlockMove ((Ptr) (*target).name.u.portTypeStr + 1, (Ptr) signature, 4L); /*kloooge*/
	#if !TARGET_API_MAC_CARBON
	}
	#endif

	
	return (noErr);
	} /*targettoprocessinfo*/


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


static pascal OSErr handlerecordableevent (const AppleEvent *event, AppleEvent *reply, UInt32 refcon) {
	
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
	short rnum;
	Handle haete;
	
	long curA5 = SetUpAppA5 ();
	
	err = landsystem7getsenderinfo (event, &psn, &fs, &signature);
	
	if (err != noErr) {
		
		RestoreA5 (curA5);
		
		return (err);
		}
	
	osapushfastcontext (hcg);
	
	AEInstallCoercionHandler (typeInsertionLoc, typeObjectSpecifier, coerceInsltoTEXTUPP, 0, true, false);
	
	#if TARGET_API_MAC_CARBON == 1
	LMSetApplZone(homezone);
	#else	
	SetZone (homezone);
	#endif
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
			
			#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/
			
				datahandletostring (&desc, bs);
			
			#else
			
				texthandletostring (desc.dataHandle, bs);
			
			#endif
			
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
	
	osasetclientzone (hcg);
	
	AERemoveCoercionHandler (typeInsertionLoc, typeChar, coerceInsltoTEXTUPP, false);
	
	osapopfastcontext (hcg);
	
	RestoreA5 (curA5);
	
	return (err);
	} /*handlerecordableevent*/

//Code change by Timothy Paustian Monday, June 26, 2000 9:37:56 PM
//
#if !TARGET_RT_MAC_CFM || TARGET_API_MAC_CARBON
	#define handlerecordableeventUPP handlerecordableevent

#else

	static RoutineDescriptor handlerecordableeventDesc = BUILD_ROUTINE_DESCRIPTOR (uppAEEventHandlerProcInfo, handlerecordableevent);

	#define handlerecordableeventUPP (&handlerecordableeventDesc)

#endif

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
		
		
		#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/
		
			newdescwithhandle (&sourcedesc, typeChar, (**hcg).recordingstate.hrecordedtext);
			
			if (AEGetDescDataSize (&sourcedesc) > 0) { /*script was created by recording process*/
			
		#else
		
			sourcedesc.dataHandle = (**hcg).recordingstate.hrecordedtext;
		
			if (sourcedesc.dataHandle != nil) { /*script was created by recording process*/
	
		#endif
		
			
			sourcedesc.descriptorType = typeChar;
			
			err = osaCompile (hcg, &sourcedesc, kOSAModeNull, &compiledScriptID);
			}
		}
	
	(**hcg).recordingstate.flrecording = false;
	
	disposehandle ((**hcg).recordingstate.hrecordedtext);
	
	return (err);
	} /*osaStopRecording*/


static pascal OSAError osaScriptingComponentName (
			hdlcomponentglobals	hglobals,
			AEDesc*				resultingScriptingComponentName) {
	
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


static pascal OSAError osaDisplay (
			hdlcomponentglobals	hglobals,
			OSAID				scriptValueID,
			DescType			desiredType,
			long				modeFlags,
			AEDesc*				resultingText) {
	
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


static pascal OSAError osaMakeContext (
			hdlcomponentglobals	hglobals,
			const AEDesc*		contextName,
			OSAID				parentContext,
			OSAID*				resultingContextID) {
	
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
			
			#if TARGET_API_MAC_CARBON == 1
			LMSetApplZone(homezone);
			#else	
			SetZone (homezone);
			#endif		
			fl = (*(**hg).handleverbroutine) (hverb);
			
			getverbresult (hverb, vresult);
			
			osasetclientzone (hcg);
			
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
					
					#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/
					
						{
						Handle  h;
						
						copydatahandle (&desc, &h);
						
						err = numberfromhandle (h);
						
						disposehandle (h);
						}
					
					#else
					
						err = numberfromhandle (desc.dataHandle);
					
					#endif
					
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
			OSType				selector,
			const AEDesc*		selectorData,
			DescType			desiredType,
			AEDesc*				resultingDebugInfoOrDataToSet) {
	
	OSAError err;
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
			
			#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/
			
				datahandletostring (selectorData, bs);
			
			#else
			
				texthandletostring ((*selectorData).dataHandle, bs);
			
			#endif
			
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
	GrafPtr saveport;
	//Code change by Timothy Paustian Monday, June 26, 2000 9:38:15 PM
	//
	#if TARGET_API_MAC_CARBON == 1
	saveport = GetQDGlobalsThePort();
	#else
	saveport = quickdrawglobal (thePort);
	#endif
		
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
	GrafPtr thePort;
	#if TARGET_API_MAC_CARBON == 1
	thePort = GetQDGlobalsThePort();
	#else
	thePort = quickdrawglobal (thePort);
	#endif
	if (thePort != saveport)
		SetPort (saveport);
	}
	return (result);
	} /*callosafunction*/


#if !TARGET_RT_MAC_CFM || TARGET_API_MAC_CARBON

	#define cmpcloseUPP	((ComponentFunctionUPP) cmpclose)
	#define cmpcandoUPP	((ComponentFunctionUPP) cmpcando)
	#define cmpversionUPP	((ComponentFunctionUPP) cmpversion)
	#define osaLoadUPP	((ComponentFunctionUPP) osaLoad)
	#define osaStoreUPP	((ComponentFunctionUPP) osaStore)
	#define osaExecuteUPP	((ComponentFunctionUPP) osaExecute)
	#define osaDisplayUPP	((ComponentFunctionUPP) osaDisplay)
	#define osaScriptErrorUPP	((ComponentFunctionUPP) osaScriptError)
	#define osaDisposeUPP	((ComponentFunctionUPP) osaDispose)
	#define osaSetScriptInfoUPP	((ComponentFunctionUPP) osaSetScriptInfo)
	#define osaGetScriptInfoUPP	((ComponentFunctionUPP) osaGetScriptInfo)
	#define osaCompileUPP	((ComponentFunctionUPP) osaCompile)
	#define osaGetSourceUPP	((ComponentFunctionUPP) osaGetSource)
	#define osaCoerceFromDescUPP	((ComponentFunctionUPP) osaCoerceFromDesc)
	#define osaCoerceToDescUPP	((ComponentFunctionUPP) osaCoerceToDesc)
	#define osaStartRecordingUPP	((ComponentFunctionUPP) osaStartRecording)
	#define osaStopRecordingUPP	((ComponentFunctionUPP) osaStopRecording)
	#define osaScriptingComponentNameUPP	((ComponentFunctionUPP) osaScriptingComponentName)
	#define osaLoadExecuteUPP	((ComponentFunctionUPP) osaLoadExecute)
	#define osaCompileExecuteUPP	((ComponentFunctionUPP) osaCompileExecute)
	#define osaDoScriptUPP	((ComponentFunctionUPP) osaDoScript)
	#define osaMakeContextUPP	((ComponentFunctionUPP) osaMakeContext)
	#define osaSetResumeDispatchProcUPP	((ComponentFunctionUPP) osaSetResumeDispatchProc)
	#define osaGetResumeDispatchProcUPP	((ComponentFunctionUPP) osaGetResumeDispatchProc)
	#define osaExecuteEventUPP	((ComponentFunctionUPP) osaExecuteEvent)
	#define osaDoEventUPP	((ComponentFunctionUPP) osaDoEvent)
	#define osaSetActiveProcUPP	((ComponentFunctionUPP) osaSetActiveProc)
	#define osaSetDebugProcUPP	((ComponentFunctionUPP) osaSetDebugProc)
	#define osaDebugUPP	((ComponentFunctionUPP) osaDebug)
	#define osaSetSendProcUPP	((ComponentFunctionUPP) osaSetSendProc)
	#define osaGetSendProcUPP	((ComponentFunctionUPP) osaGetSendProc)
	#define osaSetCreateProcUPP	((ComponentFunctionUPP) osaSetCreateProc)
	#define osaGetCreateProcUPP	((ComponentFunctionUPP) osaGetCreateProc)
	#define osaSetDefaultTargetUPP	((ComponentFunctionUPP) osaSetDefaultTarget)

#else

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

	static RoutineDescriptor cmpcloseDesc = BUILD_ROUTINE_DESCRIPTOR (cmpcloseProcInfo, cmpclose);
	static RoutineDescriptor cmpcandoDesc = BUILD_ROUTINE_DESCRIPTOR (cmpcandoProcInfo, cmpcando);
	static RoutineDescriptor cmpversionDesc = BUILD_ROUTINE_DESCRIPTOR (cmpversionProcInfo, cmpversion);
	static RoutineDescriptor osaLoadDesc = BUILD_ROUTINE_DESCRIPTOR (osaLoadProcInfo, osaLoad);
	static RoutineDescriptor osaStoreDesc = BUILD_ROUTINE_DESCRIPTOR (osaStoreProcInfo, osaStore);
	static RoutineDescriptor osaExecuteDesc = BUILD_ROUTINE_DESCRIPTOR (osaExecuteProcInfo, osaExecute);
	static RoutineDescriptor osaDisplayDesc = BUILD_ROUTINE_DESCRIPTOR (osaDisplayProcInfo, osaDisplay);
	static RoutineDescriptor osaScriptErrorDesc = BUILD_ROUTINE_DESCRIPTOR (osaScriptErrorProcInfo, osaScriptError);
	static RoutineDescriptor osaDisposeDesc = BUILD_ROUTINE_DESCRIPTOR (osaDisposeProcInfo, osaDispose);
	static RoutineDescriptor osaSetScriptInfoDesc = BUILD_ROUTINE_DESCRIPTOR (osaSetScriptInfoProcInfo, osaSetScriptInfo);
	static RoutineDescriptor osaGetScriptInfoDesc = BUILD_ROUTINE_DESCRIPTOR (osaGetScriptInfoProcInfo, osaGetScriptInfo);
	static RoutineDescriptor osaCompileDesc = BUILD_ROUTINE_DESCRIPTOR (osaCompileProcInfo, osaCompile);
	static RoutineDescriptor osaGetSourceDesc = BUILD_ROUTINE_DESCRIPTOR (osaGetSourceProcInfo, osaGetSource);
	static RoutineDescriptor osaCoerceFromDescDesc = BUILD_ROUTINE_DESCRIPTOR (osaCoerceFromDescProcInfo, osaCoerceFromDesc);
	static RoutineDescriptor osaCoerceToDescDesc = BUILD_ROUTINE_DESCRIPTOR (osaCoerceToDescProcInfo, osaCoerceToDesc);
	static RoutineDescriptor osaStartRecordingDesc = BUILD_ROUTINE_DESCRIPTOR (osaStartRecordingProcInfo, osaStartRecording);
	static RoutineDescriptor osaStopRecordingDesc = BUILD_ROUTINE_DESCRIPTOR (osaStopRecordingProcInfo, osaStopRecording);
	static RoutineDescriptor osaScriptingComponentNameDesc = BUILD_ROUTINE_DESCRIPTOR (osaScriptingComponentNameProcInfo, osaScriptingComponentName);
	static RoutineDescriptor osaLoadExecuteDesc = BUILD_ROUTINE_DESCRIPTOR (osaLoadExecuteProcInfo, osaLoadExecute);
	static RoutineDescriptor osaCompileExecuteDesc = BUILD_ROUTINE_DESCRIPTOR (osaCompileExecuteProcInfo, osaCompileExecute);
	static RoutineDescriptor osaDoScriptDesc = BUILD_ROUTINE_DESCRIPTOR (osaDoScriptProcInfo, osaDoScript);
	static RoutineDescriptor osaMakeContextDesc = BUILD_ROUTINE_DESCRIPTOR (osaMakeContextProcInfo, osaMakeContext);
	static RoutineDescriptor osaSetResumeDispatchProcDesc = BUILD_ROUTINE_DESCRIPTOR (osaSetResumeDispatchProcProcInfo, osaSetResumeDispatchProc);
	static RoutineDescriptor osaGetResumeDispatchProcDesc = BUILD_ROUTINE_DESCRIPTOR (osaGetResumeDispatchProcProcInfo, osaGetResumeDispatchProc);
	static RoutineDescriptor osaExecuteEventDesc = BUILD_ROUTINE_DESCRIPTOR (osaExecuteEventProcInfo, osaExecuteEvent);
	static RoutineDescriptor osaDoEventDesc = BUILD_ROUTINE_DESCRIPTOR (osaDoEventProcInfo, osaDoEvent);
	static RoutineDescriptor osaSetActiveProcDesc = BUILD_ROUTINE_DESCRIPTOR (osaSetActiveProcProcInfo, osaSetActiveProc);
	static RoutineDescriptor osaSetDebugProcDesc = BUILD_ROUTINE_DESCRIPTOR (osaSetDebugProcProcInfo, osaSetDebugProc);
	static RoutineDescriptor osaDebugDesc = BUILD_ROUTINE_DESCRIPTOR (osaDebugProcInfo, osaDebug);
	static RoutineDescriptor osaSetSendProcDesc = BUILD_ROUTINE_DESCRIPTOR (osaSetSendProcProcInfo, osaSetSendProc);
	static RoutineDescriptor osaGetSendProcDesc = BUILD_ROUTINE_DESCRIPTOR (osaGetSendProcProcInfo, osaGetSendProc);
	static RoutineDescriptor osaSetCreateProcDesc = BUILD_ROUTINE_DESCRIPTOR (osaSetCreateProcProcInfo, osaSetCreateProc);
	static RoutineDescriptor osaGetCreateProcDesc = BUILD_ROUTINE_DESCRIPTOR (osaGetCreateProcProcInfo, osaGetCreateProc);
	/*
	static RoutineDescriptor osaSetDefaultTargetDesc = BUILD_ROUTINE_DESCRIPTOR (osaSetDefaultTargetProcInfo, osaSetDefaultTarget);
	*/

	#define cmpcloseUPP (&cmpcloseDesc)
	#define cmpcandoUPP (&cmpcandoDesc)
	#define cmpversionUPP (&cmpversionDesc)
	#define osaLoadUPP (&osaLoadDesc)
	#define osaStoreUPP (&osaStoreDesc)
	#define osaExecuteUPP (&osaExecuteDesc)
	#define osaDisplayUPP (&osaDisplayDesc)
	#define osaScriptErrorUPP (&osaScriptErrorDesc)
	#define osaDisposeUPP (&osaDisposeDesc)
	#define osaSetScriptInfoUPP (&osaSetScriptInfoDesc)
	#define osaGetScriptInfoUPP (&osaGetScriptInfoDesc)
	#define osaCompileUPP (&osaCompileDesc)
	#define osaGetSourceUPP (&osaGetSourceDesc)
	#define osaCoerceFromDescUPP (&osaCoerceFromDescDesc)
	#define osaCoerceToDescUPP (&osaCoerceToDescDesc)
	#define osaStartRecordingUPP (&osaStartRecordingDesc)
	#define osaStopRecordingUPP (&osaStopRecordingDesc)
	#define osaScriptingComponentNameUPP (&osaScriptingComponentNameDesc)
	#define osaLoadExecuteUPP (&osaLoadExecuteDesc)
	#define osaCompileExecuteUPP (&osaCompileExecuteDesc)
	#define osaDoScriptUPP (&osaDoScriptDesc)
	#define osaMakeContextUPP (&osaMakeContextDesc)
	#define osaSetResumeDispatchProcUPP (&osaSetResumeDispatchProcDesc)
	#define osaGetResumeDispatchProcUPP (&osaGetResumeDispatchProcDesc)
	#define osaExecuteEventUPP (&osaExecuteEventDesc)
	#define osaDoEventUPP (&osaDoEventDesc)
	#define osaSetActiveProcUPP (&osaSetActiveProcDesc)
	#define osaSetDebugProcUPP (&osaSetDebugProcDesc)
	#define osaDebugUPP (&osaDebugDesc)
	#define osaSetSendProcUPP (&osaSetSendProcDesc)
	#define osaGetSendProcUPP (&osaGetSendProcDesc)
	#define osaSetCreateProcUPP (&osaSetCreateProcDesc)
	#define osaGetCreateProcUPP (&osaGetCreateProcDesc)
	#define osaSetDefaultTargetUPP (&osaSetDefaultTargetDesc)

#endif

static pascal ComponentResult osaDispatch (register ComponentParameters *params, Handle storage) {
	
	ComponentResult result = noErr;
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
					
					// 5.0.1: might not be - assert (clienta5 == (long) LMGetCurrentA5 ());
					
				#endif
				//Code change by Timothy Paustian Monday, June 26, 2000 9:39:54 PM
				//We don't need this
				#if !TARGET_API_MAC_CARBON
				SetComponentInstanceA5 ((ComponentInstance) self, selfa5);
				#endif

				if (newcomponentglobals (self, clienta5, &hglobals))
					SetComponentInstanceStorage ((ComponentInstance) self, (Handle) hglobals);
				else
					result = memFullErr;
				#if !TARGET_API_MAC_CARBON
				RestoreA5 (clienta5);
				#endif

				break;
				}
			
			case kComponentCloseSelect:
				
				result = CallComponentFunctionWithStorage (storage, params, cmpcloseUPP);
				
				break;
			
			case kComponentCanDoSelect:
				result = CallComponentFunction (params, cmpcandoUPP);
				
				break;
			
			case kComponentVersionSelect:
				result = CallComponentFunction (params, cmpversionUPP);
				
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
				func = osaLoadUPP;
				
				break;
			
			case kOSASelectStore:
				func = osaStoreUPP;
				
				break;
			
			case kOSASelectExecute:
				func = osaExecuteUPP;
				
				break;
			
			case kOSASelectDisplay:
				func = osaDisplayUPP;
				
				break;
			
			case kOSASelectScriptError:
				func = osaScriptErrorUPP;
				
				break;
			
			case kOSASelectDispose:
				func = osaDisposeUPP;
				
				break;
			
			case kOSASelectSetScriptInfo:
				func = osaSetScriptInfoUPP;
				
				break;
			
			case kOSASelectGetScriptInfo:
				func = osaGetScriptInfoUPP;
				
				break;
			
			case kOSASelectCompile:
				func = osaCompileUPP;
				
				break;
			
			case kOSASelectGetSource:
				func = osaGetSourceUPP;
				
				break;
			
			case kOSASelectCoerceFromDesc:
				func = osaCoerceFromDescUPP;
				
				break;
			
			case kOSASelectCoerceToDesc:
				func = osaCoerceToDescUPP;
				
				break;
			
			case kOSASelectStartRecording:
				func = osaStartRecordingUPP;
				
				break;
			
			case kOSASelectStopRecording:
				func = osaStopRecordingUPP;
				
				break;
			
			case kOSASelectScriptingComponentName:
				func = osaScriptingComponentNameUPP;
				
				break;
			
			case kOSASelectLoadExecute:
				func = osaLoadExecuteUPP;
				
				break;
			
			case kOSASelectCompileExecute:
				func = osaCompileExecuteUPP;
				
				break;
			
			case kOSASelectDoScript:
				func = osaDoScriptUPP;
				
				break;
			
			case kOSASelectMakeContext:
				func = osaMakeContextUPP;
				
				break;
			
			case kOSASelectSetResumeDispatchProc:
				func = osaSetResumeDispatchProcUPP;
				
				break;
			
			case kOSASelectGetResumeDispatchProc:
				func = osaGetResumeDispatchProcUPP;
				
				break;
			
			case kOSASelectExecuteEvent:
				func = osaExecuteEventUPP;
				
				break;
			
			case kOSASelectDoEvent:
				func = osaDoEventUPP;
				
				break;
			
			case kOSASelectSetActiveProc:
				func = osaSetActiveProcUPP;
				
				break;
			
			case kOSASelectSetDebugProc:
				func = osaSetDebugProcUPP;
				
				break;
			
			case kOSASelectDebug:
				func = osaDebugUPP;
				
				break;
			
			case kOSASelectSetSendProc:
				func = osaSetSendProcUPP;
				
				break;
			
			case kOSASelectGetSendProc:
				func = osaGetSendProcUPP;
				
				break;
			
			case kOSASelectSetCreateProc:
				func = osaSetCreateProcUPP;
				
				break;
			
			case kOSASelectGetCreateProc:
				func = osaGetCreateProcUPP;
				
				break;
			
			#if 0
			
			case kOSASelectSetDefaultTarget:
				func = osaSetDefaultTargetUPP;
				
				break;
			
			#endif
			
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
		
		#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/
		
			{
			Handle h;
			
			copydatahandle (&errordesc, &h);
			
			err = numberfromhandle (h);
			
			disposehandle (h);
			}
		
		#else
		
			err = numberfromhandle (errordesc.dataHandle);
		
		#endif
		
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
		
		#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/
		
			datahandletostring (&errordesc, bserror);
		
		#else
		
			texthandletostring (errordesc.dataHandle, bserror);
		
		#endif
		
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
	long appA5;
	
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
	
	appA5 = SetUpCurA5 (); /*3.0a*/
	
	instance = OpenDefaultComponent (kOSAComponentType, type);
	
	RestoreA5 (appA5);
	
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
	
	#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/
		
		{
		Handle h;
		
		copydatahandle (scriptdata, &h);
		
		if (OSAGetStorageType (h, &idserver) != noErr)
			idserver = (*scriptdata).descriptorType;
		
		disposehandle (h);
		}	
	
	#else
	
		if (OSAGetStorageType ((*scriptdata).dataHandle, &idserver) != noErr)
			idserver = (*scriptdata).descriptorType;
	
	#endif
	
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
	
	scriptsource.descriptorType = typeChar;
	
	#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/
	
		newdescwithhandle (&scriptsource, typeChar, htext);
	
	#else
	
		scriptsource.dataHandle = htext;
	
	#endif
	
	id = kOSANullScript;
	
	err = OSACompile (comp, &scriptsource, kOSAModeCompileIntoContext, &id);
	
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
	
	#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/
	
		{
		Handle h;
		
		copydatahandle (&scriptdata, &h);
		
		return (setbinaryvalue (h, scriptdata.descriptorType, vcode));
		}
	
	#else
	
		return (setbinaryvalue (scriptdata.dataHandle, scriptdata.descriptorType, vcode));
		
	#endif
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
	
	#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/
	
		{
		Handle h;
		
		copydatahandle (&scriptsource, &h);
		
		return (setheapvalue (h, stringvaluetype, vsource));
		}
	
	#else
	
		return (setheapvalue (scriptsource.dataHandle, stringvaluetype, vsource));
		
	#endif
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


/*
boolean isosascriptvalue (const tyvaluerecord *val) {
	
	/*
	enforce a strong definition of an osascriptvalue: a binary whose type
	is 'scpt', that might be confused with a packed Frontier script
	%/
	
	Handle x;
	OSErr err;
	OSType subtype;
	
	if ((*val).valuetype != binaryvaluetype)
		return (false);
	
	x = (*val).data.binaryvalue;
	
	if (getbinarytypeid (x) != typeOSAGenericStorage) /*it might be; can't tell%/
		return (false);
	
	err = OSAGetStorageType (x, &subtype);
	
	return (err == noErr);
	} /*isosascriptvalue*/


static pascal OSErr osaclientactive (long refcon) {
	
	/*
	while executing an osa script, we want to check for cmd-period, 
	and allow other threads to run. this does part of the job.
	*/
	
	OSErr err = noErr;
	
	long curA5 = SetUpAppA5 ();
	
	if (!langbackgroundtask (false) || languserescaped (false))
		err = userCanceledErr;
	
	RestoreA5 (curA5);
	
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


static pascal OSErr osaclientsend (const AppleEvent *, AppleEvent *, AESendMode, AESendPriority, long, AEIdleUPP, AEFilterUPP, long);

#if !TARGET_RT_MAC_CFM || TARGET_API_MAC_CARBON
	
	#define osaclientactiveUPP (&osaclientactive)
	
	#define osaclientidleUPP (&osaclientidleproc)
	
	#define osaclientsendUPP (&osaclientsend)
	
#else

	static RoutineDescriptor osaclientactiveDesc = BUILD_ROUTINE_DESCRIPTOR (uppOSAActiveProcInfo, osaclientactive);
	
	static RoutineDescriptor osaclientidleDesc = BUILD_ROUTINE_DESCRIPTOR (uppAEIdleProcInfo, osaclientidleproc);
	
	static RoutineDescriptor osaclientsendDesc = BUILD_ROUTINE_DESCRIPTOR (uppOSASendProcInfo, osaclientsend);
	
	#define osaclientactiveUPP (&osaclientactiveDesc)
	
	#define osaclientidleUPP (&osaclientidleDesc)
	
	#define osaclientsendUPP (&osaclientsendDesc)
	
#endif


static pascal OSErr osaclientsend (const AppleEvent *event, AppleEvent *reply,
		AESendMode sendmode, AESendPriority priority, long timeout,
        AEIdleUPP idleproc, AEFilterUPP filterproc, long refcon) {
			
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
	
	long curA5 = SetUpAppA5 ();
	
	hlg = landgetglobals ();
	
	err = (*(**hlg).eventsendcallback) (event, reply, sendmode, priority, timeout, osaclientidleUPP, filterproc);
	
	/*
	err = AESend (event, reply, sendmode, priority, timeout, osaclientidleUPP, filterproc);
	*/
	
	if (err == noErr) {
		
		if (!langbackgroundtask (false) || languserescaped (false))
			err = userCanceledErr;
		}
	
	RestoreA5 (curA5);
	
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
	
	#if !TARGET_API_MAC_CARBON
	homeresmap = LMGetTopMapHndl ();
	#endif

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
	
	desc.componentType = type;
	
	desc.componentSubType = 'LAND';
	
	desc.componentManufacturer = 'LAND';
	
	desc.componentFlags = flags;
	
	desc.componentFlagsMask = 0;
	
	hname = getcomponentstringhandle (idname);
	
	hdescription = getcomponentstringhandle (iddescription);
	
	hicon = GetIcon (129);

	#ifdef PIKE	
		comp = RegisterComponent (&desc, NewComponentRoutineProc (dispatch), 0, hname, hdescription, hicon);
	#else
		comp = RegisterComponent (&desc, NewComponentRoutineProc (dispatch), registerComponentGlobal, hname, hdescription, hicon);
	#endif

	disposehandle (hname);
	
	disposehandle (hdescription);
	
	#if !TARGET_API_MAC_CARBON
	if (comp != nil)
		SetComponentRefcon (comp, (long) LMGetCurrentA5 ());
	#endif

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
	
	long flags;
	
	#if !TARGET_API_MAC_CARBON
	RememberA5 ();
	#endif
	
	#if TARGET_API_MAC_CARBON == 1
	homezone = LMGetApplZone();
	#else	
	homezone = GetZone ();
	#endif
		
	osagethomeresfile ();
	
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
	short idstring;
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
			idstring = specificclientstring;
		
		else {
			numbertostring (ctclients, bs);
			
			idstring = multipleclientsstring;
			}
		
		if (getrecordingstring (idstring, bsprompt)) {
			
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
		
		#if flruntime
		
		if (iscurrentapplication (psn)) /*runtime is it's own menusharing client*/
			continue;
		
		#endif
		
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
	
	#ifndef PIKE
		
	initmenusharingcomponent ();
	
	#endif
	
	#if !flruntime
	
		initwindowsharingcomponent ();
	
	#endif
	
	#ifdef dropletcomponent
	
		initdropletcomponent ();
	
	#endif
	
	RegisterComponentResourceFile (filegetapplicationrnum (), true); /*2.1b4*/
	
	AEInstallCoercionHandler (typeType, typeObjectSpecifier, coerceTypetoObjUPP, 0, true, true);
	
	return (newclearhandle (longsizeof (tyclientlist), (Handle *) &hclientlist));
	} /*osacomponentstart*/

#endif

