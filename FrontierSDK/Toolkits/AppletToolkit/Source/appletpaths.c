
/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include "appletpaths.h"
#include "appletmemory.h"
#include "appletstrings.h"



static boolean directorytopath (long DirID, short vnum, bigstring path) {
	
	CInfoPBRec block;
	bigstring bsdirectory;
	OSErr errcode;
	
	setemptystring (path);
	
	clearbytes (&block, longsizeof (block));
	
	block.dirInfo.ioNamePtr = bsdirectory;
	
	block.dirInfo.ioDrParID = DirID;
	
	do {
		block.dirInfo.ioVRefNum = vnum;
		
		block.dirInfo.ioFDirIndex = -1;
		
		block.dirInfo.ioDrDirID = block.dirInfo.ioDrParID;
		
		errcode = PBGetCatInfo(&block,false);
		
		if (errcode != noErr)
			return (false);
		
		if (!pushchar (':', bsdirectory))
			return (false);
			
		if (!pushstring (path, bsdirectory))
			return (false);
		
		copystring (bsdirectory, path);
		} while (block.dirInfo.ioDrDirID != fsRtDirID);
	
	return (true);
	} /*directorytopath*/


static boolean PathNameFromWD (long vnum, bigstring path) {
	
	WDPBRec block;
	
	clearbytes (&block, longsizeof (block));
	
	block.ioVRefNum = vnum;
	
	PBGetWDInfo (&block, false);
	
	return (directorytopath (block.ioWDDirID, block.ioWDVRefNum, path));
	} /*PathNameFromWD*/
	
	
boolean filegetfolder (ptrfilespec pfs, bigstring folderpath) {
	
	return (directorytopath ((*pfs).parID, (*pfs).vRefNum, folderpath));
	} /*filegetfolder*/


boolean filegetpath (ptrfilespec pfs, bigstring path) {
	
	if (!directorytopath ((*pfs).parID, (*pfs).vRefNum, path))
		return (false);
		
	pushstring ((*pfs).name, path);
	
	return (true);
	} /*filegetpath*/
	
	
