
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

#ifndef stringsinclude
#define stringsinclude


#ifndef memoryinclude
	#include "memory.h"
#endif

#ifdef PASCALSTRINGVERSION
	#define nullterminate(s) do {s [stringlength(s) + 1] = '\0';} while (0)
#else
	#define nullterminate(s) ((void *) 0)
#endif

/*global empty string*/

extern byte zerostring [];

extern unsigned char lowercasetable[256];


/*Latin to Mac conversion table: 7.0b28 PBS*/

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


/*prototypes*/

extern short getstringlength (bigstring);

/*extern unsigned char getlower (unsigned char c);*/
#define getlower(c) (lowercasetable[(unsigned char)c])

extern boolean equaltextidentifiers (byte * string1, byte * string2, short len);

extern boolean equalstrings (const bigstring, const bigstring);

extern boolean equalidentifiers (const bigstring, const bigstring);

extern short comparestrings (bigstring, bigstring);

extern boolean stringlessthan (bigstring, bigstring);

extern boolean pushstring (bigstring, bigstring);

extern boolean deletestring (bigstring, short, short);

extern boolean deletefirstchar (bigstring);

extern short popleadingchars (bigstring, byte);

extern short poptrailingchars (bigstring, byte);

extern boolean pushspace (bigstring);

extern boolean pushlong (long, bigstring);

extern boolean pushint (short, bigstring);

extern boolean pushboolean (boolean, bigstring);

extern boolean insertchar (byte, bigstring);

extern boolean pushchar (byte, bigstring);

extern pushstringresource (short, short, bigstring);

extern boolean insertstring (bigstring, bigstring);

extern void midstring (bigstring, short, short, bigstring);

extern boolean textfindreplace (Handle, Handle, Handle, boolean, boolean);

extern boolean stringfindreplace (bigstring, bigstring, Handle, boolean, boolean);

extern boolean dropnonalphas (bigstring);

extern boolean streamdropnonalphas (handlestream *);

extern boolean scanstring (byte, bigstring, short *);

extern boolean stringfindchar (byte, bigstring);

extern boolean stringreplaceall (char, char, bigstring);

extern boolean textlastword (ptrbyte, long, byte, bigstring);

extern boolean textfirstword (ptrbyte, long, byte, bigstring);

extern boolean textnthword (ptrbyte, long, long, byte, boolean, long *, long *);

extern long textcountwords (ptrbyte, long, byte, boolean);

extern boolean lastword (bigstring, byte, bigstring);

extern void poplastword (bigstring, byte);

extern boolean firstword (bigstring, byte, bigstring);

extern boolean nthword (bigstring, short, byte, bigstring);

extern boolean nthfield (bigstring, short, byte, bigstring);

extern short countwords (bigstring, byte);

extern boolean textcommentdelete (Handle);

extern long langcommentdelete (byte, byte *, long);

extern void commentdelete (byte, bigstring);

extern boolean whitespacechar (byte);

extern boolean poptrailingwhitespace (bigstring);

extern boolean firstsentence (bigstring);

extern void uppertext (ptrbyte, long);

extern void lowertext (ptrbyte, long);

extern void allupper (bigstring);

extern void alllower (bigstring);

extern boolean capitalizefirstchar (bigstring);

extern boolean isallnumeric (bigstring);

extern void filledstring (byte, short, bigstring);

extern void padwithzeros (bigstring, short);

extern void copystring (const bigstring, bigstring);

extern void copyptocstring (const bigstring, char *);

extern void copyctopstring (const char *, bigstring);

#ifdef WIN95VERSION
extern void copyrezstring (const bigstring, bigstring);
#endif

#ifdef WIN95VERSION
	#ifdef __MWERKS__
		extern ULONG wcslen(const unsigned short*);	// JES 12/04/2002: Make strings.c compile in CW8
	#endif											// AR 10/17/2004: ...but don't break the MS VC6 build
#endif

#ifdef MACVERSION
#define copyrezstring(a,b) copystring(a,b)
#endif

extern void copyheapstring (hdlstring, bigstring);

extern boolean pushheapstring (hdlstring, bigstring);

extern void timedatestring (long, bigstring);

extern void ellipsize (bigstring, short);

extern void parsedialogstring (const bigstring, ptrstring, ptrstring, ptrstring, ptrstring, bigstring);

extern boolean parsedialoghandle (Handle hsource, Handle h0, Handle h1, Handle h2, Handle h3);

extern boolean setparseparams (bigstring, bigstring, bigstring, bigstring);

extern void parseparamstring (bigstring);

extern void getstringresource (short, bigstring);

extern void parsenumberstring (short, short, long, bigstring);

extern void convertpstring (bigstring);

extern void convertcstring (bigstring);

extern long textpatternmatch (byte *ptext, long lentext, bigstring bsfind, boolean flunicase);

extern short patternmatch (bigstring, bigstring);

extern boolean addstrings (bigstring, bigstring, bigstring);

extern boolean subtractstrings (bigstring, bigstring, bigstring);

extern void ostypetostring (OSType, bigstring);

extern boolean stringtoostype (bigstring, OSType *);

extern boolean hexstringtonumber (bigstring, long *);

extern void bytestohexstring (ptrvoid, long, bigstring);

extern boolean bytestohex (Handle, Handle *);

extern boolean hextobytes (Handle, Handle *);

extern void numbertohexstring (long, bigstring);

extern void kstring (long, bigstring);

extern void dirtostring (tydirection, bigstring);

extern boolean stringtodir (bigstring, tydirection *);

extern boolean midinsertstring (bigstring, bigstring, short);

extern boolean replacestring (bigstring, short, short, bigstring);

extern boolean stringaddcommas (bigstring);

extern boolean stringdeletechars (bigstring, char);

extern boolean assurelastchariscolon (bigstring);

extern boolean utf16toansi (Handle, Handle); /*7.0b42 PBS*/

extern boolean utf8toansi (Handle, Handle); /*7.0b42 PBS*/

extern boolean ansitoutf8 (Handle, Handle); /*7.0b42 PBS*/

extern boolean ansitoutf16 (Handle, Handle); /*7.0b42 PBS*/

extern boolean pullstringsuffix (bigstring, bigstring, unsigned char); /*7.0.2b1 Radio PBS*/


extern void initstrings (void);


extern boolean stringinitverbs (void); /*stringverbs.c*/

extern long countleadingchars (Handle, byte);

extern void handlepopleadingchars (Handle htext, byte ch);

extern void handlepoptrailingchars (Handle htext, byte ch);

extern void latintomac (Handle h); /*7.0b28 PBS*/

extern void mactolatin (Handle h); /*7.0b41 PBS*/
#endif


