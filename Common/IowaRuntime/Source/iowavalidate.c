
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

#include "frontier.h"
#include "standard.h"

#include "shelltypes.h"
#include "iowacore.h"

#ifndef macBirdRuntime

	#include <applet.h>

#endif


static void __getmessage (bigstring bsin, bigstring bsout) {
	
	copystring ("\pFailed validation check: ", bsout);
	
	pushstring (bsin, bsout);
	} /*__getmessage*/
	
	
#define invalid(bs) {bigstring bsout; __getmessage (bs, bsout); DebugStr (bs); return (false);}


static hdlobject hcheck;


static boolean checkobjectnamevisit (hdlobject h) {
	
	if (h != hcheck) {
	
		if (equalhandles ((**hcheck).objectname, (**h).objectname))
			invalid ("\ptwo objects have the same name");
		}
	
	return (true); /*keep visiting*/
	} /*checkobjectnamevisit*/
	

static void checkobjectname (hdlobject h) {
	
	if ((**iowadata).runmode) /*a lot of older cards have objects without names*/
		return;
		
	hcheck = h;
	
	visitobjects ((**iowadata).objectlist, &checkobjectnamevisit);
	} /*checkobjectname*/
	
	
static boolean validateobjectlist (hdlobject hlist) {
	
	hdlcard hc = iowadata;
	hdlobject h = hlist;
	
	while (h != nil) {
		
		if ((**h).owningcard != iowadata)
			invalid ("\powningcard is incorrect");
			
		if (!(**iowadata).runmode) { /*a lot of older cards have objects without names*/
		
			if ((**h).objectname == nil)
				invalid ("\pall objects must have names");
			}
			
		checkobjectname (h);
			
		/*check recalcstatus, limited set of possible values*/ {
			
			short x = (**h).objectrecalcstatus;
			
			if ((x < neverrecalc) || (x > timerecalc))
				invalid ("\pobjectrecalcstatus is out of range");
				
			if ((x != timerecalc) && ((**h).objectrecalcperiod != 0))
				invalid ("\pobjectrecalcperiod is not 0");
			}
			
		/*check that the waste array is all zero*/ {
			
			short i;
			
			for (i = 0; i < sizeof ((**h).waste); i++)
				if ((**h).waste [i] != 0)
					invalid ("\pnon-zero waste item");
			}
			
		switch ((**h).objecttype) {
			
			case grouptype:
				if ((**h).childobjectlist == nil)
					invalid ("\pchild list shouldn't be nil");
					
				if ((**h).childobjectlist == h)
					invalid ("\pcircular child list");
					
				validateobjectlist ((**h).childobjectlist);
								
				break;
				
			default: {
				bigstring errorstring;
				
				setstringlength (errorstring, 0);
				
				if (!calldebug (h, errorstring))
					invalid (errorstring);
				
				break;
				}
			} /*switch*/
						
		if ((**h).nextobject == h)
			invalid ("\pcircular list");
			
		h = (**h).nextobject;
		} /*while*/
	
	return (true);
	} /*validateobjectlist*/
	
	
static boolean validateruntimecard (void) {
	
	return (true);
	} /*validateruntimecard*/
	
	
static boolean validateeditedcard (void) {
	
	#ifndef macBirdRuntime /*it has no app.xxx record*/
	
		/*should really be in applet toolkit*/ {
			
			WindowPtr w = (**app.appwindow).macwindow;
			
			if ((hdlappwindow) GetWRefCon (w) != app.appwindow)
				invalid ("\papplet toolkit structure is kablooey");
			}
		
		if (iowadata != (hdlcard) (**app.appwindow).appdata)
			invalid ("\piowadata and the appwindow don't agree");
		
		if ((**iowadata).runmode)
			invalid ("\pflrunmode is wrong");
	
	#endif
	
	return (true);
	} /*validateeditedcard*/
	
	
boolean iowavalidate (boolean flruntimecard) {
	
	/*
	you can call this when you think all the iowa globals are set
	correctly. we look at everything, and try to find a problem.
	
	you tell us whether or not you think the current card is running
	or being edited.
	
	1.0b24 dmb: don't do any validating if we're not a debug build
	*/
	
	#ifdef fldebug
		
		if (iowadata == nil)
			invalid ("\piowadata == nil");
			
		validateobjectlist ((**iowadata).objectlist);
		{
		CGrafPtr	thePort;
		#if TARGET_API_MAC_CARBON == 1
		thePort = GetQDGlobalsThePort();
		#else
		thePort = quickdrawglobal (thePort);
		#endif
		
		if (thePort != (**iowadata).drawwindow)	/*dmb 1.31.96: use quickdrawglobal*/
			invalid ("\pdrawing into the wrong port");
		}
		#ifdef iowaRuntimeInApp 
			if (flruntimecard) 
				validateruntimecard ();
			else 
				validateeditedcard ();
		#else
			#ifdef iowaRuntime
				validateruntimecard ();
			#else
				validateeditedcard ();
			#endif
		#endif
	
	#endif
	
	return (true);
	} /*iowavalidate*/




