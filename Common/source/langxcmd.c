
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
#include "search.h"
#include "strings.h"
#include "ops.h"
#include "frontierconfig.h"
#include "lang.h"
#include "langinternal.h"
#include "langexternal.h"
#include "langxcmd.h"
#include "tablestructure.h"
#include "tableverbs.h"
#include "process.h"



XCmdPtr plangxcmdrec = nil;



static void scantext (byte ch, ptrbyte *ptext) {
	
	/*
	5/5/92 dmb: support routine for xcmdcallback
	*/
	
	register ptrbyte p = *ptext;
	register byte c = ch;
	
	while (true) {
		
		if (*p == c) {
			
			*ptext = p;
			
			return;
			}
		
		++p;
		}
	} /*scantext*/


static long scanlength (byte ch, ptrbyte ptext) {
	
	register ptrbyte pstart = (ptrbyte) ptext;
	ptrbyte pend = pstart;
	
	scantext (ch, &pend);
	
	return ((long) (pend - pstart));
	} /*scanlength*/


void truncatecstringhandle (Handle h) {
	
	/*
	11/6/92 dmb: scan the text in h for a null terminator, and
	truncate it there.
	*/
	
	long len;
	
	len = scanlength (chnul, (ptrbyte) *h);
	
	if (len < gethandlesize (h))
		sethandlesize (h, len); /*truncate at zero terminator*/
	} /*truncatecstringhandle*/


static void valtostring (tyvaluerecord *val, bigstring bsval) {
	
	/*
	take the newly-allocated value, make a string out of it, and 
	dispose the value record
	*/
	
	coercetostring (val);
	
	pullstringvalue (val, bsval);
	
	releaseheaptmp ((*val).data.stringvalue);
	} /*valtostring*/


static void stringtoval (bigstring bsval, tyvaluetype type, tyvaluerecord *val) {
	
	setstringvalue (bsval, val);
	
	disablelangerror ();
	
	if (!coercevalue (val, type)) {
		
		releaseheaptmp ((*val).data.stringvalue);
		
		(*val).data.stringvalue = nil;
		}
	
	enablelangerror ();
	} /*stringtoval*/


static boolean resolvexcmdglobal (bigstring bsglobal, hdlhashtable *htable, bigstring bsname) {
	
	/*
	7/8/92 dmb: resolve a HyperCard global reference.  If the global string 
	can be interpreted as an existing local or global value, use that value. 
	otherwise, interpret it as the name of an item in scratchpad
	*/
	
	bigstring bs;
	boolean fl;
	static byte nameglobalstable [] = "\pscratchpad";
	
	copystring (bsglobal, bs);
	
	disablelangerror ();
	
	fl = langexpandtodotparams (bs, htable, bsname);
	
	enablelangerror ();
	
	if (!fl) {
		
		*htable = nil;
		
		copystring (bs, bsname);
		}
	
	if (*htable == nil) {
		
		if (!langsearchpathlookup (bsname, htable)) {
			
			if (!findnamedtable (roottable, nameglobalstable, htable))
				*htable = roottable;
			}
		}
	
	return (true);
	} /*resolvexcmdglobal*/


void xcmdcallback (void) {
	
	/*
	5/4/92 dmb: HyperCard 1.0 XCMD callback emulation
	
	7/8/92 dmb: set return code; use xresNotImp for unknown request
	
	9/25/92 dmb: added SendHCEvent callback support. also spotted & fixed 
	a bug in ExtToStr handler.
	
	10/3/92 dmb: it turns out that the protocol requires that the caller 
	of the xcmd put the xcmdptr into a global for us; the xcmd code 
	doesn't pass it to us consistently.
	
	11/6/92 dmb: use new truncatecstringhandle instead of just popping last char.
	also, fixed NumToHex, NumToStr
	*/
	
	register XCmdPtr pxcb;
	register long *args;
	register long *out;
	tyvaluerecord val;
	bigstring bs;
	hdlhashtable htable;
	hdlhashnode hnode;
	
	pxcb = plangxcmdrec; /*copy global into register*/
	
	args = (*pxcb).inArgs;
	out = (long *) &(*pxcb).outArgs;
	
	(*pxcb).result = xresSucc;
	
	switch ((*pxcb).request) {
		
		/*  HyperTalk Utilities  */
		case 0x02: /*Handle EvalExpr (StringPtr expr)*/
			
			if (!langrunstringnoerror ((ptrstring) args [0], bs))
				(*pxcb).result = xresFail;
			
			else {
				
				pushchar (chnul, bs);
				
				newtexthandle (bs, (Handle *) out);
				}
			
			break;
		
		case 0x01: /*void SendCardMessage (StringPtr msg)*/
		case 0x05: /*void SendHCMessage (StringPtr msg)*/
			
			if (!langrunstringnoerror ((ptrstring) args [0], bs))
				(*pxcb).result = xresFail;
			
			break;
		
		/*
		case 0x26: /*void RunHandler (Handle handler)%/
			
			break;
		*/
		
		
		/*  Memory Utilities  */
		case 0x12: /*Handle GetGlobal (StringPtr globName)*/
			
			resolvexcmdglobal ((ptrstring) args [0], &htable, bs);
			
			if (hashtablelookup (htable, bs, &val, &hnode)
				&& copyvaluerecord (val, &val)
				&& coercetostring (&val)
				&& enlargehandle (val.data.stringvalue, 1, zerostring)) {
				
				exemptfromtmpstack (&val);
				
				*out = (long) val.data.stringvalue;
				}
			else
				*out = 0L;
			
			break;
		
		case 0x13: /*void SetGlobal (StringPtr globName, Handle globValue)*/
			
			resolvexcmdglobal ((ptrstring) args [0], &htable, bs);
			
			initvalue (&val, stringvaluetype);
			
			if (!copyhandle ((Handle) args [1], &val.data.stringvalue))
				(*pxcb).result = xresFail;
			
			else {
				
				truncatecstringhandle (val.data.stringvalue); /*strip zero terminator*/
				
				if (!hashtableassign (htable, bs, val))
					disposevaluerecord (val, false);
				}
			
			break;
		
		case 0x06: /*void ZeroBytes (Ptr dstPtr, long longCount)*/
			clearbytes ((byte *) args [0], args [1]);
			
			break;
		
		
		/*  String Utilities  */
		case 0x1c: /*void ScanToReturn (Ptr *scanPtr)*/
			
			scantext (chreturn, (ptrbyte *) args [0]);
			
			/*** test: ++*(ptrbyte *) args [0]; /*point past return*/
			
			break;
		
		case 0x27: /*void ScanToZero (Ptr *scanPtr)*/
			
			scantext (chnul, (ptrbyte *) args [0]);
			
			break;
		
		case 0x1a: /*Boolean StringEqual (StringPtr str1, StringPtr str2)*/
			*out = (long) equalstrings ((ptrstring) args [0], (ptrstring) args [1]);
			
			break;
		
		case 0x03: { /*long StringLength (Ptr strPtr)*/
			
			*out = scanlength (chnul, (ptrbyte) args [0]);
			
			break;
			}
		
		case 0x04: { /*Ptr StringMatch (StringPtr pattern, Ptr target)*/
			
			tysearchparameters saveparams = searchparams;
			long ix = 0;
			long len;
			
			searchparams.flunicase = false;
			searchparams.flwholewords = false;
			searchparams.flregexp = false;
			copystring ((ptrstring) args [0], searchparams.bsfind);
			
			textsearch ((ptrbyte) args [1], infinity, &ix, &len);
			
			*out = (long) ((ptrbyte) args [1] + ix);
			
			searchparams = saveparams;
			
			break;
			}
		
		case 0x20: /*void ZeroTermHandle (Handle hndl)*/
			
			enlargehandle ((Handle) args [0], 1, zerostring);
			
			break;
		
		
		/*  String Conversions  */
		case 0x10: /*void BoolToStr (Boolean bool, StringPtr str)*/
			setbooleanvalue ((Boolean) args [0], &val);
			
			valtostring (&val, (ptrstring) args [1]);
			
			break;
		
		case 0x11: { /*void ExtToStr (extended num, StringPtr str)*/
			
			
				setdoublevalue (* (long double *) args [0], &val);
			
			
			valtostring (&val, (ptrstring) args [1]);
				
			break;
			}
		
		case 0x0d: /*void LongToStr (long posNum, StringPtr str)*/
			setlongvalue ((unsigned long) args [0], &val); /*11/6/92 dmb*/
			
			valtostring (&val, (ptrstring) args [1]);
			
			break;
		
		case 0x0f: { /*void NumToHex (long num, short nDigits, StringPtr str)*/
			
			long n = args [0];
			byte hex [16];
			
			if (args [1] <= 4)
				n = (short) n;
			
			numbertohexstring (n, hex); /*11/6/92 dmb*/
			
			deletestring (hex, 1, 2); /*remove the "0x"*/
			
			copystring (hex, (ptrstring) args [2]);
			
			break;
			}
		
		case 0x0e: /*void NumToStr (long num, StringPtr str)*/
			setlongvalue ((long) args [0], &val); /*11/6/92 dmb*/
			
			valtostring (&val, (ptrstring) args [1]);
			
			break;
		
		case 0x07: /*Handle PasToZero (StringPtr str)*/
			copystring ((ptrstring) args [0], bs);
			
			pushchar (chnul, bs);
			
			newtexthandle (bs, (Handle *) out);
			
			break;
		
		case 0x2b: /*void PointToStr (Point pt, StringPtr str)*/
			setpointvalue (*(Point *) args [0], &val);
			
			valtostring (&val, (ptrstring) args [1]);
			
			break;
		
		case 0x2c: /*void RectToStr (Rect *rct, StringPtr str)*/
			newheapvalue ((Rect *) args [0], longsizeof (Rect), rectvaluetype, &val);
			
			valtostring (&val, (ptrstring) args [1]);
			
			break;
		
		case 0x1b: /*void ReturnToPas (Ptr zeroStr, StringPtr pasStr)*/
			texttostring ((ptrbyte) args [0], scanlength (chreturn, (ptrbyte) args [0]), (ptrstring) args [1]);
			
			break;
		
		case 0x0b: /*Boolean StrToBool (StringPtr str)*/
			stringtoval ((ptrstring) args [0], booleanvaluetype, &val);
			
			*out = (long) val.data.flvalue;
			
			break;
		
		case 0x0c: { /*extended StrToExt (StringPtr str)*/
				
				stringtoval ((ptrstring) args [0], doublevaluetype, &val);
				
				*(double *) args [1] = **val.data.doublevalue;
				
			
			break;
			}
		
		case 0x09: /*long StrToLong (StringPtr str)*/
			stringtoval ((ptrstring) args [0], longvaluetype, &val);
			
			*out = abs (val.data.longvalue);
			
			break;
		
		case 0x0a: /*long StrToNum (StringPtr str)*/
			stringtoval ((ptrstring) args [0], longvaluetype, &val);
			
			*out = (long) val.data.longvalue;
			
			break;
		
		case 0x2d: /*void StrToPoint (StringPtr str, Point *pt)*/
			stringtoval ((ptrstring) args [0], pointvaluetype, &val);
			
			*(Point *) args [1] = val.data.pointvalue;
			
			break;
		
		case 0x2e: /*void StrToRect (StringPtr str, Rect *rct)*/
			stringtoval ((ptrstring) args [0], rectvaluetype, &val);
			
			*(Rect *) args [1] = **val.data.rectvalue;
			
			break;
		
		case 0x08: /*void ZeroToPas (Ptr zeroStr, StringPtr pasStr)*/
			texttostring ((ptrbyte) args [0], scanlength (chnul, (ptrbyte) args [0]), (ptrstring) args [1]);
			
			break;
		
		#if 0
		
		/*  Field Utilities  */
		case 0x16: /*Handle GetFieldByID (Boolean cardFieldFlag, short fieldID)*/
			newfilledhandle (zerostring, 1L, (Handle *) out);
			
			break;
		
		case 0x14: /*Handle GetFieldByName (Boolean cardFieldFlag, StringPtr fieldName)*/
			newfilledhandle (zerostring, 1L, (Handle *) out);
			
			break;
		
		case 0x15: /*Handle GetFieldByNum (Boolean cardFieldFlag, short fieldNum)*/
			newfilledhandle (zerostring, 1L, (Handle *) out);
			
			break;
		
		case 0x19: /*void SetFieldByID (Boolean cardFieldFlag, short fieldID, Handle fieldVal)*/
			break;
		
		case 0x17: /*void SetFieldByName (Boolean cardFieldFlag, StringPtr fieldName, Handle fieldVal)*/
			break;
		
		case 0x18: /*void SetFieldByNum (Boolean cardFieldFlag, short fieldNum, Handle fieldVal)*/
			break;
		
		case 0x2f: /*TEHandle GetFieldTE (Boolean cardFieldFlag, short fieldID, short fieldNum, StringPtr fieldNamePtr)*/
			*out = 0;
			
			break;
															
		case 0x30: /*void SetFieldTE (Boolean cardFieldFlag, short fieldID, short fieldNum, StringPtr fieldNamePtr, TEHandle fieldTE)*/
			break;
		
		#endif
		
		case 0x22: /*void SendHCEvent(EventRecord *event)*/
			
			/****component***/
			
			shellpostevent ((EventRecord *) &args [0]); /*yes, the event itself is in args*/
			
			shellforcebackgroundtask (); /*come back as soon as possible*/
			
			if (!processyield ()) /*we've been killed*/
				(*pxcb).result = xresFail;
			
			break;
		
		default:
			*out = 0L;
			
			(*pxcb).result = xresNotImp;
			
			break;
		}
	
	plangxcmdrec = pxcb; /*reset in case another xcmd ran in the background*/
	} /*xcmdcallback*/


