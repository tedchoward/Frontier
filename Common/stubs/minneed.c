
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

#ifdef MACVERSION
#include <Packages.h>
#include <ctype.h>
#include <standard.h>
#endif

#ifdef WIN95VERSION
#include "standard.h"
#endif

#include "memory.h"
#include "quickdraw.h"
#include "strings.h"
#include "ops.h"
#include "resources.h"
#include "shell.rsrc.h"
#include "langinternal.h"


#if 0 // (odbengine==1)
static byte bshexprefix [] = "0x";


#define ctparseparams 4
byte zerostring [] = "\0"; /*use this to conserve constant space*/

void uppertext (ptrbyte ptext, long ctchars) {
	
	register long n = ctchars;
	register ptrbyte p = ptext;
	register byte ch;
	
	while (--n >= 0) {
		
		ch = *p;
		
		if ((ch >= 'a') && (ch <= 'z'))
			*p -= 32;
		
		p++;
		} /*while*/
	} /*uppertext*/


void lowertext (ptrbyte ptext, long ctchars) {
	
	register long n = ctchars;
	register ptrbyte p = ptext;
	register byte ch;
	
	while (--n >= 0) {
		
		ch = *p;
		
		if ((ch >= 'A') && (ch <= 'Z'))
			*p += 32;
		
		p++;
		} /*while*/
	} /*lowertext*/


void allupper (bigstring bs) {
	
	uppertext (bs + 1, *bs);
	} /*allupper*/


void alllower (bigstring bs) {
	
	lowertext (bs + 1, *bs);
	} /*alllower*/
	


void copystring (bigstring bssource, bigstring bsdest) {

	/*
	create a copy of bssource in bsdest.  copy the length byte and
	all the characters in the source string.

	assume the strings are pascal strings, with the length byte in
	the first character of the string.
	*/

	register short i, len;
	
	if (bssource == nil) { /*special case, handled at lowest level*/
		
		setemptystring (bsdest);
		
		return;
		}
	
	len = (short) stringlength(bssource);
	CopyMemory (bsdest, bssource, len+1);
//	for (i = 0; i <= len; i++) 
//		bsdest [i] = bssource [i];
	} /*copystring*/

void bytestohexstring (ptrvoid pdata, long ctbytes, bigstring bshex) {
	
	register byte *p = pdata;
	register byte x;
	register short ct = ctbytes;
	byte bsbyte [3];
	static byte hex [16] = "0123456789ABCDEF";
	
	setemptystring (bshex);
	
	setstringlength (bsbyte, 2);
	
	ct = min (ctbytes, lenbigstring / 2 - 2); /*leave room for hex prefix*/
	
	while (--ct >= 0) {
		
		x = *p++;
		
		bsbyte [1] = hex [(x >> 4) & 0x000F];
		
		bsbyte [2] = hex [x & 0x000F];
		
		pushstring (bsbyte, bshex);
		}
	
	insertstring (bshexprefix, bshex);
	} /*bytestohexstring*/

#endif

#define dberrorlist 256

static byte * dberrorstrings [] = {
		/* [1] */
		STR_File_was_created_by_an_incompatible_version_of_this_program,
		/* [2] */
		STR_Internal_error_attempted_to_read_a_free_block
	};


static byte * langmiscstrings [] = {
		/* [1] */
		STR_unknown,
		/* [2] */
		STR_error
	};


static byte * stacknames [] = {
		/* [1] */
		STR_hash_table
	};


static byte * langerrorstrings [] = {
		/* [1] */
		STR_Cant_delete_XXX_because_it_hasnt_been_defined,
		/* [2] */
		STR_Stack_overflow_XXX_stack,
		/* [3] */
		STR_The_name_XXX_hasnt_been_defined,
		/* [4] */
		STR_Address_value_doesnt_refer_to_a_valid_table
	};


#if 0 // (odbengine==1)

boolean equalstrings (bigstring bs1, bigstring bs2) {

	/*
	return true if the two strings (pascal type, with length-byte) are
	equal.  return false otherwise.
	*/

	register ptrbyte p1 = bs1, p2 = bs2;
	register short ct = *p1 + 1;
	
	if (*p1 != *p2) /*different lengths*/
		return (false);
	
	while (ct--) 
		
		if (*p1++ != *p2++)
		
			return (false);
		
	return (true); /*loop terminated*/
	} /*equalstrings*/

short comparestrings (bigstring bs1, bigstring bs2) {

	/*
	3.0.2b1 dmb: use std c lib as much as possible
	*/
	
	register short len1 = stringlength (bs1);
	register short len2 = stringlength (bs2);
	register short n;
	
	n = memcmp ((char *) bs1 + 1, (char *) bs2 + 1, min (len1, len2));
	
	if (n == 0)
		n = sgn (len1 - len2);
	
	return (n);
	} /*comparestrings*/


boolean stringlessthan (register bigstring bs1, register bigstring bs2) {
	
	return (comparestrings (bs1, bs2) < 0);
	} /*stringlessthan*/

	
boolean pushstring (bigstring bssource, bigstring bsdest) {

	/*
	insert the source string at the end of the destination string.
	
	assume the strings are pascal strings, with the length byte in
	the first character of the string.
	
	return false if the resulting string would be too long.
	*/
	
	register short lensource = stringlength (bssource);
	register short lendest = stringlength (bsdest);
	register byte *psource, *pdest;
	
	if ((lensource + lendest) > lenbigstring) /*resulting string would be too long*/
		return (false);
		
	CopyMemory (bsdest + lendest, bssource, lensource+1);
	
//	psource = bssource + 1;
	
	setstringlength(bsdest,  lendest + lensource);
	
//	while (lensource--) *pdest++ = *psource++;
	
	return (true);
	} /*pushstring*/


boolean deletestring (bigstring bs, short ixdelete, short ctdelete) {
	
	/*
	delete ct chars in the indicated string, starting with the character
	at offset ix.
	*/
	
	register short ix = ixdelete;
	register short ct = ctdelete;
	register short len = stringlength (bs);
	register long ctmove;
	register ptrbyte pfrom, pto;		
	
	if ((ix > len) || (ix < 0))
		return (false);
		
	if (ct <= 0)
		return (ct == 0);
		
	ctmove = len - ix - ct + 1;
	 
	if (ctmove > 0) {
		
		pfrom = bs + ix + ct;
		
		pto = bs + ix;
		
		moveleft (pfrom, pto, ctmove);
		}
	
	setstringlength (bs, len - ct);
	
	return (true);
	} /*deletestring*/


boolean deletefirstchar (bigstring bs) {	
	
	return (deletestring (bs, 1, 1));
	} /*deletefirstchar*/
	
	
short popleadingchars (bigstring bs, byte ch) {
	
	/*
	2.1b1 dmb: return the number of characters popped
	*/
	
	register short len = stringlength (bs);
	register short i;
	
	for (i = 1; i <= len; i++) {
		
		if (bs [i] != ch) {
			
			deletestring (bs, 1, --i);
			
			return (i);
			}
		} /*for*/
		
	setemptystring (bs);
	
	return (len);
	} /*popleadingchars*/


boolean pushchar (byte ch, bigstring bs) {
	
	/*
	insert the character at the end of a pascal string.
	*/
	
	register short len;
	
	len = stringlength(bs); 
	
	if (len >= lenbigstring)
		return (false);
	
	setstringcharacter(bs, len, ch);
	
	setstringlength(bs, len+1);
	
	return (true);
	} /*pushchar*/
	
	
boolean pushspace (bigstring bs) {

	/*
	an oft-repeated function, add a space at the end of bs.
	*/
	
	return (pushchar (chspace, bs));
	} /*pushspace*/
	
	
boolean pushlong (long num, bigstring bsdest) {

	bigstring bsint;
	
	numbertostring (num, bsint);
	
	return (pushstring (bsint, bsdest));
	} /*pushlong*/
	

boolean pushint (short num, bigstring bsdest) {
	
	return (pushlong ((long) num, bsdest));
	} /*pushint*/

boolean insertstring (bigstring bssource, bigstring bsdest) {
	
	/*
	insert the source string at the beginning of the destination string.
	
	return false if the resulting string would be longer than 255 chars.
	*/
	
	register short len1 = stringlength (bssource), len2 = stringlength (bsdest);
	bigstring bs;
	
	if ((len1 + len2) > lenbigstring) /*resulting string would be too long*/
		return (false);
		
	copystring (bssource, bs);
	
	pushstring (bsdest, bs);
	
	copystring (bs, bsdest);
	
	return (true);
	} /*insertstring*/


boolean insertchar (byte ch, bigstring bsdest) {
	
	register byte *pdest = bsdest;
	register short len = stringlength (pdest);
	
	if (len == lenbigstring)
		return (false);
	
	moveright (pdest + 1, pdest + 2, len);
	
	setstringlength (pdest, len + 1);
	
	pdest [1] = ch;
	
	return (true);
	} /*insertchar*/


void midstring (bigstring bssource, short ix, short len, bigstring bsdest) {
	
	/*
	2.1b2 dmb: to make calling easier, protect again negative lengths
	*/
	
	if (len <= 0)
		setemptystring (bsdest);
	
	else {
		
		setstringlength (bsdest, len);
		
		moveleft (&bssource [ix], &bsdest [0], (long) len);
		}
	} /*midstring*/


boolean scanstring (byte ch, bigstring bs, short *ix) {
	
	/*
	return in ix the index in the string of the first occurence of chscan.
	
	return false if it wasn't found, true otherwise.
	
	dmb 10/26/90: p is now initialized correctly to bs + i, not bs + 1
	*/
	
	register short i;
	register ptrbyte p;
	register byte c = ch;
	register short len = stringlength (bs);
	
	for (i = *ix, p = bs + i; i < len; i++) 
		
		if (*p++ == c) {
			
			*ix = i;
			
			return (true);
			}
			
	return (false);
	} /*scanstring*/


boolean stringfindchar (byte ch, bigstring bs) {
	
	/*
	simpler entrypoint for scanstring when caller just wants 
	to know if ch appears anywhere in bs
	*/
	
	short ix = 1;
	
	return (scanstring (ch, bs, &ix));
	} /*stringfindchar*/


boolean stringreplaceall (char ch1, char ch2, bigstring bs) {
	
	/*
	replace all instances of ch1 in bs with ch2
	*/
	
	short ix = 1;
	
	while (scanstring (ch1, bs, &ix))
		bs [ix] = ch2;
	
	return (true);
	} /*stringreplaceall*/


boolean textlastword (ptrbyte ptext, long lentext, byte chdelim, bigstring bsdest) {
	
	/*
	copy the last word from bs, and put it into bsdest.
	
	search backwards from the end of the source string until you find
	chdelim.
	*/
	
	register long len = lentext;
	register long i;
	register byte ch = chdelim;
	
	for (i = len; i > 0; i--) {
		
		if (ptext [i - 1] == ch)
			break;
		} /*for*/
	
	texttostring (ptext + i, len - i, bsdest);
	
	return (true);
	} /*textlastword*/


boolean textfirstword (ptrbyte ptext, long lentext, byte chdelim, bigstring bsdest) {
	
	/*
	copy the first word from bs, and put it into bsdest.
	
	search forwards from the beginning of the source string until you 
	find chdelim.
	*/
	
	register long len = lentext;
	register long i;
	register byte ch = chdelim;
	
	for (i = 0; i < len; i++) {
		
		if (ptext [i] == ch)
			break;
		} /*for*/
	
	texttostring (ptext, i, bsdest);
	
	return (true);
	} /*textfirstword*/


boolean textnthword (ptrbyte ptext, long lentext, long wordnum, byte chdelim, boolean flstrict, long *ixword, long *lenword) {
	
	/*
	6/10/91 dmb: a single word preceeded or followed by chdelim should be 
	counted as just one word
	
	8/7/92 dmb: added flstrict parameter. when set, every delimiter starts a new word, 
	even consecutive characters, possibly yielding empty words
	*/
	
	register long len = lentext;
	register long ix;
	register long ixlastword;
	register long ctwords = 1;
	register byte ch = chdelim;
	boolean fllastwasdelim = true;
	
	ix = 0;
	
	ixlastword = 0;
	
	while (true) { /*scan string*/
		
		if (ix >= len) /*reached end of string*/
			break;
		
		if (ptext [ix] == ch) { /*at a delimiter*/
			
			if (ix == len - 1) /*trailing delimiter, don't bump word count*/
				break;
			
			if (flstrict || !fllastwasdelim) {
				
				if (ctwords >= wordnum) /*we've found the end of the right word*/
					break;
				
				ctwords++;
				}
			
			ixlastword = ix + 1; /*next word starts after the delimiter*/
			
			fllastwasdelim = true;
			}
		else
			fllastwasdelim = false;
		
		ix++; /*advance to next character*/
		} /*while*/
	
	/*
	texttostring (ptext + ixlastword, ix - ixlastword, bsword);
	
	if (isemptystring (bsword) && !flstrict)
		return (false);
	*/
	
	*ixword = ixlastword;
	
	*lenword = ix - ixlastword;
	
	if ((*lenword == 0) && !flstrict)
		return (false);
	
	return (ctwords == wordnum);
	} /*textnthword*/


long textcountwords (ptrbyte ptext, long lentext, byte chdelim, boolean flstrict) {
	
	/*
	8/7/92 dmb: added flstrict parameter
	*/
	
	register long wordnum = 1;
//	bigstring bsword;
	long ixword;
	long lenword;
	
	while (true) {
		
		if (!textnthword (ptext, lentext, wordnum, chdelim, flstrict, &ixword, &lenword))
			return (wordnum - 1);
		
		wordnum++;
		} /*while*/
	} /*textcountwords*/


boolean lastword (bigstring bssource, byte chdelim, bigstring bsdest) {
	
	return (textlastword (bssource + 1, stringlength (bssource), chdelim, bsdest));
	} /*lastword*/
	

void poplastword (bigstring bs, byte chdelim) {
	
	bigstring bsword;
	
	if (lastword (bs, chdelim, bsword)) 
	
		setstringlength (bs, stringlength (bs) - stringlength (bsword));
	} /*poplastword*/


boolean firstword (bigstring bssource, byte chdelim, bigstring bsdest) {
	
	return (textfirstword (bssource + 1, stringlength (bssource), chdelim, bsdest));
	} /*firstword*/


boolean nthword (bigstring bs, short wordnum, byte chdelim, bigstring bsword) {
	
	long ixword;
	long lenword;
	
	if (!textnthword (bs + 1, stringlength (bs), wordnum, chdelim, false, &ixword, &lenword))
		return (false);
	
	texttostring (bs + 1 + ixword, lenword, bsword);
	
	return (true);
	} /*nthword*/


short countwords (bigstring bs, byte chdelim) {
	
	return (textcountwords (bs + 1, stringlength (bs), chdelim, false));
	} /*countwords*/


void commentdelete (byte chdelim, bigstring bs) {
	
	/*
	scan the string from left to right.  if we find a comment delimiting
	character, delete all the characters to the right, including the
	delimiter and return.
	
	10/30/91 dmb: ignore comment characters that are embedded in quoted strings
	
	5/4/93 dmb: handle single-quoted strings too
	
	3.0.2b1 dmb: handle escape sequences. note that we don't have to deal with 
	the 0x## form because ## can never be quote characters, and thus can't 
	confuse our string parsing
	*/
	
	register byte chcomment = chdelim;
	register short i, ct;
	register byte ch;
	boolean flinstring = false;
	boolean flinescapesequence = false;
	byte chendstring;
	
	ct = stringlength (bs);
	
	for (i = 1; i <= ct; i++) {
		
		ch = bs [i];
		
		if (flinstring) {
			
			if (flinescapesequence)
				flinescapesequence = false; /*it's been consumed*/
			
			else {
				
				if (ch == (byte) '\\')
					flinescapesequence = true;
				else
					flinstring = ch != chendstring;
				}
			}
		
		else if (ch == (byte) '"') {
			
			flinstring = true;
			
			chendstring = '"';
			}
		
		else if (ch == (byte) 'Ò') {
			
			flinstring = true;
			
			chendstring = 'Ó';
			}
		
		else if (ch == (byte) '\'') {
			
			flinstring = true;
			
			chendstring = '\'';
			}
		
		else if (ch == chcomment) {
			
			setstringlength (bs, i - 1);
			
			return;
			}
		}
	} /*commentdelete*/


boolean whitespacechar (byte ch) {
	
	return ((ch == ' ') || (ch == '\t') || (ch == '\n') || (ch == '\r'));
	} /*whitespacechar*/
	
	
boolean poptrailingwhitespace (bigstring bs) {
	
	/*
	return true if there were trailing "whitespace" characters to be popped.
	*/
	
	register short i, ct;
	
	ct = stringlength (bs);
	
	for (i = ct; i > 0; i--)
	
		if (!whitespacechar (bs [i])) { /*found a non-blank character*/
			
			setstringlength (bs, i);
			
			return (i < ct);
			}
	
	setemptystring (bs);
	
	return (true); /*string is all blank*/
	} /*poptrailingwhitespace*/
	
	
boolean firstsentence (bigstring bs) { 
	
	/*
	pops all characters after the first period followed by a space.
	
	return true if any chars were popped.
	*/
	
	register short i;
	register short len = stringlength (bs);
	
	for (i = 1; i <= len; i++) {
		
		if (bs [i] == '.') {
			
			if (i == len) /*no next character, no chars to pop*/
				return (false);
				
			if (whitespacechar (bs [i+1])) { /*truncate length and return*/
				
				setstringlength (bs, i);
				
				return (true);
				}
			}
		} /*for*/
		
	return (false); /*no chars popped*/
	} /*firstsentence*/


	
boolean capitalizefirstchar (bigstring bs) {
	
	register char ch;
	
	if (stringlength (bs) == 0)
		return (true);
		
	ch = bs [1];
	
	if ((ch < 'a') || (ch > 'z'))
		return (false);
		
	bs [1] = ch - 32;
	
	return (true);
	} /*capitalizefirstchar*/
	
	
boolean isallnumeric (bigstring bs) {
	
	/*
	11/6/92 dmb: allow first character to be a sign instead of a digit
	*/
	
	register short ct = stringlength (bs);
	register ptrbyte p = &bs [1];
	register byte ch;
	
	while (--ct >= 0) {
		
		ch = *p++;
		
		if (!isnumeric (ch)) {
			
			if (ct == stringlength (bs) - 1) { /*checking first character*/
				
				if ((ch == '-') || (ch == '+')) /*sign character -- it's cool*/
					continue;
				}
			
			return (false);
			}
		} /*while*/
		
	return (true); /*composed entirely of numeric chars*/
	} /*isallnumeric*/

void filledstring (byte ch, short ct, bigstring bs) {
	
	if (ct < 0)
		ct = 0;
		
	setstringlength(bs, ct);
	
	fillchar (bs, (long) ct, ch);
	} /*filledstring*/
	
void copyheapstring (hdlstring hsource, bigstring bsdest) {
	
	/*
	a safe way of copying a string out of the heap into a stack-allocated or
	global string.
	*/
	
	register hdlstring h = hsource;
	
	if (h == nil) { /*nil handles are empty strings*/
		
		setemptystring (bsdest);
		
		return;
		}
	
	HLock ((Handle) h);
	
	copystring (*h, bsdest);
	
	HUnlock ((Handle) h);
	} /*copyheapstring*/
	
void convertpstring (bigstring bs) {
	
	/*
	convert a pascal string to a c string.
	*/
	
	register short len;
	
	len = (short) bs [0];
	
	moveleft (&bs [1], &bs [0], (long) len);
	
	bs [len] = 0; /*dmb 8/1/90:  shouldn't add 1 to len here*/
	} /*convertpstring*/
	
	
void convertcstring (bigstring bs) {
	
	/*
	convert a c string to a pascal string.
	*/
	
	register ptrbyte p;
	register short len = 0;
	
	p = &bs [0]; /*point at the first byte in c string*/
	
	while (*p++) len++;
	
	moveright (&bs [0], &bs [1], (long) len);
	
	bs [0] = (byte) len;
	} /*convertcstring*/
#endif

#if 0 // (odbengine==1)

#ifdef fldebug
	
	void shellinternalerrormessage (bigstring bs) {
		
		DebugStr (bs);
		} /*shellinternalerrormessage*/

#else
	
	void shellinternalerrormessage (short n) {
		} /*shellinternalerrormessage*/

#endif
#endif

#if 0 // (odbengine==1)

void ostypetostring (OSType type, bigstring bs) {
	
	setstringlength (bs, sizeof (OSType));
	
	moveleft (&type, &bs [1], longsizeof (OSType));
	} /*ostypetostring*/
	
	
void parsedialogstring (bigstring bssource, bigstring bs0, bigstring bs1, bigstring bs2, bigstring bs3, bigstring bsresult)
	{
	}
	
#endif

#if 0 // (odbengine==1)
void memoryerror ()
	{
	}
#endif

#ifdef WIN95VERSION
void recttowinrect (Rect * rIn, RECT * rOut) {
	rOut->left = rIn->left;
	rOut->top = rIn->top;
	rOut->right = rIn->right;
	rOut->bottom = rIn->bottom;
	}

void winrecttorect (RECT * rIn, Rect * rOut) {
	rOut->left = (short)rIn->left;
	rOut->top = (short)rIn->top;
	rOut->right = (short)rIn->right;
	rOut->bottom = (short)rIn->bottom;
	}
#endif

#ifdef PASCALSTRINGVERSION
#ifdef WIN95VERSION
char * pwstringbaseaddress(char * bs) {
	bs[bs[0]+1]=0;
	return (bs+1);
	}

void pwsetstringlength(char * bs, short len) {
	bs[0]=(char)len;
//	bs[len+1] = 0;
	}
#endif
#endif



