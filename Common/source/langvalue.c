
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

#include "frontierconfig.h"
#include "memory.h"
#include "cursor.h"
#include "db.h"
#include "dialogs.h"
#include "error.h"
#include "file.h"
#include "font.h"
#include "kb.h"
#include "launch.h"
#include "ops.h"
#include "smallicon.h"
#include "strings.h"
#include "oplist.h"
#include "lang.h"
#include "langinternal.h"
#include "langexternal.h"
#include "langipc.h"
#include "langsystem7.h"
#include "langtokens.h"
#include "process.h"
#include "tablestructure.h"
#include "tableverbs.h"
#ifdef flcomponent
	#include "osacomponent.h"
#endif

#include "timedate.h"
#include "byteorder.h"	/* 2006-04-08 aradke: endianness conversion macros */



boolean flparamerrorenabled = true;

boolean flnextparamislast = false;

bigstring bsfunctionname; /*available for use in error messages*/

boolean flcoerceexternaltostring = false;

boolean flinhibitnilcoercion = false;


static byte bshexprefix [] = STR_hexprefix;

static tyfunctype functiontoken; /*use this if string is empty*/

static boolean fllocaldotparamsonly = false; /*set to inhibit searchpathvisit in langgetdotparams*/

#pragma pack(2)
typedef struct tyfastflagsvaluerecord {
	
	tyvaluetype valuetype;
	
	byte flags: 8; /*instead of 4 separate bits fields to clear*/
	
	tyvaluedata data;
	} tyfastflagsvaluerecord;
#pragma options align=reset


boolean langsymbolreference (hdlhashtable htable, bigstring bs, tyvaluerecord *val, hdlhashnode * hnode) {
	
	/*
	a bundle that was getting replicated everywhere.  look up the indicated name in
	the table, and return true with *val equal to its value if the symbol is defined.
	*/
	
	boolean fl;
	
	pushhashtable (htable);
	
	fl = langgetsymbolval (bs, val, hnode);
	
	pophashtable ();
	
	if (!fl) {
		
		if ((htable == nil) && isemptystring (bs))
			langerror (niladdresserror);
		else
			langparamerror (unknownidentifiererror, bs);
		
		return (false);
		}
	
	return (true);
	} /*langsymbolreference*/


static boolean gettokenvisit (bigstring bsname, hdlhashnode hnode, tyvaluerecord val, ptrvoid refcon) {
#pragma unused(bsname, hnode, refcon)
	if (val.valuetype != tokenvaluetype)
		return (false);
		
	return (val.data.tokenvalue == functiontoken);
	} /*gettokenvisit*/
	
	
static boolean gettokenname (bigstring bsname) {
	
	register boolean fl;
	
	fl = hashinversesearch (hbuiltinfunctions, &gettokenvisit, nil, bsname);
	
	return (fl);
	} /*gettokenname*/


void initvalue (tyvaluerecord *val, tyvaluetype type) {
	
	/*
	call this routine to set the type and clear other fields of a new value
	*/
	
	(*val).valuetype = type;
	
	(*val).data.longvalue = 0;
	
	(*(tyfastflagsvaluerecord *) val).flags = 0;
	} /*initvalue*/


boolean setnilvalue (tyvaluerecord *val) {
	
	initvalue (val, novaluetype);

	return (true);
	} /*setnilvalue*/


boolean setbooleanvalue (boolean fl, tyvaluerecord *val) {
	
	/*
	set val to be a the boolean value fl.  can't fail.
	*/
	
	(*val).valuetype = booleanvaluetype;
	
	(*val).data.flvalue = fl;
	
	(*(tyfastflagsvaluerecord *) val).flags = 0;
	
	return (true);
	} /*setbooleanvalue*/


boolean setcharvalue (byte ch, tyvaluerecord *val) {
	
	/*
	set val to be a the character value ch.  can't fail.
	*/
	
	(*val).valuetype = charvaluetype;
	
	(*val).data.chvalue = ch;
	
	(*(tyfastflagsvaluerecord *) val).flags = 0;
	
	return (true);
	} /*setcharvalue*/


boolean setintvalue (short x, tyvaluerecord *val) {
	
	/*
	set val to be a the integer value x.  can't fail.
	*/
	
	(*val).valuetype = intvaluetype;
	
	(*val).data.intvalue = x;
	
	(*(tyfastflagsvaluerecord *) val).flags = 0;
	
	return (true);
	} /*setintvalue*/


boolean setlongvalue (long x, tyvaluerecord *val) {
	
	/*
	set val to be a the long value x.  can't fail.
	*/
	
	(*val).valuetype = longvaluetype;
	
	(*val).data.longvalue = x;
	
	(*(tyfastflagsvaluerecord *) val).flags = 0;
	
	return (true);
	} /*setlongvalue*/


boolean setdatevalue (unsigned long x, tyvaluerecord *val) {

	/*
	set val to be a the date value x.  can't fail.
	*/
	
	(*val).valuetype = datevaluetype;
	
	(*val).data.datevalue = x;
	
	(*(tyfastflagsvaluerecord *) val).flags = 0;
	
	return (true);
	} /*setdatevalue*/


boolean setdirectionvalue (tydirection x, tyvaluerecord *val) {
	
	/*
	set val to be a the direction value x.  can't fail.
	*/
	
	(*val).valuetype = directionvaluetype;
	
	(*val).data.dirvalue = x;
	
	(*(tyfastflagsvaluerecord *) val).flags = 0;
	
	return (true);
	} /*setdirectionvalue*/


boolean setostypevalue (OSType x, tyvaluerecord *val) {
	
	/*
	set val to be a the ostype value x.  can't fail.
	*/
	
	(*val).valuetype = ostypevaluetype;
	
	(*val).data.ostypevalue = x;
	
	(*(tyfastflagsvaluerecord *) val).flags = 0;
	
	return (true);
	} /*setostypevalue*/


boolean setpointvalue (Point x, tyvaluerecord *val) {
	
	/*
	set val to be a the Point value x.  can't fail.
	*/
	
	(*val).valuetype = pointvaluetype;
	
	(*val).data.pointvalue = x;
	
	(*(tyfastflagsvaluerecord *) val).flags = 0;
	
	return (true);
	} /*setpointvalue*/


boolean setfixedvalue (Fixed x, tyvaluerecord *val) {
	
	/*
	set val to be a the Fixed value x.  can't fail.
	*/
	
	(*val).valuetype = fixedvaluetype;
	
	(*val).data.fixedvalue = x;
	
	(*(tyfastflagsvaluerecord *) val).flags = 0;
	
	return (true);
	} /*setfixedvalue*/


boolean setsinglevalue (float x, tyvaluerecord *val) {
	
	/*
	set val to be a the single value x.  can't fail.
	*/
	
	(*val).valuetype = singlevaluetype;
	
	(*val).data.singlevalue = x;
	
	(*(tyfastflagsvaluerecord *) val).flags = 0;
	
	return (true);
	} /*setsinglevalue*/


boolean newheapvalue (ptrvoid pdata, long size, tyvaluetype type, tyvaluerecord *val) {
	
	/*
	create a new heap-allocated value of the indicated type, using the 
	provided data.
	
	we record the handle in the tmpstack, so that it can be sure it gets 
	deallocated.
	*/
	
	Handle x;
	
	initvalue (val, type);
	
	if (!newfilledhandle (pdata, size, &x))
		return (false);
	
	(*val).data.binaryvalue = x;
	
	if (!pushtmpstackvalue (val)) {
		
		disposehandle (x);
		
		(*val).data.binaryvalue = nil; /*neatness counts*/
		
		return (false);
		}
	
	return (true);
	} /*newheapvalue*/


boolean setstringvalue (bigstring bs, tyvaluerecord *val) {
	
	/*
	turn val into a string value, allocating a string in the heap.
	
	the handle is recorded in the tmpstack so that it gets deallocated.
	*/
	
	return (newheapvalue (bs + 1, stringlength (bs), stringvaluetype, val));
	} /*setstringvalue*/


static boolean getaddressparts (const tyvaluerecord *val, hdlhashtable *htable, bigstring bs) {
	
	/*
	5.1.4 dmb: new strategy for addresses. this routine just dissects the address
	value. if the table is -1, the address is unresolved and bs is the full path.
	*/
	
	hdlstring hstring = (*val).data.addressvalue;
	long ixtable;
	
	copyheapstring (hstring, bs);
	
	ixtable = stringlength (bs) + 1;
	
	if (!loadfromhandle ((Handle) hstring, &ixtable, sizeof (hdlhashtable), htable))
		*htable = nil;

	return (true);
	} /*getaddressparts*/


boolean getaddressvalue (tyvaluerecord val, hdlhashtable *htable, bigstring bs) {
	
	/*
	a hash table handle might be tacked onto the end of the string handle. 
	if it's not there, return nil for htable.
	
	we'll also do something fancy: we can catch some coding errors by 
	verifying that the table handle, if present, hasn't been disposed.
	
	3/11/91 dmb: since we can't reliably resolve full paths to addresses 
	at unpack time, we need to be prepared to "upgrade" val from a full 
	path to a name/table pair on the fly.  if no table has been appended 
	to the heap string value, try to do so now.
	
	3/19/92 dmb: handle new -1 htable convention for unresolved addresses 
	*/
	
	
	if (!getaddressparts (&val, htable, bs))
		return (false);

	if (*htable == (hdlhashtable) -1) { /*an unresolved address*/

			boolean fl;

			pushhashtable (roottable);
			
			//disablelangerror ();
			
			fl = langexpandtodotparams (bs, htable, bs);
			
			//enablelangerror ();
			
			pophashtable ();
			
			if (!fl)
				return (false);
			}
		
	else {
		if (!validhandle ((Handle) *htable)) {
			
			langparamerror (badaddresserror, bs);
			
			return (false);
			}
		}
	
	return (true);
	
	/*
	setheapstring (emptystring, hstring); /%avoid recursion%/
	
	disablelangerror (); /%we don't want to generate errors here%/
	
	if (langexpandtodotparams (bs, htable, bs)) { /%valid path%/
		
		setheapstring (bs, hstring); /%now we have just the name%/
		
		enlargehandle ((Handle) hstring, sizeof (hdlhashtable), htable); /%should never fail%/
		}
	else
		setheapstring (bs, hstring); /%restore original%/
	
	enablelangerror ();
	
	return (true);
	*/
	} /*getaddressvalue*/


boolean setaddressencoding (tyvaluerecord *val, boolean flfullpath) {
	
	/*
	5.1.4 dmb: to represent a full path, set htable to -1. otherwise, 
	the address data is the item name, with htable a valid handle.

	...but don't set full path encoding if the address is a local one
	*/

	hdlhashtable htable;
	bigstring bs;
	Handle h;

	if (!getaddressparts (val, &htable, bs))
		return (false);
	
	if (flfullpath == (htable == (hdlhashtable) -1))
		return (true);

	// set htable, bs for the flfullpath state
	if (flfullpath) {
		
		if (htable && (**htable).fllocaltable) //never encode local tables with full paths
			return (true);

		if (!getaddresspath (*val, bs))
			return (false);

		htable = (hdlhashtable) -1;
		}
	else {
		if (!getaddressvalue (*val, &htable, bs))
			return (false);
		}

	// set the value with the new encoding
	h = (Handle) (*val).data.addressvalue;

	if (!sethandlecontents (bs, stringsize (bs), h))
		return (false);
	
	if (!enlargehandle (h, sizeof (htable), &htable))
		return (false);
	
	return (true);
	} /*setaddressencoding*/


boolean setaddressvalue (hdlhashtable htable, const bigstring bs, tyvaluerecord *val) {
	
	return (setexemptaddressvalue (htable, bs, val) && pushtmpstackvalue (val));
	} /*setaddressvalue*/


boolean setexemptaddressvalue (hdlhashtable htable, const bigstring bs, tyvaluerecord *val) {
	
	/*
	turn val into an address value.  we store a string in the heap which
	is the path to the guy whose address we're storing.
	
	2/2/91 dmb: addresses now contain pointer to hash table.  see comment 
	in addressofvalue.
	*/
	
	hdlstring hstring;
	
	initvalue (val, addressvaluetype);
	
	if (!newheapstring (bs, &hstring))
		return (false);
	
	if (!enlargehandle ((Handle) hstring, sizeof (htable), &htable)) {
		
		disposehandle ((Handle) hstring);
		
		return (false);
		}
	
	(*val).data.addressvalue = hstring;
	
	return (true);
	} /*setexemptaddressvalue*/


/*
boolean setpasswordvalue (bigstring bs, tyvaluerecord *val) {
	
	if (!setstringvalue (bs, val))
		return (false);
	
	initvalue (val, passwordvaluetype);
	
	return (true);
	} /%setpasswordvalue%/
*/


boolean setheapvalue (Handle x, tyvaluetype type, tyvaluerecord *val) {
	
	/*
	simple utility routine for setting up any kind of heap-allocated value.
	
	unlikely to fail.  but if it does, dispose the handle; the caller isn't 
	managing the memory any more.

	5.0a24 dmb: on failure, disposevaluerecord, not disposehandle.
	*/
	
	initvalue (val, type);
	
	(*val).data.binaryvalue = x;
	
	if (!pushtmpstackvalue (val)) {
		
		disposevaluerecord (*val, false);
		
		return (false);
		}
	
	return (true);
	} /*setheapvalue*/


boolean setbinaryvalue (Handle x, OSType typeid, tyvaluerecord *val) {
	
	/*
	simple utility routine.  prepend the typeid to the handle and 
	set the heap value.
	
	on failure, dispose of the handle; caller should assume we own it 
	either way.
	*/
	
	memtodisklong (typeid);

	// kw - 2005-12-12 filemaker empty string fix
	// we need to be able to handle empty strings that arrive as a binary['utxt']
	if (x == nil)
		newhandle(0, &x);

	if (!insertinhandle (x, 0L, &typeid, sizeof (typeid))) {
		
		disposehandle (x);
		
		return (false);
		}
	
	return (setheapvalue (x, binaryvaluetype, val));
	} /*setbinaryvalue*/


boolean setbinarytypeid (Handle x, OSType typeid) {
	
	/*
	poke the typeid of the given binary handle
	*/
	
	if (gethandlesize (x) < sizeof (OSType)) /*defensive driving*/
		return (false);
	
	**(OSType **) x = conditionallongswap (typeid);
	
	return (true);
	} /*setbinarytypeid*/


OSType getbinarytypeid (Handle x) {
	
	/*
	peek at the typeid of the binary handle
	*/
	
	return (conditionallongswap (**(OSType **) x));
	} /*getbinarytypeid*/


boolean stripbinarytypeid (Handle x) {

	return (pullfromhandle (x, 0L, sizeof (OSType), nil));
	} /*stripbinarytypeid*/


boolean setdoublevalue (double x, tyvaluerecord *val) {
	
	/*
	turn val into a double value, allocated in the heap.
	
	the handle is recorded in the tmpstack so that it gets deallocated.
	*/
	
	return (newheapvalue (&x, sizeof (x), doublevaluetype, val));
	} /*setdoublevalue*/


boolean setfilespecvalue (ptrfilespec x, tyvaluerecord *val) {
	
	/*
	turn val into a filespec value, allocated in the heap.
	
	the handle is recorded in the tmpstack so that it gets deallocated.
	*/
	
	long len = filespecsize (*x);
	
	return (newheapvalue (x, len, filespecvaluetype, val));
	} /*setfilespecvalue*/


boolean setexternalvalue (Handle x, tyvaluerecord *val) {
	
	/*
	turn val into an external value, using the provided external handle as data.
	*/
	
	(*val).valuetype = externalvaluetype;
	
	(*val).data.externalvalue = x;
	
	(*(tyfastflagsvaluerecord *) val).flags = 0;
	
	return (true);
	} /*setexternalvalue*/


boolean copyvaluedata (tyvaluerecord *val) {
	
	/*
	7/26/91 dmb: the beginning of something new.  the problem: passing 
	large objects (like external values) on the stack is prohibitively 
	expensive.  up to this point, builtin functions like typeof () and 
	sizeof () can cheat by avoiding the evaluation of their parameters, 
	but normal handler's can't take external values as parameters.
	
	the idea is to defer the copying if the value data until it's actually 
	used in an assignment.  if it's never assigned, it never should have to 
	be duplicated.  in copyvaluercord, we now set a bit indicating that the 
	value's data hasn't been duplicated, i.e. the data is owned by another 
	(the original) value.  in disposevaluerecord, if the flag is set 
	nothing is disposed.  in assignvalue, if the flag is set the data is 
	duplicated at that time by calling this function.
	
	for the time being, this mechanism is only used for external values.  it 
	would improve performance to use it for all heap values, but a thorough 
	pass would have to be made over the code to eliminate the assumtion that 
	string values, for instance, own their handle.  right now, coercion functions 
	routinely dispose of the old string value by hand.
	
	5.0.2b12 dmb: use new langexternalcopyvalue (for outlines)
	*/
	
	register tyvaluerecord *v = val;
	Handle x;
	//hdlhashtable ht;
	//bigstring bs;
	boolean fl;
	
	if (!(*v).fltmpdata) /*nothing to do*/
		return (true);
	
	switch ((*v).valuetype) {
		
		/*
		case addressvaluetype:
			if (!getaddressvalue (*v, &ht, bs))
				return (false);
			
			if (!setaddressvalue (ht, bs, v))
				return (false);
			
			break;
		
		case stringvaluetype: 
		case passwordvaluetype:
		case rectvaluetype:
		case patternvaluetype:
		case rgbvaluetype:
		case objspecvaluetype:
		case filespecvaluetype:
		case aliasvaluetype:
		case doublevaluetype:
		case binaryvaluetype:
	#ifndef oplanglists
		case listvaluetype:
		case recordvaluetype:
	#endif
			#ifdef flnewfeatures
				
				if ((*v).fldiskval) {
					if (!dbrefhandle ((*v).data.diskvalue, &x))
						return (false);
					}
				else {
					if (!copyhandle ((*v).data.binaryvalue, &x))
						return (false);
					}
			#else
			
				if (!copyhandle ((*v).data.binaryvalue, &x))
					return (false);
			
			#endif
			
			if (!setheapvalue (x, (*v).valuetype, v))
				return (false);
			
			break;
		
	#ifdef oplanglists
		case listvaluetype:
		case recordvaluetype:
	#endif
		*/
		case codevaluetype:			
			if (!langpackvalue (*v, &x, HNoNode))
				return (false);
			
			fl = langunpackvalue (x, v);
			
			disposehandle (x);
			
			if (!fl)
				return (false);
			
			break;
		
		case externalvaluetype:
			
			if (!langexternalcopyvalue (v, v))
				return (false);
			
			break;
		
		default:
			shellinternalerror (idbadtempdatatype, STR_Internal_error_bad_type_for_temp_data);
			
			break;
		} /*switch*/
	
	(*v).fltmpdata = 0; /*we own the data now*/
	
	return (true); 
	} /*copyvaluedata*/


#ifdef tmpcopydebug
static boolean findheaptmp (tyvaluerecord *v) {
	
	register short ctloops;
	register tyvaluerecord *p;
	
	if (currenthashtable == nil)
		return (false);
	
	p = (**currenthashtable).tmpstack;
	
	for (ctloops = (**currenthashtable).cttmpstack; ctloops--; ++p) {
		
		if ((*p).data.binaryvalue == (*v).data.binaryvalue)
			return (true);
		}
	
	return (false);
	} /*findheaptmp*/
#endif


boolean copyvaluerecord (tyvaluerecord v, tyvaluerecord *vreturned) {
	
	/*
	create a copy of v in vreturned.  for strings and passwords and other
	relatively small heap-allocated objects, we create a copy of the data.
	
	for other types, externals and binaries, we create another reference
	to the heap-allocated object.  it's important that values copied for
	externals and binaries not be stored in the symbol table structure.
	
	2/15/91 dmb: no longer treat binary values like externals.  since we 
	now support passing values by reference (i.e. address values), the 
	script writer can avoid the overhead of large objects when desired. 
	binary values can now be used like any other automatic type.
	
	8/16/91 dmb: going even further, we'll now use the new fl.tmpdata 
	mechanism to avoid copying binary values when we don't need to.
	
	12/26/91 dmb: make sure that if an allocation failure occurs, we return 
	an empty value.
	
	4.0.2b1 dmb: handle fldiskvals
	
	5.0.2b12 dmb: use new opcopylist for lists

	5.1.4 dmb: don't do anything special for addresses; just copy the current
	binary state.
	*/
	
	Handle x;
	hdllistrecord hlist;
	
#ifdef tmpcopydebug
	static long ctdups = 0;
	static long cttmps = 0;
	static long cthits = 0;
	
	++ctdups;
	
	if (v.fltmpstack) {
		
		++cttmps;
		
		if (findheaptmp (&v)) {
			++cthits;
			
			*vreturned = v;
			(*vreturned).fltmpdata = true;		
			return (true);
			}
		}
#endif

	switch (v.valuetype) {
		
		case addressvaluetype:
		case stringvaluetype:
		case passwordvaluetype:
		case rectvaluetype:
		case patternvaluetype:
		case rgbvaluetype:
		case objspecvaluetype:
		case filespecvaluetype:
		case aliasvaluetype:
		case doublevaluetype:
		case binaryvaluetype:
	#ifndef oplanglists
		case listvaluetype:
		case recordvaluetype:
	#endif
			initvalue (vreturned, novaluetype);
			
				
				if (v.fldiskval) {
					/*
					4.0.2b1 dmb: for disk-based scalars, the copy will be the actual 
					data, while the original value (and the hashtable node) will still
					be on disk
					*/
					
					if (!dbrefhandle (v.data.diskvalue, &x))
						return (false);
					}
				else {
					if (!copyhandle (v.data.binaryvalue, &x))
						return (false);
					}

			return (setheapvalue (x, v.valuetype, vreturned));
		
	#ifdef oplanglists
		case listvaluetype:
		case recordvaluetype:
			initvalue (vreturned, v.valuetype);
			
			if (!opcopylist (v.data.listvalue, &hlist))
				return (false);
			
			return (setheapvalue ((Handle) hlist, v.valuetype, vreturned));
	#endif

		case codevaluetype:
		case externalvaluetype:
			*vreturned = v;
			
			(*vreturned).fltmpdata = true; /*see hashassign, disposevaluerecord*/
			
			break;
		
		default:
			*vreturned = v;
			
			break;
		} /*switch*/
	
	return (true); 
	} /*copyvaluerecord*/

#ifdef DATABASE_DEBUG

void debug_disposevaluerecord (tyvaluerecord val, boolean fldisk, long line, char *sourcefile)

#else

void disposevaluerecord (tyvaluerecord val, boolean fldisk)

#endif
{
	
	/*
	9/24/92 dmb: never dispose code trees here; there's no such thing as 
	a code value whose data isn't a piece of compiled code that will 
	disposed independently.
	
	4/20/93 dmb: now that code values are treated more consistently, and 
	code values that shouldn't be disposed have their tmpdata flag set, we 
	can go ahead and dispose code trees here.
	
	4.0.2b1 dmb: handle fldiskvals

	6.1b18 AR: since we allow externals on the tmp stack, we need to remove them, too
	*/
	
	if (val.fltmpdata) /*val doesn't own it's data*/
		return;
	
	switch (val.valuetype) {
		
		case stringvaluetype:
		case passwordvaluetype:
		case addressvaluetype:
		case rectvaluetype:
		case patternvaluetype:
		case rgbvaluetype:
		case objspecvaluetype:
		case filespecvaluetype:
		case aliasvaluetype:
		case doublevaluetype:
		case binaryvaluetype:
	#ifndef oplanglists
		case listvaluetype:
		case recordvaluetype:
	#endif
			if (val.fldiskval) { /*4.0.2b1 dmb: see langhash comments for details*/
			
				if (fldisk) {
					#ifdef DATABASE_DEBUG
						debug_dbpushreleasestack (val.data.diskvalue, (long) langexternalgettypeid (val), line, sourcefile);
					#else
						dbpushreleasestack (val.data.diskvalue, (long) langexternalgettypeid (val));
					#endif
					}
				}
			else {
				exemptfromtmpstack (&val);
				
				disposehandle (val.data.binaryvalue);
				}
			
			break;
		
	#ifdef oplanglists
		case listvaluetype:
		case recordvaluetype:
			exemptfromtmpstack (&val);
			
			opdisposelist (val.data.listvalue);
			
			break;
	#endif

		case codevaluetype:
			exemptfromtmpstack (&val);
			
			langdisposetree (val.data.codevalue);
			
			break;
		
		case externalvaluetype:
			exemptfromtmpstack (&val);

			langexternaldisposevalue (val, fldisk);
			
			break;
		
		default:
			/* do nothing */
			break;
			
		} /*switch*/
	} /*disposevaluerecord*/


#if 0

static boolean isgarbagetype (tyvaluetype type) {
	
	/*
	3.0.2b1 dmb: return true if the given type should be garbage collected 
	during a an arithmetic operation
	*/
	
	switch (type) {
		
		case stringvaluetype:
		case doublevaluetype:
		case binaryvaluetype:
		case listvaluetype:
		case recordvaluetype:
			return (true);
		
		default:
			return (false);
		} /*switch*/
	} /*isgarbagetype*/

#endif


void disposevalues (tyvaluerecord *val1, tyvaluerecord *val2) {
	
	/*
	the assertion made by the caller is that val1 and val2 have served
	their purpose and will never be used again.  if they have heap-allocated
	objects hanging off them, release the memory.
	
	we take two params because most operations are binary, for unary ops
	call with val2 == nil.
	
	1/16/91 dmb: use langheapallocated instead of testing valuetype for 
	stringvaluetype
	
	1/23/91 dmb: oops. this version of dispose needs to work similarly to 
	copyvaluerecord, only dealing with string-based handles.  binary and 
	external handles are not to be disposed.  since addresses and passwords 
	are not likely to be passed here, we can only check for stringvaluetype, 
	as originally coded.  other values will remain in temp stack
	
	3.0.2b1 dmb: use new isgarbagetype to decide whether garbage collection 
	is needed
	
	5.0.2b10 dmb: dispose heap values that are actually on the tmp stack; don't
	base it on their type
	
	2004-12-30 SMD: now extern instead of static
	*/
	
	register tyvaluerecord *v1 = val1;
	register tyvaluerecord *v2 = val2;
	
	if (v1 != nil)
		
		if ((*v1).fltmpstack) { //isgarbagetype ((*v1).valuetype)) {
			
		#ifdef oplanglists
			disposevaluerecord (*v1, true);
		#else
			releaseheaptmp ((Handle) (*v1).data.stringvalue);
		#endif
			}
	
	if (v2 != nil)
		
		if ((*v2).fltmpstack) { //isgarbagetype ((*v2).valuetype)) {

		#ifdef oplanglists
			disposevaluerecord (*v2, true);
		#else
			releaseheaptmp ((Handle) (*v2).data.stringvalue);
		#endif
			}
	
	} /*disposevalues*/


boolean stringisboolean (bigstring bs, boolean *flboolean) {
	
	alllower (bs);
	
	if (equalstrings (bs, bstrue)) {
	
		*flboolean = true;
		
		return (true);
		}
	
	if (equalstrings (bs, bsfalse)) {
	
		*flboolean = false;
		
		return (true);;
		}
	
	return (false);
	} /*stringisboolean*/


void pullstringvalue (const tyvaluerecord *v, bigstring bsval) {
	
	texthandletostring ((Handle) (*v).data.stringvalue, bsval);
	} /*pullstringvalue*/


static boolean stringtoboolean (tyvaluerecord *val) {
	
	/*
	4/19/91 dmb: if not "false", any non-empty string is true
	*/
	
	boolean flboolean;
	bigstring bs;
	
	pullstringvalue (val, bs);
	
	if (!stringisboolean (bs, &flboolean))
		flboolean = !isemptystring (bs);
	
	releaseheaptmp ((*val).data.stringvalue);
	
	return (setbooleanvalue (flboolean, val));
	} /*stringtoboolean*/


static boolean stringtochar (tyvaluerecord *val) {
	
	bigstring bs;
	
	pullstringvalue (val, bs);
	
	if (stringlength (bs) != 1) { /*we only accept strings that are 1 character long*/
		
		langparamerror (stringcharerror, bs);
		
		return (false);
		}
	
	releaseheaptmp ((Handle) (*val).data.stringvalue);
	
	setcharvalue (bs [1], val);
	
	return (true);
	} /*stringtochar*/


static boolean stringtolong (tyvaluerecord *val) {
	
	/*
	5/7/93 dmb: don't accept hex strings that don't begin with "0x"
	*/
	
	long x;
	bigstring bs;
	double d;
	tydirection dir;
	boolean flboolean;
	
	pullstringvalue (val, bs);
	
	stringdeletechars (bs, ','); /*get rid of all commas before converting*/
	
	if (isallnumeric (bs)) {
		
		stringtonumber (bs, &x);
		
		goto exit;
		}
	
	if (patternmatch (bshexprefix, bs) && hexstringtonumber (bs, &x))
		goto exit;
	
	if (stringtofloat (bs, &d)) {
		
		x = (long) d;
		
		goto exit;
		}
	
	if (stringtodir (bs, &dir)) {
		
		x = (long) dir;
		
		goto exit;
		}
	
	if (stringisboolean (bs, &flboolean)) {
		
		x= (long) flboolean;
		
		goto exit;
		}
	
	langparamerror (stringlongerror, bs);
	
	return (false);
	
	exit:
	
	releaseheaptmp ((Handle) (*val).data.stringvalue);
	
	return (setlongvalue (x, val));
	} /*stringtolong*/
	

static boolean stringtodirection (tyvaluerecord *val) {
	
	tydirection dir;
	bigstring bs;
	
	pullstringvalue (val, bs);
	
	if (!stringtodir (bs, &dir)) {
	
		langerror (invaliddirectionerror);
		
		return (false);
		}
		
	releaseheaptmp ((Handle) (*val).data.stringvalue);
	
	setdirectionvalue (dir, val);
	
	return (true);
	} /*stringtodirection*/


static boolean langstringtoostype (tyvaluerecord *val) {
	
	OSType type;
	bigstring bs;
	
	pullstringvalue (val, bs);
	
	if (!stringtoostype (bs, &type)) {
		
		langparamerror (ostypecoerceerror, bs);
		
		return (false);
		}
	
	releaseheaptmp ((Handle) (*val).data.stringvalue);
	
	setostypevalue (type, val);
	
	return (true);
	} /*langstringtoostype*/


boolean stringtoaddress (tyvaluerecord *val) {
	
	/*
	2/2/91 dmb: addresses are no longer just strings.  see comment 
	in addressofvalue
	
	7/10/91 dmb: special case empty string to empty address
	
	7/12/91 dmb: if htable is nil, do a searchpath lookup on it.
	
	10/14/91 dmb: fixed emptystring case; addresses still have length bytes, while 
	strings do not.
	
	7/21/97 dmb: support standalone window titles/paths with langfindexternalwindow

	5.1b21 dmb: leave htable nil for @root
	*/
	
	register tyvaluerecord *v = val;
	bigstring bs;
	hdlhashtable htable;
	
	pullstringvalue (v, bs);
	
	if (isemptystring (bs)) {
		
		htable = nil;
		
		/*
		(*v).valuetype = addressvaluetype;
		
		return (true);
		*/
		}
	else {
		
		if (!langexpandtodotparams (bs, &htable, bs)) {
		
			#ifdef xxxversion5orgreater
			
			hdlwindowinfo hinfo;
			
			if (flextendedsymbolsearch && shellfindnamedwindow (bs, &hinfo))
				htable = currenthashtable;
			else
			
			#endif
			
			return (false);
			}
		
		if ((htable == nil) && !equalstrings (bs, nameroottable))
			langsearchpathlookup (bs, &htable);
		}
	
	releaseheaptmp ((Handle) (*v).data.stringvalue);
	
	return (setaddressvalue (htable, bs, v));
	} /*stringtoaddress*/


/*
static boolean stringtofixed (tyvaluerecord *val) {
	
	if (!stringtolong (val))
		return (false);
	
	return (setfixedvalue (FixRatio ((*val).data.longvalue, 1), val));
	} /%stringtofixed%/
*/


static long nthint (bigstring bs, short n) {
	
	/*
	10/4/91 dmb: added range check, using coercetoint.  caller must check 
	fllangerror.
	
	3/18/92 dmb: pop leading spaces before converting to int
	*/
	
	long x;
	bigstring bsint;
	tyvaluerecord val;
	
	nthword (bs, n, ',', bsint);
	
	stringdeletechars (bsint, '(');
	
	stringdeletechars (bsint, ')');
	
	popleadingchars (bsint, ' ');
	
	if ((stringlength (bsint) > 2) && (bsint [1] == '0') && (bsint [2] == 'x'))
		hexstringtonumber (bsint, &x);
	else
		stringtonumber (bsint, &x);
	
	setlongvalue (x, &val);
	
	coercetoint (&val); /*may generate range-checking error*/
	
	return (val.data.intvalue);
	} /*nthint*/


static boolean stringtopoint (tyvaluerecord *val) {
	
	Point pt;
	bigstring bs;
	
	pullstringvalue (val, bs);
	
	if (countwords (bs, ',') != 2) {
		
		langparamerror (pointcoerceerror, bs);
		
		return (false);
		}
	
	pt.h = nthint (bs, 1);
	
	pt.v = nthint (bs, 2);
	
	if (fllangerror)
		return (false);
	
	releaseheaptmp ((Handle) (*val).data.stringvalue);
	
	setpointvalue (pt, val);
	
	return (true);
	} /*stringtopoint*/


static boolean stringtorect (tyvaluerecord *val) {
	
	Rect r;
	bigstring bs;
	
	pullstringvalue (val, bs);
	
	if (countwords (bs, ',') != 4) {
		
		langparamerror (rectcoerceerror, bs);
		
		return (false);
		}
	
	r.top = nthint (bs, 1);
	
	r.left = nthint (bs, 2);
	
	r.bottom = nthint (bs, 3);
	
	r.right = nthint (bs, 4);
	
	if (fllangerror)
		return (false);
	
	if (!sethandlecontents (&r, sizeof (r), (Handle) (*val).data.stringvalue))
		return (false);
	
	(*val).valuetype = rectvaluetype;
	
	return (true);
	} /*stringtorect*/


static boolean stringtorgb (tyvaluerecord *val) {
	
	RGBColor rgb;
	bigstring bs;
	
	pullstringvalue (val, bs);
	
	if (countwords (bs, ',') != 3) {
		
		langparamerror (rgbcoerceerror, bs);
		
		return (false);
		}
	
	rgb.red = nthint (bs, 1);
	
	rgb.green = nthint (bs, 2);
	
	rgb.blue = nthint (bs, 3);
	
	if (fllangerror)
		return (false);
	
	if (!sethandlecontents (&rgb, sizeof (rgb), (Handle) (*val).data.stringvalue))
		return (false);
	
	(*val).valuetype = rgbvaluetype;
	
	return (true);
	} /*stringtorgb*/


#if 0 //THINK_C

	#define pattern(p) (p)

#else
	
	#define pattern(p) (p.pat)

#endif

static boolean stringtopattern (tyvaluerecord *val) {
	
	/*
	10/14/91 dmb: implemented.
	*/
	
	bigstring bs;
	Pattern pat;
	
	pullstringvalue (val, bs);
	
	subtractstrings (bs, bshexprefix, bs); /*remove prefix, if present*/
	
	if (stringlength (bs) != 16)
		goto error;
	
	setstringlength (bs, 8); /*limit string to 1st longword of hex digits*/
	
	if (!hexstringtonumber (bs, (long *) &pattern (pat) [0])) /*first 8 characters weren't hex*/
		goto error;
	
	moveleft (bs + 9, bs + 1, 8); /*shift last 8 digits into string*/
	
	if (!hexstringtonumber (bs, (long *) &pattern (pat) [4])) /*second 8 characters bytes weren't hex*/
		goto error;
	
	if (!sethandlecontents (&pat, sizeof (Pattern), (Handle) (*val).data.stringvalue))
		return (false);
	
	(*val).valuetype = patternvaluetype;
	
	return (true);
	
	error:
	
	langparamerror (patterncoerceerror, bs);
	
	return (false);
	} /*stringtopattern*/


static boolean pointtostring (Point pt, bigstring bs) {
	
	shorttostring (pt.h, bs);
	
	pushchar (',', bs);
	
	pushint (pt.v, bs);
	
	return (true);
	} /*pointtostring*/


static boolean recttostring (Rect r, bigstring bs) {
	
	shorttostring (r.top, bs);
	
	pushchar (',', bs);
	
	pushint (r.left, bs);
	
	pushchar (',', bs);
	
	pushint (r.bottom, bs);
	
	pushchar (',', bs);
	
	pushint (r.right, bs);
	
	return (true);
	} /*recttostring*/


static boolean rgbtostring (RGBColor rgb, bigstring bs) {
	
	shorttostring (rgb.red, bs);
	
	pushchar (',', bs);
	
	pushint (rgb.green, bs);
	
	pushchar (',', bs);
	
	pushint (rgb.blue, bs);
	
	return (true);
	} /*rgbtostring*/


static boolean patterntostring (Pattern pat, bigstring bs) {
	bytestohexstring (&pat, sizeof (Pattern), bs);
	
	return (true);

	} /*patterntostring*/


boolean langgetspecialtable (bigstring bs, hdlhashtable *htable) {
	
	/*
	return true if the string is the name of one of the special tables.
	
	10/4/91 dmb: there is only one special table name: root.  I don't think 
	we need to check for "compiler" here anymore.
	*/
	
	if (equalstrings (bs, nameroottable)) {
		
		*htable = roottable;
		
		return (true); /*it is a special table*/
		}
	
	/*
	if (equalstrings (bs, nameinternaltable)) {
		
		*htable = internaltable;
		
		return (true); /%it is a special table%/
		}
	*/
	
	return (false); /*it isn't a special table, not an error*/
	} /*langgetspecialtable*/


boolean getaddresspath (tyvaluerecord val, bigstring bs) {
	
	/*
	if the address is in a local table, set bs to the identifier name.
	otherwise, set bs to the full path of the address.
	
	6/17/91 dmb: make sure table name itself is bracketed if necessary.

	5.0b16 dmb: must bracked locals too
	
	5.0.2b6 dmb: handle local tables that aren't stack frames

	5.1.4 dmb: handle unresolved addresses.
	
	6.2b18 AR: After the fix in langaddlocals in 6.2b16, scripts like following
	began to return unexpected results:
	
		local (adr = websites.adrdocserver); //points to a top-level GDB table
		msg (adr + "." + "string");
		
	This would yield D:\\docserver.root.string instead of ["D:\\docserver.root"].string.
	Here is where we attempt to fix it.
	*/
	
	register hdlhashtable ht;
	hdlhashtable htable;
	bigstring bspath;
	
 	if (!getaddressparts (&val, &htable, bs))
		return (false);

	if (htable == (hdlhashtable) -1)
		return (true);

	if (!validhandle ((Handle) htable))
		htable = nil;
	
	ht = htable; /*move into register*/
	
	if (ht == nil) { /*6.2b18 AR: What exactly does ht == nil imply?*/
		if (!isemptystring (bs)) /*6.2b18 AR*/
			langexternalbracketname (bs);			
		return (true);
		}

	if ((**ht).fllocaltable) {
		
		setemptystring (bspath);
		
		if ((**ht).prevhashtable == nil) // not a stack frame, has a path
			if (!langexternalgetquotedpath (ht, emptystring, bspath))
				return (false);
		}
	else  {
		
		if (langgetspecialtable (bs, &htable) && (htable == ht)) /*"root", etc.*/
			return (true);
		
		if (!langexternalgetquotedpath (ht, emptystring, bspath))
			return (false);
		}
	
	if (!isemptystring (bs)) {
		
		langexternalbracketname (bs);
		
		if (!isemptystring (bspath))
			pushchar ('.', bspath);
		}
	
	insertstring (bspath, bs);
	
	return (true);
	} /*getaddresspath*/


static boolean addresstostring (tyvaluerecord *val) {
	
	register tyvaluerecord *v = val;
	bigstring bspath;
	
	if (!getaddresspath (*v, bspath))
		return (false);
	
	releaseheaptmp ((Handle) (*v).data.addressvalue);
	
	return (setstringvalue (bspath, v));
	} /*addresstostring*/


static boolean binarytoscalar (tyvaluerecord *val, tyvaluetype type) {
	
	/*
	5.0a22 dmb: handle any-endian
	*/

	register tyvaluerecord *v = val;
	register Handle h = (*v).data.binaryvalue;
	tyvaluerecord scalarval;
	long size;
	long x;
	
	scalarval.valuetype = type;
	
	if (!langgetvalsize (scalarval, &size))
		return (false);
	
	if (gethandlesize (h) - sizeof (OSType) != (unsigned long) size) {
		
		langlongparamerror (binarycoerceerror, size);
		
		return (false);
		}
	
	x = 0; /*clear all bytes*/
	
	moveleft (*h + sizeof (OSType), &x, size);
	
	/*
	if (!(*v).fl.tmpdata)
	*/
		releaseheaptmp (h);
	
	initvalue (v, type);
	
	(*v).data.longvalue = x;

	/*
	switch (size) {
		
		case 1:
			(*v).data.chvalue = *(byte *)&x;
			break;
		case 2:
			(*v).data.intvalue = conditionalshortswap (*(short *)&x);
			break;
		case 4:
		default:
			(*v).data.longvalue = conditionallongswap (x);
			break;
		}
	*/

	return (true);
	} /*binarytoscalar*/


boolean coercebinaryval (tyvaluerecord *val, tyvaluetype type, long size, tyvaluetype binarytype) {
	
	/*
	change the type of the given binary value to type.  if the size 
	parameter is greater than zero, require that the current value be 
	that size before doing the coersion
	
	2.1b3 dmb: added 'type' parameter for binarytype validity checking
	*/
	
	register tyvaluerecord *v = val;
	OSType typeid;
	
	if (size > 0) { /*specific size required*/
		
		if (gethandlesize ((*v).data.binaryvalue) - sizeof (OSType) != (unsigned long) size) {
			
			langlongparamerror (binarycoerceerror, size);
			
			return (false);
			}
		}
	
	if (binarytype != novaluetype) { /*specific type required*/
		
		typeid = langgettypeid (binarytype);
		
		if (getbinarytypeid ((*v).data.binaryvalue) != typeid) {
			
			langostypeparamerror (binarytypecoerceerror, typeid);
			
			return (false);
			}
		}
	
	copyvaluedata (v); /*make sure we own handle*/
	
	pullfromhandle ((*v).data.binaryvalue, 0L, sizeof (typeid), &typeid);
	
	(*v).valuetype = type;
	
	return (true);
	} /*coercebinaryval*/


boolean langcoerceerror (tyvaluerecord *val, tyvaluetype valuetype) {
	
	bigstring bstype1, bstype2;
	
	langgettypestring ((*val).valuetype, bstype1);
	
	langgettypestring (valuetype, bstype2);
	
	lang2paramerror (coercionnotpossibleerror, bstype1, bstype2);
	
	return (true);
	} /*langcoerceerror*/


static boolean getbinarynumber (Handle x, long *n) {
	
	/*
	return a numeric interpretation of the binary handle.
	
	we're allowed to munge the handle; the caller will be disposing it.
	*/
	
	register long ctbytes;
	
	ctbytes = gethandlesize (x) - sizeof (OSType);
	
	switch(ctbytes) {
		
		case 1:
			*n = ((*(char **) x) [4]);
			
			break;
		
		case 2:
			*n = conditionalshortswap ((*(short **) x) [2]);
			
			break;
		
		case 4:
			*n = conditionallongswap ((*(long **) x) [1]);
			
			break;
		
		default:
			return (false);
		}
	
	return (true);
	} /*getbinarynumber*/



boolean coercetoboolean (tyvaluerecord *v) {
	
	//
	// 2006-06-24 creedon: for Mac, FSRef-ized
	//
	// 2.1b4 dmb: handle filespec values
	//
	// 1992-02-07 dmb: binary to boolean is now true when non-empty, except when it's the size of a boolean.
	//
	
	register boolean fl;
	
	switch ((*v).valuetype) {
		
		case booleanvaluetype:
			return (true);
		
		case novaluetype:
			if (flinhibitnilcoercion)
				return (false);
			
			fl = false;
			
			break;
		
		case charvaluetype:
			fl = (*v).data.chvalue != 0;
			
			break;
		
		case intvaluetype:
			fl = (*v).data.intvalue != 0;
			
			break;
		
		case longvaluetype:
		case datevaluetype:
		case ostypevaluetype:
		case fixedvaluetype:
			fl = (*v).data.longvalue != 0;
			
			break;
		
		case enumvaluetype:
			fl = (*v).data.enumvalue != 0 && (*v).data.enumvalue != 'fals';
		
			break;
		
		case directionvaluetype:
			fl = (*v).data.dirvalue != nodirection;
			
			break;
		
		case addressvaluetype:
			if (!addresstostring (v))
				return (false);
			
			/*no break; fall thru to string coersion*/
		
		case stringvaluetype:
			return (stringtoboolean (v));
		
		case singlevaluetype:
			fl = (*v).data.singlevalue != 0.0;
			
			break;
		
		case doublevaluetype:
			fl = **(*v).data.doublevalue != 0.0;
			
			break;
		
		case binaryvaluetype: {
			long n;
			register Handle x = (*v).data.binaryvalue;
			
			if (getbinarynumber (x, &n))
				fl = n != 0;
			else
				fl = gethandlesize (x) > sizeof (OSType);
			
			releaseheaptmp (x);
			
			break;
			}
		
		case filespecvaluetype: {
		
			bigstring bs;

			getfsfile ( *( *v ).data.filespecvalue, bs ); // 3/7/97 dmb

			fl = ! isemptystring (bs);
			
			break;
		
			}
			
		case listvaluetype:
		case recordvaluetype:
			return (coercelistvalue (v, booleanvaluetype));
		
		default:
			langerror (booleancoerceerror);
			
			return (false);
		} /*switch*/
	
	setbooleanvalue (fl, v);
	
	return (true);
	} /*coercetoboolean*/


static boolean longrangeerror (double f) {
	
	bigstring bs;
	
	if ((f > 2147483647.0) || (f < -2147483648.0)) {
		
		floattostring (f, bs);
		
		langparamerror (floattolongerror, bs);
		
		return (true);
		}
	
	return (false);
	} /*longrangeerror*/


boolean coercetolong (tyvaluerecord *v) {
	
	/*
	12/22/92 dmb: must releaseheaptmp when coercing binary type
	
	3.0.2 dmb: range check when coercing floats
	*/
	
	register long x;
	register double f;
	
	switch ((*v).valuetype) {
		
		case longvaluetype:
			return (true);
		
		case novaluetype:
			if (flinhibitnilcoercion)
				return (false);
			
			x = (long) 0;
			
			break;
		
		case booleanvaluetype:
			x = (long) (*v).data.flvalue;
			
			break;
		
		case charvaluetype:
			x = (long) (*v).data.chvalue;
			
			break;
		
		case intvaluetype:
			x = (long) (*v).data.intvalue;
			
			break;
		
		case directionvaluetype:
			x = (long) (*v).data.dirvalue;
			
			break;
		
		case datevaluetype:
			x = (long) (*v).data.datevalue;
			
			break;
		
		case stringvaluetype:
			return (stringtolong (v));
		
		case ostypevaluetype:
		case pointvaluetype:
			x = (long) (*v).data.ostypevalue;
			
			break;

		case fixedvaluetype:
			x = (long) FixRound ((*v).data.fixedvalue);
			
			break;
			
		case singlevaluetype:
			f = (*v).data.singlevalue;
			
			if (longrangeerror (f))
				return (false);
			
			x = (long) f;
			
			break;
		
		case doublevaluetype:
			f = **(*v).data.doublevalue;
			
			if (longrangeerror (f))
				return (false);
			
			x = (long) f;
			
			break;
		
		case binaryvaluetype: {
			long n;
			register Handle h = (*v).data.binaryvalue;
			
			if (!getbinarynumber (h, &n))
				return (binarytoscalar (v, longvaluetype));
			
			x = n;
			
			releaseheaptmp (h);
			
			break;
			}
		
		case listvaluetype:
		case recordvaluetype:
			return (coercelistvalue (v, longvaluetype));
		
		default:
			langerror (longcoerceerror);
			
			return (false);
		} /*switch*/
	
	return (setlongvalue (x, v));
	} /*coercetolong*/


static boolean coercetolongfortype (tyvaluerecord *v, tyvaluetype type) {
	
	boolean fl;
	
	disablelangerror ();
	
	fl = coercetolong (v);
	
	enablelangerror ();
	
	if (!fl)
		langcoerceerror (v, type);
	
	return (fl);
	} /*coercetolongfortype*/


boolean coercetoint (tyvaluerecord *v) {
	
	/*
	4/26/96 4.0b7 dmb: special case novaluetype since flinhibitnilcoercion can't
	pass though coercetolongfortype
	*/
	
	register long x;
	
	switch ((*v).valuetype) {
		
		case intvaluetype:
			return (true);
		
		case novaluetype:
			if (flinhibitnilcoercion)
				return (false);
			
			x = 0;
			
			break;
		
		case binaryvaluetype: {
			long n;
			register Handle h = (*v).data.binaryvalue;
			
			if (!getbinarynumber (h, &n))
				return (binarytoscalar (v, intvaluetype));
			
			x = n;
			
			releaseheaptmp (h);
			
			break;
			}
		
		case stringvaluetype:
			if (!stringtolong (v))
				return (false);
			
			x = (*v).data.longvalue;
			
			break;
		
		case listvaluetype:
		case recordvaluetype:
			return (coercelistvalue (v, intvaluetype));
		
		default:
			if (!coercetolongfortype (v, intvaluetype))
				return (false);
			
			x = (*v).data.longvalue;
		}
	
	if (x > intinfinity) {
		
		if (x == longinfinity) /*the one exception to the rule*/
		
			x = intinfinity; /*trade the long version of infinity for the short version*/
			
		else {
			
			langlongparamerror (inttoolargeerror, x);
			
			return (false);
			}
		}
	
	if (x < intminusinfinity) {
		
		langlongparamerror (inttoosmallerror, x);
		
		return (false);
		}
	
	return (setintvalue ((short) x, v));
	} /*coercetoint*/


static boolean coercetochar (tyvaluerecord *v) {
	
	register long x;
	
	switch ((*v).valuetype) {
		
		case charvaluetype:
			return (true);
		
		case novaluetype:
			if (flinhibitnilcoercion)
				return (false);
			
			x = (long) '0';
			
			break;
		
		case booleanvaluetype:
			x = (long) (*v).data.flvalue;
			
			break;
		
		case intvaluetype:
			x = (long) (*v).data.intvalue;
			
			break;
		
		case stringvaluetype:
			return (stringtochar (v));
		
		case longvaluetype:
			x = (long) (*v).data.longvalue;
			
			break;	
			
		case directionvaluetype:
			x = (long) (*v).data.dirvalue;
			
			break;
		
		case binaryvaluetype:
			return (binarytoscalar (v, charvaluetype));
		
		case listvaluetype:
		case recordvaluetype:
			return (coercelistvalue (v, charvaluetype));
		
		default:
			langerror (charcoerceerror);
			
			return (false);
		} /*switch*/
	
	if ((x < 0) || (x > 255)) {
		
		langlongparamerror (charoutofrangeerror, x);
		
		return (false);
		}
	
	setcharvalue ((byte) x, v);
	
	return (true);
	} /*coercetochar*/


static boolean coercetodirection (tyvaluerecord *v) {
	
	register tydirection x;
	
	switch ((*v).valuetype) {
		
		case directionvaluetype:
			return (true);
		
		case novaluetype:
			if (flinhibitnilcoercion)
				return (false);
			
			x = (tydirection) nodirection;
			
			break;
		
		case longvaluetype:
		case ostypevaluetype:
			x = (tydirection) (*v).data.longvalue;
			
			break;
			
		case datevaluetype:
			x = (tydirection) (*v).data.datevalue;
			
			break;
		
		case booleanvaluetype:
			x = (tydirection) (*v).data.flvalue;
			
			break;
		
		case charvaluetype:
			x = (tydirection) (*v).data.chvalue;
			
			break;
		
		case intvaluetype:
			x = (tydirection) (*v).data.intvalue;
			
			break;
			
		case stringvaluetype:
			return (stringtodirection (v));
		
		case binaryvaluetype:
			return (binarytoscalar (v, directionvaluetype));
		
		case listvaluetype:
		case recordvaluetype:
			return (coercelistvalue (v, directionvaluetype));
		
		default:
			langcoerceerror (v, directionvaluetype);
			
			return (false);
		} /*switch*/
	
	if (!validdirection (x)) {
		
		langerror (invaliddirectionerror);
		
		return (false);
		}
	
	setdirectionvalue (x, v);
	
	return (true);
	} /*coercetodirection*/
	

static boolean coercetodate (tyvaluerecord *v) {
	
	/*
	9/13/91 dmb: use new stringtotime to coerce from string
	*/
	
	register long x;
	
	switch ((*v).valuetype) {
		
		case datevaluetype:
			return (true);
		
		case novaluetype: {
			if (flinhibitnilcoercion)
				return (false);
			
			x = timenow ();
			
			break;
			}
		
		case longvaluetype:
		case ostypevaluetype:
			x = (*v).data.longvalue;
			
			break;
		
		case charvaluetype:
			x = (long) (*v).data.chvalue;
			
			break;
		
		case booleanvaluetype:
			x = (long) (*v).data.flvalue;
			
			break;
		
		case intvaluetype:
			x = (long) (*v).data.intvalue;
			
			break;
			
		case directionvaluetype:
			x = (long) (*v).data.dirvalue;
			
			break;
		
		case stringvaluetype: {
			bigstring bs;
			unsigned long ltime;
			
			pullstringvalue (v, bs);
			
			if (!stringtotime (bs, &ltime)) {
				
				langerror (datecoerceerror);
				
				return (false);
				}
			
			x = ltime;
			
			break;
			}
		
		case binaryvaluetype:
			return (binarytoscalar (v, datevaluetype));
		
		case listvaluetype:
		case recordvaluetype:
			return (coercelistvalue (v, datevaluetype));
		
		default:
			langerror (datecoerceerror);
			
			return (false);
		} /*switch*/
	
	setdatevalue (x, v);
	
	return (true);
	} /*coercetodate*/


boolean coercetoostype (tyvaluerecord *v) {
	
	register OSType x;
	
	switch ((*v).valuetype) {
		
		case ostypevaluetype:
			return (true);
		
		case novaluetype:
			if (flinhibitnilcoercion)
				return (false);
			
			x = (OSType) '\?\?\?\?';
			
			break;
		
		case longvaluetype:
		case enumvaluetype:
			x = (OSType) (*v).data.longvalue;
			
			break;
		
		case directionvaluetype:
			x = (OSType) (*v).data.dirvalue;
			
			break;
		
		case datevaluetype:
			x = (OSType) (*v).data.datevalue;
			
			break;
		
		case booleanvaluetype:
			x = (OSType) (*v).data.flvalue;
			
			break;
		
		case charvaluetype:
			x = (OSType) (*v).data.chvalue;
			
			break;
		
		case intvaluetype:
			x = (OSType) (*v).data.intvalue;
			
			break;
		
		case stringvaluetype:
			return (langstringtoostype (v));
		
		case binaryvaluetype:
			return (binarytoscalar (v, ostypevaluetype));
		
		case listvaluetype:
		case recordvaluetype:
			return (coercelistvalue (v, ostypevaluetype));
		
		default:
			langcoerceerror (v, ostypevaluetype);
			
			return (false);
		} /*switch*/
	
	setostypevalue (x, v);
	
	return (true);
	} /*coercetoostype*/


static boolean coercetopoint (tyvaluerecord *v) {
	
	register Point x;
	
	switch ((*v).valuetype) {
		
		case pointvaluetype:
			return (true);
		
		case novaluetype:
			if (flinhibitnilcoercion)
				return (false);
			
			x.h = x.v = 0;
			
			break;
		
		case ostypevaluetype:
		case longvaluetype:
			x = *(Point *) &(*v).data.longvalue;
			
			break;
		
		case stringvaluetype:
			return (stringtopoint (v));
		
		case binaryvaluetype:
			return (binarytoscalar (v, pointvaluetype));
		
		case listvaluetype:
		case recordvaluetype:
			return (coercelistvalue (v, pointvaluetype));
		
		default:
			langcoerceerror (v, pointvaluetype);
			
			return (false);
		} /*switch*/
	
	setpointvalue (x, v);
	
	return (true);
	} /*coercetopoint*/


static boolean newheaprecordvalue (long size, tyvaluetype type, tyvaluerecord *val) {
	
	Handle x;
	
	if (!newclearhandle (size, &x))
		return (false);
	
	return (setheapvalue (x, type, val));
	} /*newheaprecordvalue*/


static boolean coercetorect (tyvaluerecord *v) {
	
	switch ((*v).valuetype) {
		
		case rectvaluetype:
			return (true);
		
		case novaluetype:
			if (flinhibitnilcoercion)
				return (false);
			
			return (newheaprecordvalue (sizeof (Rect), rectvaluetype, v));
		
		case stringvaluetype:
			return (stringtorect (v));
		
		case binaryvaluetype:
			return (coercebinaryval (v, rectvaluetype, sizeof (Rect), novaluetype));
		
		case listvaluetype:
		case recordvaluetype:
			return (coercelistvalue (v, rectvaluetype));
		
		default:
			langcoerceerror (v, rectvaluetype);
			
			return (false);
		} /*switch*/
	} /*coercetorect*/


boolean coercetorgb (tyvaluerecord *v) {
	
	switch ((*v).valuetype) {
		
		case rgbvaluetype:
			return (true);
		
		case novaluetype:
			if (flinhibitnilcoercion)
				return (false);
			
			return (newheaprecordvalue (sizeof (RGBColor), rgbvaluetype, v));
		
		case stringvaluetype:
			return (stringtorgb (v));
		
		case binaryvaluetype:
			return (coercebinaryval (v, rgbvaluetype, sizeof (RGBColor), novaluetype));
		
		case listvaluetype:
		case recordvaluetype:
			return (coercelistvalue (v, rgbvaluetype));
		
		default:
			langcoerceerror (v, rgbvaluetype);
			
			return (false);
		} /*switch*/
	} /*coercetorgb*/


static boolean coercetopattern (tyvaluerecord *v) {
	switch ((*v).valuetype) {
		
		case patternvaluetype:
			return (true);
		
		case novaluetype:
			if (flinhibitnilcoercion)
				return (false);
			
			return (newheaprecordvalue (sizeof (Pattern), patternvaluetype, v));
		
		case stringvaluetype:
			return (stringtopattern (v));
		
		case binaryvaluetype:
			return (coercebinaryval (v, patternvaluetype, sizeof (Pattern), novaluetype));
		
		case listvaluetype:
		case recordvaluetype:
			return (coercelistvalue (v, patternvaluetype));
		
		default:
			langcoerceerror (v, patternvaluetype);
			
			return (false);
		} /*switch*/

	} /*coercetopattern*/


static boolean coercetofixed (tyvaluerecord *v) {
	/*
	6/29/92 dmb: added support for single & double types
	
	4/26/96 4.0b7 dmb: special case novaluetype since flinhibitnilcoercion can't
	pass though coercetolongfortype
	*/
	
	Fixed x;
	
	switch ((*v).valuetype) {
		
		case fixedvaluetype:
			return (true);
		
		case novaluetype:
			if (flinhibitnilcoercion)
				return (false);
			
			x = 0;
			
			break;
		
		/*
		case stringvaluetype:
			return (stringtofixed (v));
		*/
		
		case singlevaluetype:
			x = (Fixed) (long) ((*v).data.singlevalue * 65536.0);
			
			break;
		
		case doublevaluetype:
			x = (Fixed) (long) (**(*v).data.doublevalue * 65536.0);
			
			releaseheaptmp ((Handle) (*v).data.doublevalue);
			
			break;
		
		case binaryvaluetype:
			return (binarytoscalar (v, fixedvaluetype));
		
		case listvaluetype:
		case recordvaluetype:
			return (coercelistvalue (v, fixedvaluetype));
		
		default:
			if (!coercetolongfortype (v, fixedvaluetype))
				return (false);
			
			if (!coercetoint (v)) /*may generate range-checking error*/
				return (false);
			
			x = FixRatio ((*v).data.intvalue, 1);
			
			break;
		} /*switch*/
	
	return (setfixedvalue (x, v));

	} /*coercetofixed*/


static boolean coercetosingle (tyvaluerecord *v) {
	
	/*
	6/29/92 dmb: added support for non-trucated fixed->float coercion
	
	4/26/96 4.0b7 dmb: special case novaluetype since flinhibitnilcoercion can't
	pass though coercetolongfortype
	*/
	
	float x;
	
	switch ((*v).valuetype) {
		
		case singlevaluetype:
			return (true);
		
		case doublevaluetype:
			x = (float) **(*v).data.doublevalue;
			
			break;
		
		case novaluetype:
			if (flinhibitnilcoercion)
				return (false);
			
			x = (float) 0.0;
			
			break;
		
		case fixedvaluetype:
			x = (float) ((*v).data.longvalue / 65536.0);
			
			break;
		
		case stringvaluetype: {
			double d;
			bigstring bs;
			
			pullstringvalue (v, bs);
			
			if (!stringtofloat (bs, &d)) {
				
				langparamerror (floatcoerceerror, bs);
				
				return (false);
				}
			
			x = (float) d;
			
			break;
			}
		
		case binaryvaluetype:
			return (binarytoscalar (v, singlevaluetype));
		
		case listvaluetype:
		case recordvaluetype:
			return (coercelistvalue (v, singlevaluetype));
		
		default:
			if (!coercetolongfortype (v, singlevaluetype))
				return (false);
			
			x = (float) (*v).data.longvalue;
			
			break;
		} /*switch*/
	
	disposevaluerecord (*v, true);
	
	return (setsinglevalue (x, v));
	} /*coercetosingle*/


static boolean coercetodouble (tyvaluerecord *v) {
	
	/*
	3/5/92 dmb: added missing break statement in singlevaluetype case, fixing bus error
	
	6/29/92 dmb: added support for non-trucated fixed->double coercion
	
	12/21/92 dmb: added case for datetype to preserve unsigned-ness
	
	4/26/96 4.0b7 dmb: special case novaluetype since flinhibitnilcoercion can't
	pass though coercetolongfortype
	*/
	
	double x;
	
	switch ((*v).valuetype) {
		
		case doublevaluetype:
			return (true);
		
		case singlevaluetype:
			x = (*v).data.singlevalue;
			
			break;
		
		case novaluetype:
			if (flinhibitnilcoercion)
				return (false);
			
			x = 0.0;
			
			break;
		
		case stringvaluetype: {
			bigstring bs;
			
			pullstringvalue (v, bs);
			
			if (!stringtofloat (bs, &x)) {
				
				langparamerror (floatcoerceerror, bs);
				
				return (false);
				}
			
			break;
			}
		
		case fixedvaluetype:
			x = (double) (*v).data.longvalue / 65536.0;
			
			break;
		
		case datevaluetype:
			x = (double) (*v).data.datevalue;
			
			break;
		
		case binaryvaluetype:
#if noextended
		{
			long double lx;
			extended80 x80;

			/*first do type & length checking, resulting in x80 value in v*/

				if (!coercebinaryval (v, doublevaluetype, sizeof (extended80), novaluetype))
				return (false);

			/*now convert to actual double value*/

			x80 = (**(extended80 **) (*v).data.doublevalue);

			safex80told (&x80, &lx);
			return (setdoublevalue (lx, v));
		}
#else

			return (coercebinaryval (v, doublevaluetype, sizeof (double), novaluetype));

			#endif
		
		case listvaluetype:
		case recordvaluetype:
			return (coercelistvalue (v, doublevaluetype));
		
		default:
			if (!coercetolongfortype (v, doublevaluetype))
				return (false);
			
			x = (*v).data.longvalue;
			
			break;
		} /*switch*/
	
	disposevaluerecord (*v, true);
	
	return (newheapvalue (&x, sizeof (x), doublevaluetype, v));
	} /*coercetodouble*/


static unsigned char hexchartonum (unsigned char ch) {

	if ((ch >= 'a') && (ch <= 'z')) /*DW 10/13/95 -- fix for toys.parseArgs*/
		ch -= 32;

	if ((ch >= '0') && (ch <= '9'))
		ch = ch - '0';
	else
		ch = (ch - 'A') + 10;
	
	return (ch);
	} /*hexchartonum*/

static void decode (bigstring bs) {
	unsigned long ixtext, lentext;
	unsigned char *p;

	p = (unsigned char *) stringbaseaddress (bs);

	lentext = stringlength (bs);

	ixtext = 0;

	while (true) {

		if (ixtext >= lentext)
			break;

		switch (*p) {

			case '%': {
				unsigned char ch1 = *(p + 1), ch2 = *(p + 2);

				/*{Str255 s; s [0] = 3; s [1] = '%'; s [2] = ch1; s [3] = ch2; DebugStr (s);}*/

				*p = (hexchartonum (ch1) * 16) + hexchartonum (ch2);
				
				moveleft (p + 3, p + 1, lentext - ixtext - 3);

				lentext -= 2;

				break;
				}

			case '+':
				*p = ' ';

				break;
			} /*switch*/

		p++;

		ixtext++;
		} /*while*/

	setstringlength (bs, lentext);
	} /*decode*/


boolean coercetofilespec (tyvaluerecord *v) {
	
	//
	// 2006-06-24 creedon: for Mac, FSRef-ized
	//
	// 2.1b12 dmb: changed string->filespec error message to be filenotfounderror
	//
	// 2.1b3 dmb: coerce zero to nil filespec
	//
	// 2.1b2 dmb: don't enforce any particular size when coercing from binary; filespecs are now variable-length
	//
	
	bigstring bs;
	byte fileurl [] = "\x08" "file:///";
	tyfilespec fs;
	
	clearbytes ( &fs, sizeof ( fs ) );
	
	switch ((*v).valuetype) {
		
		case filespecvaluetype:
			return ( true );
		
		case novaluetype:
			if (flinhibitnilcoercion)
				return (false);
			
			break;
		
		case longvaluetype:
			if ( (*v).data.longvalue != 0)
				langcoerceerror (v, filespecvaluetype);
			
			break;
		
		case stringvaluetype:
			pullstringvalue (v, bs);
				
			if (equaltextidentifiers (stringbaseaddress(bs), stringbaseaddress(fileurl), stringlength(fileurl) )) {
				
				
				/* Convert string to standard file string.*/
				deletestring (bs, 1, stringlength (fileurl));
				
				decode (bs);

					stringreplaceall ('/', ':', bs);
				}
			
			if (!pathtofilespec (bs, &fs)) {
				
				filenotfounderror (bs);
				
				/*
				langparamerror (filespeccoerceerror, bs);
				*/
				
				return (false);
				}
			
			break;
		
		case binaryvaluetype:
			return (coercebinaryval (v, filespecvaluetype, 0L, filespecvaluetype));
		
		case listvaluetype:
		case recordvaluetype:
			return (coercelistvalue (v, filespecvaluetype));
		
		case aliasvaluetype:
			if (!aliastofilespec ((AliasHandle) (*v).data.aliasvalue, &fs))
				return (false);
			
			break;
		
		case objspecvaluetype:
			return (objspectofilespec (v));
		
		default:
			langcoerceerror (v, filespecvaluetype);
			
			return (false);
		} /*switch*/
	
	disposevaluerecord (*v, true);
	
	return (newheapvalue (&fs, sizeof (fs), filespecvaluetype, v));	
	} /*coercetofilespec*/


static void  bigvaltostring (tyvaluerecord *v, bigstring bs) {
	
	/*
	7.0b59 PBS: fixed overflow bug by making bsval larger.
	A string such as "12345 characters" would cause an overflow of one byte.
	*/

	//byte bsval [16];
	byte bsval [32];
	
	hashgettypestring (*v, bs);
	
	pushchar (':', bs);
	pushchar (' ', bs);
	
	hashgetvaluestring (*v, bsval);
	
	pushstring (bsval, bs);
	} /*bigvaltostring*/


boolean coercetostring (tyvaluerecord *val) {

	//
	// 2007-06-12 creedon: empty bs at start of function, fix for problem
	//				   w/filespecvaluetype case returning gibberish for
	//				   invalid fs
	//
	// 2006-06-24 creedon: for Mac, FSRef-ized
	//
	// 4.1b4 dmb: if flcoerceexternaltostring is not enabled, create a
	//		    reasonable display string for externals
	//
	// 2.1b3 dmb: don't ignore return value from objspectostring
	//
	// 1992-08-10 dmb: added flcoerceexternaltostring flag to prevent
	//			    external-to-string coercion except when explicitly
	//			    requested by stringfunc in langverbs.c
	//
	
	register tyvaluerecord *v = val;
	
	Handle h;
	bigstring bs;
	
	setemptystring ( bs );
	
	if (!langheapallocated (v, &h))
		h = nil;
		
	switch ((*v).valuetype) {
	
		case stringvaluetype:
			return (true);
			
		case novaluetype: {
		
			if ( flinhibitnilcoercion )
				return ( false );
				
			break;
			
			}
			
		/*
		case passwordvaluetype:
			(*v).valuetype = stringvaluetype;
			
			return (true);
		*/
		
		case addressvaluetype:
			return (addresstostring (v));
			
		case booleanvaluetype:
			if ((*v).data.flvalue)
				copystring (bstrue, bs);
			else
				copystring (bsfalse, bs);
				
			break;
			
		case charvaluetype:
			setstringwithchar ((*v).data.chvalue, bs);
			
			break;
			
		case intvaluetype:
			shorttostring ((*v).data.intvalue, bs);
			
			break;
			
		case longvaluetype:
			numbertostring ((*v).data.longvalue, bs);
			
			break;
			
		case ostypevaluetype:
		case enumvaluetype:
			ostypetostring ((*v).data.ostypevalue, bs);
			
			break;
			
		case directionvaluetype:
			dirtostring ((*v).data.dirvalue, bs);
			
			break;
			
		case datevaluetype:
			timedatestring ((*v).data.datevalue, bs);
			
			break;
			
		case fixedvaluetype: {
			double x = (double) (*v).data.longvalue / 65536;
			
			floattostring (x, bs);
			
			/*
			shorttostring (FixRound ((*v).data.fixedvalue), bs);
			*/
			
			break;
			}
			
		case singlevaluetype:
			floattostring ((*v).data.singlevalue, bs);
			
			break;
			
		case doublevaluetype:
			floattostring (**(*v).data.doublevalue, bs);
			
			break;
			
		case pointvaluetype:
			pointtostring ((*v).data.pointvalue, bs);
			
			break;
			
		case rectvaluetype:
			recttostring (**(*v).data.rectvalue, bs);
			
			break;
			
		case rgbvaluetype:
			rgbtostring (**(*v).data.rgbvalue, bs);
			
			break;
			
		case patternvaluetype:
			patterntostring (**(*v).data.patternvalue, bs);
			
			break;
			
		case objspecvaluetype:
			if (!objspectostring ((*v).data.objspecvalue, bs))
				return (false);
				
			break;
			
		case aliasvaluetype:
			aliastostring ((*v).data.aliasvalue, bs);
			
			break;
			
		case filespecvaluetype: {
		
			tyfilespec fs = **( *v ).data.filespecvalue;
			
			filespectopath ( &fs, bs );
			
			break;
			
			}
			
		case binaryvaluetype:
			if (!copyvaluedata (v))
				return (false);
				
			stripbinarytypeid ((*v).data.binaryvalue);
			
			(*v).valuetype = stringvaluetype;
			
			return (true);
			
		case listvaluetype:
		case recordvaluetype:
			return (coercelistvalue (v, stringvaluetype));
			
		case codevaluetype:
			bigvaltostring (v, bs);
			
			break;
			
		case externalvaluetype:
			if (!flcoerceexternaltostring) {
			
				/* 4.1b4 dmb*/
				/*
				langbadexternaloperror (badexternaloperationerror, *v);
				return (false);
				*/
				bigvaltostring (v, bs);
				
				break;
				}
				
			if (!newemptyhandle (&h))
				return (false);
				
			if (!langexternalpacktotext ((hdlexternalhandle) (*v).data.externalvalue, h)) {
			
				disposehandle (h);
				
				return (false);
				}
				
			disposevaluerecord (*v, true);
			
			return (setheapvalue (h, stringvaluetype, v));
			
		default:
			langerror (stringcoerceerror);
			
			return (false);
		} /*switch*/
		
	disposevaluerecord (*v, true);
	
	return (setstringvalue (bs, v));
	} /*coercetostring*/


boolean coercetoaddress (tyvaluerecord *v) {
	
	/*
	9/23/92 dmb: use new objspectoaddress for special obj specifier handling
	*/
	
	register tyvaluetype type = (*v).valuetype;
	bigstring bs;
	boolean fl;
	
	if (type == addressvaluetype)
		return (true);
	
	disablelangerror ();
	
	if (type == objspecvaluetype) /*special case; don't want use string representation*/
		fl = objspectoaddress (v);
	else
		fl = coercetostring (v) && stringtoaddress (v);
	
	enablelangerror ();
	
	if (!fl) {
		
		coercetostring (v);
		
		pullstringvalue (v, bs);
		
		langparamerror (addresscoerceerror, bs);
		
		return (false);
		}
	
	return (true);
	} /*coercetoaddress*/


boolean coercetobinary (tyvaluerecord *val) {
	
	/*
	12/24/91 dmb: added special case for coercing null objspec
	*/
	
	register tyvaluerecord *v = val;
	OSType typeid;
	
	typeid = langgettypeid ((*v).valuetype);
	
	switch ((*v).valuetype) {
		
		case binaryvaluetype:
			return (true);
		
		case novaluetype:
			if (flinhibitnilcoercion)
				return (false);
			
			if (!setstringvalue (emptystring, v))
				return (false);
			
			break;
		
		#if noextended
			
			case doublevaluetype: {
				Handle x;
				
				if (!langpackvalue (*v, &x, HNoNode))
					return (false);
				
				return (setheapvalue (x, binaryvaluetype, v));
				}
		
		#else
		
			case doublevaluetype:
			
		#endif
		
		#ifndef oplanglists
			case listvaluetype:
			case recordvaluetype:
		#endif
		case stringvaluetype:
		case passwordvaluetype:
		case rectvaluetype:
		case patternvaluetype:
		case rgbvaluetype:
		case filespecvaluetype:
		case aliasvaluetype:
			break;
		
		#ifdef oplanglists
			case listvaluetype:
			case recordvaluetype: {
				Handle x;
				
				if (!langpackvalue (*v, &x, HNoNode))
					return (false);
				
				return (setheapvalue (x, binaryvaluetype, v));
				}
		#endif
		
		case objspecvaluetype:
			if ((*v).data.objspecvalue == nil) { /*null spec; special case*/
				
				if (!setstringvalue (zerostring, v))
					return (false);
				
				typeid = 'null';
				}
			
			break;
		
		default: {
				Handle x;
				
				if (!langpackvalue (*v, &x, HNoNode))
					return (false);
				
				return (setheapvalue (x, binaryvaluetype, v));
				}
			
			langerror (cantcoercetobinaryerror);
			
			return (false);
		} /*switch*/
	
	memtodisklong (typeid);

	if (!insertinhandle ((*v).data.binaryvalue, 0L, &typeid, sizeof (typeid)))
		return (false);
	
	(*v).valuetype = binaryvaluetype;
	
	(*v).fltmpdata = 0;
	
	return (true);
	} /*coercetobinary*/

/*
boolean truevalue (tyvaluerecord val) { //6.2b15 AR: removed because the return value of coercetoboolean can't be ignored
	
	coercetoboolean (&val);
	
	return (val.data.flvalue);
	}*/ /*truevalue*/


boolean coercevalue (tyvaluerecord *val, tyvaluetype valuetype) {
	
	/*
	coerce the indicated valuerecord to the indicated type, if possible.
	
	return true if it worked.
	
	2.1b2 dmb: added coercion to notype
	*/
	
	register tyvaluerecord *v = val;
	
	switch (valuetype) {
		
		case novaluetype:
			disposevaluerecord (*v, true);
			
			initvalue (v, novaluetype);
			
			return (true);
		
		case booleanvaluetype:
			return (coercetoboolean (v));
			
		case charvaluetype:
			return (coercetochar (v));
		
		case intvaluetype:
			return (coercetoint (v));
		
		case longvaluetype:
			return (coercetolong (v));
		
		case directionvaluetype:
			return (coercetodirection (v));
		
		case ostypevaluetype:
			return (coercetoostype (v));
		
		case enumvaluetype:
			if (!coercetoostype (v))
				return (false);
			
			(*v).valuetype = enumvaluetype;
			
			return (true);
		
		case stringvaluetype:
			return (coercetostring (v));
		
		case addressvaluetype:
			return (coercetoaddress (v));
		
		case datevaluetype:
			return (coercetodate (v));
		
		case fixedvaluetype:
			return (coercetofixed (v));
		
		case singlevaluetype:
			return (coercetosingle (v));
		
		case doublevaluetype:
			return (coercetodouble (v));
		
		case pointvaluetype:
			return (coercetopoint (v));
		
		case rectvaluetype:
			return (coercetorect (v));
		
		case rgbvaluetype:
			return (coercetorgb (v));
		
		case patternvaluetype:
			return (coercetopattern (v));
		
		case filespecvaluetype:
			return (coercetofilespec (v));
		
		case aliasvaluetype:
			return (coercetoalias (v));
		
		case objspecvaluetype:
			return (coercetoobjspec (v));
		
		case binaryvaluetype:
			return (coercetobinary (v));
		
		case listvaluetype:
		case recordvaluetype:
			return (coercetolist (v, valuetype));
		
		default:
			langcoerceerror (v, valuetype);
			
			return (false);
		} /*switch*/
	
	return (true);
	} /*coercevalue*/


static short coercionweight (tyvaluetype type) {
	
	/*
	9/22/92 dmb: return the relative weight associated with the given 
	value type. 
	
	the coercion weight determines the result type when two dissimilar 
	values are involved in a binary operation such as addition or 
	subtraction.
	
	note that in many cases there are ties, and the first operand will 
	end up determining the result type.
	
	12/11/92 dmb: added case for novaluetype
	
	4/14/93 dmb: weight records heavier than lists (forcing error if list non-empty)
	
	2.1b6 dmb: strings weigh in heavier than objspecs
	*/
	
	switch (type) {
		
		case novaluetype:
			return (0);
		
		case booleanvaluetype:
			return (1);
		
		case intvaluetype:
		case tokenvaluetype:
			return (2);
		
		case directionvaluetype:
		case charvaluetype:
		case longvaluetype:
		case ostypevaluetype:
		case pointvaluetype:
			return (3);
		
		case datevaluetype:
			return (4);
		
		case fixedvaluetype:
		case singlevaluetype:
			return (5);
		
		case doublevaluetype:
			return (7);
		
		case rectvaluetype:
		case patternvaluetype:
		case rgbvaluetype:
		case filespecvaluetype:
		case aliasvaluetype:
		case addressvaluetype:
		case externalvaluetype:
			return (8);
		
		case objspecvaluetype:
			return (9);
		
		case stringvaluetype:
		case passwordvaluetype:
			return (10);
		
		case binaryvaluetype:
			return (11);
		
		case listvaluetype:
			return (12);
		
		case recordvaluetype:
			return (13);
		
		default:
			return (1);
		}
	} /*coercionweight*/


boolean coercetypes (tyvaluerecord *v1, tyvaluerecord *v2) {
	
	/*
	the caller wants to do a binary operation with the two valuerecords.
	
	we convert/coerce the types of the two so they are the same type on
	exit, with converted data.
	
	return false if the two types are incompatible, ie no coersion is 
	possible.
	
	3/6/91 dmb: if v2 is a complex type (heap allocated), and v1 is a scalar 
	(not heap allocated), try to coerce to the complex type, instead of the 
	first type.  this ensures that msg (x + " secs.") will work without 
	explicit casting.
	
	5/30/91 dmb: use new langheaptype function.  also, check for same type
	
	9/22/92 dmb: use new coercionweight function instead of langheaptype; it 
	offers more granularity, and is written specifically to serve this routine
	
	2004-12-30 SMD: now extern instead of static
	*/
	
	register tyvaluetype t1 = (*v1).valuetype;
	register tyvaluetype t2 = (*v2).valuetype;
	
	if (t1 == t2) /*easy case; short circuit*/
		return (true);
	
	if (t1 == externalvaluetype) {
		
		langbadexternaloperror (badexternaloperationerror, *v1);
		
		return (false);
		}
	
	if (t2 == externalvaluetype) {
		
		langbadexternaloperror (badexternaloperationerror, *v2);
		
		return (false);
		}
	
	if (coercionweight (t2) > coercionweight (t1))
		return (coercevalue (v1, t2));
	
	return (coercevalue (v2, t1));
	} /*coercetypes*/


static boolean langgettableval (hdlhashtable htable, bigstring bsname, hdlhashtable *hval) {
	
	boolean fl;
	
	if (htable == nil)
		return (false);
	
	pushhashtable (htable);
	
	fl = langexternalgettable (bsname, hval);
	
	pophashtable ();
	
	return (fl);
	} /*langgettableval*/


boolean langgetidentifier (hdltreenode htree, bigstring bs) {
	
	/*
	call this guy if you are at a node which should contain an identifier, the name
	of something in a symbol table.
	
	we allow the user to bracket a name like this ["ct" + "seconds"] so he can construct
	the name of the variable at runtime.  above this level, this feature is transparent.
	
	dmb 4.1b2: added call to releaseheaptmp to avoid overflow
	*/
	
	register hdltreenode h = htree;
	tyvaluerecord val;		
	
	switch ((**h).nodetype) {
	
		case identifierop:
			pullstringvalue (&(**h).nodeval, bs);
		
			return (true);
		
		case bracketop: 
			//I need to look at this code, there are stale handles here
			if (!evaluatetree ((**h).param1, &val))
				return (false);
			
			if (!coercetostring (&val))
				return (false);
			
			pullstringvalue (&val, bs);
			
			releaseheaptmp ((Handle) val.data.stringvalue);
		
			return (true);
		
		default:
			langlongparamerror (unexpectedopcodeerror, (long) (**h).nodetype);
		} /*switch*/
	
	return (false);
	} /*langgetidentifier*/


typedef boolean (*tysearchpathcallback) (hdlhashtable, bigstring, hdlhashtable *);


static boolean langsearchpathvisit (tysearchpathcallback visit, bigstring bsname, hdlhashtable *htable) {
	
	/*
	look in the paths table for addresses of tables to look in.
	
	call the visit routine for each table pointed to in the paths table.
	we pass along bsname and htable for convenience; we never look at them
	ourself.
	
	return true when a visit routine returns true; return false when an 
	error occurs or when we run out of addresses
	
	4/3/92 dmb: added check for unresolved address

	5.1b21 dmb: handle guest databases via filewindowtable
	*/
	
	register hdlhashtable ht = pathstable;
	register hdlhashnode nomad;
	hdlhashtable hsearch;
	bigstring bs;
	
	if (ht == nil)
		return (false);
	
	nomad = (**ht).hfirstsort;
	
	while (nomad != nil) {
		
		/*
		val = (**nomad).val;
		*/
		
		if ((**nomad).val.valuetype != addressvaluetype) /*not an address*/
			goto next;
		
		if ((**nomad).flunresolvedaddress)
			if (!hashresolvevalue (ht, nomad))
				goto next;
		
		if (!getaddressvalue ((**nomad).val, &hsearch, bs)) /*address error*/
			goto next;
		
		if (!langgettableval (hsearch, bs, &hsearch)) /*not the address of a table*/
			goto next;
		
		if ((*visit) (hsearch, bsname, htable))
			return (true);
		
		next:
		
		if (fllangerror)
			break;
		
		nomad = (**nomad).sortedlink;
		} /*while*/
	
	if (filewindowtable != nil) {
		
		for (nomad = (**filewindowtable).hfirstsort; nomad != nil; nomad = (**nomad).sortedlink)
			if (langexternalvaltotable ((**nomad).val, &hsearch, nomad))
				if ((*visit) (hsearch, bsname, htable))
					return (true);
		}

	return (false);
	} /*langsearchpathvisit*/


static boolean langgettableitemname (hdlhashtable htable, tyvaluerecord *valindex, bigstring bsname) {
	
	/*
	3.0.2 dmb: don't reuse valindex when calling hashgetiteminfo. we don't want to 
	change caller's index to be the value of the cell itself!
	
	5.0a23 dmb: don't ask hashgetiteminfo for the node's value. An error resolving
	the node leads to a bogus message
	*/
	
	register tyvaluerecord *v = valindex;
	
	if ((*v).valuetype == stringvaluetype)  {
		
		pullstringvalue (v, bsname);
		
		releaseheaptmp ((Handle) (*v).data.stringvalue);
		}
	else {
		register long ix;
		
		if (!coercetolong (v))
			return (false);
		
		ix = (*v).data.longvalue;
		
		if ((ix <= 0) || !hashgetiteminfo (htable, ix - 1, bsname, nil)) {
			
			langlongparamerror (tabletoosmallerror, (long) ix);
			
			return (false);
			}
		}
	
	return (true);
	} /*langgettableitemname*/


boolean langgetdotparams (hdltreenode htree, hdlhashtable *htable, bigstring bsname) {
	
	/*
	we get a "dot param" pair from the indicated node.  we return a handle to 
	a hashtable that's supposed to have a value with the indicated name.
	
	if all we find is a string node, we return with the table set to nil, this
	implies that we should use the current hashtable, whatever that might mean.
	
	this routine is recursive, so it can process a whole structure of dot-ops.
	
	2/5/91 dmb: it helps if we look in the handler table automatically
	
	2/6/91 dmb: for array references, why not resolve to the hash name here, 
	so no one else has to know about the "$n" encoding?  as it stands, things 
	can break when an attempt to make a full path is made.
	
	5.0.2b6 dmb: if we can't resolve a name to a table, try looking it up in 
	local chain, context free.  first change in a long time!
	*/
	
	register hdltreenode h = htree;
	register tytreetype nodetype = (**h).nodetype;
	hdlhashtable hsubtable;
	register boolean fl;
	tyvaluerecord val;
	
	*htable = nil; /*default, in case a table isn't specified*/
	
	langseterrorline (h); /*set globals for error reporting*/
	
	switch (nodetype) {
		
		case identifierop:
		case bracketop:
			return (langgetidentifier (h, bsname));
		
		case dereferenceop:
			if (!evaluatetree ((**h).param1, &val))
				return (false);
			
			if (!coercetoaddress (&val)) /*might recurse via langexpandtodotparams*/
				return (false);
			
			return (getaddressvalue (val, htable, bsname));
		
		case dotop:
		case arrayop: /*only arrays & dots allowed past here*/
			break;
		
		default:
			langlongparamerror (unexpectedopcodeerror, (long) nodetype);
			
			return (false);
		}
	
	if (!langgetdotparams ((**h).param1, &hsubtable, bsname)) /*recurse*/
		return (false);
	
	if (hsubtable == nil) { /*we're at the very first table in the dot list*/
		
		if (langgetspecialtable (bsname, htable)) /*translate "root" to roottable, etc.*/
			goto L1;
		
		if (langexternalgettable (bsname, htable)) /*found bsname in current context*/
			goto L1;
		
		if (fllocaldotparamsonly)
			fl = false;
		else {
			
			fl = langsearchpathvisit (&langgettableval, bsname, htable); /*check user paths*/
			
			if (!fl) { // about to fail; last ditch effort for local paths
				
				flfindanyspecialsymbol = true;
				
				fl = langexternalgettable (bsname, htable);
				
				flfindanyspecialsymbol = false;
				}
			}
		}
	else
		fl = langgettableval (hsubtable, bsname, htable);
	
	if (!fl) {
	
		langparamerror (nosuchtableerror, bsname);
		
		return (false);
		}
	
	L1: /*deal with param2 here*/
	
	if (nodetype == arrayop) { /*param2 is an index*/
		
		tyvaluerecord valindex;
		
		if (!evaluatetree ((**h).param2, &valindex))
			return (false);
		
		return (langgettableitemname (*htable, &valindex, bsname));
		}
	
	return (langgetidentifier ((**h).param2, bsname));
	} /*langgetdotparams*/


boolean langexpandtodotparams (bigstring bs, hdlhashtable *htable, bigstring bsname) {
	
	/*
	1/14/91 dmb: bs should be the path to an object in the database, a 
	value in the symbol table.  to locate it, we compile the string 
	and try to evaluate the code as a dot param pair
	*/
	
	Handle htext;
	hdltreenode hmodule;
	boolean fl;
	unsigned long savelines;
	unsigned short savechars;
	
	if (!newtexthandle (bs, &htext))
		return (false);
	
	savelines = ctscanlines;
	
	savechars = ctscanchars;
	
	fl = langcompiletext (htext, false, &hmodule); /*always disposes htext*/
	
	if (fl) {
		
		register hdltreenode h = (**hmodule).param1; /*copy into register*/
		
		if (!langgetdotparams (h, htable, bsname)) { /*error occurred*/
			
			fl = false; /*make sure we return false*/
			
			if (!langerrorenabled ()) { /*call has errors disabled -- try to provide bsname*/
				
				if ((**h).nodetype == dotop) {
					
					h = (**h).param2;
					
					if ((**h).nodetype == identifierop)
						langgetidentifier (h, bsname);
					}
				}
			}
		
		langdisposetree (hmodule);
		}
	
	ctscanlines = savelines;
	
	ctscanchars = savechars;
	
	return (fl);
	} /*langexpandtodotparams*/


boolean langtablelookup (hdlhashtable intable, bigstring bsname, hdlhashtable *htable) {
	
	/*
	if bsname exists in intable, set htable to intable and return true.
	
	9/14/92 dmb: don't set *htable unless we find bsname
	*/
	
	if (intable == nil)
		return (false);
	
	if (!hashtablesymbolexists (intable, bsname))
		return (false);
	
	*htable = intable; /*don't set this on failure*/
	
	return (true);
	} /*langtablelookup*/


boolean langsearchpathlookup (bigstring bs, hdlhashtable *htable) {
	
	/*
	2/2/91 dmb: factored code: our identifier search path.
	
	look in the current symbol chain first, then in the handlers table, 
	then in the user table.  finally, check for a special table name.
	
	if it's found, return true with htable set to the containing table. 
	otherwise, set htable to currenthashtable and return false
	
	9/14/92 dmb: on failure, only set *htable to current hashtable if 
	it wasn't already set (by langfindsymbol)
	*/
	
	hdlhashnode hnode;
	
	if (langfindsymbol (bs, htable, &hnode)) /*found it in local chain*/
		return (true);
	
	/*
	if (langtablelookup (handlertable, bs, htable))
		return (true);
	
	if (langtablelookup (iacgluetable, bs, htable))
		return (true);
	
	if (langtablelookup (usertable, bs, htable))
		return (true);
	*/
	
	if (langgetspecialtable (bs, htable))
		return (true);
	
	if (langsearchpathvisit (&langtablelookup, bs, htable))
		return (true);
	
	if (langtablelookup (filewindowtable, bs, htable)) // 5.0d16 dmb
		return (true);
	
	if (*htable == nil) { /*wasn't set to "with" table by langfindsymbol*/
		
		*htable = currenthashtable; /*undeclared variables assumed to be local*/
		}
	
	return (false); /*not found*/
	} /*langsearchpathlookup*/


boolean langgetdottedsymbolval (hdltreenode htree, hdlhashtable *htable, bigstring bs, tyvaluerecord *val, hdlhashnode * hnode) {
	
	/*
	the caller wants the value of the variable indicated by htree, which could be
	a dotted id.  we return the table it was found in, in case the caller also wants
	to set the value of the symbol.
	
	2/5/91 dmb: use searchpathlookup before calling langgetsymbolval so we can 
	find things in usertable and handlerstable
	
	4/17/91 dmb: use langsymbolreference to save code & handle "root"
	*/
	
	register hdlhashtable *ht = htable;
	
	if (!langgetdotparams (htree, ht, bs))
		return (false);
	
	if (*ht == nil)
		langsearchpathlookup (bs, ht); /*always sets ht*/
	
	return (langsymbolreference (*ht, bs, val, hnode));
	} /*langgetdottedsymbolval*/


#if oldarrays

static boolean langsetdottedsymbolval (hdlhashtable htable, bigstring bs, tyvaluerecord val) {
	
	register hdlhashtable ht = htable;
	register boolean fl;
	
	if (ht != nil)
		pushhashtable (ht);
		
	fl = langsetsymbolval (bs, val);
	
	if (ht != nil)
		pophashtable ();
		
	return (fl);
	} /*langsetdottedsymbolval*/

#endif


boolean langhashtablelookup (hdlhashtable htable, const bigstring bs, tyvaluerecord *vreturned, hdlhashnode *hnode) {
	
	/*
	a simple wrapper for hashtablelookup: this code was replicated all over the place
	
	6.1d4 AR: created for better error reporting in the kernelized webserver.
	*/

	if (!hashtablelookup (htable, bs, vreturned, hnode)) {
		
		langparamerror (unknownidentifiererror, bs);
		
		return (false);
		}
	
	return (true);
	} /*langhashtablelookup*/


boolean langlookupstringvalue (hdlhashtable ht, bigstring bs, bigstring bsval) {
	
	/*
	a generic piece of code: look up the string value, with coercion and errors

	6.1d3 AR: moved from langhtml.c to langvalue.c
	*/
	
	tyvaluerecord val;
	hdlhashnode hnode;
	
	if (!langhashtablelookup (ht, bs, &val, &hnode))
		return (false);
	
	if (val.valuetype != stringvaluetype)
		if (!copyvaluerecord (val, &val) || !coercetostring (&val))
			return (false);
	
	pullstringvalue (&val, bsval);
	
	return (true);
	} /*langlookupstringvaluecoerce*/



boolean langlookupaddressvalue (hdlhashtable ht, bigstring bs, tyaddress *addressval) {
	
	/*
	a generic piece of code: look up the address value, with errors
	
	5.1.4 dmb: copy the value before coercing it

	6.1d3 AR: moved from langhtml.c to langvalue.c
	*/
	
	tyvaluerecord val;
	hdlhashnode hnode;
	
	if (!langhashtablelookup (ht, bs, &val, &hnode))
		return (false);
	
	if (val.valuetype != addressvaluetype)
		if (!copyvaluerecord (val, &val) || !coercetoaddress (&val))
			return (false);
	
	return (getaddressvalue (val, &(*addressval).ht, (*addressval).bs));
	} /*langlookupaddressvalue*/


boolean langlookuplongvalue (hdlhashtable ht, bigstring bs, long *x) {
	
	/*
	6.1d4 AR: look up the long value, with errors
	*/
	
	tyvaluerecord val;
	hdlhashnode hnode;
	
	if (!langhashtablelookup (ht, bs, &val, &hnode))
		return (false);
		
	if (val.valuetype != longvaluetype)
		if (!copyvaluerecord (val, &val) || !coercetolong (&val))
			return (false);
	
	*x = val.data.longvalue;
	
	return (true);
	} /*langlookuplongvalue*/


boolean langlookupbooleanvalue (hdlhashtable ht, bigstring bs, boolean *fl) {
	
	/*
	6.1d2 AR: look up the boolean value, with errors
	
	6.1d3 AR: moved from langhtml.c to langvalue.c
	*/
	
	tyvaluerecord val;
	hdlhashnode hnode;
	
	if (!langhashtablelookup (ht, bs, &val, &hnode))
		return (false);
		
	if (val.valuetype != booleanvaluetype)
		if (!copyvaluerecord (val, &val) || !coercetoboolean (&val))
			return (false);
	
	*fl = val.data.flvalue;
	
	return (true);
	} /*langlookupbooleanvalue*/


boolean langtablecopyvalue (hdlhashtable hsource, hdlhashtable hdest, bigstring bs) {
		
	tyvaluerecord val;
	Handle hpacked;
	boolean fl;
	hdlhashnode hnode;

	assert (hsource != nil);
	
	assert (hdest != nil);
	
	if (!langhashtablelookup (hsource, bs, &val, &hnode))
		return (false);
	
	if (!langpackvalue (val, &hpacked, hnode)) /*error packing -- probably out of memory*/
		return (false);
	
	fl = langunpackvalue (hpacked, &val);
	
	disposehandle (hpacked);
	
	if (!fl)
		return (false);
	
	if (!hashtableassign (hdest, bs, val)) {
	
		disposevaluerecord (val, true);
		
		return (false);
		}

	return (true);
	} /*langtablecopyvalue*/



static boolean indexparam (hdltreenode hfirst, short pnumber, hdltreenode *h) {
	
	/*
	starting with hfirst param, step down pnumber - 1 times and return
	a handle to the parameter tree.
	
	return false if there aren't that many parameters in the list.
	*/
	
	register hdltreenode nomad = hfirst;
	register short ctloops = pnumber;
	
	while (nomad != nil) { /*haven't fallen off end of list*/
		
		if (--ctloops <= 0)
			break;
		
		nomad = (**nomad).link;
		} /*for*/
	
	*h = nomad;
	
	return (ctloops <= 0); /*true if we traversed the number requested -- even zero*/
	} /*indexparam*/


static boolean getparam (hdltreenode hfirst, short pnumber, hdltreenode *h) {
	
	/*
	return a handle to the pnumber'th parameter starting with hfirst as 
	parameter number 1.  if not found display an error dialog box.
	*/
	
	boolean fllastparam = flnextparamislast;
	
	flnextparamislast = false; /*must be reset every time*/
	
	if (!indexparam (hfirst, pnumber, h)) {
		
		if (flparamerrorenabled) {
		
			if (isemptystring (bsfunctionname)) /*a tokenized function*/
		
				gettokenname (bsfunctionname); /*convert functiontoken to its name*/
			
			langparamerror (notenoughparameterserror, bsfunctionname);
			}
		
		return (false);
		}
	
	if (fllastparam) { /*caller is requiring that this be the last paramater*/
		
		hdltreenode hextra;
		
		if (indexparam (hfirst, pnumber + 1, &hextra)) {
			
			if (isemptystring (bsfunctionname)) /*a tokenized function*/
			
				gettokenname (bsfunctionname); /*convert functiontoken to its name*/
			
			langparamerror (toomanyparameterserror, bsfunctionname);
			
			return (false);
			}
		}
	
	return (true);
	} /*getparam*/


boolean langcheckparamcount (hdltreenode hfirst, short pexpected) {
	
	/*
	make sure there are the correct number of parameters provided by 
	trying to get the last one.
	*/
	
	hdltreenode hparam;
	
	flnextparamislast = true; /*we asserting that this must be the last one*/
	
	return (getparam (hfirst, pexpected, &hparam));
	} /*langcheckparamcount*/


short langgetparamcount (hdltreenode hfirst) {
	
	register hdltreenode x = hfirst;
	register short ct = 0;
	
	while (x != nil) {
		
		ct++;
		
		x = (**x).link;
		} /*while*/
		
	return (ct);
	} /*langgetparamcount*/


static boolean evaluateparam (hdltreenode hparam, tyvaluerecord *vparam) {

	/*
	2.1b2 dmb: broke out to be shared w/new paramlist stuff
	*/
	
	bigstring bssave;
	
	copystring (bsfunctionname, bssave);
	
	if (!evaluatetree (hparam, vparam))
		return (false);
	
	langseterrorline (hparam); /*restore to param before caller attempts coercion*/
	
	copystring (bssave, bsfunctionname); /*restore name too*/
	
	return (true);
	} /*evaluateparam*/


boolean evaluatereadonlyparam (hdltreenode hparam, tyvaluerecord *vparam) {

	/*
	5.0.2b18 dmb: generalized getreadonlytextvalue; this is broadly useful.
	
	5.0.2b19 dmb: handle constop
	*/
	
	bigstring bssave;
	hdlhashtable htable = nil;
	bigstring bs;
	tyvaluerecord val;
	hdlhashnode hnode;
	
	copystring (bsfunctionname, bssave);
	
	switch ((**hparam).nodetype) {
	
		case constop:
			*vparam = (**hparam).nodeval;
			
			break;
		
		case identifierop:	// use idvalue w/out the final copyvaluerecord
		case bracketop:
			if (!langgetidentifier (hparam, bs))
				return (false);
			
			if (!langsearchpathlookup (bs, &htable))
				;
			
			break;
		
		case dotop:  // use dotvalue w/out the copyvaluerecord
			if (!langgetdotparams (hparam, &htable, bs))
				return (false);
			
			break;
		
		case dereferenceop: // use dereferencevalue w/out the copy
			if (!evaluatetree ((**hparam).param1, &val))
				return (false);
			
			if (!coercetoaddress (&val))
				return (false);
			
			if (!getaddressvalue (val, &htable, bs))
				return (false);
			
			if (htable == nil)
				langsearchpathlookup (bs, &htable);
			
			break;
		
		default:
			if (!evaluatetree (hparam, vparam))
				return (false);
			
			break;
		}
	
	if (htable != nil)
		if (!langsymbolreference (htable, bs, vparam, &hnode))
			return (false);
	
	langseterrorline (hparam); /*restore to param before caller attempts coercion*/
	
	copystring (bssave, bsfunctionname); /*restore name too*/
	
	return (true);
	} /*evaluatereadonlyparam*/


boolean getparamvalue (hdltreenode hfirst, short pnum, tyvaluerecord *vreturned) {

	/*
	hfirst points to the first parameter in a function's parameter list.
	the caller wants the value of pnum'th parameter.
	
	7/30/91 dmb: added langseterrorline call in case the parameter is a function 
	call and changes it.  otherwise, coercion error would yeild a bogus position.
	*/
	
	hdltreenode hparam;
	
	if (!getparam (hfirst, pnum, &hparam))
		return (false);
	
	return (evaluateparam (hparam, vreturned));
	} /*getparamvalue*/


boolean getreadonlyparamvalue (hdltreenode hfirst, short pnum, tyvaluerecord *vreturned) {

	/*
	5.0.2b18 dmb: new routine. may return non-temp values. be careful how you use it!
	
	2003-04-28 AR: declared global so it could be used in langregexp.c
	*/
	
	hdltreenode hparam;
	
	if (!getparam (hfirst, pnum, &hparam))
		return (false);
	
	return (evaluatereadonlyparam (hparam, vreturned));
	} /*getreadonlyparamvalue*/


boolean getoptionalparam (hdltreenode hfirst, short *ctconsumed, short *ctpositional, bigstring bsparam, hdltreenode *h) {

	/*
	get the next optional parameter. ctpositional is the number of positional parameters
	that have been found. ctconsumed is the total number of parameters that have been 
	found.
	
	on error, return false. if the parameter doesns't exist, set h to nil and return true.
	
	our error checking isn't as good as langaddfuncparams, because we don't have a 
	list of all of the needed parameters. if a named parameter is provided that doesn't
	match any requested name, ctconsumed will be less than the param count, so the error 
	will be "too many parameters".
	*/
	
	hdltreenode nomad;
	hdltreenode hparam = nil;
	bigstring bsname;
	short ctskip = *ctpositional;
	boolean fllastparam = flnextparamislast;
	
	flnextparamislast = false; /*must be reset every time*/
	
	for (nomad = hfirst; nomad != nil; nomad = (**nomad).link) { //walk the entire list
		
		if (--ctskip >= 0) //not an  potentially optional parameter
			continue;
		
		if ((**nomad).nodetype != fieldop) { //parameter given by position
			
			hparam = nomad;
			
			++*ctpositional;
			
			++*ctconsumed;
			
			break;
			}
		
		if (!langgetidentifier ((**nomad).param1, bsname))
			return (false);
		
		if (equalidentifiers (bsname, bsparam)) { //parameter given by name
			
			hparam = (**nomad).param2;
			
			++*ctconsumed;
			
			break;
			}
		}
	
	if (fllastparam) {
	
		if (!langcheckparamcount (hfirst, *ctconsumed))
			return (false);
		}
	
	*h = hparam;
	
	return (true);
	} /*getoptionalparam*/


boolean getoptionalparamvalue (hdltreenode hfirst, short *ctconsumed, short *ctpositional, bigstring bsparam, tyvaluerecord *vreturned) {

	/*
	get the next optional parameter. ctpositional is the number of positional parameters
	that have been found. ctconsumed is the total number of parameters that have been 
	found.
	
	the value of vreturned on entry is the default, and indicates the type of value being sought.
	
	on error, return false. if the parameter doesns't exist, don't touch vreturned
	and return true.
	
	our error checking isn't as good as langaddfuncparams, because we don't have a 
	list of all of the needed parameters. if a named parameter is provided that doesn't
	match any requested name, ctconsumed will be less than the param count, so the error 
	will be "too many parameters".
	*/
	
	hdltreenode hparam = nil;
	tyvaluetype ptype = (*vreturned).valuetype;
	
	if (!getoptionalparam (hfirst, ctconsumed, ctpositional, bsparam, &hparam))
		return (false);
	
	if (hparam == nil) //param not provided, don't touch param value
		return (true);
	
	return (evaluateparam (hparam, vreturned) && coercevalue (vreturned, ptype));
	} /*getoptionalparamvalue*/


boolean getoptionaladdressparam (hdltreenode hfirst, short *ctconsumed, short *ctpositional, bigstring bsparam, hdlhashtable *ht, bigstring bsname) {
	
	/*
	Get an optional parameter that is expected to be an address value.
	If the param is not specified or if it is nil, we don't return anything.
	
	2006-03-10 aradke: transplanted from langregexp.c
	*/
	
	tyvaluerecord vparam;
	
	setaddressvalue (nil, emptystring, &vparam);

	if (!getoptionalparamvalue (hfirst, ctconsumed, ctpositional, bsparam, &vparam))
		return (false);
	
	return (getaddressvalue (vparam, ht, bsname));
	} /*getoptionaladdressparam*/


boolean getoptionaltableparam (hdltreenode hfirst, short *ctconsumed, short *ctpositional, bigstring bsparam, hdlhashtable *htable) {
	
	/*
	Get an optional parameter that is expected to be the address of a table.
	If the param is not specified or if it is nil, we don't return anything.
	If the param is a valid address, we make sure a table exists at the
	indicated location and return a handle to it.

	2006-03-10 aradke: transplanted from langregexp.c
	*/
	
	hdlhashtable ht;
	bigstring bsname;
	
	if (!getoptionaladdressparam (hfirst, ctconsumed, ctpositional, bsparam, &ht, bsname))
		return (false);
	
	if (ht == nil && isemptystring (bsname))
		return (true);
	
	if (!langassignnewtablevalue  (ht, bsname, htable))
		return (false);

	return (true);
	} /*getoptionaltableparam*/


boolean getaddressparam (hdltreenode hfirst, short pnum, tyvaluerecord *val) {
	
	if (!getparamvalue (hfirst, pnum, val))
		return (false);
	
	return (coercetoaddress (val));
	} /*getaddressparam*/


boolean getvarparam (hdltreenode hfirst, short pnum, hdlhashtable *htable, bigstring bsname) {
	
	/*
	8/20/91 dmb: a bold new implementation...
	
	we want to go soft on the @ operator, so users can type edit (x) instead of 
	edit (@x), etc.; we want to make the '@' optional for built-in functions.  also, 
	there are verbs where we take values by address to reduce memory overhead: unpack, 
	wp.unpacktext, and op.unpacktext (as of right now).
	
	so, instead of evaluating the parameters and trying do coerce it to an address, 
	we take things more slowly.  first, we check for a valid lhs expressiong by try 
	to get dotparams from the tree.  if that works, we check the value at that db 
	location.  if it exists and is an address, we use it.  otherwise, we use the 
	dotparam pair as the address.
	
	since this looseness can cause problems if the address of an address value makes 
	sense, or if coercion (say, from string) is desired, not everyone will use this 
	version.
	
	note that we need to try the dotparam method before the traditional method, to 
	avoid making copies of binaries or coercing externals to strings if the address 
	operator is indeed missing.
	*/
	
	tyvaluerecord val;
	
		
		if (!getaddressparam (hfirst, pnum, &val))
			return (false);
	
	if (!getaddressvalue (val, htable, bsname))
		return (false);
	
	if (*htable == nil) { /*no table specified in getdotparams*/
		
		langsearchpathlookup (bsname, htable);
		}
	
	return (true);
	} /*getvarparam*/


boolean getbooleanparam (hdltreenode hfirst, short pnum, tyvaluerecord *v) {
	
	if (!getparamvalue (hfirst, pnum, v))
		return (false);
		
	return (coercetoboolean (v));
	} /*getbooleanparam*/
	
	
boolean getcharparam (hdltreenode hfirst, short pnum, tyvaluerecord *v) {
	
	if (!getparamvalue (hfirst, pnum, v))
		return (false);
		
	return (coercetochar (v));
	} /*getcharparam*/
	
	
boolean getintparam (hdltreenode hfirst, short pnum, tyvaluerecord *v) {
	
	if (!getparamvalue (hfirst, pnum, v))
		return (false);
		
	return (coercetoint (v));
	} /*getintparam*/


boolean getlongparam (hdltreenode hfirst, short pnum, tyvaluerecord *v) {
	
	if (!getparamvalue (hfirst, pnum, v))
		return (false);
		
	return (coercetolong (v));
	} /*getlongparam*/
	
	
boolean getdateparam (hdltreenode hfirst, short pnum, tyvaluerecord *v) {
	
	if (!getparamvalue (hfirst, pnum, v))
		return (false);
		
	return (coercetodate (v));
	} /*getdateparam*/


boolean getstringparam (hdltreenode hfirst, short pnum, tyvaluerecord *v) {
	
	if (!getparamvalue (hfirst, pnum, v))
		return (false);
		
	return (coercetostring (v));
	} /*getstringparam*/


boolean getdirectionparam (hdltreenode hfirst, short pnum, tyvaluerecord *v) {
	
	if (!getparamvalue (hfirst, pnum, v))
		return (false);
		
	return (coercetodirection (v));
	} /*getdirectionparam*/


boolean getostypeparam (hdltreenode hfirst, short pnum, tyvaluerecord *v) {
	
	if (!getparamvalue (hfirst, pnum, v))
		return (false);
		
	return (coercetoostype (v));
	} /*getostypeparam*/


boolean getpointparam (hdltreenode hfirst, short pnum, tyvaluerecord *v) {
	
	if (!getparamvalue (hfirst, pnum, v))
		return (false);
	
	return (coercetopoint (v));
	} /*getpointparam*/


boolean getrectparam (hdltreenode hfirst, short pnum, tyvaluerecord *v) {
	
	if (!getparamvalue (hfirst, pnum, v))
		return (false);
	
	return (coercetorect (v));
	} /*getrectparam*/


boolean getrgbparam (hdltreenode hfirst, short pnum, tyvaluerecord *v) {
	
	if (!getparamvalue (hfirst, pnum, v))
		return (false);
	
	return (coercetorgb (v));
	} /*getrgbparam*/


boolean getpatternparam (hdltreenode hfirst, short pnum, tyvaluerecord *v) {
	
	if (!getparamvalue (hfirst, pnum, v))
		return (false);
	
	return (coercetopattern (v));
	} /*getpatternparam*/


boolean getfixedparam (hdltreenode hfirst, short pnum, tyvaluerecord *v) {
	
	if (!getparamvalue (hfirst, pnum, v))
		return (false);
	
	return (coercetofixed (v));
	} /*getfixedparam*/


boolean getsingleparam (hdltreenode hfirst, short pnum, tyvaluerecord *v) {
	
	if (!getparamvalue (hfirst, pnum, v))
		return (false);
	
	return (coercetosingle (v));
	} /*getsingleparam*/


boolean getdoubleparam (hdltreenode hfirst, short pnum, tyvaluerecord *v) {
	
	if (!getparamvalue (hfirst, pnum, v))
		return (false);
	
	return (coercetodouble (v));
	} /*getdoubleparam*/


boolean getfilespecparam (hdltreenode hfirst, short pnum, tyvaluerecord *v) {
	
	if (!getparamvalue (hfirst, pnum, v))
		return (false);
	
	return (coercetofilespec (v));
	} /*getfilespecparam*/


boolean getaliasparam (hdltreenode hfirst, short pnum, tyvaluerecord *v) {
	
	if (!getparamvalue (hfirst, pnum, v))
		return (false);
	
	return (coercetoalias (v));
	} /*getaliasparam*/


boolean getobjspecparam (hdltreenode hfirst, short pnum, tyvaluerecord *v) {
	
	hdltreenode hparam;
	
	if (!getparam (hfirst, pnum, &hparam))
		return (false);
	
	if (isobjspectree (hparam))
		return (evaluateobjspec (hparam, v));
	
	if (!getparamvalue (hfirst, pnum, v))
		return (false);
	
	return (coercetoobjspec (v));
	} /*getobjspecparam*/


boolean getbinaryparam (hdltreenode hfirst, short pnum, tyvaluerecord *v) {
	
	if (!getparamvalue (hfirst, pnum, v))
		return (false);
	
	return (coercetobinary (v));
	} /*getbinaryparam*/


boolean getvarvalue (hdltreenode hfirst, short pnum, hdlhashtable *htable, bigstring bsname, tyvaluerecord *val, hdlhashnode * hnode) {
	
	if (!getvarparam (hfirst, pnum, htable, bsname))
		return (false);
	
		if (isemptystring (bsname) && ((***htable).fllocaltable))
			return (setnilvalue (val));
	
	return (langsymbolreference (*htable, bsname, val, hnode));
	} /*getvarvalue*/


boolean getstringvalue (hdltreenode hfirst, short pnum, bigstring bs) {
	
	/*
	carefully get a string value for the parameter.  if it needed to be coerced to
	a string we must release the string.  otherwise the system chews memory as 
	non-strings are coerced to strings.
	
	we'll need a routine like this for every type that is heap-allocated.
	
	5.0.2b18 dmb: try getting readonly text
	*/
	
	tyvaluerecord v;
	
	if (!getreadonlyparamvalue (hfirst, pnum, &v))
		return (false);
	
	if (v.valuetype == stringvaluetype) { /*already a string, easy case*/
		
		pullstringvalue (&v, bs);
		
		return (true);
		}
	
	if (!v.fltmpstack && !copyvaluerecord (v, &v))
		return (false);
	
	if (!coercetostring (&v))
		return (false);
	
	pullstringvalue (&v, bs); /*copy from the heap into the stack*/
	
	releaseheaptmp ((Handle) v.data.stringvalue); /*the secret of our success*/
	
	return (true);
	} /*getstringvalue*/


boolean gettextvalue (hdltreenode hfirst, short pnum, Handle *textval) {
	
	/*
	get a string handle for the parameter.  don't make a copy
	*/
	
	tyvaluerecord v;
	
	if (!getstringparam (hfirst, pnum, &v))
		return (false);
	
	*textval = v.data.stringvalue;
	
	return (true);
	} /*gettextvalue*/


boolean getexempttextvalue (hdltreenode hfirst, short pnum, Handle *textval) {
	
	/*
	return a string handle parameter that is free & clear.
	
	the caller must make sure that the handle is either disposed or consumed 
	in some way.
	*/
	
	tyvaluerecord v;
	
	if (!getstringparam (hfirst, pnum, &v))
		return (false);
	
	if (!exemptfromtmpstack (&v)) /*wasn't found in current temp stack*/
		return (copyhandle (v.data.stringvalue, textval));
	
	*textval = v.data.stringvalue;
	
	return (true);
	} /*getexempttextvalue*/


boolean getreadonlytextvalue (hdltreenode hfirst, short pnum, Handle *textval) {
	
	/*
	5.0.2b17 dmb: new routine.
	
	return a string handle parameter that may still be attached to an odb cell.
	
	the caller must make sure that the handle is copied if it is to be modified
	in any way. it must not be disposed.
	
	the point of this routine is speed -- avoiding extra copying
	*/
	
	tyvaluerecord val;
	
	if (!getreadonlyparamvalue (hfirst, pnum, &val))
		return (false);
	
	if (val.valuetype != stringvaluetype) {
	
		if (!val.fltmpstack && !copyvaluerecord (val, &val))
			return (false);
		
		if (!coercetostring (&val))
			return (false);
		}
	
	*textval = val.data.stringvalue;
	
	return (true);
	} /*getreadonlytextvalue*/


boolean getbooleanvalue (hdltreenode hfirst, short pnum, boolean *flval) {
	
	tyvaluerecord val;
	
	if (!getbooleanparam (hfirst, pnum, &val)) 
		return (false);
		
	*flval = val.data.flvalue;
	
	return (true);
	} /*getbooleanvalue*/


boolean getcharvalue (hdltreenode hfirst, short pnum, char *chval) {
	
	tyvaluerecord val;
	
	if (!getcharparam (hfirst, pnum, &val)) 
		return (false);
	
	*chval = val.data.chvalue;
	
	return (true);
	} /*getcharvalue*/


boolean getintvalue (hdltreenode hfirst, short pnum, short *intval) {
	
	tyvaluerecord val;
	
	if (!getintparam (hfirst, pnum, &val)) 
		return (false);
		
	*intval = val.data.intvalue;
	
	return (true);
	} /*getintvalue*/


boolean getlongvalue (hdltreenode hfirst, short pnum, long *lval) {
	
	tyvaluerecord val;
	
	if (!getlongparam (hfirst, pnum, &val)) 
		return (false);
		
	*lval = val.data.longvalue;
	
	return (true);
	} /*getlongvalue*/


boolean getdirectionvalue (hdltreenode hfirst, short pnum, tydirection *dirval) {
	
	tyvaluerecord val;
	
	if (!getdirectionparam (hfirst, pnum, &val))
		return (false);
		
	*dirval = val.data.dirvalue;
	
	return (true);
	} /*getdirectionvalue*/


boolean getdatevalue (hdltreenode hfirst, short pnum, unsigned long *dateval) {
	
	tyvaluerecord val;
	
	if (!getdateparam (hfirst, pnum, &val)) 
		return (false);
	
	*dateval = val.data.datevalue;
	
	return (true);
	} /*getdatevalue*/


boolean getostypevalue (hdltreenode hfirst, short pnum, OSType *typeval) {
	
	tyvaluerecord val;
	
	if (!getostypeparam (hfirst, pnum, &val))
		return (false);
	
	*typeval = val.data.ostypevalue;
	
	return (true);
	} /*getostypevalue*/


boolean getfilespecvalue ( hdltreenode hfirst, short pnum, ptrfilespec fsval ) {
	
	tyvaluerecord val;
	
	if (!getfilespecparam (hfirst, pnum, &val))
		return (false);
		
	*fsval = **val.data.filespecvalue;
	
	return (true);
	} /*getfilespecvalue*/


boolean getpointvalue (hdltreenode hfirst, short pnum, Point *ptval) {
	
	tyvaluerecord val;
	
	if (!getpointparam (hfirst, pnum, &val))
		return (false);
	
	*ptval = val.data.pointvalue;
	
	return (true);
	} /*getpointvalue*/


boolean getrectvalue (hdltreenode hfirst, short pnum, Rect *rectval) {
	
	tyvaluerecord val;
	
	if (!getrectparam (hfirst, pnum, &val))
		return (false);
	
	*rectval = **val.data.rectvalue;
	
	return (true);
	} /*getrectvalue*/


boolean getrgbvalue (hdltreenode hfirst, short pnum, RGBColor *rgbval) {
	
	tyvaluerecord val;
	
	if (!getrgbparam (hfirst, pnum, &val))
		return (false);
	
	*rgbval = **val.data.rgbvalue;
	
	return (true);
	} /*getrgbvalue*/


boolean getbinaryvalue (hdltreenode hfirst, short pnum, boolean flreadonly, Handle *x) {
	
	/*
	get the indicated binary parameter value.  by convention, we expect 
	binary parameters to be passed by address, so this implementation 
	differs from other getxxxvalue routines.
	
	if flreadonly is true, the caller doesn't plan to modify or consume the 
	value in any way, so a copy should only be made if necessary for coersion.
	*/
	
	tyvaluerecord val;
	hdlhashtable htable;
	bigstring bsname;
	hdlhashnode hnode;
	
	if (!getvarvalue (hfirst, pnum, &htable, bsname, &val, &hnode))
		return (false);
	
	if ((!flreadonly) || (val.valuetype != binaryvaluetype)) {
		
		if (!copyvaluerecord (val, &val))
			return (false);
		
		if (!coercetobinary (&val))
			return (false);
		}
	
	*x = val.data.binaryvalue;
	
	return (true);
	} /*getbinaryvalue*/


boolean langsetbooleanvarparam (hdltreenode hfirst, short pnum, boolean fl) {
	
	hdlhashtable htable;
	bigstring bsname;
	tyvaluerecord val;
	
	if (!getvarparam (hfirst, pnum, &htable, bsname))
		return (false);
	
	setbooleanvalue (fl, &val);
	
	return (langsetsymboltableval (htable, bsname, val));
	} /*langsetbooleanvarparam*/


boolean langsetlongvarparam (hdltreenode hfirst, short pnum, long n) {
	
	hdlhashtable htable;
	bigstring bsname;
	tyvaluerecord val;
	
	if (!getvarparam (hfirst, pnum, &htable, bsname))
		return (false);
	
	setlongvalue (n, &val);
	
	return (langsetsymboltableval (htable, bsname, val));
	} /*langsetlongvarparam*/


boolean langsetstringvarparam (hdltreenode hfirst, short pnum, bigstring bsset) {
	
	hdlhashtable htable;
	bigstring bsname;
	tyvaluerecord val;
	
	if (!getvarparam (hfirst, pnum, &htable, bsname))
		return (false);
	
	setstringvalue (bsset, &val);
	
	return (hashtableassign (htable, bsname, val));
	} /*langsetstringvarparam*/


boolean setintvarparam (hdltreenode hfirst, short pnum, short n) {
	
	hdlhashtable htable;
	bigstring bsname;
	tyvaluerecord val;
	
	if (!getvarparam (hfirst, pnum, &htable, bsname))
		return (false);
	
	setintvalue (n, &val);
	
	return (langsetsymboltableval (htable, bsname, val));
	} /*setintvarparam*/


#if lazythis_optimization
	static int ctlazythis = 0;
#endif

boolean idvalue (hdltreenode htree, tyvaluerecord *val) {
	
	/*
	trade in an identifier for a value.  return false if the identifier 
	isn't defined.
	
	2/11/91 dmb: must lookup in search path
	
	8/15/92 dmb: check idvaluecallback for fileloop

	11/13/01 dmb: added lazy with evaluation
	*/
	
	bigstring bs;
	hdlhashtable htable;
	hdlhashnode hnode;
	
	/*
	if (langcallbacks.idvaluecallback != nil) {
		
		if ((*langcallbacks.idvaluecallback) (htree, val))
			return (true);
		
		if (fllangerror)
			return (false);
		}
	*/
	
	if (!langgetidentifier (htree, bs))
		return (false);
	
	if (!langsearchpathlookup (bs, &htable)) {

		#if lazythis_optimization
		
		if (equalidentifiers (bs, STR_this)) { /*PBS 05/12/01, dmb 11/14/01*/
		
			ctlazythis++;

			if (langgetthisaddress (&htable, bs)) {
		
				setaddressvalue (htable, bs, val);
			
				return (true);
				}
			}
		#endif
		}
	
	if (!langsymbolreference (htable, bs, val, &hnode))
		return (false);
	
	return (copyvaluerecord (*val, val));
	} /*idvalue*/


boolean dotvalue (hdltreenode h, tyvaluerecord *val) {
	
	/*
	h may be a tree node of type "dotop".  we return a value from an external
	symbol table.
	
	4/17/91 dmb: use langsymbolreference to save code & handle "root"
	
	6/11/92 dmb: check for objspec trees
	*/
	
	bigstring bsvarname;
	hdlhashtable htable;
	boolean flerrornode = (h == herrornode);
	hdlhashnode hnode;
	
	if (isobjspectree (h))
		return (evaluateobjspec (h, val));
	
	if (!langgetdotparams (h, &htable, bsvarname))
		return (false);
	
	assert (htable != nil);

	if (flerrornode)
		langseterrorline (h);
	
	if (!langsymbolreference (htable, bsvarname, val, &hnode))
		return (false);
	
	return (copyvaluerecord (*val, val));
	} /*dotvalue*/


boolean addressofvalue (hdltreenode htree, tyvaluerecord *val) {
	
	/*
	2/2/91 dmb: unfortunatly, paths have two problems as addresses.  first, 
	they're really inefficient:  we have to do a full path search to 
	generate the string, and then have to invoke the compiler, build a 
	code tree, and evaluate the code tree to get back to dotparams.  the 
	second problem is that, except when running under the debugger, local 
	tables don't have names and aren't linked into the database, so full 
	paths can't be generated and names at different scope levels can't be 
	distinguished.
	
	the solution is to just keep the indentifier name as a string, and 
	stash the hashtable handle into the value record instead of relying on 
	a full path.  this should be reliable and much, much faster.  the only 
	down side is that address values now only have run-time life; after 
	that, only the identifier name survives.  the same problem exists in 
	normal, compiled languages -- memory addresses don't survive program 
	launches.  note, however, that casting still provides the functionality 
	of saving addresses in the database, i.e. address ("scratchpad.x") does 
	what you want it to do.
	*/
	
	hdlhashtable htable;
	bigstring bsname;
	
	if (!langgetdotparams (htree, &htable, bsname))
		return (false);
	
	if ((htable == nil) && !equalstrings (bsname, nameroottable))
		langsearchpathlookup (bsname, &htable);
	
	return (setaddressvalue (htable, bsname, val));
	} /*addressofvalue*/


boolean dereferencevalue (hdltreenode htree, tyvaluerecord *val) {
	
	hdlhashtable htable;
	bigstring bs;
	hdlhashnode hnode;
	
	if (!evaluatetree (htree, val))
		return (false);
	
	if (!coercetoaddress (val))
		return (false);
	
	if (!getaddressvalue (*val, &htable, bs))
		return (false);
	
	if (!langsymbolreference (htable, bs, val, &hnode))
		return (false);
	
	return (copyvaluerecord (*val, val));
	} /*dereferencevalue*/


static boolean getvalidstringindex (tyvaluerecord *vstring, bigstring bsname, tyvaluerecord *vindex, long *idx) {
	
	/*
	2.1b3 dmb: if string is a binary, account for binary subtype
	*/
	
	register long ix;
	register long ixmax;
	
	if (!coercetolong (vindex))
		return (false);
	
	ix = (*vindex).data.longvalue;
	
	ixmax = gethandlesize ((*vstring).data.stringvalue);
	
	if ((*vstring).valuetype == binaryvaluetype) /*skip binary subtype*/
		ixmax -= sizeof (OSType);
	
	if (ix < 1 || ix > ixmax) {
		
		langarrayreferror (arrayindexerror, bsname, vstring, vindex);
		
		return (false);
		}
	
	if ((*vstring).valuetype == binaryvaluetype) /*skip binary subtype*/
		ix += sizeof (OSType);
	
	*idx = ix;
	
	return (true);
	} /*getvalidstringindex*/


static boolean stringassignvalue (tyvaluerecord *varray, bigstring bsname, tyvaluerecord *vindex, tyvaluerecord *vassign) {
	
	/*
	varray is an actual dbvalue, not a temp value.
	
	bsname is passed for error reporting only.

	5.0.2b21 dmb: must copy vassign before coercing it.
	*/
	
	Handle hstring;
	long ix;
	tyvaluerecord vchar;
	
	if (!getvalidstringindex (varray, bsname, vindex, &ix))
		return (false);
	
	if (!copyvaluerecord (*vassign, &vchar) || !coercetochar (&vchar))
		return (false);
	
	hstring = (*varray).data.stringvalue;
	
	(*hstring) [ix - 1] = vchar.data.chvalue; /*changes actual string*/
	
	return (true);
	} /*stringassignvalue*/


static boolean stringarrayvalue (tyvaluerecord *varray, bigstring bsname, tyvaluerecord *vindex, tyvaluerecord *vreturned) {
	
	Handle hstring;
	long ix;
	
	if (!getvalidstringindex (varray, bsname, vindex, &ix))
		return (false);
	
	hstring = (*varray).data.stringvalue;
	
	return (setcharvalue ((*hstring) [ix - 1], vreturned));
	} /*stringarrayvalue*/


static boolean stringdeletevalue (tyvaluerecord *varray, bigstring bsname, tyvaluerecord *vindex) {
	
	long ix;
	
	if (!getvalidstringindex (varray, bsname, vindex, &ix))
		return (false);
	
	return (pullfromhandle ((*varray).data.stringvalue, ix - 1, 1L, nil));
	} /*stringdeletevalue*/


static boolean tablearrayvalue (tyvaluerecord *varray, bigstring bsname, tyvaluerecord *vindex, tyvaluerecord *val) {
	
	/*
	get the value from the table in varray specified by the vindex.  update 
	bsname to the name of that value. (initially, it's the name of the table 
	itself.)
	*/
	
	hdlhashtable htable;
	bigstring stringindex;
	long intindex;
	hdlhashnode hnode;
	
	if (!langexternalvaltotable (*varray, &htable, HNoNode)) {
		
		langarrayreferror (arraynottableerror, bsname, varray, nil);
		
		return (false);
		}
	
	if ((*vindex).valuetype == stringvaluetype) {
	
		boolean fl;
		
		pullstringvalue (vindex, stringindex);
		
		pushhashtable (htable);
		
		fl = langgetsymbolval (stringindex, val, &hnode);
		
		pophashtable ();
		
		if (!fl) {
			
			langarrayreferror (arraystringindexerror, bsname, varray, vindex);
			
			return (false);
			}
		
		copystring (stringindex, bsname);
		}
	else {
		if (!coercetolong (vindex))
			return (false);
		
		intindex = (*vindex).data.longvalue;
		
		if ((intindex <= 0) || !hashgetiteminfo (htable, intindex - 1, bsname, val)) {
			
			langarrayreferror (arrayindexerror, bsname, varray, vindex);
			
			return (false);
			}
		}
	
	return (true);
	} /*tablearrayvalue*/


#define maxarraystack 7

#pragma pack(2)
typedef struct tyarrayspec {
	
	tyvaluerecord varray; /*the array containing the retrieved value (which may itself be an array)*/
	
	tyvaluerecord vindex; /*the index into varray that the retrieved value came from*/
	} tyarrayspec;


typedef struct tyarraystack {
	
	short topstack;
	
	tyarrayspec element [maxarraystack];
	} tyarraystack;
#pragma options align=reset


static boolean parsearrayreference (hdltreenode htree, tyarraystack *pstack, hdlhashtable *htable, bigstring bsname, tyvaluerecord *val) {
	
	/*
	3.0.2 dmb: see comment in assignordeletearrayvalue. we recurse until we get 
	to the root of the array reference, at which point htable & bsname are set by 
	langgetdottedsymbolval.
	
	when the parent array is a table, we don't retrieve the actual node value, not 
	a copy. This allows us to directly modify the value instead of having to assign 
	back into the table later. if the caller is just getting the value, it will make 
	a copy itself.
	
	as we evaluate each level, we store the array/index information into pstack, so  
	that our caller can update the values if desired. (otherwise it can pass nil.)
	anything we put into the srraystack must be protected from garbage collection. 
	any call to evaluatetree might call a function, which empties the temp stack.
	
	3.0.2b3 make retrieval of actual val optional by accepting nil for val.  this is 
	necessary to allow nameOf and assignment to work with non-existant items
	
	5.0b17: if the value was a tmp, use pushtmpstackvalue to make sure it goes back
	*/
	
	tyvaluerecord varray;
	tyvaluerecord vindex;
	register short top = 0;
	boolean fltmp;
	boolean fl;
	hdlhashnode hnode;
	
	if ((**htree).nodetype != arrayop) {
		
		assert (val != nil);
		
		return (langgetdottedsymbolval (htree, htable, bsname, val, &hnode));
		}
	
	if (!evaluatetree ((**htree).param2, &vindex))
		return (false);
	
	fltmp = exemptfromtmpstack (&vindex); /*protect index while we recurse*/
	
	fl = parsearrayreference ((**htree).param1, pstack, htable, bsname, &varray); /*recurse*/
	
	if (fltmp)
		pushtmpstackvalue (&vindex);
	
	if (!fl)
		return (false);
	
	if (pstack != nil) { /*need to record array reference*/
		
		top = (*pstack).topstack;
		
		if (top == maxarraystack) {
			
			langerror (tmpstackoverflowerror); /*the message for this error fits*/
			
			return (false);
			}
		
		(*pstack).element [top].varray = varray;
		}
	
	if (val != nil) { /*need to retrieve value*/
		
		switch (varray.valuetype) { /*note: vindex may be coerced within this switch*/
			
			case stringvaluetype:
				fl = stringarrayvalue (&varray, bsname, &vindex, val);
				
				break;
			
			case binaryvaluetype:
				if (!stringarrayvalue (&varray, bsname, &vindex, val))
					return (false);
				
				fl = coercetoint (val);
				
				break;
			
			case listvaluetype:
			case recordvaluetype:
				fl = listarrayvalue (&varray, bsname, &vindex, val);
				
				break;
			
			case externalvaluetype:
				fl = tablearrayvalue (&varray, bsname, &vindex, val);
				
				break;
			
			default:
				langarrayreferror (arraynottableerror, bsname, &varray, nil);
				
				return (false);
			}
		
		if (!fl)
			return (false);
		}
	
	if (pstack != nil) {
		
		(*pstack).element [top].vindex = vindex;
		
		(*pstack).topstack++; /*actually accept new stack element*/
		}
	
	return (true);
	} /*parsearrayreference*/


static boolean setarrayelement (tyvaluerecord *varray, tyvaluerecord *vindex, tyvaluerecord *vassign, bigstring bsname, boolean fljustdirtytable) {
	
	/*
	assign the value in vassign to the vindex element of varray. if assigning 
	into a table, make sure to remove value from temp stack
	*/
	
	boolean fl;
	hdlhashtable htable;
	
	switch ((*varray).valuetype) {
		
		case stringvaluetype:
		case binaryvaluetype:
			if (vassign == nil)
				fl = stringdeletevalue (varray, bsname, vindex);
			else
				fl = stringassignvalue (varray, bsname, vindex, vassign);
			
			break;
		
		case listvaluetype:
		case recordvaluetype:
			if (vassign == nil)
				fl = listdeletevalue (varray, bsname, vindex);
			else
				fl = listassignvalue (varray, bsname, vindex, vassign);
			
			break;
		
		case externalvaluetype:
			if (!langexternalvaltotable (*varray, &htable, HNoNode)) {
				
				langarrayreferror (arraynottableerror, bsname, varray, nil);
				
				return (false);
				}
			
			if (!langgettableitemname (htable, vindex, bsname))
				return (false);
			
			if (fljustdirtytable) {
				
				langsymbolchanged (htable, bsname, HNoNode, true);
				
				fl = true;
				}
			else {
				
				pushhashtable (htable);
				
				if (vassign == nil)
					fl = hashdelete (bsname, true, true);
				else
					fl = langsetsymbolval (bsname, *vassign);
				
				pophashtable ();
				
				if (fl && vassign)
					exemptfromtmpstack (vassign);
				}
			
			break;
		
		default:
			langarrayreferror (arraynottableerror, bsname, varray, nil);
			
			return (false);
		}
	
	return (fl);
	} /*setarrayelement*/


static boolean assignordeletearrayvalue (register hdltreenode h, tyvaluerecord *vassign, tytreetype op, tyvaluerecord *vold, tyvaluerecord *vnew) {
	
	/*
	3.0.2 dmb: now I see why I didn't get this into 3.0. Very hairy. One key 
	complication: array indexes must only be evaluated once, in case they 
	contain expressions with side effects. Since sub-elements aren't directly 
	addressable, we must maintain a list of subarrays and subindexes so that once 
	we've retrieved the smallest element that is referred to, we can assign 
	back into all of the containing arrays. I can't see how to do this with 
	straight recursion, but it looks like a stack will do the trick.
	
	when assigning back into parent arrays, as soon as we hit a non-temp value, 
	we're modifying an actual table value and just need to mark its parent as 
	dirty. we don't need to continue dirtying the parents of that table, so 
	we can stop the traversal once we've done so.
	
	3.0.2b3: don't try to retrieve element value unless we're modifying it.
	
	5.0b17: if the value was a tmp, use pushtmpstackvalue to make sure it goes back
	
	5.0.2b14 dmb: to support modifyassignvalue, we must notice when the result of
	an addition or subtraction is heap allocated, but not on the temp stack. in that
	case, we modified the first operand (the LHS) directly, and must not hashassign
	the result.
	*/
	
	tyvaluerecord *varray;
	tyvaluerecord velement;
	tyarraystack arraystack = {0};
	hdlhashtable htable;
	bigstring bsname;
	boolean fljustdirtytable = false;
	register short top;
	boolean flincdec = (op == addop) || (op == subtractop);
	boolean fltmp;
	boolean fl;
	tyvaluerecord *vref = nil;
	
	if (flincdec)
		vref = &velement;
	
	fltmp = (vassign != nil) && exemptfromtmpstack (vassign);
	
	fl = parsearrayreference (h, &arraystack, &htable, bsname, vref);
	
	if (fltmp)
		pushtmpstackvalue (vassign);
	
	if (!fl)
		return (false);
	
	if (flincdec) {
		
		if (vold)
			*vold = velement;
		
		// 5.0.2b14 dmb - not anymore: copyvaluerecord (velement, &velement); /*make sure it's a temp*/
		
		if (op == addop)
			fl = addvalue (velement, *vassign, vassign);
		else
			fl = subtractvalue (velement, *vassign, vassign);
		
		if (!fl)
			return (false);
		
		if (vnew)
			*vnew = *vassign;
		
		fljustdirtytable = langheapallocated (vassign, nil) && !(*vassign).fltmpstack; // 5.0.2b14
		}
	
	top = arraystack.topstack;
	
	while (--top >= 0) { /*descend the stack, assigning subelements into their parent arrays*/
		
		varray = &arraystack.element [top].varray;
		
		if (!setarrayelement (varray, &arraystack.element [top].vindex, vassign, bsname, fljustdirtytable))
			return (false);
		
		if ((*varray).valuetype == externalvaluetype) /*we just assigned to or dirtied a table*/
			break;
		
		fljustdirtytable = true; /*we're updating an actual value; don't need to assign to table*/
		
		vassign = varray; /*this array will become an element of the parent array*/
		}
	
	langsymbolchanged (htable, bsname, HNoNode, true);
	
	return (true);
	} /*assignordeletearrayvalue*/


static boolean assignordeletevalue (register hdltreenode hlhs, tyvaluerecord *vassign, tytreetype op, tyvaluerecord *vold, tyvaluerecord *vnew) {
	
	/*
	2.1b2 dmb: common code; delete item if vassign is nil
	
	3.0.2 dmb: new feature, our caller passes on opcode which might be addop. in that 
	case, we add vassign to the existing value, and return both the old and new value 
	in vold and vnew. right now we're using this to implement ++ and --. in the future, 
	this will make implementing += and -= trivial, and *= and /= pretty easy too.
	
	4.1b4 dmb: added fllangexternalvalueprotect flag to disable protection
	
	5.0a18 dmb: generate error if lhs is undefined; must declare variables
	
	5.0.2b10 dmb: don't dup vassign if it's already a temp

	5.0.2b11 dmb: don't keep table pushed during addvalue call

	5.0.2b13 dmb: undid b10 change; crashes string.innerCaseName.
	*/
	
	hdlhashtable htable;
	bigstring bsname;
	hdlhashnode hnode;
	tyvaluerecord vtmp;
	boolean fl = false;
	
	if (vassign != nil) {
		
		if (fllangexternalvalueprotect && (*vassign).valuetype == externalvaluetype) { /*4.1b4 dmb*/
			
			langbadexternaloperror (externalassignerror, *vassign);
			
			return (false);
			}
		
		if (!(*vassign).fltmpstack && !copyvaluerecord (*vassign, vassign))
			return (false);
		}
	
	if ((**hlhs).nodetype == arrayop) { /*may be array into object other than a table*/
		
		return (assignordeletearrayvalue (hlhs, vassign, op, vold, vnew));
		}
	
	if (!langgetdotparams (hlhs, &htable, bsname))
		return (false);
	
	if (htable == nil)
		langsearchpathlookup (bsname, &htable); /*12/18/92*/
	
	assert (htable != nil);
	
	switch (op) {
		
		case noop: /*delete*/
			return (hashtabledelete (htable, bsname));
		
		case assignop:
			pushhashtable (htable);

			fl = langsetsymbolval (bsname, *vassign);
			
			pophashtable ();
			
			if (!fl)
				return (false);
			
			exemptfromtmpstack (vassign); /*save value from being garbage collected*/
			
			return (true);
		
		case addop:
		case subtractop:
			pushhashtable (htable);

			fl = langfindsymbol (bsname, &htable, &hnode);
			
			pophashtable ();
			
			if (!fl) {
								
				if (equalstrings (bsname, nameroottable)) {
				
					bigstring bstype;
					
					langexternaltypestring ((hdlexternalhandle) rootvariable, bstype);
					
					lang2paramerror (badexternalassignmenterror, bstype, bsname);
					}
				else
					langparamerror (unknownidentifiererror, bsname);
				
				return (false);
				}
			
			vtmp = (**hnode).val;
			
			if (vold)
				*vold = vtmp;
			
			if (op == addop)
				fl = addvalue (vtmp, *vassign, &vtmp);
			else
				fl = subtractvalue (vtmp, *vassign, &vtmp);
			
			if (!fl)
				return (false);
			
			exemptfromtmpstack (&vtmp);
			
			assert (!vtmp.fltmpstack);

			(**hnode).val = vtmp;
			
			langsymbolchanged (htable, bsname, hnode, true); /*value changed*/
			
			if (vnew)
				*vnew = vtmp;
			
			return (true);

		default:
			langlongparamerror (unexpectedopcodeerror, (long) op);

			return (false);
		}
	} /*assignordeletevalue*/


boolean assignvalue (register hdltreenode hlhs, tyvaluerecord vrhs) {
	
	/*
	3/11/91 dmb: disallow externalvaluetype assignments
	
	10/8/91 dmb: set can clear new fllanghashassignprotect flag to avoid 
	destructive assignment.
	
	12/26/91 dmb: don't exempt from temp stack until assignment is sucessful
	
	12/18/92 dmb: do a searchpathlookup when htable is nil; otherwise, can't 
	assign to a non-dotted id in paths table
	
	2.1b2 dmb: do binary values too
	*/
	
	boolean fl;
	
	fllanghashassignprotect = true;
	
	fl = assignordeletevalue (hlhs, &vrhs, assignop, nil, nil);
	
	fllanghashassignprotect = false;
	
	return (fl);
	} /*assignvalue*/


boolean deletevalue (hdltreenode hdel, tyvaluerecord *vreturned) {
	
	/*
	2.1b2 dmb: new routine to handle array-based deletions
	*/
	
	if (!assignordeletevalue (hdel, nil, noop, nil, nil))
		return (false);
	
	return (setbooleanvalue (true, vreturned));
	} /*deletevalue*/


boolean modifyassignvalue (register hdltreenode hlhs, tyvaluerecord vrhs, tytreetype modifyop, tyvaluerecord *vassigned, boolean flneednewcopy) {
	
	/*
	5.0.2b10 dmb: new routine for +=, -=, *=, /=. Currently, only += is generated, 
	as a compiler optimization (not a supported operator).
	
	if flneednewcopy is true, caller needs copy of result value to be on the temp stack.
	*/
	
	boolean fl;
	
	fllanghashassignprotect = true;
	
	fl = assignordeletevalue (hlhs, &vrhs, modifyop, nil, vassigned);
	
	fllanghashassignprotect = false;
	
	if (!fl)
		return (false);
	
	if (((*vassigned).valuetype == externalvaluetype) || !flneednewcopy)
		return (setbooleanvalue (true, vassigned)); /*could be a local extern*/
	else
		return ((*vassigned).fltmpstack || copyvaluerecord (*vassigned, vassigned)); /*side-effect of assignment*/
	} /*modifyassignvalue*/


boolean arrayvalue (hdltreenode h, tyvaluerecord *val) {
	
	/*
	get an array reference.  h's nodetype == arrayop.  the first param must be the
	name of an external variable, of type table.
	
	valindex holds a value which is either a string, or can be coerced to a short.
	
	if it's a short index, return the value of the nth item in the table, order is
	determined by the sort order of the table.  the index is 1-based, the first 
	item is number 1.
	
	if it's a string index, we return the value of the item whose key equals the
	string.
	
	1/25/91 dmb: must return copy, not original
	
	6/11/92 dmb: check for objspec trees
	
	2.1b2 dmb: do binary values too
	
	5.0.2b10 dmb: don't recopy val unnecessarily
	*/
	
	hdlhashtable htable;
	bigstring bsname;
	
	if (isobjspectree (h))
		return (evaluateobjspec (h, val));
	
	if (!parsearrayreference (h, nil, &htable, bsname, val))
		return (false);
	
	if (!(*val).fltmpstack)
		if (!copyvaluerecord (*val, val))
			return (false);
	
	return (true);
	} /*arrayvalue*/


boolean incrementvalue (boolean flincr, boolean flpre, hdltreenode hvar, tyvaluerecord *vreturned) {
	
	/*
	implement all four autoincrement operations.  if flincr is true then we add,
	if false, we subtract.  if flpre, we do it before determining the returned
	value, if false we do the increment or decrement and then return the value.
	
	7/12/90 DW: allow dotted id's on auto-increment/decrement.
	
	9/6/91 dmb: work on a copy of the original value; it may be heap-allocated.  
	also, must exempt from temp stack before assignment
	*/
	
	tyvaluerecord oldval, newval, valauto;
	tytreetype op;
	register boolean fl;
	
	setlongvalue (1, &valauto);
	
	if (flincr)
		op = addop;
	else
		op = subtractop;
	
	fl = assignordeletevalue (hvar, &valauto, op, &oldval, &newval);
	
	if (fl) {
	
		if (flpre)
			fl = copyvaluerecord (newval, vreturned);
		else
			fl = copyvaluerecord (oldval, vreturned);
		}
	
	return (fl);
	} /*incrementvalue*/


boolean idstringvalue (hdlhashtable htable, bigstring bsvarname, bigstring bsvalue) {
	
	/*
	7/30/91 dmb: rewrote; used to trash original value
	*/
	
	tyvaluerecord val;
	boolean fl;
	hdlhashnode hnode;
	
	setemptystring (bsvalue);
	
	pushhashtable (htable);
	
	fl = langgetsymbolval (bsvarname, &val, &hnode);
	
	pophashtable ();
	
	if (!fl)
		return (false);
	
	if (!copyvaluerecord (val, &val))
		return (false);
	
	if (!coercetostring (&val))
		return (false);
	
	pullstringvalue (&val, bsvalue);
	
	return (true);
	} /*idstringvalue*/


boolean addvalue (tyvaluerecord v1, tyvaluerecord v2, tyvaluerecord *vreturned) {
	
	/*
	7/16/92 dmb: added special case for adding two characters together 
	to form a string
	
	2.1b2 dmb: special case for filespec on lhs, don't coerce types
	
	5.0a23 dmb: use pushhandle, not concathandles for string and binary addition
				added special case for novaluetype
	
	5.0 dmb: since we throw away v1 and v2, let's see where we can boost
	performance by adding directly to v1 and making that the return value.
	*/
	
	register boolean fl = true;
	tyvaluetype v2type = v2.valuetype;
	
	if (v2.valuetype == novaluetype) {
	
		*vreturned = v1;
		
		return (true);
		}
	
	if (v1.valuetype == novaluetype) {
		
		*vreturned = v2;
		
		return (true);
		}
	
	if (v1.valuetype != filespecvaluetype) {
		
		if (!coercetypes (&v1, &v2)) {
			
			disposevalues (&v1, &v2);
			
			return (false);
			}
		}
	
	initvalue (vreturned, v1.valuetype);
	
	switch (v1.valuetype) {
		
		case booleanvaluetype:
			(*vreturned).data.flvalue = v1.data.flvalue || v2.data.flvalue;
			
			break;
		
		case charvaluetype:
			if (v2type == charvaluetype) { /*special case: adding two character together*/
				byte s [4];
				
				setstringwithchar (v1.data.chvalue, s);
				
				pushchar (v2.data.chvalue, s);
				
				fl = setstringvalue (s, vreturned);
				}
			else
				(*vreturned).data.chvalue = v1.data.chvalue + v2.data.chvalue;
			
			break;
		
		case intvaluetype:
			(*vreturned).data.intvalue = v1.data.intvalue + v2.data.intvalue;
			
			break;
		
		case longvaluetype:	
		case ostypevaluetype:
			(*vreturned).data.longvalue = v1.data.longvalue + v2.data.longvalue;
			
			break;
			
		case directionvaluetype:
			(*vreturned).data.dirvalue = (tydirection) ((short) v1.data.dirvalue + (short) v2.data.dirvalue);
			
			break;
		
		case datevaluetype:
			(*vreturned).data.datevalue = v1.data.datevalue + v2.data.datevalue;
			
			break;
		
		case fixedvaluetype:
			(*vreturned).data.fixedvalue = v1.data.fixedvalue + v2.data.fixedvalue;
			
			break;
		
		case singlevaluetype:
			(*vreturned).data.singlevalue = v1.data.singlevalue + v2.data.singlevalue;
			
			break;
		
		case doublevaluetype:
			fl = setdoublevalue (**v1.data.doublevalue + **v2.data.doublevalue, vreturned);
			
			break;
		
		case stringvaluetype: {
				fl = pushhandle (v2.data.stringvalue, v1.data.stringvalue);
				
				if (!fl)
					break;
				
				*vreturned = v1;
				
				v1.valuetype = novaluetype;
			
			break;
			}
		
		case binaryvaluetype: {
				stripbinarytypeid (v2.data.binaryvalue);
				
				fl = pushhandle (v2.data.binaryvalue, v1.data.binaryvalue);
				
				if (!fl)
					break;
				
				setbinarytypeid (v1.data.binaryvalue, '\?\?\?\?');
				
				*vreturned = v1;
				
				v1.valuetype = novaluetype;
			
			break;
			}
		
		case filespecvaluetype:
			return (filespecaddvalue (&v1, &v2, vreturned));
		
		case listvaluetype:
		case recordvaluetype:
			fl = listaddvalue (&v1, &v2, vreturned);
			
			break;
		
		default:
			langerror (additionnotpossibleerror);
			
			fl = false; /*addition is not defined*/
			
			break;
		} /*switch*/
	
	disposevalues (&v1, &v2);
	
	return (fl);
	} /*addvalue*/
	
	
boolean subtractvalue (tyvaluerecord v1, tyvaluerecord v2, tyvaluerecord *vreturned) {
	
	/*
	2.1b2 dmb: special case for filespec on lhs, don't coerce types
	*/
	
	register boolean fl = true;
	
	if (v1.valuetype != filespecvaluetype) {
		
		if (!coercetypes (&v1, &v2)) {
			
			disposevalues (&v1, &v2);
			
			return (false);
			}
		}
	
	initvalue (vreturned, v1.valuetype);
	
	switch (v1.valuetype) {
		
		case booleanvaluetype:
			(*vreturned).data.flvalue = v1.data.flvalue - v2.data.flvalue;
			
			break;
		
		case charvaluetype:
			(*vreturned).data.chvalue = v1.data.chvalue - v2.data.chvalue;
			
			break;
		
		case intvaluetype:
			(*vreturned).data.intvalue = v1.data.intvalue - v2.data.intvalue;
			
			break;
		
		case longvaluetype:	
		case ostypevaluetype:
			(*vreturned).data.longvalue = v1.data.longvalue - v2.data.longvalue;
			
			break;
			
		case directionvaluetype:
			(*vreturned).data.dirvalue = (tydirection) ((short) v1.data.dirvalue - (short) v2.data.dirvalue);
			
			break;
			
		case datevaluetype:
			(*vreturned).data.datevalue = v1.data.datevalue - v2.data.datevalue;
			
			break;
		case fixedvaluetype:
			(*vreturned).data.fixedvalue = v1.data.fixedvalue - v2.data.fixedvalue;
			
			break;
		case singlevaluetype:
			(*vreturned).data.singlevalue = v1.data.singlevalue - v2.data.singlevalue;
			
			break;
		
		case doublevaluetype:
			fl = setdoublevalue (**v1.data.doublevalue - **v2.data.doublevalue, vreturned);
			
			break;
		
		case stringvaluetype: {
				long ix;
				
				ix = searchhandle (v1.data.stringvalue, v2.data.stringvalue, 0, longinfinity);
				
				if (ix >= 0)
					pullfromhandle (v1.data.stringvalue, ix, gethandlesize (v2.data.stringvalue), nil);
				
				*vreturned = v1;
				
				v1.valuetype = novaluetype;
			
			break;
			
			}
		
		case filespecvaluetype:
			return (filespecsubtractvalue (&v1, &v2, vreturned));
		
		case listvaluetype:
		case recordvaluetype:
			fl = listsubtractvalue (&v1, &v2, vreturned);
			
			break;
		
		default:
			langerror (subtractionnotpossibleerror);
			
			fl = false; /*subtraction is not defined*/
			
			break;
		} /*switch*/
	
	disposevalues (&v1, &v2);
	
	return (fl);
	} /*subtractvalue*/
	
	
boolean multiplyvalue (tyvaluerecord v1, tyvaluerecord v2, tyvaluerecord *vreturned) {
	
	register boolean fl = true;
	
	if (!coercetypes (&v1, &v2)) {
		
		disposevalues (&v1, &v2);
		
		return (false);
		}
	
	initvalue (vreturned, v1.valuetype);
	
	switch (v1.valuetype) {
		
		case booleanvaluetype:
			(*vreturned).data.flvalue = v1.data.flvalue * v2.data.flvalue;
			
			break;
		
		case charvaluetype:
			(*vreturned).data.chvalue = v1.data.chvalue * v2.data.chvalue;
			
			break;
		
		case intvaluetype:
			(*vreturned).data.intvalue = v1.data.intvalue * v2.data.intvalue;
			
			break;
		
		case longvaluetype:	
			(*vreturned).data.longvalue = v1.data.longvalue * v2.data.longvalue;
			
			break;
			
		case directionvaluetype:
			(*vreturned).data.dirvalue = (tydirection) ((short) v1.data.dirvalue * (short) v2.data.dirvalue);
			
			break;
			
		case datevaluetype:
			(*vreturned).data.datevalue = v1.data.datevalue * v2.data.datevalue;
			
			break;
		case fixedvaluetype:
			(*vreturned).data.fixedvalue = FixMul (v1.data.fixedvalue, v2.data.fixedvalue);
			
			break;
			
		case singlevaluetype:
			(*vreturned).data.singlevalue = v1.data.singlevalue * v2.data.singlevalue;
			
			break;
		
		case doublevaluetype:
			fl = setdoublevalue (**v1.data.doublevalue * **v2.data.doublevalue, vreturned);
			
			break;
		
		default:
			langerror (multiplicationnotpossibleerror);
			
			fl = false; /*multiplication is not defined*/
			
			break;
		} /*switch*/
	
	disposevalues (&v1, &v2);
		
	return (fl);
	} /*multiplyvalue*/


static boolean nonzerovalue (tyvaluerecord val) {
	
	/*
	9/17/91 dmb: pulled code from dividevalue so it can be shared by modvalue
	*/
	
	register long denom;
	
	switch (val.valuetype) {
		
		case booleanvaluetype:
			denom = val.data.flvalue;
			
			break;
		
		case charvaluetype:
			denom = val.data.chvalue;
			
			break;
			
		case intvaluetype:
			denom = val.data.intvalue;
			
			break;
			
		case longvaluetype:	
			denom = val.data.longvalue;
			
			break;
		
		case directionvaluetype:
			denom = val.data.dirvalue;
			
			break;
			
		case datevaluetype:
			denom = val.data.datevalue;
			
			break;
		
		case singlevaluetype:
			denom = (val.data.singlevalue == 0.0? 0 : 1); /*avoid rounding errors*/
			
			break;
		
		case doublevaluetype:
			denom = (**val.data.doublevalue == 0.0? 0 : 1); /*avoid rounding errors*/
			
			break;
		
		default:
			denom = 1; /*not zero*/
		} /*switch*/
	
	if (denom == 0) {
		
		langerror (dividebyzeroerror);
		
		return (false);
		}
	
	return (true);
	} /*nonzerovalue*/


boolean dividevalue (tyvaluerecord v1, tyvaluerecord v2, tyvaluerecord *vreturned) {
	
	register boolean fl = true;
	
	if (!coercetypes (&v1, &v2)) {
		
		disposevalues (&v1, &v2);
		
		return (false);
		}
	
	if (!nonzerovalue (v2)) {
		
		disposevalues (&v1, &v2);
		
		return (false);
		}
	
	initvalue (vreturned, v1.valuetype);
	
	switch (v1.valuetype) {
		
		case booleanvaluetype:
			(*vreturned).data.flvalue = v1.data.flvalue / v2.data.flvalue;
			
			break;
		
		case charvaluetype:
			(*vreturned).data.chvalue = v1.data.chvalue / v2.data.chvalue;
			
			break;
			
		case intvaluetype:
			(*vreturned).data.intvalue = v1.data.intvalue / v2.data.intvalue;
			
			break;
			
		case longvaluetype:
			(*vreturned).data.longvalue = v1.data.longvalue / v2.data.longvalue;
			
			break;
			
		case directionvaluetype:
			(*vreturned).data.dirvalue = (tydirection) ((short) v1.data.dirvalue / (short) v2.data.dirvalue);
			
			break;
			
		case datevaluetype:
			(*vreturned).data.datevalue = v1.data.datevalue / v2.data.datevalue;
			
			break;
		
		case singlevaluetype:
			(*vreturned).data.singlevalue = v1.data.singlevalue / v2.data.singlevalue;
			
			break;
		
		case doublevaluetype:
			fl = setdoublevalue (**v1.data.doublevalue / **v2.data.doublevalue, vreturned);
			
			break;
		
		default:
			langerror (divisionnotpossibleerror);
			
			fl = false; /*division is not defined*/
			
			break;
		} /*switch*/
	
	disposevalues (&v1, &v2);
		
	return (fl);
	} /*dividevalue*/


boolean modvalue (tyvaluerecord v1, tyvaluerecord v2, tyvaluerecord *vreturned) {
	
	/*
	9/17/91 dmb: check modulus for zero
	*/
	
	register boolean fl = true;
	
	if (!coercetypes (&v1, &v2)) {
		
		disposevalues (&v1, &v2);
		
		return (false);
		}
	
	if (!nonzerovalue (v2)) {
		
		disposevalues (&v1, &v2);
		
		return (false);
		}
	
	initvalue (vreturned, v1.valuetype);
	
	switch (v1.valuetype) {
		
		case booleanvaluetype:
			(*vreturned).data.flvalue = v1.data.flvalue % v2.data.flvalue;
			
			break;
		
		case charvaluetype:
			(*vreturned).data.chvalue = v1.data.chvalue % v2.data.chvalue;
			
			break;
		
		case intvaluetype:
			(*vreturned).data.intvalue = v1.data.intvalue % v2.data.intvalue;
			
			break;
		
		case longvaluetype:	
			(*vreturned).data.longvalue = v1.data.longvalue % v2.data.longvalue;
			
			break;
		
		case directionvaluetype:
			(*vreturned).data.dirvalue = (tydirection) ((short) v1.data.dirvalue % (short) v2.data.dirvalue);
			
			break;
		
		case datevaluetype:
			(*vreturned).data.datevalue = v1.data.datevalue % v2.data.datevalue;
			
			break;
		
		default:
			langerror (modulusnotpossibleerror);
			
			fl = false; /*modulus operation is not defined*/
			
			break;
		} /*switch*/
	
	disposevalues (&v1, &v2);
	
	return (fl);
	} /*modvalue*/


boolean EQvalue (tyvaluerecord v1, tyvaluerecord v2, tyvaluerecord *vreturned) {
	
	/*
	12/21/92 dmb: added case for novaluetype
	
	2.1b3 dmb: if values can't be coerced, vreturned is false but don't generate 
	an error
	*/
	
	boolean flcomparable;
	boolean fl = true;
	
	initvalue (vreturned, booleanvaluetype);
	
	disablelangerror ();
	
	flcomparable = coercetypes (&v1, &v2);
	
	enablelangerror ();
	
	if (!flcomparable) {
		
		disposevalues (&v1, &v2);
		
		return (true);
		}
	
	switch (v1.valuetype) {
		
		case novaluetype:
			(*vreturned).data.flvalue = true;
			
			break;
		
		case booleanvaluetype:
			(*vreturned).data.flvalue = v1.data.flvalue == v2.data.flvalue;
			
			break;
		
		case charvaluetype:
			(*vreturned).data.flvalue = v1.data.chvalue == v2.data.chvalue;
			
			break;
			
		case intvaluetype:
		case tokenvaluetype:
			(*vreturned).data.flvalue = v1.data.intvalue == v2.data.intvalue;
			
			break;
		
		case longvaluetype:
		case ostypevaluetype:
		case pointvaluetype:
		case fixedvaluetype:
		case singlevaluetype:
		case enumvaluetype:
			(*vreturned).data.flvalue = v1.data.longvalue == v2.data.longvalue;
			
			break;
			
		case directionvaluetype:
			(*vreturned).data.flvalue = v1.data.dirvalue == v2.data.dirvalue;
			
			break;
			
		case datevaluetype:
			(*vreturned).data.flvalue = v1.data.datevalue == v2.data.datevalue;
			
			break;
		
		case addressvaluetype: {
			bigstring bs1, bs2;
			hdlhashtable ht1, ht2;
			
			if (!getaddressvalue (v1, &ht1, bs1))
				return (false);
			
			if (!getaddressvalue (v2, &ht2, bs2))
				return (false);
			
			(*vreturned).data.flvalue = (ht1 == ht2) && equalidentifiers (bs1, bs2);
			
			break;
			}
		
		case listvaluetype:
		case recordvaluetype:
			fl = listcomparevalue (&v1, &v2, EQop, vreturned);
			
			break;
		
		case stringvaluetype:
		case binaryvaluetype:
		case doublevaluetype:
		case rectvaluetype:
		case rgbvaluetype:
		case patternvaluetype:
		case aliasvaluetype:
		case objspecvaluetype:
			(*vreturned).data.flvalue = equalhandles ((Handle) v1.data.binaryvalue, (Handle) v2.data.binaryvalue);
			
			break;
		
		case filespecvaluetype:
			(*vreturned).data.flvalue = equalfilespecs (*v1.data.filespecvalue, *v2.data.filespecvalue);
			
			break;
		
		default:
			langerror (comparisonnotpossibleerror);
			
			fl = false;
			
			break;
		} /*switch*/
	
	disposevalues (&v1, &v2);
	
	return (fl);
	} /*EQvalue*/


boolean NEvalue (tyvaluerecord v1, tyvaluerecord v2, tyvaluerecord *vreturned) {

	if (!EQvalue (v1, v2, vreturned)) 
		return (false);
	
	(*vreturned).data.flvalue = !(*vreturned).data.flvalue;
	
	return (true);
	} /*NEvalue*/
	
	
boolean GTvalue (tyvaluerecord v1, tyvaluerecord v2, tyvaluerecord *vreturned) {
	
	/*
	1/25/93 dmb: added case for novaluetype
	*/
	
	boolean fl = true;
	
	if (!coercetypes (&v1, &v2)) {
		
		disposevalues (&v1, &v2);
		
		return (false);
		}
	
	initvalue (vreturned, booleanvaluetype);
	
	switch (v1.valuetype) {
		
		case novaluetype:
			(*vreturned).data.flvalue = false;
			
			break;
		
		case booleanvaluetype:
			(*vreturned).data.flvalue = v1.data.flvalue > v2.data.flvalue;
					
			break;
		
		case charvaluetype:
			(*vreturned).data.flvalue = v1.data.chvalue > v2.data.chvalue;
					
			break;
					
		case intvaluetype:
		case tokenvaluetype:
			(*vreturned).data.flvalue = v1.data.intvalue > v2.data.intvalue;
					
			break;
		
		case longvaluetype:
		case ostypevaluetype:
		case fixedvaluetype:
			(*vreturned).data.flvalue = v1.data.longvalue > v2.data.longvalue;
					
			break;
		
		case directionvaluetype:
			(*vreturned).data.flvalue = (short) v1.data.dirvalue > (short) v2.data.dirvalue;
			
			break;
			
		case datevaluetype:
			(*vreturned).data.flvalue = timegreaterthan (v1.data.datevalue, v2.data.datevalue);
			
			break;
		
		case singlevaluetype:
			(*vreturned).data.flvalue = v1.data.singlevalue > v2.data.singlevalue;
			
			break;
		
		case doublevaluetype:
			(*vreturned).data.flvalue = **v1.data.doublevalue > **v2.data.doublevalue;
			
			break;
		
		case stringvaluetype:
			(*vreturned).data.flvalue = comparehandles (v1.data.stringvalue, v2.data.stringvalue) == 1;
			
			break;
		
		default:
			langerror (comparisonnotpossibleerror);
			
			fl = false; /*operation is not defined*/
			
			break;
		} /*switch*/
	
	disposevalues (&v1, &v2);
	
	return (fl);
	} /*GTvalue*/


boolean LEvalue (tyvaluerecord v1, tyvaluerecord v2, tyvaluerecord *vreturned) {

	if (!GTvalue (v1, v2, vreturned)) 
		return (false);
	
	(*vreturned).data.flvalue = !(*vreturned).data.flvalue;
	
	return (true);
	} /*LEvalue*/
	
	
boolean LTvalue (tyvaluerecord v1, tyvaluerecord v2, tyvaluerecord *vreturned) {
	
	/*
	1/25/93 dmb: added case for novaluetype
	*/
	
	boolean fl = true;
	
	if (!coercetypes (&v1, &v2)) {
		
		disposevalues (&v1, &v2);
		
		return (false);
		}
	
	initvalue (vreturned, booleanvaluetype);
	
	switch (v1.valuetype) {
		
		case novaluetype:
			(*vreturned).data.flvalue = false;
			
			break;
		
		case booleanvaluetype:
			(*vreturned).data.flvalue = v1.data.flvalue < v2.data.flvalue;
					
			break;
		
		case charvaluetype:
			(*vreturned).data.flvalue = v1.data.chvalue < v2.data.chvalue;
					
			break;
		
		case intvaluetype:
		case tokenvaluetype:
			(*vreturned).data.flvalue = v1.data.intvalue < v2.data.intvalue;
					
			break;
		
		case longvaluetype:
		case ostypevaluetype:
		case fixedvaluetype:
			(*vreturned).data.flvalue = v1.data.longvalue < v2.data.longvalue;
					
			break;
			
		case directionvaluetype:
			(*vreturned).data.flvalue = (short) v1.data.dirvalue < (short) v2.data.dirvalue;
			
			break;
			
		case datevaluetype:
			(*vreturned).data.flvalue = timelessthan (v1.data.datevalue, v2.data.datevalue);
					
			break;
		
		case singlevaluetype:
			(*vreturned).data.flvalue = v1.data.singlevalue < v2.data.singlevalue;
			
			break;
		
		case doublevaluetype:
			(*vreturned).data.flvalue = **v1.data.doublevalue < **v2.data.doublevalue;
			
			break;
		
		case stringvaluetype:
			(*vreturned).data.flvalue = comparehandles (v1.data.stringvalue, v2.data.stringvalue) == -1;
			
			break;
		
		default:
			langerror (comparisonnotpossibleerror);
			
			fl = false; /*operation is not defined*/
			
			break;
		} /*switch*/
	
	disposevalues (&v1, &v2);
		
	return (fl);
	} /*LTvalue*/


boolean GEvalue (tyvaluerecord v1, tyvaluerecord v2, tyvaluerecord *vreturned) {

	if (!LTvalue (v1, v2, vreturned)) 
		return (false);
	
	(*vreturned).data.flvalue = !(*vreturned).data.flvalue;
	
	return (true);
	} /*GEvalue*/


static boolean stringcomparevalue (tyvaluerecord *v1, tyvaluerecord *v2, tytreetype op, tyvaluerecord *vreturned) {

	register Handle h1, h2;
	long ixstart = 0;
	long ixlimit = longinfinity;
	long result;
	
	initvalue (vreturned, booleanvaluetype);
	
	if (!coercetostring (v1))
		return (false);
	
	if (!coercetostring (v2))
		return (false);
	
	h1 = (*v1).data.stringvalue;
	
	h2 = (*v2).data.stringvalue;
	
	if (op == beginswithop) {
		ixlimit = gethandlesize (h2);
		}
	else if (op == endswithop) {
		ixstart = gethandlesize (h1) - gethandlesize (h2);
		}
	
	result = searchhandle (h1, h2, ixstart, ixlimit);
	
	(*vreturned).data.flvalue = result >= 0;
	
	return (true);
	} /*stringcomparevalue*/


static boolean specialcomparisonvalue (register tyvaluerecord *v1, register tyvaluerecord *v2, tytreetype op, tyvaluerecord *vreturned) {
	
	switch ((*v1).valuetype) {
		
		case listvaluetype:
		case recordvaluetype:
			if (!coercetypes (v1, v2))
				return (false);
			
			return (listcomparevalue (v1, v2, op, vreturned));
		
		default:
			return (stringcomparevalue (v1, v2, op, vreturned));
		}
	} /*specialcomparisonvalue*/


boolean beginswithvalue (tyvaluerecord v1, tyvaluerecord v2, tyvaluerecord *vreturned) {
	
	return (specialcomparisonvalue (&v1, &v2, beginswithop, vreturned));
	} /*beginswithvalue*/


boolean endswithvalue (tyvaluerecord v1, tyvaluerecord v2, tyvaluerecord *vreturned) {

	return (specialcomparisonvalue (&v1, &v2, endswithop, vreturned));
	} /*endswithvalue*/


boolean containsvalue (tyvaluerecord v1, tyvaluerecord v2, tyvaluerecord *vreturned) {

	return (specialcomparisonvalue (&v1, &v2, containsop, vreturned));
	} /*containsvalue*/


boolean andandvalue (tyvaluerecord v1, hdltreenode hp2, tyvaluerecord *vreturned) {
	
	/*
	9/4/91 dmb: take code tree for second parameter to allow short-circuit 
	evaluation
	
	6.2b15 AR: Call coercetoboolean directly instead of the now defunct truevalue
	*/
	
	tyvaluerecord v2;
	boolean fl;
	
	if (!coercetoboolean (&v1))
		return (false);
	
	if (!v1.data.flvalue) /*no need to check param 2*/
		fl = false;
	else {
		
		if (!evaluatetree (hp2, &v2))
			return (false);
		
		if (!coercetoboolean (&v2))
			return (false);
		
		fl = v2.data.flvalue;
		}
	
	return (setbooleanvalue (fl, vreturned));
	} /*andandvalue*/


boolean ororvalue (tyvaluerecord v1, hdltreenode hp2, tyvaluerecord *vreturned) {
	
	/*
	9/4/91 dmb: take code tree to second parameter to allow short-circuit 
	evaluation
	
	6.2b15 AR: Call coercetoboolean directly instead of the now defunct truevalue
	*/
	
	tyvaluerecord v2;
	boolean fl;
	
	if (!coercetoboolean (&v1))
		return (false);

	if (v1.data.flvalue) /*no need to check param 2*/
		fl = true;	
	else {
		
		if (!evaluatetree (hp2, &v2))
			return (false);
		
		if (!coercetoboolean (&v2))
			return (false);

		fl = v2.data.flvalue;
		}
	
	return (setbooleanvalue (fl, vreturned));
	} /*ororvalue*/
	
	
boolean unaryminusvalue (tyvaluerecord v1, tyvaluerecord *vreturned) {
	
	register boolean fl = true;
	
	initvalue (vreturned, v1.valuetype);
	
	switch (v1.valuetype) {
		
		case booleanvaluetype:
			(*vreturned).data.flvalue = -v1.data.flvalue;
			
			break;
			
		case charvaluetype:
			(*vreturned).data.chvalue = -v1.data.chvalue;
			
			break;
			
		case intvaluetype:
			(*vreturned).data.intvalue = -v1.data.intvalue;
			
			break;
			
		case longvaluetype:
			(*vreturned).data.longvalue = -v1.data.longvalue;
			
			break;
		case fixedvaluetype:
			(*vreturned).data.fixedvalue = -v1.data.fixedvalue;
		case singlevaluetype:
			(*vreturned).data.singlevalue = -v1.data.singlevalue;
			
			break;
		
		case doublevaluetype:
			fl = setdoublevalue (-**v1.data.doublevalue, vreturned);
			
			break;
		
		default:
			langerror (unaryminusnotpossibleerror);
			
			fl = false; /*unary minus is not defined*/
			
			break;
		} /*switch*/
	
	disposevalues (&v1, nil);
	
	return (fl);
	} /*unaryminusvalue*/


boolean notvalue (tyvaluerecord v1, tyvaluerecord *vreturned) {
	
	/*
	7/10/91 dmb: rewrote to coerce to boolean, rather than applying the 
	boolean not operation on each oringal value type.  less code, and 
	more eliable.
	*/
	
	if (!coercetoboolean (&v1))
		return (false);
	
	return (setbooleanvalue (!v1.data.flvalue, vreturned));
	} /*notvalue*/


static boolean sizefunc (hdltreenode hparam1, tyvaluerecord *vreturned) {
	
	tyvaluerecord v;
	long size;
	
	flnextparamislast = true;
	
	if (!getreadonlyparamvalue (hparam1, 1, &v))
		return (false);
	
	if (!langgetvalsize (v, &size)) {
		
		langerror (cantsizeerror); 
			
		return (false);
		}
	
	return (setlongvalue (size, vreturned));
	} /*sizefunc*/


static boolean typefunc (hdltreenode hparam1, tyvaluerecord *vreturned) {
	
	tyvaluerecord v;
	
	flnextparamislast = true;
	
	if (!getreadonlyparamvalue (hparam1, 1, &v))
		return (false);
	
	if (v.valuetype == externalvaluetype)
		v.valuetype = (tyvaluetype) (outlinevaluetype + langexternalgettype (v));
	
	setostypevalue (langgettypeid (v.valuetype), vreturned);
	
	return (true);
	} /*typefunc*/


static boolean namefunc (hdltreenode hparam1, tyvaluerecord *vreturned) {
	
	/*
	2.1b2 dmb: added support for getting name of record items
	
	3.0.2b3: don't try to retrieve array value when we're indexing by name; 
	just use name in that case
	
	5.0.2b8 dmb: if the named item exists, extract its true name to fix case.
	*/
	
	register hdltreenode hp1 = hparam1;
	hdlhashtable htable;
	bigstring bsvarname;
	tyvaluerecord valarray;
	tyvaluerecord valindex;
	tyvaluerecord valitem;
	hdlhashnode hn;
	bigstring bsname;
	
	if (!langcheckparamcount (hp1, 1))
		return (false);
	
	disablelangerror (); /*any error will result in a null return*/
	
	if (langgetdotparams (hp1, &htable, bsname)) { // 5.0.2: make sure name has original case if item exists
		
		if ((htable != nil) && hashtablelookupnode (htable, bsname, &hn))
			gethashkey (hn, bsname);
		}
	else { /*try something else*/
		
		setemptystring (bsname); /*default return, override dotparam remnant*/
		
		if ((**hp1).nodetype == arrayop) { /*may be array into record*/
			
			#if oldarrays
				
				if (!getarrayandindex (hp1, &htable, bsvarname, &valarray, &valindex))
					goto exit;
			
			#else
			
				tyarraystack arraystack = {0};
				short parent;
				
				if (!parsearrayreference (hp1, &arraystack, &htable, bsvarname, nil))
					goto exit;
				
				parent = arraystack.topstack - 1;
				
				valarray = arraystack.element [parent].varray;
				
				valindex = arraystack.element [parent].vindex;

			#endif
			
			if (valarray.valuetype != recordvaluetype)
				goto exit;
			
			#ifdef oplanglists
				switch (valindex.valuetype) {
					
					case ostypevaluetype:
						ostypetostring (valindex.data.ostypevalue, bsname);
						break;
					
					case stringvaluetype:
						pullstringvalue (&valindex, bsname);
						break;
					
					default:
						if (!coercetolong (&valindex))
							goto exit;
						
						if (!langgetlistitem (&valarray, valindex.data.longvalue, bsname, &valitem))
							goto exit;
						
						disposevaluerecord (valitem, true);
						
						break;
					}
			#else
				if (valindex.valuetype == ostypevaluetype)
					key = valindex.data.ostypevalue;
				
				else {
					
					if (!coercetolong (&valindex))
						goto exit;
					
					if (!langgetlistitem (&valarray, valindex.data.longvalue, &key, &valitem))
						goto exit;
					
					disposevaluerecord (valitem, true);
					}
				
				ostypetostring (key, bsname);
			#endif
			}
		}
	
	exit:
	
	enablelangerror ();
	
	return (setstringvalue (bsname, vreturned));
	} /*namefunc*/


static boolean parentfunc (hdltreenode hparam1, tyvaluerecord *vreturned) {
	
	/*
	4.0b7 4/26/96 dmb: new verb
	
	5.0d19 dmb: special-case file window table as nil parent

	5.0b7 dmb: deal with items in locally-created table
	
	5.0.2 dmb: we now call findinparenttable, which encorporated our patentsearch 
	functionality. the table's parenthashtable is maintained for better performance
	*/
	
	register hdltreenode hp1 = hparam1;
	hdlhashtable htable = nil;
	bigstring bsname;
	boolean flnoparent;
	
	if (!langcheckparamcount (hp1, 1))
		return (false);
	
	disablelangerror (); /*any error will result in a null return*/
	
	if (!langgetdotparams (hp1, &htable, bsname)) { /*need to try something else*/
		
		if ((**hp1).nodetype == arrayop) { /*may be array into record*/
		
			parsearrayreference (hp1, nil, &htable, bsname, nil);
			}
		}
	
	enablelangerror ();
	
	if (htable == nil) {
	
		if (!equalstrings (bsname, nameroottable))	// leave it nil if we're at root
			langsearchpathlookup (bsname, &htable);
		}
	
	setemptystring (bsname);
	
	if (htable == nil || htable == filewindowtable)
		flnoparent = true;
	else
		flnoparent = !findinparenttable (htable, &htable, bsname); // 5.0.2b13 dmb;
	
	/*
	else {
		if ((**htable).fllocaltable) {
			
			flnoparent = true;
			
			for (h = currenthashtable; h != nil; h = (**h).prevhashtable) {
				
				if (parentsearch (h, htable, true, &htable, bsname)) {
					
					flnoparent = false;
					
					break;
					}
				}
			}
		else
			flnoparent = !parentsearch (roottable, htable, true, &htable, bsname);
		}
	*/
	
	if (flnoparent)
		return (setstringvalue (bsname, vreturned));
	else
		return (setaddressvalue (htable, bsname, vreturned));
	} /*parentfunc*/


static boolean indexfunc (hdltreenode hparam1, tyvaluerecord *vreturned) {
	
	/*
	6.1d7 AR: Started implementation of indexOf verb.
	*/
	
	register hdltreenode hp1 = hparam1;
	hdlhashtable htemp, htable = nil;
	bigstring bsname, bstemp;
	register hdlhashnode nomad;
	long ix = 0;
	
	if (!langcheckparamcount (hp1, 1))
		return (false);
	
	disablelangerror ();

	if (!langgetdotparams (hp1, &htable, bsname))
		goto exit;

	if (htable == nil && !equalstrings (bsname, nameroottable))	// leave it nil if we're at root
		langsearchpathlookup (bsname, &htable);
	
	if (htable == nil || htable == filewindowtable || !findinparenttable (htable, &htemp, bstemp))
		goto exit;

	nomad = (**htable).hfirstsort;

	while (nomad != nil) {

		ix++;
		
		if (equalidentifiers (bsname, (**nomad).hashkey)) /*search is over*/
			goto exit;
		
		nomad = (**nomad).sortedlink;
		} /*while*/

	ix = 0; /*not found*/

exit:
			
	enablelangerror ();

	return (setlongvalue (ix, vreturned));
	} /*indexfunc*/


#define infrontierprocess() (iscurrentapplication (langipcself))

static boolean tablefindnode (hdlhashtable intable, hdlhashnode fornode, hdlhashtable *foundintable, bigstring foundname) {

	register hdlhashtable ht = intable;
	register hdlhashnode x;
	register short i;
	tyvaluerecord val;
	register hdlexternalvariable hv;
	
	for (i = 0; i < ctbuckets; i++) {
		
		x = (**ht).hashbucket [i];
		
		while (x != nil) { /*chain through the hash list*/
			
			if (x == fornode) { /*bravo!  we found it...*/
				
				*foundintable = ht;
				
				gethashkey (x, foundname);
				
				return (true);
				}
			
			val = (**x).val;
			
			if (val.valuetype != externalvaluetype) 
				goto nextx;
				
			hv = (hdlexternalvariable) val.data.externalvalue;
			
			if ((**hv).id != idtableprocessor)
				goto nextx;
			
			if (tablefindnode ((hdlhashtable) (**hv).variabledata, fornode, foundintable, foundname))
				return (true); /*unwind recursion*/
				
			nextx:
			
			x = (**x).hashlink; /*advance to next node in chain*/
			} /*while*/
		} /*for*/
		
	return (false);
	} /*tablefindnode*/


static boolean kernelerrorroutine (long scripterrorrefcon, long lnum, short charnum, hdlhashtable *htable, bigstring bsname) {
#pragma unused(lnum, charnum)
	/*
	display an error for a system script -- a handler, agent or startup script.
	
	called back from langerror.c.  we receive a handle to the table node that
	made the error, and we zoom out a window that displays the attached script,
	in text mode, with the character cursor pointing to line number lnum at
	offset charnum -- the exact spot where the error occured, we hope...
	
	2/4/91 dmb: allow hdlheadrecords to end up here too
	
	4.0b8 dmb: if caller provides non-null htable, don't show error, just return 
	its location. bsname better be non-null too!

	4/7/97 dmb: handle standalone scripts
	*/
	
	register hdlhashnode h = (hdlhashnode) scripterrorrefcon;
	
	if (h == nil) /*defensive driving*/
		return (false);
	
	if (htable != nil) { /*caller wants table, name*/
		
		return (tablefindnode (efptable, h, htable, bsname));
		}
	
	return (false);
	} /*kernelerrorroutine*/


boolean kernelfunctionvalue (hdlhashtable htable, bigstring bsverb, hdltreenode hparam1, tyvaluerecord *vreturned) {
	
	/*
	9/23/91 dmb: make sure langerrors don't go unnoticed
	
	2.1b1(?) dmb: this is the bottleneck where we potentially execute the 
	verb via an appleevent if necessary. if the kernel table says its verbs 
	require a window, we call the value routine with the actual token and 
	nil parameters. if it returns true, that tokens needs to be interpreted 
	while Frontier is the active process.
	*/
	
	register boolean fl;
	register hdlhashtable ht = htable;
	langvaluecallback valueroutine;
	bigstring bserror;
	hdlhashnode hnode;
	tyvaluerecord val;
	boolean flprofiling = currentprocess && (**currentprocess).flprofiling;
	
	valueroutine = (**ht).valueroutine;
	
	assert (valueroutine != nil); /*this was checked at compile time in pushkernelcall*/
	
	fl = hashtablelookupnode (ht, bsverb, &hnode); /*get the token value*/
	
	if (fl)
		val = (**hnode).val;
	
	if (!valueroutine || !fl || (val.valuetype != tokenvaluetype)) { /*should never happen; preflighted at compile time*/
		
		langparamerror (notefperror, bsverb);
		
		return (false);
		}
	
#if isFrontier && (MACVERSION || RABTEMPOUT)
	if ((**ht).flverbsrequirewindow && !infrontierprocess ()) { /*verb may need to be run in frontier process*/
		
		if ((*valueroutine) (val.data.tokenvalue, nil, nil, nil)) /*yup*/
			return (langipckernelfunction (ht, bsverb, hparam1, vreturned));
		}
#endif	
	setemptystring (bserror);
	
	if (flprofiling) {
		
		if (!langpusherrorcallback (kernelerrorroutine, (long) hnode))
			return (false);
		}
	
	fl = (*valueroutine) (val.data.tokenvalue, hparam1, vreturned, bserror);
	
	if (!fl && !isemptystring (bserror)) {
		
		setparseparams (bsverb, nil, nil, nil); /*insert verb name if called for*/
		
		parseparamstring (bserror);
		
		langerrormessage (bserror);
		}
	
	if (flprofiling)
		langpoperrorcallback ();
	
	return (fl && !fllangerror);
	} /*kernelfunctionvalue*/


static boolean kernelcall (hdltreenode hcode, hdltreenode hparam1, tyvaluerecord *vreturned) {
	
	register hdltreenode h = hcode;
	hdlhashtable htable;
	bigstring bsverb;
	
	h = (**h).param1;
	
	assert ((**h).nodetype == kernelop);
	
	getaddressvalue ((**h).nodeval, &htable, bsverb);
	
	return (kernelfunctionvalue (htable, bsverb, hparam1, vreturned));
	} /*kernelcall*/


static boolean langaddlocalsymbols (hdltreenode hnamelist, hdlhashtable htable, short *ctuninitialized) {
	
	/*
	add the names in hnamelist to the indicated table.  
	
	2.1b2 dmb: this code could be generalized to be shared with langaddlocals
	but error reporting is slightly different, and we're not calling the 
	debugger. the main issue w/error reporting is that the source code of the 
	handler itself is not pushed yet.
	
	4.1b4 dmb: added fllangexternalvalueprotect flag to disable protection
	
	4.1b5 dmb: push root table and choin to it so that addresses are resolved properly
	*/
	
	register hdltreenode nomad = hnamelist;
	tyvaluerecord val;
	bigstring bs;
	boolean fl = false;
	
	*ctuninitialized = 0;
	
	pushhashtable (roottable);	/*4.1b5 dmb: need root table to resolve addresses properly*/
	
	chainhashtable (htable);	/*we chain to root to isolate evaluation from caller's context*/
	
	while (true) { /*step through name list, inserting each into symbol table*/
		
		if (nomad == nil) { /*reached the end of the names list*/
			
			fl = true;
			
			break;
			}
		
		if ((**nomad).nodetype == assignlocalop) {
			
			if (!evaluateparam ((**nomad).param2, &val))
				break;
			
			if (fllangexternalvalueprotect && val.valuetype == externalvaluetype) {  /*4.1b4 dmb*/
				
				langbadexternaloperror (externalassignerror, val);
				
				break;
				}
			
			if (!langgetidentifier ((**nomad).param1, bs))
				break;
			}
		else {
			
			initvalue (&val, novaluetype);
			
			val.data.longvalue = -1; /*make this discernable from a nil assignment*/
			
			if (!langgetidentifier (nomad, bs))
				break;
			
			++*ctuninitialized;
			}
		
		val.flformalval = true; /*value isn't from actual parameter*/
		
		if (!hashassign (bs, val)) /*error creating new symbol*/
			break;
		
		exemptfromtmpstack (&val); /*it's been successfully added to local table*/
		
		cleartmpstack (); /*dealloc all outstanding temporary values*/	
		
		nomad = (**nomad).link; /*advance to next name in list*/
		} /*while*/
	
	unchainhashtable ();	/*4.1b5 dmb: now we're chained to root; see above*/
	
	pophashtable ();
	
	return (fl);
	} /*langaddlocalsymbols*/


static boolean langaddfuncparams (hdltreenode hformal, hdltreenode hactual, hdlhashtable htable) {
	
	/*
	2.1b2 dmb: broke out of langfunccall and updated to support named 
	parameters. syntactically, a parameter list consists of zero or more 
	unnamed parameters followed by zero or more named parameters; they 
	can't be intermixed.
	
	as discused in langfunccall's header comment, htable must not be pushed 
	while evaluating parameter values.
	*/
	
	register hdltreenode hf = hformal;
	register hdltreenode ha = hactual;
	bigstring bsname;
	tyvaluerecord val;
	tyvaluerecord vexists;
	boolean fl = false;
	short ctunassigned = 0;
	hdlhashnode hnode;
	
	while (true) {
		
		if (ha == nil) { /*ran out of actual parameters*/
		
			if (hf != nil) { /*there are still formal parameters waiting*/
				
				if (!langaddlocalsymbols (hf, htable, &ctunassigned))
					break;
				}
			
			if (ctunassigned == 0) /*everything worked out nicely*/
				fl = true;
			else
				langparamerror (notenoughparameterserror, bsfunctionname);
			
			break;
			}
		
		if ((**ha).nodetype == fieldop) { /*all following params will also be fields*/
			
			if (hf != nil) { /*must be first fieldop*/
				
				if (!langaddlocalsymbols (hf, htable, &ctunassigned))
					break;
				
				hf = nil; /*all remaining formal parameters have been added*/
				}
			
			if (!langgetidentifier ((**ha).param1, bsname))
				break;
			
			if (!hashtablelookup (htable, bsname, &vexists, &hnode)) { /*no such parameter*/
				
				langseterrorline (ha);
				
				lang2paramerror (unknownparametererror, bsfunctionname, bsname);
				
				break;
				}
			
			if (!vexists.flformalval) { /*already been assigned an actual value*/
			
				langseterrorline (ha);
				
				lang2paramerror (duplicateparametererror, bsfunctionname, bsname);
				
				break;
				}
			
			if ((vexists.valuetype == novaluetype) && (vexists.data.longvalue == -1))
				--ctunassigned;
			
			if (!evaluateparam ((**ha).param2, &val))
				break;
			}
		else {
			
			if (hf == nil) { /*ran out of parameters in the declaration*/
				
				langparamerror (toomanyparameterserror, bsfunctionname);
				
				break;
				}
			
			if ((**hf).nodetype == assignlocalop) {
				
				if (!langgetidentifier ((**hf).param1, bsname))
					break;
				}
			else {
				
				if (!langgetidentifier (hf, bsname))
					break;
				}
			
			if (!evaluateparam (ha, &val))
				break;
			
			hf = (**hf).link; /*advance to the next parameter name*/
			}
		
		if (fllangexternalvalueprotect && val.valuetype == externalvaluetype) {  /*4.1b4 dmb*/
			
			langbadexternaloperror (externalgetvalueerror, val);
			
			break;
			}
		
		val.flformalval = false; /*make sure it's clear*/
		
		if (!hashtableassign (htable, bsname, val)) /*assignment goes into the local table*/
			break;
		
		exemptfromtmpstack (&val); /*make sure it doesn't get released automatically*/
		
		ha = (**ha).link; /*advance to the next parameter value*/
		} /*while*/
	
	return (fl);
	} /*langaddfuncparams*/


static boolean binaryfunctionvalue (hdlhashnode hnode, bigstring bsname, hdltreenode hparam1, tyvaluerecord *vreturned) {
	
	/*
	2.1b3 dmb: the OSA subroutine calling support strongly motivated making 
	binary values directly callable in general.  here's the first cut.
	*/
	
	tyvaluerecord v = (**hnode).val;
	
	switch (getbinarytypeid (v.data.binaryvalue)) {
		
		case 'XCMD':
		case 'XFCN':
			break;
		
		case 'UCMD':
			break;
		
	#ifdef flcomponent
		default:
			return (evaluateosascript (&v, hparam1, bsname, vreturned));
	#endif
		}
	
	langparamerror (notfunctionerror, bsname);
	
	return (false);
	} /*binaryfunctionvalue*/


boolean langfunctioncall (hdltreenode hcallernode, hdlhashtable htable, hdlhashnode hnode, bigstring bsname, hdltreenode hcode, hdltreenode hparam1, tyvaluerecord *vreturned) {
	
	/*
	run the code pointed to by hcode.  hparam1 points at the first parameter to
	the handler.
	
	a rather lengthy comment describing the format of a module:
	
	suppose this is the source for the module:
		
		on aaa (bbb)
			msg (bbb)
			
	this is what the code tree looks like:
	
		module op
			module op
				tree for "msg (bbb)"
				proc op
					id
						"aaa"
					id
						"bbb"
						
	7/9/90 DW: the doubling up of moduleops only occurs for handlers that appear
	inside their own scripts, as implemented in the global handler table.  for
	a local handler, there is only one level of moduleops.
	
	2/15/91 dmb: parameter values must be evaluated in the original table so 
	that undeclared variables passed by address are created in the right place.
	
	9/23/91 dmb: fixed problem reporting errors when evaluating parameters, due to 
	the fact that too much of our new context was already set up.  the best thing 
	to do is to avoid pushing a new stack frame and the new source code until after 
	all parameters have been evaluated.  to permit this, magictable handling is now 
	at a deeper level -- in newhashtable and landpushlocalchain, so we can allocate 
	a locals table here while deferring the pushing of a local chain until later 
	(in evaluatelist).
	
	also, added hcallernode parameter so that a special debugger call can be made 
	after all parameters have been evaluated.  this way, if the user steps into a 
	function call in the parameter list, he can step out to the original call 
	instead of getting lost.
	
	xxx 9/24/92 dmb: manage ctlocks & fldisposewhenunlocked flags in code's hash node
	
	4.1b3 dmb: added call to new langseterrorcallbackline for stack tracing (on error)
	*/
	
	register hdltreenode hname;
	register boolean fl;
	hdlhashtable hlocaltable;
#if (version5orgreater && defined (flcomponent))
	tyvaluerecord osacode;
#endif
	
		
		if (hcode == nil) { /*can only be a kernel call -- or an error*/
			
			if ((**hnode).val.valuetype == binaryvaluetype)
				return (binaryfunctionvalue (hnode, bsname, hparam1, vreturned));
			
			return (kernelfunctionvalue (htable, bsname, hparam1, vreturned));
			}
		
		if ((**(**hcode).param1).nodetype == kernelop)
			return (kernelcall (hcode, hparam1, vreturned));
		
		#ifdef flcomponent
			
			if (isosascriptnode (hcode, &osacode)) {
				
				if (!langpushsourcecode (htable, hnode, bsname))
					return (false);
				
				// 5.0a18 dmb: this probably isn't adequate for osa execution
				langsetthisvalue (currenthashtable, htable, bsname);
				
				fl = evaluateosascript (&osacode, hparam1, bsname, vreturned);
				
				langpopsourcecode ();
				
				return (fl);
				}
			
		#endif // flcomponent
		
	
	bundle { /*safely navigate to get the name of the 1st formal param*/
		
		register hdltreenode hp2 = (**hcode).param2;
		
		if (hp2 == nil) /*no formal params*/
			hname = nil;
		else
			hname = (**hp2).param2; /*point at the name of the 1st param*/
		}
	
	if (!newhashtable (&hlocaltable)) /*new table for the function when it runs*/
		return (false);
	
	(**hlocaltable).fllocaltable = true; // 5.1.4: set now so pre-assignments know scope
	
	
	langseterrorcallbackline ();
	
	
	fl = langaddfuncparams (hname, hparam1, hlocaltable);
	
	if (fl && hcallernode)
		fl = langdebuggercall (hcallernode); /*user killed the script*/
	
	if (fl)
		fl = langpushsourcecode (htable, hnode, bsname);
	
	if (fl && !(**htable).fllocaltable)
		fl = langsetthisvalue (hlocaltable, htable, bsname);
	
	if (!fl) {
		
		disposehashtable (hlocaltable, false);
		
		return (false);
		}
	
	hmagictable = hlocaltable; /*evaluatelist uses this as its local symbol table*/
	
	/*
	++(**hn).ctlocks;
	*/
	
	fl = evaluatelist ((**hcode).param1, vreturned);
	
	flreturn = false; /*if he return-d to get out, we've gone far enough*/
	
	langpopsourcecode ();
	
	/*
	if (--(**hn).ctlocks == 0) {
		
		if ((**hn).fldisposewhenunlocked)
			disposehashnode (hn, ?, ?);
		}
	*/
	
	return (fl);
	} /*langfunctioncall*/



static boolean isentrypoint (hdltreenode hcode, bigstring bsname, bigstring bsprocname) {
	
	/*
	5.0b10 dmb: need to return any found proc name in bsidentifier, for
	error reporting
	*/

	if ((**hcode).nodetype == moduleop) { // it's a proc, or a wrapper
		
		hdltreenode hp2 = (**hcode).param2;
		
		if (hp2 && ((**hp2).nodetype == procop)) { // it's a proc, an eligable entrypoint
			
			if (bsname == nil) // no name specified, no match needed
				return (true);
			
			if (langgetidentifier ((**hp2).param1, bsprocname)) {
				
				if (equalidentifiers (bsprocname, bsname))
					return (true);
				}
			}
		}

	return (false);
	} /*isentrypoint*/


static hdltreenode langgetentrypoint (hdltreenode hcode, bigstring bsname, hdlhashtable htable, hdlhashnode hnode) {

	/*
	skip over extra level of modularity that exists in external 
	handle calls.
	
	see comment in langfunctioncall
	
	5/28/91 dmb: only skip level when the module name is the same as the 
	indicated external name (the name of the script in the database).  if no 
	name is given, no match is required
	
	11/25/91 dmb: use langgetidentifier to handle entrypoints that are 
	bracketed expressions.
	
	2/13/92 dmb: treat entrypoint name mismatch as an error if that's all
	there is in the module.  added htable/hnode parameters so that errors can be 
	reported nicely
	
	3/19/97 dmb: if there is more than one top-level module, look at them 
	all for a match
	*/
	
	hdltreenode hp1;
	hdltreenode hp2 = nil;
	boolean foundbody = false;
	bigstring bsidentifier;
	
	if (isentrypoint (hcode, bsname, bsidentifier))
		return (hcode);

	for (hp1 = (**hcode).param1; hp1 != nil; hp1 = (**hp1).link) {
	
		switch ((**hp1).nodetype) {
			
			case noop:
				break;
			
			case moduleop:
				if (hp2)
					foundbody = true; // multiple modules, treat as having body (for error reporting)
				
				if (isentrypoint (hp1, bsname, bsidentifier))
					return (hp1);
				
				hp2 = (**hp1).param2; // remember, for error reporting
				
				break;
			
			default:
				foundbody = true;
				
				break;
			}
		}
	
	if (!foundbody && hp2) { // only found one or more modules with mis-matched hnames
		
		langpushsourcecode (htable, hnode, bsfunctionname); /*point user at bad script...*/
		
		langseterrorline ((**hp2).param1); /*...rather than the call to it*/
		
		lang2paramerror (badentrypointnameerror, bsname, bsidentifier);
		
		langpopsourcecode ();
		
		return (nil);
		}
	
	return (hcode);
	} /*langgetentrypoint*/



#if 0

boolean langgetlocalhandlercode (bigstring bs, hdltreenode *hcode) {
	
	hdlhashtable htable;
	tyvaluerecord val;
	
	if (!langgetsymbolval (bs, &val)) /*not found in local chain*/
		return (false);
	
	if (val.valuetype != codevaluetype) /*not a local handler*/
		return (false);
	
	*hcode = val.data.codevalue;
	
	return (true);
	} /*langgetlocalhandlercode*/

#endif


boolean langgetnodecode (hdlhashtable ht, bigstring bs, hdlhashnode hnode, hdltreenode *hcode) {

	tyvaluerecord val = (**hnode).val;
	
	switch (val.valuetype) {
		
		case externalvaluetype: /*might be a script*/
			
			if (!langexternalvaltocode (val, hcode)) // error; not a code node
				return (false);
			
			if (*hcode == nil) { /*it needs to be compiled*/
				
				if (!(*langcallbacks.scriptcompilecallback) (hnode, hcode)) /*error compiling the script*/
					return (false);
				
				langseterrorline (herrornode);	/*4.1b4 dmb: compiling screws up the line/char globals*/
				}
			
			break; /*get entry point*/
			
		case codevaluetype: /*probably a local handler*/
			
			*hcode = val.data.codevalue; /*easy case*/
			
			break; /*get entry point*/
		
		case tokenvaluetype: /*probably a kernel call*/
			
			if ((**ht).valueroutine == nil) // error; not a kernel table
				return (false);
			
			*hcode = nil; /*special case*/
			
			return (true); /*we're done*/
		
		case binaryvaluetype:
			
			*hcode = nil; /*special case*/
			
			return (true); /*we're done*/
		
		default: /*symbol is defined, but it isn't code or a script*/
			return (false);
		}
	
	*hcode = langgetentrypoint (*hcode, bs, ht, hnode);
	
	return (true);
	} /*langgetnodecode*/


static boolean langgethandlercode (hdlhashtable intable, hdltreenode hnamenode, hdltreenode *hcode, hdlhashtable *htable, hdlhashnode *hnode) {
	
	/*
	hnamenode points to the name of the handler to be executed.  it could be a 
	dotop -- in which case it's the name-path to a handler.
	
	if it names a real node, and it's of type script, we return a handle to the
	code to be executed.
	
	1/7/90 dmb: we no longer want to generate any errors if we can't find 
	the specified script, so we can be called again to look somewhere else.  
	however, once we find the script, we should call langerror if an error 
	is encountered, so our caller will know not to try again.
	
	10/4/91 dmb: last-minute quick-fix for an n-squared performance problem. 
	langhandlercall does something the language doesn't usually do: it will 
	resolve a reference to an object whose table has the same name as a table 
	ahead of it in the search path.  this allows the user to add to the builtin 
	tables by adding their own version locally; verbs will be found in whichever 
	table they're in.  However, the behavior isn't supported language-wide, and 
	it would be messy to do so. (because in general we need for langgetdotparams 
	resolve non-existant addresses, and quickly.)  Anyway, since this routine is 
	called from a searchpathvisit, we want to inhibit langgetdotparams from doing 
	its searchpathvisit -- except for the first time, when we're called in the 
	current scope.  the fllocaldotparamsonly is set for this purpose.
	
	4/8/93 dmb: allow for codevalue case that isn't a local handler
	
	2.1b9 dmb: scan for entry point into local handlers; they may have 
	been created by script.getCode
	
	3/19/97 dmb: moved code extracting code into new langgetnodecode.
	*/
	
	register hdlhashtable ht;
	bigstring bs;
	register boolean fl;
	
	setemptystring (bs);
	
	disablelangerror (); /*no dialog if an error is encountered*/
	
	if (intable != currenthashtable) /*not being called for default scope*/
		fllocaldotparamsonly = true;
	
	pushhashtable (intable);
	
	fl = langgetdotparams (hnamenode, htable, bs);
	
	pophashtable ();
	
	fllocaldotparamsonly = false;
	
	if (!isemptystring (bs)) {
		
		if (fl || isemptystring (bsfunctionname)) /*copy into global even on error*/
			copystring (bs, bsfunctionname);
		}
	
	enablelangerror ();
	
	if (!fl) 
		return (false);
	
	ht = *htable; /*move into register*/
	
	if (ht == nil) { /*no table specified*/
		
		pushhashtable (intable);
		
		fl = langfindsymbol (bs, htable, hnode);
		
		pophashtable ();
		
		if (!fl) /*not found in specified chain -- no error*/
			return (false);
		
		ht = *htable; /*update register*/
		}
	else {
		
		pushhashtable (ht); /*it was a dotted name, look in the indicated table*/
		
		fl = hashlookupnode (bs, hnode);
		
		pophashtable ();
		
		if (!fl) { /*not found in specified table -- this is not an error (yet)*/
			
			/*
			langparamerror (unknownfunctionerror, bs);
			*/
			
			return (false);
			}
		}
	
	/*we've found the table entry, now let's try to get some code out of it*/
	
	if (!langgetnodecode (ht, bs, *hnode, hcode)) {
		
		langparamerror (notfunctionerror, bs);
		
		return (false);
		}
	
	return (true);
	} /*langgethandlercode*/


static struct { /*handler info for visit routine*/
	
	hdltreenode htree; /*code tree specifying handler code node*/
	
	hdlhashnode hnode; /*hash node containing code*/
	
	hdltreenode hcode; /*handler code itself*/
	} handlercode;


static boolean langgethandlervisit (hdlhashtable intable, bigstring bs, hdlhashtable *htable) {
#pragma unused(bs)
	return (langgethandlercode (intable, handlercode.htree, &handlercode.hcode, htable, &handlercode.hnode));
	} /*langgethandlervisit*/


boolean langhandlercall (hdltreenode htree, hdltreenode hparam1, tyvaluerecord *vreturned) {
	
	/*
	first try to find the handler in the local chain of symbol tables.
	
	then look in the all tables in the search path.
	
	then look in the kernel table.
	
	then give up.
	
	1/7/90: see comment in langgethandlercode.  no longer disablelangerror around 
	calls to it.
	
	10/3/91 dmb: support having the kernel table in the paths table; don't 
	assume found code is non-nil
	
	2/13/92 dmb: check fllangerror after getting handler code
	
	2.1b5 dmb: need to push source when evaluating an osa script
	
	3.0a dmb: before generating an error for an unknown function, give landipc 
	a chance to handle it using the subroutine event mechanism.

	5.0b11 dmb: added another big band-aid for bracked expressions. they can't work
	with the existing logic, because the bracketed expression needs full scoping, but
	then calling the result of the expression also needs full scoping. so I added a 
	special case, after all else has failed. it couldprobably just as well go first.

	really, this entire routine,along of gethandlercode, should be rewritten to just
	evaluate htree, then call it. whatever is needed to maintain its current ability
	to find a second table satifying the dotparams should be added to the general 
	search path mechanism.
	*/
	
	hdltreenode hcode;
	hdlhashtable htable;
	hdlhashnode hnode;
	// boolean fl;
	// tyvaluerecord osacode;
	
	setemptystring (bsfunctionname); /*must initialize for langgethandlercode error logic*/
	
	if (langgethandlercode (currenthashtable, htree, &hcode, &htable, &hnode)) /*found it in root structure*/
		goto runhandler;
	
	if (fllangerror) /*found it, but error getting code*/
		return (false);
	
	handlercode.htree = htree;
	
	if (langsearchpathvisit (&langgethandlervisit, nil, &htable)) {
		
		hcode = handlercode.hcode;
		
		hnode = handlercode.hnode;
		
		goto runhandler;
		}
	
	if (langgethandlercode (efptable, htree, &hcode, &htable, &hnode)) {
		
		assert (hcode == nil); /*see special case in gethandlercode*/
		
		goto runhandler;
		}
	
		if ((**htree).nodetype == dotop) { // 8.0.4 dmb: handle remote functions for SCNS
			
			if (langisremotefunction (htree))
				return (langremotefunctioncall (htree, hparam1, vreturned));
			}

		if ((**htree).nodetype == bracketop) { // 5.0b11 dmb: fixed calling bracketed expresssions
			bigstring bs;
			
			if (langgetidentifier (htree, bs)) {
				
				if (langsearchpathlookup (bs, &htable)) {
					
					hashtablelookupnode (htable, bs, &hnode);
					
					if (!langgetnodecode (htable, bs, hnode, &hcode)) {
						
						langparamerror (notfunctionerror, bs);
						
						return (false);
						}

					copystring (bs, bsfunctionname);

					goto runhandler;
					}
				}
			}
			

	#if isFrontier && MACVERSION
		if (langipchandlercall (htree, bsfunctionname, hparam1, vreturned)) /*3.0a*/
			return (true);
	#endif
	
	langparamerror (unknownfunctionerror, bsfunctionname);
	
	return (false);
	
	runhandler:
	
	if (fllangerror)
		return (false);
	
	return (langfunctioncall (htree, htable, hnode, bsfunctionname, hcode, hparam1, vreturned));
	} /*langrunhandlercode*/


static boolean builtinvalue (tyfunctype token, hdltreenode hparam1, tyvaluerecord *vreturned) {
	
	/*
	4/2/93 dmb: syscrashfunc: takes optional string param.
	
	2.1b2 dmb: broke this out of functionvalue so we don't share stack 
	space with handler calls
	*/
	
	register tyvaluerecord *v = vreturned;
	register hdltreenode hp1 = hparam1;
	
	setbooleanvalue (true, v);
	
	setemptystring (bsfunctionname);
	
	functiontoken = token; /*set global*/
	
	switch (token) {
		
		case sizeoffunc:
			return (sizefunc (hp1, v));
		
		case typeoffunc:
			return (typefunc (hp1, v));
		
		case nameoffunc:
			return (namefunc (hp1, v));
		
		case parentoffunc:
			return (parentfunc (hp1, v));

		case indexoffunc:
			return (indexfunc (hp1, v));
		
		case definedfunc: {
			tyvaluerecord val;
			
			if (!langcheckparamcount (hp1, 1))
				return (false);
			
			disablelangerror (); /*any error will result in false return*/
			
			(*v).data.flvalue = getreadonlyparamvalue (hp1, 1, &val);
			
			enablelangerror ();
			
			return (true);
			}
		
		case packfunc:
			return (langpackverb (hp1, v));
		
		case unpackfunc:
			return (langunpackverb (hp1, v));
			
		#if isFrontier && (MACVERSION || RABNOTIMPEMENTED)

			case appleeventfunc:
				return (langipcmessage (hp1, normalmsg, v));
			
			case findereventfunc:
				return (langipcmessage (hp1, noreplymsg, v));
			
			case complexeventfunc:
				return (langipccomplexmessage (hp1, v));
			
			case tableeventfunc:
				return (langipctablemessage (hp1, v));
			
		#endif	

		case objspecfunc:
			flnextparamislast = true;
			
			return (getobjspecparam (hp1, 1, v));
		
		case setobjspecfunc:
			return (setobjspecverb (hp1, v));
		

			case gestaltfunc: {
				OSType selector;
				long result;
				
				flnextparamislast = true;
				
				if (!getostypevalue (hp1, 1, &selector))
					return (false);
				
				if (!gestalt (selector, &result))
					result = -1;
				
				return (setlongvalue (result, v));
				}
			
		case syscrashfunc: {
			
			bigstring bs;
			
			flparamerrorenabled = false;
			
			if (getstringvalue (hp1, 1, bs))
				DebugStr (bs);
			else
				Debugger ();
			
			flparamerrorenabled = true;
			
			return (true);
			}
		
		#if isFrontier && !flruntime
		
		case myMooffunc: {
			short ticksbetweenframes;
			long totalticks = 0;
			
			if (!getintvalue (hp1, 1, &ticksbetweenframes))
				return (false);
			
			if (langgetparamcount (hp1) > 1) {
				
				flnextparamislast = true;
				
				if (!getlongvalue (hp1, 2, &totalticks))
					return (false);
				}
			
			shellblockevents ();
			
			(*v).data.flvalue = myMoof (ticksbetweenframes, totalticks);
			
			shellpopevents ();
			
			return (true);
			}
		
		#endif
		
			default:
				/* do nothing */
				break;
				
		} /*switch*/
	
	#ifdef fldebug
	
	langerrormessage (STR_Internal_error_unimplemented_function_call);
	
	#endif
	
	setbooleanvalue (false, v);
	
	return (false);
	} /*builtinvalue*/


boolean functionvalue (hdltreenode htree, hdltreenode hparam1, tyvaluerecord *vreturned) {
	
	/*
	call the function and return the value it returns.
	
	hparam1 points at the first parameter to the function being called.
	*/
	
	tyvaluerecord val = (**htree).nodeval;
	
	if (((**htree).nodetype == dotop) || (val.valuetype != tokenvaluetype))
		return (langhandlercall (htree, hparam1, vreturned));
	else
		return (builtinvalue ((tyfunctype) val.data.tokenvalue, hparam1, vreturned));
	} /*functionvalue*/

