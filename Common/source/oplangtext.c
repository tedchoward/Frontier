
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
#include "lang.h"
#include "langinternal.h"
#include "op.h"
#include "opinternal.h"




static short langtextlastlevel; /*globals used for visit routines*/

//static Handle hlangtext;

static boolean flfirstlangtextline;

static short ctlinesincontinuation;

static boolean fllastwascomment;

static boolean flmakeitpretty;

static Handle *plastcomment;




static boolean backslashdelete (Handle bs) {
	
	register long len = gethandlesize (bs);
	
	if (len > 0 && (*bs) [len - 1] == '\\') {
		
		//setstringlength (bs, len - 1);
		popfromhandle (bs, 1, nil);
		
		return (true);
		}
	
	return (false);
	} /*backslashdelete*/


static boolean poptrailingwhitespacehandle (Handle bs) {
	
	/*
	return true if there were trailing "whitespace" characters to be popped.
	*/
	
	register long i, ct;
	
	ct = gethandlesize (bs);
	
	for (i = ct; i > 0; i--)
	
		if (!whitespacechar ((*bs) [i - 1])) { /*found a non-blank character*/
			
			sethandlesize (bs, i);
			
			return (i < ct);
			}
	
	sethandlesize (bs, 0);
	
	return (true); /*string is all blank*/
	} /*poptrailingwhitespacehandle*/


static boolean oplangtextvisit (hdlheadrecord hnode, handlestream *langtext) {
	
	/*
	push the text of the indicated node onto the langtext output handle.
	
	we flatten out the structure in the language understood by the parser.
	
	e.g. if we move in a level, we insert a left curly bracket.  out a level
	a right curly bracket.
	
	carriage returns are inserted into the text so that the parser knows
	which line generated the code, this makes  error reporting and 
	debugging possible.
	
	11/12/91 dmb: push '{' when actually entering deeper level rather than 
	when subheads are present, so single comment subhead is handled correctly.
	also, don't push a semicolon before an empty statement; otherwise, blank 
	lines in a locals block (and perhaps elsewhere) generate syntax errors.
	
	12/9/91 dmb: support backslash for continuation lines.  also, need to look 
	ahead for non-comment lines in order to place opening '{' at end of correct 
	line; otherwise, debugger can step onto a comment line
	
	12/13/91 dmb: added support for flmakeitpretty so that langtext can be 
	exported in a useful form
	
	5/6/93 dmb: test for "else" is now case-insensitive
	
	2.1b2 dmb: don't add returns until after continuation lines are complete

	5.0a16 dmb: don't require else to be on a line by itself
	
	6.0a13 dmb: rewrote to use handles, handlestreams
	*/
	
	register hdlheadrecord h = hnode;
	register short level;
	bigstring bs;
	Handle bshead;
	Handle bscomment = nil;
	boolean fltobecontinued = false;
	boolean flcomment;
	hdlheadrecord nomad;
	bigstring bsfirst;
	byte ch;
	long len1, len2;
	boolean fl = false;
	
	level = (**h).headlevel;
	
	flcomment = opnestedincomment (h);
	
	if (flcomment) { /*just generate a blank line for a comment line*/
		
		bshead = nil; //setemptystring (bshead);
		
		if (flmakeitpretty) {
			
			//opgetheadstring (h, bshead);
			if (!copyhandle ((**h).headstring, &bshead)) 
				goto exit;
			
			//insertchar (chcomment, bshead);
			ch = chcomment;
			
			if (!insertinhandle (bshead, 0, &ch, 1L))
				goto exit;
			
			//setemptystring (bscomment);
			}
		
		nomad = h;
		
		if (opnavigate (left, &nomad) && opnestedincomment (nomad))
			goto L2;
		
		if (level > langtextlastlevel) {
			
			nomad = h;
			
			while (true) {
				
				if (!opnavigate (down, &nomad)) /*no more siblings*/
					goto L2;
				
				if (!opnestedincomment (nomad))
					break;
				}
			}
		
		fltobecontinued = ctlinesincontinuation > 0; /*don't let comment interrupt continuation*/
		}
	else {
		
		//getheadstring (h, bshead);
		if (!copyhandle ((**h).headstring, &bshead)) 
			goto exit;
		
		fltobecontinued = backslashdelete (bshead); /*pop off continuation marker*/
		}
	
	len1 = gethandlesize (bshead); /*get size with comments*/
	
	if (len1 > 0) {
		
		len2 = langcommentdelete (chcomment, *bshead, len1);
		
		if (len2 >= 0) { //comment detected
		
			if (flmakeitpretty) { /*for exporting, we want to *add* comment character*/
				
				if (!loadhandleremains (len2, bshead, &bscomment))
					goto exit;
				
				if ((gethandlesize (bscomment) > 0) && (len2 > 0))
					insertinhandle (bscomment, 0, " ", 1L);
				}
			
			sethandlesize (bshead, len2);
			}
		}
	
	if (!fltobecontinued && bshead)
		poptrailingwhitespacehandle (bshead); /*pop blanks, returns, tabs, etc at end of string*/
	
	if (ctlinesincontinuation == 0) { /*not continuing a previous line*/
		
		if (level > langtextlastlevel) { /*beginning of new body*/
			
			setstringwithchar ('{', bs); /*need opening brace*/
			
			if (flmakeitpretty)
				insertchar (chspace, bs);
			}
		else {
			
			filledstring ('}', langtextlastlevel - level, bs); /*may need closing braces*/
			
			if ((level < langtextlastlevel) || !fllastwascomment) {
				
				if (bshead)
					textfirstword (*bshead, gethandlesize (bshead), chspace, bsfirst);
				else
					setemptystring (bsfirst);

				if (!equalidentifiers (bsfirst, STR_else)) /*never want a semicolon before else*/
					pushchar (';', bs);
				}
			}
		
		if (!writehandlestreamstring (langtext, bs))
			goto exit;
		
		langtextlastlevel = level;
		}
	
	fllastwascomment = flcomment || isemptyhandle (bshead);
	
	L2:
	
	if (flmakeitpretty) {
		
		if (!writehandlestreamhandle (langtext, *plastcomment))
			goto exit;
		
		disposehandle (*plastcomment);
		
		//copystring (bscomment, plastcomment);
		*plastcomment = bscomment;
		}
	
	if (!flfirstlangtextline && (ctlinesincontinuation == 0)) {
		
		setstringwithchar (chreturn, bs); /*return goes before the head string*/

	#ifdef xxxWIN95VERSION
		pushchar (chlinefeed, bs);
	#endif
		
		if (flmakeitpretty) {
			
			while (--level >= 0)
				pushchar (chtab, bs);
			}
		
		if (!writehandlestreamstring (langtext, bs))
			goto exit;
		}
	
	flfirstlangtextline = false;
	
	if (!writehandlestreamhandle (langtext, bshead))
		goto exit;
	
	if (fltobecontinued)
		++ctlinesincontinuation;
	else {
		
		if (ctlinesincontinuation > 0) {
			
			if (!flmakeitpretty) {
				
				filledstring (chreturn, ctlinesincontinuation, bs); /*account for all input lines*/
				
				if (!writehandlestreamstring (langtext, bs))
					goto exit;
				}
			
			ctlinesincontinuation = 0;
			}
		}
	
	fl = true; //natural exit
	
  exit:
  	disposehandle (bshead);
  	
	return (fl);
	} /*oplangtextvisit*/


boolean opgetlangtext (hdloutlinerecord houtline, boolean flpretty, Handle *htext) {
	
	/*
	7/29/91 dmb: use writeeditbuffer to flush typing, not unload/load
	
	2.1b8 dmb: call new langaddapplescriptsyntax for 'ascr' signature
	
	6.0a13 dmb: rewrote to use handles, handlestreams
	*/
	
	register hdloutlinerecord ho = houtline;
	bigstring bs;
	Handle bslastcomment;
	OSType signature = (**ho).outlinesignature;
	handlestream s;
	
	#if !flruntime
	
	if ((**ho).hbuffer != nil) { /*the user is editing text, ibeam cursor mode*/
		
		oppushoutline (ho);
		
		opwriteeditbuffer (); /*force buffer into outline structure*/
		
		oppopoutline ();
		}
	
	#endif
	
	setemptystring (bs);
	
//	if (!newgrowinghandle (0, htext))
//		return (false);
	
	*htext = nil;
	
	openhandlestream (nil, &s); //handle will be created on first write
	
	if (signature != 'LAND') {
		
		if (!opoutlinetotextstream (ho, true, &s))
			goto error;
		
		*htext = closehandlestream (&s);
		
		#ifdef MACVERSION

			if (signature == 'ascr')
				langaddapplescriptsyntax (*htext);
			
		#endif
		
		return (true);
		}
	
//	hlangtext = *htext; /*copy so visit routine can find it*/
	
	langtextlastlevel = 0;
	
	flfirstlangtextline = true;
	
	ctlinesincontinuation = 0;
	
	fllastwascomment = true;
	
	//setemptystring (bslastcomment);
	bslastcomment = nil;
	
	plastcomment = &bslastcomment; /*make available to visit routine*/
	
	flmakeitpretty = flpretty;
	
	if (!opsiblingvisiter ((**houtline).hsummit, false, &oplangtextvisit, &s))
		goto error;
	
	filledstring ('}', langtextlastlevel, bs); /*close all outstanding levels with }s*/
	
	if (!writehandlestreamstring (&s, bs))
		goto error;
	
	if (flmakeitpretty)
		if (!writehandlestreamhandle (&s, bslastcomment))
			goto error;
	
	disposehandle (bslastcomment);
	
	*htext = closehandlestream (&s);
	
	return (true);
	
	error: {
	
		disposehandlestream (&s);
		
		return (false);
		}
	} /*opgetlangtext*/



