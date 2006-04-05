
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

#ifdef MACVERSION
	#if TARGET_API_MAC_CARBON
		#include "MoreFilesX.h"
	#else
		#include "MoreFilesExtras.h"
	#endif
#endif

#include "filealias.h"
#include "cursor.h"
#include "dialogs.h"
#include "error.h"
#include "mac.h"
#include "memory.h"
#include "ops.h"
#include "quickdraw.h"
#include "resources.h"
#include "strings.h"
#include "frontierwindows.h"
#include "file.h"
#include "shell.h"
#include "shell.rsrc.h"
#include "langinternal.h" /*for langbackgroundtask*/


#ifdef MACVERSION
#ifdef flcomponent

	#include "SetUpA5.h"

#endif
#endif



boolean equalfilespecs (const tyfilespec *fs1, const tyfilespec *fs2) {
	
	/*
	5.0a25 dmb: until we set the volumeID to zero for all Win fsspecs,
	we must not compare them here
	*/

	#ifdef MACVERSION
		if ((*fs1).vRefNum != (*fs2).vRefNum)
			return (false);
		
		if ((*fs1).parID != (*fs2).parID)
			return (false);
		
		return (equalstrings ((ptrstring) fsname (fs1), (ptrstring) fsname (fs2)));
	#endif
	#ifdef WIN95VERSION
		//if ((*fs1).volumeID != (*fs2).volumeID)
		//	return (false);
		
		return (lstrcmp(fsname (fs1), fsname (fs2)) == 0);
#endif
	} /*equalfilespecs*/


boolean filesetposition (hdlfilenum fnum, long position) {
	
	/*
	5.0.2b6 dmb: report errors
	*/
	
	#ifdef MACVERSION
		return (!oserror (SetFPos (fnum, fsFromStart, position)));
	#endif

	#ifdef WIN95VERSION
		if (SetFilePointer (fnum, position, NULL, FILE_BEGIN) == -1L) {
			
			winerror ();
			
			return (false);
			}
		
		return (true);
	#endif
	} /*filesetposition*/
	
	
boolean filegetposition (hdlfilenum fnum, long *position) {
	
	#ifdef MACVERSION
		return (!oserror (GetFPos (fnum, position)));
	#endif

	#ifdef WIN95VERSION
		*position = SetFilePointer (fnum, 0L, NULL, FILE_CURRENT);

		if (*position == -1L) {
			
			winerror ();

			return (false);
			}

		return (true);
	#endif
	} /*filegetposition*/


boolean filegeteof (hdlfilenum fnum, long *position) {
	
	#ifdef MACVERSION
		/*
		6/x/91 mao
		*/

		return (!oserror (GetEOF (fnum, position)));
	#endif

	#ifdef WIN95VERSION
		long origpos;

		filegetposition (fnum, &origpos);

		*position = SetFilePointer (fnum, 0L, NULL, FILE_END);

		filesetposition (fnum, origpos);

		if (*position == -1L) {
			
			winerror ();

			return (false);
			}

		return (true);
	#endif
	} /*filegeteof*/


boolean fileseteof (hdlfilenum fnum, long position) {
	
	/*
	5.0.2b6 dmb: report errors
	*/
	
	#ifdef MACVERSION
		return (!oserror (SetEOF (fnum, position)));
	#endif

	#ifdef WIN95VERSION
		long currentPosition;

		currentPosition = SetFilePointer (fnum, 0L, NULL, FILE_CURRENT);
		
		if (currentPosition == -1L)
			goto error;
		
		if (SetFilePointer (fnum, position, NULL, FILE_BEGIN) == -1L)
			goto error;
		
		if (!SetEndOfFile (fnum))
			goto error;
				
		if (SetFilePointer (fnum, currentPosition, NULL, FILE_BEGIN) == -1L)
			goto error;
		
		return (true);
		
	error:
		winerror ();
		
		return (false);
	#endif
	} /*fileseteof*/


long filegetsize (hdlfilenum fnum) {
	
	/*
	get the size of a file that's already open.
	*/
	
	#ifdef MACVERSION
		long lfilesize;
		
		if (GetEOF (fnum, &lfilesize) != noErr)
			lfilesize = 0;
		
		return (lfilesize);
	#endif

	#ifdef WIN95VERSION
		long lfilesize;

		lfilesize = GetFileSize (fnum, NULL);
		if (lfilesize == -1L)
			lfilesize = 0L;

		return (lfilesize);
	#endif
	} /*filegetsize*/


boolean filetruncate (hdlfilenum fnum) {
	
	return (fileseteof (fnum, 0L));
	} /*filetruncate*/


boolean filewrite (hdlfilenum fnum, long ctwrite, void *buffer) {
	
	/*
	write ctwrite bytes from buffer to the current position in file number
	fnum.  return true iff successful.
	*/

	if (ctwrite > 0) {
		
		#ifdef MACVERSION
			if (oserror (FSWrite (fnum, &ctwrite, buffer)))
				return (false);
		#endif

		#ifdef WIN95VERSION
			DWORD numberBytesWritten;

			if (WriteFile (fnum, buffer, ctwrite, &numberBytesWritten, NULL)) {
				
				if ((DWORD)ctwrite == numberBytesWritten)
					return (true);
				}
			
			winerror();

			return (false);
		#endif
		}
	
	return (true);
	} /*filewrite*/


boolean filereaddata (hdlfilenum fnum, long ctread, long *ctactual, void *buffer) {
	
	/*
	lower level than fileread, we can read less than the number of 
	bytes requested.
	*/
	
	*ctactual = ctread;
	
	if (ctread > 0) {
		
		#ifdef MACVERSION
			OSErr ec = FSRead (fnum, ctactual, buffer);
			
			if (ec != noErr && ec != eofErr) {
				
				oserror (ec);
				
				return (false);
				}
		#endif
		
		#ifdef WIN95VERSION
			if (!ReadFile (fnum, buffer, ctread, ctactual, NULL)) {
				
				winerror ();
				
				return (false);
				}
		#endif
		}
	
	return (true);
	} /*filereaddata*/
	
	
boolean fileread (hdlfilenum fnum, long ctread, void *buffer) {
	
	/*
	read ctread bytes from the current position in file number fnum into
	the buffer.  return true iff successful.
	*/
	
	long ctactual;
	
	if (!filereaddata (fnum, ctread, &ctactual, buffer))
		return (false);
	
	if (ctactual < ctread) {
		
		#ifdef MACVERSION
			oserror (eofErr);
		#endif
		#ifdef WIN95VERSION
			oserror (ERROR_HANDLE_EOF);
		#endif

		return (false);
		}
	
	return (true);
	} /*fileread*/
	
	
boolean filegetchar (hdlfilenum fnum, char *ch) {
	
	/*
	long pos;
	
	if (!filegetposition (fnum, &pos))
		return (false);
		
	if (pos >= filegetsize (fnum)) {
		
		*fleof = true;
		
		return (true);
		}
	
	*fleof = false;
	*/
	
	return (fileread (fnum, (long) 1, ch));
	} /*filegetchar*/
	
	
boolean fileputchar (hdlfilenum fnum, char ch) {
	
	return (filewrite (fnum, (long) 1, &ch));
	} /*fileputchar*/


boolean filewritehandle (hdlfilenum fnum, Handle h) {
	
	/*
	write the indicated handle to the open file indicated by fnum at the
	current position in the file.
	*/
	
	return (filewrite (fnum, gethandlesize (h), *h));
	} /*filewritehandle*/


/*
static boolean filereadhandlebytes (short fnum, long ctbytes, Handle *hreturned) {
	
	/%
	6/x/91 mao
	this one is parallel to filewritehandle
	%/
	
	register Handle h;
	
	if (!newclearhandle (ctbytes, hreturned))
		return (false);
		
	h = *hreturned; /%copy into register%/
		
	if (oserror (fileread (fnum, ctbytes, *h))) {
		
		disposehandle (h);
		
		return (false);
		}
		
	return (true);
	} /%filereadhandlebytes%/
*/


boolean filereadhandle (hdlfilenum fnum, Handle *hreturned) {
	
	/*
	not exactly parallel to filewritehandle.  we read the whole file into the 
	indicated handle and return true if it worked.
	*/
	
	register long lfilesize;
	register Handle h;
	
	lfilesize = filegetsize (fnum);
	
	if (!newclearhandle (lfilesize, hreturned))
		return (false);
		
	h = *hreturned; /*copy into register*/
		
	if (!fileread (fnum, lfilesize, *h)) {
		
		disposehandle (h);
		
		return (false);
		}
		
	return (true);
	} /*filereadhandle*/


#ifdef MACVERSION	

//Code change by Timothy Paustian Monday, June 19, 2000 3:15:01 PM
//Changed to Opaque call for Carbon

static pascal void iocompletion (ParmBlkPtr pb) {

	DisposePtr ((Ptr) pb);
	} /*iocompletion*/


#if TARGET_RT_MAC_CFM || TARGET_RT_MAC_MACHO

	#if TARGET_API_MAC_CARBON

		//looks like we need some kind of file UPP
		//do we need to create a UPP, yes we do.
		IOCompletionUPP	iocompletionDesc = nil;

		#define iocompletionUPP (iocompletionDesc)

	#else

		static RoutineDescriptor iocompletionDesc = BUILD_ROUTINE_DESCRIPTOR (uppIOCompletionProcInfo, iocompletion);

		#define iocompletionUPP (&iocompletionDesc)

	#endif

#else

	static IOCompletionUPP iocompletionUPP = &iocompletion;

#endif

#endif //MACVERSION

boolean flushvolumechanges (const tyfilespec *fs, hdlfilenum fnum) {
#ifdef MACVERSION
#	pragma unused(fnum)

	/*
	4.1b7 dmb: was -- FlushVol (nil, (*fs).vRefNum);
	
	now use PB call to do asynch flush
	*/
	
	ParamBlockRec *pb;
	
	pb = (ParamBlockRec *) NewPtrClear (sizeof (ParamBlockRec));
	
	(*pb).volumeParam.ioVRefNum = (*fs).vRefNum;
	
	(*pb).volumeParam.ioCompletion = iocompletionUPP;
	
	PBFlushVolAsync (pb);
#endif	

#ifdef WIN95VERSION
	if (fnum != NULL)
		FlushFileBuffers (fnum);
#endif
	return (true);
	} /*flushvolumechanges*/

//Code change by Timothy Paustian Wednesday, July 26, 2000 10:52:49 PM
//new routine to create UPPS for the async file saves.
void fileinit (void) {
	#if TARGET_API_MAC_CARBON
	if(iocompletionDesc == nil)
		iocompletionDesc = NewIOCompletionUPP(iocompletion);
	#endif
	} /*fileinit*/


void fileshutdown(void) {

	#if TARGET_API_MAC_CARBON
	if(iocompletionDesc != nil)
		DisposeIOCompletionUPP(iocompletionDesc);
	#endif
	} /*fileshutdown*/


static boolean filecreateandopen (const tyfilespec *fs, OSType creator, OSType filetype, hdlfilenum *fnum) {
#ifdef MACVERSION	
		OSStatus	err = noErr;
//		char		s[256];
		
	setfserrorparam (fs);
	
	if (oserror (FSpCreate (fs, creator, filetype, smSystemScript))) /*failed to open the file for writing*/
		return (false);
	
	if (oserror (FSpOpenDF (fs, fsRdWrPerm, fnum))) {
		
		FSClose (*fnum);
		
		deletefile (fs);
		
		return (false); /*failed to open the file for writing*/
		}
	
	#if TARGET_API_MAC_CARBON
		{
		FSRef	myRef;
		err = FSpMakeFSRef (fs,&myRef);
		if (!err)
			FSSetNameLocked(&myRef);
		}
	#endif
		
	return (true);
#endif

#ifdef WIN95VERSION
	HANDLE fref;

	fref = (Handle) CreateFile (stringbaseaddress (fsname (fs)), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ,
												NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	*fnum = 0;

	if (fref == INVALID_HANDLE_VALUE) {
		
		winfileerror (fs);

		return (false);
		}

	*fnum = (hdlfilenum) fref;
	return (true);
#endif
	} /*filecreateandopen*/


boolean fileopenorcreate (const tyfilespec *fs, OSType creator, OSType filetype, hdlfilenum *fnum) {
#ifdef MACVERSION	
	/*
	open or create a file indicated by fs.  return with fnum set to the Mac 
	filesystem's file number for the file.
	
	if we open a file, we takes what we get in the creator and filetype department.
	however, if we create a file it's of the indicated filetype and creator.
	*/
	
	if (FSpOpenDF (fs, fsRdWrPerm, fnum) == noErr) {
		/*file exists and is open*/
		#if TARGET_API_MAC_CARBON
			FSRef	myRef;
			if (FSpMakeFSRef (fs,&myRef))
				FSSetNameLocked(&myRef);
		#else
			FSpSetNameLocked(&fs);
		#endif
		return (true);
	}
	return (filecreateandopen (fs, creator, filetype, fnum));
#endif

#ifdef WIN95VERSION
	HANDLE fref;
	DWORD dAccess;
	char fn[300];

	dAccess = GENERIC_READ | GENERIC_WRITE;

	copystring (fsname (fs), fn);
	
	nullterminate (fn);

	fref = (Handle) CreateFile (stringbaseaddress(fn), dAccess, FILE_SHARE_READ,
												NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	*fnum = 0;

	if (fref == INVALID_HANDLE_VALUE) {
		
		winfileerror (fs);

		return (false);
		}

	*fnum = (hdlfilenum) fref;
	return (true);
#endif
	} /*fileopenorcreate*/


boolean opennewfile (const tyfilespec *fs, OSType creator, OSType filetype, hdlfilenum *fnum) {
	
	boolean flfolder;
	
	if (fileexists (fs, &flfolder)) { /*file exists, delete it*/
		//WriteToConsole("We're deleting a file that already exists. No idea why.");
		if (!deletefile (fs))
			return (false);
		}
	
	return (filecreateandopen (fs, creator, filetype, fnum));
	} /*opennewfile*/


boolean openfile (const tyfilespec *fs, hdlfilenum *fnum, boolean flreadonly) {
	
#ifdef MACVERSION
	/*
	5/23/91 dmb: make sure we clear fnum on error; if file is already open, 
	FSOpen will set fnum to the existing channel
	
	4.1b4 dmb: added flreadonly paramater
	*/
	
	OSErr errcode;
	OSErr err = noErr;
	short perm;
	
	setfserrorparam (fs); /*in case error message takes a filename parameter*/
	
	if (flreadonly)
		perm = fsRdPerm;
	else
		perm = fsRdWrPerm;

	errcode = FSpOpenDF (fs, perm, fnum);
	
	if (oserror (errcode)) {
		
		*fnum = 0;
		
		return (false);
		}
	#if TARGET_API_MAC_CARBON
		{
		FSRef	myRef;
		err = FSpMakeFSRef (fs,&myRef);
		if (!err)
			err = FSSetNameLocked(&myRef);
		}
	#else
		err = FSpSetNameLocked(fs);
	#endif
	
	if (err!=noErr){
		//sprintf(s,"error locking file name: %d",err);
		//WriteToConsole(s);
	}
	
	return (true);
#endif

#ifdef WIN95VERSION
	HANDLE fref;
	DWORD dAccess;
	char fn[300];

	if (flreadonly)
		dAccess = GENERIC_READ;
	else
		dAccess = GENERIC_READ | GENERIC_WRITE;

	copystring (fsname (fs), fn);

	nullterminate (fn);

	fref = (Handle) CreateFile (stringbaseaddress(fn), dAccess, FILE_SHARE_READ,
												NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	*fnum = 0;

	if (fref == INVALID_HANDLE_VALUE) {
		
		winfileerror (fs);

		return (false);
		}

	*fnum = (hdlfilenum) fref;
	return (true);
#endif
	} /*openfile*/
	
/*******

From MoreFilesExtras. 
I'm going to use this to yank the fsspec back out from a refnum,
so that I can clear the finder name lock from the file prior to closing it.

*******/
	
	
boolean closefile (hdlfilenum fnum) {
#ifdef MACVERSION	
	OSStatus	err = noErr;
//	char		s[256];
		
	/*
	dmb 8/20/90:  check for 0.
	*/
	
	if (fnum != 0) {
		#if TARGET_API_MAC_CARBON
			FSRef myRef;
			err = FileRefNumGetFSRef (fnum, &myRef);
			if (err == noErr) {
				FSClearNameLocked (&myRef);
				}
		#else
			FSSpec fs;
			err = FSpGetFileLocation (fnum,&fs);
			if (err==noErr) {
				FSpClearNameLocked (&fs);
				}
		#endif
			else {
				//sprintf(s,"Error is %d",err);
				//WriteToConsole(s);
				}
				
		return (!oserror (FSClose (fnum)));
		}
	
	return (true);
#endif

#ifdef WIN95VERSION
	if (fnum != 0) {
		
		if (!CloseHandle (fnum)) {
			
			winerror ();

			return (false);
			}
		}
	
	return (true);
#endif
	} /*closefile*/


boolean deletefile (const tyfilespec *fs) {
	
	/* 
	2.1b2 dmb: new filespec-based version

	5.0.1 dmb: always setsferror param
	*/
	
	#ifdef MACVERSION
		setfserrorparam (fs); /*in case error message takes a filename parameter*/
		#if TARGET_API_MAC_CARBON
			{
			FSRef	myRef;
			if (FSpMakeFSRef (fs,&myRef))
				FSClearNameLocked(&myRef);
			}
		#else
			FSpClearNameLocked(fs);
		#endif
		return (!oserror (FSpDelete (fs)));
	#endif

	#ifdef WIN95VERSION
		char fn[300];
		boolean fldeletefolder;

		setfserrorparam (fs); /*in case error message takes a filename parameter*/
		
		copystring (fsname (fs), fn);

		cleanendoffilename(fn);

		nullterminate (fn);

		if (!fileisfolder (fs, &fldeletefolder))
			return (false);

		if (fldeletefolder) {
			
			if (RemoveDirectory (stringbaseaddress(fn)))
				return (true);
			
			goto error;
			}

		if (DeleteFile (stringbaseaddress(fn)))
			return (true);

	error:
		winerror ();

		return (false);
	#endif
	} /*deletefile*/
	


boolean fileexists (const tyfilespec *fs, boolean *flfolder) {
#ifdef MACVERSION	
	/*
	we figure if we can get info on it, it must exist.
	
	can't call filegetinfo because it has an error message if the file 
	isn't found.
	
	7/1/91 dmb: special case empty string so we don't try to get info about 
	the default volume and return true.
	
	2.1b2 dmb: filespec implementation
	
	5.0.2b15 rab: return false to non-existent volume specs.
	*/
	
	CInfoPBRec pb;
	
	*flfolder = false;
	
	if (isemptystring (fsname (fs)))
		return (false);
	
	clearbytes (&pb, sizeof (pb));
	
	pb.hFileInfo.ioNamePtr = (StringPtr) fsname (fs);
	
	pb.hFileInfo.ioVRefNum = (*fs).vRefNum;
	
	pb.hFileInfo.ioDirID = (*fs).parID;
	
	if (PBGetCatInfoSync (&pb) != noErr)
		return (false);
	
	*flfolder = foldertest (&pb);
	
	return (true);
#endif
#ifdef WIN95VERSION
	HANDLE ff;
	WIN32_FIND_DATA ffd;
	char fn[300];

	*flfolder = false;

	copystring (fsname (fs), fn);

	/*if ends with \ get ride of it... and handle the root*/

	cleanendoffilename (fn);

	nullterminate (fn);

	if (stringlength(fn) == 2) {
		if (isalpha(fn[1]) && fn[2] == ':') {
			*flfolder = true;
			return (fileisvolume(fs));
			}
		}

	ff = FindFirstFile (stringbaseaddress(fn), &ffd);

	if (ff == INVALID_HANDLE_VALUE)
		return (false);

	*flfolder = ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY?true:false;

	FindClose (ff);
	return (true);
#endif
	} /*fileexists*/


boolean filegetfilename (const tyfilespec *pfs, bigstring name) {

#ifdef MACVERSION
	/*
	filespecs for volumes have their strings set to the empty string.
	*/
	
	copystring (fsname (pfs), name);
	
	if (stringlength (name) > 0)
		return (true);
		
	return (filegetvolumename ((*pfs).vRefNum, name));
#endif

#ifdef WIN95VERSION
	lastword ((ptrstring) fsname (pfs), '\\', name);

	return (true);
#endif
	} /*filegetfilename*/

