
/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include "iacinternal.h"



Boolean IACpushlongparam (long val, OSType keyword) {

	return (IACpushlongitem (IACglobals.event, val, keyword));
	} /*IACpushlongparam*/


Boolean IACreturnlong (long x) {
	
	return (IACpushlongitem (IACglobals.reply, x, keyDirectObject));
	} /*IACreturnlong*/


Boolean IACgetlongparam (OSType keyword, long *val) {

	if (!IACgetlongitem (IACglobals.event, keyword, val)) {

		IACparamerror (IACglobals.errorcode, "\plong", keyword);
		
		return (false);
		}
	
	IACglobals.nextparamoptional = false; /*must be reset for each param*/
	
	return (true);
	} /*IACgetlongparam*/
	
	
Boolean IACgetlongitem (AEDescList *list, long n, long *val) {
	
	OSErr ec;
	DescType key;
	DescType typeCode;
	Size actualSize;
	
	if ((*list).descriptorType != typeAEList) {
		
		ec = AEGetKeyPtr (list, n, typeLongInteger, &typeCode, (Ptr) val, sizeof (*val), &actualSize);

		if (ec != errAEDescNotFound)
			goto done;
		}

	ec = AEGetNthPtr (list, n, typeLongInteger, &key, &typeCode, (Ptr) val, sizeof (*val), &actualSize);
	
	done:
	
	IACglobals.errorcode = ec;
	
	return (ec == noErr);
	} /*IACgetlongitem*/


Boolean IACpushlongitem (AEDescList *list, long val, long n) {
	
	OSErr ec;
	
	if ((*list).descriptorType != typeAEList)
		ec = AEPutKeyPtr (list, n, typeLongInteger, (Ptr)&val, sizeof (val));
	else
		ec = AEPutPtr (list, n, typeLongInteger, (Ptr)&val, sizeof (val));
	
	IACglobals.errorcode = ec;
	
	return (ec == noErr);
	} /*IACpushlongitem*/


