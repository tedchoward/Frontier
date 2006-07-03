
/*	$Id$    */

/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include "appletdefs.h"
#include "appletquickdraw.h"
#include "appletzoom.h"

#define zoomfixer 65536L

#define zoomsteps 16

Fixed zoomfract;




static short zoomblend (short i1, short i2) {

	Fixed smallFix,bigFix,tempFix;

	smallFix = zoomfixer * i1;
	
	bigFix = zoomfixer * i2;
	
	tempFix = FixMul (zoomfract, bigFix) + FixMul (zoomfixer-zoomfract, smallFix);
	
	return (FixRound (tempFix));
	} /*zoomblend*/
	

static void zoomrect (Rect *smallrect, Rect *bigrect, Boolean zoomup) {

	Fixed factor;
	Rect rect1,rect2,rect3,rect4;
	GrafPtr savePort,deskPort;
	short i;
	
	GetPort (&savePort);
	
	OpenPort (deskPort = (GrafPtr) NewPtr (sizeof (GrafPort)));
	
	InitPort (deskPort);
	
	SetPort (deskPort);
	
	PenPat (&quickdrawglobal (gray));
	
	PenMode (notPatXor);
	
	if (zoomup) {
	
		rect1 = *smallrect;
		
		factor = FixRatio(6,5);
		
		zoomfract = FixRatio(541,10000);
		}
	else {
		rect1 = *bigrect;
		
		factor = FixRatio(5,6);
		
		zoomfract = zoomfixer;
		}
		
	rect2 = rect1;
	
	rect3 = rect1;
	
	FrameRect (&rect1);
	
	for (i = 1; i<= zoomsteps; i++) {
	
		rect4.left = zoomblend (smallrect->left, bigrect->left);
		
		rect4.right = zoomblend (smallrect->right, bigrect->right);
		
		rect4.top = zoomblend (smallrect->top, bigrect->top);
		
		rect4.bottom = zoomblend (smallrect->bottom, bigrect->bottom);
		
		FrameRect (&rect4);
		
		FrameRect (&rect1);
		
		rect1 = rect2;
		
		rect2 = rect3;
		
		rect3 = rect4;
		 
		zoomfract = FixMul (zoomfract,factor);
		} /*for*/
		
	FrameRect (&rect1);
	
	FrameRect (&rect2);
	
	FrameRect (&rect3);
	
	ClosePort (deskPort);
	
	DisposPtr ((Ptr)deskPort);
	
	PenNormal ();
	
	SetPort (savePort);
	} /*zoomrect*/
	
	
void zoomport (WindowPtr w, boolean flup) {

	/*
	Zooms the window referenced by "w" either from an inivisible
	state to a visible state, or vice versa.  Pass true in the "flup"
	boolean parameter to zoom a window to open, an false to zoom
	it close.  The WindowPtr must have already been created elsewhere,
	and zooming the window invisible only hides the window, it does
	not destroy the WindowPtr data.
	*/
	
	Rect r1, r2, r3;

	SetPort (w);
	
	SetRect (&r1, 0, 20, 0, 20);
	
	r3 = (*w).portRect;
	
	r2 = r3;
	
	InsetRect (&r2, (r3.right - r3.left + 20) / 2, (r3.bottom - r3.top + 20) / 2);

	localtoglobalrect (&r2);
	
	localtoglobalrect (&r3);

	if (flup) {
	
		zoomrect (&r1, &r2, true);
		
		zoomrect (&r2, &r3, true);
		
		ShowWindow (w);
		
		SetPort (w);
		}
	else {
		HideWindow (w);
		
		zoomrect (&r2, &r3, false);
		
		zoomrect (&r1, &r2, false);
		}
	} /*zoomport*/
	
	
