
/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include "iacinternal.h"


Boolean IACpushshortparam (short val, OSType keyword) {
	
	return (IACpushshortitem (IACglobals.event, val, keyword));
	} /*IACpushshortparam*/


Boolean IACreturnshort (short x) {
	
	return (IACpushshortitem (IACglobals.reply, x, keyDirectObject));
	} /*IACreturnshort*/


Boolean IACgetshortparam (OSType keyword, short *val) {
	
	if (!IACgetshortitem (IACglobals.event, keyword, val)) {
		IACparamerror (IACglobals.errorcode, "\pshort", keyword);
		
		return (false);
		}
	
	IACglobals.nextparamoptional = false; /*must be reset for each param*/
	
	return (true);
	} /*IACgetshortparam*/
	
	
Boolean IACgetshortitem (AEDescList *list, long n, short *val) {
	
	OSErr ec;
	DescType key;
	DescType typeCode;
	Size actualSize;
	
	if ((*list).descriptorType != typeAEList) {
	
		ec = AEGetKeyPtr (list, n, typeShortInteger, &typeCode, (Ptr)val,
						  sizeof (*val), &actualSize);
						  
		if (ec != errAEDescNotFound)
			goto done;
		}

	ec = AEGetNthPtr (list, n, typeShortInteger, &key, &typeCode, (Ptr) val, sizeof (*val), &actualSize);
	
	done:
	
	IACglobals.errorcode = ec;
	
	return (ec == noErr);
	} /*IACgetshortitem*/


Boolean IACpushshortitem (AEDescList *list, short val, long n) {
	
	OSErr ec;

	if ((*list).descriptorType != typeAEList)
		ec = AEPutKeyPtr (list, n, typeShortInteger, (Ptr)&val, sizeof (val));
	else
		ec = AEPutPtr (list, n, typeShortInteger, (Ptr)&val, sizeof (val));
	
	IACglobals.errorcode = ec;
	
	return (ec == noErr);
	} /*IACpushshortitem*/


