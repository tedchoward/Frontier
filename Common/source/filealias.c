
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

/******************************************************************************
**
**  Project Name:	LDRProjects
**     File Name:	MAF.c
**
**   Description:	New C routine for creating alias files given source &
**					destination FSSpecs
**
*******************************************************************************
**                       A U T H O R   I D E N T I T Y
*******************************************************************************
**
**	Initials	Name
**	--------	-----------------------------------------------
**	LDR			Leonard Rosenthol
**	LSR			Larry Rosenstein
**
*******************************************************************************
**                      R E V I S I O N   H I S T O R Y
*******************************************************************************
**
**	  Date		Time	Author	Description
**	--------	-----	------	---------------------------------------------
**	09/03/91	21:01	LDR		Installed latest version from LSR
**	09/02/91	20:36	LDR		Moved into separate file
**						LSR		Original version
**
******************************************************************************/

#include "frontier.h"
#include "standard.h"

#include "filealias.h"
#include "error.h"
#include "resources.h"
#include "file.h"


typedef struct {
	OSType		folderType;
	OSType		aliasType;
} MappingEntry;


#define kNumIconTypes 6
#define kNumFolders 9


	// Resource ID used for alias file's icon (when needed)
	
#define kCustomAliasIconID -16496


	// Forward declarations
	
static Boolean FSpIsVolume(const FSSpec* fsSpec);
static Boolean FSpIsFolder(const FSSpec* fsSpec, long* dirID, unsigned short* fdFlags);
static Boolean CopyCustomIcons(const FSSpec* source, short sourceID,
							short destRefnum, short destID);
static Boolean CopyDriverIcon(const FSSpec* volSpec, short destRefnum, short destID);
static OSErr DetermineAliasInfo(const FSSpec* fsSpec, OSType* creator, OSType* fType,
							Boolean* gotCustom, FSSpec* iconSpec, short* rsrcID, unsigned short* fdFlags);


#if 0
	todo:
		different kinds of disks (servers, floppies, ...)
		filesharing?
#endif


static pascal OSErr CreateAliasFile(const FSSpec* targetFile, const FSSpec* aliasFile) {
	
	/*
	3.0.2b1 dmb: grab target's finfo flags and carry over label index
	*/
	
	OSType			fileCreator, fileType;
	FSSpec			iconSpec;
	Boolean			gotCustom;
	short			rsrcID;
	short			aliasRefnum;
	AliasHandle		alias;
	unsigned short	targetflags;
	FInfo			finf;
	
		// Get type/creator for alias file & custom icon file spec
	OSErr err = DetermineAliasInfo(targetFile, &fileCreator, &fileType, 
											&gotCustom, &iconSpec, &rsrcID, &targetflags);
	if (err != noErr) return err;
	
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
	
	finf.fdFlags |= (targetflags & 0x000E); //grab label index
	
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
					DisposeHandle(h);
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
	
	//Code change by Timothy Paustian Monday, June 19, 2000 10:36:34 PM
	//I am getting rid of this routine. We will see if it is called - This does get called
	//by the file verb, make alias. We could use a script to do this, there has to be a better way.
	//check call. I think this is just not going to be implemented in the carbon version.
	//not a big deal IMHO.
	#pragma unused(volSpec)
	#pragma unused(destRefnum)
	#pragma unused(destID)
	return false;
		
}


	// Figures out information about an alias for the indicated file (fsSpec)
	// Returns type & creator of file; whether the target has a custom icon (and
	//		the file from which to get it, and the resource ID of those icons)
	
static OSErr DetermineAliasInfo(const FSSpec* fsSpec, OSType* creator, OSType* fType,
							Boolean* gotCustom, FSSpec* iconSpec, short* rsrcID, unsigned short* fdFlags) {
	
	/*
	3.0.2b1 dmb: return the fdFlags of the target. to minimize code changes, 
	we're not dealing with volumes
	*/
	
	OSErr err;
	long dirID;
	FInfo finderStuff;
	
	*gotCustom = false;
	*rsrcID = kCustomIconResource;	// default value
	
	if (FSpIsVolume(fsSpec)) {
			// temporarily, all volumes are given the same type
		
		*fdFlags = 0; /*lazy -- will lose label info*/
		
		*creator = 'MACS';
		*fType = kContainerHardDiskAliasType; //// need more work here

		err = FSMakeFSSpec(fsSpec->vRefNum, 2, "\pIcon\015", iconSpec);
		
		if (err == noErr) // volume has a custom icon file (maybe)
			*gotCustom = true;
	}
	
	else {
		
		if (FSpIsFolder(fsSpec, &dirID, fdFlags)) {
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
			*gotCustom = (*fdFlags & 0x0400) != 0;
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
			
			*fdFlags = finderStuff.fdFlags;
			
			if (err != noErr) return noErr;
			
			*creator = finderStuff.fdCreator;
			*fType = finderStuff.fdType;
			
			if (*fType == 'APPL')
				*fType = kApplicationAliasType; 	// special case for aliases to applications
				
			*gotCustom = (finderStuff.fdFlags & 0x0400) != 0;
			if (*gotCustom)
				FSMakeFSSpec(fsSpec->vRefNum, fsSpec->parID, fsSpec->name, iconSpec);
			}
		}
	
	return noErr;
	}


boolean MakeAliasFile (const FSSpec *srcFile, const FSSpec *destFile) {
	
	
	//Code change by Timothy Paustian Tuesday, June 20, 2000 1:41:43 PM
	//We don't do no stinking alias in Carbon.
	//#if TARGET_API_MAC_CARBON == 1
	//#pragma unused(srcFile)
	//#pragma unused(destFile)
	//return false;
	//#else
		
	long result;
	
	OSErr err = Gestalt (gestaltAliasMgrAttr, &result);
	
	if ((err != noErr) || (result == 0))
		return (false);
	
	return (!oserror (CreateAliasFile(srcFile, destFile)));
	//#endif
		
}



