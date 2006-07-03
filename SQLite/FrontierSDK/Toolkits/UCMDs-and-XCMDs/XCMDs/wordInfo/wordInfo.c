
/*	$Id$    */

/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include <HyperXCmd.h>




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
	
	
static void setShortValue (XCmdPtr paramPtr, Str255 name, short value) {
	
	Str255 s;
	Handle h;
	
	NumToString (value, s);
	
	h = PasToZero (paramPtr, s);
	
	SetGlobal (paramPtr, name, h);
	
	DisposHandle (h);
	} /*setShortValue*/
	

static void getShortParam (XCmdPtr paramPtr, short paramNum, short *x) {

	Str255 s;
	long num;
	
	ZeroToPas (paramPtr, *(paramPtr->params) [paramNum], s);
	
	StringToNum (s, &num);
	
	*x = (short) num;
	} /*getShortParam*/
	
	
pascal void main (XCmdPtr paramPtr) {
	
	Handle htext;
	short n;
	Str255 word;
	short wordcount;
	short wordindex;
	
	htext = paramPtr->params [0];
	
	getShortParam (paramPtr, 1, &n);
	
	wordindex = indexNthWord (htext, n, &wordcount);
	
	indexNthWord (htext, 32767, &wordcount); /*get the real value of wordcount*/
	
	getword (htext, wordindex, word);
	
	setShortValue (paramPtr, "\pwordCount", wordcount);
	
	setShortValue (paramPtr, "\pwordIndex", wordindex);
	
	paramPtr->returnValue = PasToZero (paramPtr, word);
	} /*main*/



