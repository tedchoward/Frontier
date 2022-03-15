
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

#include "bitmaps.h"
#include "icon.h"
#include "memory.h"
#include "quickdraw.h"
#include "sounds.h"
#include "frontier_strings.h"
#include "shell.rsrc.h"
#include "langexternal.h"
#include "tableinternal.h"


/*
static char *iconstrings [] = {
	
	"\pZoom",			/%0%/
	
	"\pOutline", 		/%1%/
	
	"\pWP-Text", 		/%2%/
	
	nil,
	
	"\pTable", 			/%4%/
	
	"\pScript", 		/%5%/
	
	"\pMenuBar", 		/%6%/
	
	"\pPicture" 		/%7%/
	};
*/


boolean tablecursoriszoomable (void) {
	
	hdlhashtable htable;
	bigstring bsname;
	tyvaluerecord val;
	hdlhashnode hnode;
	
	if (!tablegetcursorinfo (&htable, bsname, &val, &hnode))
		return (false);
	
	return ((val.valuetype == externalvaluetype) || (val.valuetype == novaluetype));
	} /*tablecursoriszoomable*/


boolean tablecheckzoombutton (void) {
	
	/*
	2.1b6 dmb: changed checking logic. we get called by callers who need the 
	flag to be right, instead of when it might change. return true of it did 
	change.
	*/
	
	register hdltableformats hf = tableformatsdata;
	register boolean flenabled;
	
	flenabled = tablecursoriszoomable ();
	
	if ((**hf).fliconenabled != flenabled) {
		
		(**hf).fliconenabled = flenabled;
		
		return (true);
		}
	
	return (false);
	
	/*
	register short ixicontitle;
	bigstring bs;
	tyvaluerecord val;
	
	ixicontitle = 0; /%default%/
	
	if (tablegetcursorinfo (bs, &val)) {
		
		if (val.valuetype == externalvaluetype)
			ixicontitle = langexternalgettype (val) + 1;
		}
	
	if ((**hi).ixicontitle != ixicontitle) {
		
		(**hi).ixicontitle = ixicontitle;
		
		(**hi).fliconenabled = flenabled;
		
		invalrect ((**hi).iconrect);
		}
	*/
	} /*tablecheckzoombutton*/


void tabledrawzoombutton (boolean flpressed) {
	
	hdltableformats hf = tableformatsdata;
	Rect r = (**hf).iconrect;
	bigstring bs;
	boolean flbitmap;
	
	tablecheckzoombutton (); /*check now instead of when it changes*/
	
	/*
	copystring (iconstrings [info.ixicontitle], bs);
	*/
	
	if (!isemptyrect (r)) {
			
		shellgetstring (zoombuttonstring, bs);
		
		flbitmap = openbitmap (r, tableformatswindow);
		
		eraserect (r);
		
		drawlabeledwindoidicon (r, bs, (**hf).flactive && (**hf).fliconenabled, flpressed);
		
		if (flbitmap)
			closebitmap (tableformatswindow);
		}
	} /*tabledrawzoombutton*/


boolean tablezoombuttonhit (void) {
	
	register hdltableformats hf = tableformatsdata;
	
	if (!trackicon ((**hf).iconrect, &tabledrawzoombutton))  /*user changed its mind*/
		return (true);
	
	return (tabledive ());
	} /*tablezoombuttonhit*/


boolean tablecursorisrunnable (void) {
	
	/*
	returns true if the value pointed to by the table cursor can be run.
	*/
	
	hdlhashtable  htable;
	bigstring bs;
	tyvaluerecord val;
	hdlhashnode hnode;
	
	if (!tablegetcursorinfo (&htable, bs, &val, &hnode)) 
		return (false);
		
	return (val.valuetype == stringvaluetype
			|| (val.valuetype == externalvaluetype
					&& langexternalgettype (val) == idscriptprocessor));
	} /*tablecursorisrunnable*/


boolean tableruncursor (void) {
	
	tyvaluerecord val;
	hdlhashtable htable;
	bigstring bs;
	bigstring bsscript, bsresult;
	Handle htext;
	hdlhashnode hnode;
	
	if (!tableexiteditmode ())
		return (false);
	
	if (!tablegetcursorinfo (&htable, bs, &val, &hnode)) {
		
		ouch ();
		
		return (false);
		}
	
	switch (val.valuetype) {
		
		case stringvaluetype: {
			if (!copyhandle (val.data.stringvalue, &htext))
				return (false);
			
			return (langrunhandle (htext, bsresult)); /*consumes htext*/
			}
		
		case externalvaluetype:
			if (langexternalgettype (val) == idscriptprocessor) {
			
				tablegetcursorpath (bsscript);
				
				pushstring ((ptrstring) "\x02()", bsscript);
				
				return (langrunstring (bsscript, bsresult));
				} /*if*/
				
			break;

		default:
			/* do nothing */
			break;

		} /*switch*/
	
	ouch ();
	
	return (false);
	} /*tableruncursor*/




