
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

#include "shell.rsrc.h"
#include "shell.h"
#include "cursor.h"
#include "font.h"
#include "langinternal.h"
#include "kb.h"
#include "memory.h"
#include "op.h"
#include "opinternal.h"
#include "lang.h" /*7.0b4 PBS*/


extern boolean tablevaltotable (tyvaluerecord val, hdlhashtable *htable, hdlhashnode hnode);

extern void pullstringvalue (const tyvaluerecord *v, bigstring bsval);

boolean opsetrefcon (hdlheadrecord hnode, ptrvoid pdata, long lendata) {
	
	/*
	each node in the structure can have a handle linked into it through the hrefcon
	field.  the application can allocate and access this handle himself, or use these
	routines.  when the structure is saved (oppack), the handle is saved with the
	node, and restored when the outline is unpacked.
	
	7/21/92 dmb: make sure the refcon is _exactly_ the right size, not any bigger
	*/
	
	Handle hrefcon = (**hnode).hrefcon;
	long len = lendata;
	
	if (hrefcon == nil) { /*never been set before*/
		
		Handle hnew;
		
		//if (!newclearhandle (len, &hnew))
		if (!newfilledhandle (pdata, len, &hnew)) // 7.31.97 dmb
			return (false);
			
		(**hnode).hrefcon = hrefcon = hnew;
		
		return (true);
		}
	
	if (gethandlesize (hrefcon) != len) { /*not the right size for the data*/
		
		if (!sethandlesize (hrefcon, len))
			return (false);
		}
		
	moveleft (pdata, *hrefcon, len);
	
	return (true);
	} /*opsetrefcon*/
	
	
boolean opgetrefcon (hdlheadrecord hnode, ptrvoid pdata, long lendata) {
	
	/*
	get the refcon data from the indicated node.  always returns true -- if there
	isn't enough data in the refcon field, just fill pdata with the indicated 
	number of 0's.
	
	10/23/90 dmb: if the refcon field is nil, return false (but still fill pdata 
	with zeros).  many callers expect this.
	*/
	
	Handle hrefcon = (**hnode).hrefcon;
	long lenrefcon;
	
	lenrefcon = gethandlesize (hrefcon); /*handles nil*/
	
	if (lenrefcon < lendata) /*not enough data, make sure it's all zero*/
		clearbytes (pdata, lendata);
	
	if (hrefcon == nil)
		return (false);
	
	if (lenrefcon > lendata) /*somehow there's more data available than caller wants*/
		lenrefcon = lendata;
	
	moveleft (*hrefcon, pdata, lenrefcon);
	
	return (true);
	} /*opgetrefcon*/
	
	
void opemptyrefcon (hdlheadrecord hnode) {
	
	if ((**hnode).hrefcon == nil) /*nothing to do*/
		return;
		
	opdirtyoutline ();
		
	disposehandle ((**hnode).hrefcon);
	
	(**hnode).hrefcon = nil;
	} /*opemptyrefcon*/
	
	
boolean ophasrefcon (hdlheadrecord hnode) {
	
	return ((**hnode).hrefcon != nil);
	} /*ophasrefcon*/


boolean opattributesgetoneattribute (hdlheadrecord hnode, bigstring bsattname, tyvaluerecord *val) {
	
	/*
	7.0b16 PBS: get one value from a packed attributes table.
	*/
	
	tyvaluerecord vattributes;
	tyvaluerecord v;
	hdlhashtable htable;
	hdlhashnode hn = nil;
	hdlhashnode hnatt;
	boolean fl = false;

	disablelangerror ();
	
	if (!opattributesgetpackedtablevalue (hnode, &vattributes))
		
		return (false);
	
	if (!tablevaltotable (vattributes, &htable, hn))
		
		goto exit;
	
	if (hashtablelookup (htable, bsattname, &v, &hnatt)) {
	
		copyvaluerecord (v, val);
		
		disposevaluerecord (v, false);
		
		exemptfromtmpstack (val);

		fl = true;
		} /*if*/
	
	exit:
	
	disposevaluerecord (vattributes, false);
	
	enablelangerror ();
	
	return (fl);	
	} /*opattributesgetoneattribute*/


boolean opattributesgetpackedtablevalue (hdlheadrecord hnode, tyvaluerecord *val) {
	
	/*
	7.0b16 PBS: get the table packed as a binary in a headline's refcon.
	*/
	
	Handle hrefcon = (**hnode).hrefcon;
	tyvaluerecord linkedval;
	boolean fl = false;
	
	if (!ophasrefcon (hnode)) {/*if no refcon, no attributes*/
	
		langerrormessage ("\x39""Can't get attributes because this headline has no refcon.");
		
		goto exit1;
		}
	
	if (!langunpackvalue (hrefcon, &linkedval)) { /*try to unpack the refcon*/
		
		langerrormessage ("\x3e""Can't get attributes because of an error unpacking the refcon.");

		goto exit1;
		}
	
	if (linkedval.valuetype != binaryvaluetype) { /*must be a binary*/
		
		langerrormessage ("\x3e""Can't get attributes because the refcon is not of binary type.");

		goto exit2;
		}
	
	if (!langunpackvalue (linkedval.data.binaryvalue, val)) { /*it's a packed binary: unpack it.*/
		
		langerrormessage ("\x3e""Can't get attributes because of an error unpacking the refcon.");

		goto exit2;
		}
	
	fl = true;

	exit2:
	
	disposevaluerecord (linkedval, false);
	
	exit1:
	
	return (fl);
	} /*opattributesgetpackedtable*/


boolean opattributesgettypestring (hdlheadrecord hnode, bigstring bstype) {
	
	/*
	7.0b4 PBS: Get the node type of a headline.
	Unpack the refcon, look in the table for a type attribute, set bstype
	equal to that string.
	
	7.0b14 PBS: Fixed memory leaks.
	*/
	
	Handle hrefcon = (**hnode).hrefcon;
	tyvaluerecord linkedval;
	tyvaluerecord val;
	hdlhashtable htable;
	hdlhashnode hn = nil;
	hdlhashnode hnheadlinetype;
	tyvaluerecord valheadlinetype;
	boolean fl = false;
	
	disablelangerror ();

	if (!ophasrefcon (hnode)) //if no refcon, not attributes
	
		goto exit3;

	if (!langunpackvalue (hrefcon, &linkedval)) //try to unpack the refcon
	
		goto exit3;
	
	if (linkedval.valuetype != binaryvaluetype) //must be a binary
		
		goto exit2;
	
	if (!langunpackvalue (linkedval.data.binaryvalue, &val)) //it's a packed binary: unpack it.
		
		goto exit2;
	
	if (val.valuetype != externalvaluetype) //it must be a table
		
		goto exit1;
	
	if (!tablevaltotable (val, &htable, hn))
		
		goto exit1;
	
	if (hashtablelookup (htable, "\x04""type", &valheadlinetype, &hnheadlinetype)) {
	
		pullstringvalue (&valheadlinetype, bstype);
		
		fl = true;
		} /*if*/
	
	exit1: /*7.0b14 PBS: fix memory leaks*/
	
	disposevaluerecord (val, false);
	
	exit2:
	
	disposevaluerecord (linkedval, false);
	
	exit3:
	
	enablelangerror ();

	return (fl);
	} /*opattributesgettypestring*/



