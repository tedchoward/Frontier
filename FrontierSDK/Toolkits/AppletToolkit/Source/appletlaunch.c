
/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include <Processes.h>
#include "appletmemory.h"
#include "appletlaunch.h"


static Boolean appRunning (OSType appid) {
	
	/*
	return true if the server application is running. 
	*/
	
	ProcessInfoRec info;
	ProcessSerialNumber psn;
	Str255 bsname;
	FSSpec fss;
	
	info.processInfoLength = sizeof (info);
	
	info.processName = bsname; /*place to store process name*/
	
	info.processAppSpec = &fss; /*place to store process filespec*/
	
	psn.highLongOfPSN = kNoProcess;
	
	psn.lowLongOfPSN = kNoProcess;
	
	while (GetNextProcess (&psn) == noErr) {
		
	 	info.processInfoLength = sizeof (ProcessInfoRec);
	 	
		if (GetProcessInformation (&psn, &info) != noErr)
			continue; /*keep going -- ignore error*/
		
		if (info.processSignature == appid)
			return (true);
		} /*while*/
	
	return (false); /*loop completed, no server*/
	} /*appRunning*/


static Boolean hasdesktopmanager (short vnum) {
	
	GetVolParmsInfoBuffer volparms;
	HParamBlockRec pb;
	
	pb.volumeParam.ioVRefNum = vnum;
	
	pb.volumeParam.ioNamePtr = nil;
	
	pb.ioParam.ioBuffer = (Ptr) &volparms;
	
	pb.ioParam.ioReqCount = sizeof (volparms);
	
	if (PBHGetVolParmsSync (&pb) != noErr)
		return (false);
	
	return ((volparms.vMAttrib & (1 << bHasDesktopMgr)) != 0);
	} /*hasdesktopmanager*/


static Boolean findApp (OSType appid, FSSpec *fs) {
	
	/*
	find the app whose creator id is appid. return true if we found it, 
	false otherwise.
	*/
	
	DTPBRec dt;
	ParamBlockRec pb;
	Str255 appfname;
	
	clearbytes (&pb, (long) sizeof (pb));
	
	while (true) {
		
		++pb.volumeParam.ioVolIndex;
		
		if (PBGetVInfoSync (&pb) != noErr)
			return (false);
		
		if (!hasdesktopmanager (pb.volumeParam.ioVRefNum))
			continue;
		
		dt.ioNamePtr = NULL;
		
		dt.ioVRefNum = pb.volumeParam.ioVRefNum;
		
		if (PBDTGetPath (&dt) != noErr)
			return (false);
		
		dt.ioNamePtr = (StringPtr) &appfname;
		
		dt.ioIndex = 0;
		
		dt.ioFileCreator = appid;
		
		if (PBDTGetAPPLSync (&dt) == noErr) {
		
			if (FSMakeFSSpec (pb.volumeParam.ioVRefNum, dt.ioAPPLParID, appfname, fs) == noErr)
				return (true);
			}
		} /*while*/
		
	return (false);
	} /*findApp*/


static Boolean launchApp (FSSpec *fs) {
	
	LaunchParamBlockRec pb;
	
	clearbytes (&pb, (long) sizeof (pb));
	
	pb.launchAppSpec = fs;
	
	pb.launchBlockID = extendedBlock;
	
	pb.launchEPBLength = extendedBlockLen;
	
	pb.launchControlFlags = launchContinue | launchNoFileFlags/* | launchDontSwitch*/;
	
	return (LaunchApplication (&pb) == noErr);
	} /*launchApp*/


Boolean launchServer (OSType serverid) {
	
	/*
	if the application whose creator id is serverid is running, return true.
	
	if not, we look for the application and try to launch it. we wait until it's 
	actually running and ready to receive Apple Events.
	*/
	
	FSSpec fs;
	
	if (appRunning (serverid))
		return (true);
		
	if (!findApp (serverid, &fs))
		return (false);
		
	if (!launchApp (&fs))
		return (false);
		
	while (!appRunning (serverid)) {
		
		EventRecord ev;
		
		EventAvail (everyEvent, &ev);
		} /*while*/
		
	return (true);
	} /*launchServer*/
	
	
