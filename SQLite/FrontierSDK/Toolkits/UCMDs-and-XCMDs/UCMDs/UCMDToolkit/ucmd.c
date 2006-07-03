
/*	$Id$    */

/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include "ucmd.h"

#ifdef __MWERKS__
	#include <A4Stuff.h>
#endif

#ifdef THINK_C
	
	#include <SetUpA4.h>
	#define EnterCodeResource() do {RememberA0 ();SetUpA4();} while (0)
	#define ExitCodeResource()	RestoreA4()

#endif

#if __powerc

/*
	on the powerPC, our main entry point is a actually a routine descriptor, 
	pointing to our native main code. the global __procinfo is used by the 
	MWC startup code to build the descriptor.
*/
	ProcInfoType __procinfo = uppAEEventHandlerProcInfo;
	
#endif

pascal OSErr main (AppleEvent *event, AppleEvent *reply, long refcon) {
	
	EnterCodeResource ();
	
	IACglobals.event = event;
	
	IACglobals.reply = reply;
	
	IACglobals.refcon = refcon;
	
	UCMDmain ();
	
	ExitCodeResource ();
	
	return (noErr);
	} /*main*/


