
/*	$Id$    */

/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include <HyperXCmd.h>


static void allupperhandle (Handle htext) {
	
	short ix = 0;
	
	while (true) {
		
		char ch = (*htext) [ix];
		
		if (ch == (char) 0) /*strings are null-terminated*/
			return;
		
		if ((ch >= 'a') && (ch <= 'z'))
			ch -= 32;
			
		(*htext) [ix] = ch;
		
		ix++;
		} /*for*/
	} /*allupperhandle*/
	

pascal void main (XCmdPtr paramPtr) {
	
	Handle htext;
	
	htext = paramPtr->params [0];
	
	HandToHand (&htext); /*make a copy of the handle*/
	
	allupperhandle (htext);
	
	paramPtr->returnValue = htext;
	} /*main*/
