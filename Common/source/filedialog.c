
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

	# include "mac.h"
	# include "MoreFilesX.h"
	
#endif

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
#include "filealias.h"


#ifdef MACVERSION

	#define sfgetfileid 5000
	#define sfputfileid 5001
	#define sfgetfolderid 5002
	#define sfgetdiskid 5003

	//Code change by Timothy Paustian Tuesday, June 20, 2000 2:22:02 PM
	//Nav services code for Frontier.
	
	static OSErr macputfiledialog (bigstring, ptrfilespec fs, OSType);

	static OSErr macgetfiledialog (SInt16 dialogtype, bigstring prompt, ptrfilespec fs, OSType filecreator, ptrsftypelist filetypes); /* 2005-09-23 creedon */ 


	#if ! TARGET_API_MAC_CARBON

		static pascal Boolean knowntypesfilter (ParmBlkPtr pb, tysfdata *pdata) {
			
			short i;
			
			if (pb->fileParam.ioFlAttrib & ioDirMask)
				return (0);
			
			if (pdata->sftypes == nil) // show all files
				return (0);
			
			for (i = 0; i < pdata->sftypes->cttypes; ++i) {
				
				OSType type = pdata->sftypes->types [i];
				byte bstype [6];
				bigstring bssuffix;
				
				ostypetostring (type, bstype);
				
				lastword (pb->fileParam.ioNamePtr, '.', bssuffix);
				
				if (stringlength (bssuffix) == 3) //handle 8.3 names
					setstringlength (bstype, 3);
				
				if (equalidentifiers (bssuffix, bstype))
					return (0);
				
				if (pb->fileParam.ioFlFndrInfo.fdType == type)
					return (0);
				}
			
			return (-1); // didn't find it in our list
			} /*knowntypesfilter*/

				
		#if !TARGET_RT_MAC_CFM
			
			#define knowntypesfilterUPP ((FileFilterYDUPP) &knowntypesfilter)

		#else

			#if !TARGET_API_MAC_CARBON
			static RoutineDescriptor knowntypesfilterDesc = BUILD_ROUTINE_DESCRIPTOR (uppFileFilterYDProcInfo, knowntypesfilter);

			#define knowntypesfilterUPP (&knowntypesfilterDesc)
			
			#endif

		#endif

	#endif /* !TARGET_API_MAC_CARBON */

#endif

#ifdef WIN95VERSION

	static void buildfilter (char * filter, short * len, bigstring bsname, bigstring bsext) {
		short namelen, extlen;

		namelen = stringlength (bsname);
		extlen = stringlength (bsext);

		memmove (filter + *len, stringbaseaddress(bsname), namelen);
		*len = *len + namelen;
		memmove (filter + *len, "\0", 1);
		*len = *len + 1;
		memmove (filter + *len, stringbaseaddress(bsext), extlen);
		*len = *len + extlen;
		memmove (filter + *len, "\0", 1);
		*len = *len + 1;
		memmove (filter + *len, "\0", 1);  //alway finish the filter but don't count it in the length
		} /*buildfilter*/

#endif


boolean sfdialog ( tysfverb sfverb, bigstring bsprompt, ptrsftypelist filetypes, ptrfilespec fspec, OSType filecreator ) {

	//
	// return true if the user selected a file with one of the SF routines, return false otherwise.
	//
	
	#ifdef MACVERSION

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
		
	#endif // MACVERSION

	#ifdef WIN95VERSION

		#pragma unused ( filecreator )
		
		/*
		2005-10-06 creedon: added filecreator, unused on Windows
		
		5.0.2b4 dmb: fixed bug in above change that would generate an error for empty paths

		5.0.1 dmb: make sure default file and directory are valid, or we'll fail (silently)
		*/

		TCHAR szFile[MAX_PATH];
		OPENFILENAME OpenFileName;
		BROWSEINFO BrowseInfo;
		LPITEMIDLIST itemList;
		char title [256];
		char filter [1024];
		short filterlen = 0;
		char defaultdir [256];
		char defaultfile [256];
		bigstring extension;
		byte type [6];
		bigstring osstring;
		boolean fl = false;
		short i;
		boolean fldatabases = false;
		boolean flfatpages = false;
		// Global pointer to the shell's IMalloc interface.  
		static LPMALLOC pMalloc = NULL;


		OpenFileName.lStructSize       = sizeof(OPENFILENAME);
		OpenFileName.hwndOwner         = shellframewindow;
		OpenFileName.hInstance         = shellinstance;
		OpenFileName.lpstrFilter       = NULL;
		OpenFileName.lpstrCustomFilter = NULL;
		OpenFileName.nMaxCustFilter    = 0;
		OpenFileName.nFilterIndex      = 0;
		OpenFileName.lpstrFile		   = stringbaseaddress(fsname (fspec));
		OpenFileName.nMaxFile          = sizeof(fsname (fspec)) - 2;
		OpenFileName.lpstrFileTitle    = NULL;
		OpenFileName.nMaxFileTitle     = 0;
		OpenFileName.lpstrTitle			= title;
		OpenFileName.lpstrInitialDir	= NULL;
		OpenFileName.lpstrTitle			= NULL;
		OpenFileName.nFileOffset       = 0;
		OpenFileName.nFileExtension    = 0;
		OpenFileName.lpstrDefExt       = NULL;
		OpenFileName.lCustData         = (LPARAM)NULL;
		OpenFileName.lpfnHook 		   = NULL;
		OpenFileName.lpTemplateName    = 0;

		strcpy (szFile, "");
		
		// set the title
		if (bsprompt != NULL) {
			
			copyptocstring (bsprompt, title);
			
			OpenFileName.lpstrTitle = title;
			}

		// set default dir, file and extension fields
		if (!isemptystring (fsname (fspec))) {

			tyfilespec fsdir;
			boolean flfolder;

			folderfrompath (fsname (fspec), defaultdir);
			
			if (pathtofilespec (defaultdir, &fsdir) && 
				fileexists (&fsdir, &flfolder) && flfolder) {
				
				OpenFileName.lpstrInitialDir = defaultdir;
				
				//if (!isemptystring (defaultdir))
				//	OpenFileName.nFileOffset = stringlength (defaultdir) + 1;
				
				convertpstring (defaultdir);
				}
			
			filefrompath (fsname (fspec), defaultfile);

			lastword (defaultfile, ':', defaultfile); //skip any Mac path 

			//OpenFileName.nFileExtension = stringlength (fsname (fspec));
			
			lastword (defaultfile, '.', extension);
			
			//if (stringlength (extension) < stringlength (fsname (fspec)))
			//	OpenFileName.nFileExtension -= stringlength (extension);
			
			copystring (defaultfile, fsname (fspec));

			nullterminate (fsname (fspec));
			}

		releasethreadglobals ();
		
		switch (sfverb) {
			
			case sfputfileverb:
				OpenFileName.Flags = OFN_SHOWHELP | OFN_EXPLORER | OFN_OVERWRITEPROMPT; //| OFN_NOCHANGEDIR;
				
				if (filetypes != nil) {
					setemptystring (filter);

					//RAB: 1/22/98 use string since the windows type can be any case
					ostypetostring ((*filetypes).types [0], osstring);

					if (equalidentifiers (osstring, BIGSTRING ("\x04" "fatp")))
						copystring (BIGSTRING ("\x016" "Fat Page [*.fatp]\0*.*\0"), filter);

					else if (equalidentifiers (osstring, BIGSTRING ("\x04" "ftop")))
						copystring (BIGSTRING ("\x015" "Outline [*.ftop]\0*.*\0"), filter);

					else if (equalidentifiers (osstring, BIGSTRING ("\x04" "ftwp")))
						copystring (BIGSTRING ("\x019" "WP Document [*.ftwp]\0*.*\0"), filter);

					else if (equalidentifiers (osstring, BIGSTRING ("\x04" "fttb")))
						copystring (BIGSTRING ("\x013" "Table [*.fttb]\0*.*\0"), filter);

					else if (equalidentifiers (osstring, BIGSTRING ("\x04" "ftmb")))
						copystring (BIGSTRING ("\x012" "Menu [*.ftmb]\0*.*\0"), filter);

					else if (equalidentifiers (osstring, BIGSTRING ("\x04" "ftsc")))
						copystring (BIGSTRING ("\x014" "Script [*.ftsc]\0*.*\0"), filter);

					else if (equalidentifiers (osstring, BIGSTRING ("\x04" "ftds")))
						copystring (BIGSTRING ("\x01c" "Desktop Script [*.ftds]\0*.*\0"), filter);

					else if (equalidentifiers (osstring, BIGSTRING ("\x04" "root")))
						copystring (BIGSTRING ("\x016" "Database [*.root]\0*.*\0"), filter);

					if (! isemptystring (filter)) {
						convertpstring (filter);

						OpenFileName.lpstrFilter = filter;
						}

					ostypetostring ((*filetypes).types [0], type);
					
					popleadingchars (type, '.');

					poptrailingwhitespace (type);
						
	//				convertpstring (type);
					
	//				OpenFileName.lpstrDefExt = type;
					OpenFileName.lpstrDefExt = NULL;
					}

				// Call the common dialog function.
				fl = GetSaveFileName (&OpenFileName);

				if (fl && (filetypes != NULL) && (stringlength(type) > 0)) {

					setstringlength (fsname (fspec), strlen(stringbaseaddress(fsname (fspec))));
					lastword (fsname (fspec), '.', extension);

					if ((stringlength (fsname (fspec)) == stringlength (extension)) || (stringlength (extension) > 4)) {	/* no extension */
						pushstring ("\x01.", fsname(fspec));
						pushstring (type, fsname(fspec));
						nullterminate (fsname (fspec));
						}
					}
			
				break;
			
			case sfgetfileverb:
				OpenFileName.Flags = OFN_SHOWHELP | OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST; //| OFN_NOCHANGEDIR;

				// create the filter string
				if (filetypes != nil) { /*showing specific files*/

					//copystring ("\x09Types: [\0", filter);
					setemptystring (extension);

					for (i = 0; i < (*filetypes).cttypes; ++i) {
						
						if (i > 0)
							pushchar (';', extension);
						
						if ((*filetypes).types [i] == 'root')
							fldatabases = true;

						if ((*filetypes).types [i] == 'fatp')
							flfatpages = true;

						if ((*filetypes).types [i] == 'ROOT')
							fldatabases = true;

						if ((*filetypes).types [i] == 'FATP')
							flfatpages = true;

						ostypetostring ((*filetypes).types [i], type);
						
						popleadingchars (type, '.');

						poptrailingwhitespace (type);
						
						pushstring (BIGSTRING ("\x02" "*."), extension);

						pushstring (type, extension);

						if (stringlength (type) > 3) {
							
							setstringlength (type, 3);

							pushstring (BIGSTRING ("\x03" ";*."), extension);
							
							pushstring (type, extension);
							}
						}
					
					filterlen = 0;

					buildfilter (filter, &filterlen, BIGSTRING ("\x1c" "Openable Types: [*.clickers]"), extension);
					
					if (fldatabases)
						buildfilter (filter, &filterlen, BIGSTRING ("\x12" "Databases [*.root]"), BIGSTRING ("\x0c" "*.root;*.roo"));

					//					pushstring ("\x20\0Databases [*.root]\0*.roo;*.root", filter);

					if (flfatpages) {
						buildfilter (filter, &filterlen, BIGSTRING ("\x12" "Fat Pages [*.fatp]"), BIGSTRING ("\x5a" "*.fatp;*.fat;*.FTsc;*.FTs;*.FTwp;*.FTw;*.FTop;*.FTo;*.FTmb;*.FTm;*.FTtb;*.FTt;*.Ftds;*.FTd"));
						buildfilter (filter, &filterlen, BIGSTRING ("\x17" "Frontier Menus [*.FTmb]"),    BIGSTRING ("\x0c" "*.FTmb;*.FTm"));
						buildfilter (filter, &filterlen, BIGSTRING ("\x1a" "Frontier Outlines [*.FTop]"), BIGSTRING ("\x0c" "*.FTop;*.FTo"));
						buildfilter (filter, &filterlen, BIGSTRING ("\x19" "Frontier Scripts [*.FTsc]"),  BIGSTRING ("\x0c" "*.FTsc;*.FTs"));
						buildfilter (filter, &filterlen, BIGSTRING ("\x21" "Frontier Desktop Scripts [*.FTds]"),  BIGSTRING ("\x0c" "*.FTds;*.FTd"));
						buildfilter (filter, &filterlen, BIGSTRING ("\x18" "Frontier Tables [*.FTtb]"),	  BIGSTRING ("\x0c" "*.FTtb;*.FTt"));
						buildfilter (filter, &filterlen, BIGSTRING ("\x19" "Frontier WP Text [*.FTwp]"),  BIGSTRING ("\x0c" "*.FTwp;*.FTw"));
						}

					//					pushstring ("\x20\0Fat Pages [*.fatp]\0*.fat;*.fatp", filter);

					buildfilter (filter, &filterlen, BIGSTRING ("\x12" "Plain Text [*.txt]"),  BIGSTRING ("\x12" "*.Text;*.txt;*.tex"));
					buildfilter (filter, &filterlen, BIGSTRING ("\x0f" "All Files [*.*]"),  BIGSTRING ("\x03" "*.*"));

					OpenFileName.lpstrFilter = filter;

					ostypetostring ((*filetypes).types [0], type);
					
					//RAB: 1/22/98  added next two lines
					//				poptrailingwhitespace is no longer done in ostypetostring.
					popleadingchars (type, '.');

					poptrailingwhitespace (type);

					convertpstring (type);
					
					OpenFileName.lpstrDefExt = type;
					}

				// Call the common dialog function.
				fl = GetOpenFileName (&OpenFileName);
			
				break;
			
			case sfgetfolderverb:
			case sfgetdiskverb:
				// Get the shell's allocator. 
				if (pMalloc == NULL && !SUCCEEDED(SHGetMalloc(&pMalloc))) 
					break; 
	 
				BrowseInfo.hwndOwner = OpenFileName.hwndOwner;
				BrowseInfo.pidlRoot = NULL;
				BrowseInfo.pszDisplayName = szFile;
				BrowseInfo.lpszTitle = OpenFileName.lpstrTitle;
				BrowseInfo.ulFlags = BIF_RETURNONLYFSDIRS;
				if (sfverb == sfgetdiskverb)
					BrowseInfo.ulFlags |= BIF_RETURNFSANCESTORS;
				BrowseInfo.lpfn = NULL;
				BrowseInfo.lParam = 0;
				
				itemList = SHBrowseForFolder (&BrowseInfo);
				
				if (itemList != NULL) {

					fl = SHGetPathFromIDList (itemList, szFile);

					copyctopstring (szFile, fsname (fspec));

					if (sfverb == sfgetdiskverb) {

						firstword (fsname (fspec), ':', fsname (fspec));
					
						pushstring ("\x02:\\", fsname (fspec));
						}
					else {

						pushstring ("\x01\\", fsname (fspec));
						}
					
					nullterminate (fsname (fspec));

					// deallocate itemList
					pMalloc->lpVtbl->Free (pMalloc, itemList);

			/*
			LPSHELLFOLDER ppshf;
			if (SHGetDesktopFolder (&ppshf) == NOERROR) {
				ULONG ctchars;
				WCHAR szWide [300];

				MultiByteToWideChar (CP_ACP, 0, szFile, -1, szWide, 300);

				ppshf->lpVtbl->ParseDisplayName (ppshf, NULL, szWide, &ctchars, &itemList,NULL);

				fl = SHGetPathFromIDList (itemList, szFile);

				pMalloc->lpVtbl->Free (pMalloc, itemList);

				ppshf->lpVtbl->Release (ppshf);
				}
			*/
					}
				
				break;
			
			}
		
		grabthreadglobals ();
		
		if (!fl) {
			
			oserror (GetLastError ());
			
			return (false);
			}
		
		setstringlength (fsname (fspec), strlen(stringbaseaddress(fsname (fspec))));
		
		return (true);

	#endif // WIN95VERSION

	} // sfdialog


#ifdef MACVERSION

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
				
				if (FSRefValid (&fsref)) {
					
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

		options.message = CFStringCreateWithPascalString (kCFAllocatorDefault, prompt, kCFStringEncodingMacRoman);
		
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
		
		if ((dialogtype == sfgetfileid)) { // translate into a type list NavServices understands
		
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

#endif
