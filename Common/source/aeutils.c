
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

#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/

#include "frontier.h"
#include "standard.h"

#include "aeutils.h"
#include "memory.h"


static boolean getdescdata (AEDesc *desc, Handle *h) {
	
	/*
	PBS 03/14/02: get data from AEDesc's data handle as
	a handle. The handle is allocated here. Caller
	will dispose of handle, unless this function returns false.
	*/
	
	Size len;
	
	len = AEGetDescDataSize (desc);
	
	if (len < 1) {
		
		*h = nil;
		
		return (false);
		}
	
	if (!newclearhandle (len, h)) {
		
		*h = nil;
		
		return (false);
		}
	
	lockhandle (*h);
	
	if (AEGetDescData (desc, **h, len) != noErr) {
		 
		 disposehandle (*h);
		 
		 *h = nil;
		 
		 return (false);
		 } /*if*/
	
	unlockhandle (*h);
	
	return (true);
	} /*getdescdata*/


OSErr putdescdatapointer (AEDesc *desc, DescType typeCode, ptrvoid pvoid, long len) {
	
	/*
	PBS 03/14/02: put data into the datahandle.
	*/

	return (AEReplaceDescData (typeCode, pvoid, len, desc) == noErr);	
	} /*putdescdatapointer*/


boolean putdeschandle (AEDesc *desc, DescType typeCode, Handle h) {
	
	/*
	PBS 03/14/02: set a datahandle from a handle.
	*/

	OSErr err = noErr;
	
	lockhandle (h);
	
	err = AEReplaceDescData (typeCode, *h, gethandlesize (h), desc);
		
	unlockhandle (h);
	
	return (err != noErr);
	} /*putdeschandle*/


/*boolean newdescnull (AEDesc *desc) {

	AEInitializeDescInline (desc);
	
	return (AECreateDesc (typeNull, nil, 0, desc) != noErr);
	} /*newdescnull*/
boolean newdescnull (AEDesc *desc, DescType typeCode) {  // MJL 08/12/03: Broken boolean obj spec fix
    AEInitializeDescInline (desc);
    return (AECreateDesc (typeCode, nil, 0, desc) != noErr);
} /*newdescnull*/


boolean nildatahandle (AEDesc *desc) {

	AEInitializeDescInline (desc);
	return (true);
	//return (AEReplaceDescData (typeNull, nil, 0, desc) == noErr);	
	} /*nildatahandle*/
	

boolean newdescwithhandle (AEDesc *desc, DescType typeCode, Handle h) {
	
	/*
	PBS 03/14/02: Create a new AEDesc with the given type and value.
	*/
	
	OSErr err = noErr;
	long len = gethandlesize (h);
	
	AEInitializeDescInline (desc);
		
	if (h == nil)
		
		err = AECreateDesc (typeCode, nil, 0, desc);
		
	else {
	
		lockhandle (h);
		
		err = AECreateDesc (typeCode, *h, len, desc);

		unlockhandle (h);
		} /*else*/
	
	return (err == noErr);		
	} /*newdescwithhandle*/
	
	
boolean datahandletostring (AEDesc *desc, bigstring bs) {
	
	/*
	PBS 03/14/02: get data from AEDesc's data handle as
	a bigstring.
	
	2002-10-13 AR: Removed variable len to eliminate compiler warning
	about unused variables
	*/

	Handle h;
	
	if (!getdescdata (desc, &h))
		return (false);
	
	texthandletostring (h, bs);
	
	disposehandle (h);
	
	return (true);
	} /*datahandletostring*/


boolean copydatahandle (AEDesc *desc, Handle *h) {
	
	/*
	PBS 03/14/02: copy the datahandle to another handle.
	*/
	
	return (getdescdata (desc, h));
	} /*copydatahandle*/


#endif