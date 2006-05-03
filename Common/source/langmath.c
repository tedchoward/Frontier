
/*	$Id$    */

/******************************************************************************

    UserLand Frontier(tm) -- High performance Web content management,
    object database, system-level and Internet scripting environment,
    including source code editing and debugging.

    Copyright (C) 1992-2004 UserLand Software, Inc.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

******************************************************************************/

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
			tyvaluerecord v1copy, v2copy;
			tyvaluerecord * vResult = NULL;
			boolean fl = false;
			
			if (!getreadonlyparamvalue (hp1, 1, &v1))
				break;
			
			flnextparamislast = true;
			
			if (!getreadonlyparamvalue (hp1, 2, &v2))
				break;
			
			if (!copyvaluerecord (v1, &v1copy))  /* so that we don't coerce the original */
				break;
			
			if (!copyvaluerecord (v2, &v2copy))  /* so that we don't coerce the original */
				break;
			
			if (!coercetypes (&v1copy, &v2copy))
			{
				disposevalues (&v1copy, &v2copy);
				
				break;
			}
			
			switch (v1copy.valuetype) {
				
				case novaluetype: {  /* no test needed, nil is nil*/
					initvalue (v, novaluetype);
					
					fl = true;
					
					break;
					}
				
				case booleanvaluetype: /* if v2 is true, then return v1 (false < true) */
					vResult = v2copy.data.flvalue ? &v1 : &v2;
					
					break;
				
				/* all the rest return the greater of the two values */
				case charvaluetype:
					vResult = ( v1copy.data.chvalue <= v2copy.data.chvalue ) ? &v1 : &v2;
					
					break;
							
				case intvaluetype:
					vResult = ( v1copy.data.intvalue <= v2copy.data.intvalue ) ? &v1 : &v2;
					
					break;
				
				case longvaluetype:
				case ostypevaluetype:
					vResult = ( v1copy.data.longvalue <= v2copy.data.longvalue ) ? &v1 : &v2;
					
					break;
				
				case directionvaluetype:
					vResult = ( (short) v1copy.data.dirvalue <= (short) v2copy.data.dirvalue ) ? &v1 : &v2;
					
					break;
					
				case datevaluetype:
					vResult = timegreaterthan( v2copy.data.datevalue, v1copy.data.datevalue ) ? &v1 : &v2;
					
					break;
				
				case singlevaluetype:
					vResult = ( v1copy.data.singlevalue <= v2copy.data.singlevalue ) ? &v1 : &v2;
					
					break;
				
				case doublevaluetype:
					vResult = ( **v1copy.data.doublevalue <= **v2copy.data.doublevalue ) ? &v1 : &v2;
					
					break;
				
				case stringvaluetype:
					vResult = ( comparehandles( v1copy.data.stringvalue, v2copy.data.stringvalue ) == 1 ) ? &v2 : &v1;
					
					break;
				
				default:
					langerror (comparisonnotpossibleerror);
					
					fl = false; /*operation is not defined*/
					
					break;
			} /*switch*/
			
			if ( vResult != NULL )
			{
				if ( copyvaluerecord( *vResult, v ) )
					fl = true;
				
				disposevalues( &v1copy, &v2copy );
			}
			
			return ( fl );
		}
		
		case maxfunc: { /* 2004/12/29 smd */
			
			tyvaluerecord v1, v2;
			tyvaluerecord v1copy, v2copy;
			tyvaluerecord * vResult = NULL;
			boolean fl = false;
			
			if (!getreadonlyparamvalue (hp1, 1, &v1))
				break;
			
			flnextparamislast = true;
			
			if (!getreadonlyparamvalue (hp1, 2, &v2))
				break;
			
			if (!copyvaluerecord (v1, &v1copy))  /* so that we don't coerce the original */
				break;
			
			if (!copyvaluerecord (v2, &v2copy))  /* so that we don't coerce the original */
				break;
			
			if (!coercetypes (&v1copy, &v2copy))
			{
				disposevalues (&v1, &v2);
				
				break;
			}
			
			switch (v1copy.valuetype) {
				
				case novaluetype: {  /* no test needed, nil is nil*/
					initvalue (v, novaluetype);
					
					fl = true;
					
					break;
					}
				
				case booleanvaluetype: /* if v2 is true, then return v1 (false < true) */
					vResult = v1copy.data.flvalue ? &v1 : &v2;
					
					break;
				
				/* all the rest return the greater of the two values */
				case charvaluetype:
					vResult = ( v1copy.data.chvalue >= v2copy.data.chvalue ) ? &v1 : &v2;
					
					break;
							
				case intvaluetype:
					vResult = ( v1copy.data.intvalue >= v2copy.data.intvalue ) ? &v1 : &v2;
					
					break;
				
				case longvaluetype:
				case ostypevaluetype:
					vResult = ( v1copy.data.longvalue >= v2copy.data.longvalue ) ? &v1 : &v2;
					
					break;
				
				case directionvaluetype:
					vResult = ( (short) v1copy.data.dirvalue >= (short) v2copy.data.dirvalue ) ? &v1 : &v2;
					
					break;
					
				case datevaluetype:
					vResult = timegreaterthan( v1copy.data.datevalue, v2copy.data.datevalue ) ? &v1 : &v2;
					
					break;
				
				case singlevaluetype:
					vResult = ( v1copy.data.singlevalue >= v2copy.data.singlevalue ) ? &v1 : &v2;
					
					break;
				
				case doublevaluetype:
					vResult = ( **v1copy.data.doublevalue >= **v2copy.data.doublevalue ) ? &v1 : &v2;
					
					break;
				
				case stringvaluetype:
					vResult = ( comparehandles( v1copy.data.stringvalue, v2copy.data.stringvalue ) != -1 ) ? &v1 : &v2;
					
					break;
				
				default:
					langerror (comparisonnotpossibleerror);
					
					fl = false; /*operation is not defined*/
					
					break;
			} /*switch*/
			
			if ( vResult != NULL )
			{
				if ( copyvaluerecord( *vResult, v ) )
					fl = true;
				
				disposevalues( &v1copy, &v2copy );
			}
			
			return ( fl );
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
