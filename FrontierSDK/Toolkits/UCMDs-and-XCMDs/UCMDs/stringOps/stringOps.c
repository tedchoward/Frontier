
/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include <math.h>
#include <ucmd.h>


#define alluppertoken 	'allu'


static void allupperhandle (Handle htext) {
	
	short i;
	short ct;
	
	ct = GetHandleSize (htext);
	
	for (i = 0; i < ct; i++) {
		
		char ch = (*htext) [i];
		
		if ((ch >= 'a') && (ch <= 'z'))
			ch -= 32;
			
		(*htext) [i] = ch;
		} /*for*/
	} /*allupperhandle*/
	

static void allupperverb (void) {
	
	Handle htext;
	
	if (!IACgettextparam ((OSType) keyDirectObject, &htext))
		return;
		
	allupperhandle (htext);
	
	IACreturntext (htext);
	} /*allupperverb*/
	

void UCMDmain (void) {
		
	switch (IACgetverbtoken ()) {
	
		case alluppertoken:
			allupperverb (); break;
			
		default:
			IACnothandlederror (); break;
			
		} /*switch*/
	} /*UCMDmain*/
