
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

#include <standard.h>
#include "memory.h"
#include "strings.h"
#include "quickdraw.h"
#include "shell.h"
#include "shell.rsrc.h"
#include "shellundo.h"
#include "lang.h"
#include "wpengine.h"
#include "wpinternal.h"



typedef struct tywpvariable {
	
	unsigned char bsname [1]; /*variable name*/
	
	unsigned char bsvalue [1]; /*expanded text*/
	} tywpvariable, *ptrwpvariable, **hdlwpvariable;


typedef struct typackedvarheader {
	
	short version; /*in case format changes*/
	
	short ctvariables; /*not strictly necessary, but might aid unpacking*/
	
	long unused; /*space for growth*/
	} typackedvarheader;


typedef hdlwpvariable tyvariablelist [1], *ptrvariablelist, **hdlvariablelist;


static UserStyles variableprocs; /*must be statically allocated*/


#ifdef flvariables


boolean wpvariablerecalc (void) {
	
	/*
	force a total recalc & redisplay
	*/
	
	wptotalrecalc ();
	} /*wpvariablerecalc*/


static void mungedeleterecord (Handle harray, long ixrecord, long recordsize) {
	
	/*
	remove the indicated record from the heap-allocated array, sliding all 
	subsequent records left to fill the gap, and shortening the handle 
	accordingly.
	*/
	
	Munger (harray, ixrecord * recordsize, nil, recordsize, nil, 0L);
	} /*mungedeleterecord*/


static boolean wpfindvariable (bigstring bsname, short *ixlist) {
	
	/*
	search the current variable list for the indicated name return its 
	index in ixlist.   return true if the variable is found, else false.
	*/
	
	register hdlvariablelist hlist;
	register short ix;
	register short ctvars;
	
	if (!wpsetglobals ())
		return (false);
	
	hlist = (hdlvariablelist) (**wpdata).hvariablelist;
	
	ctvars = gethandlesize ((Handle) hlist) / sizeof (hdlwpvariable); /*nil list OK*/
	
	for (ix = 0; ix < ctvars; ++ix)
		
		if (equalstrings ((ptrstring) *(*hlist) [ix], bsname)) {
			
			*ixlist = ix;
			
			return (true);
			}
	
	return (false);
	} /*wpfindvariable*/


boolean wpdeletevariable (bigstring bsname) {
	
	/*
	dispose the hdlwpvariable with the indicated name.  delete its entry 
	in the variable list.
	*/
	
	register hdlvariablelist hlist;
	short ixlist;
	
	if (!wpfindvariable (bsname, &ixlist))
		return (false);
	
	hlist = (hdlvariablelist) (**wpdata).hvariablelist;
	
	disposehandle ((Handle) (*hlist) [ixlist]);
	
	mungedeleterecord ((Handle) hlist, (long) ixlist, longsizeof (hdlwprecord));
	
	return (true);
	} /*wpdeletevariable*/


void wpdisposevariablelist (Handle hvariablelist) {
	
	/*
	1/23/91 dmb: fixed pointer derefence bug -- heap trasher
	*/
	
	register hdlvariablelist hlist = (hdlvariablelist) hvariablelist;
	register short ix;
	register short ctvars;
	
	ctvars = gethandlesize ((Handle) hlist) / sizeof (hdlwpvariable); /*nil list OK*/
	
	for (ix = 0; ix < ctvars; ++ix)
		disposehandle ((Handle) (*hlist) [ix]); /*name, value pair*/
	
	disposehandle ((Handle) hlist);
	} /*wpdisposevariablelist*/


boolean wpgetnthvariable (short ixlist, bigstring bsname) {
	
	register hdlvariablelist hlist;
	register short ctvars;
	
	if (!wpsetglobals ())
		return (false);
	
	hlist = (hdlvariablelist) (**wpdata).hvariablelist;
	
	ctvars = gethandlesize ((Handle) hlist) / sizeof (hdlwpvariable); /*nil list OK*/
	
	if ((ixlist <= 0) || (ixlist > ctvars))
		return (false);
	
	copystring ((ptrstring) *(*hlist) [ixlist - 1], bsname);
	
	return (true);
	} /*wpgetnthvariable*/


#ifdef flstandalone

boolean wpgetvariablevalue (bigstring bsname, bigstring bsvalue) {
	
	register hdlvariablelist hlist;
	short ixlist;
	ptrstring pname, pvalue;
	boolean flagentsenabled, fl;
	
	if (!wpfindvariable (bsname, &ixlist))
		return (false);
	
	hlist = (hdlvariablelist) (**wpdata).hvariablelist;
	
	pname = (ptrstring) *(*hlist) [ixlist];
	
	pvalue = pname + stringsize (pname);
	
	copystring (pvalue, bsvalue);
	
	return (true);
	} /*wpgetvariablevalue*/

#else

boolean wpgetvariablevalue (bsname, bsvalue) bigstring bsname, bsvalue; {
	
	boolean fl;
	
	/*
	agentsdisable (true);
	*/
	
	fl = langrunstringnoerror (bsname, bsvalue);
	
	/*
	agentsdisable (false);
	*/
	
	return (true);
	} /*wpgetvariablevalue*/

#endif


boolean wpsetvariablevalue (bigstring bsname, bigstring bsvalue) {
	
	register hdlvariablelist hlist;
	register hdlwpvariable hvariable;
	short ixlist;
	register ptrstring pname;
	ptrstring pvalue;
	
	if (!wpfindvariable (bsname, &ixlist))
		return (false);
	
	hlist = (hdlvariablelist) (**wpdata).hvariablelist;
	
	hvariable = (*hlist) [ixlist];
	
	pname = (ptrstring) *hvariable;
	
	if (!sethandlesize ((Handle) hvariable, stringsize (pname) + stringsize (bsvalue)))
		return (false);
	
	pname = (ptrstring) *hvariable; /*may have moved when resized*/
	
	pvalue = pname + stringsize (pname);
	
	copystring (bsvalue, pvalue);
	
	if ((**wpdata).flexpandvariables && !(**wpdata).flinhibitdisplay)
		wpvariablerecalc ();
	} /*wpsetvariablevalue*/


boolean wpnewvariable (bigstring bsname, bigstring bsvalue) {
	
	/*
	add the indicated variable name to the current variable list, with 
	the given value.  the if name is already in the list, just update 
	its the variable's value
	*/
	
	register hdlwprecord hwp = wpdata;
	register Handle hvariablelist;
	short ixlist;
	Handle hvariable;
	
	if (wpfindvariable (bsname, &ixlist)) /*already exists*/
		return (wpsetvariablevalue (bsname, bsvalue));
	
	if (!wpsetglobals ())
		return (false);
	
	if (!newfilledhandle (bsname, stringsize (bsname), &hvariable))
		return (false);
	
	if (!enlargehandle (hvariable, stringsize (bsvalue), bsvalue)) {
		
		disposehandle (hvariable);
		
		return (false);
		}
	
	hvariablelist = (**hwp).hvariablelist;
	
	if (hvariablelist == nil) {
		
		hvariablelist = NewHandle (0);
		
		(**hwp).hvariablelist = hvariablelist;
		}
	
	if (!enlargehandle (hvariablelist, longsizeof (hvariable), &hvariable)) {
		
		disposehandle (hvariable);
		
		return (false);
		}
	
	return (true);
	} /*wpnewvariable*/


boolean wppackvariablelist (Handle *hpackedvarlist) {
	
	/*
	since variable names and values all have length bytes, packing 
	end-to-end is easy.
	*/
	
	register hdlvariablelist hlist;
	register short ctvars;
	register short ix;
	register Handle h;
	typackedvarheader header;
	long unused;
	
	hlist = (hdlvariablelist) (**wpdata).hvariablelist;
	
	ctvars = gethandlesize ((Handle) hlist) / sizeof (hdlwpvariable); /*nil list OK*/
	
	header.version = 0;
	
	header.ctvariables = ctvars;
	
	header.unused = 0;
	
	if (!newfilledhandle (&header, longsizeof (header), hpackedvarlist))
		return (false);
	
	h = *hpackedvarlist; /*move into register*/
	
	for (ix = 0; ix < ctvars; ++ix) {
		
		if (!pushhandle ((Handle) (*hlist) [ix], h)) { /*name, value pair*/
			
			disposehandle (h);
			
			return (false);
			}
		
		if (!enlargehandle (h, longsizeof (unused), &unused)) { /*future expansion*/
			
			disposehandle (h);
			
			return (false);
			}
		}
	
	return (true);
	} /*wppackvariablelist*/


boolean wpunpackvariablelist (Handle hpackedvarlist) {
	
	/*
	unpack hpackedvarlist, using wpnewvariable to set the hvariablelist 
	in wpdata.  
	
	scan the format runs in wpdata and set the procptrs for any variable 
	formats found.
	
	always dispose hpackedlist, even when an error occurs during unpacking.  we 
	don't need to clean up anything else on error, since everything allocated 
	is attached to wpdata.
	*/
	
	register short ctvars;
	register Handle h = hpackedvarlist;
	typackedvarheader header;
	long ix = 0;
	boolean fl = false;
	bigstring bsname, bsvalue;
	
	if (hpackedvarlist == nil) /*no variables exist -- easy to handle*/
		return (true);
	
	/*unpack variable list*/
	
	if (!loadfromhandle (h, &ix, longsizeof (header), &header))
		goto exit;
	
	ctvars = header.ctvariables;
	
	while (--ctvars >= 0) {
		
		if (!loadfromhandle (h, &ix, (long) stringsize (*h + ix), bsname))
			goto exit;
		
		if (!loadfromhandle (h, &ix, (long) stringsize (*h + ix), bsvalue))
			goto exit;
		
		if (!wpnewvariable (bsname, bsvalue))
			goto exit;
		
		ix += sizeof (long); /*skip over unused field*/
		}
	
	/*walk through user style runs and set up callbacks*/
	
	if (wpsetglobals ()) {
		
		long pos, count;
		FormatRec fmt;
		short ixfmt;
		
		pos = 0; /*start at beginning*/
		
		while ((ixfmt = WSFindUserStyle (wpbuffer, 0L, 0L, &pos, &count, &fmt)) >= 0) {
			
			fmt.UserProcs = &variableprocs;
			
			WSSetFormatRec (wpbuffer, &fmt, ixfmt);
			
			pos += count;
			}
		}
	
	fl = true; /*if we got here, we succeeded*/
	
	exit:
	
	disposehandle (hpackedvarlist);
	
	return (fl);
	} /*wpunpackvariablelist*/


boolean wpgetvariablestring (FormatRec *pfmt, bigstring bsdisplay) {
	
	hdlvariablelist hlist;
	short ixvariable;
	hdlstring hname;
	
	ixvariable = (short) (*pfmt).UserSpace;
	
	hlist = (hdlvariablelist) (**wpdata).hvariablelist;
	
	hname = (hdlstring) (*hlist) [ixvariable];
	
	if ((**wpdata).flexpandvariables) {
		
		if (!wpgetvariablevalue (*hname, bsdisplay))
			copystring ("\p???", bsdisplay);
		}
	else {
		
		copystring (*hname, bsdisplay);
		
		/*
		insertstring ("\p{", bsdisplay);
		
		pushchar ('}', bsdisplay);
		*/
		}
	
	return (true);
	} /*wpgetvariablestring*/


boolean wpremovevariablestyle (void) {
	
	/*
	the caller is about to insert some text, and wants to ensure that 
	the current cursor position doesn't inadvertantly have the userstyles 
	set up for a variable.  this would occur if the user clicked immediately 
	to the right of an existing variable, or just deleted a variable.
	
	we assume that wp's globals are set up, and any selection has already 
	been deleted.
	
	return true if a user space was actually removed
	*/
	
	long allspaces, anyspaces;
	
	if (WSGetUserSpace (wpbuffer, &allspaces, &anyspaces) == 0) /*no user space*/
		return (false);
	
	assert (allspaces == anyspaces); /*sanity check -- insert point should be uniform*/
	
	WSSetUserStyle (wpbuffer, nil, 0L, 0L);
	
	return (true);
	} /*wpremovevariablestyle*/


boolean wpinsertvariable (bigstring bsname) {
	
	/*
	replace the selection with the given variable name.  if the variable 
	doesn't exist, create it with a null value
	*/
	
	boolean flrecalc;
	byte chvariable;
	short ixvariable;
	
	if (!wpfindvariable (bsname, &ixvariable)) {
		
		if (!wpnewvariable (bsname, emptystring))
			return (false);
		
		if (!wpfindvariable (bsname, &ixvariable))
			return (false);
		}
	
	if (!wppreedit ())
		return (false);
	
	chvariable = '-'; /*a wordbreak character to represent the variable*/
	
	pushundoaction (undoformatstring);
	
	wpresettyping ();
	
	flrecalc = wpdelete (true); /*insert doesn't automagically delete selection*/
	
	wppushformatundo ();
	
	flrecalc = WSSetUserStyle (wpbuffer, &variableprocs, 0L, (long) ixvariable) || flrecalc;
	
	wppushinsertundo (false, true);
	
	flrecalc = WSInsert (wpbuffer, (Ptr) &chvariable, 1) || flrecalc;
	
	wppostedit (true);
	} /*wpinsertvariable*/


boolean wpgetevaluate (void) {
	
	return ((**wpdata).flexpandvariables);
	} /*wpgetevaluate*/


boolean wpsetevaluate (boolean flexpand) {
	
	/*
	2/18/91 dmb: always highlight variables when not expanded
	*/
	
	if (flexpand == (**wpdata).flexpandvariables)
		return (false);
	
	(**wpdata).flexpandvariables = flexpand;
	
	(**wpdata).flhilitevariables = !flexpand;
	
	wpvariablerecalc ();
	
	return (true);
	} /*wpsetevaluate*/


boolean wphilitevariables (boolean flhilite) {
	
	if (flhilite == (**wpdata).flhilitevariables)
		return (false);
	
	(**wpdata).flhilitevariables = flhilite;
	
	wpvariablerecalc ();
	
	return (true);
	} /*wphilitevariables*/


static pascal boolean wpmeasurevariable (WSHandle hwp, FormatRec *pfmt, byte *ptext, short ctchars, short slop, short *pwidths) {
	
	bigstring bsdisplay;
	register short ct = ctchars;
	register short width = 0;
	
	wpgetvariablestring (pfmt, bsdisplay);
	
	*pwidths = 0;
	
	while (--ctchars >= 0) {
		
		width += StringWidth (bsdisplay);
		
		*++pwidths = width;
		}
	
	return (TRUE);
	} /*wpmeasurevariable*/


static pascal boolean wpdrawvariable (WSHandle hwp, FormatRec *pfmt, byte *ptext, short ctchars, short slop, short ascent, short descent) {
	
	bigstring bsdisplay;
	Point ptstart;
	Rect r;
	
	wpgetvariablestring (pfmt, bsdisplay);
	
	getpenpoint (&topLeft (r)); /*in case we're highlighting below*/
	
	while (--ctchars >= 0)
		pendrawstring (bsdisplay);
	
	if ((**wpdata).flhilitevariables && !(**wpdata).flprinting) {
		
		getpenpoint (&botRight (r)); /*topleft was set before drawstring*/
		
		r.top -= ascent;
		
		r.bottom += descent;
		
		grayframerect (r);
		}
	
	return (TRUE);
	} /*wpdrawvariable*/

void wpvariableinit (void) {
	
	/*
	set up the userprocs for the variable format records
	*/
	
	variableprocs.hMeasureText = (ProcPtr) &wpmeasurevariable;
	
	variableprocs.hDrawText = (ProcPtr) &wpdrawvariable;
	
	variableprocs.hFmtDelete = (ProcPtr) nil; /*nothing to dispose*/
	
	variableprocs.hFontInfo = nil; /*nothing abnormal here*/
	} /*wpvariableinit*/


#else

boolean wpremovevariablestyle (void) {
	
	return (false);
	} /*wpremovevariablestyle*/


boolean wpunpackvariablelist (Handle hpackedvarlist) {
	
	disposehandle (hpackedvarlist); /*checks for nil*/
	
	return (true);
	} /*wpunpackvariablelist*/


boolean wppackvariablelist (Handle *hpackedvarlist) {
	
	return (newemptyhandle (hpackedvarlist));
	} /*wppackvariablelist*/


void wpdisposevariablelist (Handle hvariablelist) {
	
	disposehandle (hvariablelist);
	} /*wpdisposevariablelist*/

void wpvariableinit (void) {
	
	} /*wpvariableinit*/

#endif



