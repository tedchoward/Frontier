
/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include "frontier.h"
#include "standard.h"

#include "iacinternal.h"


/*
7/16/94 dmb: Updated for Univeral Headers, PowerPC
*/




Boolean IACinstallhandler (AEEventClass eventclass, AEEventID id, ProcPtr handler) {
	
	OSErr ec;
	
	#if TARGET_API_MAC_CARBON == 1

		ec = AEInstallEventHandler (eventclass, id, NewAEEventHandlerUPP (handler), 0, false);
	
	#else
	
		ec = AEInstallEventHandler (eventclass, id, NewAEEventHandlerProc (handler), 0, false);
	
	#endif
	
	IACglobals.errorcode = ec;
	
	return (ec == noErr);
	} /*AEinstallhandler*/


Boolean IACremovehandler (AEEventClass eventclass, AEEventID id, ProcPtr handler) {

	OSErr ec;

		#if TARGET_RT_MAC_CFM
			#if TARGET_API_MAC_CARBON == 1
				AEEventHandlerUPP theHandler = NewAEEventHandlerUPP(handler);
				ec = AERemoveEventHandler (eventclass, id, theHandler, false);
				DisposeAEEventHandlerUPP(theHandler);

			#else	
			
				long refcon;
				
				ec = AEGetEventHandler (eventclass, id, (AEEventHandlerUPP *) &handler, &refcon, false);
				
				if (ec == noErr)
					ec = AERemoveEventHandler (eventclass, id, (AEEventHandlerUPP) handler, false);
			#endif//TARGET_API_MAC_CARBON == 0
		#else

		ec = AERemoveEventHandler (eventclass, id, (AEEventHandlerUPP) handler, false);
		
		#endif//end TARGET_RT_MAC_CFM
	
	IACglobals.errorcode = ec;
	
	return (ec == noErr);
	} /*IACremovehandler*/


Boolean IAChandlerinstalled (OSType vclass, OSType vtoken, Boolean flsystemhandler) {
	
	/*
	return true if there's a handler installed with the indicated class and id.
	
	if flsystemhandler is true, we only consider system event handlers, if false 
	we only consider app-specific handlers.
	*/
	
	OSErr ec;
	AEEventHandlerUPP handler;
	long refcon;
	
	ec = AEGetEventHandler (vclass, vtoken, &handler, &refcon, flsystemhandler);
	
	IACglobals.errorcode = ec;
	
	return (ec == noErr);
	} /*IAChandlerinstalled*/


Boolean IACinstallcoercionhandler (DescType fromtype, DescType totype, ProcPtr handler) {
	
	OSErr ec;
	
	#if TARGET_API_MAC_CARBON == 1
	AECoercePtrUPP	theHandler = NewAECoercePtrUPP(handler);
	ec = AEInstallCoercionHandler (fromtype, totype, theHandler, (long) 0, false, false);
	//should we delete this, my gut feeling is no, not until the app ends
	#else
	ec = AEInstallCoercionHandler (fromtype, totype, (AECoercionHandlerUPP) NewAECoercePtrProc (handler), (long) 0, false, false);
	#endif
		
	IACglobals.errorcode = ec;

	return (ec == noErr);
	} /*IACinstallcoercionhandler*/



