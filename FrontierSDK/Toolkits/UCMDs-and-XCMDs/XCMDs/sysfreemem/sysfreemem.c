
/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include <HyperXCmd.h>


pascal void main (XCmdPtr paramPtr) {
	
	/*
	This XFCN returns the amount of system memory that's free. 
	*/
	
	Str255 s;
		
	NumToString (TempFreeMem (), s);
	
	paramPtr->returnValue = PasToZero (paramPtr, s);
	} /*main*/
	
	

