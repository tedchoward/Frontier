
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

#include "iowacore.h"
#include <appletfrontier.h>
#include "iowaparser.h"


#define translateids false /*set true to allow filtering of id's thru the translator script*/


static Handle groupvalue;


static boolean groupvaluevisit (hdlobject h) {

	Handle hvalue;
	
	if (!callspeaksforgroup (h)) /*keep visiting*/
		return (true);
		
	if (!(**h).objectflag) /*keep visiting*/
		return (true);
	
	getobjectvalue (h, &hvalue); 
	
	if (!newtexthandle ("\pÒ", &groupvalue))  /*dmb 1.0b21: use curly quotes*/
		goto error;
		
	if (!pushhandleonhandle (hvalue, groupvalue))
		goto error;
		
	if (!pushtexthandle ("\pÓ", groupvalue))
		goto error;

	return (false); /*stop visiting*/
	
	error:
	
	disposehandle (groupvalue);
	
	groupvalue = nil;
	
	return (false);
	} /*groupvaluevisit*/
	
	
static boolean getgroupvalue (hdlobject hobj, Handle *hvalue) {
	
	/*
	return the value of the first radio button in the group that's on.
	*/
	
	groupvalue = nil;
	
	visitobjects ((**hobj).childobjectlist, &groupvaluevisit);
	
	if (groupvalue == nil) 
		return (false);
	
	*hvalue = groupvalue;
	
	pushtemphandle (groupvalue);
	
	return (true);
	} /*getgroupvalue*/
	
	
static boolean getvalueforscript (hdlobject hself, bigstring bsname, Handle *hvalue) {
	
	hdlobject hobj;
	
	/*check it against SELF -- new in 1.0b15*/ {
		
		if (hself != nil) {
		
			bigstring bsupper;
			
			copystring (bsname, bsupper);
			
			allupper (bsupper);
			
			if (equalstrings (bsupper, "\pSELF")) {
				
				bigstring bs, bsobjectname;
				
				copystring ("\pÒ", bs); /*dmb 1.0b21: use curly quotes*/
				
				getobjectnamestring (hself, bsobjectname);
				
				pushstring (bsobjectname, bs);
				
				pushstring ("\pÓ", bs);
				
				if (!newtexthandle (bs, hvalue))
					return (false);
				
				pushtemphandle (*hvalue);
				
				return (true);
				}
			}
		}
	
	if (!lookupname (bsname, &hobj)) {
	
		if (!newtexthandle (bsname, hvalue))
			return (false);
			
		pushtemphandle (*hvalue);
		
		return (true);
		}
		
	switch ((**hobj).objecttype) {
		
		case grouptype:
			return (getgroupvalue (hobj, hvalue));
			
		default: 
			return (callgetvalue (hobj, hvalue));
		} /*switch*/
	} /*getvalueforscript*/
	
	
static boolean isfirstidchar (char ch) {
	
	/*
	return true if the character could be the start of an identifier.
	*/
	
	if ((ch >= 'a') && (ch <= 'z'))
		return (true);
		
	if ((ch >= 'A') && (ch <= 'Z'))
		return (true);
	
	if (ch == '_')
		return (true);
		
	return (false);
	} /*isfirstidchar*/
	
	
static boolean issubsequentidchar (char ch) {
	
	if (isfirstidchar (ch))
		return (true);
	
	if ((ch >= '0') && (ch <= '9'))
		return (true);
		
	if (ch == '.') /*DW 4/11/93*/
		return (true);
		
	return (false);
	} /*issubsequentidchar*/
	
	
boolean preparseScript (hdlobject hself, Handle script, Handle *returns) {
	
	/*
	preparsing means replacing all references to named card objects
	with the values of those objects.
	
	dmb 1.0b20 - fixed memory trashing bug when substituting a value
	that is shorter than the name it replaces
	*/
	
	Handle scriptcopy;
	long ixscript;
	long len;
	long ix;
	short lenname, lenvalue;
	bigstring bsname;
	long newlen;
	Handle hvalue;
	char ch;
	
	if (!copyhandle (script, &scriptcopy)) /*work with a copy*/
		return (false);
	
	ixscript = 0;
	
	len = GetHandleSize (scriptcopy);
	
	while (true) {
	
		if (ixscript >= len)
			break;
			
		ch = (*scriptcopy) [ixscript];
		
		if (ch == '"') {
			
			while (true) {
				
				ixscript++;
			
				if (ixscript >= len)
					break;
					
				if ((*scriptcopy) [ixscript] == '"') {
					
					ixscript++;
					
					break;
					}
				} /*while*/
				
			continue;
			}
		
		if (ch == '\'') {
			
			while (true) {
				
				ixscript++;
			
				if (ixscript >= len)
					break;
					
				if ((*scriptcopy) [ixscript] == '\'') {
					
					ixscript++;
					
					break;
					}
				} /*while*/
				
			continue;
			}
		
		if (!isfirstidchar (ch)) {
	
			ixscript++;
		
			continue;
			}
		
		ix = ixscript + 1;
		
		while (true) { 
			
			if (ix < len) {
			
				if (issubsequentidchar ((*scriptcopy) [ix])) {
			
					ix++;
				
					continue;
					}
				}
				
			lenname = ix - ixscript;
			
			setstringlength (bsname, lenname);
			
			moveleft (&(*scriptcopy) [ixscript], &bsname [1], lenname);
			
			if (!getvalueforscript (hself, bsname, &hvalue)) {
				
				ixscript = ix;
				
				break;
				}
			
			lenvalue = GetHandleSize (hvalue);
			
			newlen = ixscript + lenvalue + len - ix;
			
			if (newlen > len) { /*dmb 1.0b20*/
				
				if (!sethandlesize (scriptcopy, newlen)) {
				
					disposetemphandle (hvalue);
					
					disposehandle (scriptcopy);
					
					return (false);
					}
				}
			
			BlockMoveData (&(*scriptcopy) [ix], &(*scriptcopy) [ixscript + lenvalue], len - ix);
			
			BlockMoveData (*hvalue, &(*scriptcopy) [ixscript], lenvalue);
			
			/*
			SetHandleSize (scriptcopy, newlen);
			
			moveleft (&(*scriptcopy) [ix], &(*scriptcopy) [ixscript], len - ix);
			
			moveright (&(*scriptcopy) [ixscript], &(*scriptcopy) [ixscript + lenvalue], len - ix);
			
			moveleft (*hvalue, &(*scriptcopy) [ixscript], lenvalue);
			*/
			
			disposetemphandle (hvalue);
			
			len = newlen;
			
			ixscript += lenvalue;
			
			break;
			} /*while*/
		} /*while*/
	
	sethandlesize (scriptcopy, len); /*dmb 1.0b20 - shrink to fit now, if necessary*/
	
	*returns = scriptcopy;
	
	return (true);
	} /*preparseScript*/
	

