
/*	$Id$    */

/*
 *	MacHeaders.c
 *
 *	Script to generate the 'MacHeaders<xxx>' precompiled header for Metrowerks C/C++.
 *  Copyright © 1993 metrowerks inc.  All rights reserved.
 */

/*
 *	Required for c-style toolbox glue function: c2pstr and p2cstr
 *	the inverse operation (pointers_in_A0) is performed at the end ...
 */

#if !(powerc || __CFM68K__)
 #pragma d0_pointers on
#endif


//Code change by Timothy Paustian Friday, April 28, 2000 9:40:09 PM
//For carbon support
#undef ACCESSOR_CALLS_ARE_FUNCTIONS
#define ACCESSOR_CALLS_ARE_FUNCTIONS 1
#undef OPAQUE_TOOLBOX_STRUCTS
#define OPAQUE_TOOLBOX_STRUCTS 1
//Code change by Timothy Paustian Saturday, June 3, 2000 10:02:03 PM
//Changed to Opaque call for Carbon
//nows its time to start the final shift to carbon.
//When this flag is set you have to link against the carbonLib.
#define TARGET_API_MAC_CARBON 1

/*	To allow the use of ToolBox calls which have now become obsolete on PowerPC, but
 *	which are still needed for System 6 applications, we need to #define OBSOLETE.  If
 *	your application will never use these calls then you can comment out this #define.
 *	NB: This is only for 68K ...
 */

#define		MSL_USE_PRECOMPILED_HEADERS 0
#define		OLDROUTINENAMES 	0
#define 	OLDROUTINELOCATIONS 0
#define		CGLUESUPPORTED 0

//#if !defined(powerc) && !defined(OBSOLETE)
// #define OBSOLETE	1
//#endif

/*
 *	Metrowerks-specific definitions
 *
 *	These definitions are commonly used but not in Apple's headers. We define
 *	them in our precompiled header so we can use the Apple headers without modification.
 */

#define PtoCstr		p2cstr
#define CtoPstr		c2pstr
#define PtoCString	p2cstr
#define CtoPString	c2pstr

#define topLeft(r)	(((Point *) &(r))[0])
#define botRight(r)	(((Point *) &(r))[1])

#define TRUE		true
#define FALSE		false

#ifndef powerc
 #include <MixedMode.h>
 long GetCurrentA5(void)
  ONEWORDINLINE(0x200D);
#endif

/*
 *	Apple #include files
 *
 *	Uncomment any additional #includes you want to add to MacHeaders.
 */

#pragma syspath_once on

//	#include <ADSP.h>
	#include <AEObjects.h>
	#include <AEPackObject.h>
	#include <AERegistry.h>
	#include <AEUserTermTypes.h>
//	#include <AIFF.h>
	#include <Aliases.h>
	#include <AppleEvents.h>
//	#include <AppleGuide.h>
	#include <AppleScript.h>
//	#include <AppleTalk.h>
//	#include <ASDebugging.h>
//	#include <ASRegistry.h>
//	#include <Balloons.h>
//	#include <CMApplication.h>
//	#include <CMComponent.h>
//	#include <CodeFragments.h>
	#include <ColorPicker.h>
//	#include <CommResources.h>
//	#include <Components.h>
	#include <ConditionalMacros.h>
//	#include <Connections.h>
//	#include <ConnectionTools.h>
	#include <Controls.h>
	//Code change by Timothy Paustian Saturday, April 29, 2000 9:29:50 PM
	//Changed to allow Use of Universal Headers 3.3.1
	#include <ControlDefinitions.h>
//	#include <ControlStrip.h>
//	#include <CRMSerialDevices.h>
//	#include <CTBUtilities.h>
//	#include <CursorCtl.h>
//	#include <CursorDevices.h>
//	#include <DatabaseAccess.h>
//	#include <DeskBus.h>
	#include <Devices.h>
	#include <Dialogs.h>
//	#include <Dictionary.h>
//	#include <DisAsmLookup.h>
//	#include <Disassembler.h>
	#include <DiskInit.h>
//	#include <Disks.h>
//	#include <Displays.h>
//	#include <Drag.h>
//	#include <Editions.h>
//	#include <ENET.h>
	#include <EPPC.h>
//	#include <ErrMgr.h>
	#include <Errors.h>
	#include <Events.h>
//	#include <fenv.h>
	#include <Files.h>
//	#include <FileTransfers.h>
//	#include <FileTransferTools.h>
	#include <FileTypesAndCreators.h>
//	#include <Finder.h>
	#include <FixMath.h>
	#include <Folders.h>
	#include <Fonts.h>
//	#include <fp.h>
//	#include <FragLoad.h>
//	#include <FSM.h>
	#include <Gestalt.h>
//	#include <HyperXCmd.h>
	#include <Icons.h>
//	#include <ImageCodec.h>
//	#include <ImageCompression.h>
//	#include <IntlResources.h>
//	#include <Language.h>
	#include <Lists.h>
	#include <LowMem.h>
//	#include <MachineExceptions.h>
//	#include <MacTCP.h>
//	#include <MediaHandlers.h>
	#include <Memory.h>
	#include <Menus.h>
//	#include <MIDI.h>
	#include <MixedMode.h>
//	#include <Movies.h>
//	#include <MoviesFormat.h>
//	#include <Notification.h>
//	#include <OSA.h>
//	#include <OSAComp.h>
//	#include <OSAGeneric.h>
	#include <OSUtils.h>
	#include <Packages.h>
//	#include <Palettes.h>
//	#include <Picker.h>
//	#include <PictUtil.h>
//	#include <PictUtils.h>
	#include <PLStringFuncs.h>
//	#include <Power.h>
//	#include <PPCToolbox.h>
	#include <Printing.h>
	#include <Processes.h>
//	#include <QDOffscreen.h>
	#include <Quickdraw.h>
//	#include <QuickdrawText.h>
//	#include <QuickTimeComponents.h>
	#include <Resources.h>
//	#include <Retrace.h>
//	#include <ROMDefs.h>
#ifndef powerc
//	#include <SANE.h>
#endif
	#include <Scrap.h>
	//Code change by Timothy Paustian Sunday, May 7, 2000 8:37:27 PM
	//Needed for compilation with UH 3.4
	#include <Script.h>
//	#include <SCSI.h>
	#include <SegLoad.h>
//	#include <Serial.h>
//	#include <ShutDown.h>
//	#include <Slots.h>
	#include <Sound.h>
//	#include <SoundComponents.h>
//	#include <SoundInput.h>
//	#include <Speech.h>
	#include <StandardFile.h>
//	#include <Start.h>
	#include <Strings.h>
//	#include <Terminals.h>
//	#include <TerminalTools.h>
	#include <TextEdit.h>
//	#include <TextServices.h>
	#include <TextUtils.h>
	#include <Threads.h>
//	#include <Timer.h>
	#include <ToolUtils.h>
//	#include <Translation.h>
//	#include <TranslationExtensions.h>
	#include <Traps.h>
//	#include <TSMTE.h>
	#include <Types.h>
//	#include <Unmangler.h>
//	#include <Video.h>
	#include <Windows.h>
//	#include <WorldScript.h>


/*
 *	required for c-style toolbox glue function: c2pstr and p2cstr
 *	(match the inverse operation at the start of the file ...
 */

#if !(powerc || __CFM68K__)
 #pragma d0_pointers reset
#endif

	#define MACVERSION 1
	#define fldebug 1
	#define flnewfeatures 1
	#define version42orgreater 1
	#define version5orgreater 1
	#define flcomponent 1
	#define isFrontier 1
	#undef dropletcomponent
	#undef fliowa
	#define threadverbs 1
	#define oplanglists 1
	#ifdef __powerc
		#define noextended 1
	#else
		#define noextended 0
	#endif
	#define macBirdRuntime 1
	#undef appRunsCards /*for Applet Toolkit, Iowa Runtime is baked in*/
	#define iowaRuntimeInApp /*iowa code knows it's in an app*/
	#define iowaRuntime /*iowa code knows it's not compiling in Card Editor*/
	#define cmdPeriodKillsCard
	#define IOAinsideApp /*all the IOA's are baked into the app*/
	#undef coderesource /*we're not running inside a code resource*/
	#include <stdlib.h>
	#include <stdio.h>
	#include <standard.h>
	#include "quickdraw.h"
	#include "dialogs.h"
	#include "threads.h"

#pragma syspath_once off
