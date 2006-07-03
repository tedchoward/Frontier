
/*	$Id$    */

/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include "frontier.h"
#include "standard.h"

#include "iacinternal.h"

#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/
	#include "aeutils.h" /*PBS 03/14/02: AE OS X fix.*/
#endif

Boolean IACpushtextparam (Handle val, OSType keyword) {

	return (IACpushbinaryitem (IACglobals.event, val, typeChar, keyword));
	} /*IACpushtextparam*/


Boolean IACreturntext (Handle x) {
	
	return (IACpushbinaryitem (IACglobals.reply, x, typeChar, keyDirectObject));
	} /*IACreturntext*/
	
	
Boolean IACgettextparam (OSType keyword, Handle *htext) {
	
	if (!IACgettextitem (IACglobals.event, keyword, htext)) {
		
		IACparamerror (IACglobals.errorcode, "\ptext", keyword);
		
		return (false);
		}
	
	IACglobals.nextparamoptional = false; /*must be reset for each param*/
	
	return (true);
	} /*IACgettextparam*/
	

Boolean IACgettextitem (AEDescList *list, long n, Handle *val) {
	
	register OSErr ec;
	AEDesc desc;
	DescType key;
	
	if ((*list).descriptorType != typeAEList) {
	
		ec = AEGetKeyDesc (list, n, typeChar, &desc);
		
		if (ec != errAEDescNotFound)
			goto done;
		}

	ec = AEGetNthDesc (list, n, typeChar, &key, &desc);
	
	done:
	
	IACglobals.errorcode = ec;
	
	if (ec == noErr) {
	
		#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/

			copydatahandle (&desc, val);
			
		#else
		
			*val = desc.dataHandle;	
		
		#endif
		
		return (true);
		}
	else {
		*val = NULL;
		
		return (false);
		}
	} /*IACgettextitem*/
	

Boolean IACpushtextitem (AEDescList *list, Handle val, long n) {
	
	return (IACpushbinaryitem (list, val, typeChar, n));
	} /*IACpushtextitem*/


Boolean IACpushtextitemcopy (AEDescList *list, Handle val, long n) {
	
	Handle hcopy = val;
	
	if (hcopy == nil) {
		
		hcopy = NewHandle (0);
		
		if (hcopy == nil)
			return (false);
		}
	else {
		if (HandToHand (&hcopy) != noErr)
			return (false);
		}
			
	return (IACpushbinaryitem (list, hcopy, typeChar, n));
	} /*IACpushtextitemcopy*/


