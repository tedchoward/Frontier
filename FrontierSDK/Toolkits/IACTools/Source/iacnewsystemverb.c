
/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include "iacinternal.h"


/*
8/24/92 DW: This is the only system event verb that UCMDs need as of Frontier
SDK 2.0. We included this in a separate file so that UCMDs don't have to have all
the overhead of all the routines in iacsysevents.c.
*/


Boolean IACnewsystemverb (OSType vclass, OSType vtoken, AppleEvent *event) {

	/*
	6/29/92 DW: special entry point for messages sent to system event handlers.
	
	implementation detail: this is accomplished by sending the message to ourself.
	*/
	
	AEAddressDesc adr; 
	OSErr ec;
	
	ProcessSerialNumber psn;

	psn.highLongOfPSN = 0;

	psn.lowLongOfPSN = kCurrentProcess;

	AECreateDesc (typeProcessSerialNumber, (Ptr) &psn, sizeof (psn), &adr);
	
	ec = AECreateAppleEvent (
		
		vclass, vtoken, &adr, kAutoGenerateReturnID, kAnyTransactionID, event);
	
	AEDisposeDesc (&adr);

	IACglobals.errorcode = ec;
	
	return (ec == noErr);
	} /*IACnewsystemverb*/
	
	
