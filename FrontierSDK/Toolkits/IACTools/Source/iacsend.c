
/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include "frontier.h"
#include "standard.h"

#include "iacinternal.h"

#undef useA4

#if !__powerc

	#ifdef THINK_C
	
		#if __option (a4_globals)
			#define useA4 1
		#endif
	
		#define PrepareCallback() RememberA4() 
		#define EnterCallback() SetUpA4()
		#define ExitCallback() RestoreA4()
		
	#endif

	#ifdef __MWERKS__
	
		#if !__A5__
			#define useA4 1
		#endif
	
	#endif

#endif

#if useA4

	#include <SetupA4.h>

#endif



/*
8/24/92 DW: This file includes routines that are likely to be used by code 
that sends Apple Events.

6/6/93 JWB: Change #ifdef coderesource instances to #if __option (a4_globals)
to make this automatic.
            Change a4IACwaitroutine () to forward the value returned by
            IACwaitroutine ()

7/16/94 dmb: Updated for Univeral Headers, PowerPC.
1/15/96 dmb: Updated for current CW SetupA4.h
*/


Boolean IACnewverb (OSType receiver, OSType vclass, OSType vtoken, AppleEvent *event) {

	/*
	create a new AppleEvent record addressed to the recipient, of the indicated class,
	with the indicated token identifier. return true if it worked, false otherwise.
	*/
	
	AEAddressDesc adr; 
	OSErr errcode;
	
	AECreateDesc (typeApplSignature, (Ptr) &receiver, sizeof (receiver), &adr);
	
	errcode = AECreateAppleEvent (vclass, vtoken, &adr, kAutoGenerateReturnID, kAnyTransactionID, event);
	
	AEDisposeDesc (&adr);
	
	IACglobals.errorcode = errcode;
	
	return (errcode == noErr);
	} /*IACnewverb*/


#if useA4

	static pascal short a4IACwaitroutine (EventRecord *ev, long *sleep, RgnHandle *mousergn) {
		
		short result;
		
		EnterCallback ();
		
		result = IACwaitroutine (ev, sleep, mousergn);
		
		ExitCallback ();
		
		return (result);
		} /*a4IACwaitroutine*/
	
	#if GENERATINGCFM
		
		static RoutineDescriptor a4IACwaitroutineDesc = BUILD_ROUTINE_DESCRIPTOR (uppAEIdleProcInfo, a4IACwaitroutine);
		
		#define a4IACwaitroutineUPP (&a4IACwaitroutineDesc)
		
	#else
		
		#define a4IACwaitroutineUPP ((AEIdleUPP) a4IACwaitroutine)
	
	#endif

#endif


Boolean IACsendverb (AppleEvent *event, AppleEvent *reply) {

	/*
	caller must dispose of the reply.
	*/

	register OSErr ec;
	long mode;
	
	mode = kAEWaitReply + kAECanInteract + kAECanSwitchLayer;
	
	#if useA4
	
		PrepareCallback ();
		
		ec = AESend (
			
			event, reply, mode, kAENormalPriority, kNoTimeOut, 
			
			a4IACwaitroutineUPP, nil);
		
	#else
	
		//Code change by Timothy Paustian Friday, July 28, 2000 1:16:49 PM
		//Note that the IACwaitroutineUPP is initialized in iacapps line 218
		ec = AESend (
			
			event, reply, mode, kAENormalPriority, kNoTimeOut, 
			
			IACwaitroutineUPP, nil);
		
	#endif
	
	AEDisposeDesc (event);	
	
	IACglobals.errorcode = ec;
	
	return (ec == noErr);
	} /*IACsendverb*/


Boolean IACsendverbnoreply (AppleEvent *event, AppleEvent *reply) {

	OSErr ec;
	long mode;
 
	mode = kAENoReply + kAENeverInteract;
 
	ec = AESend (event, reply, mode, kAEHighPriority, kNoTimeOut, nil, nil);
 
	AEDisposeDesc (event); 
 
	IACglobals.errorcode = ec;
	
	return (ec == noErr);
	} /*IACsendverbnoreply*/


Boolean IACiserrorreply (Str255 errorstring) {
	
	/*
	return true if the reply is an error -- if so it has an 'errn' value
	and an 'errs' value. if we return false, the reply is not an error.
	*/
	
	OSErr ec;
	AEDesc desc;
	
	ec = AEGetParamDesc (IACglobals.reply, (AEKeyword) 'errn', 'shor', &desc);
	
	if (ec != noErr) /*the reply isn't an error*/
		return (false);

	AEDisposeDesc (&desc);
	
	ec = AEGetParamDesc (IACglobals.reply, (AEKeyword) 'errs', 'TEXT', &desc);
	
	if (ec != noErr)
		IACcopystring ("\pUnknown error.", errorstring);
	else {
	
		#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/
		
			long lenstring = AEGetDescDataSize (&desc);
			
		#else
		
			long lenstring = GetHandleSize (desc.dataHandle);
		
		#endif
		
		if (lenstring > 255)
			lenstring = 255;
			
		errorstring [0] = (char) lenstring;
		//Code change by Timothy Paustian Saturday, May 20, 2000 2:12:15 PM
		//Changed to Opaque call for Carbon
		//This has not been tested.
		#if ACCESSOR_CALLS_ARE_FUNCTIONS == 1
		{
		Size dataSize = AEGetDescDataSize(&desc);
		Ptr	theData = NewPtr(dataSize);
		AEGetDescData(&desc, theData, dataSize);
		BlockMove (theData, &errorstring [1], lenstring);
		DisposePtr(theData);
		}
		#else
		BlockMove (*desc.dataHandle, &errorstring [1], lenstring);
		#endif
		}
	
	AEDisposeDesc (&desc);
	
	return (true); /*there was an error returned*/
	} /*IACiserrorreply*/


Boolean IACdisposeverb (AppleEvent *event) {
	
	IACglobals.errorcode = AEDisposeDesc (event);

	return (IACglobals.errorcode == noErr);
	} /*IACdisposeverb*/
	
	

