
/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include "frontier.h"
#include "standard.h"

#include "iacinternal.h"
#include "aeutils.h"


Boolean IACpushbooleanparam (Boolean val, OSType keyword) {

	return (IACpushbooleanitem (IACglobals.event, val, keyword));
	} /*IACpushbooleanparam*/


Boolean IACreturnboolean (Boolean fl) {

	return (IACpushbooleanitem (IACglobals.reply, fl, keyDirectObject));
	} /*IACreturnboolean*/


Boolean IACgetbooleanparam (OSType keyword, Boolean *val) {

	if (!IACgetbooleanitem (IACglobals.event, keyword, val)) {
		
		IACparamerror (IACglobals.errorcode, "\pboolean", keyword);
		
		return (false);
		}
	
	IACglobals.nextparamoptional = false; /*must be reset for each param*/
	
	return (true);
	} /*IACgetbooleanparam*/
	
	
Boolean IACgetbooleanitem (AEDescList *list, long n, Boolean *val) {
	
	OSErr ec;
	DescType key;
	DescType typeCode;
	Size actualSize;
	
	if ((*list).descriptorType != typeAEList) {

		ec = AEGetKeyPtr (list, n, typeBoolean, &typeCode, (Ptr) val, sizeof (*val), &actualSize);

		if (ec != errAEDescNotFound)
			goto done;
		}

	ec = AEGetNthPtr (list, n, typeBoolean, &key, &typeCode, (Ptr) val, sizeof (*val), &actualSize);

	done:
	
	IACglobals.errorcode = ec;
	
	return (ec == noErr);
	} /*IACgetbooleanitem*/


Boolean IACpushbooleanitem (AEDescList *list, Boolean val, long n) {
	
	OSErr ec;
	
	if (val) /*create a language-independent boolean*/
		val = 1;
	
	if ((*list).descriptorType != typeAEList)
		ec = AEPutKeyPtr (list, n, typeBoolean, (Ptr) &val, sizeof (val));
	else
		ec = AEPutPtr (list, n, typeBoolean, (Ptr) &val, sizeof (val));
	
	IACglobals.errorcode = ec;
	
	return (ec == noErr);
	} /*IACpushbooleanitem*/


