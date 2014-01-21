/*
	File:		HelloWorldTool.c
	
	Contains:	Contains the tool (shell) source which parses command line arguments and 
				calls FSCopyObject.

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
				("Apple") in consideration of your agreement to the following terms, and your
				use, installation, modification or redistribution of this Apple software
				constitutes acceptance of these terms.  If you do not agree with these terms,
				please do not use, install, modify or redistribute this Apple software.

				In consideration of your agreement to abide by the following terms, and subject
				to these terms, Apple grants you a personal, non-exclusive license, under Apple’s
				copyrights in this original Apple software (the "Apple Software"), to use,
				reproduce, modify and redistribute the Apple Software, with or without
				modifications, in source and/or binary forms; provided that if you redistribute
				the Apple Software in its entirety and without modifications, you must retain
				this notice and the following text and disclaimers in all such redistributions of
				the Apple Software.  Neither the name, trademarks, service marks or logos of
				Apple Computer, Inc. may be used to endorse or promote products derived from the
				Apple Software without specific prior written permission from Apple.  Except as
				expressly stated in this notice, no other rights or licenses, express or implied,
				are granted by Apple herein, including but not limited to any patent rights that
				may be infringed by your derivative works or by other works in which the Apple
				Software may be incorporated.

				The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
				WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
				WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
				PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
				COMBINATION WITH YOUR PRODUCTS.

				IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
				CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
				GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
				ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
				OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
				(INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
				ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	Copyright © 2003-2004 Apple Computer, Inc., All Rights Reserved
*/

#include <CoreServices/CoreServices.h>
#include <stdio.h>
#include "FSCopyObject.h"

#define kNeedsHelpErr			1
#define kInvalidArgumentErr		2
#define kInvalidDestinationErr	3
#define kCouldNotCreateCFString	4
#define kCouldNotGetStringData	5

#define MAX_PATH				1024

static void TestFSCopyObject( DupeAction dupeAction, const HFSUniStr255 *name, const FSRef *sourceRef, const FSRef *destRef );

static void	TestFSDeleteObjects( const FSRef *sourceRef );

static pascal OSErr MyFilterProc(	Boolean				containerChanged,
									ItemCount			currentLevel,
									OSErr				currentOSErr,
									const FSCatalogInfo	*catalogInfo,
									const FSRef			*ref,
									const FSSpec		*spec,
									const HFSUniStr255	*name,
									void				*yourDataPtr );

static OSErr MyFSPathMakeRef( const unsigned char *path, FSRef *ref );

static OSErr ConvertCStringToHFSUniStr(const char* cStr, HFSUniStr255 *uniStr);

int main (int argc, const char * argv[])
{
	HFSUniStr255	uniName,
					*uniNamePtr = NULL;
	FSRef			sourceRef,
					destRef;
	OSErr			err = (argc > 1) ? noErr : kInvalidArgumentErr;
	unsigned char	*srcPath		= NULL,
					*dstPath		= NULL,
					*nameStr		= NULL;
	char		 	dupeAction		= 'x';
	int			 	i, j;

	printf("\n");

													/* parse the arg list */
	for( i = 1, j = 2; i < argc && j <= argc && err == noErr; i += 2, j += 2)
	{
		if( strcmp("-a", argv[i]) == 0 )			/* dupeAction:  action FSCopyObject will take			*/
			dupeAction = argv[j][0];
		else if( strcmp("-s", argv[i]) == 0 )		/* sourcePath:  path to the source object				*/
			srcPath = (unsigned char*) argv[j];
		else if( strcmp("-d", argv[i]) == 0 )		/* destDirPath: path to the destination directory		*/
			dstPath = (unsigned char*) argv[j];		/*				required for copying but not deleting	*/
		else if( strcmp("-n", argv[i]) == 0 )		/* name:		name of the new object, optional		*/
			nameStr = (unsigned char*) argv[j];
		else if( strcmp("-help", argv[i]) == 0 )	/* help:		prints out the FSCopyObject usage text	*/
			err = kNeedsHelpErr;
		else
			err = kInvalidArgumentErr;
	}

	if( err == noErr )						/* Get FSRef to the source object	*/
		err = MyFSPathMakeRef( srcPath, &sourceRef );
	if( err == noErr && dupeAction != 'd' )	/* Get FSRef to destrination object	*/
	{										/* if we are copying				*/
		FSCatalogInfo	tmpCatInfo;			
		
			/* We don't have to worry about the symlink problem (2489632) here	*/
			/* cause we would want to copy into the target of the symlink		*/
			/* anyways.  And if its not a symlink, no problems...				*/
		err = FSPathMakeRef( dstPath, &destRef, NULL );
		if( err == noErr )					/* make sure the dest is a directory*/
			err = FSGetCatalogInfo( &destRef, kFSCatInfoNodeFlags, &tmpCatInfo, NULL, NULL, NULL);
		if( err == noErr && (tmpCatInfo.nodeFlags & kFSNodeIsDirectoryMask) == 0 )
			err = kInvalidDestinationErr;
	}
	if( err == noErr && nameStr != NULL )
	{
		uniNamePtr = &uniName;
		err = ConvertCStringToHFSUniStr( (const char*)nameStr, &uniName );
	}
	if( err == noErr )
	{
		switch( dupeAction )
		{
			case	'c':
				TestFSCopyObject( kDupeActionStandard, uniNamePtr, &sourceRef, &destRef );	break;
			case	'R':
				TestFSCopyObject( kDupeActionReplace,  uniNamePtr, &sourceRef, &destRef );	break;
			case	'r':
				TestFSCopyObject( kDupeActionRename,   uniNamePtr, &sourceRef, &destRef );	break;
			case	'd':
				TestFSDeleteObjects( &sourceRef );											break;
			default:
				err = kInvalidArgumentErr;													break;
		}
	}

	if( err == kNeedsHelpErr || err == kInvalidArgumentErr || err == kInvalidDestinationErr )
	{
		if( err == kInvalidArgumentErr )
			printf( "%s -- Invalid arguments\n", argv[0] );
		else if( err == kInvalidDestinationErr )
			printf( "%s -- The destrination MUST be a directory\n", argv[0] );

		printf( "FSCopyObject 1.5\n\n" );
		printf( "Usage:\tFSCopyObject -a dupeAction -s sourcePath [-d destDirPath] [-n name] [-help]\n" );
		printf( "\tdupeAction:  action FSCopyObject will take\n" );
		printf( "\t\tc -- run FSCopyObject with kDupeActionStandard\n" );
		printf( "\t\tR -- run FSCopyObject with kDupeActionReplace\n" );
		printf( "\t\tr -- run FSCopyObject with kDupeActionRename\n" );
		printf( "\t\td -- run FSDeleteObjects\n" );
		printf( "\tsourcePath:  path to the source object\n" );
		printf( "\tdestDirPath: path to the destination directory, required for copying but not deleting\n" );
		printf( "\tname: optional name for the new object\n" );
		printf( "\thelp: prints out info on FSCopyObject\n\n" );
		printf( "Ex:\tFSCopyObject -a R -s /Applications/iTunes.app -d ~/Desktop/ -n MyITunes.app\n\n" );
	}
	else if( err != noErr )
	{
		printf( "%s -- An error occurred: %d\n\n", argv[0], err );
	}

    return 0;
}

static void TestFSCopyObject( DupeAction dupeAction, const HFSUniStr255 *name, const FSRef *sourceRef, const FSRef *destRef )
{
	FSRef			newRef;
	OSErr			osErr;
	
	osErr = FSCopyObject( sourceRef, destRef, 0, kFSCatInfoNone, 
							  dupeAction, name, true, false, MyFilterProc, NULL, &newRef, NULL);
	
	printf( "%s returned: %d\n", __FUNCTION__, osErr );
}

static void TestFSDeleteObjects( const FSRef *sourceRef )
{
	OSErr		osErr;
	
	osErr = FSDeleteObjects( sourceRef );
		
	printf( "%s returned: %d\n", __FUNCTION__, osErr );
}

static pascal OSErr MyFilterProc(	Boolean				containerChanged,
									ItemCount			currentLevel,
									OSErr				currentOSErr,
									const FSCatalogInfo	*catalogInfo,
									const FSRef			*ref,
									const FSSpec		*spec,
									const HFSUniStr255	*name,
									void				*yourDataPtr )
{	
#pragma unused ( containerChanged, catalogInfo, ref, name, yourDataPtr )

	FSSpec	tmpSpec = *spec;
	
	/* If an error occured during the copy, you can process that error here			*/
	/* or, for example, you can also process key combo's (i.e. command-'.') to		*/
	/* cancel copies in	progress (i.e. return userCanceledErr)						*/
	/* Note: If currentOSErr != noErr, the FSRef and other info might not be valid	*/

	if( currentOSErr == noErr )
	{
		tmpSpec.name[ tmpSpec.name[0] + 1 ] = '\0';
		printf( "%d  %s: %d\n", (int)currentLevel, tmpSpec.name + 1,currentOSErr );
	} 
	
	/* if noErr: continue with the copy						  */
	/* else bail											  */
	return currentOSErr;
}

	/* Due to a bug in the X File Manager, 2489632,				*/
	/* FSPathMakeRef doesn't handle symlinks properly.  It		*/
	/* automatically resolves it and returns an FSRef to the	*/
	/* symlinks target, not the symlink itself.  So this is a	*/
	/* little workaround for it...								*/
	/*															*/
	/* We could call lstat() to find out if the object is a		*/
	/* symlink or not before jumping into the guts of the		*/
	/* routine, but its just as simple, and fast when working	*/
	/* on a single item like this, to send everything through	*/
	/* this routine												*/
static OSErr MyFSPathMakeRef( const unsigned char *path, FSRef *ref )
{
	FSRef			tmpFSRef;
	char			tmpPath[ MAX_PATH ],
					*tmpNamePtr;
	OSErr			err;
	
					/* Get local copy of incoming path					*/
	strcpy( tmpPath, (char*)path );

					/* Get the name of the object from the given path	*/
					/* Find the last / and change it to a '\0' so		*/
					/* tmpPath is a path to the parent directory of the	*/
					/* object and tmpNamePtr is the name				*/
	tmpNamePtr = strrchr( tmpPath, '/' );
	if( *(tmpNamePtr + 1) == '\0' )
	{				/* in case the last character in the path is a /	*/
		*tmpNamePtr = '\0';
		tmpNamePtr = strrchr( tmpPath, '/' );
	}
	*tmpNamePtr = '\0';
	tmpNamePtr++;
	
					/* Get the FSRef to the parent directory			*/
	err = FSPathMakeRef( (unsigned char*)tmpPath, &tmpFSRef, NULL );
	if( err == noErr )
	{				
					/* Convert the name to a Unicode string and pass it	*/
					/* to FSMakeFSRefUnicode to actually get the FSRef	*/
					/* to the object (symlink)							*/
		HFSUniStr255	uniName;
		err = ConvertCStringToHFSUniStr( tmpNamePtr, &uniName );
		if( err == noErr )
			err = FSMakeFSRefUnicode( &tmpFSRef, uniName.length, uniName.unicode, kTextEncodingUnknown, &tmpFSRef );
	}
	
	if( err == noErr )
		*ref = tmpFSRef;
	
	return err;
}


static OSErr ConvertCStringToHFSUniStr(const char* cStr, HFSUniStr255 *uniStr)
{
	OSErr err = noErr;
	CFStringRef tmpStringRef = CFStringCreateWithCString( kCFAllocatorDefault, cStr, kCFStringEncodingMacRoman );
	if( tmpStringRef != NULL )
	{
		if( CFStringGetCString( tmpStringRef, (char*)uniStr->unicode, sizeof(uniStr->unicode), kCFStringEncodingUnicode ) )
			uniStr->length = CFStringGetLength( tmpStringRef );
		else
			err = kCouldNotGetStringData;
			
		CFRelease( tmpStringRef );
	}
	else
		err = kCouldNotCreateCFString;
	
	return err;
}
