
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

	#include <sys/param.h>
	
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
#include "frontier_strings.h"
#include "frontierwindows.h"
#include "file.h"
#include "shell.h"
#include "shell.rsrc.h"
#include "langinternal.h" /*for langbackgroundtask*/


boolean equalfilespecs ( const ptrfilespec fs1, const ptrfilespec fs2 ) {
	
	//
	// 2009-09-03 aradke: converted FSRef name field from CFStringRef to HFSUniStr255
	//
	// 2006-06-18 creedon: for Mac, FSRef-ized
	//
	// 5.0a25 dmb: until we set the volumeID to zero for all Win fsspecs, we must not compare them here
	//

	#ifdef MACVERSION
	
		CFStringRef cfname1, cfname2;
		boolean flequal;
		
		if (fs1->flags.flvolume != fs2->flags.flvolume)
			return (false);
	
		if (FSCompareFSRefs (&fs1->ref, &fs2->ref) != noErr)
			return (false);
			
		cfname1 = CFStringCreateWithCharacters (kCFAllocatorDefault, fs1->name.unicode, fs1->name.length);
		cfname2 = CFStringCreateWithCharacters (kCFAllocatorDefault, fs2->name.unicode, fs2->name.length);

		flequal = ( CFStringCompare (cfname1, cfname2, 0) == kCFCompareEqualTo );
		
		CFRelease (cfname1);
		CFRelease (cfname2);

		return (flequal);
		
	#endif
	
	#ifdef WIN95VERSION

		// if ((*fs1).volumeID != (*fs2).volumeID)
		//	return (false);
		
		return ( comparestrings ( fsname ( fs1 ), fsname ( fs2 ) ) == 0 );
		
	#endif

	} // equalfilespecs


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

boolean flushvolumechanges (const ptrfilespec fs, hdlfilenum fnum) {

	#ifdef MACVERSION
	
		# pragma unused(fnum)

		/*
		4.1b7 dmb: was -- FlushVol (nil, (*fs).vRefNum);
		
		now use PB call to do asynch flush
		*/
		
		ParamBlockRec *pb;
		
		pb = (ParamBlockRec *) NewPtrClear (sizeof (ParamBlockRec));
		
		FSCatalogInfo catalogInfo;
		OSErr err;
		
		err = FSGetCatalogInfo ( &fs->ref, kFSCatInfoVolume, &catalogInfo, NULL, NULL, NULL );
		
		(*pb).volumeParam.ioVRefNum = catalogInfo.volume;
		
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


static boolean filecreateandopen ( ptrfilespec fs, OSType creator, OSType filetype, hdlfilenum *fnum) {
	
	//
	// 2009-09-03 aradke: converted tyfilespec.name field from CFStringRef to HFSUniStr255
	//
	// 2006-09-15 creedon: for Mac, FSRef-ized
	//
	
	#ifdef MACVERSION
		
		FSRef fsref;
		OSErr err;
		
		setfserrorparam ( fs );
		
		err = FSCreateFileUnicode ( &( *fs ).ref, (*fs).name.length, (*fs).name.unicode, kFSCatInfoNone, NULL, &fsref, NULL );
		
		if ( oserror ( err ) )
			return (false);
		
		HFSUniStr255 dataforkname;
		
		err = FSGetDataForkName ( &dataforkname );
		
		if ( oserror ( err ) )
			return ( false );
		
		err = FSOpenFork ( &fsref, dataforkname.length, dataforkname.unicode, fsRdWrPerm, fnum );

		if ( oserror ( err ) ) {
			
			FSCloseFork (*fnum);	// FIXME: is *fnum actually valid if FSOpenFork failed?
			
			deletefile (fs);
			
			return (false); // failed to open the file for writing
			
			}
	
		FSCatalogInfo catalogInfo;
		
		err = FSGetCatalogInfo(&fsref, kFSCatInfoFinderInfo, &catalogInfo, NULL, NULL, NULL);
		
		if (oserror(err)) {
			return false;
		}
		
		((FileInfo *)&catalogInfo.finderInfo)->finderFlags |= kNameLocked;
		
		err = FSSetCatalogInfo(&fsref, kFSCatInfoFinderInfo, &catalogInfo);
		
		if (oserror(err)) {
			return false;
		}
			
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
	
	} // filecreateandopen


boolean opennewfile ( ptrfilespec fs, OSType creator, OSType filetype, hdlfilenum *fnum ) {

	//
	// 2006-09-13 creedon: for Mac, FSRef-ized
	//
	
	boolean flfolder;
	
	if ( fileexists ( fs, &flfolder ) ) { // file exists, delete it
	
		//WriteToConsole("We're deleting a file that already exists. No idea why.");
		
		if ( ! deletefile ( fs ) )
			return ( false );
		}
	
	return ( filecreateandopen ( fs, creator, filetype, fnum ) );
	
	} // opennewfile


boolean openfile ( const ptrfilespec fs, hdlfilenum *fnum, boolean flreadonly ) {
	
	//
	// 2006-06-26 creedon: for Mac, FSRef-ized
	//
	// 4.1b4 dmb: on Mac added flreadonly paramater
	//
	// 1991-05-23 dmb: on Mac make sure we clear fnum on error; if file is already open, 
	// FSOpen will set fnum to the existing channel
	//
		
	#ifdef MACVERSION
		
		FSRef fsref;
		short perm;
		
		setfserrorparam ( fs ); // in case error message takes a filename parameter
		
		if (flreadonly)
			perm = fsRdPerm;
		else
			perm = fsRdWrPerm;

		HFSUniStr255 dataforkname;
		
		if ( oserror ( FSGetDataForkName ( &dataforkname ) ) )
			return ( false );
		
		if ( oserror ( macgetfsref ( fs, &fsref ) ) )
			return ( false );
			
		if ( oserror ( FSOpenFork ( &fsref, dataforkname.length, dataforkname.unicode, perm, fnum ) ) ) {
			
			*fnum = 0;
			
			return (false);
			}
		
		FSCatalogInfo catalogInfo;
		
		OSErr err = FSGetCatalogInfo(&fsref, kFSCatInfoFinderInfo, &catalogInfo, NULL, NULL, NULL);
		
		if (oserror(err)) {
			return false;
		}
		
		((FileInfo *)&catalogInfo.finderInfo)->finderFlags |= kNameLocked;
		
		err = FSSetCatalogInfo(&fsref, kFSCatInfoFinderInfo, &catalogInfo);
		
		if (oserror(err)) {
			return false;
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
	
	} // openfile


boolean closefile (hdlfilenum fnum) {

	//
	// 2006-06-22 creedon: for Mac FSRef-ized
	//
	// 1990-08-20 dmb: on Mac check for 0.
	//
	// From MoreFilesExtras.   I'm going to use this to yank the fsspec back out from a refnum, so that I can clear the finder
	// name lock from the file prior to closing it.
	//
		
	#ifdef MACVERSION
	
		OSStatus err = noErr;
			
		if ( fnum != 0 ) {

			FSRef fsr;
			
			err = FSGetForkCBInfo(fnum, 0, NULL, NULL, NULL, &fsr, NULL);
			
			if ( err == noErr ) {
				FSCatalogInfo catalogInfo;
				
				err = FSGetCatalogInfo(&fsr, kFSCatInfoFinderInfo, &catalogInfo, NULL, NULL, NULL);
				
				if (oserror(err)) {
					return false;
				}
				
				((FileInfo *)&catalogInfo.finderInfo)->finderFlags &= ~kNameLocked;
				
				err = FSSetCatalogInfo(&fsr, kFSCatInfoFinderInfo, &catalogInfo);
				
				if (oserror(err)) {
					return false;
				}
				}
			else {
				//sprintf(s,"Error is %d",err);
				//WriteToConsole(s);
				}
			
			err = FSCloseFork ( fnum );
			
			return ( ! oserror ( err ) );
			
			}
		
		return ( true );
		
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
	
	} // closefile


boolean deletefile ( const ptrfilespec fs ) {
	
	//
	// 2006-06-26 creedon: for Mac, FSRef-ized
	//
	// 5.0.1 dmb: always setfserror param
	//
	// 2.1b2 dmb: on Mac new filespec-based version
	//
	
	#ifdef MACVERSION
	
		FSRef fsref;

		setfserrorparam ( fs ); // in case error message takes a filename parameter
		
		if ( oserror ( macgetfsref ( fs, &fsref ) ) )
			return ( false );

		FSCatalogInfo catalogInfo;
		
		OSErr err = FSGetCatalogInfo(&fsref, kFSCatInfoFinderInfo, &catalogInfo, NULL, NULL, NULL);
		
		if (oserror(err)) {
			return false;
		}
		
		((FileInfo *)&catalogInfo.finderInfo)->finderFlags |= kNameLocked;
		
		err = FSSetCatalogInfo(&fsref, kFSCatInfoFinderInfo, &catalogInfo);
		
		if (oserror(err)) {
			return false;
		}
	
		return ( ! oserror ( FSDeleteObject ( &fsref ) ) );
		
	#endif

	#ifdef WIN95VERSION
	
		char fn[300];
		boolean fldeletefolder;

		setfserrorparam (fs); // in case error message takes a filename parameter
		
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
	
	} // deletefile
	


boolean fileexists ( const ptrfilespec fs, boolean *flfolder ) {

	//
	// we figure if we can get info on it, it must exist.
	//
	// can't call filegetinfo because it has an error message if the file isn't found.
	//
	// 2009-09-03 aradke: converted tyfilespec.name field from CFStringRef to HFSUniStr255
	//
	// 2006-06-19 creedon:	Mac OS X bundles/packages are not considered folders
	//
	//				for Mac, FSRef-ized
	//
	// 5.0.2b15 rab: on Mac return false to non-existent volume specs.
	//
	// 2.1b2 dmb: on Mac filespec implementation
	//
	// 7/1/91 dmb: on Mac special case empty string so we don't try to get info about the default volume and return true.
	//
	
	#ifdef MACVERSION
	
		FSCatalogInfo catalogInfo;
		FSRef fsref;
	
		*flfolder = false;
		
		if (macgetfsref(fs, &fsref) != noErr)
			return (false);
		
		if (FSGetCatalogInfo(&fsref, kFSCatInfoNodeFlags, &catalogInfo, NULL, NULL, NULL) != noErr)
			return (false);
		
		*flfolder = ((catalogInfo.nodeFlags & kFSNodeIsDirectoryMask) != 0);

		/* Mac OS X bundles/packages are not considered folders */ {
	
			boolean flisapplication, flisbundle;
			
			LSIsApplication (&fsref, &flisapplication, &flisbundle);

			if (flisapplication || flisbundle)
				*flfolder = false;
			}
			
		return (true);
		
	#endif
	
	#ifdef WIN95VERSION
	
		HANDLE ff;
		WIN32_FIND_DATA ffd;
		char fn[300];

		*flfolder = false;

		copystring (fsname (fs), fn);

		/*if ends with \ get rid of it... and handle the root*/

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
	
	} // fileexists

