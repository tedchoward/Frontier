
/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include <GestaltEqu.h>
#include <Aliases.h>
#include <Finder.h>
#include <Folders.h>
#include "appletdefs.h"
#include "appletfiles.h"
#include "appletmemory.h"
#include "appletstrings.h"
#include "appletfilesinternal.h"
#include "appletfilealias.h"

#define usesourcefiletype /*DW 9/6/93 -- inherit file type/creator from source file*/

#define MAXCHAINS 10 /*maximum number of aliases to resolve before giving up*/

typedef struct {
	OSType		folderType;
	OSType		aliasType;
} MappingEntry;


#define kNumIconTypes 6
#define kNumFolders 9


	// Resource ID used for alias file's icon (when needed)
	
#define kCustomAliasIconID -16496


	// Forward declarations
	
Boolean FSpIsVolume(const FSSpec* fsSpec);
Boolean FSpIsFolder(const FSSpec* fsSpec, long* dirID, unsigned short* fdFlags);
Boolean CopyCustomIcons(const FSSpec* source, short sourceID,
							short destRefnum, short destID);
Boolean CopyDriverIcon(const FSSpec* volSpec, short destRefnum, short destID);
OSErr DetermineAliasInfo(const FSSpec* fsSpec, OSType* creator, OSType* fType,
							Boolean* gotCustom, FSSpec* iconSpec, short* rsrcID);



static pascal OSErr CreateAliasFile(const FSSpec* targetFile, const FSSpec* aliasFile)
{
	OSType		fileCreator, fileType;
	FSSpec		iconSpec;
	Boolean		gotCustom;
	short		rsrcID;
	short		aliasRefnum;
	AliasHandle	alias;
	FInfo		finf;
	
		// Get type/creator for alias file & custom icon file spec
	OSErr err = DetermineAliasInfo(targetFile, &fileCreator, &fileType, 
											&gotCustom, &iconSpec, &rsrcID);
	if (err != noErr) return err;
	
	#ifdef usesourcefiletype /*DW 9/6/93 -- inherit file type/creator from source file*/
		{
		FSSpec fs = *targetFile;
		tyfileinfo info;
		
		if (filegetinfo (&fs, &info)) {
			
			fileCreator = info.filecreator;
			
			fileType = info.filetype;
			}
		}
	#endif
	
		// Create the alias file (it must not exist already)
	FSpCreateResFile(aliasFile, fileCreator, fileType, 0);
	err = ResError();
	if (err != noErr) return err;

		// Create the alias handle
	err = NewAlias(aliasFile, targetFile, &alias);
	if (err != noErr || alias == NULL) return err;
	
	aliasRefnum = FSpOpenResFile(aliasFile, fsRdWrPerm);
	// should exist because it was just created
	
		// Add the alias handle to alias file
	AddResource((Handle)alias, rAliasType, 0, "\p");
	err = ResError();
	
		// Copy custom icons (if necessary)
	if (gotCustom)
		gotCustom = CopyCustomIcons(&iconSpec, rsrcID, aliasRefnum, kCustomAliasIconID);
		
	if (!gotCustom && FSpIsVolume(targetFile))	// must get icon from disk driver
		gotCustom = CopyDriverIcon(targetFile, aliasRefnum, kCustomAliasIconID);
		
	CloseResFile(aliasRefnum);

	FSpGetFInfo(aliasFile, &finf);
	
	finf.fdCreator = fileCreator;
	finf.fdType = fileType;
	finf.fdFlags |= 0x8000;		// set alias bit
	if (gotCustom)
		finf.fdFlags |= 0x0400;	// set custom icon bit
	finf.fdFlags &= (~0x0100);	// clear inited
	
	FSpSetFInfo(aliasFile, &finf);

	return noErr;
}


	// Returns true if fsSpec refers to a volume
	
static Boolean FSpIsVolume(const FSSpec* fsSpec)
{
	return fsSpec->parID == 1;
}

	// Returns true if fsSpec refers to a folder; also returns folder's dirID & finder flags
	
static Boolean FSpIsFolder(const FSSpec* fsSpec, long* dirID, unsigned short* fdFlags)
{
	CInfoPBRec pb;

	if (FSpIsVolume(fsSpec)) return false;
	
	pb.dirInfo.ioNamePtr = (StringPtr)&fsSpec->name;
	pb.dirInfo.ioVRefNum = fsSpec->vRefNum;
	pb.dirInfo.ioFDirIndex = 0;
	pb.dirInfo.ioDrDirID = fsSpec->parID;
	
	if (PBGetCatInfoSync(&pb) != noErr) return false;
	
	*dirID = pb.dirInfo.ioDrDirID;
	*fdFlags = pb.dirInfo.ioDrUsrWds.frFlags;
	
	return (pb.dirInfo.ioFlAttrib & 0x10) != 0;
}


	// Copies all icon resources from indicated source file (using indicated rsrc ID to (already
	// opened) destination file (using destID).  source->vRefNum == 0 implies system resource file
	// Returns true if an icon was added to the destination.
	// (Doesn't work if source == destination.)
	
static Boolean CopyCustomIcons(const FSSpec* source, short sourceID,
							short destRefnum, short destID)
{
	short		iconRefnum;
	char*		table = "ICN#icl4icl8ics#ics4ics8";
	OSType*		p;
	Boolean		copiedSomething = false;
	int			i;
	OSErr		err;
	
	if (source->vRefNum != 0)
		iconRefnum = FSpOpenResFile(source, fsRdPerm);
	else
		iconRefnum = 0;	// system resource file
		
	if (iconRefnum != -1) {
		for (i = 0, p = (OSType*)table; i < kNumIconTypes; i++, p++)
		{
			Handle h;
			
			UseResFile(iconRefnum);
			h = Get1Resource(*p, sourceID);
			
			if (h != NULL) {
				DetachResource(h);
				
				UseResFile(destRefnum);
				AddResource(h, *p, destID, "\p");
				err = ResError();

				if (err != noErr)
					DisposHandle(h);
				else
					copiedSomething = true;
			}
		}
		
		if (iconRefnum != 0)
			CloseResFile(iconRefnum);
	}
	
	return copiedSomething;
}


	// Copies all icon resources from indicated source file (using indicated rsrc ID to (already
	// opened) destination file (using destID).  source->vRefNum == 0 implies system resource file
	// Returns true if an icon was added to the destination.
	// (Doesn't work if source == destination.)
	
static Boolean CopyDriverIcon(const FSSpec* volSpec, short destRefnum, short destID)
{
	HParamBlockRec	pb;
	ParamBlockRec	cpb;
	OSErr			err;
	Handle			h;
	
	if (!FSpIsVolume(volSpec)) return false;
	
	pb.volumeParam.ioNamePtr = NULL;
	pb.volumeParam.ioVRefNum = volSpec->vRefNum;
	pb.volumeParam.ioVolIndex = 0;
	
	err = PBHGetVInfoSync(&pb);
	if (err != noErr) return false;
	
		// set up for Control call
	cpb.cntrlParam.ioCRefNum = pb.volumeParam.ioVDRefNum;
	cpb.cntrlParam.ioVRefNum = pb.volumeParam.ioVDrvInfo;
	
		// first try csCode 22
	cpb.cntrlParam.csCode = 22;
	err = PBControl(&cpb, false);
	
	if (err != noErr) {
			// try csCode 21;
		cpb.cntrlParam.csCode = 21;
		err = PBControlSync(&cpb);
	}
	
	if (err != noErr) return false;

	h = appnewhandle(kLargeIconSize);		// size of ICN#
	if (h == NULL) return false;
	
		// copy ICN# into handle
	BlockMove(*(Ptr*)&cpb.cntrlParam.csParam, *h, kLargeIconSize);

	UseResFile(destRefnum);
	AddResource(h, 'ICN#', destID, "\p");
	err = ResError();
	
	if (err != noErr)
		DisposHandle(h);
		
	return err == noErr;
}


	// Figures out information about an alias for the indicated file (fsSpec)
	// Returns type & creator of file; whether the target has a custom icon (and
	//		the file from which to get it, and the resource ID of those icons)
	
static OSErr DetermineAliasInfo(const FSSpec* fsSpec, OSType* creator, OSType* fType,
							Boolean* gotCustom, FSSpec* iconSpec, short* rsrcID)
{
	OSErr err;
	long dirID;
	FInfo finderStuff;
	
	*gotCustom = false;
	*rsrcID = kCustomIconResource;	// default value
	
	if (FSpIsVolume(fsSpec)) {
			// temporarily, all volumes are given the same type
			
		*creator = 'MACS';
		*fType = kContainerHardDiskAliasType; //// need more work here

		err = FSMakeFSSpec(fsSpec->vRefNum, 2, "\pIcon\015", iconSpec);
		
		if (err == noErr) // volume has a custom icon file (maybe)
			*gotCustom = true;
	}
	
	else if (FSpIsFolder(fsSpec, &dirID, &finderStuff.fdFlags)) {
			// table mapping special folder ID to alias file type
		MappingEntry table[kNumFolders] =
		{	{ kAppleMenuFolderType,			kAppleMenuFolderAliasType },
			{ kControlPanelFolderType,		kControlPanelFolderAliasType },
			{ kExtensionFolderType, 		kExtensionFolderAliasType },
			{ kPreferencesFolderType, 		kPreferencesFolderAliasType },
			{ kPrintMonitorDocsFolderType, 	kPrintMonitorDocsFolderAliasType },
			{ kWhereToEmptyTrashFolderType,	kContainerTrashAliasType },
			{ kTrashFolderType, 			kContainerTrashAliasType },
			{ kStartupFolderType, 			kStartupFolderAliasType },
			{ kSystemFolderType, 			kSystemFolderAliasType }
		};
		MappingEntry* p;
		int i;
		
			// see if original folder has a custom icon
		*gotCustom = (finderStuff.fdFlags & 0x0400) != 0;
		if (*gotCustom)
			FSMakeFSSpec(fsSpec->vRefNum, dirID, "\pIcon\015", iconSpec);
		
		*creator = 'MACS';
		*fType = kContainerFolderAliasType;			// default file type for alias file
		
				// see if the folder is one of the special ones; if so, modify the file type
		for (i=1, p = table; i<=kNumFolders; i++, p++) {
			short foundVRefnum;
			long foundDirID;
			
			err = FindFolder(fsSpec->vRefNum, p->folderType, false, &foundVRefnum, &foundDirID);
			if (err == noErr && foundDirID == dirID) {
				*fType = p->aliasType;
				break;
			}
		}
	}
	
	else {	// alias to a file
		err = FSpGetFInfo(fsSpec, &finderStuff);
		if (err != noErr) return noErr;
		
		*creator = finderStuff.fdCreator;
		*fType = finderStuff.fdType;
		
		if (*fType == 'APPL')
			*fType = kApplicationAliasType; 	// special case for aliases to applications
			
		*gotCustom = (finderStuff.fdFlags & 0x0400) != 0;
		if (*gotCustom)
			FSMakeFSSpec(fsSpec->vRefNum, fsSpec->parID, fsSpec->name, iconSpec);
	}
	
	return noErr;
}


#if false

pascal OSErr FollowFinderAlias (const FSSpec *fromFile, AliasHandle alias, Boolean logon, FSSpec *target, Boolean *wasChanged) = {
                                   
	/*
    resolves an alias taken from a Finder alias file,
	updating the alias record (but not the alias resource in the file) if
	necessary.
	*/
	
	0x700F, 0xA823};  /*MOVEQ #$0F,D0; _AliasDispatch;*/
	/*FollowFinderAlias*/

#endif
	
	
static pascal OSErr IsAliasFile (const FSSpec *pfs, Boolean *flalias, Boolean *flfolder) {
                             
	CInfoPBRec pb;
	OSErr ec;
	Str255 fname;
      
	assert ((pfs != nil) || (flalias != nil) || (flfolder != nil));
	
	copystring ((void *) (*pfs).name, fname);
      
	*flalias = *flfolder = false;
	
	pb.hFileInfo.ioCompletion = nil;

	pb.hFileInfo.ioNamePtr = (StringPtr) &fname;

	pb.hFileInfo.ioVRefNum = pfs->vRefNum;

	pb.hFileInfo.ioDirID = pfs->parID;

	pb.hFileInfo.ioFVersNum = 0;  /* MFS compatibility, see TN #204 */

	pb.hFileInfo.ioFDirIndex = 0;
      
	ec = PBGetCatInfoSync(&pb);
      
	/*set flalias if the item is not a directory and the aliasFile bit is set */

	if (ec == noErr) { /*check directory bit*/
		
		if ((pb.hFileInfo.ioFlAttrib & ioDirMask) != 0)
			*flfolder = true;
        else if ((pb.hFileInfo.ioFlFndrInfo.fdFlags & 0x8000) != 0)
			*flalias = true;
		}
      
	return (ec);
	} /*IsAliasFile*/


static pascal OSErr ResolveAliasFileMountOption (FSSpec *pfs, Boolean resolvechains, Boolean *targetIsFolder, Boolean *wasAliased, Boolean mountRemoteVols) {

	short resnum;
	Handle alisHandle;
	FSSpec fsorig;
	Boolean updateFlag, foundFlag, wasAliasedTemp, specChangedFlag;
	short chainCount;
	OSErr ec;
      
	assert ((pfs != nil) || (targetIsFolder != nil) || (wasAliased != nil));
      
	fsorig = *pfs; /*so FSSpec can be restored in case of error*/

	chainCount = MAXCHAINS; /*circular alias chain protection*/
	
	resnum = -1; /*resource file not open*/
      
	*targetIsFolder = foundFlag = specChangedFlag = false;

	do { /*loop through chain of alias files*/
		
		chainCount--;
		
		/*check if FSSpec is an alias file or a directory*/
		/*note that targetIsFolder => NOT wasAliased*/
		
        ec = IsAliasFile (pfs, wasAliased, targetIsFolder);
        
        if (ec != noErr || !(*wasAliased)) 
        	break;
        
        resnum = FSpOpenResFile (pfs, fsCurPerm); /*get the resource file reference number*/
        
        ec = ResError ();
        
        if (resnum == -1) 
        	break;
		
		alisHandle = Get1IndResource (rAliasType, 1); /*the first 'alis' resource in the file is the appropriate alias*/
		
		ec = ResError ();
		
        if (alisHandle == nil) 
        	break;
		
		LoadResource (alisHandle); /*load the resource explicitly in case SetResLoad(FALSE)*/
		
		ec = ResError ();
        
        if (ec != noErr) 
        	break;
    
        ec = FollowFinderAlias (pfs, (AliasHandle) alisHandle, mountRemoteVols, pfs, &updateFlag);
		
		/*FollowFinderAlias returns nsvErr if volume not mounted*/
        
        if (ec == noErr) {
        
            if (updateFlag) { /*the resource in the alias file needs updating*/
            
				ChangedResource(alisHandle);
				
				WriteResource(alisHandle);
				}
          
          	specChangedFlag = true; /*in case of error, restore file spec*/
          
          	ec = IsAliasFile (pfs, &wasAliasedTemp, targetIsFolder);
          	
          	if (ec == noErr) /*we're done unless it was an alias file and we're following a chain*/
				foundFlag = !(wasAliasedTemp && resolvechains);
			}
        
		CloseResFile (resnum);
		
		resnum = -1;
		} while ((ec == noErr) && (chainCount > 0) && (!foundFlag));

	if (chainCount == 0 && !foundFlag) /*return file not found error for circular alias chains*/
      	ec = fnfErr; 
      	
	/*if error occurred, close resource file and restore the original FSSpec*/

	if (resnum != -1) 
		CloseResFile (resnum);

	if (ec != noErr && specChangedFlag) 
		*pfs = fsorig;
            
	return (ec);
	} /*ResolveAliasFileMountOption*/
	
	
boolean filemakealias (const FSSpec *srcFile, const FSSpec *destFile) {
	
	return (!fileerror ((ptrfilespec) srcFile, CreateAliasFile (srcFile, destFile)));
	} /*filemakealias*/


boolean fileresolvealias (FSSpec *fs, boolean flmountvolume) {
	
	/*
	DW 9/15/93: add flmountvolume parameter, include code from Apple technote.
	
	DW 9/16/93: don't report errors in a dialog. too often it's the wrong thing
	to do. if you want to report an error to the user, cook up your own dialog.
	*/

	Boolean flfolder, flalias;
	OSErr ec;
	
	if (flmountvolume) 
		ec = ResolveAliasFile (fs, true, &flfolder, &flalias);
	else {
		ec = ResolveAliasFileMountOption (fs, true, &flfolder, &flalias, false);
		
		if (ec == nsvErr) /*no dialog, just return false, the caller must be prepared*/
			return (false);
		}
	
	return (ec == noErr);
	} /*fileresolvealias*/
	
	
