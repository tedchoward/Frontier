
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

/*
services.c -- be a services client on OS X.

Apple docs URL on setting up a Carbon app to use Services:
http://developer.apple.com/techpubs/macosx/Carbon/HumanInterfaceToolbox/MenuManager/appservices/intro/index.html

7.1b42 PBS 12/14/01
*/ 

#include "frontier.h"
#include "standard.h"

#include "menu.h"
#include "strings.h"
#include "cancoon.h"
#include "launch.h"
#include "tablestructure.h"
#include "popup.h"
#include "meprograms.h"
#include "dockmenu.h"
#include "opinternal.h"
#include "services.h"


/*These aren't in the Universal Interfaces included with CW 7.
They'll probably be in the next version, so be prepared
to undefine them.

#define kEventServiceCopy 1
#define kEventServicePaste 2
#define kEventServiceGetTypes 3

#define kEventClassService 'serv'

enum {
  kEventParamScrapRef           = 'scrp',	//    typeScrapRef
  kEventParamServiceCopyTypes   = 'svsd',	//    typeCFMutableArrayRef
  kEventParamServicePasteTypes  = 'svpt',	//    typeCFMutableArrayRef
  kEventParamServiceMessageName = 'svmg',	//    typeCFStringRef
  kEventParamServiceUserData    = 'svud',	//    typeCFStringRef
  typeScrapRef                  = 'scrp',	//    ScrapRef
  typeCFMutableArrayRef         = 'cfma'	//    CFMutableArrayRef
};

extern CFStringRef CreateTypeStringWithOSType (OSType inType);
*/


/*Prototypes*/

pascal OSStatus serviceshandlercopy (EventHandlerCallRef nextHandler, EventRef inEvent, void* userData);
pascal OSStatus serviceshandlerpaste (EventHandlerCallRef nextHandler, EventRef inEvent, void* userData);
pascal OSStatus serviceshandlergettypes (EventHandlerCallRef nextHandler, EventRef inEvent, void* userData);
  
static const OSType servicesdatatypes [] = {'TEXT'}; /*support TEXT only*/


/*Functions*/


pascal OSStatus serviceshandlercopy (EventHandlerCallRef nextHandler, EventRef inEvent, void* userData) {
#pragma unused(userData, nextHandler)

	/*
	Copy the current selection to the Services-specific scrap.
	
	2002-10-13 AR: Removed variable declarations for count and ix
	to eliminate a compiler warning about unused variables.
	*/
	
	ScrapRef servicesscrap;
	Handle hscrap;
	boolean flconverted = false;
	long bytecount;

	
	if (shellwindowinfo == nil)
		return (eventNotHandledErr);
	
	if (!newclearhandle (256, &hscrap))
		return (eventNotHandledErr);
		
	shellpushwindowglobals (shellwindowinfo);
	
	(*shellglobals.copyroutine) ();
	
	shellconvertscrap ('TEXT', &hscrap, &flconverted); /*Okay, this is just 'TEXT'-specific right here.*/
	
	if (!flconverted) {
		
		disposehandle (hscrap);
		
		return (eventNotHandledErr);
		} /*if*/
	
	shellpopglobals ();
	
	GetEventParameter (inEvent, kEventParamScrapRef, typeScrapRef, nil, sizeof (ScrapRef), nil, &servicesscrap);
	
    bytecount = gethandlesize (hscrap);
	
	lockhandle (hscrap);
	
	PutScrapFlavor (servicesscrap, 'TEXT', 0, bytecount, (*hscrap));
	
	unlockhandle (hscrap);
	
	disposehandle (hscrap);
	
	return (noErr);
	} /*serviceshandlercopy*/


pascal OSStatus serviceshandlerpaste (EventHandlerCallRef nextHandler, EventRef inEvent, void* userData) {
#pragma unused(userData, nextHandler)

	ScrapRef servicesscrap, currentscrap;
	long bytecount;
	OSErr err;
	Handle hscrap;

	if (shellwindowinfo == nil)
		return (eventNotHandledErr);

	GetEventParameter (inEvent, kEventParamScrapRef, typeScrapRef, nil, sizeof (ScrapRef), nil, &servicesscrap);
	
	ClearCurrentScrap ();
	
	GetCurrentScrap (&currentscrap);

	err = GetScrapFlavorSize (servicesscrap, 'TEXT', &bytecount);
	
	if (err != noErr)
		return (eventNotHandledErr);
	
	if (!newclearhandle (bytecount, &hscrap))
		return (eventNotHandledErr);

	lockhandle (hscrap);
	
	err = GetScrapFlavorData (servicesscrap, 'TEXT', &bytecount, *hscrap);
	
	unlockhandle (hscrap);
	
	if (err != noErr) {
	
		disposehandle (hscrap);
		
		return (eventNotHandledErr);
		} /*if*/
	
	lockhandle (hscrap);
	
    PutScrapFlavor (currentscrap, 'TEXT', 0, bytecount, *hscrap);
 
 	unlockhandle (hscrap);
 	
 	disposehandle (hscrap);
 	
 	shellpushwindowglobals (shellwindowinfo);

 	(*shellglobals.pasteroutine) ();
 	
 	shellpopglobals ();
 
	return (noErr);
	} /*serviceshandlerpaste*/


pascal OSStatus serviceshandlergettypes (EventHandlerCallRef nextHandler, EventRef inEvent, void* userData) {
#pragma unused(userData, nextHandler)

	CFMutableArrayRef copytypes, pastetypes;
	short ix, count;
	
	GetEventParameter (
			inEvent,
			kEventParamServiceCopyTypes,
			typeCFMutableArrayRef,
			NULL,
			sizeof (CFMutableArrayRef),
			NULL,
			&copytypes);

	GetEventParameter (
			inEvent,
			kEventParamServicePasteTypes,
			typeCFMutableArrayRef,
			NULL,
			sizeof (CFMutableArrayRef),
			NULL,
			&pastetypes);

	count = sizeof (servicesdatatypes) / sizeof (OSType);

	/*Place our data types in the copytypes and pastetypes arrays. This code is generalized
	so it's possible to support more than just 'TEXT' -- see servicesdatatype array at
	the top of this file.*/
	
	for (ix = 0; ix < count; ix++) {

		CFStringRef type = CreateTypeStringWithOSType (servicesdatatypes [ix]);
		
	    if (type) {

			CFArrayAppendValue (copytypes, type);
				
			CFArrayAppendValue (pastetypes, type);

			CFRelease (type);
			} /*if*/
		} /*for*/

	return (noErr);
	} /*serviceshandlergettypes*/


static void servicesinstallhandlers (void) {
	
	/*
	Install the Services Carbon Events handler.
	Return values aren't checked, because if this doesn't work,
	it's not fatal.
	
	These are all installed as application-level handlers.
	
	I could have done one handler that does a switch to see what the event
	type is, but I prefer multiple handlers, it feels cleaner.
	*/
	
	const EventTypeSpec gettypeevent = {kEventClassService, kEventServiceGetTypes};
	const EventTypeSpec copyevent = {kEventClassService, kEventServiceCopy};
	const EventTypeSpec pasteevent = {kEventClassService, kEventServicePaste};
	
	InstallApplicationEventHandler (NewEventHandlerUPP (serviceshandlergettypes), 1, &gettypeevent, 0, NULL);

	InstallApplicationEventHandler (NewEventHandlerUPP (serviceshandlercopy), 1, &copyevent, 0, NULL);
	
	InstallApplicationEventHandler (NewEventHandlerUPP (serviceshandlerpaste), 1, &pasteevent, 0, NULL);
	} /*servicesinstallhandlers*/


void initservices (void) {

	/*
	Called from shell.c.
	*/
	
	servicesinstallhandlers ();
	} /*initservices*/