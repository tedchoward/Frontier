
/*	$Id$    */

/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include <ucmd.h>


#define testtoken 	'test'


static void testverb (void) {
	
	Handle htext, hreturns;
	
	if (!IACgettextparam ((OSType) keyDirectObject, &htext))
		return;
		
	runscript (htext, &hreturns);
	
	IACreturntext (hreturns);
	} /*testverb*/
	

void UCMDmain (void) {
		
	switch (IACgetverbtoken ()) {
	
		case testtoken:
			testverb (); break;
			
		default:
			IACnothandlederror (); break;
			
		} /*switch*/
	} /*UCMDmain*/
