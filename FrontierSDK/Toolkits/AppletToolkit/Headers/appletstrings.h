
/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#define stringsinclude


#ifndef appletdefsinclude

	#include "appletdefs.h"
	
#endif


boolean isPunctuationChar (char);

boolean isAlphaChar (char);

short comparestrings (void *, void *);

void copystring (void *, void *);

boolean pushstring (void *, void *);

boolean pushspace (void *);

void pushlong (long, void *);

void pushint (short, void *);

void pushtimeonstring (void *);

void allupper (void *);

void alllower (void *);

boolean stringlessthan (void *, void *);

void midstring (void *, short, short, void *);

boolean pushchar (byte, void *);

void pushostype (OSType, void *);

boolean scanstring (byte, void *, short *);

boolean deletestring (void *, short, short);

boolean firstword (void *, char, void *);

boolean lastword (void *, char, void *);

boolean equalstrings (void *, void *);

boolean unicaseequalstrings (void *, void *);

short patternmatch (void *, void *);

void filledstring (char, short, void *);

boolean insertstring (unsigned char *, unsigned char *);

boolean insertchar (char, void *);

boolean stringtonumber (unsigned char *, long *);

void getsizestring (unsigned long, void *);

void gettimestring (long, void *);

void commentdelete (byte, void *);

boolean poptrailingwhitespace (void *);
	
boolean replacestring (void *, short, short, void *);

boolean stringfindchar (byte, void *);

void parseparamstring (bigstring, bigstring, bigstring, bigstring, bigstring, bigstring);

void colortostring (RGBColor, bigstring);

void stringtocolor (bigstring, RGBColor *);

boolean stringendswith (bigstring, bigstring, boolean);
