
/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include <GestaltEqu.h>
#include <applet.h>
#include <appletmsg.h>
#include <appletmain.h>
#include <appletmsg.h>
#include "appletidle.h"



static boolean flinterruptedbyuser = false;

static long lasteventtickcount = 0;




static boolean callgestalt (long *ticks) {
	
	if (Gestalt ('idle', ticks) == noErr) /*the init is installed*/
		return (true);
		
	*ticks = TickCount () - lasteventtickcount;
	
	return (true);
	} /*callgestalt*/
	

boolean getidletime (long *ticks) {
	
	if (!callgestalt (ticks))
		return (false);
		
	*ticks -= (**app.appwindow).idleatreset;
	
	return (true);
	} /*getidletime*/
	

static boolean resetsystemidlevisit (hdlappwindow appwindow) {
	
	long idleticks;
	
	assert (appwindow != nil);
	
	callgestalt (&idleticks);
	
	(**appwindow).idleatreset = idleticks;
	
	(**appwindow).systemidlecomplete = false;
	
	return (true);
	} /*resetsystemidlevisit*/
	
	
void appresetsystemidle (EventRecord *ev) {
	
	boolean allwindows = false;
	WindowPtr w;
	
	if ((*ev).what != nullEvent) { /*DW 4/28/95: support init-less installs*/
		
		/*appsetmessage (app.appwindow, "\pxxxxxx");*/
	
		lasteventtickcount = TickCount ();
		}
	
	switch ((*ev).what) {
		
		case keyDown: case autoKey: 
			w = FrontWindow ();
			
			break;
			
		case mouseDown:	case mouseUp: {
		
			switch (FindWindow ((*ev).where, &w)) {
				
				case inContent: case inDrag: case inGrow:
				case inGoAway: case inZoomIn: case inZoomOut:
					break; /*fall thru*/
					
				default:
					return; /*avoid*/
				} /*switch*/
			
			break;
			}
		
		case osEvt:
			if (((*ev).message & resumeFlag) == 0) /*we're being switched out, not in*/
				return;
				
			allwindows = true;
			
			break;
			
		case diskEvt:
			allwindows = true;
			
			break;
			
		default:
			return;
		} /*switch*/

	flinterruptedbyuser = false;
	
	if (allwindows)
		visitappwindows (&resetsystemidlevisit);
	else {
		hdlappwindow appwindow;
		
		if (getappwindow (w, &appwindow))
			resetsystemidlevisit (appwindow);
		}
	} /*appresetsystemidle*/


boolean appuseractive (void) {
	
	/*
	call this from your system idle handler, we return true if
	the user did something after your handler started. you 
	should break out of your loop and return asap.
	*/
	
	long idleticks, shouldbe;
	EventRecord ev;
	
	if (flinterruptedbyuser)
		return (true);
		
	if (EventAvail (everyEvent, &ev)) {
	
		flinterruptedbyuser = true;
		
		return (true);
		}
	
	if (!getidletime (&idleticks)) /*the system idle traps aren't patched*/
		return (false);
	
	shouldbe = TickCount () - (**app.appwindow).lastsystemidle;
	
	if (idleticks < shouldbe) { 
		
		flinterruptedbyuser = true;
		
		return (true);
		}
	
	return (false);
	} /*appuseractive*/
	
	
static boolean systemidlevisit (hdlappwindow appwindow) {
	
	/*
	DW 8/26/93: I couldn't understand why CB's idle routine would start
	up while I was holding down the right arrow key, just after doing
	the first movement, and before doing the second. then I figured it
	out -- the system waits "KeyThresh" ticks before sending the second
	keystroke. just long enough if systemidleticks is set to 20, to sneak
	another call to the idle routine. made a jerky start even jerkier.
	changed cb's systemidleticks value to 0, and added a check here that
	effectively ignores systemidleticks values that are less than
	KeyThresh.
	*/
	
	hdlappwindow ha = appwindow;
	long idleticks;
	EventRecord ev;
	
	if ((**ha).systemidlecallback == nil) /*no callback installed*/
		return (true);
		
	if (EventAvail (everyEvent, &ev)) /*we've got more important things to do*/
		return (true);
	
	if (!getidletime (&idleticks)) /*the system idle traps aren't patched*/
		return (true);
		
	if (idleticks <= LMGetKeyThresh ()) /*see comment at head of routine*/
		return (true);
		
	if (idleticks < (**ha).systemidleticks) /*haven't idled long enough*/
		return (true);
		
	if ((**ha).systemidlecomplete) /*the idle task completed since the user did anything*/
		return (true);
	
	(**app.appwindow).lastsystemidle = TickCount () - idleticks;
	
	flinterruptedbyuser = false;
	
	setappwindow (ha);

	appprecallback ();

	(*(**ha).systemidlecallback) ((**ha).systemidlerefcon);

	(*app.preupdatecallback) ();

	apppostcallback ();	
			
	return (true); /*keep traversing*/
	} /*systemidlevisit*/
	
	
void appsystemidle (void) {

	visitappwindows (&systemidlevisit);
	} /*appsystemidle*/
	
	
