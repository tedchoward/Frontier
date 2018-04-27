
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

#include "frontier.h"
#include "standard.h"

#include "error.h"
#include "memory.h"
#include "fileloop.h"
#include "strings.h"
#include "lang.h"
#include "langinternal.h"
#include "langexternal.h"
#include "langsystem7.h"
#include "oplist.h"
#include "ops.h"

#ifdef flcomponent

	#include "osacomponent.h"
	
#endif


	#include <sys/param.h>
	

#define fltryerrorstackcode false


byte bscontainername [] = "\x0a" "_container\0";

static byte nametryerrorval [] = "\x08" "tryerror\0";

#if fltryerrorstackcode
	static byte nametryerrorstackval [] = "\x0d" "tryerrorstack\0";
#endif


void langseterrorline (hdltreenode hnode) {
	
	/*
	the node is about to be executed in some fashion.  if there's an error
	in processing it, we want the error to point at the line number in the 
	source that it was generated from.
	
	6/23/92 dmb: added herrornode for script debugger
	*/
	
	if (hnode && langerrorenabled ()) { /*no one is blocking error reporting*/
		
		ctscanlines = (**hnode).lnum; /*set the global*/
		
		ctscanchars = (**hnode).charnum;
		
		herrornode = hnode;
		}
	} /*langseterrorline*/


#ifdef fldebug

	#if fldebug > 0
	
	static boolean validvalue (tyvaluerecord *val) {
		
		Handle h;
		
		if (!langheapallocated (val, &h))
			return (true); 
		
		return (validhandle (h));
		} /*validvalue*/


	static boolean leakingmemory (tyvaluerecord *val) {
		
		/*
		1/14/91 dmb: verify that val is in the temp stack if it's heap-allocated
		*/
		
		register Handle h;
		register short ctloops;
		register tyvaluerecord *p;
		Handle hcheck;
		
		if (!langheapallocated (val, &hcheck))
			return (false);
		
		if ((*val).fltmpdata) /*data has never been copied from original*/
			return (false);
		
		if ((*val).valuetype == externalvaluetype)
			return (false);
		
		h = hcheck; /*move into register*/
		
		if (currenthashtable == nil)
			return (true);
		
		p = (**currenthashtable).tmpstack;
		
		for (ctloops = (**currenthashtable).cttmpstack; ctloops--; ++p) { /*step through tmpstack*/
			
			if ((*p).data.binaryvalue == h) { /*found the temp in the stack*/
				
				assert (validvalue (val)); /*make sure it's not already disposed*/
				
				return (false);
				}
			} /*while*/
		
		return (true); /*didn't find it -- it's a leak!*/
		} /*leakingmemory*/

	#else
	
		#define validvalue(v) (true)
		#define leakingmemory(v) (false)
	
	#endif
	
#endif


static boolean evaluateloop (hdltreenode hloop, tyvaluerecord *valtree) {
	
	/*
	8/20/91 dmb: call the debugger for each loop iteration
	*/
	
	register hdltreenode h = hloop;
	tyvaluerecord val1;
	
	while (true) {
		
		if ((**h).param2 != nil) { /*termination condition is attached*/
		
			if (!evaluatetree ((**h).param2, &val1))
				return (false);
			
			assert (!leakingmemory (&val1));
			
			if (!coercetoboolean (&val1))
				return (false);
			
			if (!val1.data.flvalue) /*we've reached termination of the loop*/
				return (true);
			}
			
		cleartmpstack (); /*dealloc all outstanding temporary values*/
		
		flbreak = false;
		
		flcontinue = false;
		
		if (!evaluatelist ((**h).param4, valtree))
			return (false);
		
		flcontinue = false;
		
		assert (!leakingmemory (valtree));
		
		if (flbreak || flreturn) {
			
			flbreak = false; /*only good for one level*/
			
			return (true);
			}
		
		if (!langdebuggercall (h)) /*user killed the script*/
			return (false);
		
		if (!evaluatetree ((**h).param3, valtree)) /*run increment code*/
			return (false);
		
		assert (!leakingmemory (valtree));
		} /*while*/
	} /*evaluateloop*/


static boolean evaluateforloop (hdltreenode hloop, tyvaluerecord val1, tyvaluerecord val2, long inc, tyvaluerecord *valtree) {
	
	/*
	9/26/91 dmb: added "for loop" construct like pascal:
		
		for counter = x1 to x2
			<body>
	
	initially, I considered generating a standard loop code tree so that 
	no special interpretation would be required, but I dropped that for 
	two reasons:
	
	1. since the terminal (counter) would appear in all three loop parameters 
	(initialization; condition; termination), it's code tree would have to 
	be duplicated twice -- and there is no langcopytree () routine anyway.
	
	2. given that this is a special-case loop, we can forgo all of the 
	looping code's generality and improve performance
	
	3/6/92 dmb: added inc parameter and support for downto keyword
	
	4.28.97 dmb: if look terminates naturally, loop counter should be value of 
	the statement, as it is with the c-like "loop" construct
	
	5.0.2b10 dmb: assignvalue takes care of fllanghashassignprotect
	*/
	
	register hdltreenode h = hloop;
	register hdltreenode hcounter = (**h).param3;
	long x1;
	long x2;
	
	if (!coercetolong (&val1))
		return (false);
	
	if (!coercetolong (&val2))
		return (false);
	
	for (x1 = val1.data.longvalue, x2 = val2.data.longvalue; ; x1 += inc) {
		
		copyvaluerecord (val1, valtree); // 4.28.97 dmb: leave un-incremented loop counter as return value
		
		if (inc > 0) {
			
			if (x1 > x2)
				break;
			}
		else {
			
			if (x1 < x2)
				break;
			}
		
		setlongvalue (x1, &val1);
		
		if (!assignvalue (hcounter, val1))
			return (false);
		
		cleartmpstack (); /*dealloc all outstanding temporary values*/
		
		flbreak = false;
		
		flcontinue = false;
		
		if (!evaluatelist ((**h).param4, valtree))
			return (false);
		
		flcontinue = false;
		
		assert (!leakingmemory (valtree));
		
		if (flbreak || flreturn) {
			
			flbreak = false; /*only good for one level*/
			
			return (true);
			}
		
		if (!langdebuggercall (h)) /*user killed the script*/
			return (false);
		} /*for*/
	
	return (true);
	} /*evaluateforloop*/


static boolean evaluateforinloop (hdltreenode hloop, tyvaluerecord vlist, tyvaluerecord *valtree) {
	
	/*
	3/17/93 dmb: a new loop for list processing:
		
		for counter in x
			<body>
	
	3/19/93 dmb: make work for tables too.
	
	4/14/93 dmb: don't try to coerce records to lists; leave them as is.
	
	4.1b8 dmb: fixed bug when evaluatetree within the loop returns false.
	
	4.28.97 dmb: put list "counter" value in valtree, so it's the statement value
	if nothing is return-d.

	6.1d19 AR: Make work for tables too. (Again?)

	6.1b12 AR: Fixed crashes when trying to loop over root table.
	*/
	
	hdlhashtable htable;
	register hdltreenode h = hloop;
	register long ixlist = 0;
	long ctitems;
	register hdltreenode hcounter = (**h).param2;
	boolean fltable = false;
	boolean fl = false;
	bigstring bsname;
	hdlhashnode hnode;
	
	if (vlist.valuetype == addressvaluetype) {
		
		hdlhashtable ht;
		bigstring bs;
		tyvaluerecord val;

		if (!getaddressvalue (vlist, &ht, bs))
			return (false);
	
		if (ht == nil && langgetspecialtable (bs, &htable))
			fltable = true;

		if (!fltable) {

			if (ht == nil && !langsearchpathlookup (bs, &ht)) {
				langparamerror (unknownidentifiererror, bs);
				return (false);
				}
		
			if (!langhashtablelookup (ht, bs, &val, &hnode))
				return (false);

			if (langexternalvaltotable (val, &htable, hnode))
				fltable = true;
			}
		}
	
	if (!fltable) {
	
		if (vlist.valuetype != recordvaluetype)
			if (!coercetolist (&vlist, listvaluetype))
				return (false);
		
		if (!langgetlistsize (&vlist, &ctitems))
			return (false);
		
		exemptfromtmpstack (&vlist); /*protect our list*/
		}
	
	while (true) {
		
		if (fltable) {

			if (!hashgetiteminfo (htable, ixlist++, bsname, nil)) { /* 0-based */

				fl = true;

				break;
				}

			setaddressvalue (htable, bsname, valtree);
			}

		else {

			if (++ixlist > ctitems) { /* 1-based */
			
				fl = true;
			
				break;
				}
		
			if (!langgetlistitem (&vlist, ixlist, nil, valtree)) // 4.28.97 dmb: use valtree, not local val
				break;
			}
		
		fl = assignvalue (hcounter, *valtree);
		
		if (!fl)
			break;
		
		cleartmpstack (); /*dealloc all outstanding temporary values*/
		
		flbreak = false;
		
		flcontinue = false;
		
		fl = evaluatelist ((**h).param4, valtree);
		
		if (!fl)
			break;
		
		flcontinue = false;
		
		assert (!leakingmemory (valtree));
		
		if (flbreak || flreturn) {
			
			flbreak = false; /*only good for one level*/
			
			fl = true;
			
			break;
			}
		
		
		if (!langdebuggercall (h)) /*user killed the script*/
			break;
		} /*for*/
	
	if (!fltable)
		disposetmpvalue (&vlist);
	
	return (fl);
	} /*evaluateforinloop*/

#if 0 /*support for fileloop file filtering*/

#include "strings.h"
#include "tableverbs.h"


static hdlhashtable hfiletable = nil;

static hdltreenode hpathtree = nil;

/*
static hdltreenode hfilecalltree = nil;
*/

static boolean fileidvaluecallback (hdltreenode htree, tyvaluerecord *val) {
	
	register hdltreenode h = htree;
	bigstring bsverb;
	
	if ((**h).nodetype != identifierop)
		return (false);
	
	langgetidentifier (h, bsverb);
	
	if (!hashtablesymbolexists (hfiletable, bsverb))
		return (false);
	
	return (kernelfunctionvalue (hfiletable, bsverb, hpathtree, val));
	} /*fileidvaluecallback*/


static hdltreenode hfiltertree;


static boolean fileloopfilter (bigstring bsfolder, bigstring bsfile) {
	
	/*	
	6.2b15 AR: Call coercetoboolean directly instead of the now defunct truevalue
	*/
	
	tyvaluerecord val;
	boolean fl;
	bigstring bspath;
	Handle hpath;
	
	if (hfiletable == nil) { /****need to do 1-time initialization*/
		
		initvalue (&val, stringvaluetype);
		
		if (!newconstnode (val, &hpathtree))
			return (false);
		
		findnamedtable (efptable, "\pfile", &hfiletable);
		}
	
	if (isfolderpath (bsfile))
		return (true);
	
	addstrings (bsfolder, bsfile, bspath);
	
	if (!newtexthandle (bspath, &hpath))
		return (false);
	
	(**hpathtree).nodeval.data.stringvalue = hpath;
	
	langcallbacks.idvaluecallback = &fileidvaluecallback;
	
	fl = evaluatetree (hfiltertree, &val);
	
	cleartmpstack ();
	
	langcallbacks.idvaluecallback = nil;
	
	disposehandle (hpath);
	
	if (!fl)
		return (false);
	
	if (!coercetoboolean (&val))
		return (false);
	
	return (val.data.flvalue);
	} /*fileloopfilter*/

#endif

static boolean fileloopguts (hdltreenode htree, ptrfilespec fsfolder, bigstring bsidentifier, long ctlevels, tyvaluerecord *valtree) {

	//
	// the core of fileloop evaluation, now recursive.
	//
	// if ctlevels is -1, do one level and process folders just like files.
	//
	// if ctlevels is not -1, only process files, and recurse for folders to the specified depth
	//
	// 2006-10-26 creedon: on Mac, fix a problem with CFRelease releasing data that needed to hang around
	//
	// 2006-10-03 creedon: for Mac, minimally FSRef-ized
	//
	// 1992-10-08 dmb: break & return must kick out of all levels of recursion (whew!)
	//
	
	register hdltreenode h = htree;
	register boolean fl;
	tyvaluerecord val;
	tyfilespec fs;
	boolean flfolder;
	Handle hfileloop;
	
	clearfilespec (&fs);

	
		if (!macfilespecisvalid (fsfolder)) // loop over mounted volumes
			fl = diskinitloop (nil, &hfileloop);
		else
		
	
	fl = fileinitloop (fsfolder, nil, &hfileloop);
	
	if (!fl)
		return (false);
	
	while (filenextloop (hfileloop, &fs, &flfolder)) { // get the next file in the directory
			
		if ((ctlevels != -1) && flfolder) {
			
			if (ctlevels > 0) {
				
				fl = fileloopguts (h, &fs, bsidentifier, ctlevels - 1, valtree);
				
				if (!fl)
					break;
				
				if (flbreak || flreturn)
					break;
				}
			
			continue;
			}
		
		fl = setfilespecvalue (&fs, &val);
		
		if (!fl)
			break;
		
		fl = langsetsymbolval (bsidentifier, val); // user program gets the name
		
		if (!fl)
			break;
		
		exemptfromtmpstack (&val);
		
		cleartmpstack (); // dealloc all outstanding temporary values
		
		flbreak = false; 
		
		flcontinue = false;
		
		fl = evaluatelist ((**h).param3, valtree); // run the body of the loop once
		
		flcontinue = false;
		
		if (!fl)
			break;
		
		assert (!leakingmemory (valtree));
		
		if (flbreak || flreturn)
			break;
		
		fl = langdebuggercall (h);
		
		if (!fl) // user killed the script
			break;
			
		} // while
	
	fileendloop (hfileloop);
	
	return (fl);
	
	} // fileloopguts


static boolean evaluatefileloop (hdltreenode hloop, tyvaluerecord *valtree) {
	
	/*
	7/9/90 DW: bug -- fileloops leak memory.
	
	4/24/92 dmb: removed misplaced & redundant leakingmemory check
	
	10/8/92 dmb: must clear flbreak here so that fileloopguts can leave it set 
	while unwinding recursion
	*/
	
	register hdltreenode h = hloop;
	tyfilespec fsfolder;
	bigstring bsidentifier;
	tyvaluerecord val;
	register hdltreenode hp = (**h).param2;
	long ctlevels;
	
	if (!evaluatetree (hp, &val)) /*the path is the second parameter*/
		return (false);
	
	if (!coercetofilespec (&val))
		return (false);
	
	assert (!leakingmemory (&val));
	
	fsfolder = **val.data.filespecvalue;
	
	if (!langgetidentifier ((**h).param1, bsidentifier))
		return (false);
	
	hp = (**h).param4; /*copy into register*/
	
	if (hp == nil) /*no depth clause*/
		ctlevels = -1;
	
	else {
		
		if (!evaluatetree (hp, &val))
			return (false);
		
		if (!coercetolong (&val))
			return (false);
		
		ctlevels = val.data.longvalue;
		
		if (ctlevels <= 0)
			return (true);
		
		--ctlevels;
		}
	
	langseterrorline (h); /*reset error reporting after evaulatetree calls*/
	
	if (!fileloopguts (h, &fsfolder, bsidentifier, ctlevels, valtree))
		return (false);
	
	flbreak = false; /*good for all levels of single fileloop*/
	
	return (true);
	} /*evaluatefileloop*/


static boolean evaluatecase (hdltreenode hcase, tyvaluerecord *valtree) {
	
	/*
	we must evaluate the case expression for each item, because EQvalue disposes of
	both values after it is finished doing the comparison.  this may impact the
	effect of the case statement if any of the expressions have side-effects.
	
	12/6/91 dmb: the side effect issue cannot be ignored; changed to code 
	pre-evaluate case, and work with a copy for each case item.
	
	3/6/92 dmb: changed syntax to allow empty statements lists; if match is found w/no 
	statements, bump down to next case item that has a statement list to evaluate.
	*/
	
	register hdltreenode h = hcase;
	register hdltreenode nomad;
	tyvaluerecord valcase, valcopy, valitem, valtest;
	boolean fl;
	
	if (!evaluatetree ((**h).param1, &valcase)) /*the case value is the 1st param*/
		return (false);
	
	exemptfromtmpstack (&valcase);
	
	fl = false; /*assume the worst*/
	
	nomad = (**h).param2; /*list of cases is the 2nd param*/
	
	while (true) {
		
		if (nomad == nil) { /*ran out of case items, execute else part -- if it's here*/
			
			if ((**h).param3 == nil) /*no else*/
				fl = setbooleanvalue (false, valtree); /*seems like a reasonable return*/
			else
				fl = evaluatelist ((**h).param3, valtree);
			
			break;
			}
			
		if (!copyvaluerecord (valcase, &valcopy)) /*the case value is the 1st param*/
			break;
		
		assert (!leakingmemory (&valcopy));
		
		if (!evaluatetree ((**nomad).param1, &valitem))
			break;
		
		assert (!leakingmemory (&valitem));
		
		if (!EQvalue (valcopy, valitem, &valtest)) /*also disposes of both values*/
			break;
		
		if (valtest.data.flvalue) { /*found matching case*/
			
			while ((**nomad).param2 == nil) { /*find next case item that has a body*/
				
				nomad = (**nomad).link;
				
				if (nomad == nil) { /*no body found to execute*/
					
					fl = setbooleanvalue (true, valtree); /*return true since a match was found*/
					
					goto exit;
					}
				}
			
			fl = evaluatelist ((**nomad).param2, valtree);
			
			break;
			}
		
		nomad = (**nomad).link;
		
		cleartmpstack ();
		
		assert (!leakingmemory (valtree));
		} /*while*/
	
	exit:
	
	disposevaluerecord (valcase, false);
	
	return (fl);
	} /*evaluatecase*/
	

/*
static boolean addmodulecontext (hdlhashtable htable, hdlhashnode hnode, bigstring bsname, hdltreenode htree) {
	
	/%
	add all of the top-level locals and modules to the current context
	%/
	
	register hdltreenode h;
	register tytreetype op;
	tyvaluerecord val;
	boolean fl = true;
	
	if (!langpushsourcecode (htable, hnode, bsname))
		return (false);
	
	for (h = (**htree).param1; h != nil; h = (**h).link) {
		
		op = (**h).nodetype;
		
		if ((op == localop) || (op == moduleop)) {
			
			if (!evaluatetree (h, &val)) {
				
				fl = false;
				
				break;
				}
			}
		}
	
	
	langpopsourcecode ();
	
	return (fl);
	} /%addmodulecontext%/
*/


static long langgetlexicalrefcon (void) {
	
	/*
	2/12/92 dmb: a bit of repeated code.  in fact, there may be potential 
	external callers of this routine.
	
	3.0.2 dmb: return -1 if no error callback/refcon has been pushed
	*/
	
	register hdlerrorstack hs;


		if (langcallbacks.scripterrorstack == nil)
			return (-1);
	
		
	hs = langcallbacks.scripterrorstack;

	
	
		if (hs == nil)
			return (-1);
	
		if ((long) (*hs) == -1)
			return (-1);
	
	
	if ((hs == nil) || ((**hs).toperror == 0)) {

		return (-1);
		}
	else {

		
			if ((**hs).stack == nil)
				return (-1);
				
			if ((**hs).toperror < 1)
				return (-1);
				
			if ((**hs).toperror > cterrorcallbacks)
				return (-1);
		
		return ((**hs).stack [(**hs).toperror - 1].errorrefcon);
		}
	} /*langgetlexicalrefcon*/


static boolean evaluatewith (hdltreenode hwith, tyvaluerecord *valtree) {
	
	/*
	evaluate a with block.  hparam1 should be a list of terminals that 
	specify tables to add to the search path for the with block.
	
	8/31/92 dmb: added supports for multiple with items, object specifiers
	*/
	
	register hdltreenode h = hwith;
	register hdltreenode hterm;
	hdlhashtable htable;
	bigstring bs;
	hdlhashtable hlocaltable;
	register hdlhashtable ht;
	tyvaluerecord valtable;
	tyvaluerecord valwith;
	short n = 0;
	hdlhashnode hnode;
	
	if (!newhashtable (&hlocaltable)) /*new table for the function when it runs*/
		return (false);
	
	ht = hlocaltable; /*copy into register*/
	
	(**ht).fllocaltable = true;  // 5.1.4 dmb: set now so pre-assignments will know locality of table

	(**ht).lexicalrefcon = langgetlexicalrefcon (); /*'with' expressions use local scope*/
	
	chainhashtable (ht); /*need it in scope to handle multiple items in list*/
	
	/*populate the local table with path values*/
	
	for (hterm = (**h).param1; hterm != nil; hterm = (**hterm).link) {
		
		if (isobjspectree (hterm)) {
			
			if (!evaluateobjspec (hterm, &valwith))
				goto error;
			
			copystring (bscontainername, bs);
			}
		else {
			
			if (!langgetdottedsymbolval (hterm, &htable, bs, &valtable, &hnode))
				goto error;
			
			/*
			if (langexternalvaltocode (valtable, &hcode)) { /%5/14/93 dmb%/
				
				if (!hashtablelookupnode (htable, bs, &hnode))
					goto error;
				
				if (hcode == nil) {
					
					if (!langcompilescript (hnode, &hcode))
						goto error;
					}
				
				if (!addmodulecontext (htable, hnode, bs, hcode))
					goto error;
				
				continue;
				}
			*/
			
			if (!langexternalvaltotable (valtable, &htable, hnode)) {
				
				langparamerror (badwithstatementerror, bs);
				
				goto error;
				}
			
			if (!setaddressvalue (htable, zerostring, &valwith))
				goto error;
			
			if (n == 7) { /*maximum value of ctwithvalues*/
				
				langlongparamerror (toomanywithtableserror, n);
				
				goto error;
				}
			
			langgetwithvaluename (++n, bs);
			
			(**ht).ctwithvalues = n; /*optimization for langfindsymbol*/
			}
		
		if (!hashtableassign (ht, bs, valwith)) {
			
			disposevaluerecord (valwith, false);
			
			goto error;
			}
		
		exemptfromtmpstack (&valwith); /*its in the local table now*/
		}
	
	unchainhashtable ();
	
	hmagictable = ht; /*evaluatelist uses this as its local symbol table*/
	
	return (evaluatelist ((**h).param2, valtree));
	
	error: {
		
		unchainhashtable ();
		
		disposehashtable (ht, false);
		
		return (false);
		}
	} /*evaluatewith*/


Handle tryerror = nil;

Handle tryerrorstack = nil;  //This code is left in so process.c, etc do not have to change


#if fltryerrorstackcode

	static boolean pusherrorstackitem (bigstring bstitle, bigstring bsname, unsigned long errorline, unsigned short errorchar) {
	#ifdef NEVER
		bigstring bsline, bschar;
		long len, offset;
		Handle h;
		boolean fl;

		numbertostring ((long) errorline, bsline);
		numbertostring ((long) errorchar, bschar);
		// "{\"" + bstitle + "\",\"" + bsname + "\"," + bsline + "," + bschar + "}"
		//  2 + stringlength (bstitle) + 3 + stringlength (bsname) + 2 + stringlength (bsline) + 1 + stringlength (bschar) + 1
		len = 2 + stringlength (bstitle) + 3 + stringlength (bsname) + 2 + stringlength (bsline) + 1 + stringlength (bschar) + 1;
		
		if (! newhandle (len, &h))
			return (false);
		
		moveleft ("{\"", *h, 2);
		offset = 2;
		moveleft ((ptrstring) stringbaseaddress(bstitle), *h+offset, stringlength(bstitle));
		offset += stringlength (bstitle);
		moveleft ("\",\"", *h+offset, 3);
		offset += 3;
		moveleft ((ptrstring) stringbaseaddress(bsname), *h+offset, stringlength(bsname));
		offset += stringlength (bsname);
		moveleft ("\",", *h+offset, 2);
		offset += 2;
		moveleft ((ptrstring) stringbaseaddress(bsline), *h+offset, stringlength(bsline));
		offset += stringlength (bsline);
		moveleft (",", *h+offset, 1);
		offset += 1;
		moveleft ((ptrstring) stringbaseaddress(bschar), *h+offset, stringlength(bschar));
		offset += stringlength (bschar);
		moveleft ("}", *h+offset, 1);

		fl = insertinhandle (tryerrorstack, gethandlesize(tryerrorstack), *h, len);
		
		disposehandle (h);
		
		return (fl);
	#endif		

	#ifdef tryerrorstackusinglists
		hdllistrecord hnew;
		hdllistrecord htry;
		tyvaluerecord val;

		htry = (hdllistrecord) tryerrorstack;

		if (tryerrorstack == nil)
			if (! opnewlist (&htry, false))
				return (false);
			else
				tryerrorstack = (Handle) htry;

		if (! opnewlist (&hnew, false))
			return (false);

		if (! langpushliststring (hnew, bstitle))
			goto error;

		if (! langpushliststring (hnew, bsname))
			goto error;

		if (! langpushlistlong (hnew, (long)errorline))
			goto error;

		if (! langpushlistlong (hnew, (long)errorchar))
			goto error;

		if (! setheapvalue ((Handle) hnew, listvaluetype, &val))
			goto error;

		if (langpushlistval (htry, nil, &val))
			return (true);
		
		disposevaluerecord (val, false);
		return (false);

	error:
		opdisposelist (hnew);

		return (false);
	#endif
	return (true);
		} /*pusherrorstackitem*/


	static boolean langsettryerrorstack () {

		register hdlerrorstack hs = langcallbacks.scripterrorstack;
		register short ix;
		register short ixtop;
		hdlhashtable htable;
		hdlwindowinfo hparent;
		bigstring bsname;
		bigstring bstitle;
		tyerrorrecord *pe;
		
		if (hs == nil)
			return (false);
		
		ixtop = (**hs).toperror;
		
	//	if (! newtexthandle ("\x01" "{", &tryerrorstack))
	//		return (false);
		
		for (ix = ixtop - 1; ix >= 0; --ix) {
			
			pe = &(**hs).stack [ix];
			
			if ((*pe).errorcallback == nil ||
				!(*(*pe).errorcallback) ((*pe).errorrefcon, 0, 0, &htable, bsname)) {
			
				langgetstringlist (anomynousthreadstring, bsname); 
				langgetstringlist (anomynousthreadstring, bstitle); 
				}
			else {
				if (!langexternalgetfullpath (htable, bsname, bstitle, &hparent))
					langgetstringlist (anomynousthreadstring, bstitle);
				}

			
			if (!pusherrorstackitem (bstitle, bsname, (*pe).errorline, (*pe).errorchar)) { /*terminate visit on error*/
				disposehandle (tryerrorstack);
				return (false);
				}
				
	//		if (ix > 0)
	//			pushtexthandle ("\x01" ",", tryerrorstack);
	//		else
	//			pushtexthandle ("\x01" "}", tryerrorstack);
				
			}
			
		return (true);
		} /*langsettryerrorstack*/
#endif

static boolean langtryerror (bigstring bsmsg, ptrvoid refcon) {
#pragma unused (refcon)

	/*
	6/25/92 dmb: when an error occurs during a try block, we stash it in 
	the tryerror handle.  it is later placed in the stack frame of the 
	else statement, if it exists, by evaluatelist
	*/
	
	assert (tryerror == nil);
	
	newtexthandle (bsmsg, &tryerror); /*if out of mem, script won't be able to get error*/
	
	#if fltryerrorstackcode
		assert (tryerrorstack == nil);
		
		langsettryerrorstack();
	#endif

	return (true);
	} /*langtryerror*/


static boolean evaluatetry (hdltreenode htry, tyvaluerecord *valtree) {
	
	register hdltreenode h = htry;
	boolean fl;
	langerrormessagecallback savecallback;
	
	assert (tryerror == nil);

	#if fltryerrorstackcode
		assert (tryerrorstack == nil);
	#endif
	
	savecallback = langcallbacks.errormessagecallback;
	
	langcallbacks.errormessagecallback = &langtryerror;
	
	fl = evaluatelist ((**h).param2, valtree);
	
	langcallbacks.errormessagecallback = savecallback;
	
	if (!fllangerror) {
		
		assert (tryerror == nil);

		#if fltryerrorstackcode
			assert (tryerrorstack == nil);
		#endif		

		return (fl); /*might be false if script has been killed*/
		}
	
	fllangerror = false; /*recover*/
	
	h = (**h).param3;
	
	if (h == nil) {
		
		disposehandle (tryerror);
		
		tryerror = nil;
		
		#if fltryerrorstackcode
			opdisposelist ((hdllistrecord) tryerrorstack);
	//		disposehandle (tryerrorstack);
			
			tryerrorstack = nil;
		#endif
		
		return (true);
		}

	//assert (tryerror != nil); //6.1b8 AR: attempt to catch "tryerror not defined" situations
	
	return (evaluatelist (h, valtree)); /*will take care of tryerror automatically*/
	} /*evaluatetry*/


static boolean langaddlocals (hdltreenode hnode) {
	
	/*
	add the names in hnamelist to the most-local symbol table.  
	
	return false if the symbol is already declared in the local table, or 
	if there was a memory allocation error.
	
	1/17/91 dmb: simplified code and moved tmpstack logic from assignlocalop 
	clause to bottom of loop, so on error val is still in tmpstack and will 
	be disposed later
	
	xxx 12/11/92 dmb: set unassigned locals to novaluetype, not zero (long). this 
	xxx allows them to more easily be used in expressions and yield expected results.
	
	2.1b2 dmb: added debugger call for local initial assignments
	
	4.1b4 dmb: added fllangexternalvalueprotect flag to disable protection
	
	4.28.97 dmb: make sure assignments copy tmpdata
	*/
	
	register hdltreenode nomad = (**hnode).param1;
	
	while (true) { /*step through name list, inserting each into symbol table*/
		
		tyvaluerecord val;
		bigstring bs;
		
		if (nomad == nil) /*reached the end of the names list*/
			return (true);
		
		if ((**nomad).nodetype == assignlocalop) {
			
			if (!langdebuggercall (nomad)) /*2.1b2*/
				return (false);
			
			if (!evaluatetree ((**nomad).param2, &val))
				return (false);
			
			if (fllangexternalvalueprotect && val.valuetype == externalvaluetype) { /*4.1b4 dmb*/
				
				langbadexternaloperror (externalassignerror, val);
				
				return (false);
				}
			
			if (!langgetidentifier ((**nomad).param1, bs))
				return (false);
			}
		else {
			
			initvalue (&val, novaluetype);
			
			if (!langgetidentifier (nomad, bs))
				return (false);
			}
			
		if (hashsymbolexists (bs)) { /*multiply-defined symbol*/
			
			langseterrorline (nomad); /*point right at the offending name*/
		
			langparamerror (multiplesymbolerror, bs);
			
			return (false);
			}
		
		// 4.28.97 dmb: do what hashassign does with tmpdata. shouls hashinsert do this?
		
		if (val.fltmpdata) { /*val doesn't own it's data*/
			
			if (!copyvaluedata (&val))
				return (false);
			
			exemptfromtmpstack (&val); /***should wait until success*/
			}
		
		hashsetlocality (&val, true); /*6.2b16 AR*/
		
		if (!hashinsert (bs, val)) /*error creating new symbol*/
			return (false);
		
		exemptfromtmpstack (&val); /*it's been successfully added to local table*/
		
		cleartmpstack (); /*dealloc all outstanding temporary values*/	
		
		nomad = (**nomad).link; /*advance to next name in list*/
		} /*while*/
	} /*langaddlocals*/


static boolean langaddhandler (hdltreenode hnode) {
	
	/*
	add a handler node to the current hashtable.  the structure below hnode
	must conform to the structure documented in the comment at the head of
	langfunccall.
	
	2/5/91 dmb: stuff the current scripterrorrefcon into the (otherwise unused) 
	nodeval so that the script debugger and error dialog can trace handler 
	calls back to their source.  this would otherwise be quite difficult to 
	do, since the address of a local handler can be passed around between 
	scripts and called from anywhere.  see scriptpushsourcecode in scripts.c.
	*/
	
	register hdltreenode h = hnode;
	bigstring bs;
	tyvaluerecord val;
	
	if (!langgetidentifier ((**(**h).param2).param1, bs))
		return (false);
	
	if (hashsymbolexists (bs)) { /*name already defined in most-local table*/
		
		langseterrorline (h); /*point right at the offending name*/
		
		langparamerror (multiplesymbolerror, bs);
		
		return (false);
		}
	
	initvalue (&val, codevaluetype);
	
	val.data.codevalue = h;
	
	val.fltmpdata = true; /*data belong to code tree*/
	
	if (!hashinsert (bs, val))
		return (false);
	
	bundle { /*link this code value to the source from whence it came*/
		
		initvalue (&val, longvaluetype);
		
		val.data.longvalue = langgetlexicalrefcon ();
		
		(**h).nodeval = val;
		}
	
	return (true);
	} /*langaddhandler*/


static boolean needassignmentresult (hdltreenode hp) {
	
	/*
	5.0.2b21 dmb: new rule: we need the result of an assingnment if it's the 
	last statement in a body _and_ it's in the outermost scope of the script.
	
	5.1.1 dmb: removed new rule of 5.0.2b21; too many broken scripts
	*/
	
	//register hdlerrorstack hs = langcallbacks.scripterrorstack;
	
	if ((**hp).link != nil)
		return (false);
	
	/*
	if ((hs != nil) && ((**hs).toperror > 1))
		return (false);
	*/
	
	return (true);
	} /*needassignmentresult*/


#define newparams

#pragma pack(2)
	typedef struct typaraminfo {
		
		byte evalparam1;
		
		byte evalparam2;
		} typaraminfo;
#pragma options align=reset
	
	#define nope ((byte) -1)
	
	static typaraminfo paraminfolist [cttreetypes] = {
		
		{nope, nope},	/*noop*/
		
		{true, true},	/*addop*/
		
		{true, true},	/*subtractop*/
		
		{true, true},	/*multiplyop*/
		
		{true, true},	/*divideop*/
		
		{true, true},	/*modop*/
		
		{nope, nope},	/*identifierop*/
		
		{nope, nope},	/*constop*/
		
		{true, nope},	/*unaryop*/
		
		{true, nope},	/*notop*/
		
		{false, true},	/*assignop*/
		
		{false, false},	/*functionop*/
		
		{true, true},	/*EQop*/
		
		{true, true},	/*NEop*/
		
		{true, true},	/*GTop*/
		
		{true, true},	/*LTop*/
		
		{true, true},	/*GEop*/
		
		{true, true},	/*LEop*/
		
		{true, false},	/*ororop*/
		
		{true, false},	/*andandop*/
		
		{false, nope},	/*incrpreop*/ /* ++x */
		
		{false, nope},	/*incrpostop*/ /* x++ */
		
		{false, nope},	/*decrpreop*/ /* --x */
		
		{false, nope},	/*decrpostop*/ /* x-- */
		
		{true, false},	/*loopop*/
		
		{false, false},	/*fileloopop*/
		
		{true, true},	/*forloopop*/
		
		{nope, nope},	/*breakop*/
		
		{true, nope},	/*returnop*/
		
		{false, nope},	/*bundleop*/
		
		{true, false},	/*ifop*/
		
		{nope, nope},	/*procop*/
		
		{false, nope},	/*localop*/
		
		{false, false},	/*moduleop*/
		
		{false, false},	/*dotop*/
		
		{false, false},	/*arrayop*/
		
		{false, nope},	/*addressofop*/
		
		{false, nope},	/*dereferenceop*/
		
		{true, true},	/*assignlocalop*/
		
		{nope, nope},	/*bracketop*/
		
		{false, false},	/*caseop*/
		
		{nope, nope},	/*caseitemop*/ 
		
		{nope, nope},	/*casebodyop*/
		
		{nope, nope},	/*kernelop*/
		
		{nope, nope},	/*continueop*/
		
		{false, false},	/*withop*/
		
		{true, true},	/*fordownloopop*/
		
		{false, false},	/*tryop*/
		
		{true, true},	/*beginswithop*/
		
		{true, true},	/*endswithop*/
		
		{true, true},	/*containsop*/
		
		{true, true},	/*rangeop*/
		
		{false, nope},	/*listop*/
		
		{nope, nope},	/*fieldop*/
		
		{false, nope},	/*recordop*/
		
		{true, false},	/*forinloopop*/
		
		{false, nope},	/*globalop*/
		
		{true, nope},	/*osascriptop*/
		
		{false, true},	/*addvalueop*/
		
		{false, true},	/*subtractvalueop*/
		
		{false, true},	/*multiplyvalueop*/
		
		{false, true}	/*dividevalueop*/
		};


static boolean evaltree (hdltreenode htree, tyvaluerecord *valtree) {
	
	/*
	1/16/91 dmb: set default return value so we don't pass garbage 
	back to caller
	
	2/1/91 dmb: don't call the debugger on a return op.  I'm not sure why 
	this was ever done here, but it causes the debugger to step on return 
	statements twice.
	
	2/1/91 dmb: bracketop should be treated just like identifierop.  used 
	to be missed, resulting in false return w/no error reported.  added 
	unexpected opcode error message in case this happens again.
	
	9/4/91 dmb: fixed potential memory leak of val1 while evaluating val2. 
	also, updated handling of ororop and andandop, which now do short-circuit
	evaluation.
	
	12/26/91 dmb: in assignop, don't set valtree is assignment fails
	
	5/29/92 dmb: added try statement handling

	5.0b7 dmb: for returnop, actually pust externals on the temp stack.
	really, at this point, we should treat externals like any other 
	heap-allocated value and use the temp stack all the time, not the 
	fltmpdata hack from before we beefed up the temp stack for lists. but
	we're too close to shipping to do that now.
	
	5.0b17 dmb: for assignop, we have the same issue as returnop. but for
	this implicit return value, don't copy externs; just return true. also
	make sure val1 goes back onto the temp stack if it was there.
	
	5.0.2b10 dmb: for returnop, don't copy the already-temp val1. for assignop,
	only return the value as a side effect if htree is the last in its list.
	
	5.0.2b12 dmb: don't push external on tmpstack if it's already there
	
	5.0.2b21 dmb: assignop, returnop check fltmpdata to handle codetype too. probably
	don't need to check to externalvaluetype anymore in assignop.
	
	6.2b15 AR: For ifop, call coercetoboolean directly instead of the now defunct truevalue
	*/
	
	register hdltreenode h = htree;
	register tytreetype op;
	register short ctparams;
	tyvaluerecord val1, val2;
	
	setbooleanvalue (true, valtree); /*default returned value*/
	
	if (fllangerror) { /*a language error dialog has appeared, unwind*/
		
		return (false); /*return false, aid in the unwinding process*/
		}
	
	if (h == nil)
		return (true);
	
	op = (**h).nodetype; /*copy into register*/
	
	ctparams = (**h).ctparams;
	
	if (ctparams > 0) {
		
		#ifdef newparams
		
		typaraminfo info = paraminfolist [op];
		
		#endif
		
		initvalue (&val1, novaluetype); /*so we can indiscriminantly pass to tmpstack routines*/
		
		#ifdef newparams
		
		if (info.evalparam1)
		
		#else
		
		if (evalparam1 (op))
		
		#endif
			{
			if (!evaluatetree ((**h).param1, &val1))
				return (false);
			
			assert (!leakingmemory (&val1));
			
			if (flreturn)
				return (true);
			}
		
		if (ctparams > 1) {
			
			#ifdef newparams
			
			if (info.evalparam2)
			
			#else
			
			if (evalparam2 (op))
			
			#endif
				{
				boolean fl, fltmp;
				
				fltmp = exemptfromtmpstack (&val1); /*if still novaluetype, does nothing*/
				
				fl = evaluatetree ((**h).param2, &val2);
				
				if (fltmp)
					pushtmpstackvalue (&val1); /*5.0b17: use this call to make sure it goes back*/
				
				if (!fl)
					return (false);
				
				assert (!leakingmemory (&val2));
				
				assert (validvalue (&val1));
				
				if (flreturn)
					return (true);
				}
			}
		}
	
	langseterrorline (h); /*set globals for error reporting*/
	
//printf ("evaltree: op = %d/n", op);
	
	switch (op) {
		
		case noop:
			return (true); /*noop's are very agreeable*/
		
		case localop:
			return (langaddlocals (h));
			
		case moduleop:
			return (langaddhandler (h));
		
		case identifierop:
		case bracketop:
			return (idvalue (h, valtree));
		
		case dotop:
			return (dotvalue (h, valtree));
			
		case addressofop:
			return (addressofvalue ((**h).param1, valtree));
		
		case dereferenceop:
			return (dereferencevalue ((**h).param1, valtree));
		
		case arrayop:
			return (arrayvalue (h, valtree));
		
		case constop:
			return (copyvaluerecord ((**h).nodeval, valtree));
			
		case assignop:
			if (!assignvalue ((**h).param1, val2))
				return (false);
			
		//	*valtree = val2;
		//	
		//	return (true);
			
			if ((val2.valuetype == externalvaluetype) || val2.fltmpdata || !needassignmentresult (h))
				return (setbooleanvalue (true, valtree)); /*could be a local extern*/
			else
				return (copyvaluerecord (val2, valtree)); /*side-effect of assignment*/
		
		case functionop:
			return (functionvalue ((**h).param1, (**h).param2, valtree));
		
		case addop:
			return (addvalue (val1, val2, valtree));	
		
		case subtractop:
			return (subtractvalue (val1, val2, valtree));
		
		case unaryop:
			return (unaryminusvalue (val1, valtree));
		
		case multiplyop:
			return (multiplyvalue (val1, val2, valtree));
		
		case divideop:
			return (dividevalue (val1, val2, valtree));
		
		case addvalueop:			
			return (modifyassignvalue ((**h).param1, val2, addop, valtree, needassignmentresult (h)));
		
		case subtractvalueop:
			return (modifyassignvalue ((**h).param1, val2, subtractop, valtree, needassignmentresult (h)));
		
		case multiplyvalueop:
			return (modifyassignvalue ((**h).param1, val2, multiplyop, valtree, needassignmentresult (h)));
		
		case dividevalueop:
			return (modifyassignvalue ((**h).param1, val2, divideop, valtree, needassignmentresult (h)));
		
		case modop:
			return (modvalue (val1, val2, valtree));
		
		case notop:
			return (notvalue (val1, valtree));
			
		case EQop:
			return (EQvalue (val1, val2, valtree));
		
		case NEop:
			return (NEvalue (val1, val2, valtree));
			
		case GTop:
			return (GTvalue (val1, val2, valtree));
			
		case LTop:
			return (LTvalue (val1, val2, valtree));
		
		case GEop:
			return (GEvalue (val1, val2, valtree));
		
		case LEop:
			return (LEvalue (val1, val2, valtree));
		
		case beginswithop:
			return (beginswithvalue (val1, val2, valtree));
		
		case endswithop:
			return (endswithvalue (val1, val2, valtree));
		
		case containsop:
			return (containsvalue (val1, val2, valtree));
		
		case ororop:
			return (ororvalue (val1, (**h).param2, valtree));
			
		case andandop:
			return (andandvalue (val1, (**h).param2, valtree));
		
		case breakop:
			/*
			if (!langdebuggercall (h)) /%user killed the script%/
				return (false);
			*/
			
			flbreak = true; /*set global*/
			
			return (true); /*keep surfacing until someone "catches" it*/
		
		case continueop:
			flcontinue = true; /*set global*/
			
			return (true);
		
		case withop:
			return (evaluatewith (h, valtree));
		
		case returnop:
			/*
			if (!langdebuggercall (h)) /%user killed the script%/
				return (false);
			*/
			
			flreturn = true; /*set global*/
			
			if (fllangexternalvalueprotect && val1.valuetype == externalvaluetype) { /*4.1b4 dmb*/
				
				langbadexternaloperror (externalreturnerror, val1); /*10/25*/
				
				return (false);
				}
			
				
				*valtree = val1;
				
				if (val1.fltmpdata) {
				
					if (!copyvaluedata (valtree))
						return (false);
					
					if (!(*valtree).fltmpstack)
						pushtmpstackvalue (valtree);
					}
			

			if ((*valtree).valuetype == novaluetype) { /*return () -- no value provided*/
				
				setbooleanvalue (true, valtree);
				}
			
			return (true);
		
		case bundleop:
			return (evaluatelist ((**h).param1, valtree));
			
		case ifop:		
			if (!coercetoboolean (&val1))
				return (false);
			
			if (val1.data.flvalue) 
				h = (**h).param2;
			else 
				h = (**h).param3;
			
			if (h == nil)
				return (true);
				
			return (evaluatelist (h, valtree));
			
		case caseop:
			return (evaluatecase (h, valtree));
			
		case loopop: 
			return (evaluateloop (h, valtree));
		
		case fileloopop: 
			return (evaluatefileloop (h, valtree));
		
		case forloopop:
			return (evaluateforloop (h, val1, val2, 1, valtree));
		
		case fordownloopop:
			return (evaluateforloop (h, val1, val2, -1, valtree));
		
		case incrpreop:
			return (incrementvalue (true, true, (**h).param1, valtree));
			
		case incrpostop:
			return (incrementvalue (true, false, (**h).param1, valtree));
			
		case decrpreop:
			return (incrementvalue (false, true, (**h).param1, valtree));
			
		case decrpostop:
			return (incrementvalue (false, false, (**h).param1, valtree));
		
		case tryop:
			return (evaluatetry (h, valtree));
		
		case rangeop:
			langerror (badrangeoperationerror);
			
			return (false);
		
		case fieldop:
			langerror (badfieldoperationerror);
			
			return (false);
		
		case listop:
			return (makelistvalue ((**h).param1, valtree));
		
		case recordop:
			return (makerecordvalue ((**h).param1, false, valtree));
		
		case forinloopop:
			return (evaluateforinloop (h, val1, valtree));
		
		/*
		case globalop:
			return (langaddlocals (h, true));
		*/
		
		#ifdef flcomponent
		case osascriptop:
			if (isosascriptnode (h, &val1))
				return (evaluateosascript (&val1, nil, zerostring, valtree));
		#endif
		
		default:
			/* do nothing for procop, assignlocalop, caseitemop, casebodyop, kernelop, globalop */
			break;
		} /*switch*/
	
	langlongparamerror (unexpectedopcodeerror, (long) op);
	
	return (false); /*unimplemented opcode*/
	} /*evaltree*/


boolean evaluatetree (hdltreenode htree, tyvaluerecord *valtree) {
	
	/*
	7/25/92 dmb: added this wrapper to allow fllangerror to be checked 
	every time.
	
	9/1/92 dmb: added stack overflow detection code
	*/
	
	if (!langcheckstackspace ())
		return (false);
	
	return (evaltree (htree, valtree) && !fllangerror);
	} /*evaluatetree*/



boolean evaluatelist (hdltreenode hfirst, tyvaluerecord *val) {
	
	/*
	this is something like the main-event-loop for CanCoon's interpreter.
	
	chain through a list of statements, evaluating each one and then advance
	to the next.  the value we return is the value generated by the last
	statement in the list.
	
	we allow an external caller to hand us a pre-stuffed symbol table through the
	global hmagictable.  we take care of chaining it into the runtime stack, and
	releasing it before we exit.  the global is reset to nil, so that it has to
	be reset on every use.  
	
	7/10/90 DW: allocate a local table for every level -- it's really cheap in
	time, and also cheap in space.  this allows automatic locals to be reliably
	allocated in the local space, and may help in the future in making other
	things work.  now there is one table in the chain for every level, even if
	there are no local variables or local handlers.
	
	7/23/90 DW: we have to protect the returned value from being deallocated
	as part of the local list's tmp stack.  if it's a string, the caller will
	get a garbage handle.  so we move the value from the local tmpstack into
	the next-most-global tmpstack.
	
	9/4/90 DW: Major rewrite -- wrote the Ultimate SuperStresserª script, and
	it works!
	
	9/4/91 dmb: on break and return, make sure langerror isn't missed
	
	9/23/91 dmb: magic table handling is now buried a little deeper -- in 
	newhashtable.
	
	2/12/92 dmb: set lexicalrefcon when pushing local frame.  see langfindsymbol
	
	9/1/92 dmb: added decent stack overflow detection/handling code
	
	9/27/92 dmb: added languserescaped (false) call inside of loop
	
	2.1b2 dmb: added langbadexternaloperror check after each evaluatetree

	11/13/01 dmb: try lazy with evaluation
	*/
	
	register hdltreenode programcounter = hfirst;
	register boolean fl = false;
	hdlhashtable hlocals; 
	boolean fltmpval;
	boolean flhavelocals, flneedlocals, flneedthis;
	hdlhashtable hthis;
	bigstring bsthis;


	setbooleanvalue (false, val); /*default returned value*/
	
	if (!langcheckstackspace ())
		return (false);
	
	flhavelocals = (**currenthashtable).fllocaltable;
	
	flneedthis = !flhavelocals && (hmagictable == nil);
	
	#if fltryerrorstackcode
		flneedlocals = !flhavelocals || (hmagictable != nil) || (tryerror != nil) || (tryerrorstack != nil);
	#else
		flneedlocals = !flhavelocals || (hmagictable != nil) || (tryerror != nil);
	#endif

	if (!flneedlocals) { /*pre-scan statement list to see if we need a local frame*/
		
		register hdltreenode h;
		register tytreetype op;
		
		for (h = programcounter; h != nil; h = (**h).link) {
			
			op = (**h).nodetype;
			
			if ((op == localop) || (op == moduleop)) {
				
				flneedlocals = true;
				
				break;
				}
			}
		}
	
	if (flneedlocals) {
		
		if (!langpushlocalchain (&hlocals))
			return (false);
		
		(**hlocals).lexicalrefcon = langgetlexicalrefcon ();
		
		if (flneedthis) {
				if (langgetthisaddress (&hthis, bsthis))
					langsetthisvalue (hlocals, hthis, bsthis);
			}
		
		if (tryerror != nil) {
			
			tyvaluerecord errorval;
			
			if (setheapvalue (tryerror, stringvaluetype, &errorval))
				if (hashassign (nametryerrorval, errorval))
					exemptfromtmpstack (&errorval);
			
			tryerror = nil;
			}

		#if fltryerrorstackcode
			if (tryerrorstack != nil) {
				
				tyvaluerecord errorval;
				
	//			if (setheapvalue (tryerrorstack, listvaluetype, &errorval))
				if (setheapvalue (tryerrorstack, stringvaluetype, &errorval))
					if (hashassign (nametryerrorstackval, errorval))
						exemptfromtmpstack (&errorval);
				
				tryerrorstack = nil;
				}
		#endif

		}
	else
		hlocals = currenthashtable;
	
	while (true) { /*visit each statement in the statement list*/
		
		if (fllangerror) /*a language error dialog has appeared, unwind*/
			break;
		
		if (programcounter == nil) { /*reached the end of the list*/
			
			fl = true; /*don't halt the interpreter*/
			
			break;
			}
		
		cleartmpstack (); /*dealloc all outstanding temporary values*/
		
		langseterrorline (programcounter); /*set globals for error reporting*/
		
		if (languserescaped (false)) /*user killed the script*/
			break;
		
		if (!langdebuggercall (programcounter)) /*user killed the script*/
			break;
		
		if (!evaluatetree (programcounter, val))
			break;
		
		#if defined(fldebug) && (fldebug > 1)
			assert (validvalue (val));
			
			assert (!leakingmemory (val));
		#endif
		
		if (flbreak || flreturn || flcontinue) {
			
			fl = !fllangerror; /*don't halt the interpreter, except on error*/
			
			break;
			}
		
		programcounter = (**programcounter).link; /*advance to next statement*/
		} /*while*/
	
	/*
	1/31/97 dmb: below is the site of a major osamenusharing bug. It can fail!
	I've seen it myself. But it's also been reported by Timothy Paustian 
	<paustian@bact.wisc.edu> [About the Frontier menu in Web Warrior], Tattoo Mabonzo K. 
	<vip052@pophost.eunet.be> [db.save verb] and at least one more person.

	5.0b18 dmb: if this does trigger, make some attempt to exit cleanly
	*/
	
	if (hlocals != currenthashtable) { /*should never happen*/

		assert (hlocals == currenthashtable); /*context change in background destroyed our state*/
		
		langerror (undefinederror);
		
		currenthashtable = hlocals;
		
		fl = false;
		}
	
	/*finished processing list, either natural termination, a break, return or error*/
	
	assert (!fl || !leakingmemory (val));
	
	fltmpval = exemptfromtmpstack (val); /*must survive disposing of local table & background task*/
	
	if (flneedlocals) /*pop the runtime stack*/
		
		langpoplocalchain (hlocals);
	
	if (fl) /*give agents a shot while val is exempt from temp*/
		
		fl = langbackgroundtask (false); /*background task can cause termination*/
	
	if (fltmpval)
		pushtmpstackvalue (val); /*insert into the next-most-global tmpstack*/
	
	if (!fl) { /*failure of some sort -- return immediately*/
		
		if (flstackoverflow) { /*error was stack overflow*/
			
			langcheckstacklimit (idprogramstack, 1, 0); /*report now that stack has been popped a bit */
			
			flstackoverflow = false; /*it's been reported*/
			}
		
		return (false);
		}
	
	if (languserescaped (true)) /*user pressed cmd-period, unwind recursion -- quickly*/
		return (false);
	
	return (true); /*fell through the bottom of the list*/
	} /*evaluatelist*/

