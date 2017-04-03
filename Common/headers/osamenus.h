
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

#define osamenusinclude

#ifndef __APPLEEVENTS__

	#include <AppleEvents.h>

#endif

#ifndef windowsharinginclude

	// #include <uisharing.h>
	// #include <uisinternal.h>
	
	#define msComponentType 'SHMN'
	#define msComponentSubType 0
	
	#define msInitSharedMenusCommand 0x2001 
	#define msSharedMenuHitCommand 0x2002
	#define msSharedScriptRunningCommand 0x2003
	#define msCancelSharedScriptCommand	0x2004
	#define msCheckSharedMenusCommand 0x2005
	#define msDisposeSharedMenusCommand 0x2007
	#define msIsSharedMenuCommand 0x2008
	#define msEnableSharedMenusCommand 0x2009
	#define msRunSharedMenuItemCommand 0x200A
	#define msSetScriptErrorCallbackCommand 0x200B
	#define msSetEventFilterCallbackCommand 0x200C
	#define msSetMenusInserterCallbackCommand 0x200D
	#define msSetMenusRemoverCallbackCommand 0x200E
	#define msDirtySharedMenusCommand 0x200F
	

#endif


#define __MENUSHARING__ /*so other modules can tell that we've been included*/


/*routines shared between osamenus.c and osacomponent.c*/

extern OSAError osageterror (void); /*osacomponent.c*/

extern pascal OSErr osadefaultactiveproc (long);

extern pascal OSAError osaDispose (hdlcomponentglobals, OSAID);

extern pascal OSAError osaSetActiveProc (hdlcomponentglobals, OSAActiveUPP, long);

extern pascal OSAError osaSetSendProc (hdlcomponentglobals, OSASendUPP, long);

extern pascal OSAError osaCompileExecute (hdlcomponentglobals, const AEDesc *, OSAID, long, OSAID *);

extern pascal OSAError osaDoScript (hdlcomponentglobals, const AEDesc *, OSAID, DescType, long, AEDesc *);

extern boolean osafindclienteventfilter (long, long *);


extern boolean initmenusharingcomponent (void); /*osamenus.c*/




