
/*	$Id$    */

/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include "frontier.h"
#include "standard.h"

#include "iacinternal.h"

#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/
	#include "aeutils.h"  /*PBS 03/14/02: AE OS X fix.*/
#endif

/*
8/24/92 DW: The routines in this file are usually only needed in double-
clickable applications.

7/19/94 dmb: Added IACwaitroutineUPP for Universal Headers / PowerPC 
compatibility
*/

#define typeTargetID 'targ'

OSErr IACdrivefilelist (tyFScallback handlespecroutine) {
	
	/*
	the opendoc and printdoc required events take a list of filespecs as a 
	parameter. we factor out the common code, and make it a little easier for an
	application to handle these events.
	
	you supply a callback routine that handles a single filespec, you could
	print it or open it, depending on which of the required events is being
	invoked.
	*/

	AEDesc desc;
	long ctfiles;
	DescType actualtype;
	long actualsize;
	AEKeyword actualkeyword;
	FSSpec fs;
	long i;
	OSErr ec;
						
	ec = AEGetKeyDesc (IACglobals.event, keyDirectObject, typeAEList, &desc);
	
	IACglobals.errorcode = ec;
	
	if (ec != noErr) 
		return (ec);
		
	ec = AECountItems (&desc, &ctfiles);
	
	IACglobals.errorcode = ec;
	
	if (ec != noErr) 
		return (ec);
				
	for (i = 1; i <= ctfiles; i ++) {
	
		ec = AEGetNthPtr (
			&desc, i, typeFSS, &actualkeyword, &actualtype, 
			
			(Ptr) &fs, sizeof (fs), &actualsize);
							
		IACglobals.errorcode = ec;
	
		if (ec != noErr) {
		
			AEDisposeDesc (&desc);
			
			return (ec);
			}
			
		if (!(*handlespecroutine) (&fs))
			return (-1);
		} /*for*/
		
	return (noErr);
	} /*IACdrivefilelist*/


static OSType IACgetprocesscreator (void) {
	
	/*
	get the 4-character creator identifier for the application we're running 
	inside of.
	*/
	
	ProcessSerialNumber psn;
	ProcessInfoRec info;
	
	GetCurrentProcess (&psn);
	
	info.processInfoLength = (long) sizeof (info);
	
	info.processName = nil;
	
	info.processAppSpec = nil;
	
	//Code check by Timothy Paustian Tuesday, April 25, 2000 10:18:44 PM
	//This call should be OK, it is not accessing any forbidden fields
	GetProcessInformation (&psn, &info);
	
	return (info.processSignature);
	} /*IACgetprocesscreator*/
	
	
static pascal OSErr CoerceTargetIDToType (DescType typecode, Ptr dataptr, Size datasize, DescType totype, long refcon, AEDesc *result) {
	
	#pragma unused (typecode, datasize, totype)
	
	TargetID target;
	Ptr addressoftype;
	
	#if !TARGET_API_MAC_CARBON
	ProcessSerialNumber psn;
	ProcessInfoRec info;
	OSErr ec;
	#endif
	
	BlockMove (dataptr, &target, sizeof (target));
	
	//Code change by Timothy Paustian Sunday, June 25, 2000 10:08:45 PM
	//This is a bit of a hack but should work. We will just get stuff directly out of
	//the target rec. I hope this works. Maybe this never gets called in carbon.
	#if !TARGET_API_MAC_CARBON
	if (target.location.locationKindSelector == ppcNoLocation) { /*local program*/
		
		ec = GetProcessSerialNumberFromPortName (&target.name, &psn);
		
		if (ec != noErr)
			return (ec);
		
		info.processInfoLength = (long) sizeof (info);
		
		info.processName = nil;
		
		info.processAppSpec = nil;
		
		//Code check by Timothy Paustian Tuesday, April 25, 2000 10:20:35 PM
		//I think this call is OK, but not sure.
		ec = GetProcessInformation (&psn, &info);
		
		if (ec != noErr)
			return (ec);
			
		addressoftype = (Ptr) &info.processSignature;
		}
		
	else { /*not a local program*/
	#endif
		
		if (target.name.portKindSelector == ppcByCreatorAndType)
			addressoftype = (Ptr) &target.name.u.port.portCreator;
		else
			addressoftype = ((Ptr) &target.name.u.portTypeStr) + 1; /*kloooge*/
	
	#if !TARGET_API_MAC_CARBON
	}
	#endif

		
	(*result).descriptorType = typeType;
	
	#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/
	
		return (putdescdatapointer (result, typeType, addressoftype, 4));
	
	#else
	
		return (PtrToHand (addressoftype, &(*result).dataHandle, 4));
	
	#endif
	} /*CoerceTargetIDToType*/
	
	
static pascal OSErr CoercePSNToType (DescType typecode, Ptr dataptr, Size datasize, DescType totype, long refcon, AEDesc *result) {
	
	#pragma unused (typecode, datasize, totype, refcon)
	
	ProcessInfoRec info;
	OSErr ec;
	
	info.processInfoLength = (long) sizeof (info);
	
	info.processName = nil;
	
	info.processAppSpec = nil;
	
	//Code check by Timothy Paustian Tuesday, April 25, 2000 10:22:33 PM
	//somewhat of a dangerous call, This routine assumed the dataptr coming in
	//is a ProcessSerialNumber, only accesses the signature so OK.
	ec = GetProcessInformation ((ProcessSerialNumber *) dataptr, &info);
	
	if (ec != noErr)
		return (ec);
		
	(*result).descriptorType = typeType;
	
	#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/
	
		return (putdescdatapointer (result, typeType, &info.processSignature, 4));
	
	#else
	
		return (PtrToHand (&info.processSignature, &(*result).dataHandle, 4));
	
	#endif
	} /*CoercePSNToType*/


Boolean IACinit (void) {
	
	/*
	returns true if the machine we're running on has Apple events, and if we are
	able to install our coercion handlers. it may do more in future versions.
	
	call this routine before using any of the other iac.c routines. disable your
	Apple event support if we return false.
	
	8/23/92 DW: only initialize once, no matter how many times we're called.
	*/
	
	static Boolean fl = false; 
	
	if (fl) 
		return (true);
		
	fl = true;
	
	if (!IAChaveappleevents ())
		return (false);
		
	if (!IACinstallcoercionhandler (typeProcessSerialNumber, typeType, (ProcPtr) &CoercePSNToType))
		return (false);
		
	if (!IACinstallcoercionhandler (typeTargetID, typeType, (ProcPtr) &CoerceTargetIDToType))
		return (false);
	
	#if TARGET_API_MAC_CARBON == 1
	
		IACwaitroutineUPP = NewAEIdleUPP ((AEIdleProcPtr) &IACwaitroutine);
	
	#else

		IACwaitroutineUPP = NewAEIdleProc (IACwaitroutine);
	
	#endif
	
	IACglobals.idprocess = IACgetprocesscreator ();

	IACglobals.waitroutine = nil;
	
	IACglobals.nextparamoptional = false;
		
	return (true);
	} /*IACinit*/
	
	
Boolean IAChaveappleevents (void) {

	/*
	return true if Apple Events are available.
	*/
	
	long lGestaltAppleEventsPresent;
	
	if (Gestalt (gestaltAppleEventsAttr, &lGestaltAppleEventsPresent) != noErr)
		return (false);
	
	return (lGestaltAppleEventsPresent != 0);
	} /*IAChaveappleevents*/


	
	

