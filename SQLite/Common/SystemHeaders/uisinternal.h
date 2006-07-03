
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

#define uisinternalinclude


#ifndef __COMPONENTS__
	
	#include <Components.h>

#endif


#ifndef uisharinginclude

	#include <uisharing.h>
	
#endif


/*window sharing stuff -- for communication with Iowa Runtime component and others*/

	#define wsInterfaceVersion 0x00010001

	#define wsComponentType 'SHUI'
	
	#ifdef iowaRuntimeInApp
		#define wsComponentSubType 'TEST' 
	#else	
		#define wsComponentSubType 'WIND' 
	#endif
	
	#define wsStubStartCommand 1 
	#define wsRecalcObjectCommand 2
	#define wsUnusedCommand 3
	#define wsRunFromHandleCommand 4
	#define wsRunModalHandleCommand 5 
	#define wsEventHandlerCommand 6
	#define wsWindowIsCardCommand 7
	#define wsCloseWindowCommand 8
	#define wsSetObjectValueCommand 9
	#define wsGetObjectValueCommand 10
	#define wsGetObjectHandleCommand 11
	#define wsUpdateCommand 12
	#define wsEditCommand 13

	typedef struct tyWindowSharingGlobals {
		
		ComponentInstance windowserver;
	
		OSErr errorcode; /*a copy of the system error code after a ws call*/
		
		Boolean flcloseallwindows; /*true when server gets an option-click in a window's close box*/
		} tyWindowSharingGlobals;
	
	extern tyWindowSharingGlobals wsGlobals;


/*menu sharing stuff -- for communication with the Frontier 2.1 OSA component*/

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
	#define msSetIdleProcCallbackCommand 0x200C

	typedef struct tyMenuSharingGlobals { 
	
		ComponentInstance menuserver; 
		
		short idinsertafter;
		} tyMenuSharingGlobals;

	extern tyMenuSharingGlobals msGlobals;
	
