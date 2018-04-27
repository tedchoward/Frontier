
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
#include "strings.h"
#include "ops.h"
#include "resources.h"
#include "lang.h"
#include "langinternal.h"
#include "langhtml.h"
#include "langsystem7.h"
#include "tablestructure.h"
#include "kernelverbs.h"
#include "kernelverbdefs.h"
#include "timedate.h"
#include "oplist.h"
#include "tableverbs.h"

#include "md5.h" //6.1b11 AR


/*Latin to Mac conversion table: 7.0b28 PBS*/
/* moved to stringverbs.c: 2004-10-28 aradke */

static unsigned char latintomactable [] = {

	0, 1, 2, 3,
	4, 5, 6, 7,
	8, 9, 10, 11,
	12, 13, 14, 15,
	16, 17, 18, 19,
	20, 21, 22, 23,
	24, 25, 26, 27,
	28, 29, 30, 31,
	32, 33, 34, 35,
	36, 37, 38, 39,
	40, 41, 42, 43,
	44, 45, 46, 47,
	48, 49, 50, 51,
	52, 53, 54, 55,
	56, 57, 58, 59,
	60, 61, 62, 63,
	64, 65, 66, 67,
	68, 69, 70, 71,
	72, 73, 74, 75,
	76, 77, 78, 79,
	80, 81, 82, 83,
	84, 85, 86, 87,
	88, 89, 90, 91,
	92, 93, 94, 95,
	96, 97, 98, 99,
	100, 101, 102, 103,
	104, 105, 106, 107,
	108, 109, 110, 111,
	112, 113, 114, 115,
	116, 117, 118, 119,
	120, 121, 122, 123,
	124, 125, 126, 127,
	165, 170, 173, 176,
	179, 183, 186, 189,
	195, 197, 201, 209,
	212, 217, 218, 182,
	198, 206, 226, 227,
	228, 240, 246, 247,
	249, 250, 251, 253,
	254, 255, 245, 196,
	202, 193, 162, 163,
	219, 180, 207, 164,
	172, 169, 187, 199,
	194, 208, 168, 248,
	161, 177, 211, 210,
	171, 181, 166, 225,
	252, 213, 188, 200,
	185, 184, 178, 192,
	203, 231, 229, 204,
	128, 129, 174, 130,
	233, 131, 230, 232,
	237, 234, 235, 236,
	220, 132, 241, 238,
	239, 205, 133, 215,
	175, 244, 242, 243,
	134, 160, 222, 167,
	136, 135, 137, 139,
	138, 140, 190, 141,
	143, 142, 144, 145,
	147, 146, 148, 149,
	221, 150, 152, 151,
	153, 155, 154, 214,
	191, 157, 156, 158,
	159, 224, 223, 216
	};

/*Mac to Latin conversion table: 7.0b35 PBS*/
/* moved to stringverbs.c: 2004-10-28 aradke */

static unsigned char mactolatintable [] = {

	0, 1, 2, 3,
	4, 5, 6, 7,
	8, 9, 10, 11,
	12, 13, 14, 15,
	16, 17, 18, 19,
	20, 21, 22, 23,
	24, 25, 26, 27,
	28, 29, 30, 31,
	32, 33, 34, 35,
	36, 37, 38, 39,
	40, 41, 42, 43,
	44, 45, 46, 47,
	48, 49, 50, 51,
	52, 53, 54, 55,
	56, 57, 58, 59,
	60, 61, 62, 63,
	64, 65, 66, 67,
	68, 69, 70, 71,
	72, 73, 74, 75,
	76, 77, 78, 79,
	80, 81, 82, 83,
	84, 85, 86, 87,
	88, 89, 90, 91,
	92, 93, 94, 95,
	96, 97, 98, 99,
	100, 101, 102, 103,
	104, 105, 106, 107,
	108, 109, 110, 111,
	112, 113, 114, 115,
	116, 117, 118, 119,
	120, 121, 122, 123,
	124, 125, 126, 127,
	196, 197, 199, 201,
	209, 214, 220, 225,
	224, 226, 228, 227,
	229, 231, 233, 232,
	234, 235, 237, 236,
	238, 239, 241, 243,
	242, 244, 246, 245,
	250, 249, 251, 252,
	221, 176, 162, 163,
	167, 128, 182, 223,
	174, 169, 129, 180,
	168, 130, 198, 216,
	131, 177, 190, 132,
	165, 181, 143, 133,
	189, 188, 134, 170,
	186, 135, 230, 248,
	191, 161, 172, 136,
	159, 137, 144, 171,
	187, 138, 160, 192,
	195, 213, 145, 166,
	173, 139, 179, 178,
	140, 185, 247, 215,
	255, 141, 142, 164,
	208, 240, 222, 254,
	253, 183, 146, 147,
	148, 194, 202, 193,
	203, 200, 205, 206,
	207, 204, 211, 212,
	149, 210, 218, 219,
	217, 158, 150, 151,
	175, 152, 153, 154,
	184, 155, 156, 157
	};


#define stringerrorlist 263
#define notimplementederror 1


typedef enum tystringtoken { /*verbs that are processed by string.c*/
	
	deletefunc,
	
	insertfunc,
	
	popleadingfunc,
	
	poptrailingfunc,
	
	trimwhitespacefunc,
	
	popsuffixfunc,
	
	hassuffixfunc,
	
	midfunc,
	
	nthcharfunc,
	
	nthfieldfunc,
	
	countfieldsfunc,
	
	setwordcharfunc,
	
	getwordcharfunc,
	
	firstwordfunc,
	
	lastwordfunc,
	
	nthwordfunc,
	
	countwordsfunc,
	
	commentdeletefunc,
	
	firstsentencefunc,
	
	patternmatchfunc,
	
	hexfunc,
	
	timestringfunc,
	
	datestringfunc,
	
	uppercasefunc,
	
	lowercasefunc,
	
	filledstringfunc,
	
	addcommasfunc,
	
	replacefunc,
	
	replaceallfunc,
	
	lengthfunc,
	
	isalphafunc,
	
	isnumericfunc,
	
	ispunctuationfunc,
	
	processmacrosfunc,
	
	urldecodefunc,
	
	urlencodefunc,
	
	parseargsfunc,
	
	iso8859encodefunc,

	getgifheightwidthfunc,

	getjpegheightwidthfunc,

	wrapfunc,

	davenetmassagerfunc,
	
	parseaddressfunc,
	
	dropnonalphasfunc,
	
	padwithzerosfunc,
	
	ellipsizefunc,
	
	innercasefunc,
	
	urlsplitfunc,

	hashmd5func,
	
	latintomacfunc,
	
	mactolatinfunc, /*7.0b35 PBS*/

	utf16toansifunc, /*7.0b41 PBS*/

	utf8toansifunc, /*7.0b41 PBS*/

	ansitoutf8func, /*7.0b41 PBS*/

	ansitoutf16func, /*7.0b42 PBS*/
	
	multiplereplaceallfunc, /*7.1b17 PBS*/

	macromantoutf8func, // 2006-02-25 creedon
	
	utf8tomacromanfunc, // 2006-02-25 creedon
	
	convertcharsetfunc, /* 2006-04-14 smd */
	
	ischarsetavailablefunc, /* 2006-04-23 smd */

	ctstringverbs
	} tystringtoken;


char chword = ' '; /*used when getting first word, last word, etc from a string*/



long countleadingchars (Handle htext, byte ch) {
	
	long len = gethandlesize (htext);
	long i;
	
	for (i = 0; i < len; i++) {
		
		if ((*(byte **) htext) [i] != ch)
			return (i);
		} /*for*/
	
	return (len);
	} /*countleadingchars*/


void handlepopleadingchars (Handle htext, byte ch) {

	pullfromhandle (htext, 0, countleadingchars (htext, ch), nil);

	} /*handlepopleadingchars*/


void handlepoptrailingchars (Handle htext, byte ch) {

	long i;
	long len = gethandlesize (htext);

	if (len <= 0)
		return;

	for (i = len - 1; i >= 0; i--)
		if ((*(byte **) htext) [i] != ch)
			break;
	
	sethandlesize (htext, i + 1); /*shrinking, can't fail*/
	} /*handlepoptrailingchars*/


static void handletrimwhitespace (Handle htext) {

	long len = gethandlesize (htext);
	long ixbegin, ixend;
	
	if (len <= 0)
		return;
	
	for (ixbegin = 0; ixbegin < len; ixbegin++)		
		if ((*htext) [ixbegin] != ' '
				&& (*htext) [ixbegin] != '\t'
				&& (*htext) [ixbegin] != '\r'
				&& (*htext) [ixbegin] != '\n')
			break;

	for (ixend = len - 1; ixend >= 0; ixend--)
		if ((*htext) [ixend] != ' '
				&& (*htext) [ixend] != '\t'
				&& (*htext) [ixend] != '\r'
				&& (*htext) [ixend] != '\n')
			break;

	if (ixend < ixbegin)
		sethandlesize (htext, 0);
	else {
		sethandlesize (htext, ixend + 1); /*shrinking, can't fail*/
		pullfromhandle (htext, 0, ixbegin, nil);
		}

	return;
	} /*handletrimwhitespace*/


static void handlepopsuffix (Handle h, byte ch) {
	
	long ix, len = gethandlesize (h);
	
	if (len <= 0)
		return;
	
	for (ix = len - 1; ix >= 0; ix--)
		if ((byte)((*h) [ix]) == ch) {
		
			sethandlesize (h, ix);
			
			return;
			}
	
	return;
	} /*handlepopsuffix*/


static boolean handlehassuffix (Handle hsuffix, Handle h) {
	
	return (searchhandle (h, hsuffix, gethandlesize (h) - gethandlesize (hsuffix), longinfinity) >= 0);
	} /*handlehassuffix*/


static boolean ellipsizehandle (Handle h, long maxlen) {
	
	long len = gethandlesize (h);
	long ix;
	
	if (len <= maxlen)
		return (true);
	
	if (maxlen < 3)
		maxlen = 3;

	if (!sethandlesize (h, maxlen))
		return (false);
	
	for (ix = 1; ix <= 3; ix++)
		(*h) [maxlen - ix] = '.';
	
	return (true);
	} /*handleellipsize*/


static void fillstring (ptrvoid pfill, long ctfill, bigstring bsfill) {
	
	/*
	do a mass memory fill, like memset, but of a repeated string --
	copy ctfill bsfills at pfill.
	*/
	
	ptrbyte p = pfill;
	long ct = ctfill;
	short len = stringlength (bsfill);
	ptrbyte s = bsfill + 1;
	
	while (--ct >= 0) {
		
		moveleft (s, p, len);
		
		p += len;
		}
	} /*fillstring*/


static boolean wordwraphandle (Handle htext) {
	
	/*
	DW 12/3/94: rewrote the routine. it's now a two-pass thing.
	
	5.0.2b8 dmb: handle cr-lf line endings. lf-only line ending are not handled.
	*/
	
	unsigned long handlesize = gethandlesize (htext);
	unsigned long ix = 0;
	char ch, chnext, chprev = ' ';
	boolean fllinestart = true;
		
	for (ix = 0; ix < handlesize; ++ix) { /*pass 1 -- remove leading white space on each line*/
	
		if (ix >= handlesize)
			break;
		
		ch = (*htext) [ix];
		
		if (fllinestart && ((ch == ' ') || (ch == chtab) || (ch == chlinefeed))) {
			
			moveleft (&(*htext) [ix + 1], &(*htext) [ix], handlesize - ix);
		
			handlesize--;
			
			ix--;
			
			continue;
			}
			
		if (ch == chreturn /*|| ch == chlinefeed*/) {
			
			fllinestart = true;
			
			continue;
			}
		
		fllinestart = false; 
		} /*while*/
	
	for (ix = 0; ix < handlesize; chprev = ch, ++ix) { /*pass 2 -- strip out the extraneous hard returns*/
		
		ch = (*htext) [ix];
		
		if (ch != chreturn) 
			continue;
		
		if (ix == (handlesize - 1)) { /*last char is a return*/
			
			/*sethandlesize (htext, handlesize - 1);*/ /*disabled -- the text might not be the whole message*/
			
			break; /*finished processing the text*/
			}
		
		chnext = (*htext) [ix + 1];
		
	//	if (chnext == chlinefeed) {
	//		
	//		if (ix == (handlesize - 2)) /*last chars are cr-lf*/
	//		break;
	//		
	//		chnext = (*htext) [ix + 2];
	//		}
		
		if ((chnext == chreturn) || (chprev == chreturn)) /*leave double or triple returns alone*/
			continue;
		
		if ((chnext != ' ') && (chprev != ' ')) {
			
			(*htext) [ix] = ' '; /*replace it with a space*/
			
			continue;
			}
			
		moveleft (&(*htext) [ix + 1], &(*htext) [ix], handlesize - ix);
		
		handlesize--;
		} /*while*/
		
	sethandlesize (htext, handlesize);
		
	return (true);
	} /*wordwraphandle*/
					
				
static boolean daveNetMassager (short indentlen, short maxlinelen, Handle h, Handle *hmassaged) {
	
	unsigned long handlesize = gethandlesize (h);
	unsigned long ixhandle = 0;
	short linelen, wordlen;
	bigstring indent, dashes, word, cr, space;
	Handle hnew;
	short i;
	
	if (!newemptyhandle (&hnew))
		return (false);
		
	filledstring (' ', indentlen, indent);
	
	setstringlength (dashes, 0);
	
	setstringlength (word, 0);
	
	setstringwithchar (chreturn, cr);
	
	setstringwithchar (' ', space);
	
	for (i = 1; i <= 16; i++)
		pushstring (BIGSTRING ("\x04" " ---"), dashes);
	
	if (!pushtexthandle (indent, hnew))
		goto error;
		
	linelen = indentlen;
	
	while (true) {
	
		if (ixhandle >= handlesize)
			break;
		
		if ((*h) [ixhandle] == chreturn) {
			
			if (equalstrings (word, BIGSTRING ("\x03" "---"))) {
				
				if (!pushtexthandle (dashes, hnew))
					goto error;
				
				setstringlength (word, 0);
				}
			
			if (!pushtexthandle (cr, hnew))
				goto error;
				
			if (!pushtexthandle (indent, hnew))
				goto error;
				
			linelen = indentlen;
				
			ixhandle++;
			}
		else { /*first char is not a return*/
			
			while (true) { /*skip over leading blanks*/
				
				if ((*h) [ixhandle] != ' ')
					break;
					
				ixhandle++;
				
				if (ixhandle >= handlesize)
					goto exit;
				} /*while*/
			
			setstringlength (word, 0);
			
			while (true) { /*pop off the first word*/
				
				char ch = (*h) [ixhandle];
				
				if ((ch == ' ') || (ch == chreturn)) 
					break;
				
				pushchar (ch, word);
				
				ixhandle++;
				
				if (ixhandle >= handlesize)
					break;
				} /*while*/
				
			while (true) { /*skip over leading blanks*/
				
				if (ixhandle >= handlesize)
					break;
				
				if ((*h) [ixhandle] != ' ')
					break;
					
				ixhandle++;
				} /*while*/
			
			wordlen = stringlength (word);
			
			if ((linelen + wordlen) > maxlinelen) {
				
				if (!pushtexthandle (cr, hnew))
					goto error;
					
				if (!pushtexthandle (indent, hnew))
					goto error;
					
				if (!pushtexthandle (word, hnew))
					goto error;
					
				if (!pushtexthandle (space, hnew))
					goto error;
					
				linelen = indentlen + wordlen;
				}
			else {
				if (!pushtexthandle (word, hnew))
					goto error;
					
				if (!pushtexthandle (space, hnew))
					goto error;
					
				linelen += wordlen;
				}
			}
		} /*while*/
	
	exit:
	
	*hmassaged = hnew;
	
	return (true);
	
	error:
	
	disposehandle (hnew);
	
	*hmassaged = nil;
	
	return (false);
	} /*daveNetMassager*/
	


static boolean getpositivelongvalue (hdltreenode hparam1, short pnum, long *x) {
	
	if (!getlongvalue (hparam1, pnum, x))
		return (false);
	
	if (*x < 0) {
		
		langerror (numbernotpositiveerror);
		
		return (false);
		}
	
	return (true);
	} /*getpositivelongvalue*/


static boolean stringreplaceverb (hdltreenode hparam1, tyvaluerecord *vreturned, boolean flreplaceall) {
	
	/*
	6.1d7 AR: Now handles unicase searching.

	6.1b11 AR: Use getoptionalparamvalue.
	*/

	Handle h = nil;
	Handle hfind;
	Handle hreplace;
	tyvaluerecord vcase;
	short ctconsumed = 3;
	short ctpositional = 3;
	
	if (!getexempttextvalue (hparam1, 1, &h))
		return (false);

	if (!getreadonlytextvalue (hparam1, 2, &hfind))
		goto exit;
	
	if (!getreadonlytextvalue (hparam1, 3, &hreplace))
		goto exit;
	
	setbooleanvalue (true, &vcase);

	flnextparamislast = true;

	if (!getoptionalparamvalue (hparam1, &ctconsumed, &ctpositional, BIGSTRING ("\x0f""flCaseSensitive"), &vcase)) 
		goto exit;

	if (!textfindreplace (hfind, hreplace, h, flreplaceall, !(boolean)vcase.data.flvalue))
		goto exit;
	
	return (setheapvalue (h, stringvaluetype, vreturned));

exit:

	disposehandle (h);

	return (false);
	} /*stringreplaceverb*/


static boolean commentdeleteverb (hdltreenode hp1, tyvaluerecord *v) {
	
	/*
	5.018 dmb: skip over :// for URLs

	5.0.1 dmb: need to skip on more char to handle file:///
	*/

	Handle x;
	
	flnextparamislast = true;
	
	if (!getexempttextvalue (hp1, 1, &x))
		return (false);
	
	if (!textcommentdelete (x)) {
		
		disposehandle (x);
		
		return (false);
		}
	
	return (setheapvalue (x, stringvaluetype, v));
	} /*commentdeleteverb*/

boolean parseaddress (Handle htext, tyvaluerecord *v); /*called from langhtml.c*/

boolean parseaddress (Handle htext, tyvaluerecord *v) {
	
	/*
	5.0.2b8 dmb: new verb
	
	6.1d4 AR: report proper error message if langcompiletext fails.
	*/
	
	hdltreenode hmodule;
	hdllistrecord hlist = nil;
	boolean fl;
	unsigned short savelines;
	unsigned short savechars;
		
	savelines = ctscanlines;
	
	savechars = ctscanchars;
	
	disablelangerror ();
	
	fl = langcompiletext (htext, false, &hmodule); /*always disposes htext*/
	
	enablelangerror ();
	
	ctscanlines = savelines;
	
	ctscanchars = savechars;

	if (!fl) {
		
		langerror (parseaddresserror); /*Can't parse the address because of a syntax error.*/
		
		return (false);
		}

	fl = opnewlist (&hlist, false);
	
	disablelangerror ();
	
	fl = fl && langbuildnamelist ((**hmodule).param1, hlist);
	
	enablelangerror ();
	
	langdisposetree (hmodule);
	
	if (!fl) {
			
		opdisposelist (hlist);
		
		langerror (parseaddresserror); /*Can't parse the address because it's not a valid address.*/
		
		return (false);
		}
	
	return (setheapvalue ((Handle) hlist, listvaluetype, v));
	} /*parseaddressverb*/


static boolean parseaddressverb (hdltreenode hp1, tyvaluerecord *v) {
		
	Handle htext;
	
	flnextparamislast = true;
	
	if (!getexempttextvalue (hp1, 1, &htext))
		return (false);
		
	return (parseaddress (htext, v));
	} /*parseaddressverb*/


static boolean dropnonalphasverb (hdltreenode hp1, tyvaluerecord *v) {
	
	/*
	5.0.2b14 dmb: sucked into kernel
	
	5.1.4 dmb: gets moved into new streamdropnonalphas
	*/
	
	Handle x;
	handlestream s;
	
	flnextparamislast = true;
	
	if (!getexempttextvalue (hp1, 1, &x))
		return (false);
	
	openhandlestream (x, &s);
	
	streamdropnonalphas (&s);
	
	closehandlestream (&s);
	
	return (setheapvalue (x, stringvaluetype, v));
	} /*dropnonalphasverb*/


static boolean padwithzerosverb (hdltreenode hp1, tyvaluerecord *v) {
	
	/*
	6.2a8 AR: Rewritten to remove 256 char limit (which would sometimes crash Frontier)
	*/

	Handle h;
	long paddedlength, currentlength, delta;

	if (!getexempttextvalue (hp1, 1, &h))
		return (false);

	currentlength = gethandlesize (h);

	flnextparamislast = true;

	if (!getlongvalue (hp1, 2, &paddedlength))
		goto error;

	delta = paddedlength - currentlength;

	if (delta > 0) {

		if (!sethandlesize (h, paddedlength))
			goto error;

		moveright (*h, *h + delta, currentlength);

		memset (*h, '0', delta);
		}

	return (setheapvalue (h, stringvaluetype, v));

error:

	disposehandle (h);

	return (false);
	} /*padwithzerosverb*/


static boolean stringmultiplereplace (Handle h, hdlhashtable ht, boolean flunicase, bigstring bsstartstring, bigstring bsendstring) {
	
	/*
	7.0b58 PBS: front end for string.replaceAll -- do a replace all for
	each item in a table.
	*/
	
	hdlhashnode hn;
	tyvaluerecord v;
	bigstring hashkey;
	boolean flstartstring = false, flendstring = false;

	if (stringlength (bsstartstring) > 0)
		flstartstring = true;

	if (stringlength (bsendstring) > 0)
		flendstring = true;

	for (hn = (**ht).hfirstsort; hn != nil; hn = (**hn).sortedlink) {
		
		v = ((**hn).val);
		
		if (copyvaluerecord (v, &v) && coercevalue (&v, stringvaluetype)) {
			
			Handle hfind;
			
			gethashkey (hn, hashkey);
			
			if (!newtexthandle (hashkey, &hfind))
				return (false);
			
			if (flstartstring) {
				if (!inserttextinhandle (hfind, 0, bsstartstring)) {
					disposehandle (hfind);
					return (false);
					} /*if*/
			} /*if*/

			if (flendstring) {
				if (!inserttextinhandle (hfind, gethandlesize (hfind), bsendstring)) {
					disposehandle (hfind);
					return (false);
					} /*if*/
				} /*if*/

			if (!textfindreplace (hfind, v.data.stringvalue, h, true, flunicase)) {
				disposehandle (hfind);
				return (false);
				} /*if*/
			
			disposehandle (hfind);

			disposevaluerecord (v, false); // don't let tmp stack accumulate
			} /*if*/			
		} /*for*/
	
	return (true);
	} /*stringmultiplereplace*/


static boolean stringmultiplereplaceallverb (hdltreenode hp, tyvaluerecord *v) {
	
	/*
	7.0b58 PBS: do multiple replace-alls from a table. Used by localization
	support in Manila and mainResponder.
	
	2002-10-30 AR: actually get optional parameters by name rather than by position
	*/
	
	Handle h;
	hdlhashtable ht;
	boolean flcasesensitive = true;
	bigstring bsstartstring, bsendstring;
	short ctconsumed = 0;
	short ctpositional;
	tyvaluerecord vcasesensitive, vstartchars, vendchars;

	if (!getexempttextvalue (hp, ++ctconsumed, &h))
		return (false);

	if (!gettablevalue (hp, ++ctconsumed, &ht)) {
		disposehandle (h);
		return (false);
		} /*if*/	
	
	ctpositional = ctconsumed;
	
	/* get optional flCaseSensitive param */
	
	setbooleanvalue (true, &vcasesensitive);
	
	if (!getoptionalparamvalue (hp, &ctconsumed, &ctpositional, (BIGSTRING ("\x0f" "flCaseSensitive")), &vcasesensitive))
		return (false);
	
	flcasesensitive = vcasesensitive.data.flvalue;
	
	/* get optional startCharacters param */
	
	initvalue (&vstartchars, stringvaluetype);
	
	if (!getoptionalparamvalue (hp, &ctconsumed, &ctpositional, (BIGSTRING ("\x0f" "startCharacters")), &vstartchars))
		return (false);
	
	pullstringvalue (&vstartchars, bsstartstring);
	
	/* get optional endCharacters param */
	
	flnextparamislast = true;
	
	initvalue (&vendchars, stringvaluetype);
	
	if (!getoptionalparamvalue (hp, &ctconsumed, &ctpositional, (BIGSTRING ("\x0d" "endCharacters")), &vendchars))
		return (false);
	
	pullstringvalue (&vendchars, bsendstring);

	/* now do the replacement */
			
	stringmultiplereplace (h, ht, !flcasesensitive, bsstartstring, bsendstring);
	
	return (setheapvalue (h, stringvaluetype, v));	
	} /*stringmultiplereplaceallverb*/


static void grabnthfield (Handle htext, long fieldnum, byte chdelim, Handle *hfield, boolean flstrict) {
	
	/* 12/5/2004 smd: optimization - now takes hfield parameter, a pointer to the 
	handle for the extracted field */
	
	long ixword, lenword;
	
	if (!textnthword ((ptrbyte) *htext, gethandlesize (htext), fieldnum, chdelim, flstrict, &ixword, &lenword))
		lenword = 0;
	
	if (!loadfromhandletohandle (htext, &ixword, lenword, false, hfield))
		newemptyhandle (hfield);
	
	} /*grabnthfield*/



static void macuppertext (byte *p, long ct) {
	
	/*
	5.1.1 dmb: we use UpperText for handle international chars, but the 
	length is a short...
	*/
	//Code change by Timothy Paustian Monday, June 19, 2000 3:09:12 PM
	//Updated to new routine.
	//UppercaseText(p, ct, smSystemScript);	
	
	/*7.0b28 PBS: restore the loop. This way we don't
	have a >32K bug.*/

	while (ct > infinity) {
		
		
			UppercaseText ((Ptr) p, infinity, smSystemScript);	
		
		
		p += infinity;
		
		ct -= infinity;
		}
	
	
		UppercaseText ((Ptr) p, ct, smSystemScript);	
	
		
	} /*macuppertext*/


static void maclowertext (byte *p, long ct) {
	
	/*
	5.1.1 dmb: we use LowerText for handle international chars, but the 
	length is a short...
	*/
	//Code change by Timothy Paustian Monday, June 19, 2000 3:09:28 PM
	//Changed to Opaque call for Carbon
//	LowercaseText(p, ct, smSystemScript);

	/*7.0b28 PBS: restore the loop. This way we don't
	have a >32K bug.*/
	
	while (ct > infinity) {
		

			LowercaseText ((Ptr) p, infinity, smSystemScript);
	
		
		p += infinity;
		
		ct -= infinity;
		}
	

		LowercaseText ((Ptr) p, ct, smSystemScript);

	} /*maclowertext*/



static void innercasehandle (Handle h) {
	
	long ix, len;
	byte ch;

	lockhandle (h);

	maclowertext ((byte *) (*h), gethandlesize (h));
	
	unlockhandle (h);

	handlepopleadingchars (h, ' ');

	handlepoptrailingchars (h, ' ');
	
	len = gethandlesize (h);
	
	if (len <= 0)
		return;

	lockhandle (h);

	for (ix = len - 1; ix >= 0; ix--)
		if ((*h) [ix] == ' ') {
			
			moveleft (*h + ix + 1, *h + ix, len - ix);
			
			ch = toupper ((*h) [ix]);
			
			(*h) [ix] = ch;
			
			len--;
			}

	unlockhandle (h);
	
	sethandlesize (h, len);

	return;
	} /*innercasehandle*/


static boolean urlsplit (Handle h, tyvaluerecord *vreturn) {
	
	/*
	6.1d5 AR: Assume a valid complete URL, split it into three parts, return as a list.
	*/
	
	Handle htemp;
	hdllistrecord hlist = nil;
	long ix = 0, ct = 0;
	long len = gethandlesize (h);
	
	if (len <= 0)
		goto urlerror;
	
	for (ct = 0; ct < len; ct++)
		if ((*h) [ct] == ':')
			break;
	
	if (ct >= len)
		goto urlerror;
	
	while (++ct < len)
		if ((*h) [ct] != '/')
			break;
	
	if (ct >= len)
		goto urlerror;
	
	if (!opnewlist (&hlist, false))
		goto exit;
		
	if (!loadfromhandletohandle (h, &ix, ct, false, &htemp)
			|| !langpushlisttext (hlist, htemp))
		goto exit;

	for (ct = ix; ct < len; ct++)
		if ((*h) [ct] == '/')
			break;

	if (ct >= len)
		goto urlerror;

	if (!loadfromhandletohandle (h, &ix, ct - ix, false, &htemp)
			|| !langpushlisttext (hlist, htemp))
		goto exit;
	
	ix++;
	
	if (ix == len) {
		if (!newemptyhandle (&htemp))
			goto exit;
		}
	else
		if (!loadfromhandletohandle (h, &ix, len - ix, false, &htemp))
			goto exit;
	
	if (!langpushlisttext (hlist, htemp))
		goto exit;

	assert (ix == len); /*make sure we got the whole thing*/

	return (setheapvalue ((Handle) hlist, listvaluetype, vreturn));

urlerror:
	langerror (urlspliterror);

exit:	
	opdisposelist (hlist);
	
	return (false);
	} /*urlsplit*/


void latintomac (Handle h) {
	
	/*
	7.0b28 PBS: convert text from Latin to Mac character sets.
	*/
	
	long ix = 0;
	long lentext = gethandlesize (h);
	
	while (true) {
	
		unsigned char ch = (*h) [ix];
		unsigned char chreplace;
		
		if (ch > 127) {
		
			chreplace = latintomactable [ch];
		
			(*h) [ix] = chreplace;
			} /*if*/
			
		ix++;
		
		if (ix >= lentext)
			break;
		} /*while*/
	} /*latintomac*/


void mactolatin (Handle h) {
	
	/*
	7.0b35 PBS: convert text from Mac to Latin character sets.
	*/
	
	long ix = 0;
	long lentext = gethandlesize (h);
	
	while (true) {
	
		unsigned char ch = (*h) [ix];
		unsigned char chreplace;
		
		if (ch > 127) {
		
			chreplace = mactolatintable [ch];
		
			(*h) [ix] = chreplace;
			} /*if*/
			
		ix++;
		
		if (ix >= lentext)
			break;
		} /*while*/
	} /*latintomac*/


static boolean isCharsetAvailableVerb( hdltreenode hp1, tyvaluerecord *v )
{
	bigstring bscharset;
	
	flnextparamislast = true;
	
	if ( !getstringvalue( hp1, 1, bscharset ) ) /* IANA names can't be longer than 40 ASCII characters, so this is safe */
		return (false);
	
	(*v).data.flvalue = isTextEncodingAvailable( bscharset );
	
	return (true);
}


static boolean stringfunctionvalue (short token, hdltreenode hparam1, tyvaluerecord *vreturned, bigstring bserror) {
	
	/*
	bridges string.c with the language.  the name of the verb is bs, its first parameter
	is hparam1, and we return a value in vreturned.
	
	we use a limited number of support routines from lang.c to get parameters and
	to return values. 
	
	return false only if the error is serious enough to halt the running of the script
	that called us, otherwise error values are returned through the valuerecord, which
	is available to the script.
	
	if we return false, we try to provide a descriptive error message in the 
	returned string bserror.
	
	8/15/91: most string verbs now deal with any length text.  for efficiency, 
	many call getstringcopy, which returns a copy of a string handle.  
	to ease error handling, these parameters are retrieved last, even 
	though they're often the first parameter.
	
	10/13/91 dmb: set return value to false by default, so on error it's always false
	
	10/30/91 dmb: if nthword is out of range, return the empty string
	
	2/14/92 dmb: replaced getstringcopy calls with getexempttextvalue, which is new 
	and does the job better.  also, recoded string.mid so that existing handle is 
	reused instead of creating a new one.
	
	3/9/92 dmb: use Script manager for upper/lower casing; added countfields, nthfield; 
	added nthchar (to kernel)
	
	6/3/92 dmb: firstwordfunc now uses (nthword, 1) to match nthwordfunc
	
	9/11/92 dmb: added insertfuc (used to be just glue)
	
	3/3/92 dmb: lock handle when calling Upper/LowerText

	5.0a12 dmb: handle // comments
	
	5.0.2b17 dmb: use new getreadonlytextvalue where appropriate
	
	5.1.1 dmb: remove 32K limit in string.upper/lower [Mac]
	
	6.1d4 AR: added popTrailing, trimWhiteSpace, popSuffix, ellipsize, innerCaseName
	
	6.1d5 AR: added urlSplit, hasSuffix.
	string.dateString and string.timeString now optionally accept a date parameter.
	
	12/4/2004 smd: optimization - midfunc (ie string.mid) now uses a readonly handle for
	the input string, and copies only the results to the output string, for performance
	
	12/5/2004 smd: optimization - deletefunc now uses a readonly handle for the input
	string, creates a new handle sized precisely for the output string, and copies 
	two parts from the input to the output (substrings before and after the deleted chars)
	
	2006-02-25 creedon: added macRomanToUtf8 and utf8ToMacRoman
	*/
	
	hdltreenode hp1 = hparam1;
	tyvaluerecord *v = vreturned;
	short errornum = 0;
	bigstring bs; //utility
	
	setbooleanvalue (false, v); /*by default, string functions return false*/
	
	switch (token) {
		
		case setwordcharfunc:
			flnextparamislast = true;
			
			if (!getcharvalue (hp1, 1, &chword))
				return (false);
			
			(*v).data.flvalue = true;
			
			return (true);
			
		case getwordcharfunc:
			if (!langcheckparamcount (hp1, 0)) /*no parameters expected*/
				return (false);
			
			return (setcharvalue (chword, v));
	
		case deletefunc: {
			/* 12/5/2004 smd: optimized */
			
			Handle x;
			Handle hstring;
			long ctdelete;
			long ixdelete;
			long lenbefore;
			long ixbefore;
			long ixafter;
			long lenafter;
			long hlen;
			
			if (!getpositivelongvalue (hp1, 2, &ixdelete))
				return (false);
			
			flnextparamislast = true;
			
			if (!getpositivelongvalue (hp1, 3, &ctdelete))
				return (false);
			
			if (!getreadonlytextvalue (hp1, 1, &hstring))
				return (false);
			
			hlen = gethandlesize (hstring);
			
			if (ixdelete > 0)
				--ixdelete; /*convert to zero-base*/
			
			lenbefore = ixdelete;
			
			ctdelete = min (ctdelete, hlen - ixdelete); /* in case ctdelete is too big */
			
			ixafter = ixdelete + ctdelete;
			
			lenafter = hlen - ixafter;
			
			if (!newhandle (lenbefore + lenafter, &x))
				return (false);
			
			ixbefore = 0;
			if (lenbefore > 0)
				if (!loadfromhandle (hstring, &ixbefore, lenbefore, *x))
					return (false);
			
			ixbefore += ctdelete;
			if (lenafter > 0)
				if (!loadfromhandle (hstring, &ixbefore, lenafter, &((*x) [lenbefore]) ))
					return (false);
			
			return (setheapvalue (x, stringvaluetype, v));
			}
		
		case insertfunc: {
			Handle hsource;
			Handle hdest;
			long ixinsert;
			boolean fl;
			
			if (!getreadonlytextvalue (hp1, 1, &hsource)) 
				return (false);
			
			flnextparamislast = true;
			
			if (!getpositivelongvalue (hp1, 3, &ixinsert))
				return (false);
			
			if (!getexempttextvalue (hp1, 2, &hdest)) /*get last to simplify error handing*/
				return (false);
			
			if (ixinsert > 0) /*convert to zero-base*/
				--ixinsert;
			
			ixinsert = min (ixinsert, gethandlesize (hdest));
			
			lockhandle (hsource);
			
			fl = insertinhandle (hdest, ixinsert, *hsource, gethandlesize (hsource));
			
			unlockhandle (hsource);
			
			if (!fl) {
				
				disposehandle (hdest);
				
				return (false);
				}
			
			return (setheapvalue (hdest, stringvaluetype, v));
			}
		
		case popleadingfunc: {
			Handle x;
			char ch;
			
			flnextparamislast = true;
			
			if (!getcharvalue (hp1, 2, &ch))
				return (false);
			
			if (!getexempttextvalue (hp1, 1, &x))
				return (false);
			
			handlepopleadingchars (x, ch);
			
			return (setheapvalue (x, stringvaluetype, v));
			}

		case poptrailingfunc: {
			Handle x;
			char ch;
			
			flnextparamislast = true;
			
			if (!getcharvalue (hp1, 2, &ch))
				return (false);
			
			if (!getexempttextvalue (hp1, 1, &x))
				return (false);
			
			handlepoptrailingchars (x, ch);
			
			return (setheapvalue (x, stringvaluetype, v));
			}

		case trimwhitespacefunc: {
			Handle x;
			
			flnextparamislast = true;
			
			if (!getexempttextvalue (hp1, 1, &x))
				return (false);
			
			handletrimwhitespace (x);
			
			return (setheapvalue (x, stringvaluetype, v));
			}
		
		case popsuffixfunc: {
			Handle x;
			char ch = '.';

			if (langgetparamcount (hp1) > 1) {
				
				flnextparamislast = true;
				
				if (!getcharvalue (hp1, 2, &ch))
					return (false);
				}
						
			if (!getexempttextvalue (hp1, 1, &x)) /*get last to simplify error handling*/
				return (false);
			
			handlepopsuffix (x, ch);
			
			return (setheapvalue (x, stringvaluetype, v));
			}

		case hassuffixfunc: {
			Handle h, hsuffix;
			
			if (!getreadonlytextvalue (hp1, 1, &hsuffix))
				return (false);
			
			flnextparamislast = true;
			
			if (!getreadonlytextvalue (hp1, 2, &h))
				return (false);
			
			return (setbooleanvalue (handlehassuffix (hsuffix, h), v));
			}
		
		case midfunc: {
			
			/* 12/4/2004 smd: optimized */
			
			Handle hstring;
			Handle x;
			long ix;
			long newlen;
			
			
			if (!getpositivelongvalue (hp1, 2, &ix))
				return (false);
			
			flnextparamislast = true;
			
			if (!getlongvalue (hp1, 3, &newlen)) 
				return (false);
			
			if (!getreadonlytextvalue (hp1, 1, &hstring)) /*get last to simplify error handling*/
				return (false);
			
			if (ix > 0)
				--ix; /*convert to zero-base*/
			
			newlen = min (newlen, gethandlesize (hstring) - ix);
			
			
			if (newlen > 0) {
				if (!newhandle (newlen, &x))
					return (false);
				if (!loadfromhandle (hstring, &ix, newlen, *x))
					return (false);
				}
			else if (!newemptyhandle (&x))
				return (false);
			
			return (setheapvalue (x, stringvaluetype, v));
			}
		
		case nthcharfunc: {
			Handle htext;
			byte ch;
			long charnum;
			
			if (!getreadonlytextvalue (hp1, 1, &htext)) 
				return (false);
			
			flnextparamislast = true;
			
			if (!getpositivelongvalue (hp1, 2, &charnum))
				return (false);
			
			if (charnum == 0 || charnum > gethandlesize (htext))
				return (setstringvalue (zerostring, v));
			
			ch = *((byte *) *htext + charnum - 1);
			
			return (setcharvalue (ch, v));
			}
		
		case nthfieldfunc: {
			Handle htext;
			Handle x;
			char chdelim;
			long fieldnum;
			
			/*
			if (!gettextvalue (hp1, 1, &htext)) 
				return (false);
			*/
			
			if (!getcharvalue (hp1, 2, &chdelim)) {
				return (false);
				} /*if*/
			
			flnextparamislast = true;
			
			if (!getpositivelongvalue (hp1, 3, &fieldnum)) {
				return (false);
				} /*if*/			

			if (!getreadonlytextvalue (hp1, 1, &htext)) /*get last to simplify error handing*/
				return (false);

			grabnthfield (htext, fieldnum, chdelim, &x, true);
			
			return (setheapvalue (x, stringvaluetype, v));
			}
		
		case countfieldsfunc: {
			Handle htext;
			char chdelim;
			long ctfields;
			
			if (!getreadonlytextvalue (hp1, 1, &htext)) 
				return (false);
			
			flnextparamislast = true;
			
			if (!getcharvalue (hp1, 2, &chdelim))
				return (false);
			
			ctfields = textcountwords ((ptrbyte) *htext, gethandlesize (htext), chdelim, true);
			
			setlongvalue (ctfields, v);
			
			return (true);
			}
		
		case firstwordfunc: {
			Handle htext;
			Handle x;
			
			flnextparamislast = true;
			
			if (!getreadonlytextvalue (hp1, 1, &htext)) /*get last to simplify error handing*/
				return (false);
			
			grabnthfield (htext, 1, chword, &x, false);
			
			return (setheapvalue (x, stringvaluetype, v));
			}
		
		case lastwordfunc: {
			Handle htext;
			
			flnextparamislast = true;
			
			if (!getreadonlytextvalue (hp1, 1, &htext)) 
				return (false);
			
			textlastword ((ptrbyte) *htext, gethandlesize (htext), chword, bs);
			
			return (setstringvalue (bs, v));
			}
	
		case nthwordfunc: {
			Handle htext;
			Handle x;
			long wordnum;
			
			flnextparamislast = true;
			
			if (!getpositivelongvalue (hp1, 2, &wordnum))
				return (false);
			
			if (!getreadonlytextvalue (hp1, 1, &htext)) /*get last to simplify error handing*/
				return (false);
			
			grabnthfield (htext, wordnum, chword, &x, false);
			
			return (setheapvalue (x, stringvaluetype, v));
			}
		
		case countwordsfunc: {
			Handle htext;
			long ctwords;
			
			flnextparamislast = true;
			
			if (!getreadonlytextvalue (hp1, 1, &htext)) 
				return (false);
			
			ctwords = textcountwords ((ptrbyte) *htext, gethandlesize (htext), chword, false);
			
			setlongvalue (ctwords, v);
			
			return (true);
			}
		
		case commentdeletefunc:
			return (commentdeleteverb (hp1, v));
		
		case firstsentencefunc:
			flnextparamislast = true;
			
			if (!getstringvalue (hp1, 1, bs)) 
				return (false);
			
			firstsentence (bs);
			
			return (setstringvalue (bs, v));
		
		case uppercasefunc: {
			Handle x;
			
			flnextparamislast = true;
			
			if (!getexempttextvalue (hp1, 1, &x))
				return (false);
			
			lockhandle (x);
			
			macuppertext ((byte *) (*x), gethandlesize (x));
			
			unlockhandle (x);
			
			return (setheapvalue (x, stringvaluetype, v));
			}
		
		case lowercasefunc: {
			Handle x;
			
			flnextparamislast = true;
			
			if (!getexempttextvalue (hp1, 1, &x))
				return (false);
			
			/*
			*/
			
			lockhandle (x);
		
			maclowertext ((byte *) (*x), gethandlesize (x));
			
			unlockhandle (x);
			
			return (setheapvalue (x, stringvaluetype, v));
			}
		
		case filledstringfunc: {
			long ctfill;
			Handle x;
			
			if (!getstringvalue (hp1, 1, bs))
				return (false);
			
			flnextparamislast = true;
			
			if (!getpositivelongvalue (hp1, 2, &ctfill))
				return (false);
			
			ctfill = min (ctfill, longinfinity / stringlength (bs));
			
			if (!newhandle (ctfill * stringlength (bs), &x))
				return (false);
			
			fillstring (*x, ctfill, bs);
			
			return (setheapvalue (x, stringvaluetype, v));
			}
		
		case patternmatchfunc: {
			Handle hsearch, hpattern;
			long ixmatch;
			short ctconsumed = 2;
			short ctpositional = 2;
			tyvaluerecord vixstart;
			
			if (!getreadonlytextvalue (hp1, 1, &hpattern))
				return (false);
			
			if (!getreadonlytextvalue (hp1, 2, &hsearch))
				return (false);
			
			flnextparamislast = true;
			
			setlongvalue (1, &vixstart);
			
			/* 2004-12-27 smd: added optional ix parameter */
			if (!getoptionalparamvalue (hp1, &ctconsumed, &ctpositional, BIGSTRING ("\x02""ix"), &vixstart))
				return (false);
			
			if (vixstart.data.longvalue > 0) /*switch to 0-index*/
				--vixstart.data.longvalue;
			
			ixmatch = searchhandle (hsearch, hpattern, vixstart.data.longvalue, longinfinity);
			
			setlongvalue (ixmatch + 1, v);
			
			return (true);
			}
		
		case hexfunc: {
			long n;
			
			flnextparamislast = true;
			
			if (!getlongvalue (hp1, 1, &n))
				return (false);
			
			numbertohexstring (n, bs);
			
			return (setstringvalue (bs, v));
			}
		
		case timestringfunc: {
			unsigned long dt = timenow ();
			
			/*
			if (!langcheckparamcount (hp1, 0))
				return (false);
			*/
			
			if (langgetparamcount (hp1) > 0) {
				
				flnextparamislast = true;
				
				if (!getdatevalue (hp1, 1, &dt))
					return (false);
				}
			
			timetotimestring (dt, bs, true);
			
			return (setstringvalue (bs, v));
			}
		
		case datestringfunc: {
			unsigned long dt = timenow ();
			
			/*
			if (!langcheckparamcount (hp1, 0))
				return (false);
			*/
			
			if (langgetparamcount (hp1) > 0) {
				
				flnextparamislast = true;
				
				if (!getdatevalue (hp1, 1, &dt))
					return (false);
				}
			
			timetodatestring (dt, bs, true);
			
			return (setstringvalue (bs, v));
			}
		
		case addcommasfunc:
			flnextparamislast = true;
			
			if (!getstringvalue (hp1, 1, bs))
				return (false);
			
			stringaddcommas (bs);
			
			return (setstringvalue (bs, v));
		
		case replacefunc:
			return (stringreplaceverb (hp1, v, false));
		
		case replaceallfunc:
			return (stringreplaceverb (hp1, v, true));
		
		case lengthfunc: {
			Handle htext;
			
			flnextparamislast = true;
			
			if (!getreadonlytextvalue (hp1, 1, &htext)) 
				return (false);
			
			return (setlongvalue (gethandlesize (htext), v));
			}
		
		case isalphafunc: {
			char ch;
	
			flnextparamislast = true;
			
			if (!getcharvalue (hp1, 1, &ch))
				return (false);
			
			(*v).data.flvalue = isalpha ((unsigned char)ch) != 0;
			
			return (true);
			}
			
		case isnumericfunc: {
			char ch;
	
			flnextparamislast = true;
			
			if (!getcharvalue (hp1, 1, &ch))
				return (false);
			
			(*v).data.flvalue = isdigit ((unsigned char)ch) != 0;
			
			return (true);
			}
			
		case ispunctuationfunc: {
			char ch;
	
			flnextparamislast = true;
			
			if (!getcharvalue (hp1, 1, &ch))
				return (false);
			
			(*v).data.flvalue = ispunct ((unsigned char)ch) != 0;
			
			return (true);
			}
		
		case processmacrosfunc:
			return (processhtmlmacrosverb (hparam1, v));
		
		case urldecodefunc:
			return (urldecodeverb (hparam1, v));
		
		case urlencodefunc:
			return (urlencodeverb (hparam1, v));
		
		case parseargsfunc:
			return (parseargsverb (hparam1, v));
		
		case iso8859encodefunc:
			return (iso8859encodeverb (hparam1, v));

		case getgifheightwidthfunc:
			return (getgifheightwidthverb (hparam1, v));

		case getjpegheightwidthfunc:
			return (getjpegheightwidthverb (hparam1, v));

		case wrapfunc: {
			Handle htext;
			boolean result;
			
			flnextparamislast = true;
			
			if (!getexempttextvalue (hp1, 1, &htext)) 
				return (false);
			
			result = wordwraphandle (htext);
			
			result = result && setheapvalue (htext, stringvaluetype, v);

			return (result);
			}

		case davenetmassagerfunc: {
			Handle htext, hnew;
			boolean result;
			short indentlen, maxlinelen; 
			
			if (!getintvalue (hp1, 1, &indentlen))
				return (false);

			if (!getintvalue (hp1, 2, &maxlinelen))
				return (false);

			flnextparamislast = true;
			
			if (!getreadonlytextvalue (hp1, 3, &htext)) 
				return (false);
			
			result = daveNetMassager (indentlen, maxlinelen, htext, &hnew);
			
			result = result && setheapvalue (hnew, stringvaluetype, v);
			
			return (result);
			}
		
		case parseaddressfunc:
			return (parseaddressverb (hparam1, v));
		
		case dropnonalphasfunc:
			return (dropnonalphasverb (hparam1, v));
		
		case padwithzerosfunc:
			return (padwithzerosverb (hparam1, v));
		
		case ellipsizefunc: {
			Handle htext;
			long maxlen = 35;
			
			flnextparamislast = true;
				
			if (!getpositivelongvalue (hparam1, 2, &maxlen))
				return (false);

			if (!getexempttextvalue (hparam1, 1, &htext))
				return (false);

			if (!ellipsizehandle (htext, maxlen)) {
			
				disposehandle (htext);
				
				return (false);
				}

			return (setheapvalue (htext, stringvaluetype, v));
			}

		case innercasefunc: {
			Handle x;
			
			flnextparamislast = true;
			
			if (!getexempttextvalue (hp1, 1, &x))
				return (false);
			
			innercasehandle (x);
						
			return (setheapvalue (x, stringvaluetype, v));
			}
		
		case urlsplitfunc: {
			Handle x;
			
			flnextparamislast = true;
			
			if (!getreadonlytextvalue (hp1, 1, &x))
				return (false);
						
			return (urlsplit (x, v));
			}

		case hashmd5func: {
			Handle x;
			MD5_CTX hashcontext; /* MD5 context. */
			unsigned char checksum[16];
			short ctconsumed = 1;
			short ctpositional = 1;
			tyvaluerecord vtranslate;
		
			if (!getreadonlytextvalue (hp1, 1, &x))
				return (false);

			setbooleanvalue (true, &vtranslate);

			flnextparamislast = true;

			if (!getoptionalparamvalue (hp1, &ctconsumed, &ctpositional, BIGSTRING ("\x0b""flTranslate"), &vtranslate)) 
				return (false);

/*			if (langgetparamcount (hparam1) > 1) {

				flnextparamislast = true;
		
				if (!getbooleanvalue (hparam1, 2, &flTranslate))
					return (false);
				}
*/
			lockhandle (x);
	
			MD5Init (&hashcontext);
			
			MD5Update (&hashcontext, (unsigned char *)(*x), gethandlesize (x));
			
			MD5Final (checksum, &hashcontext);
			
			unlockhandle (x);

			if (vtranslate.data.flvalue) { /*return a hex string*/
				bigstring lbs;
				unsigned char enc[] = "0123456789abcdef";
				long ix;
				Handle h;

				for(ix = 0; ix < 16; ix++) {
					setstringcharacter (lbs, 2*ix, enc[(int)((checksum[ix])/16)]);
					setstringcharacter (lbs, 2*ix+1, enc[(int)((checksum[ix])%16)]);
					}/*for*/

				setstringlength (lbs, 32);

				if (!newtexthandle (lbs, &h))
					return (false);

				return (setheapvalue (h, stringvaluetype, v));
				}

			else { /*return a binary of unknown type*/
				Handle h;

				if (!newfilledhandle (checksum, 16, &h))
					return (false);

				return (setbinaryvalue (h, typeunknown, v));
				}
			
			return (true);
			}
			
		case latintomacfunc: { /*7.0b28 PBS: convert from Latin to Mac character sets*/
		
			Handle h;
			
			flnextparamislast = true;
			
			if (!getexempttextvalue (hp1, 1, &h))
				return (false);
				
			latintomac (h);

			return (setheapvalue (h, stringvaluetype, v));
			}
		
		case mactolatinfunc: { /*7.0b35 PBS: convert from Mac to Latin character sets*/
		
			Handle h;
			
			flnextparamislast = true;
			
			if (!getexempttextvalue (hp1, 1, &h))
				return (false);
				
			mactolatin (h);

			return (setheapvalue (h, stringvaluetype, v));
			}

		case utf16toansifunc: { /*7.0b41: convert from UTF-16 to ANSI*/

			Handle h, hresult;

			flnextparamislast = true;

			if (!getexempttextvalue (hp1, 1, &h))
				return (false);

			newemptyhandle (&hresult);

			if (!utf16toansi (h, hresult))
				return (false);

			disposehandle(h); // kw 2005-07-23 --- memleak

			return (setheapvalue (hresult, stringvaluetype, v));
			}

		case utf8toansifunc: { /*7.0b41: convert from UTF-8 to ANSI*/

			Handle h, hresult;

			flnextparamislast = true;

			if (!getexempttextvalue (hp1, 1, &h))
				return (false);

			newemptyhandle (&hresult);

			if (!utf8toansi (h, hresult))
				return (false);

			disposehandle(h); // kw 2005-07-23 --- memleak

			return (setheapvalue (hresult, stringvaluetype, v));
			}

		case ansitoutf8func: { /*7.0b41: convert from ANSI to UTF-8*/

			Handle h, hresult;

			flnextparamislast = true;

			if (!getexempttextvalue (hp1, 1, &h))
				return (false);

			newemptyhandle (&hresult);

			if (!ansitoutf8 (h, hresult))
				return (false);

			disposehandle(h); // kw 2005-07-23 --- memleak

			return (setheapvalue (hresult, stringvaluetype, v));
			}

		case ansitoutf16func: { /*7.0b41: convert from ANSI to UTF-16*/

			Handle h, hresult;

			flnextparamislast = true;

			if (!getexempttextvalue (hp1, 1, &h))
				return (false);

			newemptyhandle (&hresult);

			if (!ansitoutf16 (h, hresult))
				return (false);

			disposehandle (h); // kw 2005-07-23 --- memleak

			return (setheapvalue (hresult, stringvaluetype, v));
			}

		case multiplereplaceallfunc: { /*7.0b58: multiple string.replaceAlls*/
			
			return (stringmultiplereplaceallverb (hp1, v));
			}
			
		case macromantoutf8func: { /* 2006-02-25 creedon: convert mac roman character set to utf-8 */

			Handle h, hresult;

			flnextparamislast = true;

			if (!getreadonlytextvalue (hp1, 1, &h))
				goto error;

			newemptyhandle (&hresult);

			if (!macromantoutf8 (h, hresult))
				goto error;

			return (setheapvalue (hresult, stringvaluetype, v));
			}

		case utf8tomacromanfunc: { /* 2006-02-25 creedon: convert utf-8 character set to mac roman */

			Handle h, hresult;

			flnextparamislast = true;

			if (!getreadonlytextvalue (hp1, 1, &h))
				goto error;

			newemptyhandle (&hresult);

			if (!utf8tomacroman (h, hresult))
				goto error;

			return (setheapvalue (hresult, stringvaluetype, v));
			}
		
		case convertcharsetfunc:  /* 2006-04-13 smd: convert any character set to any other character set, if the os supports it */
		{
			Handle h, hresult;
			bigstring charsetIn, charsetOut;
			
			/*
				the "internet names" of character sets are 40 ASCII chars or less,
				so this is a safe way to get the params
			*/
			if ( !getstringvalue( hp1, 1, charsetIn ) )
				goto error;
			
			if ( !getstringvalue( hp1, 2, charsetOut ) )
				goto error;
			
			flnextparamislast = true;
			
			if ( !getreadonlytextvalue( hp1, 3, &h ) )
				goto error;
			
			newemptyhandle( &hresult );
			
			if ( !convertCharset( h, hresult, charsetIn, charsetOut ) )
				goto error;
			
			return ( setheapvalue( hresult, stringvaluetype, v ) );
		}
		
		case ischarsetavailablefunc:  /* 2006-04-23 smd: determine if a character set is available, based on an internet-friendly (IANA) name */
		{
			return isCharsetAvailableVerb( hparam1, v );
		}
		
		default:
			errornum = notimplementederror;
			
			goto error;
		} /*switch*/
	
	error:
	
	if (errornum != 0) /*get error string*/
		getstringlist (stringerrorlist, errornum, bserror);
	
	return (false);
	} /*stringfunctionvalue*/


boolean stringinitverbs (void) {
	
	/*
	if you just changed or added some definitions in stringinitbuiltins, call
	stringinstallbuiltins here.  rebuild, run the program, come back and change
	it to stringloadbuiltins, rebuild and go on...
	
	12/18/90 dmb: no longer save hash tables in program file, so we just 
	initialize the builtins directly.
	
	2.1b5 dmb: verb initialization is now resource-based
	*/
	
	return (loadfunctionprocessor (idstringverbs, &stringfunctionvalue));
	} /*stringinitverbs*/

