
/*	$Id$    */

/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include "appletdefs.h"
#include "appletcursor.h"



static long ticklastroll = 0;

static short beachballstate = cursorisbeachball4;

static short earthstate = cursorisearth7;



void setcursortype (tycursortype cursor) {
	
	CursHandle hcursor;
	
	if (cursor == cursorisarrow) {
		
		SetCursor (&quickdrawglobal (arrow));
		
		return;
		}
	
	hcursor = GetCursor (cursor);
	
	if (hcursor != nil)	
		SetCursor (*hcursor);
	} /*setcursortype*/


void obscurecursor (void) {
	
	ObscureCursor ();
	} /*obscurecursor*/


static boolean rollingtimerexpired (void) {
	
	long tc;
	
	tc = TickCount ();
	
	if ((ticklastroll == 0) || (tc > (ticklastroll + 6))) {
	
		ticklastroll = tc;
		
		return (true);
		}
		
	return (false);
	} /*rollingtimerexpired*/
	
	
void rollbeachball (void) {
	
	tycursortype state;
	
	if (rollingtimerexpired ()) {
	
		state = (tycursortype) (beachballstate + 1);
		
		if (state > cursorisbeachball4) 
			state = cursorisbeachball1;
		
		setcursortype (state);
		
		beachballstate = state;
		}
	} /*rollbeachball*/


void initbeachball (void) {
	
	ticklastroll = 0;
	
	rollbeachball ();
	} /*initbeachball*/
	
	
void initearth (void) {
	
	earthstate = cursorisearth1;
	
	ticklastroll = TickCount ();
	} /*initearth*/


void rollearth (void) {
	
	tycursortype state;
	
	if (rollingtimerexpired ()) {
	
		state = (tycursortype) (earthstate + 1); 
		
		if (state > cursorisearth7) /*wrap around*/
			state = cursorisearth1;
						
		setcursortype (state);
		
		earthstate = state;
		}
	} /*rollearth*/


void watchcursor () {
	
	CursHandle hcursor;
	
	hcursor = GetCursor (watchCursor);
	
	if (hcursor != nil) 
		SetCursor (*hcursor);
	} /*watchcursor*/
	
	
void arrowcursor () {
	
	SetCursor (&quickdrawglobal (arrow));
	} /*arrowcursor*/
	
	


