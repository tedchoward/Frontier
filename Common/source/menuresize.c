
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

#include "quickdraw.h"
#include "popup.h"
#include "windowlayout.h"
#include "scrollbar.h"
#include "dialogs.h"
#include "shell.h"
#include "shellbuttons.h"
#include "op.h"
#include "opinternal.h"
#include "menueditor.h"
#include "menuinternal.h"



#define itemmargin 8

// 5.0a25 dmb: wire off Zoom button
#undef iconrectheight
#undef iconrectwidth
#define iconrectheight 0
#define iconrectwidth 0



static void mesetalloutlinerects (Rect r) {

	oppushoutline ((**menudata).menuoutline);
	
	insetrect (&r, 1, 1);
	
	opresize (r);
	
	#ifdef WIN95VERSION
		opupdatenow ();
	#endif

	oppopoutline ();
	} /*mesetalloutlinerects*/


boolean meresetwindowrects (hdlwindowinfo hw) {
	
	register hdlmenurecord hm = (hdlmenurecord) (**hw).hdata;
	short scrollbarwidth;
	short msgheight;
	Rect rcontent;
	Rect menurect;
	Rect r;
	
	if (outlinedata == nil)
		return (false);
	
	/*set up some values for all rect computations*/ {
	
		scrollbarwidth = getscrollbarwidth ();
		
		msgheight = popupheight; 
		}
	
	/*get contentrect and do info.growiconrect*/ {
		
		rcontent = (**hw).contentrect;
		
		shellcalcgrowiconrect (rcontent, hw);
		}
	
	/*inset contentrect uniformly*/ {
		
		insetrect (&rcontent, windowmargin, windowmargin);
		}
	
	/*do iconrect*/ {
	
		r = rcontent;
		
		r.bottom = r.top + iconrectheight;
		
		r.left = r.right - iconrectwidth;
		
	//	insetrect (&r, -4, 0); /*a little extra width for title*/
		
		if (hm != nil)
			(**hm).iconrect = r; 
		}
		
	/*do menurect*/ {
		
		menurect = rcontent;

		menurect.right -= iconrectwidth + windowmargin + scrollbarwidth;

		menurect.bottom -= msgheight + windowmargin;

		if (hm != nil)
			(**hm).menuoutlinerect = menurect;
		}
	
	/*do cmdkeypopuprect*/ {
		
		r = rcontent;
		
		r.top = r.bottom - msgheight;

		r.right = r.left + cmdkeypopupwidth;
		
		if (hm != nil)
			(**hm).cmdkeypopuprect = r; 
		}
	
	/*do messagerect*/ {
		
		r.left = r.right + popupbetweenwidth;
		
		r.right = menurect.right + scrollbarwidth;
		
		(**hw).messagerect = r; 
		}
		
	/*do vertscrollbar*/ {
		
		r = menurect; /*the space occupied by the grid of cells*/
		
		r.left = r.right; /*scrollbar is just to right of grid*/
		
		r.right = r.left + scrollbarwidth;
		
		setscrollbarrect ((**hw).vertscrollbar, r);
		
		//showscrollbar ((**hw).vertscrollbar);
		}
	
	return (true);
	} /*meresetwindowrects*/


void meresize (void) {
	
	Rect outlinerect;
	
	megetoutlinerect (&outlinerect); /*flow through code that adjusts the rect*/
	
	insetrect (&outlinerect, 1, 1);
	
	mesetalloutlinerects (outlinerect);
	} /*meresize*/


