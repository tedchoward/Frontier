
/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#include <Script.h>
#include "applet.h"
#include "appletmain.h"
#include "appletfilesinternal.h"
#include "appletfolder.h"
#include "appletfilealias.h"
#include "appletfiledelete.h"
#include "appletfiledesktop.h"
#include "appletsyserror.h"
#include "appletfilecopy.h"



static boolean copyfork (short fsource, short fdest, Handle hbuffer) {
	
	/*
	copy either the data fork or resource fork of the indicated file.
	
	return true iff the file copy was successful.
	
	5/19/92 dmb: call langbackgroundtask when a script is running to make it more 
	likely that we'll yield the processor to another app.  also, only allow 
	background tasks if fork is larger than a single buffer.  note that if a script 
	is copying a bunch of files in a loop, the interpreter is already allowing 
	backgrounding between files.
	*/

	register long buffersize = GetHandleSize (hbuffer);
	long ctbytes;
	register OSErr errcode;
	
	SetFPos (fsource, fsFromStart, 0L); 
	
	SetFPos (fdest, fsFromStart, 0L);
	
	while (true) { 
		
		ctbytes = buffersize;
		
		HLock (hbuffer);
		
		errcode = FSRead (fsource, &ctbytes, *hbuffer);
		
		HUnlock (hbuffer);
		
		if ((errcode != noErr) && (errcode != eofErr)) {
			
			fileerror (nil, errcode);
			
			return (false);
			}
			
		if (ctbytes == 0) /*last read got no bytes*/
			return (true);
		
		HLock (hbuffer);
		
		errcode = FSWrite (fdest, &ctbytes, *hbuffer);
		
		HUnlock (hbuffer);
		
		if (fileerror (nil, errcode)) 
			return (false);
		
		if (ctbytes < buffersize) /*copy of fork is finished*/
			return (true);
		
		appserviceeventqueue ();
		
		rollbeachball (); /*roll the beachball cursor if there is one*/
		} /*while*/
	} /*copyfork*/


static boolean largefilebuffer (Handle *hbuffer) {
	
	/*
	allocate a "large" buffer for a file copy or some kind of transfer.
	
	we ask for a block the size of half of the current free space.  if we can't 
	get it, we'll ask for the largest free block.  in any case, we fail if 
	we can't get a block of at least a K
	
	3/16/92 dmb: total rewrite; old version _always_ compacted the heap, 
	resulting in poor performance.
	*/
	
	register long ctbytes;
	register Handle h;
	
	*hbuffer = nil; /*default return*/
	
	ctbytes = memavail () / 2;
	
	if (ctbytes < 1024) { /*no enough memory to work with*/
		
		sysmemoryerror ();
		
		return (false);
		}
	
	ctbytes &= ~(1024 - 1); /*fast round down to nearest multiple of 1024*/
	
	h = appnewhandle (ctbytes);
	
	if (h != nil) {
		
		*hbuffer = h;
		
		return (true);
		}
	
	/*allocation failed; heap has been compacted; find largest block size*/
	
	ctbytes = CompactMem (maxSize); /*recompact the heap, finding out size of largest block*/
	
	if (ctbytes < 1024) { /*largest block is too small to use*/
		
		sysmemoryerror ();
		
		return (false);
		}
	
	ctbytes &= ~(1024 - 1); /*fast round down to nearest multiple of 1024*/
	
	*hbuffer = appnewhandle (ctbytes);
	
	return (*hbuffer != nil); /*shouldn't fail at this point, but let's be sure*/
	} /*largefilebuffer*/


static boolean openforkforcopy (FSSpec *fs, boolean flresource, short *fnum) {
	
	/*
	we need a special entry-point to allow the userprogram to copy a file that's
	already open.  we checked this with the debugger -- FSOpen returns the right
	fnum for our file.
	
	3/2/92 dmb: rewrote using PBOpen asking for read-only permission.  removed 
	fldontclose parameter.  also, support either fork with this routine
	*/
	
	OSErr errcode;
	HParamBlockRec pb;
	
	clearbytes (&pb, longsizeof (pb));
	
	pb.fileParam.ioDirID = (*fs).parID;
	
	pb.ioParam.ioNamePtr = (*fs).name;
	
	pb.ioParam.ioVRefNum = (*fs).vRefNum;
	
	pb.ioParam.ioPermssn = fsRdPerm;
	
	if (flresource)
		errcode = PBHOpenRFSync (&pb);
	else
		errcode = PBHOpenSync (&pb);
	
	*fnum = pb.ioParam.ioRefNum;
	
	if (fileerror (fs, errcode)) 
		return (false);
		
	return (true);
	} /*openforkforcopy*/
	

static boolean copyonefile (FSSpec *fsource, FSSpec *fdest, boolean fldata, boolean flresources) {

	/*
	create a copy of the indicated file in the destination volume, with the indicated
	name.  
	
	we allocate a good-sized buffer in the heap, then open and copy the data fork then
	open and copy the resource fork of the source file.
	
	return true if the operation was successful, false otherwise.
	
	7/27/90 DW: add fldata, flresources -- allows selective copying of the two
	forks of each file.  we assume one of these two booleans is true.
	
	3/2/92 dmb: use new openforkforcopy to safely open in-use file forks for copying
	
	3/16/92 dmb: maintain all public Finder flags, not just creator/type & dates
	*/
	
	Handle hbuffer;
	register OSErr errcode;
	short sourcerefnum = 0, destrefnum = 0;
	OSType filetype, filecreator;
	CInfoPBRec pb;
	CInfoPBRec pb2;
	unsigned short publicflags = 0xFCCE; /*non-reserved Finder flags as per IM-VI 9-37*/
	
	if (!largefilebuffer (&hbuffer)) 
		return (false);
	
	if (!getmacfileinfo (fsource, &pb)) 
		goto error;
	
	filecreator = pb.hFileInfo.ioFlFndrInfo.fdCreator;
	
	filetype = pb.hFileInfo.ioFlFndrInfo.fdType;
	
	setfileerrorfile (fsource);
		
	if (fldata) {
	
		if (!openforkforcopy (fsource, false, &sourcerefnum))
			goto error;
		
		if (!filenew (fdest, filecreator, filetype, &destrefnum)) 
			goto error;
		
		if (!copyfork (sourcerefnum, destrefnum, hbuffer)) /*copy data fork*/
			goto error;
		
		fileclose (sourcerefnum); 
			
		fileclose (destrefnum); 
		
		sourcerefnum = 0;
		
		destrefnum = 0;
		}
	
	if (flresources) {
		
		if (!openforkforcopy (fsource, true, &sourcerefnum))
			goto error;
		
		errcode = FSpOpenRF (fdest, fsWrPerm, &destrefnum);
		
		if (fileerror (fdest, errcode))
			goto error;
		
		if (!copyfork (sourcerefnum, destrefnum, hbuffer)) /*copy resource fork*/
			goto error;
		
		fileclose (sourcerefnum);
			
		fileclose (destrefnum); 
		
		sourcerefnum = 0;
		
		destrefnum = 0;
		}
	
	disposehandle (hbuffer);
	
	hbuffer = nil; /*if error don't dispose of it again*/
	
	if (!getmacfileinfo (fdest, &pb2)) 
		goto error;
	 
	pb2.hFileInfo.ioFlCrDat = pb.hFileInfo.ioFlCrDat;
	
	pb2.hFileInfo.ioFlMdDat = pb.hFileInfo.ioFlMdDat;
	
	pb2.hFileInfo.ioFlFndrInfo.fdFlags = (pb.hFileInfo.ioFlFndrInfo.fdFlags & publicflags);
	
	if (!setmacfileinfo (fdest, &pb2))
		goto error;
	
	filecopycomment (fsource, fdest);
	
	return (true); /*the file copy was successful*/
	
	error: /*goto here to release the buffer, close files and return false*/
	
	if (hbuffer != nil)
		disposehandle (hbuffer);
	
	if (sourcerefnum != 0)
		fileclose (sourcerefnum);
	
	if (destrefnum != 0)
		fileclose (destrefnum);
	
	filedelete (fdest); /*no file created on error*/
	
	return (false);
	} /*copyonefile*/


boolean foldercopy (ptrfilespec fssource, ptrfilespec fsdest, long *dirid) {
	
	OSErr ec;
	
	ec = FSpDirCreate (fsdest, smSystemScript, dirid);
	
	if (ec != noErr) {
		
		fileerror (fssource, ec);
		
		return (false);
		}
	
	/*copy folder attributes from source to dest*/ {
		
		CInfoPBRec pb;
		
		if (getmacfileinfo (fssource, &pb)) {
			
			assert (pb.dirInfo.ioFlAttrib == 16); /*just the folder bit set*/
			
			setmacfileinfo (fsdest, &pb);
			}	
		}
	
	filecopycomment (fssource, fsdest);
	
	return (true);
	} /*foldercopy*/
	

boolean filteredcopy (FSSpec *, FSSpec *); /*prototype*/


static boolean foldercopyvisit (bigstring bsitem, tyfileinfo *info, long refcon) {
	
	register FSSpec *fsfolder = (FSSpec *) refcon;
	FSSpec fssource, fsdest;
	
	filemakespec ((*info).vnum, (*info).dirid, bsitem, &fssource);
	
	filegetsubitemspec (fsfolder, bsitem, &fsdest);
	
	return (filteredcopy (&fssource, &fsdest));
	} /*foldercopyvisit*/


static tyfilecopycallback copyfilter = nil;


boolean filteredcopy (FSSpec *fssource, FSSpec *fsdest) {
	
	tyfileinfo info;
	
	/*call the copy filter callback*/ {
		
		if (copyfilter != nil) {
			
			boolean flskipfile, flcancel;
			
			flcancel = false; 
			
			flskipfile = false;
			
			(*copyfilter) (fssource, fsdest, &flskipfile, &flcancel);
			
			if (flcancel)
				return (false);
			
			if (flskipfile) 
				return (true);
			}
		}
		
	filegetinfo (fssource, &info);
	
	if (info.flfolder) {
		
		FSSpec fsfolder;
		long dirid;
		
		if (!foldercopy (fssource, fsdest, &dirid))
			return (false);
		
		getfolderfilespec ((*fsdest).vRefNum, dirid, &fsfolder);
		
		return (folderloop (fssource, false, &foldercopyvisit, (long) &fsfolder));
		}
		
	if (info.flalias) {
		
		if (!fileresolvealias (fssource, true))
			return (false);
			
		return (filemakealias (fssource, fsdest));
		}
		
	return (copyonefile (fssource, fsdest, true, true));
	} /*filteredcopy*/


boolean filecopy (FSSpec *fssource, FSSpec *fsdest, tyfilecopycallback callback) {
	
	/*
	copies a file, folder or alias
	
	we callback to you on everything we copy. you can tell us to skip the file
	or folder, or you can cancel the copy.
	*/
	
	copyfilter = callback;
	
	return (filteredcopy (fssource, fsdest));
	} /*filecopy*/





