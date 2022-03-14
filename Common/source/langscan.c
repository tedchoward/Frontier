
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
#include "ops.h"
#include "yytab.h" /*token defines*/
#include "lang.h"
#include "langinternal.h"
#include "langparser.h"
#include "byteorder.h"	/* 2006-04-08 aradke: endianness conversion macros */



#define chstartcomment (byte) chcomment

#define chendscanstring (byte)0 /*returned when we've run out of text*/

unsigned long ctscanlines; /*number of lines that have been scanned, for error reporting*/

unsigned short ctscanchars; /*number of chars passed over on current line, for error reporting*/


static Handle hscanstring; /*this is the text that we're parsing*/

static boolean fllinebasedscan;

static long ixparsestring; /*index of next character to be returned*/

static long lenparsestring; /*the number of characters in the parse string*/

static boolean flsenteol;


bigstring bstoken; /*for viewing with the debugger, text of last token*/




boolean isfirstidentifierchar (byte ch) {
	
	/*
	could this character be the first character in an identifier?
	*/
	
	return (isalpha (ch) || (ch == '_'));
	
	/*
		((ch >= 'a') && (ch <= 'z')) ||

		((ch >= 'A') && (ch <= 'Z')) ||

		(ch == '_'));
	*/
	} /*isfirstidentifierchar*/
	
	
boolean isidentifierchar (byte ch) {
	
	/*
	could the character be the second through nth character 
	in an identifier?
	*/
	
	if (isfirstidentifierchar (ch))
		return (true);
	
	if (isdigit (ch))
		return (true);
		
	if (ch == chtrademark)
		return (true);
		
	return (false);
	} /*isidentifierchar*/


boolean langisidentifier (bigstring bs) {
	
	/*
	called externally to determine when quoting in necessary in path 
	construction
	
	4.1b2 dmb: check the constants table too
	*/
	
	register short ct = stringlength (bs);
	register byte *s = bs;
	tyvaluerecord val;
	hdlhashnode hnode;
	
	if (ct == 0) /*empty string*/
		return (false);
	
	if (!isfirstidentifierchar (*++s))
		return (false);
	
	while (--ct > 0)
		if (!isidentifierchar (*++s))
			return (false);
	
	if (hashtablelookup (hkeywordtable, bs, &val, &hnode)) /*it's a keyword*/
		return (false);
	
	if (hashtablelookup (hconsttable, bs, &val, &hnode)) /*dmb 4.1b2 - it's a constant*/
		return (false);
	
	return (true);
	} /*langisidentifier*/



#if (odbengine==0)
static boolean midinsertchar (byte ch, bigstring bs, short ixinsert) {
	
	byte bs1 [4];  /*rab 3/21/97 was 2*/
	
	if (ixinsert > lenbigstring)
		return (false);
	
	if (stringlength (bs) == lenbigstring) /*overflow -- push character off end*/
		
		setstringlength (bs, lenbigstring - 1);
	
	setstringwithchar (ch, bs1);
	
	return (midinsertstring (bs1, bs, ixinsert)); /*should always be true*/
	} /*midinsertchar*/


boolean langdeparsestring (bigstring bs, byte chendquote) {
	
	/*
	add any necessary escape sequences to make the string compilable. 
	return true if we don't have to truncate the string to do so.
	*/
	
	register byte ch;
	register short ix;
	register short ct = stringlength (bs);
	byte bshex [16]; /*should only need 7 bytes*/
	
	for (ix = 1; --ct >= 0; ++ix) {
		
		ch = bs [ix];
		
		if (isprint (ch)) { /*look for the few printable characters that might need quoting*/
			
			switch (ch) {
				
				case '\\':
					break;
				
				case '\'':
				case '\"':
				case chclosecurlyquote:
					if (ch != chendquote) /*don't always need to quote these*/
						ch = 0;
					
					break;
					
				default:
					ch = 0;
					
					break;
				}
			
			if (ch == 0) /*no quote necessary*/
				continue;
			}
		
		if (ch >= 128) /*extended ascii*/
			continue;
		
		if (!midinsertchar ('\\', bs, ix++))
			return (false);
		
		switch (ch) {
			
			case '\n':
				ch = 'n';
				
				break;
			
			case '\r':
				ch = 'r';
				
				break;
			
			case '\t':
				ch = 't';
				
				break;
			
			case '\\':
			case '\'':
			case '\"':
			case chclosecurlyquote:
				break;
			
			default:
				if (!midinsertchar ('x', bs, ix++))
					return (false);
				
				numbertohexstring (ch, bshex);
				
				if (!midinsertchar (bshex [5], bs, ix++)) /*skip over 0x00*/
					return (false);
				
				ch = bshex [6];
				
				break;
			}
		
		if (ix > lenbigstring)
			return (false);
		
		bs [ix] = ch;
		}
	
	return (true);
	} /*langdeparsestring*/


void parsesetscanstring (Handle htext, boolean fllinebased) {
	
	hscanstring = htext; /*copy into global*/
	
	fllinebasedscan = fllinebased;
	
	lenparsestring = gethandlesize (htext);
	
	ixparsestring = 0;
	
	ctscanlines = 1;
	
	ctscanchars = 0;
	
	flsenteol = false;
	} /*parsesetscanstring*/


unsigned long parsegetscanoffset (unsigned long ctlines, unsigned short ctchars) {
	
	/*
	convert ctlines/ctchars to a zero-based, absolute offset consistent 
	with the parsing of a non-linebased scan string
	*/
	
	return (((((unsigned long) ctlines) - 1) << 16) + ctchars);
	} /*parsegetscanoffset*/


void parsesetscanoffset (unsigned long offset) {
	
	/*
	convert a zero-based, absolute offset to a ctlines/ctchars pair consistent 
	with the parsing of a non-linebased scan string, and set our globals to 
	that value (for error reporting)
	*/
	
	ctscanlines = 1 + (offset >> 16);
	
	ctscanchars = offset & 0xffff;
	} /*parsesetscanoffset*/


static boolean parsestringempty (void) {
	
	return (ixparsestring >= lenparsestring);
	} /*parsestringempty*/
	
	
static byte parsepopchar (void) {
	
	/*
	return the first character from the parse string and remove it
	from the string.  if there are no characters in the string we 
	return the null character.
	
	5/7/93 dmb: support non-linebased scans.  instead of returns, we 
	start a new "line" when the character count is about to overflow.
	
	Dave, sorry about the ?: construct -- it's the only one in the product!  :)
	*/
	
	register ptrbyte p;
	register byte ch;
	
	if (ixparsestring >= lenparsestring) /*string is empty*/
		return (chendscanstring);
	
	p = (ptrbyte) *hscanstring + ixparsestring++;
	
	ch = *p;
	
	ctscanchars++; /*for error reporting*/

//************ RAB ADDED 10/29/97
	if (ixparsestring < lenparsestring) {
		p = (ptrbyte) *hscanstring + ixparsestring;
		if (*p == chlinefeed) {  //lose it
			++ixparsestring;
			++ctscanchars;
			}
		}
	
	if (fllinebasedscan? (ch == chreturn) : (ctscanchars == 0xffff)) { /*passed over another line, or overflowing*/
		
		ctscanlines++; /*for error reporting*/
		
		ctscanchars = 0; /*the number of chars we've passed over in the text*/
		}
	
	return (ch);
	} /*parsepopchar*/


static byte parsefirstchar (void) {
	
	/*
	return the character at the head of the parse stream without 
	popping it.
	*/
	
	register long ix = ixparsestring;
	register ptrbyte p;
	
	if (ix >= lenparsestring)
		return (chendscanstring);

	p = (ptrbyte) *hscanstring + ix;
	
	return (*p);
	} /*parsefirstchar*/
	
	
static byte parsenextchar (void) {
	
	/*
	return the character at the head of the parse stream without 
	popping it.
	*/
	
	register long ix = ixparsestring + 1;
	register ptrbyte p;
	
	if (ix >= lenparsestring)
		return (chendscanstring);
	
	p = (ptrbyte) *hscanstring + ix;
	
	return (*p);
	} /*parsenextchar*/


static void parsepopidentifier (bigstring bs) {
	
	/*
	pull characters off the front of the input stream as long as
	we're still getting identifier characters.  
	*/
	
	setstringlength (bs, 0);
	
	while (true) {
		
		if (!isidentifierchar (parsefirstchar ())) /*finished accumulating identifier*/
			return;
		
		pushchar (parsepopchar (), bs); /*add char to the end of the string*/
		} /*while*/
	} /*parsepopidentifier*/


static boolean parsepopnumber (tyvaluerecord *val) {
	
	/*
	pull characters off the front of the input stream as long as
	we're still getting digits.  when we hit the first non-digit,
	convert what we got into a long and return it.
	
	we expect at least one numeric digit to be there, and do not
	provide for an error return.
	
	5/29/91 dmb: support hex constants in the form "0xhhhhhhhh"
	
	10/21/91 dmb: detect overly-large numbers, like 999999999999.
	*/
	
	bigstring bsnumber;
	register boolean flhex = false;
	register boolean flfloat = false;
	
	if (parsefirstchar () == '0') { /*check for hex constant*/
		
		parsepopchar ();
		
		if (parsefirstchar () == 'x') {
			
			parsepopchar ();
			
			flhex = true;
			}
		}
	
	setemptystring (bsnumber);
	
	while (true) {
		
		register byte ch = parsefirstchar ();
		
		if ((ch == '.') && !flfloat)
			flfloat = true;
		
		else {
			
			if (!isdigit (ch) && !(flhex && isxdigit (ch))) {
				
				if (flfloat) {
					
					double d;
					Handle x;
					
					stringtofloat (bsnumber, &d);
					
					if (!newfilledhandle (&d, longsizeof (d), &x))
						return (false);
					
					initvalue (val, doublevaluetype);
					
					(*val).data.binaryvalue = x;
					}
				else {
					long x;
					bigstring bstest;
					
					if (flhex) {
						
						if (stringlength (bsnumber) > 10)
							goto overflow;
						
						hexstringtonumber (bsnumber, &x);
						}
					else {
						stringtonumber (bsnumber, &x);
						
						popleadingchars (bsnumber, '0');
						
						numbertostring (x, bstest);
						
						popleadingchars (bstest, '0');
						
						if (!equalstrings (bsnumber, bstest))
							goto overflow;
						}
					
					setlongvalue (x, val);
					}
				
				#ifdef fldebug
				
				copystring (bsnumber, bstoken); /*for debugging*/
				
				#endif
				
				return (true);
				}
			}
		
		pushchar (parsepopchar (), bsnumber);
		} /*while*/
	
	overflow:
	
	langparamerror (numbertoolargeerror, bsnumber);
	
	return (false);
	} /*parsepopnumber*/


static byte parsepopescapesequence (void) {
	
	/*
	get the next string character out of the input stream, i.e. a 
	character that is part of a string or character constant.  this is 
	where we handle backslashes for special characters
	*/
	
	register byte ch;
	bigstring bs;
	long x;
	
	ch = parsepopchar ();
	
	switch (ch) {
		
		case 'n':
			return ('\n');
		
		case 'r':
			return ('\r');
		
		case 't':
			return ('\t');
		
		case '\\':
			return ('\\');
		
		case '\'':
			return ('\'');
		
		case '\"':
			return ('\"');
		
		case 'x':
			if (!isxdigit (parsefirstchar ()))
				return (0);
			
			ch = parsepopchar ();
			
			setstringwithchar (ch, bs);
			
			if (isxdigit (parsefirstchar ()))
				pushchar (parsepopchar (), bs);
			
			hexstringtonumber (bs, &x);
			
			return ((byte) x);
		
		default:
			return (ch);
		}
	} /*parsepopescapesequence*/


static boolean buildtexthandle (bigstring bs, Handle *htext) {
	
	/*
	5.0.2 dmb: move the characters from bs to the given text handle.
	
	either create or add to htext.
	*/
	
	boolean fl;

	if (*htext == nil)
		fl = newtexthandle (bs, htext);
	else
		fl = pushtexthandle (bs, *htext);
	
	return (fl);
	} /*buildtexthandle*/


static boolean parsepopstringconst (Handle *htext) {
	
	/*
	pop a string constant off the front of the input stream and
	return a handle to the string allocated in the heap.
	
	return with an error if the string wasn't properly
	terminated, or if there was an allocation error.
	
	5/6/93: don't allow a string to span input lines
	
	2.1b2 dmb: handle escape sequences
	
	2.1b6 dmb: if an escape sequence is for a nul character, we 
	can't tell if that's chendscanstring or not; so check for 
	that before parsing '\'s.  we'll find out soon enough if we're 
	really out of text to scan

	5.0.2 dmb: don't limit string literals to 255 chars; use new 
	buildtexthandle for continuation
	*/
	
	register byte ch, chstop;
	bigstring bs;
	unsigned long lnum;
	unsigned short cnum;
	
	chstop = parsepopchar (); /*pop off opening doublequote, our terminator*/
	
	lnum = ctscanlines;
	
	cnum = ctscanchars;
	
	if (chstop == chopencurlyquote)
		chstop = chclosecurlyquote;
	else
		chstop = (byte) '"';
	
	*htext = nil;

	setstringlength (bs, 0);
	
	while (true) {
		
		ch = parsepopchar ();
		
		if (ch == chstop) /*properly terminated string*/
			return (buildtexthandle (bs, htext));
		
		if (ch == chreturn) /*don't allow string to span lines*/
			break;
		
		if (ch == chendscanstring) /*ran out of characters*/
			break;
		
		if (ch == '\\')
			ch = parsepopescapesequence ();
		
		if (!pushchar (ch, bs)) { /*add the char to the end of the string*/
			
			if (!buildtexthandle (bs, htext))
				return (false);

			setstringwithchar (ch, bs);
			}
		} /*while*/
	
	ctscanlines = lnum; /*make error message point at string start*/
	
	ctscanchars = cnum;
	
	langerror (stringnotterminatederror);
	
	return (false);
	} /*parsepopstringconst*/
	

static void parsepopcomment (void) {
	
	/*
	consume characters up to and including the next chendcomment.
	
	comments cannot span more than one line, so endofline also causes
	us to return.
	*/
	
	register byte ch;
	
	while (true) {
		
		ch = parsepopchar ();
		
		if ((ch == chendcomment) || (ch == chreturn) || (ch == chendscanstring))
			return;
		} /*while*/
	} /*parsepopcomment*/


static boolean parsepopblanks (void) {
	
	/*
	pop all the leading white space.  return false if the input stream is 
	empty, true otherwise.

	5.0a12 dmb: handle // comments
	*/
	
	register byte ch;
	
	while (true) {
		
		if (parsestringempty ())
			return (false);
		
		ch = parsefirstchar ();
		
		if ((ch == chstartcomment) || (ch == '/' && parsenextchar () == '/')) {
			
			parsepopcomment (); /*pop everything up to and including endcomment char*/
			}
			
		else {
			if ((ch != ' ') && (ch != chtab) && (ch != chreturn))
				return (true);
			
			parsepopchar (); /*consume a whitespace character*/
			}
		} /*while*/
	} /*parsepopblanks*/
	

static boolean parsepopcharconst (tyvaluerecord *val) {
	
	/*
	pop a character constant in the form of '<char>' off the input
	stream.  return with error if there was no char following the 
	first ' or if the character immediately following <char> is not another '.
	
	3/29/91 dmb: handle 4-character ostype constants as well 1-char.
	
	2.1b2 dmb: handle escape sequences
	
	2.1b6 dmb: see comment in parsepopstringconst; check for the end 
	of the scan string before parsing escape sequences
	*/
	
	register byte ch;
	register short len = 0;
	byte ch4 [4];
	OSType osvalue;
	unsigned long lnum;
	unsigned short cnum;
	
	parsepopchar (); /*get rid of first single-quote*/
	
	lnum = ctscanlines;
	
	cnum = ctscanchars;
	
	while (true) {
		
		ch = parsepopchar ();
		
		if (ch == chsinglequote) { /*end of char const*/
			
			switch (len) {
				
				case 1: /*normal character const*/
					
					setcharvalue (ch4 [0], val);
					
					#ifdef fldebug
					
					pushchar ((*val).data.chvalue, bstoken); /*for debugging*/
					
					#endif
					
					return (true);
				
				case 4: /*ostype (long) character const*/
					
					moveleft (ch4, &osvalue, 4L);
					
					memtodisklong (osvalue);

					setostypevalue (osvalue, val);
					
					#ifdef fldebug
					
					ostypetostring (osvalue, bstoken); /*for debugging*/
					
					#endif
					
					return (true);
				
				default:
					goto error;
				}
			}
		
		if (len == 4) /*about to get too large*/
			goto error;
		
		if (ch == chendscanstring) /*test before doing esc sequences*/
			goto error;
		
		if (ch == '\\')
			ch = parsepopescapesequence ();
		
		ch4 [len++] = ch; /*add character to constant*/
		} /*while*/
	
	error:
	
	ctscanlines = lnum; /*make error message point at string start*/
	
	ctscanchars = cnum;
	
	langerror (badcharconsterror);
	
	return (false);
	} /*parsepopcharconst*/
	

static tokentype langscanner (hdltreenode *nodetoken) {
	
	/*
	scan the input string for the next token, return the character 
	that we stopped on in chtoken.
	
	if it's an identifier or a constant, nodetoken will be non-nil.
	
	11/22/91 dmb: return zero when out of text, after returning exactly 
	one (non-zero) eoltoken.
	
	5.0.2b10 dmb: exempt const identifier from tmp stack before pushing it
	into code tree. it will be disposed on error
	*/
	
	register byte ch, chfirst, chsecond;
	bigstring bs;
	tyvaluerecord val;
	hdlhashnode hnode;
	
	*nodetoken = nil; /*default*/
	
	#ifdef fldebug
	
	setstringlength (bstoken, 0); 
	
	#endif
	
	if (!parsepopblanks ()) { /*ran out of text*/
		
		if (flsenteol)
			return (0);
		
		flsenteol = true; /*we're about to...*/
		
		return (eoltoken);
		}
	
	chfirst = ch = parsefirstchar (); /*lookahead at the next character*/
	
	if (ch == chsinglequote) { /*a single-quote, character constant*/
		
		if (!parsepopcharconst (&val))
			return (errortoken);
		
		if (!newconstnode (val, nodetoken))
			return (0 /*errortoken*/);
			
		return (constanttoken);
		}
	
	if ((ch == chdoublequote) || (ch == chopencurlyquote)) { /*a string constant*/
		
		initvalue (&val, stringvaluetype);
		
		if (!parsepopstringconst (&val.data.stringvalue))
			return (errortoken);
		
		#ifdef fldebug
		
		texthandletostring (val.data.stringvalue, bstoken); /*8/13*/
		
		#endif
		
		if (!newconstnode (val, nodetoken))
			return (0 /*errortoken*/);
		
		return (constanttoken);
		}
	
	if (isdigit (ch)) {
		
		if (!parsepopnumber (&val)) /*might be a long or a float*/
			return (errortoken);
		
		if (!newconstnode (val, nodetoken))
			return (0 /*errortoken*/);
		
		return (constanttoken);
		}
	
	if (isfirstidentifierchar (ch)) {
		
		register boolean fl;
		
		parsepopidentifier (bs);
		
		#ifdef fldebug
		
		copystring (bs, bstoken);
		
		#endif
		
		fl = hashtablelookup (hkeywordtable, bs, &val, &hnode);
		
		if (fl) 
			return ((tokentype) val.data.tokenvalue); /*it's a reserved word*/
		
		fl = hashtablelookup (hconsttable, bs, &val, &hnode);
		
		if (fl) { /*it's a pre-defined constant*/
			
			if (!copyvaluerecord (val, &val))
				return (0);
			
			exemptfromtmpstack (&val);
			
			if (!newconstnode (val, nodetoken))
				return (0 /*errortoken*/);
			
			return (constanttoken);
			}
		
		initvalue (&val, stringvaluetype);
		
		if (!newtexthandle (bs, &val.data.stringvalue))
			return (0 /*errortoken*/);
		
		if (!newidnode (val, nodetoken))
			return (0 /*errortoken*/);
		
		return (identifiertoken);
		}
	
	parsepopchar (); /*consume the token char*/
	
	chsecond = parsefirstchar (); /*may need to look ahead to determine this token*/
	
	#ifdef fldebug
	
	pushchar (chfirst, bstoken);
	
	#endif
	
	switch (chfirst) {
		
		case ',': case '(': case ')': case ';': case '{': case '}': case '.':
		
		case ':':
		
		case '[': case ']': case '@': case '^':
			return (chfirst); /*the ascii value is the token*/
		
		case (byte) '¥':
			return ('.');
		
		case chnotequals:
			return (NEtoken);
			
		case '*':
			return (multiplytoken);
		
		case '/': case chdivide:
			return (dividetoken);
		
		case '%':
			return (modtoken);
		
		case (byte) '²':
			return (LEtoken);
			
		case (byte) '³':
			return (GEtoken);
			
		case '+':
			if (chsecond == '+') {
				
				parsepopchar (); /*consume the second char*/
				
				#ifdef fldebug
				
				pushchar ('+', bstoken);
				
				#endif
				
				return (plusplustoken);
				}
				
			return (addtoken);
			
		case '-':
			if (chsecond == '-') {
				
				#ifdef fldebug
				
				pushchar ('-', bstoken);
				
				#endif
				
				parsepopchar (); /*consume the second char*/
				
				return (minusminustoken);
				}
				
			return (subtracttoken);
			
		case '=':
			if (chsecond == '=') {
				
				parsepopchar (); /*consume the second char*/
				
				#ifdef fldebug
				
				pushchar ('=', bstoken);
				
				#endif
				
				return (EQtoken);
				}
				
			return (assigntoken);
			
		case '&':
			if (chsecond == '&') {
				
				parsepopchar (); /*consume the second char*/
				
				#ifdef fldebug
				
				pushchar ('&', bstoken);
				
				#endif
				
				return (andandtoken);
				}
				
			return (bitandtoken);
			
		case '|':
			if (chsecond == '|') {
				
				parsepopchar (); /*consume the second char*/
				
				#ifdef fldebug
				
				pushchar ('|', bstoken);
				
				#endif
				
				return (orortoken);
				}
				
			return (bitortoken);
			
		case '<':
			if (chsecond == '=') {
				
				parsepopchar (); /*consume the second char*/
				
				#ifdef fldebug
				
				pushchar ('=', bstoken);
				
				#endif
				
				return (LEtoken);
				}
				
			return (LTtoken);
			
		case '>':
			if (chsecond == '=') {
				
				parsepopchar (); /*consume the second char*/
				
				#ifdef fldebug
				
				pushchar ('=', bstoken);
				
				#endif
				
				return (GEtoken);
				}
				
			return (GTtoken);
			
		case '!':
			if (chsecond == '=') {
				
				parsepopchar (); /*consume the second char*/
				
				#ifdef fldebug
				
				pushchar ('=', bstoken);
				
				#endif
				
				return (NEtoken);
				}
				
			return (nottoken);
		} /*switch*/
	
	setstringwithchar (chfirst, bstoken);
	
	//assert (ixparsestring <= lenparsestring && hscanstring);
	
	langparamerror (illegaltokenerror, bstoken); /*all the legal tokens are caught above*/
	
	return (errortoken);
	} /*langscanner*/


tokentype parsegettoken (hdltreenode *nodetoken) {
	
	/*
	a bottleneck that makes debugging easier.  
	
	if you want to see the string that generated the current token, 
	display "bstoken" -- its a globalÉ
	*/
	
	register tokentype token;
	
	token = langscanner (nodetoken);
	
	return (token);
	} /*parsegettoken*/


boolean langstriptextsyntax (Handle htext) {
	
	/*
	strip out braces and semicolons from the source text, so it 
	can be pasted into a script outline
	*/
	
	boolean fldone = false;
	hdltreenode hnode;
	tokentype token, token2 = 0;
	long ixstart;
	long ix1 = 0, ix2 = 0;
	unsigned long line2 =  0;
	short ctendbraces = 0;
	
	parsesetscanstring (htext, true);
	
	disablelangerror ();
	
	while (!fldone) {
		
		ixstart = ixparsestring;
		
		token = langscanner (&hnode);
		
		langdisposetree (hnode); /*we don't need it*/
		
		if (ix2 > 0) { /*something waiting to be stripped*/
			
			if ((token2 == '}') && (ctendbraces > 0))
				--ctendbraces;
			
			else {
				
				if ((token2 == '}') || (ctscanlines > line2)) { /*we got a return, go ahead & strip it*/
					
					short len;
					
					if (token2 == '}') /* 2005-01-15 creedon - fix suggested by JES w/caveats < http://sourceforge.net/tracker/index.php?func=detail&aid=1093595&group_id=120666&atid=687798 > */
						ix1 = ix2 - 1;

					len = ix2 - ix1;
					
					pullfromhandle (htext, ix1, len, nil); /*get rid of the token source*/
					
					lenparsestring -= len; /*make adjustments*/
					
					ixparsestring -= len; /*ditto*/
					
					ixstart -= len;
					
					ctendbraces = 0; /*reset*/
					}
				else {
					
					if (token2 == '{')
						++ctendbraces;
					}
				}
			
			ix2 = 0; /*reset*/
			}
		
		switch (token) {
			
			case errortoken:
			case eoltoken:
				fldone = true;
				
				break;
			
			case '{':
			case '}':
			case ';':
				token2 = token;
				
				ix1 = ixstart;
				
				ix2 = ixparsestring;
				
				line2 = ctscanlines;
				
				break;
			
			default:
				break;
			}
		}
	
	if (ix2 > 0) /*something waiting to be stripped*/
		pullfromhandle (htext, ix1, ix2 - ix1, nil);
	
	enablelangerror ();
	
	return (true);
	} /*langstriptextsyntax*/


boolean langaddapplescriptsyntax (Handle hscript) {
	
	/*
	add vertical bars where necessary to "quote" UserTalk dotted 
	identifiers for AppleScript.
	*/
	
	boolean fldone = false;
	hdltreenode hnode;
	tokentype token, lasttoken = 0;
	long ixstart;
	long ix1 = 0;
	unsigned long line1 = 0;
	bigstring bsid;
	boolean flgotid = false;
	boolean flgotdottedid = false;
	byte chbar = '|';
	
	parsesetscanstring (hscript, true);
	
	disablelangerror ();
	
	while (!fldone) {
		
		ixstart = ixparsestring;
		
		token = langscanner (&hnode);
		
		switch (token) {
			
			/*
			case errortoken:
				flgotbar == firstchar (bstoken) == (byte) '|';
				
				flgotid = false;
				
				break;
			*/
			
			case eoltoken:
				fldone = true;
				
				break;
			
			case identifiertoken:
				if (flgotid && lasttoken == '.') { /*continuation of a dotted id*/
					
					flgotdottedid = true;
					}
				else { /*treat as beginning of a new id*/
					
					flgotid = true;
					
					flgotdottedid = false;
					
					pullstringvalue (&(**hnode).nodeval, bsid);
					
					ix1 = ixparsestring - stringlength (bsid);
					
					line1 = ctscanlines;
					}
				
				break;
			
			case '.':
				if (lasttoken == identifiertoken) /*don't have dottedid, until we get another identifier*/
					flgotdottedid = false;
				else
					flgotid = false;
				
				break;
			
			case '(':
				if (flgotid && flgotdottedid && ctscanlines == line1) { /*we're there*/
					
					insertinhandle (hscript, ixstart, &chbar, sizeof (chbar)); /*do second bar 1st*/
					
					insertinhandle (hscript, ix1, &chbar, sizeof (chbar));
					
					lenparsestring += 2; /*make adjustments*/
					
					ixparsestring += 2; /*ditto*/
					}
				
				flgotid = false;
				
				break;
			
			default:
				/*
				flgotbar = false;
				*/
				
				flgotid = false;
				
				break;
			}
		
		langdisposetree (hnode); /*we don't need it anymore*/
		
		lasttoken = token;
		}
	
	enablelangerror ();
	
	return (true);
	} /*langaddapplescriptsyntax*/



/*
yyoverflow (bsevent, p1, size1, p2, size2, p3, size3, p4) bigstring bsevent; ptrbyte p1, p2, p3; short size1, size2, size3; {
	
	DebugStr ("\pyyoverflow");
	} /%yyoverflow%/
*/

#endif





	
	
	

