
/*	$Id$    */

/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include "frontier.h"
#include "standard.h"

#include "iacinternal.h"



Boolean IACpushfilespecparam (FSSpec *x, OSType keyword) {
	
	return (IACpushfilespecitem (IACglobals.event, x, keyword));
	} /*IACpushfilespecparam*/


Boolean IACreturnfilespec (FSSpec *x) {
	
	return (IACpushfilespecitem (IACglobals.reply, x, keyDirectObject));
	} /*IACreturnfilespec*/


Boolean IACgetfilespecparam (OSType keyword, FSSpec *val) {
	
	if (!IACgetfilespecitem (IACglobals.event, keyword, val)) {
		
		IACparamerror (IACglobals.errorcode, "\pfilespec", keyword);
		
		return (false);
		}
	
	IACglobals.nextparamoptional = false; /*must be reset for each param*/
	
	return (true);
	} /*IACgetfilespecparam*/
	

Boolean IACgetfilespecitem (AEDescList *list, long n, FSSpec *val) {
	
	OSErr ec;
	DescType key;
	DescType typeCode;
	Size actualSize;
	
	if ((*list).descriptorType != typeAEList) {
		
		ec = AEGetKeyPtr (list, n, typeFSS, &typeCode, (Ptr) val, sizeof (*val), &actualSize);
			
		if (ec != errAEDescNotFound)
			goto done;
		}

	ec = AEGetNthPtr (list, n, typeFSS, &key, &typeCode, (Ptr) val, sizeof (*val), &actualSize);
	
	done:
	
	IACglobals.errorcode = ec;
	
	return (ec == noErr);
	} /*IACgetfilespecitem*/


Boolean IACpushfilespecitem (AEDescList *list, FSSpec *x, long n) {
	
	OSErr ec;
	
	if ((*list).descriptorType != typeAEList)
		ec = AEPutKeyPtr (list, n, typeFSS, (Ptr) x, sizeof (*x));
	else
		ec = AEPutPtr (list, n, typeFSS, (Ptr) x, sizeof (*x));
	
	IACglobals.errorcode = ec;
	
	return (ec == noErr);
	} /*IACpushfilespecitem*/


