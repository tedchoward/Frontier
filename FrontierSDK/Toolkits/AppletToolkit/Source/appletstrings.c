
/*╘ copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include <Packages.h>
#include "appletops.h"
#include "appletmemory.h"
#include "appletstrings.h"


boolean isPunctuationChar (char ch) {
	
	if ((ch >= '!') && (ch <= '/')) 
		return (true);
		
	if ((ch >= ':') && (ch <= '?'))
		return (true);
	
	return (false);
	} /*isPunctuationChar*/
	

boolean isAlphaChar (char ch) {
	
	if ((ch >= 'a') && (ch <= 'z'))
		return (true);
		
	if ((ch >= 'A') && (ch <= 'Z'))
		return (true);
		
	if ((ch >= '0') && (ch <= '9')) 
		return (true);
		
	if (ch == '_')
		return (true);

	return (false);
	} /*isAlphaChar*/
	

static short IUcomparestrings (bigstring bs1, bigstring bs2) {

	/*
	return zero if the two strings (pascal type, with length-byte) are
	equal.  return -1 if bs1 is less than bs2, or +1 if bs1 is greater than
	bs2.
	
	use the Macintosh international utility routine IUCompString, which 
	performs dictionary string comparison and accounts for accented characters
	*/
	
	short result;
	
	result = IUCompString (bs1, bs2);
	
	return (result);
	} /*IUcomparestrings*/


short comparestrings (void *pvoid1, void *pvoid2) {

	/*
	return zero if the two strings (pascal type, with length-byte) are
	equal.  return -1 if bs1 is less than bs2, or +1 if bs1 is greater than
	bs2.
	
	this version does not call the IU package, and performs much better
	than the one that does.
	
	dmb 9/21/93: fixed two bugs
	*/
	
	unsigned char *bs1 = (unsigned char *) pvoid1, *bs2 = (unsigned char *) pvoid2;
	unsigned char len1 = bs1 [0];
	unsigned char len2 = bs2 [0];
	unsigned char *p1 = &bs1 [1];
	unsigned char *p2 = &bs2 [1];
	
	while (len1 > 0) {
		
		if (len2 == 0) /*bs1 is longer, therefore bs1 is greater than bs2*/
			return (1);
		
		if (*p1 != *p2) {
			
			if (*p1 < *p2)
				return (-1);
			else
				return (+1);
			}
		
		p1++; p2++; 
		
		len1--; len2--;
		} /*while*/
	
	if (len2 == 0) /*the strings are of equal length, same chars*/
		return (0);
		
	return (-1); /*bs2 is longer, therefore bs1 is less than bs2*/
	} /*comparestrings*/


boolean equalstrings (void *pvoid1, void *pvoid2) {

	/*
	return true if the two strings (pascal type, with length-byte) are
	equal.  return false otherwise.
	*/

	ptrchar p1 = (ptrchar) pvoid1, p2 = (ptrchar) pvoid2;
	char ct = *p1 + 1;
	
	/*DW 12/31/95*/ {
	
		unsigned char len1, len2;
		
		len1 = *p1;
		
		len2 = *p2;
		
		if (len1 != len2)
			return (false);
		}
	
	while (ct--) 
		
		if (*p1++ != *p2++)
		
			return (false);
		
	return (true); /*loop terminated*/
	} /*equalstrings*/
	
	
boolean unicaseequalstrings (void *pvoid1, void *pvoid2) {
	
	bigstring bscopy1, bscopy2;
	
	copystring (pvoid1, bscopy1);
	
	copystring (pvoid2, bscopy2);
	
	alllower (bscopy1);
	
	alllower (bscopy2);
	
	return (equalstrings (bscopy1, bscopy2));
	} /*unicaseequalstrings*/
	

void copystring (void *bssource, void *bsdest) {

	/*
	create a copy of bssource in bsdest.  copy the length byte and
	all the characters in the source string.

	assume the strings are pascal strings, with the length byte in
	the first character of the string.
	
	DW 4/3/95: wouldn't copy strings that are longer than 128 chars!
	*/

	short i, len;
	
	len = (short) ((unsigned char *) bssource) [0];
	
	for (i = 0; i <= len; i++) 
		((unsigned char *) bsdest) [i] = ((unsigned char *) bssource) [i];
	} /*copystring*/


boolean pushstring (void *bssource, void *bsdest) {

	/*
	insert the source string at the end of the destination string.
	
	assume the strings are pascal strings, with the length byte in
	the first character of the string.
	*/
	
	unsigned short lensource = stringlength ((unsigned char *) bssource);
	unsigned short lendest = stringlength ((unsigned char *) bsdest);
	char *psource, *pdest;
	
	if ((lensource + lendest) > lenbigstring)
		return (false);
		
	pdest = (ptrchar) bsdest + (unsigned char) lendest + 1;
	
	psource = (ptrchar) bssource + 1;
	
	((unsigned char *) bsdest) [0] += (unsigned char) lensource;
	
	while (lensource--) *pdest++ = *psource++;
	
	return (true);
	} /*pushstring*/
	

boolean pushspace (void *pstring) {
		
	bigstring bsspace;
	
	setstringlength (bsspace, 1);
	
	bsspace [1] = ' ';
	
	return (pushstring (bsspace, pstring));
	} /*pushspace*/
	

void pushlong (long num, void *pstring) {

	bigstring bsint;
	
	NumToString (num, bsint);
	
	pushstring (bsint, pstring);
	} /*pushlong*/
	

void pushint (short num, void *pstring) {
	
	pushlong ((long) num, pstring);
	} /*pushint*/
	
	
void pushostype (OSType os, void *pstring) {
	
	bigstring bsos;
	
	setstringlength (bsos, 4);
	
	moveleft (&os, &bsos [1], 4);
	
	pushstring (bsos, pstring);
	} /*pushostype*/
	
	
void pushtimeonstring (void *pstring) {
	
	unsigned long now;
	bigstring bstime;
	
	GetDateTime (&now);
	
	gettimestring (now, bstime);
	
	pushstring (bstime, pstring);
	} /*pushtimeonstring*/
	
	
void allupper (void *pstring) {
	
	ptrchar p = (ptrchar) pstring;
	char len = *p;
	char ch;
	
	p++;
	
	while (len--) {
		
		ch = *p;
		
		if ((ch >= 'a') && (ch <= 'z'))
			*p -= 32;
			
		p++;
		} /*while*/
	} /*allupper*/
	
	
void alllower (void *pstring) {
	
	ptrchar p = (ptrchar) pstring;
	char len = *p;
	char ch;
	
	p++;
	
	while (len--) {
		
		ch = *p;
		
		if ((ch >= 'A') && (ch <= 'Z'))
			*p += 32;
			
		p++;
		} /*while*/
	} /*alllower*/
	
	
boolean stringlessthan (void *p1, void *p2) {
	
	unsigned char *bs1 = (unsigned char *) p1, *bs2 = (unsigned char *) p2;
	short i, ctloops;
	char ch1, ch2;
	short len1, len2;
	
	len1 = (short) bs1 [0];
	
	len2 = (short) bs2 [0];
	
	ctloops = minint (len1, len2);
	
	for (i = 1; i <= ctloops; i++) {
		
		ch1 = bs1 [i];
		
		ch2 = bs2 [i];
		
		if (ch1 != ch2) /*we have our answer*/
			return (ch1 < ch2);
		} /*for*/
	
	return (len1 < len2); /*loop terminated, strings are equal up to the min length*/
	} /*stringlessthan*/
	
	
void midstring (void *psource, short ix, short len, void *pdest) {
	
	unsigned char *bssource = (unsigned char *) psource, *bsdest = (unsigned char *) pdest;
	
	setstringlength (bsdest, len);
	
	moveleft (bssource + ix, bsdest + 1, (long) len);
	} /*midstring*/
	
	
boolean pushchar (byte ch, void *pstring) {
	
	/*
	insert the character at the end of a pascal string.
	*/
	
	unsigned char *bs = (unsigned char *) pstring;
	short len;
	
	len = bs [0]; 
	
	if (len >= lenbigstring)
		return (false);
	
	bs [++len] = ch;
	
	bs [0] = len;
	
	return (true);
	} /*pushchar*/
	
	
boolean scanstring (byte ch, void *pstring, short *ix) {
	
	/*
	return in ix the index in the string of the first occurence of chscan.
	NOTE: ix MUST be initizlied to starting location!!!
	
	return false if it wasn't found, true otherwise.
	
	dmb 10/26/90: p is now initialized correctly to bs + i, not bs + 1
	
	ldr 10/08/95: get the length of pstring, not bs!
	*/
	
	byte *bs = (byte *) pstring;
	short i;
	ptrbyte p;
	byte c = ch;
	short len = stringlength ((unsigned char *)pstring);
	
	for (i = *ix, p = bs + i; i <= len; i++) 
		
		if (*p++ == c) {
			
			*ix = i;
			
			return (true);
			}
			
	return (false);
	} /*scanstring*/


boolean deletestring (void *pstring, short ixdelete, short ctdelete) {
	
	byte *bs = (byte *) pstring;
	short ix = ixdelete;
	short ct = ctdelete;
	short len = stringlength (bs);
	long ctmove;
	ptrbyte pfrom, pto;
	short maxct;
	
	maxct = len - ix + 1;
	
	if (ct > maxct)
		ct = maxct;	
	
	if ((ix > len) || (ix < 1))
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
	
	
boolean firstword (void *psource, char chdelim, void *pdest) {
	
	/*
	copy the first word from bs, and put it into bsdest.
	
	search forwards from the beginning of the source string until you 
	find chdelim.
	*/
	
	unsigned char *bssource = (unsigned char *) psource, *bsdest = (unsigned char *) pdest;
	short len = stringlength (bssource);
	short i;
	char ch = chdelim;
	
	for (i = 1; i <= len; i++) {
		
		if (bssource [i] == ch) {
			
			midstring (bssource, 1, i - 1, bsdest);
			
			return (true);
			}
		} /*for*/
		
	copystring (bssource, bsdest);
	
	return (true);
	} /*firstword*/


boolean lastword (void *psource, char chdelim, void *pdest) {
	
	/*
	copy the last word from bs, and put it into bsdest.
	
	search backwards from the end of the source string until you find
	chdelim.
	*/
	
	unsigned char *bssource = (unsigned char *) psource, *bsdest = (unsigned char *) pdest;
	short len = stringlength (bssource);
	short i;
	char ch = chdelim;
	
	for (i = len; i > 0; i--) {
		
		if (bssource [i] == ch) {
			
			midstring (bssource, i + 1, len - i, bsdest);
			
			return (true);
			}
		} /*for*/
		
	copystring (bssource, bsdest);
	
	return (true);
	} /*lastword*/
	
	
short patternmatch (void *ppattern, void *pstring) {
	
	unsigned char *bspattern = (unsigned char *) ppattern, *bs = (unsigned char *) pstring;
	short lenstring = stringlength (bs);
	short lenpattern = stringlength (bspattern);
	short ixstring = 1;
	byte chfirst;
	short i, ix;
	
	if ((lenstring == 0) || (lenpattern == 0))
		return (0);
	
	chfirst = bspattern [1];
	
	while (true) {
		
		if (bs [ixstring] == chfirst) { /*matched at least first character in string*/
			
			for (i = 2; i <= lenpattern; i++) {
				
				ix = ixstring + i - 1;
				
				if (ix > lenstring) /*gone off end of string, can't match*/
					return (0);
				
				if (bs [ix] != bspattern [i]) 
					goto L1;
				} /*for*/
			
			return (ixstring); /*loop terminated, full match*/
			}
		
		L1: /*advance to next character in string*/
		
		if (++ixstring > lenstring) /*reached end of string, not found*/
			return (0);
		} /*while*/
	} /*patternmatch*/


void filledstring (char ch, short ct, void *pstring) {
	
	unsigned char *bs = (unsigned char *) pstring;
	
	if (ct < 0)
		ct = 0;
		
	bs [0] = ct;
	
	fillchar (&bs [1], (long) ct, ch);
	} /*filledstring*/


boolean insertstring (unsigned char *psource, unsigned char *pdest) {
	
	/*
	insert the source string at the beginning of the destination string.
	
	return false if the resulting string would be longer than 255 chars.
	*/
	
	register short len1 = stringlength (psource), len2 = stringlength (pdest);
	bigstring bs;
	
	if ((len1 + len2) > lenbigstring) /*resulting string would be too long*/
		return (false);
		
	copystring (psource, bs);
	
	pushstring (pdest, bs);
	
	copystring (bs, pdest);
	
	return (true);
	} /*insertstring*/


boolean insertchar (char ch, void *pstring) {
	
	byte *pdest = (byte *) pstring;
	short len = stringlength (pdest);
	
	if (len == lenbigstring)
		return (false);
	
	moveright (pdest + 1, pdest + 2, len);
	
	setstringlength (pdest, len + 1);
	
	pdest [1] = ch;
	
	return (true);
	} /*insertchar*/


void stringtonumber (void *pstring, long *num, boolean *flerror) {
	
	unsigned char *bs = (unsigned char *) pstring;
	short len = stringlength (bs);
	boolean notfirstchar = false;
	short i;
	
	for (i = 1; i <= len; i++) {
		
		char ch = bs [i];
		
		if (ch == ' ')
			continue;
			
		if ((ch == '+') || (ch == '-')) {
			
			if (notfirstchar) {
				
				*flerror = true;
				
				return;
				}
				
			notfirstchar = true;
			
			continue;
			}
			
		if ((ch < '0') || (ch > '9')) {
			
			*flerror = true;
			
			return;
			}
			
		notfirstchar = true;
		} /*for*/
	
	StringToNum ((ConstStr255Param) bs, num); /*call the toolbox routine*/
	
	*flerror = false;
	} /*stringtonumber*/
	
	
void getsizestring (unsigned long size, void *pstring) {
	
	unsigned char *bs = (unsigned char *) pstring;
	unsigned long meg = 1024L * 1024L;
	
	if (size > meg) { /*greater than 1 megabyte*/
		
		unsigned long leftofdecimal, rightofdecimal;
		bigstring bsright;
		
		leftofdecimal = size / meg;
		
		rightofdecimal = ((size % meg) + 50) / 100;
		
		NumToString (leftofdecimal, (byte *) bs);
		
		NumToString (rightofdecimal, bsright);
		
		setstringlength (bsright, 1);
		
		if (bsright [1] != '0') {
		
			pushstring ("\p.", bs);
		
			pushstring (bsright, bs);
			}
		
		pushstring ("\pMB", bs);
		}
	else {
		unsigned long x = size / 1024;
	
		if ((size % 1024) > 512)
			x++;
		
		if (x == 0)
			copystring ("\pzero ", bs);
		else
			NumToString (x, (byte *) bs);
			
		pushstring ("\pK", bs);
		}
	} /*getsizestring*/		


void gettimestring (long time, void *pstring) {
	
	unsigned char *bs = (unsigned char *) pstring;
	bigstring bstime;
	
	IUDateString (time, abbrevDate, (byte *) bs);
	
	IUTimeString (time, false, bstime);
	
	pushstring ("\p; ", bs);
	
	pushstring (bstime, bs);
	} /*gettimestring*/	
	
	
void commentdelete (byte chdelim, void *pstring) {
	
	/*
	scan the string from left to right.  if we find a comment delimiting
	character, delete all the characters to the right, including the
	delimiter and return.
	
	10/30/91 dmb: ignore comment characters that are embedded in quoted strings
	*/
	
	unsigned char *bs = (unsigned char *) pstring;
	byte chcomment = chdelim;
	short i, ct;
	byte ch;
	boolean flinstring = false;
	byte chendstring;
	
	ct = stringlength (bs);
	
	for (i = 1; i <= ct; i++) {
		
		ch = bs [i];
		
		if (flinstring)
			flinstring = ch != chendstring;
		
		else if (ch == (byte) '"') {
			
			flinstring = true;
			
			chendstring = '"';
			}
		
		else if (ch == (byte) 'р') {
			
			flinstring = true;
			
			chendstring = 'с';
			}
		
		else if (ch == chcomment) {
			
			setstringlength (bs, i - 1);
			
			return;
			}
		}
	} /*commentdelete*/
	
	
static boolean whitespacechar (char ch) {
	
	return (ch == ' ');
	} /*whitespacechar*/
	

boolean poptrailingwhitespace (void *pstring) {
	
	/*
	return true if there were trailing "whitespace" characters to be popped.
	*/
	
	unsigned char *bs = (unsigned char *) pstring;
	short i, ct;
	
	ct = stringlength (bs);
	
	for (i = ct; i > 0; i--)
	
		if (!whitespacechar (bs [i])) { /*found a non-blank character*/
			
			setstringlength (bs, i);
			
			return (i < ct);
			}
	
	setemptystring (bs);
	
	return (true); /*string is all blank*/
	} /*poptrailingwhitespace*/


static boolean midinsertstring (void *pinsert, void *pstring, short ixinsert) {
	
	/*
	insert bsinsert in the indicated string at offset ixinsert.
	*/
	
	unsigned char *bsinsert = (unsigned char *) pinsert, *bs = (unsigned char *) pstring;
	short ix = ixinsert;
	short leninsert = stringlength (bsinsert);
	short origlen = stringlength (bs);
	short newlen = leninsert + origlen;
	
	if (newlen > lenbigstring)
		return (false);
	
	moveright (&bs [ix], &bs [ix + leninsert], (long) origlen - ix + 1);
	
	moveleft (&bsinsert [1], &bs [ix], (long) leninsert);
	
	setstringlength (bs, newlen);
	
	return (true);
	} /*midinsertstring*/


boolean replacestring (void *preplace, short ix, short ctreplace, void *pinsert) {
	
	/*
	4/3/92 dmb: check for overflow before doing deletion
	*/
	
	unsigned char *bsreplace = (unsigned char *) preplace, *bsinsert = (unsigned char *) pinsert;
	
	if (stringlength (bsreplace) + (stringlength (bsinsert) - ctreplace) > lenbigstring)
		return  (false);
	
	deletestring (bsreplace, ix, ctreplace);
	
	midinsertstring (bsinsert, bsreplace, ix);
	
	return (true);
	} /*replacestring*/


boolean stringfindchar (byte ch, void *pstring) {
	
	/*
	simpler entrypoint for scanstring when caller just wants 
	to know if ch appears anywhere in bs
	*/
	
	short ix = 1;
	
	return (scanstring (ch, pstring, &ix));
	} /*stringfindchar*/


void parseparamstring (bigstring bssource, bigstring bs0, bigstring bs1, bigstring bs2, bigstring bs3, bigstring bsresult) {
	
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
	short len;
	short i;
	byte ch;
	StringPtr params [4];
	short paramnum;
	
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
	
	/***subtractstrings (bsresult, "\p рс", bsresult); /*in case there was a missing param*/
	} /*parseparamstring*/


void colortostring (RGBColor rgb, bigstring bs) {
	
	setstringlength (bs, 0);
	
	pushint (rgb.red, bs);
	
	pushchar (',', bs);
	
	pushint (rgb.green, bs);
	
	pushchar (',', bs);
	
	pushint (rgb.blue, bs);
	} /*colortostring*/
	 
	
void stringtocolor (bigstring bs, RGBColor *rgb) {
	
	bigstring bsint;
	long num;
	boolean flerror;
	
	(*rgb).red = (*rgb).blue = (*rgb).green = 0;
	
	if (firstword (bs, ',', bsint)) {
		
		stringtonumber (bsint, &num, &flerror);
		
		if (!flerror)
			(*rgb).red = num;
		
		deletestring (bs, 1, stringlength (bsint) + 1);
		}
	
	if (firstword (bs, ',', bsint)) {
		
		stringtonumber (bsint, &num, &flerror);
		
		if (!flerror)
			(*rgb).green = num;
		
		deletestring (bs, 1, stringlength (bsint) + 1);
		}
	
	stringtonumber (bs, &num, &flerror);
		
	if (!flerror)
		(*rgb).blue = num;
	} /*stringtocolor*/
	
	 
boolean stringendswith (bigstring ending, bigstring bs, boolean flunicase) {
	
	short endinglength = stringlength (ending);
	bigstring bscopy;
	
	if (stringlength (bs) < endinglength)
		return (false);
		
	midstring (bs, stringlength (bs) - endinglength + 1, endinglength, bscopy);
	
	if (flunicase)
		return (unicaseequalstrings (bscopy, ending));
	else
		return (equalstrings (bscopy, ending));
	} /*stringendswith*/
	

	
	
	
