
/*	$Id$    */

/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include "frontier.h"
#include "standard.h"

#include <appletdefs.h>
#include "iac.h"

#define typeTargetID 'targ'


#if TARGET_API_MAC_OS8

static void MakePascalStringWLen (StringPtr theDest, int theDestLen, char *theSrc, int theSrcLen) {
	
	/*
	10/22/91 dmb: imported this code from Leonard Rosenthal.  Reformmatted 
	somewhat, but left variable names as he had them.  comments are his.
	
	Given a C string, put it into a pascal storage area.	We also make
	sure that the length of the pascal string isn't too long (the caller
	specifiys the longest length possible).	We are passed in the length
	of the C string... This makes our life simpler...
	*/
	
	theDest [0] = theSrcLen > theDestLen ? theDestLen : theSrcLen;
	
	BlockMove (theSrc, &(theDest[1]), theDest [0]);
	} /*MakePascalStringWLen*/


static OSErr HCProgramToPortAndLoc (char *theName, short len, LocationNameRec *theLoc, PortInfoRec *thePort) {
	
	/*
	10/22/91 dmb: imported this code from Leonard Rosenthal.  Reformmatted 
	somewhat, but left variable names as he had them.  comments are his.
	
	Convert a Hypercard program name (<zone>:<mac>:<program>) to a port.
	If that program has more than one port open, we take the first port 
	we find!
	*/
	//Code change by Timothy Paustian Monday, June 26, 2000 2:02:16 PM
	//This routine just won't work. I doubt Hypercard is a big player on OS X
	
	char *appleZone = 0;
	char *macName = 0;
	char *progName = 0;
	char *theLastChar = 0;
	PPCPortRec thePortSearchSpec;
	IPCListPortsPBRec thePBRec;
	int theErr;
	
	/*
	Assemble a location.	This is a bit of a pain, as we must
	carefully unpack the incomming string...
	*/
	
	/*
	First job -- find the end of the incomming string so we don't
	run off into memory...
	*/
	
	theLastChar = theName;
	
	appleZone = theName; /* First thing there... */
	
	while (--len >= 0) {
		
		if (*theLastChar == ':') {
			
			if (!macName) {
				
				macName = theLastChar + 1;
				}
			else if (!progName) {
				
				progName = theLastChar + 1;
				}
			}
		
		theLastChar++;
		}
	
	/*
	Right, make sure that we got everything...
	*/
	
	if ((progName == 0) || (macName == 0))
		return (1);	/* Random error... */
	
	/*
	Next, assemble a port record that we can use to specify what
	the hell we are looking for...	Use a roman script (sorry, guys),
	match only names that have our program, and match all types.
	That way we will get the first type.
	*/
	
	thePortSearchSpec.nameScript = smRoman;
	
	thePortSearchSpec.portKindSelector = ppcByString;
	
	MakePascalStringWLen ((StringPtr) thePortSearchSpec.name, 32, progName, (Size)(theLastChar - progName));
	
	MakePascalStringWLen ((StringPtr) thePortSearchSpec.u.portTypeStr, 32, "=", 1);
	
	/*
	Next job is to fill in the location record that the guy passed
	in.	The objString is the mac we wish to connect to.	The zone
	is the apple zone.	We let the type be random...	so we set it
	to PPCToolBox as defined by IM VI 6 7-23.
	*/
	
	theLoc -> locationKindSelector = ppcNBPLocation;
	
	/*bundle*/ {
		
		EntityName *theE = &(theLoc -> u.nbpEntity);
	
		MakePascalStringWLen ((StringPtr) &(theE -> objStr), 32, macName, (int) (progName - macName - 1));
		
		MakePascalStringWLen ((StringPtr) &(theE -> typeStr), 32, "PPCToolBox", 10);
		
		MakePascalStringWLen ((StringPtr) &(theE -> zoneStr), 32, appleZone, (macName - appleZone - 1));
		}
	
	/*
	Right.	Finally, we fill in the parameter block we are to pass
	to IPCListPorts.
	*/
	
	thePBRec.startIndex = 0;
	thePBRec.requestCount = 1;
	thePBRec.portName = &thePortSearchSpec;
	thePBRec.locationName = theLoc;
	thePBRec.bufferPtr = thePort;
	
	/*
	Call the damm routine and try to get the stupid port back!
	*/
	//Code change by Timothy Paustian Sunday, June 25, 2000 10:22:24 PM
	//This call won't work in carbon. Is it worth supporting?
	#if TARGET_API_MAC_CARBON == 1
	//I set it to a bogus error that makes some sense, but this is a cop out. :-(
	theErr = noResponseErr;
	#else	
	theErr = IPCListPortsSync (&thePBRec);
	#endif
	
	return (theErr);
	} /*HCProgramToPortAndLoc*/


static boolean goodstring32 (byte s32 []) {
	
	byte *s = s32;
	short len = *s;
	
	if ((len == 0) || (len > 32))
		return (false);
	
	while (--len >= 0) {
		
		if ((*++s) < ' ') /*non-printing character*/
			return (false);
		}
	
	return (true);
	} /*goodstring32*/


static pascal Boolean string2networkaddress (ConstStr255Param bsadr, TargetID *target) {
	
	/*
	10/27/91 dmb: even when HCProgramToPortAndLoc's IPCListPorts call returns noErr,
	the network address may be invalid.  so a reality check is done on the port name 
	to attempt to verify that something has actually been found
	*/
		
	LocationNameRec loc;
	PortInfoRec	port;
	OSErr ec;
	
	ec = HCProgramToPortAndLoc ((char *) bsadr + 1, stringlength (bsadr), &loc, &port);
	
	if (ec != noErr) {
		
		IACglobals.errorcode = ec;
		
		return (false);
		}
	
	if (!goodstring32 (port.name.name)) { /*make sure it actually found something*/
		
		IACglobals.errorcode = noResponseErr;
		
		return (false);
		}
	
	(*target).sessionID = 0;
	
	(*target).location = loc;
	
	(*target).name = port.name;
	
	return (true);
	} /*string2networkaddress*/

#else

static pascal Boolean string2networkaddress (ConstStr255Param bsadr, TargetID *target) {

	/*
	2004-10-21 aradke: Can't do this on Carbon, give up and return bogus error
	*/

	IACglobals.errorcode = noResponseErr;

	return (false);

	} /*string2networkaddress*/

#endif


Boolean IACnewnetworkverb (ConstStr255Param bsadr, OSType vclass, OSType vtoken, AppleEvent *event) {
	
	TargetID target;
	AEAddressDesc adr; 
	OSErr ec;
	
	if (!string2networkaddress (bsadr, &target))
		return (false);
	
	ec = AECreateDesc (typeTargetID, (Ptr) &target, sizeof (target), &adr);
	
	if (ec != noErr)
		goto exit;
	
	ec = AECreateAppleEvent (vclass, vtoken, &adr, kAutoGenerateReturnID, kAnyTransactionID, event);
	
	/*AEDisposeDesc (&adr);*/
	
	exit:
	
	IACglobals.errorcode = ec;
	
	return (ec == noErr);
	} /*IACnewnetworkverb*/

