
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

/*

wdef.c	

Windoids wdef for PowWow

Variants for this wdef:
	
	¥ 0:	Has a grow zone (and drawgrowicon is implemented).  Does not have rotating states.
	¥ 1:	(not used)
	¥ 2:	Has no grow zone.  Has rotating states.
	

Copyright ©1989 Peter Winer

Peter Winer
894 Parma Way
Los Altos, CA 94024
(415) 948 7142

Revision History:

05-16-89		PWW		Creation

*/

#include <Quickdraw.h>
#include <Windows.h>
#include <Fonts.h>
#include <Memory.h>
#include <ToolUtils.h>

#include <Types.h>
#include <OSUtils.h>
#include <Events.h>
#include <Windows.h>
#include <StdLib.h>
#include <SANE.h>

/*
constants
*/
	#define	kDragHeight		15

/*
types
*/

	typedef struct tywindrects {
		Rect dragrect;
		Rect closerect;
	} tywindrects, *typtrwindrects, **tyhdlwindrects;
		
	typedef Rect *typtrrect;
		

/*
prototypes
*/

	void drawtitle (
		WindowPtr w,
		Rect r);
	
	void filltitlebar (
		WindowPeek peekptr,
		short varcode,
		short flhilited);
	
	void tracestrucrgn (
		WindowPeek peekptr,
		Rect r);
	
	void docalc (
		WindowPeek peekptr);
		
	void dodraw (
		WindowPeek peekptr,
		short varcode,
		long param);
	
	void calcgrow (
		WindowPtr w,
		Rect *r);
	
	long dohit (
		WindowPeek peekptr, 
		short varcode,
		long param);
	
	void dogrow (
		short varcode,
		long param);
	
	void dodrawgrowicon (
		WindowPeek peekptr,
		short varcode);
	
	void donew (
		WindowPeek peekptr);	
	
	void dodispose (
		WindowPeek peekptr);
	
	pascal long winddef (
		short varcode,
		WindowPeek peekptr,
		short message,
		long param);


pascal long main (
	short varcode,
	WindowPeek peekptr,
	short message,
	long param) {

	switch (message) {
			
		case wDraw:
			dodraw (peekptr, varcode, param);

			break;
		
		case wHit: 
			return (dohit (peekptr, varcode, param));
			
			break;
			
		case wCalcRgns: 
			docalc (peekptr);
			
			break;
			
		case wNew: 
			donew (peekptr);
			
			break;
			
		case wDispose: 
			dodispose (peekptr);
			
			break;
			
		case wGrow:
			dogrow (varcode, param);
			
			break;
			
		case wDrawGIcon:
			dodrawgrowicon (peekptr, varcode);
			
			break;
			
		} /* switch */

	return (0);
	} /* main */


static short odd (i)
	short i; {
	
	return (i % 2);
	} /* odd */


static void localtoglobalrect (r)
	Rect *r; {
	
	LocalToGlobal ((Point *) r);
	
	LocalToGlobal ((Point *) r + 1);
	
	/*
	Point pttemp;
	
	pttemp.h = (*r).left;
	
	pttemp.v = (*r).top;
	
	LocalToGlobal (&pttemp);
	
	(*r).left = pttemp.h;
	
	(*r).top = pttemp.v;
	
	pttemp.h = (*r).right;
	
	pttemp.v = (*r).bottom;
	
	LocalToGlobal (&pttemp);
	
	(*r).right = pttemp.h;
	
	(*r).bottom = pttemp.v;
	*/
	} /* localtoglobalrect */
	
	
static void globaltolocalrect (r)
	Rect *r; {
	
	GlobalToLocal ((Point *) r);
	
	GlobalToLocal ((Point *) r + 1);
	
	/*
	Point pttemp;
	
	pttemp.h = (*r).left;
	
	pttemp.v = (*r).top;
	
	GlobalToLocal (&pttemp);
	
	(*r).left = pttemp.h;
	
	(*r).top = pttemp.v;
	
	pttemp.h = (*r).right;
	
	pttemp.v = (*r).bottom;
	
	GlobalToLocal (&pttemp);
	
	(*r).right = pttemp.h;
	
	(*r).bottom = pttemp.v;
	*/
	} /* globaltolocalrect */
	
	
static void drawtitle (
	WindowPtr w,
	Rect r) {
	
	/*
	Str255 bstitle;
	*/
	StringHandle htitle;
	FontInfo charset;
	short hcoord;
	short vcoord;
	short savefont;
	short savesize;
	short saveface;
	WindowPtr currentport;
	
	/*
	GetWTitle (w, bstitle);
	*/
	
	htitle = (*(WindowPeek) htitle).titleHandle;
	
	GetPort (&currentport);
	
	savefont = (*currentport).txFont;
	
	savesize = (*currentport).txSize;
	
	saveface = (*currentport).txFace;
		
	TextFont (geneva);
	
	TextSize (9);
	
	TextFace (0);
	
	GetFontInfo (&charset);
	
	HLock ((Handle) htitle);
	
	hcoord = r.right - r.left - StringWidth (*htitle);
	
	
	hcoord = hcoord >> 1;
	
	hcoord += r.left;
	
	vcoord = r.bottom - r.top - charset.ascent - charset.descent;
	
	vcoord = vcoord >> 1;
	
	vcoord += r.top + charset.ascent;
	
	r.top = vcoord - charset.ascent;
	
	r.bottom = vcoord + charset.descent;
	
	r.left = hcoord;
	
	r.right = hcoord + StringWidth (*htitle);
		
	InsetRect (&r, -2, 0);
	
	EraseRect (&r);
	
	MoveTo (hcoord, vcoord);
	
	DrawString (*htitle);
	
	HUnlock ((Handle) htitle);
	
	(*currentport).txFont = savefont;
	
	(*currentport).txSize = savesize;
	
	(*currentport).txFace = saveface;
	} /* drawtitle */
	
	
static void filltitlebar (
	WindowPeek peekptr,
	short varcode,
	short flhilited) {

	#pragma unused (varcode)
	
	Pattern pat;
	tyhdlwindrects hdlwindrects;
	Rect r;
		
	hdlwindrects = (tyhdlwindrects) (*peekptr).dataHandle;
	
	HLock ((*peekptr).dataHandle);
	
	r = (**hdlwindrects).dragrect;
		
	if (!flhilited) {
		FrameRect (&r);
		
		InsetRect (&r, 1, 1);
		
		EraseRect (&r);
		
		HUnlock ((*peekptr).dataHandle);

		return;
		}
		
	if (odd (r.top)) {
		if (odd (r.left))
			StuffHex (&pat, "\p0055005500550055");
		else 
			StuffHex (&pat, "\p00AA00AA00AA00AA");
		}
	else {
		if (odd (r.left))
			StuffHex (&pat, "\p5500550055005500");
		else 
			StuffHex (&pat, "\pAA00AA00AA00AA00");
		}
		
	InsetRect (&r, 1, 1);

	FillRect (&r, pat);

	FrameRect (&(**hdlwindrects).dragrect);
						
	EraseRect (&(**hdlwindrects).closerect);

	FrameRect (&(**hdlwindrects).closerect);
	
	HUnlock ((*peekptr).dataHandle);
	} /*filltitlebar*/
	

static void tracestrucrgn (
	WindowPeek peekptr,
	Rect r) {
	
	/*
	12/10/91 dmb: rewrote using rect/union rgn instead of open/close rgn
	*/
	
	register RgnHandle strucrgn = (*peekptr).strucRgn;
	register RgnHandle temprgn = NewRgn ();
	
	RectRgn (strucrgn, &r);
	
	OffsetRect (&r, 1, 1);
	
	RectRgn (temprgn, &r);
	
	UnionRgn (temprgn, strucrgn, strucrgn);
	
	DisposeRgn (temprgn);
	
	/*
	OpenRgn ();
	
	MoveTo (r.left, r.top);
	
	LineTo (r.right - 1, r.top);
	
	LineTo (r.right - 1, r.top + 2);
	
	LineTo (r.right, r.top + 2);
	
	LineTo (r.right, r.bottom);
	
	LineTo (r.left + 2, r.bottom);
	
	LineTo (r.left + 2, r.bottom - 1);
	
	LineTo (r.left, r.bottom - 1);
	
	LineTo (r.left, r.top);
	
	CloseRgn ((*peekptr).strucRgn);
	*/
	} /* tracestrucrgn */


static void docalc (
	WindowPeek peekptr) {
	
	Rect r;
	Rect rbounds;
	
	tyhdlwindrects hdlwindrects;

	HLock ((*peekptr).dataHandle);
	
	hdlwindrects = (tyhdlwindrects) (*peekptr).dataHandle;
	
	r = (*peekptr).port.portRect;
	
	if ((*peekptr).port.portBits.rowBytes > 0)
		rbounds = (*peekptr).port.portBits.bounds;
	else
		rbounds = (**(*(CGrafPtr) (peekptr)).portPixMap).bounds;
	
	OffsetRect (&r, - rbounds.left, - rbounds.top);
	
	RectRgn ((*peekptr).contRgn, &r);
	
	r.left = r.left - 1;
	
	r.top = r.top - kDragHeight;
	
	r.right = r.right + 1;
	
	r.bottom = r.bottom + 1;
	
	tracestrucrgn (peekptr, r);
	
	r.bottom = r.top + kDragHeight;
	
	(**hdlwindrects).dragrect = r;
	
	InsetRect (&r, 6, 2);
	
	r.right = r.left + kDragHeight - 4;
	
	(**hdlwindrects).closerect = r;
	
	HUnlock ((*peekptr).dataHandle);
	} /* docalc */


static void dodraw (
	WindowPeek peekptr,
	short varcode,
	long param) {
	
	tyhdlwindrects hwindrects;
	Rect r;
	Pattern pat;
	
	if (!(*peekptr).visible)
		return;
	
	hwindrects = (tyhdlwindrects) (*peekptr).dataHandle;
	
	param = LoWord (param); /*TN #290*/
	
	if (param == 4) {
		HLock ((*peekptr).dataHandle);
		
		InvertRect (&(**hwindrects).closerect);
		
		HUnlock ((*peekptr).dataHandle);
		
		return;
		}
	
	if (param != 0)
		return;
		
	docalc (peekptr);
	
	filltitlebar (peekptr, varcode, (*peekptr).hilited);
	
	StuffHex ((Ptr) &pat, "\pFFFFFFFFFFFFFFFF");
	
	PenPat (pat);
		
	FrameRgn ((*peekptr).strucRgn);
	
	r = (**(*peekptr).strucRgn).rgnBBox;
	
	MoveTo (r.left, r.bottom - 2);
	
	LineTo (r.right - 2, r.bottom - 2);
	
	LineTo (r.right - 2, r.top);
	
	drawtitle ((WindowPtr) peekptr, (**hwindrects).dragrect);
	} /* dodraw */


static void calcgrow (
	WindowPtr w,
	Rect *r) {
	
	WindowPtr currentport;	
	
	*r = (*w).portRect;
	
	(*r).top = (*r).bottom - 12;
	
	(*r).left = (*r).right - 12;
	
	(*r).bottom = (*r).bottom + 1;
	
	(*r).right = (*r).right + 1;
		
	GetPort (&currentport);
	
	SetPort (w);
	
	localtoglobalrect (r);
	
	SetPort (currentport);
	} /* calcgrow */
	
	
static long dohit (
	WindowPeek peekptr, 
	short varcode,
	long param) {

	Point pt;
	tyhdlwindrects hwindrects;
	Rect r;
	
	docalc (peekptr);

	pt.h = LoWord (param);
	
	pt.v = HiWord (param);

	hwindrects = (tyhdlwindrects) (*peekptr).dataHandle;

	if (PtInRect (pt, &(**hwindrects).closerect))
		return (wInGoAway);
	
	if (PtInRect (pt, &(**hwindrects).dragrect)) 
		return (wInDrag);
		
	if (varcode != 2) {
		calcgrow ((WindowPtr) peekptr, &r);
		
		if (PtInRect (pt, &r))
			return (wInGrow);
		}
		
	if (PtInRgn (pt, (*peekptr).contRgn))
		return (wInContent);

	return (wNoHit);
	} /* dohit */
	
	
static void dogrow (
	short varcode,
	long param) {
	
	typtrrect ptrrect;
	Rect r;
	
	if (varcode == 2)
		return;
		
	ptrrect = (typtrrect) param;
	
	r = *ptrrect;
	
	r.left = r.left - 1;
	
	r.top = r.top - kDragHeight;
	
	r.right = r.right + 2;
	
	r.bottom = r.bottom + 2;
			
	FrameRect (&r);
	
	MoveTo ((*ptrrect).right - 12, r.top + 1);
	
	LineTo ((*ptrrect).right - 12, r.bottom - 1);
	} /* dogrow */


static void dodrawgrowicon (
	WindowPeek peekptr,
	short varcode) {
	
	Rect r;
	
	if (varcode == 2)
		return;
		
	calcgrow ((WindowPtr) peekptr, &r);
	
	SetPort ((WindowPtr) peekptr);
	
	globaltolocalrect (&r);
	
	PenNormal ();
	
	EraseRect (&r);
	
	FrameRect (&r);
	
	InsetRect (&r, 2, 2);
	
	r.top = r.top + 2;
	
	r.left = r.left + 2;
	
	FrameRect (&r);
	
	OffsetRect (&r, -2, -2);
	
	r.bottom = r.bottom - 1;
	
	r.right = r.right - 1;
	
	EraseRect (&r);
	
	FrameRect (&r);
	} /* dodrawgrowicon */


static void donew (
	WindowPeek peekptr) {
	
	tyhdlwindrects hdlwindrects;

	(*peekptr).dataHandle = NewHandle (sizeof (tywindrects));
	
	hdlwindrects = (tyhdlwindrects) (*peekptr).dataHandle;
	
	docalc (peekptr);
	} /* donew */
	

static void dodispose (
	WindowPeek peekptr) {
	
	tyhdlwindrects hdlwindrects;

	hdlwindrects = (tyhdlwindrects) (*peekptr).dataHandle;
	
	DisposHandle ((*peekptr).dataHandle);
	} /* dodispose */


