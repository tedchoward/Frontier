
/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include "frontier.h"
#include "standard.h"

#include "iacinternal.h"



Boolean IACpushrecordparam (AERecord * val, OSType keyword) {
	
	return (IACpushrecorditem (IACglobals.event, val, keyword));
	} /*IACpushrecordparam*/


Boolean IACreturnrecord (AERecord *val) {
	
	return (IACpushrecorditem (IACglobals.reply, val, keyDirectObject));
	} /*IACreturnrecord*/


Boolean IACgetrecordparam (OSType keyword, AERecord *val) {
	
	if (!IACgetrecorditem (IACglobals.event, keyword, val)) {
		
		IACparamerror (IACglobals.errorcode, "\precord", keyword);
		
		return (false);
		}
	
	IACglobals.nextparamoptional = false; /*must be reset for each param*/
	
	return (true);
	} /*IACgetrecordparam*/
	

Boolean IACgetrecorditem (AEDescList *list, long n, AERecord *val) {
	
	OSErr ec;
	DescType key;
	
	if ((*list).descriptorType != typeAEList) {
	
		ec = AEGetKeyDesc (list, n, typeAERecord, val);
		
		if (ec != errAEDescNotFound)
			goto done;
		}

	ec = AEGetNthDesc (list, n, typeAERecord, &key, val);
	
	done:
	
	IACglobals.errorcode = ec;
	
	return (ec == noErr);
	} /*IACgetrecorditem*/


Boolean IACpushrecorditem (AEDescList *list, AERecord *x, long n) {
	
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
	} /*IACpushrecorditem*/


Boolean IACnewrecord (AERecord *record) {
	
	OSErr ec;
	
	ec = AECreateList (NULL, (Size) 0, true, record);
	
	IACglobals.errorcode = ec;
	
	return (ec == noErr);
	} /*IACnewrecord*/


Boolean IACgetiteminfo (AERecord *record, long n, AEKeyword *key, DescType *type, Size *bytes) {

	OSErr ec;
	
	ec = AEGetNthPtr (record, n, typeWildCard, key, type, NULL, (Size) 0, bytes);
	
	IACglobals.errorcode = ec;
	
	return (ec == noErr);
	} /*IACgetiteminfo*/



