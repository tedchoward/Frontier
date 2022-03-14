
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
#include "ops.h"
#include "frontier_strings.h"
#include "cursor.h"
#include "db.h"
#include "file.h"
#include "font.h"
#include "resources.h"
#include "lang.h"
#include "langinternal.h"
#include "langexternal.h"
#include "langsystem7.h"
#include "tableinternal.h"
#include "tablestructure.h"
#include "process.h"
#include "oplist.h"




#define minstackspace 0x0800


boolean flstackoverflow = false;

boolean flfindanyspecialsymbol = false; /*see langfindsymbol*/

//short flextendedsymbolsearch = true; /*see langgetsymbolval*/



typedef struct tytypeinfo {
	
	OSType id;
	
	byte *bsname;
	} tytypeinfo;


static tytypeinfo typeinfo [ctvaluetypes] = {
	
	{'\?\?\?\?', STR_unknown},
	
	{'char', STR_char},
	
	{'shor', STR_int},
	
	{'long', STR_long},
	
	{'bad1', STR_unused1}, /*8/13*/
	
	{'data', STR_binary},
	
	{'bool', STR_boolean},
	
	{'tokn', STR_token},
	
	{'date', STR_date},
	
	{'addr', STR_address},
	
	{'code', STR_compiled_code},
	
	{'exte', STR_double},
	
	{'TEXT', STR_string}, /*8/13*/
	
	{'xtrn', STR_external},
	
	{'dir ', STR_direction},
	
	{'bad2', STR_unused2}, /*9/17/91*/
	
	{'type', STR_string4},
	
	{'bad3', STR_unused3},
	
	{'QDpt', STR_point},
	
	{'qdrt', STR_rect},
	
	{'tptn', STR_pattern},
	
	{'cRGB', STR_rgb}, /*12/11/92 dmb*/
	
	{'fixd', STR_fixed},
	
	{'sing', STR_single},
	
	{'doub', STR_double},
	
	{'obj ', STR_objspec},
	
	{'fss ', STR_filespec},
	
	{'alis', STR_alias},
	
	{'enum', STR_enumerator},
	
	{'list', STR_list},
	
	{'reco', STR_record},
	
	/*
	the following value types, outline - pictvaluetype, are never used directly.
	the value would actually be externalvaluetype; these are for flattening 
	external types into a typevaluetype
	*/
	
	{'optx', STR_outline},
	
	{'wptx', STR_wptext},
	
	#ifdef fliowa
	
	{'intf', STR_interface},
	
	#else
	
	{'head', STR_headline},
	
	#endif
	
	{'tabl', STR_table},
	
	{'scpt', STR_script},
	
	{'mbar', BIGSTRING (STR_menubar)},
	
	{'pict', STR_picture}
	
	};


boolean langgettypestring (tyvaluetype type, bigstring bs) {
	
	boolean fl = true;
	
	if ((type < novaluetype) || (type >= ctvaluetypes)) {
		
		type = novaluetype;
		
		fl = false;
		}
	
	copystring ((ptrstring) typeinfo [type].bsname, bs);
	
	return (fl);
	} /*langgettypestring*/


tyvaluetype langgettype (tyvaluerecord val) {

	/*
	6.2b5 AR
	*/

	tyvaluetype type = val.valuetype;
	
	if ((type < novaluetype) || (type >= ctvaluetypes))
		return (uninitializedvaluetype);
	
	if (type == externalvaluetype)
		type = (tyvaluetype) (outlinevaluetype + langexternalgettype (val));
	
	return (type);
	}/*langgettype*/
	
	
OSType langgettypeid (tyvaluetype type) {
	
	if ((type < novaluetype) || (type >= ctvaluetypes))
		return ((OSType) 0);
	
	return (typeinfo [type].id);
	} /*langgettypeid*/


tyvaluetype langgetvaluetype (OSType ostypeid) {
	
	/*
	3/2/92 dmb: added check for old 'doub' value of doublevaluetype
	
	12/11/92 dmb: added backward compatibility check for rgb values
	*/
	
	register tyvaluetype type;
	
	for (type = novaluetype;  type < ctvaluetypes;  ++type)
		if (typeinfo [type].id == ostypeid)
			return (type);
	
	if (ostypeid == 'RGB ') /*12/11/92 dmb*/
		return (rgbvaluetype);
	
	return ((tyvaluetype) -1);
	} /*langgetvaluetype*/


boolean langgoodbinarytype (tyvaluetype type) {
	
	/*
	11/6/92 dmb: need this to qualify iac values & other binary vals
	
	return true if the binary type can be coerced into a normal scalar value
	*/
	
	if ((type < novaluetype) || (type >= outlinevaluetype)) /*outside of good range*/
		return (false);
	
	switch (type) {
		
		case oldstringvaluetype:
		case tokenvaluetype:
		case codevaluetype:
		case externalvaluetype:
		case passwordvaluetype:
		case unused2valuetype:
		case olddoublevaluetype:
			return (false);
		
		default:
			return (true);
		}
	} /*langgoodbinarytype*/


boolean langheaptype (tyvaluetype type) {
	
	/*
	return true if the value is heap-allocated.
	
	11/4/90 DW: return true if it's an external type.
	
	2/15/91 dmb: return true if it's a binary value
	
	5/21/91 dmb: added a bunch of new heap-allocated values.
	*/
	
	switch (type) {
		
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
		case listvaluetype:
		case recordvaluetype:
		case externalvaluetype:
		case codevaluetype:
			return (true);
		
		default:
			return (type >= outlinevaluetype); /*virtual external types are heap-allocated*/
		} /*switch*/
	} /*langheaptype*/


boolean langscalartype (tyvaluetype type) {
	
	return (!langheaptype (type));
	} /*langscalartype*/


boolean langheapallocated (tyvaluerecord *val, Handle *heaphandle) {
	
	/*
	return true if the value is heap-allocated.  if so, *heaphandle points to 
	the heap memory allocated for the value.

	5.0.1 dmb: allow heaphandle to be nil, in case caller doesn't need it
	*/
	
	assert (val != nil);
	
	if (!langheaptype ((*val).valuetype))
		return (false);
	
	if (heaphandle != nil)
		*heaphandle = (*val).data.binaryvalue;
	
	return (true);
	} /*langheapallocated*/


void langgetwithvaluename (short n, bigstring bswith) {
	
	/*
	8/31/92 dmb: return the name for the nth with value for a stack frame.
	
	n should be 1-based.
	*/
	
	bigstring bsint;
	
	copystring (STR_with, bswith);
	
	numbertostring (n, bsint);
	
	if (stringlength (bsint) < 2)
		pushchar ('0', bswith);
	
	pushstring (bsint, bswith);
	} /*langgetwithvaluename*/


boolean langfindsymbol (const bigstring bs, hdlhashtable *htable, hdlhashnode *hnode) {
	
	/*
	search through the stack of symbol tables until you find one that has the
	symbol named by bs defined.
	
	return with handles to the table it was found in and the hash node that
	holds the value of the symbol.
	
	2/12/92 dmb: implemented lexicalrefcon check to prevent dangerous dynamic 
	scoping possibilities.  specifically, we're preventing a script from accessing 
	the local variables in a stack frame that comes from another script object.
	
	8/7/92 dmb: make sure currenthashtable isn't nil before setting refcon
	
	9/14/92 dmb: set htable to the innermost valid with table or local table 
	encountered so that new symbols can be placed there by default. also, fixed  
	with handling so that tables are visited in the right order -- last to first
	
	12/18/92 dmb: removed a bit of functionality for the safety of our users: don't 
	allow "with" statement to create context for assigning undeclared values.
	
	3.0.2 dmb: to be a "special" symbol that can be seen across lexical scope, bs 
	must now end with an '_' as well as start with one. This excludes the default 
	container for object model expressions, while retaining the current target.
	
	3.0.3 dmb: sadly, while the above change worked fine, we _sometimes_ need to 
	include the default container in a context-free (lexically-independent) fashion.
	so, we introduce the flfindanyspecialsymbol global. in the default (false) case, 
	we require the trailing underscore. if set to true (temporarily), we'll treat 
	any indentifier that starts with an underscore specially.
	
	5.0.2b6 dmb: caller who sets flfindanyspecialsymbol is asserting that whatever 
	is being searched for is a special symbol of sorts, whatever its name might be.
	so don't require it to start with an underscore. so, flspecialsymbol really 
	means just "context free"
	*/
	
	register hdlhashtable h = currenthashtable;
	register long refcon;
	register long lexrefcon;
	register boolean flspecialsymbol = false;
	register short n;
	
	*htable = nil;
	
	*hnode = nil;
	
	if (h == nil)
		return (false);
	
	/*maybe treat as context-free*/
	flspecialsymbol = flfindanyspecialsymbol || ((bs [1] == '_') && (lastchar (bs) == '_'));
	
	refcon = (**h).lexicalrefcon;
	
	while (true) { /*chain through each linked hash table*/
		
		if (h == nil)  /*symbol not defined*/
			return (false);

		//assert (validhandle ((Handle) h));
		
		lexrefcon = (**h).lexicalrefcon;
		
		if ((!(**h).fllocaltable) || flspecialsymbol || (refcon == 0) || (lexrefcon == refcon) || (lexrefcon == 0)) {
			
			if (hashtablelookupnode (h, bs, hnode)) { /*symbol is defined in htable*/
				
				*htable = h;
				
				return (true);
				}
			
			for (n = (**h).ctwithvalues; n > 0; --n) { /*scan with values*/
				
				tyvaluerecord valwith;
				hdlhashtable hwith;
				bigstring bswith;
				hdlhashnode hnode2;
				
				langgetwithvaluename (n, bswith);
				
				if (!hashtablelookup (h, bswith, &valwith, &hnode2)) /*missing with value; keep going*/
					continue;
				
				if (!getaddressvalue (valwith, &hwith, bswith)) /*error*/
					return (false);
				
				if (!isemptystring (bswith)) { // not encoded as expected
					
					if (!hashtablelookup (hwith, bswith, &valwith, &hnode2))
						return (false);

					if (!langexternalvaltotable (valwith, &hwith, hnode2))
						return (false);
					}

				if (hashtablelookupnode (hwith, bs, hnode)) { /*found symbol*/
					
					*htable = hwith;
					
					return (true);
					}
				
				/*12/18/92 dmb
				if (*htable == nil) /%this is the innermost with value -- return to caller%/
					*htable = hwith;
				*/
				}
			
			if (*htable == nil) /*this is the innermost local table -- return to caller*/
				*htable = h;
			}
		
		h = (**h).prevhashtable;
		} /*while*/
	} /*langfindsymbol*/


/*
boolean langfindexternalwindow (const bigstring bs, Handle *hdata) {
	
	/%
	7.21.97 dmb: see if the string specifies a standalong window that has data
	%/
	
	hdlwindowinfo hinfo;

	if (flextendedsymbolsearch > 0 && 
		shellfindnamedwindow (bs, &hinfo) && 
		(**hinfo).parentwindow == nil)
		
		return (shellgetexternaldata (hinfo, hdata));
	
	return (false);
	} /%langfindexternalwindow%/
*/


boolean langgetsymbolval (const bigstring bs, tyvaluerecord *vreturned, hdlhashnode *hnode) {
	
	/*
	6/13/91 dmb: moved special-case check for root table in from langsymbolreference
	
	2003-05-18 AR: Disabled special check for fllocaltable to prevent the address of a value
	in a local table to resolve to the root table if the value is actually named "root".
	This could potentially cause top-level tables in Frontier.root to be overwritten unintentionally.
	It's not clear what purpose this check was meant to serve in the first place.
	*/
	
	hdlhashtable htable;
//	hdlhashnode hnode;
	
	if (langfindsymbol (bs, &htable, hnode)) {
		
		*vreturned = (***hnode).val;
		
		return (true);
		}
	
	if (equalstrings (bs, nameroottable)) {
		
		if ((currenthashtable == roottable) || (currenthashtable == nil) /* || ((**currenthashtable).fllocaltable) */) {
			
			setexternalvalue (rootvariable, vreturned);
			
			return (true);
			}
		}
	
	return (false);
	} /*langgetsymbolval*/


boolean langsetsymbolval (const bigstring bs, tyvaluerecord val) {
	
	/*
	if the symbol is defined in one of the chain of symbol tables, then
	assign the value.
	
	if the symbol isn't defined anywhere, create a new symbol with the
	name in the most global symbol table.  this is where automatic 
	declaration of variables is implemented.
	
	7/10/90 DW: instead of allocating automatic variables in the most-
	global table, allocate them in the most-local table.  see comment
	at the head of evaluatelist.
	
	9/14/92 dmb: see comment in langfindsymbol; if htable is non-nil, it 
	is now valid even when false is returned.
	
	5.0a18 dmb: require declarations! (if user's pref is set)
	*/
	
	hdlhashtable htable;
	hdlhashnode hnode;
	
	if (false && langgetuserflag (idrequiredeclarationsscript, false)) {
	
		if (langfindsymbol (bs, &htable, &hnode) || (htable != nil)) { /*name is defined, or with statement set table*/
			
			return (hashtableassign (htable, bs, val));
			}
		
		langparamerror (unknownidentifiererror, bs);
		
		return (false);
		}
	else {
	
		if (langfindsymbol (bs, &htable, &hnode) || (htable != nil)) { /*name is defined, or with statement set table*/
			
			return (hashtableassign (htable, bs, val));
			}
		
		return (hashassign (bs, val));
		}
	} /*langsetsymbolval*/


boolean langsetsymboltableval (hdlhashtable htable, const bigstring bs, tyvaluerecord val) {
	
	boolean fl;
	
	pushhashtable (htable);
	
	fl = langsetsymbolval (bs, val);
	
	pophashtable ();
	
	return (fl);
	} /*langsetsymboltableval*/


boolean langsetstringval (const bigstring bsname, const bigstring bsval) {
	
	tyvaluerecord val;
	Handle htext;
	
	initvalue (&val, stringvaluetype);
	
	if (!newtexthandle (bsval, &htext))
		return (false);
	
	val.data.stringvalue = htext;
	
	if (!langsetsymbolval (bsname, val)) {
		
		disposehandle (htext);
		
		return (false);
		}
	
	return (true);
	} /*langsetstringval*/


boolean langsetbinaryval (hdlhashtable htable, const bigstring bsname, Handle x) {
	
	tyvaluerecord val;
	
	initvalue (&val, binaryvaluetype);
	
	val.data.binaryvalue = x;
	
	if (!langsetsymboltableval (htable, bsname, val)) {
		
		disposehandle (x);
		
		return (false);
		}
	
	return (true);
	} /*langsetbinaryval*/


boolean langassigntextvalue (hdlhashtable ht, bigstring bs, Handle h) {

	/*
	6.1d4 AR: efficient way to do this operation; no tmpstack overhead.
	*/
	
	tyvaluerecord val;
	
	assert (h != nil);
	
	initvalue (&val, stringvaluetype);

	val.data.stringvalue = h;

	if (!hashtableassign (ht, bs, val))
		return (false);
		
	return (true);
	} /*hashtableassignstringhandle*/



boolean langassignstringvalue (hdlhashtable ht, const bigstring bs, const bigstring bsval) {
	
	/*
	efficient way to do this operation; no tmpstack overhead.
	
	note: very similar to langsetstringval, but w/out the symbol lookup logic
	*/
	
	tyvaluerecord val;
	Handle htext;
	
	initvalue (&val, stringvaluetype);
	
	if (!newtexthandle (bsval, &htext))
		return (false);
	
	val.data.stringvalue = htext;
	
	if (!hashtableassign (ht, bs, val)) {
		
		disposehandle (htext);
		
		return (false);
		}
	
	return (true);
	} /*langassignstringvalue*/


boolean langassignbooleanvalue (hdlhashtable ht, const bigstring bs, boolean flval) {
	
	/*
	could be useful...
	*/
	
	tyvaluerecord val;
	
	setbooleanvalue (flval, &val);
	
	return (hashtableassign (ht, bs, val));
	} /*langassignbooleanvalue*/


boolean langassigncharvalue (hdlhashtable ht, const bigstring bs, unsigned char ch) {
	
	/*
	might as well!
	*/
	
	tyvaluerecord val;
	
	setcharvalue (ch, &val);
	
	return (hashtableassign (ht, bs, val));
	} /*langassigncharvalue*/


boolean langassignlongvalue (hdlhashtable ht, const bigstring bs, long x) {
	
	/*
	might as well!
	*/
	
	tyvaluerecord val;
	
	setlongvalue (x, &val);
	
	return (hashtableassign (ht, bs, val));
	} /*langassignbooleanvalue*/


boolean langassignaddressvalue (hdlhashtable ht, const bigstring bs, const tyaddress *adr) {
	
	/*
	efficient way to do this operation; no tmpstack overhead.
	
	6.1d4 AR: Modified to no longer use tmpstack (which it did, previously)
	by cribbing some code from setaddressvalue.
	*/
	
	tyvaluerecord val;
	hdlstring hstring;
	
	if (!newheapstring ((*adr).bs, &hstring))
		return (false);
	
	if (!enlargehandle ((Handle) hstring, sizeof ((*adr).ht), (void *) &(*adr).ht)) {

		disposehandle ((Handle) hstring);

		return (false);
		}
	
	initvalue (&val, addressvaluetype);
	
	val.data.addressvalue = hstring;
	
	if (!hashtableassign (ht, bs, val)) {
		
		disposevaluerecord (val, false);
		
		return (false);
		}
	
	return (true);
	} /*langassignaddressvalue*/


boolean langassignnewtablevalue (hdlhashtable ht, const bigstring bs, hdlhashtable *newtable) {
	
	/*
	create a new, empty table and assign it to the given table/name in the database
	*/
	
	tyvaluerecord val;
	
	if (!tablenewtablevalue (newtable, &val))
		return (false);
	
	if (!hashtableassign (ht, bs, val)) {
		
		disposevaluerecord (val, false);
		
		return (false);
		}
	
	(***newtable).fllocaltable = (**ht).fllocaltable;
	
	return (true);
	} /*langassignnewtablevalue*/


boolean langsuretablevalue (hdlhashtable ht, const bigstring bs, hdlhashtable *htable) {

	/*
	5.1.4 dmb: if the cell has a table, return it. otherwise, create a new table,
	assign it, and return it.
	*/
	
	tyvaluerecord val;
	hdlhashnode hnode;
	
	if (hashtablelookup (ht, bs, &val, &hnode))
		if (langexternalvaltotable (val, htable, hnode))
			return (true);
	
	return (langassignnewtablevalue  (ht, bs, htable));
	} /*langsuretablevalue*/


#ifndef odbengine

boolean langgetvalsize (tyvaluerecord v, long *size) {
	
	/*
	4/26/93 dmb: count tree nodes
	
	2.1b3 dmb: expand filespec to string to avoid breaking 2.0 scripts
	*/
	
	register long x;
	
	switch (v.valuetype) {
		
		case addressvaluetype: {
			bigstring bs;
			
			getaddresspath (v, bs);
			
			x = stringlength (bs);
			
			break;
			}
			
		case booleanvaluetype:
			x = longsizeof (v.data.flvalue);
			
			break;
			
		case charvaluetype:
			x = longsizeof (v.data.chvalue);
			
			break;
		
		case intvaluetype:
			x = longsizeof (v.data.intvalue);
			
			break;
		
		case longvaluetype:
		case ostypevaluetype:
		case enumvaluetype:
		
			x = longsizeof (v.data.longvalue);
			
			break;
		
		case directionvaluetype:
			x = longsizeof (v.data.dirvalue);
			
			break;
		
		case tokenvaluetype:
			x = longsizeof (v.data.tokenvalue);
			
			break;
		
		case pointvaluetype:
			x = longsizeof (v.data.pointvalue);
			
			break;
		
		case datevaluetype:
			x = longsizeof (v.data.datevalue);
			
			break;
		
		case fixedvaluetype:
			x = longsizeof (v.data.fixedvalue);
			
			break;
		
		case singlevaluetype:
			x = longsizeof (v.data.singlevalue);
			
			break;
		
		case filespecvaluetype: {
			bigstring bs;
			
			if (filespectopath (*v.data.filespecvalue, bs))
				x = stringlength (bs);
			else
				x = gethandlesize ((Handle) v.data.filespecvalue);
			
			break;
			}
		
		#if defined(__powerc) || defined(WIN95VERSION)
		
			case doublevaluetype:
				x = sizeof (extended80);
				
				break;
			
		#else
				case doublevaluetype:
		#endif
		
		case stringvaluetype:
		case passwordvaluetype:
		case rectvaluetype:
		case patternvaluetype:
		case rgbvaluetype:
		case objspecvaluetype:
		case aliasvaluetype:
			x = gethandlesize (v.data.binaryvalue);
			
			break;
		
		case binaryvaluetype:
			x = gethandlesize (v.data.binaryvalue) - sizeof (OSType); /*don't count key*/
			
			break;
		
		case listvaluetype:
		case recordvaluetype: {
			long listsize;
			
			if (!langgetlistsize (&v, &listsize))
				return (false);
			
			x = listsize;
			
			break;
			}
		
		case codevaluetype:
			x = langcounttreenodes (v.data.codevalue);
			
			break;
		
		case externalvaluetype: {
			long externalsize;
			
			if (!langexternalgetvalsize (v, &externalsize))
				return (false);
			
			x = externalsize;
			
			break;
			}
			
		default:
			return (false);
		} /*switch*/
	
	*size = x;
	
	return (true);
	} /*langgetvalsize*/

#endif

boolean langgetstringlist (short id, bigstring bs) {
	
	return (getstringlist (langinterfacelist, id, bs));
	} /*langgetstringlist*/


boolean langgetmiscstring (short id, bigstring bs) {
	
	return (getstringlist (langmiscstringlist, id, bs));
	} /*langgetmiscstring*/


boolean langcheckstacklimit (tystackid idstack, short topstack, short maxstack) {
	
	/*
	make sure that the stack isn't full.  if it is, generate a langerror
	*/
	
	bigstring bsstack;
	
	if (topstack < maxstack)
		return (true);
	
	getstringlist (langstacklist, idstack, bsstack);
	
	langparamerror (stackoverflowerror, bsstack);
	
	return (false);
	} /*langcheckstacklimit*/

#if !odbengine
boolean langcheckstackspace (void) {
	
	/*
	9/30/92 dmb: moved stack detection code here so it can be called when needed
	
	3.0.1 dmb: use new processstackspace instead of StackSpace trap
	*/
	
#if !flruntime
	if (processstackspace () < minstackspace) {
		
		flstackoverflow = true; /*flag for reporting when it's safe*/
		
		return (false);
		}
#endif 	
	return (true);
	} /*langcheckstackspace*/
#endif

void langbadexternaloperror (short errornum, tyvaluerecord externalval) {
	
	bigstring bstype;
	bigstring bsscriptextra;
	
	langexternaltypestring ((hdlexternalhandle) externalval.data.externalvalue, bstype);
	
	if (langexternalgettype (externalval) == idscriptprocessor)
		langgetstringlist (useparenthesisstring, bsscriptextra);
	else
		langgetstringlist (useaddressoperatorstring, bsscriptextra);
	
	lang2paramerror (errornum, bstype, bsscriptextra);
	} /*langbadexternaloperror*/

#if !odbengine
void langarrayreferror (short stringnum, bigstring bsname, const tyvaluerecord *valarray, tyvaluerecord *valindex) {
	
	/*
	valindex is nil, the error is trying to treat valarray as an array in the 1st place.
	
	otherwise, it's a bad name or item number
	*/
	
	bigstring bstype;
	bigstring bsindex;
	
	hashgettypestring (*valarray, bstype);
	
	if (valindex == nil) {
		
		stringnum = arraynottableerror;
		
		setemptystring (bsindex);
		}
	else {
		switch ((*valindex).valuetype) {
			
			case intvaluetype:
			case longvaluetype:
				stringnum = arrayindexerror;
				
				break;
			
			default:
				stringnum= arraystringindexerror;
			}
		
		coercetostring (valindex);
		
		pullstringvalue (valindex, bsindex);
		}

	if (bsname == nil)
		lang3paramerror (stringnum, emptystring, bstype, bsindex);
	else
		lang3paramerror (stringnum, bsname, bstype, bsindex);

	} /*langarrayreferror*/
#endif


boolean langgetuserflag (short idscript, boolean fldefault) {
	
	/*
	5.0a18 dmb: make sure table globals are set up; fixed crashing bug.
	*/
	
	bigstring bspref;
	hdlhashtable htable;
	bigstring bs;
	tyvaluerecord val;
	boolean fl;
	hdlhashnode hnode;
	
	if (roottable == nil)
		return (fldefault);
	
	getsystemtablescript (idscript, bspref);
	
	disablelangerror ();
	
	pushhashtable (roottable);
	
	if (
		langexpandtodotparams (bspref, &htable, bs) && 
	
		hashtablelookup (htable, bs, &val, &hnode) &&
		
		copyvaluerecord (val, &val) && coercetoboolean (&val)
		)
		
		fl = val.data.flvalue;
	else
		fl = fldefault;
	
	pophashtable ();
	
	enablelangerror ();
	
	return (fl);
	} /*langgetuserflag*/


boolean langsetuserflag (short idscript, boolean fl) {
	
	/*
	5.0a21 dmb: might as well provide the matching set routine
	*/
	
	bigstring bspref, bsignore;
	
	if (roottable == nil)
		return (false);
	
	getsystemtablescript (idscript, bspref);
	
	pushchar ('=', bspref);
	
	if (fl)
		pushstring (bstrue, bspref);
	else
		pushstring (bsfalse, bspref);
	
	return (langrunstringnoerror (bspref, bsignore));
	} /*langsetuserflag*/


boolean langgetthisaddress (hdlhashtable *htable, bigstring bsthis) {
	
	/*
	5.0a18 dmb: find the current lexical address. right now, we're sticking 
	this in stack frames when necessary. but we could also make "this" a magic 
	value that is calculated when referenced; idvalue would call this function
	*/
	
	register hdlerrorstack hs = langcallbacks.scripterrorstack;
	tyerrorrecord *pe;
	
	if ((hs == nil) || ((**hs).toperror == 0))
		return (false);
	
	pe = &(**hs).stack [(**hs).toperror - 1];
	
	if ((*pe).errorcallback == nil)
		return (false);
	
	return ((*(*pe).errorcallback) ((*pe).errorrefcon, 0, 0, htable, bsthis));
	} /*langgetthisaddress*/


boolean langsetthisvalue (hdlhashtable hlocaltable, hdlhashtable htable, bigstring bsname) {
	
	/*
	5.0a18 dmb: new feature: this. it remains to be seen whether or not
	we're setting it whereever we need it. another alternative is to 
	calculate it when refererenced, in idvalue, based on the errorcallback
	stack
	*/
	
	#ifdef version5orgreater

	tyvaluerecord val;
	
	if (!setaddressvalue (htable, bsname, &val))
		return (false);
	
	if (!hashtableassign (hlocaltable, STR_this, val))
		return (false);
		
	exemptfromtmpstack (&val);

	#endif

	return (true);
	} /*langsetthisvalue*/


boolean langbuildnamelist (hdltreenode htree, hdllistrecord hlist) {
	
	/*
	5.0.2b8 dmb: new routine. recursively build an ordered list of 
	names from the dotparams in the code tree. similar, but more 
	limited, than langgetdotparams
	*/
	
	bigstring bsname;
	
	switch ((**htree).nodetype) {
		
		case identifierop:
		case bracketop:
			if (!langgetidentifier (htree, bsname))
				return (false);
			
			return (langpushliststring (hlist, bsname));
		
		case dotop:
			if (!langbuildnamelist ((**htree).param1, hlist))
				return (false);
			
			if (!langgetidentifier ((**htree).param2, bsname))
				return (false);
			
			return (langpushliststring (hlist, bsname));
		
		default:
			langlongparamerror (unexpectedopcodeerror, (**htree).nodetype);
			
			return (false);
		}		
	} /*buildnamelist*/


boolean langfastaddresstotable (hdlhashtable hstart, bigstring bsaddress, hdlhashtable *htable) {
	
	/*
	5.0.2 dmb: super-fast lookup of a table, given a simple path starting from the
	given table. no periods are allowed in any item name; no square brackets are used.
	*/
	
	tyvaluerecord val;
	short i;
	bigstring bs;
	hdlhashtable ht = hstart;
	hdlhashnode hnode;
	
	for (i = 1; nthword (bsaddress, i, '.', bs); ++i) {
		
		if (!hashtablelookup (ht, bs, &val, &hnode) || !langexternalvaltotable (val, &ht, hnode)) {
			
			langparamerror (nosuchtableerror, bs);
			
			return (false);
			}
		}
	
	*htable = ht;
	
	return (true);
	} /*langfastaddresstotable*/


boolean langvaltocode (tyvaluerecord *vcode, hdltreenode *hcode) {

	/*
	02/04/02 dmb: get a script code value. Can be a local script code value,
	or the code of a script. no copies are made. hcode may even be nil; a 
	true return just confirms that the value _can_ be called as a script.
	*/

//	boolean fl;

	if ((*vcode).valuetype == codevaluetype) {

		*hcode = (*vcode).data.codevalue;

		return (true);
		}
	else {

		return (langexternalvaltocode (*vcode, hcode));
		}
	} /*langvaltocode*/


boolean langfollowifaddressvalue (tyvaluerecord *v) {

	/*
	1/15/02 dmb: new function. return true if we successfully follow an addres.
	*/

	hdlhashtable ht;
	bigstring bs;
	hdlhashnode hnode;
	boolean fl;

	if ((*v).valuetype != addressvaluetype)
		return (false);

	if (!getaddressvalue ((*v), &ht, bs))
		return (false);

	disablelangerror ();

	fl = langhashtablelookup (ht, bs, v, &hnode);

	enablelangerror ();

	return (fl);
	} /*langfollowifaddressvalue*/





