
/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include "iacinternal.h"


/*
8/24/92 DW: This file includes routines that are likely to be used by code 
that receives Apple Events.
*/


Boolean IACreturnerror (short errn, Str255 errs) {
	
	/*
	return an error string and number in an AppleEvent.
	
	SDK 2.0: if the string is nil or empty, don't push it on the reply.
	*/
	
	AppleEvent *savedevent;
	Boolean fl;
	
	savedevent = IACglobals.event;
	
	IACglobals.event = IACglobals.reply; /*push params on the reply record*/
	
	fl = true;
	
	if (errs != nil) 
		if (errs [0] > 0)
			fl = IACpushstringparam (errs, keyErrorString);
			
	if (fl)
		fl = IACpushshortparam (errn, keyErrorNumber);
	
	IACglobals.event = savedevent; /*restore*/
	
	return (fl);
	} /*IACreturnerror*/
	

void IACnothandlederror (void) {
	
	/*
	call this if you receive an Apple event that you are not set up to handle.
	
	we return the standard "event not handled" error return. you can safely 
	return noErr to the Apple Event Manager.
	*/
	
	IACreturnerror (errAEEventNotHandled, nil);
	} /*IACnothandlederror*/
	
	
Boolean IACnextparamisoptional (void) {
	
	IACglobals.nextparamoptional = true;
	
	return (true);
	} /*IACnextparamisoptional*/


void IACparamerror (OSErr errn, Str255 typestring, OSType paramkey) {

	/*
	build an error string that looks like:
	
		The 'read' verb requires a string parameter with a key of '----'
	
	And return it to the caller.
	*/
	
	Str255 errs;
	Str255 bs;
	OSType verbtoken;
	
	if (IACglobals.nextparamoptional) { /*it's not an error, since the caller set this flag*/
	
		IACglobals.nextparamoptional = false; /*must be reset for each param*/
	
		return;
		}
		
	verbtoken = IACgetverbtoken ();
	
	IACcopystring ("\pThe '", errs);
	
	bs [0] = 4;
	
	BlockMove (&verbtoken, &bs [1], 4);
	
	IACpushstring (bs, errs);
	
	IACpushstring ("\p' verb requires a ", errs);
	
	IACpushstring (typestring, errs);
	
	IACpushstring ("\p parameter with a key of '", errs);
	
	bs [0] = 4;
	
	BlockMove (&paramkey, &bs [1], 4);
	
	IACpushstring (bs, errs);
	
	IACpushstring ("\p'", errs);
	
	IACreturnerror (errn, errs);
	} /*IACparamerror*/
	
	
OSType IACgetverbtoken (void) {
	
	/*
	get the id/token of a verb from an AppleEvent
	*/
	
	OSType verbtoken;
	
	if (!IACgetlongattr (IACglobals.event, keyEventIDAttr, typeType, (long *) &verbtoken))
		verbtoken = (OSType) 0;
	
	return (verbtoken);
	} /*IACgetverbtoken*/


OSType IACgetverbclass (void) {
	
	/*
	get the id/token of a verb from an AppleEvent
	*/
	
	OSType verbclass;
	
	if (!IACgetlongattr (IACglobals.event, keyEventClassAttr, typeType, (long *) &verbclass))
		verbclass = (OSType) 0;
	
	return (verbclass);
	} /*IACgetverbclass*/


OSType IACgetsender (void) { 

	/*
	get application signature from address attribute. automatically
	invokes our coercion handlers.
	
	this is needed by the Menu Sharing Toolkit -- when the user presses
	cmd-period to halt a script, the next time we get a message from the
	Menu Sharing server (usually Frontier) we return an error. but we only
	return the error if the call came from Frontier, we use this routine
	to identify the sender of the message.
	
	thanks to Kevin Calhoun of Apple Computer for providing this code! 
	*/

	OSType sender;
	Size actualsize;
	DescType actualtype;
	OSErr errcode;
	
	errcode = AEGetAttributePtr (
		IACglobals.event, keyAddressAttr, typeType, &actualtype, (Ptr) &sender, 
		
		(long) sizeof (sender), &actualsize);

	IACglobals.errorcode = errcode;
	
	return (sender);
	} /*IACgetsender*/

		
