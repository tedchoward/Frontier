/*
 *  langmath.c
 *  Frontier
 *
 *  Created by Seth Dillingham on 12/29/04.
 *  Copyright 2004 __MyCompanyName__. All rights reserved.
 *
 */
#include <math.h>

#include "frontier.h"
#include "standard.h"

#include "error.h"
#include "memory.h"
#include "ops.h"
#include "resources.h"
#include "lang.h"
#include "langinternal.h"
#include "langexternal.h"
#include "langsystem7.h"
#include "tablestructure.h"
#include "kernelverbs.h"
#include "kernelverbdefs.h"
#include "shell.rsrc.h"
#include "timedate.h"
#include "langmath.h"

#define matherrorlist 269
#define notimplementederror 1

typedef enum tymathtoken { /*verbs that are processed by langmath.c*/
	
	minfunc,
	
	maxfunc,
	
	sqrtfunc,

	cmathverbs
	} tymathtoken;

static boolean mathfunctionvalue (short token, hdltreenode hparam1, tyvaluerecord *vreturned, bigstring bserror) {
	
	hdltreenode hp1 = hparam1;
	tyvaluerecord *v = vreturned;
	short errornum = 0;
	
	setbooleanvalue (false, v); /*by default, math functions return false*/
	
	switch (token) {
	
		case minfunc: { /* 2004/12/29 smd */
			
			tyvaluerecord v1, v2;
			boolean fl;
			
			if (!getparamvalue (hp1, 1, &v1))
				break;
			
			flnextparamislast = true;
			
			if (!getparamvalue (hp1, 2, &v2))
				break;
			
			if (!coercetypes (&v1, &v2)) {
				
				break;
			}
			
			switch (v1.valuetype) {
				
				case novaluetype: {  /* no test needed, nil is nil*/
					initvalue (v, novaluetype);
					return true;
					}
				
				case booleanvaluetype: /* just return an or */
					return setbooleanvalue (v1.data.flvalue || v2.data.flvalue, v);
				
				/* all the rest return the greater of the two values */
				case charvaluetype:
					return setcharvalue (min (v1.data.chvalue, v2.data.chvalue), v);
							
				case intvaluetype:
					return setintvalue (min (v1.data.intvalue, v2.data.intvalue), v);
				
				case longvaluetype:
					return setlongvalue (min (v1.data.longvalue, v2.data.longvalue), v);
				
				case ostypevaluetype:
					return setostypevalue (min (v1.data.longvalue, v2.data.longvalue), v);
					
/*				case fixedvaluetype:
					return setfixedvalue (min (v1.data.longvalue, v2.data.longvalue), v);
*/				
				case directionvaluetype:
					return setdirectionvalue (min ((short) v1.data.dirvalue, (short) v2.data.dirvalue), v);
					
				case datevaluetype:
					return setdatevalue (timegreaterthan (v1.data.datevalue, v2.data.datevalue) ? v1.data.datevalue : v2.data.datevalue, v);
				
				case singlevaluetype:
					return setsinglevalue (min (v1.data.singlevalue, v2.data.singlevalue), v);
				
				case doublevaluetype:
					return setdoublevalue (min (**v1.data.doublevalue, **v2.data.doublevalue), v);
				
				case stringvaluetype: {
					
					Handle h;
					
					if (comparehandles (v1.data.stringvalue, v2.data.stringvalue) == 1)
						copyhandle (v2.data.stringvalue, &h);
					else
						copyhandle (v1.data.stringvalue, &h);
						
					return (setheapvalue (h, stringvaluetype, v));
					}
				
				default:
					langerror (comparisonnotpossibleerror);
					
					fl = false; /*operation is not defined*/
					
					break;
				} /*switch*/
			
			return (false);
			}
		
		case maxfunc: { /* 2004/12/29 smd */
			
			tyvaluerecord v1, v2;
			boolean fl;
			
			if (!getparamvalue (hparam1, 1, &v1))
				break;
			
			flnextparamislast = true;
			
			if (!getparamvalue (hparam1, 2, &v2))
				break;
			
			if (!coercetypes (&v1, &v2)) {
				
				disposevalues (&v1, &v2);
				
				break;
			}
			
			switch (v1.valuetype) {
				
				case novaluetype: {  /* no test needed, nil is nil*/
					initvalue (v, novaluetype);
					return true;
					}
				
				case booleanvaluetype: /* just return an or */
					return setbooleanvalue (v1.data.flvalue || v2.data.flvalue, v);
				
				/* all the rest return the greater of the two values */
				case charvaluetype:
					return setcharvalue (max (v1.data.chvalue, v2.data.chvalue), v);
							
				case intvaluetype:
					return setintvalue (max (v1.data.intvalue, v2.data.intvalue), v);
				
				case longvaluetype:
					return setlongvalue (max (v1.data.longvalue, v2.data.longvalue), v);
				
				case ostypevaluetype:
					return setostypevalue (max (v1.data.longvalue, v2.data.longvalue), v);
					
/*				case fixedvaluetype:
					return setfixedvalue (max (v1.data.longvalue, v2.data.longvalue), v);
*/				
				case directionvaluetype:
					return setdirectionvalue (max ((short) v1.data.dirvalue, (short) v2.data.dirvalue), v);
					
				case datevaluetype:
					return setdatevalue (timegreaterthan (v1.data.datevalue, v2.data.datevalue) ? v1.data.datevalue : v2.data.datevalue, v);
				
				case singlevaluetype:
					return setsinglevalue (max (v1.data.singlevalue, v2.data.singlevalue), v);
				
				case doublevaluetype:
					return setdoublevalue (max (**v1.data.doublevalue, **v2.data.doublevalue), v);
				
				case stringvaluetype: {
					
					Handle h;
					
					if (comparehandles (v1.data.stringvalue, v2.data.stringvalue) == 1)
						copyhandle (v1.data.stringvalue, &h);
					else
						copyhandle (v2.data.stringvalue, &h);
						
					return (setheapvalue (h, stringvaluetype, v));
					}
				
				default:
					langerror (comparisonnotpossibleerror);
					
					fl = false; /*operation is not defined*/
					
					break;
				} /*switch*/
			
			return (false);
			}
		
		case sqrtfunc: { /* 2004/12/29 smd */
			
			tyvaluerecord v1;
			double d;
			
			flnextparamislast = true;
			
			if (!getdoubleparam (hp1, 1, &v1))
				break;
			
			d = sqrt (**v1.data.doublevalue);
			
			return (setdoublevalue (d, v));
			}

		default:
			errornum = notimplementederror;
			
			goto error;
		} /*switch*/
	
	error:
	
	if (errornum != 0) /*get error string*/
		getstringlist (matherrorlist, errornum, bserror);
	
	return (false);
	} /*langmathvalue*/
	

boolean mathinitverbs (void) {
	
	/*
	2004-12-29 smd: new math verbs
	*/
	
	return (loadfunctionprocessor (idmathverbs, &mathfunctionvalue));
	} /*mathinitverbs*/
