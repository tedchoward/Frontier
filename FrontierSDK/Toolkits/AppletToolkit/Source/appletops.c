
/*	$Id$    */

/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include <Processes.h>
#include "appletstrings.h"
#include "appletops.h"


#ifdef MPWC

	#include <osevents.h>

#endif


/*
7/19/94 dmb: Updated for Univeral Headers
*/



typedef struct tymemoryconfig {
	
	long minstacksize; /*minimum stack space required*/
	
	long minheapsize; /*minimum heap space to run program*/
	
	long avghandlesize; /*to determine # of master blocks; use zero for none*/
	
	long reserved1;
	
	long reserved2;
	} tymemoryconfig, **hdlmemoryconfig;



boolean initmacintosh () {

	/*
	the magic stuff that every Macintosh application needs to do 
	before doing anything else.
	
	2/4/93 dmb: added support for MCFG resource to enforce heap/stack sizes
	*/
	
	short i;
	hdlmemoryconfig h;
	long ctbytes;
	long ctstack, ctheap;
	short ctmasters = 10;
	
	h = (hdlmemoryconfig) Get1Resource ('MCFG', 1);

	if (h != nil) { /*first check stack size*/
	
		ctbytes = (**h).minstacksize;
	
		ctstack = LMGetCurStackBase () - GetApplLimit (); /*current stack size*/
	
		if (ctbytes > ctstack)
			SetApplLimit (LMGetCurStackBase () - ctbytes);
		}
		
	MaxApplZone ();
	
	if (h != nil) { /*check heap size and master pointers*/
		
		ctbytes = (**h).minheapsize;
		
		ctheap = FreeMem ();
		
		if (ctbytes > ctheap)
			return (false);
		
		ctbytes = (**h).avghandlesize;
		
		if (ctbytes > 0)
			ctmasters = (ctheap / ctbytes) / (*GetZone ()).moreMast;
		
		ReleaseResource ((Handle) h); /*we're done with it*/
		}
	
	for (i = 1; i <= ctmasters; i++) 
		MoreMasters ();
	
	InitGraf (&quickdrawglobal (thePort));
	
	InitFonts ();
	
	FlushEvents (everyEvent, 0);
	
	InitWindows ();
	
	InitMenus ();
	
	TEInit ();
	
	InitDialogs (0L);
	
	InitCursor ();
	
	for (i = 1; i <= 5; i++) { /*register with Multifinder*/
		
		EventRecord ev;
		
		EventAvail (everyEvent, &ev); /*see TN180 -- splash screen*/
		} /*for*/
	
	return (true);
	} /*initmacintosh*/
	

short minint (short x, short y) {

	if (x < y)
		return (x);
	else
		return (y);
	} /*minint*/
	
	
boolean cometofront () {
	
	short i;
	ProcessSerialNumber psn;
	EventRecord ev;
	
	GetCurrentProcess (&psn);
	
	SetFrontProcess (&psn);
	
	for (i = 1; i <= 3; i++)
		WaitNextEvent (nullEvent, &ev, 1, nil);	

	return (true);
	} /*cometofront*/
	
	
void delayticks (short ct) {
	
	long tc;
	
	tc = TickCount () + ct;
	
	while (TickCount () < tc) {}
	} /*delayticks*/
	
	
void delayseconds (short ct) {
	
	delayticks (60 * ct);
	} /*delayseconds*/
	
	
void timestamp (unsigned long *stamp) {
	
	GetDateTime (stamp);
	} /*timestamp*/
	
	
boolean getstringlist (short listnum, short id, bigstring bs) {
	
	/*
	the Mac routine GetIndString doesn't set ResError true when we fall off
	the end of the indicated list, so we return false if the returned string
	is of zero length.
	*/
	
	GetIndString (bs, listnum, id);
	
	return (stringlength (bs) > 0);
	} /*getstringlist*/
	
	


	
	