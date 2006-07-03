
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

#include "font.h"
#include "quickdraw.h"
#include "memory.h"
#include "strings.h"
#include "textedit.h"



void edittextbox (bigstring bs, Rect r, short fontnum, short fontsize, short fontstyle) {
	
	pushstyle (fontnum, fontsize, fontstyle);
	
	#ifdef MACVERSION
		TETextBox (stringbaseaddress (bs), (long) stringlength (bs), &r, teJustLeft);
	#endif

	#ifdef WIN95VERSION
		setWindowsFont();
	
		DrawText (getcurrentDC(), stringbaseaddress (bs), stringlength (bs), &r, 
				DT_LEFT | DT_END_ELLIPSIS | DT_WORDBREAK);
	
		clearWindowsFont();
	#endif
	
	popstyle ();
	} /*edittextbox*/


boolean edittwostringbox (bigstring bs1, bigstring bs2, Rect r, short fontnum, short fontsize) {
	
	/*
	a front-end for the textedit routine TextBox.  we take two strings, concatenate
	them in a buffer with two carriage returns between the strings, display the
	result, dispose the buffer.
	*/
	
	long len;
	Handle hbuffer;
	bigstring bs;
	
	if (!newtexthandle (bs1, &hbuffer))
		return (false);
	
	if (stringlength (bs2) > 0) {
		
		setstringlength (bs, 2);
		
		bs [1] = bs [2] = chreturn;
		
		if (!pushtexthandle (bs, hbuffer)) {
			
			disposehandle (hbuffer);
			
			return (false);
			}
			
		if (!pushtexthandle (bs2, hbuffer)){
			
			disposehandle (hbuffer);
			
			return (false);
			}
		}
		
	len = gethandlesize (hbuffer);
		
	pushstyle (fontnum, fontsize, 0);
	
	lockhandle (hbuffer);
	
	#ifdef MACVERSION
		TETextBox (*hbuffer, len, &r, teJustLeft);
	#endif

	#ifdef WIN95VERSION
		setWindowsFont();
		
		DrawText (getcurrentDC(), *hbuffer, len, &r, DT_LEFT | DT_END_ELLIPSIS | DT_WORDBREAK);
		
		clearWindowsFont();
	#endif
	
	
	unlockhandle (hbuffer);
	
	disposehandle (hbuffer);
	
	popstyle ();
	
	return (true);
	} /*edittwostringbox*/


