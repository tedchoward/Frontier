
/*	$Id$    */

/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include "appletdefs.h"
#include "appletmath.h"


long divup (long n, long d) {
	
	/*
	divide numerator n by divisor d, rouding up
	*/
	
	if (n >= 0)
		return ((n + d - 1) / d);
	
	return ((n - d + 1) / d);
	} /*divup*/


long divround (long n, long d) {
	
	/*
	divide numerator n by divisor d, rouding as closely as possible
	*/
	
	if (n >= 0)
		return ((n + d / 2) / d);
	
	return ((n - d / 2) / d);
	} /*divround*/


long quantumize (long n, long q) {
	
	/*
	return the lowest multiple of quantum q that is greater (further from 
	zero) that n.  useful for quantumizing scroll distances
	
	9/25/92 dmb: check for zero; it's bogus, but data can be that way
	*/
	
	if (q == 0)
		return (n);
	
	return (divup (n, q) * q);
	} /*quantumize*/
	
	
short getsign (long x) {	

	if (x < 0)
		return (-1);
		
	if (x == 0)
		return (0);
		
	return (1);
	} /*getsign*/
	


