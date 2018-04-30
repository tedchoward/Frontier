
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


#include "font.h"
#include "memory.h"
#include "quickdraw.h"
#include "strings.h"
#include "ops.h"
#include "resources.h"
#include "shell.rsrc.h"
#include "tablestructure.h"
#include "timedate.h"
#include "langinternal.h" /* 2006-02-26 creedon */
#include "byteorder.h"	/* 2006-04-08 aradke: endianness conversion macros */


#define ctparseparams 4

#define hextoint(ch) (isdigit(ch)? (ch - '0') : (getlower (ch) - 'a' + 10))
	
#define stringerrorlist 263

/*
	constants related to text-conversion functions
*/
#define cs_utf8			BIGSTRING( "\xA5" "utf-8" )
#define cs_utf16		BIGSTRING( "\x06" "utf-16" )
#define cs_iso88591		BIGSTRING( "\xA0" "iso-8859-1" )
#define cs_macintosh	BIGSTRING( "\x09" "macintosh" )


byte zerostring [] = "\0"; /*use this to conserve constant space*/

unsigned char lowercasetable[256];

static hdlstring parseparams [ctparseparams] = {nil, nil, nil, nil};

static hdlstring dirstrings [ctdirections];

static byte bshexprefix [] = STR_hexprefix;


/* declarations */

static boolean converttextencoding( Handle, Handle, const long, const long, long * );
static boolean getTextEncodingIDFromIANA( bigstring, long * );



/* definitions */

boolean equalstrings (const bigstring bs1, const bigstring bs2) {

	/*
	return true if the two strings (pascal type, with length-byte) are
	equal.  return false otherwise.
	*/

	register ptrbyte p1 = (ptrbyte) stringbaseaddress (bs1);
	register ptrbyte p2 = (ptrbyte) stringbaseaddress (bs2);
	register short ct = stringlength (bs1);
	
	if (ct != stringlength (bs2)) /*different lengths*/
		return (false);
	
	while (--ct >= 0) 
		
		if (*p1++ != *p2++)
		
			return (false);
		
	return (true); /*loop terminated*/
	} /*equalstrings*/


boolean equaltextidentifiers (byte * string1, byte * string2, short len) {
	while (--len >= 0) 
		
		if (getlower (*string1++) != getlower (*string2++))
		
			return (false);
	
	return (true); /*loop terminated*/
	} /*equaltextidentifiers*/


boolean equalidentifiers (const bigstring bs1, const bigstring bs2) {
	
	register long ct = stringlength (bs1);
	
	if (ct != stringlength (bs2)) /*different lengths*/

		return (false);

	else {

		register ptrbyte p1 = ((ptrbyte) bs1) + ct;
		register ptrbyte p2 = ((ptrbyte) bs2) + ct;

		while (--ct >= 0) 

			if (getlower (*p1--) != getlower (*p2--))
			
				return (false);
		}
	
	return (true); /*loop terminated*/
	} /*equalidentifiers*/



short comparestrings (bigstring bs1, bigstring bs2) {

	/*
	3.0.2b1 dmb: use std c lib as much as possible
	*/
	
	register short len1 = stringlength (bs1);
	register short len2 = stringlength (bs2);
	register short n;
	
	n = memcmp (stringbaseaddress (bs1), stringbaseaddress (bs2), min (len1, len2));
	
	if (n == 0)
		n = sgn (len1 - len2);

	/*
	AR 2004-08-24: memcmp is not guaranteed to return -1 || 0 || +1,
	but some of our callers expected -1 as the only negative value,
	make it so...
	*/
	if (n < -1)
		return (-1);
	
	return (n);
	} /*comparestrings*/

/*
short comparestrings (bigstring bs1, bigstring bs2) {

	#*
	return zero if the two strings (pascal type, with length-byte) are
	equal.  return -1 if bs1 is less than bs2, or +1 if bs1 is greater than
	bs2.
	
	use the Machintosh international utility routine IUCompString, which 
	performs dictionary string comparison and accounts for accented characters
	%/
	
	return (IUCompString (bs1, bs2));
	} #*comparestrings*/



short compareidentifiers (bigstring bs1, bigstring bs2) {

	/*
	2004-11-09 aradke: cross between comparestrings and equalidentifiers,
	useful for sorting identifiers.

	return zero if the two strings (pascal type, with length-byte) are
	equal.  return -1 if bs1 is less than bs2, or +1 if bs1 is greater than
	bs2. the comparison is not case-sensitive.
	*/
	
	register short n = min (stringlength (bs1), stringlength (bs2)) + 1;
	register ptrbyte p1 = (ptrbyte) stringbaseaddress (bs1);
	register ptrbyte p2 = (ptrbyte) stringbaseaddress (bs2);
	
	while (--n)
		if (getlower (*p1++) != getlower (*p2++))
			return ((getlower (*--p1) < getlower (*--p2)) ? -1 : +1); /*rewind*/
	
	return (sgn (stringlength (bs1) - stringlength (bs2)));
	} /*compareidentifiers*/


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
		
	pdest = stringbaseaddress (bsdest) + lendest;
	
	psource = stringbaseaddress (bssource);
	
	setstringlength (bsdest, lendest + lensource);
	
	moveleft (psource, pdest, lensource);
	
	return (true);
	} /*pushstring*/


boolean deletestring (bigstring bs, short ixdelete, short ctdelete) {
	
	/*
	delete ct chars in the indicated string, starting with the character
	at 1-based offset ix.
	*/
	
	register short len = stringlength (bs);
	register long ctmove;
	register ptrbyte pfrom, pto;		
	
	if ((ixdelete > len) || (ixdelete < 1))
		return (false);
		
	if (ctdelete <= 0)
		return (ctdelete == 0);
		
	--ixdelete;	// make zero-based

	ctmove = len - ixdelete - ctdelete;
	 
	if (ctmove > 0) {
		
		pfrom = stringbaseaddress (bs) + ixdelete + ctdelete;
		
		pto = stringbaseaddress (bs) + ixdelete;
		
		moveleft (pfrom, pto, ctmove);
		}
	
	setstringlength (bs, len - ctdelete);
	
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
	
	for (i = 0; i < len; i++) {
		
		if (getstringcharacter (bs, i) != ch) {
			
			deletestring (bs, 1, i);
			
			return (i);
			}
		} /*for*/
		
	setemptystring (bs);
	
	return (len);
	} /*popleadingchars*/


short poptrailingchars (bigstring bs, byte ch) {
	
	/*
	5.1.3 dmb: pop the trailing characters, and return the new length
	*/
	
	while (!isemptystring (bs) && (lastchar (bs) == ch))
		setstringlength (bs, stringlength (bs) - 1);
	
	return (stringlength (bs));
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

/*
pushboolean (boolean flboo, bigstring bsdest) {
	
	bigstring bsboo;
	
	if (flboo)
		copystring ((ptrstring) "\ptrue", bsboo);
	else
		copystring ((ptrstring) "\pfalse", bsboo);
		
	pushstring (bsboo, bsdest);
	} #*pushboolean*/
	
	
/*
pushstringresource (short listnum, short resnum, bigstring bs) {
	
	bigstring bsres;
	
	GetIndString (bsres, listnum, resnum);
	
	pushstring (bsres, bs);
	} #*pushstringresource*/
	
	
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
	
	moveright (stringbaseaddress (pdest), stringbaseaddress (pdest) + 1, len);
	
	setstringlength (pdest, len + 1);
	
	setstringcharacter (pdest, 0, ch);
	
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
		
		moveleft (stringbaseaddress (bssource) + ix - 1, stringbaseaddress (bsdest), (long) len);
		}
	} /*midstring*/


boolean textfindreplace (Handle hfind, Handle hreplace, Handle hsearch, boolean flreplaceall, boolean flunicase) {

	/*
	dmb: this could be upgraded to use a handlestream
	
	6.1d4 AR: Updated to use a handlestream as per dmb's suggestion

	6.1d7 AR: Updated to handle unicase searching. Minor optimizations.
	*/
	
	long ctfind, ctreplace;
	handlestream s;
	
	openhandlestream (hsearch, &s);
	
	lockhandle (hreplace);
	
	ctreplace = gethandlesize (hreplace);
	
	ctfind = gethandlesize (hfind);
	
	while (true) {
		
		s.pos = flunicase
			? searchhandleunicase (hsearch, hfind, s.pos, s.eof)
			: searchhandle (hsearch, hfind, s.pos, s.eof);

		if (s.pos < 0) /*no match found*/
			break;

		if (!mergehandlestreamdata (&s, ctfind, *hreplace, ctreplace)) {
			
			closehandlestream (&s);

			unlockhandle (hreplace);
			
			return (false); //usually an out of memory error
			}
		
		if (!flreplaceall)
			break;
		}
	
	unlockhandle (hreplace);
	
	closehandlestream (&s);

	return (true);
	} /*textfindreplace*/


boolean stringfindreplace (bigstring bsfind, bigstring bsreplace, Handle hsearch, boolean flreplaceall, boolean flunicase) {

	/*
	6.1b6 AR: Wrapper for textfindreplace. I got tired of replicating this functionality.
	*/

	Handle hfind = nil;
	Handle hreplace = nil;
	boolean fl;

	fl = newtexthandle (bsfind, &hfind);

	fl = fl && newtexthandle (bsreplace, &hreplace);

	fl = fl && textfindreplace (hfind, hreplace, hsearch, flreplaceall, flunicase);

	disposehandle (hfind);

	disposehandle (hreplace);

	return (fl);
	}/*stringfindreplace*/


boolean dropnonalphas (bigstring bs) {


	/*
	5.1.4 dmb: strip non alphas and return true if the string isn't empty
	*/
	
	register short ct = stringlength (bs);
	
	while (--ct >= 0) {
		
		if (!isalnum (getstringcharacter (bs, ct)))
			deletestring (bs, ct + 1, 1);
		}
	
	return (!isemptystring (bs));
	} /*dropnonalphas*/


boolean streamdropnonalphas (handlestream *s) {


	/*
	5.1.4 dmb: we needed this two places, to here it is
	*/
	
	for ((*s).pos = 0; (*s).pos < (*s).eof; ) {
		
		if (isalnum ((*(*s).data) [(*s).pos]))
			++(*s).pos;
		else
			pullfromhandlestream (s, 1, nil);
		}
	
	return (true);
	} /*streamdropnonalphas*/


boolean scanstring (byte ch, bigstring bs, short *ix) {
	
	/*
	return in ix the index in the string of the first occurence
	of chscan.
	
	return false if it wasn't found, true otherwise.
	
	dmb 10/26/90: p is now initialized correctly to bs + i, not bs + 1
	
	dmb 1/22/97: using new string macros, be careful to preserve 1-basedness
	*/
	
	register short i;
	register ptrbyte p;
	register byte c = ch;
	register short len = stringlength (bs);
	
	for (i = *ix - 1, p = stringbaseaddress (bs) + i; i < len; i++) 
		
		if (*p++ == c) {
			
			*ix = i + 1;
			
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
	
	5.0d14 dmb: scanstring is 1-based, setstringcharacter is 0-based.
	*/
	
	short ix = 1;
	
	while (scanstring (ch1, bs, &ix))
		setstringcharacter (bs, ix - 1, ch2);
	
	return (true);
	} /*stringreplaceall*/


boolean stringswapall (char ch1, char ch2, bigstring bs) {

	/*
	2009-08-30 aradke: replace all instances of ch1 with ch2 and vice versa, useful for converting posix file paths
	*/
	
	int ct = stringlength(bs);
	unsigned char* p = stringbaseaddress(bs);
	
	while (ct > 0) {
	
		if (*p == ch1)
			*p = ch2;
		else if (*p == ch2)
			*p = ch1;
		
		p++;
		ct--;
		}
	
	return (true);
	} /*stringswapall*/


boolean textlastword (ptrbyte ptext, long len, byte chdelim, bigstring bsdest) {
	
	/*
	copy the last word from bs, and put it into bsdest.
	
	search backwards from the end of the source string until you find
	chdelim.
	*/
	
	register long i;
	
	for (i = len; i > 0; i--) {
		
		if (ptext [i - 1] == chdelim)
			break;
		} /*for*/
	
	texttostring (ptext + i, len - i, bsdest);
	
	return (true);
	} /*textlastword*/


boolean textfirstword (ptrbyte ptext, long len, byte chdelim, bigstring bsdest) {
	
	/*
	copy the first word from bs, and put it into bsdest.
	
	search forwards from the beginning of the source string until you 
	find chdelim.
	*/
	
	register long i;
	
	for (i = 0; i < len; i++) {
		
		if (ptext [i] == chdelim)
			break;
		} /*for*/
	
	texttostring (ptext, i, bsdest);
	
	return (true);
	} /*textfirstword*/


boolean textnthword (ptrbyte ptext, long len, long wordnum, byte chdelim, boolean flstrict, long *ixword, long *lenword) {
	
	/*
	6/10/91 dmb: a single word preceeded or followed by chdelim should be 
	counted as just one word
	
	8/7/92 dmb: added flstrict parameter. when set, every delimiter starts a new word, 
	even consecutive characters, possibly yielding empty words
	*/
	
	register long ix;
	register long ixlastword;
	long ctwords = 1;
	boolean fllastwasdelim = true;
	
	ix = 0;
	
	ixlastword = 0;
	
	while (true) { /*scan string*/
		
		if (ix >= len) /*reached end of string*/
			break;
		
		if (ptext [ix] == chdelim) { /*at a delimiter*/
			
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
	
	return (textlastword (stringbaseaddress (bssource), stringlength (bssource), chdelim, bsdest));
	} /*lastword*/
	

void poplastword (bigstring bs, byte chdelim) {
	
	bigstring bsword;
	
	if (lastword (bs, chdelim, bsword)) 
	
		setstringlength (bs, stringlength (bs) - stringlength (bsword));
	} /*poplastword*/


boolean firstword (bigstring bssource, byte chdelim, bigstring bsdest) {
	
	return (textfirstword (stringbaseaddress (bssource), stringlength (bssource), chdelim, bsdest));
	} /*firstword*/


boolean nthword (bigstring bs, short wordnum, byte chdelim, bigstring bsword) {
	
	long ixword;
	long lenword;
	
	if (!textnthword (stringbaseaddress (bs), stringlength (bs), wordnum, chdelim, false, &ixword, &lenword))
		return (false);
	
	texttostring (stringbaseaddress (bs) + ixword, lenword, bsword);
	
	return (true);
	} /*nthword*/


boolean nthfield (bigstring bs, short fieldnum, byte chdelim, bigstring bsfield) {
	
	/*
	5.0.2b19 dmb: just like nthword, but flstrict
	*/
	
	long ixfield;
	long lenfield;
	
	if (!textnthword (stringbaseaddress (bs), stringlength (bs), fieldnum, chdelim, true, &ixfield, &lenfield))
		return (false);
	
	texttostring (stringbaseaddress (bs) + ixfield, lenfield, bsfield);
	
	return (true);
	} /*nthfield*/


short countwords (bigstring bs, byte chdelim) {
	
	return (textcountwords (stringbaseaddress (bs), stringlength (bs), chdelim, false));
	} /*countwords*/


boolean textcommentdelete (Handle x) {
	
	/*
	5.0.2b15 dmb: moved guts out of commentdeleteverb. shorten the handle to 
	omit any comments -- ignoring URLs. (smarter than bigstring commentdelete.)
	*/
	
	Handle hcomment;
	byte ch = chcomment;
	long ixcomment, ix2;
	
	//scan for doubleslash
	if (!newtexthandle (BIGSTRING ("\x02//"), &hcomment))
		return (false);
	
	ixcomment = searchhandle (x, hcomment, 0, longinfinity);
	
	while (ixcomment > 0) { //watch out for urls
		
		if ((*x) [ixcomment - 1] != ':')
			break;
		
		ixcomment = searchhandle (x, hcomment, ixcomment + 2, longinfinity);
		}
	
	//scan for Mac comment
	sethandlecontents (&ch, 1L, hcomment);
	
	if (ixcomment >= 0) {
	
		ix2 = searchhandle (x, hcomment, 0, ixcomment);
		
		if (ix2 >= 0)
			ixcomment = ix2;
		}
	else
		ixcomment = searchhandle (x, hcomment, 0, longinfinity);
	
	if (ixcomment >= 0)
		sethandlesize (x, ixcomment);
	
	disposehandle (hcomment);			//RAB: 1/27/98, clean up after one self
	
	return (true);
	} /*textcommentdelete*/


long langcommentdelete (byte chdelim, byte *ptext, long ct) {
	
	/*
	scan the string from left to right.  if we find a comment delimiting
	character, return its offset
	
	10/30/91 dmb: ignore comment characters that are embedded in quoted strings
	
	5/4/93 dmb: handle single-quoted strings too
	
	3.0.2b1 dmb: handle escape sequences. note that we don't have to deal with 
	the 0x## form because ## can never be quote characters, and thus can't 
	confuse our string parsing

	5.0a12 dmb: handle // comments
	
	6.0a13 dmb: take ptext, ct instead of a big string, and return the comment
	offset instead of deleting anything. Note: textcommentdelete does _not_ 
	handle quoted strings.
	*/
	
	register long i;
	register byte ch;
	boolean flinstring = false;
	boolean flinescapesequence = false;
	byte chendstring = 0;
	
	for (i = 0; i < ct; i++) {
		
		ch = ptext [i];
		
		if (flinstring) {
			
			if (flinescapesequence)
				flinescapesequence = false; /*it's been consumed*/
			
			else {
				
				if (ch == (byte) '\\')
					flinescapesequence = true;
				else
					flinstring = (ch != chendstring);
				}
			}
		
		else if (ch == (byte) '"') {
			
			flinstring = true;
			
			chendstring = '"';
			}
		
		else if (ch == (byte) 'р') {
			
			flinstring = true;
			
			chendstring = 'с';
			}
		
		else if (ch == (byte) '\'') {
			
			flinstring = true;
			
			chendstring = '\'';
			}
		
		else if (ch == chdelim)
			return (i);

		else if (ch == '/') {
			
			if ((i + 1 < ct) && ptext [i+1] == '/')
				return (i);
			}
		}
	
	return (-1);
	} /*langcommentdelete*/


void commentdelete (byte chdelim, bigstring bs) {
	
	/*
	6.0a13 dmb: converted to use new langcommentdelete
	*/
	
	long i;
	
	i = langcommentdelete (chdelim, stringbaseaddress (bs), stringlength (bs));
	
	if (i >= 0)
		setstringlength (bs, i);
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
	
	for (i = 0; i < len; i++) {
		
		if (getstringcharacter (bs, i) == '.') {
			
			if (i == len - 1) /*no next character, no chars to pop*/
				return (false);
				
			if (whitespacechar (getstringcharacter (bs, i+1))) { /*truncate length and return*/
				
				setstringlength (bs, i+1);
				
				return (true);
				}
			}
		} /*for*/
		
	return (false); /*no chars popped*/
	} /*firstsentence*/

/*  This is now initialized by initstrings and getlower is a macro in strings.h
static boolean initlowercase = false;
char lowercasetable[256];

unsigned char getlower (unsigned char c) { /-fast lowercase functions-/
	int i;

	if (! initlowercase) {
		for (i = 0; i < 256; i++)
			lowercasetable[i] = tolower (i);

		initlowercase = true;
		}

	return (lowercasetable [c]);
	} /-getlower-/

*/

void uppertext (ptrbyte ptext, long ctchars) {
	
	register ptrbyte p = ptext;
	register byte ch;
	
	while (--ctchars >= 0) {
		
		ch = *p;
		
		*p++ = toupper (ch);
		}
	} /*uppertext*/


void lowertext (ptrbyte ptext, long ctchars) {
	
	register ptrbyte p = ptext;
	register byte ch;
	
	while (--ctchars >= 0) {
		
		ch = *p;
		
		*p++ = getlower (ch);
		}
	} /*lowertext*/


void allupper (bigstring bs) {
	
	uppertext (stringbaseaddress (bs), stringlength (bs));
	} /*allupper*/


void alllower (bigstring bs) {
	
	lowertext (stringbaseaddress (bs), stringlength (bs));
	} /*alllower*/
	
	
boolean capitalizefirstchar (bigstring bs) {
	
	register char ch;
	
	if (stringlength (bs) == 0)
		return (true);
		
	ch = getstringcharacter (bs, 0);
	
	if (!islower (ch))
		return (false);
		
	setstringcharacter (bs, 0, toupper (ch));
	
	return (true);
	} /*capitalizefirstchar*/
	
	
boolean isallnumeric (bigstring bs) {
	
	/*
	11/6/92 dmb: allow first character to be a sign instead of a digit
	*/
	
	register short ct = stringlength (bs);
	register ptrbyte p = stringbaseaddress (bs);
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
	
	/*
	1/17/97 dmb: recoded with string macros
	*/

	if (ct < 0)
		ct = 0;
	
	setstringlength (bs, ct);

	memset (stringbaseaddress (bs), ch, (long) ct);
	/*
	bs [0] = ct;

	memset (&bs [1], ch, (long) ct);
	*/
	} /*filledstring*/


void padwithzeros (bigstring bs, short len) {
	
	/*
	2003-05-01 AR: insert zeros at beginning of string until
	we have reached the requested length;
	*/
	
	if (len > lenbigstring)
		len = lenbigstring;

	while (len - stringlength (bs) > 0)
		insertchar ('0', bs);

	} /*padwithzeros*/
	
	
void copystring (const bigstring bssource, bigstring bsdest) {

	/*
	create a copy of bssource in bsdest.  copy the length byte and
	all the characters in the source string.

	assume the strings are pascal strings, with the length byte in
	the first character of the string.

	1/17/97 dmb: recoded with string macros
	*/

	register short len;
	
	if (bssource == nil) { /*special case, handled at lowest level*/
		
		setemptystring (bsdest);
		
		return;
		}
	
	len = stringsize (bssource);

	moveleft ((ptrstring) bssource, bsdest, len);


	/*
	len = (short) bssource [0];
	
	for (i = 0; i <= len; i++) 
		bsdest [i] = bssource [i];
	*/
	} /*copystring*/


void copyptocstring (const bigstring bssource, char *sdest) {

	short len = stringlength (bssource);

	memmove (sdest, stringbaseaddress (bssource), len);

	sdest [len] = '\0';
	} /*copyptocstring*/


void copyctopstring (const char *ssource, bigstring bsdest) {

	short len = strlen (ssource);  /*YES: use strlen, this is a C string*/

	memmove (stringbaseaddress (bsdest), ssource, len);

	setstringlength (bsdest, len);
	} /*copyctopstring*/




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
	
	// HLock ((Handle) h);
	
	copystring (*h, bsdest);
	
	// HUnlock ((Handle) h);
	} /*copyheapstring*/
	
	
boolean pushheapstring (hdlstring hsource, bigstring bsdest) {
	
	register hdlstring h = hsource;
	register boolean fl;
	
	if (h == nil) /*nil handles are empty strings*/
		return (true);
		
	HLock ((Handle) h);
	
	fl = pushstring (*h, bsdest);
	
	HUnlock ((Handle) h);
	
	return (fl);
	} /*pushheapstring*/


void timedatestring (long ptime, bigstring bs) {
	bigstring bstime;

	timetodatestring (ptime, bs, false);	

	getstringlist (interfacelistnumber, timedateseperatorstring, bstime);

	pushstring (bstime, bs);

	timetotimestring (ptime, bstime, true);
		
	pushstring (bstime, bs);

	} /*timedatestring*/


	static byte bsellipses [] = "\x01и";

void ellipsize (bigstring bs, short width) {

	/*
	if the string fits inside the given number of pixels, fine -- do nothing
	and return.
	
	if not, return a string that does fit, with ellipses representing the 
	deleted characters.  ellipses are generated by pressing option-semicolon.

	3/28/97 dmb: rewrote x-platform

	5.0.1 dmb: reenable Win code, but only for long strings to work around a 
	crashing bug I can't figure out.

	5.0.2 dmb: redisable Win code. We've reported thier User Breakpoint in DrawText
	bug, which we can't seem to workaround. I'm making our own code faster instead.
	*/
	
	#ifdef xxxWIN95VERSION
		
		if (stringlength (bs) > 16) {

			RECT r;
			
			r.top = 0;
			r.bottom = 50;
			r.left = 0;
			r.right = width;
			
			pushemptyclip ();
			
			convertpstring (bs);
			
			setWindowsFont();
			
			DrawText (getcurrentDC(), bs, -1, &r, DT_END_ELLIPSIS | DT_MODIFYSTRING | DT_NOPREFIX);
			
			clearWindowsFont();
			
			convertcstring (bs);
			
			popclip ();
			}
	#endif
		{
		byte len;
		
		if (stringpixels (bs) <= width) //nothing to do, the string fits
			return;
		
		len = stringlength (bs); //current length in characters
		
		if (len < 2) //too short to truncate
			return;
		
		width -= stringpixels (bsellipses); //subtract width of ellipses
		
		//cut in half until it's shorter than available width
		do
			setstringlength (bs, len /= 2);
		while
			((len > 1) && (stringpixels (bs) > width));
		
		//undo last halving, then go character by character
		setstringlength (bs, len *= 2);

		while (len > 1) {
			
			setstringlength (bs, --len);

			if (stringpixels (bs) <= width)
				break;
			}

		pushstring (bsellipses, bs);
		}
	} /*ellipsize*/


void parsedialogstring (const bigstring bssource, ptrstring bs0, ptrstring bs1, ptrstring bs2, ptrstring bs3, bigstring bsresult) {
		
	/*
	parse a string with up to four string parameters, following the syntax
	used by the Macintosh Dialog Manager in parsing strings.
	
	return the result of parsing bs in bsresult.
	
	where ^0 appears in text, push bs0 on the result string.
	
	may get fancier later, what if we allow you to imbed UserLand code?
	
	we work on a copy of the source string, so you may pass in the same string in
	bssource and bsresult.
	
	dmb 10/8/90: accept nil parameters
	*/
	
	bigstring bs;
	register short len;
	register short i;
	register byte ch;
	ptrstring params [ctparseparams];
	register short paramnum;
	
	copystring (bssource, bs); /*work on a copy of the source string*/
	
	len = stringlength (bs); /*copy into register*/
	
	params [0] = bs0;
	
	params [1] = bs1;
	
	params [2] = bs2;
	
	params [3] = bs3;
	
	setemptystring (bsresult);
	
	for (i = 1; i <= len; i++) {
		
		ch = bs [i];
		
		if (ch != '^')
			pushchar (ch, bsresult);
			
		else {
			if (i == len) /*the ^ is at the end of the string, no number*/
				return;
			
			paramnum = bs [i + 1] - '0'; /*index into params array*/
			
			if ((paramnum >= 0) && (paramnum <= 3)) {
				
				assert (params [paramnum] != nil); /*string should always be provided*/
				
				pushstring (params [paramnum], bsresult);
				
				i++; /*advance over numeric character*/
				}
			else
				pushchar ('^', bsresult); 
			}
		} /*for*/
	
	/***subtractstrings (bsresult, "\p рс", bsresult); #*in case there was a missing param*/
	} /*parsedialogstring*/

boolean parsedialoghandle (Handle hsource, Handle h0, Handle h1, Handle h2, Handle h3) {
		
	/*
	6.1d1 AR: parse a handle with up to four handle parameters, following the syntax
	used by the Macintosh Dialog Manager in parsing strings.
	
	we work on the source handle.
	
	where ^0 appears in text, insert h0 into the source handle, etc.
	*/
	
	register long i, sizediff;
	register long maxi = gethandlesize (hsource) - 1; /*we only need to loop till the 2nd to last char*/
	Handle params [ctparseparams];
	register short paramnum;
	
	params [0] = h0;
	
	params [1] = h1;
	
	params [2] = h2;
	
	params [3] = h3;
	
	for (i = 0; i < maxi; i++) {
				
		if ((*hsource) [i] != '^') /*look for carets only*/
			continue;

		paramnum = (*hsource) [i + 1] - '0'; /*index into params array*/
		
		if ((paramnum < 0) || (paramnum > 3)) /*check range*/
			continue;
		
		assert (params [paramnum] != nil); /*handle should always be provided*/
		
		if (!pullfromhandle (hsource, i, 2, nil))
			return (false);
		
		if (!inserthandleinhandle (params [paramnum], hsource, i))
			return (false);
		
		sizediff = gethandlesize (params [paramnum]) - 2; /*advance over inserted characters*/

		i += sizediff;

		maxi += sizediff;

		} /*for*/
	
	return (true);
	
	} /*parsedialogstring*/


boolean setparseparams (bigstring bs0, bigstring bs1, bigstring bs2, bigstring bs3) {
	
	/*
	dmb 10/8/90: accept nil parameters
	*/
	
	register short i;
	
	for (i = 0; i < ctparseparams; i++) {
	
		disposehandle ((Handle) parseparams [i]);
		
		parseparams [i] = nil;
		} /*for*/
	
	if (bs0 != nil)
		if (!newheapstring (bs0, &parseparams [0]))
			return (false);
	
	if (bs1 != nil)
		if (!newheapstring (bs1, &parseparams [1]))
			return (false);
	
	if (bs2 != nil)
		if (!newheapstring (bs2, &parseparams [2]))
			return (false);
	
	if (bs3 != nil)
		if (!newheapstring (bs3, &parseparams [3]))
			return (false);
	
	return (true);
	} /*setparseparams*/
	
	
void parseparamstring (bigstring bsparse) {
	
	bigstring bs [ctparseparams];
	register short i;
	register hdlstring hstring;
	
	for (i = 0; i < ctparseparams; i++) {
		
		hstring = parseparams [i];
		
		if (hstring == nil)
			setemptystring (bs [i]);
		else
			copyheapstring (hstring, bs [i]);
		} /*for*/
	
	parsedialogstring (bsparse, bs [0], bs [1], bs [2], bs [3], bsparse);
	} /*parseparamstring*/


void getstringresource (short resnum, bigstring bs) {
	/*
	get a string resource, numbered resnum.  if there's an error,
	set the string to the empty string.
	*/
	
	StringHandle hstring;
	
	hstring = GetString (resnum);
	
	if (hstring == nil)
		setemptystring (bs);
	else
		copyheapstring (hstring, bs);
	} /*getstringresource*/


void parsenumberstring (short listnum, short id, long number, bigstring bsparse) {
	
	/*
	if number is one, use id; otherwise, use id + 1.  then parse the number in
	*/
	
	bigstring bsnumber;
	
	numbertostring (number, bsnumber);
	
	if (number != 1)
		++id;
	
	getstringlist (listnum, id, bsparse);
	
	parsedialogstring (bsparse, bsnumber, nil, nil, nil, bsparse);
	} /*parsenumberstring*/




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


long textpatternmatch (byte *ptext, long lentext, bigstring bsfind, boolean flunicase) {
	
	/*
	return the offset into ptext of the match, or -1.
	
	this uses an algorithm attributed to Boyer-Moore, I believe.  it uses 
	cryptic macros above to support case-insensitivity with maximum performance.
	
	2006-04-02 aradke: The search failed when the pattern was longer than 129 chars
		because the jump table for the Boyer-Moore algorithm was defined as an array
		of signed chars, causing an overflow for jump distances larger than 129 chars.
		The fix is to define the jump table as an array of signed longs instead. [bug #1463056]
		http://sourceforge.net/tracker/index.php?func=detail&aid=1463056&group_id=120666&atid=687798
	*/
	
	register byte *p = ptext;
	byte *pend = p + lentext;
	byte *psave;
	long i;
	long dist;
	long lenpattern;
	long jump [256];
	boolean flcase = !flunicase;
	ptrstring bspattern = bsfind;
	#define checklower(c) (flcase? (c) : getlower (c))
	
	lenpattern = stringlength (bspattern);
	
	/*fill in the jump array according to the characters in the search pattern*/
	
	for (i = 0;  i <= 255;  i++)
		jump [i] = lenpattern;
	
	for (i = 1;  i <= lenpattern;  i++)
		jump [bspattern [i]] = 1 - i;
	
	/*start the search*/
	
	p += lenpattern - 1;
	
	while (p < pend) {
		
		dist = jump [checklower (*p)];
		
		p += dist;
		
		if (dist <= 0) { /*p was a member of the target pattern*/
			
			psave = p; /*p is now @potential beginning of string*/
			
			if (p + lenpattern > pend) /*not enough room for a match*/
				return (-1);
			
			i = 1;
			
			while (checklower (*p++) == bspattern [i++]) {
				
				if (i > lenpattern)
					return (psave - ptext);
				}
			
			/*fell through; reset search to position following last check*/
			
			p = psave - dist + 1;
			}
		}
	
	return (-1);
	} /*textpatternmatch*/


short patternmatch (bigstring bspattern, bigstring bs) {
	
	/*
	the beginning of something bigger.  first version -- search for the 
	pattern in the string, return the offset of the first occurrence of
	the pattern.  0 if not found.
	*/
	
	register short lenstring = stringlength (bs);
	register short lenpattern = stringlength (bspattern);
	register short ixstring = 0;
	register byte chfirst;
	register short i, ix;
	
	if ((lenstring == 0) || (lenpattern == 0))
		return (0);
	
	chfirst = getstringcharacter (bspattern, 0);
	
	while (true) {
		
		if (getstringcharacter (bs, ixstring) == chfirst) { /*matched at least first character in string*/
			
			for (i = 1; i < lenpattern; i++) {
				
				ix = ixstring + i;
				
				if (ix >= lenstring) /*gone off end of string, can't match*/
					return (0);
				
				if (getstringcharacter (bs, ix) != getstringcharacter (bspattern, i)) 
					goto L1;
				} /*for*/
			
			return (ixstring + 1); /*loop terminated, full match*/
			}
		
		L1: /*advance to next character in string*/
		
		if (++ixstring > lenstring) /*reached end of string, not found*/
			return (0);
		} /*while*/
	} /*patternmatch*/


boolean addstrings (bigstring bs1, bigstring bs2, bigstring bsdest) {
	
	/*
	return the result of concatenating the first string and the second 
	string.
	
	return false if the result is too long
	*/
	
	copystring (bs1, bsdest);
	
	return (pushstring (bs2, bsdest));
	} /*addstrings*/


boolean subtractstrings (bigstring bs1, bigstring bs2, bigstring bsdest) {
	
	/*
	given the first two strings, return a string that's the result of deleting 
	the first instance of the second string from the first string.
	
	return false if the second string doesn't appear in the first.
	*/
	
	register short ix;
	
	copystring (bs1, bsdest); /*default: return first string unchanged*/
	
	ix = patternmatch (bs2, bs1);
	
	if (ix > 0) { /*found it*/
		
		deletestring (bsdest, ix, stringlength (bs2));
		
		return (true);
		}
	
	return (false); /*didn't change string*/
	} /*subtractstrings*/
	

void ostypetostring (OSType type, bigstring bs) {
	
	setstringlength (bs, sizeof (OSType));
	
	disktomemlong (type);

	moveleft (&type, stringbaseaddress (bs), longsizeof (OSType));

	//RAB: 1/22/98 removed this from here and added it in filedialog.c
	//#ifdef WIN95VERSION
	//	poptrailingwhitespace (bs);
	//#endif
	} /*ostypetostring*/
	
	
boolean stringtoostype (bigstring bs, OSType *type) {
	
	/*
	2.1b4 dmb: always copy into type, even when returning false for bad length
	*/

	// kw 2005-11-28 changed to unsigned for better match with stringlength()
	register unsigned short len = stringlength (bs);
	boolean fl = true;
	
	*type = '    ';
	
	if (len > sizeof (OSType)) {
		
		len = sizeof (OSType);
		
		fl = false;
		}
		
	moveleft (stringbaseaddress (bs), type, (long) len);
	
	disktomemlong (*type);
	
	return (fl);
	} /*stringtoostype*/


boolean hexstringtonumber (bigstring bshex, long *n) {
	
	register long x = 0;
	register char ch;
	register short i;
	register short len;
	bigstring bs;
	boolean fl = true;
	boolean flsignextend;
	
	copystring (bshex, bs);
	
	stringdeletechars (bs, chspace);
	
	subtractstrings (bs, bshexprefix, bs);
	
	flsignextend = stringlength (bs) == 4;
	
	popleadingchars (bs, '0');
	
	alllower (bs);
	
	len = min (stringlength (bs), 8);
	
	for (i = 1; i <= len; ++i) {
		
		x <<= 4; /*one nibble per character*/
		
		ch = getstringcharacter (bs, i - 1);
		
		if (isxdigit (ch))
			x += hextoint (ch);
		
		else {
			fl = false;
			
			break;
			}
		}
	
	if (flsignextend) { /*need so sign-extend as an integer*/
		
		i = x;
		
		x = i;
		}
	
	*n = x;
	
	return (fl);
	} /*hexstringtonumber*/


void bytestohexstring (ptrvoid pdata, long ctbytes, bigstring bshex) {
	
	register byte *p = pdata;
	register byte x;
	register long ct = ctbytes;
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


void numbertohexstring (long number, bigstring bshex) {
	
	/*
	5.0a21 dmb: set byte order to little-endian (Motorola)
	*/

	if ((number < -32768) || (number > 32767))  {
		
		memtodisklong (number);

		bytestohexstring (&number, sizeof (long), bshex);
		}
	
	else {
		
		short x = number;
		
		memtodiskshort (x);

		bytestohexstring (&x, sizeof (short), bshex);
		}
	} /*numbertohexstring*/


boolean bytestohex (Handle hbytes, Handle *hhex) {
	
	register byte *pbytes;
	register byte *phex;
	register long ct;
	byte x;
	static byte hex [16] = "0123456789ABCDEF";
	
	ct = gethandlesize (hbytes);
	
	if (!newhandle (stringlength (bshexprefix) + ct * 2, hhex))
		return (false);
	
	pbytes = (byte *) *hbytes; /*it's not going to move now that we've allocated return handle*/
	
	phex = (byte *) **hhex;
	
	moveleft (stringbaseaddress (bshexprefix), phex, stringlength (bshexprefix));
	
	phex += stringlength (bshexprefix);
	
	while (--ct >= 0) {
		
		x = *pbytes++;
		
		*phex++ = hex [(x >> 4) & 0x000F];
		
		*phex++ = hex [x & 0x000F];
		}
	
	return (true);
	} /*bytestohex*/


boolean hextobytes (Handle hhex, Handle *hbytes) {
	
	/*
	5.0a2 dmb: ct must be long, not short
	*/
	
	register byte *pbytes;
	register byte *phex;
	register long ct;
	long x;
	char ch;
	boolean flhashexprefix;
	
	ct = gethandlesize (hhex);
	
	phex = (byte *) *hhex;
	
	flhashexprefix = (phex [1] == 'x');
	
	if (flhashexprefix)
		ct -= 2;
	
	ct /= 2; /*one byte per two hex characters*/
	
	if (!newhandle (ct, hbytes))
		return (false);
	
	pbytes = (byte *) **hbytes; /*it's not going to move now that we've allocated return handle*/
	
	phex = (byte *) *hhex;
	
	if (flhashexprefix)
		phex += 2;
	
	while (--ct >= 0) {
		
		ch = *phex++; /*get high nibble first*/
		
		x = hextoint (ch) << 4;
		
		ch = *phex++; /*get low nibble*/
		
		x |= hextoint (ch);
		
		*pbytes++ = x;
		}
	
	return (true);
	} /*bytestohexstring*/


void kstring (long ctbytes, bigstring bs) {
	
	bigstring bskilo;
	
	numbertostring (ctbytes / 1024, bs);
	
	getstringlist (interfacelistnumber, kilobytestring, bskilo);
	
	pushstring (bskilo, bs);
	} /*kstring*/


void dirtostring (tydirection dir, bigstring bs) {
	
	register short ix;
	
	ix = dirtoindex (dir);
	
	if (ix < 0)
		ix = 0; /*index for nodirection*/
	
	copyheapstring (dirstrings [ix], bs);
	} /*dirtostring*/


boolean stringtodir (bigstring bs, tydirection *dir) {
	
	register short i;
	
	alllower (bs); /*matching is unicase*/
	
	for (i = 0; i < ctdirections; i++) {
		
		bigstring bsdirection;
		
		copyheapstring (dirstrings [i], bsdirection);
		
		alllower (bsdirection);
		
		if (equalstrings (bs, bsdirection)) {
			
			*dir = indextodir (i);
			
			return (true);
			}
		} /*for*/
	
	*dir = nodirection;
	
	return (false);
	} /*stringtodir*/
	

boolean midinsertstring (bigstring bsinsert, bigstring bs, short ixinsert) {
	
	/*
	insert bsinsert in the indicated string at offset ixinsert.
	*/
	
	short ix = ixinsert - 1; // make zero-based
	short leninsert = stringlength (bsinsert);
	short origlen = stringlength (bs);
	short newlen = leninsert + origlen;
	ptrbyte pinsert = stringbaseaddress (bs) + ix;
	
	if (newlen > lenbigstring)
		return (false);
	
	moveright (pinsert, pinsert + leninsert, (long) origlen - ix);
	
	moveleft (stringbaseaddress (bsinsert), pinsert, (long) leninsert);
	
	setstringlength (bs, newlen);
	
	return (true);
	} /*midinsertstring*/


boolean replacestring (bigstring bsreplace, short ix, short ctreplace, bigstring bsinsert) {
	
	/*
	4/3/92 dmb: check for overflow before doing deletion
	*/
	
	if (stringlength (bsreplace) + (stringlength (bsinsert) - ctreplace) > lenbigstring)
		return  (false);
	
	deletestring (bsreplace, ix, ctreplace);
	
	midinsertstring (bsinsert, bsreplace, ix);
	
	return (true);
	} /*replacestring*/


boolean stringaddcommas (bigstring bs) {
	
	/*
	turn a string like "98760241" into "98,760,241".
	
	5.0a24 dmb: bug don't turn "760241" into ",760,241"
	*/
	
	register short ix = stringlength (bs); /*start at the end of the string*/
	bigstring bscomma;
	
	getstringlist (interfacelistnumber, commastring, bscomma);
	
	while (true) {
		
		ix -= 3;
		
		if (ix <= 0) /*ran out of characters*/
			return (true);
		
		if (getstringcharacter (bs, ix) == '-') /*no comma between a minus and the number*/
			return (true);
		
		midinsertstring (bscomma, bs, ix + 1);
		} /*while*/
	} /*stringaddcommas*/
	
	
boolean stringdeletechars (bigstring bs, char ch) {
	
	/*
	delete all occurrences of the char in the string.
	
	return true if we actually deleted a character.
	*/
	
	register short i;
	register short len;
	register boolean fl = false;
	
	len = stringlength (bs);
	
	for (i = 1; i <= len; i++) {
		
		if (getstringcharacter (bs, i - 1) == ch) {
			
			deletestring (bs, i, 1);
			
			i--; /*look at the same position again*/
			
			len--; /*one fewer chars to look at*/
			
			fl = true;
			}
		} /*for*/
	
	return (fl);
	} /*stringdeletechars*/
			

boolean assurelastchariscolon (bigstring bs) {

	if (lastchar (bs) == ':')
		return (true);
		
	return (pushchar (':', bs));
	} /*assurelastchariscolon*/



static short getTextEncodingErrorNumFromOSCode( long osStatusCode )
{
	switch ( osStatusCode )
	{
		case kTextUnsupportedEncodingErr:
			return 3;
		case kTextMalformedInputErr:
			return 4;
		case kTextUndefinedElementErr:
			return 5;
		case kTECNoConversionPathErr:
			return 6;
		case kTECPartialCharErr:
			return 7;
		
		default:
			return 2;
	}
}

static void setTextEncodingConversionError( const bigstring encIn, const bigstring encOut, long osStatusCode )
{
	short errStringNum = getTextEncodingErrorNumFromOSCode( osStatusCode );
	bigstring rezErrorMessage, bsErrorMessage;
	
	if ( ! getstringlist( stringerrorlist, errStringNum, rezErrorMessage ) )
		getstringlist( stringerrorlist, 1, rezErrorMessage );
	
	parsedialogstring( rezErrorMessage, (ptrstring) encIn, (ptrstring) encOut, nil, nil, bsErrorMessage );
	
	langerrormessage( bsErrorMessage );
}




/* convert an "internet name" for an encoding into a platform-specific id for the character set */

static boolean getTextEncodingIDFromIANA( bigstring bsEncodingName, long * encodingId )
{

		OSStatus err;
		TextEncoding oneEncoding;
		
		err = TECGetTextEncodingFromInternetName( &oneEncoding, bsEncodingName );
		
		if ( err != noErr )
		{
			setTextEncodingConversionError( bsEncodingName, BIGSTRING( "" ), (long) err );
			
			return ( false );
		}
		
		*encodingId = (long) oneEncoding;
		

	
	return (true);
}

/*
	determine if the specified character set is recognized by the OS
	bsEncodingName is an IANA-friendly name like "utf-8" , "macintosh", or "iso-8859-1"
*/
boolean isTextEncodingAvailable( bigstring bsEncodingName )
{
	long encodingId;
	boolean fl;
	
	disablelangerror();
	
	fl = getTextEncodingIDFromIANA( bsEncodingName, &encodingId );
	
	enablelangerror();
	
	return ( fl );
}

static boolean converttextencoding( Handle h, Handle hresult, const long inputcharset, const long outputcharset, long * OSStatusCode )
{

	TECObjectRef converter;		
	OSStatus status;
	ByteCount ctorigbytes, ctoutputbytes, ctflushedbytes;		
	long sizeoutputbuffer;
	long pullBytes = 0;
	long lentext = gethandlesize (h);

	status = TECCreateConverter (&converter, inputcharset, outputcharset);
	if ( status != noErr ) {
		TECDisposeConverter (converter);
		*OSStatusCode = (long) status;
		return ( false );
	}

	sizeoutputbuffer = lentext * 4;

	if (sizeoutputbuffer < 32) /*docs say use 32 minimum*/
		sizeoutputbuffer = 32;

	if (!sethandlesize (hresult, sizeoutputbuffer)) {  // out of memory
		TECDisposeConverter (converter);
		return (false);
	}

	// 2005-08-26 --- kw better bytemark detecting
	// see http://en.wikipedia.org/wiki/Byte_Order_Mark
	if (inputcharset == kCFStringEncodingUTF8) // we handle utf-8 input
	{
		if (	(*h) [0] == '\xEF'
			 && (*h) [1] == '\xBB'
			 && (*h) [2] == '\xBF')
				pullBytes = 3;
	}
	else
	{
		if (inputcharset == kTextEncodingUnicodeDefault) // utf-16
		{
			if (	(	(*h) [0] == '\xEF'
					 && (*h) [1] == '\xFF')
				||	(	(*h) [0] == '\xFF'
					 && (*h) [1] == '\xFE'))
				pullBytes = 2;
		}
	}

	lentext = gethandlesize (h) - pullBytes;

	status = TECConvertText( converter, (ConstTextPtr)(*h + pullBytes), lentext, &ctorigbytes, (TextPtr)(*hresult), sizeoutputbuffer, &ctoutputbytes );
	
	if ( status != noErr )
	{
		TECDisposeConverter (converter);
		
		*OSStatusCode = (long) status;
		return ( false );
	}

	TECFlushText (converter, (TextPtr)(*hresult), sizeoutputbuffer, &ctflushedbytes);

	TECDisposeConverter (converter);

	sethandlesize (hresult, ctoutputbytes + ctflushedbytes);
	
	return (true);


	} /*converttextencoding*/


boolean convertCharset( Handle hString, Handle hresult, bigstring charsetIn, bigstring charsetOut )
{
	long err;
	
		
		TextEncoding teInputSet, teOutputSet;


	
	if ( ! getTextEncodingIDFromIANA( charsetIn, (long *) &teInputSet ) )
		return (false);
	
	if ( ! getTextEncodingIDFromIANA( charsetOut, (long *) &teOutputSet ) )
		return (false);
	
	if ( ! converttextencoding( hString, hresult, (long) teInputSet, (long) teOutputSet, &err ) )
	{
		setTextEncodingConversionError( charsetIn, charsetOut, err );
		
		return ( false );
	}
	
	return ( true );
}


boolean utf16toansi( Handle h, Handle hresult ) {

	/*
	7.0b42 PBS: convert from UTF-16 to iso-8859-1 character sets.
	*/
	
	
		
		long err;
		
		if ( !converttextencoding( h, hresult, kTextEncodingUnicodeDefault, kTextEncodingWindowsLatin1, &err ) )
		
			goto error;


	return (true);
	
	
	error:
		
		setTextEncodingConversionError( cs_utf16, cs_iso88591, (long) err );
		
		return ( false );
	
	} /*utf16toansi*/


boolean utf8toansi (Handle h, Handle hresult) {

	/*
	70b42 PBS: convert from UTF-8 to ANSI.
	*/
	
	long err;
	
	
		
		if ( !converttextencoding( h, hresult, kCFStringEncodingUTF8, kTextEncodingWindowsLatin1, &err ) )
		{
			setTextEncodingConversionError( cs_utf8, cs_iso88591, err );
			
			return (false);
		}


	return (true);
	
	} /*utf8toansi*/


boolean ansitoutf8 (Handle h, Handle hresult) {

	/*
	7.0b42 PBS: convert from ANSI to UTF-8.
	*/
	
	long err;
	
	
	
		if ( !converttextencoding( h, hresult, kTextEncodingWindowsLatin1, kCFStringEncodingUTF8, &err ) )
		{
			setTextEncodingConversionError( cs_iso88591, cs_utf8, err );
			
			return ( false );
		}
	

	return (true);
	} /*ansitoutf8*/


boolean ansitoutf16 (Handle h, Handle hresult) {

	/*
	7.0b42 PBS: convert from ANSI to UTF-16.
	*/
	
	long err;
	
	
	
		if ( !converttextencoding( h, hresult, kTextEncodingWindowsLatin1, kTextEncodingUnicodeDefault, &err ) )
		{
			setTextEncodingConversionError( cs_iso88591, cs_utf16, err );
			
			return ( false );
		}


	return (true);
	} /*ansitoutf8*/


boolean pullstringsuffix (bigstring bssource, bigstring bssuffix, unsigned char chsuffix) {

	/*
	7.0.2b1 Radio PBS: get the suffix from a string.
	On return, bssource lacks the suffix and the suffix character.
	bssuffix contains the suffix minus the suffix character.
	*/

	short lensource = stringlength (bssource);
	short ct = lensource;
	char ch;
	boolean fl = false;

	copystring (emptystring, bssuffix);

	while (true) {

		if (ct < 1)
			break;

		ch = bssource [ct];
	
		if (ch == chsuffix) {

			fl = true;

			break;
			} /*if*/

		insertchar (ch, bssuffix);

		ct--;
		} /*while*/

	
	if (fl)

		setstringlength (bssource, (stringlength (bssource) - stringlength (bssuffix)) - 1);

	else

		copystring (emptystring, bssuffix);
		
	return (fl);
	} /*pullsuffix*/


boolean macromantoutf8 (Handle h, Handle hresult) {

	/*
	2006-05-03 creedon: fixed bug introduced in changeover to use converttextencoding function, reverse encIn and encOut
				   values
	2006-02-24 creedon: convert from Mac Roman character set to UTF-8, cribbed from ansitoutf8
	*/
	
	long err;
	long encIn, encOut;
	
	
		
		encIn = kTextEncodingMacRoman;
		encOut = kCFStringEncodingUTF8;
	
	
	if ( !converttextencoding( h, hresult, encIn, encOut, &err ) )
	{
		setTextEncodingConversionError( cs_utf8, cs_macintosh, err );
		
		return ( false );
	}

	return (true);
	} /* macromantoutf8 */


boolean utf8tomacroman (Handle h, Handle hresult) {

	/*
	2006-02-26 creedon: convert from UTF-8 character set to Mac Roman, cribbed from utf8toansi
	*/
	
	long err;
	long encIn, encOut;
	
	
		
		encIn = kCFStringEncodingUTF8;
		encOut = kTextEncodingMacRoman;
	
	
	if ( !converttextencoding( h, hresult, encIn, encOut, &err ) )
	{
		setTextEncodingConversionError( cs_utf8, cs_macintosh, err );
		
		return ( false );
	}
	
	return (true);
	} /* utf8tomacroman */



void initstrings (void) {
	
	register short i;

	for (i = 0; i < 256; i++)
		lowercasetable[i] = tolower (i);
	
	clearbytes (&parseparams, longsizeof (parseparams));
	
	for (i = 1; i <= ctdirections; i++) {
		
		bigstring bs;
		
		getstringlist (directionlistnumber, i, bs);
		
		newheapstring (bs, &dirstrings [i - 1]);
		} /*for*/
	} /*initstrings*/
