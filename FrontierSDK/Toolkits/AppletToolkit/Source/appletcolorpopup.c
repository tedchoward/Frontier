
/*	$Id$    */

/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


/*
written by Dave Shaver, Quark, 303-849-3311, adapted by DW 2/9/93.
*/

#include "appletdefs.h"
#include "appletquickdraw.h"
#include "appletcursor.h"
#include "appletmemory.h"
#include "appletcolorpopup.h"


#define mPopupMsg 3
#define PATCHSIZE 8
#define OUTSETSIZE 1
#define ITEMSIZE (PATCHSIZE+OUTSETSIZE+1)
CSpecArray *ctable;
MenuHandle colormenu;


static void getitemrect (short item, Rect *ritem) {

	short x, y;
	Rect r;
	
	item--;
	
	x = item % 32;
	
	y = item / 32;
	
	r.top = 1 + y * ITEMSIZE;
	
	r.left = 1 + x * ITEMSIZE;
	
	r.bottom = r.top + ITEMSIZE;
	
	r.right = r.left + ITEMSIZE;
	
	*ritem = r;
	} /*getitemrect*/
	
	
static void frameitem (short item, Rect menurect) {

	Rect r;
	
	if (item <= 0)
		return;
		
	pushforecolor (&blackcolor);
	
	pushpen ();
		
	PenMode (patXor);
	
	getitemrect (item, &r);
	
	OffsetRect (&r, menurect.left, menurect.top);
	
	FrameRect (&r);
	
	poppen ();
	
	popforecolor ();
	} /*frameitem*/
	
	
static void drawcolorpopup (Rect menurect) {

	short i, j;
	Rect r;	
	
	pushforecolor (&lightbluecolor);
	
	PaintRect (&menurect);
	
	for (i = 0; i < 8; i++) {
	
		for (j = 0; j < 32; j++) {
		
			getitemrect (i * 32 + j + 1, &r);
			
			OffsetRect (&r, menurect.left, menurect.top);
			
			RGBForeColor (&blackcolor);
			
			InsetRect (&r, 1, 1);
			
			PaintRect (&r);
			
			InsetRect (&r, 1, 1);
			
			RGBForeColor (&(*ctable) [i * 32 + j].rgb);
			
			PaintRect (&r);
			} /*for*/
		} /*for*/
	
	popforecolor ();
	} /*drawcolorpopup*/


static pascal void colormenuhandler (short msg, MenuHandle hmenu, Rect *menurect, Point pt, short *item) {
	
	switch (msg) {
	
		case mDrawMsg: 
			drawcolorpopup (*menurect);
			
			break;
			
		case mChooseMsg: {
			short x;
			Rect rinset;
			
			rinset = *menurect;
			
			InsetRect (&rinset, OUTSETSIZE, OUTSETSIZE);
			
			if (!PtInRect (pt, &rinset)) 
				x = 0;
			else {
				x = 
					32L * ((pt.v - (*menurect).top - OUTSETSIZE) / ITEMSIZE) +
					(pt.h - (*menurect).left - OUTSETSIZE) / ITEMSIZE + 1;
				
				if (x < 0) 
					x = 0;
				}
				
			if (*item == x) /*already highlighted*/
				break;
				
			frameitem (*item, *menurect); /*unhighlight the original item*/
		
			frameitem (x, *menurect); /*highlight the newly selected item*/
				
			*item = x;
			
			break;
			}
			
		case mSizeMsg:
			(**hmenu).menuHeight = 8L * ITEMSIZE + 2;
			
			(**hmenu).menuWidth = 32L * ITEMSIZE + 2;
			
			break;
			
		case mPopupMsg:
			(*menurect).top = pt.h;
			
			(*menurect).right = pt.v;
			
			(*menurect).bottom = (*menurect).top + (**hmenu).menuHeight;
			
			(*menurect).left = (*menurect).right - (**hmenu).menuWidth;
			
			break;
		} /*switch*/
	} /*colormenuhandler*/


static boolean flpopupinit = false;


static boolean initcolorpopup (void) {
	
	CTabHandle syspal;
	Handle mdefh;
	
	if (!systemhascolor ()) 
		return (false);
		
	syspal = GetCTable (8);
	
	HLock ((Handle) syspal);
	
	ctable = &(**syspal).ctTable;
	
	mdefh = appnewhandle (6);
	
	*(short *)*mdefh = 0x4EF9;
	
	*(long *)(*mdefh + sizeof (short)) = (long) &colormenuhandler;
	
	colormenu = GetMenu (256); /*256 blank entries in this menu*/
	
	(**colormenu).menuProc = mdefh;
	
	CalcMenuSize (colormenu);
	
	/*create a picture, didn't make it faster, commented*/ {
	
		/*
		WindowPtr w;
		Rect r;
		
		w = GetNewCWindow (128, nil, (WindowPtr) -1);
		
		pushmacport (w);
		
		r.top = r.left = 0;
		
		r.bottom = (**colormenu).menuHeight;
		
		r.right = (**colormenu).menuWidth;
		
		ClipRect (&r);
		
		colorpopuppict = OpenPicture (&r);
		
		drawcolorpopup (r);
		
		ClosePicture ();
		
		popmacport ();
		
		DisposeWindow (w);
		*/
		}
	
	return (true);
	} /*initcolorpopup*/


boolean clickcolorpopup (Point pt, RGBColor *rgb) {
	
	long result;
	short lo, hi;
	
	if (!flpopupinit)
		flpopupinit = initcolorpopup ();
	
	setcursortype (cursorisarrow);
	
	CalcMenuSize (colormenu);
	
	InsertMenu (colormenu, hierMenu);

	LocalToGlobal (&pt);
	
	result = PopUpMenuSelect (colormenu, pt.v, pt.h, -1);

	DeleteMenu ((**colormenu).menuID);
	
	lo = LoWord (result);
	
	hi = HiWord (result);
	
	if (hi > 0) { /*something was selected*/
	
		*rgb = (*ctable) [lo - 1].rgb;
		
		return (true);
		}
	
	return (false);
	} /*clickcolorpopup*/
	

