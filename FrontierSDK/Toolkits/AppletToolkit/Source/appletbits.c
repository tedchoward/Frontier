
/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include "frontier.h"
#include "standard.h"

#include "appletdefs.h"
#include "appletbits.h"


unsigned short setbit (unsigned short bits, short bitnum) {
	
	return (bits | ((unsigned short) 1 << bitnum));
	} /*setbit*/
	

unsigned short clearbit (unsigned short bits, short bitnum) {
	
	return (bits & (~((unsigned long) 1 << bitnum)));
	} /*clearbit*/
	

boolean getbit (unsigned short bits, short bitnum) {

	return ((bits >> bitnum) & 1);
	} /*getbit*/
	
	


	

