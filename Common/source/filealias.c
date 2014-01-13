
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

/******************************************************************************
**
**  Project Name:	LDRProjects
**     File Name:	MAF.c
**
**   Description:	New C routine for creating alias files given source &
**					destination FSSpecs
**
*******************************************************************************
**                       A U T H O R   I D E N T I T Y
*******************************************************************************
**
**	Initials	Name
**	--------	-----------------------------------------------
**	LDR			Leonard Rosenthol
**	LSR			Larry Rosenstein
**  TCH         Ted C. Howard
**
*******************************************************************************
**                      R E V I S I O N   H I S T O R Y
*******************************************************************************
**
**	  Date		Time	Author	Description
**	----------	-----	------	---------------------------------------------
**  01/13/2013	15:45	TCH		Rewrote to use Apple Events. No other 
**								supported way to make alias files.
**	09/03/1991	21:01	LDR		Installed latest version from LSR
**	09/02/1991	20:36	LDR		Moved into separate file
**						LSR		Original version
**
******************************************************************************/

#include "frontier.h"
#include "standard.h"

#include "shelltypes.h"
#include "filealias.h"
#include "file.h"
#include "error.h"

#include "AppleEventUtils.h"

enum {
	kFinderFileType			= 'FNDR',
	kFinderCreatorType		= 'MACS',
	kFinderProcessType		= 'FNDR',
	kFinderProcessSignature	= 'MACS'
};

// Forward declarations
static OSErr finderCreateAliasFile(const FSRefPtr sourceFile, const FSRefPtr destFile, bigstring newfilename);
	
boolean MakeAliasFile (const ptrfilespec srcFile, const ptrfilespec destFile) {
	OSErr err = noErr;
	FSRef sourceFileRef; // FSRef of target file
	FSRef destFileRef; // FSRef of parent folder of new alias file
	bigstring destFileName; // name of new alias file
	
	macgetfsref(srcFile, &sourceFileRef);
	
	destFileRef = destFile->ref; 
	
	getfsfile(destFile, destFileName);
	
	err = finderCreateAliasFile(&sourceFileRef, &destFileRef, destFileName);
	
	return !oserror(err);
}

// Sends an Apple Event to the Finder requesting it create an alias for the given file
static OSErr finderCreateAliasFile(const FSRefPtr sourceFile, const FSRefPtr destFile, bigstring newfilename) {
	OSErr err = noErr;
	AEIdleUPP idleProcUPP = createIdleUPP();
	
	AppleEvent appleEvent = { typeNull, nil };
	
	err = createAppleEventSignatureTarget(kFinderFileType, kFinderCreatorType, kAECoreSuite, kAECreateElement, &appleEvent);
	
	if (noErr == err) {
		OSType aliasType = typeAlias;
		
		err = AEPutParamPtr(&appleEvent, keyAEObjectClass, typeType, &aliasType, sizeof (OSType));
		if (noErr == err) {
			err = addAliasParameterFromFSRef(sourceFile, keyASPrepositionTo, &appleEvent);
			if (noErr == err) {
				err = addAliasParameterFromFSRef(destFile, keyAEInsertHere, &appleEvent);
				if (noErr == err) {
					AERecord properties = { typeNull, nil };
					
					err = AECreateList(nil, 0, true, &properties);
					if (noErr == err) {
						err = AEPutKeyPtr(&properties, keyAEName, typeChar, newfilename+1, *newfilename);
						if (noErr == err) {
							err = AEPutParamDesc(&appleEvent, keyAEPropData, &properties);
							if (noErr == err) {
								err = sendEventReturnVoid(idleProcUPP, &appleEvent);
							}
						}
						
						disposeAEDesc(&properties);
					}
				}
			}
		}
		
		(void)disposeAEDesc(&appleEvent);
	}
	
	DisposeAEIdleUPP(idleProcUPP);
	
	return err;
}