
/*	$Id$    */

/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include "frontier.h"
#include "standard.h"

#include "iacinternal.h"


Boolean IACpushlistparam (AEDescList *val, OSType keyword) {
	
	return (IACpushlistitem (IACglobals.event, val, keyword));
	} /*IACpushlistparam*/


Boolean IACreturnlist (AEDescList *val) {
	
	return (IACpushlistitem (IACglobals.reply, val, keyDirectObject));
	} /*IACreturnlist*/


Boolean IACgetlistparam (OSType keyword, AEDescList *val) {
	
	if (!IACgetlistitem (IACglobals.event, keyword, val)) {
		
		IACparamerror (IACglobals.errorcode, "\plist", keyword);
		
		return (false);
		}
	
	IACglobals.nextparamoptional = false; /*must be reset for each param*/
	
	return (true);
	} /*IACgetlistparam*/
	

Boolean IACnewlist (AEDescList *list) {
	
	OSErr ec;
	
	ec = AECreateList (NULL, (Size) 0, false, list);
	
	IACglobals.errorcode = ec;
	
	return (ec == noErr);
	} /*IACnewlist*/


Boolean IACgetlistitem (AEDescList *list, long n, AEDescList *val) {
	
	OSErr ec;
	DescType key;
	
	if ((*list).descriptorType != typeAEList) {
	
		ec = AEGetKeyDesc (list, n, typeAEList, val);
		
		if (ec != errAEDescNotFound)
			goto done;
		}

	ec = AEGetNthDesc (list, n, typeAEList, &key, val);

	done:
	
	IACglobals.errorcode = ec;
	
	return (ec == noErr);
	} /*IACgetlistitem*/


Boolean IACpushlistitem (AEDescList *list, AEDescList *x, long n) {
	
	OSErr ec;
	
	if ((*list).descriptorType != typeAEList)
		ec = AEPutKeyDesc (list, n, x);
	else
		ec = AEPutDesc (list, n, x);
	
	IACglobals.errorcode = ec;
	
	if (IACglobals.dontdisposenextparam)
		IACglobals.dontdisposenextparam = false;
	else
		AEDisposeDesc (x);
	
	(*x).descriptorType = typeNull;
	
	#if TARGET_API_MAC_CARBON != 1 /*PBS 03/14/02: AE OS X fix.*/
	
		(*x).dataHandle = NULL;
	
	#endif
	
	return (ec == noErr);
	} /*IACpushlistitem*/

