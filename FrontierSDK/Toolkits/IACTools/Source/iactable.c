
/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include "iacinternal.h"

#if TARGET_API_MAC_CARBON == 1 /*PBS 03/14/02: AE OS X fix.*/
	#include "aeutils.h"  /*PBS 03/14/02: AE OS X fix.*/
#endif

#define typeTable 'tabl'


Boolean IACpushtableparam (Handle val, OSType keyword) {

	return (IACpushbinaryparam (val, typeTable, keyword));
	} /*IACpushtableparam*/


Boolean IACreturntable (Handle x) {

	return (IACreturnbinary (x, typeTable));
	} /*IACreturntable*/
	
	
Boolean IACgettableparam (OSType keyword, Handle *hbinary) {
	
	OSType binarytype;
	
	if (!IACgetbinaryparam (keyword, hbinary, &binarytype))
		return (false);
	
	if (binarytype != typeTable) {
		
		DisposeHandle (*hbinary);
		
		*hbinary = nil;
		
		IACparamerror (1, "\ptable", keyword);
		
		return (false);
		}
	
	return (true);
	} /*IACgettableparam*/
	

Boolean IACgettableitem (AEDescList *list, long n, Handle *val) {
	
	OSErr ec;
	AEDesc desc;
	DescType key;
	
	if ((*list).descriptorType != typeAEList) {
	
		ec = AEGetKeyDesc (list, n, typeTable, &desc);
		
		if (ec != errAEDescNotFound)
			goto done;
		}

	ec = AEGetNthDesc (list, n, typeTable, &key, &desc);
	
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
	} /*IACgettableitem*/
	

Boolean IACpushtableitem (AEDescList *list, Handle val, long n) {
	
	return (IACpushbinaryitem (list, val, typeTable, n));
	} /*IACpushtableitem*/


