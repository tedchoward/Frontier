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

#include "frontier.h"
#include "standard.h"

#include "filealias.h"
#include "cursor.h"
#include "dialogs.h"
#include "error.h"
#include "memory.h"
#include "ops.h"
#include "quickdraw.h"
#include "resources.h"
#include "strings.h"
#include "timedate.h"
#include "frontierwindows.h"
#include "file.h"
#include "shell.h"
#include "shell.rsrc.h"
#include "langinternal.h" /*for langbackgroundtask*/

/* Modern Macs use slash ('/') as the path separator, not colon (':') */
#define kFPlatformPathSeparatorChar '/'

#import <Foundation/Foundation.h>
#include <sys/param.h>
#include <sys/mount.h>
#include <sys/stat.h>

static short applicationresnum = -1;

typedef struct tyfileinfo tyvolinfo;

void setfserorparam(const ptrfilespec fs) {
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	
	NSURL *fileURL = (NSURL *)fs->url;
	NSString *path = [url path];
	bigstring bs;
	
	CFStringGetPascalString((CFStringRef)path, bs, sizeof (bigstring), kCFStringEncodingMacRoman);
	
	setoserrorparam(bs);
	
	[pool release];
}

boolean endswithpathsep(bigstring bs) {
	UInt8 ch = getstringcharacter(bs, stringlength(bs) - 1);
	
	return ch == kFPlatformPathSeparatorChar;
}

boolean cleanendoffilename(bigstring bs) {
	if (endswithpathsep(bs)) {
		setstringlength(bs, stringlength(bs) - 1);
		return true;
	}
	
	return false;
}

boolean getmachinename(bigstring bsname) {
	CFStringRef machineName = CSCopyMachineName();
	
	boolean fl = CFStringGetPascalString (machineName, bsname, sizeof (bigstring), kCFStringEncodingMacRoman);
	
	CFRelease(machineName);
	
	if (!fl) {
		setemptystring (bsname);
	}
	
	return fl;
	
}

static BOOL pathIsVolume(NSString *path) {
	NSArray *mountedLocalVolumePaths = [[NSWorkspace sharedWorkspace] mountedLocalVolumePaths];
	return [mountedLocalVolumePaths containsObject:path];
}

boolean filegetfsvolumeinfo(const ptrfilespec fs, tyfileinfo *info) {
	boolean success = false;
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	
	NSURL *fileURL = (NSURL *)fs->url;
	NSString *filePath = [fileURL path];
	
	NSFileManager *defaultManager = [NSFileManager defaultManager];
	NSWorkspace *sharedWorkspace = [NSWorkspace sharedWorkspace];
	NSError *err;
	
	if (pathIsVolume(filePath)) {
		success = true;
		
		memset(info, 0, sizeof (tyfileinfo));
		
		info->flvolume = true;
		info->flbusy = true;
		
		NSDictionary *fileSystemAttributes = [defaultManager attributesOfFileSystemForPath:filePath error:err];		
		
		if (!err) {
			info->cttotalbytes = [[fileSystemAttributes objectForKey:NSFileSystemSize] unsignedLongValue];
			info->ctfreebytes = [[fileSystemAttributes objectForKey:NSFileSystemFreeSize] unsignedLongValue];
			info->vnum = [[fileSystemAttributes objectForKey:NSFileSystemNumber] shortValue];
		} else {
			//TODO: error reporting
			NSLog(@"Error accessing file system attributes: %@\n%@", [err localizedDescription], [err userInfo]);
		}
		
		
		
		const char *path = [filePath fileSystemRepresentation];
		
		struct statfs buf;
		
		if (statfs(path, &buf) != -1) {		
			info->blocksize = buf.f_bsize;
			
			info->ctfiles = buf.f_files;
		}
		
		BOOL removable, writable, unmountable;
		NSString *description, *fileSystemType;
		
		[sharedWorkspace getFileSystemInfoForPath:filePath 
									  isRemovable:&removable
									   isWritable:&writable 
									isUnmountable:&unmountable 
									  description:&description 
											 type:&fileSystemType];
		
		info->flejectable = unmountable;
		
	}
	
	[pool release];
	
	return success;
}

boolean filegetinfo(const ptrfilespec fs, tyfileinfo *info) {
	boolean success = false;
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	
	NSURL *fileURL = (NSURL *)fs->url;
	NSString *filePath = [fileURL path];
	FSRef fileRef;
	CFURLGetFSRef(fileURL, &fileRef);

	NSError *err = nil;
	
	FSCatalogInfo catalogInfo;
	FSRef parentRef;
	
	OSErr status = FSGetCatalogInfo(&fileRef, 
					 kFSCatInfoParentDirID | kFSCatInfoCreateDate | kFSCatInfoContentMod | kFSCatInfoVolume | kFSCatInfoNodeFlags | kFSCatInfoValence | kFSCatInfoFinderInfo | kFSCatInfoDataSizes,
					 &catalogInfo, 
					 NULL, 
					 NULL, 
					 &parentRef);
	
	UInt16 finderBits = 0;
	
	if (status == noErr) {
		success = true;
		
		memset(info, 0, sizeof (tyfileinfo));
		
		info->dirid = catalogInfo.parentDirID;
		
		CFAbsoluteTime sinceJan01;
		UCConvertUTCDateTimeToCFAbsoluteTime(&catalogInfo.createDate, &sinceJan01);
		UCConvertCFAbsoluteTimeToSeconds(sinceJan01, &info->timecreated);
		
		UCConvertUTCDateTimeToCFAbsoluteTime(&catalogInfo.contentModDate, &sinceJan01);
		UCConvertCFAbsoluteTimeToSeconds(sinceJan01, &info->timemodified);
		
		info->vnum = catalogInfo.volume;
		
		if (info->dirid == fsRtParID) {
			filegetfsvolumeinfo(fs, info);
		} else {
			info->flvolume = false;
		}
		
		if ((catalogInfo.nodeFlags & kFSNodeForkOpenMask) != 0) {
			// if it's a volume, fllocked may already be set
			info->fllocked = true;
		} else {
			info->fllocked = (catalogInfo.nodeFlags & kFSNodeLockedMask) != 0;
		}
		
		info->flfolder = (catalogInfo.nodeFlags & kFSNodeIsDirectoryMask) != 0;
		
		if (info->flfolder) {
			// folders are considered "busy" if there are any files within
			// the folder
			info->flbusy = catalogInfo.valence > 0;
			
			FolderInfo *folderInfo = (FolderInfo *)catalogInfo.finderInfo;
			info->iconposition = folderInfo->location;
			
			if (!info->flvolume) {
				info->ctfiles = catalogInfo.valence;
			}
			
			info->folderview = (tyfolderview)0;
			
			finderBits = folderInfo->finderFlags;
		} else {
			FileInfo *fileInfo = (FileInfo *)catalogInfo.finderInfo;
			
			info->ixlabel = (fileInfo->finderFlags & kColor) >> 1;
			info->flbusy = (catalogInfo.nodeFlags & kFSNodeForkOpenMask) != 0;
			info->filecreator = fileInfo->fileCreator;
			info->filetype = fileInfo->fileType;
			info->sizedatafork = catalogInfo.dataLogicalSize;
			info->iconposition = fileInfo->location;
			
			finderBits = fileInfo->finderFlags;
		}
		
		info->flalias = (finderBits & kIsAlias) != 0;
		info->flbundle = (finderBits & kHasBundle) != 0;
		info->flinvisible = (finderBits & kIsInvisible) != 0;
		info->flstationery = (finderBits & kIsStationery) != 0;
		info->flshared = (finderBits & kIsShared) != 0;
		info->flnamelocked = (finderBits & kNameLocked) != 0;
		info->flcustomicon = (finderBits & kHasCustomIcon) != 0;
		
		if (info->flfolder) {
			// Mac OS X bundles/packages are not considered folders
			if ([[NSWorkspace sharedWorkspace] isFilePackageAtPath:filePath]) {
				info->flfolder = false;
				info->flbundle = true;
			}
		} else {
			if (!info->filetype) {
				// if no type code, then look at file extension
				NSString *fileExtension = [filePath pathExtension];
				NSUInteger extLength = [fileExtension length];
				if (extLength > 0 && extLength <= 4) {
					// This is not a good way to determine file type, but it's what the old system does
					// TODO: switch to uniform type identifiers
					
					bigstring bsext;
					CFStringGetPascalString((CFStringRef)fileExtension, bsext, sizeof (bigstring), kCFStringEncodingMacRoman);
					stringtoostype(bsext, &info->filetype);
				}
			}
		}
		
	} else {
		oserror(status);
	}
	
	[pool release];
	return success;
}

boolean filegetvolumename(UInt16 vnum, bigstring volname) {
	HFSUniStr255 unicodeString;
	OSErr err;
	
	err = FSGetVolumeInfo(vnum, 0, NULL, kFSVolInfoNone, NULL, &unicodeString, NULL);
	
	if (err == noErr) {
		CFStringRef cfName = CFStringCreateWithCharacters(kCFAllocatorDefault, unicodeString.unicode, unicodeString.length);
		CFStringGetPascalString(cfName, volname, sizeof (bigstring), kCFStringEncodingMacRoman);
		
		CFRelease(cfName);
		
		return true;
	} else {
		oserror(err);
		return false;
	}
}

boolean fileisbusy(const ptrfilespec fs, boolean *flbusy) {
	tyfileinfo info;
	
	if (!filegetinfo(fs, &info)) {
		return false;
	}
	
	*flbusy = info.flbusy;
	
	return true;
}

boolean filehasbundle(const ptrfilespec fs, boolean *flbundle) {
	tyfileinfo info;
	
	if (!filegetinfo(fs, &info)) {
		return false;
	}
	
	*flbundle = info.flbundle;
	
	return true;
}

boolean filesetbundle(const ptrfilespec fs, boolean flbundle) {
	FSRef fileRef;
	FSCatalogInfo catalogInfo;
	OSErr err;
	
	CFURLGetFSRef(fs->url, &fileRef);
	err = FSGetCatalogInfo(&fileRef, kFSCatInfoNodeFlags | kFSCatInfoFinderInfo, &catalogInfo, NULL, NULL, NULL);
	
	if (err == noErr) {
		if (catalogInfo.nodeFlags & kFSNodeIsDirectoryMask) {
			if (flbundle) {
				((FolderInfo *)catalogInfo.finderInfo)->finderFlags |= kHasBundle; //turn on
			} else {
				((FolderInfo *)catalogInfo.finderInfo)->finderFlags &= ~kHasBundle; //turn off
			}
		} else {
			if (flbundle) {
				((FileInfo *)catalogInfo.finderInfo)->finderFlags |= kHasBundle; //turn on
			} else {
				((FileInfo *)catalogInfo.finderInfo)->finderFlags &= ~kHasBundle; //turn off
			}
		}
		
		err = FSSetCatalogInfo(&fileRef, kFSCatInfoFinderInfo, &catalogInfo);
		
		if (err == noErr) {
			return true;
		}
	}
	
	oserror(err);
	return false;
}

boolean fileisalias(const ptrfilespec fs, boolean *flalias) {
	tyfileinfo info;
	
	if (!filegetinfo(fs, &info)) {
		return false;
	}
	
	*flalias = info.flalias;
	
	return true;
}


boolean fileisvisible(const ptrfilespec fs, boolean *flvisible) {
	tyfileinfo info;
	
	if (!filegetinfo(fs, &info)) {
		return false;
	}
	
	*flvisible = !info.flinvisible;
	
	return true;	
}

boolean filesetvisible(const ptrfilespec fs, boolean flvisible) {
	FSRef fileRef;
	FSCatalogInfo catalogInfo;
	OSErr err;
	
	CFURLGetFSRef(fs->url, fileRef);
	
	err = FSGetCatalogInfo(&fileRef, kFSCatInfoFinderInfo, &catalogInfo, NULL, NULL, NULL);
	
	// this is confusing because the parameter is visible, but the finder flag is isInvisible
	// to make a file visible, we turn off the isInvisible flag
	
	if (err == noErr) {
		if (!flvisible) {
			((FileInfo *)catalogInfo.finderInfo)->finderFlags |= kIsInvisible; //turn on
		} else {
			((FileInfo *)catalogInfo.finderInfo)->finderFlags &= ~kIsInvisible; //turn off
		}
		
		err = FSSetCatalogInfo(&fileRef, kFSCatInfoFinderInfo, &catalogInfo);
		
		if (err == noErr) {
			return true;
		}
	}
	
	oserror(err);
	return false;
}

boolean getfiletype(const ptrfilespec fs, OSType *type) {
	tyfileinfo info;
	
	*type = '    ';
	
	if (!filegetinfo(fs, &info)) {
		return false;
	}
	
	*type = info.filetype;
	
	return true;
}

boolean getfilecreator(const ptrfilespec fs, OSType *creator) {
	tyfileinfo info;
	
	*creator = '    ';
	
	if (!filegetinfo(fs, &info)) {
		return false;
	}
	
	*creator = info.filecreator;
	
	return true;
}

boolean filesize(const ptrfilespec fs, UInt32 *size) {
	// NOTE: I'm ignoring resource forks. The 90s called and wanted them back
	tyfileinfo info;
	
	if (!filegetinfo(fs, &info)) {
		return false;
	}
	
	*size = info.sizedatafork;
	
	return true;
}

boolean fileisfolder(const ptrfilespec fs, boolean *flfolder) {
	tyfileinfo info;
	
	if (!filegetinfo(fs, &info)) {
		return false;
	}
	
	*flfolder = info.flfolder;
	
	return true;	
}

boolean fileisvolume(const ptrfilespec fs) {
	tyfileinfo info;
	
	if (!filegetinfo(fs, &info)) {
		return false;
	}
	
	return info.flvolume;
}

boolean fileislocked(const ptrfilespec fs, boolean *fllocked) {
	tyfileinfo info;
	
	if (!filegetinfo(fs, &info)) {
		return false;
	}
	
	*fllocked = info.fllocked;
	
	return true;
}

boolean getfiledates(const ptrfilespec fs, UInt32 *datecreated, UInt32 *datemodified) {
	tyfileinfo info;
	
	if (!filegetinfo(fs, &info)) {
		return false;
	}
	
	*datecreated = info.timecreated;
	*datemodified = info.timemodified;
	
	return true;
}

boolean setfiledates(const ptrfilespec fs, UInt32 datecreated, UInt32 datemodified) {
	CFAbsoluteTime sinceJan01;
	FSRef fileRef;
	FSCatalogInfo catalogInfo;
	OSErr err;
	
	CFURLGetFSRef(fs->url, &fileRef);
	err = FSGetCatalogInfo(&fileRef, kFSCatInfoCreateDate | kFSCatInfoContentMod, &catalogInfo, NULL, NULL, NULL);
	
	if (err == noErr) {
		UCConvertSecondsToCFAbsoluteTime(datecreated, sinceJan01);
		UCConvertCFAbsoluteTimeToUTCDateTime(sinceJan01, &catalogInfo.createDate);
		
		UCConvertSecondsToCFAbsoluteTime(datemodified, sinceJan01);
		UCConvertCFAbsoluteTimeToUTCDateTime(sinceJan01, &catalogInfo.contentModDate);
		
		err = FSSetCatalogInfo(&fileRef, kFSCatInfoCreateDate | kFSCatInfoContentMod, &catalogInfo);
		
		if (err == noErr) {
			return true;
		}
	}
	
	oserror(err);
	return false;
}