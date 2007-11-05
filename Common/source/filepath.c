
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


#define flaux false /*if true, we're running under the A/UX operating system*/


#ifdef MACVERSION

	static tyfilespec fsdefault; // we maintain our own default directory


	boolean directorytopath ( const ptrfilespec fs, bigstring path ) {

		//
		// 2006-09-09 creedon: FSRef-ized
		//			       another way might be to use FSRefMakePath and then push insert the volume name on it
		//

		OSErr err;
		OSStatus status;
		
		/* bail if fs is Volumes directory */ {
		
			FSRef volumesfsref;
			OSErr err;
			
			status = FSPathMakeRef ( "/Volumes", &volumesfsref, NULL );
			
			err = FSCompareFSRefs ( &( *fs ).fsref , &volumesfsref );
			
			if ( err == noErr ) {
			
				setemptystring ( path );
				
				return ( true );
				
				}
				
			} // bail

		CFMutableStringRef ioPath = CFStringCreateMutable ( NULL, 0 );
		FSCatalogInfo catalogInfo;
		FSRef localRef = ( *fs ).fsref;
		HFSUniStr255 names [ 100 ];
		int i, n;
		UniChar inSepChar = ':';
		
		err = noErr;

		clearbytes ( &catalogInfo, longsizeof ( catalogInfo ) );

		for ( n = 0 ; err == noErr && catalogInfo.nodeID != fsRtDirID && n < 100 ; n++ )
			err = FSGetCatalogInfo ( &localRef, kFSCatInfoNodeID, &catalogInfo, &names [ n ], NULL, &localRef );

		if ( err != noErr )
			return ( false );
			
		for ( i = n - 1; i >= 0; --i ) {
			CFStringAppendCharacters ( ioPath, names [ i ].unicode, names [ i ].length );

			// if ( i > 0 )
			
			CFStringAppendCharacters ( ioPath, &inSepChar, 1 );
			}
		
		return ( CFStringGetPascalString ( ioPath, path, 256, kCFStringEncodingMacRoman ) );

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
	
		bigstring bs;
		boolean flfolder;
		FSCatalogInfo catalogInfo;
		tyfilespec fsp, fst = *fs;
		HFSUniStr255 outName;
		OSErr err;
		
		( void ) extendfilespec ( &fst, &fst );
		
		err = FSGetCatalogInfo ( &fst.fsref, kFSCatInfoParentDirID | kFSCatInfoVolume, &catalogInfo, &outName, NULL,
			&fsp.fsref );
			
		if ( err != noErr )
			return ( false );
		
		setemptystring ( bspath );
		
		if ( catalogInfo.parentDirID != fsRtParID ) // it's not a volume so lets get the directory path
			if ( ! directorytopath ( &fsp, bspath ) )
				return ( false );
		
		HFSUniStr255ToStr255 ( &outName, bs );
		
		pushstring ( bs, bspath );
		
		if ( fst.path != NULL ) {
			
			pushchar ( ':', bspath );
			
			CFStringRefToStr255 ( fst.path, bs );
			
			pushstring ( bs, bspath );

			return ( true );
			
			}

		if ( fileexists ( &fst, &flfolder ) )
			if ( flfolder )
				assurelastchariscolon ( bspath );
		
		return ( true );
		
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


boolean pathtofilespec ( bigstring bspath, ptrfilespec fs ) {
	
	//
	// 2007-11-05 creedon: for Windows, call GetLongPathName to always store long path name
	//
	// 2006-10-16 creedon: for Mac, FSRef-ized
	//
	// 5.0d8 dmb: clear fs first thing
	//
	// 2.1b2 dmb: use new fsdefault for building filespec. note that if bspath isn't a partial path, the vref and dirid will be
	//		   ignored.
	//
	// 2.1b2 dmb: added special case for empty string.  also, added drive number interpretation here.
	//
	// 1993-06-11 dmb: if FSMakeFSSpec returns fnfErr, the spec is cool (but file doesn't exist)
	//
	// 1991-012-17 dmb: dont append path to default directory if it's a full path
	//
	// 1991-07-05 dmb: use FSMakeFSSpec if it's available.  since it only returns noErr if the file exists, and we want to handle
	//		     non-existant files, we don't give up right away.
	//
	
	bigstring bsfolder;
	
	#ifdef WIN95VERSION
	
		char fn [ 300 ];
		
	#endif
	
	clearbytes ( fs, sizeof ( *fs ) );

	if ( isemptystring ( bspath ) )
		return ( true );
		
	#ifdef MACVERSION
	
		FSRef fsr;
		HFSUniStr255 name;
		OSErr err;
		OSStatus status;
		bigstring bs, bspathtmp;
		short ix = 1, ixslashpos = 0, slashpos [ 255 ];
		
		copystring ( bspath, bspathtmp );

		cleanendoffilename ( bspathtmp );
		
		if ( scanstring ( ':', bspath, &ix ) && ( ix > 1 ) ) { // full path, includes a colon, not the first character
		
			bigstring bsfile, bsfullpath;
		 
			copystring ( bspathtmp, bsfullpath );
				
			insertstring ( BIGSTRING ( "\x09" ":Volumes:" ), bsfullpath );

			copystring ( bsfullpath,  bs );

			/* convert to / delimited path */ {
			
				while ( scanstring ( '/', bs, &ix ) )
					slashpos [ ixslashpos++ ] = ix++;
				
				stringreplaceall ( ':', '/', bs );
				
				while ( ixslashpos > 0 )
					setstringcharacter ( bs, slashpos [ --ixslashpos ] - 1, ':' );
				}
			
			/* convert from Mac Roman to UTF-8 */ {
			
				CFStringRef csr = CFStringCreateWithPascalString ( kCFAllocatorDefault, bs,
					kCFStringEncodingMacRoman );
			
				CFStringGetCString ( csr, ( char * ) bs, sizeof ( bs ), kCFStringEncodingUTF8 );
				
				CFRelease ( csr );
		
				}
			
			status = FSPathMakeRef ( bs, &fsr, NULL );

			if ( status == noErr ) {
			
				err = FSGetCatalogInfo ( &fsr, kFSCatInfoNone, NULL, &name, NULL, &fsr );
				
				( *fs ).fsref = fsr;
				
				( *fs ).path = CFStringCreateWithCharacters ( kCFAllocatorDefault, name.unicode, name.length );
				
				return ( true );
				
				} // if
			
			filefrompath ( bsfullpath, bsfile );
			
			folderfrompath ( bsfullpath, bsfolder );
			
			/* convert to / delimited path */ {
			
				ix = 1;
				ixslashpos = 0;
					
				while ( scanstring ( '/', bsfolder, &ix ) )
					slashpos [ ixslashpos++ ] = ix++;
				
				stringreplaceall ( ':', '/', bsfolder );
				
				while ( ixslashpos > 0 )
					setstringcharacter ( bsfolder, slashpos [ --ixslashpos ] - 1, ':' );
				}
			
			/* convert from Mac Roman to UTF-8 */ {
			
				CFStringRef csr = CFStringCreateWithPascalString ( kCFAllocatorDefault, bsfolder,
					kCFStringEncodingMacRoman );
			
				CFStringGetCString ( csr, ( char * ) bsfolder, sizeof ( bsfolder ), kCFStringEncodingUTF8 );
				
				CFRelease ( csr );
		
				}
			
			status = FSPathMakeRef ( bsfolder, &fsr, NULL );

 			if ( status == noErr ) {
			
				( *fs ).fsref = fsr;
				
				( *fs ).path = CFStringCreateWithPascalString ( kCFAllocatorDefault, bsfile,
					kCFStringEncodingMacRoman );
			
				return ( true );
				
				} // if
				
			} // end full path
			
		else { // partial path
		
			bigstring bspartialpath;
			tyfilespec fst;
			
			copystring ( bspathtmp, bspartialpath );
			
			if ( bspartialpath [ 1 ] != chpathseparator )
				insertchar ( chpathseparator, bspartialpath );
			
			( void ) extendfilespec ( &fsdefault, &fst );
			
			status = FSRefMakePath ( &fst.fsref, ( UInt8 * ) bs, 256 ); // bs is now a c string
			
			convertcstring ( bs ); // bs is now a bigstring
			
			/* convert bs to colon delimited path */ {
			
				ix = 1;
				ixslashpos = 0;
				
				while ( scanstring ( ':', bs, &ix ) )
					slashpos [ ixslashpos++ ] = ix++;
				
				stringreplaceall ( '/', ':', bs );
				
				while ( ixslashpos > 0 )
					setstringcharacter ( bs, slashpos [ --ixslashpos ] - 1, '/' );
				}
			
			insertstring ( bs, bspartialpath ); // we now have a full path
			
			copystring ( bspartialpath, bs );
			
			 /* convert bs to / delimited path */ {
			 
				 ix = 1;
				 ixslashpos = 0;
				
				 while ( scanstring ( '/', bs, &ix ) )
					slashpos [ ixslashpos++ ] = ix++;
				
				stringreplaceall ( ':', '/', bs );
				
				while ( ixslashpos > 0 )
					setstringcharacter ( bs, slashpos [ --ixslashpos ] - 1, ':' );
				}
			
			/* convert from Mac Roman to UTF-8 */ {
			
				CFStringRef csr = CFStringCreateWithPascalString ( kCFAllocatorDefault, bsfolder,
					kCFStringEncodingMacRoman );
			
				CFStringGetCString ( csr, ( char * ) bsfolder, sizeof ( bsfolder ), kCFStringEncodingUTF8 );
				
				CFRelease ( csr );
		
				}
			
			status = FSPathMakeRef ( bs, &fsr, NULL );

			if ( status == noErr ) {
			
				err = FSGetCatalogInfo ( &fsr, kFSCatInfoNone, NULL, &name, NULL, &fsr );

				( *fs ).fsref = fsr;
				
				( *fs ).path = CFStringCreateWithCharacters ( kCFAllocatorDefault, name.unicode, name.length );
			
				return ( true );
				
				} // if
				
			/* try parent of path */ {
			
				bigstring bsfile;
			
				filefrompath ( bspartialpath, bsfile );
				
				folderfrompath ( bspartialpath, bsfolder );
				
				 /* convert to slash delimited path */ {
				 
					 ix = 1;
					 ixslashpos = 0;
					
					 while ( scanstring ( '/', bsfolder, &ix ) )
						slashpos [ ixslashpos++ ] = ix++;
					
					stringreplaceall ( ':', '/', bsfolder );
					
					while ( ixslashpos > 0 )
						setstringcharacter ( bsfolder, slashpos [ --ixslashpos ] - 1, ':' );
					}
				
				/* convert from Mac Roman to UTF-8 */ {
				
					CFStringRef csr = CFStringCreateWithPascalString ( kCFAllocatorDefault, bsfolder,
						kCFStringEncodingMacRoman );
				
					CFStringGetCString ( csr, ( char * ) bsfolder, sizeof ( bsfolder ), kCFStringEncodingUTF8 );
					
					CFRelease ( csr );
		
					}
				
				status = FSPathMakeRef ( bsfolder, &fsr, NULL );

				if ( status == noErr ) {
				
					( *fs ).fsref = fsr;
					
					( *fs ).path = CFStringCreateWithPascalString ( kCFAllocatorDefault, bsfile,
						kCFStringEncodingMacRoman );
				
					return ( true );
					
					} // if
					
				} // end try parent of path 
				
			} // end partial path
		
		return ( false );
		
	#endif

	#ifdef WIN95VERSION
	
		nullterminate ( bspath );
		
		if ( GetLongPathName ( stringbaseaddress ( bspath ), fn, 298 ) )	
			copyctopstring ( fn, bspath );
		
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
	
		if ( ( *fs ).path != NULL )
			CFRelease ( ( *fs ).path );
		
		( *fs ).path = CFStringCreateWithPascalString ( kCFAllocatorDefault, bsfile, kCFStringEncodingMacRoman );
		
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
	
		if ( ( *fs ).path != NULL ) {

			if ( CFStringRefToStr255 ( ( *fs ).path, bsfile ) )
				return ( true );
			}
		else {
		
			if ( ! FSRefGetNameStr255 ( &( *fs ).fsref, bsfile ) )
				return ( false );
			}
				
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
		OSErr err = FSGetCatalogInfo ( &( *fs ).fsref, kFSCatInfoVolume, &catalogInfo, NULL, NULL, NULL );
		
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
	// 2006-06-18 creedon: FSRef-ized
	//
	// 2005-07-18 creedon, karstenw: created
	//

	#ifdef MACVERSION
	
		getapplicationfilespec ( nil, &fsdefault );
		
		( void ) getfilespecparent ( &fsdefault );
		
	#endif
	
	} /* initfsdefault */

