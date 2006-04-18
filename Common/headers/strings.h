
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

/*prototypes*/

extern short getstringlength (bigstring);

/*extern unsigned char getlower (unsigned char c);*/
#define getlower(c) (lowercasetable[(unsigned char)c])

extern boolean equaltextidentifiers (byte * string1, byte * string2, short len);

extern boolean equalstrings (const bigstring, const bigstring);

extern boolean equalidentifiers (const bigstring, const bigstring);

extern short comparestrings (bigstring, bigstring);

extern short compareidentifiers (bigstring, bigstring);

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

extern boolean pushstringresource (short, short, bigstring);

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

extern boolean macromantoutf8 (Handle, Handle); /* 2006-02-25 creedon */

extern boolean utf8tomacroman (Handle, Handle); /* 2006-02-25 creedon */

extern boolean converttextencoding (Handle, Handle, long, long);

#endif

