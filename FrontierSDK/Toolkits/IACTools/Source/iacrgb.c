
/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include "iacinternal.h"


#define typeRGBColor 'cRGB'



Boolean IACpushRGBColorparam (RGBColor *x, OSType keyword) {
	
	return (IACpushRGBColoritem (IACglobals.event, x, keyword));
	} /*IACpushRGBColorparam*/


Boolean IACreturnRGBColor (RGBColor *x) {

	return (IACpushRGBColoritem (IACglobals.reply, x, keyDirectObject));
	} /*IACreturnRGBColor*/


Boolean IACgetRGBColorparam (OSType keyword, RGBColor *val) {

	if (!IACgetRGBColoritem (IACglobals.event, keyword, val)) {
	
		IACparamerror (IACglobals.errorcode, "\pRGBColor", keyword);
		
		return (false);
		}
	
	IACglobals.nextparamoptional = false; /*must be reset for each param*/
	
	return (true);
	} /*IACgetRGBColorparam*/
	

Boolean IACgetRGBColoritem (AEDescList *list, long n, RGBColor *val) {
	
	OSErr ec;
	DescType key;
	DescType typeCode;
	Size actualSize;
	
	if ((*list).descriptorType != typeAEList) {
		
		ec = AEGetKeyPtr (list, n, typeRGBColor, &typeCode, (Ptr) val, sizeof (*val), &actualSize);
			
		if (ec != errAEDescNotFound)
			goto done;
		}

	ec = AEGetNthPtr (list, n, typeRGBColor, &key, &typeCode, (Ptr)val, sizeof (*val), &actualSize);
	
	done:
	
	IACglobals.errorcode = ec;
	
	return (ec == noErr);
	} /*IACgetRGBColoritem*/


Boolean IACpushRGBColoritem (AEDescList *list, RGBColor *x, long n) {
	
	OSErr ec;
	
	if ((*list).descriptorType != typeAEList)
		ec = AEPutKeyPtr (list, n, typeRGBColor, (Ptr) x, sizeof (*x));
	else
		ec = AEPutPtr (list, n, typeRGBColor, (Ptr) x, sizeof (*x));
	
	IACglobals.errorcode = ec;
	
	return (ec == noErr);
	} /*IACpushRGBColoritem*/


