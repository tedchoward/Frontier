
/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include "appletdefs.h"
#include "appletcomponent.h"


void checkComponentInstance (ComponentInstance *instance) {
	
	/*
	if the instance disappears, disable our connection with the
	component by setting the instance to 0.
	*/
	
	ComponentInstance x = *instance;
	
	if (x != 0) {
	
		if (GetComponentVersion (x) == badComponentInstance) /*no longer valid*/
			x = 0;
		}
		
	*instance = x;
	} /*checkComponentInstance*/
