
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


#include "cursor.h"



static tycursortype lastcursor = cursorisdirty;

static tycursortype beachballstate = cursorisbeachball4;

/*
static short earthstate = cursorisearth7;
*/

static long ticklastroll = 0;

static tydirection rolldirection = right;




void setcursortype (tycursortype newcursor) {
	
	/*
	7/30/90 dmb:  don't assume that cursor is never changed behind your back
	*/

	register tycursortype cursor = newcursor;
	#ifdef MACVERSION
		CursHandle hcursor;
	#endif
	#ifdef WIN95VERSION
		LPTSTR idc;
		HCURSOR hcursor;
	#endif
	
	lastcursor = cursor; /*remember for next time*/

	if (cursor == cursorisdirty)
		return;
	
#ifdef MACVERSION	
	if (cursor == cursorisarrow)
		ticklastroll = 0; /*disable rolling until reinitialized*/

	if (cursor == cursorisarrow) {
		
		//Code change by Timothy Paustian Sunday, May 7, 2000 11:00:01 PM
		//Changed to Opaque call for Carbon
		#if ACCESSOR_CALLS_ARE_FUNCTIONS == 1
		Cursor	theArrow;
		GetQDGlobalsArrow(&theArrow);
		SetCursor(&theArrow);
		#else
		SetCursor (&qd.arrow);
		#endif
		return;
		}
	
	hcursor = GetCursor (cursor);
	
	if (hcursor == nil) /*resource error*/
		return;
	
	SetCursor (*hcursor);
#endif

#ifdef WIN95VERSION
	if (cursor < cursorisbeachball1 || cursor > cursorisbeachball4)
		ticklastroll = 0; /*disable rolling until reinitialized*/
	
	switch (cursor) {
		
		case cursorisibeam:
			idc = IDC_IBEAM;
			break;

		case cursoriswatch:
			idc = IDC_WAIT;
			break;

		case cursorisverticalrails:
			idc = IDC_SIZEWE;
			break;

		case cursorishorizontalrails:
			idc = IDC_SIZENS;
			break;

		case cursorisarrow:
			idc = IDC_ARROW;
			break;

		default:
			hcursor = LoadCursor (shellinstance, MAKEINTRESOURCE (cursor));
			
			if (hcursor != NULL) {
				
				SetCursor (hcursor);
				
				return;
				}

			idc = IDC_ARROW;
			break;
		}

	
	SetCursor (LoadCursor (NULL, idc));
#endif

	} /*setcursortype*/


void obscurecursor (void) {
#ifdef MACVERSION	
	ObscureCursor ();
#endif
	} /*obscurecursor*/


static boolean rollingtimerexpired (void) {
	
	register long tc;
	
	if (ticklastroll == 0) /*timer hasn't been initted*/
		return (false);
	
	tc = gettickcount ();

#ifdef WIN95VERSION
	if ((ticklastroll + 30) > tc) /*7.0b20 PBS: don't let the Windows cursor change often -- it flashes*/
		return (false);
#endif

#ifdef MACVERSION
	
	if ((ticklastroll + 6) > tc) /*a tenth of a second hasn't passed since last bump*/
		return (false);
#endif
		
	ticklastroll = tc; /*enough time has passed, reset the timer*/
	
	return (true);
	} /*rollingtimerexpired*/


void initbeachball (tydirection dir) {
	
	rolldirection = dir; /*always set*/
	
	if (ticklastroll == 0) { /*beach ball isn't already initted*/
		
		beachballstate = cursorisbeachball4;
		
#ifdef MACVERSION
		InitCursor (); /*make sure it's visible*/
#endif
#ifdef WIN95VERSION
		ShowCursor(TRUE);
		SetCursor (LoadCursor (NULL, MAKEINTRESOURCE(IDC_ARROW)));
#endif
		ticklastroll = gettickcount ();
		}
	} /*initbeachball*/


void rollbeachball (void) {
	
	#ifdef MACVERSION
		register tycursortype state;
		
		if (rollingtimerexpired ()) {
			
			state = beachballstate;
			
			if (rolldirection == right) {
				
				if (--state < cursorisbeachball1)
					state = cursorisbeachball4;
				}
			else {
				
				if (++state > cursorisbeachball4) /*wrap around*/
					state = cursorisbeachball1;
				}
			
			setcursortype (state);
			
			beachballstate = state;
			}
	#endif

	#ifdef WIN95VERSION
		if (rollingtimerexpired ())
			setcursortype (cursoriswatch);
	#endif
	} /*rollbeachball*/


boolean beachballcursor (void) {
	
	/*
	return true if the cursor is one of the beachballs.
	
	12/26/90 dmb: new test accounts for the fact that, after an initbeachball, 
	lastcursor won't be a beach ball until the timer has expired.  this test 
	will return true if either rolling cursor is active (earth or beach ball)
	*/
	
	/*
	return ((lastcursor >= cursorisbeachball1) && (lastcursor <= cursorisbeachball4));
	*/
	
	return (ticklastroll != 0);
	} /*beachballcursor*/

/*
void initearth (void) {
	
	earthstate = cursorisearth1;
	
	ticklastroll = gettickcount ();
	} /*initearth%/


void rollearth (void) {
	
	register short state;
	
	if (rollingtimerexpired ()) {
	
		state = earthstate + 1; 
		
		if (state > cursorisearth7) /*wrap around%/
			state = cursorisearth1;
						
		setcursortype (state);
		
		earthstate = state;
		}
	} /*rollearth%/
*/




