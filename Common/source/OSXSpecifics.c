
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

#include "OSXSpecifics.h"

/* OSXSpecifics.c
	
	This file and its header represent a one-stop shop for setting up OSX specific things.
	The goal is to only need to call the setup procedure once, and have the function pointers
	available for the rest of the time Frontier is running. I'll hone this down over time,
	but for right now, this is good enough.
	
*/


//		if (strcmp((char*)*par->params[0], "true")==0) useQDText(1);
//		else useQDText(0);



static OSStatus LoadFrameworkBundle(CFStringRef framework, CFBundleRef *bundlePtr)
{
	OSStatus 	err;
	FSRef 		frameworksFolderRef;
	CFURLRef	baseURL;
	CFURLRef	bundleURL;
	
	if ( bundlePtr == nil )	return( -1 );
	
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
}


void useQDText(int i)
{
	OSStatus 			err;
	CFBundleRef 		sysBundle;
	UInt32 				newFlags = 0;
	char				s[64];
	QuartzTextPtr		QuartzText;

	
	s[0]=0;
	err = LoadFrameworkBundle( CFSTR("ApplicationServices.framework"), &sysBundle );
	if (err == noErr) {
		QuartzText = CFBundleGetFunctionPointerForName(sysBundle, CFSTR("SwapQDTextFlags") );
		if (QuartzText != nil) {
			//newFlags = kQDUseCGTextRendering | kQDUseCGTextMetrics;
			if (i == 1) err = QuartzText(kQDUseCGTextRendering);
			else {
				err = QuartzText(0);
				err = 0;
			}
		}
	}
}
