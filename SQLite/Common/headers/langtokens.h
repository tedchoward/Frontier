
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

#define langtokensinclude



/*
DW 3/20/90:

this file isolates the declarations of the language's verbs.  an earlier version 
had these declarations in lang.h.  but about 30 files include lang.h, so adding
a new verb required all of these to be recompiled.  too bad only two files really
care about the token value -- lang.c and langvalue.c.

so we split the declaration of tyfunctype off into this file.  makes it possible
to add verbs when there's nothing to do in the kitchen or the yard!
*/




typedef enum tyfunctype {
	
	appleeventfunc,
	
	complexeventfunc,
	
	findereventfunc,
	
	tableeventfunc,
	
	objspecfunc,
	
	setobjspecfunc,
	
	packfunc,
	
	unpackfunc,
	
	definedfunc,
	
	typeoffunc,
	
	sizeoffunc,
	
	nameoffunc,
	
	parentoffunc,

	indexoffunc,
	
	gestaltfunc,
	
	syscrashfunc,
	
	myMooffunc,
	
	equalsfunc = 400, /*must agree with numbers in langparser.y*/
	
	notequalsfunc = 401,
	
	greaterthanfunc = 402,
	
	lessthanfunc = 403,
	
	greaterthanorequalfunc = 404,
	
	lessthanorequalfunc = 405,
	
	notfunc = 406,
	
	andfunc = 407,
	
	orfunc = 408,
	
	beginswithfunc = 409,
	
	endswithfunc = 410,
	
	containsfunc = 411,
	
	loopfunc = 500,
	
	fileloopfunc = 501,
	
	infunc = 502,
	 
	breakfunc = 503,
	
	returnfunc = 504,
	
	iffunc = 505,
	
	thenfunc = 506,
	
	elsefunc = 507,
	
	bundlefunc = 508,
	
	localfunc = 509,
	
	onfunc = 510,
	
	whilefunc = 511, 
	
	casefunc = 512,
	
	kernelfunc = 513,
	
	forfunc = 514,
	
	tofunc = 515,
	
	downtofunc = 516,
	
	continuefunc = 517,
	
	withfunc = 518,
	
	tryfunc = 519,
	
	globalfunc = 520
	
	} tyfunctype;


