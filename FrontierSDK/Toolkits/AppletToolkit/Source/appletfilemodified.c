
/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include "appletmemory.h"
#include "appletfolder.h"
#include "appletfilesinternal.h"
#include "appletfilemodified.h"


boolean filegetmodified (ptrfilespec pfs, long *modified) {
	
	/*
	single entry-point to get a folder's modification date. need this
	to be absolutely as fast as possible.
	*/
		
	CInfoPBRec pb;
	
	if (!getmacfileinfo (pfs, &pb))
		return (false);
		
	*modified = pb.dirInfo.ioDrMdDat;
	
	return (true);
	} /*filegetmodified*/
	

static boolean setmodified (ptrfilespec pfs, long modified) {

	CInfoPBRec pb;
	
	if (!getmacfileinfo (pfs, &pb))
		return (false);
		
	pb.dirInfo.ioDrMdDat = modified;
	
	return (setmacfileinfo (pfs, &pb));
	} /*setmodified*/


boolean filetouchparentfolder (ptrfilespec pfs) {
	
	/*
	set the modification date of the file's parent folder. this is 
	the protocol used by Clay Basket and the Finder to tell if the 
	contents of a folder needs to be refreshed.
	
	dmb 9/21/93: broke this out into it's own routine. when touching 
	the parent, the current time should always be used, even if the file's
	mod date is being set to something else. also, this needs to be 
	directly callable; renaming a file touches the parent, but doesn't 
	change the file's mod date.
	*/
	
	FSSpec fsfolder;
	long timenow;
	
	GetDateTime ((unsigned long *) &timenow);
	
	filegetparentfolderspec (pfs, &fsfolder);
	
	return (setmodified (&fsfolder, timenow));
	} /*filetouchparentfolder*/


boolean filesetmodified (ptrfilespec pfs, long modified) {

	/*
	set the modification date of the file. if it's 0, 
	we use the current time.
	
	also set the modification date of its parent folder.
	*/
	
	if (modified == 0)
		GetDateTime ((unsigned long *) &modified);
	
	if (!setmodified (pfs, modified))
		return (false);
	
	filetouchparentfolder (pfs);
	
	return (true);
	} /*filesetmodified*/


boolean filetouchallparentfolders (ptrfilespec pfs) {
	
	tyfilespec fs = *pfs, fsparent;
	long now;
	
	GetDateTime ((unsigned long *) &now);
	
	while (true) {
		
		if (!filegetparentfolderspec (&fs, &fsparent))
			return (true);
		
		if (!setmodified (&fsparent, now))
			return (false);
		
		fs = fsparent;
		} /*while*/
	} /*filetouchallparentfolders*/
	
	
