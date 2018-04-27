
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


	# include "mac.h"
	

#include "filealias.h"
#include "cursor.h"
#include "dialogs.h"
#include "error.h"
#include "memory.h"
#include "ops.h"
#include "quickdraw.h"
#include "resources.h"
#include "scrap.h"
#include "strings.h"
#include "threads.h"
#include "frontierwindows.h"
#include "file.h"
#include "shell.h"
#include "shell.rsrc.h"
#include "langinternal.h" /*for langbackgroundtask*/
#include "versions.h" /* 2005-09-23 creedon */



	#define sfgetfileid 5000
	#define sfputfileid 5001
	#define sfgetfolderid 5002
	#define sfgetdiskid 5003

	//Code change by Timothy Paustian Tuesday, June 20, 2000 2:22:02 PM
	//Nav services code for Frontier.
	
	static OSErr macputfiledialog (bigstring, ptrfilespec fs, OSType);

	static OSErr macgetfiledialog (SInt16 dialogtype, bigstring prompt, ptrfilespec fs, OSType filecreator, ptrsftypelist filetypes); /* 2005-09-23 creedon */ 






boolean sfdialog ( tysfverb sfverb, bigstring bsprompt, ptrsftypelist filetypes, ptrfilespec fspec, OSType filecreator ) {

	//
	// return true if the user selected a file with one of the SF routines, return false otherwise.
	//
	

		// 2010-03-14 aradke: pick up default location from fspec, type filtering checks extension if file type not set 
		//
		// 2009-09-20 aradke: discontinue use of FSSpecs and "un-hack" this routine (see 2000-06-20)
		//
		// 2006-11-04 creedon: fix a problem with returning gibberish if fs was pointing to a volume
		//
		// 2006-09-15 creedon: minimally FSRef-ized
		//
		// 2005-10-06 creedon: added filecreator parameter, used for get file dialog
		//
		// 2005-09-21 creedon: changed from TimsGetFile to macgetfiledialog
		//
		// 2000-06-20 Timothy Paustian: I am going to hack the heck out of this routine. See what you think
		// 
		// 4.1b13 dmb: un-commented out the shellwritescrap and shellactive calls.
		// I don't know how they got commented; I must have been experimenting with 
		// something.
		//
		// 2.1b2 dmb: updated interface to be filespec-based. for putfile, the name in the 
		// filespec is the default name. for all verbs, a non-empty filespec seeds the dialog
		//
		// 2.1b1 dmb: set sfdata.sfname to an impossible value to ensure button update
		//
		// 6/11/93 dmb: recoded to System 7 Standard File
		//
		// 12/18/92 dmb: call shellwritescrap
		//
		// 9/15/92 dmb: when picking a folder, grab path from sfstring instead of reply
		// record.
		//
		// 2/10/92 dmb: added call to new shellactivate; standard file breaks when brought 
		// up in the background.
		//
		// 12/27/91 dmb: if the default fname includes a colon, attempt to set default 
		// directory, and remove folder specification from dialog default
		//
		// 4/16/91 dmb: replaced filetype parameter with pointer to type list.  callers 
		// can pass nil when irrelevant, or when all types are to be shown.
		// 
		// 11/16/90 dmb: replace flput boolean with sfverb enum.  added code for folder 
		// and disk selection dialogs
		//
		// 8/1/90 dmb:  if filetype is zero, show all files by passing zero to SFGetFile
		//

		OSErr err = noErr;
		OSType filetype = 0;
		
		if ( filetypes != NULL )
			filetype = ( *filetypes ).types [ 0 ];
		
		shellwritescrap (textscraptype);

		shellactivate ();

		switch (sfverb) {
			
			case sfputfileverb:
				err = macputfiledialog (bsprompt, fspec, filetype);
				break;
			
			case sfgetfileverb:
				err = macgetfiledialog (sfgetfileid, bsprompt, fspec, filecreator, filetypes);
				break;
			
			case sfgetfolderverb:
				err = macgetfiledialog (sfgetfolderid, bsprompt, fspec, kNavGenericSignature, NULL);
				break;
			
			case sfgetdiskverb:
				err = macgetfiledialog (sfgetdiskid, bsprompt, fspec, kNavGenericSignature, NULL);
				break;
			}

		// if the user canceled return false, I know that oserror can handle this, but lets make it obvious that we are
		// checking.
		
		if (userCanceledErr == err) 
			return false;

		if (oserror (err)) 
			return false;

		return (true);
		


	} // sfdialog



	boolean initfiledialog (void) {

		
		
		
		return (true);
		} /*initfile*/


	typedef struct {
		ptrfilespec fslocation;
		ptrsftypelist filtertypes;
		OSType filecreator;
	} navcallbackcontext;


	pascal static void macnaveventcallback(NavEventCallbackMessage cbselector, NavCBRecPtr cbparams, void* refcon) {
			
		// 2010-03-13 aradke: set initial location of file navigation dialog
		
		switch (cbselector) {
				
			case kNavCBStart: {
				
				navcallbackcontext* context = (navcallbackcontext*) refcon;
				ptrfilespec fs = context->fslocation;
				FSRef fsref;
				
				if (macgetfsref (fs, &fsref) != noErr) {
					
					fsref = fs->ref;	// try parent for putfile
					}

				if (noErr == FSGetCatalogInfo(&fsref, kFSCatInfoNone, NULL, NULL, NULL, NULL)) {
					
					OSErr err;
					AEDesc desc;
					
					err = AECreateDesc (typeFSRef, &fsref, sizeof(FSRef), &desc);
				
					if (err == noErr)
						err = NavCustomControl (cbparams->context, kNavCtlSetLocation, (void*)&desc);
					}
				}
			}/*switch*/
		} /*macfiledialogeventcallback*/


	pascal static boolean macnavfiltercallback(AEDesc *item, void* info, void* refcon, NavFilterModes mode) {

		// 2010-03-13 aradke: filter by file creator, type and extension.
		//		file.type(f) returns the extension if no actual file type is set.
		//		with this callback, the file will be shown if the specified list of
		//		file types contains file.type(f).
		
		if (mode == kNavFilteringBrowserList && item->descriptorType == typeFSRef) {
			
			FSRef fsref;
			LSItemInfoRecord iteminfo;
			boolean flcontainer, flapp, flpackage;
			OSErr err;
			
			err = AEGetDescData (item, &fsref, sizeof(FSRef));
			
			if (err != noErr)
				return (false);
			
			err = LSCopyItemInfoForRef (&fsref, kLSRequestAllInfo, &iteminfo);
			
			if (err != noErr)
				return (false);

			flcontainer = ((iteminfo.flags & kLSItemInfoIsContainer) != 0);
			flapp = ((iteminfo.flags & kLSItemInfoIsApplication) != 0);
			flpackage = ((iteminfo.flags & kLSItemInfoIsPackage) != 0);

			if (flcontainer && !flpackage && !flapp) {
					
				return (true);	// always show plain folders
				}
			
			navcallbackcontext* context = (navcallbackcontext*) refcon;
			OSType filtercreator = context->filecreator;
			ptrsftypelist filtertypes = context->filtertypes;
			
			if (filtercreator != kNavGenericSignature)
				if (iteminfo.creator != filtercreator)
					return (false);

			if (filtertypes == NULL || ((filtertypes->cttypes > 0) && (filtertypes->types[0] == kUnknownType)))
				return (true);
			
			OSType filetype = iteminfo.filetype;
			
			if (filetype == kLSUnknownType) {	// try file extension
				
				if (iteminfo.extension == NULL)
					return (false);
				
				bigstring bsext;
				
				cfstringreftobigstring (iteminfo.extension, bsext);
				
				filetype = '    ';	// init to four spaces in case actual extension is shorter
				
				stringtoostype (bsext, &filetype);
				}

			for (short k = 0; k < filtertypes->cttypes; ++k) {
				
				if ((filetype == filtertypes->types[k]) || (filtertypes->types[k] == kNavGenericSignature))
					return (true);
				}
			}
		
		return (false);
		} /*macnavfiltercallback*/


	static OSErr macputfiledialog (bigstring prompt, ptrfilespec fs, OSType filetype) {
		
		// 2009-09-20 aradke: discontinue use of FSSpecs
		//
		// Code change by Timothy Paustian Tuesday, June 20, 2000 2:55:17 PM.  New routine to use Nav services for this
		// instead of CustomPutFile.
		
		NavDialogCreationOptions options;
		NavDialogRef dialog;
		NavEventUPP eventupp;
		navcallbackcontext context;
		NavReplyRecord reply;
		OSErr err = noErr;
		boolean flfolder = false;	// default
		
		if ( filetype == 0 )
			filetype = 'TEXT';
		
		err = NavGetDefaultDialogCreationOptions (&options);
		
		if (err != noErr)
			return (err);
		
		options.clientName = CFStringCreateWithCString (kCFAllocatorDefault, APPNAME_SHORT, kCFStringEncodingMacRoman);

    if (prompt != nil) {
      options.message = CFStringCreateWithPascalString (kCFAllocatorDefault, prompt, kCFStringEncodingMacRoman);
    } else {
      options.message = CFSTR("");
    }
		
		disablelangerror();
		
		fileisfolder(fs, &flfolder);
		
		enablelangerror();
		
		if (!flfolder) {
			options.saveFileName = CFStringCreateWithCharacters (kCFAllocatorDefault, fs->name.unicode, fs->name.length);
			}
		else {
			bigstring bsuntitled;
			
			setemptystring (bsuntitled);
			
			getuntitledfilename (bsuntitled);
			
			options.saveFileName = CFStringCreateWithPascalString (kCFAllocatorDefault, bsuntitled, kCFStringEncodingMacRoman);
			}
		
		options.optionFlags |= kNavNoTypePopup; // 2000-08-25 AR
		
		context.fslocation = fs;
		context.filtertypes = NULL;
		context.filecreator = kNavGenericSignature;
		
		eventupp = NewNavEventUPP(&macnaveventcallback);
		
		err = NavCreatePutFileDialog (&options, filetype, 'LAND', eventupp, &context, &dialog);
		
		if (err == noErr) {
				
			err = NavDialogRun(dialog);
			
			if (err == noErr) {
			
				err = NavDialogGetReply (dialog, &reply);
				
				if (err == noErr) {
				
					if (reply.validRecord) { // get fsref and name to make filespec
					
						AEKeyword key;
						DescType actualtype;
						Size actualsize;
						FSRef parentref;
						
						err = AEGetNthPtr (&(reply.selection), 1, typeFSRef, &key, &actualtype, &parentref, sizeof (parentref), &actualsize);
						
						if (err == noErr) {
						
							assert (actualtype == typeFSRef);
							
							fs->flags.flvolume = false;
							
							fs->ref = parentref;
							
							fs->name.length = CFStringGetBytes (reply.saveFileName, CFRangeMake (0, min(CFStringGetLength (reply.saveFileName), 255)), 
																kCFStringEncodingUnicode, 0, false, (UInt8 *)(fs->name.unicode), 255, NULL );
							}
						}
					else {
						err = userCanceledErr;
						}
					
					NavDisposeReply (&reply);
					}
				}

			NavDialogDispose (dialog);
			}
		
		DisposeNavEventUPP(eventupp);
		
		CFRelease (options.clientName);
		CFRelease (options.message);
		CFRelease (options.saveFileName);

		return (err);
		
		} // macputfiledialog


	static OSErr macgetfiledialog (SInt16 dialogtype, bigstring prompt, ptrfilespec fs, OSType filecreator, ptrsftypelist filetypes) {

		// 2009-09-20 aradke: discontinue use of FSSpecs, merge with getfolderorvolumedialog
		//
		// 2005-10-06 creedon: added filecreator parameter
		//
		// 2005-09-21 creedon: created, cribbed from TimsGetFile

		NavDialogCreationOptions options;
		NavDialogRef dialog;
		NavEventUPP eventupp;
		NavObjectFilterUPP filterupp;
		navcallbackcontext context;
		NavReplyRecord reply;
		//NavTypeListHandle typeList = NULL;
		OSErr err;

		err = NavGetDefaultDialogCreationOptions (&options);
		
		if (err != noErr)
			return (err);
		
		options.clientName = CFStringCreateWithCString (kCFAllocatorDefault, APPNAME_SHORT, kCFStringEncodingMacRoman);
		
		options.message = CFStringCreateWithPascalString (kCFAllocatorDefault, prompt, kCFStringEncodingMacRoman );
		
		options.optionFlags &= ~kNavAllowMultipleFiles;		// no multiple files for now
	   
		options.optionFlags &= ~kNavAllowPreviews;			// clear preview option
	   
		options.optionFlags |= kNavSupportPackages;			// show packages
		
		options.optionFlags |= kNavNoTypePopup;				// disable type pop-up

		// options.dialogOptionFlags |= kNavAllowOpenPackages; // can open packages
		
		eventupp = NewNavEventUPP(&macnaveventcallback);
		filterupp = NewNavObjectFilterUPP(&macnavfiltercallback);
		
		context.fslocation = fs;
		context.filtertypes = filetypes;
		context.filecreator = filecreator;
		
		if (dialogtype == sfgetfileid) { // translate into a type list NavServices understands
		
			if ((filetypes != NULL) || (filecreator != kNavGenericSignature)) {
				
				/*
				NavTypeListPtr typesP = nil;
				SInt32 hSize = (sizeof (NavTypeList) + sizeof (OSType) * (filetypes->cttypes - 1));
				
				newhandle (hSize, (Handle*) &typeList);
				
				typesP = (NavTypeListPtr) *((Handle) typeList);
				
				typesP->componentSignature = filecreator;
				typesP->reserved = 0;
				typesP->osTypeCount = filetypes->cttypes;
				
				BlockMoveData (&(filetypes->types), typesP->osType, (Size) (sizeof (OSType) * filetypes->cttypes));
				
				options.optionFlags |= kNavAllFilesInPopup;			// add "All Files" to pop-up
				options.optionFlags |= kNavSelectAllReadableItem;	// select "All Readable Items" in pop-up
				options.optionFlags &= ~kNavNoTypePopup;			// re-enable type pop-up

				err = NavCreateGetFileDialog (&options, typeList, eventupp, NULL, filterupp, &context, &dialog);
				*/
				
				err = NavCreateGetFileDialog (&options, NULL, eventupp, NULL, filterupp, &context, &dialog);
				}
			else {

				err = NavCreateGetFileDialog (&options, NULL, eventupp, NULL, NULL, &context, &dialog);
				}
			}
		else if (dialogtype == sfgetfolderid) {
				
			err = NavCreateChooseFolderDialog (&options, eventupp, NULL, &context, &dialog);
			}
		else {
			assert (dialogtype == sfgetdiskid);
			
			err = NavCreateChooseVolumeDialog (&options, eventupp, NULL, &context, &dialog);
			}

		if (err == noErr) {
		
			err = NavDialogRun (dialog);
			
			if (err == noErr) {
			
				err = NavDialogGetReply (dialog, &reply);
				
				if (err == noErr) {

					if (reply.validRecord) {
					
						AEKeyword key;
						DescType actualtype;
						Size actualsize;
						FSRef fsref;
						
						err = AEGetNthPtr (&(reply.selection), 1, typeFSRef, &key, &actualtype, &fsref, sizeof (fsref), &actualsize);
						
						if (err == noErr) {
						
							assert (actualtype == typeFSRef);
							
							err = macmakefilespec (&fsref, fs);
							}
						}
					else {
						err = userCanceledErr;
						}

					NavDisposeReply (&reply); // dispose of NavReplyRecord, resources, descriptors
					}
				}
		
			NavDialogDispose (dialog);
			}
		
		DisposeNavEventUPP(eventupp);
		DisposeNavObjectFilterUPP(filterupp);
		
		CFRelease (options.clientName);
		CFRelease (options.message);
		
		//disposehandle ((Handle) typeList);
	 
		return (err);
		} /* macgetfiledialog */

