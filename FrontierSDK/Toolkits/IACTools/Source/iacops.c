
/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


/*
7/16/94 dmb: Updated for Univeral Headers, PowerPC
*/


#include "iacinternal.h"


AEIdleUPP IACwaitroutineUPP = nil;


void IACcopystring (void *source, void *dest) {

	/*
	create a copy of source in dest.  copy the length byte and
	all the characters in the source string.

	assume the strings are pascal strings, with the length byte in
	the first character of the string.
	*/
	
	char *s = source, *d = dest;
	short i, len;
	
	len = (short) s [0];
	
	for (i = 0; i <= len; i++) 
		*d++ = *s++;
	} /*IACcopystring*/


Boolean IACpushstring (void *source, void *dest) {

	/*
	insert the source string at the end of the destination string.
	
	assume the strings are pascal strings, with the length byte in
	the first character of the string.
	*/
	
	char *s = source, *d = dest;
	short lensource = s [0];
	short lendest = d [0];
	short newlen = lensource + lendest;
	
	if (newlen > 255)
		return (false);
		
	d [0] = (char) newlen;
	
	d += (char) lendest + 1; /*point at the position we're copying into*/
	
	s++; /*point at 1st char after length byte*/
	
	while (lensource--) 
		*d++ = *s++;
	
	return (true);
	} /*IACpushstring*/
	
	
Boolean IACgetlongattr (AppleEvent *event, AEKeyword key, DescType type, long *pattr) {
	
	OSErr ec;
	Size actualsize;
	
	ec = AEGetAttributePtr (event, key, type, &type, (Ptr) pattr, sizeof (long), &actualsize);
	
	IACglobals.errorcode = ec;
	
	return (ec == noErr);
	} /*IACgetlongattr*/


Boolean IACbreakembrace (void) {
	
	/*
	return true if the user is holding down the cmd, option and shift keys.
	*/
	
	Ptr p;
	KeyMap keys;
	
	GetKeys (keys);
	
	p = (Ptr) keys; 
	
	return (BitTst (p, 63) && BitTst (p, 48) && BitTst (p, 61));
	} /*IACbreakembrace*/


pascal short IACwaitroutine (EventRecord *ev, long *sleep, RgnHandle *mousergn) {
	
	/*
	called by the AppleEvent manager during AESend to give the user a chance to 
	break out of a wait loop.
	*/
	
	if (IACbreakembrace ()) /*user holding down cmd, option and shift keys*/
		return (-1); /*stop waiting*/
	
	if (IACglobals.waitroutine == nil) /*keep waiting*/
		return (0);
	
	return ((*IACglobals.waitroutine) (ev));
	} /*IACwaitroutine*/




