
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

#include "error.h"
#include "memory.h"
#include "strings.h"
#include "ops.h"
#include "file.h"
#include "launch.h" // 2005-07-18 creedon

#ifdef MACVERSION

	#include "MoreFilesX.h" // 2006-05-31 creedon
	#include <CoreFoundation/CFString.h> // 2006-08-10 creedon
	#include <sys/param.h> // 2006-10-16 creedon

#endif // MACVERSION


#ifdef MACVERSION

	static tyfilespec fsdefault; // we maintain our own default directory


	boolean directorytopath ( const ptrfilespec fs, bigstring path ) {

		//
		// 2006-09-09 creedon: FSRef-ized
		//			       another way might be to use FSRefMakePath and then push insert the volume name on it
		//

		OSErr err;
		OSStatus status;
		FSRef volumesfsref;
		
		status = FSPathMakeRef ((UInt8 *)"/Volumes", &volumesfsref, NULL);
		
		if ((status == noErr) && (FSCompareFSRefs (&fs->ref, &volumesfsref) == noErr)) {	/* bail if fs is Volumes directory */
		
			setemptystring (path);
			
			return (true);
			}

		CFMutableStringRef ioPath = CFStringCreateMutable (NULL, 0);
		FSCatalogInfo catalogInfo;
		FSRef fsrefnomad = fs->ref;
		HFSUniStr255 names[100];
		int i, n;
		UniChar inSepChar = ':';
		
		err = noErr;

		clearbytes (&catalogInfo, longsizeof (catalogInfo));

		for (n = 0; err == noErr && catalogInfo.nodeID != fsRtDirID && n < 100; n++) {

			err = FSGetCatalogInfo (&fsrefnomad, kFSCatInfoNodeID, &catalogInfo, &names[n], NULL, &fsrefnomad);
			}
			
		if (err != noErr)
			return (false);
			
		for (i = n - 1; i >= 0; --i) {

			CFStringAppendCharacters (ioPath, names[i].unicode, names[i].length);
			
			CFStringAppendCharacters (ioPath, &inSepChar, 1);
			}
		
		boolean success = CFStringGetPascalString (ioPath, path, 256, kCFStringEncodingMacRoman);
		CFRelease(ioPath); //PBS 2/28/11: fixed memory leak
		return success;
	} // directorytopath

#endif


boolean filegetdefaultpath ( ptrfilespec fs ) {

	//
	// 2006-06-25 creedon: for Mac, FSRef-ized
	//
	
	#ifdef MACVERSION
	
		*fs = fsdefault;
		
		return ( true );

	#endif

	#ifdef WIN95VERSION
	
		DWORD sz;

		sz = GetCurrentDirectory (257, stringbaseaddress (fsname (fs)));
		
		if (sz == 0) {
			
			oserror (GetLastError ());
			
			return (false);
			}
		
		strcat (stringbaseaddress (fsname (fs)), "\\");

		setstringlength(fsname (fs), sz + 1);
		
		return (true);
		
	#endif		
	} /*filegetdefaultpath*/


boolean filesetdefaultpath ( const ptrfilespec fs ) {

	//
	// 2006-06-18 creedon: for Mac, FSRef-ized
	//
	
	#ifdef MACVERSION

		setfserrorparam ( fs );
		
		fsdefault = *fs;
		
		return (true);
		
	#endif

	#ifdef WIN95VERSION
	
		if (isemptystring (fsname (fs)))
			return (true);
		
		if (!SetCurrentDirectory (stringbaseaddress(fsname (fs)))) {
			
			oserror (GetLastError ());
			
			return (false);
			}

		return (true);
		
	#endif
	
	} // filesetdefaultpath


boolean filespectopath (const ptrfilespec fs, bigstring bspath) {
	
	//
	// 2006-09-09 creedon: for Mac, FSRef-ized
	//
	// 2.1a7 dmb: if it's a null filespec, return the empty string
	//
	// 1991-12-17 dmb: don't check for folderness if file doesn't exist-- we dont want to generate any errors here
	//
	// 1991-10-14 dmb: make sure folder paths end in :
	//
	// 1991-06-28 dmb:	when you resolve an alias of a volume, the fsspec has a parent dirid of 1.  we catch this as a
	//				special case here, and return the empty string as the path
	//
	
	#ifdef MACVERSION
	
		boolean flfolder;
		
		setemptystring (bspath);

		if (!directorytopath (fs, bspath))
			return (false);
		
		if (!fs->flags.flvolume) {
			
			bigstring bsfile;
			
			fsnametobigstring (&fs->name, bsfile);
			
			pushstring (bsfile, bspath);
			}
		
		if (fileexists (fs, &flfolder))
			if (flfolder)
				assurelastchariscolon (bspath);
		
		return (true);
		
	#endif

	#ifdef WIN95VERSION
	
		// 5.0d12 dmb: use GetFullPath to clean up 8.3 names
		char * fileptr;
		
		copyptocstring (fsname (fs), bspath);
		
		GetFullPathName (bspath, lenbigstring, bspath, &fileptr);

		convertcstring (bspath);
		
		nullterminate (bspath);

		return (true);
		
	#endif
	
	} // filespectopath


#ifdef MACVERSION

OSStatus pathtofsref ( bigstring bspath, FSRef *ref ) {

	/*
	2009-08-30 aradke: mac-only helper function for converting from a pascal string (path) to an FSRef
	*/
	Boolean filePath = lastchar(bspath) == ':';
	
	CFStringRef hfsPathStr = CFStringCreateWithPascalString(kCFAllocatorDefault, bspath, kCFStringEncodingMacRoman);
	
	CFURLRef fileURL = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, hfsPathStr, kCFURLHFSPathStyle, filePath);
	
	CFRelease(hfsPathStr);
	
	CFURLGetFSRef(fileURL, ref);
	
	CFRelease(fileURL);
	
	return noErr;

	} /*pathtofsref*/

#endif


boolean pathtofilespec ( bigstring bspath, ptrfilespec fs ) {

    //
    // 2010-01-24 creedon: fix for relative paths not working on mac,
    //                     bsfullpath was ending up with :: in it as well as
    //                     the full path to the application, see initfsdefault
    //                     function
    //
	// 2009-08-30 aradke: refactored mac version to make it easier to understand.
	//			fixed bug where a bspath containing a non-existing volume name was accepted as valid,
	//			e.g. filespec("foobar:"), thus deviating from previous behaviour.
	//
	// 2006-10-16 creedon: for Mac, FSRef-ized
	//
	// 5.0d8 dmb: clear fs first thing
	//
	// 2.1b2 dmb: use new fsdefault for building filespec. note that if bspath
    //            isn't a partial path, the vref and dirid will be ignored.
	//
	// 2.1b2 dmb: added special case for empty string.  also, added drive
    //            number interpretation here.
	//
	// 1993-06-11 dmb: if FSMakeFSSpec returns fnfErr, the spec is cool (but
    //                 file doesn't exist)
	//
	// 1991-012-17 dmb: dont append path to default directory if it's a full
    //                  path
	//
	// 1991-07-05 dmb: use FSMakeFSSpec if it's available.  since it only
    //                 returns noErr if the file exists, and we want to handle
	//                 non-existant files, we don't give up right away.
	//
	
	#ifdef MACVERSION
		FSRef fsr;
		bigstring bspathtmp, bsfullpath, bsfile, bsfolder;
		short ix = 1;
		boolean flvolume = false;
	#endif

	#ifdef WIN95VERSION
		bigstring bsfolder;
	#endif

	clearbytes ( fs, sizeof ( *fs ) );

	if ( isemptystring ( bspath ) )
		return ( true );
		
	#ifdef MACVERSION

		// create cleaned-up full path representation of our input suitable for pathtosref
	
		copystring ( bspath, bspathtmp );

		cleanendoffilename ( bspathtmp );
		
		if ( scanstring ( ':', bspath, &ix ) && ( ix > 1 ) ) {
		
			// contains a colon but doesn't start with one, so it must be a full path
			
			if ( ix == stringlength ( bspath ) )	// the colon we found is the last char, so bspath is a volume name
				flvolume = true;
                
			copystring ( bspathtmp, bsfullpath );
			}
            
		else {
		
			// it's a partial path, prefix with default directory (see initfsdefault)
		
			if ( ! filespectopath ( &fsdefault, bsfullpath ) )	// get path of default directory
				return ( false );
                
           // delete first path separator if partial path begins with one because bsfullpath always ends with one
           
			if ( bspathtmp [ 1 ] == chpathseparator )
				deletefirstchar ( bspathtmp );
                
			pushstring ( bspathtmp, bsfullpath );	// append partial path
			}
            
		// now see if the full path resolves 
			
		if ( pathtofsref ( bsfullpath, &fsr ) == noErr ) {

			return ( macmakefilespec ( &fsr, fs ) == noErr );
			}
			
		// full path did not resolve but we actually only require the parent folder to exist
		
		if ( ! flvolume ) {		// volumes don't have a parent folder
		
			filefrompath ( bsfullpath, bsfile );
			
			folderfrompath ( bsfullpath, bsfolder );

			if ( pathtofsref ( bsfolder, &fsr ) == noErr ) {
				
				clearfilespec ( fs );
				
				fs->ref = fsr;
				
				bigstringtofsname ( bsfile, &fs->name );
			
				return ( true );
				}
			}
		
		return ( false );
		
	#endif

	#ifdef WIN95VERSION
	
		copystring (bspath, fsname (fs));

		folderfrompath (bspath, bsfolder);

		if ((isemptystring (bsfolder)) && (! fileisvolume(fs))) {

			filegetdefaultpath (fs);

			pushstring (bspath, fsname (fs));
			}
		
		nullterminate (fsname (fs));
		
		return (true);

	#endif

	} // pathtofilespec


boolean setfsfile ( ptrfilespec fs, bigstring bsfile ) {

	//
	// 2006-10-18 creedon: for Mac, FSRef-ized
	//
	// 2004-10-26 aradke: Since the getmacfileinfo/foldertest gymnastics do not seem to fit any particular purpose and since
	//			     none of our callers seem to rely it since they usually pass in a file rather than a directory, I
	//			     commented it out.
	// 
	//			     The only time we get called with a directory is apparently by shellopendefaultfile on startup in the
	//			     Carbon/Mach-O build. getapplicationfilespec returns a directory in that case and the code below
	//			     somehow screwed up when called to set the filename to Frontier.root so that it wouldn't be found.
	//
	
	#ifdef MACVERSION
		
		bigstringtofsname (bsfile, &fs->name);
		
		return ( true );
		
	#endif

	#ifdef WIN95VERSION
	
		bigstring bsfolder;
		
		folderfrompath (fsname (fs), bsfolder);
		
		pushstring (bsfile, bsfolder);

		copystring (bsfolder, fsname (fs));

		nullterminate (fsname (fs));

		return (true);
		
	#endif
	
	} // setfsfile


boolean getfsfile ( const ptrfilespec fs, bigstring bsfile ) {

	//
	// 2007-08-01 creedon: check FSRefGetNameStr255 return false if not true
	//
	// 2006-06-18 creedon: for Mac, FSRef-ized
	//
	
	#ifdef MACVERSION
		
		macgetfilespecnameasbigstring ( fs, bsfile );
				
		if ( stringlength ( bsfile ) > 0 )
			return ( true );
			
		long vnum;
		
		getfsvolume ( fs, &vnum );
		
		return ( filegetvolumename ( vnum, bsfile ) );
		
	#endif
	
	#ifdef WIN95VERSION
	
		lastword ((ptrstring) fs -> fullSpecifier, '\\', bsfile);

		return (true);
		
	#endif
	} // getfsfile


boolean getfsvolume ( const ptrfilespec fs, long *vnum ) {
	
	//
	// 2006-07-12 creedon: for Mac, FSRef-ized
	//
	// 5.1.5b11 dmb:	get the volume that is actually specified in the fspec.
	//
	//					don't expand partial paths using the default directory.
	//

	#ifdef MACVERSION
	
		FSCatalogInfo catalogInfo;
		OSErr err = FSGetCatalogInfo ( &fs->ref, kFSCatInfoVolume, &catalogInfo, NULL, NULL, NULL );
		
		if ( catalogInfo.volume == 0 )
			return ( false );
		
		*vnum = catalogInfo.volume;

		return ( err == noErr );
		
	#endif
	
	#ifdef WIN95VERSION

		*vnum = 0;
	
		return ( true );
		
	#endif
	
	} // getfsvolume


void initfsdefault (void) {

	//
    // 2010-01-24 creedon: assign fsdefault the path that the application is
    //                     in, not the path of the application itself,
    //                     restoring previous behavior, fix for problem with
    //                     relative paths not working
    //
	// 2006-06-18 creedon: FSRef-ized
	//
	// 2005-07-18 creedon, karstenw: created
	//
    
	#ifdef MACVERSION
	
		getapplicationfilespec ( nil, &fsdefault );
		
        macgetfilespecparent ( &fsdefault, &fsdefault );
        
	#endif
	
	} /* initfsdefault */

