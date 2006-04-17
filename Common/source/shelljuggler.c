
/*	$Id$    */

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

#include "memory.h"
#include "cursor.h"
#include "launch.h"
#include "scrap.h"
#include "shell.h"
#include "shellprivate.h"
#include "langinternal.h"
#include "scripts.h"
#include "process.h"



static boolean flshellactive = true; /*under multifinder, are we in the foreground?*/




boolean shellisactive (void) {
	
	return (flshellactive);
	} /*shellisactive*/


boolean shellactivate (void) {
	
	/*
	2/10/92 dmb: added entrypoint for shellsysverbs and dialog operations
	
	1/22/93 dmb: use langpartialeventloop when a script is running
	
	2.1b12 dmb: don't return quickly if shellactive is true. we may be 
	trying to bring a client process to the front. besided, the overhead 
	of activating an already-active application is small, and this isn't 
	a performance bottleneck anyway.
	*/
	
	/*
	if (flshellactive)
		return (true);
	*/
	
	if (!activateapplication (nil)) /*bring ourselves to the front*/
		return (false);
	
	#ifdef flcomponent
	
	if (fldisableyield)
		return (true);
	
	#endif

#ifdef MACVERSION
	while (!flshellactive) { /*wait 'till we're actually in front*/
		
		if (flscriptrunning)
			langpartialeventloop (osMask | updateMask | activMask);
		else
			shellpartialeventloop (osMask | updateMask | activMask);
		}
#endif
	
	return (true);
} /*shellactivate*/


static boolean shelljugglervisit (WindowPtr w, ptrvoid refcon) {
#pragma unused (refcon)

	shellpushglobals (w);
	
	(*shellglobals.resumeroutine) (flshellactive);
	
	shellpopglobals ();
	
	return (true);
	} /*shelljugglervisit*/
	
	
void shellhandlejugglerevent (void) {
	
	/*
	7/13/90 DW: deactivate the front window when we're swapped into the
	background on a juggler event.  activate it when we're swapped back 
	into the foreground.

	2006-04-17 aradke: updated for Intel Macs using endianness-agnostic code
	*/
	
	register boolean flresume;
	long message = shellevent.message;
	
	if (((message >> 24) & 0xff) == suspendResumeMessage) { /*suspend or resume subevent*/
		
		boolean fl;
		
		flresume = ((message & resumeFlag) != 0); /*copy into register*/
		
		flshellactive = flresume; /*set global*/
		
		if (flresume) { /*force update of mouse cursor*/
		
			setcursortype (cursorisdirty); /*we don't know what state it's in*/
			
			shellforcecursoradjust (); /*make it appear as if mouse moved*/
			}
		
		fl = shellpushfrontglobals ();
		
		if (fl)
			shellactivatewindow (shellwindow, flshellactive);

		#if defined(TARGET_API_MAC_OS8) && (TARGET_API_MAC_OS8 == 1)
			/*
			2006-04-17 aradke: convertClipboardFlag is never set on Carbon
			*/
			
			if ((message & convertClipboardFlag) != 0) {
				
				if (flresume)
					; /*shellreadscrap ()*/ /*12/28/90 dmb: see comment in shellreadscrap*/
				else
					shellwritescrap (anyscraptype);
				}
		#endif
		
		if (fl)
			shellpopglobals ();
		
		/* 4.0b7 dmb: new feature, just like startup & shutdown scripts */
		if (flresume)
			scriptrunresumescripts ();
		else
			scriptrunsuspendscripts ();
		
		shellvisittypedwindows (-1, &shelljugglervisit, nil); /*send message to all open windows*/
		}
	} /*shellhandlejugglerevent*/
	
	
