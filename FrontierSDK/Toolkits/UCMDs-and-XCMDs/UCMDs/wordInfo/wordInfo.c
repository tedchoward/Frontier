
/*	$Id$    */

/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include <math.h>
#include <ucmd.h>


#define indexNthWordToken 	'ixnw'
#define getWordCountToken 	'gwct'
#define getNthWordToken		'gnwd'




static Boolean iswhite (char ch) {
	
	if ((ch >= 'a') && (ch <= 'z'))
		return (false);
		
	if ((ch >= 'A') && (ch <= 'Z'))
		return (false);
		
	if ((ch >= '0') && (ch <= '9'))
		return (false);
		
	if (ch == (char) 39) /*single quote*/
		return (false);
		
	return (true);
	} /*iswhite*/
	
	
static short indexNthWord (Handle htext, short n, short *wordcount) {
	
	short i;
	short ctchars;
	Boolean inwhite = true;
	Boolean thischarwhite;
	
	*wordcount = 0;
	
	ctchars = GetHandleSize (htext);
	
	if (ctchars == 0)
		return (0);
	
	for (i = 0; i < ctchars; i++) {
		
		thischarwhite = iswhite ((*htext) [i]);
		
		if (inwhite) {
			
			if (!thischarwhite) {
				
				(*wordcount)++;
				
				if (*wordcount >= n)
					return (i + 1); /*returned value is 1-based*/
					
				inwhite = false;
				}
			}
		else {
			if (thischarwhite)
				inwhite = true;
			}
		} /*indexNthWord*/
		
	return (0); /*aren't that many words*/
	} /*indexNthWord*/
	
	
static void getword (Handle htext, short ix, Str255 bs) {
	
	char ch;
	short ctchars;
	short len = 0;
	
	ix--; /*convert from 1-based to 0-based*/
	
	ctchars = GetHandleSize (htext);
	
	bs [0] = 0;
	
	while (true) {
		
		ch = (*htext) [ix++];
		
		if (iswhite (ch))
			return;
		
		bs [0] = (char) ++len;
		
		bs [len] = ch;
		
		if (ix >= ctchars)
			return;
		} /*while*/
	} /*getword*/
	
	
static void indexNthWordVerb (void) {
	
	Handle htext;
	short n;
	short wordcount;
	
	if (!IACgettextparam ((OSType) keyDirectObject, &htext))
		return;
	
	if (!IACgetshortparam ((OSType) 'n   ', &n))
		return;
		
	IACreturnshort (indexNthWord (htext, n, &wordcount));
	
	DisposHandle (htext);
	} /*indexNthWordVerb*/
	

static void getWordCountVerb (void) {
	
	Handle htext;
	short wordcount;
	
	if (!IACgettextparam ((OSType) keyDirectObject, &htext))
		return;
	
	indexNthWord (htext, 32767, &wordcount);
	
	IACreturnshort (wordcount);
	
	DisposHandle (htext);
	} /*getWordCountVerb*/
	

static void getNthWordVerb (void) {
	
	Handle htext;
	short n;
	short wordcount;
	Str255 word;
	
	if (!IACgettextparam ((OSType) keyDirectObject, &htext))
		return;
	
	if (!IACgetshortparam ((OSType) 'n   ', &n))
		return;

	getword (htext, indexNthWord (htext, n, &wordcount), word);
	
	IACreturnstring (word);
	
	DisposHandle (htext);
	} /*getNthWordVerb*/
	

void UCMDmain (void) {
		
	switch (IACgetverbtoken ()) {
	
		case indexNthWordToken:
			indexNthWordVerb (); break;
			
		case getWordCountToken:
			getWordCountVerb (); break;
			
		case getNthWordToken:
			getNthWordVerb (); break;
			
		default:
			IACnothandlederror (); break;
			
		} /*switch*/
	} /*UCMDmain*/
