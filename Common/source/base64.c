
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

#ifdef MACVERSION
	#include <standard.h>
#endif
#ifdef WIN95VERSION
	#include "standard.h"
#endif

#include "memory.h"
#include "lang.h"
#include "base64.h"


static char encodingTable [64] = {

    'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P',
    
    'Q','R','S','T','U','V','W','X','Y','Z','a','b','c','d','e','f',
    
    'g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v',
    
    'w','x','y','z','0','1','2','3','4','5','6','7','8','9','+','/'
	};


static unsigned char gethandlechar (Handle h, unsigned long ix) {

	return ((*h) [ix]);
	} /*gethandlechar*/


static void sethandlechar (Handle h, unsigned long ix, unsigned char ch) {

	(*h) [ix] = ch;
	} /*sethandlechar*/


#define new_base64 1

#ifdef new_base64

boolean base64encodehandle (Handle htext, Handle h64, short linelength) { 

	/*
	encode the handle. some funny stuff about linelength -- it only makes
	sense to make it a multiple of 4. if it's not a multiple of 4, we make it
	so (by only checking it every 4 characters. 

	further, if it's 0, we don't add any line breaks at all.

	5.0a18 dmb: line breaks are /r, not /n
	
	6.1d4 AR: updated to grow h64 in one go instead of once for every
	three bytes of htext.
	*/

	unsigned long ixtext;
	unsigned long lentext;
	unsigned long len64;
	unsigned long pos64 = 0;
	long ctremaining;
	unsigned char inbuf [3], outbuf [4];
	short i;
	short charsonline = 0, ctcopy;
    
	ixtext = 0;

	lentext = gethandlesize (htext);
	
	len64 = (lentext / 3) * 4;
	
	if (lentext % 3 > 0)
		len64 += 4;
	
	if (linelength > 0) { /* add room for linebreaks */
		
		unsigned long ctquadruplets = linelength / 4;
		
		if (linelength % 4 > 0)
			++ctquadruplets;
		
		linelength = ctquadruplets * 4;
			
		len64 += (len64 / linelength);
		}

	if (!sethandlesize (h64, len64))
		return (false);

	while (true) {

		ctremaining = lentext - ixtext;
	
		if (ctremaining <= 0)
			break;
				
		for (i = 0; i < 3; i++) { 
		
			unsigned long ix = ixtext + i;
		
			if (ix < lentext)
				inbuf [i] = gethandlechar (htext, ix);
			else
				inbuf [i] = 0;
			} /*for*/
		
		outbuf [0] = (inbuf [0] & 0xFC) >> 2;
    
		outbuf [1] = ((inbuf [0] & 0x03) << 4) | ((inbuf [1] & 0xF0) >> 4);
    
		outbuf [2] = ((inbuf [1] & 0x0F) << 2) | ((inbuf [2] & 0xC0) >> 6);
    	
		outbuf [3] = inbuf [2] & 0x3F;
		
		ctcopy = 4;
	
		switch (ctremaining) {
		
			case 1: 
				ctcopy = 2; 
			
				break;
		
			case 2: 
				ctcopy = 3; 
			
				break;
			} /*switch*/

		for (i = 0; i < ctcopy; i++)
			sethandlechar (h64, pos64 + i, encodingTable [outbuf [i]]);
		
		for (i = ctcopy; i < 4; i++)
			sethandlechar (h64, pos64 + i, '=');
		
		ixtext += 3;
	
		charsonline += 4;
		
		pos64 += 4;
	
		if (linelength > 0) { /*DW 4/8/97 -- 0 means no line breaks*/
	
			if (charsonline >= linelength) {
			
				charsonline = 0;
				
				sethandlechar (h64, pos64++, '\r');
				}
			}
		} /*while*/

	assert (pos64 == len64); /*there's a bug in here if this assertion doesn't hold*/

	return (true);
	} /*encodehandle*/

#elif

boolean base64encodehandle (Handle htext, Handle h64, short linelength) { 

	/*
	encode the handle. some funny stuff about linelength -- it only makes
	sense to make it a multiple of 4. if it's not a multiple of 4, we make it
	so (by only checking it every 4 characters. 

	further, if it's 0, we don't add any line breaks at all.

	5.0a18 dmb: line breaks are /r, not /n
	*/

	unsigned long ixtext;
	unsigned long lentext;
	unsigned long origsize;
	long ctremaining;
	unsigned char inbuf [3], outbuf [4];
	short i;
	short charsonline = 0, ctcopy;
    
	ixtext = 0;

	lentext = gethandlesize (htext);

	while (true) {

		ctremaining = lentext - ixtext;
	
		if (ctremaining <= 0)
			break;
				
		for (i = 0; i < 3; i++) { 
		
			unsigned long ix = ixtext + i;
		
			if (ix < lentext)
				inbuf [i] = gethandlechar (htext, ix);
			else
				inbuf [i] = 0;
			} /*for*/
		
		outbuf [0] = (inbuf [0] & 0xFC) >> 2;
    
		outbuf [1] = ((inbuf [0] & 0x03) << 4) | ((inbuf [1] & 0xF0) >> 4);
    
		outbuf [2] = ((inbuf [1] & 0x0F) << 2) | ((inbuf [2] & 0xC0) >> 6);
    	
		outbuf [3] = inbuf [2] & 0x3F;
	
		origsize = gethandlesize (h64);
	
		if (!sethandlesize (h64, origsize + 4))
			return (false);
		
		ctcopy = 4;
	
		switch (ctremaining) {
		
			case 1: 
				ctcopy = 2; 
			
				break;
		
			case 2: 
				ctcopy = 3; 
			
				break;
			} /*switch*/

		for (i = 0; i < ctcopy; i++)
			sethandlechar (h64, origsize + i, encodingTable [outbuf [i]]);
		
		for (i = ctcopy; i < 4; i++)
			sethandlechar (h64, origsize + i, '=');
		
		ixtext += 3;
	
		charsonline += 4;
	
		if (linelength > 0) { /*DW 4/8/97 -- 0 means no line breaks*/
	
			if (charsonline >= linelength) {
			
				charsonline = 0;
			
				origsize = gethandlesize (h64);
			
				if (!sethandlesize (h64, origsize + 1))
					return (false);
				
				sethandlechar (h64, origsize, '\r');
				}
			}
		} /*while*/

	return (true);
	} /*encodehandle*/

#endif


#ifdef new_base64

boolean base64decodehandle (Handle h64, Handle htext) {

	unsigned long ixtext;
	unsigned long lentext;
	unsigned char ch;
	unsigned char inbuf [4], outbuf [3];
	short ixinbuf;
	boolean flendtext = false;
	handlestream s;
    
	ixtext = 0;

	lentext = gethandlesize (h64);

	ixinbuf = 0;
	
	openhandlestream (htext, &s);

	while (true) {
		short ctcharsinbuf = 3;

		if (ixtext >= lentext)
			break;
		
		ch = gethandlechar (h64, ixtext++);
	
		if ((ch >= 'A') && (ch <= 'Z'))
			ch = ch - 'A';
	
		else if ((ch >= 'a') && (ch <= 'z'))
			ch = ch - 'a' + 26;
		
		else if ((ch >= '0') && (ch <= '9'))
			ch = ch - '0' + 52;
	
		else if (ch == '+')
			ch = 62;
		
		else if (ch == '/')
			ch = 63;

		else if (ch == '=') /*no op -- can't ignore this one*/
			flendtext = true;
	
		else
			continue; 
		 
		if (flendtext) {
		
			if (ixinbuf == 0)
				break;
			
			if ((ixinbuf == 1) || (ixinbuf == 2))
				ctcharsinbuf = 1;
			else
				ctcharsinbuf = 2;
		
			ixinbuf = 3;
			}
	
		inbuf [ixinbuf++] = ch;
	
		if (ixinbuf == 4) {
		
			ixinbuf = 0;
		
			outbuf [0] = (inbuf [0] << 2) | ((inbuf [1] & 0x30) >> 4);
		
			outbuf [1] = ((inbuf [1] & 0x0F) << 4) | ((inbuf [2] & 0x3C) >> 2);
		
			outbuf [2] = ((inbuf [2] & 0x03) << 6) | (inbuf [3] & 0x3F);

			if (!writehandlestream (&s, outbuf, ctcharsinbuf))
				return (false);
			}
	
		if (flendtext)
			break;
		} /*while*/
	
	closehandlestream (&s);
	
	return (true);
	} /*decodehandle*/

#elif

boolean base64decodehandle (Handle h64, Handle htext) {

	unsigned long ixtext;
	unsigned long lentext;
	unsigned long origsize;
	unsigned char ch;
	unsigned char inbuf [4], outbuf [3];
	short i, ixinbuf;
	boolean flignore;
	boolean flendtext = false;
    
	ixtext = 0;

	lentext = gethandlesize (h64);

	ixinbuf = 0;

	while (true) {
	
		if (ixtext >= lentext)
			break;
		
		ch = gethandlechar (h64, ixtext++);
	
		flignore = false;
	
		if ((ch >= 'A') && (ch <= 'Z'))
			ch = ch - 'A';
	
		else if ((ch >= 'a') && (ch <= 'z'))
			ch = ch - 'a' + 26;
		
		else if ((ch >= '0') && (ch <= '9'))
			ch = ch - '0' + 52;
	
		else if (ch == '+')
			ch = 62;
		
		else if (ch == '=') /*no op -- can't ignore this one*/
			flendtext = true;
		
		else if (ch == '/')
			ch = 63;
	
		else
			flignore = true; 
	
		if (!flignore) {
	
			short ctcharsinbuf = 3;
			boolean flbreak = false;
			 
			if (flendtext) {
			
				if (ixinbuf == 0)
					break;
				
				if ((ixinbuf == 1) || (ixinbuf == 2))
					ctcharsinbuf = 1;
				else
					ctcharsinbuf = 2;
			
				ixinbuf = 3;
			
				flbreak = true;
				}
		
			inbuf [ixinbuf++] = ch;
		
			if (ixinbuf == 4) {
			
				ixinbuf = 0;
			
				outbuf [0] = (inbuf [0] << 2) | ((inbuf [1] & 0x30) >> 4);
			
				outbuf [1] = ((inbuf [1] & 0x0F) << 4) | ((inbuf [2] & 0x3C) >> 2);
			
				outbuf [2] = ((inbuf [2] & 0x03) << 6) | (inbuf [3] & 0x3F);
	
				origsize = gethandlesize (htext);
			
				if (!sethandlesize (htext, origsize + ctcharsinbuf))
					return (false);
	
				for (i = 0; i < ctcharsinbuf; i++) 
					sethandlechar (htext, origsize + i, outbuf [i]);
				}
		
			if (flbreak)
				break;
			}
		} /*while*/

	return (true);
	} /*decodehandle*/

#endif


boolean base64encodeverb (hdltreenode hparam1, tyvaluerecord *vreturned) {

	Handle h64, htext;
	short linelength;

	if (!gettextvalue (hparam1, 1, &htext))
		return (false);
	
	flnextparamislast = true;
	
	if (!getintvalue (hparam1, 2, &linelength))
		return (false);

	if (!newemptyhandle (&h64))
		return (false);
	
	if (!base64encodehandle (htext, h64, linelength)) {

		disposehandle (h64);

		return (false);
		}
	
	return (setheapvalue (h64, stringvaluetype, vreturned));
	} /*base64encodeverb*/


boolean base64decodeverb (hdltreenode hparam1, tyvaluerecord *vreturned) {
	
	/*
	5.0.1 dmb: return a string, not an unknown binary type
	*/

	Handle h64, htext;
	
	flnextparamislast = true;
	
	if (!gettextvalue (hparam1, 1, &h64))
		return (false);
	
	if (!newemptyhandle (&htext))
		return (false);
	
	if (!base64decodehandle (h64, htext)) {
		
		disposehandle (htext);
		
		return (false);
		}
	
	return (setheapvalue (htext, stringvaluetype, vreturned));
	} /*base64decodeverb*/


