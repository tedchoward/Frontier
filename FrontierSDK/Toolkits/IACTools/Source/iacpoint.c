
/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include "frontier.h"
#include "standard.h"

#include "iacinternal.h"


#define typePoint 'QDpt'


Boolean IACpushpointparam (Point val, OSType keyword) {
	
	return (IACpushpointitem (IACglobals.event, val, keyword));
	} /*IACpushpointparam*/


Boolean IACreturnpoint (Point x) {
	
	return (IACpushpointitem (IACglobals.reply, x, keyDirectObject));
	} /*IACreturnpoint*/


Boolean IACgetpointparam (OSType keyword, Point *val) {
	
	if (!IACgetpointitem (IACglobals.event, keyword, val)) {
	
		IACparamerror (IACglobals.errorcode, "\ppoint", keyword);
		
		return (false);
		}
	
	IACglobals.nextparamoptional = false; /*must be reset for each param*/
	
	return (true);
	} /*IACgetpointparam*/
	
	
Boolean IACgetpointitem (AEDescList *list, long n, Point *val) {
	
	OSErr ec;
	DescType key;
	DescType typeCode;
	Size actualSize;
	
	if ((*list).descriptorType != typeAEList) {
	
		ec = AEGetKeyPtr (list, n, typePoint, &typeCode, (Ptr)val,
						  sizeof (*val), &actualSize);

		if (ec != errAEDescNotFound)
			goto done;
		}

	ec = AEGetNthPtr (list, n, typePoint, &key, &typeCode, (Ptr) val, sizeof (*val), &actualSize);
	
	done:
	
	IACglobals.errorcode = ec;
	
	return (ec == noErr);
	} /*IACgetpointitem*/


Boolean IACpushpointitem (AEDescList *list, Point val, long n) {
	
	OSErr ec;
	
	if ((*list).descriptorType != typeAEList)
		ec = AEPutKeyPtr (list, n, typePoint, (Ptr)&val, sizeof (val));
	else
		ec = AEPutPtr (list, n, typePoint, (Ptr)&val, sizeof (val));
	
	IACglobals.errorcode = ec;
	
	return (ec == noErr);
	} /*IACpushpointitem*/

