
/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include "appletinternal.h"
#include "appletmain.h"
#include "appletmenu.h"
#include "appletmenuops.h"
#include "appletwindowmenu.h"


MenuHandle hdlwindowmenu;

static WindowPtr **windowarray = nil; /*dynamic array of window pointers*/

static short ixwindowarray;

static short firstwindowmenuslot = 1;



typedef boolean (*tyvisitroutine) (WindowPtr);


static boolean visitmacwindows (tyvisitroutine visitproc) {
	
	WindowPtr w = FrontWindow ();
	
	while (w != nil) {
		
		if (!(*visitproc) (w)) /*stop the traversal*/
			return (false);
		
		w = (WindowPtr) (*(WindowPeek) w).nextWindow; 
		} /*while*/
		
	return (true); /*completed the traversal*/
	} /*visitmacwindows*/
	
	
static boolean countvisit (WindowPtr w) {

	ixwindowarray++;
	
	return (true);
	} /*countvisit*/
	
	
static short countmacwindows (void) {
	
	ixwindowarray = 0;
	
	visitmacwindows (&countvisit);
	
	return (ixwindowarray);
	} /*countmacwindows*/
	

static boolean fillwindowarrayvisit (WindowPtr w) {
	
	(*windowarray) [ixwindowarray++] = w;

	return (true); /*keep going*/
	} /*fillwindowarrayvisit*/
	
	
void adjustwindowmenu (void) {
/*
	LDR 10/29/95	if there are no windows in the menu, disable the menu title
*/

	MenuHandle hmenu = hdlwindowmenu;
	short ctwindows, i;
	
	for (i = countmenuitems (hmenu); i >= firstwindowmenuslot; i--)
		deletemenuitem (hmenu, i);
		
	disposehandle ((Handle) windowarray);
	
	windowarray = nil;
	
	ctwindows = countmacwindows ();
	
	if (!newclearhandle (ctwindows * longsizeof (WindowPtr), (Handle *) &windowarray))
		return;
	
	ixwindowarray = 0;
	
	visitmacwindows (&fillwindowarrayvisit);
	
	/*sort the window array*/ {
		
		bigstring bssmallest;
		short ixsmallest;
		short i, j;
		bigstring bs;
		
		for (i = 0; i < ctwindows; i++) {
			
			ixsmallest = i;
			
			GetWTitle ((*windowarray) [i], bssmallest);
			
			alllower (bssmallest);
			
			for (j = i + 1; j < ctwindows; j++) {
				
				GetWTitle ((*windowarray) [j], bs);
				
				alllower (bs);
				
				if (stringlessthan (bs, bssmallest)) {
					
					copystring (bs, bssmallest);
					
					ixsmallest = j;
					}
				} /*for*/
			
			if (ixsmallest != i) {
				
				WindowPtr x = (*windowarray) [i];
				
				(*windowarray) [i] = (*windowarray) [ixsmallest];
				
				(*windowarray) [ixsmallest] = x;
				}
			} /*for*/
		}
		
	for (i = 0; i < ctwindows; i++) {
		
		WindowPtr w = (*windowarray) [i];
		bigstring windowtitle;
		short ct;
		hdlappwindow appwindow;
		
		GetWTitle (w, windowtitle);
		
		pushmenuitem (hmenu, windowtitle, true);
		
		ct = countmenuitems (hmenu);
		
		if (getappwindow (w, &appwindow)) {
		
			if ((**appwindow).flmadechanges)
				stylemenuitem (hmenu, ct, underline);
			}
			
		if (w == FrontWindow ()) 
			checkmenuitem (hmenu, ct, true);
		} /*for*/
		
	if (countmenuitems (hmenu) == 0)
		disableallmenuitems(hmenu);
	else
		enableallmenuitems(hmenu);

	} /*adjustwindowmenu*/
	
	
void selectwindowmenu (short iditem) {

	if (windowarray != nil)
		SelectWindow ((*windowarray) [iditem - firstwindowmenuslot]);
	} /*selectwindowmenu*/
	
	
void initwindowmenu (void) {

	if (app.haswindowmenu) {
	
		installmenu (windowmenu, &hdlwindowmenu);
	
		firstwindowmenuslot = countmenuitems (hdlwindowmenu) + 1;
		}
	} /*initwindowmenu*/
	
	
	
	

