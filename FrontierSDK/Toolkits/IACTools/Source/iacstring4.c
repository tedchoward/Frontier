
/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include "frontier.h"
#include "standard.h"

#include "iacinternal.h"



Boolean IACpushstring4param (OSType val, OSType keyword) {

	return (IACpushstring4item (IACglobals.event, val, keyword));
	} /*IACpushstring4param*/


Boolean IACreturnstring4 (OSType x) {
	
	return (IACpushstring4item (IACglobals.reply, x, keyDirectObject));
	} /*IACreturnstring4*/


Boolean IACgetstring4param (OSType keyword, OSType *val) {
	
	if (!IACgetstring4item (IACglobals.event, keyword, val)) {
		
		IACparamerror (IACglobals.errorcode, "\pstring4", keyword);
		
		return (false);
		}
	
	IACglobals.nextparamoptional = false; /*must be reset for each param*/
	
	return (true);
	} /*IACgetstring4param*/
	

Boolean IACgetstring4item (AEDescList *list, long n, OSType *val) {
	
	OSErr ec;
	DescType key;
	DescType typeCode;
	Size actualSize;
	
	if ((*list).descriptorType != typeAEList) {
		
		ec = AEGetKeyPtr (list, n, typeType, &typeCode, (Ptr)val, sizeof (*val), &actualSize);
			
		if (ec != errAEDescNotFound)
			goto done;
		
		}

	ec = AEGetNthPtr (list, n, typeType, &key, &typeCode, (Ptr) val, sizeof (*val), &actualSize);
	
	done:
	
	IACglobals.errorcode = ec;
	
	return (ec == noErr);
	} /*IACgetstring4item*/


Boolean IACpushstring4item (AEDescList *list, OSType val, long n) {
	
	OSErr ec;
	
	if ((*list).descriptorType != typeAEList)
		ec = AEPutKeyPtr (list, n, typeType, (Ptr)&val, sizeof (val));
	else
		ec = AEPutPtr (list, n, typeType, (Ptr)&val, sizeof (val));
	
	IACglobals.errorcode = ec;
	
	return (ec == noErr);
	} /*IACpushstring4item*/


