
/*	$Id$    */

/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include "appletdefs.h"
#include "appletmouse.h"


tymouserecord mousestatus;


boolean mousetrack (Rect r, tymousetrackcallback displaycallback) {
	
	/*
	hang out in this routine until the mouse button comes up.  return true if
	the mouse point is in the indicated rectangle when we return.
	
	7/17/90 DW: if the mouse wasn't down when we enter, return true.  this is
	a heuristic that may allow some mouse-dependent routines to be driven by
	a script.  we figure that if the mouse isn't down now, it never was down,
	and whether it's inside any particular rectangle is irrelevent.
	
	7/17/90 DW: add callback routine to display the object being tracked as
	the mouse moves in and out of the rectangle.  set it to nil if you don't
	need this feature.
	
	7/17/90 DW: only call the callback when the state of the object changes,
	assume display is correct when we're entered.
	*/
	
	boolean flinrectnow;
	boolean flwasinrect;
	
	if (!StillDown ()) /*see comment above*/
		return (true);
	
	flwasinrect = true; /*at least for the first iteration of loop*/
	
	while (StillDown ()) { /*stay in holding pattern*/
	
		Point pt;
		
		GetMouse (&pt);
	
		flinrectnow = PtInRect (pt, &r);
		
		if (flinrectnow != flwasinrect) { /*state of object changed*/
		
			if (displaycallback != nil)
				(*displaycallback) (flinrectnow);
			}
			
		flwasinrect = flinrectnow;
		} /*while*/
	
	return (flwasinrect);
	} /*mousetrack*/


static short absint (short x) {
	
	if (x < 0)
		x = -x;
		
	return (x);
	} /*absint*/
	
	
static short pointdist (Point pt1, Point pt2) {
	
	return (absint (pt1.h - pt2.h) + absint (pt1.v - pt2.v));
	} /*pointdist*/


boolean mousedoubleclick (Point pt) {

	/*
	using the globals mouseuptime and mouseuppoint determine if a
	mouseclick at pt, right now, is a double click.
	
	dmb 9/6/90:  pt parameter is no longer used.  superceeded by new 
	mousedown globals
	*/
	
	#pragma unused (pt)
	
	boolean fldoubleclick;
	short diff;
	
	fldoubleclick = (mousestatus.mousedowntime - mousestatus.mouseuptime) < GetDblTime ();
	
	if (fldoubleclick) { /*qualifies so far*/
		
		diff = pointdist (mousestatus.mousedownpoint, mousestatus.mouseuppoint);
		
		fldoubleclick = diff < 5; /*keep it set if mouse hasn't wandered too far*/
		}
	
	if (fldoubleclick) { /*user must doubleclick again to get effect*/
		
		mousestatus.mouseuptime = 0L;
		
		mousestatus.mouseuppoint.h = mousestatus.mouseuppoint.v = 0;
		}
	
	mousestatus.fldoubleclickdisabled = fldoubleclick; /*copy into global*/
	
	return (fldoubleclick);
	} /*mousedoubleclick*/


boolean mousecheckautoscroll (Point pt, Rect r, boolean flhoriz, tydirection *dir) {
	
	/*
	if the point pt is outside of the rect r in the indicated dimension (flhoriz), 
	return true and set dir to the corresponding scroll direction.
	*/
	
	tydirection d = nodirection;
	
	if (flhoriz) {
	
		if (pt.h < r.left)
			d = right;
		
		else if (pt.h > r.right)
			d = left;
		}
	else {
		
		if (pt.v < r.top)
			d = down;
		
		else if (pt.v > r.bottom)
			d = up;
		}
	
	*dir = d;
	
	return (d != nodirection);
	} /*mousecheckautoscroll*/


