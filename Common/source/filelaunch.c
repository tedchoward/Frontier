
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

#include "file.h"
#include "filesystem7.h"
#include "ops.h"



#define kAEFinderEvents 'FNDR'
#define kAEShowClipboard 'shcl'
#define kAEPutAway 'sput'
#define keyAEFinderSelection 'fsel'
#define kAEDrag 'drag'
#define keyAEFinderMisc 'fmsc'
#define kAEEmptyTrash 'empt'
#define kAEOpenSelection 'sope'
#define kAEPrintSelection 'spri'

#if TARGET_API_MAC_CARBON == 1
#else
static ProcessSerialNumber FinderNumber;
//Code change by Timothy Paustian Wednesday, June 21, 2000 4:04:43 PM
//in Carbon you have to use high level apple events. Since what we are trying 
//to do is launch an application, lets try that.
static OSErr GetFinderPort(void) {
		
	IPCListPortsPBRec ipc;
	PPCPortRec port;
	LocationNameRec where;
	short i;
	short err;
	long first4;
	PortInfoRec FinderPort;

	if (FinderNumber.lowLongOfPSN || FinderNumber.highLongOfPSN) {
		err = GetPortNameFromProcessSerialNumber(&FinderPort.name, &FinderNumber);
		if (err == noErr) return noErr;
	};

	for (i = 0; ; i++) {
		ipc.ioCompletion = 0;
		ipc.startIndex = i;
		ipc.requestCount = 1;
		ipc.portName = &port;
			port.nameScript = 0;	
			port.name[0] = 1;
			port.name[1] = '=';
			port.portKindSelector = ppcByString;
			port.u.portTypeStr[0] = 1;
			port.u.portTypeStr[1] = '=';
		ipc.locationName = &where;
			where.locationKindSelector = ppcNoLocation;
		ipc.bufferPtr = &FinderPort;
		
		err = IPCListPortsSync(&ipc);
		if (err != noErr) return err;
		if (ipc.actualCount != 1) return -1;
		BlockMove(&FinderPort.name.u.portTypeStr[1], &first4, 4);
		if (first4 != 'MACS') continue;
		GetProcessSerialNumberFromPortName(&FinderPort.name, &FinderNumber);
		return 0;
	};
		
}
#endif

/*
the MFDispatch calls are only needed if you want to make the request
anonymously.  Normally you don't have to do this.  I do it because I
don't want the Finder sending replies back to the frontmost app.
*/

pascal short MFDispatch(short number) ONEWORDINLINE (0xA88F);

short System7Open (FSSpec fs2) {

	/*
	3/12/92 dmb: commented out MFDispatch calls, seeking A/UX compatibility

	6/1/92 dmb: added return (err) at end of function

	8/20/92 dmb: special case for a/ux
	*/

	#if TARGET_API_MAC_CARBON == 1
	//Code change by Timothy Paustian Wednesday, June 21, 2000 4:06:47 PM
	//Maybe I'm nuts, but I think I can do this very easily.
	//2/25/2001 Tim P No I can't this needs to be an apple event
	//I need to send an apple event to have the thing opened
	//The below code works. It is a total rip off of the example code in apples
	//Finder launch example. http://developer.apple.com/samplecode/Sample_Code/Interapplication_Comm/FinderLaunch.htm
	//I have pretty much left it as is. We could clean it up by not creating a targetList, But
	//I am leaving the ability in there in case it's wante later
	OSErr err;
	AppleEvent theAEvent, theReply;
	AEAddressDesc fndrAddress;
	AEDescList targetListDesc;
	OSType fndrCreator;
	Boolean wasChanged;
	AliasHandle targetAlias;
	long index;
	long nTargets = 1;
	FSSpec	targets[1];
	FSSpec	*targetList;
	targets[0] = fs2;
	targetList = targets;
		/* verify parameters */
	if ((nTargets == 0) || (targetList == NULL)) return paramErr;

		/* set up locals  */
	AECreateDesc(typeNull, NULL, 0, &theAEvent);
	AECreateDesc(typeNull, NULL, 0, &fndrAddress);
	AECreateDesc(typeNull, NULL, 0, &theReply);
	AECreateDesc(typeNull, NULL, 0, &targetListDesc);
	targetAlias = NULL;
	fndrCreator = 'MACS';

		/* create an open documents event targeting the finder */
	err = AECreateDesc(typeApplSignature, (Ptr) &fndrCreator,
		sizeof(fndrCreator), &fndrAddress);
	if (err != noErr) goto bail;
	err = AECreateAppleEvent(kCoreEventClass, kAEOpenDocuments,
		&fndrAddress, kAutoGenerateReturnID,
		kAnyTransactionID, &theAEvent);
	if (err != noErr) goto bail;

		/* create the list of files to open */
	err = AECreateList(NULL, 0, false, &targetListDesc);
	if (err != noErr) goto bail;
	for ( index=0; index < nTargets; index++) {
		if (targetAlias == NULL)
			err = NewAlias(NULL, (targetList + index), &targetAlias);
		else err = UpdateAlias(NULL, (targetList + index), targetAlias, &wasChanged);
		if (err != noErr) goto bail;
		HLock((Handle) targetAlias);
		err = AEPutPtr(&targetListDesc, (index + 1), typeAlias, *targetAlias, GetHandleSize((Handle) targetAlias));
		HUnlock((Handle) targetAlias);
		if (err != noErr) goto bail;
	}

		/* add the file list to the apple event */
	err = AEPutParamDesc(&theAEvent, keyDirectObject, &targetListDesc);
	if (err != noErr) goto bail;

		/* send the event to the Finder */
	err = AESend(&theAEvent, &theReply, kAENoReply,
		kAENormalPriority, kAEDefaultTimeout, NULL, NULL);

		/* clean up and leave */
bail:
	if (targetAlias != NULL) DisposeHandle((Handle) targetAlias);
	AEDisposeDesc(&targetListDesc);
	AEDisposeDesc(&theAEvent);
	AEDisposeDesc(&fndrAddress);
	AEDisposeDesc(&theReply);
	return err;
	
	#else
			
	static EventRecord er;
	short err;
	AliasHandle a1 = 0, a2 = 0;
	long hs1, hs2;
	FSSpec fs1;
	Handle message;
	CInfoPBRec	pb;
	long version;
	
	err = GetFinderPort();
	
	if (err != noErr)
		return err;
	
	/*
	err = FSMakeFSSpec(vRefNum, dirID, name, &fs2);			
	if (err != noErr)
		return err;
	*/
	
	/* get some catalog information, which we will use later. */
	pb.dirInfo.ioCompletion=0;
	pb.dirInfo.ioNamePtr = fs2.name;
	pb.dirInfo.ioVRefNum = fs2.vRefNum;
	pb.dirInfo.ioFDirIndex = 0;
	pb.dirInfo.ioDrDirID = fs2.parID;
	
	err = PBGetCatInfoSync(&pb);
	
	if (err != noErr)
		return err;
	
	err = NewAliasMinimal(&fs2, &a2);
	
	if (err != noErr)
		return err;
	
	fs1 = fs2;
	
	if (gestalt (gestaltAUXVersion, &version)) { /*running under A/UX; cover bugs*/
		
		if (fs1.parID == 1) { /*it's a volume*/
			
			fs1.vRefNum = -1; /*a/ux wants to see the root as the parent*/
			}
		else { /*not a volume: path parsing works better under a/ux*/
			
			bigstring path;
			
			if (!filespectopath (&fs1, path))
				return (false);
			
			folderfrompath (path, path);
			
			if (!pathtofilespec (path, &fs1))
				return (false);
			
			goto gotfs1;
			}
		}
	else {
		
		if (fs1.parID == 1) { /*it's a volume*/
			
			err = FindFolder(fs1.vRefNum, kDesktopFolderType, false, &fs1.vRefNum, &fs1.parID);
			
			if (err != noErr)
				return err;
			}
		}
	
	err = FSMakeFSSpec(fs1.vRefNum, fs1.parID, "\p", &fs1);
	
	if (err != noErr)
		return err;
	
	gotfs1:
	
	err = NewAliasMinimal(&fs1, &a1);
	
	if (err != noErr)
		return err;
	
	hs1 = GetHandleSize((Handle) a1);
	hs2 = GetHandleSize((Handle) a2);
	
	MoveHHi((Handle) a1); HLock((Handle) a1);
	MoveHHi((Handle) a2); HLock((Handle) a2);
	
	er.what = kHighLevelEvent;				/* PHLE fills this in, but we do it just to be clear. */
	er.message = kAEFinderEvents;			/* this is the event class */
	*(long *)&er.where = kAEOpenSelection;	/* this is the event type - see Card 131 */
	er.when = TickCount();					/* this also gets filled in, but... */
	er.modifiers = 0;						/* doesn't matter */
	
	message = NewHandle (24 + hs1 + 28 + hs2);
	
	/*
	normally you'd use Apple Events manager calls to pack up a message and
	send it.  I do things this way only because I can't be sure the frontmost
	app has initialized that manager.
	*/
	
	if (message) {
		register long *filler = (void *)*message;
		
		*filler++ = 'aevt';
		*filler++ = 0x00010001;
		*filler++ = ';;;;';
		*filler++ = '----';
		*filler++ = 'alis';
		*filler++ = hs1;
		BlockMove(*a1, filler, hs1); filler = (long *)(hs1 + (long) filler);
		*filler++ = 'fsel';
		*filler++ = 'list';
		*filler++ = hs2 + 16;
		*filler++ = 1;
		*filler++ = 0;
		*filler++ = 'alis';
		*filler++ = hs2;
		BlockMove(*a2, filler, hs2);
		};
	
	DisposeHandle((Handle) a1);
	DisposeHandle((Handle) a2);
	
	if (!message)
		return (memFullErr);
	
	/*
	MFDispatch(0x40);
	*/
	HLock(message);
	
	err = PostHighLevelEvent(&er, &FinderNumber, 237,
	                         *message, GetHandleSize(message),
	                         receiverIDisPSN);
	
	/*
	MFDispatch(0x41);
	*/
	
	if ((pb.dirInfo.ioFlAttrib & ioDirMask) || pb.hFileInfo.ioFlFndrInfo.fdType == 'cdev') {
		SetFrontProcess(&FinderNumber);
		};
	
	DisposeHandle(message);
	#endif
		
	
	return (err);
	} /*System7Open*/
	
