
/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include "frontier.h"
#include "standard.h"

#include "iacinternal.h"


#define typeRect 'qdrt'


Boolean IACpushrectparam (Rect *x, OSType keyword) {
	
	return (IACpushrectitem (IACglobals.event, x, keyword));
	} /*IACpushrectparam*/


Boolean IACreturnrect (Rect *x) {
	
	return (IACpushrectitem (IACglobals.reply, x, keyDirectObject));
	} /*IACreturnrect*/


Boolean IACgetrectparam (OSType keyword, Rect *val) {
	
	if (!IACgetrectitem (IACglobals.event, keyword, val)) {
		IACparamerror (IACglobals.errorcode, "\prectangle", keyword);
		
		return (false);
		}
	
	IACglobals.nextparamoptional = false; /*must be reset for each param*/
	
	return (true);
	} /*IACgetrectparam*/
	
	
Boolean IACgetrectitem (AEDescList *list, long n, Rect *val) {
	
	OSErr ec;
	DescType key;
	DescType typeCode;
	Size actualSize;
	
	if ((*list).descriptorType != typeAEList) {
	
		ec = AEGetKeyPtr (list, n, typeRect, &typeCode, (Ptr) val, sizeof (*val), &actualSize);

		if (ec != errAEDescNotFound)
			goto done;
		}

	ec = AEGetNthPtr (list, n, typeRect, &key, &typeCode, (Ptr) val, sizeof (*val), &actualSize);
	
	done:
	
	IACglobals.errorcode = ec;
	
	return (ec == noErr);
	} /*IACgetrectitem*/


Boolean IACpushrectitem (AEDescList *list, Rect *x, long n) {
	
	OSErr ec;
	
	if ((*list).descriptorType != typeAEList)
		ec = AEPutKeyPtr (list, n, typeRect, (Ptr) x, sizeof (*x));
	else
		ec = AEPutPtr (list, n, typeRect, (Ptr) x, sizeof (*x));
	
	IACglobals.errorcode = ec;
	
	return (ec == noErr);
	} /*IACpushrectitem*/


