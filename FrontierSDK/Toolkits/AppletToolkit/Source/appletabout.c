
/*	$Id$    */

/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include "appletinternal.h"
#include "appletops.h"
#include "appletquickdraw.h"
#include "appletmain.h"
#include "appletzoom.h"
#include "appletabout.h"


#ifdef MPWC

#include <toolutils.h>
#include <osevents.h>

#endif



#define aboutlistnumber 128

WindowPtr aboutwindow = nil;





#define doline(x)\
																			\
	MoveTo (r.left + (r.right - r.left - StringWidth (x)) / 2, linev);		\
																			\
	DrawString (x);															\
																			\
	linev += lineinc;


static void drawabout (WindowPtr w) {
	
	register short i;
	register short linev = 24;
	register short lineinc = 14;
	bigstring lines [6];
	Rect r;
	
	for (i = 0; i <= 5; i++)	
		GetIndString (lines [i], aboutlistnumber, i + 1);
	
	r = (*w).portRect;
	
	InsetRect (&r, 4, 4);
	
	TextFont (systemFont);
	
	TextSize (12);
	
	doline (lines [0]);
	
	TextFont (geneva);
	
	TextSize (9);
	
	doline (lines [1]);
	
	linev += lineinc; /*skip a line*/
	
	doline (lines [2]);
	
	doline (lines [3]);
	
	MoveTo (r.left + 4, r.bottom - 6);
	
	DrawString (lines [4]);
	
	MoveTo (r.right - StringWidth (lines [5]) - 4, r.bottom - 6);
	
	DrawString (lines [5]);
	} /*drawabout*/
	

void openabout (Boolean flzoom) { /*can also be called to implement the splash window*/

	Rect r;
	
	SetRect (&r, 0, 0, 340, 120);
	
	pushmacport (aboutwindow = NewWindow (nil, &r, (ConstStr255Param)"\p", false, altDBoxProc, (WindowPtr) -1, false, 0));
		
	centerwindow ((DialogPtr) aboutwindow, quickdrawglobal (screenBits).bounds);
	
	if (flzoom)
		zoomport (aboutwindow, true);
	else
		ShowWindow (aboutwindow);
	
	drawabout (aboutwindow);
	} /*openabout*/
	
	
void closeabout (Boolean flzoom) {

	if (aboutwindow != nil) {
	
		HideWindow (aboutwindow);
		
		if (flzoom)
			zoomport (aboutwindow, false);
		else
			HideWindow (aboutwindow);
		
		DisposeWindow (aboutwindow);
		
		aboutwindow = nil;
		
		popmacport ();
		}
	} /*closeabout*/
	
	
static boolean aboutwindowopen (void) {
	
	return (aboutwindow != nil);
	} /*aboutwindowopen*/


static boolean abouthandleevent (EventRecord ev, boolean *boxgoesaway) {

	/*
	return true if the event is consumed, false if not.
	
	set boxgoesaway to true if the event should cause the about box to disappear.
	*/
	
	*boxgoesaway = false;
	
	switch (ev.what) {
	
		case keyDown: case autoKey: {
		
			register char ch = ev.message & charCodeMask;
			
			if ((ch == chreturn) || (ch == chenter)) {
				
				*boxgoesaway = true;
				
				return (true); /*event consumed*/
				}
				
			break;
			}
			
		case mouseDown:	{	
			WindowPtr w;
			
			FindWindow (ev.where, &w);
			
			if (w != aboutwindow) /*event definitely not consumed*/
				return (false);
				
			if (w == FrontWindow ())
				*boxgoesaway = true;
			else
				SelectWindow (w);
			
			return (true); /*event consumed*/
			}
			
		case updateEvt: {
			register WindowPtr eventwindow = (WindowPtr) (ev.message);
	
			if (eventwindow != aboutwindow)
				return (false); /*event not consumed*/
			
			/*handle update, we might be using a screen saver*/
			
			pushmacport (eventwindow);
			
			BeginUpdate (eventwindow);
			
			drawabout (eventwindow);
			
			EndUpdate (eventwindow);
			
			popmacport ();
								
			return (true); /*event consumed, don't get rid of the about box*/
			}
			
		case activateEvt:								
			return (true); /*event consumed, don't get rid of the about box*/
		} /*switch*/
	
	return (false); /*event not consumed*/
	} /*abouthandleevent*/


extern boolean flexitmainloop;


void aboutcommand (void) {
	
	EventRecord ev;
	boolean flexitabout = false;
	
	openabout (true);
	
	while ((!flexitabout) && (!flexitmainloop)) {
		
		WaitNextEvent (everyEvent, &ev, 1, nil);
		
		if (!abouthandleevent (ev, &flexitabout)) /*event not consumed by about box*/
			appleteventdispatcher (&ev);
		} /*while*/
	
	closeabout (true);
	} /*aboutcommand*/




