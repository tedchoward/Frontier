
/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include "frontier.h"
#include "standard.h"

#include "iacinternal.h"


/*
7/25/94 dmb: updated for PowerPC, fp.h
*/

/*  11/8/94 JWB Determine the size of C type double using compile-time tests.   */
/*              It may be necessary to add tests to cover additional compilers. */

#if defined(powerc) || defined(__powerc) || defined(applec) || defined(__SC__) || defined(__GNUC__)

	#define DOUBLE_BYTES 8

#elif defined(THINK_C)

	#if __option(double_8)

		#define DOUBLE_BYTES 8

	#elif !__option(native_fp)

		#define DOUBLE_BYTES 12

	#elif defined(mc68881)

		#define DOUBLE_BYTES 12

	#else

		#define DOUBLE_BYTES 10

	#endif

#elif defined(__MWERKS__)

	#if __IEEEdoubles__

		#define DOUBLE_BYTES 8

	#elif __MC68881__

		#define DOUBLE_BYTES 12

	#else

		#define DOUBLE_BYTES 10

	#endif
#endif


#if DOUBLE_BYTES == 12

/* 11/8/94 JWB  To avoid dependencies on unknown libraries, the conversions between 10 and 12 */
/*              byte formats are done here.  The conversion is done in a way suitable for the */
/*              THINK C Universal format, which should not cause problems with the other      */
/*              compilers.  */


static void iacx96to80 (short * px96, short * px80) {
	*px80 = *px96++;     /* Skipping x96[1], the unused word */

	*++px80 = *++px96;

	*++px80 = *++px96;

	*++px80 = *++px96;

	*++px80 = *++px96;
	} /*xx80to96*/

static void iacx80to96 (short * px80, short * px96) {
	*px96 = *px80--;      /* Arrange to copy x80[0] to both x96[0] and x96[1], in case the
							 THINK C "Universal" doubles are in use. */

	*++px96 = *++px80;

	*++px96 = *++px80;

	*++px96 = *++px80;

	*++px96 = *++px80;

	*++px96 = *++px80;
	} /*xx96to80*/
#endif


#if DOUBLE_BYTES == 8

Boolean IACpushdoubleparam (double val, OSType keyword) {
	
	register OSErr ec;
	AEDesc valDesc = { typeNull, nil };
	
	ec = AECoercePtr (typeFloat, &val, sizeof (val), typeExtended, &valDesc);
	
	if (ec == noErr)

		ec = AEPutParamDesc (IACglobals.event, (AEKeyword) keyword, &valDesc);

	IACglobals.errorcode = ec;
	
	(void) AEDisposeDesc (&valDesc);
	
	return (ec == noErr);
	} /*IACpushdoubleparam*/


Boolean IACreturndouble (double x) {
	
	register OSErr ec;
	AEDesc valDesc = { typeNull, nil };
	
	ec = AECoercePtr (typeFloat, &x, sizeof (x), typeExtended, &valDesc);
	
	if (ec == noErr)

		ec = AEPutParamDesc (IACglobals.reply, keyDirectObject, &valDesc);

	IACglobals.errorcode = ec;
	
	(void) AEDisposeDesc (&valDesc);
	
	return (ec == noErr);
	} /*IACreturndouble*/


Boolean IACgetdoubleparam (OSType keyword, double *val) {
	
	register OSErr ec;
	DescType actualtype;
	Size actualsize;

	ec = AEGetParamPtr (
		
		IACglobals.event, (AEKeyword) keyword, typeFloat, 
		
		&actualtype, (Ptr) val, sizeof (double), &actualsize);
	
	IACglobals.errorcode = ec;
	
	if (ec != noErr) {
		
		IACparamerror (ec, "\pdouble", keyword);
		
		return (false);
		}
	
	IACglobals.nextparamoptional = false; /*must be reset for each param*/

	return (true);
	} /*IACgetdoubleparam*/
	

Boolean IACgetdoubleitem (AEDescList *list, long n, double *val) {
	
	register OSErr ec;
	DescType key;
	DescType typeCode;
	Size actualSize;

	if ((*list).descriptorType == typeAERecord) {
		
		ec = AEGetKeyPtr (list, n, typeFloat, &typeCode, (Ptr) val, sizeof (double), &actualSize);
			
		if (ec != errAEDescNotFound)
			goto done;
		}
	
	ec = AEGetNthPtr (list, n, typeFloat, &key, &typeCode, (Ptr) val,
						sizeof (double), &actualSize);
	
	done:
	
	IACglobals.errorcode = ec;
	
	return (ec == noErr);
	} /*IACgetdoubleitem*/


Boolean IACpushdoubleitem (AEDescList *list, double val, long n) {
	
	register OSErr ec;
	AEDesc valDesc = {typeNull, nil};
	
	ec = AECoercePtr (typeFloat, (Ptr)&val, sizeof (val), typeExtended, &valDesc);
	
	if (ec == noErr)
		if ((*list).descriptorType == typeAERecord)
			ec = AEPutKeyDesc (list, n, &valDesc);
		else
			ec = AEPutDesc (list, n, &valDesc);
	
	IACglobals.errorcode = ec;
	
	(void)AEDisposeDesc (&valDesc);
	
	return (ec == noErr);
	} /*IACpushdoubleitem*/


#elif DOUBLE_BYTES == 12

Boolean IACpushdoubleparam (double val, OSType keyword) {
	
	register OSErr ec;
	short value[5];
	
	iacx96to80 ((short *)&val, value); /*convert from double to extended*/		
	
	ec = AEPutParamPtr (
		
		IACglobals.event, (AEKeyword) keyword, typeExtended, 
		
		(Ptr) &value, sizeof (value));
		
	IACglobals.errorcode = ec;
	
	return (ec == noErr);
	} /*IACpushdoubleparam*/


Boolean IACreturndouble (double x) {
	
	register OSErr ec;
	short value[5];
	
	iacx96to80 ((short *)&x, value); /*convert from double to extended*/		
	
	ec = AEPutParamPtr (
		
		IACglobals.reply, keyDirectObject, typeExtended, (Ptr)&value, 
		
		(Size) sizeof (value));
		
	IACglobals.errorcode = ec;
	
	return (ec == noErr);
	} /*IACreturndouble*/


Boolean IACgetdoubleparam (OSType keyword, double *val) {
	
	register OSErr ec;
	DescType actualtype;
	Size actualsize;
	short value[5];
	
	ec = AEGetParamPtr (
		
		IACglobals.event, (AEKeyword) keyword, typeExtended, 
		
		&actualtype, (Ptr) value, sizeof (value), &actualsize);
	
	IACglobals.errorcode = ec;
	
	if (ec != noErr) {
		
		IACparamerror (ec, "\pdouble", keyword);
		
		return (false);
		}
	
	IACglobals.nextparamoptional = false; /*must be reset for each param*/
	
	iacx80to96 (value, (short *)val);

	return (true);
	} /*IACgetdoubleparam*/
	

Boolean IACgetdoubleitem (AEDescList *list, long n, double *val) {
	
	register OSErr ec;
	DescType key;
	DescType typeCode;
	Size actualSize;
	short value[5];

	if ((*list).descriptorType == typeAERecord) {
		
		ec = AEGetKeyPtr (list, n, typeExtended, &typeCode, (Ptr) value, sizeof (value), &actualSize);
			
		if (ec != errAEDescNotFound)
			goto done;
		}
	
	ec = AEGetNthPtr (list, n, typeExtended, &key, &typeCode, (Ptr) value, sizeof (value), &actualSize);
	
	done:
	
	IACglobals.errorcode = ec;
	
	iacx80to96 (value, (short *)val);
	
	return (ec == noErr);
	} /*IACgetdoubleitem*/


Boolean IACpushdoubleitem (AEDescList *list, double val, long n) {
	
	register OSErr ec;
	short	value[5];

	iacx96to80 ((short *)&val, value); /*convert from double to extended*/		
	
	if ((*list).descriptorType == typeAERecord)
		ec = AEPutKeyPtr (list, n, typeExtended, (Ptr)value, sizeof (value));
	else
		ec = AEPutPtr (list, n, typeExtended, (Ptr)value, sizeof (value));
	
	IACglobals.errorcode = ec;
	
	return (ec == noErr);
	} /*IACpushdoubleitem*/


#elif DOUBLE_BYTES == 10

Boolean IACpushdoubleparam (double val, OSType keyword) {
	
	register OSErr ec;
	
	ec = AEPutParamPtr (
		
		IACglobals.event, (AEKeyword) keyword, typeExtended, 
		
		(Ptr) &val, sizeof (val));
		
	IACglobals.errorcode = ec;
	
	return (ec == noErr);
	} /*IACpushdoubleparam*/


Boolean IACreturndouble (double x) {
	
	register OSErr ec;
	
	ec = AEPutParamPtr (
		
		IACglobals.reply, keyDirectObject, typeExtended, (Ptr)&x, 
		
		(Size) sizeof (x));
		
	IACglobals.errorcode = ec;
	
	return (ec == noErr);
	} /*IACreturndouble*/


Boolean IACgetdoubleitem (AEDescList *list, long n, double *val) {
	
	register OSErr ec;
	DescType key;
	DescType typeCode;
	Size actualSize;

	if ((*list).descriptorType == typeAERecord) {
		
		ec = AEGetKeyPtr (list, n, typeExtended, &typeCode, (Ptr)val, sizeof (double), &actualSize);
			
		if (ec != errAEDescNotFound)
			goto done;
		}
	
	ec = AEGetNthPtr (list, n, typeExtended, &key, &typeCode, (Ptr)val, sizeof (double), &actualSize);
	
	done:
	
	IACglobals.errorcode = ec;
	
	return (ec == noErr);
	} /*IACgetdoubleitem*/


Boolean IACgetdoubleparam (OSType keyword, double *val) {
	
	register OSErr ec;
	DescType typeCode;
	Size actualSize;
		
	ec = AEGetParamPtr (
		IACglobals.event, (AEKeyword) keyword, typeExtended,

		&typeCode, (Ptr)val, sizeof (double), &actualSize);
				
	IACglobals.errorcode = ec;
	
	return (ec == noErr);
	} /*IACgetdoubleitem*/


Boolean IACpushdoubleitem (AEDescList *list, double val, long n) {
	
	register OSErr ec;

	if ((*list).descriptorType == typeAERecord)
		ec = AEPutKeyPtr (list, n, typeExtended, (Ptr)&val, sizeof (double));
	else
		ec = AEPutPtr (list, n, typeExtended, (Ptr)&val, sizeof (double));
	
	IACglobals.errorcode = ec;
	
	return (ec == noErr);
	} /*IACpushdoubleitem*/


#else

#error Invalid value of DOUBLE_BYTES (see top of file)

#endif


