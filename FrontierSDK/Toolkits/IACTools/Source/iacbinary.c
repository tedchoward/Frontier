
/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include "frontier.h"
#include "standard.h"

#include "iacinternal.h"
#include "aeutils.h"

#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/
	#include "aeutils.h"
#endif


Boolean IACpushbinaryparam (Handle val, OSType binaryfieldtype, OSType keyword) {

	return (IACpushbinaryitem (IACglobals.event, val, binaryfieldtype, keyword));
	} /*IACpushbinaryparam*/


Boolean IACreturnbinary (Handle x, OSType binaryfieldtype) {
	
	return (IACpushbinaryitem (IACglobals.reply, x, binaryfieldtype, keyDirectObject));
	} /*IACreturnbinary*/
	
	
Boolean IACgetbinaryparam (OSType keyword, Handle *hbinary, OSType *binaryfieldtype) {

	if (!IACgetbinaryitem (IACglobals.event, keyword, hbinary, binaryfieldtype)) {
	
		IACparamerror (IACglobals.errorcode, "\pbinary", keyword);
		
		return (false);
		}

	IACglobals.nextparamoptional = false; /*must be reset for each param*/
	
	return (true);
	} /*IACgetbinaryparam*/


Boolean IACgetbinaryitem (AEDescList *list, long n, Handle *val, OSType *binaryfieldtype) {
	
	OSErr ec;
	AEDesc desc;
	DescType key;
	
	if ((*list).descriptorType != typeAEList) {
	
		ec = AEGetKeyDesc (list, n, typeWildCard, &desc);
		
		if (ec != errAEDescNotFound)
			goto done;
		}

	ec = AEGetNthDesc (list, n, typeWildCard, &key, &desc);
	
	done:
	
	IACglobals.errorcode = ec;
	
	if (ec == noErr) {
	
		*binaryfieldtype = desc.descriptorType;
		
		#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/
		
			copydatahandle (&desc, val);
			
		#else
		
			*val = desc.dataHandle;	
		
		#endif
		
		return (true);
		}
	else {
		*binaryfieldtype = 0L;
		
		*val = NULL;
		
		return (false);
		}
	} /*IACgetbinaryitem*/


Boolean IACpushbinaryitem (AEDescList *list, Handle val, OSType binaryfieldtype, long n) {
	
	/*
		dmb 4.1: added IACglobals.dontdisposenextparam field
	*/
		
	OSErr ec;
	AEDesc desc;
	
	desc.descriptorType = binaryfieldtype;
	
	#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/
	
		newdescwithhandle (&desc, binaryfieldtype, val);
		
	#else
	
		desc.dataHandle = val;
	
	#endif
	
	if ((*list).descriptorType != typeAEList)
		
		ec = AEPutKeyDesc (list, n, &desc);
	else
		ec = AEPutDesc (list, n, &desc);
	
	IACglobals.errorcode = ec;
	
	if (IACglobals.dontdisposenextparam)
		IACglobals.dontdisposenextparam = false;
	else
		AEDisposeDesc (&desc);
	
	return (ec == noErr);
	} /*IACpushbinaryitem*/


