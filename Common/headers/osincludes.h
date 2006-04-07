
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
	2004-10-23 aradke: From now on, add all system includes to this file.
		It is included from frontier.h, which is included in all source files.
*/

#ifndef __osincludes_h__
#define __osincludes_h__


#ifdef WIN95VERSION

	#define WIN32_LEAN_AND_MEAN 1

	#include <windows.h>

	#include <commdlg.h>
	#include <commctrl.h>
	#include <windowsx.h>
	#include <objbase.h>
	#include <shlobj.h>
	#include <exdisp.h>
	#include <Shlwapi.h>
	#include <Shellapi.h>
	#include <tlhelp32.h>
	#include <malloc.h>
	#include <crtdbg.h>
	#include <mmsystem.h>
	#include <winsock2.h>
	#include <mswsock.h>

#endif /* WIN95VERSION */


#ifdef MACVERSION

	#define OTDEBUG	1	/* must define before including OpenTransport headers */

	#if TARGET_API_MAC_CARBON

		#ifdef FRONTIER_FRAMEWORK_INCLUDES /* building for Mach-O with framework-style includes */

			#ifdef FRONTIER_FLAT_HEADERS /* building for Mach-O with flat header-style includes */

				#include <Carbon.h>
				#include <ApplicationServices.h>
				#include <Movies.h>

			#else
			
			#include <Carbon/Carbon.h>
			#include <ApplicationServices/ApplicationServices.h>
			#include <QuickTime/Movies.h>

			#endif

			#define	ELASTERRNO	ELAST

			/* cribbed from pre-OSX StandardFile.h */
			struct StandardFileReply {
			  Boolean             sfGood;
			  Boolean             sfReplacing;
			  OSType              sfType;
			  FSSpec              sfFile;
			  ScriptCode          sfScript;
			  short               sfFlags;
			  Boolean             sfIsFolder;
			  Boolean             sfIsVolume;
			  long                sfReserved1;
			  short               sfReserved2;
			};
			typedef struct StandardFileReply        StandardFileReply;

		#else /*CFM*/

			#define ACCESSOR_CALLS_ARE_FUNCTIONS	1
			#define OPAQUE_TOOLBOX_STRUCTS			1
	
			#define OTUNIXERRORS	1
	
			#define OLDROUTINENAMES		0
			#define OLDROUTINELOCATIONS	0
			#define CGLUESUPPORTED		0
	
			#define PtoCstr		p2cstr
			#define CtoPstr		c2pstr
			#define PtoCString	p2cstr
			#define CtoPString	c2pstr
	
			#define TRUE		true
			#define FALSE		false
	
			#include <Carbon.h>
			#include <CFBundle.h>
			#include <MacHelp.h>
			#include <Movies.h>
	
		#endif
	
		/* cribbed from pre-OSX AppleTalk.h -- for TargetID */
		struct EntityName {
		  Str32Field          objStr;
		  Str32Field          typeStr;
		  Str32Field          zoneStr;
		};
		typedef struct EntityName               EntityName;
	
		/* cribbed from pre-OSX PPCToolbox.h -- for TargetID */
		typedef SInt16 PPCLocationKind;
		typedef SInt16 PPCPortKinds;
		enum {
		  ppcByCreatorAndType           = 1,    /* Port type is specified as colloquial Mac creator and type */
		  ppcByString                   = 2     /* Port type is in pascal string format */
		};
		typedef SInt16 PPCXTIAddressType;
		struct PPCXTIAddress {
		  PPCXTIAddressType   fAddressType;           /* A constant specifying what kind of network address this is */
		  UInt8               fAddress[96];           /* The contents of the network address (variable length, NULL terminated). */
		};
		typedef struct PPCXTIAddress            PPCXTIAddress;
		typedef PPCXTIAddress *                 PPCXTIAddressPtr;
		struct PPCAddrRec {
		  UInt8               Reserved[3];            /* reserved - must be initialize to 0          */
		  UInt8               xtiAddrLen;             /* size of the xtiAddr field             */
		  PPCXTIAddress       xtiAddr;                /* the transport-independent network address   */
		};
		typedef struct PPCAddrRec               PPCAddrRec;
		typedef PPCAddrRec *                    PPCAddrRecPtr;
		struct LocationNameRec {
		  PPCLocationKind     locationKindSelector;   /* which variant */
		  union {
		    EntityName          nbpEntity;            /* NBP name entity                   */
		    Str32               nbpType;              /* just the NBP type string, for PPCOpen  */
		    PPCAddrRec          xtiType;              /* an XTI-type network address record     */
		  }                       u;
		};
		typedef struct LocationNameRec          LocationNameRec;
		typedef LocationNameRec *               LocationNamePtr;
		struct PPCPortRec {
		  ScriptCode          nameScript;             /* script of name */
		  Str32Field          name;                   /* name of port as seen in browser */
		  PPCPortKinds        portKindSelector;       /* which variant */
		  union {
		    Str32               portTypeStr;          /* pascal type string */
		    struct {
		      OSType              portCreator;
		      OSType              portType;
		    }                       port;
		  }                       u;
		};
		typedef struct PPCPortRec               PPCPortRec;
		typedef PPCPortRec *                    PPCPortPtr;
		struct PortInfoRec {
		  SInt8               filler1;
		  Boolean             authRequired;
		  PPCPortRec          name;
		};
		typedef struct PortInfoRec              PortInfoRec;
		typedef PortInfoRec *                   PortInfoPtr;
	
		/* cribbed from pre-OSX EPPC.h -- for TargetID */
		struct TargetID {
		  long                sessionID;
		  PPCPortRec          name;
		  LocationNameRec     location;
		  PPCPortRec          recvrName;
		};
		typedef struct TargetID                 TargetID;
		typedef TargetID *                      TargetIDPtr;
		typedef TargetIDPtr *                   TargetIDHandle;
		typedef TargetIDHandle                  TargetIDHdl;
		typedef TargetID                        SenderID;
		typedef SenderID *                      SenderIDPtr;
	
	#endif /*TARGET_API_MAC_CARBON*/
	
	#if TARGET_API_MAC_OS8
	
		#define ACCESSOR_CALLS_ARE_FUNCTIONS	0
		#define OPAQUE_TOOLBOX_STRUCTS			0

		#define OTUNIXERRORS	1
	
		#define OLDROUTINENAMES		0
		#define OLDROUTINELOCATIONS	0
		#define CGLUESUPPORTED		0
	
		#define PtoCstr		p2cstr
		#define CtoPstr		c2pstr
		#define PtoCString	p2cstr
		#define CtoPString	c2pstr
	
		#define TRUE		true
		#define FALSE		false
	
		#include <AEObjects.h>
		#include <AEPackObject.h>
		#include <AERegistry.h>
		#include <AEUserTermTypes.h>
		#include <Aliases.h>
		#include <AppleEvents.h>
		#include <AppleScript.h>
		#include <ASRegistry.h>
		#include <Balloons.h>
		#include <CodeFragments.h>
		#include <ColorPicker.h>
		#include <Components.h>
		#include <ConditionalMacros.h>
		#include <Controls.h>
		//Code change by Timothy Paustian Saturday, April 29, 2000 9:29:50 PM
		//Changed to allow Use of Universal Headers 3.3.1
		#include <ControlDefinitions.h>
		#include <Devices.h>
		#include <Dialogs.h>
		#include <DiskInit.h>
		#include <EPPC.h>
		#include <Events.h>
		#include <Files.h>
		#include <FileTypesAndCreators.h>
		#include <FixMath.h>
		#include <Folders.h>
		#include <Fonts.h>
		#include <fp.h>
		#include <Gestalt.h>
		#include <Icons.h>
		#include <Lists.h>
		#include <LowMem.h>
		#include <MacErrors.h>
		#include <MacTypes.h>
		#include <MacWindows.h>
		#include <Math64.h>
		#include <Memory.h>
		#include <Menus.h>
		#include <MixedMode.h>
		#include <Movies.h>
		#include <Navigation.h>
		#include <Notification.h>
		#include <OpenTransport.h>
		#include <OpenTptInternet.h>
		#include <OSA.h>
		#include <OSAComp.h>
		#include <OSUtils.h>
		#include <Packages.h>
		#include <Palettes.h>
		#include <PLStringFuncs.h>
		#include <PPCToolbox.h>
		#include <Printing.h>
		#include <Processes.h>
		#include <QDOffscreen.h>
		#include <Quickdraw.h>
		#include <Resources.h>
		#include <Scrap.h>
		//Code change by Timothy Paustian Sunday, May 7, 2000 8:37:27 PM
		//Needed for compilation with UH 3.4
		#include <Script.h>
		#include <SegLoad.h>
		#include <Sound.h> 
		#include <StandardFile.h>
		#include <Strings.h>
		#include <TextEdit.h>
		#include <TextEncodingConverter.h>
		#include <TextUtils.h>
		#include <Threads.h>
		#include <ToolUtils.h>
		#include <Traps.h>
		#include <UnicodeConverter.h>

	#endif /*TARGET_API_MAC_OS8*/

	#define topLeft(r)	(((Point *) &(r))[0])
	#define botRight(r)	(((Point *) &(r))[1])

#endif /*MACVERSION*/

/* common standard C headers */

#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

/*
	2006-04-07 aradke: determine native byte order.
		checking whether WIN32 is defined is rather crude,
		but I haven't found a better way yet.
*/

#if defined(__LITTLE_ENDIAN__) || defined(WIN32)
	#define PACKFLIPPED	1	/* enable little endian / big endian conversion for database file functions */
#elif defined(__BIG_ENDIAN__)
	#undef PACKFLIPPED		/* big endian is native database format, no need to convert */
#else
	#error Failed to determine native byte order of target architecture, update osincludes.h
#endif


#endif /* __osincludes_h__ */
