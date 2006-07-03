
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

#define uisharinginclude


#define noMenuSharing 0x1 /*masks for uisInit*/
#define noWindowSharing 0x2


/*core routines for UI Sharing clients*/

	Boolean uisInit (ProcPtr, short, OSType, unsigned short);
	
	Boolean uisHandleEvent (EventRecord *, Boolean *);
	
	Boolean uisEdit (short);
	
	Boolean uisSharedMenuHit (short, short);
	
	Boolean uisIsSharedWindow (WindowPtr);
	
	Boolean uisCloseSharedWindow (WindowPtr);
	
	void uisCloseAllSharedWindows (void);
	
	void uisClose (void);
	

/*for apps that want to use Iowa Runtime as their dialog manager*/

	#define uisInitEvent 1000 /*constants for (*ev).what in callbacks*/
	#define uisMajorRecalcEvent 1001
	#define uisMinorRecalcEvent 1002
	#define uisButtonHitEvent 1003
	#define uisCloseEvent 1004
	#define uisRunScriptEvent 1005
	#define uisCancelEvent 1006
	#define uisSetHandleEvent 1007

	typedef void (*uisEventCallback) (EventRecord *);

	Boolean uisRunModalHandle (Handle, Boolean, Str255, short, short, uisEventCallback);

	Boolean uisRunModalResource (short, Boolean, Str255, short, short, uisEventCallback);

	Boolean uisOpenCardResource (short, Boolean, Str255, short, short, uisEventCallback);

	Boolean uisOpenHandle (Handle, Boolean, Str255, short, short, uisEventCallback);

	Boolean uisSetObjectValue (Handle, Str255, Handle);

	Boolean uisGetObjectValue (Handle, Str255, Handle *);

	Boolean uisGetObjectHandle (Handle, Str255, Handle *);

	Boolean uisRecalcObject (Handle);
	
	Boolean uisUpdate (Handle);
	
/*for a stub app whose sole purpose is to run a card*/

	Boolean uisStubStart (void);





