
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
#include "cursor.h"
#include "strings.h"
#include "search.h"

#include "langregexp.h"


tysearchparameters searchparams;


boolean isword (byte *ptext, long lentext, long pos, long len) {
	
	/*
	return true of the charater sequence indicated by the offset and 
	length (pos, len) in the text stream ptext begins and ends at word 
	boundaries.
	*/
	
	
		OffsetTable offsets;
		//Code change by Timothy Paustian Sunday, June 25, 2000 12:50:40 PM
		//updated to System 7.0 version
		FindWordBreaks ((Ptr) ptext, lentext, pos, true, nil, offsets, smCurrentScript);
		
		if (offsets [0].offFirst != pos)
			return (false);
		
		FindWordBreaks ((Ptr) ptext, lentext, pos + len, false, nil, offsets, smCurrentScript);
		
		if (offsets [0].offSecond != pos + len)
			return (false);
	


	return (true);
	} /*isword*/


boolean textsearch (byte *ptext, long lentext, long *offset, long *lenmatch) {

	/*
	Note that regexptextsearch might cause relocation of memory,
	i.e. if ptext points to the contents of handle, the handle must be locked.
	*/
	
	long ixstart = *offset;
	long ixmatch;

	if (searchparams.flregexp)
		return (regexptextsearch (ptext, lentext, offset, lenmatch));
	
	while (true) {
		
		ixmatch = textpatternmatch (ptext + ixstart, lentext - ixstart, searchparams.bsfind, searchparams.flunicase);
		
		if (ixmatch < 0)
			return (false);
		
		ixmatch += ixstart;
		
		if (!searchparams.flwholewords || isword (ptext, lentext, ixmatch, stringlength (searchparams.bsfind))) {
			
			*offset = ixmatch;
			
			*lenmatch = stringlength (searchparams.bsfind);
			
			return (true);
			}
		
		ixstart = ixmatch + 1; /*continue after beginning of false match*/
		}
	} /*textsearch*/


boolean handlesearch (Handle h, long *offset, long *lenmatch) {

	boolean fl;

	lockhandle (h);
	
	fl = textsearch (BIGSTRING (*h), gethandlesize (h), offset, lenmatch);
	
	unlockhandle (h);
	
	return (fl);
	} /*handlesearch*/


boolean stringsearch (bigstring bs, short *ix, short *length) {
	
	long offset = *ix;
	long len;
	
	if (!textsearch (stringbaseaddress (bs), stringlength (bs), &offset, &len))
		return (false);
	
	*ix = offset;
	*length = len;
	
	return (true);
	} /*stringsearch*/

/*
void getsearchstring (bigstring bs) {
	
	copystring (searchparams.bsorigfind, bs);
	
	if (searchparams.flunicase)
		alllower (bs);
	}*/ /*getsearchstring*/


void startnewsearch (boolean flzoomfound, boolean flreplaceall) {
	
	searchparams.ctreplaced = 0;
	
	searchparams.flzoomfound = flzoomfound;
	
	searchparams.flreplaceall = flreplaceall;
	} /*startnewsearch*/


boolean startingtosearch (long refcon) {
	
	/*
	9/12/91: new routine, key to maintaining search scope...
	
	a search command has been given, and the caller is about to start the 
	search in its context (its window), indentified (uniquely) by refcon.
	
	we determine whether or not this search command should establish a new 
	context; if so, refcon becomes the searchrefcon and we return true.  
	currently, no callers need to know if a new context was established, and 
	the return value is ignored.
	
	as the search proceeds, we'll use the searchrefcon to determine if 
	whether searches should continue from a given context, and whether 
	they should wrap.
	
	12/17/91 dmb: clear searchparams.flwindowzoomed flag every time -- see 
	tablefind.c.
	*/
	
	initbeachball (right);
	
	searchparams.flwindowzoomed = false; /*clear flag every time*/
	
	if (!searchparams.flfirsttime)
		return (false);
	
	searchparams.searchrefcon = refcon;
	
	return (true);
	} /*startingtosearch*/


boolean searchshouldcontinue (long refcon) {
	
	/*
	9/12/91 dmb: refcon identifies the context that has just been searched. 
	the caller is asking whether or not it should surface to an enclosing 
	context (i.e. a parent table) to continue the search.
	
	we return true if the search isn't limited to a single object, or if 
	the context that was just searched isn't the context where the search 
	began.
	*/
	
	if (!searchparams.floneobject)
		return (true);
	
	return (searchparams.searchrefcon != refcon);
	} /*searchshouldcontinue*/


boolean searchshouldwrap (long refcon) {
	
	/*
	9/12/91 dmb: refcon identifies the context that has just been searched. 
	the caller is asking whether or not it should wrap around to the top of 
	its context to continue the search.
	
	we return true if the search shouldn't continue in an enclosing context, 
	and wrapping option is enabled.
	*/
	
	if (searchshouldcontinue (refcon))
		return (false);
	
	return (searchparams.flwraparound);
	} /*searchshouldwrap*/


void endcurrentsearch (void) {
	
	/*
	9/12/91 dmb: the caller is indicating that the next time a search command 
	is issued, it should not be a continuation of the last search.
	*/
	
	searchparams.flfirsttime = true; /*set for next time*/
	
	searchparams.ctreplaced = 0;
	
	searchparams.searchrefcon = 0;
	} /*endcurrentsearch*/


boolean initsearch (void) {
	
	searchparams.flunicase = true;
	
	searchparams.flwraparound = true;
	
	searchparams.flwholewords = false;
	
	searchparams.flonelevel = false;
	
	searchparams.flonetype = false;
	
	searchparams.floneobject = false;
	
	searchparams.flclosebehind = false;
	
	searchparams.flzoomfound = true;
	
	searchparams.flreplaceall = false;
	
	searchparams.flfirsttime = true;
	
	searchparams.flregexp = false;
	
	searchparams.searchrefcon = 0;
	
	setemptystring (searchparams.bsorigfind);
	
	setemptystring (searchparams.bsorigreplace);
	
	setemptystring (searchparams.bsfind);
	
	setemptystring (searchparams.bsreplace);

	
	searchparams.hcompiledpattern = nil;
	
	searchparams.hovector = nil;

	
	return (true);
	} /*initsearch*/




