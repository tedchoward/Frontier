
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

#include "CallMachOFrameWork.h"
	

OSStatus LoadFrameworkBundle(CFStringRef framework, CFBundleRef *bundlePtr) /*Adapted from Apple sample code*/
{
	OSStatus 	err;
	FSRef 		frameworksFolderRef;
	CFURLRef	baseURL;
	CFURLRef	bundleURL;
	
	assert(bundlePtr != nil);
	
	*bundlePtr = nil;
	
	baseURL = nil;
	bundleURL = nil;
	
	err = FSFindFolder(kOnAppropriateDisk, kFrameworksFolderType, true, &frameworksFolderRef);
	if (err == noErr) {
		baseURL = CFURLCreateFromFSRef(kCFAllocatorSystemDefault, &frameworksFolderRef);
		if (baseURL == nil) {
			err = coreFoundationUnknownErr;
		}
	}
	if (err == noErr) {
		bundleURL = CFURLCreateCopyAppendingPathComponent(kCFAllocatorSystemDefault, baseURL, framework, false);
		if (bundleURL == nil) {
			err = coreFoundationUnknownErr;
		}
	}
	if (err == noErr) {
		*bundlePtr = CFBundleCreate(kCFAllocatorSystemDefault, bundleURL);
		if (*bundlePtr == nil) {
			err = coreFoundationUnknownErr;
		}
	}
	if (err == noErr) {
	    if ( ! CFBundleLoadExecutable( *bundlePtr ) ) {
			err = coreFoundationUnknownErr;
	    }
	}

	// Clean up.
	
	if (err != noErr && *bundlePtr != nil) {
		CFRelease(*bundlePtr);
		*bundlePtr = nil;
	}
	if (bundleURL != nil) {
		CFRelease(bundleURL);
	}	
	if (baseURL != nil) {
		CFRelease(baseURL);
	}	
	
	return err;
} /*LoadFrameworkBundle*/


void *getframeworkfuncptr (CFStringRef framework, CFStringRef functionname) {

	/*
	2005-01-15 aradke: give us the name of a framework and the name of a function
	in the framework and we will retrieve a callable pointer to that function.
	*/
	
	CFBundleRef bundlePtr;
	
	 if (LoadFrameworkBundle (framework, &bundlePtr) != noErr)
		return (nil);
	
	return (CFBundleGetFunctionPointerForName (bundlePtr, functionname));
	} /*getframeworkfuncptr*/


static UInt32 gluetemplate[6] = {0x3D800000, 0x618C0000, 0x800C0000, 0x804C0004, 0x7C0903A6, 0x4E800420};

void *convertcfmtomachofuncptr (void *cfmfp) {
	
	/*
	2004-11-28 aradke: adapted from Apple's CFM_MachO_CFM sample:
	
	http://developer.apple.com/samplecode/CFM_MachO_CFM/CFM_MachO_CFM.html
	
	allocate a block of cfm glue code with instructions for calling the CFM function.
	the caller is responsible for disposal via disposemachofuncptr 
	*/

    UInt32 *mfp = (UInt32*) NewPtr (sizeof(gluetemplate));

    mfp[0] = gluetemplate[0] | ((UInt32) cfmfp >> 16);
    mfp[1] = gluetemplate[1] | ((UInt32) cfmfp & 0xFFFF);
    mfp[2] = gluetemplate[2];
    mfp[3] = gluetemplate[3];
    mfp[4] = gluetemplate[4];
    mfp[5] = gluetemplate[5];

    MakeDataExecutable (mfp, sizeof(gluetemplate));

    return ((void *) mfp);
	} /*convertcfmtomachofuncptr*/


void disposemachofuncptr (void *mfp) {
	
	if (mfp != nil)
		DisposePtr (mfp);
	} /*disposemachofuncptr*/


typedef struct tvectorstruct {
	ProcPtr	procaddr;
	UInt32	toc;
	} tvectorstruct, *tvectorptr;


void *convertmachotocfmfuncptr (void *mfp) {
	
	/*
	2004-11-28 aradke: allocate a fake TVector and set its procptr entry
	to the mach procptr handed to us by the caller. the toc entry can
	safely be set to nil since it's ignored in this context.
	*/
	
    tvectorptr cfmfp = (tvectorptr) NewPtr (sizeof(tvectorstruct));
	
	if (MemError()  == noErr && cfmfp != nil) {
		
		cfmfp->procaddr = (ProcPtr) mfp;
		cfmfp->toc = 0; /*ignored*/
		}

    return ((void *) cfmfp);
	} /*convertmachotocfmfuncptr*/


void disposecfmfuncptr (void *cfmfp) {
	
	if (cfmfp != nil)
		DisposePtr (cfmfp);
	} /*disposecfmfuncptr*/
