
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

#include "memory.h"
#include "frontier_strings.h"
#include "error.h"
#include "lang.h"
#include "langinternal.h"
#include "langipc.h"
#include "langsystem7.h"
#include "oplist.h"
#include "op.h"


boolean langgetlistsize (const tyvaluerecord *vlist, long *size) {

	*size = opcountlistitems ((*vlist).data.listvalue);

	return (true);
	} /*langgetlistsize*/


boolean getnthlistval (hdllistrecord hlist, long n, ptrstring pkey, tyvaluerecord *val) {
	
	Handle hdata;
	
	if (!opgetlisthandle (hlist, n, pkey, &hdata))
		return (false);
	
	if (!langunpackvalue (hdata, val))
		return (false);
	
	pushvalueontmpstack (val);
	
	return (true);
	} /*getnthlistval*/


boolean setnthlistval (hdllistrecord hlist, long n, ptrstring pkey, tyvaluerecord *val) {
	
	Handle hdata;
	
	if (!langpackvalue (*val, &hdata, HNoNode))
		return (false);
	
	if (n == 0) // AE list compatibility thing
		return (oppushhandle (hlist, pkey, hdata));
	else
		return (opsetlisthandle (hlist, n, pkey, hdata));
	} /*setnthlistval*/


boolean langpushlistval (hdllistrecord hlist, ptrstring pkey, tyvaluerecord *val) {
	
	Handle hdata;
	
	if (!langpackvalue (*val, &hdata, HNoNode))
		return (false);
	
	return (oppushhandle (hlist, pkey, hdata));
	} /*langpushlistval*/


boolean langpushlisttext (hdllistrecord hlist, Handle hstring) {
	
	Handle hdata;
	tyvaluerecord val;
	boolean fl;
	
	initvalue (&val, stringvaluetype);
	
	val.data.stringvalue = hstring;
	
	fl = langpackvalue (val, &hdata, HNoNode);
	
	disposehandle (hstring);
	
	if (fl)
		fl = oppushhandle (hlist, nil, hdata);
	
	return (fl);
	} /*langpushlisttext*/


boolean langpushliststring (hdllistrecord hlist, bigstring bs) {
	
	Handle htext;
	
	if (!newtexthandle (bs, &htext))
		return (false);
	
	return (langpushlisttext (hlist, htext));
	} /*langpushliststring*/


boolean langpushlistaddress (hdllistrecord hlist, hdlhashtable ht, bigstring bs) {
	
	Handle hdata;
	tyvaluerecord val;
	boolean fl;
	
	if (!setexemptaddressvalue (ht, bs, &val))
		return (false);
	
	fl = langpackvalue (val, &hdata, HNoNode);
	
	disposevaluerecord (val, true);
	
	if (fl)
		fl = oppushhandle (hlist, nil, hdata);
	
	return (fl);
	} /*langpushlistaddress*/


boolean langpushlistlong (hdllistrecord hlist, long num) {
	Handle hdata;
	tyvaluerecord val;
	boolean fl;

	setlongvalue (num, &val);

	fl = langpackvalue (val, &hdata, HNoNode);
		
	if (fl)
		fl = oppushhandle (hlist, nil, hdata);
	
	return (fl);
	} /*langpushlistlong*/


static boolean evaluatefield (hdltreenode htree, bigstring key, boolean flformal, tyvaluerecord *itemval) {
	
	/*
	3/23/93 dmb: htree is a x:y field specification. return the field key, 
	and the field data as a descriptor record whose dataHandle is not on the 
	temp stack.

	02/05/02 dmb: added flformal param and handling
	*/
	
	register hdltreenode h = htree;
	tyvaluerecord keyval;
	
	assert ((**h).nodetype == fieldop);
	
	if (flformal) {

		if (!langgetidentifier ((**h).param1, key))
			return (false);
		}
	else {

		if (!evaluatetree ((**h).param1, &keyval))
			return (false);
		
		if (!coercetostring (&keyval))
			return (false);
	
		pullstringvalue (&keyval, key);

		disposevaluerecord (keyval, false);
		}
	
	if (!evaluatetree ((**h).param2, itemval))
		return (false);
	
	return (true);
	} /*evaluatefield*/


boolean makelistvalue (hdltreenode htree, tyvaluerecord *vreturned) {
	
	/*
	4/1/93 dmb: don't cleartmpstack here -- we might be creating a value in 
	an expression. just take care of what we create.
	*/
	
	register hdltreenode h;
	hdllistrecord hlist;
	tyvaluerecord itemval;
	
	if (!opnewlist (&hlist, false))
		return (false);
	
	for (h = htree; h != nil; h = (**h).link) { /*process each expression in the list*/
		
		if (!evaluatetree (h, &itemval))
			goto error;
		
		if (!langpushlistval (hlist, nil, &itemval))
			goto error;
		}
	
	return (setheapvalue ((Handle) hlist, listvaluetype, vreturned));
	
	error: {
		
		opdisposelist (hlist);
		
		return (false);
		}
	} /*makelistvalue*/


boolean makerecordvalue (hdltreenode htree, boolean flformal, tyvaluerecord *vreturned) {
	
	/*
	4/1/93 dmb: don't cleartmpstack here -- we might be creating a value in 
	an expression. just take care of what we create.

	02/05/02 dmb: added flformal param and handling
	*/
	
	register hdltreenode h;
	hdllistrecord hlist;
	bigstring key;
	tyvaluerecord itemval;
	
	if (!opnewlist (&hlist, true))
		return (false);
	
	for (h = htree; h != nil; h = (**h).link) { /*process each expression in the list*/
		
		if (!evaluatefield (h, key, flformal, &itemval))
			goto error;
		
		if (!langpushlistval (hlist, key, &itemval))
			goto error;
		}
	
	return (setheapvalue ((Handle) hlist, recordvaluetype, vreturned));
	
	error: {
		
		opdisposelist (hlist);
		
		return (false);
		}
	} /*makerecordvalue*/


boolean langgetlistitem (const tyvaluerecord *vlist, long ix, ptrstring pkey, tyvaluerecord *vitem) {
	
	if (getnthlistval ((*vlist).data.listvalue, ix, pkey, vitem))
		return (true);
	
	if (!fllangerror) {
		
		tyvaluerecord vindex;
		
		setlongvalue (ix, &vindex);
		
		langarrayreferror (0, pkey, vlist, &vindex);
		}
	
	return (false);
	} /*langgetlistitem*/


static boolean listtostring (hdllistrecord hlist, tyvaluerecord *val) {
	
	/*
	12/22/92 dmb: starter version: 255-char limit
	
	4/2/93 dmb: next version: 255-char limit for individual items only.
	
	2006-02-20 aradke: rewrite using handlestreams for efficiency.
		also in preparation for revoming the 255-char limit for list items.
	*/
	
	long i, n;
	bigstring key;
	tyvaluerecord itemval;
	bigstring bs;
	handlestream s;
	boolean flisrecord = opgetisrecord (hlist);
	
	n = opcountlistitems (hlist);
	
	openhandlestream (nil, &s);
	
	if (!writehandlestreamchar (&s, '{'))	/* creates handle, need to dispose later */
		return (false);
	
	for (i = 1; i <= n; ++i) {
		
		if (!getnthlistval (hlist, i, key, &itemval))
			goto error;
		
		if (flisrecord) {
				
			if (!writehandlestreamchar (&s, chdoublequote))
				goto error;
			
			langdeparsestring (key, chdoublequote); /*add needed escape sequences*/
			
			if (!writehandlestreamstring (&s, key))
				goto error;
				
			if (!writehandlestreamchar (&s, chdoublequote))
				goto error;
				
			if (!writehandlestreamchar (&s, ':'))
				goto error;
			}
		
		if (!getobjectmodeldisplaystring (&itemval, bs)) /*max 253 characters*/
			goto error;
		
		disposevaluerecord (itemval, true); /*don't clog temp stack*/
		
		if (!writehandlestreamstring (&s, bs))
			goto error;
		
		if (i < n)
			if (!writehandlestreamstring (&s, BIGSTRING ("\x02, ")))
				goto error;
		}
	
	if (!writehandlestreamchar (&s, '}'))
		goto error;
	
	return (setheapvalue (closehandlestream (&s), stringvaluetype, val));
	
	error:
	
	disposehandlestream (&s);
	
	return (false);
	} /*listtostring*/


static boolean makeintegerlist (tyvaluerecord *v, tyvaluetype listtype, void *pints, short ctints, hdllistrecord *intlist) {
	
	/*
	2.1b2 dmb: create a list containing the short integers in the pints array 
	
	2.1b8 dmb: for now, disallow coercion to record. later, we might have 
	an array of keys for each type.
	*/
	
	register short *lpi = (short *) pints;
	boolean flrecord = listtype == recordvaluetype;
	tyvaluerecord itemval;
	
	if (flrecord) {
		
		langcoerceerror (v, listtype);
		
		return (false);
		}
	
	if (!opnewlist (intlist, flrecord))
		return (false);
	
	while (--ctints >= 0) { /*process each int in the array*/
		
		setintvalue (*lpi++, &itemval);

		if (!langpushlistval (*intlist, nil, &itemval))
			goto error;
		}
	
	return (true);
	
	error:
		
		opdisposelist (*intlist);
		
		return (false);
	} /*makeintegerlist*/


static boolean pullintegerlist (hdllistrecord intlist, short ctints, void *pints) {
	
	/*
	2.1b2 dmb: pull out the array of short integers from the list
	*/
	
	register short *lpi = (short *) pints;
	tyvaluerecord val;
	long ctitems;
	long n;
	
	ctitems = opcountlistitems (intlist);
	
	if (ctitems < ctints) {
		
		langlongparamerror (listcoerceerror, ctints);
		
		return (false);
		}
	
	for (n = 1; n <= ctints; ++n) { /*grab each int in the list*/
		
		if (!getnthlistval (intlist, n, nil, &val))
			return (false);

		if (!coercevalue (&val, intvaluetype))
			return (false);

		*lpi++ = val.data.intvalue;
		}
	
	return (true);
	} /*pullintegerlist*/


static Point swappoint (Point pt) {
	
	short temp = pt.h;
	
	pt.h = pt.v;
	
	pt.v = temp;
	
	return (pt);
	} /*swappoint*/


static boolean nofunctioncallsvisit (hdltreenode hnode, void *refcon) {
#pragma unused (refcon)

	return ((**hnode).nodetype != functionop);
	} /*nofunctioncallsvisit*/


static boolean codecontainsfunctioncalls (hdltreenode hnode) {
	
	/*
	5.1.3 dmb: return true if the code tree doesn't contain any function calls.
	*/
	
	return (!langvisitcodetree (hnode, &nofunctioncallsvisit, nil));
	} /*codecontainsfunctioncalls*/
			

static boolean stringtolist (tyvaluerecord *val, tyvaluetype type) {
	
	/*
	4.1b2 dmb: if a string is actually a list expression, that's what 
	this coercion should yield.

	5.0b17 dmb: don't dispose of val unless evaluatetree succeeds
	*/
	
	Handle htext;
	hdltreenode hmodule;
	boolean fl = false;
	unsigned short savelines;
	unsigned short savechars;
	
	if (gethandlesize ((*val).data.stringvalue) == 0) { /*empty string -> null list*/
		
		disposevaluerecord (*val, true);
		
		if (type == listvaluetype)
			return (makelistvalue (nil, val));
		else
			return (makerecordvalue (nil, false, val));
		}
	
	if (!copyhandle ((*val).data.stringvalue, &htext))
		return (false);
	
	savelines = ctscanlines;
	
	savechars = ctscanchars;
	
	disablelangerror ();
	
	fl = langcompiletext (htext, false, &hmodule); /*always disposes htext*/
	
	enablelangerror ();
	
	if (fl) {
		
		register hdltreenode h = (**hmodule).param1;
		register tytreetype op = (**h).nodetype;
		tyvaluerecord vlist;
		
		if (type == listvaluetype)
			fl = op == listop;
		else
			fl = op == recordop;
		
		if (fl)
			fl = !codecontainsfunctioncalls (h); // 5.1.3 dmb: security!
		
		if (fl) {
			
			disablelangerror (); /*we'll generate own error w/correct position*/
			
			fl = evaluatetree (h, &vlist);
			
			enablelangerror ();

			if (fl) {
				
				disposevaluerecord (*val, true);
				
				*val = vlist;
				}
			}
		
		langdisposetree (hmodule);
		}
	
	ctscanlines = savelines;
	
	ctscanchars = savechars;
	
	return (fl);
	} /*stringtolist*/


boolean coercetolist (tyvaluerecord *val, tyvaluetype type) {
	
	/*
	4/14/93 dmb: support list <-> record coercion when empty, but don't allow 
	it otherwise
	
	2.1b8 dmb: coercing a list to a record creates a true record
	
	4.1b2 dmb: use stringtolist for string values
	*/
	
	register tyvaluerecord *v = val;
	register tyvaluetype vtype = (*v).valuetype;
	hdllistrecord hlist = nil;
	long size;
	
	if (vtype == type)
		return (true);
	
	switch (vtype) {
		
		case novaluetype:
			if (flinhibitnilcoercion)
				return (false);
			
			if (type == listvaluetype)
				return (makelistvalue (nil, v));
			else
				return (makerecordvalue (nil, false, v));
		
		case externalvaluetype:
			langbadexternaloperror (badexternaloperationerror, *v);
			
			return (false);
		
		case listvaluetype:
		case recordvaluetype:
			if (!langgetlistsize (v, &size))
				return (false);
			
			if (size > 0) {
				
				langcoerceerror (v, type);
				
				return (false);
				}
			
			if (!opnewlist (&hlist, type == recordvaluetype))
				return (false);
			
			break;
		
		case pointvaluetype: {
			Point pt = swappoint ((*v).data.pointvalue);
			
			if (!makeintegerlist (v, type, &pt, 2, &hlist))
				return (false);
			
			break;
			}
		
		case rgbvaluetype: {
			RGBColor rgb = **(*v).data.rgbvalue;
			
			if (!makeintegerlist (v, type, &rgb, 3, &hlist))
				return (false);
			
			break;
			}
		
		case rectvaluetype: {
			Rect r = **(*v).data.rectvalue;
			
			if (!makeintegerlist (v, type, &r, 4, &hlist))
				return (false);
			
			break;
			}
		
	#ifdef MACVERSION
		case objspecvaluetype:
		case binaryvaluetype: {
			AEDesc vdesc, listdesc;
			OSErr err;
			boolean fl;
			
			if (!coercetobinary (v))
				return (false);
			
			binarytodesc ((*v).data.binaryvalue, &vdesc); // still in temp stack
			
			err = AECoerceDesc (&vdesc, langgettypeid (type), &listdesc);
			
			if (err != noErr) {
				
				if (err == errAECoercionFail) {
					
					coercevalue (v, vtype); /*back to it's original type for the error message*/
					
					langcoerceerror (v, type);
					}
				else
					oserror (err);
				
				return (false);
				}
			
			fl = langipcconvertaelist (&listdesc, v);
			
			AEDisposeDesc (&listdesc);
			
			return (fl);
			}
	#endif
		
		case stringvaluetype:
			
			if (stringtolist (v, type))
				return (true);
			
			/*else fall through...*/
		
		default:
			/*
			create a list containing the item. note that this does not 
			emulate version 4.x's AE functionality, where the value 
			might be coerceable to a list directly.
			*/

			if (!opnewlist (&hlist, type == recordvaluetype))
				return (false);
			
			if (!langpushlistval (hlist, nil, v)) {
				
				opdisposelist (hlist);
				
				langcoerceerror (v, type);
				
				return (false);
				}
			
			break;
		}
	
	disposevaluerecord (*v, true);
	
	return (setheapvalue ((Handle) hlist, type, v));
	} /*coercetolist*/


static boolean coercelistcontents (tyvaluerecord *val, tyvaluetype totype, hdllistrecord hlist, long ctitems) {
#pragma unused(val)

	/*
	4.0b7 dmb:  when list->totype coercion otherwise fails, we try to coerce each item 
	in the list to totype. if we success, we'll return true even though the value we 
	return will still be a list, not a totype value. since this used to be a failure 
	case, it can't break working scripts. but it means that glue scripts can coerce to
	objspec, alias, or whatever and still allow a list of those items be pass through.
	*/
	
	short ix;
	
	for (ix = 1; ix <= ctitems; ++ix) {

		tyvaluerecord itemval;
		
		if (!getnthlistval (hlist, ix, nil, &itemval))
			return (false);
		
		if (!coercevalue (&itemval, totype))
			return (false);
		
		if (!setnthlistval (hlist, ix, nil, &itemval))
			return (false);

		disposevaluerecord (itemval, false);

		}
	
	return (true);
	} /*coercelistcontents*/


boolean coercelistvalue (tyvaluerecord *val, tyvaluetype totype) {
	
	/*
	2.1b6 dmb: coercing a list to a boolean indicates whether or not the 
	list is empty, except when the list contains a single, boolean item.
	
	2.1b8 dmb: for a single-item list, try to coerce item to desired type, 
	instead of requiring the the type already match
	*/
	
	register tyvaluerecord *v = val;
	hdllistrecord hlist;
	long ctitems;
	tyvaluerecord itemval;
	
	if (totype == (*v).valuetype)
		return (true);
	
	hlist = (*v).data.listvalue;
	
	switch (totype) {
		
		case listvaluetype:
			opsetisrecord (hlist, false);

			return (true);
		
		case stringvaluetype:
			if (!listtostring (hlist, &itemval))
				return (false);
			
			disposevaluerecord (*v, true);
			
			*v = itemval;
			
			break;
		
		case binaryvaluetype:
			return (coercetobinary (v));
		
		case pointvaluetype: {
			Point pt;
			
			if (!pullintegerlist (hlist, 2, &pt))
				return (false);
			
			if (!setpointvalue (swappoint (pt), v))
				return (false);
			
			break;
			}
		
		case rgbvaluetype: {
			RGBColor rgb = **(*v).data.rgbvalue;
			
			if (!pullintegerlist (hlist, 3, &rgb))
				return (false);
			
			if (!newheapvalue (&rgb, sizeof (rgb), rgbvaluetype, v))
				return (false);
			
			break;
			}
		
		case rectvaluetype: {
			Rect r = **(*v).data.rectvalue;
			
			if (!pullintegerlist (hlist, 4, &r))
				return (false);
			
			if (!newheapvalue (&r, sizeof (r), rectvaluetype, v))
				return (false);
			
			break;
			}
		
		default:
			ctitems = opcountlistitems (hlist);
			
			if (ctitems == 1) {
				
				if (!getnthlistval (hlist, 1, nil, &itemval))
					return (false);
				
				if (!coercevalue (&itemval, totype))
					return (false);
				
				disposevaluerecord (*v, true);
				
				*v = itemval;
				
				return (true);
				}
			
			if (totype == booleanvaluetype) {
				
				if (!setbooleanvalue (ctitems > 0, v))
					return (false);
				
				break;
				}
			
			/*
			langcoerceerror (v, totype);
			
			return (false);
			*/
			return (coercelistcontents (v, totype, hlist, ctitems));	// 4.0b7 dmb
		}
	
	return (true);
	} /*coercelistvalue*/


static boolean equalsublists (hdllistrecord list1, hdllistrecord list2, long ixcompare, long ctcompare, boolean flbykey) {
	
	/*
	compare the sublist of list1 starting at ixcompare with list2.  if flbykey, 
	order doesn't matter and ixcompare is ignored.  ctcompare is expected to be 
	the size of list2
	
	5.0a24 dmb: with oplanglists, we can use EQvalue. which can recurse to here.
	*/
	
	register long ix;
	register long n = ctcompare;
	tyvaluerecord item1;
	tyvaluerecord item2;
	tyvaluerecord vequal;
	bigstring key1;
	bigstring key2;
	boolean fl = true;
	
	for (ix = 1; ix <= n; ++ix) {
		
		if (!getnthlistval (list2, ix, key1, &item2))
			return (false);
		
		if (flbykey) {
			
			fl = getnthlistval (list1, -1, key1, &item1);
			}
		else {
			if (!getnthlistval (list1, ix + ixcompare, key2, &item1))
				fl = false;
			else
				fl = equalstrings (key1, key2);
			}
		
		disablelangerror ();
		
		fl = fl && EQvalue (item1, item2, &vequal) && vequal.data.flvalue;
		
		enablelangerror ();
		
		if (!fl)
			break;
		}
	
	return (fl);
	} /*equalsublists*/


boolean listaddvalue (tyvaluerecord *v1, tyvaluerecord *v2, tyvaluerecord *vreturned) {
	
	/*
	add the two list values. both lists should be of the same type, since langvalue 
	does the necessary coercion first. But if one is an AEList and the other an AERecord,
	the AE Manager will generate errors if appropriate.
	
	when adding records, only add fields from the 2nd record that don't already exist
	in the first
	
	2.1b8 dmb: initialize errcode to noErr, or random error results if adding 
	two records where the 1st item in record 2 is already in record 1
	
	5.0.2b10 dmb: since we throw away v1 and v2, we can boost performance
	by adding directly to v1 and making that the return value.
	
	10.0a1 hra: we were always looping through v2, so adding a long list to a small
	list was significantly slower (2x or more) than adding the small list to the long
	list. Now we always loop through the shortest of the 2 lists.
	*/
	
	hdllistrecord list2;
	hdllistrecord list3;
	long i, n, m;
	Handle hitem, hignore;
	bigstring key;
	
	m = opcountlistitems ((*v1).data.listvalue);
	n = opcountlistitems ((*v2).data.listvalue);
	
	if ( (m > n) || opgetisrecord ((*v2).data.listvalue) ) {
		/* Either the first list is longer than the second:
		   append to the first, or we are adding records so we want to
		   de-dupe the entries always in the same way
		*/
		list2 = (*v2).data.listvalue;
		*vreturned = *v1;
		initvalue (v1, novaluetype);

		list3 = (*vreturned).data.listvalue;

		for (i = 1; i <= n; ++i) { /*copy values over from second list*/
		
			if (!opgetlisthandle (list2, i, key, &hitem))
				return (false);
			
			if (!copyhandle (hitem, &hitem))
				return (false);

			if (opgetisrecord (list3)) { /* discard duplicate keys */
				
				if (opgetlisthandle (list3, -1, key, &hignore))
					disposehandle (hitem); /* discard the duplicate. don't push it over */
				else
					oppushhandle (list3, key, hitem);
				}
			else
				oppushhandle (list3, key, hitem);
			} /*for*/

	} else { /* the second list is longer than the first: prepend to the second */
		list2 = (*v1).data.listvalue;
		*vreturned = *v2;
		initvalue (v2, novaluetype);
		
		list3 = (*vreturned).data.listvalue;
		
		for (i = m; i >= 1; --i) { /*copy values over from first list, in reverse*/
		
			if (!opgetlisthandle (list2, i, key, &hitem))
				return (false);
			
			if (!copyhandle (hitem, &hitem))
				return (false);

			opunshifthandle (list3, key, hitem);
			} /*for*/
	}		
	
	return (true);
	} /*listaddvalue*/


boolean listsubtractvalue (tyvaluerecord *v1, tyvaluerecord *v2, tyvaluerecord *vreturned) {
	
	/*
	add the two list values. both lists should be of the same type, since langvalue 
	does the necessary coercion first. But if one is an AEList and the other an AERecord,
	the AE Manager will generate errors if appropriate.
	
	when adding records, only add fields from the 2nd record that don't already exist
	in the first
	*/
	
	hdllistrecord list1 = (*v1).data.listvalue;
	hdllistrecord list2 = (*v2).data.listvalue;
	hdllistrecord list3;
	long ix1, ix2, n1, n2;
	Handle item1, item2;
	bigstring key;
	
	//if (!copyvaluerecord (*v1, vreturned))
	//	return (false);
	
	*vreturned = *v1;
	
	initvalue (v1, novaluetype);

	list3 = (*vreturned).data.listvalue;

	n1 = opcountlistitems (list1);
	
	n2 = opcountlistitems (list2);
	
	if (opgetisrecord (list3)) {
		
		for (ix1 = 1; ix1 <= n2; ++ix1) { /*delete values that appear in second record*/
			
			if (!opgetlisthandle (list2, ix1, key, &item1))
				goto error;
			
			if (opgetlisthandle (list3, -1, key, &item2)) {
				
				if (equalhandles (item1, item2))
					opdeletelistitem (list3, -1, key);
				}
			}
		}
	else {
		
		ix2 = n1 - n2;
		
		for (ix1 = 0; ix1 <= ix2; ++ix1) {
			
			if (equalsublists (list1, list2, ix1, n2, false)) {
				
				while (--n2 >= 0)
					if (!opdeletelistitem (list3, ix1 + 1, nil))
						goto error;
				
				break;
				}
			}
		}
	
	return (true);
	
	error:
	
	return (false);
	} /*listsubtractvalue*/


static boolean comparelists (hdllistrecord list1, hdllistrecord list2, tytreetype comparisonop) {
	
	/*
	compare the two lists, returning true if the comparison holds, false 
	if it doesn't or an error occurs
	*/
	
	long n1, n2;
	register long ix1, ix2;
	boolean flbykey;
	
	n1 = opcountlistitems (list1);

	n2 = opcountlistitems (list2);
	
	ix1 = 0;
	
	ix2 = n1 - n2;
	
	if (ix2 < 0) /*v1 can't beginwith, endwith, contain or be equal to v2*/
		goto exit;
	
	flbykey = opgetisrecord (list1);
	
	switch (comparisonop) {
		
		case beginswithop:
			ix2 = 0;
			
			flbykey = false;
			
			break;
		
		case endswithop:
			ix1 = ix2;
			
			flbykey = false;
			
			break;
		
		case EQop:
			if (ix2 != 0) /*n2 != n1*/
				goto exit;
			
			break;
		
		case containsop:
			if (flbykey)
				ix2 = 0;
			
			break;
		
		default:
			break;
		}

	
	for (; ix1 <= ix2; ++ix1) {
		
		if (equalsublists (list1, list2, ix1, n2, flbykey))
			return (true);
		}
	
	exit:
	
	return (false);
	} /*comparelists*/


boolean listcomparevalue (tyvaluerecord *v1, tyvaluerecord *v2, tytreetype comparisonop, tyvaluerecord *vreturned) {
	
	boolean fl;
	
	fl = comparelists ((*v1).data.listvalue, (*v2).data.listvalue, comparisonop);
	
	if (fllangerror)
		return (false);
	
	return (setbooleanvalue (fl, vreturned));
	} /*listcomparevalue*/


boolean coercetolistposition (tyvaluerecord *val) {
	
	/*
	get a list position parameter -- either an index (number) or 
	a keyword (string4)

	5.0a19 dmb: for Frontier5, string4 -> string index
	*/
	
	tyvaluerecord *v = val;
	boolean fl;
	
	switch ((*v).valuetype) {
		
		case longvaluetype:
		case stringvaluetype:
			return (true);
		
		case ostypevaluetype:
			return (coercetostring (v));

		default:
			disablelangerror ();
			
			fl = coercetolong (v) || coercetostring (v);
			
			enablelangerror ();
			
			if (fl)
				return (true);
		}
	
	langerror (badipclistposerror);
	
	return (false);
	} /*coercetolistposition*/


boolean listarrayvalue (tyvaluerecord *vlist, bigstring bsname, register tyvaluerecord *vindex, tyvaluerecord *vreturned) {
	
	/*
	bsname is provided for error reporting only
	*/
	
	hdllistrecord hlist = (*vlist).data.listvalue;
	bigstring key;
	boolean fl;
	
	if (!coercetolistposition (vindex))
		return (false);
	
	if ((*vindex).valuetype == longvaluetype)
		fl = getnthlistval (hlist, (*vindex).data.longvalue, nil, vreturned);
	else {
		
		pullstringvalue (vindex, key);
		
		fl = getnthlistval (hlist, -1, key, vreturned);
		}

	if (!fl && !fllangerror)
		langarrayreferror (0, bsname, vlist, vindex);
	
	return (fl);
	} /*listarrayvalue*/


boolean listassignvalue (tyvaluerecord *vlist, bigstring bsname, register tyvaluerecord *vindex, tyvaluerecord *vassign) {
	
	/*
	bsname is provided for error reporting only
	*/
	
	hdllistrecord hlist = (*vlist).data.listvalue;
	bigstring key;
	boolean fl;
	
	if (!coercetolistposition (vindex))
		return (false);
	
	if ((*vindex).valuetype == longvaluetype)
		fl = setnthlistval (hlist, (*vindex).data.longvalue, nil, vassign);
	else{
		
		pullstringvalue (vindex, key);
		
		fl = setnthlistval (hlist, -1, key, vassign);
		}

	if (!fl && !fllangerror)
		langarrayreferror (0, bsname, vlist, vindex);

	return (fl);
	} /*listassignvalue*/


boolean listdeletevalue (tyvaluerecord *vlist, bigstring bsname, register tyvaluerecord *vindex) {
	
	/*
	bsname is provided for error reporting only
	*/
	
	hdllistrecord hlist = (*vlist).data.listvalue;
	bigstring key;
	boolean fl;
	
	if (!coercetolistposition (vindex))
		return (false);
	
	if ((*vindex).valuetype == longvaluetype)
		fl = opdeletelistitem (hlist, (*vindex).data.longvalue, nil);
	else{
		
		pullstringvalue (vindex, key);
		
		fl = opdeletelistitem (hlist, -1, key);
		}

	if (!fl && !fllangerror)
		langarrayreferror (0, bsname, vlist, vindex);

	return (fl);
	} /*listdeletevalue*/


typedef struct tylangvisitlistinfo {
	ptrvoid refcon;
	langvisitlistvaluescallback visit;
	} tylangvisitlistinfo;


static boolean langvisitlistvaluesvisit (Handle hdata, ptrstring bskey, ptrvoid refcon) {
#pragma unused (bskey)

	/*
	2004-11-04 aradke: helper for langvisitlistvalues, called from opvisitlist.
	*/

	tylangvisitlistinfo *info = (tylangvisitlistinfo *) refcon;
	tyvaluerecord val;
	boolean fl;
	
	if (!langunpackvalue (hdata, &val))
		return (false);
		
	fl = (*info).visit (&val, (*info).refcon);
	
	disposevaluerecord (val, true);

	return (fl);
	} /*langvisitlistvaluesvisit*/


boolean langvisitlistvalues (tyvaluerecord *vlist, langvisitlistvaluescallback visit, ptrvoid refcon) {
	
	/*
	2004-11-04 aradke: rewritten to use opvisitlist, called from langregexp.c.
	*/
	
	hdllistrecord hlist = (hdllistrecord) (*vlist).data.binaryvalue;
	tylangvisitlistinfo info;
	
	info.refcon = refcon;
	info.visit = visit;
	
	return (opvisitlist (hlist, &langvisitlistvaluesvisit, &info));
	} /*langvisitlistvalues*/





