
/*	$Id$    */

/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include "appletmemory.h"
#include "appletstrings.h"
#include "appletwordwrap.h"



boolean uppercasehandle (Handle htext) {
	
	unsigned long ctchars;
	ptrchar p;
	short i;
	char ch;
	
	ctchars = gethandlesize (htext);
	
	lockhandle (htext);
	
	p = *htext;
	
	for (i = 1; i <= ctchars; i++) {
		
		ch = *p;
		
		if ((ch >= 'a') && (ch <= 'z'))
			ch -= 32;
		
		*p = ch;
		
		p++;
		} /*for*/
		
	unlockhandle (htext);
	
	return (true);
	} /*uppercasehandle*/
	

boolean wordwraphandle (Handle htext) {
	
	/*
	DW 12/3/94: rewrote the routine. it's now a two-pass thing.
	*/
	
	unsigned long handlesize = gethandlesize (htext);
	unsigned long ix = 0;
	char ch, chnext, chprev = ' ';
	boolean fllinestart = true;
		
	while (true) { /*pass 1 -- remove leading white space on each line*/
	
		if (ix >= handlesize)
			break;
		
		ch = (*htext) [ix];
		
		if (fllinestart && ((ch == ' ') || (ch == chtab))) {
			
			moveleft (&(*htext) [ix + 1], &(*htext) [ix], handlesize - ix);
		
			handlesize--;
		
			sethandlesize (htext, handlesize);
			
			ix--;
			
			goto pass1nextchar;
			}
			
		if (ch == chreturn) {
			
			fllinestart = true;
			
			goto pass1nextchar;
			}
			
		fllinestart = false; 
		
		pass1nextchar:
		
		ix++;
		} /*while*/
		
	ix = 0; /*start at the beginning of the handle for pass 2*/
	
	while (true) { /*pass 2 -- strip out the extraneous hard returns*/
		
		if (ix >= handlesize)
			break;
		
		ch = (*htext) [ix];
		
		if (ch != chreturn) 
			goto pass2nextchar;
			
		if (ix == (handlesize - 1)) { /*last char is a return*/
			
			/*sethandlesize (htext, handlesize - 1);*/ /*disabled -- the text might not be the whole message*/
			
			break; /*finished processing the text*/
			}
		
		chnext = (*htext) [ix + 1];
		
		if ((chnext == chreturn) || (chprev == chreturn)) /*leave double or triple returns alone*/
			goto pass2nextchar;
			
		if ((chnext != ' ') && (chprev != ' ')) {
			
			(*htext) [ix] = ' '; /*replace it with a space*/
			
			goto pass2nextchar;
			}
			
		moveleft (&(*htext) [ix + 1], &(*htext) [ix], handlesize - ix);
		
		handlesize--;
		
		sethandlesize (htext, handlesize);
		
		pass2nextchar:
		
		chprev = ch;
		
		ix++;
		} /*while*/
		
	return (true);
	} /*wordwraphandle*/
					
				
boolean daveNetMassager (short indentlen, short maxlinelen, Handle h, Handle *hmassaged) {
	
	unsigned long handlesize = gethandlesize (h);
	unsigned long ixhandle = 0;
	short linelen, wordlen;
	bigstring indent, dashes, word, cr, space;
	Handle hnew;
	short i;
	
	if (!getnewhandle (0, &hnew))
		return (false);
		
	filledstring (' ', indentlen, indent);
	
	setstringlength (dashes, 0);
	
	setstringlength (word, 0);
	
	setstringwithchar (chreturn, cr);
	
	setstringwithchar (' ', space);
	
	for (i = 1; i <= 16; i++)
		pushstring ("\p ---", dashes);
	
	if (!pushtexthandle (indent, hnew))
		goto error;
		
	linelen = indentlen;
	
	while (true) {
	
		if (ixhandle >= handlesize)
			break;
		
		if ((*h) [ixhandle] == chreturn) {
			
			if (equalstrings (word, "\p---")) {
				
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
	
	
	
	