
/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include "iacinternal.h"


#define typeDate 'date'




Boolean IACpushdateparam (long val, OSType keyword) {

	return (IACpushdateitem (IACglobals.event, val, keyword));
	} /*IACpushdateparam*/


Boolean IACreturndate (long x) {
	
	return (IACpushdateitem (IACglobals.reply, x, keyDirectObject));
	} /*IACreturndate*/


Boolean IACgetdateparam (OSType keyword, long *val) {
	
	if (!IACgetdateitem (IACglobals.event, keyword, val)) {
		
		IACparamerror (IACglobals.errorcode, "\pdate", keyword);
		
		return (false);
		}
	
	IACglobals.nextparamoptional = false; /*must be reset for each param*/
	
	return (true);
	} /*IACgetdateparam*/
	

Boolean IACgetdateitem (AEDescList *list, long n, long *val) {
	
	OSErr ec;
	DescType key;
	DescType typeCode;
	Size actualSize;
	
	if ((*list).descriptorType != typeAEList) {
		
		ec = AEGetKeyPtr (list, n, typeDate, &typeCode, (Ptr) val, sizeof (*val), &actualSize);
			
		if (ec != errAEDescNotFound)
			goto done;
		}

	ec = AEGetNthPtr (list, n, typeDate, &key, &typeCode, (Ptr) val, sizeof (*val), &actualSize);
	
	done:
	
	IACglobals.errorcode = ec;
	
	return (ec == noErr);
	} /*IACgetdateitem*/


Boolean IACpushdateitem (AEDescList *list, long val, long n) {
	
	OSErr ec;
	
	if ((*list).descriptorType != typeAEList)
		ec = AEPutKeyPtr (list, n, typeDate, (Ptr) &val, sizeof (val));
	else
		ec = AEPutPtr (list, n, typeDate, (Ptr)&val, sizeof (val));
	
	IACglobals.errorcode = ec;
	
	return (ec == noErr);
	} /*IACpushdateitem*/

