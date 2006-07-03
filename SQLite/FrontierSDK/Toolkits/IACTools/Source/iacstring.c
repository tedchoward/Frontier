
/*	$Id$    */

/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include "frontier.h"
#include "standard.h"

#include "iacinternal.h"

	
Boolean IACpushstringparam (StringPtr val, OSType keyword) {
		
	return (IACpushstringitem (IACglobals.event, val, keyword));
	} /*IACpushstringparam*/


Boolean IACreturnstring (StringPtr x) {
	
	return (IACpushstringitem (IACglobals.reply, x, keyDirectObject));
	} /*IACreturnstring*/


Boolean IACgetstringparam (OSType keyword, StringPtr val) {
	
	if (!IACgetstringitem (IACglobals.event, keyword, val)) {

		IACparamerror (IACglobals.errorcode, "\pstring", keyword);
		
		return (false);
		}
	
	IACglobals.nextparamoptional = false; /*must be reset for each param*/
	
	return (true);
	} /*IACgetstringparam*/


Boolean IACgetstringitem (AEDescList *list, long n, StringPtr val) {
	
	register OSErr ec;
	DescType key;
	DescType typeCode;
	Size actualSize;
	
	if ((*list).descriptorType != typeAEList) {
	
		ec = AEGetKeyPtr (list, n, typeChar, &typeCode, (Ptr)&val[1], (Size)255, &actualSize);

		if (ec != errAEDescNotFound)
			goto finishString;
		}

	ec = AEGetNthPtr (list, n, typeChar, &key, &typeCode, (Ptr)&val[1], (Size)255, &actualSize);

	finishString:
	
	if (actualSize > 255)
		val[0] = 255;
	else
		val[0] = actualSize;
	
	IACglobals.errorcode = ec;
	
	return (ec == noErr);
	} /*IACgetstringitem*/


Boolean IACpushstringitem (AEDescList *list, StringPtr val, long n) {
	
	register OSErr ec;
	
	if ((*list).descriptorType != typeAEList)
		ec = AEPutKeyPtr (list, n, typeChar, (Ptr)&val[1], (Size)val[0]);
	else
		ec = AEPutPtr (list, n, typeChar, (Ptr)&val[1], (Size)val[0]);
	
	IACglobals.errorcode = ec;
	
	return (ec == noErr);
	} /*IACpushstringitem*/


